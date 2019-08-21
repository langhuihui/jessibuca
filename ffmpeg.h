#pragma once
extern "C"
{
#include <libavcodec/avcodec.h>
}

class FFmpeg : public VideoDecoder
{
  public:
    AVCodec *codec;
    AVCodecParserContext *parser;
    AVCodecContext *dec_ctx = NULL;
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
        VideoDecoder::clear();
        av_parser_close(parser);
        free(dec_ctx->extradata);
        avcodec_free_context(&dec_ctx);
    }
    void decodeHeader(IOBuffer& data, int codec_id) override
    {
        codec = avcodec_find_decoder(codec_id == 7 ? AV_CODEC_ID_H264 : AV_CODEC_ID_H265);
        emscripten_log(0, "codec = %d,ptr = %d", codec_id, codec);
        parser = av_parser_init(codec->id);
        dec_ctx = avcodec_alloc_context3(codec);
        if (codec_id == 7)
        {
            data >>= 5;
            //dec_ctx->extradata = (u8 *)(const u8 *)data;
            dec_ctx->extradata_size = data.length;
            dec_ctx->extradata = (u8 *)malloc(dec_ctx->extradata_size);
            memcpy( dec_ctx->extradata,(const u8 *)data,dec_ctx->extradata_size);
            auto ret = avcodec_open2(dec_ctx, codec, NULL);
            emscripten_log(0, "avcodec_open2:%d", ret);
        }
        else
        {
            // u8 lengthSizeMinusOne = data[27];
            // lengthSizeMinusOne &= 0x03;
            // NAL_unit_length = lengthSizeMinusOne;
            // data.offset = 31 + index;
            // int vps=0,sps=0,pps=0;
            // data.read2B(vps);
            // _decode((const char*)data,vps);
            // data>>=vps;
            // data>>=3;
            // data.read2B(sps);
            // _decode((const char*)data,sps);
            // data>>=sps;
            // data>>=3;
            // data.read2B(pps);
            // _decode((const char*)data,pps);
        }
    }
    void decode(IOBuffer&data) override
    {
        _decode(data);
    }
    void _decode(IOBuffer data) override
    {  //emscripten_log(0, "len:%d", len);
        int ret = av_parser_parse2(parser, dec_ctx, &pkt->data, &pkt->size,
                                   (const u8 *)(data), data.length, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret >= 0 && pkt->size)
        {
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
        }
        else
        {
            emscripten_log(0, "ffmpeg decode ret:%d", ret);
        }
    }
};
