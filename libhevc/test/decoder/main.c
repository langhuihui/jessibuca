/******************************************************************************
*
* Copyright (C) 2012 Ittiam Systems Pvt Ltd, Bangalore
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*  File Name         : main.c                                               */
/*                                                                           */
/*  Description       : Contains an application that demonstrates use of HEVC*/
/*                      decoder API                                          */
/*                                                                           */
/*  List of Functions :                                                      */
/*                                                                           */
/*  Issues / Problems : None                                                 */
/*                                                                           */
/*  Revision History  :                                                      */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   Harish          Initial Version                      */
/*****************************************************************************/
/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef X86_MINGW
#include <signal.h>
#endif

#ifndef IOS
#include <malloc.h>
#endif
#ifdef IOS_DISPLAY
#include "cast_types.h"
#else
#include "ihevc_typedefs.h"
#endif

#include "iv.h"
#include "ivd.h"
#include "ihevcd_cxa.h"
#include "ithread.h"

#ifdef WINDOWS_TIMER
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define ALIGN8(x) ((((x) + 7) >> 3) << 3)
#define NUM_DISPLAY_BUFFERS 4
#define DEFAULT_FPS         30

#define ENABLE_DEGRADE 0
#define MAX_DISP_BUFFERS    64
#define EXTRA_DISP_BUFFERS  0
#define STRLENGTH 1000

#define ADAPTIVE_TEST
#define ADAPTIVE_MAX_WD 8192
#define ADAPTIVE_MAX_HT 4096
//#define TEST_FLUSH
#define FLUSH_FRM_CNT 100


#ifdef IOS
#define PATHLENMAX 500
char filename_with_path[PATHLENMAX];
#endif

#ifdef PROFILE_ENABLE
#ifdef X86_MSVC
typedef  LARGE_INTEGER TIMER;
#else
//#ifdef X86_MINGW
typedef struct timeval TIMER;
//#endif
#endif
#else
typedef WORD32 TIMER;
#endif

#ifdef PROFILE_ENABLE
#ifdef X86_MSVC
#define GETTIME(timer) QueryPerformanceCounter(timer);
#else
//#ifdef X86_MINGW
#define GETTIME(timer) gettimeofday(timer,NULL);
//#endif
#endif

#ifdef X86_MSVC
#define ELAPSEDTIME(s_start_timer,s_end_timer, s_elapsed_time, frequency) \
                  { \
                   TIMER s_temp_time;   \
                   s_temp_time.LowPart = s_end_timer.LowPart - s_start_timer.LowPart ; \
                   s_elapsed_time = (UWORD32) ( ((DOUBLE)s_temp_time.LowPart / (DOUBLE)frequency.LowPart )  * 1000000); \
                }
#else
//#ifdef X86_MINGW
#define ELAPSEDTIME(s_start_timer,s_end_timer, s_elapsed_time, frequency) \
                   s_elapsed_time = ((s_end_timer.tv_sec - s_start_timer.tv_sec) * 1000000) + (s_end_timer.tv_usec - s_start_timer.tv_usec);
//#endif
#endif

#else
#define GETTIME(timer)
#define ELAPSEDTIME(s_start_timer,s_end_timer, s_elapsed_time, frequency)
#endif


/* Function declarations */
#ifndef MD5_DISABLE
void calc_md5_cksum(UWORD8 *pu1_inbuf, UWORD32 u4_stride, UWORD32 u4_width, UWORD32 u4_height, UWORD8 *pu1_cksum_p);
#else
#define calc_md5_cksum(a, b, c, d, e)
#endif
#ifdef SDL_DISPLAY
void* sdl_disp_init(UWORD32, UWORD32, WORD32, WORD32, WORD32, WORD32, WORD32, WORD32 *, WORD32 *);
void sdl_alloc_disp_buffers(void *);
void sdl_display(void *, WORD32);
void sdl_set_disp_buffers(void *, WORD32, UWORD8 **, UWORD8 **, UWORD8 **);
void sdl_disp_deinit(void *);
void sdl_disp_usleep(UWORD32);
IV_COLOR_FORMAT_T sdl_get_color_fmt(void);
UWORD32 sdl_get_stride(void);
#endif

#ifdef INTEL_CE5300
void* gdl_disp_init(UWORD32, UWORD32, WORD32, WORD32, WORD32, WORD32, WORD32, WORD32 *, WORD32 *);
void gdl_alloc_disp_buffers(void *);
void gdl_display(void *, WORD32);
void gdl_set_disp_buffers(void *, WORD32, UWORD8 **, UWORD8 **, UWORD8 **);
void gdl_disp_deinit(void *);
void gdl_disp_usleep(UWORD32);
IV_COLOR_FORMAT_T gdl_get_color_fmt(void);
UWORD32 gdl_get_stride(void);
#endif

#ifdef FBDEV_DISPLAY
void* fbd_disp_init(UWORD32, UWORD32, WORD32, WORD32, WORD32, WORD32, WORD32, WORD32 *, WORD32 *);
void fbd_alloc_disp_buffers(void *);
void fbd_display(void *, WORD32);
void fbd_set_disp_buffers(void *, WORD32, UWORD8 **, UWORD8 **, UWORD8 **);
void fbd_disp_deinit(void *);
void fbd_disp_usleep(UWORD32);
IV_COLOR_FORMAT_T fbd_get_color_fmt(void);
UWORD32 fbd_get_stride(void);
#endif

#ifdef IOS_DISPLAY
void* ios_disp_init(UWORD32, UWORD32, WORD32, WORD32, WORD32, WORD32, WORD32, WORD32 *, WORD32 *);
void ios_alloc_disp_buffers(void *);
void ios_display(void *, WORD32);
void ios_set_disp_buffers(void *, WORD32, UWORD8 **, UWORD8 **, UWORD8 **);
void ios_disp_deinit(void *);
void ios_disp_usleep(UWORD32);
IV_COLOR_FORMAT_T ios_get_color_fmt(void);
UWORD32 ios_get_stride(void);
#endif

typedef struct
{
    UWORD32 u4_piclen_flag;
    UWORD32 u4_file_save_flag;
    UWORD32 u4_chksum_save_flag;
    UWORD32 u4_max_frm_ts;
    IV_COLOR_FORMAT_T e_output_chroma_format;
    IVD_ARCH_T e_arch;
    IVD_SOC_T e_soc;
    UWORD32 dump_q_rd_idx;
    UWORD32 dump_q_wr_idx;
    WORD32  disp_q_wr_idx;
    WORD32  disp_q_rd_idx;

    void *cocodec_obj;
    UWORD32 share_disp_buf;
    UWORD32 num_disp_buf;
    UWORD32 b_pic_present;
    WORD32 i4_degrade_type;
    WORD32 i4_degrade_pics;
    UWORD32 u4_num_cores;
    UWORD32 disp_delay;
    WORD32 trace_enable;
    CHAR ac_trace_fname[STRLENGTH];
    CHAR ac_piclen_fname[STRLENGTH];
    CHAR ac_ip_fname[STRLENGTH];
    CHAR ac_op_fname[STRLENGTH];
    CHAR ac_op_chksum_fname[STRLENGTH];
    ivd_out_bufdesc_t s_disp_buffers[MAX_DISP_BUFFERS];
    iv_yuv_buf_t s_disp_frm_queue[MAX_DISP_BUFFERS];
    UWORD32 s_disp_frm_id_queue[MAX_DISP_BUFFERS];
    UWORD32 loopback;
    UWORD32 display;
    UWORD32 full_screen;
    UWORD32 fps;
    UWORD32 max_wd;
    UWORD32 max_ht;
    UWORD32 max_level;

    UWORD32 u4_strd;

    /* For signalling to display thread */
    UWORD32 u4_pic_wd;
    UWORD32 u4_pic_ht;

    /* For IOS diplay */
    WORD32 i4_screen_wd;
    WORD32 i4_screen_ht;

    //UWORD32 u4_output_present;
    WORD32  quit;
    WORD32  paused;


    void *pv_disp_ctx;
    void *display_thread_handle;
    WORD32 display_thread_created;
    volatile WORD32 display_init_done;
    volatile WORD32 display_deinit_flag;

    void* (*disp_init)(UWORD32, UWORD32, WORD32, WORD32, WORD32, WORD32, WORD32, WORD32 *, WORD32 *);
    void (*alloc_disp_buffers)(void *);
    void (*display_buffer)(void *, WORD32);
    void (*set_disp_buffers)(void *, WORD32, UWORD8 **, UWORD8 **, UWORD8 **);
    void (*disp_deinit)(void *);
    void (*disp_usleep)(UWORD32);
    IV_COLOR_FORMAT_T (*get_color_fmt)(void);
    UWORD32 (*get_stride)(void);
} vid_dec_ctx_t;



typedef enum
{
    INVALID,
    HELP,
    VERSION,
    INPUT_FILE,
    OUTPUT,
    CHKSUM,
    SAVE_OUTPUT,
    SAVE_CHKSUM,
    CHROMA_FORMAT,
    NUM_FRAMES,
    NUM_CORES,

    SHARE_DISPLAY_BUF,
    LOOPBACK,
    DISPLAY,
    FULLSCREEN,
    FPS,
    TRACE,
    CONFIG,

    DEGRADE_TYPE,
    DEGRADE_PICS,
    ARCH,
    SOC,
    PICLEN,
    PICLEN_FILE,
}ARGUMENT_T;

typedef struct
{
    CHAR argument_shortname[4];
    CHAR argument_name[128];
    ARGUMENT_T argument;
    CHAR description[512];
}argument_t;

static const argument_t argument_mapping[] =
{
    { "-h",  "--help",                   HELP,
        "Print this help\n" },
    { "-c", "--config",      CONFIG,
        "config file (Default: test.cfg)\n" },

    { "-v",  "--version",                VERSION,
        "Version information\n" },
    { "-i",  "--input",                  INPUT_FILE,
        "Input file\n" },
    { "-o",  "--output",                 OUTPUT,
        "Output file\n" },
    { "--",  "--piclen",                 PICLEN,
        "Flag to signal if the decoder has to use a file containing number of bytes in each picture to be fed in each call\n" },
    { "--",  "--piclen_file",                 PICLEN_FILE,
        "File containing number of bytes in each picture - each line containing one size\n" },
    { "--",  "--chksum",          CHKSUM,
        "Output MD5 Checksum file\n" },
    { "-s", "--save_output",            SAVE_OUTPUT,
        "Save Output file\n" },
    { "--", "--save_chksum",            SAVE_CHKSUM,
        "Save Check sum file\n" },
    { "--",  "--chroma_format",          CHROMA_FORMAT,
        "Output Chroma format Supported values YUV_420P, YUV_422ILE, RGB_565, YUV_420SP_UV, YUV_420SP_VU\n" },
    { "-n", "--num_frames",             NUM_FRAMES,
        "Number of frames to be decoded\n" },
    { "--", "--num_cores",              NUM_CORES,
        "Number of cores to be used\n" },
    { "--",  "--degrade_type",  DEGRADE_TYPE,
        "Degrade type : 0: No degrade 0th bit set : Disable SAO 1st bit set : Disable deblocking 2nd bit set : Faster inter prediction filters 3rd bit set : Fastest inter prediction filters\n" },
    { "--",  "--degrade_pics",  DEGRADE_PICS,
        "Degrade pics : 0 : No degrade  1 : Only on non-reference frames  2 : Do not degrade every 4th or key frames  3 : All non-key frames  4 : All frames" },
    { "--", "--share_display_buf",      SHARE_DISPLAY_BUF,
        "Enable shared display buffer mode\n" },
    { "--", "--loopback",      LOOPBACK,
        "Enable playback in a loop\n" },
    { "--", "--display",      DISPLAY,
        "Enable display (uses SDL)\n" },
    { "--", "--fullscreen",      FULLSCREEN,
        "Enable full screen (Only for GDL and SDL)\n" },
    { "--", "--fps",      FPS,
        "FPS to be used for display \n" },
    { "-i",  "--trace",                   TRACE,
        "Trace file\n" },
    { "--",  "--arch", ARCH,
        "Set Architecture. Supported values  ARM_NONEON, ARM_A9Q, ARM_A7, ARM_A5, ARM_NEONINTR, X86_GENERIC, X86_SSSE3, X86_SSE4 \n" },
    { "--",  "--soc", SOC,
        "Set SOC. Supported values  GENERIC, HISI_37X \n" },
};

#define PEAK_WINDOW_SIZE            8
#define DEFAULT_SHARE_DISPLAY_BUF   0
#define STRIDE                      0
#define DEFAULT_NUM_CORES           1

#define DUMP_SINGLE_BUF 0
#define IV_ISFATALERROR(x)         (((x) >> IVD_FATALERROR) & 0x1)

#define ivd_cxa_api_function        ihevcd_cxa_api_function

#ifdef IOS
char filename_trace[PATHLENMAX];
#endif

#if ANDROID_NDK
/*****************************************************************************/
/*                                                                           */
/*  Function Name : raise                                                    */
/*                                                                           */
/*  Description   : Needed as a workaround when the application is built in  */
/*                  Android NDK. This is an exception to be called for divide*/
/*                  by zero error                                            */
/*                                                                           */
/*  Inputs        : a                                                        */
/*  Globals       :                                                          */
/*  Processing    : None                                                     */
/*                                                                           */
/*  Outputs       :                                                          */
/*  Returns       :                                                          */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/
int raise(int a)
{
    printf("Divide by zero\n");
    return 0;
}
#endif

#ifdef _WIN32
/*****************************************************************************/
/* Function to print library calls                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*  Function Name : memalign                                                 */
/*                                                                           */
/*  Description   : Returns malloc data. Ideally should return aligned memory*/
/*                  support alignment will be added later                    */
/*                                                                           */
/*  Inputs        : alignment                                                */
/*                  size                                                     */
/*  Globals       :                                                          */
/*  Processing    :                                                          */
/*                                                                           */
/*  Outputs       :                                                          */
/*  Returns       :                                                          */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

void *ihevca_aligned_malloc(void *pv_ctxt, WORD32 alignment, WORD32 i4_size)
{
    (void)pv_ctxt;
    return (void *)_aligned_malloc(i4_size, alignment);
}

void ihevca_aligned_free(void *pv_ctxt, void *pv_buf)
{
    (void)pv_ctxt;
    _aligned_free(pv_buf);
    return;
}
#endif

#if IOS
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
#endif

#if (!defined(IOS)) && (!defined(_WIN32))
void *ihevca_aligned_malloc(void *pv_ctxt, WORD32 alignment, WORD32 i4_size)
{
   (void)pv_ctxt;
    return memalign(alignment, i4_size);
}

void ihevca_aligned_free(void *pv_ctxt, void *pv_buf)
{
    (void)pv_ctxt;
    free(pv_buf);
    return;
}
#endif
/*****************************************************************************/
/*                                                                           */
/*  Function Name : set_degrade                                 */
/*                                                                           */
/*  Description   : Control call to set degrade level       */
/*                                                                           */
/*                                                                           */
/*  Inputs        : codec_obj  - Codec Handle                                */
/*                  type - degrade level value between 0 to 4                */
/*                    0 : No degrade                                         */
/*                    1st bit : Disable SAO                                  */
/*                    2nd bit : Disable Deblock                              */
/*                    3rd bit : Faster MC for non-ref                        */
/*                    4th bit : Fastest MC for non-ref                       */
/*                  pics - Pictures that are are degraded                    */
/*                    0 : No degrade                                         */
/*                    1 : Non-ref pictures                                   */
/*                    2 : Pictures at given interval are not degraded        */
/*                    3 : All non-key pictures                               */
/*                    4 : All pictures                                       */
/*  Globals       :                                                          */
/*  Processing    : Calls degrade control to the codec                       */
/*                                                                           */
/*  Outputs       :                                                          */
/*  Returns       : Control call return status                               */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

IV_API_CALL_STATUS_T set_degrade(void *codec_obj, UWORD32 type, WORD32 pics)
{
    ihevcd_cxa_ctl_degrade_ip_t s_ctl_ip;
    ihevcd_cxa_ctl_degrade_op_t s_ctl_op;
    void *pv_api_ip, *pv_api_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_ip.u4_size = sizeof(ihevcd_cxa_ctl_degrade_ip_t);
    s_ctl_ip.i4_degrade_type = type;
    s_ctl_ip.i4_nondegrade_interval = 4;
    s_ctl_ip.i4_degrade_pics = pics;

    s_ctl_op.u4_size = sizeof(ihevcd_cxa_ctl_degrade_op_t);

    pv_api_ip = (void *)&s_ctl_ip;
    pv_api_op = (void *)&s_ctl_op;

    s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_ip.e_sub_cmd = (IVD_CONTROL_API_COMMAND_TYPE_T)IHEVCD_CXA_CMD_CTL_DEGRADE;

    e_dec_status = ivd_cxa_api_function((iv_obj_t *)codec_obj, pv_api_ip, pv_api_op);

    if(IV_SUCCESS != e_dec_status)
    {
        printf("Error in setting degrade level \n");
    }
    return (e_dec_status);

}

/*****************************************************************************/
/*                                                                           */
/*  Function Name : enable_skipb_frames                                      */
/*                                                                           */
/*  Description   : Control call to enable skipping of b frames              */
/*                                                                           */
/*                                                                           */
/*  Inputs        : codec_obj : Codec handle                                 */
/*  Globals       :                                                          */
/*  Processing    : Calls enable skip B frames control                       */
/*                                                                           */
/*  Outputs       :                                                          */
/*  Returns       : Control call return status                               */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

IV_API_CALL_STATUS_T enable_skipb_frames(void *codec_obj,
                                         vid_dec_ctx_t *ps_app_ctx)
{
    ivd_ctl_set_config_ip_t s_ctl_ip;
    ivd_ctl_set_config_op_t s_ctl_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_ip.u4_disp_wd = ps_app_ctx->u4_strd;
    s_ctl_ip.e_frm_skip_mode = IVD_SKIP_B;

    s_ctl_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
    s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

    e_dec_status = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_ip,
                                        (void *)&s_ctl_op);

    if(IV_SUCCESS != e_dec_status)
    {
        printf("Error in Enable SkipB frames \n");
    }

    return e_dec_status;
}
/*****************************************************************************/
/*                                                                           */
/*  Function Name : disable_skipb_frames                                     */
/*                                                                           */
/*  Description   : Control call to disable skipping of b frames             */
/*                                                                           */
/*                                                                           */
/*  Inputs        : codec_obj : Codec handle                                 */
/*  Globals       :                                                          */
/*  Processing    : Calls disable B frame skip control                       */
/*                                                                           */
/*  Outputs       :                                                          */
/*  Returns       : Control call return status                               */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

IV_API_CALL_STATUS_T disable_skipb_frames(void *codec_obj,
                                          vid_dec_ctx_t *ps_app_ctx)
{
    ivd_ctl_set_config_ip_t s_ctl_ip;
    ivd_ctl_set_config_op_t s_ctl_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_ip.u4_disp_wd = ps_app_ctx->u4_strd;
    s_ctl_ip.e_frm_skip_mode = IVD_SKIP_NONE;

    s_ctl_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
    s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

    e_dec_status = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_ip,
                                        (void *)&s_ctl_op);

    if(IV_SUCCESS != e_dec_status)
    {
        printf("Error in Disable SkipB frames\n");
    }

    return e_dec_status;
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name : enable_skippb_frames                                     */
/*                                                                           */
/*  Description   : Control call to enable skipping of P & B frames          */
/*                                                                           */
/*                                                                           */
/*  Inputs        : codec_obj : Codec handle                                 */
/*  Globals       :                                                          */
/*  Processing    : Calls enable skip P and B frames control                 */
/*                                                                           */
/*  Outputs       :                                                          */
/*  Returns       : Control call return status                               */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

IV_API_CALL_STATUS_T enable_skippb_frames(void *codec_obj,
                                          vid_dec_ctx_t *ps_app_ctx)
{
    ivd_ctl_set_config_ip_t s_ctl_ip;
    ivd_ctl_set_config_op_t s_ctl_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_ip.u4_disp_wd = ps_app_ctx->u4_strd;
    s_ctl_ip.e_frm_skip_mode = IVD_SKIP_PB;

    s_ctl_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
    s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

    e_dec_status = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_ip,
                                        (void *)&s_ctl_op);
    if(IV_SUCCESS != e_dec_status)
    {
        printf("Error in Enable SkipPB frames\n");
    }

    return e_dec_status;
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name : disable_skippb_frames                                    */
/*                                                                           */
/*  Description   : Control call to disable skipping of P and B frames       */
/*                                                                           */
/*                                                                           */
/*  Inputs        : codec_obj : Codec handle                                 */
/*  Globals       :                                                          */
/*  Processing    : Calls disable P and B frame skip control                 */
/*                                                                           */
/*  Outputs       :                                                          */
/*  Returns       : Control call return status                               */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

IV_API_CALL_STATUS_T disable_skippb_frames(void *codec_obj,
                                           vid_dec_ctx_t *ps_app_ctx)
{
    ivd_ctl_set_config_ip_t s_ctl_ip;
    ivd_ctl_set_config_op_t s_ctl_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_ctl_ip.u4_disp_wd = ps_app_ctx->u4_strd;
    s_ctl_ip.e_frm_skip_mode = IVD_SKIP_NONE;

    s_ctl_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
    s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

    e_dec_status = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_ip,
                                        (void *)&s_ctl_op);
    if(IV_SUCCESS != e_dec_status)
    {
        printf("Error in Disable SkipPB frames\n");
    }

    return e_dec_status;
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name : release_disp_frame                                       */
/*                                                                           */
/*  Description   : Calls release display control - Used to signal to the    */
/*                  decoder that this particular buffer has been displayed   */
/*                  and that the codec is now free to write to this buffer   */
/*                                                                           */
/*                                                                           */
/*  Inputs        : codec_obj : Codec Handle                                 */
/*                  buf_id    : Buffer Id of the buffer to be released       */
/*                              This id would have been returned earlier by  */
/*                              the codec                                    */
/*  Globals       :                                                          */
/*  Processing    : Calls Release Display call                               */
/*                                                                           */
/*  Outputs       :                                                          */
/*  Returns       : Status of release display call                           */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

IV_API_CALL_STATUS_T release_disp_frame(void *codec_obj, UWORD32 buf_id)
{
    ivd_rel_display_frame_ip_t s_video_rel_disp_ip;
    ivd_rel_display_frame_op_t s_video_rel_disp_op;
    IV_API_CALL_STATUS_T e_dec_status;

    s_video_rel_disp_ip.e_cmd = IVD_CMD_REL_DISPLAY_FRAME;
    s_video_rel_disp_ip.u4_size = sizeof(ivd_rel_display_frame_ip_t);
    s_video_rel_disp_op.u4_size = sizeof(ivd_rel_display_frame_op_t);
    s_video_rel_disp_ip.u4_disp_buf_id = buf_id;

    e_dec_status = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_video_rel_disp_ip,
                                        (void *)&s_video_rel_disp_op);
    if(IV_SUCCESS != e_dec_status)
    {
        printf("Error in Release Disp frame\n");
    }


    return (e_dec_status);
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name : get_version                                      */
/*                                                                           */
/*  Description   : Control call to get codec version              */
/*                                                                           */
/*                                                                           */
/*  Inputs        : codec_obj : Codec handle                                 */
/*  Globals       :                                                          */
/*  Processing    : Calls enable skip B frames control                       */
/*                                                                           */
/*  Outputs       :                                                          */
/*  Returns       : Control call return status                               */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

IV_API_CALL_STATUS_T get_version(void *codec_obj)
{
    ivd_ctl_getversioninfo_ip_t s_ctl_dec_ip;
    ivd_ctl_getversioninfo_op_t s_ctl_dec_op;
    UWORD8 au1_buf[512];
    IV_API_CALL_STATUS_T status;
    s_ctl_dec_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_dec_ip.e_sub_cmd = IVD_CMD_CTL_GETVERSION;
    s_ctl_dec_ip.u4_size = sizeof(ivd_ctl_getversioninfo_ip_t);
    s_ctl_dec_op.u4_size = sizeof(ivd_ctl_getversioninfo_op_t);
    s_ctl_dec_ip.pv_version_buffer = au1_buf;
    s_ctl_dec_ip.u4_version_buffer_size = sizeof(au1_buf);

    status = ivd_cxa_api_function((iv_obj_t *)codec_obj,
                                  (void *)&(s_ctl_dec_ip),
                                  (void *)&(s_ctl_dec_op));

    if(status != IV_SUCCESS)
    {
        printf("Error in Getting Version number e_dec_status = %d u4_error_code = %x\n",
               status, s_ctl_dec_op.u4_error_code);
    }
    else
    {
        printf("Ittiam Decoder Version number: %s\n",
               (char *)s_ctl_dec_ip.pv_version_buffer);
    }
    return status;
}
/*****************************************************************************/
/*                                                                           */
/*  Function Name : codec_exit                                               */
/*                                                                           */
/*  Description   : handles unrecoverable errors                             */
/*  Inputs        : Error message                                            */
/*  Globals       : None                                                     */
/*  Processing    : Prints error message to console and exits.               */
/*  Outputs       : Error mesage to the console                              */
/*  Returns       : None                                                     */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes (Describe the changes made)  */
/*         07 06 2006   Sankar          Creation                             */
/*                                                                           */
/*****************************************************************************/
void codec_exit(CHAR *pc_err_message)
{
    printf("%s\n", pc_err_message);
    exit(-1);
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name : dump_output                                              */
/*                                                                           */
/*  Description   : Used to dump output YUV                                  */
/*  Inputs        : App context, disp output desc, File pointer              */
/*  Globals       : None                                                     */
/*  Processing    : Dumps to a file                                          */
/*  Returns       : None                                                     */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes (Describe the changes made)  */
/*         07 06 2006   Sankar          Creation                             */
/*                                                                           */
/*****************************************************************************/
void dump_output(vid_dec_ctx_t *ps_app_ctx,
                 iv_yuv_buf_t *ps_disp_frm_buf,
                 UWORD32 u4_disp_frm_id,
                 FILE *ps_op_file,
                 FILE *ps_op_chksum_file,
                 WORD32 i4_op_frm_ts,
                 UWORD32 file_save,
                 UWORD32 chksum_save)

{

    UWORD32 i;
    iv_yuv_buf_t s_dump_disp_frm_buf;
    UWORD32 u4_disp_id;

    memset(&s_dump_disp_frm_buf, 0, sizeof(iv_yuv_buf_t));

    if(ps_app_ctx->share_disp_buf)
    {
        if(ps_app_ctx->dump_q_wr_idx == MAX_DISP_BUFFERS
                        )
            ps_app_ctx->dump_q_wr_idx = 0;

        if(ps_app_ctx->dump_q_rd_idx == MAX_DISP_BUFFERS
                        )
            ps_app_ctx->dump_q_rd_idx = 0;

        ps_app_ctx->s_disp_frm_queue[ps_app_ctx->dump_q_wr_idx] =
                        *ps_disp_frm_buf;
        ps_app_ctx->s_disp_frm_id_queue[ps_app_ctx->dump_q_wr_idx] =
                        u4_disp_frm_id;
        ps_app_ctx->dump_q_wr_idx++;

        if((WORD32)i4_op_frm_ts >= (WORD32)(ps_app_ctx->disp_delay - 1))
        {
            s_dump_disp_frm_buf =
                            ps_app_ctx->s_disp_frm_queue[ps_app_ctx->dump_q_rd_idx];
            u4_disp_id =
                            ps_app_ctx->s_disp_frm_id_queue[ps_app_ctx->dump_q_rd_idx];
            ps_app_ctx->dump_q_rd_idx++;
        }
        else
        {
            return;
        }
    }
    else
    {
        s_dump_disp_frm_buf = *ps_disp_frm_buf;
        u4_disp_id = u4_disp_frm_id;
    }

    release_disp_frame(ps_app_ctx->cocodec_obj, u4_disp_id);

    if(0 == file_save && 0 == chksum_save)
        return;

    if(NULL == s_dump_disp_frm_buf.pv_y_buf)
        return;

    if(ps_app_ctx->e_output_chroma_format == IV_YUV_420P)
    {
#if DUMP_SINGLE_BUF
        {
            UWORD8 *buf = s_dump_disp_frm_buf.pv_y_buf - 80 - (s_dump_disp_frm_buf.u4_y_strd * 80);

            UWORD32 size = s_dump_disp_frm_buf.u4_y_strd * ((s_dump_disp_frm_buf.u4_y_ht + 160) + (s_dump_disp_frm_buf.u4_u_ht + 80));
            fwrite(buf, 1, size ,ps_op_file);

        }
#else
        if(0 != file_save)
        {
            UWORD8 *buf;

            buf = (UWORD8 *)s_dump_disp_frm_buf.pv_y_buf;
            for(i = 0; i < s_dump_disp_frm_buf.u4_y_ht; i++)
            {
                fwrite(buf, 1, s_dump_disp_frm_buf.u4_y_wd, ps_op_file);
                buf += s_dump_disp_frm_buf.u4_y_strd;
            }

            buf = (UWORD8 *)s_dump_disp_frm_buf.pv_u_buf;
            for(i = 0; i < s_dump_disp_frm_buf.u4_u_ht; i++)
            {
                fwrite(buf, 1, s_dump_disp_frm_buf.u4_u_wd, ps_op_file);
                buf += s_dump_disp_frm_buf.u4_u_strd;
            }
            buf = (UWORD8 *)s_dump_disp_frm_buf.pv_v_buf;
            for(i = 0; i < s_dump_disp_frm_buf.u4_v_ht; i++)
            {
                fwrite(buf, 1, s_dump_disp_frm_buf.u4_v_wd, ps_op_file);
                buf += s_dump_disp_frm_buf.u4_v_strd;
            }

        }

        if(0 != chksum_save)
        {
            UWORD8 au1_y_chksum[16];
            UWORD8 au1_u_chksum[16];
            UWORD8 au1_v_chksum[16];
            calc_md5_cksum((UWORD8 *)s_dump_disp_frm_buf.pv_y_buf,
                           s_dump_disp_frm_buf.u4_y_strd,
                           s_dump_disp_frm_buf.u4_y_wd,
                           s_dump_disp_frm_buf.u4_y_ht,
                           au1_y_chksum);
            calc_md5_cksum((UWORD8 *)s_dump_disp_frm_buf.pv_u_buf,
                           s_dump_disp_frm_buf.u4_u_strd,
                           s_dump_disp_frm_buf.u4_u_wd,
                           s_dump_disp_frm_buf.u4_u_ht,
                           au1_u_chksum);
            calc_md5_cksum((UWORD8 *)s_dump_disp_frm_buf.pv_v_buf,
                           s_dump_disp_frm_buf.u4_v_strd,
                           s_dump_disp_frm_buf.u4_v_wd,
                           s_dump_disp_frm_buf.u4_v_ht,
                           au1_v_chksum);

            fwrite(au1_y_chksum, sizeof(UWORD8), 16, ps_op_chksum_file);
            fwrite(au1_u_chksum, sizeof(UWORD8), 16, ps_op_chksum_file);
            fwrite(au1_v_chksum, sizeof(UWORD8), 16, ps_op_chksum_file);
        }
#endif
    }
    else if((ps_app_ctx->e_output_chroma_format == IV_YUV_420SP_UV)
                    || (ps_app_ctx->e_output_chroma_format == IV_YUV_420SP_VU))
    {
#if DUMP_SINGLE_BUF
        {

            UWORD8 *buf = s_dump_disp_frm_buf.pv_y_buf - 24 - (s_dump_disp_frm_buf.u4_y_strd * 40);

            UWORD32 size = s_dump_disp_frm_buf.u4_y_strd * ((s_dump_disp_frm_buf.u4_y_ht + 80) + (s_dump_disp_frm_buf.u4_u_ht + 40));
            fwrite(buf, 1, size ,ps_op_file);
        }
#else
        {
            UWORD8 *buf;

            buf = (UWORD8 *)s_dump_disp_frm_buf.pv_y_buf;
            for(i = 0; i < s_dump_disp_frm_buf.u4_y_ht; i++)
            {
                fwrite(buf, 1, s_dump_disp_frm_buf.u4_y_wd, ps_op_file);
                buf += s_dump_disp_frm_buf.u4_y_strd;
            }

            buf = (UWORD8 *)s_dump_disp_frm_buf.pv_u_buf;
            for(i = 0; i < s_dump_disp_frm_buf.u4_u_ht; i++)
            {
                fwrite(buf, 1, s_dump_disp_frm_buf.u4_u_wd, ps_op_file);
                buf += s_dump_disp_frm_buf.u4_u_strd;
            }
        }
#endif
    }
    else if(ps_app_ctx->e_output_chroma_format == IV_RGBA_8888)
    {
        UWORD8 *buf;

        buf = (UWORD8 *)s_dump_disp_frm_buf.pv_y_buf;
        for(i = 0; i < s_dump_disp_frm_buf.u4_y_ht; i++)
        {
            fwrite(buf, 1, s_dump_disp_frm_buf.u4_y_wd * 4, ps_op_file);
            buf += s_dump_disp_frm_buf.u4_y_strd * 4;
        }
    }
    else
    {
        UWORD8 *buf;

        buf = (UWORD8 *)s_dump_disp_frm_buf.pv_y_buf;
        for(i = 0; i < s_dump_disp_frm_buf.u4_y_ht; i++)
        {
            fwrite(buf, 1, s_dump_disp_frm_buf.u4_y_strd * 2, ps_op_file);
            buf += s_dump_disp_frm_buf.u4_y_strd * 2;
        }
    }

    fflush(ps_op_file);
    fflush(ps_op_chksum_file);

}


/*****************************************************************************/
/*                                                                           */
/*  Function Name : print_usage                                              */
/*                                                                           */
/*  Description   : Prints argument format                                   */
/*                                                                           */
/*                                                                           */
/*  Inputs        :                                                          */
/*  Globals       :                                                          */
/*  Processing    : Prints argument format                                   */
/*                                                                           */
/*  Outputs       :                                                          */
/*  Returns       :                                                          */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

void print_usage(void)
{
    WORD32 i = 0;
    WORD32 num_entries = sizeof(argument_mapping) / sizeof(argument_t);
    printf("\nUsage:\n");
    while(i < num_entries)
    {
        printf("%-32s\t %s", argument_mapping[i].argument_name,
               argument_mapping[i].description);
        i++;
    }
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name : get_argument                                             */
/*                                                                           */
/*  Description   : Gets argument for a given string                         */
/*                                                                           */
/*                                                                           */
/*  Inputs        : name                                                     */
/*  Globals       :                                                          */
/*  Processing    : Searches the given string in the array and returns       */
/*                  appropriate argument ID                                  */
/*                                                                           */
/*  Outputs       : Argument ID                                              */
/*  Returns       : Argument ID                                              */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

ARGUMENT_T get_argument(CHAR *name)
{
    WORD32 i = 0;
    WORD32 num_entries = sizeof(argument_mapping) / sizeof(argument_t);
    while(i < num_entries)
    {
        if((0 == strcmp(argument_mapping[i].argument_name, name)) ||
                        ((0 == strcmp(argument_mapping[i].argument_shortname, name)) &&
                                        (0 != strcmp(argument_mapping[i].argument_shortname, "--"))))
        {
            return argument_mapping[i].argument;
        }
        i++;
    }
    return INVALID;
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name : get_argument                                             */
/*                                                                           */
/*  Description   : Gets argument for a given string                         */
/*                                                                           */
/*                                                                           */
/*  Inputs        : name                                                     */
/*  Globals       :                                                          */
/*  Processing    : Searches the given string in the array and returns       */
/*                  appropriate argument ID                                  */
/*                                                                           */
/*  Outputs       : Argument ID                                              */
/*  Returns       : Argument ID                                              */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

void parse_argument(vid_dec_ctx_t *ps_app_ctx, CHAR *argument, CHAR *value)
{
    ARGUMENT_T arg;

    arg = get_argument(argument);
    switch(arg)
    {
        case HELP:
            print_usage();
            exit(-1);
        case VERSION:
            break;
        case INPUT_FILE:
            sscanf(value, "%s", ps_app_ctx->ac_ip_fname);
            //input_passed = 1;
            break;

        case OUTPUT:
            sscanf(value, "%s", ps_app_ctx->ac_op_fname);
            break;

        case CHKSUM:
            sscanf(value, "%s", ps_app_ctx->ac_op_chksum_fname);
            break;

        case SAVE_OUTPUT:
            sscanf(value, "%d", &ps_app_ctx->u4_file_save_flag);
            break;

        case SAVE_CHKSUM:
            sscanf(value, "%d", &ps_app_ctx->u4_chksum_save_flag);
            break;

        case CHROMA_FORMAT:
            if((strcmp(value, "YUV_420P")) == 0)
                ps_app_ctx->e_output_chroma_format = IV_YUV_420P;
            else if((strcmp(value, "YUV_422ILE")) == 0)
                ps_app_ctx->e_output_chroma_format = IV_YUV_422ILE;
            else if((strcmp(value, "RGB_565")) == 0)
                ps_app_ctx->e_output_chroma_format = IV_RGB_565;
            else if((strcmp(value, "RGBA_8888")) == 0)
                ps_app_ctx->e_output_chroma_format = IV_RGBA_8888;
            else if((strcmp(value, "YUV_420SP_UV")) == 0)
                ps_app_ctx->e_output_chroma_format = IV_YUV_420SP_UV;
            else if((strcmp(value, "YUV_420SP_VU")) == 0)
                ps_app_ctx->e_output_chroma_format = IV_YUV_420SP_VU;
            else
            {
                printf("\nInvalid colour format setting it to IV_YUV_420P\n");
                ps_app_ctx->e_output_chroma_format = IV_YUV_420P;
            }

            break;
        case NUM_FRAMES:
            sscanf(value, "%d", &ps_app_ctx->u4_max_frm_ts);
            break;

        case NUM_CORES:
            sscanf(value, "%d", &ps_app_ctx->u4_num_cores);
            break;
        case DEGRADE_PICS:
            sscanf(value, "%d", &ps_app_ctx->i4_degrade_pics);
            break;
        case DEGRADE_TYPE:
            sscanf(value, "%d", &ps_app_ctx->i4_degrade_type);
            break;
        case SHARE_DISPLAY_BUF:
            sscanf(value, "%d", &ps_app_ctx->share_disp_buf);
            break;
        case LOOPBACK:
            sscanf(value, "%d", &ps_app_ctx->loopback);
            break;
        case DISPLAY:
#if defined(SDL_DISPLAY) || defined(FBDEV_DISPLAY) || defined(INTEL_CE5300) || defined(IOS_DISPLAY)
            sscanf(value, "%d", &ps_app_ctx->display);
#else
            ps_app_ctx->display = 0;
#endif
            break;
        case FULLSCREEN:
            sscanf(value, "%d", &ps_app_ctx->full_screen);
            break;
        case FPS:
            sscanf(value, "%d", &ps_app_ctx->fps);
            if(ps_app_ctx->fps <= 0)
                ps_app_ctx->fps = DEFAULT_FPS;
            break;
        case ARCH:
            if((strcmp(value, "ARM_NONEON")) == 0)
                ps_app_ctx->e_arch = ARCH_ARM_NONEON;
            else if((strcmp(value, "ARM_A9Q")) == 0)
                ps_app_ctx->e_arch = ARCH_ARM_A9Q;
            else if((strcmp(value, "ARM_A7")) == 0)
                ps_app_ctx->e_arch = ARCH_ARM_A7;
            else if((strcmp(value, "ARM_A5")) == 0)
                ps_app_ctx->e_arch = ARCH_ARM_A5;
            else if((strcmp(value, "ARM_NEONINTR")) == 0)
                ps_app_ctx->e_arch = ARCH_ARM_NEONINTR;
            else if((strcmp(value, "X86_GENERIC")) == 0)
                ps_app_ctx->e_arch = ARCH_X86_GENERIC;
            else if((strcmp(value, "X86_SSSE3")) == 0)
                ps_app_ctx->e_arch = ARCH_X86_SSSE3;
            else if((strcmp(value, "X86_SSE42")) == 0)
                ps_app_ctx->e_arch = ARCH_X86_SSE42;
            else if((strcmp(value, "X86_AVX2")) == 0)
                ps_app_ctx->e_arch = ARCH_X86_AVX2;
            else if((strcmp(value, "MIPS_GENERIC")) == 0)
                ps_app_ctx->e_arch = ARCH_MIPS_GENERIC;
            else if((strcmp(value, "MIPS_32")) == 0)
                ps_app_ctx->e_arch = ARCH_MIPS_32;
            else if((strcmp(value, "ARMV8_GENERIC")) == 0)
                ps_app_ctx->e_arch = ARCH_ARMV8_GENERIC;
            else
            {
                printf("\nInvalid Arch. Setting it to ARM_A9Q\n");
                ps_app_ctx->e_arch = ARCH_ARM_A9Q;
            }

            break;
        case SOC:
            if((strcmp(value, "GENERIC")) == 0)
                ps_app_ctx->e_soc = SOC_GENERIC;
            else if((strcmp(value, "HISI_37X")) == 0)
                ps_app_ctx->e_soc = SOC_HISI_37X;
            else
            {
                ps_app_ctx->e_soc = atoi(value);
/*
                printf("\nInvalid SOC. Setting it to GENERIC\n");
                ps_app_ctx->e_soc = SOC_GENERIC;
*/
            }
            break;
        case PICLEN:
            sscanf(value, "%d", &ps_app_ctx->u4_piclen_flag);
            break;

        case PICLEN_FILE:
            sscanf(value, "%s", ps_app_ctx->ac_piclen_fname);
            break;

        case INVALID:
        default:
            printf("Ignoring argument :  %s\n", argument);
            break;
    }
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name : read_cfg_file                                            */
/*                                                                           */
/*  Description   : Reads arguments from a configuration file                */
/*                                                                           */
/*                                                                           */
/*  Inputs        : ps_app_ctx  : Application context                        */
/*                  fp_cfg_file : Configuration file handle                  */
/*  Globals       :                                                          */
/*  Processing    : Parses the arguments and fills in the application context*/
/*                                                                           */
/*  Outputs       : Arguments parsed                                         */
/*  Returns       : None                                                     */
/*                                                                           */
/*  Issues        :                                                          */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

void read_cfg_file(vid_dec_ctx_t *ps_app_ctx, FILE *fp_cfg_file)
{

    CHAR line[STRLENGTH];
    CHAR description[STRLENGTH];
    CHAR value[STRLENGTH];
    CHAR argument[STRLENGTH];
    void *ret;
    while(0 == feof(fp_cfg_file))
    {
        line[0] = '\0';
        ret = fgets(line, STRLENGTH, fp_cfg_file);
        if(NULL == ret)
            break;
        argument[0] = '\0';
        /* Reading Input File Name */
        sscanf(line, "%s %s %s", argument, value, description);
        if(argument[0] == '\0')
            continue;

        parse_argument(ps_app_ctx, argument, value);
    }


}

/*!
**************************************************************************
* \if Function name : dispq_producer_dequeue \endif
*
* \brief
*    This function gets a free buffer index where display data can be written
*    This is a blocking call and can be exited by setting quit to true in
*    the application context
*
* \param[in]  ps_app_ctx  : Pointer to application context
*
* \return
*    returns Next free buffer index for producer
*
* \author
*  Ittiam
*
**************************************************************************
*/
WORD32 dispq_producer_dequeue(vid_dec_ctx_t *ps_app_ctx)
{
    WORD32 idx;

    /* If there is no free buffer wait */

    while(((ps_app_ctx->disp_q_wr_idx + 1) % NUM_DISPLAY_BUFFERS) == ps_app_ctx->disp_q_rd_idx)
    {

        ithread_msleep(1);

        if(ps_app_ctx->quit)
            return (-1);
    }

    idx = ps_app_ctx->disp_q_wr_idx;
    return (idx);
}

/*!
**************************************************************************
* \if Function name : dispq_producer_queue \endif
*
* \brief
*    This function adds buffer which can be displayed
*
* \param[in]  ps_app_ctx  : Pointer to application context
*
* \return
*    returns Next free buffer index for producer
*
* \author
*  Ittiam
*
**************************************************************************
*/
WORD32 dispq_producer_queue(vid_dec_ctx_t *ps_app_ctx)
{
    ps_app_ctx->disp_q_wr_idx++;
    if(ps_app_ctx->disp_q_wr_idx == NUM_DISPLAY_BUFFERS)
        ps_app_ctx->disp_q_wr_idx = 0;

    return (0);
}
/*!
**************************************************************************
* \if Function name : dispq_consumer_dequeue \endif
*
* \brief
*    This function gets a free buffer index where display data can be written
*    This is a blocking call and can be exited by setting quit to true in
*    the application context
*
* \param[in]  ps_app_ctx  : Pointer to application context
*
* \return
*    returns Next free buffer index for producer
*
* \author
*  Ittiam
*
**************************************************************************
*/
WORD32 dispq_consumer_dequeue(vid_dec_ctx_t *ps_app_ctx)
{
    WORD32 idx;

    /* If there is no free buffer wait */

    while(ps_app_ctx->disp_q_wr_idx == ps_app_ctx->disp_q_rd_idx)
    {

        ithread_msleep(1);

        if(ps_app_ctx->quit)
            return (-1);
    }

    idx = ps_app_ctx->disp_q_rd_idx;
    return (idx);
}

/*!
**************************************************************************
* \if Function name : dispq_producer_queue \endif
*
* \brief
*    This function adds buffer which can be displayed
*
* \param[in]  ps_app_ctx  : Pointer to application context
*
* \return
*    returns Next free buffer index for producer
*
* \author
*  Ittiam
*
**************************************************************************
*/
WORD32 dispq_consumer_queue(vid_dec_ctx_t *ps_app_ctx)
{
    ps_app_ctx->disp_q_rd_idx++;
    if(ps_app_ctx->disp_q_rd_idx == NUM_DISPLAY_BUFFERS)
        ps_app_ctx->disp_q_rd_idx = 0;

    return (0);
}

/*****************************************************************************/
/*                                                                           */
/*  Function Name : display_thread                                           */
/*                                                                           */
/*  Description   : Thread to display the frame                              */
/*                                                                           */
/*                                                                           */
/*  Inputs        : pv_ctx  : Application context                            */
/*                                                                           */
/*  Globals       :                                                          */
/*  Processing    : Wait for a buffer to get produced by decoder and display */
/*                  that frame                                               */
/*                                                                           */
/*  Outputs       :                                                          */
/*  Returns       : None                                                     */
/*                                                                           */
/*  Issues        : Pause followed by quit is making some deadlock condn     */
/*                  If decoder was lagging initially and then fasten up,     */
/*                  display will also go at faster rate till it reaches      */
/*                  equilibrium wrt the initial time                         */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 05 2013   100578          Initial Version                      */
/*                                                                           */
/*****************************************************************************/

WORD32 display_thread(void *pv_ctx)
{
    vid_dec_ctx_t *ps_app_ctx = (vid_dec_ctx_t *)pv_ctx;


    UWORD32 frm_duration; /* in us */
    UWORD32 current_time;
    UWORD32 expected_time;
    TIMER   s_end_timer;
    TIMER   s_first_frame_time;
    UWORD32 first_frame_displayed;

#ifdef WINDOWS_TIMER
    TIMER frequency;
#endif

#ifdef WINDOWS_TIMER
    QueryPerformanceFrequency(&frequency);
#endif
    first_frame_displayed = 0;
    expected_time = 0;
    frm_duration = 1000000 / ps_app_ctx->fps;

    /* Init display and allocate display buffers */
    ps_app_ctx->pv_disp_ctx = (void *)ps_app_ctx->disp_init(ps_app_ctx->u4_pic_wd,
                                                            ps_app_ctx->u4_pic_ht,
                                                            ps_app_ctx->i4_screen_wd,
                                                            ps_app_ctx->i4_screen_ht,
                                                            ps_app_ctx->max_wd,
                                                            ps_app_ctx->max_ht,
                                                            ps_app_ctx->full_screen,
                                                            &ps_app_ctx->quit,
                                                            &ps_app_ctx->paused);
    ps_app_ctx->alloc_disp_buffers(ps_app_ctx->pv_disp_ctx);

    ps_app_ctx->display_init_done = 1;

    while(1)
    {
        WORD32 rd_idx;

        rd_idx = dispq_consumer_dequeue(ps_app_ctx);
        if(ps_app_ctx->quit)
            break;

        ps_app_ctx->display_buffer(ps_app_ctx->pv_disp_ctx, rd_idx);

        if(0 == first_frame_displayed)
        {
            GETTIME(&s_first_frame_time);
            first_frame_displayed = 1;
        }

        /*********************************************************************/
        /* Sleep based on the expected time of arrival of current buffer and */
        /* the Current frame                                                 */
        /*********************************************************************/

        GETTIME(&s_end_timer);
        ELAPSEDTIME(s_first_frame_time, s_end_timer, current_time, frequency);

        /* time in micro second */
        expected_time += frm_duration;

        //printf("current_time %d expected_time %d diff %d \n", current_time, expected_time, (expected_time - current_time));
        /* sleep for the diff. in time */
        if(current_time < expected_time)
            ps_app_ctx->disp_usleep((expected_time - current_time));
        else
            expected_time += (current_time - expected_time);

        dispq_consumer_queue(ps_app_ctx);

    }


    while(0 == ps_app_ctx->display_deinit_flag)
    {
        ps_app_ctx->disp_usleep(1000);
    }
    ps_app_ctx->disp_deinit(ps_app_ctx->pv_disp_ctx);

    /* destroy the display thread */
    ithread_exit(ps_app_ctx->display_thread_handle);

    return 0;
}

void flush_output(iv_obj_t *codec_obj,
                  vid_dec_ctx_t *ps_app_ctx,
                  ivd_out_bufdesc_t *ps_out_buf,
                  UWORD8 *pu1_bs_buf,
                  UWORD32 *pu4_op_frm_ts,
                  FILE *ps_op_file,
                  FILE *ps_op_chksum_file,
                  UWORD32 u4_ip_frm_ts,
                  UWORD32 u4_bytes_remaining)
{
    WORD32 ret;

    do
    {

        ivd_ctl_flush_ip_t s_ctl_ip;
        ivd_ctl_flush_op_t s_ctl_op;

        if(*pu4_op_frm_ts >= (ps_app_ctx->u4_max_frm_ts + ps_app_ctx->disp_delay))
            break;

        s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_FLUSH;
        s_ctl_ip.u4_size = sizeof(ivd_ctl_flush_ip_t);
        s_ctl_op.u4_size = sizeof(ivd_ctl_flush_op_t);
        ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_ip,
                                   (void *)&s_ctl_op);

        if(ret != IV_SUCCESS)
        {
            printf("Error in Setting the decoder in flush mode\n");
        }

        if(IV_SUCCESS == ret)
        {
            ivd_video_decode_ip_t s_video_decode_ip;
            ivd_video_decode_op_t s_video_decode_op;

            s_video_decode_ip.e_cmd = IVD_CMD_VIDEO_DECODE;
            s_video_decode_ip.u4_ts = u4_ip_frm_ts;
            s_video_decode_ip.pv_stream_buffer = pu1_bs_buf;
            s_video_decode_ip.u4_num_Bytes = u4_bytes_remaining;
            s_video_decode_ip.u4_size = sizeof(ivd_video_decode_ip_t);
            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[0] =
                            ps_out_buf->u4_min_out_buf_size[0];
            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[1] =
                            ps_out_buf->u4_min_out_buf_size[1];
            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[2] =
                            ps_out_buf->u4_min_out_buf_size[2];

            s_video_decode_ip.s_out_buffer.pu1_bufs[0] =
                            ps_out_buf->pu1_bufs[0];
            s_video_decode_ip.s_out_buffer.pu1_bufs[1] =
                            ps_out_buf->pu1_bufs[1];
            s_video_decode_ip.s_out_buffer.pu1_bufs[2] =
                            ps_out_buf->pu1_bufs[2];
            s_video_decode_ip.s_out_buffer.u4_num_bufs =
                            ps_out_buf->u4_num_bufs;

            s_video_decode_op.u4_size = sizeof(ivd_video_decode_op_t);

            /*****************************************************************************/
            /*   API Call: Video Decode                                                  */
            /*****************************************************************************/
            ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_video_decode_ip,
                                       (void *)&s_video_decode_op);

            if(1 == s_video_decode_op.u4_output_present)
            {
                dump_output(ps_app_ctx, &(s_video_decode_op.s_disp_frm_buf),
                            s_video_decode_op.u4_disp_buf_id, ps_op_file,
                            ps_op_chksum_file,
                            *pu4_op_frm_ts, ps_app_ctx->u4_file_save_flag,
                            ps_app_ctx->u4_chksum_save_flag);

                (*pu4_op_frm_ts)++;
            }
        }
    }while(IV_SUCCESS == ret);

}

#ifdef X86_MINGW
void sigsegv_handler()
{
    printf("Segmentation fault, Exiting.. \n");
    exit(-1);
}
#endif

UWORD32 default_get_stride(void)
{
    return 0;
}


IV_COLOR_FORMAT_T default_get_color_fmt(void)
{
    return IV_YUV_420P;
}
/*****************************************************************************/
/*                                                                           */
/*  Function Name : main                                                     */
/*                                                                           */
/*  Description   : Application to demonstrate codec API                     */
/*                                                                           */
/*                                                                           */
/*  Inputs        : argc    - Number of arguments                            */
/*                  argv[]  - Arguments                                      */
/*  Globals       :                                                          */
/*  Processing    : Shows how to use create, process, control and delete     */
/*                                                                           */
/*  Outputs       : Codec output in a file                                   */
/*  Returns       :                                                          */
/*                                                                           */
/*  Issues        : Assumes both PROFILE_ENABLE to be                        */
/*                  defined for multithread decode-display working           */
/*                                                                           */
/*  Revision History:                                                        */
/*                                                                           */
/*         DD MM YYYY   Author(s)       Changes                              */
/*         07 09 2012   100189          Initial Version                      */
/*         09 05 2013   100578          Multithread decode-display           */
/*****************************************************************************/
#ifdef IOS
int hevcdec_main(char *homedir, char *documentdir, int screen_wd, int screen_ht)
#else
int main(WORD32 argc, CHAR *argv[])
#endif
{
    CHAR ac_cfg_fname[STRLENGTH];
    FILE *fp_cfg_file = NULL;
    FILE *ps_piclen_file = NULL;
    FILE *ps_ip_file = NULL;
    FILE *ps_op_file = NULL;
    FILE *ps_op_chksum_file = NULL;
    WORD32 ret;
    CHAR ac_error_str[STRLENGTH];
    vid_dec_ctx_t s_app_ctx;
    UWORD8 *pu1_bs_buf = NULL;

    ivd_out_bufdesc_t *ps_out_buf;
    UWORD32 u4_num_bytes_dec = 0;
    UWORD32 file_pos = 0;

    UWORD32 u4_ip_frm_ts = 0, u4_op_frm_ts = 0;

    WORD32 u4_bytes_remaining = 0;
    UWORD32 i;
    UWORD32 u4_ip_buf_len;
    UWORD32 frm_cnt = 0;
    WORD32 total_bytes_comsumed;

#ifdef PROFILE_ENABLE
    UWORD32 u4_tot_cycles = 0;
    UWORD32 u4_tot_fmt_cycles = 0;
    UWORD32 peak_window[PEAK_WINDOW_SIZE];
    UWORD32 peak_window_idx = 0;
    UWORD32 peak_avg_max = 0;
#ifdef INTEL_CE5300
    UWORD32 time_consumed = 0;
    UWORD32 bytes_consumed = 0;
#endif
#endif

#ifdef WINDOWS_TIMER
    TIMER frequency;
#endif
    WORD32 width = 0, height = 0;
    iv_obj_t *codec_obj;
#if defined(GPU_BUILD) && !defined(X86)
//    int ioctl_init();
//    ioctl_init();
#endif

#ifdef X86_MINGW
    //For getting printfs without any delay
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#endif
#ifdef IOS
    sprintf(filename_trace, "%s/iostrace.txt", homedir);
    printf("\ntrace file name = %s", filename_trace);
#endif

#ifdef X86_MINGW
    {
        signal(SIGSEGV, sigsegv_handler);
    }
#endif


#ifndef IOS
    /* Usage */
    if(argc < 2)
    {
        printf("Using test.cfg as configuration file \n");
        strcpy(ac_cfg_fname, "test.cfg");
    }
    else if(argc == 2)
    {
        strcpy(ac_cfg_fname, argv[1]);
    }

#else
    strcpy(ac_cfg_fname, "test.cfg");

#endif


    /***********************************************************************/
    /*                  Initialize Application parameters                  */
    /***********************************************************************/

    strcpy(s_app_ctx.ac_ip_fname, "\0");
    s_app_ctx.dump_q_wr_idx = 0;
    s_app_ctx.dump_q_rd_idx = 0;
    s_app_ctx.display_thread_created = 0;
    s_app_ctx.disp_q_wr_idx = 0;
    s_app_ctx.disp_q_rd_idx = 0;
    s_app_ctx.disp_delay = 0;
    s_app_ctx.loopback = 0;
    s_app_ctx.display = 0;
    s_app_ctx.full_screen = 0;
    s_app_ctx.u4_piclen_flag = 0;
    s_app_ctx.fps = DEFAULT_FPS;
    file_pos = 0;
    total_bytes_comsumed = 0;
    u4_ip_frm_ts = 0;
    u4_op_frm_ts = 0;
#ifdef PROFILE_ENABLE
    memset(peak_window, 0, sizeof(WORD32) * PEAK_WINDOW_SIZE);
#endif
    s_app_ctx.share_disp_buf = DEFAULT_SHARE_DISPLAY_BUF;
    s_app_ctx.u4_num_cores = DEFAULT_NUM_CORES;
    s_app_ctx.i4_degrade_type = 0;
    s_app_ctx.i4_degrade_pics = 0;
    s_app_ctx.e_arch = ARCH_ARM_A9Q;
    s_app_ctx.e_soc = SOC_GENERIC;

    s_app_ctx.u4_strd = STRIDE;

    s_app_ctx.display_thread_handle           = malloc(ithread_get_handle_size());
    s_app_ctx.quit          = 0;
    s_app_ctx.paused        = 0;
    //s_app_ctx.u4_output_present = 0;

    s_app_ctx.get_stride = &default_get_stride;

    s_app_ctx.get_color_fmt = &default_get_color_fmt;

    /* Set function pointers for display */
#ifdef SDL_DISPLAY
    s_app_ctx.disp_init = &sdl_disp_init;
    s_app_ctx.alloc_disp_buffers = &sdl_alloc_disp_buffers;
    s_app_ctx.display_buffer = &sdl_display;
    s_app_ctx.set_disp_buffers = &sdl_set_disp_buffers;
    s_app_ctx.disp_deinit = &sdl_disp_deinit;
    s_app_ctx.disp_usleep = &sdl_disp_usleep;
    s_app_ctx.get_color_fmt = &sdl_get_color_fmt;
    s_app_ctx.get_stride = &sdl_get_stride;
#endif

#ifdef FBDEV_DISPLAY
    s_app_ctx.disp_init = &fbd_disp_init;
    s_app_ctx.alloc_disp_buffers = &fbd_alloc_disp_buffers;
    s_app_ctx.display_buffer = &fbd_display;
    s_app_ctx.set_disp_buffers = &fbd_set_disp_buffers;
    s_app_ctx.disp_deinit = &fbd_disp_deinit;
    s_app_ctx.disp_usleep = &fbd_disp_usleep;
    s_app_ctx.get_color_fmt = &fbd_get_color_fmt;
    s_app_ctx.get_stride = &fbd_get_stride;
#endif

#ifdef INTEL_CE5300
    s_app_ctx.disp_init = &gdl_disp_init;
    s_app_ctx.alloc_disp_buffers = &gdl_alloc_disp_buffers;
    s_app_ctx.display_buffer = &gdl_display;
    s_app_ctx.set_disp_buffers = &gdl_set_disp_buffers;
    s_app_ctx.disp_deinit = &gdl_disp_deinit;
    s_app_ctx.disp_usleep = &gdl_disp_usleep;
    s_app_ctx.get_color_fmt = &gdl_get_color_fmt;
    s_app_ctx.get_stride = &gdl_get_stride;
#endif

#ifdef IOS_DISPLAY
    s_app_ctx.disp_init = &ios_disp_init;
    s_app_ctx.alloc_disp_buffers = &ios_alloc_disp_buffers;
    s_app_ctx.display_buffer = &ios_display;
    s_app_ctx.set_disp_buffers = &ios_set_disp_buffers;
    s_app_ctx.disp_deinit = &ios_disp_deinit;
    s_app_ctx.disp_usleep = &ios_disp_usleep;
    s_app_ctx.get_color_fmt = &ios_get_color_fmt;
    s_app_ctx.get_stride = &ios_get_stride;
#endif

    s_app_ctx.display_deinit_flag = 0;
    s_app_ctx.e_output_chroma_format = IV_YUV_420SP_UV;
    /*************************************************************************/
    /* Parse arguments                                                       */
    /*************************************************************************/

#ifndef IOS
    /* Read command line arguments */
    if(argc > 2)
    {
        for(i = 1; i < (UWORD32)argc; i += 2)
        {
            if(CONFIG == get_argument(argv[i]))
            {
                strcpy(ac_cfg_fname, argv[i + 1]);
                if((fp_cfg_file = fopen(ac_cfg_fname, "r")) == NULL)
                {
                    sprintf(ac_error_str, "Could not open Configuration file %s",
                            ac_cfg_fname);
                    codec_exit(ac_error_str);
                }
                read_cfg_file(&s_app_ctx, fp_cfg_file);
                fclose(fp_cfg_file);
            }
            else
            {
                parse_argument(&s_app_ctx, argv[i], argv[i + 1]);
            }
        }
    }
    else
    {
        if((fp_cfg_file = fopen(ac_cfg_fname, "r")) == NULL)
        {
            sprintf(ac_error_str, "Could not open Configuration file %s",
                    ac_cfg_fname);
            codec_exit(ac_error_str);
        }
        read_cfg_file(&s_app_ctx, fp_cfg_file);
        fclose(fp_cfg_file);
    }
#else
    sprintf(filename_with_path, "%s/%s", homedir, ac_cfg_fname);
    if((fp_cfg_file = fopen(filename_with_path, "r")) == NULL)
    {
        sprintf(ac_error_str, "Could not open Configuration file %s",
                ac_cfg_fname);
        codec_exit(ac_error_str);

    }
    read_cfg_file(&s_app_ctx, fp_cfg_file);
    fclose(fp_cfg_file);

#endif
#ifdef PRINT_PICSIZE
    /* If the binary is used for only getting number of bytes in each picture, then disable the following features */
    s_app_ctx.u4_piclen_flag = 0;
    s_app_ctx.u4_file_save_flag = 0;
    s_app_ctx.u4_chksum_save_flag = 0;
    s_app_ctx.i4_degrade_pics = 0;
    s_app_ctx.i4_degrade_type = 0;
    s_app_ctx.loopback = 0;
    s_app_ctx.share_disp_buf = 0;
    s_app_ctx.display = 0;
#endif

    /* If display is enabled, then turn off shared mode and get color format that is supported by display */
    if(1 == s_app_ctx.display)
    {
        s_app_ctx.share_disp_buf = 0;
        s_app_ctx.e_output_chroma_format = s_app_ctx.get_color_fmt();
    }
    if(strcmp(s_app_ctx.ac_ip_fname, "\0") == 0)
    {
        printf("\nNo input file given for decoding\n");
        exit(-1);
    }


    /***********************************************************************/
    /*          create the file object for input file                      */
    /***********************************************************************/
#ifdef IOS
    sprintf(filename_with_path, "%s/%s", homedir, s_app_ctx.ac_ip_fname);
    ps_ip_file = fopen(filename_with_path, "rb");
#else
    ps_ip_file = fopen(s_app_ctx.ac_ip_fname, "rb");
#endif
    if(NULL == ps_ip_file)
    {
        sprintf(ac_error_str, "Could not open input file %s",
                s_app_ctx.ac_ip_fname);
        codec_exit(ac_error_str);
    }
    /***********************************************************************/
    /*          create the file object for input file                      */
    /***********************************************************************/
    if(1 == s_app_ctx.u4_piclen_flag)
    {
#ifdef IOS
        sprintf(filename_with_path, "%s/%s", homedir, s_app_ctx.ac_piclen_fname);
        ps_piclen_file = fopen(filename_with_path, "rb");
#else
        ps_piclen_file = fopen(s_app_ctx.ac_piclen_fname, "rb");
#endif
        if(NULL == ps_piclen_file)
        {
            sprintf(ac_error_str, "Could not open piclen file %s",
                    s_app_ctx.ac_piclen_fname);
            codec_exit(ac_error_str);
        }
    }

    /***********************************************************************/
    /*          create the file object for output file                     */
    /***********************************************************************/
    if(1 == s_app_ctx.u4_file_save_flag)
    {
#ifdef IOS
        sprintf(filename_with_path, "%s/%s", documentdir, s_app_ctx.ac_op_fname);
        ps_op_file = fopen(filename_with_path, "wb");
#else
        ps_op_file = fopen(s_app_ctx.ac_op_fname, "wb");
#endif

        if(NULL == ps_op_file)
        {
            sprintf(ac_error_str, "Could not open output file %s",
                    s_app_ctx.ac_op_fname);
            codec_exit(ac_error_str);
        }
    }

    /***********************************************************************/
    /*          create the file object for check sum file                  */
    /***********************************************************************/
    if(1 == s_app_ctx.u4_chksum_save_flag)
    {
#if IOS
        sprintf(filename_with_path, "%s/%s", documentdir, s_app_ctx.ac_op_chksum_fname);
        ps_op_chksum_file = fopen(filename_with_path, "wb");
#else
        ps_op_chksum_file = fopen(s_app_ctx.ac_op_chksum_fname, "wb");
#endif
        if(NULL == ps_op_chksum_file)
        {
            sprintf(ac_error_str, "Could not open check sum file %s",
                    s_app_ctx.ac_op_chksum_fname);
            codec_exit(ac_error_str);
        }
    }
    /***********************************************************************/
    /*                      Create decoder instance                        */
    /***********************************************************************/
    {

        ps_out_buf = (ivd_out_bufdesc_t *)malloc(sizeof(ivd_out_bufdesc_t));

        /*****************************************************************************/
        /*   API Call: Initialize the Decoder                                        */
        /*****************************************************************************/
        {
            ihevcd_cxa_create_ip_t s_create_ip;
            ihevcd_cxa_create_op_t s_create_op;
            void *fxns = &ivd_cxa_api_function;

            s_create_ip.s_ivd_create_ip_t.e_cmd = IVD_CMD_CREATE;
            s_create_ip.s_ivd_create_ip_t.u4_share_disp_buf = s_app_ctx.share_disp_buf;
            s_create_ip.s_ivd_create_ip_t.e_output_format = (IV_COLOR_FORMAT_T)s_app_ctx.e_output_chroma_format;
            s_create_ip.s_ivd_create_ip_t.pf_aligned_alloc = ihevca_aligned_malloc;
            s_create_ip.s_ivd_create_ip_t.pf_aligned_free = ihevca_aligned_free;
            s_create_ip.s_ivd_create_ip_t.pv_mem_ctxt = NULL;
            s_create_ip.s_ivd_create_ip_t.u4_size = sizeof(ihevcd_cxa_create_ip_t);
            s_create_op.s_ivd_create_op_t.u4_size = sizeof(ihevcd_cxa_create_op_t);



            ret = ivd_cxa_api_function(NULL, (void *)&s_create_ip,
                                       (void *)&s_create_op);
            if(ret != IV_SUCCESS)
            {
                sprintf(ac_error_str, "Error in Create %8x\n",
                        s_create_op.s_ivd_create_op_t.u4_error_code);
                codec_exit(ac_error_str);
            }
            codec_obj = (iv_obj_t*)s_create_op.s_ivd_create_op_t.pv_handle;
            codec_obj->pv_fxns = fxns;
            codec_obj->u4_size = sizeof(iv_obj_t);
            s_app_ctx.cocodec_obj = codec_obj;
        }

    }


    /*************************************************************************/
    /* set num of cores                                                      */
    /*************************************************************************/
    {

        ihevcd_cxa_ctl_set_num_cores_ip_t s_ctl_set_cores_ip;
        ihevcd_cxa_ctl_set_num_cores_op_t s_ctl_set_cores_op;

        s_ctl_set_cores_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_set_cores_ip.e_sub_cmd = (IVD_CONTROL_API_COMMAND_TYPE_T)IHEVCD_CXA_CMD_CTL_SET_NUM_CORES;
        s_ctl_set_cores_ip.u4_num_cores = s_app_ctx.u4_num_cores;
        s_ctl_set_cores_ip.u4_size = sizeof(ihevcd_cxa_ctl_set_num_cores_ip_t);
        s_ctl_set_cores_op.u4_size = sizeof(ihevcd_cxa_ctl_set_num_cores_op_t);

        ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_set_cores_ip,
                                   (void *)&s_ctl_set_cores_op);
        if(ret != IV_SUCCESS)
        {
            sprintf(ac_error_str, "\nError in setting number of cores");
            codec_exit(ac_error_str);
        }

    }
    /*************************************************************************/
    /* set processsor                                                        */
    /*************************************************************************/
    {

        ihevcd_cxa_ctl_set_processor_ip_t s_ctl_set_num_processor_ip;
        ihevcd_cxa_ctl_set_processor_op_t s_ctl_set_num_processor_op;

        s_ctl_set_num_processor_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_set_num_processor_ip.e_sub_cmd = (IVD_CONTROL_API_COMMAND_TYPE_T)IHEVCD_CXA_CMD_CTL_SET_PROCESSOR;
        s_ctl_set_num_processor_ip.u4_arch = s_app_ctx.e_arch;
        s_ctl_set_num_processor_ip.u4_soc = s_app_ctx.e_soc;
        s_ctl_set_num_processor_ip.u4_size = sizeof(ihevcd_cxa_ctl_set_processor_ip_t);
        s_ctl_set_num_processor_op.u4_size = sizeof(ihevcd_cxa_ctl_set_processor_op_t);

        ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_set_num_processor_ip,
                                   (void *)&s_ctl_set_num_processor_op);
        if(ret != IV_SUCCESS)
        {
            sprintf(ac_error_str, "\nError in setting Processor type");
            codec_exit(ac_error_str);
        }

    }

    flush_output(codec_obj, &s_app_ctx, ps_out_buf,
                 pu1_bs_buf, &u4_op_frm_ts,
                 ps_op_file, ps_op_chksum_file,
                 u4_ip_frm_ts, u4_bytes_remaining);

    /*****************************************************************************/
    /*   Decode header to get width and height and buffer sizes                  */
    /*****************************************************************************/
    {
        ivd_video_decode_ip_t s_video_decode_ip;
        ivd_video_decode_op_t s_video_decode_op;



        {
            ivd_ctl_set_config_ip_t s_ctl_ip;
            ivd_ctl_set_config_op_t s_ctl_op;


            s_ctl_ip.u4_disp_wd = STRIDE;
            if(1 == s_app_ctx.display)
                s_ctl_ip.u4_disp_wd = s_app_ctx.get_stride();

            s_ctl_ip.e_frm_skip_mode = IVD_SKIP_NONE;
            s_ctl_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
            s_ctl_ip.e_vid_dec_mode = IVD_DECODE_HEADER;
            s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
            s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
            s_ctl_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
            s_ctl_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

            ret = ivd_cxa_api_function((iv_obj_t*)codec_obj, (void *)&s_ctl_ip,
                                       (void *)&s_ctl_op);
            if(ret != IV_SUCCESS)
            {
                sprintf(ac_error_str,
                        "\nError in setting the codec in header decode mode");
                codec_exit(ac_error_str);
            }
        }

        /* Allocate input buffer for header */
        u4_ip_buf_len = 256 * 1024;
        pu1_bs_buf = (UWORD8 *)malloc(u4_ip_buf_len);

        if(pu1_bs_buf == NULL)
        {
            sprintf(ac_error_str,
                    "\nAllocation failure for input buffer of i4_size %d",
                    u4_ip_buf_len);
            codec_exit(ac_error_str);
        }

        do
        {
            WORD32 numbytes;
            if(0 == s_app_ctx.u4_piclen_flag)
            {
                fseek(ps_ip_file, file_pos, SEEK_SET);
                numbytes = u4_ip_buf_len;
            }
            else
            {
                WORD32 entries;
                entries = fscanf(ps_piclen_file, "%d\n", &numbytes);
                if(1 != entries)
                    numbytes = u4_ip_buf_len;
            }

            u4_bytes_remaining = fread(pu1_bs_buf, sizeof(UWORD8), numbytes,
                                       ps_ip_file);

            if(0 == u4_bytes_remaining)
            {
                sprintf(ac_error_str, "\nUnable to read from input file");
                codec_exit(ac_error_str);
            }

            s_video_decode_ip.e_cmd = IVD_CMD_VIDEO_DECODE;
            s_video_decode_ip.u4_ts = u4_ip_frm_ts;
            s_video_decode_ip.pv_stream_buffer = pu1_bs_buf;
            s_video_decode_ip.u4_num_Bytes = u4_bytes_remaining;
            s_video_decode_ip.u4_size = sizeof(ivd_video_decode_ip_t);
            s_video_decode_op.u4_size = sizeof(ivd_video_decode_op_t);

            /*****************************************************************************/
            /*   API Call: Header Decode                                                  */
            /*****************************************************************************/
            ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_video_decode_ip,
                                       (void *)&s_video_decode_op);

            if(ret != IV_SUCCESS)
            {
                sprintf(ac_error_str, "\nError in header decode %x",
                        s_video_decode_op.u4_error_code);
                // codec_exit(ac_error_str);
            }

            u4_num_bytes_dec = s_video_decode_op.u4_num_bytes_consumed;
#ifndef PROFILE_ENABLE
            printf("%d\n",s_video_decode_op.u4_num_bytes_consumed);
#endif
            file_pos += u4_num_bytes_dec;
            total_bytes_comsumed += u4_num_bytes_dec;
        }while(ret != IV_SUCCESS);

        /* copy pic_wd and pic_ht to initialize buffers */
        s_app_ctx.u4_pic_wd = s_video_decode_op.u4_pic_wd;
        s_app_ctx.u4_pic_ht = s_video_decode_op.u4_pic_ht;

        free(pu1_bs_buf);

#if IOS_DISPLAY
        s_app_ctx.i4_screen_wd = screen_wd;
        s_app_ctx.i4_screen_ht = screen_ht;
#endif
        {

            ivd_ctl_getbufinfo_ip_t s_ctl_ip;
            ivd_ctl_getbufinfo_op_t s_ctl_op;
            WORD32 outlen = 0;

            s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
            s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_GETBUFINFO;
            s_ctl_ip.u4_size = sizeof(ivd_ctl_getbufinfo_ip_t);
            s_ctl_op.u4_size = sizeof(ivd_ctl_getbufinfo_op_t);
            ret = ivd_cxa_api_function((iv_obj_t*)codec_obj, (void *)&s_ctl_ip,
                                       (void *)&s_ctl_op);
            if(ret != IV_SUCCESS)
            {
                sprintf(ac_error_str, "Error in Get Buf Info %x", s_ctl_op.u4_error_code);
                codec_exit(ac_error_str);
            }

            /* Allocate bitstream buffer */
            u4_ip_buf_len = s_ctl_op.u4_min_in_buf_size[0];
#ifdef ADAPTIVE_TEST
            u4_ip_buf_len = ADAPTIVE_MAX_WD * ADAPTIVE_MAX_HT * 3 >> 1;
#endif
            pu1_bs_buf = (UWORD8 *)malloc(u4_ip_buf_len);

            if(pu1_bs_buf == NULL)
            {
                sprintf(ac_error_str,
                        "\nAllocation failure for input buffer of i4_size %d",
                        u4_ip_buf_len);
                codec_exit(ac_error_str);
            }

#ifdef ADAPTIVE_TEST
            switch(s_app_ctx.e_output_chroma_format)
            {
                case IV_YUV_420P:
                {
                    s_ctl_op.u4_min_out_buf_size[0] = ADAPTIVE_MAX_WD * ADAPTIVE_MAX_HT;
                    s_ctl_op.u4_min_out_buf_size[1] = ADAPTIVE_MAX_WD * ADAPTIVE_MAX_HT >> 2;
                    s_ctl_op.u4_min_out_buf_size[2] = ADAPTIVE_MAX_WD * ADAPTIVE_MAX_HT >> 2;
                    break;
                }
                case IV_YUV_420SP_UV:
                case IV_YUV_420SP_VU:
                {
                    s_ctl_op.u4_min_out_buf_size[0] = ADAPTIVE_MAX_WD * ADAPTIVE_MAX_HT;
                    s_ctl_op.u4_min_out_buf_size[1] = ADAPTIVE_MAX_WD * ADAPTIVE_MAX_HT >> 1;
                    s_ctl_op.u4_min_out_buf_size[2] = 0;
                    break;
                }
                case IV_YUV_422ILE:
                {
                    s_ctl_op.u4_min_out_buf_size[0] = ADAPTIVE_MAX_WD * ADAPTIVE_MAX_HT * 2;
                    s_ctl_op.u4_min_out_buf_size[1] = 0;
                    s_ctl_op.u4_min_out_buf_size[2] = 0;
                    break;
                }
                case IV_RGBA_8888:
                {
                    s_ctl_op.u4_min_out_buf_size[0] = ADAPTIVE_MAX_WD * ADAPTIVE_MAX_HT * 4;
                    s_ctl_op.u4_min_out_buf_size[1] = 0;
                    s_ctl_op.u4_min_out_buf_size[2] = 0;
                    break;
                }
                case IV_RGB_565:
                {
                    s_ctl_op.u4_min_out_buf_size[0] = ADAPTIVE_MAX_WD * ADAPTIVE_MAX_HT * 2;
                    s_ctl_op.u4_min_out_buf_size[1] = 0;
                    s_ctl_op.u4_min_out_buf_size[2] = 0;
                    break;
                }
                default:
                    break;

            }
#endif
            /* Allocate output buffer only if display buffers are not shared */
            /* Or if shared and output is 420P */
            if((0 == s_app_ctx.share_disp_buf) || (IV_YUV_420P == s_app_ctx.e_output_chroma_format))
            {
                ps_out_buf->u4_min_out_buf_size[0] =
                                s_ctl_op.u4_min_out_buf_size[0];
                ps_out_buf->u4_min_out_buf_size[1] =
                                s_ctl_op.u4_min_out_buf_size[1];
                ps_out_buf->u4_min_out_buf_size[2] =
                                s_ctl_op.u4_min_out_buf_size[2];

                outlen = s_ctl_op.u4_min_out_buf_size[0];
                if(s_ctl_op.u4_min_num_out_bufs > 1)
                    outlen += s_ctl_op.u4_min_out_buf_size[1];

                if(s_ctl_op.u4_min_num_out_bufs > 2)
                    outlen += s_ctl_op.u4_min_out_buf_size[2];

                ps_out_buf->pu1_bufs[0] = (UWORD8 *)malloc(outlen);
                if(ps_out_buf->pu1_bufs[0] == NULL)
                {
                    sprintf(ac_error_str, "\nAllocation failure for output buffer of i4_size %d",
                            outlen);
                    codec_exit(ac_error_str);
                }

                if(s_ctl_op.u4_min_num_out_bufs > 1)
                    ps_out_buf->pu1_bufs[1] = ps_out_buf->pu1_bufs[0]
                                    + (s_ctl_op.u4_min_out_buf_size[0]);

                if(s_ctl_op.u4_min_num_out_bufs > 2)
                    ps_out_buf->pu1_bufs[2] = ps_out_buf->pu1_bufs[1]
                                    + (s_ctl_op.u4_min_out_buf_size[1]);

                ps_out_buf->u4_num_bufs = s_ctl_op.u4_min_num_out_bufs;
            }

#ifdef APP_EXTRA_BUFS
            s_app_ctx.disp_delay = EXTRA_DISP_BUFFERS;
            s_ctl_op.u4_num_disp_bufs += EXTRA_DISP_BUFFERS;
#endif

            /*****************************************************************************/
            /*   API Call: Allocate display buffers for display buffer shared case       */
            /*****************************************************************************/

            for(i = 0; i < s_ctl_op.u4_num_disp_bufs; i++)
            {

                s_app_ctx.s_disp_buffers[i].u4_min_out_buf_size[0] =
                                s_ctl_op.u4_min_out_buf_size[0];
                s_app_ctx.s_disp_buffers[i].u4_min_out_buf_size[1] =
                                s_ctl_op.u4_min_out_buf_size[1];
                s_app_ctx.s_disp_buffers[i].u4_min_out_buf_size[2] =
                                s_ctl_op.u4_min_out_buf_size[2];

                outlen = s_ctl_op.u4_min_out_buf_size[0];
                if(s_ctl_op.u4_min_num_out_bufs > 1)
                    outlen += s_ctl_op.u4_min_out_buf_size[1];

                if(s_ctl_op.u4_min_num_out_bufs > 2)
                    outlen += s_ctl_op.u4_min_out_buf_size[2];

                s_app_ctx.s_disp_buffers[i].pu1_bufs[0] = (UWORD8 *)malloc(outlen);

                if(s_app_ctx.s_disp_buffers[i].pu1_bufs[0] == NULL)
                {
                    sprintf(ac_error_str,
                            "\nAllocation failure for output buffer of i4_size %d",
                            outlen);
                    codec_exit(ac_error_str);
                }

                if(s_ctl_op.u4_min_num_out_bufs > 1)
                    s_app_ctx.s_disp_buffers[i].pu1_bufs[1] =
                                    s_app_ctx.s_disp_buffers[i].pu1_bufs[0]
                                                    + (s_ctl_op.u4_min_out_buf_size[0]);

                if(s_ctl_op.u4_min_num_out_bufs > 2)
                    s_app_ctx.s_disp_buffers[i].pu1_bufs[2] =
                                    s_app_ctx.s_disp_buffers[i].pu1_bufs[1]
                                                    + (s_ctl_op.u4_min_out_buf_size[1]);

                s_app_ctx.s_disp_buffers[i].u4_num_bufs =
                                s_ctl_op.u4_min_num_out_bufs;
            }
            s_app_ctx.num_disp_buf = s_ctl_op.u4_num_disp_bufs;
        }

        /* Create display thread and wait for the display buffers to be initialized */
        if(1 == s_app_ctx.display)
        {
            if(0 == s_app_ctx.display_thread_created)
            {
                s_app_ctx.display_init_done = 0;
                ithread_create(s_app_ctx.display_thread_handle, NULL,
                                                    (void *) &display_thread, (void *) &s_app_ctx);
                s_app_ctx.display_thread_created = 1;

                while(1)
                {
                    if(s_app_ctx.display_init_done)
                        break;

                    ithread_msleep(1);
                }
            }
            s_app_ctx.u4_strd = s_app_ctx.get_stride();
        }


        /*****************************************************************************/
        /*   API Call: Send the allocated display buffers to codec                   */
        /*****************************************************************************/
        {
            ivd_set_display_frame_ip_t s_set_display_frame_ip;
            ivd_set_display_frame_op_t s_set_display_frame_op;

            s_set_display_frame_ip.e_cmd = IVD_CMD_SET_DISPLAY_FRAME;
            s_set_display_frame_ip.u4_size = sizeof(ivd_set_display_frame_ip_t);
            s_set_display_frame_op.u4_size = sizeof(ivd_set_display_frame_op_t);

            s_set_display_frame_ip.num_disp_bufs = s_app_ctx.num_disp_buf;

            memcpy(&(s_set_display_frame_ip.s_disp_buffer),
                   &(s_app_ctx.s_disp_buffers),
                   s_app_ctx.num_disp_buf * sizeof(ivd_out_bufdesc_t));

            ret = ivd_cxa_api_function((iv_obj_t *)codec_obj,
                                       (void *)&s_set_display_frame_ip,
                                       (void *)&s_set_display_frame_op);

            if(IV_SUCCESS != ret)
            {
                sprintf(ac_error_str, "Error in Set display frame");
                codec_exit(ac_error_str);
            }

        }

    }

    /*************************************************************************/
    /* Get frame dimensions for display buffers such as x_offset,y_offset    */
    /* etc. This information might be needed to set display buffer           */
    /* offsets in case of shared display buffer mode                         */
    /*************************************************************************/
    {

        ihevcd_cxa_ctl_get_frame_dimensions_ip_t s_ctl_get_frame_dimensions_ip;
        ihevcd_cxa_ctl_get_frame_dimensions_op_t s_ctl_get_frame_dimensions_op;

        s_ctl_get_frame_dimensions_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_get_frame_dimensions_ip.e_sub_cmd =
                        (IVD_CONTROL_API_COMMAND_TYPE_T)IHEVCD_CXA_CMD_CTL_GET_BUFFER_DIMENSIONS;
        s_ctl_get_frame_dimensions_ip.u4_size =
                        sizeof(ihevcd_cxa_ctl_get_frame_dimensions_ip_t);
        s_ctl_get_frame_dimensions_op.u4_size =
                        sizeof(ihevcd_cxa_ctl_get_frame_dimensions_op_t);

        ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_get_frame_dimensions_ip,
                                   (void *)&s_ctl_get_frame_dimensions_op);
        if(IV_SUCCESS != ret)
        {
            sprintf(ac_error_str, "Error in Get buffer Dimensions");
            codec_exit(ac_error_str);
        }

/*
        printf("Frame offsets due to padding\n");
        printf("s_ctl_get_frame_dimensions_op.x_offset[0] %d s_ctl_get_frame_dimensions_op.y_offset[0] %d\n",
               s_ctl_get_frame_dimensions_op.u4_x_offset[0],
               s_ctl_get_frame_dimensions_op.u4_y_offset[0]);
*/
    }


    /*************************************************************************/
    /* Get VUI parameters                                                    */
    /*************************************************************************/
    {

        ihevcd_cxa_ctl_get_vui_params_ip_t s_ctl_get_vui_params_ip;
        ihevcd_cxa_ctl_get_vui_params_op_t s_ctl_get_vui_params_op;

        s_ctl_get_vui_params_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_get_vui_params_ip.e_sub_cmd =
                        (IVD_CONTROL_API_COMMAND_TYPE_T)IHEVCD_CXA_CMD_CTL_GET_VUI_PARAMS;
        s_ctl_get_vui_params_ip.u4_size =
                        sizeof(ihevcd_cxa_ctl_get_vui_params_ip_t);
        s_ctl_get_vui_params_op.u4_size =
                        sizeof(ihevcd_cxa_ctl_get_vui_params_op_t);

        ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_get_vui_params_ip,
                                   (void *)&s_ctl_get_vui_params_op);
        if(IV_SUCCESS != ret)
        {
            sprintf(ac_error_str, "Error in Get VUI params");
            //codec_exit(ac_error_str);
        }

    }


    /*************************************************************************/
    /* Set the decoder in frame decode mode. It was set in header decode     */
    /* mode earlier                                                          */
    /*************************************************************************/
    {

        ivd_ctl_set_config_ip_t s_ctl_ip;
        ivd_ctl_set_config_op_t s_ctl_op;

        s_ctl_ip.u4_disp_wd = STRIDE;
        if(1 == s_app_ctx.display)
            s_ctl_ip.u4_disp_wd = s_app_ctx.get_stride();
        s_ctl_ip.e_frm_skip_mode = IVD_SKIP_NONE;

        s_ctl_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
        s_ctl_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
        s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
        s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
        s_ctl_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);

        s_ctl_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

        ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_ip, (void *)&s_ctl_op);

        if(IV_SUCCESS != ret)
        {
            sprintf(ac_error_str, "Error in Set Parameters");
            //codec_exit(ac_error_str);
        }

    }
    /*************************************************************************/
    /* If required disable deblocking and sao at given level                 */
    /*************************************************************************/
    set_degrade(codec_obj, s_app_ctx.i4_degrade_type, s_app_ctx.i4_degrade_pics);
#ifdef WINDOWS_TIMER
    QueryPerformanceFrequency(&frequency);
#endif
#ifndef PRINT_PICSIZE
    get_version(codec_obj);
#endif
    while(u4_op_frm_ts < (s_app_ctx.u4_max_frm_ts + s_app_ctx.disp_delay))
    {

#ifdef TEST_FLUSH
        if(u4_ip_frm_ts == FLUSH_FRM_CNT)
        {
            ivd_ctl_flush_ip_t s_ctl_ip;
            ivd_ctl_flush_op_t s_ctl_op;

            s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
            s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_FLUSH;
            s_ctl_ip.u4_size = sizeof(ivd_ctl_flush_ip_t);
            s_ctl_op.u4_size = sizeof(ivd_ctl_flush_op_t);
            ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_ip,
                                       (void *)&s_ctl_op);

            if(ret != IV_SUCCESS)
            {
                printf("Error in Setting the decoder in flush mode\n");
            }
            file_pos = 0;

            fseek(ps_ip_file, file_pos, SEEK_SET);

        }
#endif
        if(u4_ip_frm_ts < s_app_ctx.num_disp_buf)
        {
            release_disp_frame(codec_obj, u4_ip_frm_ts);
        }


        /*************************************************************************/
        /* set num of cores                                                      */
        /*************************************************************************/
#ifdef DYNAMIC_NUMCORES
        {

            ihevcd_cxa_ctl_set_num_cores_ip_t s_ctl_set_cores_ip;
            ihevcd_cxa_ctl_set_num_cores_op_t s_ctl_set_cores_op;

            s_ctl_set_cores_ip.e_cmd = IVD_CMD_VIDEO_CTL;
            s_ctl_set_cores_ip.e_sub_cmd = IHEVCD_CXA_CMD_CTL_SET_NUM_CORES;
            s_ctl_set_cores_ip.u4_num_cores =  1 + 3 * (u4_ip_frm_ts % 2);
            s_ctl_set_cores_ip.u4_size = sizeof(ihevcd_cxa_ctl_set_num_cores_ip_t);
            s_ctl_set_cores_op.u4_size = sizeof(ihevcd_cxa_ctl_set_num_cores_op_t);

            ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_set_cores_ip,
                                       (void *)&s_ctl_set_cores_op);
            if(ret != IV_SUCCESS)
            {
                sprintf(ac_error_str, "\nError in setting number of cores");
                codec_exit(ac_error_str);
            }

        }
#endif
        /***********************************************************************/
        /*   Seek the file to start of current frame, this is equavelent of    */
        /*   having a parcer which tells the start of current frame            */
        /***********************************************************************/
        {
            WORD32 numbytes;

            if(0 == s_app_ctx.u4_piclen_flag)
            {
                fseek(ps_ip_file, file_pos, SEEK_SET);
                numbytes = u4_ip_buf_len;
            }
            else
            {
                WORD32 entries;
                entries = fscanf(ps_piclen_file, "%d\n", &numbytes);
                if(1 != entries)
                    numbytes = u4_ip_buf_len;
            }

            u4_bytes_remaining = fread(pu1_bs_buf, sizeof(UWORD8),
                                       numbytes, ps_ip_file);

            if(u4_bytes_remaining == 0)
            {
                if(1 == s_app_ctx.loopback)
                {
                    file_pos = 0;
                    if(0 == s_app_ctx.u4_piclen_flag)
                    {
                        fseek(ps_ip_file, file_pos, SEEK_SET);
                        numbytes = u4_ip_buf_len;
                    }
                    else
                    {
                        WORD32 entries;
                        entries = fscanf(ps_piclen_file, "%d\n", &numbytes);
                        if(1 != entries)
                            numbytes = u4_ip_buf_len;
                    }


                    u4_bytes_remaining = fread(pu1_bs_buf, sizeof(UWORD8),
                                               numbytes, ps_ip_file);
                }
                else
                    break;
            }
        }

        /*********************************************************************/
        /* Following calls can be enabled at diffent times                   */
        /*********************************************************************/
#if ENABLE_DEGRADE
        if(u4_op_frm_ts >= 10000)
            disable_deblocking(codec_obj, 4);

        if(u4_op_frm_ts == 30000)
            enable_deblocking(codec_obj);

        if(u4_op_frm_ts == 10000)
            enable_skippb_frames(codec_obj);

        if(u4_op_frm_ts == 60000)
            disable_skippb_frames(codec_obj);

        if(u4_op_frm_ts == 30000)
            enable_skipb_frames(codec_obj);

        if(u4_op_frm_ts == 60000)
            disable_skipb_frames(codec_obj);
#endif


        {
            ivd_video_decode_ip_t s_video_decode_ip;
            ivd_video_decode_op_t s_video_decode_op;
#ifdef PROFILE_ENABLE
            UWORD32 s_elapsed_time;
            TIMER s_start_timer;
            TIMER s_end_timer;
#endif


            s_video_decode_ip.e_cmd = IVD_CMD_VIDEO_DECODE;
            s_video_decode_ip.u4_ts = u4_ip_frm_ts;
            s_video_decode_ip.pv_stream_buffer = pu1_bs_buf;
            s_video_decode_ip.u4_num_Bytes = u4_bytes_remaining;
            s_video_decode_ip.u4_size = sizeof(ivd_video_decode_ip_t);
            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[0] =
                            ps_out_buf->u4_min_out_buf_size[0];
            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[1] =
                            ps_out_buf->u4_min_out_buf_size[1];
            s_video_decode_ip.s_out_buffer.u4_min_out_buf_size[2] =
                            ps_out_buf->u4_min_out_buf_size[2];

            s_video_decode_ip.s_out_buffer.pu1_bufs[0] =
                            ps_out_buf->pu1_bufs[0];
            s_video_decode_ip.s_out_buffer.pu1_bufs[1] =
                            ps_out_buf->pu1_bufs[1];
            s_video_decode_ip.s_out_buffer.pu1_bufs[2] =
                            ps_out_buf->pu1_bufs[2];
            s_video_decode_ip.s_out_buffer.u4_num_bufs =
                            ps_out_buf->u4_num_bufs;
            s_video_decode_op.u4_size = sizeof(ivd_video_decode_op_t);

            /* Get display buffer pointers */
            if(1 == s_app_ctx.display)
            {
                WORD32 wr_idx;

                wr_idx = dispq_producer_dequeue(&s_app_ctx);

                if(s_app_ctx.quit)
                    break;

                s_app_ctx.set_disp_buffers(s_app_ctx.pv_disp_ctx, wr_idx,
                                           &s_video_decode_ip.s_out_buffer.pu1_bufs[0],
                                           &s_video_decode_ip.s_out_buffer.pu1_bufs[1],
                                           &s_video_decode_ip.s_out_buffer.pu1_bufs[2]);
            }

            /*****************************************************************************/
            /*   API Call: Video Decode                                                  */
            /*****************************************************************************/

            GETTIME(&s_start_timer);

            ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_video_decode_ip,
                                       (void *)&s_video_decode_op);


            GETTIME(&s_end_timer);
            ELAPSEDTIME(s_start_timer, s_end_timer, s_elapsed_time, frequency);
#ifdef PROFILE_ENABLE
            {
                UWORD32 peak_avg, id;
                u4_tot_cycles += s_elapsed_time;
                peak_window[peak_window_idx++] = s_elapsed_time;
                if(peak_window_idx == PEAK_WINDOW_SIZE)
                    peak_window_idx = 0;
                peak_avg = 0;
                for(id = 0; id < PEAK_WINDOW_SIZE; id++)
                {
                    peak_avg += peak_window[id];
                }
                peak_avg /= PEAK_WINDOW_SIZE;
                if(peak_avg > peak_avg_max)
                    peak_avg_max = peak_avg;
                frm_cnt++;

                printf("FrameNum: %4d TimeTaken(microsec): %6d AvgTime: %6d PeakAvgTimeMax: %6d Output: %2d NumBytes: %6d \n",
                       frm_cnt, s_elapsed_time, u4_tot_cycles / frm_cnt, peak_avg_max, s_video_decode_op.u4_output_present, s_video_decode_op.u4_num_bytes_consumed);

            }
#ifdef INTEL_CE5300
            time_consumed += s_elapsed_time;
            bytes_consumed += s_video_decode_op.u4_num_bytes_consumed;
            if(!(frm_cnt % (s_app_ctx.fps)))
            {
                time_consumed = time_consumed / s_app_ctx.fps;
                printf("Average decode time(micro sec) for the last second = %6d\n", time_consumed);
                printf("Average bitrate(kb) for the last second = %6d\n", (bytes_consumed * 8) / 1024);
                time_consumed = 0;
                bytes_consumed = 0;

            }
#endif
#else
            printf("%d\n", s_video_decode_op.u4_num_bytes_consumed);
#endif

            if(ret != IV_SUCCESS)
            {
                printf("Error in video Frame decode : ret %x Error %x\n", ret,
                       s_video_decode_op.u4_error_code);
            }

            if((IV_SUCCESS != ret) &&
                            ((s_video_decode_op.u4_error_code & 0xFF) == IVD_RES_CHANGED))
            {
                ivd_ctl_reset_ip_t s_ctl_ip;
                ivd_ctl_reset_op_t s_ctl_op;

                flush_output(codec_obj, &s_app_ctx, ps_out_buf,
                             pu1_bs_buf, &u4_op_frm_ts,
                             ps_op_file, ps_op_chksum_file,
                             u4_ip_frm_ts, u4_bytes_remaining);

                s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
                s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_RESET;
                s_ctl_ip.u4_size = sizeof(ivd_ctl_reset_ip_t);
                s_ctl_op.u4_size = sizeof(ivd_ctl_reset_op_t);

                ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_ip,
                                           (void *)&s_ctl_op);
                if(IV_SUCCESS != ret)
                {
                    sprintf(ac_error_str, "Error in Reset");
                    codec_exit(ac_error_str);
                }
                /*************************************************************************/
                /* set num of cores                                                      */
                /*************************************************************************/
                {

                    ihevcd_cxa_ctl_set_num_cores_ip_t s_ctl_set_cores_ip;
                    ihevcd_cxa_ctl_set_num_cores_op_t s_ctl_set_cores_op;

                    s_ctl_set_cores_ip.e_cmd = IVD_CMD_VIDEO_CTL;
                    s_ctl_set_cores_ip.e_sub_cmd = (IVD_CONTROL_API_COMMAND_TYPE_T)IHEVCD_CXA_CMD_CTL_SET_NUM_CORES;
                    s_ctl_set_cores_ip.u4_num_cores = s_app_ctx.u4_num_cores;
                    s_ctl_set_cores_ip.u4_size = sizeof(ihevcd_cxa_ctl_set_num_cores_ip_t);
                    s_ctl_set_cores_op.u4_size = sizeof(ihevcd_cxa_ctl_set_num_cores_op_t);

                    ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_set_cores_ip,
                                               (void *)&s_ctl_set_cores_op);
                    if(ret != IV_SUCCESS)
                    {
                        sprintf(ac_error_str, "\nError in setting number of cores");
                        codec_exit(ac_error_str);
                    }

                }
                /*************************************************************************/
                /* set processsor                                                        */
                /*************************************************************************/
                {

                    ihevcd_cxa_ctl_set_processor_ip_t s_ctl_set_num_processor_ip;
                    ihevcd_cxa_ctl_set_processor_op_t s_ctl_set_num_processor_op;

                    s_ctl_set_num_processor_ip.e_cmd = IVD_CMD_VIDEO_CTL;
                    s_ctl_set_num_processor_ip.e_sub_cmd = (IVD_CONTROL_API_COMMAND_TYPE_T)IHEVCD_CXA_CMD_CTL_SET_PROCESSOR;
                    s_ctl_set_num_processor_ip.u4_arch = s_app_ctx.e_arch;
                    s_ctl_set_num_processor_ip.u4_soc = s_app_ctx.e_soc;
                    s_ctl_set_num_processor_ip.u4_size = sizeof(ihevcd_cxa_ctl_set_processor_ip_t);
                    s_ctl_set_num_processor_op.u4_size = sizeof(ihevcd_cxa_ctl_set_processor_op_t);

                    ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_ctl_set_num_processor_ip,
                                               (void *)&s_ctl_set_num_processor_op);
                    if(ret != IV_SUCCESS)
                    {
                        sprintf(ac_error_str, "\nError in setting Processor type");
                        codec_exit(ac_error_str);
                    }

                }
            }
            /*************************************************************************/
            /* Get SEI mastering display color volume parameters                     */
            /*************************************************************************/
            if(1 == s_video_decode_op.u4_output_present)
            {

                ihevcd_cxa_ctl_get_sei_mastering_params_ip_t s_ctl_get_sei_mastering_params_ip;
                ihevcd_cxa_ctl_get_sei_mastering_params_op_t s_ctl_get_sei_mastering_params_op;

                s_ctl_get_sei_mastering_params_ip.e_cmd = IVD_CMD_VIDEO_CTL;
                s_ctl_get_sei_mastering_params_ip.e_sub_cmd =
                                (IVD_CONTROL_API_COMMAND_TYPE_T)IHEVCD_CXA_CMD_CTL_GET_SEI_MASTERING_PARAMS;
                s_ctl_get_sei_mastering_params_ip.u4_size =
                                sizeof(ihevcd_cxa_ctl_get_sei_mastering_params_ip_t);
                s_ctl_get_sei_mastering_params_op.u4_size =
                                sizeof(ihevcd_cxa_ctl_get_sei_mastering_params_op_t);

                ret = ivd_cxa_api_function((iv_obj_t *)codec_obj,
                                     (void *)&s_ctl_get_sei_mastering_params_ip,
                                     (void *)&s_ctl_get_sei_mastering_params_op);
                if(IV_SUCCESS != ret)
                {
                    sprintf(ac_error_str, "Error in Get SEI mastering params");
                    //codec_exit(ac_error_str);
                }

            }


            if((1 == s_app_ctx.display) &&
                            (1 == s_video_decode_op.u4_output_present))
            {
                dispq_producer_queue(&s_app_ctx);
            }

            if(IV_B_FRAME == s_video_decode_op.e_pic_type)
                s_app_ctx.b_pic_present |= 1;

            u4_num_bytes_dec = s_video_decode_op.u4_num_bytes_consumed;

            file_pos += u4_num_bytes_dec;
            total_bytes_comsumed += u4_num_bytes_dec;
            u4_ip_frm_ts++;


            if(1 == s_video_decode_op.u4_output_present)
            {
                width = s_video_decode_op.s_disp_frm_buf.u4_y_wd;
                height = s_video_decode_op.s_disp_frm_buf.u4_y_ht;
                dump_output(&s_app_ctx, &(s_video_decode_op.s_disp_frm_buf),
                            s_video_decode_op.u4_disp_buf_id, ps_op_file,
                            ps_op_chksum_file,
                            u4_op_frm_ts, s_app_ctx.u4_file_save_flag,
                            s_app_ctx.u4_chksum_save_flag);

                u4_op_frm_ts++;
            }
            else
            {
                if((s_video_decode_op.u4_error_code >> IVD_FATALERROR) & 1)
                {
                    printf("Fatal error\n");
                    break;
                }
            }

        }
    }

    /***********************************************************************/
    /*      To get the last decoded frames, call process with NULL input    */
    /***********************************************************************/
    flush_output(codec_obj, &s_app_ctx, ps_out_buf,
                 pu1_bs_buf, &u4_op_frm_ts,
                 ps_op_file, ps_op_chksum_file,
                 u4_ip_frm_ts, u4_bytes_remaining);

    /* set disp_end flag */
    s_app_ctx.quit = 1;


#ifdef PROFILE_ENABLE
    printf("Summary\n");
    printf("Input filename                  : %s\n", s_app_ctx.ac_ip_fname);
    printf("Output Width                    : %-4d\n", width);
    printf("Output Height                   : %-4d\n", height);

    if(frm_cnt)
    {
        double avg = u4_tot_cycles / frm_cnt;
        double bytes_avg = total_bytes_comsumed / frm_cnt;
        double bitrate = (bytes_avg * 8 * s_app_ctx.fps) / 1000000;
        printf("Bitrate @ %2d fps(mbps)          : %-6.2f\n", s_app_ctx.fps, bitrate);
        printf("Average decode time(micro sec)  : %-6d\n", (WORD32)avg);
        printf("Avg Peak decode time(%2d frames) : %-6d\n", PEAK_WINDOW_SIZE, (WORD32)peak_avg_max);
        avg = (u4_tot_cycles + u4_tot_fmt_cycles) * 1.0 / frm_cnt;

        if(0 == s_app_ctx.share_disp_buf)
            printf("FPS achieved (with format conv) : %-3.2f\n", 1000000 / avg);
        else
            printf("FPS achieved                    : %-3.2f\n", 1000000 / avg);
    }
#endif
    /***********************************************************************/
    /*   Clear the decoder, close all the files, free all the memory       */
    /***********************************************************************/
    if(1 == s_app_ctx.display)
    {
        s_app_ctx.display_deinit_flag = 1;
        /* wait for display to finish */
        if(s_app_ctx.display_thread_created)
        {
            ithread_join(s_app_ctx.display_thread_handle, NULL);
        }
        free(s_app_ctx.display_thread_handle);
    }

    {
        ivd_delete_ip_t s_delete_dec_ip;
        ivd_delete_op_t s_delete_dec_op;

        s_delete_dec_ip.e_cmd = IVD_CMD_DELETE;
        s_delete_dec_ip.u4_size = sizeof(ivd_delete_ip_t);
        s_delete_dec_op.u4_size = sizeof(ivd_delete_op_t);

        ret = ivd_cxa_api_function((iv_obj_t *)codec_obj, (void *)&s_delete_dec_ip,
                                   (void *)&s_delete_dec_op);

        if(IV_SUCCESS != ret)
        {
            sprintf(ac_error_str, "Error in Codec delete");
            codec_exit(ac_error_str);
        }
    }
    /***********************************************************************/
    /*              Close all the files and free all the memory            */
    /***********************************************************************/
    {
        fclose(ps_ip_file);

        if(1 == s_app_ctx.u4_file_save_flag)
        {
            fclose(ps_op_file);
        }
        if(1 == s_app_ctx.u4_chksum_save_flag)
        {
            fclose(ps_op_chksum_file);
        }

    }

    if(0 == s_app_ctx.share_disp_buf)
    {
        free(ps_out_buf->pu1_bufs[0]);
    }

    for(i = 0; i < s_app_ctx.num_disp_buf; i++)
    {
        free(s_app_ctx.s_disp_buffers[i].pu1_bufs[0]);
    }

    free(ps_out_buf);
    free(pu1_bs_buf);

    if(s_app_ctx.display_thread_handle)
        free(s_app_ctx.display_thread_handle);

    return (0);
}
