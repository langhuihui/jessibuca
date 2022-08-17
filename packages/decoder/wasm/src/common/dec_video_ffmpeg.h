#ifndef  __DECODER_VIDEO_FFMPEG_H__
#define  __DECODER_VIDEO_FFMPEG_H__



#include "dec_base_ffmpeg.h"
#include "dec_video_base.h"

class Decorder_Video_FFMPEG : public Decorder_Base_FFMPEG, public DecoderVideoBase {

public:

    int mVideoWith = 0;
    int mVideoHeight = 0;
    int mVType = 0;
    int mVFomat = 0;

    unsigned char* mYUV = nullptr;

public:

    Decorder_Video_FFMPEG(DecoderVideoBaseObserver* obs);
    virtual ~Decorder_Video_FFMPEG();

    virtual void init(int vtype,  unsigned char* extraData, unsigned int extraDataSize);

    virtual void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp);

    virtual void clear();

    virtual void frameReady(unsigned int timestamp);
};

#endif