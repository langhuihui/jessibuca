#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "dec_avc_libavc.h"
#include "dec_hevc_libhevc.h"
#include "dec_video_base.h"
#include "av_type.h"

using namespace emscripten;
using namespace std;


//视频类型，全局统一定义，JS层也使用该定义


class VideoDecoder : public DecoderVideoBaseObserver {

public:

    int mVideoWith = 0;
    int mVideoHeight = 0;
    int mVType = 0;
    int mVFormat = 0;
    DecoderVideoBase* mDecoderV;


    val mJsObject;

    unsigned char* mCodecInfo;
    int mCodecInfoLen;
    int mCodecInfoMaxLen;

    unsigned char* mBuf;
    int mBufMaxLen;

    bool mInit = false;

public:

    VideoDecoder(val&& v);
    virtual ~VideoDecoder();

    void setCodec(string vtype, string format, string extra);

    void decode(string input, unsigned int isKeyFrame, unsigned int timestamp);

    virtual void videoInfo(int width, int height);
    virtual void yuvData(unsigned char* yuv, unsigned int timestamp);
     
    void clear();

    void parseAVCExtraData(unsigned char* extradata, int extradatalen);
    void parseHEVCExtraData(unsigned char* extradata, int extradatalen);
    bool convertAnnexB(unsigned char* data, int datalen);
    int addCodecInfo(unsigned char* data, int datalen);
    void copyCodecInfo(unsigned char* data, int datalen);

    void reportError(const char* format, ...);

};


VideoDecoder::VideoDecoder(val&& v) : mJsObject(move(v)) {

    mCodecInfo = NULL;
    mCodecInfoLen = 0;
    mCodecInfoMaxLen = 0;

    mBuf = NULL;
    mBufMaxLen = 0;
  
}

VideoDecoder::~VideoDecoder() {

    clear();

    printf("VideoDecoder dealloc \n");

}

void VideoDecoder::clear() {

    mVideoWith = 0;
    mVideoHeight = 0;
    
    if (mDecoderV) {
        delete mDecoderV;
        mDecoderV = nullptr;
    }

    if (mCodecInfo) {
        free(mCodecInfo);
        mCodecInfo = NULL;
        mCodecInfoLen = 0;
        mCodecInfoMaxLen = 0;
    }

    if (mBuf) {
        free(mBuf);
        mBuf = NULL;
        mBufMaxLen = 0;
    }
}

void VideoDecoder::reportError(const char* format, ...) {

    va_list ap;
  
    va_start(ap, format);
    char* buf = nullptr;
    vasprintf(&buf, format, ap); 
    va_end(ap);


    mJsObject.call<void>("errorInfo", string(buf));
}

void VideoDecoder::copyCodecInfo(unsigned char* data, int datalen) {

    if (mCodecInfoLen + datalen > mCodecInfoMaxLen) {

        if (mCodecInfoMaxLen == 0) {

            mCodecInfoMaxLen =  (mCodecInfoLen + datalen) > 1024 ? (mCodecInfoLen + datalen) : 1024;
            mCodecInfo = (unsigned char*)malloc(mCodecInfoMaxLen);


        } else {

            mCodecInfoMaxLen = 2*(mCodecInfoLen + datalen);

            unsigned char* newBuffer = (unsigned char*)malloc(mCodecInfoMaxLen);
            memcpy(newBuffer, mCodecInfo, mCodecInfoLen);
            free(mCodecInfo);

            mCodecInfo = newBuffer;

        }
    }

    memcpy(mCodecInfo + mCodecInfoLen, data, datalen);
    mCodecInfoLen += datalen;

}


void VideoDecoder::parseAVCExtraData(unsigned char* extradata, int extradatalen) {

    unsigned char startCode[4] = {0, 0, 0, 1};

    
    int offset = 5;

    if (offset > extradatalen) {

        printf("extradata len:%d too short, can not parse \n", extradatalen);
        reportError("Parse AVC ExtraData error, because it's too short, len:%d, at least 5 bytes", extradatalen);
        return;
    }

    int spsnum = extradata[offset]&0x1F;
    offset += 1;

    unsigned int spslen = 0;
    unsigned char* spslenptr = (unsigned char*)&spslen;
    spslenptr[0] = extradata[offset+1];
    spslenptr[1] = extradata[offset];
    offset += 2;

    // copy sps
    copyCodecInfo(startCode, 4);
    copyCodecInfo(&extradata[offset], spslen);

    printf("parse AVC sps len %d\n", spslen);

    offset += spslen;

    int ppsnum = extradata[offset];
    offset += 1;

    unsigned int ppslen = 0;
    unsigned char* ppslenptr = (unsigned char*)&ppslen;
    ppslenptr[0] = extradata[offset+1];
    ppslenptr[1] = extradata[offset];
    offset += 2;

    // copy pps
    copyCodecInfo(startCode, 4);
    copyCodecInfo(&extradata[offset], ppslen);

     printf("parse AVC pps len %d\n", ppslen);

}

void VideoDecoder::parseHEVCExtraData(unsigned char* extradata, int extradatalen) {

    unsigned char startCode[4] = {0, 0, 0, 1};

    int offset = 22;

    if (offset > extradatalen) {

        printf("extradata len:%d too short, can not parse \n", extradatalen);
            reportError("Parse Hevc ExtraData error, because it's too short, len:%d, at least 22 bytes", extradatalen);
        return;
    }

    int nalsnum = extradata[offset];
    offset++;

    printf("parse HEVC nalsnum %d\n", nalsnum);

    for (int i = 0; i < nalsnum; i++) {

        int naltype = extradata[offset]&0x3F;
        offset++;

        unsigned int onenalnum = 0;
        unsigned char* onenalnumptr = (unsigned char*)&onenalnum;
        onenalnumptr[0] = extradata[offset+1];
        onenalnumptr[1] = extradata[offset];
        offset+=2;

        printf("parse HEVC naltype %d onenalnum %d\n", naltype, onenalnum);

        for (int j = 0; j < onenalnum; j++) {

            unsigned int onenallen = 0;
            unsigned char* onenallenptr = (unsigned char*)&onenallen;
            onenallenptr[0] = extradata[offset+1];
            onenallenptr[1] = extradata[offset];
            offset+=2;

            // copy vps, sps, pps
            if (naltype == 32 || naltype == 33 || naltype == 34) {

                copyCodecInfo(startCode, 4);
                copyCodecInfo(&extradata[offset], onenallen);

            } 

            offset += onenallen;

        }

    }

}

void VideoDecoder::setCodec(string vtype, string format, string extra)
{
     printf("Use Video SIMD Decoder, VideoDecoder::setCodec vtype %s, format %s, extra %d \n", vtype.c_str(), format.c_str(), extra.length());
    
    clear();

    int videotype = 0;
    int videoformat = 0;

    if (vtype.compare("avc") == 0) {

        videotype = Video_H264;

        if (format.compare("avcc") == 0) {

            videoformat = Format_AVCC;
            parseAVCExtraData((unsigned char*)extra.data(), extra.length());

        } else if (format.compare("annexb") == 0) {

            videoformat = Format_AVC_AnnexB;
        } else {

            printf("Video Decoder not support vtype %s, format %s \n", vtype.c_str(), format.c_str());
            return;
        }

        mDecoderV = new Decoder_AVC_LIBAVC(this);

    } else if (vtype.compare("hevc") == 0) {

        videotype = Video_H265;

        if (format.compare("hvcc") == 0) {

            videoformat = Format_HVCC;
            parseHEVCExtraData((unsigned char*)extra.data(), extra.length());

        } else if (format.compare("annexb") == 0) {

            videoformat = Format_HEVC_AnnexB;
        }
         else {

            printf("Video Decoder not support vtype %s, format %s \n", vtype.c_str(), format.c_str());
             return;
        }

         mDecoderV = new Decoder_HEVC_LIBHEVC(this);

    } else {

        printf("Video Decoder not support vtype %s, format %s \n", vtype.c_str(), format.c_str());
        return;
    }

    mVType = videotype;
    mVFormat = videoformat;

    mDecoderV->init(mVType, NULL, 0);
    mInit = true;
}

bool VideoDecoder::convertAnnexB(unsigned char* data, int datalen) {

    int offset = 0;
    bool bf = false; //检测 帧里是否有codec 信息

    while(offset < datalen) {

        unsigned int nallen = 0;
        unsigned char* nallenptr = (unsigned char*)&nallen;
        nallenptr[0] = data[offset+3];
        nallenptr[1] = data[offset+2];
        nallenptr[2] = data[offset+1];
        nallenptr[3] = data[offset];
        
        data[offset] = 0;
        data[offset+1] = 0;
        data[offset+2] = 0;
        data[offset+3] = 1;

        offset += 4;

        if (!bf) {

            int naltype = 0;

            if (mVType == Video_H264) {

                naltype = data[offset]&0x1F;

                //avc sps
                if (naltype == 7) {

                bf = true;
                }

            } else {

                naltype = (data[offset]&0x7E)>>1;

                //hevc sps
                if (naltype == 33) {

                    bf = true;
                }
            }

        }

        offset += nallen;
    }

    if (offset != datalen) {

        printf("convertAnnexB error offset %d datalen %d \n", offset, datalen);
        
    } 

  //  printf("convertAnnexB end offset %d datalen %d bf %d \n", offset, datalen, bf);

    return bf;
}

int VideoDecoder::addCodecInfo(unsigned char* data, int datalen) {

    int buflen = datalen + mCodecInfoLen;

    if (buflen > mBufMaxLen) {

        if (mBuf) {
            free(mBuf);
        }
        
        mBuf = (unsigned char*)malloc(buflen);
        mBufMaxLen = buflen;
    }

    memcpy(mBuf, mCodecInfo, mCodecInfoLen);
    memcpy(mBuf + mCodecInfoLen, data, datalen);

    return buflen;

}

void  VideoDecoder::decode(string input, unsigned int isKeyFrame, unsigned int timestamp)
{
    if (!mInit) {

        printf("VideoDecoder has not Init when decode \n");
        return;
    }

    unsigned int bufferLen = input.length();
    unsigned char* buffer = (unsigned char*)input.data();

    if (mVFormat == Format_AVC_AnnexB || mVFormat == Format_HEVC_AnnexB) {

        mDecoderV->decode(buffer, bufferLen, timestamp);

    } else {

        bool bf = convertAnnexB(buffer, bufferLen);

        if (isKeyFrame && !bf) {

            //给关键帧补齐sps/pps/vps信息
            int len = addCodecInfo(buffer, bufferLen);
            mDecoderV->decode(mBuf, len, timestamp);

        } else {

            mDecoderV->decode(buffer, bufferLen, timestamp);
        }

    }

}

void VideoDecoder::videoInfo(int width, int height){

    mVideoWith = width;
    mVideoHeight = height;

    mJsObject.call<void>("videoInfo",  mVideoWith, mVideoHeight);
}

void VideoDecoder::yuvData(unsigned char* yuv, unsigned int timestamp) {

    int size = mVideoWith*mVideoHeight;
    unsigned char* yuvArray[3] = {yuv, yuv + size, yuv + size*5/4};

    // printf("yuv %d %d %d %d %d %d\n", yuv[0], yuv[1], yuv[2], yuv[mVideoWith*mVideoHeight/2], yuv[mVideoWith*mVideoHeight/2+1], yuv[mVideoWith*mVideoHeight/2+2]);
    mJsObject.call<void>("yuvData", (unsigned int)yuvArray, timestamp);

}



EMSCRIPTEN_BINDINGS(my_module) {
     class_<VideoDecoder>("VideoDecoder")
    .constructor<val>()
    .function("setCodec", &VideoDecoder::setCodec)
    .function("decode", &VideoDecoder::decode)
    .function("clear", &VideoDecoder::clear);
}
