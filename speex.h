#pragma once
#include <speex/speex.h>
class SpeexDecoder : public AudioDecoder {
    i16 *audioOutput;
	void *speexState;
	SpeexBits speexBits;
    public:
    SpeexDecoder():AudioDecoder(){
        audioOutput = (i16 *)malloc(640);
		auto mode = speex_lib_get_mode(SPEEX_MODEID_WB);
		speexState = speex_decoder_init(mode);
		speex_bits_init(&speexBits);
		emscripten_log(0, "speex init!");
    }
    virtual ~SpeexDecoder(){
        speex_decoder_destroy(speexState);
		speex_bits_destroy(&speexBits);
		free(audioOutput);
    }
    int _decode(IOBuffer &input) override
	{
        u8 *output = outputBuffer + bufferFilled;
		if (input.length <= 11)
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
	}
};