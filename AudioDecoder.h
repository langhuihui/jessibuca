#pragma once
#ifdef USE_SPEEX
#include <speex/speex.h>
#endif
#ifdef USE_AAC
#include "neaacdec.h"
// #include "libfdk-aac/libAACdec/include/aacdecoder_lib.h"
#endif
#ifdef USE_MP3
#include "libmad/mad.h"
//#include "libid3tag/tag.h"
#include "mp3Decoder.h"
#endif
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
	int bufferLength;
	int bufferFilled;

public:
	u8 *outputBuffer;
#ifdef USE_AAC
	faacDecHandle faacHandle;
	// HANDLE_AACDECODER aacHandler;
	// UCHAR* inBuffer[FILEREAD_MAX_LAYERS];
	// UINT inBufferLength[FILEREAD_MAX_LAYERS] = {0};
	// UINT bytesValid[FILEREAD_MAX_LAYERS] = {0};
#endif
#ifdef USE_SPEEX
	i16 *audioOutput;
	void *speexState;
	SpeexBits speexBits;
#endif
#ifdef USE_MP3
	MP3Decoder mp3Decoder;
#endif
	void init(int bf)
	{
		bufferLength = bf;
		outputBuffer = (u8 *)malloc(bufferLength);
		emscripten_log(0, "set audio bufferLength:%d", bf);
	}
	AudioDecoder() : bufferFilled(0)
	{
#ifdef USE_SPEEX
		audioOutput = (i16 *)malloc(640);
		auto mode = speex_lib_get_mode(SPEEX_MODEID_WB);
		speexState = speex_decoder_init(mode);
		speex_bits_init(&speexBits);
		emscripten_log(0, "speex init!");
#endif
#ifdef USE_AAC
		faacHandle = faacDecOpen();
		// aacHandler = aacDecoder_Open(TT_MP4_ADIF,1);
		emscripten_log(0, "aac init! %d", faacHandle);
#endif
		emscripten_log(0, "audio init! %d", this);
	}
	~AudioDecoder()
	{
		emscripten_log(0, "audio decoder release\n");
#ifdef USE_AAC
		faacDecClose(faacHandle);
		// aacDecoder_Close(aacHandler);
#endif
#ifdef USE_SPEEX
		speex_decoder_destroy(speexState);
		speex_bits_destroy(&speexBits);
		free(audioOutput);
#endif
		free(outputBuffer);
		emscripten_log(0, "audio decoder release!\n");
	}
	void clear()
	{
	}
	bool decode(int audioType, MemoryStream &data)
	{
		int samplesBytes = 0;
		switch (audioType)
		{
		case Codec_AAC:
			samplesBytes = decodeAAC(data, outputBuffer + bufferFilled);
			break;
		case Codec_MP3:
			samplesBytes = decodeMP3(data, outputBuffer + bufferFilled, outputBuffer + bufferLength);
			break;
		case Codec_Speex:
			samplesBytes = decodeSpeex(data, outputBuffer + bufferFilled);
			break;
		}
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
	int decodeSpeex(MemoryStream &input, u8 *output)
	{
#ifdef USE_SPEEX
		if (input.length() <= 11)
		{
			memset(output, 0, 640);
		}
		else
		{
			speex_bits_read_from(&speexBits, (const char *)input, 52);
			speex_decode_int(speexState, &speexBits, audioOutput);
			memcpy(output, audioOutput, 640);
		}
		return 640;
#endif
		return 0;
	}
	int decodeAAC(MemoryStream &input, u8 *output)
	{
#ifdef USE_AAC
		//0 = AAC sequence header��1 = AAC raw��
		if (input.readB<1, u8>())
		{
			faacDecFrameInfo frame_info;

			auto pcm_data = faacDecDecode(faacHandle, &frame_info, (unsigned char *)input.point(), input.length());

			if (frame_info.error > 0)
			{
				emscripten_log(1, "!!%s\n", NeAACDecGetErrorMessage(frame_info.error));
			}
			else
			{
				/*
				emscripten_log(0,"frame info: bytesconsumed %d, channels %d, header_type %d                object_type %d, samples %d, samplerate %d\n",
				frame_info.bytesconsumed,
				frame_info.channels, frame_info.header_type,
				frame_info.object_type, frame_info.samples,
				frame_info.samplerate);
				*/
				int samplesBytes = frame_info.samples << 1;
				// inBuffer[0] =  (unsigned char *)input.point();
				// bytesValid[0] = inBufferLength[0] =  input.length();
				// aacDecoder_Fill(aacHandler,inBuffer, inBufferLength,bytesValid);

				// auto decoderErr = aacDecoder_DecodeFrame(aacHandler,,0);
				memcpy(output, pcm_data, samplesBytes);
				return samplesBytes;
			}
		}
		else
		{
			unsigned long samplerate;
			unsigned char channels;

			// aacDecoder_ConfigRaw(aacHandler,(unsigned char *)input.point(), 4)
			// auto info = aacDecoder_GetStreamInfo(aacHandler);
			// samplerate = info->sampleRate;
			// channels = info->numChannels;
			auto config = faacDecGetCurrentConfiguration(faacHandle);
			config->defObjectType = LTP;
			faacDecSetConfiguration(faacHandle, config);
			faacDecInit2(faacHandle, (unsigned char *)input.point(), 4, &samplerate, &channels);
			emscripten_log(0, "aac samplerate:%d channels:%d", samplerate, channels);
		}
#endif
		return 0;
	}
#ifdef USE_MP3
	signed int scale(mad_fixed_t sample)
	{
		/* round */
		sample += (1L << (MAD_F_FRACBITS - 16));

		/* clip */
		if (sample >= MAD_F_ONE)
			sample = MAD_F_ONE - 1;
		else if (sample < -MAD_F_ONE)
			sample = -MAD_F_ONE;

		/* quantize */
		return sample >> (MAD_F_FRACBITS + 1 - 16);
	}
#endif
	int decodeMP3(MemoryStream &input, u8 *output, u8 *end)
	{

#ifdef USE_MP3
		int ret = mp3Decoder.decode(input);
		int samplesBytes = 0;
		while (ret != -1)
		{
			mad_pcm &pcm = mp3Decoder.getPCM();
			unsigned int nchannels, nsamples;
			mad_fixed_t const *left_ch, *right_ch;
			nchannels = pcm.channels;
			nsamples = pcm.length;
			left_ch = pcm.samples[0];
			right_ch = pcm.samples[1];
			samplesBytes += nsamples * 2 * nchannels;
			//i16 left_output[576];
			//emscripten_log(0,"%d %d",left_ch[0],left_ch[1])
			while (nsamples--)
			{
				signed int sample;
				/* output sample(s) in 16-bit signed little-endian PCM */
				sample = scale(*left_ch++);
				(*output++) = ((sample >> 0) & 0xff);
				(*output++) = ((sample >> 8) & 0xff);
				if (nchannels == 2)
				{
					sample = scale(*right_ch++);
					(*output++) = ((sample >> 0) & 0xff);
					(*output++) = ((sample >> 8) & 0xff);
				}
				//memcpy(output, &sample, 2);
				//output+=2;
				//*(left_output++) = (i16)sample;
				// if (nchannels == 2) {
				//   sample = scale(*right_ch++);
				//   putchar((sample >> 0) & 0xff);
				//   putchar((sample >> 8) & 0xff);
				// }
			}
			//memcpy(output, left_output, 1152);
			// output += 1152;
			if (output >= end)
			{
				return samplesBytes;
			}
			//emscripten_log(0, "mad_frame_decode channels:%d nsamples:%d",nchannels,nsamples);
			ret = mp3Decoder.decode();
		}
		return samplesBytes;
#endif
		return 0;
	}
};