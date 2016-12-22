#pragma once

struct VideoPacket
{
	clock_t timestamp;
	MemoryStream data;
	VideoPacket(clock_t t, MemoryStream&& data) :timestamp(t), data(data)
	{

	}
	VideoPacket() :timestamp(0), data()
	{

	}
};
class VideoDecoder
{
public:
	val* jsObject;
#ifdef USE_H265
	de265_PTS pts;
	de265_decoder_context* h265DecContext;
#else
	H264SwDecInst decInst;
	H264SwDecInput decInput;
	H264SwDecOutput decOutput;
	H264SwDecPicture decPicture;
	H264SwDecInfo decInfo;
	u32 picDecodeNumber;
	u32 picDisplayNumber;
#endif
	u8* heap;
	
	u32 videoWidth;
	u32 videoHeight;
	u32 p_yuv[3];
	
	bool webgl;
	
	VideoDecoder():heap(nullptr), webgl(false)
	{
#ifdef USE_H265
		h265DecContext = de265_new_decoder();
		emscripten_log(0, "H265 init");
#else
		H264SwDecRet ret;
		u32 disableOutputReordering = 0;
		ret = H264SwDecInit(&decInst, disableOutputReordering);
		if (ret != H264SWDEC_OK) {
			emscripten_log(0, "DECODER INITIALIZATION FAILED\n");
			return;
		}
		picDecodeNumber = picDisplayNumber = 1;
		emscripten_log(0, "H264 init");
#endif
	}
	~VideoDecoder()
	{
		if(heap)free(heap);
#ifdef USE_H265
		de265_free_decoder(h265DecContext);
#else
		H264SwDecRelease(&decInst);
#endif

	}
	void decodeVideoSize(u32 width, u32 height)
	{
		videoWidth = width;
		videoHeight = height;
		emscripten_log(0, "canvas:%d,%d", videoWidth, videoHeight);
		if (webgl) {
			heap = (u8*)p_yuv;
			jsObject->call<void>("setVideoSize", videoWidth, videoHeight, (int)heap >> 2);
		}
		else {
			auto outSize = videoHeight * videoHeight << 2;
			auto cacheSize = 0x2000000;
			auto size = outSize + cacheSize;
			auto chunkSize = 0x1000000;
			auto heapSize = chunkSize;
			while (heapSize < size) {
				heapSize += chunkSize;
			}
			heap = (u8*)malloc(heapSize);
			jsObject->call<void>("setVideoSize", videoWidth, videoHeight, (int)heap);
		}
	}
	void decodeYUV420()
	{
		if (!webgl) {
			yuv420toRGB((u8*)p_yuv[0], (u8*)p_yuv[1], (u8*)p_yuv[2], heap, videoWidth, videoHeight);
		}
		jsObject->call<void>("draw");
	}
#ifndef USE_H265
	u32 broadwayDecode() {
		decInput.picId = picDecodeNumber;

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
			emscripten_log(0, "%d", -ret);
			decInput.dataLen = 0;
			break;
		}
		return ret;
	}
#endif
	void decode(const char* data, int len) {
		//emscripten_log(0, "%d %d",len,(int)data);
#ifdef USE_H265
		de265_push_NAL(h265DecContext, data, len, 0, nullptr);
		int more = 1;
		while (more) {
			more = 0;
			auto err = de265_decode(h265DecContext, &more);
			if (err != DE265_OK) {
				emscripten_log(0, "%d", err);
				break;
			}
			const de265_image* img = de265_get_next_picture(h265DecContext);
			if (img) {
				int out_stride;
				//u8* y = (u8*)de265_get_image_plane(img, 0, &out_stride);
				//u8* u = (u8*)de265_get_image_plane(img, 1, &out_stride);
				//u8* v = (u8*)de265_get_image_plane(img, 2, &out_stride);
				for(int i=0;i<3;i++)
					p_yuv[i] = (u32)de265_get_image_plane(img, i, &out_stride);
				videoWidth = de265_get_image_width(img, 0);
				videoHeight = de265_get_image_height(img, 0);
				decodeYUV420();
				de265_release_next_picture(h265DecContext);
			}
		}
#else
		decInput.pStream = (u8*)data;
		decInput.dataLen = len;
		u32 i = 0;
		do {
			u8 *start = decInput.pStream;
			u32 ret = broadwayDecode();
			//emscripten_log(0,"Decoded Unit #%d, Size: %x, Result: %i\n", i++, (decInput.pStream - start), ret);
		} while (decInput.dataLen > 0);
#endif
	}
};