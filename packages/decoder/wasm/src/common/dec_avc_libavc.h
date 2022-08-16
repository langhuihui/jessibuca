#ifndef  __DECODER_AVC_H__
#define  __DECODER_AVC_H__

#include "decodervideo.h"

class AVCCodec;

class Decoder_AVC_Android : public DecoderVideo
{

    public:

         void    *mCodecCtx;            // Codec context
         int      mVideoWith;          
         int      mVideoHeight;
         unsigned char* mYUV;
       
        AVCCodec* mCodec;


    public:

        Decoder_AVC_Android(DecoderVideoObserver* obs);

        virtual void init();
        virtual void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp);
        virtual ~Decoder_AVC_Android();
};




#endif