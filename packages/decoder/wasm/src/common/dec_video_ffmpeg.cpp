#include "dec_video_ffmpeg.h"
#include "av_type.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
}

Decorder_Video_FFMPEG::Decorder_Video_FFMPEG(DecoderVideoBaseObserver* obs) : Decorder_Base_FFMPEG(), DecoderVideoBase(obs) {
  
}

Decorder_Video_FFMPEG::~Decorder_Video_FFMPEG() {

    clear();

    printf("Decorder_Video_FFMPEG dealloc \n");

}

void Decorder_Video_FFMPEG::clear() {

    if (mYUV) {
        free(mYUV);
        mYUV = nullptr;
    }

    mVideoWith = 0;
    mVideoHeight = 0;

    Decorder_Base_FFMPEG::clear();
}

void Decorder_Video_FFMPEG::init(int vtype,  unsigned char* extraData, unsigned int extraDataSize)
{
   // printf("Use Video NOT-SIMD Decoder, VideoDecoder::setCodec vtype %s, format %s, extra %d \n", vtype.c_str(), format.c_str(), extra.length());
    
    clear();

    int videotype = 0;
    int videoformat = 0;
    enum AVCodecID codecID = AV_CODEC_ID_NONE;

    if (vtype == Video_H264) {

        codecID = AV_CODEC_ID_H264;

    } else if (vtype == Video_H265) {

        codecID = AV_CODEC_ID_HEVC;

    } else {

        printf("Video Decoder not support vtype %d\n", vtype);
        return;
    }


    Decorder_Base_FFMPEG::initCodec(codecID);

    printf("FFmpeg Video Decoder, Init, VideoDecoder::setCodec vtype %d, extradatasize %d \n", vtype, extraDataSize);
    

    if (extraData && extraDataSize > 0) {

        mDecCtx->extradata_size = extraDataSize;
        mDecCtx->extradata = extraData;
    }

    avcodec_open2(mDecCtx, mCodec, NULL);

    mInit = true;
}


void  Decorder_Video_FFMPEG::decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp)
{

   //  printf("FFmpeg Video Decoder, decode len:0x%x, 0x%x 0x%x 0x%x 0x%x 0x%x \n", buflen, buf[0], buf[1], buf[2], buf[3], buf[4]);

    if (!mInit) {

        printf("VideoDecoder has not Init when decode \n");
        return;
    }

    Decorder_Base_FFMPEG::decode(buf, buflen, timestamp);

}

void  Decorder_Video_FFMPEG::frameReady(unsigned int timestamp) {

    if (mVideoWith != mFrame->width || mVideoHeight != mFrame->height) {

        mVideoWith = mFrame->width;
        mVideoHeight = mFrame->height;

        mObserver->videoInfo(mVideoWith, mVideoHeight);

        if (mYUV) {
            free(mYUV);
        }
            
        mYUV = (unsigned char*)malloc(mVideoWith * mVideoHeight * 3 /2);
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

     mObserver->yuvData(mYUV, mFrame->pts);

}
