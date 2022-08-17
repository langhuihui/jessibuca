#ifndef  __DECODER_AVC_LIBAVC_H__
#define  __DECODER_AVC_LIBAVC_H__

#include "dec_video_base.h"

class AVCCodec;

class Decoder_AVC_LIBAVC : public DecoderVideoBase
{

    public:

         void    *mCodecCtx;            // Codec context
         int      mVideoWith;          
         int      mVideoHeight;
       
        AVCCodec* mCodec;


    public:

        Decoder_AVC_LIBAVC(DecoderVideoBaseObserver* obs);

        virtual void init(int vtype,  unsigned char* extraData, unsigned int extraDataSize);
        virtual void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp);
        virtual ~Decoder_AVC_LIBAVC();
};




#endif