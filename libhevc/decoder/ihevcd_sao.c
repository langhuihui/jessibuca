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
 *  ihevc_sao.c
 *
 * @brief
 *  Contains function definitions for sample adaptive offset process
 *
 * @author
 *  Srinivas T
 *
 * @par List of Functions:
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

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
#include "ihevc_defs.h"
#include "ihevc_structs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_cabac_tables.h"
#include "ihevc_sao.h"
#include "ihevc_mem_fns.h"

#include "ihevc_error.h"
#include "ihevc_common_tables.h"

#include "ihevcd_trace.h"
#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_nal.h"
#include "ihevcd_bitstream.h"
#include "ihevcd_job_queue.h"
#include "ihevcd_utils.h"

#include "ihevc_deblk.h"
#include "ihevc_deblk_tables.h"
#include "ihevcd_profile.h"
#include "ihevcd_sao.h"
#include "ihevcd_debug.h"

#define SAO_SHIFT_CTB    8

/**
 * SAO at CTB level is implemented for a shifted CTB(8 pixels in x and y directions)
 */
void ihevcd_sao_ctb(sao_ctxt_t *ps_sao_ctxt)
{
    codec_t *ps_codec = ps_sao_ctxt->ps_codec;
    UWORD8 *pu1_src_luma;
    UWORD8 *pu1_src_chroma;
    WORD32 src_strd;
    WORD32 ctb_size;
    WORD32 log2_ctb_size;
    sps_t *ps_sps;
    sao_t *ps_sao;
    WORD32 row, col;
    UWORD8 au1_avail_luma[8];
    UWORD8 au1_avail_chroma[8];
    WORD32 i;
    UWORD8 *pu1_src_top_luma;
    UWORD8 *pu1_src_top_chroma;
    UWORD8 *pu1_src_left_luma;
    UWORD8 *pu1_src_left_chroma;
    UWORD8 au1_src_top_right[2];
    UWORD8 au1_src_bot_left[2];
    UWORD8 *pu1_no_loop_filter_flag;
    WORD32 loop_filter_strd;

    WORD8 ai1_offset_y[5];
    WORD8 ai1_offset_cb[5];
    WORD8 ai1_offset_cr[5];

    PROFILE_DISABLE_SAO();

    ai1_offset_y[0] = 0;
    ai1_offset_cb[0] = 0;
    ai1_offset_cr[0] = 0;

    ps_sps = ps_sao_ctxt->ps_sps;
    log2_ctb_size = ps_sps->i1_log2_ctb_size;
    ctb_size = (1 << log2_ctb_size);
    src_strd = ps_sao_ctxt->ps_codec->i4_strd;
    pu1_src_luma = ps_sao_ctxt->pu1_cur_pic_luma + ((ps_sao_ctxt->i4_ctb_x + ps_sao_ctxt->i4_ctb_y * ps_sao_ctxt->ps_codec->i4_strd) << (log2_ctb_size));
    pu1_src_chroma = ps_sao_ctxt->pu1_cur_pic_chroma + ((ps_sao_ctxt->i4_ctb_x + ps_sao_ctxt->i4_ctb_y * ps_sao_ctxt->ps_codec->i4_strd / 2) << (log2_ctb_size));

    ps_sao = ps_sao_ctxt->ps_pic_sao + ps_sao_ctxt->i4_ctb_x + ps_sao_ctxt->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb;
    loop_filter_strd =  (ps_sps->i2_pic_width_in_luma_samples + 63) / 64;

    /* Current CTB */
    {
        WORD32 sao_wd_luma;
        WORD32 sao_wd_chroma;
        WORD32 sao_ht_luma;
        WORD32 sao_ht_chroma;

        WORD32 remaining_rows;
        WORD32 remaining_cols;

        remaining_cols = ps_sps->i2_pic_width_in_luma_samples - (ps_sao_ctxt->i4_ctb_x << log2_ctb_size);
        sao_wd_luma = MIN(ctb_size, remaining_cols);
        sao_wd_chroma = MIN(ctb_size, remaining_cols);

        remaining_rows = ps_sps->i2_pic_height_in_luma_samples - (ps_sao_ctxt->i4_ctb_y << log2_ctb_size);
        sao_ht_luma = MIN(ctb_size, remaining_rows);
        sao_ht_chroma = MIN(ctb_size, remaining_rows) / 2;

        pu1_src_top_luma = ps_sao_ctxt->pu1_sao_src_top_luma + (ps_sao_ctxt->i4_ctb_x << log2_ctb_size);
        pu1_src_top_chroma = ps_sao_ctxt->pu1_sao_src_top_chroma + (ps_sao_ctxt->i4_ctb_x << log2_ctb_size);
        pu1_src_left_luma = ps_sao_ctxt->pu1_sao_src_left_luma + (ps_sao_ctxt->i4_ctb_y << log2_ctb_size);
        pu1_src_left_chroma = ps_sao_ctxt->pu1_sao_src_left_chroma + (ps_sao_ctxt->i4_ctb_y << log2_ctb_size);

        pu1_no_loop_filter_flag = ps_sao_ctxt->pu1_pic_no_loop_filter_flag +
                        ((ps_sao_ctxt->i4_ctb_y * ctb_size) / 8) * loop_filter_strd +
                        ((ps_sao_ctxt->i4_ctb_x * ctb_size) / 64);

        ai1_offset_y[1] = ps_sao->b4_y_offset_1;
        ai1_offset_y[2] = ps_sao->b4_y_offset_2;
        ai1_offset_y[3] = ps_sao->b4_y_offset_3;
        ai1_offset_y[4] = ps_sao->b4_y_offset_4;

        ai1_offset_cb[1] = ps_sao->b4_cb_offset_1;
        ai1_offset_cb[2] = ps_sao->b4_cb_offset_2;
        ai1_offset_cb[3] = ps_sao->b4_cb_offset_3;
        ai1_offset_cb[4] = ps_sao->b4_cb_offset_4;

        ai1_offset_cr[1] = ps_sao->b4_cr_offset_1;
        ai1_offset_cr[2] = ps_sao->b4_cr_offset_2;
        ai1_offset_cr[3] = ps_sao->b4_cr_offset_3;
        ai1_offset_cr[4] = ps_sao->b4_cr_offset_4;

        for(i = 0; i < 8; i++)
        {
            au1_avail_luma[i] = 255;
            au1_avail_chroma[i] = 255;
        }


        if(0 == ps_sao_ctxt->i4_ctb_x)
        {
            au1_avail_luma[0] = 0;
            au1_avail_luma[4] = 0;
            au1_avail_luma[6] = 0;

            au1_avail_chroma[0] = 0;
            au1_avail_chroma[4] = 0;
            au1_avail_chroma[6] = 0;
        }

        if(ps_sps->i2_pic_wd_in_ctb - 1 == ps_sao_ctxt->i4_ctb_x)
        {
            au1_avail_luma[1] = 0;
            au1_avail_luma[5] = 0;
            au1_avail_luma[7] = 0;

            au1_avail_chroma[1] = 0;
            au1_avail_chroma[5] = 0;
            au1_avail_chroma[7] = 0;
        }

        if(0 == ps_sao_ctxt->i4_ctb_y)
        {
            au1_avail_luma[2] = 0;
            au1_avail_luma[4] = 0;
            au1_avail_luma[5] = 0;

            au1_avail_chroma[2] = 0;
            au1_avail_chroma[4] = 0;
            au1_avail_chroma[5] = 0;
        }

        if(ps_sps->i2_pic_ht_in_ctb - 1 == ps_sao_ctxt->i4_ctb_y)
        {
            au1_avail_luma[3] = 0;
            au1_avail_luma[6] = 0;
            au1_avail_luma[7] = 0;

            au1_avail_chroma[3] = 0;
            au1_avail_chroma[6] = 0;
            au1_avail_chroma[7] = 0;
        }


        if(0 == ps_sao->b3_y_type_idx)
        {
            /* Update left, top and top-left */
            for(row = 0; row < sao_ht_luma; row++)
            {
                pu1_src_left_luma[row] = pu1_src_luma[row * src_strd + (sao_wd_luma - 1)];
            }
            ps_sao_ctxt->pu1_sao_src_top_left_luma_curr_ctb[0] = pu1_src_top_luma[sao_wd_luma - 1];

            ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_luma, &pu1_src_luma[(sao_ht_luma - 1) * src_strd], sao_wd_luma);

        }
        else
        {
            UWORD8 au1_src_copy[(MAX_CTB_SIZE + 2) * (MAX_CTB_SIZE + 2)];
            UWORD8 *pu1_src_copy = au1_src_copy + (MAX_CTB_SIZE + 2) + 1;
            WORD32 tmp_strd = MAX_CTB_SIZE + 2;
            WORD32 no_loop_filter_enabled = 0;

            /* Check the loop filter flags and copy the original values for back up */
            {
                UWORD32 u4_no_loop_filter_flag;
                WORD32 min_cu = 8;
                UWORD8 *pu1_src_tmp = pu1_src_luma;

                for(i = 0; i < (sao_ht_luma + min_cu - 1) / min_cu; i++)
                {
                    u4_no_loop_filter_flag = (*(UWORD32 *)(pu1_no_loop_filter_flag + i * loop_filter_strd)) >>
                                    ((((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_luma) / 8) % 8);
                    u4_no_loop_filter_flag &= (1 << ((sao_wd_luma + (min_cu - 1)) / min_cu)) - 1;

                    if(u4_no_loop_filter_flag)
                    {
                        WORD32 tmp_wd = sao_wd_luma;
                        no_loop_filter_enabled = 1;
                        while(tmp_wd > 0)
                        {
                            if(CTZ(u4_no_loop_filter_flag))
                            {
                                u4_no_loop_filter_flag  >>= (CTZ(u4_no_loop_filter_flag));
                                pu1_src_tmp += MIN((WORD32)CTZ(u4_no_loop_filter_flag), tmp_wd);
                                pu1_src_copy += MIN((WORD32)CTZ(u4_no_loop_filter_flag), tmp_wd);
                                tmp_wd -= CTZ(u4_no_loop_filter_flag) * min_cu;
                            }
                            else
                            {
                                for(row = 0; row < MIN(min_cu, sao_ht_luma - (i - 1) * min_cu); row++)
                                {
                                    for(col = 0; col < MIN((WORD32)CTZ(~u4_no_loop_filter_flag) * min_cu, tmp_wd); col++)
                                    {
                                        pu1_src_copy[row * src_strd + col] = pu1_src_tmp[row * tmp_strd + col];
                                    }
                                }

                                u4_no_loop_filter_flag  >>= (CTZ(~u4_no_loop_filter_flag));
                                pu1_src_tmp += MIN((WORD32)CTZ(~u4_no_loop_filter_flag), tmp_wd);
                                pu1_src_copy += MIN((WORD32)CTZ(~u4_no_loop_filter_flag), tmp_wd);
                                tmp_wd -= CTZ(~u4_no_loop_filter_flag) * min_cu;
                            }
                        }

                        pu1_src_tmp -= sao_wd_luma;
                    }

                    pu1_src_tmp += min_cu * src_strd;
                    pu1_src_copy += min_cu * tmp_strd;
                }
            }

            if(1 == ps_sao->b3_y_type_idx)
            {
                ps_codec->s_func_selector.ihevc_sao_band_offset_luma_fptr(pu1_src_luma,
                                                                          src_strd,
                                                                          pu1_src_left_luma,
                                                                          pu1_src_top_luma,
                                                                          ps_sao_ctxt->pu1_sao_src_top_left_luma_curr_ctb,
                                                                          ps_sao->b5_y_band_pos,
                                                                          ai1_offset_y,
                                                                          sao_wd_luma,
                                                                          sao_ht_luma);
            }
            else // if(2 <= ps_sao->b3_y_type_idx)
            {
                au1_src_top_right[0] = pu1_src_top_luma[sao_wd_luma];
                au1_src_bot_left[0] = pu1_src_luma[sao_ht_luma * src_strd - 1];
                ps_codec->apf_sao_luma[ps_sao->b3_y_type_idx - 2](pu1_src_luma,
                                                                  src_strd,
                                                                  pu1_src_left_luma,
                                                                  pu1_src_top_luma,
                                                                  ps_sao_ctxt->pu1_sao_src_top_left_luma_curr_ctb,
                                                                  au1_src_top_right,
                                                                  au1_src_bot_left,
                                                                  au1_avail_luma,
                                                                  ai1_offset_y,
                                                                  sao_wd_luma,
                                                                  sao_ht_luma);
            }

            /* Check the loop filter flags and copy the original values back if they are set */
            if(no_loop_filter_enabled)
            {
                UWORD32 u4_no_loop_filter_flag;
                WORD32 min_cu = 8;
                UWORD8 *pu1_src_tmp = pu1_src_luma;

                for(i = 0; i < (sao_ht_luma + min_cu - 1) / min_cu; i++)
                {
                    u4_no_loop_filter_flag = (*(UWORD32 *)(pu1_no_loop_filter_flag + i * loop_filter_strd)) >> ((((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_luma) / 8) % 8);
                    u4_no_loop_filter_flag &= (1 << ((sao_wd_luma + (min_cu - 1)) / min_cu)) - 1;

                    if(u4_no_loop_filter_flag)
                    {
                        WORD32 tmp_wd = sao_wd_luma;
                        while(tmp_wd > 0)
                        {
                            if(CTZ(u4_no_loop_filter_flag))
                            {
                                u4_no_loop_filter_flag  >>= (CTZ(u4_no_loop_filter_flag));
                                pu1_src_tmp += MIN((WORD32)CTZ(u4_no_loop_filter_flag), tmp_wd);
                                pu1_src_copy += MIN((WORD32)CTZ(u4_no_loop_filter_flag), tmp_wd);
                                tmp_wd -= CTZ(u4_no_loop_filter_flag) * min_cu;
                            }
                            else
                            {
                                for(row = 0; row < MIN(min_cu, sao_ht_luma - (i - 1) * min_cu); row++)
                                {
                                    for(col = 0; col < MIN((WORD32)CTZ(~u4_no_loop_filter_flag) * min_cu, tmp_wd); col++)
                                    {
                                        pu1_src_tmp[row * src_strd + col] = pu1_src_copy[row * tmp_strd + col];
                                    }
                                }

                                u4_no_loop_filter_flag  >>= (CTZ(~u4_no_loop_filter_flag));
                                pu1_src_tmp += MIN((WORD32)CTZ(~u4_no_loop_filter_flag), tmp_wd);
                                pu1_src_copy += MIN((WORD32)CTZ(~u4_no_loop_filter_flag), tmp_wd);
                                tmp_wd -= CTZ(~u4_no_loop_filter_flag) * min_cu;
                            }
                        }

                        pu1_src_tmp -= sao_wd_luma;
                    }

                    pu1_src_tmp += min_cu * src_strd;
                    pu1_src_copy += min_cu * tmp_strd;
                }
            }

        }

        if(0 == ps_sao->b3_cb_type_idx)
        {
            for(row = 0; row < sao_ht_chroma; row++)
            {
                pu1_src_left_chroma[2 * row] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 2)];
                pu1_src_left_chroma[2 * row + 1] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 1)];
            }
            ps_sao_ctxt->pu1_sao_src_top_left_chroma_curr_ctb[0] = pu1_src_top_chroma[sao_wd_chroma - 2];
            ps_sao_ctxt->pu1_sao_src_top_left_chroma_curr_ctb[1] = pu1_src_top_chroma[sao_wd_chroma - 1];

            ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_chroma, &pu1_src_chroma[(sao_ht_chroma - 1) * src_strd], sao_wd_chroma);
        }
        else
        {
            UWORD8 au1_src_copy[(MAX_CTB_SIZE + 4) * (MAX_CTB_SIZE + 2)];
            UWORD8 *pu1_src_copy = au1_src_copy + (MAX_CTB_SIZE + 4) + 2;
            WORD32 tmp_strd = MAX_CTB_SIZE + 4;
            WORD32 no_loop_filter_enabled = 0;

            /* Check the loop filter flags and copy the original values for back up */
            {
                UWORD32 u4_no_loop_filter_flag;
                WORD32 min_cu = 4;
                UWORD8 *pu1_src_tmp = pu1_src_chroma;

                for(i = 0; i < (sao_ht_chroma + min_cu - 1) / min_cu; i++)
                {
                    u4_no_loop_filter_flag = (*(UWORD32 *)(pu1_no_loop_filter_flag + i * loop_filter_strd)) >> ((((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_chroma) / 8) % 8);
                    u4_no_loop_filter_flag &= (1 << ((sao_wd_chroma + (min_cu - 1)) / min_cu)) - 1;

                    if(u4_no_loop_filter_flag)
                    {
                        WORD32 tmp_wd = sao_wd_chroma;
                        no_loop_filter_enabled = 1;
                        while(tmp_wd > 0)
                        {
                            if(CTZ(u4_no_loop_filter_flag))
                            {
                                u4_no_loop_filter_flag  >>= (CTZ(u4_no_loop_filter_flag));
                                pu1_src_tmp += MIN((WORD32)CTZ(u4_no_loop_filter_flag), tmp_wd);
                                pu1_src_copy += MIN((WORD32)CTZ(u4_no_loop_filter_flag), tmp_wd);
                                tmp_wd -= CTZ(u4_no_loop_filter_flag) * min_cu;
                            }
                            else
                            {
                                for(row = 0; row < MIN(min_cu, sao_ht_chroma - (i - 1) * min_cu); row++)
                                {
                                    for(col = 0; col < MIN((WORD32)CTZ(~u4_no_loop_filter_flag) * min_cu, tmp_wd); col++)
                                    {
                                        pu1_src_copy[row * src_strd + col] = pu1_src_tmp[row * tmp_strd + col];
                                    }
                                }

                                u4_no_loop_filter_flag  >>= (CTZ(~u4_no_loop_filter_flag));
                                pu1_src_tmp += MIN((WORD32)CTZ(~u4_no_loop_filter_flag), tmp_wd);
                                pu1_src_copy += MIN((WORD32)CTZ(~u4_no_loop_filter_flag), tmp_wd);
                                tmp_wd -= CTZ(~u4_no_loop_filter_flag) * min_cu;
                            }
                        }

                        pu1_src_tmp -= sao_wd_chroma;
                    }

                    pu1_src_tmp += min_cu * src_strd;
                    pu1_src_copy += min_cu * tmp_strd;
                }
            }

            if(1 == ps_sao->b3_cb_type_idx)
            {
                ps_codec->s_func_selector.ihevc_sao_band_offset_chroma_fptr(pu1_src_chroma,
                                                                            src_strd,
                                                                            pu1_src_left_chroma,
                                                                            pu1_src_top_chroma,
                                                                            ps_sao_ctxt->pu1_sao_src_top_left_chroma_curr_ctb,
                                                                            ps_sao->b5_cb_band_pos,
                                                                            ps_sao->b5_cr_band_pos,
                                                                            ai1_offset_cb,
                                                                            ai1_offset_cr,
                                                                            sao_wd_chroma,
                                                                            sao_ht_chroma
                                                                           );
            }
            else // if(2 <= ps_sao->b3_cb_type_idx)
            {
                au1_src_top_right[0] = pu1_src_top_chroma[sao_wd_chroma];
                au1_src_top_right[1] = pu1_src_top_chroma[sao_wd_chroma + 1];
                au1_src_bot_left[0] = pu1_src_chroma[sao_ht_chroma * src_strd - 2];
                au1_src_bot_left[1] = pu1_src_chroma[sao_ht_chroma * src_strd - 1];
                ps_codec->apf_sao_chroma[ps_sao->b3_cb_type_idx - 2](pu1_src_chroma,
                                                                     src_strd,
                                                                     pu1_src_left_chroma,
                                                                     pu1_src_top_chroma,
                                                                     ps_sao_ctxt->pu1_sao_src_top_left_chroma_curr_ctb,
                                                                     au1_src_top_right,
                                                                     au1_src_bot_left,
                                                                     au1_avail_chroma,
                                                                     ai1_offset_cb,
                                                                     ai1_offset_cr,
                                                                     sao_wd_chroma,
                                                                     sao_ht_chroma);
            }

            /* Check the loop filter flags and copy the original values back if they are set */
            if(no_loop_filter_enabled)
            {
                UWORD32 u4_no_loop_filter_flag;
                WORD32 min_cu = 4;
                UWORD8 *pu1_src_tmp = pu1_src_chroma;

                for(i = 0; i < (sao_ht_chroma + min_cu - 1) / min_cu; i++)
                {
                    u4_no_loop_filter_flag = (*(UWORD32 *)(pu1_no_loop_filter_flag + i * loop_filter_strd)) >> ((((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_chroma) / 8) % 8);
                    u4_no_loop_filter_flag &= (1 << ((sao_wd_chroma + (min_cu - 1)) / min_cu)) - 1;

                    if(u4_no_loop_filter_flag)
                    {
                        WORD32 tmp_wd = sao_wd_chroma;
                        while(tmp_wd > 0)
                        {
                            if(CTZ(u4_no_loop_filter_flag))
                            {
                                u4_no_loop_filter_flag  >>= (CTZ(u4_no_loop_filter_flag));
                                pu1_src_tmp += MIN((WORD32)CTZ(u4_no_loop_filter_flag), tmp_wd);
                                pu1_src_copy += MIN((WORD32)CTZ(u4_no_loop_filter_flag), tmp_wd);
                                tmp_wd -= CTZ(u4_no_loop_filter_flag) * min_cu;
                            }
                            else
                            {
                                for(row = 0; row < MIN(min_cu, sao_ht_chroma - (i - 1) * min_cu); row++)
                                {
                                    for(col = 0; col < MIN((WORD32)CTZ(~u4_no_loop_filter_flag) * min_cu, tmp_wd); col++)
                                    {
                                        pu1_src_tmp[row * src_strd + col] = pu1_src_copy[row * tmp_strd + col];
                                    }
                                }

                                u4_no_loop_filter_flag  >>= (CTZ(~u4_no_loop_filter_flag));
                                pu1_src_tmp += MIN((WORD32)CTZ(~u4_no_loop_filter_flag), tmp_wd);
                                pu1_src_copy += MIN((WORD32)CTZ(~u4_no_loop_filter_flag), tmp_wd);
                                tmp_wd -= CTZ(~u4_no_loop_filter_flag) * min_cu;
                            }
                        }

                        pu1_src_tmp -= sao_wd_chroma;
                    }

                    pu1_src_tmp += min_cu * src_strd;
                    pu1_src_copy += min_cu * tmp_strd;
                }
            }

        }

    }
}

void ihevcd_sao_shift_ctb(sao_ctxt_t *ps_sao_ctxt)
{
    codec_t *ps_codec = ps_sao_ctxt->ps_codec;
    UWORD8 *pu1_src_luma;
    UWORD8 *pu1_src_chroma;
    WORD32 src_strd;
    WORD32 ctb_size;
    WORD32 log2_ctb_size;
    sps_t *ps_sps;
    sao_t *ps_sao;
    pps_t *ps_pps;
    slice_header_t *ps_slice_hdr, *ps_slice_hdr_base;
    tile_t *ps_tile;
    UWORD16 *pu1_slice_idx;
    UWORD16 *pu1_tile_idx;
    WORD32 row, col;
    UWORD8 au1_avail_luma[8];
    UWORD8 au1_avail_chroma[8];
    UWORD8 au1_tile_slice_boundary[8];
    UWORD8 au4_ilf_across_tile_slice_enable[8];
    WORD32 i;
    UWORD8 *pu1_src_top_luma;
    UWORD8 *pu1_src_top_chroma;
    UWORD8 *pu1_src_left_luma;
    UWORD8 *pu1_src_left_chroma;
    UWORD8 au1_src_top_right[2];
    UWORD8 au1_src_bot_left[2];
    UWORD8 *pu1_no_loop_filter_flag;
    UWORD8 *pu1_src_backup_luma;
    UWORD8 *pu1_src_backup_chroma;
    WORD32 backup_strd;
    WORD32 loop_filter_strd;

    WORD32 no_loop_filter_enabled_luma = 0;
    WORD32 no_loop_filter_enabled_chroma = 0;
    UWORD8 *pu1_sao_src_top_left_chroma_curr_ctb;
    UWORD8 *pu1_sao_src_top_left_luma_curr_ctb;
    UWORD8 *pu1_sao_src_luma_top_left_ctb;
    UWORD8 *pu1_sao_src_chroma_top_left_ctb;
    UWORD8 *pu1_sao_src_top_left_luma_top_right;
    UWORD8 *pu1_sao_src_top_left_chroma_top_right;
    UWORD8  u1_sao_src_top_left_luma_bot_left;
    UWORD8  *pu1_sao_src_top_left_luma_bot_left;
    UWORD8 *au1_sao_src_top_left_chroma_bot_left;
    UWORD8 *pu1_sao_src_top_left_chroma_bot_left;
    /* Only 5 values are used, but arrays are large
     enough so that SIMD functions can read 64 bits at a time */
    WORD8 ai1_offset_y[8];
    WORD8 ai1_offset_cb[8];
    WORD8 ai1_offset_cr[8];
    WORD32  chroma_yuv420sp_vu = ps_sao_ctxt->is_chroma_yuv420sp_vu;

    PROFILE_DISABLE_SAO();

    ai1_offset_y[0] = 0;
    ai1_offset_cb[0] = 0;
    ai1_offset_cr[0] = 0;

    ps_sps = ps_sao_ctxt->ps_sps;
    ps_pps = ps_sao_ctxt->ps_pps;
    ps_tile = ps_sao_ctxt->ps_tile;

    log2_ctb_size = ps_sps->i1_log2_ctb_size;
    ctb_size = (1 << log2_ctb_size);
    src_strd = ps_sao_ctxt->ps_codec->i4_strd;
    ps_slice_hdr_base = ps_sao_ctxt->ps_codec->ps_slice_hdr_base;
    ps_slice_hdr = ps_slice_hdr_base + (ps_sao_ctxt->i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1));

    pu1_slice_idx = ps_sao_ctxt->pu1_slice_idx;
    pu1_tile_idx = ps_sao_ctxt->pu1_tile_idx;
    pu1_src_luma = ps_sao_ctxt->pu1_cur_pic_luma + ((ps_sao_ctxt->i4_ctb_x + ps_sao_ctxt->i4_ctb_y * ps_sao_ctxt->ps_codec->i4_strd) << (log2_ctb_size));
    pu1_src_chroma = ps_sao_ctxt->pu1_cur_pic_chroma + ((ps_sao_ctxt->i4_ctb_x + ps_sao_ctxt->i4_ctb_y * ps_sao_ctxt->ps_codec->i4_strd / 2) << (log2_ctb_size));

    /*Stores the left value for each row ctbs- Needed for column tiles*/
    pu1_sao_src_top_left_luma_curr_ctb = ps_sao_ctxt->pu1_sao_src_top_left_luma_curr_ctb + ((ps_sao_ctxt->i4_ctb_y));
    pu1_sao_src_top_left_chroma_curr_ctb = ps_sao_ctxt->pu1_sao_src_top_left_chroma_curr_ctb + (2 * (ps_sao_ctxt->i4_ctb_y));
    pu1_sao_src_luma_top_left_ctb = ps_sao_ctxt->pu1_sao_src_luma_top_left_ctb + ((ps_sao_ctxt->i4_ctb_y));
    pu1_sao_src_chroma_top_left_ctb = ps_sao_ctxt->pu1_sao_src_chroma_top_left_ctb + (2 * ps_sao_ctxt->i4_ctb_y);
    u1_sao_src_top_left_luma_bot_left = ps_sao_ctxt->u1_sao_src_top_left_luma_bot_left; // + ((ps_sao_ctxt->i4_ctb_y));
    pu1_sao_src_top_left_luma_bot_left = ps_sao_ctxt->pu1_sao_src_top_left_luma_bot_left + ((ps_sao_ctxt->i4_ctb_y));
    au1_sao_src_top_left_chroma_bot_left = ps_sao_ctxt->au1_sao_src_top_left_chroma_bot_left; // + (2 * ps_sao_ctxt->i4_ctb_y);
    pu1_sao_src_top_left_chroma_bot_left = ps_sao_ctxt->pu1_sao_src_top_left_chroma_bot_left + (2 * ps_sao_ctxt->i4_ctb_y);
    pu1_sao_src_top_left_luma_top_right = ps_sao_ctxt->pu1_sao_src_top_left_luma_top_right + ((ps_sao_ctxt->i4_ctb_x));
    pu1_sao_src_top_left_chroma_top_right = ps_sao_ctxt->pu1_sao_src_top_left_chroma_top_right + (2 * ps_sao_ctxt->i4_ctb_x);

    ps_sao = ps_sao_ctxt->ps_pic_sao + ps_sao_ctxt->i4_ctb_x + ps_sao_ctxt->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb;
    loop_filter_strd =  (ps_sps->i2_pic_width_in_luma_samples + 63) >> 6;
    backup_strd = 2 * MAX_CTB_SIZE;

    DEBUG_INIT_TMP_BUF(ps_sao_ctxt->pu1_tmp_buf_luma, ps_sao_ctxt->pu1_tmp_buf_chroma);

    {
        /* Check the loop filter flags and copy the original values for back up */
        /* Luma */

        /* Done unconditionally since SAO is done on a shifted CTB and the constituent CTBs
         * can belong to different slice with their own sao_enable flag */
        {
            UWORD32 u4_no_loop_filter_flag;
            WORD32 loop_filter_bit_pos;
            WORD32 log2_min_cu = 3;
            WORD32 min_cu = (1 << log2_min_cu);
            UWORD8 *pu1_src_tmp_luma = pu1_src_luma;
            WORD32 sao_blk_ht = ctb_size - SAO_SHIFT_CTB;
            WORD32 sao_blk_wd = ctb_size;
            WORD32 remaining_rows;
            WORD32 remaining_cols;

            remaining_rows = ps_sps->i2_pic_height_in_luma_samples - ((ps_sao_ctxt->i4_ctb_y << log2_ctb_size) + ctb_size - SAO_SHIFT_CTB);
            remaining_cols = ps_sps->i2_pic_width_in_luma_samples - ((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) + ctb_size - SAO_SHIFT_CTB);
            if(remaining_rows <= SAO_SHIFT_CTB)
                sao_blk_ht += remaining_rows;
            if(remaining_cols <= SAO_SHIFT_CTB)
                sao_blk_wd += remaining_cols;

            pu1_src_tmp_luma -= ps_sao_ctxt->i4_ctb_x ? SAO_SHIFT_CTB : 0;
            pu1_src_tmp_luma -= ps_sao_ctxt->i4_ctb_y ? SAO_SHIFT_CTB * src_strd : 0;

            pu1_src_backup_luma = ps_sao_ctxt->pu1_tmp_buf_luma;

            loop_filter_bit_pos = (ps_sao_ctxt->i4_ctb_x << (log2_ctb_size - 3)) +
                            (ps_sao_ctxt->i4_ctb_y << (log2_ctb_size - 3)) * (loop_filter_strd << 3);
            if(ps_sao_ctxt->i4_ctb_x > 0)
                loop_filter_bit_pos -= 1;

            pu1_no_loop_filter_flag = ps_sao_ctxt->pu1_pic_no_loop_filter_flag +
                            (loop_filter_bit_pos >> 3);

            for(i = -(ps_sao_ctxt->i4_ctb_y ? SAO_SHIFT_CTB : 0) >> log2_min_cu;
                            i < (sao_blk_ht + (min_cu - 1)) >> log2_min_cu; i++)
            {
                WORD32 tmp_wd = sao_blk_wd;

                u4_no_loop_filter_flag = (*(UWORD32 *)(pu1_no_loop_filter_flag + i * loop_filter_strd)) >>
                                (loop_filter_bit_pos & 7);
                u4_no_loop_filter_flag &= (1 << ((tmp_wd + (min_cu - 1)) >> log2_min_cu)) - 1;

                if(u4_no_loop_filter_flag)
                {
                    no_loop_filter_enabled_luma = 1;
                    while(tmp_wd > 0)
                    {
                        if(CTZ(u4_no_loop_filter_flag))
                        {
                            pu1_src_tmp_luma += MIN((WORD32)(CTZ(u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            pu1_src_backup_luma += MIN((WORD32)(CTZ(u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            tmp_wd -= CTZ(u4_no_loop_filter_flag) << log2_min_cu;
                            u4_no_loop_filter_flag  >>= (CTZ(u4_no_loop_filter_flag));
                        }
                        else
                        {
                            for(row = 0; row < min_cu; row++)
                            {
                                for(col = 0; col < MIN((WORD32)(CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd); col++)
                                {
                                    pu1_src_backup_luma[row * backup_strd + col] = pu1_src_tmp_luma[row * src_strd + col];
                                }
                            }
                            pu1_src_tmp_luma += MIN(((WORD32)CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            pu1_src_backup_luma += MIN(((WORD32)CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            tmp_wd -= CTZ(~u4_no_loop_filter_flag) << log2_min_cu;
                            u4_no_loop_filter_flag  >>= (CTZ(~u4_no_loop_filter_flag));
                        }
                    }

                    pu1_src_tmp_luma -= sao_blk_wd;
                    pu1_src_backup_luma -= sao_blk_wd;
                }

                pu1_src_tmp_luma += (src_strd << log2_min_cu);
                pu1_src_backup_luma += (backup_strd << log2_min_cu);
            }
        }

        /* Chroma */

        {
            UWORD32 u4_no_loop_filter_flag;
            WORD32 loop_filter_bit_pos;
            WORD32 log2_min_cu = 3;
            WORD32 min_cu = (1 << log2_min_cu);
            UWORD8 *pu1_src_tmp_chroma = pu1_src_chroma;
            WORD32 sao_blk_ht = ctb_size - 2 * SAO_SHIFT_CTB;
            WORD32 sao_blk_wd = ctb_size;
            WORD32 remaining_rows;
            WORD32 remaining_cols;

            remaining_rows = ps_sps->i2_pic_height_in_luma_samples - ((ps_sao_ctxt->i4_ctb_y << log2_ctb_size) + ctb_size - 2 * SAO_SHIFT_CTB);
            remaining_cols = ps_sps->i2_pic_width_in_luma_samples - ((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) + ctb_size - 2 * SAO_SHIFT_CTB);
            if(remaining_rows <= 2 * SAO_SHIFT_CTB)
                sao_blk_ht += remaining_rows;
            if(remaining_cols <= 2 * SAO_SHIFT_CTB)
                sao_blk_wd += remaining_cols;

            pu1_src_tmp_chroma -= ps_sao_ctxt->i4_ctb_x ? SAO_SHIFT_CTB * 2 : 0;
            pu1_src_tmp_chroma -= ps_sao_ctxt->i4_ctb_y ? SAO_SHIFT_CTB * src_strd : 0;

            pu1_src_backup_chroma = ps_sao_ctxt->pu1_tmp_buf_chroma;

            loop_filter_bit_pos = (ps_sao_ctxt->i4_ctb_x << (log2_ctb_size - 3)) +
                            (ps_sao_ctxt->i4_ctb_y << (log2_ctb_size - 3)) * (loop_filter_strd << 3);
            if(ps_sao_ctxt->i4_ctb_x > 0)
                loop_filter_bit_pos -= 2;

            pu1_no_loop_filter_flag = ps_sao_ctxt->pu1_pic_no_loop_filter_flag +
                            (loop_filter_bit_pos >> 3);

            for(i = -(ps_sao_ctxt->i4_ctb_y ? 2 * SAO_SHIFT_CTB : 0) >> log2_min_cu;
                            i < (sao_blk_ht + (min_cu - 1)) >> log2_min_cu; i++)
            {
                WORD32 tmp_wd = sao_blk_wd;

                u4_no_loop_filter_flag = (*(UWORD32 *)(pu1_no_loop_filter_flag + i * loop_filter_strd)) >>
                                (loop_filter_bit_pos & 7);
                u4_no_loop_filter_flag &= (1 << ((tmp_wd + (min_cu - 1)) >> log2_min_cu)) - 1;

                if(u4_no_loop_filter_flag)
                {
                    no_loop_filter_enabled_chroma = 1;
                    while(tmp_wd > 0)
                    {
                        if(CTZ(u4_no_loop_filter_flag))
                        {
                            pu1_src_tmp_chroma += MIN(((WORD32)CTZ(u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            pu1_src_backup_chroma += MIN(((WORD32)CTZ(u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            tmp_wd -= CTZ(u4_no_loop_filter_flag) << log2_min_cu;
                            u4_no_loop_filter_flag  >>= (CTZ(u4_no_loop_filter_flag));
                        }
                        else
                        {
                            for(row = 0; row < min_cu / 2; row++)
                            {
                                for(col = 0; col < MIN(((WORD32)CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd); col++)
                                {
                                    pu1_src_backup_chroma[row * backup_strd + col] = pu1_src_tmp_chroma[row * src_strd + col];
                                }
                            }

                            pu1_src_tmp_chroma += MIN(((WORD32)CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            pu1_src_backup_chroma += MIN(((WORD32)CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            tmp_wd -= CTZ(~u4_no_loop_filter_flag) << log2_min_cu;
                            u4_no_loop_filter_flag  >>= (CTZ(~u4_no_loop_filter_flag));
                        }
                    }

                    pu1_src_tmp_chroma -= sao_blk_wd;
                    pu1_src_backup_chroma -= sao_blk_wd;
                }

                pu1_src_tmp_chroma += ((src_strd / 2) << log2_min_cu);
                pu1_src_backup_chroma += ((backup_strd / 2) << log2_min_cu);
            }
        }
    }

    DEBUG_PROCESS_TMP_BUF(ps_sao_ctxt->pu1_tmp_buf_luma, ps_sao_ctxt->pu1_tmp_buf_chroma);

    /* Top-left CTB */
    if(ps_sao_ctxt->i4_ctb_x > 0 && ps_sao_ctxt->i4_ctb_y > 0)
    {
        WORD32 sao_wd_luma = SAO_SHIFT_CTB;
        WORD32 sao_wd_chroma = 2 * SAO_SHIFT_CTB;
        WORD32 sao_ht_luma = SAO_SHIFT_CTB;
        WORD32 sao_ht_chroma = SAO_SHIFT_CTB;

        WORD32 ctbx_tl_t = 0, ctbx_tl_l = 0, ctbx_tl_r = 0, ctbx_tl_d = 0, ctbx_tl = 0;
        WORD32 ctby_tl_t = 0, ctby_tl_l = 0, ctby_tl_r = 0, ctby_tl_d = 0, ctby_tl = 0;
        WORD32 au4_idx_tl[8], idx_tl;

        slice_header_t *ps_slice_hdr_top_left;
        {
            WORD32 top_left_ctb_indx = (ps_sao_ctxt->i4_ctb_y - 1) * ps_sps->i2_pic_wd_in_ctb +
                                        (ps_sao_ctxt->i4_ctb_x - 1);
            ps_slice_hdr_top_left = ps_slice_hdr_base + pu1_slice_idx[top_left_ctb_indx];
        }


        pu1_src_luma -= (sao_wd_luma + sao_ht_luma * src_strd);
        pu1_src_chroma -= (sao_wd_chroma + sao_ht_chroma * src_strd);
        ps_sao -= (1 + ps_sps->i2_pic_wd_in_ctb);
        pu1_src_top_luma = ps_sao_ctxt->pu1_sao_src_top_luma + (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_luma;
        pu1_src_top_chroma = ps_sao_ctxt->pu1_sao_src_top_chroma + (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_chroma;
        pu1_src_left_luma = ps_sao_ctxt->pu1_sao_src_left_luma + (ps_sao_ctxt->i4_ctb_y << log2_ctb_size) - sao_ht_luma;
        pu1_src_left_chroma = ps_sao_ctxt->pu1_sao_src_left_chroma + (ps_sao_ctxt->i4_ctb_y << log2_ctb_size) - (2 * sao_ht_chroma);

        if(ps_slice_hdr_top_left->i1_slice_sao_luma_flag)
        {
            if(0 == ps_sao->b3_y_type_idx)
            {
                /* Update left, top and top-left */
                for(row = 0; row < sao_ht_luma; row++)
                {
                    pu1_src_left_luma[row] = pu1_src_luma[row * src_strd + (sao_wd_luma - 1)];
                }
                pu1_sao_src_luma_top_left_ctb[0] = pu1_src_top_luma[sao_wd_luma - 1];

                ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_luma, &pu1_src_luma[(sao_ht_luma - 1) * src_strd], sao_wd_luma);


            }

            else if(1 == ps_sao->b3_y_type_idx)
            {
                ai1_offset_y[1] = ps_sao->b4_y_offset_1;
                ai1_offset_y[2] = ps_sao->b4_y_offset_2;
                ai1_offset_y[3] = ps_sao->b4_y_offset_3;
                ai1_offset_y[4] = ps_sao->b4_y_offset_4;

                ps_codec->s_func_selector.ihevc_sao_band_offset_luma_fptr(pu1_src_luma,
                                                                          src_strd,
                                                                          pu1_src_left_luma,
                                                                          pu1_src_top_luma,
                                                                          pu1_sao_src_luma_top_left_ctb,
                                                                          ps_sao->b5_y_band_pos,
                                                                          ai1_offset_y,
                                                                          sao_wd_luma,
                                                                          sao_ht_luma
                                                                         );
            }

            else // if(2 <= ps_sao->b3_y_type_idx)
            {
                ai1_offset_y[1] = ps_sao->b4_y_offset_1;
                ai1_offset_y[2] = ps_sao->b4_y_offset_2;
                ai1_offset_y[3] = ps_sao->b4_y_offset_3;
                ai1_offset_y[4] = ps_sao->b4_y_offset_4;

                for(i = 0; i < 8; i++)
                {
                    au1_avail_luma[i] = 255;
                    au1_tile_slice_boundary[i] = 0;
                    au4_idx_tl[i] = 0;
                    au4_ilf_across_tile_slice_enable[i] = 1;
                }

                /******************************************************************
                 * Derive the  Top-left CTB's neighbor pixel's slice indices.
                 *
                 *          TL_T
                 *       4  _2__5________
                 *     0   |    |       |
                 *    TL_L | TL | 1 TL_R|
                 *         |____|_______|____
                 *        6|TL_D|7      |    |
                 *         | 3  |       |    |
                 *         |____|_______|    |
                 *              |            |
                 *              |            |
                 *              |____________|
                 *
                 *****************************************************************/

                /*In case of slices, unless we encounter multiple slice/tiled clips, don't enter*/
                {
                    if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
                    {
                        {
                            /*Assuming that sao shift is uniform along x and y directions*/
                            if((0 == (1 << log2_ctb_size) - sao_wd_luma) && (ps_sao_ctxt->i4_ctb_y > 1) && (ps_sao_ctxt->i4_ctb_x > 1))
                            {
                                ctby_tl_t = ps_sao_ctxt->i4_ctb_y - 2;
                                ctbx_tl_l = ps_sao_ctxt->i4_ctb_x - 2;
                            }
                            else if(!(0 == (1 << log2_ctb_size) - sao_wd_luma))
                            {
                                ctby_tl_t = ps_sao_ctxt->i4_ctb_y - 1;
                                ctbx_tl_l = ps_sao_ctxt->i4_ctb_x - 1;
                            }
                            ctbx_tl_t = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_tl_l = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_tl_r = ps_sao_ctxt->i4_ctb_x;
                            ctby_tl_r = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_tl_d =  ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_tl_d =  ps_sao_ctxt->i4_ctb_y;

                            ctbx_tl = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_tl = ps_sao_ctxt->i4_ctb_y - 1;
                        }

                        if(!ps_slice_hdr->i1_first_slice_in_pic_flag)
                        {
                            /*Calculate slice indices for neighbor pixels*/
                            idx_tl   = pu1_slice_idx[ctbx_tl + (ctby_tl * ps_sps->i2_pic_wd_in_ctb)];
                            au4_idx_tl[2] = au4_idx_tl[4] = *(pu1_slice_idx + ctbx_tl_t + (ctby_tl_t * ps_sps->i2_pic_wd_in_ctb));
                            au4_idx_tl[0] =  pu1_slice_idx[ctbx_tl_l + (ctby_tl_l * ps_sps->i2_pic_wd_in_ctb)];
                            au4_idx_tl[1] = au4_idx_tl[5] = pu1_slice_idx[ctbx_tl_r + (ctby_tl_r * ps_sps->i2_pic_wd_in_ctb)];
                            au4_idx_tl[3] = au4_idx_tl[6] =   pu1_slice_idx[ctbx_tl_d + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];
                            au4_idx_tl[7] = pu1_slice_idx[ctbx_tl_d + 1 + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];

                            if((0 == (1 << log2_ctb_size) - sao_wd_luma))
                            {
                                if(ps_sao_ctxt->i4_ctb_x == 1)
                                {
                                    au4_idx_tl[6] = -1;
                                    au4_idx_tl[4] = -1;
                                }
                                else
                                {
                                    au4_idx_tl[6] = pu1_slice_idx[(ctbx_tl_d - 1) + (ctby_tl_r * ps_sps->i2_pic_wd_in_ctb)];
                                }
                                if(ps_sao_ctxt->i4_ctb_y == 1)
                                {
                                    au4_idx_tl[5] = -1;
                                    au4_idx_tl[4] = -1;
                                }
                                else
                                {
                                    au4_idx_tl[5] = pu1_slice_idx[(ctbx_tl_l + 1) + (ctby_tl_l * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_tl[4] = pu1_slice_idx[(ctbx_tl_t - 1) + (ctby_tl_t * ps_sps->i2_pic_wd_in_ctb)];
                                }
                                au4_idx_tl[7] = pu1_slice_idx[(ctbx_tl_d + 1) + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];
                            }

                            /* Verify that the neighbor ctbs dont cross pic boundary.
                             * Between each neighbor and the current CTB, the i1_slice_loop_filter_across_slices_enabled_flag
                             * of the pixel having a greater address is checked. Accordingly, set the availability flags.
                             * Hence, for top and left pixels, current ctb flag is checked. For right and down pixels,
                             * the respective pixel's flags are checked
                             */

                            if((0 == (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_luma))
                            {
                                au4_ilf_across_tile_slice_enable[4] = 0;
                                au4_ilf_across_tile_slice_enable[6] = 0;
                            }
                            else
                            {
                                au4_ilf_across_tile_slice_enable[6] = (ps_slice_hdr_base + au4_idx_tl[6])->i1_slice_loop_filter_across_slices_enabled_flag;
                            }
                            if((0 == (ps_sao_ctxt->i4_ctb_y << log2_ctb_size) - sao_ht_luma))
                            {
                                au4_ilf_across_tile_slice_enable[5] = 0;
                                au4_ilf_across_tile_slice_enable[4] = 0;
                            }
                            else
                            {
                                au4_ilf_across_tile_slice_enable[5] = (ps_slice_hdr_base + idx_tl)->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[4] = (ps_slice_hdr_base + idx_tl)->i1_slice_loop_filter_across_slices_enabled_flag;
                            }
                            au4_ilf_across_tile_slice_enable[2] = (ps_slice_hdr_base + idx_tl)->i1_slice_loop_filter_across_slices_enabled_flag;
                            au4_ilf_across_tile_slice_enable[0] = (ps_slice_hdr_base + idx_tl)->i1_slice_loop_filter_across_slices_enabled_flag;
                            au4_ilf_across_tile_slice_enable[1] = (ps_slice_hdr_base + au4_idx_tl[1])->i1_slice_loop_filter_across_slices_enabled_flag;
                            au4_ilf_across_tile_slice_enable[3] = (ps_slice_hdr_base + au4_idx_tl[3])->i1_slice_loop_filter_across_slices_enabled_flag;
                            au4_ilf_across_tile_slice_enable[7] = (ps_slice_hdr_base + au4_idx_tl[7])->i1_slice_loop_filter_across_slices_enabled_flag;

                            if(au4_idx_tl[5] > idx_tl)
                            {
                                au4_ilf_across_tile_slice_enable[5] = (ps_slice_hdr_base + au4_idx_tl[5])->i1_slice_loop_filter_across_slices_enabled_flag;
                            }

                            /*
                             * Between each neighbor and the current CTB, the i1_slice_loop_filter_across_slices_enabled_flag
                             * of the pixel having a greater address is checked. Accordingly, set the availability flags.
                             * Hence, for top and left pixels, current ctb flag is checked. For right and down pixels,
                             * the respective pixel's flags are checked
                             */
                            for(i = 0; i < 8; i++)
                            {
                                /*Sets the edges that lie on the slice/tile boundary*/
                                if(au4_idx_tl[i] != idx_tl)
                                {
                                    au1_tile_slice_boundary[i] = 1;
                                }
                                else
                                {
                                    au4_ilf_across_tile_slice_enable[i] = 1;
                                }
                            }

                            ps_codec->s_func_selector.ihevc_memset_mul_8_fptr((UWORD8 *)au4_idx_tl, 0, 8 * sizeof(WORD32));
                        }

                        if(ps_pps->i1_tiles_enabled_flag)
                        {
                            /* Calculate availability flags at slice boundary */
                            if(((ps_tile->u1_pos_x == ps_sao_ctxt->i4_ctb_x) || (ps_tile->u1_pos_y == ps_sao_ctxt->i4_ctb_y)) && (!((0 == ps_tile->u1_pos_x) && (0 == ps_tile->u1_pos_y))))
                            {
                                /*If ilf across tiles is enabled, boundary availability for tiles is not checked. */
                                if(!ps_pps->i1_loop_filter_across_tiles_enabled_flag)
                                {
                                    /*Set the boundary arrays*/
                                    /*Calculate tile indices for neighbor pixels*/
                                    idx_tl   = pu1_tile_idx[ctbx_tl + (ctby_tl * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_tl[2] = au4_idx_tl[4] = *(pu1_tile_idx + ctbx_tl_t + (ctby_tl_t * ps_sps->i2_pic_wd_in_ctb));
                                    au4_idx_tl[0] =  pu1_tile_idx[ctbx_tl_l + (ctby_tl_l * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_tl[1] = au4_idx_tl[5] = pu1_tile_idx[ctbx_tl_r + (ctby_tl_r * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_tl[3] = au4_idx_tl[6] =   pu1_tile_idx[ctbx_tl_d + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_tl[7] = pu1_tile_idx[ctbx_tl_d + 1 + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];

                                    if((0 == (1 << log2_ctb_size) - sao_wd_luma))
                                    {
                                        if(ps_sao_ctxt->i4_ctb_x == 1)
                                        {
                                            au4_idx_tl[6] = -1;
                                            au4_idx_tl[4] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_tl[6] = pu1_tile_idx[(ctbx_tl_d - 1) + (ctby_tl_r * ps_sps->i2_pic_wd_in_ctb)];
                                        }
                                        if(ps_sao_ctxt->i4_ctb_y == 1)
                                        {
                                            au4_idx_tl[5] = -1;
                                            au4_idx_tl[4] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_tl[5] = pu1_tile_idx[(ctbx_tl_l + 1) + (ctby_tl_l * ps_sps->i2_pic_wd_in_ctb)];
                                            au4_idx_tl[4] = pu1_tile_idx[(ctbx_tl_t - 1) + (ctby_tl_t * ps_sps->i2_pic_wd_in_ctb)];
                                        }
                                        au4_idx_tl[7] = pu1_tile_idx[(ctbx_tl_d + 1) + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];
                                    }
                                    for(i = 0; i < 8; i++)
                                    {
                                        /*Sets the edges that lie on the tile boundary*/
                                        if(au4_idx_tl[i] != idx_tl)
                                        {
                                            au1_tile_slice_boundary[i] |= 1;
                                            au4_ilf_across_tile_slice_enable[i] &= ps_pps->i1_loop_filter_across_tiles_enabled_flag; //=0
                                        }
                                    }
                                }
                            }
                        }


                        /*Set availability flags based on tile and slice boundaries*/
                        for(i = 0; i < 8; i++)
                        {
                            /*Sets the edges that lie on the slice/tile boundary*/
                            if((au1_tile_slice_boundary[i]) && !(au4_ilf_across_tile_slice_enable[i]))
                            {
                                au1_avail_luma[i] = 0;
                            }
                        }
                    }
                }

                if(0 == (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_luma)
                {
                    au1_avail_luma[0] = 0;
                    au1_avail_luma[4] = 0;
                    au1_avail_luma[6] = 0;
                }

                if(ps_sps->i2_pic_wd_in_ctb == ps_sao_ctxt->i4_ctb_x)
                {
                    au1_avail_luma[1] = 0;
                    au1_avail_luma[5] = 0;
                    au1_avail_luma[7] = 0;
                }
                //y==1 case
                if((0 == (ps_sao_ctxt->i4_ctb_y << log2_ctb_size) - sao_ht_luma))
                {
                    au1_avail_luma[2] = 0;
                    au1_avail_luma[4] = 0;
                    au1_avail_luma[5] = 0;
                }
                if(ps_sps->i2_pic_ht_in_ctb == ps_sao_ctxt->i4_ctb_y)
                {
                    au1_avail_luma[3] = 0;
                    au1_avail_luma[6] = 0;
                    au1_avail_luma[7] = 0;
                }

                {
                    au1_src_top_right[0] = pu1_src_top_luma[sao_wd_luma];
                    u1_sao_src_top_left_luma_bot_left = pu1_src_left_luma[sao_ht_luma];
                    ps_codec->apf_sao_luma[ps_sao->b3_y_type_idx - 2](pu1_src_luma,
                                                                      src_strd,
                                                                      pu1_src_left_luma,
                                                                      pu1_src_top_luma,
                                                                      pu1_sao_src_luma_top_left_ctb,
                                                                      au1_src_top_right,
                                                                      &u1_sao_src_top_left_luma_bot_left,
                                                                      au1_avail_luma,
                                                                      ai1_offset_y,
                                                                      sao_wd_luma,
                                                                      sao_ht_luma);
                }
            }

        }
        else if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
        {
            /* Update left, top and top-left */
            for(row = 0; row < sao_ht_luma; row++)
            {
                pu1_src_left_luma[row] = pu1_src_luma[row * src_strd + (sao_wd_luma - 1)];
            }
            pu1_sao_src_luma_top_left_ctb[0] = pu1_src_top_luma[sao_wd_luma - 1];

            ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_luma, &pu1_src_luma[(sao_ht_luma - 1) * src_strd], sao_wd_luma);
        }

        if(ps_slice_hdr_top_left->i1_slice_sao_chroma_flag)
        {
            if(0 == ps_sao->b3_cb_type_idx)
            {
                for(row = 0; row < sao_ht_chroma; row++)
                {
                    pu1_src_left_chroma[2 * row] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 2)];
                    pu1_src_left_chroma[2 * row + 1] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 1)];
                }
                pu1_sao_src_chroma_top_left_ctb[0] = pu1_src_top_chroma[sao_wd_chroma - 2];
                pu1_sao_src_chroma_top_left_ctb[1] = pu1_src_top_chroma[sao_wd_chroma - 1];

                ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_chroma, &pu1_src_chroma[(sao_ht_chroma - 1) * src_strd], sao_wd_chroma);

            }

            else if(1 == ps_sao->b3_cb_type_idx)
            {
                ai1_offset_cb[1] = ps_sao->b4_cb_offset_1;
                ai1_offset_cb[2] = ps_sao->b4_cb_offset_2;
                ai1_offset_cb[3] = ps_sao->b4_cb_offset_3;
                ai1_offset_cb[4] = ps_sao->b4_cb_offset_4;

                ai1_offset_cr[1] = ps_sao->b4_cr_offset_1;
                ai1_offset_cr[2] = ps_sao->b4_cr_offset_2;
                ai1_offset_cr[3] = ps_sao->b4_cr_offset_3;
                ai1_offset_cr[4] = ps_sao->b4_cr_offset_4;

                if(chroma_yuv420sp_vu)
                {
                    ps_codec->s_func_selector.ihevc_sao_band_offset_chroma_fptr(pu1_src_chroma,
                                                                                src_strd,
                                                                                pu1_src_left_chroma,
                                                                                pu1_src_top_chroma,
                                                                                pu1_sao_src_chroma_top_left_ctb,
                                                                                ps_sao->b5_cr_band_pos,
                                                                                ps_sao->b5_cb_band_pos,
                                                                                ai1_offset_cr,
                                                                                ai1_offset_cb,
                                                                                sao_wd_chroma,
                                                                                sao_ht_chroma
                                                                               );
                }
                else
                {
                    ps_codec->s_func_selector.ihevc_sao_band_offset_chroma_fptr(pu1_src_chroma,
                                                                                src_strd,
                                                                                pu1_src_left_chroma,
                                                                                pu1_src_top_chroma,
                                                                                pu1_sao_src_chroma_top_left_ctb,
                                                                                ps_sao->b5_cb_band_pos,
                                                                                ps_sao->b5_cr_band_pos,
                                                                                ai1_offset_cb,
                                                                                ai1_offset_cr,
                                                                                sao_wd_chroma,
                                                                                sao_ht_chroma
                                                                               );
                }
            }

            else // if(2 <= ps_sao->b3_cb_type_idx)
            {
                ai1_offset_cb[1] = ps_sao->b4_cb_offset_1;
                ai1_offset_cb[2] = ps_sao->b4_cb_offset_2;
                ai1_offset_cb[3] = ps_sao->b4_cb_offset_3;
                ai1_offset_cb[4] = ps_sao->b4_cb_offset_4;

                ai1_offset_cr[1] = ps_sao->b4_cr_offset_1;
                ai1_offset_cr[2] = ps_sao->b4_cr_offset_2;
                ai1_offset_cr[3] = ps_sao->b4_cr_offset_3;
                ai1_offset_cr[4] = ps_sao->b4_cr_offset_4;
                for(i = 0; i < 8; i++)
                {
                    au1_avail_chroma[i] = 255;
                    au1_tile_slice_boundary[i] = 0;
                    au4_idx_tl[i] = 0;
                    au4_ilf_across_tile_slice_enable[i] = 1;
                }
                /*In case of slices*/
                {
                    if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
                    {
                        if((0 == (1 << log2_ctb_size) - sao_wd_chroma) && (ps_sao_ctxt->i4_ctb_y > 1) && (ps_sao_ctxt->i4_ctb_x > 1))
                        {
                            ctby_tl_t = ps_sao_ctxt->i4_ctb_y - 2;
                            ctbx_tl_l = ps_sao_ctxt->i4_ctb_x - 2;
                        }
                        else if(!(0 == (1 << log2_ctb_size) - sao_wd_chroma))
                        {
                            ctby_tl_t = ps_sao_ctxt->i4_ctb_y - 1;
                            ctbx_tl_l = ps_sao_ctxt->i4_ctb_x - 1;
                        }
                        ctbx_tl_t = ps_sao_ctxt->i4_ctb_x - 1;
                        ctby_tl_l = ps_sao_ctxt->i4_ctb_y - 1;

                        ctbx_tl_r = ps_sao_ctxt->i4_ctb_x;
                        ctby_tl_r = ps_sao_ctxt->i4_ctb_y - 1;

                        ctbx_tl_d =  ps_sao_ctxt->i4_ctb_x - 1;
                        ctby_tl_d =  ps_sao_ctxt->i4_ctb_y;

                        ctbx_tl = ps_sao_ctxt->i4_ctb_x - 1;
                        ctby_tl = ps_sao_ctxt->i4_ctb_y - 1;

                        if(!ps_slice_hdr->i1_first_slice_in_pic_flag)
                        {

                            idx_tl   = pu1_slice_idx[ctbx_tl + (ctby_tl * ps_sps->i2_pic_wd_in_ctb)];
                            au4_idx_tl[2] = au4_idx_tl[4] = *(pu1_slice_idx + ctbx_tl_t + (ctby_tl_t * ps_sps->i2_pic_wd_in_ctb));
                            au4_idx_tl[0] =  pu1_slice_idx[ctbx_tl_l + (ctby_tl_l * ps_sps->i2_pic_wd_in_ctb)];
                            au4_idx_tl[1] = au4_idx_tl[5] = pu1_slice_idx[ctbx_tl_r + (ctby_tl_r * ps_sps->i2_pic_wd_in_ctb)];
                            au4_idx_tl[3] = au4_idx_tl[6] =   pu1_slice_idx[ctbx_tl_d + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];
                            au4_idx_tl[7] = pu1_slice_idx[ctbx_tl_d + 1 + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];

                            if((0 == (1 << log2_ctb_size) - sao_wd_chroma))
                            {
                                if(ps_sao_ctxt->i4_ctb_x == 1)
                                {
                                    au4_idx_tl[6] = -1;
                                    au4_idx_tl[4] = -1;
                                }
                                else
                                {
                                    au4_idx_tl[6] = pu1_slice_idx[(ctbx_tl_d - 1) + (ctby_tl_r * ps_sps->i2_pic_wd_in_ctb)];
                                }
                                if(ps_sao_ctxt->i4_ctb_y == 1)
                                {
                                    au4_idx_tl[5] = -1;
                                    au4_idx_tl[4] = -1;
                                }
                                else
                                {
                                    au4_idx_tl[5] = pu1_slice_idx[(ctbx_tl_l + 1) + (ctby_tl_l * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_tl[4] = pu1_slice_idx[(ctbx_tl_t - 1) + (ctby_tl_t * ps_sps->i2_pic_wd_in_ctb)];
                                }
                                au4_idx_tl[7] = pu1_slice_idx[(ctbx_tl_d + 1) + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];
                            }

                            /* Verify that the neighbor ctbs don't cross pic boundary
                             * Also, the ILF flag belonging to the higher pixel address (between neighbor and current pixels) must be assigned*/
                            if((0 == (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_chroma))
                            {
                                au4_ilf_across_tile_slice_enable[4] = 0;
                                au4_ilf_across_tile_slice_enable[6] = 0;
                            }
                            else
                            {
                                au4_ilf_across_tile_slice_enable[6] = (ps_slice_hdr_base + au4_idx_tl[6])->i1_slice_loop_filter_across_slices_enabled_flag;
                            }
                            if((0 == (ps_sao_ctxt->i4_ctb_y << (log2_ctb_size - 1)) - sao_ht_chroma))
                            {
                                au4_ilf_across_tile_slice_enable[5] = 0;
                                au4_ilf_across_tile_slice_enable[4] = 0;
                            }
                            else
                            {
                                au4_ilf_across_tile_slice_enable[4] = (ps_slice_hdr_base + idx_tl)->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[5] = (ps_slice_hdr_base + au4_idx_tl[5])->i1_slice_loop_filter_across_slices_enabled_flag;
                            }
                            au4_ilf_across_tile_slice_enable[2] = (ps_slice_hdr_base + idx_tl)->i1_slice_loop_filter_across_slices_enabled_flag;
                            au4_ilf_across_tile_slice_enable[0] = (ps_slice_hdr_base + idx_tl)->i1_slice_loop_filter_across_slices_enabled_flag;
                            au4_ilf_across_tile_slice_enable[1] = (ps_slice_hdr_base + au4_idx_tl[1])->i1_slice_loop_filter_across_slices_enabled_flag;
                            au4_ilf_across_tile_slice_enable[3] = (ps_slice_hdr_base + au4_idx_tl[3])->i1_slice_loop_filter_across_slices_enabled_flag;
                            au4_ilf_across_tile_slice_enable[7] = (ps_slice_hdr_base + au4_idx_tl[7])->i1_slice_loop_filter_across_slices_enabled_flag;
                            /*
                             * Between each neighbor and the current CTB, the i1_slice_loop_filter_across_slices_enabled_flag
                             * of the pixel having a greater address is checked. Accordingly, set the availability flags
                             */
                            for(i = 0; i < 8; i++)
                            {
                                /*Sets the edges that lie on the slice/tile boundary*/
                                if(au4_idx_tl[i] != idx_tl)
                                {
                                    au1_tile_slice_boundary[i] = 1;
                                }
                                else
                                {
                                    au4_ilf_across_tile_slice_enable[i] = 1;
                                }
                            }

                            /*Reset indices*/
                            for(i = 0; i < 8; i++)
                            {
                                au4_idx_tl[i] = 0;
                            }
                        }
                        if(ps_pps->i1_tiles_enabled_flag)
                        {
                            /* Calculate availability flags at slice boundary */
                            if(((ps_tile->u1_pos_x == ps_sao_ctxt->i4_ctb_x) || (ps_tile->u1_pos_y == ps_sao_ctxt->i4_ctb_y)) && (!((0 == ps_tile->u1_pos_x) && (0 == ps_tile->u1_pos_y))))
                            {
                                /*If ilf across tiles is enabled, boundary availability for tiles is not checked. */
                                if(!ps_pps->i1_loop_filter_across_tiles_enabled_flag)
                                {
                                    /*Set the boundary arrays*/
                                    /*Calculate tile indices for neighbor pixels*/
                                    idx_tl   = pu1_tile_idx[ctbx_tl + (ctby_tl * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_tl[2] = au4_idx_tl[4] = *(pu1_tile_idx + ctbx_tl_t + (ctby_tl_t * ps_sps->i2_pic_wd_in_ctb));
                                    au4_idx_tl[0] =  pu1_tile_idx[ctbx_tl_l + (ctby_tl_l * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_tl[1] = au4_idx_tl[5] = pu1_tile_idx[ctbx_tl_r + (ctby_tl_r * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_tl[3] = au4_idx_tl[6] =   pu1_tile_idx[ctbx_tl_d + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_tl[7] = pu1_tile_idx[ctbx_tl_d + 1 + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];

                                    if((0 == (1 << log2_ctb_size) - sao_wd_luma))
                                    {
                                        if(ps_sao_ctxt->i4_ctb_x == 1)
                                        {
                                            au4_idx_tl[6] = -1;
                                            au4_idx_tl[4] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_tl[6] = pu1_tile_idx[(ctbx_tl_d - 1) + (ctby_tl_r * ps_sps->i2_pic_wd_in_ctb)];
                                        }
                                        if(ps_sao_ctxt->i4_ctb_y == 1)
                                        {
                                            au4_idx_tl[5] = -1;
                                            au4_idx_tl[4] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_tl[5] = pu1_tile_idx[(ctbx_tl_l + 1) + (ctby_tl_l * ps_sps->i2_pic_wd_in_ctb)];
                                            au4_idx_tl[4] = pu1_tile_idx[(ctbx_tl_t - 1) + (ctby_tl_t * ps_sps->i2_pic_wd_in_ctb)];
                                        }
                                        au4_idx_tl[7] = pu1_tile_idx[(ctbx_tl_d + 1) + (ctby_tl_d * ps_sps->i2_pic_wd_in_ctb)];
                                    }
                                    for(i = 0; i < 8; i++)
                                    {
                                        /*Sets the edges that lie on the tile boundary*/
                                        if(au4_idx_tl[i] != idx_tl)
                                        {
                                            au1_tile_slice_boundary[i] |= 1;
                                            au4_ilf_across_tile_slice_enable[i] &= ps_pps->i1_loop_filter_across_tiles_enabled_flag; //=0
                                        }
                                    }
                                }
                            }
                        }

                        for(i = 0; i < 8; i++)
                        {
                            /*Sets the edges that lie on the slice/tile boundary*/
                            if((au1_tile_slice_boundary[i]) && !(au4_ilf_across_tile_slice_enable[i]))
                            {
                                au1_avail_chroma[i] = 0;
                            }
                        }
                    }
                }

                if(0 == (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_chroma)
                {
                    au1_avail_chroma[0] = 0;
                    au1_avail_chroma[4] = 0;
                    au1_avail_chroma[6] = 0;
                }
                if(ps_sps->i2_pic_wd_in_ctb == ps_sao_ctxt->i4_ctb_x)
                {
                    au1_avail_chroma[1] = 0;
                    au1_avail_chroma[5] = 0;
                    au1_avail_chroma[7] = 0;
                }

                if(0 == (ps_sao_ctxt->i4_ctb_y << (log2_ctb_size - 1)) - sao_ht_chroma)
                {
                    au1_avail_chroma[2] = 0;
                    au1_avail_chroma[4] = 0;
                    au1_avail_chroma[5] = 0;
                }
                if(ps_sps->i2_pic_ht_in_ctb == ps_sao_ctxt->i4_ctb_y)
                {
                    au1_avail_chroma[3] = 0;
                    au1_avail_chroma[6] = 0;
                    au1_avail_chroma[7] = 0;
                }

                {
                    au1_src_top_right[0] = pu1_src_top_chroma[sao_wd_chroma];
                    au1_src_top_right[1] = pu1_src_top_chroma[sao_wd_chroma + 1];
                    au1_sao_src_top_left_chroma_bot_left[0] = pu1_src_left_chroma[2 * sao_ht_chroma];
                    au1_sao_src_top_left_chroma_bot_left[1] = pu1_src_left_chroma[2 * sao_ht_chroma + 1];
                    if((ctb_size == 16) && (ps_sao_ctxt->i4_ctb_y != ps_sps->i2_pic_ht_in_ctb - 1))
                    {
                        au1_sao_src_top_left_chroma_bot_left[0] = pu1_src_chroma[sao_ht_chroma * src_strd - 2];
                        au1_sao_src_top_left_chroma_bot_left[1] = pu1_src_chroma[sao_ht_chroma * src_strd - 1];
                    }

                    if(chroma_yuv420sp_vu)
                    {
                        ps_codec->apf_sao_chroma[ps_sao->b3_cb_type_idx - 2](pu1_src_chroma,
                                                                             src_strd,
                                                                             pu1_src_left_chroma,
                                                                             pu1_src_top_chroma,
                                                                             pu1_sao_src_chroma_top_left_ctb,
                                                                             au1_src_top_right,
                                                                             au1_sao_src_top_left_chroma_bot_left,
                                                                             au1_avail_chroma,
                                                                             ai1_offset_cr,
                                                                             ai1_offset_cb,
                                                                             sao_wd_chroma,
                                                                             sao_ht_chroma);
                    }
                    else
                    {
                        ps_codec->apf_sao_chroma[ps_sao->b3_cb_type_idx - 2](pu1_src_chroma,
                                                                             src_strd,
                                                                             pu1_src_left_chroma,
                                                                             pu1_src_top_chroma,
                                                                             pu1_sao_src_chroma_top_left_ctb,
                                                                             au1_src_top_right,
                                                                             au1_sao_src_top_left_chroma_bot_left,
                                                                             au1_avail_chroma,
                                                                             ai1_offset_cb,
                                                                             ai1_offset_cr,
                                                                             sao_wd_chroma,
                                                                             sao_ht_chroma);
                    }
                }
            }
        }
        else if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
        {
            for(row = 0; row < sao_ht_chroma; row++)
            {
                pu1_src_left_chroma[2 * row] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 2)];
                pu1_src_left_chroma[2 * row + 1] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 1)];
            }
            pu1_sao_src_chroma_top_left_ctb[0] = pu1_src_top_chroma[sao_wd_chroma - 2];
            pu1_sao_src_chroma_top_left_ctb[1] = pu1_src_top_chroma[sao_wd_chroma - 1];

            ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_chroma, &pu1_src_chroma[(sao_ht_chroma - 1) * src_strd], sao_wd_chroma);
        }

        pu1_src_luma += sao_wd_luma + sao_ht_luma * src_strd;
        pu1_src_chroma += sao_wd_chroma + sao_ht_chroma * src_strd;
        ps_sao += (1 + ps_sps->i2_pic_wd_in_ctb);
    }


    /* Top CTB */
    if((ps_sao_ctxt->i4_ctb_y > 0))
    {
        WORD32 sao_wd_luma = ctb_size - SAO_SHIFT_CTB;
        WORD32 sao_wd_chroma = ctb_size - 2 * SAO_SHIFT_CTB;
        WORD32 sao_ht_luma = SAO_SHIFT_CTB;
        WORD32 sao_ht_chroma = SAO_SHIFT_CTB;

        WORD32 ctbx_t_t = 0, ctbx_t_l = 0, ctbx_t_r = 0, ctbx_t_d = 0, ctbx_t = 0;
        WORD32 ctby_t_t = 0, ctby_t_l = 0, ctby_t_r = 0, ctby_t_d = 0, ctby_t = 0;
        WORD32 au4_idx_t[8], idx_t;

        WORD32 remaining_cols;

        slice_header_t *ps_slice_hdr_top;
        {
            WORD32 top_ctb_indx = (ps_sao_ctxt->i4_ctb_y - 1) * ps_sps->i2_pic_wd_in_ctb +
                                        (ps_sao_ctxt->i4_ctb_x);
            ps_slice_hdr_top = ps_slice_hdr_base + pu1_slice_idx[top_ctb_indx];
        }

        remaining_cols = ps_sps->i2_pic_width_in_luma_samples - ((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) + sao_wd_luma);
        if(remaining_cols <= SAO_SHIFT_CTB)
        {
            sao_wd_luma += remaining_cols;
        }
        remaining_cols = ps_sps->i2_pic_width_in_luma_samples - ((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) + sao_wd_chroma);
        if(remaining_cols <= 2 * SAO_SHIFT_CTB)
        {
            sao_wd_chroma += remaining_cols;
        }

        pu1_src_luma -= (sao_ht_luma * src_strd);
        pu1_src_chroma -= (sao_ht_chroma * src_strd);
        ps_sao -= (ps_sps->i2_pic_wd_in_ctb);
        pu1_src_top_luma = ps_sao_ctxt->pu1_sao_src_top_luma + (ps_sao_ctxt->i4_ctb_x << log2_ctb_size);
        pu1_src_top_chroma = ps_sao_ctxt->pu1_sao_src_top_chroma + (ps_sao_ctxt->i4_ctb_x << log2_ctb_size);
        pu1_src_left_luma = ps_sao_ctxt->pu1_sao_src_left_luma + (ps_sao_ctxt->i4_ctb_y << log2_ctb_size) - sao_ht_chroma;
        pu1_src_left_chroma = ps_sao_ctxt->pu1_sao_src_left_chroma + (ps_sao_ctxt->i4_ctb_y << log2_ctb_size) - (2 * sao_ht_chroma);

        if(0 != sao_wd_luma)
        {
            if(ps_slice_hdr_top->i1_slice_sao_luma_flag)
            {
                if(0 == ps_sao->b3_y_type_idx)
                {
                    /* Update left, top and top-left */
                    for(row = 0; row < sao_ht_luma; row++)
                    {
                        pu1_src_left_luma[row] = pu1_src_luma[row * src_strd + (sao_wd_luma - 1)];
                    }
                    pu1_sao_src_luma_top_left_ctb[0] = pu1_src_top_luma[sao_wd_luma - 1];

                    ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_luma, &pu1_src_luma[(sao_ht_luma - 1) * src_strd], sao_wd_luma);

                }

                else if(1 == ps_sao->b3_y_type_idx)
                {
                    ai1_offset_y[1] = ps_sao->b4_y_offset_1;
                    ai1_offset_y[2] = ps_sao->b4_y_offset_2;
                    ai1_offset_y[3] = ps_sao->b4_y_offset_3;
                    ai1_offset_y[4] = ps_sao->b4_y_offset_4;

                    ps_codec->s_func_selector.ihevc_sao_band_offset_luma_fptr(pu1_src_luma,
                                                                              src_strd,
                                                                              pu1_src_left_luma,
                                                                              pu1_src_top_luma,
                                                                              pu1_sao_src_luma_top_left_ctb,
                                                                              ps_sao->b5_y_band_pos,
                                                                              ai1_offset_y,
                                                                              sao_wd_luma,
                                                                              sao_ht_luma
                                                                             );
                }

                else // if(2 <= ps_sao->b3_y_type_idx)
                {
                    ai1_offset_y[1] = ps_sao->b4_y_offset_1;
                    ai1_offset_y[2] = ps_sao->b4_y_offset_2;
                    ai1_offset_y[3] = ps_sao->b4_y_offset_3;
                    ai1_offset_y[4] = ps_sao->b4_y_offset_4;

                    ps_codec->s_func_selector.ihevc_memset_mul_8_fptr(au1_avail_luma, 255, 8);
                    ps_codec->s_func_selector.ihevc_memset_mul_8_fptr(au1_tile_slice_boundary, 0, 8);
                    ps_codec->s_func_selector.ihevc_memset_mul_8_fptr((UWORD8 *)au4_idx_t, 0, 8 * sizeof(WORD32));

                    for(i = 0; i < 8; i++)
                    {

                        au4_ilf_across_tile_slice_enable[i] = 1;
                    }
                    /******************************************************************
                     * Derive the  Top-left CTB's neighbor pixel's slice indices.
                     *
                     *               T_T
                     *          ____________
                     *         |    |       |
                     *         | T_L|  T    |T_R
                     *         |    | ______|____
                     *         |    |  T_D  |    |
                     *         |    |       |    |
                     *         |____|_______|    |
                     *              |            |
                     *              |            |
                     *              |____________|
                     *
                     *****************************************************************/

                    /*In case of slices*/
                    {
                        if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
                        {

                            ctbx_t_t = ps_sao_ctxt->i4_ctb_x;
                            ctby_t_t = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_t_l = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_t_l = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_t_r = ps_sao_ctxt->i4_ctb_x;
                            ctby_t_r = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_t_d =  ps_sao_ctxt->i4_ctb_x;
                            ctby_t_d =  ps_sao_ctxt->i4_ctb_y;

                            ctbx_t = ps_sao_ctxt->i4_ctb_x;
                            ctby_t = ps_sao_ctxt->i4_ctb_y - 1;

                            if(!ps_slice_hdr->i1_first_slice_in_pic_flag)
                            {
                                /*Calculate neighbor ctb slice indices*/
                                if(0 == ps_sao_ctxt->i4_ctb_x)
                                {
                                    au4_idx_t[0] = -1;
                                    au4_idx_t[6] = -1;
                                    au4_idx_t[4] = -1;
                                }
                                else
                                {
                                    au4_idx_t[0] = au4_idx_t[4] = pu1_slice_idx[ctbx_t_l + (ctby_t_l * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_t[6] = pu1_slice_idx[ctbx_t_d - 1 + (ctby_t_d * ps_sps->i2_pic_wd_in_ctb)];
                                }
                                idx_t   = pu1_slice_idx[ctbx_t + (ctby_t * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_t[2] = au4_idx_t[5] = pu1_slice_idx[ctbx_t_t + (ctby_t_t * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_t[1] = pu1_slice_idx[ctbx_t_r + (ctby_t_r * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_t[3] = au4_idx_t[7] = pu1_slice_idx[ctbx_t_d + (ctby_t_d * ps_sps->i2_pic_wd_in_ctb)];

                                /*Verify that the neighbor ctbs don't cross pic boundary.*/
                                if(0 == ps_sao_ctxt->i4_ctb_x)
                                {
                                    au4_ilf_across_tile_slice_enable[4] = 0;
                                    au4_ilf_across_tile_slice_enable[6] = 0;
                                    au4_ilf_across_tile_slice_enable[0] = 0;
                                }
                                else
                                {
                                    au4_ilf_across_tile_slice_enable[4] = au4_ilf_across_tile_slice_enable[0] = (ps_slice_hdr_base + idx_t)->i1_slice_loop_filter_across_slices_enabled_flag;
                                    au4_ilf_across_tile_slice_enable[6] = (ps_slice_hdr_base + au4_idx_t[6])->i1_slice_loop_filter_across_slices_enabled_flag;
                                }



                                au4_ilf_across_tile_slice_enable[5] = (ps_slice_hdr_base + idx_t)->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[2] = (ps_slice_hdr_base + idx_t)->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[1] = (ps_slice_hdr_base + au4_idx_t[1])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[3] = (ps_slice_hdr_base + au4_idx_t[3])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[7] = (ps_slice_hdr_base + au4_idx_t[7])->i1_slice_loop_filter_across_slices_enabled_flag;

                                if(au4_idx_t[6] < idx_t)
                                {
                                    au4_ilf_across_tile_slice_enable[6] = (ps_slice_hdr_base + idx_t)->i1_slice_loop_filter_across_slices_enabled_flag;
                                }

                                /*
                                 * Between each neighbor and the current CTB, the i1_slice_loop_filter_across_slices_enabled_flag
                                 * of the pixel having a greater address is checked. Accordingly, set the availability flags
                                 */

                                for(i = 0; i < 8; i++)
                                {
                                    /*Sets the edges that lie on the slice/tile boundary*/
                                    if(au4_idx_t[i] != idx_t)
                                    {
                                        au1_tile_slice_boundary[i] = 1;
                                        /*Check for slice flag at such boundaries*/
                                    }
                                    else
                                    {
                                        au4_ilf_across_tile_slice_enable[i] = 1;
                                    }
                                }
                                /*Reset indices*/
                                for(i = 0; i < 8; i++)
                                {
                                    au4_idx_t[i] = 0;
                                }
                            }

                            if(ps_pps->i1_tiles_enabled_flag)
                            {
                                /* Calculate availability flags at slice boundary */
                                if(((ps_tile->u1_pos_x == ps_sao_ctxt->i4_ctb_x) || (ps_tile->u1_pos_y == ps_sao_ctxt->i4_ctb_y)) && (!((0 == ps_tile->u1_pos_x) && (0 == ps_tile->u1_pos_y))))
                                {
                                    /*If ilf across tiles is enabled, boundary availability for tiles is not checked. */
                                    if(!ps_pps->i1_loop_filter_across_tiles_enabled_flag)
                                    {
                                        /*Calculate neighbor ctb slice indices*/
                                        if(0 == ps_sao_ctxt->i4_ctb_x)
                                        {
                                            au4_idx_t[0] = -1;
                                            au4_idx_t[6] = -1;
                                            au4_idx_t[4] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_t[0] = au4_idx_t[4] = pu1_tile_idx[ctbx_t_l + (ctby_t_l * ps_sps->i2_pic_wd_in_ctb)];
                                            au4_idx_t[6] = pu1_tile_idx[ctbx_t_d - 1 + (ctby_t_d * ps_sps->i2_pic_wd_in_ctb)];
                                        }
                                        idx_t   = pu1_tile_idx[ctbx_t + (ctby_t * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_t[2] = au4_idx_t[5] = pu1_tile_idx[ctbx_t_t + (ctby_t_t * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_t[1] = pu1_tile_idx[ctbx_t_r + (ctby_t_r * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_t[3] = au4_idx_t[7] = pu1_tile_idx[ctbx_t_d + (ctby_t_d * ps_sps->i2_pic_wd_in_ctb)];

                                        for(i = 0; i < 8; i++)
                                        {
                                            /*Sets the edges that lie on the tile boundary*/
                                            if(au4_idx_t[i] != idx_t)
                                            {
                                                au1_tile_slice_boundary[i] |= 1;
                                                au4_ilf_across_tile_slice_enable[i] &= ps_pps->i1_loop_filter_across_tiles_enabled_flag;
                                            }
                                        }
                                    }
                                }
                            }

                            for(i = 0; i < 8; i++)
                            {
                                /*Sets the edges that lie on the slice/tile boundary*/
                                if((au1_tile_slice_boundary[i]) && !(au4_ilf_across_tile_slice_enable[i]))
                                {
                                    au1_avail_luma[i] = 0;
                                }
                            }
                        }
                    }


                    if(0 == ps_sao_ctxt->i4_ctb_x)
                    {
                        au1_avail_luma[0] = 0;
                        au1_avail_luma[4] = 0;
                        au1_avail_luma[6] = 0;
                    }

                    if(ps_sps->i2_pic_width_in_luma_samples - (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) <= sao_wd_luma)
                    {
                        au1_avail_luma[1] = 0;
                        au1_avail_luma[5] = 0;
                        au1_avail_luma[7] = 0;
                    }

                    if(0 == (ps_sao_ctxt->i4_ctb_y << log2_ctb_size) - sao_ht_luma)
                    {
                        au1_avail_luma[2] = 0;
                        au1_avail_luma[4] = 0;
                        au1_avail_luma[5] = 0;
                    }

                    if(ps_sps->i2_pic_ht_in_ctb == ps_sao_ctxt->i4_ctb_y)
                    {
                        au1_avail_luma[3] = 0;
                        au1_avail_luma[6] = 0;
                        au1_avail_luma[7] = 0;
                    }

                    {
                        au1_src_top_right[0] = pu1_sao_src_top_left_luma_top_right[0];
                        u1_sao_src_top_left_luma_bot_left = pu1_src_luma[sao_ht_luma * src_strd - 1];
                        ps_codec->apf_sao_luma[ps_sao->b3_y_type_idx - 2](pu1_src_luma,
                                                                          src_strd,
                                                                          pu1_src_left_luma,
                                                                          pu1_src_top_luma,
                                                                          pu1_sao_src_luma_top_left_ctb,
                                                                          au1_src_top_right,
                                                                          &u1_sao_src_top_left_luma_bot_left,
                                                                          au1_avail_luma,
                                                                          ai1_offset_y,
                                                                          sao_wd_luma,
                                                                          sao_ht_luma);
                    }
                }
            }
            else if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
            {
                /* Update left, top and top-left */
                for(row = 0; row < sao_ht_luma; row++)
                {
                    pu1_src_left_luma[row] = pu1_src_luma[row * src_strd + (sao_wd_luma - 1)];
                }
                pu1_sao_src_luma_top_left_ctb[0] = pu1_src_top_luma[sao_wd_luma - 1];

                ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_luma, &pu1_src_luma[(sao_ht_luma - 1) * src_strd], sao_wd_luma);
            }
        }

        if(0 != sao_wd_chroma)
        {
            if(ps_slice_hdr_top->i1_slice_sao_chroma_flag)
            {
                if(0 == ps_sao->b3_cb_type_idx)
                {

                    for(row = 0; row < sao_ht_chroma; row++)
                    {
                        pu1_src_left_chroma[2 * row] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 2)];
                        pu1_src_left_chroma[2 * row + 1] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 1)];
                    }
                    pu1_sao_src_chroma_top_left_ctb[0] = pu1_src_top_chroma[sao_wd_chroma - 2];
                    pu1_sao_src_chroma_top_left_ctb[1] = pu1_src_top_chroma[sao_wd_chroma - 1];

                    ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_chroma, &pu1_src_chroma[(sao_ht_chroma - 1) * src_strd], sao_wd_chroma);

                }

                else if(1 == ps_sao->b3_cb_type_idx)
                {
                    ai1_offset_cb[1] = ps_sao->b4_cb_offset_1;
                    ai1_offset_cb[2] = ps_sao->b4_cb_offset_2;
                    ai1_offset_cb[3] = ps_sao->b4_cb_offset_3;
                    ai1_offset_cb[4] = ps_sao->b4_cb_offset_4;

                    ai1_offset_cr[1] = ps_sao->b4_cr_offset_1;
                    ai1_offset_cr[2] = ps_sao->b4_cr_offset_2;
                    ai1_offset_cr[3] = ps_sao->b4_cr_offset_3;
                    ai1_offset_cr[4] = ps_sao->b4_cr_offset_4;

                    if(chroma_yuv420sp_vu)
                    {
                        ps_codec->s_func_selector.ihevc_sao_band_offset_chroma_fptr(pu1_src_chroma,
                                                                                    src_strd,
                                                                                    pu1_src_left_chroma,
                                                                                    pu1_src_top_chroma,
                                                                                    pu1_sao_src_chroma_top_left_ctb,
                                                                                    ps_sao->b5_cr_band_pos,
                                                                                    ps_sao->b5_cb_band_pos,
                                                                                    ai1_offset_cr,
                                                                                    ai1_offset_cb,
                                                                                    sao_wd_chroma,
                                                                                    sao_ht_chroma
                                                                                   );
                    }
                    else
                    {
                        ps_codec->s_func_selector.ihevc_sao_band_offset_chroma_fptr(pu1_src_chroma,
                                                                                    src_strd,
                                                                                    pu1_src_left_chroma,
                                                                                    pu1_src_top_chroma,
                                                                                    pu1_sao_src_chroma_top_left_ctb,
                                                                                    ps_sao->b5_cb_band_pos,
                                                                                    ps_sao->b5_cr_band_pos,
                                                                                    ai1_offset_cb,
                                                                                    ai1_offset_cr,
                                                                                    sao_wd_chroma,
                                                                                    sao_ht_chroma
                                                                                   );
                    }
                }
                else // if(2 <= ps_sao->b3_cb_type_idx)
                {
                    ai1_offset_cb[1] = ps_sao->b4_cb_offset_1;
                    ai1_offset_cb[2] = ps_sao->b4_cb_offset_2;
                    ai1_offset_cb[3] = ps_sao->b4_cb_offset_3;
                    ai1_offset_cb[4] = ps_sao->b4_cb_offset_4;

                    ai1_offset_cr[1] = ps_sao->b4_cr_offset_1;
                    ai1_offset_cr[2] = ps_sao->b4_cr_offset_2;
                    ai1_offset_cr[3] = ps_sao->b4_cr_offset_3;
                    ai1_offset_cr[4] = ps_sao->b4_cr_offset_4;

                    for(i = 0; i < 8; i++)
                    {
                        au1_avail_chroma[i] = 255;
                        au1_tile_slice_boundary[i] = 0;
                        au4_idx_t[i] = 0;
                        au4_ilf_across_tile_slice_enable[i] = 1;
                    }

                    {
                        if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
                        {
                            ctbx_t_t = ps_sao_ctxt->i4_ctb_x;
                            ctby_t_t = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_t_l = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_t_l = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_t_r = ps_sao_ctxt->i4_ctb_x;
                            ctby_t_r = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_t_d =  ps_sao_ctxt->i4_ctb_x;
                            ctby_t_d =  ps_sao_ctxt->i4_ctb_y;

                            ctbx_t = ps_sao_ctxt->i4_ctb_x;
                            ctby_t = ps_sao_ctxt->i4_ctb_y - 1;

                            if(!ps_slice_hdr->i1_first_slice_in_pic_flag)
                            {
                                if(0 == ps_sao_ctxt->i4_ctb_x)
                                {
                                    au4_idx_t[0] = -1;
                                    au4_idx_t[6] = -1;
                                    au4_idx_t[4] = -1;
                                }
                                else
                                {
                                    au4_idx_t[0] = au4_idx_t[4] = pu1_slice_idx[ctbx_t_l + (ctby_t_l * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_t[6] = pu1_slice_idx[ctbx_t_d - 1 + (ctby_t_d * ps_sps->i2_pic_wd_in_ctb)];
                                }
                                idx_t   = pu1_slice_idx[ctbx_t + (ctby_t * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_t[2] = au4_idx_t[5] = pu1_slice_idx[ctbx_t_t + (ctby_t_t * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_t[1] = pu1_slice_idx[ctbx_t_r + (ctby_t_r * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_t[3] = au4_idx_t[7] = pu1_slice_idx[ctbx_t_d + (ctby_t_d * ps_sps->i2_pic_wd_in_ctb)];

                                /*Verify that the neighbor ctbs don't cross pic boundary.*/

                                if(0 == ps_sao_ctxt->i4_ctb_x)
                                {
                                    au4_ilf_across_tile_slice_enable[4] = 0;
                                    au4_ilf_across_tile_slice_enable[6] = 0;
                                    au4_ilf_across_tile_slice_enable[0] = 0;
                                }
                                else
                                {
                                    au4_ilf_across_tile_slice_enable[4] = au4_ilf_across_tile_slice_enable[0] = (ps_slice_hdr_base + idx_t)->i1_slice_loop_filter_across_slices_enabled_flag;
                                    au4_ilf_across_tile_slice_enable[6] = (ps_slice_hdr_base + au4_idx_t[6])->i1_slice_loop_filter_across_slices_enabled_flag;
                                }

                                au4_ilf_across_tile_slice_enable[5] = (ps_slice_hdr_base + au4_idx_t[5])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[2] = (ps_slice_hdr_base + idx_t)->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[1] = (ps_slice_hdr_base + au4_idx_t[1])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[3] = (ps_slice_hdr_base + au4_idx_t[3])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[7] = (ps_slice_hdr_base + au4_idx_t[7])->i1_slice_loop_filter_across_slices_enabled_flag;

                                if(idx_t > au4_idx_t[6])
                                {
                                    au4_ilf_across_tile_slice_enable[6] = (ps_slice_hdr_base + idx_t)->i1_slice_loop_filter_across_slices_enabled_flag;
                                }

                                /*
                                 * Between each neighbor and the current CTB, the i1_slice_loop_filter_across_slices_enabled_flag
                                 * of the pixel having a greater address is checked. Accordingly, set the availability flags
                                 */
                                for(i = 0; i < 8; i++)
                                {
                                    /*Sets the edges that lie on the slice/tile boundary*/
                                    if(au4_idx_t[i] != idx_t)
                                    {
                                        au1_tile_slice_boundary[i] = 1;
                                    }
                                    else
                                    {
                                        /*Indicates that the neighbour belongs to same/dependent slice*/
                                        au4_ilf_across_tile_slice_enable[i] = 1;
                                    }
                                }
                                /*Reset indices*/
                                for(i = 0; i < 8; i++)
                                {
                                    au4_idx_t[i] = 0;
                                }
                            }
                            if(ps_pps->i1_tiles_enabled_flag)
                            {
                                /* Calculate availability flags at slice boundary */
                                if(((ps_tile->u1_pos_x == ps_sao_ctxt->i4_ctb_x) || (ps_tile->u1_pos_y == ps_sao_ctxt->i4_ctb_y)) && (!((0 == ps_tile->u1_pos_x) && (0 == ps_tile->u1_pos_y))))
                                {
                                    /*If ilf across tiles is enabled, boundary availability for tiles is not checked. */
                                    if(!ps_pps->i1_loop_filter_across_tiles_enabled_flag)
                                    {
                                        /*Calculate neighbor ctb slice indices*/
                                        if(0 == ps_sao_ctxt->i4_ctb_x)
                                        {
                                            au4_idx_t[0] = -1;
                                            au4_idx_t[6] = -1;
                                            au4_idx_t[4] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_t[0] = au4_idx_t[4] = pu1_tile_idx[ctbx_t_l + (ctby_t_l * ps_sps->i2_pic_wd_in_ctb)];
                                            au4_idx_t[6] = pu1_tile_idx[ctbx_t_d - 1 + (ctby_t_d * ps_sps->i2_pic_wd_in_ctb)];
                                        }
                                        idx_t   = pu1_tile_idx[ctbx_t + (ctby_t * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_t[2] = au4_idx_t[5] = pu1_tile_idx[ctbx_t_t + (ctby_t_t * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_t[1] = pu1_tile_idx[ctbx_t_r + (ctby_t_r * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_t[3] = au4_idx_t[7] = pu1_tile_idx[ctbx_t_d + (ctby_t_d * ps_sps->i2_pic_wd_in_ctb)];

                                        for(i = 0; i < 8; i++)
                                        {
                                            /*Sets the edges that lie on the tile boundary*/
                                            if(au4_idx_t[i] != idx_t)
                                            {
                                                au1_tile_slice_boundary[i] |= 1;
                                                au4_ilf_across_tile_slice_enable[i] &= ps_pps->i1_loop_filter_across_tiles_enabled_flag;
                                            }
                                        }
                                    }
                                }
                            }
                            for(i = 0; i < 8; i++)
                            {
                                /*Sets the edges that lie on the slice/tile boundary*/
                                if((au1_tile_slice_boundary[i]) && !(au4_ilf_across_tile_slice_enable[i]))
                                {
                                    au1_avail_chroma[i] = 0;
                                }
                            }

                        }
                    }
                    if(0 == ps_sao_ctxt->i4_ctb_x)
                    {
                        au1_avail_chroma[0] = 0;
                        au1_avail_chroma[4] = 0;
                        au1_avail_chroma[6] = 0;
                    }

                    if(ps_sps->i2_pic_width_in_luma_samples - (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) <= sao_wd_chroma)
                    {
                        au1_avail_chroma[1] = 0;
                        au1_avail_chroma[5] = 0;
                        au1_avail_chroma[7] = 0;
                    }

                    if(0 == (ps_sao_ctxt->i4_ctb_y << (log2_ctb_size - 1)) - sao_ht_chroma)
                    {
                        au1_avail_chroma[2] = 0;
                        au1_avail_chroma[4] = 0;
                        au1_avail_chroma[5] = 0;
                    }

                    if(ps_sps->i2_pic_ht_in_ctb == ps_sao_ctxt->i4_ctb_y)
                    {
                        au1_avail_chroma[3] = 0;
                        au1_avail_chroma[6] = 0;
                        au1_avail_chroma[7] = 0;
                    }

                    {
                        au1_src_top_right[0] = pu1_sao_src_top_left_chroma_top_right[0];
                        au1_src_top_right[1] = pu1_sao_src_top_left_chroma_top_right[1];
                        au1_sao_src_top_left_chroma_bot_left[0] = pu1_src_chroma[sao_ht_chroma * src_strd - 2];
                        au1_sao_src_top_left_chroma_bot_left[1] = pu1_src_chroma[sao_ht_chroma * src_strd - 1];

                        if(chroma_yuv420sp_vu)
                        {
                            ps_codec->apf_sao_chroma[ps_sao->b3_cb_type_idx - 2](pu1_src_chroma,
                                                                                 src_strd,
                                                                                 pu1_src_left_chroma,
                                                                                 pu1_src_top_chroma,
                                                                                 pu1_sao_src_chroma_top_left_ctb,
                                                                                 au1_src_top_right,
                                                                                 au1_sao_src_top_left_chroma_bot_left,
                                                                                 au1_avail_chroma,
                                                                                 ai1_offset_cr,
                                                                                 ai1_offset_cb,
                                                                                 sao_wd_chroma,
                                                                                 sao_ht_chroma);
                        }
                        else
                        {
                            ps_codec->apf_sao_chroma[ps_sao->b3_cb_type_idx - 2](pu1_src_chroma,
                                                                                 src_strd,
                                                                                 pu1_src_left_chroma,
                                                                                 pu1_src_top_chroma,
                                                                                 pu1_sao_src_chroma_top_left_ctb,
                                                                                 au1_src_top_right,
                                                                                 au1_sao_src_top_left_chroma_bot_left,
                                                                                 au1_avail_chroma,
                                                                                 ai1_offset_cb,
                                                                                 ai1_offset_cr,
                                                                                 sao_wd_chroma,
                                                                                 sao_ht_chroma);
                        }
                    }

                }
            }
            else if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
            {
                for(row = 0; row < sao_ht_chroma; row++)
                {
                    pu1_src_left_chroma[2 * row] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 2)];
                    pu1_src_left_chroma[2 * row + 1] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 1)];
                }
                pu1_sao_src_chroma_top_left_ctb[0] = pu1_src_top_chroma[sao_wd_chroma - 2];
                pu1_sao_src_chroma_top_left_ctb[1] = pu1_src_top_chroma[sao_wd_chroma - 1];

                ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_chroma, &pu1_src_chroma[(sao_ht_chroma - 1) * src_strd], sao_wd_chroma);
            }
        }

        pu1_src_luma += sao_ht_luma * src_strd;
        pu1_src_chroma += sao_ht_chroma * src_strd;
        ps_sao += (ps_sps->i2_pic_wd_in_ctb);
    }

    /* Left CTB */
    if(ps_sao_ctxt->i4_ctb_x > 0)
    {
        WORD32 sao_wd_luma = SAO_SHIFT_CTB;
        WORD32 sao_wd_chroma = 2 * SAO_SHIFT_CTB;
        WORD32 sao_ht_luma = ctb_size - SAO_SHIFT_CTB;
        WORD32 sao_ht_chroma = ctb_size / 2 - SAO_SHIFT_CTB;

        WORD32 ctbx_l_t = 0, ctbx_l_l = 0, ctbx_l_r = 0, ctbx_l_d = 0, ctbx_l = 0;
        WORD32 ctby_l_t = 0, ctby_l_l = 0, ctby_l_r = 0, ctby_l_d = 0, ctby_l = 0;
        WORD32 au4_idx_l[8], idx_l;

        WORD32 remaining_rows;
        slice_header_t *ps_slice_hdr_left;
        {
            WORD32 left_ctb_indx = (ps_sao_ctxt->i4_ctb_y) * ps_sps->i2_pic_wd_in_ctb +
                                        (ps_sao_ctxt->i4_ctb_x - 1);
            ps_slice_hdr_left = ps_slice_hdr_base + pu1_slice_idx[left_ctb_indx];
        }

        remaining_rows = ps_sps->i2_pic_height_in_luma_samples - ((ps_sao_ctxt->i4_ctb_y << log2_ctb_size) + sao_ht_luma);
        if(remaining_rows <= SAO_SHIFT_CTB)
        {
            sao_ht_luma += remaining_rows;
        }
        remaining_rows = ps_sps->i2_pic_height_in_luma_samples / 2 - ((ps_sao_ctxt->i4_ctb_y << (log2_ctb_size - 1)) + sao_ht_chroma);
        if(remaining_rows <= SAO_SHIFT_CTB)
        {
            sao_ht_chroma += remaining_rows;
        }

        pu1_src_luma -= sao_wd_luma;
        pu1_src_chroma -= sao_wd_chroma;
        ps_sao -= 1;
        pu1_src_top_luma = ps_sao_ctxt->pu1_sao_src_top_luma + (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_luma;
        pu1_src_top_chroma = ps_sao_ctxt->pu1_sao_src_top_chroma + (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_chroma;
        pu1_src_left_luma = ps_sao_ctxt->pu1_sao_src_left_luma + (ps_sao_ctxt->i4_ctb_y << log2_ctb_size);
        pu1_src_left_chroma = ps_sao_ctxt->pu1_sao_src_left_chroma + (ps_sao_ctxt->i4_ctb_y << log2_ctb_size);


        if(0 != sao_ht_luma)
        {
            if(ps_slice_hdr_left->i1_slice_sao_luma_flag)
            {
                if(0 == ps_sao->b3_y_type_idx)
                {
                    /* Update left, top and top-left */
                    for(row = 0; row < sao_ht_luma; row++)
                    {
                        pu1_src_left_luma[row] = pu1_src_luma[row * src_strd + (sao_wd_luma - 1)];
                    }
                    /*Update in next location*/
                    pu1_sao_src_top_left_luma_curr_ctb[0] = pu1_src_top_luma[sao_wd_luma - 1];

                    ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_luma, &pu1_src_luma[(sao_ht_luma - 1) * src_strd], sao_wd_luma);

                }

                else if(1 == ps_sao->b3_y_type_idx)
                {
                    ai1_offset_y[1] = ps_sao->b4_y_offset_1;
                    ai1_offset_y[2] = ps_sao->b4_y_offset_2;
                    ai1_offset_y[3] = ps_sao->b4_y_offset_3;
                    ai1_offset_y[4] = ps_sao->b4_y_offset_4;

                    ps_codec->s_func_selector.ihevc_sao_band_offset_luma_fptr(pu1_src_luma,
                                                                              src_strd,
                                                                              pu1_src_left_luma,
                                                                              pu1_src_top_luma,
                                                                              pu1_sao_src_top_left_luma_curr_ctb,
                                                                              ps_sao->b5_y_band_pos,
                                                                              ai1_offset_y,
                                                                              sao_wd_luma,
                                                                              sao_ht_luma
                                                                             );
                }

                else // if(2 <= ps_sao->b3_y_type_idx)
                {
                    ai1_offset_y[1] = ps_sao->b4_y_offset_1;
                    ai1_offset_y[2] = ps_sao->b4_y_offset_2;
                    ai1_offset_y[3] = ps_sao->b4_y_offset_3;
                    ai1_offset_y[4] = ps_sao->b4_y_offset_4;

                    for(i = 0; i < 8; i++)
                    {
                        au1_avail_luma[i] = 255;
                        au1_tile_slice_boundary[i] = 0;
                        au4_idx_l[i] = 0;
                        au4_ilf_across_tile_slice_enable[i] = 1;
                    }
                    /******************************************************************
                     * Derive the  Top-left CTB's neighbour pixel's slice indices.
                     *
                     *
                     *          ____________
                     *         |    |       |
                     *         | L_T|       |
                     *         |____|_______|____
                     *         |    |       |    |
                     *     L_L |  L |  L_R  |    |
                     *         |____|_______|    |
                     *              |            |
                     *          L_D |            |
                     *              |____________|
                     *
                     *****************************************************************/

                    /*In case of slices or tiles*/
                    {
                        if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
                        {
                            ctbx_l_t = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_l_t = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_l_l = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_l_l = ps_sao_ctxt->i4_ctb_y;

                            ctbx_l_r = ps_sao_ctxt->i4_ctb_x;
                            ctby_l_r = ps_sao_ctxt->i4_ctb_y;

                            ctbx_l_d =  ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_l_d =  ps_sao_ctxt->i4_ctb_y;

                            ctbx_l = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_l = ps_sao_ctxt->i4_ctb_y;

                            if(!ps_slice_hdr->i1_first_slice_in_pic_flag)
                            {
                                if(0 == ps_sao_ctxt->i4_ctb_y)
                                {
                                    au4_idx_l[2] = -1;
                                    au4_idx_l[4] = -1;
                                    au4_idx_l[5] = -1;
                                }
                                else
                                {
                                    au4_idx_l[2] = au4_idx_l[4] = pu1_slice_idx[ctbx_l_t + (ctby_l_t * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_l[5] =  pu1_slice_idx[ctbx_l_t + 1 + (ctby_l_t  * ps_sps->i2_pic_wd_in_ctb)];
                                }
                                idx_l   = au4_idx_l[6] = pu1_slice_idx[ctbx_l + (ctby_l * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_l[0] = pu1_slice_idx[ctbx_l_l + (ctby_l_l * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_l[1] = au4_idx_l[7] = pu1_slice_idx[ctbx_l_r + (ctby_l_r * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_l[3] = pu1_slice_idx[ctbx_l_d + (ctby_l_d * ps_sps->i2_pic_wd_in_ctb)];

                                /*Verify that the neighbor ctbs don't cross pic boundary.*/
                                if(0 == ps_sao_ctxt->i4_ctb_y)
                                {
                                    au4_ilf_across_tile_slice_enable[2] = 0;
                                    au4_ilf_across_tile_slice_enable[4] = 0;
                                    au4_ilf_across_tile_slice_enable[5] = 0;
                                }
                                else
                                {
                                    au4_ilf_across_tile_slice_enable[2] =  (ps_slice_hdr_base + idx_l)->i1_slice_loop_filter_across_slices_enabled_flag;
                                    au4_ilf_across_tile_slice_enable[5] = au4_ilf_across_tile_slice_enable[4] = au4_ilf_across_tile_slice_enable[2];

                                }
                                //TODO: ILF flag checks for [0] and [6] is missing.
                                au4_ilf_across_tile_slice_enable[1] = (ps_slice_hdr_base + au4_idx_l[1])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[3] = (ps_slice_hdr_base + au4_idx_l[3])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[7] = (ps_slice_hdr_base + au4_idx_l[7])->i1_slice_loop_filter_across_slices_enabled_flag;

                                if(idx_l < au4_idx_l[5])
                                {
                                    au4_ilf_across_tile_slice_enable[5] = (ps_slice_hdr_base + au4_idx_l[5])->i1_slice_loop_filter_across_slices_enabled_flag;
                                }

                                /*
                                 * Between each neighbor and the current CTB, the i1_slice_loop_filter_across_slices_enabled_flag
                                 * of the pixel having a greater address is checked. Accordingly, set the availability flags
                                 */
                                for(i = 0; i < 8; i++)
                                {
                                    /*Sets the edges that lie on the slice/tile boundary*/
                                    if(au4_idx_l[i] != idx_l)
                                    {
                                        au1_tile_slice_boundary[i] = 1;
                                    }
                                    else
                                    {
                                        au4_ilf_across_tile_slice_enable[i] = 1;
                                    }
                                }
                                /*Reset indices*/
                                for(i = 0; i < 8; i++)
                                {
                                    au4_idx_l[i] = 0;
                                }
                            }

                            if(ps_pps->i1_tiles_enabled_flag)
                            {
                                /* Calculate availability flags at slice boundary */
                                if(((ps_tile->u1_pos_x == ps_sao_ctxt->i4_ctb_x) || (ps_tile->u1_pos_y == ps_sao_ctxt->i4_ctb_y)) && (!((0 == ps_tile->u1_pos_x) && (0 == ps_tile->u1_pos_y))))
                                {
                                    /*If ilf across tiles is enabled, boundary availability for tiles is not checked. */
                                    if(!ps_pps->i1_loop_filter_across_tiles_enabled_flag)
                                    {
                                        if(0 == ps_sao_ctxt->i4_ctb_y)
                                        {
                                            au4_idx_l[2] = -1;
                                            au4_idx_l[4] = -1;
                                            au4_idx_l[5] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_l[2] = au4_idx_l[4] = pu1_tile_idx[ctbx_l_t + (ctby_l_t * ps_sps->i2_pic_wd_in_ctb)];
                                            au4_idx_l[5] =  pu1_tile_idx[ctbx_l_t + 1 + (ctby_l_t  * ps_sps->i2_pic_wd_in_ctb)];
                                        }

                                        idx_l   = au4_idx_l[6] = pu1_tile_idx[ctbx_l + (ctby_l * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_l[0] = pu1_tile_idx[ctbx_l_l + (ctby_l_l * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_l[1] = au4_idx_l[7] = pu1_tile_idx[ctbx_l_r + (ctby_l_r * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_l[3] = pu1_tile_idx[ctbx_l_d + (ctby_l_d * ps_sps->i2_pic_wd_in_ctb)];

                                        for(i = 0; i < 8; i++)
                                        {
                                            /*Sets the edges that lie on the slice/tile boundary*/
                                            if(au4_idx_l[i] != idx_l)
                                            {
                                                au1_tile_slice_boundary[i] |= 1;
                                                au4_ilf_across_tile_slice_enable[i] &= ps_pps->i1_loop_filter_across_tiles_enabled_flag;
                                            }
                                        }
                                    }
                                }
                            }

                            for(i = 0; i < 8; i++)
                            {
                                /*Sets the edges that lie on the slice/tile boundary*/
                                if((au1_tile_slice_boundary[i]) && !(au4_ilf_across_tile_slice_enable[i]))
                                {
                                    au1_avail_luma[i] = 0;
                                }
                            }
                        }
                    }
                    if(0 == (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_luma)
                    {
                        au1_avail_luma[0] = 0;
                        au1_avail_luma[4] = 0;
                        au1_avail_luma[6] = 0;
                    }
                    if(ps_sps->i2_pic_wd_in_ctb == ps_sao_ctxt->i4_ctb_x)
                    {
                        au1_avail_luma[1] = 0;
                        au1_avail_luma[5] = 0;
                        au1_avail_luma[7] = 0;
                    }

                    if(0 == ps_sao_ctxt->i4_ctb_y)
                    {
                        au1_avail_luma[2] = 0;
                        au1_avail_luma[4] = 0;
                        au1_avail_luma[5] = 0;
                    }

                    if(ps_sps->i2_pic_height_in_luma_samples - (ps_sao_ctxt->i4_ctb_y  << log2_ctb_size) <= sao_ht_luma)
                    {
                        au1_avail_luma[3] = 0;
                        au1_avail_luma[6] = 0;
                        au1_avail_luma[7] = 0;
                    }

                    {
                        au1_src_top_right[0] = pu1_src_top_luma[sao_wd_luma];
                        u1_sao_src_top_left_luma_bot_left = pu1_sao_src_top_left_luma_bot_left[0];
                        ps_codec->apf_sao_luma[ps_sao->b3_y_type_idx - 2](pu1_src_luma,
                                                                          src_strd,
                                                                          pu1_src_left_luma,
                                                                          pu1_src_top_luma,
                                                                          pu1_sao_src_top_left_luma_curr_ctb,
                                                                          au1_src_top_right,
                                                                          &u1_sao_src_top_left_luma_bot_left,
                                                                          au1_avail_luma,
                                                                          ai1_offset_y,
                                                                          sao_wd_luma,
                                                                          sao_ht_luma);
                    }

                }
            }
            else if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
            {
                /* Update left, top and top-left */
                for(row = 0; row < sao_ht_luma; row++)
                {
                    pu1_src_left_luma[row] = pu1_src_luma[row * src_strd + (sao_wd_luma - 1)];
                }
                /*Update in next location*/
                pu1_sao_src_top_left_luma_curr_ctb[0] = pu1_src_top_luma[sao_wd_luma - 1];

                ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_luma, &pu1_src_luma[(sao_ht_luma - 1) * src_strd], sao_wd_luma);
            }
        }

        if(0 != sao_ht_chroma)
        {
            if(ps_slice_hdr_left->i1_slice_sao_chroma_flag)
            {
                if(0 == ps_sao->b3_cb_type_idx)
                {
                    for(row = 0; row < sao_ht_chroma; row++)
                    {
                        pu1_src_left_chroma[2 * row] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 2)];
                        pu1_src_left_chroma[2 * row + 1] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 1)];
                    }
                    pu1_sao_src_top_left_chroma_curr_ctb[0] = pu1_src_top_chroma[sao_wd_chroma - 2];
                    pu1_sao_src_top_left_chroma_curr_ctb[1] = pu1_src_top_chroma[sao_wd_chroma - 1];

                    ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_chroma, &pu1_src_chroma[(sao_ht_chroma - 1) * src_strd], sao_wd_chroma);
                }

                else if(1 == ps_sao->b3_cb_type_idx)
                {
                    ai1_offset_cb[1] = ps_sao->b4_cb_offset_1;
                    ai1_offset_cb[2] = ps_sao->b4_cb_offset_2;
                    ai1_offset_cb[3] = ps_sao->b4_cb_offset_3;
                    ai1_offset_cb[4] = ps_sao->b4_cb_offset_4;

                    ai1_offset_cr[1] = ps_sao->b4_cr_offset_1;
                    ai1_offset_cr[2] = ps_sao->b4_cr_offset_2;
                    ai1_offset_cr[3] = ps_sao->b4_cr_offset_3;
                    ai1_offset_cr[4] = ps_sao->b4_cr_offset_4;

                    if(chroma_yuv420sp_vu)
                    {
                        ps_codec->s_func_selector.ihevc_sao_band_offset_chroma_fptr(pu1_src_chroma,
                                                                                    src_strd,
                                                                                    pu1_src_left_chroma,
                                                                                    pu1_src_top_chroma,
                                                                                    pu1_sao_src_top_left_chroma_curr_ctb,
                                                                                    ps_sao->b5_cr_band_pos,
                                                                                    ps_sao->b5_cb_band_pos,
                                                                                    ai1_offset_cr,
                                                                                    ai1_offset_cb,
                                                                                    sao_wd_chroma,
                                                                                    sao_ht_chroma
                                                                                   );
                    }
                    else
                    {
                        ps_codec->s_func_selector.ihevc_sao_band_offset_chroma_fptr(pu1_src_chroma,
                                                                                    src_strd,
                                                                                    pu1_src_left_chroma,
                                                                                    pu1_src_top_chroma,
                                                                                    pu1_sao_src_top_left_chroma_curr_ctb,
                                                                                    ps_sao->b5_cb_band_pos,
                                                                                    ps_sao->b5_cr_band_pos,
                                                                                    ai1_offset_cb,
                                                                                    ai1_offset_cr,
                                                                                    sao_wd_chroma,
                                                                                    sao_ht_chroma
                                                                                   );
                    }
                }

                else // if(2 <= ps_sao->b3_cb_type_idx)
                {
                    ai1_offset_cb[1] = ps_sao->b4_cb_offset_1;
                    ai1_offset_cb[2] = ps_sao->b4_cb_offset_2;
                    ai1_offset_cb[3] = ps_sao->b4_cb_offset_3;
                    ai1_offset_cb[4] = ps_sao->b4_cb_offset_4;

                    ai1_offset_cr[1] = ps_sao->b4_cr_offset_1;
                    ai1_offset_cr[2] = ps_sao->b4_cr_offset_2;
                    ai1_offset_cr[3] = ps_sao->b4_cr_offset_3;
                    ai1_offset_cr[4] = ps_sao->b4_cr_offset_4;

                    for(i = 0; i < 8; i++)
                    {
                        au1_avail_chroma[i] = 255;
                        au1_tile_slice_boundary[i] = 0;
                        au4_idx_l[i] = 0;
                        au4_ilf_across_tile_slice_enable[i] = 1;
                    }
                    /*In case of slices*/
                    {
                        if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
                        {
                            ctbx_l_t = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_l_t = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_l_l = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_l_l = ps_sao_ctxt->i4_ctb_y;

                            ctbx_l_r = ps_sao_ctxt->i4_ctb_x;
                            ctby_l_r = ps_sao_ctxt->i4_ctb_y;

                            ctbx_l_d =  ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_l_d =  ps_sao_ctxt->i4_ctb_y;

                            ctbx_l = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_l = ps_sao_ctxt->i4_ctb_y;

                            if(!ps_slice_hdr->i1_first_slice_in_pic_flag)
                            {
                                if(0 == ps_sao_ctxt->i4_ctb_y)
                                {
                                    au4_idx_l[2] = -1;
                                    au4_idx_l[4] = -1;
                                    au4_idx_l[5] = -1;
                                }
                                else
                                {
                                    au4_idx_l[2] = au4_idx_l[4] = pu1_slice_idx[ctbx_l_t + (ctby_l_t * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_l[5] =  pu1_slice_idx[ctbx_l_t + 1 + (ctby_l_t  * ps_sps->i2_pic_wd_in_ctb)];
                                }
                                idx_l   = au4_idx_l[6] = pu1_slice_idx[ctbx_l + (ctby_l * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_l[0] = pu1_slice_idx[ctbx_l_l + (ctby_l_l * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_l[1] = au4_idx_l[7] = pu1_slice_idx[ctbx_l_r + (ctby_l_r * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_l[3] = pu1_slice_idx[ctbx_l_d + (ctby_l_d * ps_sps->i2_pic_wd_in_ctb)];

                                /*Verify that the neighbour ctbs dont cross pic boundary.*/
                                if(0 == ps_sao_ctxt->i4_ctb_y)
                                {
                                    au4_ilf_across_tile_slice_enable[2] = 0;
                                    au4_ilf_across_tile_slice_enable[4] = 0;
                                    au4_ilf_across_tile_slice_enable[5] = 0;
                                }
                                else
                                {
                                    au4_ilf_across_tile_slice_enable[2] =  (ps_slice_hdr_base + idx_l)->i1_slice_loop_filter_across_slices_enabled_flag;
                                    au4_ilf_across_tile_slice_enable[5] = au4_ilf_across_tile_slice_enable[4] = au4_ilf_across_tile_slice_enable[2];
                                }

                                if(au4_idx_l[5] > idx_l)
                                {
                                    au4_ilf_across_tile_slice_enable[5] = (ps_slice_hdr_base + au4_idx_l[5])->i1_slice_loop_filter_across_slices_enabled_flag;
                                }

                                //  au4_ilf_across_tile_slice_enable[5] = au4_ilf_across_tile_slice_enable[2] = (ps_slice_hdr_base + idx_l)->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[1] = (ps_slice_hdr_base + au4_idx_l[1])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[3] = (ps_slice_hdr_base + au4_idx_l[3])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[7] = (ps_slice_hdr_base + au4_idx_l[7])->i1_slice_loop_filter_across_slices_enabled_flag;
                                /*
                                 * Between each neighbor and the current CTB, the i1_slice_loop_filter_across_slices_enabled_flag
                                 * of the pixel having a greater address is checked. Accordingly, set the availability flags
                                 */
                                for(i = 0; i < 8; i++)
                                {
                                    /*Sets the edges that lie on the slice/tile boundary*/
                                    if(au4_idx_l[i] != idx_l)
                                    {
                                        au1_tile_slice_boundary[i] = 1;
                                    }
                                    else
                                    {
                                        au4_ilf_across_tile_slice_enable[i] = 1;
                                    }
                                }
                                /*Reset indices*/
                                for(i = 0; i < 8; i++)
                                {
                                    au4_idx_l[i] = 0;
                                }
                            }
                            if(ps_pps->i1_tiles_enabled_flag)
                            {
                                /* Calculate availability flags at slice boundary */
                                if(((ps_tile->u1_pos_x == ps_sao_ctxt->i4_ctb_x) || (ps_tile->u1_pos_y == ps_sao_ctxt->i4_ctb_y)) && (!((0 == ps_tile->u1_pos_x) && (0 == ps_tile->u1_pos_y))))
                                {
                                    /*If ilf across tiles is enabled, boundary availability for tiles is not checked. */
                                    if(!ps_pps->i1_loop_filter_across_tiles_enabled_flag)
                                    {
                                        if(0 == ps_sao_ctxt->i4_ctb_y)
                                        {
                                            au4_idx_l[2] = -1;
                                            au4_idx_l[4] = -1;
                                            au4_idx_l[5] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_l[2] = au4_idx_l[4] = pu1_tile_idx[ctbx_l_t + (ctby_l_t * ps_sps->i2_pic_wd_in_ctb)];
                                            au4_idx_l[5] =  pu1_tile_idx[ctbx_l_t + 1 + (ctby_l_t  * ps_sps->i2_pic_wd_in_ctb)];
                                        }

                                        idx_l   = au4_idx_l[6] = pu1_tile_idx[ctbx_l + (ctby_l * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_l[0] = pu1_tile_idx[ctbx_l_l + (ctby_l_l * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_l[1] = au4_idx_l[7] = pu1_tile_idx[ctbx_l_r + (ctby_l_r * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_l[3] = pu1_tile_idx[ctbx_l_d + (ctby_l_d * ps_sps->i2_pic_wd_in_ctb)];

                                        for(i = 0; i < 8; i++)
                                        {
                                            /*Sets the edges that lie on the slice/tile boundary*/
                                            if(au4_idx_l[i] != idx_l)
                                            {
                                                au1_tile_slice_boundary[i] |= 1;
                                                au4_ilf_across_tile_slice_enable[i] &= ps_pps->i1_loop_filter_across_tiles_enabled_flag; //=0
                                            }
                                        }
                                    }
                                }
                            }
                            for(i = 0; i < 8; i++)
                            {
                                /*Sets the edges that lie on the slice/tile boundary*/
                                if((au1_tile_slice_boundary[i]) && !(au4_ilf_across_tile_slice_enable[i]))
                                {
                                    au1_avail_chroma[i] = 0;
                                }
                            }
                        }
                    }
                    if(0 == (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) - sao_wd_chroma)
                    {
                        au1_avail_chroma[0] = 0;
                        au1_avail_chroma[4] = 0;
                        au1_avail_chroma[6] = 0;
                    }

                    if(ps_sps->i2_pic_wd_in_ctb == ps_sao_ctxt->i4_ctb_x)
                    {
                        au1_avail_chroma[1] = 0;
                        au1_avail_chroma[5] = 0;
                        au1_avail_chroma[7] = 0;
                    }

                    if(0 == ps_sao_ctxt->i4_ctb_y)
                    {
                        au1_avail_chroma[2] = 0;
                        au1_avail_chroma[4] = 0;
                        au1_avail_chroma[5] = 0;
                    }

                    if(ps_sps->i2_pic_height_in_luma_samples / 2 - (ps_sao_ctxt->i4_ctb_y  << (log2_ctb_size - 1)) <= sao_ht_chroma)
                    {
                        au1_avail_chroma[3] = 0;
                        au1_avail_chroma[6] = 0;
                        au1_avail_chroma[7] = 0;
                    }

                    {
                        au1_src_top_right[0] = pu1_src_top_chroma[sao_wd_chroma];
                        au1_src_top_right[1] = pu1_src_top_chroma[sao_wd_chroma + 1];
                        au1_src_bot_left[0] = pu1_sao_src_top_left_chroma_bot_left[0];
                        au1_src_bot_left[1] = pu1_sao_src_top_left_chroma_bot_left[1];
                        //au1_src_bot_left[0] = pu1_src_chroma[sao_ht_chroma * src_strd - 2];
                        //au1_src_bot_left[1] = pu1_src_chroma[sao_ht_chroma * src_strd - 1];
                        if((ctb_size == 16) && (ps_sao_ctxt->i4_ctb_x != ps_sps->i2_pic_wd_in_ctb - 1))
                        {
                            au1_src_top_right[0] = pu1_src_chroma[sao_wd_chroma - src_strd];
                            au1_src_top_right[1] = pu1_src_chroma[sao_wd_chroma - src_strd + 1];
                        }


                        if(chroma_yuv420sp_vu)
                        {
                            ps_codec->apf_sao_chroma[ps_sao->b3_cb_type_idx - 2](pu1_src_chroma,
                                                                                 src_strd,
                                                                                 pu1_src_left_chroma,
                                                                                 pu1_src_top_chroma,
                                                                                 pu1_sao_src_top_left_chroma_curr_ctb,
                                                                                 au1_src_top_right,
                                                                                 au1_src_bot_left,
                                                                                 au1_avail_chroma,
                                                                                 ai1_offset_cr,
                                                                                 ai1_offset_cb,
                                                                                 sao_wd_chroma,
                                                                                 sao_ht_chroma);
                        }
                        else
                        {
                            ps_codec->apf_sao_chroma[ps_sao->b3_cb_type_idx - 2](pu1_src_chroma,
                                                                                 src_strd,
                                                                                 pu1_src_left_chroma,
                                                                                 pu1_src_top_chroma,
                                                                                 pu1_sao_src_top_left_chroma_curr_ctb,
                                                                                 au1_src_top_right,
                                                                                 au1_src_bot_left,
                                                                                 au1_avail_chroma,
                                                                                 ai1_offset_cb,
                                                                                 ai1_offset_cr,
                                                                                 sao_wd_chroma,
                                                                                 sao_ht_chroma);
                        }
                    }

                }
            }
            else if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
            {
                for(row = 0; row < sao_ht_chroma; row++)
                {
                    pu1_src_left_chroma[2 * row] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 2)];
                    pu1_src_left_chroma[2 * row + 1] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 1)];
                }
                pu1_sao_src_top_left_chroma_curr_ctb[0] = pu1_src_top_chroma[sao_wd_chroma - 2];
                pu1_sao_src_top_left_chroma_curr_ctb[1] = pu1_src_top_chroma[sao_wd_chroma - 1];

                ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_chroma, &pu1_src_chroma[(sao_ht_chroma - 1) * src_strd], sao_wd_chroma);
            }

        }
        pu1_src_luma += sao_wd_luma;
        pu1_src_chroma += sao_wd_chroma;
        ps_sao += 1;
    }


    /* Current CTB */
    {
        WORD32 sao_wd_luma = ctb_size - SAO_SHIFT_CTB;
        WORD32 sao_wd_chroma = ctb_size - SAO_SHIFT_CTB * 2;
        WORD32 sao_ht_luma = ctb_size - SAO_SHIFT_CTB;
        WORD32 sao_ht_chroma = ctb_size / 2 - SAO_SHIFT_CTB;
        WORD32 ctbx_c_t = 0, ctbx_c_l = 0, ctbx_c_r = 0, ctbx_c_d = 0, ctbx_c = 0;
        WORD32 ctby_c_t = 0, ctby_c_l = 0, ctby_c_r = 0, ctby_c_d = 0, ctby_c = 0;
        WORD32 au4_idx_c[8], idx_c;

        WORD32 remaining_rows;
        WORD32 remaining_cols;

        remaining_cols = ps_sps->i2_pic_width_in_luma_samples - ((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) + sao_wd_luma);
        if(remaining_cols <= SAO_SHIFT_CTB)
        {
            sao_wd_luma += remaining_cols;
        }
        remaining_cols = ps_sps->i2_pic_width_in_luma_samples - ((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) + sao_wd_chroma);
        if(remaining_cols <= 2 * SAO_SHIFT_CTB)
        {
            sao_wd_chroma += remaining_cols;
        }

        remaining_rows = ps_sps->i2_pic_height_in_luma_samples - ((ps_sao_ctxt->i4_ctb_y << log2_ctb_size) + sao_ht_luma);
        if(remaining_rows <= SAO_SHIFT_CTB)
        {
            sao_ht_luma += remaining_rows;
        }
        remaining_rows = ps_sps->i2_pic_height_in_luma_samples / 2 - ((ps_sao_ctxt->i4_ctb_y << (log2_ctb_size - 1)) + sao_ht_chroma);
        if(remaining_rows <= SAO_SHIFT_CTB)
        {
            sao_ht_chroma += remaining_rows;
        }

        pu1_src_top_luma = ps_sao_ctxt->pu1_sao_src_top_luma + (ps_sao_ctxt->i4_ctb_x << log2_ctb_size);
        pu1_src_top_chroma = ps_sao_ctxt->pu1_sao_src_top_chroma + (ps_sao_ctxt->i4_ctb_x << log2_ctb_size);
        pu1_src_left_luma = ps_sao_ctxt->pu1_sao_src_left_luma + (ps_sao_ctxt->i4_ctb_y << log2_ctb_size);
        pu1_src_left_chroma = ps_sao_ctxt->pu1_sao_src_left_chroma + (ps_sao_ctxt->i4_ctb_y << log2_ctb_size);

        if((0 != sao_wd_luma) && (0 != sao_ht_luma))
        {
            if(ps_sao_ctxt->ps_slice_hdr->i1_slice_sao_luma_flag)
            {
                if(0 == ps_sao->b3_y_type_idx)
                {
                    /* Update left, top and top-left */
                    for(row = 0; row < sao_ht_luma; row++)
                    {
                        pu1_src_left_luma[row] = pu1_src_luma[row * src_strd + (sao_wd_luma - 1)];
                    }
                    pu1_sao_src_top_left_luma_curr_ctb[0] = pu1_src_top_luma[sao_wd_luma - 1];

                    ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_luma, &pu1_src_luma[(sao_ht_luma - 1) * src_strd], sao_wd_luma);

                    pu1_sao_src_top_left_luma_top_right[0] = pu1_src_luma[(sao_ht_luma - 1) * src_strd + sao_wd_luma];

                }

                else if(1 == ps_sao->b3_y_type_idx)
                {
                    ai1_offset_y[1] = ps_sao->b4_y_offset_1;
                    ai1_offset_y[2] = ps_sao->b4_y_offset_2;
                    ai1_offset_y[3] = ps_sao->b4_y_offset_3;
                    ai1_offset_y[4] = ps_sao->b4_y_offset_4;

                    ps_codec->s_func_selector.ihevc_sao_band_offset_luma_fptr(pu1_src_luma,
                                                                              src_strd,
                                                                              pu1_src_left_luma,
                                                                              pu1_src_top_luma,
                                                                              pu1_sao_src_top_left_luma_curr_ctb,
                                                                              ps_sao->b5_y_band_pos,
                                                                              ai1_offset_y,
                                                                              sao_wd_luma,
                                                                              sao_ht_luma
                                                                             );
                }

                else // if(2 <= ps_sao->b3_y_type_idx)
                {
                    ai1_offset_y[1] = ps_sao->b4_y_offset_1;
                    ai1_offset_y[2] = ps_sao->b4_y_offset_2;
                    ai1_offset_y[3] = ps_sao->b4_y_offset_3;
                    ai1_offset_y[4] = ps_sao->b4_y_offset_4;

                    for(i = 0; i < 8; i++)
                    {
                        au1_avail_luma[i] = 255;
                        au1_tile_slice_boundary[i] = 0;
                        au4_idx_c[i] = 0;
                        au4_ilf_across_tile_slice_enable[i] = 1;
                    }
                    /******************************************************************
                     * Derive the  Top-left CTB's neighbour pixel's slice indices.
                     *
                     *
                     *          ____________
                     *         |    |       |
                     *         |    | C_T   |
                     *         |____|_______|____
                     *         |    |       |    |
                     *         | C_L|   C   | C_R|
                     *         |____|_______|    |
                     *              |  C_D       |
                     *              |            |
                     *              |____________|
                     *
                     *****************************************************************/

                    /*In case of slices*/
                    {
                        if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
                        {
                            ctbx_c_t = ps_sao_ctxt->i4_ctb_x;
                            ctby_c_t = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_c_l = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_c_l = ps_sao_ctxt->i4_ctb_y;

                            ctbx_c_r = ps_sao_ctxt->i4_ctb_x;
                            ctby_c_r = ps_sao_ctxt->i4_ctb_y;

                            ctbx_c_d =  ps_sao_ctxt->i4_ctb_x;
                            ctby_c_d =  ps_sao_ctxt->i4_ctb_y;

                            ctbx_c = ps_sao_ctxt->i4_ctb_x;
                            ctby_c = ps_sao_ctxt->i4_ctb_y;

                            if(!ps_slice_hdr->i1_first_slice_in_pic_flag)
                            {
                                if(0 == ps_sao_ctxt->i4_ctb_x)
                                {
                                    au4_idx_c[6] = -1;
                                    au4_idx_c[0] = -1;
                                    au4_idx_c[4] = -1;
                                }
                                else
                                {
                                    au4_idx_c[0] =  au4_idx_c[6] = pu1_slice_idx[ctbx_c_l + (ctby_c_l * ps_sps->i2_pic_wd_in_ctb)];
                                }

                                if(0 == ps_sao_ctxt->i4_ctb_y)
                                {
                                    au4_idx_c[2] = -1;
                                    au4_idx_c[5] = -1;
                                    au4_idx_c[4] = -1;
                                }
                                else
                                {
                                    au4_idx_c[4] =  pu1_slice_idx[ctbx_c_t - 1 + (ctby_c_t  * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_c[2] = au4_idx_c[5] = pu1_slice_idx[ctbx_c_t + (ctby_c_t * ps_sps->i2_pic_wd_in_ctb)];
                                }
                                idx_c   = pu1_slice_idx[ctbx_c + (ctby_c * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_c[1] = au4_idx_c[7] = pu1_slice_idx[ctbx_c_r + (ctby_c_r * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_c[3] = pu1_slice_idx[ctbx_c_d + (ctby_c_d * ps_sps->i2_pic_wd_in_ctb)];

                                if(0 == ps_sao_ctxt->i4_ctb_x)
                                {
                                    au4_ilf_across_tile_slice_enable[6] = 0;
                                    au4_ilf_across_tile_slice_enable[0] = 0;
                                    au4_ilf_across_tile_slice_enable[4] = 0;
                                }
                                else
                                {
                                    au4_ilf_across_tile_slice_enable[6] = (ps_slice_hdr_base + au4_idx_c[6])->i1_slice_loop_filter_across_slices_enabled_flag;
                                    au4_ilf_across_tile_slice_enable[0] = (ps_slice_hdr_base + idx_c)->i1_slice_loop_filter_across_slices_enabled_flag;;
                                }
                                if(0 == ps_sao_ctxt->i4_ctb_y)
                                {
                                    au4_ilf_across_tile_slice_enable[2] = 0;
                                    au4_ilf_across_tile_slice_enable[4] = 0;
                                    au4_ilf_across_tile_slice_enable[5] = 0;
                                }
                                else
                                {
                                    au4_ilf_across_tile_slice_enable[2] = (ps_slice_hdr_base + idx_c)->i1_slice_loop_filter_across_slices_enabled_flag;
                                    au4_ilf_across_tile_slice_enable[5] = au4_ilf_across_tile_slice_enable[4] = au4_ilf_across_tile_slice_enable[2];
                                }
                                au4_ilf_across_tile_slice_enable[1] = (ps_slice_hdr_base + au4_idx_c[1])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[3] = (ps_slice_hdr_base + au4_idx_c[3])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[7] = (ps_slice_hdr_base + au4_idx_c[7])->i1_slice_loop_filter_across_slices_enabled_flag;

                                if(au4_idx_c[6] < idx_c)
                                {
                                    au4_ilf_across_tile_slice_enable[6] = (ps_slice_hdr_base + idx_c)->i1_slice_loop_filter_across_slices_enabled_flag;
                                }

                                /*
                                 * Between each neighbor and the current CTB, the i1_slice_loop_filter_across_slices_enabled_flag
                                 * of the pixel having a greater address is checked. Accordingly, set the availability flags
                                 */
                                for(i = 0; i < 8; i++)
                                {
                                    /*Sets the edges that lie on the slice/tile boundary*/
                                    if(au4_idx_c[i] != idx_c)
                                    {
                                        au1_tile_slice_boundary[i] = 1;
                                    }
                                    else
                                    {
                                        au4_ilf_across_tile_slice_enable[i] = 1;
                                    }
                                }
                                /*Reset indices*/
                                for(i = 0; i < 8; i++)
                                {
                                    au4_idx_c[i] = 0;
                                }
                            }

                            if(ps_pps->i1_tiles_enabled_flag)
                            {
                                /* Calculate availability flags at slice boundary */
                                if(((ps_tile->u1_pos_x == ps_sao_ctxt->i4_ctb_x) || (ps_tile->u1_pos_y == ps_sao_ctxt->i4_ctb_y)) && (!((0 == ps_tile->u1_pos_x) && (0 == ps_tile->u1_pos_y))))
                                {
                                    /*If ilf across tiles is enabled, boundary availability for tiles is not checked. */
                                    if(!ps_pps->i1_loop_filter_across_tiles_enabled_flag)
                                    {
                                        if(0 == ps_sao_ctxt->i4_ctb_x)
                                        {
                                            au4_idx_c[6] = -1;
                                            au4_idx_c[0] = -1;
                                            au4_idx_c[4] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_c[0] =  au4_idx_c[6] = pu1_tile_idx[ctbx_c_l + (ctby_c_l * ps_sps->i2_pic_wd_in_ctb)];
                                        }

                                        if(0 == ps_sao_ctxt->i4_ctb_y)
                                        {
                                            au4_idx_c[2] = -1;
                                            au4_idx_c[5] = -1;
                                            au4_idx_c[4] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_c[4] =  pu1_tile_idx[ctbx_c_t - 1 + (ctby_c_t  * ps_sps->i2_pic_wd_in_ctb)];
                                            au4_idx_c[2] = au4_idx_c[5] = pu1_tile_idx[ctbx_c_t + (ctby_c_t * ps_sps->i2_pic_wd_in_ctb)];
                                        }
                                        idx_c   = pu1_tile_idx[ctbx_c + (ctby_c * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_c[1] = au4_idx_c[7] = pu1_tile_idx[ctbx_c_r + (ctby_c_r * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_c[3] = pu1_tile_idx[ctbx_c_d + (ctby_c_d * ps_sps->i2_pic_wd_in_ctb)];

                                        for(i = 0; i < 8; i++)
                                        {
                                            /*Sets the edges that lie on the slice/tile boundary*/
                                            if(au4_idx_c[i] != idx_c)
                                            {
                                                au1_tile_slice_boundary[i] |= 1;
                                                au4_ilf_across_tile_slice_enable[i] &= ps_pps->i1_loop_filter_across_tiles_enabled_flag; //=0
                                            }
                                        }
                                    }
                                }
                            }

                            for(i = 0; i < 8; i++)
                            {
                                /*Sets the edges that lie on the slice/tile boundary*/
                                if((au1_tile_slice_boundary[i]) && !(au4_ilf_across_tile_slice_enable[i]))
                                {
                                    au1_avail_luma[i] = 0;
                                }
                            }

                        }
                    }
                    if(0 == ps_sao_ctxt->i4_ctb_x)
                    {
                        au1_avail_luma[0] = 0;
                        au1_avail_luma[4] = 0;
                        au1_avail_luma[6] = 0;
                    }

                    if(ps_sps->i2_pic_width_in_luma_samples - (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) <= sao_wd_luma)
                    {
                        au1_avail_luma[1] = 0;
                        au1_avail_luma[5] = 0;
                        au1_avail_luma[7] = 0;
                    }

                    if(0 == ps_sao_ctxt->i4_ctb_y)
                    {
                        au1_avail_luma[2] = 0;
                        au1_avail_luma[4] = 0;
                        au1_avail_luma[5] = 0;
                    }

                    if(ps_sps->i2_pic_height_in_luma_samples - (ps_sao_ctxt->i4_ctb_y  << log2_ctb_size) <= sao_ht_luma)
                    {
                        au1_avail_luma[3] = 0;
                        au1_avail_luma[6] = 0;
                        au1_avail_luma[7] = 0;
                    }

                    {
                        au1_src_top_right[0] = pu1_src_luma[sao_wd_luma - src_strd];
                        u1_sao_src_top_left_luma_bot_left = pu1_src_luma[sao_ht_luma * src_strd - 1];

                        ps_codec->apf_sao_luma[ps_sao->b3_y_type_idx - 2](pu1_src_luma,
                                                                          src_strd,
                                                                          pu1_src_left_luma,
                                                                          pu1_src_top_luma,
                                                                          pu1_sao_src_top_left_luma_curr_ctb,
                                                                          au1_src_top_right,
                                                                          &u1_sao_src_top_left_luma_bot_left,
                                                                          au1_avail_luma,
                                                                          ai1_offset_y,
                                                                          sao_wd_luma,
                                                                          sao_ht_luma);
                    }
                    pu1_sao_src_top_left_luma_top_right[0] = pu1_src_luma[(sao_ht_luma - 1) * src_strd + sao_wd_luma];
                    pu1_sao_src_top_left_luma_bot_left[0] = pu1_src_luma[(sao_ht_luma)*src_strd + sao_wd_luma - 1];
                }
            }
            else if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
            {
                /* Update left, top and top-left */
                for(row = 0; row < sao_ht_luma; row++)
                {
                    pu1_src_left_luma[row] = pu1_src_luma[row * src_strd + (sao_wd_luma - 1)];
                }
                pu1_sao_src_top_left_luma_curr_ctb[0] = pu1_src_top_luma[sao_wd_luma - 1];

                ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_luma, &pu1_src_luma[(sao_ht_luma - 1) * src_strd], sao_wd_luma);

                pu1_sao_src_top_left_luma_top_right[0] = pu1_src_luma[(sao_ht_luma - 1) * src_strd + sao_wd_luma];
            }
        }

        if((0 != sao_wd_chroma) && (0 != sao_ht_chroma))
        {
            if(ps_sao_ctxt->ps_slice_hdr->i1_slice_sao_chroma_flag)
            {
                if(0 == ps_sao->b3_cb_type_idx)
                {
                    for(row = 0; row < sao_ht_chroma; row++)
                    {
                        pu1_src_left_chroma[2 * row] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 2)];
                        pu1_src_left_chroma[2 * row + 1] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 1)];
                    }
                    pu1_sao_src_top_left_chroma_curr_ctb[0] = pu1_src_top_chroma[sao_wd_chroma - 2];
                    pu1_sao_src_top_left_chroma_curr_ctb[1] = pu1_src_top_chroma[sao_wd_chroma - 1];

                    ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_chroma, &pu1_src_chroma[(sao_ht_chroma - 1) * src_strd], sao_wd_chroma);

                    pu1_sao_src_top_left_chroma_top_right[0] = pu1_src_chroma[(sao_ht_chroma - 1) * src_strd + sao_wd_chroma];
                    pu1_sao_src_top_left_chroma_top_right[1] = pu1_src_chroma[(sao_ht_chroma - 1) * src_strd + sao_wd_chroma + 1];
                }

                else if(1 == ps_sao->b3_cb_type_idx)
                {
                    ai1_offset_cb[1] = ps_sao->b4_cb_offset_1;
                    ai1_offset_cb[2] = ps_sao->b4_cb_offset_2;
                    ai1_offset_cb[3] = ps_sao->b4_cb_offset_3;
                    ai1_offset_cb[4] = ps_sao->b4_cb_offset_4;

                    ai1_offset_cr[1] = ps_sao->b4_cr_offset_1;
                    ai1_offset_cr[2] = ps_sao->b4_cr_offset_2;
                    ai1_offset_cr[3] = ps_sao->b4_cr_offset_3;
                    ai1_offset_cr[4] = ps_sao->b4_cr_offset_4;

                    if(chroma_yuv420sp_vu)
                    {
                        ps_codec->s_func_selector.ihevc_sao_band_offset_chroma_fptr(pu1_src_chroma,
                                                                                    src_strd,
                                                                                    pu1_src_left_chroma,
                                                                                    pu1_src_top_chroma,
                                                                                    pu1_sao_src_top_left_chroma_curr_ctb,
                                                                                    ps_sao->b5_cr_band_pos,
                                                                                    ps_sao->b5_cb_band_pos,
                                                                                    ai1_offset_cr,
                                                                                    ai1_offset_cb,
                                                                                    sao_wd_chroma,
                                                                                    sao_ht_chroma
                                                                                   );
                    }
                    else
                    {
                        ps_codec->s_func_selector.ihevc_sao_band_offset_chroma_fptr(pu1_src_chroma,
                                                                                    src_strd,
                                                                                    pu1_src_left_chroma,
                                                                                    pu1_src_top_chroma,
                                                                                    pu1_sao_src_top_left_chroma_curr_ctb,
                                                                                    ps_sao->b5_cb_band_pos,
                                                                                    ps_sao->b5_cr_band_pos,
                                                                                    ai1_offset_cb,
                                                                                    ai1_offset_cr,
                                                                                    sao_wd_chroma,
                                                                                    sao_ht_chroma
                                                                                   );
                    }
                }

                else // if(2 <= ps_sao->b3_cb_type_idx)
                {
                    ai1_offset_cb[1] = ps_sao->b4_cb_offset_1;
                    ai1_offset_cb[2] = ps_sao->b4_cb_offset_2;
                    ai1_offset_cb[3] = ps_sao->b4_cb_offset_3;
                    ai1_offset_cb[4] = ps_sao->b4_cb_offset_4;

                    ai1_offset_cr[1] = ps_sao->b4_cr_offset_1;
                    ai1_offset_cr[2] = ps_sao->b4_cr_offset_2;
                    ai1_offset_cr[3] = ps_sao->b4_cr_offset_3;
                    ai1_offset_cr[4] = ps_sao->b4_cr_offset_4;

                    for(i = 0; i < 8; i++)
                    {
                        au1_avail_chroma[i] = 255;
                        au1_tile_slice_boundary[i] = 0;
                        au4_idx_c[i] = 0;
                        au4_ilf_across_tile_slice_enable[i] = 1;
                    }
                    {
                        if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
                        {
                            ctbx_c_t = ps_sao_ctxt->i4_ctb_x;
                            ctby_c_t = ps_sao_ctxt->i4_ctb_y - 1;

                            ctbx_c_l = ps_sao_ctxt->i4_ctb_x - 1;
                            ctby_c_l = ps_sao_ctxt->i4_ctb_y;

                            ctbx_c_r = ps_sao_ctxt->i4_ctb_x;
                            ctby_c_r = ps_sao_ctxt->i4_ctb_y;

                            ctbx_c_d =  ps_sao_ctxt->i4_ctb_x;
                            ctby_c_d =  ps_sao_ctxt->i4_ctb_y;

                            ctbx_c = ps_sao_ctxt->i4_ctb_x;
                            ctby_c = ps_sao_ctxt->i4_ctb_y;

                            if(!ps_slice_hdr->i1_first_slice_in_pic_flag)
                            {
                                if(0 == ps_sao_ctxt->i4_ctb_x)
                                {
                                    au4_idx_c[0] = -1;
                                    au4_idx_c[4] = -1;
                                    au4_idx_c[6] = -1;
                                }
                                else
                                {
                                    au4_idx_c[0] =  au4_idx_c[6] = pu1_slice_idx[ctbx_c_l + (ctby_c_l * ps_sps->i2_pic_wd_in_ctb)];
                                }

                                if(0 == ps_sao_ctxt->i4_ctb_y)
                                {
                                    au4_idx_c[2] = -1;
                                    au4_idx_c[4] = -1;
                                    au4_idx_c[5] = -1;
                                }
                                else
                                {
                                    au4_idx_c[2] = au4_idx_c[5] = pu1_slice_idx[ctbx_c_t + (ctby_c_t * ps_sps->i2_pic_wd_in_ctb)];
                                    au4_idx_c[4] =  pu1_slice_idx[ctbx_c_t - 1 + (ctby_c_t  * ps_sps->i2_pic_wd_in_ctb)];
                                }
                                idx_c = pu1_slice_idx[ctbx_c + (ctby_c * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_c[1] = au4_idx_c[7] = pu1_slice_idx[ctbx_c_r + (ctby_c_r * ps_sps->i2_pic_wd_in_ctb)];
                                au4_idx_c[3] = pu1_slice_idx[ctbx_c_d + (ctby_c_d * ps_sps->i2_pic_wd_in_ctb)];

                                if(0 == ps_sao_ctxt->i4_ctb_x)
                                {
                                    au4_ilf_across_tile_slice_enable[0] = 0;
                                    au4_ilf_across_tile_slice_enable[4] = 0;
                                    au4_ilf_across_tile_slice_enable[6] = 0;
                                }
                                else
                                {
                                    au4_ilf_across_tile_slice_enable[6] &= (ps_slice_hdr_base + au4_idx_c[6])->i1_slice_loop_filter_across_slices_enabled_flag;
                                    au4_ilf_across_tile_slice_enable[0] &= (ps_slice_hdr_base + idx_c)->i1_slice_loop_filter_across_slices_enabled_flag;
                                }

                                if(0 == ps_sao_ctxt->i4_ctb_y)
                                {
                                    au4_ilf_across_tile_slice_enable[2] = 0;
                                    au4_ilf_across_tile_slice_enable[4] = 0;
                                    au4_ilf_across_tile_slice_enable[5] = 0;
                                }
                                else
                                {
                                    au4_ilf_across_tile_slice_enable[2] &= (ps_slice_hdr_base + idx_c)->i1_slice_loop_filter_across_slices_enabled_flag;
                                    au4_ilf_across_tile_slice_enable[5] = au4_ilf_across_tile_slice_enable[4] = au4_ilf_across_tile_slice_enable[2];
                                }

                                au4_ilf_across_tile_slice_enable[1] &= (ps_slice_hdr_base + au4_idx_c[1])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[3] &= (ps_slice_hdr_base + au4_idx_c[3])->i1_slice_loop_filter_across_slices_enabled_flag;
                                au4_ilf_across_tile_slice_enable[7] &= (ps_slice_hdr_base + au4_idx_c[7])->i1_slice_loop_filter_across_slices_enabled_flag;

                                if(idx_c > au4_idx_c[6])
                                {
                                    au4_ilf_across_tile_slice_enable[6] = (ps_slice_hdr_base + idx_c)->i1_slice_loop_filter_across_slices_enabled_flag;
                                }

                                /*
                                 * Between each neighbor and the current CTB, the i1_slice_loop_filter_across_slices_enabled_flag
                                 * of the pixel having a greater address is checked. Accordingly, set the availability flags
                                 */
                                for(i = 0; i < 8; i++)
                                {
                                    /*Sets the edges that lie on the slice/tile boundary*/
                                    if(au4_idx_c[i] != idx_c)
                                    {
                                        au1_tile_slice_boundary[i] = 1;
                                    }
                                    else
                                    {
                                        au4_ilf_across_tile_slice_enable[i] = 1;
                                    }
                                }
                                /*Reset indices*/
                                for(i = 0; i < 8; i++)
                                {
                                    au4_idx_c[i] = 0;
                                }
                            }

                            if(ps_pps->i1_tiles_enabled_flag)
                            {
                                /* Calculate availability flags at slice boundary */
                                if(((ps_tile->u1_pos_x == ps_sao_ctxt->i4_ctb_x) || (ps_tile->u1_pos_y == ps_sao_ctxt->i4_ctb_y)) && (!((0 == ps_tile->u1_pos_x) && (0 == ps_tile->u1_pos_y))))
                                {
                                    /*If ilf across tiles is enabled, boundary availability for tiles is not checked. */
                                    if(!ps_pps->i1_loop_filter_across_tiles_enabled_flag)
                                    {
                                        if(0 == ps_sao_ctxt->i4_ctb_x)
                                        {
                                            au4_idx_c[6] = -1;
                                            au4_idx_c[0] = -1;
                                            au4_idx_c[4] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_c[0] =  au4_idx_c[6] = pu1_tile_idx[ctbx_c_l + (ctby_c_l * ps_sps->i2_pic_wd_in_ctb)];
                                        }

                                        if(0 == ps_sao_ctxt->i4_ctb_y)
                                        {
                                            au4_idx_c[2] = -1;
                                            au4_idx_c[5] = -1;
                                            au4_idx_c[4] = -1;
                                        }
                                        else
                                        {
                                            au4_idx_c[4] =  pu1_tile_idx[ctbx_c_t - 1 + (ctby_c_t  * ps_sps->i2_pic_wd_in_ctb)];
                                            au4_idx_c[2] = au4_idx_c[5] = pu1_tile_idx[ctbx_c_t + (ctby_c_t * ps_sps->i2_pic_wd_in_ctb)];
                                        }
                                        idx_c   = pu1_tile_idx[ctbx_c + (ctby_c * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_c[1] = au4_idx_c[7] = pu1_tile_idx[ctbx_c_r + (ctby_c_r * ps_sps->i2_pic_wd_in_ctb)];
                                        au4_idx_c[3] = pu1_tile_idx[ctbx_c_d + (ctby_c_d * ps_sps->i2_pic_wd_in_ctb)];

                                        for(i = 0; i < 8; i++)
                                        {
                                            /*Sets the edges that lie on the slice/tile boundary*/
                                            if(au4_idx_c[i] != idx_c)
                                            {
                                                au1_tile_slice_boundary[i] |= 1;
                                                au4_ilf_across_tile_slice_enable[i] &= ps_pps->i1_loop_filter_across_tiles_enabled_flag; //=0
                                            }
                                        }
                                    }
                                }
                            }

                            for(i = 0; i < 8; i++)
                            {
                                /*Sets the edges that lie on the slice/tile boundary*/
                                if((au1_tile_slice_boundary[i]) && !(au4_ilf_across_tile_slice_enable[i]))
                                {
                                    au1_avail_chroma[i] = 0;
                                }
                            }
                        }
                    }

                    if(0 == ps_sao_ctxt->i4_ctb_x)
                    {
                        au1_avail_chroma[0] = 0;
                        au1_avail_chroma[4] = 0;
                        au1_avail_chroma[6] = 0;
                    }

                    if(ps_sps->i2_pic_width_in_luma_samples - (ps_sao_ctxt->i4_ctb_x << log2_ctb_size) <= sao_wd_chroma)
                    {
                        au1_avail_chroma[1] = 0;
                        au1_avail_chroma[5] = 0;
                        au1_avail_chroma[7] = 0;
                    }

                    if(0 == ps_sao_ctxt->i4_ctb_y)
                    {
                        au1_avail_chroma[2] = 0;
                        au1_avail_chroma[4] = 0;
                        au1_avail_chroma[5] = 0;
                    }

                    if(ps_sps->i2_pic_height_in_luma_samples / 2 - (ps_sao_ctxt->i4_ctb_y  << (log2_ctb_size - 1)) <= sao_ht_chroma)
                    {
                        au1_avail_chroma[3] = 0;
                        au1_avail_chroma[6] = 0;
                        au1_avail_chroma[7] = 0;
                    }

                    {
                        au1_src_top_right[0] = pu1_src_chroma[sao_wd_chroma - src_strd];
                        au1_src_top_right[1] = pu1_src_chroma[sao_wd_chroma - src_strd + 1];

                        au1_sao_src_top_left_chroma_bot_left[0] = pu1_src_chroma[sao_ht_chroma * src_strd - 2];
                        au1_sao_src_top_left_chroma_bot_left[1] = pu1_src_chroma[sao_ht_chroma * src_strd - 1];

                        if(chroma_yuv420sp_vu)
                        {
                            ps_codec->apf_sao_chroma[ps_sao->b3_cb_type_idx - 2](pu1_src_chroma,
                                                                                 src_strd,
                                                                                 pu1_src_left_chroma,
                                                                                 pu1_src_top_chroma,
                                                                                 pu1_sao_src_top_left_chroma_curr_ctb,
                                                                                 au1_src_top_right,
                                                                                 au1_sao_src_top_left_chroma_bot_left,
                                                                                 au1_avail_chroma,
                                                                                 ai1_offset_cr,
                                                                                 ai1_offset_cb,
                                                                                 sao_wd_chroma,
                                                                                 sao_ht_chroma);
                        }
                        else
                        {
                            ps_codec->apf_sao_chroma[ps_sao->b3_cb_type_idx - 2](pu1_src_chroma,
                                                                                 src_strd,
                                                                                 pu1_src_left_chroma,
                                                                                 pu1_src_top_chroma,
                                                                                 pu1_sao_src_top_left_chroma_curr_ctb,
                                                                                 au1_src_top_right,
                                                                                 au1_sao_src_top_left_chroma_bot_left,
                                                                                 au1_avail_chroma,
                                                                                 ai1_offset_cb,
                                                                                 ai1_offset_cr,
                                                                                 sao_wd_chroma,
                                                                                 sao_ht_chroma);
                        }
                    }

                }
                pu1_sao_src_top_left_chroma_top_right[0] = pu1_src_chroma[(sao_ht_chroma - 1) * src_strd + sao_wd_chroma];
                pu1_sao_src_top_left_chroma_top_right[1] = pu1_src_chroma[(sao_ht_chroma - 1) * src_strd + sao_wd_chroma + 1];

                pu1_sao_src_top_left_chroma_bot_left[0] = pu1_src_chroma[(sao_ht_chroma)*src_strd + sao_wd_chroma - 2];
                pu1_sao_src_top_left_chroma_bot_left[1] = pu1_src_chroma[(sao_ht_chroma)*src_strd + sao_wd_chroma - 1];
            }
            else if((!ps_slice_hdr->i1_first_slice_in_pic_flag) || (ps_pps->i1_tiles_enabled_flag))
            {
                for(row = 0; row < sao_ht_chroma; row++)
                {
                    pu1_src_left_chroma[2 * row] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 2)];
                    pu1_src_left_chroma[2 * row + 1] = pu1_src_chroma[row * src_strd + (sao_wd_chroma - 1)];
                }
                pu1_sao_src_top_left_chroma_curr_ctb[0] = pu1_src_top_chroma[sao_wd_chroma - 2];
                pu1_sao_src_top_left_chroma_curr_ctb[1] = pu1_src_top_chroma[sao_wd_chroma - 1];

                ps_codec->s_func_selector.ihevc_memcpy_fptr(pu1_src_top_chroma, &pu1_src_chroma[(sao_ht_chroma - 1) * src_strd], sao_wd_chroma);

                pu1_sao_src_top_left_chroma_top_right[0] = pu1_src_chroma[(sao_ht_chroma - 1) * src_strd + sao_wd_chroma];
                pu1_sao_src_top_left_chroma_top_right[1] = pu1_src_chroma[(sao_ht_chroma - 1) * src_strd + sao_wd_chroma + 1];
            }

        }
    }




/* If no loop filter is enabled copy the backed up values */
    {
        /* Luma */
        if(no_loop_filter_enabled_luma)
        {
            UWORD32 u4_no_loop_filter_flag;
            WORD32 loop_filter_bit_pos;
            WORD32 log2_min_cu = 3;
            WORD32 min_cu = (1 << log2_min_cu);
            UWORD8 *pu1_src_tmp_luma = pu1_src_luma;
            WORD32 sao_blk_ht = ctb_size - SAO_SHIFT_CTB;
            WORD32 sao_blk_wd = ctb_size;
            WORD32 remaining_rows;
            WORD32 remaining_cols;

            remaining_rows = ps_sps->i2_pic_height_in_luma_samples - ((ps_sao_ctxt->i4_ctb_y << log2_ctb_size) + ctb_size - SAO_SHIFT_CTB);
            remaining_cols = ps_sps->i2_pic_width_in_luma_samples - ((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) + ctb_size - SAO_SHIFT_CTB);
            if(remaining_rows <= SAO_SHIFT_CTB)
                sao_blk_ht += remaining_rows;
            if(remaining_cols <= SAO_SHIFT_CTB)
                sao_blk_wd += remaining_cols;

            pu1_src_tmp_luma -= ps_sao_ctxt->i4_ctb_x ? SAO_SHIFT_CTB : 0;
            pu1_src_tmp_luma -= ps_sao_ctxt->i4_ctb_y ? SAO_SHIFT_CTB * src_strd : 0;

            pu1_src_backup_luma = ps_sao_ctxt->pu1_tmp_buf_luma;

            loop_filter_bit_pos = (ps_sao_ctxt->i4_ctb_x << (log2_ctb_size - 3)) +
                            (ps_sao_ctxt->i4_ctb_y << (log2_ctb_size - 3)) * (loop_filter_strd << 3);
            if(ps_sao_ctxt->i4_ctb_x > 0)
                loop_filter_bit_pos -= 1;

            pu1_no_loop_filter_flag = ps_sao_ctxt->pu1_pic_no_loop_filter_flag +
                            (loop_filter_bit_pos >> 3);

            for(i = -(ps_sao_ctxt->i4_ctb_y ? SAO_SHIFT_CTB : 0) >> log2_min_cu;
                            i < (sao_blk_ht + (min_cu - 1)) >> log2_min_cu; i++)
            {
                WORD32 tmp_wd = sao_blk_wd;

                u4_no_loop_filter_flag = (*(UWORD32 *)(pu1_no_loop_filter_flag + i * loop_filter_strd)) >>
                                (loop_filter_bit_pos & 7);
                u4_no_loop_filter_flag &= (1 << ((tmp_wd + (min_cu - 1)) >> log2_min_cu)) - 1;

                if(u4_no_loop_filter_flag)
                {
                    while(tmp_wd > 0)
                    {
                        if(CTZ(u4_no_loop_filter_flag))
                        {
                            pu1_src_tmp_luma += MIN(((WORD32)CTZ(u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            pu1_src_backup_luma += MIN(((WORD32)CTZ(u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            tmp_wd -= CTZ(u4_no_loop_filter_flag) << log2_min_cu;
                            u4_no_loop_filter_flag  >>= (CTZ(u4_no_loop_filter_flag));
                        }
                        else
                        {
                            for(row = 0; row < min_cu; row++)
                            {
                                for(col = 0; col < MIN(((WORD32)CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd); col++)
                                {
                                    pu1_src_tmp_luma[row * src_strd + col] = pu1_src_backup_luma[row * backup_strd + col];
                                }
                            }
                            pu1_src_tmp_luma += MIN(((WORD32)CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            pu1_src_backup_luma += MIN(((WORD32)CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            tmp_wd -= CTZ(~u4_no_loop_filter_flag) << log2_min_cu;
                            u4_no_loop_filter_flag  >>= (CTZ(~u4_no_loop_filter_flag));
                        }
                    }

                    pu1_src_tmp_luma -= sao_blk_wd;
                    pu1_src_backup_luma -= sao_blk_wd;
                }

                pu1_src_tmp_luma += (src_strd << log2_min_cu);
                pu1_src_backup_luma += (backup_strd << log2_min_cu);
            }
        }

        /* Chroma */
        if(no_loop_filter_enabled_chroma)
        {
            UWORD32 u4_no_loop_filter_flag;
            WORD32 loop_filter_bit_pos;
            WORD32 log2_min_cu = 3;
            WORD32 min_cu = (1 << log2_min_cu);
            UWORD8 *pu1_src_tmp_chroma = pu1_src_chroma;
            WORD32 sao_blk_ht = ctb_size - 2 * SAO_SHIFT_CTB;
            WORD32 sao_blk_wd = ctb_size;
            WORD32 remaining_rows;
            WORD32 remaining_cols;

            remaining_rows = ps_sps->i2_pic_height_in_luma_samples - ((ps_sao_ctxt->i4_ctb_y << log2_ctb_size) + ctb_size - 2 * SAO_SHIFT_CTB);
            remaining_cols = ps_sps->i2_pic_width_in_luma_samples - ((ps_sao_ctxt->i4_ctb_x << log2_ctb_size) + ctb_size - 2 * SAO_SHIFT_CTB);
            if(remaining_rows <= 2 * SAO_SHIFT_CTB)
                sao_blk_ht += remaining_rows;
            if(remaining_cols <= 2 * SAO_SHIFT_CTB)
                sao_blk_wd += remaining_cols;

            pu1_src_tmp_chroma -= ps_sao_ctxt->i4_ctb_x ? SAO_SHIFT_CTB * 2 : 0;
            pu1_src_tmp_chroma -= ps_sao_ctxt->i4_ctb_y ? SAO_SHIFT_CTB * src_strd : 0;

            pu1_src_backup_chroma = ps_sao_ctxt->pu1_tmp_buf_chroma;

            loop_filter_bit_pos = (ps_sao_ctxt->i4_ctb_x << (log2_ctb_size - 3)) +
                            (ps_sao_ctxt->i4_ctb_y << (log2_ctb_size - 3)) * (loop_filter_strd << 3);
            if(ps_sao_ctxt->i4_ctb_x > 0)
                loop_filter_bit_pos -= 2;

            pu1_no_loop_filter_flag = ps_sao_ctxt->pu1_pic_no_loop_filter_flag +
                            (loop_filter_bit_pos >> 3);

            for(i = -(ps_sao_ctxt->i4_ctb_y ? 2 * SAO_SHIFT_CTB : 0) >> log2_min_cu;
                            i < (sao_blk_ht + (min_cu - 1)) >> log2_min_cu; i++)
            {
                WORD32 tmp_wd = sao_blk_wd;

                u4_no_loop_filter_flag = (*(UWORD32 *)(pu1_no_loop_filter_flag + i * loop_filter_strd)) >>
                                (loop_filter_bit_pos & 7);
                u4_no_loop_filter_flag &= (1 << ((tmp_wd + (min_cu - 1)) >> log2_min_cu)) - 1;

                if(u4_no_loop_filter_flag)
                {
                    while(tmp_wd > 0)
                    {
                        if(CTZ(u4_no_loop_filter_flag))
                        {
                            pu1_src_tmp_chroma += MIN(((WORD32)CTZ(u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            pu1_src_backup_chroma += MIN(((WORD32)CTZ(u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            tmp_wd -= CTZ(u4_no_loop_filter_flag) << log2_min_cu;
                            u4_no_loop_filter_flag  >>= (CTZ(u4_no_loop_filter_flag));
                        }
                        else
                        {
                            for(row = 0; row < min_cu / 2; row++)
                            {
                                for(col = 0; col < MIN(((WORD32)CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd); col++)
                                {
                                    pu1_src_tmp_chroma[row * src_strd + col] = pu1_src_backup_chroma[row * backup_strd + col];
                                }
                            }

                            pu1_src_tmp_chroma += MIN(((WORD32)CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            pu1_src_backup_chroma += MIN(((WORD32)CTZ(~u4_no_loop_filter_flag) << log2_min_cu), tmp_wd);
                            tmp_wd -= CTZ(~u4_no_loop_filter_flag) << log2_min_cu;
                            u4_no_loop_filter_flag  >>= (CTZ(~u4_no_loop_filter_flag));
                        }
                    }

                    pu1_src_tmp_chroma -= sao_blk_wd;
                    pu1_src_backup_chroma -= sao_blk_wd;
                }

                pu1_src_tmp_chroma += ((src_strd / 2) << log2_min_cu);
                pu1_src_backup_chroma += ((backup_strd / 2) << log2_min_cu);
            }
        }
    }

}

