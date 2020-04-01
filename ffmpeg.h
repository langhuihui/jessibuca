#pragma once
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}
const int SamplingFrequencies[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, 0, 0, 0};

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

class FFmpegAAC : public FFmpeg
{
public:
    struct SwrContext *au_convert_ctx = nullptr;
    FFmpegAAC()
    {
        emscripten_log(0, "FFMpegAAC init");
    }
    ~FFmpegAAC()
    {
        swr_free(&au_convert_ctx);
        emscripten_log(0, "FFMpegAAC destory");
    }
    bool decode(IOBuffer &input)
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
            u8 config1 = input[0];
            u8 config2 = input[1];
            initCodec(AV_CODEC_ID_AAC);
            dec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
            dec_ctx->sample_rate = SamplingFrequencies[((config1 & 0x7) << 1) | (config2 >> 7)];
            dec_ctx->channels = (config2 >> 3) & 0x0F;
            dec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
            // dec_ctx->request_sample_fmt = AV_SAMPLE_FMT_S16;

            emscripten_log(0, "aac samplerate:%d channels:%d", dec_ctx->sample_rate, dec_ctx->channels);
            auto ret = avcodec_open2(dec_ctx, codec, NULL);
        }
        return 0;
    }
};
class FFmpegAVC : public FFmpeg, public VideoDecoder
{
public:
    FFmpegAVC()
    {
        emscripten_log(0, "FFMpegAVC init");
    }
    ~FFmpegAVC()
    {
        emscripten_log(0, "FFMpegAVC destory");
    }
    void clear() override
    {
        videoWidth = 0;
        videoHeight = 0;
        VideoDecoder::clear();
        FFmpeg::clear();
    }
    void decodeH264Header(IOBuffer &data) override
    {
        initCodec(AV_CODEC_ID_H264);
        dec_ctx->extradata_size = data.length;
        dec_ctx->extradata = (u8 *)malloc(dec_ctx->extradata_size);
        memcpy(dec_ctx->extradata, (const u8 *)data, dec_ctx->extradata_size);
        auto ret = avcodec_open2(dec_ctx, codec, NULL);
        //emscripten_log(0, "avcodec_open2:%d", ret);
    }
    void decodeBody(IOBuffer &data) override
    {
        // int NALUnitLength = 0;
        // while (data.length > 4)
        // {
        // 	data.read4B(NALUnitLength);
        //     data<<=4;
        // 	_decode(data(0, NALUnitLength+4));
        // 	data >>= NALUnitLength+4;
        // }
        _decode(data);
    }
    void _decode(IOBuffer data) override
    {
        // emscripten_log(0, "len:%d", data.length);
        // int ret = av_parser_parse2(parser, dec_ctx, &pkt->data, &pkt->size,
        //                            (const u8 *)(data), data.length, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        //                            emscripten_log(0, "ffmpeg pkt->size:%d", pkt->size);
        // if (ret >= 0 && pkt->size)
        // {
        int ret = 0;
        pkt->data = (u8 *)(data);
        pkt->size = data.length;
        ret = avcodec_send_packet(dec_ctx, pkt);
        while (ret >= 0)
        {
            ret = avcodec_receive_frame(dec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return;
            p_yuv[0] = (u32)frame->data[0];
            p_yuv[1] = (u32)frame->data[1];
            p_yuv[2] = (u32)frame->data[2];
            if (videoWidth != frame->width || videoHeight != frame->height)
                decodeVideoSize(frame->width, frame->height);
            decodeYUV420();
        }
        // }
        // else
        // {
        //     emscripten_log(0, "ffmpeg decode ret:%d", ret);
        // }
    }
};
