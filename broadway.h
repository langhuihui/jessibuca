#pragma once
#include "H264SwDecApi.h"
class Broadway :public VideoDecoder{
    public:
    H264SwDecInst decInst;
	H264SwDecInput decInput;
	H264SwDecOutput decOutput;
	H264SwDecPicture decPicture;
	H264SwDecInfo decInfo;
	u32 picDecodeNumber;
	u32 picDisplayNumber;
    Broadway()
	{
        H264SwDecRet ret;
		u32 disableOutputReordering = 0;
		ret = H264SwDecInit(&decInst, disableOutputReordering);
		if (ret != H264SWDEC_OK) {
			emscripten_log(0, "DECODER INITIALIZATION FAILED\n");
			return;
		}
		picDecodeNumber = picDisplayNumber = 1;
		emscripten_log(0, "H264 init");
    }
    ~Broadway()
	{
        if(decInst){
			H264SwDecRelease(decInst);
			decInst = nullptr;
		}
    }
    void _decode(const char* data, int len) override{
		u8 naluType = data[0] & 0x1f;
		if(!decInst||naluType!=1||naluType!=5)return;
        decInput.pStream = (u8*)data;
        decInput.dataLen = len;
        u32 i = 0;
        do {
            u8 *start = decInput.pStream;
            u32 ret = broadwayDecode();
            //emscripten_log(0,"Decoded Unit #%d, Size: %x, Result: %i\n", i++, (decInput.pStream - start), ret);
        } while (decInput.dataLen > 0);
    }
    u32 broadwayDecode() {
	
		H264SwDecRet ret = H264SwDecDecode(decInst, &decInput, &decOutput);

		switch (ret) {
		case H264SWDEC_HDRS_RDY_BUFF_NOT_EMPTY:
			/* Stream headers were successfully decoded, thus stream information is available for query now. */
			ret = H264SwDecGetInfo(decInst, &decInfo);
			if (ret != H264SWDEC_OK) {
				return -1;
			}

			decodeVideoSize(decInfo.picWidth, decInfo.picHeight);
			/*picSize = decInfo.picWidth * decInfo.picHeight;
			picSize = (3 * picSize) / 2;
			printf("picSize:%d", picSize);*/
			//broadwayOnHeadersDecoded();

			decInput.dataLen -= decOutput.pStrmCurrPos - decInput.pStream;
			decInput.pStream = decOutput.pStrmCurrPos;
			break;

		case H264SWDEC_PIC_RDY_BUFF_NOT_EMPTY:
			/* Picture is ready and more data remains in the input buffer,
			* update input structure.
			*/
			decInput.dataLen -= decOutput.pStrmCurrPos - decInput.pStream;
			decInput.pStream = decOutput.pStrmCurrPos;

			/* fall through */

		case H264SWDEC_PIC_RDY:
			if (ret == H264SWDEC_PIC_RDY) {
				decInput.dataLen = 0;
			}

			/* Increment decoding number for every decoded picture */
			picDecodeNumber++;
			while (H264SwDecNextPicture(decInst, &decPicture, 0) == H264SWDEC_PIC_RDY) {
				//printf(" Decoded Picture Decode: %d, Display: %d, Type: %s\n", picDecodeNumber, picDisplayNumber, decPicture.isIdrPicture ? "IDR" : "NON-IDR");
				/* Increment display number for every displayed picture */
				picDisplayNumber++;
				p_yuv[0] = (u32)decPicture.pOutputPicture;
				p_yuv[1] = p_yuv[0] + decInfo.picWidth*decInfo.picHeight;
				p_yuv[2] = p_yuv[1] + (decInfo.picWidth*decInfo.picHeight >> 2);
				decodeYUV420();
			}
			break;

		case H264SWDEC_STRM_PROCESSED:
		case H264SWDEC_STRM_ERR:
			/* Input stream was decoded but no picture is ready, thus get more data. */
			decInput.dataLen = 0;
			break;
		default:
			emscripten_log(0, "video decode %d", -ret);
			decInput.dataLen = 0;
			break;
		}
		return ret;
	}
}