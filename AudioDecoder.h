#pragma once
/*  0 = Linear PCM, platform endian
1 = ADPCM
2 = MP3
3 = Linear PCM, little endian
4 = Nellymoser 16 kHz mono
5 = Nellymoser 8 kHz mono
6 = Nellymoser
7 = G.711 A - law logarithmic PCM
8 = G.711 mu - law logarithmic PCM
9 = reserved
10 = AAC
11 = Speex
14 = MP3 8 kHz
15 = Device - specific sound
*/
#define Codec_AAC 10
#define Codec_Speex 11
#define Codec_MP3 2
class AudioDecoder
{
	protected:
	int bufferLength;
	int bufferFilled;

public:
	u8 *outputBuffer;
	unsigned long samplerate;
	unsigned char channels;
	void init(int bf)
	{
		bufferLength = bf;
		outputBuffer = (u8 *)malloc(bufferLength);
		emscripten_log(0, "set audio bufferLength:%d", bf);
	}
	AudioDecoder() : bufferFilled(0)
	{
		emscripten_log(0, "audio init! %d", this);
	}
	virtual ~AudioDecoder()
	{
		free(outputBuffer);
		emscripten_log(0, "audio decoder release!\n");
	}
	virtual void clear(){

	}
	virtual int _decode(IOBuffer &data)
	{
		return 0;
	}
	bool decode(IOBuffer &data)
	{
		int samplesBytes = _decode(data);
		// switch (audioType)
		// {
		// case Codec_AAC:
		// 	samplesBytes = decodeAAC(data, outputBuffer + bufferFilled);
		// 	break;
		// case Codec_MP3:
		// 	samplesBytes = decodeMP3(data, outputBuffer + bufferFilled, outputBuffer + bufferLength);
		// 	break;
		// case Codec_Speex:
		// 	samplesBytes = decodeSpeex(data, outputBuffer + bufferFilled);
		// 	break;
		// }
		if (samplesBytes)
		{
			bufferFilled += samplesBytes;
			if (bufferFilled == bufferLength)
			{
				bufferFilled = 0;
				return true;
			}
		}
		return false;
	}
};