#ifndef  __DECODER_HEVC_LIBHEVC_H__
#define  __DECODER_HEVC_LIBHEVC_H__

#include "dec_video_base.h"

class HEVCCodec;

class Decoder_HEVC_LIBHEVC : public DecoderVideoBase
{

    public:

         void    *mCodecCtx;            // Codec context
         int      mVideoWith;          
         int      mVideoHeight;

        HEVCCodec* mCodec;


    public:

        Decoder_HEVC_LIBHEVC(DecoderVideoBaseObserver* obs);

        virtual void init(int vtype,  unsigned char* extraData, unsigned int extraDataSize);
        virtual void decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp);
        virtual ~Decoder_HEVC_LIBHEVC();
};




#endif