#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

using namespace emscripten;
using namespace std;
typedef unsigned char u8;
typedef unsigned int u32;

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}


//视频类型，全局统一定义，JS层也使用该定义
enum VideoType {

    Video_H264 = 0x01,
    Video_H265 = 0x02

};


class Decoder {

public:
    val mJsObject;

    AVCodec *mCodec = nullptr;
    AVCodecContext *mDecCtx = nullptr;
    AVFrame *mFrame = nullptr;
    AVPacket *mPacket = nullptr;
    bool mInit = false;

public:

    Decoder(val&& v);
    virtual ~Decoder();

    void initCodec(enum AVCodecID codecID);
    virtual void clear();
    void decode(u8* buffer, u32 bufferLen, u32 timestamp);

    virtual void frameReady(u32 timestamp) {};

};

Decoder::Decoder(val&& v) : mJsObject(move(v)) {
  
}

Decoder::~Decoder() {
    clear();
}

void Decoder::initCodec(enum AVCodecID codecID) {
    
    if (mInit) {

        return;
    }

    mPacket = av_packet_alloc();
    mCodec = avcodec_find_decoder(codecID);
    mDecCtx = avcodec_alloc_context3(mCodec);
    mFrame = av_frame_alloc();

}

void Decoder::clear() {

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

void Decoder::decode(u8* buffer, u32 bufferLen, u32 timestamp) {

    int ret = 0;
    mPacket->data = buffer;
    mPacket->size = bufferLen;
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


class VideoDecoder : public Decoder {

public:

    int mVideoWith = 0;
    int mVideoHeight = 0;
    int mVType = 0;

    u8* mYUV = nullptr;

public:

    VideoDecoder(val&& v);
    virtual ~VideoDecoder();

    void setCodec(u32 vtype, string extra);

    void decode(string input, u32 isKeyFrame, u32 timestamp);

    virtual void clear();
    virtual void frameReady(u32 timestamp);

};


VideoDecoder::VideoDecoder(val&& v) : Decoder(move(v)) {
  
}

VideoDecoder::~VideoDecoder() {

    clear();

    printf("VideoDecoder dealloc \n");

}

void VideoDecoder::clear() {

    if (mYUV) {
        free(mYUV);
        mYUV = nullptr;
    }

    mVideoWith = 0;
    mVideoHeight = 0;

    Decoder::clear();
}

void VideoDecoder::setCodec(u32 vtype, string extra)
{

    printf("Use Video NOT-SIMD Decoder, VideoDecoder::setCodec vtype %d, extra %d \n", vtype, extra.length());
    
  
    clear();

    enum AVCodecID codecID;

    switch (vtype)
    {
        case Video_H264: {

            codecID = AV_CODEC_ID_H264;

            break;
        }

        case Video_H265: {

            codecID = AV_CODEC_ID_HEVC;
            break;
        }
    
        default: {

            return;
        }
    }

    mVType = vtype;


    Decoder::initCodec(codecID);
    
    u32 extraDataSize = extra.length();
    u8* extraData = (u8*)extra.data();

    if (extraData && extraDataSize > 0) {

        mDecCtx->extradata_size = extraDataSize;
        mDecCtx->extradata = extraData;
    }

    avcodec_open2(mDecCtx, mCodec, NULL);

    mInit = true;
}


void  VideoDecoder::decode(string input, u32 isKeyFrame, u32 timestamp)
{

  //   printf("VideoDecoder::decode input %d, timestamp %d \n", input.length(), timestamp);

    if (!mInit) {

        printf("VideoDecoder has not Init when decode \n");
        return;
    }

    u32 bufferLen = input.length();
    u8* buffer = (u8*)input.data();

    Decoder::decode(buffer, bufferLen, timestamp);

}

void  VideoDecoder::frameReady(u32 timestamp) {

    if (mVideoWith != mFrame->width || mVideoHeight != mFrame->height) {

        mVideoWith = mFrame->width;
        mVideoHeight = mFrame->height;

        mJsObject.call<void>("videoInfo", mVType, mVideoWith, mVideoHeight);

        if (mYUV) {
            free(mYUV);
        }
            
       
        mYUV = (u8*)malloc(mVideoWith * mVideoHeight * 3 /2);
    }

    int size = mVideoWith * mVideoHeight;
 
    int halfw = mVideoWith >> 1;
    int halfh = mVideoHeight >> 1;

    if (mVideoWith == mFrame->linesize[0]) {

        memcpy(mYUV, mFrame->data[0], size);

    } else {

        for (int i = 0; i < mVideoHeight; i++) {

            memcpy(mYUV + i*mVideoWith, mFrame->data[0] + i*mFrame->linesize[0], mVideoWith);
        }

    }

    if (halfw == mFrame->linesize[1]) {

        memcpy(mYUV + size, mFrame->data[1], size>>2);

    } else {

        for (int i = 0; i < halfh; i++) {

            memcpy(mYUV + size + i*halfw, mFrame->data[1] + i*mFrame->linesize[1], halfw);
        }

    }

    if (halfw == mFrame->linesize[2]) {

        memcpy(mYUV + size*5/4, mFrame->data[2], size>>2);

    } else {

        for (int i = 0; i < halfh; i++) {

            memcpy(mYUV + size*5/4 + i*halfw, mFrame->data[2] + i*mFrame->linesize[2], halfw);
        }

    }

  //  printf("C yuv[0-5] %d  %d  %d  %d %d %d \n", mYUV[0], mYUV[1], mYUV[2], mYUV[3], mYUV[4], mYUV[5]);

    u8* yuvArray[3] = {mYUV, mYUV + size, mYUV + size*5/4};

    mJsObject.call<void>("yuvData", (u32)yuvArray, (u32)mFrame->pts);

}


EMSCRIPTEN_BINDINGS(my_module) {
     class_<VideoDecoder>("VideoDecoder")
    .constructor<val>()
    .function("setCodec", &VideoDecoder::setCodec)
    .function("decode", &VideoDecoder::decode)
    .function("clear", &VideoDecoder::clear);
}
