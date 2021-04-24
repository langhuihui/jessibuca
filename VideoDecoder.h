#pragma once

struct VideoPacket
{
	clock_t timestamp;
	IOBuffer data;
	bool isKeyFrame;
	VideoPacket(clock_t t, IOBuffer data) : timestamp(t), data(data)
	{
		isKeyFrame = data[0] >> 4 == 1;
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
	u32 compositionTime;
	u32 timestamp;
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
		//emscripten_log(0, "canvas:%d,%d", videoWidth, videoHeight);
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
		jsObject->call<void>("draw", compositionTime, timestamp);
	}

	virtual void decodeH264Header(IOBuffer &data)
	{
		u8 lengthSizeMinusOne = data[4];
		lengthSizeMinusOne &= 0x03;
		NAL_unit_length = lengthSizeMinusOne;
		data >>= 6;
		//data.consoleHex();
		int spsLen = 0;
		int ppsLen = 0;
		data.read2B(spsLen);
		if (spsLen > 0)
		{
			_decode(data(0, spsLen));
			data >>= spsLen;
		}
		data >>= 1;
		data.read2B(ppsLen);
		if (ppsLen > 0)
		{
			_decode(data(0, ppsLen));
		}
	}
	virtual void decodeH265Header(IOBuffer &data)
	{
		u8 lengthSizeMinusOne = data[22];
		lengthSizeMinusOne &= 0x03;
		NAL_unit_length = lengthSizeMinusOne;
		data >>= 26;
		//data.consoleHex();
		int vps = 0, sps = 0, pps = 0;
		data.read2B(vps);
		_decode(data(0, vps));
		data >>= vps + 3;
		data.read2B(sps);
		_decode(data(0, sps));
		data >>= sps + 3;
		data.read2B(pps);
		_decode(data(0, pps));
	}
	bool isAVCSequence(IOBuffer &data)
	{
		return data[0] >> 4 == 1 && data[1] == 0; //0为AVCSequence Header，1为AVC NALU，2为AVC end ofsequence
	}
	virtual void decode(IOBuffer &data)
	{
		if (isAVCSequence(data))
		{
			int codec_id = data[0] & 0x0F;
			data >>= 5;
			//emscripten_log(0, "codec = %d", codec_id);
			switch (codec_id)
			{
			case 7:
				decodeH264Header(data);
				break;
			case 12:
				decodeH265Header(data);
				break;
			default:
				emscripten_log(0, "codec not support: %d", codec_id);
				break;
			}
		}
		else
		{
			data >>= 2;
			compositionTime = data.readUInt24B();
			decodeBody(data);
		}
	}
	virtual void decodeBody(IOBuffer &data)
	{
		int NALUnitLength = 0;
		while (data.length > 4)
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
			_decode(data(0, NALUnitLength));
			data >>= NALUnitLength;
		}
	}
	virtual void _decode(IOBuffer data) = 0;
};