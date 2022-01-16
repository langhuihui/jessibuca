int write_packet(void *opaque, unsigned char *buf, int buf_size);
#define OUT_FILE "/tmp/save.mp4"
class MP4
{
public:
    AVFormatContext *output_fmtctx = NULL;
    AVOutputFormat *pOutFmt = NULL;
    u8 *buf;
    val jsObject;
    int count = 0;
    int headWriten = 0;
    MP4(val &&v) : jsObject(forward<val>(v))
    {
        if (avformat_alloc_output_context2(&output_fmtctx, NULL, NULL, OUT_FILE) < 0)
        {
            emscripten_log(0, "avformat_alloc_output_context2 faild");
            return;
            //av_log(NULL, AV_LOG_ERROR, "bbs.chinaffmpeg.org 孙悟空 Cannot open the file %s\n", "./fuck.mp4");
        }
        pOutFmt = output_fmtctx->oformat;
        // buf = (u8 *)av_malloc(4096 * 4);
        //output_fmtctx->max_interleave_delta = 1000;
        //output_fmtctx->flush_packets = 1;
        // output_fmtctx->pb = avio_alloc_context(buf, 4096 * 4, 1, this, NULL, write_packet, NULL);
        int ret = avio_open(&output_fmtctx->pb, OUT_FILE, AVIO_FLAG_READ_WRITE);
        emscripten_log(0, "avio_open %d", ret);
    }
    virtual ~MP4()
    {
        int ret = av_write_trailer(output_fmtctx);
        emscripten_log(0, "av_write_trailer %d", ret);
        avio_close(output_fmtctx->pb);
        avformat_free_context(output_fmtctx);
    }
    int write(AVPacket *pkt)
    {
        if (headWriten == 0)
        {
            int ret = avformat_write_header(output_fmtctx, NULL);
            emscripten_log(0, "avformat_write_header %d", ret);
            headWriten = 1;
        }
        int ret = av_write_frame(output_fmtctx, pkt);
        emscripten_log(0, "av_write_frame %d", pkt->pts);
        //int ret = av_interleaved_write_frame(output_fmtctx, pkt);
        // if (count++ >= 10)
        // {
        //     count = 0;
        // ret = av_interleaved_write_frame(output_fmtctx, NULL);
        // }
        return 0;
    }
    int getPoint()
    {
        return (int)this;
    }
    void stop()
    {
        av_write_trailer(output_fmtctx);
        avio_close(output_fmtctx->pb);
    }
};
int write_packet(void *opaque, unsigned char *buf, int buf_size)
{
    //MP4 *mp4 = (MP4 *)opaque;
    emscripten_log(0, "write_packet %d", buf_size);
    //mp4->jsObject.call<void>("write", buf, buf_size);
    return 0;
};
#define FUNC(name) function(#name, &MP4::name)
// #undef PROP
// #define PROP(name) property(#name, &MP4::get##name, &MP4::set##name)
EMSCRIPTEN_BINDINGS(MP4)
{
    class_<MP4>("MP4")
        .constructor<val>()
        .FUNC(stop)
        .FUNC(getPoint);
}
#undef FUNC