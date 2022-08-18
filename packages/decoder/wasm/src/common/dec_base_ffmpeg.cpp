

#include "dec_base_ffmpeg.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
}

Decorder_Base_FFMPEG::Decorder_Base_FFMPEG():mInit(false) {
  
}

Decorder_Base_FFMPEG::~Decorder_Base_FFMPEG() {
    clear();
}

void Decorder_Base_FFMPEG::initCodec(enum AVCodecID codecID) {
    
    if (mInit) {

        return;
    }

    mPacket = av_packet_alloc();
    mCodec = avcodec_find_decoder(codecID);
    mDecCtx = avcodec_alloc_context3(mCodec);
    mFrame = av_frame_alloc();

}

void Decorder_Base_FFMPEG::clear() {

    if (mDecCtx) {
        avcodec_free_context(&mDecCtx);
        mDecCtx = nullptr;
    }

    if (mFrame) {
        av_frame_free(&mFrame);
        mFrame = nullptr;
    }

    if (mPacket) {
        av_packet_free(&mPacket);
        mPacket = nullptr;
    }

    mCodec = nullptr;
    mInit = false;
}

void Decorder_Base_FFMPEG::decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp) {

    int ret = 0;
    mPacket->data = buf;
    mPacket->size = buflen;
    mPacket->pts = timestamp;

    ret = avcodec_send_packet(mDecCtx, mPacket);
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(mDecCtx, mFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;

        frameReady(timestamp);
    }
}