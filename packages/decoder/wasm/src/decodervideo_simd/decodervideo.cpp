#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <stdio.h>
#include <string.h>

#include "decoderavc.h"
#include "decoderhevc.h"

using namespace emscripten;
using namespace std;
typedef unsigned char u8;
typedef unsigned int u32;


//视频类型，全局统一定义，JS层也使用该定义
enum VideoType {

    Video_H264 = 0x01,
    Video_H265 = 0x02

};

enum VideoFormatType {

    Format_AVC = 0x01,
    Format_AVC_AnnexB = 0x02,
    Format_HVCC = 0x03,
    Format_HEVC_AnnexB = 0x04
};



class VideoDecoder : public DecoderVideoObserver {

public:

    int mVideoWith = 0;
    int mVideoHeight = 0;
    int mVType = 0;
    int mVFormat = 0;
    DecoderVideo* mDecoderV;


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

    void decode(string input, u32 isKeyFrame, u32 timestamp);

    virtual void videoInfo(int width, int height);
    virtual void yuvData(unsigned char* yuv, unsigned int timestamp);
     

    void clear();

    void parseAVCExtraData(u8* extradata, int extradatalen);
    void parseHEVCExtraData(u8* extradata, int extradatalen);
    bool convertAnnexB(u8* data, int datalen);
    int addCodecInfo(u8* data, int datalen);
    void copyCodecInfo(u8* data, int datalen);

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

void VideoDecoder::copyCodecInfo(u8* data, int datalen) {

    if (mCodecInfoLen + datalen > mCodecInfoMaxLen) {

        if (mCodecInfoMaxLen == 0) {

            mCodecInfoMaxLen =  (mCodecInfoLen + datalen) > 1024 ? (mCodecInfoLen + datalen) : 1024;
            mCodecInfo = (u8*)malloc(mCodecInfoMaxLen);


        } else {

            mCodecInfoMaxLen = 2*(mCodecInfoLen + datalen);

            u8* newBuffer = (u8*)malloc(mCodecInfoMaxLen);
            memcpy(newBuffer, mCodecInfo, mCodecInfoLen);
            free(mCodecInfo);

            mCodecInfo = newBuffer;

        }
    }

    memcpy(mCodecInfo + mCodecInfoLen, data, datalen);
    mCodecInfoLen += datalen;

}


void VideoDecoder::parseAVCExtraData(u8* extradata, int extradatalen) {

    u8 startCode[4] = {0, 0, 0, 1};

    int offset = 5;
    int spsnum = extradata[offset]&0x1F;
    offset += 1;

    u32 spslen = 0;
    u8* spslenptr = (u8*)&spslen;
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

    u32 ppslen = 0;
    u8* ppslenptr = (u8*)&ppslen;
    ppslenptr[0] = extradata[offset+1];
    ppslenptr[1] = extradata[offset];
    offset += 2;

    // copy pps
    copyCodecInfo(startCode, 4);
    copyCodecInfo(&extradata[offset], ppslen);

     printf("parse AVC pps len %d\n", ppslen);

}

void VideoDecoder::parseHEVCExtraData(u8* extradata, int extradatalen) {

    u8 startCode[4] = {0, 0, 0, 1};

    int offset = 22;

    int nalsnum = extradata[offset];
    offset++;

    printf("parse HEVC nalsnum %d\n", nalsnum);

    for (int i = 0; i < nalsnum; i++) {

        int naltype = extradata[offset]&0x3F;
        offset++;

        u32 onenalnum = 0;
        u8* onenalnumptr = (u8*)&onenalnum;
        onenalnumptr[0] = extradata[offset+1];
        onenalnumptr[1] = extradata[offset];
        offset+=2;

        printf("parse HEVC naltype %d onenalnum %d\n", naltype, onenalnum);

        for (int j = 0; j < onenalnum; j++) {

            u32 onenallen = 0;
            u8* onenallenptr = (u8*)&onenallen;
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

        if (format.compare("avc") == 0) {

            videoformat = Format_AVC;
            parseAVCExtraData((u8*)extra.data(), extra.length());

        } else if (format.compare("annexb") == 0) {

            videoformat = Format_AVC_AnnexB;
        } else {

            printf("Video Decoder not support vtype %s, format %s \n", vtype.c_str(), format.c_str());
            return;
        }

        mDecoderV = new DecoderAVC(this);

    } else if (vtype.compare("hevc") == 0) {

        videotype = Video_H265;

        if (format.compare("hvcc") == 0) {

            videoformat = Format_HVCC;
            parseHEVCExtraData((u8*)extra.data(), extra.length());

        } else if (format.compare("annexb") == 0) {

            videoformat = Format_HEVC_AnnexB;
        }
         else {

            printf("Video Decoder not support vtype %s, format %s \n", vtype.c_str(), format.c_str());
             return;
        }

         mDecoderV = new DecoderHEVC(this);

    } else {

        printf("Video Decoder not support vtype %s, format %s \n", vtype.c_str(), format.c_str());
        return;
    }

    mVType = videotype;
    mVFormat = videoformat;

    mDecoderV->init();
    
    mInit = true;
}

bool VideoDecoder::convertAnnexB(u8* data, int datalen) {

    int offset = 0;
    bool bf = false; //检测 帧里是否有codec 信息

    while(offset < datalen) {

        u32 nallen = 0;
        u8* nallenptr = (u8*)&nallen;
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

int VideoDecoder::addCodecInfo(u8* data, int datalen) {

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

void  VideoDecoder::decode(string input, u32 isKeyFrame, u32 timestamp)
{
    if (!mInit) {

        printf("VideoDecoder has not Init when decode \n");
        return;
    }

    u32 bufferLen = input.length();
    u8* buffer = (u8*)input.data();

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

    mJsObject.call<void>("videoInfo", mVType, mVideoWith, mVideoHeight);
}

void VideoDecoder::yuvData(unsigned char* yuv, unsigned int timestamp) {

    int size = mVideoWith*mVideoHeight;
    unsigned char* yuvArray[3] = {yuv, yuv + size, yuv + size*5/4};

    // printf("yuv %d %d %d %d %d %d\n", yuv[0], yuv[1], yuv[2], yuv[mVideoWith*mVideoHeight/2], yuv[mVideoWith*mVideoHeight/2+1], yuv[mVideoWith*mVideoHeight/2+2]);
    mJsObject.call<void>("yuvData", (u32)yuvArray, timestamp);

}



EMSCRIPTEN_BINDINGS(my_module) {
     class_<VideoDecoder>("VideoDecoder")
    .constructor<val>()
    .function("setCodec", &VideoDecoder::setCodec)
    .function("decode", &VideoDecoder::decode)
    .function("clear", &VideoDecoder::clear);
}
