#include "dec_audio_ffmpeg.h"
#include "av_type.h"


extern "C"
{
    #include <libswresample/swresample.h>
}


Decorder_Audio_FFMPEG::Decorder_Audio_FFMPEG(DecoderAudioBaseObserver* obs) : Decorder_Base_FFMPEG(), DecoderAudioBase(obs) {
    
    //指定输出fltp raw 流，其他 采样率，通道数，位深 保持不变
    mAudioFormat = AV_SAMPLE_FMT_FLTP;

    mOutBuffer[0] = nullptr;
    mOutBuffer[1] = nullptr;

    mNotifyAudioParam = false;
}


void Decorder_Audio_FFMPEG::clear() {


    if (mConvertCtx) {
        swr_free(&mConvertCtx);
        mConvertCtx = nullptr;
    }
        
    if (mOutBuffer[0]) {
        free(mOutBuffer[0]);
        mOutBuffer[0] = nullptr;
    }

    mOutBuffer[1] = nullptr;
    mNotifyAudioParam = false;

    Decorder_Base_FFMPEG::clear();
}


Decorder_Audio_FFMPEG::~Decorder_Audio_FFMPEG() {

    clear();

    printf("Decorder_Audio_FFMPEG dealloc \n");
}

void Decorder_Audio_FFMPEG::init(int atype,  unsigned char* extraData, unsigned int extraDataSize)
{
    // printf("Use Audio Decoder, AudioDecoder::setCodec atype %s, extra %d \n", atype.c_str(), extra.length());
    
    clear();

 
    switch (atype)
    {
        case Audio_AAC: {

            Decorder_Base_FFMPEG::initCodec(AV_CODEC_ID_AAC);
            mDecCtx->extradata_size = extraDataSize;
            mDecCtx->extradata = extraData;
            avcodec_open2(mDecCtx, mCodec, NULL);
            break;
        }

        case Audio_PCMU: {

            Decorder_Base_FFMPEG::initCodec(AV_CODEC_ID_PCM_MULAW);
            mDecCtx->channel_layout = AV_CH_LAYOUT_MONO;
            mDecCtx->sample_rate = 8000;
            mDecCtx->channels = 1;
            avcodec_open2(mDecCtx, mCodec, NULL);
 
            break;
        }

        case Audio_PCMA: {

            Decorder_Base_FFMPEG::initCodec(AV_CODEC_ID_PCM_ALAW);
            mDecCtx->channel_layout = AV_CH_LAYOUT_MONO;
            mDecCtx->sample_rate = 8000;
            mDecCtx->channels = 1;
            avcodec_open2(mDecCtx, mCodec, NULL);
     
            break;
        }
    
        default: {

            return;
        }
    }

    mAType = atype;

    mInit = true;
}


void  Decorder_Audio_FFMPEG::decode(unsigned char *buf, unsigned int buflen, unsigned int timestamp)
{
    if (!mInit) {

        return;
    }

    Decorder_Base_FFMPEG::decode(buf, buflen, timestamp);

}

void  Decorder_Audio_FFMPEG::frameReady(unsigned int timestamp)  {

    auto nb_samples = mFrame->nb_samples;

    if (!mNotifyAudioParam) {
        mObserver->audioInfo(mFrame->sample_rate, mFrame->channels);
        mNotifyAudioParam = true;
    }

    // auto bytes_per_sample = av_get_bytes_per_sample(mAudioFormat);
    if (mDecCtx->sample_fmt == mAudioFormat)
    {
        mObserver->pcmData(mFrame->data, nb_samples, timestamp);
        return;
    }
    //s16 -> fltp
    if (!mConvertCtx)
    {
        mConvertCtx  = swr_alloc_set_opts(NULL, mFrame->channel_layout, mAudioFormat, mFrame->sample_rate,
                                            mDecCtx->channel_layout, mDecCtx->sample_fmt, mDecCtx->sample_rate,
                                            0, NULL);
        auto ret = swr_init(mConvertCtx);
        auto out_buffer_size = av_samples_get_buffer_size(NULL, mFrame->channels, nb_samples, mAudioFormat, 0);
        auto buffer = (uint8_t *)av_malloc(out_buffer_size);
        mOutBuffer[0] = buffer;
        mOutBuffer[1] = buffer + (out_buffer_size / 2);
    }
    // // 转换
    auto ret = swr_convert(mConvertCtx , mOutBuffer, nb_samples, (const uint8_t **)mFrame->data, nb_samples);
    while (ret > 0)
    {
        mObserver->pcmData(mOutBuffer, ret, timestamp);
        ret = swr_convert(mConvertCtx , mOutBuffer, nb_samples, (const uint8_t **)mFrame->data, 0);
    }

}