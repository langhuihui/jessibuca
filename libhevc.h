#pragma once
#include "ihevc_typedefs.h"
#include "iv.h"
#include "ivd.h"
#include "ihevcd_cxa.h"
#define STRIDE 0
#define EXECUTE(ip, op, ...)                                              \
    if (IV_SUCCESS != (ret = ihevcd_cxa_api_function(codec_obj, ip, op))) \
        printf(__VA_ARGS__);
void *ihevca_aligned_malloc(void *pv_ctxt, WORD32 alignment, WORD32 i4_size)
{
    (void)pv_ctxt;
    return malloc(i4_size);
}

void ihevca_aligned_free(void *pv_ctxt, void *pv_buf)
{
    (void)pv_ctxt;
    free(pv_buf);
    return;
}
#define CALL_API(name, errorStr, ...)           \
    {                                           \
        name##_ip_t ip = {                      \
            sizeof(name##_ip_t), __VA_ARGS__};  \
        name##_op_t op = {sizeof(name##_op_t)}; \
        EXECUTE(&ip, &op, errorStr)             \
    }
#define VIDEO_CTL(name, subcmd, errorStr, ...) CALL_API(ihevcd_cxa_ctl_##name, errorStr, IVD_CMD_VIDEO_CTL, (IVD_CONTROL_API_COMMAND_TYPE_T)subcmd, __VA_ARGS__)
class Libhevc : public VideoDecoder
{
public:
    iv_obj_t *codec_obj = NULL;
    CHAR ac_error_str[1000];
    // ivd_out_bufdesc_t ps_out_buf;
    UWORD32 u4_num_bytes_dec = 0;
    UWORD32 u4_ip_frm_ts = 0, u4_op_frm_ts = 0;
    WORD32 ret;
    UWORD32 u4_ip_buf_len;
    UWORD32 frm_cnt = 0;
    ivd_video_decode_ip_t s_video_decode_ip;
    ivd_video_decode_op_t s_video_decode_op;
    Libhevc()
    {
        ihevcd_cxa_create_ip_t s_create_ip;
        ihevcd_cxa_create_op_t s_create_op;
        s_create_ip.s_ivd_create_ip_t.e_cmd = IVD_CMD_CREATE;
        s_create_ip.s_ivd_create_ip_t.u4_share_disp_buf = 0;
        s_create_ip.s_ivd_create_ip_t.e_output_format = IV_YUV_420P;
        s_create_ip.s_ivd_create_ip_t.pf_aligned_alloc = ihevca_aligned_malloc;
        s_create_ip.s_ivd_create_ip_t.pf_aligned_free = ihevca_aligned_free;
        s_create_ip.s_ivd_create_ip_t.pv_mem_ctxt = NULL;
        s_create_ip.s_ivd_create_ip_t.u4_size = sizeof(ihevcd_cxa_create_ip_t);
        s_create_op.s_ivd_create_op_t.u4_size = sizeof(ihevcd_cxa_create_op_t);
        EXECUTE(&s_create_ip, &s_create_op, "Error in Create %8x\n",
                s_create_op.s_ivd_create_op_t.u4_error_code)
        codec_obj = (iv_obj_t *)s_create_op.s_ivd_create_op_t.pv_handle;
        codec_obj->pv_fxns = (void *)&ihevcd_cxa_api_function;
        codec_obj->u4_size = sizeof(iv_obj_t);
        VIDEO_CTL(set_num_cores, IHEVCD_CXA_CMD_CTL_SET_NUM_CORES, "\nError in setting number of cores", 1)
        VIDEO_CTL(set_processor, IHEVCD_CXA_CMD_CTL_SET_PROCESSOR, "\nError in setting Processor type", ARCH_NA, SOC_NA)

        emscripten_log(0, "libhevc init");
    }
    ~Libhevc()
    {
        CALL_API(ivd_delete, "Error in Codec delete", IVD_CMD_DELETE)
        free((void *)p_yuv[0]);
    }

    void decodeH265Header(IOBuffer &data) override
    {
        CALL_API(ivd_ctl_set_config, "\nError in setting the codec in header decode mode", IVD_CMD_VIDEO_CTL, IVD_CMD_CTL_SETPARAMS, IVD_DECODE_HEADER, STRIDE, IVD_SKIP_NONE, IVD_DISPLAY_FRAME_OUT)
        s_video_decode_ip.e_cmd = IVD_CMD_VIDEO_DECODE;
        s_video_decode_ip.u4_ts = u4_ip_frm_ts;
        s_video_decode_ip.u4_size = sizeof(ivd_video_decode_ip_t);
        s_video_decode_op.u4_size = sizeof(ivd_video_decode_op_t);
        u8 lengthSizeMinusOne = data[22];
        lengthSizeMinusOne &= 0x03;
        NAL_unit_length = lengthSizeMinusOne;
        data >>= 26;
        int vps = 0, sps = 0, pps = 0;
        data.read2B(vps);
        s_video_decode_ip.pv_stream_buffer = (void *)data.point();
        s_video_decode_ip.u4_num_Bytes = vps;
        EXECUTE(&s_video_decode_ip, &s_video_decode_op, "\nError in header vps decode %x", s_video_decode_op.u4_error_code)
        data >>= vps;
        data >>= 3;
        data.read2B(sps);
        s_video_decode_ip.pv_stream_buffer = (void *)data.point();
        s_video_decode_ip.u4_num_Bytes = sps;
        EXECUTE(&s_video_decode_ip, &s_video_decode_op, "\nError in header sps decode %x", s_video_decode_op.u4_error_code)
        data >>= sps;
        data >>= 3;
        data.read2B(pps);
        s_video_decode_ip.pv_stream_buffer = (void *)data.point();
        s_video_decode_ip.u4_num_Bytes = pps;
        EXECUTE(&s_video_decode_ip, &s_video_decode_op, "\nError in header pps decode %x", s_video_decode_op.u4_error_code)
        decodeVideoSize(s_video_decode_op.u4_pic_wd, s_video_decode_op.u4_pic_ht);
        CALL_API(ivd_ctl_set_config, "\nError in setting the codec in frame decode mode", IVD_CMD_VIDEO_CTL, IVD_CMD_CTL_SETPARAMS, IVD_DECODE_FRAME, STRIDE, IVD_SKIP_NONE, IVD_DISPLAY_FRAME_OUT)
        s_video_decode_ip.s_out_buffer.u4_num_bufs = 3;
        s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[0] = videoWidth * videoHeight;
        s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[1] = s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[2] = s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[0] >> 2;
        p_yuv[0] = (u32)(s_video_decode_ip.s_out_buffer.pu1_bufs[0] = (UWORD8 *)malloc(6 * s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[1]));
        p_yuv[1] = (u32)(s_video_decode_ip.s_out_buffer.pu1_bufs[1] = (UWORD8 *)p_yuv[0] + s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[0]);
        p_yuv[2] = (u32)(s_video_decode_ip.s_out_buffer.pu1_bufs[2] = (UWORD8 *)p_yuv[1] + s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[1]);
    }
    void _decode(IOBuffer data) override
    {
        s_video_decode_ip.pv_stream_buffer = (void *)data.point();
        s_video_decode_ip.u4_num_Bytes = data.length;
        do
        {
            ret = ihevcd_cxa_api_function(codec_obj, &s_video_decode_ip, &s_video_decode_op);
            s_video_decode_ip.u4_num_Bytes -= s_video_decode_op.u4_num_bytes_consumed;
            s_video_decode_ip.pv_stream_buffer = (UWORD8 *)s_video_decode_ip.pv_stream_buffer + s_video_decode_op.u4_num_bytes_consumed;
            if (ret == IV_SUCCESS)
                decodeYUV420();
        } while (s_video_decode_ip.u4_num_Bytes);
    }
};