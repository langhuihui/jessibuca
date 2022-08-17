#ifndef  __DECODER_AUDIO_H__
#define  __DECODER_AUDIO_H__


class DecoderAudioBaseObserver
{

public:
    virtual void audioInfo(unsigned int sampleRate, unsigned int channels) = 0;
    virtual void pcmData(unsigned char** pcmList, unsigned int samples, unsigned int timestamp) = 0;
    
};


class DecoderAudioBase
{
public:
    DecoderAudioBaseObserver* mObserver;

    DecoderAudioBase(DecoderAudioBaseObserver* obs);

    virtual void init(int atype,  unsigned char* extraData, unsigned int extraDataSize) = 0;

    virtual void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp) = 0;

    virtual ~DecoderAudioBase();
};




#endif