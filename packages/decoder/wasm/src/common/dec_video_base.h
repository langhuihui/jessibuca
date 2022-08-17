#ifndef  __DECODER_VIDEO_H__
#define  __DECODER_VIDEO_H__


class DecoderVideoBaseObserver
{

public:
    virtual void videoInfo(int width, int height) = 0;
    virtual void yuvData(unsigned char* yuv, unsigned int timestamp) = 0;
    
};


class DecoderVideoBase
{
public:
    DecoderVideoBaseObserver* mObserver;

    DecoderVideoBase(DecoderVideoBaseObserver* obs);

    virtual void init(int atype,  unsigned char* extraData, unsigned int extraDataSize) = 0;

    virtual void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp) = 0;

    virtual ~DecoderVideoBase();
};




#endif