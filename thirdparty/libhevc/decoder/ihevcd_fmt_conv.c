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
/**
*******************************************************************************
* @file
*  ihevcd_fmt_conv.c
*
* @brief
*  Contains functions for format conversion or frame copy of output buffer
*
* @author
*  Harish
*
* @par List of Functions:
*
* @remarks
*  None
*
*******************************************************************************
*/
/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ihevc_typedefs.h"
#include "iv.h"
#include "ivd.h"
#include "ihevcd_cxa.h"
#include "ithread.h"

#include "ihevc_defs.h"
#include "ihevc_debug.h"
#include "ihevc_structs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_cabac_tables.h"
#include "ihevc_disp_mgr.h"

#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_nal.h"
#include "ihevcd_bitstream.h"
#include "ihevcd_fmt_conv.h"
#include "ihevcd_profile.h"

/**
*******************************************************************************
*
* @brief Function used from copying a 420SP buffer
*
* @par   Description
* Function used from copying a 420SP buffer
*
* @param[in] pu1_y_src
*   Input Y pointer
*
* @param[in] pu1_uv_src
*   Input UV pointer (UV is interleaved either in UV or VU format)
*
* @param[in] pu1_y_dst
*   Output Y pointer
*
* @param[in] pu1_uv_dst
*   Output UV pointer (UV is interleaved in the same format as that of input)
*
* @param[in] wd
*   Width
*
* @param[in] ht
*   Height
*
* @param[in] src_y_strd
*   Input Y Stride
*
* @param[in] src_uv_strd
*   Input UV stride
*
* @param[in] dst_y_strd
*   Output Y stride
*
* @param[in] dst_uv_strd
*   Output UV stride
*
* @returns None
*
* @remarks In case there is a need to perform partial frame copy then
* by passion appropriate source and destination pointers and appropriate
* values for wd and ht it can be done
*
*******************************************************************************
*/
void ihevcd_fmt_conv_420sp_to_rgb565(UWORD8 *pu1_y_src,
                                     UWORD8 *pu1_uv_src,
                                     UWORD16 *pu2_rgb_dst,
                                     WORD32 wd,
                                     WORD32 ht,
                                     WORD32 src_y_strd,
                                     WORD32 src_uv_strd,
                                     WORD32 dst_strd,
                                     WORD32 is_u_first)
{


    WORD16  i2_r, i2_g, i2_b;
    UWORD32  u4_r, u4_g, u4_b;
    WORD16  i2_i, i2_j;
    UWORD8  *pu1_y_src_nxt;
    UWORD16 *pu2_rgb_dst_NextRow;

    UWORD8 *pu1_u_src, *pu1_v_src;

    if(is_u_first)
    {
        pu1_u_src = (UWORD8 *)pu1_uv_src;
        pu1_v_src = (UWORD8 *)pu1_uv_src + 1;
    }
    else
    {
        pu1_u_src = (UWORD8 *)pu1_uv_src + 1;
        pu1_v_src = (UWORD8 *)pu1_uv_src;
    }

    pu1_y_src_nxt   = pu1_y_src + src_y_strd;
    pu2_rgb_dst_NextRow = pu2_rgb_dst + dst_strd;

    for(i2_i = 0; i2_i < (ht >> 1); i2_i++)
    {
        for(i2_j = (wd >> 1); i2_j > 0; i2_j--)
        {
            i2_b = ((*pu1_u_src - 128) * COEFF4 >> 13);
            i2_g = ((*pu1_u_src - 128) * COEFF2 + (*pu1_v_src - 128) * COEFF3) >> 13;
            i2_r = ((*pu1_v_src - 128) * COEFF1) >> 13;

            pu1_u_src += 2;
            pu1_v_src += 2;
            /* pixel 0 */
            /* B */
            u4_b = CLIP_U8(*pu1_y_src + i2_b);
            u4_b >>= 3;
            /* G */
            u4_g = CLIP_U8(*pu1_y_src + i2_g);
            u4_g >>= 2;
            /* R */
            u4_r = CLIP_U8(*pu1_y_src + i2_r);
            u4_r >>= 3;

            pu1_y_src++;
            *pu2_rgb_dst++ = ((u4_r << 11) | (u4_g << 5) | u4_b);

            /* pixel 1 */
            /* B */
            u4_b = CLIP_U8(*pu1_y_src + i2_b);
            u4_b >>= 3;
            /* G */
            u4_g = CLIP_U8(*pu1_y_src + i2_g);
            u4_g >>= 2;
            /* R */
            u4_r = CLIP_U8(*pu1_y_src + i2_r);
            u4_r >>= 3;

            pu1_y_src++;
            *pu2_rgb_dst++ = ((u4_r << 11) | (u4_g << 5) | u4_b);

            /* pixel 2 */
            /* B */
            u4_b = CLIP_U8(*pu1_y_src_nxt + i2_b);
            u4_b >>= 3;
            /* G */
            u4_g = CLIP_U8(*pu1_y_src_nxt + i2_g);
            u4_g >>= 2;
            /* R */
            u4_r = CLIP_U8(*pu1_y_src_nxt + i2_r);
            u4_r >>= 3;

            pu1_y_src_nxt++;
            *pu2_rgb_dst_NextRow++ = ((u4_r << 11) | (u4_g << 5) | u4_b);

            /* pixel 3 */
            /* B */
            u4_b = CLIP_U8(*pu1_y_src_nxt + i2_b);
            u4_b >>= 3;
            /* G */
            u4_g = CLIP_U8(*pu1_y_src_nxt + i2_g);
            u4_g >>= 2;
            /* R */
            u4_r = CLIP_U8(*pu1_y_src_nxt + i2_r);
            u4_r >>= 3;

            pu1_y_src_nxt++;
            *pu2_rgb_dst_NextRow++ = ((u4_r << 11) | (u4_g << 5) | u4_b);

        }

        pu1_u_src = pu1_u_src + src_uv_strd - wd;
        pu1_v_src = pu1_v_src + src_uv_strd - wd;

        pu1_y_src = pu1_y_src + (src_y_strd << 1) - wd;
        pu1_y_src_nxt = pu1_y_src_nxt + (src_y_strd << 1) - wd;

        pu2_rgb_dst = pu2_rgb_dst_NextRow - wd + dst_strd;
        pu2_rgb_dst_NextRow = pu2_rgb_dst_NextRow + (dst_strd << 1) - wd;
    }


}

void ihevcd_fmt_conv_420sp_to_rgba8888(UWORD8 *pu1_y_src,
                                       UWORD8 *pu1_uv_src,
                                       UWORD32 *pu4_rgba_dst,
                                       WORD32 wd,
                                       WORD32 ht,
                                       WORD32 src_y_strd,
                                       WORD32 src_uv_strd,
                                       WORD32 dst_strd,
                                       WORD32 is_u_first)
{


    WORD16  i2_r, i2_g, i2_b;
    UWORD32  u4_r, u4_g, u4_b;
    WORD16  i2_i, i2_j;
    UWORD8  *pu1_y_src_nxt;
    UWORD32 *pu4_rgba_dst_NextRow;

    UWORD8 *pu1_u_src, *pu1_v_src;

    if(is_u_first)
    {
        pu1_u_src = (UWORD8 *)pu1_uv_src;
        pu1_v_src = (UWORD8 *)pu1_uv_src + 1;
    }
    else
    {
        pu1_u_src = (UWORD8 *)pu1_uv_src + 1;
        pu1_v_src = (UWORD8 *)pu1_uv_src;
    }

    pu1_y_src_nxt   = pu1_y_src + src_y_strd;
    pu4_rgba_dst_NextRow = pu4_rgba_dst + dst_strd;

    for(i2_i = 0; i2_i < (ht >> 1); i2_i++)
    {
        for(i2_j = (wd >> 1); i2_j > 0; i2_j--)
        {
            i2_b = ((*pu1_u_src - 128) * COEFF4 >> 13);
            i2_g = ((*pu1_u_src - 128) * COEFF2 + (*pu1_v_src - 128) * COEFF3) >> 13;
            i2_r = ((*pu1_v_src - 128) * COEFF1) >> 13;

            pu1_u_src += 2;
            pu1_v_src += 2;
            /* pixel 0 */
            /* B */
            u4_b = CLIP_U8(*pu1_y_src + i2_b);
            /* G */
            u4_g = CLIP_U8(*pu1_y_src + i2_g);
            /* R */
            u4_r = CLIP_U8(*pu1_y_src + i2_r);

            pu1_y_src++;
            *pu4_rgba_dst++ = ((u4_r << 16) | (u4_g << 8) | (u4_b << 0));

            /* pixel 1 */
            /* B */
            u4_b = CLIP_U8(*pu1_y_src + i2_b);
            /* G */
            u4_g = CLIP_U8(*pu1_y_src + i2_g);
            /* R */
            u4_r = CLIP_U8(*pu1_y_src + i2_r);

            pu1_y_src++;
            *pu4_rgba_dst++ = ((u4_r << 16) | (u4_g << 8) | (u4_b << 0));

            /* pixel 2 */
            /* B */
            u4_b = CLIP_U8(*pu1_y_src_nxt + i2_b);
            /* G */
            u4_g = CLIP_U8(*pu1_y_src_nxt + i2_g);
            /* R */
            u4_r = CLIP_U8(*pu1_y_src_nxt + i2_r);

            pu1_y_src_nxt++;
            *pu4_rgba_dst_NextRow++ = ((u4_r << 16) | (u4_g << 8) | (u4_b << 0));

            /* pixel 3 */
            /* B */
            u4_b = CLIP_U8(*pu1_y_src_nxt + i2_b);
            /* G */
            u4_g = CLIP_U8(*pu1_y_src_nxt + i2_g);
            /* R */
            u4_r = CLIP_U8(*pu1_y_src_nxt + i2_r);

            pu1_y_src_nxt++;
            *pu4_rgba_dst_NextRow++ = ((u4_r << 16) | (u4_g << 8) | (u4_b << 0));

        }

        pu1_u_src = pu1_u_src + src_uv_strd - wd;
        pu1_v_src = pu1_v_src + src_uv_strd - wd;

        pu1_y_src = pu1_y_src + (src_y_strd << 1) - wd;
        pu1_y_src_nxt = pu1_y_src_nxt + (src_y_strd << 1) - wd;

        pu4_rgba_dst = pu4_rgba_dst_NextRow - wd + dst_strd;
        pu4_rgba_dst_NextRow = pu4_rgba_dst_NextRow + (dst_strd << 1) - wd;
    }


}

/**
*******************************************************************************
*
* @brief Function used from copying a 420SP buffer
*
* @par   Description
* Function used from copying a 420SP buffer
*
* @param[in] pu1_y_src
*   Input Y pointer
*
* @param[in] pu1_uv_src
*   Input UV pointer (UV is interleaved either in UV or VU format)
*
* @param[in] pu1_y_dst
*   Output Y pointer
*
* @param[in] pu1_uv_dst
*   Output UV pointer (UV is interleaved in the same format as that of input)
*
* @param[in] wd
*   Width
*
* @param[in] ht
*   Height
*
* @param[in] src_y_strd
*   Input Y Stride
*
* @param[in] src_uv_strd
*   Input UV stride
*
* @param[in] dst_y_strd
*   Output Y stride
*
* @param[in] dst_uv_strd
*   Output UV stride
*
* @returns None
*
* @remarks In case there is a need to perform partial frame copy then
* by passion appropriate source and destination pointers and appropriate
* values for wd and ht it can be done
*
*******************************************************************************
*/

void ihevcd_fmt_conv_420sp_to_420sp(UWORD8 *pu1_y_src,
                                    UWORD8 *pu1_uv_src,
                                    UWORD8 *pu1_y_dst,
                                    UWORD8 *pu1_uv_dst,
                                    WORD32 wd,
                                    WORD32 ht,
                                    WORD32 src_y_strd,
                                    WORD32 src_uv_strd,
                                    WORD32 dst_y_strd,
                                    WORD32 dst_uv_strd)
{
    UWORD8 *pu1_src, *pu1_dst;
    WORD32 num_rows, num_cols, src_strd, dst_strd;
    WORD32 i;

    /* copy luma */
    pu1_src = (UWORD8 *)pu1_y_src;
    pu1_dst = (UWORD8 *)pu1_y_dst;

    num_rows = ht;
    num_cols = wd;

    src_strd = src_y_strd;
    dst_strd = dst_y_strd;

    for(i = 0; i < num_rows; i++)
    {
        memcpy(pu1_dst, pu1_src, num_cols);
        pu1_dst += dst_strd;
        pu1_src += src_strd;
    }

    /* copy U and V */
    pu1_src = (UWORD8 *)pu1_uv_src;
    pu1_dst = (UWORD8 *)pu1_uv_dst;

    num_rows = ht >> 1;
    num_cols = wd;

    src_strd = src_uv_strd;
    dst_strd = dst_uv_strd;

    for(i = 0; i < num_rows; i++)
    {
        memcpy(pu1_dst, pu1_src, num_cols);
        pu1_dst += dst_strd;
        pu1_src += src_strd;
    }
    return;
}



/**
*******************************************************************************
*
* @brief Function used from copying a 420SP buffer
*
* @par   Description
* Function used from copying a 420SP buffer
*
* @param[in] pu1_y_src
*   Input Y pointer
*
* @param[in] pu1_uv_src
*   Input UV pointer (UV is interleaved either in UV or VU format)
*
* @param[in] pu1_y_dst
*   Output Y pointer
*
* @param[in] pu1_uv_dst
*   Output UV pointer (UV is interleaved in the same format as that of input)
*
* @param[in] wd
*   Width
*
* @param[in] ht
*   Height
*
* @param[in] src_y_strd
*   Input Y Stride
*
* @param[in] src_uv_strd
*   Input UV stride
*
* @param[in] dst_y_strd
*   Output Y stride
*
* @param[in] dst_uv_strd
*   Output UV stride
*
* @returns None
*
* @remarks In case there is a need to perform partial frame copy then
* by passion appropriate source and destination pointers and appropriate
* values for wd and ht it can be done
*
*******************************************************************************
*/
void ihevcd_fmt_conv_420sp_to_420sp_swap_uv(UWORD8 *pu1_y_src,
                                            UWORD8 *pu1_uv_src,
                                            UWORD8 *pu1_y_dst,
                                            UWORD8 *pu1_uv_dst,
                                            WORD32 wd,
                                            WORD32 ht,
                                            WORD32 src_y_strd,
                                            WORD32 src_uv_strd,
                                            WORD32 dst_y_strd,
                                            WORD32 dst_uv_strd)
{
    UWORD8 *pu1_src, *pu1_dst;
    WORD32 num_rows, num_cols, src_strd, dst_strd;
    WORD32 i;

    /* copy luma */
    pu1_src = (UWORD8 *)pu1_y_src;
    pu1_dst = (UWORD8 *)pu1_y_dst;

    num_rows = ht;
    num_cols = wd;

    src_strd = src_y_strd;
    dst_strd = dst_y_strd;

    for(i = 0; i < num_rows; i++)
    {
        memcpy(pu1_dst, pu1_src, num_cols);
        pu1_dst += dst_strd;
        pu1_src += src_strd;
    }

    /* copy U and V */
    pu1_src = (UWORD8 *)pu1_uv_src;
    pu1_dst = (UWORD8 *)pu1_uv_dst;

    num_rows = ht >> 1;
    num_cols = wd;

    src_strd = src_uv_strd;
    dst_strd = dst_uv_strd;

    for(i = 0; i < num_rows; i++)
    {
        WORD32 j;
        for(j = 0; j < num_cols; j += 2)
        {
            pu1_dst[j + 0] = pu1_src[j + 1];
            pu1_dst[j + 1] = pu1_src[j + 0];
        }
        pu1_dst += dst_strd;
        pu1_src += src_strd;
    }
    return;
}
/**
*******************************************************************************
*
* @brief Function used from copying a 420SP buffer
*
* @par   Description
* Function used from copying a 420SP buffer
*
* @param[in] pu1_y_src
*   Input Y pointer
*
* @param[in] pu1_uv_src
*   Input UV pointer (UV is interleaved either in UV or VU format)
*
* @param[in] pu1_y_dst
*   Output Y pointer
*
* @param[in] pu1_u_dst
*   Output U pointer
*
* @param[in] pu1_v_dst
*   Output V pointer
*
* @param[in] wd
*   Width
*
* @param[in] ht
*   Height
*
* @param[in] src_y_strd
*   Input Y Stride
*
* @param[in] src_uv_strd
*   Input UV stride
*
* @param[in] dst_y_strd
*   Output Y stride
*
* @param[in] dst_uv_strd
*   Output UV stride
*
* @param[in] is_u_first
*   Flag to indicate if U is the first byte in input chroma part
*
* @returns none
*
* @remarks In case there is a need to perform partial frame copy then
* by passion appropriate source and destination pointers and appropriate
* values for wd and ht it can be done
*
*******************************************************************************
*/


void ihevcd_fmt_conv_420sp_to_420p(UWORD8 *pu1_y_src,
                                   UWORD8 *pu1_uv_src,
                                   UWORD8 *pu1_y_dst,
                                   UWORD8 *pu1_u_dst,
                                   UWORD8 *pu1_v_dst,
                                   WORD32 wd,
                                   WORD32 ht,
                                   WORD32 src_y_strd,
                                   WORD32 src_uv_strd,
                                   WORD32 dst_y_strd,
                                   WORD32 dst_uv_strd,
                                   WORD32 is_u_first,
                                   WORD32 disable_luma_copy)
{
    UWORD8 *pu1_src, *pu1_dst;
    UWORD8 *pu1_u_src, *pu1_v_src;
    WORD32 num_rows, num_cols, src_strd, dst_strd;
    WORD32 i, j;

    if(0 == disable_luma_copy)
    {
        /* copy luma */
        pu1_src = (UWORD8 *)pu1_y_src;
        pu1_dst = (UWORD8 *)pu1_y_dst;

        num_rows = ht;
        num_cols = wd;

        src_strd = src_y_strd;
        dst_strd = dst_y_strd;

        for(i = 0; i < num_rows; i++)
        {
            memcpy(pu1_dst, pu1_src, num_cols);
            pu1_dst += dst_strd;
            pu1_src += src_strd;
        }
    }
    /* de-interleave U and V and copy to destination */
    if(is_u_first)
    {
        pu1_u_src = (UWORD8 *)pu1_uv_src;
        pu1_v_src = (UWORD8 *)pu1_uv_src + 1;
    }
    else
    {
        pu1_u_src = (UWORD8 *)pu1_uv_src + 1;
        pu1_v_src = (UWORD8 *)pu1_uv_src;
    }


    num_rows = ht >> 1;
    num_cols = wd >> 1;

    src_strd = src_uv_strd;
    dst_strd = dst_uv_strd;

    for(i = 0; i < num_rows; i++)
    {
        for(j = 0; j < num_cols; j++)
        {
            pu1_u_dst[j] = pu1_u_src[j * 2];
            pu1_v_dst[j] = pu1_v_src[j * 2];
        }

        pu1_u_dst += dst_strd;
        pu1_v_dst += dst_strd;
        pu1_u_src += src_strd;
        pu1_v_src += src_strd;
    }
    return;
}



/**
*******************************************************************************
*
* @brief Function used from format conversion or frame copy
*
* @par   Description
* Function used from copying or converting a reference frame to display buffer
* in non shared mode
*
* @param[in] pu1_y_dst
*   Output Y pointer
*
* @param[in] pu1_u_dst
*   Output U/UV pointer ( UV is interleaved in the same format as that of input)
*
* @param[in] pu1_v_dst
*   Output V pointer ( used in 420P output case)
*
* @param[in] blocking
*   To indicate whether format conversion should wait till frame is reconstructed
*   and then return after complete copy is done. To be set to 1 when called at the
*   end of frame processing and set to 0 when called between frame processing modules
*   in order to utilize available MCPS
*
* @returns Error from IHEVCD_ERROR_T
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_fmt_conv(codec_t *ps_codec,
                               process_ctxt_t *ps_proc,
                               UWORD8 *pu1_y_dst,
                               UWORD8 *pu1_u_dst,
                               UWORD8 *pu1_v_dst,
                               WORD32 cur_row,
                               WORD32 num_rows)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    pic_buf_t *ps_disp_pic;
    UWORD8 *pu1_y_src, *pu1_uv_src;
    UWORD8 *pu1_y_dst_tmp, *pu1_uv_dst_tmp;
    UWORD8 *pu1_u_dst_tmp, *pu1_v_dst_tmp;
    UWORD16 *pu2_rgb_dst_tmp;
    UWORD32 *pu4_rgb_dst_tmp;
    WORD32 is_u_first;
    UWORD8 *pu1_luma;
    UWORD8 *pu1_chroma;
    sps_t *ps_sps;
    WORD32 disable_luma_copy;
    WORD32 crop_unit_x, crop_unit_y;

    if(0 == num_rows)
        return ret;

    /* In case processing is disabled, then no need to format convert/copy */
    PROFILE_DISABLE_FMT_CONV();
    ps_sps = ps_proc->ps_sps;

    crop_unit_x = 1;
    crop_unit_y = 1;

    if(CHROMA_FMT_IDC_YUV420 == ps_sps->i1_chroma_format_idc)
    {
        crop_unit_x = 2;
        crop_unit_y = 2;
    }

    ps_disp_pic = ps_codec->ps_disp_buf;
    pu1_luma = ps_disp_pic->pu1_luma;
    pu1_chroma = ps_disp_pic->pu1_chroma;


    /* Take care of cropping */
    pu1_luma    += ps_codec->i4_strd * ps_sps->i2_pic_crop_top_offset * crop_unit_y + ps_sps->i2_pic_crop_left_offset * crop_unit_x;

    /* Left offset is multiplied by 2 because buffer is UV interleaved */
    pu1_chroma  += ps_codec->i4_strd * ps_sps->i2_pic_crop_top_offset + ps_sps->i2_pic_crop_left_offset * 2;


    is_u_first = (IV_YUV_420SP_UV == ps_codec->e_ref_chroma_fmt) ? 1 : 0;

    /* In case of 420P output luma copy is disabled for shared mode */
    disable_luma_copy = 0;
    if(1 == ps_codec->i4_share_disp_buf)
    {
        disable_luma_copy = 1;
    }



    {
        pu1_y_src   = pu1_luma + cur_row * ps_codec->i4_strd;
        pu1_uv_src  = pu1_chroma + (cur_row / 2) * ps_codec->i4_strd;

        /* In case of shared mode, with 420P output, get chroma destination */
        if((1 == ps_codec->i4_share_disp_buf) && (IV_YUV_420P == ps_codec->e_chroma_fmt))
        {
            WORD32 i;
            for(i = 0; i < ps_codec->i4_share_disp_buf_cnt; i++)
            {
                WORD32 diff = ps_disp_pic->pu1_luma - ps_codec->s_disp_buffer[i].pu1_bufs[0];
                if(diff == (ps_codec->i4_strd * PAD_TOP + PAD_LEFT))
                {
                    pu1_u_dst = ps_codec->s_disp_buffer[i].pu1_bufs[1];
                    pu1_u_dst += (ps_codec->i4_strd * PAD_TOP) / 4 + (PAD_LEFT / 2);

                    pu1_v_dst = ps_codec->s_disp_buffer[i].pu1_bufs[2];
                    pu1_v_dst += (ps_codec->i4_strd * PAD_TOP) / 4 + (PAD_LEFT / 2);
                    break;
                }
            }
        }
        pu2_rgb_dst_tmp  = (UWORD16 *)pu1_y_dst;
        pu2_rgb_dst_tmp  += cur_row * ps_codec->i4_disp_strd;
        pu4_rgb_dst_tmp  = (UWORD32 *)pu1_y_dst;
        pu4_rgb_dst_tmp  += cur_row * ps_codec->i4_disp_strd;
        pu1_y_dst_tmp  = pu1_y_dst  + cur_row * ps_codec->i4_disp_strd;
        pu1_uv_dst_tmp = pu1_u_dst  + (cur_row / 2) * ps_codec->i4_disp_strd;
        pu1_u_dst_tmp = pu1_u_dst  + (cur_row / 2) * ps_codec->i4_disp_strd / 2;
        pu1_v_dst_tmp = pu1_v_dst  + (cur_row / 2) * ps_codec->i4_disp_strd / 2;

        /* In case of multi threaded implementation, format conversion might be called
         * before reconstruction is completed. If the frame being converted/copied
         * is same as the frame being reconstructed,
         * Check how many rows can be format converted
         * Convert those many rows and then check for remaining rows and so on
         */

        if((0 == ps_codec->i4_flush_mode) && (ps_codec->i4_disp_buf_id == ps_proc->i4_cur_pic_buf_id) && (1 < ps_codec->i4_num_cores))
        {
            WORD32 idx;
            UWORD8 *pu1_buf;
            WORD32 status;
            WORD32 last_row = cur_row + num_rows;
            WORD32 last_ctb_y;
            UWORD32 ctb_in_row;

            while(1)
            {
                last_row = cur_row + MAX(num_rows, (1 << ps_sps->i1_log2_ctb_size)) +
                                ps_sps->i2_pic_crop_top_offset * crop_unit_y;
                last_ctb_y = (last_row >> ps_sps->i1_log2_ctb_size) - 1;
                /* Since deblocking works with a shift of -4, -4 ,wait till next CTB row is processed */
                last_ctb_y++;
                /* In case of a  conformance window, an extra wait of one row might be needed */
                last_ctb_y++;
                last_ctb_y = MIN(last_ctb_y, (ps_sps->i2_pic_ht_in_ctb - 1));

                idx = (last_ctb_y * ps_sps->i2_pic_wd_in_ctb);

                /*Check if the row below is completely processed before proceeding with format conversion*/
                status = 1;
                for(ctb_in_row = 0; (WORD32)ctb_in_row < ps_sps->i2_pic_wd_in_ctb; ctb_in_row++)
                {
                    pu1_buf = (ps_codec->pu1_proc_map + idx + ctb_in_row);
                    status &= *pu1_buf;
                }

                if(status)
                {
                    break;
                }
                else
                {
                    ithread_yield();
                }
            }
        }


        if((IV_YUV_420SP_UV == ps_codec->e_chroma_fmt) || (IV_YUV_420SP_VU == ps_codec->e_chroma_fmt))
        {

            ps_codec->s_func_selector.ihevcd_fmt_conv_420sp_to_420sp_fptr(pu1_y_src, pu1_uv_src,
                                                                          pu1_y_dst_tmp, pu1_uv_dst_tmp,
                                                                          ps_codec->i4_disp_wd,
                                                                          num_rows,
                                                                          ps_codec->i4_strd,
                                                                          ps_codec->i4_strd,
                                                                          ps_codec->i4_disp_strd,
                                                                          ps_codec->i4_disp_strd);
        }
        else if(IV_YUV_420P == ps_codec->e_chroma_fmt)
        {

            if(0 == disable_luma_copy)
            {
                // copy luma
                WORD32 i;
                WORD32 num_cols = ps_codec->i4_disp_wd;

                for(i = 0; i < num_rows; i++)
                {
                    memcpy(pu1_y_dst_tmp, pu1_y_src, num_cols);
                    pu1_y_dst_tmp += ps_codec->i4_disp_strd;
                    pu1_y_src += ps_codec->i4_strd;
                }

                disable_luma_copy = 1;
            }

            ps_codec->s_func_selector.ihevcd_fmt_conv_420sp_to_420p_fptr(pu1_y_src, pu1_uv_src,
                                                                         pu1_y_dst_tmp, pu1_u_dst_tmp, pu1_v_dst_tmp,
                                                                         ps_codec->i4_disp_wd,
                                                                         num_rows,
                                                                         ps_codec->i4_strd,
                                                                         ps_codec->i4_strd,
                                                                         ps_codec->i4_disp_strd,
                                                                         (ps_codec->i4_disp_strd / 2),
                                                                         is_u_first,
                                                                         disable_luma_copy);

        }
        else if(IV_RGB_565 == ps_codec->e_chroma_fmt)
        {

            ps_codec->s_func_selector.ihevcd_fmt_conv_420sp_to_rgb565_fptr(pu1_y_src, pu1_uv_src,
                                                                           pu2_rgb_dst_tmp,
                                                                           ps_codec->i4_disp_wd,
                                                                           num_rows,
                                                                           ps_codec->i4_strd,
                                                                           ps_codec->i4_strd,
                                                                           ps_codec->i4_disp_strd,
                                                                           is_u_first);

        }
        else if(IV_RGBA_8888 == ps_codec->e_chroma_fmt)
        {
            ASSERT(is_u_first == 1);

            ps_codec->s_func_selector.ihevcd_fmt_conv_420sp_to_rgba8888_fptr(pu1_y_src,
                                                                             pu1_uv_src,
                                                                             pu4_rgb_dst_tmp,
                                                                             ps_codec->i4_disp_wd,
                                                                             num_rows,
                                                                             ps_codec->i4_strd,
                                                                             ps_codec->i4_strd,
                                                                             ps_codec->i4_disp_strd,
                                                                             is_u_first);

        }



    }
    return (ret);
}

