#pragma once
extern "C" {
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
    MemoryStream extradata;
    FFmpeg() : pkt(av_packet_alloc()), frame(av_frame_alloc())
    {
        emscripten_log(0, "FFMpeg init");
    }
    ~FFmpeg()
    {
        emscripten_log(0, "FFMpeg destory");
        av_parser_close(parser);
        avcodec_free_context(&dec_ctx);
        av_frame_free(&frame);
        av_packet_free(&pkt);
    }
    void clear() override{
        VideoDecoder::clear();
        extradata.clear();
         av_parser_close(parser);
         avcodec_free_context(&dec_ctx);
    }
    void decodeHeader(MemoryStream &data, int codec_id) override
    {
        codec = avcodec_find_decoder(codec_id == 7 ? AV_CODEC_ID_H264 : AV_CODEC_ID_H265);
        emscripten_log(0, "codec = %d,ptr = %d", codec_id,codec);
        parser = av_parser_init(codec->id);
        dec_ctx = avcodec_alloc_context3(codec);
        if (codec_id == 7)
        {
            u8 lengthSizeMinusOne = data[9];
            lengthSizeMinusOne &= 0x03;
            NAL_unit_length = lengthSizeMinusOne;

            data >>= 5;
            extradata << data;
            extradata.offset = data.offset;
            dec_ctx->extradata = (u8 *)(const u8 *)extradata;
            dec_ctx->extradata_size = extradata.length();
            avcodec_open2(dec_ctx, codec, NULL);
            // int spsLen = 0;
            // int ppsLen = 0;
            // data.read2B(spsLen);
            // if (spsLen > 0) {
            // 	_decode((const char*)data,spsLen);
            // 	data >>= spsLen;
            // }
            // data >>= 1;
            // data.read2B(ppsLen);
            // if (ppsLen > 0) {
            // 	_decode((const char*)data,ppsLen);
            // }
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
    void decode(MemoryStream &data) override
    {
        data >>= 5;
        _decode((const char *)data, data.length());
    }
    void _decode(const char *data, int len) override
    {
        int ret = av_parser_parse2(parser, dec_ctx, &pkt->data, &pkt->size,
                                   (const u8 *)(data), len, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
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
                if (videoWidth == 0)
                    decodeVideoSize(frame->width, frame->height);
                decodeYUV420();
            }
        }
    }
};
