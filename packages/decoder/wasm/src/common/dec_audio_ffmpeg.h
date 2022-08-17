

#ifndef  __DECODER_AUDIO_FFMPEG_H__
#define  __DECODER_AUDIO_FFMPEG_H__



#include "dec_base_ffmpeg.h"
#include "dec_audio_base.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
}

class SwrContext;

class Decorder_Audio_FFMPEG : public Decorder_Base_FFMPEG, public DecoderAudioBase  {

public:

    enum AVSampleFormat mAudioFormat;

    SwrContext *mConvertCtx = nullptr;
    unsigned char *mOutBuffer[2];

    bool mNotifyAudioParam;
    int mAType;


public:

    Decorder_Audio_FFMPEG(DecoderAudioBaseObserver* obs);

    virtual ~Decorder_Audio_FFMPEG();

    virtual void init(int atype,  unsigned char* extraData, unsigned int extraDataSize);

    virtual void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp);

    virtual void clear();

    virtual void frameReady(unsigned int timestamp);

};

#endif