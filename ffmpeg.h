#pragma once
#include <libavcodec/avcodec.h>
class FFmpeg :public VideoDecoder{
    public:
    AVCodec *codec;
    AVCodecParserContext *parser;
    AVCodecContext *dec_ctx= NULL;
    AVFrame *frame;
    AVPacket *pkt;
    FFmpeg():pkt(av_packet_alloc()){

    }
    ~FFmpeg(){
        av_parser_close(parser);
        avcodec_free_context(&dec_ctx);
        av_frame_free(&frame);
        av_packet_free(&pkt);
    }
    void decodeHeader(MemoryStream& data, int codec_id)override{
        codec = avcodec_find_decoder(codec_id==12?AV_CODEC_ID_H265:AV_CODEC_ID_H264);
        parser = av_parser_init(codec->id);
        dec_ctx = avcodec_alloc_context3(codec);
        avcodec_open2(dec_ctx, codec, NULL);
        frame = av_frame_alloc();
        VideoDecoder::decodeHeader(data,codec_id);
    }
    void _decode(const char* data, int len) override{
        int ret;
        av_parser_parse2(parser, dec_ctx, &pkt->data, &pkt->size,
                                   (const uint8_t *)data, len, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (pkt->size){
            ret = avcodec_send_packet(dec_ctx, pkt);
            while (ret >= 0) {
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    return;
                p_yuv[0] = (u32)frame->data[0];
                p_yuv[1] = (u32)frame->data[1];
                p_yuv[2] = (u32)frame->data[2];
                if(videoWidth==0)
                    decodeVideoSize(frame->width, frame->height);
                decodeYUV420();
            }
        }
    }
};
