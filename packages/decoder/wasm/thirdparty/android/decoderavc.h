#ifndef  __DECODER_AVC_H__
#define  __DECODER_AVC_H__

#include "decodervideo.h"

class AVCCodec;

class DecoderAVC : public DecoderVideo
{

    public:

         void    *mCodecCtx;            // Codec context
         int      mVideoWith;          
         int      mVideoHeight;
         unsigned char* mYUV;
       
        AVCCodec* mCodec;


    public:

        DecoderAVC(DecoderVideoObserver* obs);

        virtual void init();
        virtual void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp);
        virtual ~DecoderAVC();
};




#endif