#ifndef  __DECODER_HEVC_H__
#define  __DECODER_HEVC_H__

#include "decodervideo.h"

class HEVCCodec;

class DecoderHEVC : public DecoderVideo
{

    public:

         void    *mCodecCtx;            // Codec context
         int      mVideoWith;          
         int      mVideoHeight;
         unsigned char* mYUV;

        HEVCCodec* mCodec;


    public:

        DecoderHEVC(DecoderVideoObserver* obs);

        virtual void init();
        virtual void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp);
        virtual ~DecoderHEVC();
};




#endif