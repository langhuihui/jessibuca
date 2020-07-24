#pragma once
#include "neaacdec.h"
// #include "libfdk-aac/libAACdec/include/aacdecoder_lib.h"
class AACDecoder : public AudioDecoder {
    faacDecHandle faacHandle;
    // HANDLE_AACDECODER aacHandler;
	// UCHAR* inBuffer[FILEREAD_MAX_LAYERS];
	// UINT inBufferLength[FILEREAD_MAX_LAYERS] = {0};
	// UINT bytesValid[FILEREAD_MAX_LAYERS] = {0};
public:
    AACDecoder():AudioDecoder(){
        faacHandle = faacDecOpen();
        	// aacHandler = aacDecoder_Open(TT_MP4_ADIF,1);
		emscripten_log(0, "aac init! %d", faacHandle);
    }
    virtual ~AACDecoder()
	{
        faacDecClose(faacHandle);
        	// aacDecoder_Close(aacHandler);
    }
    int _decode(IOBuffer &input) override
    {
        u8* output = outputBuffer + bufferFilled;
        if (input.readB<1, u8>())
		{
			faacDecFrameInfo frame_info;

			auto pcm_data = faacDecDecode(faacHandle, &frame_info, (unsigned char *)input, input.length);

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

			// aacDecoder_ConfigRaw(aacHandler,(unsigned char *)input.point(), 4)
			// auto info = aacDecoder_GetStreamInfo(aacHandler);
			// samplerate = info->sampleRate;
			// channels = info->numChannels;
			auto config = faacDecGetCurrentConfiguration(faacHandle);
			config->defObjectType = HE_AAC;
			faacDecSetConfiguration(faacHandle, config);
			faacDecInit2(faacHandle, (unsigned char *)input, 4, &samplerate, &channels);
			emscripten_log(0, "aac samplerate:%d channels:%d", samplerate, channels);
		}
        return 0;
    }
};