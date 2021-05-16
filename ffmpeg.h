#pragma once
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}
const int SamplingFrequencies[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0};
const int AudioObjectTypes[] = {};
class FFmpeg
{
public:
    AVCodec *codec;
    AVCodecParserContext *parser = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    AVFrame *frame;
    AVPacket *pkt;
    val *jsObject;
    bool initialized = false;
    u32 timestamp = 0;
    FFmpeg() : pkt(av_packet_alloc()), frame(av_frame_alloc())
    {
    }
    void initCodec(enum AVCodecID id)
    {
        if (dec_ctx != nullptr)
            clear();
        codec = avcodec_find_decoder(id);
        parser = av_parser_init(codec->id);
        dec_ctx = avcodec_alloc_context3(codec);
    }
    void initCodec(enum AVCodecID id, IOBuffer &input)
    {
        initCodec(id);
        dec_ctx->extradata_size = input.length;
        dec_ctx->extradata = (u8 *)malloc(dec_ctx->extradata_size);
        memcpy(dec_ctx->extradata, (const u8 *)input, dec_ctx->extradata_size);
        avcodec_open2(dec_ctx, codec, NULL);
        initialized = true;
    }
    virtual ~FFmpeg()
    {
        av_frame_free(&frame);
        av_packet_free(&pkt);
        clear();
    }
    virtual int decode(IOBuffer &input)
    {
        int ret = 0;
        pkt->data = (u8 *)(input);
        pkt->size = input.length;
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
    virtual void _decode() = 0;
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
            dec_ctx = nullptr;
        }
    }
};

class FFmpegAudioDecoder : public FFmpeg
{
public:
    struct SwrContext *au_convert_ctx = nullptr;
    FFmpegAudioDecoder()
    {
        emscripten_log(0, "FFMpegAudioDecoder init");
    }
    ~FFmpegAudioDecoder()
    {
        swr_free(&au_convert_ctx);
        emscripten_log(0, "FFMpegAudioDecoder destory");
    }
    int decode(IOBuffer &input) override
    {

        unsigned char flag = 0;
        input.readB<1>(flag);
        unsigned char audioType = flag >> 4;
        if (initialized)
        {
            if (audioType == 10)
            {
                input.readB<1, u8>();
            }
            return FFmpeg::decode(input);
        }
        else
        {
            switch (audioType)
            {
            case 10:
                if (!input.readB<1, u8>())
                {
                    initCodec(AV_CODEC_ID_AAC, input);
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
                jsObject->call<void>("initAudioPlanar", dec_ctx->channels, dec_ctx->sample_rate);
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
        jsObject->call<void>("playAudioPlanar", int(frame->data), bytesCount, timestamp);
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

    FFmpegVideoDecoder()
    {
        emscripten_log(0, "FFMpegVideoDecoder init");
    }
    ~FFmpegVideoDecoder()
    {
        emscripten_log(0, "FFMpegVideoDecoder destory");
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
    bool isAVCSequence(IOBuffer &data)
    {
        return data[0] >> 4 == 1 && data[1] == 0; //0为AVCSequence Header，1为AVC NALU，2为AVC end ofsequence
    }
    int decode(IOBuffer &data) override
    {
        if (!initialized && isAVCSequence(data))
        {
            int codec_id = data[0] & 0x0F;
            data >>= 5;
            //emscripten_log(0, "codec = %d", codec_id);
            switch (codec_id)
            {
            case 7:
                initCodec(AV_CODEC_ID_H264, data);
                break;
            case 12:
                initCodec(AV_CODEC_ID_H265, data);
                break;
            default:
                emscripten_log(0, "codec not support: %d", codec_id);
                return -1;
            }
        }
        else
        {
            data >>= 2;
            compositionTime = data.readUInt24B();
            return FFmpeg::decode(data);
        }
    }
    void _decode() override
    {
        if (videoWidth != frame->width || videoHeight != frame->height)
        {
            videoWidth = frame->width;
            videoHeight = frame->height;
            jsObject->call<void>("setVideoSize", videoWidth, videoHeight);
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
        jsObject->call<void>("draw", compositionTime, timestamp, y, u, v);
    }
};
