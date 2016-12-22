#pragma once
#ifdef USE_SPEEX
#include <speex/speex.h>
#endif
#ifdef USE_AAC
#include "aacDecoder/include/neaacdec.h"
#endif
#ifdef USE_MP3
#include "libmad/mad.h"
#include "libid3tag/tag.h"
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
	u8* outputBuffer;
#ifdef USE_AAC
	faacDecHandle faacHandle;
	faacDecConfigurationPtr faacConfiguration;
#endif
#ifdef USE_SPEEX
	i16* audioOutput;
	void* speexState;
	SpeexBits speexBits;
#endif
#ifdef USE_MP3
	mad_stream stream;
	mad_frame frame;
	mad_synth synth;
	MemoryStream inputBuffer;
	int inputCount;
#endif
	
	AudioDecoder(int bl) :bufferLength(bl), bufferFilled(0)
	{
		outputBuffer = (u8*)malloc(bufferLength);
#ifdef USE_SPEEX
		audioOutput = (i16*)malloc(640);
		auto mode = speex_lib_get_mode(SPEEX_MODEID_WB);
		speexState = speex_decoder_init(mode);
		speex_bits_init(&speexBits);
		emscripten_log(0, "speex init!");
#endif
#ifdef USE_AAC
		faacHandle = faacDecOpen();
		faacConfiguration = faacDecGetCurrentConfiguration(faacHandle);
		emscripten_log(0, "aac init! %d", faacHandle);
#endif
#ifdef USE_MP3
		mad_stream_init(&stream);
		mad_frame_init(&frame);
		mad_synth_init(&synth);
		emscripten_log(0, "mp3 init!");
#endif
	}
	~AudioDecoder()
	{
#ifdef USE_MP3
		mad_synth_finish(&synth);
		mad_frame_finish(&frame);
		mad_stream_finish(&stream);
#endif
#ifdef USE_AAC
		faacDecClose(faacHandle);
#endif
#ifdef USE_SPEEX
		speex_decoder_destroy(speexState);
		speex_bits_destroy(&speexBits);
		free(audioOutput);
#endif
		free(outputBuffer);
	}
	bool decode(int audioType, MemoryStream & data)
	{
		int samplesBytes = 0;
		switch (audioType)
		{
		case Codec_AAC:
			samplesBytes = decodeAAC(data, outputBuffer + bufferFilled);
			break;
		case Codec_MP3:
			samplesBytes = decodeMP3(data, outputBuffer + bufferFilled);
			break;
		case Codec_Speex:
			samplesBytes = decodeSpeex(data, outputBuffer + bufferFilled);
			break;
		}
		if (samplesBytes)
		{
			bufferFilled += samplesBytes;
			if (bufferFilled == bufferLength) {
				bufferFilled = 0;
				return true;
			}
		}
		return false;
	}
	int decodeSpeex(MemoryStream& input, u8* output)
	{
#ifdef USE_SPEEX
		if (input.length() <= 11) {
			memset(output, 0, 640);
		}
		else {
			speex_bits_read_from(&speexBits, (const char *)input, 52);
			speex_decode_int(speexState, &speexBits, audioOutput);
			memcpy(output, audioOutput, 640);
		}
		return 640;
#endif
		return 0;
	}
	int decodeAAC(MemoryStream& input, u8* output)
	{
#ifdef USE_AAC
		//0 = AAC sequence header£¬1 = AAC raw¡£
		if(input.readB<1,u8>())
		{
			faacDecFrameInfo frame_info;
			//emscripten_log(0, "%d", data.length());
			auto pcm_data = faacDecDecode(faacHandle, &frame_info, (unsigned char *)input.point(), input.length());

			if (frame_info.error > 0)
			{
				emscripten_log(1, "%s\n", NeAACDecGetErrorMessage(frame_info.error));
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
				memcpy(output, pcm_data, samplesBytes);
				return samplesBytes;
			}
		}else
		{
			unsigned long samplerate;
			unsigned char channels;
			faacDecInit2(faacHandle, (unsigned char *)input.point(), 4, &samplerate, &channels);
			emscripten_log(0, "aac samplerate:%d channels:%d", samplerate, channels);
		}
#endif
		return 0;
	}

	int decodeMP3(MemoryStream& input,u8* output)
	{
		
#ifdef USE_MP3
		//int result = mad_decoder_run(&mp3Decoder, MAD_DECODER_MODE_SYNC);
		//emscripten_log(0, "mp3 result:%d", result);
		inputBuffer << input;
		inputCount++;
		if (inputCount < 5)return 0;
		mad_stream_buffer(&stream, (const unsigned char *)inputBuffer.point(), inputBuffer.length());
		
		while(true)
		{
			if (-1 == mad_frame_decode(&frame, &stream))
			{
				if (!MAD_RECOVERABLE(stream.error))
				{
					break;
				}

				switch (stream.error)
				{
				case MAD_ERROR_BADDATAPTR:
					continue;

				case MAD_ERROR_LOSTSYNC:
				{
					// excute id3 tag frame skipping  
					unsigned long tagsize = id3_tag_query(stream.this_frame, stream.bufend - stream.this_frame);
					if (tagsize > 0)mad_stream_skip(&stream, tagsize);
				}
				continue;

				default:
					continue;
				}
			}
			//mad_frame_decode(&frame, &stream);
			mad_synth_frame(&synth, &frame);
			mad_pcm &pcm = synth.pcm;
			unsigned int nchannels, nsamples;
			mad_fixed_t const *left_ch, *right_ch;
			nchannels = pcm.channels;
			nsamples = pcm.length;
			left_ch = pcm.samples[0];
			right_ch = pcm.samples[1];
			memcpy(output, pcm.samples[0], nsamples * 2);
			emscripten_log(0, "mad_frame_decode channels:%d", pcm.channels);
		}
		if(stream.error)
		{
			emscripten_log(0, "mad_frame_decode:%d", stream.error);
		}
		inputBuffer.clear();
		inputBuffer << input;
		inputCount = 1;
#endif
		return 0;
	}

};