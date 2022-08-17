#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

using namespace emscripten;
using namespace std;


#include "dec_audio_ffmpeg.h"
#include "dec_audio_base.h"
#include "av_type.h"

class AudioDecoder : public DecoderAudioBaseObserver {

public:

    int mAType;

    DecoderAudioBase* mDecoderA;
    val mJsObject;
    bool mInit;

public:

    AudioDecoder(val&& v);
    virtual ~AudioDecoder();

    void setCodec(string atype, string extra);
    void decode(string input, unsigned int timestamp);
    void clear();

    void reportError(const char* format, ...);

    virtual void audioInfo(unsigned int sampleRate, unsigned int channels);
    virtual void pcmData(unsigned char** pcmList, unsigned int samples, unsigned int timestamp);

};


AudioDecoder::AudioDecoder(val&& v) : mJsObject(move(v)) {
    
    mAType = Audio_Unknow;
    mInit = false;
}


void AudioDecoder::clear() {

    if (mDecoderA) {
        delete mDecoderA;
        mDecoderA = nullptr;
    }
}


AudioDecoder::~AudioDecoder() {

    clear();

    printf("AudioDecoder dealloc \n");
}

void AudioDecoder::reportError(const char* format, ...) {

    va_list ap;
  
    va_start(ap, format);
    char* buf = nullptr;
    vasprintf(&buf, format, ap); 
    va_end(ap);


    mJsObject.call<void>("errorInfo", string(buf));
}

void AudioDecoder::setCodec(string atype, string extra)
{
    printf("Use Audio Decoder, AudioDecoder::setCodec atype %s, extra %d \n", atype.c_str(), extra.length());
    
    clear();


    if (atype.compare("aac") == 0) {

        mAType = Audio_AAC;

    } else if (atype.compare("pcmu") == 0) {

        mAType = Audio_PCMA;

    } else if (atype.compare("pcma") == 0) {

        mAType = Audio_PCMU;

    } else {

         printf("AudioDecoder not support type %s \n", atype.c_str());
        return;
    }

    mDecoderA = new Decorder_Audio_FFMPEG(this);
    mDecoderA->init(mAType, (unsigned char*)extra.data(), extra.length());
    mInit = true;
}


void  AudioDecoder::decode(string input, unsigned int timestamp)
{
    if (!mInit) {

        return;
    }

    unsigned int bufferLen = input.length();
    unsigned char* buffer = (unsigned char*)input.data();

    mDecoderA->decode(buffer, bufferLen, timestamp);

}

void AudioDecoder::audioInfo(unsigned int sampleRate, unsigned int channels) {

    mJsObject.call<void>("audioInfo", sampleRate, channels);
}

void AudioDecoder::pcmData(unsigned char** pcmList, unsigned int samples, unsigned int timestamp) {

     mJsObject.call<void>("pcmData", (int)pcmList, samples, timestamp);

}


EMSCRIPTEN_BINDINGS(my_module) {
    class_<AudioDecoder>("AudioDecoder")
    .constructor<val>()
    .function("setCodec", &AudioDecoder::setCodec)
    .function("decode", &AudioDecoder::decode)
    .function("clear", &AudioDecoder::clear);
}
