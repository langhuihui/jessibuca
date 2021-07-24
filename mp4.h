int write_packet(void *opaque, unsigned char *buf, int buf_size);
class MP4
{
public:
    AVFormatContext *output_fmtctx = NULL;
    AVOutputFormat *pOutFmt = NULL;
    u8 *buf;
    val jsObject;
    MP4(val &&v) : jsObject(forward<val>(v))
    {
        if (avformat_alloc_output_context2(&output_fmtctx, NULL, "mp4", NULL) < 0)
        {
            //av_log(NULL, AV_LOG_ERROR, "bbs.chinaffmpeg.org 孙悟空 Cannot open the file %s\n", "./fuck.mp4");
        }
        pOutFmt = output_fmtctx->oformat;
        buf = (u8 *)av_malloc(4096);
        output_fmtctx->pb = avio_alloc_context(buf, 4096, 1, this, NULL, write_packet, NULL);
        avformat_write_header(output_fmtctx, NULL);
    }
    virtual ~MP4()
    {
        av_write_trailer(output_fmtctx);
        avio_close(output_fmtctx->pb);
        avformat_free_context(output_fmtctx);
    }
    int write(AVPacket *pkt)
    { //av_write_frame
        return av_interleaved_write_frame(output_fmtctx, pkt);
    }
    int getPoint()
    {
        return (int)this;
    }
};
int write_packet(void *opaque, unsigned char *buf, int buf_size)
{
    MP4 *mp4 = (MP4 *)opaque;
    mp4->jsObject.call<void>("write", buf, buf_size);
};
#define FUNC(name) function(#name, &MP4::name)
// #undef PROP
// #define PROP(name) property(#name, &MP4::get##name, &MP4::set##name)
EMSCRIPTEN_BINDINGS(MP4)
{
    class_<MP4>("MP4")
        .constructor<val>()
        .FUNC(getPoint);
}