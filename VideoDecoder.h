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
	int NAL_unit_length;
	bool webgl;
	
	VideoDecoder():heap(nullptr), webgl(false),NAL_unit_length(0),videoWidth(0),videoHeight(0)
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
		if(!webgl&&heap)free(heap);
#ifdef USE_H265
		de265_free_decoder(h265DecContext);
#else
		if(decInst){
			H264SwDecRelease(decInst);
			decInst = nullptr;
		}
#endif
	emscripten_log(0, "video decoder release!\n");
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
	void decodeHeader(MemoryStream& data, int codec_id){
		int index = data.offset;
		emscripten_log(0, "codec = %d", codec_id);
		if(codec_id==7){
			u8 lengthSizeMinusOne = data[9];
			lengthSizeMinusOne &= 0x03;
			NAL_unit_length = lengthSizeMinusOne;
			data.offset = 11+ index;
			//data.consoleHex();
			int spsLen = 0;
			int ppsLen = 0;
			data.read2B(spsLen);
			if (spsLen > 0) {
				_decode((const char*)data,spsLen);
				data >>= spsLen;
			}
			data >>= 1;
			data.read2B(ppsLen);
			if (ppsLen > 0) {
				_decode((const char*)data,ppsLen);
			}
			
		}else if(codec_id==12){
			u8 lengthSizeMinusOne = data[27];
			lengthSizeMinusOne &= 0x03;
			NAL_unit_length = lengthSizeMinusOne;
			data.offset = 31 + index;
			//data.consoleHex();
			int vps=0,sps=0,pps=0;
			data.read2B(vps);
			_decode((const char*)data,vps);
			data>>=vps;
			data>>=3;
			data.read2B(sps);
			_decode((const char*)data,sps);
			data>>=sps;
			data>>=3;
			data.read2B(pps);
			_decode((const char*)data,pps);
		}
	}
	void decode(MemoryStream& data) {
		int NALUnitLength = 0;
		data >>= 5;
		while (data.length() > 4) {
			switch (NAL_unit_length) {
			case 3:
				data.read4B(NALUnitLength);
				break;
			case 2:
				data.read3B(NALUnitLength);
				break;
			case 1:
				data.read2B(NALUnitLength);
				break;
			default:
				data.read1(NALUnitLength);
			}
			#ifdef USE_H265
			//emscripten_log(0, "NALUnitLength %d",NALUnitLength);
			_decode((const char *)data, NALUnitLength);
			#else
			u8 naluType = data[0] & 0x1f;
			switch (naluType) {
			case 5:
			case 1:
				_decode((const char *)data, NALUnitLength);
				break;
			}
			#endif
			data >>= NALUnitLength;
		}
	}
	void _decode(const char* data, int len) {
		//emscripten_log(0, "%d %d",len,(int)data);
#ifdef USE_H265
		de265_push_NAL(h265DecContext, data, len, 0, nullptr);
		int more = 1;
		while (more) {
			more = 0;
			auto err = de265_decode(h265DecContext, &more);
			if (err != DE265_OK) {
				//emscripten_log(0, "%d", err);
				break;
			}
			const de265_image* img = de265_get_next_picture(h265DecContext);
			if (img) {
				//emscripten_log(0, "%d", img);
				int out_stride;
				//u8* y = (u8*)de265_get_image_plane(img, 0, &out_stride);
				//u8* u = (u8*)de265_get_image_plane(img, 1, &out_stride);
				//u8* v = (u8*)de265_get_image_plane(img, 2, &out_stride);
				for(int i=0;i<3;i++)
					p_yuv[i] = (u32)de265_get_image_plane(img, i, &out_stride);
				//videoWidth = de265_get_image_width(img, 0);
				//videoHeight = de265_get_image_height(img, 0);
				if(videoWidth==0)
				decodeVideoSize(de265_get_image_width(img, 0),de265_get_image_height(img, 0));
				decodeYUV420();
			}
		}
#else
			if(!decInst)return;
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