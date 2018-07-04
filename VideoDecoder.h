#pragma once

struct VideoPacket
{
	clock_t timestamp;
	MemoryStream data;
	VideoPacket(clock_t t, MemoryStream &&data) : timestamp(t), data(forward<MemoryStream>(data))
	{
	}
	VideoPacket() : timestamp(0), data()
	{
	}
};
class VideoDecoder
{
  public:
	val *jsObject;
	u8 *heap;
	u32 videoWidth;
	u32 videoHeight;
	u32 p_yuv[3];
	int NAL_unit_length;
	bool webgl;

	VideoDecoder() : heap(nullptr), webgl(false), NAL_unit_length(0), videoWidth(0), videoHeight(0)
	{
	}
	virtual ~VideoDecoder()
	{
		if (!webgl && heap)
			free(heap);
		emscripten_log(0, "video decoder release!\n");
	}
	virtual void clear()
	{
		if (!webgl && heap)
			free(heap);
	}
	void decodeVideoSize(u32 width, u32 height)
	{
		videoWidth = width;
		videoHeight = height;
		emscripten_log(0, "canvas:%d,%d", videoWidth, videoHeight);
		if (webgl)
		{
			heap = (u8 *)p_yuv;
			jsObject->call<void>("setVideoSize", videoWidth, videoHeight, (int)heap >> 2);
		}
		else
		{
			auto outSize = videoHeight * videoHeight << 2;
			auto cacheSize = 0x2000000;
			auto size = outSize + cacheSize;
			auto chunkSize = 0x1000000;
			auto heapSize = chunkSize;
			while (heapSize < size)
			{
				heapSize += chunkSize;
			}
			heap = (u8 *)malloc(heapSize);
			jsObject->call<void>("setVideoSize", videoWidth, videoHeight, (int)heap);
		}
	}
	void decodeYUV420()
	{
		if (!webgl)
		{
			yuv420toRGB((u8 *)p_yuv[0], (u8 *)p_yuv[1], (u8 *)p_yuv[2], heap, videoWidth, videoHeight);
		}
		jsObject->call<void>("draw");
	}

	virtual void decodeHeader(MemoryStream &data, int codec_id)
	{
		int index = data.offset;
		emscripten_log(0, "codec = %d", codec_id);
		if (codec_id == 7)
		{
			u8 lengthSizeMinusOne = data[9];
			lengthSizeMinusOne &= 0x03;
			NAL_unit_length = lengthSizeMinusOne;
			data.offset = 11 + index;
			//data.consoleHex();
			int spsLen = 0;
			int ppsLen = 0;
			data.read2B(spsLen);
			if (spsLen > 0)
			{
				_decode((const char *)data, spsLen);
				data >>= spsLen;
			}
			data >>= 1;
			data.read2B(ppsLen);
			if (ppsLen > 0)
			{
				_decode((const char *)data, ppsLen);
			}
		}
		else if (codec_id == 12)
		{
			u8 lengthSizeMinusOne = data[27];
			lengthSizeMinusOne &= 0x03;
			NAL_unit_length = lengthSizeMinusOne;
			data.offset = 31 + index;
			//data.consoleHex();
			int vps = 0, sps = 0, pps = 0;
			data.read2B(vps);
			_decode((const char *)data, vps);
			data >>= vps;
			data >>= 3;
			data.read2B(sps);
			_decode((const char *)data, sps);
			data >>= sps;
			data >>= 3;
			data.read2B(pps);
			_decode((const char *)data, pps);
		}
	}
	virtual void decode(MemoryStream &data)
	{
		int NALUnitLength = 0;
		while (data.length() > 4)
		{
			switch (NAL_unit_length)
			{
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
			_decode((const char *)data, NALUnitLength);
			data >>= NALUnitLength;
		}
	}
	virtual void _decode(const char *data, int len) = 0;
};