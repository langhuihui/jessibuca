#pragma once
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}
#include "VideoDecoder.h"
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
    virtual ~FFmpeg()
    {
        av_frame_free(&frame);
        av_packet_free(&pkt);
        clear();
    }
    virtual void clear()
    {
        av_parser_close(parser);
        avcodec_free_context(&dec_ctx);
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
    int decode(IOBuffer &input)
    {
        if (input.readB<1, u8>())
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
                return frame->nb_samples * bytes_per_sample * dec_ctx->channels;
            }
        }
        else
        {
            // u8 config1 = input[0];
            // u8 config2 = input[1];
            initCodec(AV_CODEC_ID_AAC);
            // dec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
            // dec_ctx->channels = (config2 >> 3) & 0x0F;
            // dec_ctx->sample_rate = SamplingFrequencies[((config1 & 0x7) << 1) | (config2 >> 7)];
            // dec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
            // emscripten_log(0, "aac samplerate:%d channels:%d", dec_ctx->sample_rate, dec_ctx->channels);
            dec_ctx->extradata_size = input.length;
            dec_ctx->extradata = (u8 *)malloc(dec_ctx->extradata_size);
            memcpy(dec_ctx->extradata, (const u8 *)input, dec_ctx->extradata_size);
            auto ret = avcodec_open2(dec_ctx, codec, NULL);
        }
        return 0;
    }
};
class FFmpegVideoDecoder : public FFmpeg, public VideoDecoder
{
public:
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
        VideoDecoder::clear();
        FFmpeg::clear();
        if (p_yuv[0])
            free((void *)p_yuv[0]);
        if (p_yuv[1])
            free((void *)p_yuv[1]);
        if (p_yuv[2])
            free((void *)p_yuv[2]);
    }
    void decodeH264Header(IOBuffer &data) override
    {
        initCodec(AV_CODEC_ID_H264);
        dec_ctx->extradata_size = data.length;
        dec_ctx->extradata = (u8 *)malloc(dec_ctx->extradata_size);
        memcpy(dec_ctx->extradata, (const u8 *)data, dec_ctx->extradata_size);
        avcodec_open2(dec_ctx, codec, NULL);
    }
    void decodeH265Header(IOBuffer &data) override{
        initCodec(AV_CODEC_ID_H265);
        dec_ctx->extradata_size = data.length;
        dec_ctx->extradata = (u8 *)malloc(dec_ctx->extradata_size);
        memcpy(dec_ctx->extradata, (const u8 *)data, dec_ctx->extradata_size);
        avcodec_open2(dec_ctx, codec, NULL);
    }
    void decodeBody(IOBuffer &data) override
    {
        _decode(data);
    }
    void _decode(IOBuffer data) override
    {
        int ret = 0;
        pkt->data = (u8 *)(data);
        pkt->size = data.length;
        ret = avcodec_send_packet(dec_ctx, pkt);
        while (ret >= 0)
        {
            ret = avcodec_receive_frame(dec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return;
            if (videoWidth != frame->width || videoHeight != frame->height)
            {
                decodeVideoSize(frame->width, frame->height);
                int size = videoWidth * videoHeight;
                p_yuv[0] = (u32)malloc(size * 3 >> 1);
                p_yuv[1] = p_yuv[0] + size;
                p_yuv[2] = p_yuv[1] + (size >> 2);
            }
            u32 dst = p_yuv[0];
            for (int i = 0; i < videoHeight; i++)
            {
                memcpy((u8 *)dst, (const u8 *)(frame->data[0] + i * frame->linesize[0]), videoWidth);
                dst += videoWidth;
            }
            dst = p_yuv[1];
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
            decodeYUV420();
        }
    }
};
