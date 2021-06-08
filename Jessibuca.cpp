#ifdef EDITTIME
#undef __cplusplus
#define __cplusplus 201703L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
// #include <functional>
// #include <map>
// #include <queue>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
// #include <time.h>
// #include <regex>
using namespace std;
using namespace emscripten;
// #include "slice.h"
typedef unsigned char u8;
// typedef signed char i8;
// typedef unsigned short u16;
// typedef signed short i16;
typedef unsigned int u32;
// typedef signed int i32;
#define PROP(name, type)                        \
    type name;                                  \
    val get##name() const                       \
    {                                           \
        return val(name);                       \
    }                                           \
    void set##name(val value)                   \
    {                                           \
        name = value.as<type>();                \
        emscripten_log(0, #name " = %d", name); \
    }

extern "C"
{
#include <libavcodec/avcodec.h>
//#include <libswresample/swresample.h>
}
//const int SamplingFrequencies[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0};
//const int AudioObjectTypes[] = {};
class FFmpeg
{
public:
    AVCodec *codec = nullptr;
    AVCodecParserContext *parser = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    AVFrame *frame;
    AVPacket *pkt;
    val jsObject;
    bool initialized = false;
    FFmpeg(val &&v) : jsObject(forward<val>(v))
    {
    }
    void initCodec(enum AVCodecID id)
    {
        if (dec_ctx != nullptr)
        {
            clear();
        }
        pkt = av_packet_alloc();
        codec = avcodec_find_decoder(id);
        parser = av_parser_init(codec->id);
        dec_ctx = avcodec_alloc_context3(codec);
        frame = av_frame_alloc();
    }
    void initCodec(enum AVCodecID id, string input)
    {
        initCodec(id);
        dec_ctx->extradata_size = input.length();
        dec_ctx->extradata = (u8 *)input.data();
        // // dec_ctx->extradata = (u8 *)malloc(dec_ctx->extradata_size);
        // // memcpy(dec_ctx->extradata, input.c_str(), dec_ctx->extradata_size);
        avcodec_open2(dec_ctx, codec, NULL);
        initialized = true;
    }
    virtual ~FFmpeg()
    {
        clear();
    }
    virtual int decode(string input)
    {
        int ret = 0;
        pkt->data = (u8 *)(input.data());
        pkt->size = input.length();
        ret = avcodec_send_packet(dec_ctx, pkt);
        while (ret >= 0)
        {
            ret = avcodec_receive_frame(dec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return 0;
            _decode();
        }
        return 0;
    }
    virtual void _decode(){};
    virtual void clear()
    {
        if (parser)
        {
            av_parser_close(parser);
            parser = nullptr;
        }
        if (dec_ctx)
        {
            avcodec_free_context(&dec_ctx);
        }
        if(frame)
        {
           av_frame_free(&frame);
        }
        if(pkt)
        {
          av_packet_free(&pkt);
        }
        codec = nullptr;
        initialized = false;
    }
};

class FFmpegAudioDecoder : public FFmpeg
{
public:
//    struct SwrContext *au_convert_ctx = nullptr;
    FFmpegAudioDecoder(val &&v) : FFmpeg(move(v))
    {
//        emscripten_log(0, "FFMpegAudioDecoder init");
    }
    ~FFmpegAudioDecoder()
    {
//        swr_free(&au_convert_ctx);
//        emscripten_log(0, "FFMpegAudioDecoder destory");
    }
    void clear() override
    {
        FFmpeg::clear();
    }
    int decode(string input) override
    {
        u8 flag = (u8)input[0];
        u8 audioType = flag >> 4;
        if (initialized)
        {
            if (audioType == 10)
            {
                return FFmpeg::decode(input.substr(2));
            }
            return FFmpeg::decode(input.substr(1));
        }
        else
        {
            switch (audioType)
            {
            case 10:
                if (!input[1])
                {
                    initCodec(AV_CODEC_ID_AAC, input.substr(2));
                }
                break;
            case 7:
                initCodec(AV_CODEC_ID_PCM_ALAW);
                break;
            case 8:
                initCodec(AV_CODEC_ID_PCM_MULAW);
                break;
            default:
                emscripten_log(0, "audio type not support:%d", audioType);
                break;
            }
            if (initialized)
            {
                jsObject.call<void>("initAudioPlanar", dec_ctx->channels, dec_ctx->sample_rate);
            }
        }
        return 0;
    }
    void _decode() override
    {
        // emscripten_log(0, "aac channel_layout:%d", dec_ctx->channel_layout);
        //  if(au_convert_ctx==nullptr){
        //    // out_buffer = (uint8_t *)av_malloc(av_get_bytes_per_sample(dec_ctx->sample_fmt)*dec_ctx->channels*dec_ctx->frame_size);
        //    au_convert_ctx = swr_alloc();
        //     au_convert_ctx = swr_alloc_set_opts(au_convert_ctx,
        //                                         AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, dec_ctx->sample_rate,
        //                                         dec_ctx->channel_layout, dec_ctx->sample_fmt, dec_ctx->sample_rate,
        //                                         0, NULL);
        //     swr_init(au_convert_ctx);
        // }
        // // // 转换
        // swr_convert(au_convert_ctx, &output, frame->nb_samples, (const uint8_t **)frame->data , frame->nb_samples);

        int bytes_per_sample = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        // int samplesBytes = frame->nb_samples<<1;
        // int outputInterval = bytes_per_sample*dec_ctx->channels;
        // for(int i =0;i<frame->nb_samples;i++){
        //     u8* start = &output[i*4];
        //     start[0] = out_buffer[0][i*2];
        //     start[1] = out_buffer[0][i*2+1];
        //     start[2] = out_buffer[1][i*2];
        //     start[3] = out_buffer[1][i*2+1];
        // }
        //memcpy(output,frame->data[0],samplesBytes);
        //memcpy(output+samplesBytes,frame->data[1],samplesBytes);
        //    memcpy(output,frame->data[0],frame->nb_samples<<1);
        int bytesCount = frame->nb_samples * bytes_per_sample * dec_ctx->channels;
        jsObject.call<void>("playAudioPlanar", int(frame->data), bytesCount);
    }
};
class FFmpegVideoDecoder : public FFmpeg
{
public:
    u32 videoWidth = 0;
    u32 videoHeight = 0;
    u32 y = 0;
    u32 u = 0;
    u32 v = 0;
    int NAL_unit_length = 0;
    u32 compositionTime = 0;

    FFmpegVideoDecoder(val &&v) : FFmpeg(move(v))
    {
//        emscripten_log(0, "FFMpegVideoDecoder init");
    }
    ~FFmpegVideoDecoder()
    {
//        emscripten_log(0, "FFMpegVideoDecoder destory");
    }
    void clear() override
    {
        videoWidth = 0;
        videoHeight = 0;
        FFmpeg::clear();
        if (y)
        {
            free((void *)y);
            y = 0;
        }
    }
    int decode(string data) override
    {
        if (!initialized)
        {
            int codec_id = ((int)data[0]) & 0x0F;
            if (((int)(data[0]) >> 4) == 1 && data[1] == 0)
            {
//                emscripten_log(0, "codec = %d", codec_id);
                switch (codec_id)
                {
                case 7:
                    initCodec(AV_CODEC_ID_H264, data.substr(5));
                    break;
                case 12:
                    initCodec(AV_CODEC_ID_H265, data.substr(5));
                    break;
                default:
                    emscripten_log(0, "codec not support: %d", codec_id);
                    return -1;
                }
            }
        }
        else
        {
            compositionTime = (((u32)data[2]) << 16) + (((u32)data[3]) << 8) + (u32)data[4];
            return FFmpeg::decode(data.substr(5));
        }
        return 0;
    }
    void _decode() override
    {
        if (videoWidth != frame->width || videoHeight != frame->height)
        {
            videoWidth = frame->width;
            videoHeight = frame->height;
            jsObject.call<void>("setVideoSize", videoWidth, videoHeight);
            int size = videoWidth * videoHeight;
            if (y)
                free((void *)y);
            y = (u32)malloc(size * 3 >> 1);
            u = y + size;
            v = u + (size >> 2);
        }
        u32 dst = y;
        for (int i = 0; i < videoHeight; i++)
        {
            memcpy((u8 *)dst, (const u8 *)(frame->data[0] + i * frame->linesize[0]), videoWidth);
            dst += videoWidth;
        }
        dst = u;
        int halfh = videoHeight >> 1;
        int halfw = videoWidth >> 1;
        for (int i = 0; i < halfh; i++)
        {
            memcpy((u8 *)dst, (const u8 *)(frame->data[1] + i * frame->linesize[1]), halfw);
            dst += halfw;
        }

        for (int i = 0; i < halfh; i++)
        {
            memcpy((u8 *)dst, (const u8 *)(frame->data[2] + i * frame->linesize[2]), halfw);
            dst += halfw;
        }
        jsObject.call<void>("draw", compositionTime, y, u, v);
    }
};

#define FUNC(name) function(#name, &FFmpegAudioDecoder::name)
#undef PROP
#define PROP(name) property(#name, &FFmpegAudioDecoder::get##name, &FFmpegAudioDecoder::set##name)
EMSCRIPTEN_BINDINGS(FFmpegAudioDecoder)
{
    class_<FFmpegAudioDecoder>("AudioDecoder")
        .constructor<val>()
        .FUNC(clear)
        .FUNC(decode);
}
#undef FUNC
#define FUNC(name) function(#name, &FFmpegVideoDecoder::name)
#undef PROP
#define PROP(name) property(#name, &FFmpegVideoDecoder::get##name, &FFmpegVideoDecoder::set##name)
EMSCRIPTEN_BINDINGS(FFmpegVideoDecoder)
{
    class_<FFmpegVideoDecoder>("VideoDecoder")
        .constructor<val>()
        .FUNC(clear)
        .FUNC(decode);
}