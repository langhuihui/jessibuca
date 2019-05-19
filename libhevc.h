#pragma once
#include "ihevc_typedefs.h"
#include "iv.h"
#include "ivd.h"
#include "ihevcd_cxa.h"
#define asm(x) EM_ASM(x)
#define EXECUTE(ip, op, ...)                                      \
    if (ihevcd_cxa_api_function(codec_obj, ip, op) != IV_SUCCESS) \
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
class Libhevc : public VideoDecoder
{
public:
    iv_obj_t *codec_obj = NULL;
    CHAR ac_error_str[1000];
    Libhevc()
    {
        ihevcd_cxa_create_ip_t s_create_ip;
        ihevcd_cxa_create_op_t s_create_op;

        s_create_ip.s_ivd_create_ip_t.e_cmd = IVD_CMD_CREATE;
        s_create_ip.s_ivd_create_ip_t.u4_share_disp_buf = 1;
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
        emscripten_log(0, "H265 init");
    }
    ~Libhevc()
    {
    }

    void setNumOfCores(UWORD32 u4_num_cores)
    {
        ihevcd_cxa_ctl_set_num_cores_ip_t s_ctl_set_cores_ip = {
            sizeof(ihevcd_cxa_ctl_set_num_cores_ip_t), IVD_CMD_VIDEO_CTL, (IVD_CONTROL_API_COMMAND_TYPE_T)IHEVCD_CXA_CMD_CTL_SET_NUM_CORES, u4_num_cores};
        ihevcd_cxa_ctl_set_num_cores_op_t s_ctl_set_cores_op = {sizeof(ihevcd_cxa_ctl_set_num_cores_op_t)};
        EXECUTE(&s_ctl_set_cores_ip, &s_ctl_set_cores_op, "\nError in setting number of cores")
    }
    void decodeHeader(MemoryStream &data, int codec_id) override
    {
    }
    void _decode(const char *data, int len) override
    {
    }
};