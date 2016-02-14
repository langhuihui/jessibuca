#pragma once
#include "Broadway\basetype.h"
#include "Broadway\H264SwDecApi.h"
#include <speex\speex.h>
#include <time.h>
class NetConnection;
int yuv2rgbcalc(int y,int u,int v) {

	int a0 = 1192 * (y - 16);
	int a1 = 1634 * (v - 128);
	int a2 = 832 * (v - 128);
	int a3 = 400 * (u - 128);
	int a4 = 2066 * (u - 128);

	int r = (a0 + a1) >> 10 ;
	int g = (a0 - a2  - a3) >> 10 ;
	int b = (a0 + a4)  >> 10 ;

	if ((r & 255)  != r ) {
		r = min(255, max(0, r ) ) ;
	}
	if ((g & 255)  != g )  {
		g = min(255, max(0, g ) ) ;
	}
	if ((b & 255) != b ) {
		b = min(255, max(0, b ) ) ;
	}
	return (((((255 << 8) + b) << 8) + g) << 8) + r;
};
class NetStream
{
private:
	H264SwDecInst decInst;
	H264SwDecInput decInput;
	H264SwDecOutput decOutput;
	H264SwDecPicture decPicture;
	H264SwDecInfo decInfo;
	u32 picDecodeNumber;
	u32 picDisplayNumber;
	u32 picSize;
	u8* heap;
	i16* audioOutput;
	int audioBufferLength;
	u8* audioBuffer;
	int audioBufferCounts;
	void* speexState;
	SpeexBits speexBits;
	bool canvasAttached;
	void broadwayOnPictureDecoded(u8* buffer,u32 width,u32 height) {
		if (!jsThis)return;
		auto outSize = width * height << 2;
		if (webgl) {
			if (!heap) {
				heap = buffer;
				jsThis->call<void>("setVideoSize", width, height, (int)heap);
			}
			jsThis->call<void>("draw",(int)buffer);
		}
		else {
			if (!heap) {
				
				emscripten_log(0, "canvas:%d,%d", width,height);
				//auto lumaSize = width * height;
				//auto chromaSize = lumaSize >> 2;
				//auto inpSize = lumaSize + chromaSize + chromaSize;
				auto cacheSize = 0x2000000;
				auto size = outSize + cacheSize;//inpSize + outSize + cacheSize;
				auto chunkSize = 0x1000000;
				auto heapSize = chunkSize;
				while (heapSize < size) {
					heapSize += chunkSize;
				};
				heap = (u8*)malloc(heapSize);
				jsThis->call<void>("setVideoSize", width, height, (int)heap);
			};
			int ystart = 0;
			int ustart = 0;
			int vstart = 0;

			int y = 0;
			int yn = 0;
			int u = 0;
			int v = 0;

			u32* o = 0;

			int line = 0;
			int col = 0;

			int usave = 0;
			int vsave = 0;

			int ostart = 0;
			int cacheAdr = 0;
			ustart = ystart + width * height;
			vstart = ustart +( width * height>>2);
			u8* cacheStart = heap+outSize;
			for (line = 0; line < height; line+=2) {
				usave = ustart;
				vsave = vstart;
				for (col = 0; col < width; col +=2) {
					y = buffer[ystart];
					yn = buffer[ystart + width];

					u = buffer[ustart];
					v = buffer[vstart];

					cacheAdr = (y << 16) + (u << 8)+ v ;
					o =(u32*)(cacheStart + cacheAdr);
					if (!*o) *o = yuv2rgbcalc(y, u, v);
					*(u32*)(heap+ostart) = *o;

					cacheAdr = ((yn << 16) + (u << 8)) + v;
					o = (u32*)(cacheStart + cacheAdr);
					if (!*o) *o = yuv2rgbcalc(yn, u, v);
					*(u32*)(heap + (ostart + (width << 2))) = *o;
					
					//yuv2rgb5(y, u, v, ostart);
					//yuv2rgb5(yn, u, v, (ostart + widthFour)|0);
					ostart+=4;

					// next step only for y. u and v stay the same
					ystart++;
					y = buffer[ystart];
					yn = buffer[(ystart + width)];

					//yuv2rgb5(y, u, v, ostart);
					cacheAdr = (y << 16) + (u << 8)+ v ;
					o = (u32*)(cacheStart + cacheAdr);
					if (!*o) *o = yuv2rgbcalc(y, u, v);
					*(u32*)(heap + ostart) = *o;

					//yuv2rgb5(yn, u, v, (ostart + widthFour)|0);
					cacheAdr = ((yn << 16) + (u << 8)) + v;
					o = (u32*)(cacheStart + cacheAdr);
					if (!*o) *o = yuv2rgbcalc(yn, u, v);
					*(u32*)(heap + (ostart + (width << 2))) = *o;
					ostart+=4;

					//all positions inc 1

					ystart ++;
					ustart ++;
					vstart ++;
				};
				ostart += (width << 2);
				ystart += width;

			};
			jsThis->call<void>("draw");
		}
	}
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

			picSize = decInfo.picWidth * decInfo.picHeight;
			picSize = (3 * picSize) / 2;

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
				// printf(" Decoded Picture Decode: %d, Display: %d, Type: %s\n", picDecodeNumber, picDisplayNumber, decPicture.isIdrPicture ? "IDR" : "NON-IDR");
				/* Increment display number for every displayed picture */
				picDisplayNumber++;
				broadwayOnPictureDecoded((u8*)decPicture.pOutputPicture, decInfo.picWidth, decInfo.picHeight);
			}
			break;

		case H264SWDEC_STRM_PROCESSED:
		case H264SWDEC_STRM_ERR:
			/* Input stream was decoded but no picture is ready, thus get more data. */
			decInput.dataLen = 0;
			break;
		default:
			emscripten_log(0, "%d",-ret);
			decInput.dataLen = 0;
			break;
		}
		return ret;
	}
	
public:
	NetConnection *nc;
	bool isFirstVideoReceived = true;
	byte NAL_unit_length;
	int spsLen = 0;
	int ppsLen = 0;
	//string sps;
	//string pps;
	bool webgl = false;
	val* jsThis;
	int id;
	clock_t timestamp;
	int timeoutId;
	vector<RtmpPacket> videoBuffers;
	NetStream(NetConnection* _nc);

	~NetStream()
	{
		if(heap!=NULL)free(heap);
		if (audioBuffer != NULL)free(audioBuffer);
		if (audioOutput != NULL)free(audioOutput);
		speex_decoder_destroy(speexState);
		speex_bits_destroy(&speexBits);
		if (jsThis != nullptr)delete jsThis;
	}
	void onConnect(val&& result);
	void play(string name);
	void attachCanvas(val _this,bool webgl) {
		this->webgl = webgl;
		canvasAttached = true;
		emscripten_log(0, "webgl:%s", webgl ? "true" : "false");
		//consoleLog("webgl:%s",webgl?"true":"false");
		if (!jsThis)jsThis = new val(_this);
	}
	void _decodeVideo(const char* data,int len) {
		//emscripten_log(0, "%d %d",len,(int)data);
		decInput.pStream = (u8*)data;
		decInput.dataLen = len;
		u32 i = 0;
		do {
			u8 *start = decInput.pStream;
			u32 ret = broadwayDecode();
			//emscripten_log(0,"Decoded Unit #%d, Size: %x, Result: %i\n", i++, (decInput.pStream - start), ret);
		} while (decInput.dataLen > 0);
	}
	inline void decodeVideo(MemoryStream& data) {
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
			byte naluType = data[0] & 0x1f;
			switch (naluType) {
			case 5:
			case 1:
				_decodeVideo((const char *)data, NALUnitLength);
				break;
			}
			data >>= NALUnitLength;
		}
	}
	void decodeVideoBuffer() {
		timeoutId = 0;
		if (videoBuffers.size() > 0) {
			decodeVideo(videoBuffers[videoBuffers.size()-1].data);
			videoBuffers.pop_back();
			if (videoBuffers.size() > 0) {
				int time = videoBuffers[videoBuffers.size() - 1].rtmpHeader.timestamp;
				auto targetTime = clock() / 1000 - timestamp;

				if (time > targetTime) {
					timeoutId = jsThis->call<int>("checkVideoBuffer", time - targetTime);
					return;
				}
				else {
					decodeVideoBuffer();
				}
			}
		}
	}
	bool decodeVideo(RtmpHeader& rtmpHeader, MemoryStream& data) {
		
		int index = 0;
		byte frame_type = data[0];
		int codec_id = frame_type & 0x0f;
		frame_type = (frame_type >> 4) & 0x0f;
		if (codec_id != 7) {
			emscripten_log(0, "Only support video h.264/avc codec. actual=%d", codec_id);
			return -1;
		}
		byte avc_packet_type = data[1];
		if (data[0] == 0x17 && avc_packet_type == 0) {
			if (isFirstVideoReceived) {
				timestamp = clock() / 1000 - rtmpHeader.timestamp;
				byte lengthSizeMinusOne = data[9];
				lengthSizeMinusOne &= 0x03;
				NAL_unit_length = lengthSizeMinusOne;
				data.offset = 11;
				data.consoleHex();
				data.read2B(spsLen);
				if (spsLen > 0) {
					_decodeVideo((const char*)data,spsLen);
					data >>= spsLen;
				}
				data >>= 1;
				data.read2B(ppsLen);
				if (ppsLen > 0) {
					_decodeVideo((const char*)data,ppsLen);
				}
				isFirstVideoReceived = false;
			}
		}
		else {
			if (!canvasAttached)return false;
			auto targetTime = clock()/1000 - timestamp;
			//consoleLog("%d %d", rtmpHeader.timestamp,targetTime);
			if (rtmpHeader.timestamp > targetTime) {
				videoBuffers.insert(videoBuffers.begin(), RtmpPacket(rtmpHeader,data));
				if(timeoutId == 0)
					timeoutId = jsThis->call<int>("checkVideoBuffer", rtmpHeader.timestamp - targetTime);
				return false;
			}
			
			decodeVideo(data);
		}
		return true;
	}
	bool decodeAudio(RtmpHeader& rtmpHeader,MemoryStream & data) {
		data.offset = 1;
		if (data.length() <= 11) {
			memset(audioBuffer + audioBufferCounts * 640,0,640);
		}
		else {
			speex_bits_read_from(&speexBits, (const char *)data, 52);
			speex_decode_int(speexState, &speexBits, audioOutput);
			memcpy(audioBuffer + audioBufferCounts * 640, audioOutput, 640);
		}
		audioBufferCounts++;
		if (audioBufferCounts == audioBufferLength) {
			audioBufferCounts = 0;
			jsThis->call<void>("playAudio");
		}
		return true;
	}
	int initAudio(val _this,int bufferFrames) {
		audioBufferLength = bufferFrames;
		if(!jsThis)jsThis =new val(_this);
		auto mode = speex_lib_get_mode(SPEEX_MODEID_WB);
		speexState = speex_decoder_init(mode);
		speex_bits_init(&speexBits);
		audioOutput = (i16*)malloc(640);
		audioBufferCounts = 0;
		audioBuffer = (u8*)malloc(640* bufferFrames);
		return (int)audioBuffer;
	}
};

