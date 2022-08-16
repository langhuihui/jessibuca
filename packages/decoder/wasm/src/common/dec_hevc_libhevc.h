#ifndef  __DECODER_HEVC_H__
#define  __DECODER_HEVC_H__

#include "decodervideo.h"

class HEVCCodec;

class Decoder_HEVC_Android : public DecoderVideo
{

    public:

         void    *mCodecCtx;            // Codec context
         int      mVideoWith;          
         int      mVideoHeight;
         unsigned char* mYUV;

        HEVCCodec* mCodec;


    public:

        Decoder_HEVC_Android(DecoderVideoObserver* obs);

        virtual void init();
        virtual void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp);
        virtual ~Decoder_HEVC_Android();
};




#endif