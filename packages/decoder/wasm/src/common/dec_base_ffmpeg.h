
#ifndef  __DECODER_BASE_FFMPEG_H__
#define  __DECODER_BASE_FFMPEG_H__

extern "C"
{
    #include <libavcodec/avcodec.h>
}

class AVCodec;
class AVCodecContext;
class AVFrame;
class AVPacket;

class Decorder_Base_FFMPEG {

public:

    AVCodec *mCodec = nullptr;
    AVCodecContext *mDecCtx = nullptr;
    AVFrame *mFrame = nullptr;
    AVPacket *mPacket = nullptr;
    bool mInit = false;

public:

    Decorder_Base_FFMPEG();

    virtual ~Decorder_Base_FFMPEG();

    void initCodec(enum AVCodecID codecID);
    virtual void clear();
    void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp);

    virtual void frameReady(unsigned int timestamp) {};

};


#endif