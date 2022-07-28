#ifndef  __DECODER_VIDEO_H__
#define  __DECODER_VIDEO_H__


class DecoderVideoObserver
{

public:
    virtual void videoInfo(int width, int height) = 0;
    virtual void yuvData(unsigned char* yuv, unsigned int timestamp) = 0;
    
};



class DecoderVideo
{
public:
    DecoderVideoObserver* mObserver;

    DecoderVideo(DecoderVideoObserver* obs);

    virtual void init() = 0;

    virtual void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp) = 0;

    virtual ~DecoderVideo();
};




#endif