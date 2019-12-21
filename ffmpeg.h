#pragma once
extern "C"
{
#include <libavcodec/avcodec.h>
}

class FFmpeg : public VideoDecoder
{
public:
    AVCodec *codec;
    AVCodecParserContext *parser = nullptr;
    AVCodecContext *dec_ctx = nullptr;
    AVFrame *frame;
    AVPacket *pkt;
    FFmpeg() : pkt(av_packet_alloc()), frame(av_frame_alloc())
    {
        emscripten_log(0, "FFMpeg init");
    }
    ~FFmpeg()
    {
        emscripten_log(0, "FFMpeg destory");
        clear();
        av_frame_free(&frame);
        av_packet_free(&pkt);
    }
    void clear() override
    {
        videoWidth = 0;
        videoHeight = 0;
        VideoDecoder::clear();
        av_parser_close(parser);
        // free(dec_ctx->extradata);
        avcodec_free_context(&dec_ctx);
    }
    void decodeH264Header(IOBuffer &data) override
    {
        if (dec_ctx != nullptr)
            clear();
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        parser = av_parser_init(codec->id);
        dec_ctx = avcodec_alloc_context3(codec);
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
