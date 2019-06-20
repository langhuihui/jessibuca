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
 *  ihevc_inter_pred.c
 *
 * @brief
 *  Calculates the prediction samples for a given cbt
 *
 * @author
 *  Srinivas T
 *
 * @par List of Functions:
 *   - ihevc_inter_pred()
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
#include "ihevc_structs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_cabac_tables.h"
#include "ihevc_weighted_pred.h"

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

#include "ihevc_inter_pred.h"
#include "ihevcd_profile.h"

static WORD8 gai1_luma_filter[4][NTAPS_LUMA] =
{
    { 0, 0, 0, 64, 0, 0, 0, 0 },
    { -1, 4, -10, 58, 17, -5, 1, 0 },
    { -1, 4, -11, 40, 40, -11, 4, -1 },
    { 0, 1, -5, 17, 58, -10, 4, -1 } };

/* The filter uses only the first four elements in each array */
static WORD8 gai1_chroma_filter[8][NTAPS_LUMA] =
{
    { 0, 64, 0, 0, 0, 0, 0, 0 },
    { -2, 58, 10, -2, 0, 0, 0, 0 },
    { -4, 54, 16, -2, 0, 0, 0, 0 },
    { -6, 46, 28, -4, 0, 0, 0, 0 },
    { -4, 36, 36, -4, 0, 0, 0, 0 },
    { -4, 28, 46, -6, 0, 0, 0, 0 },
    { -2, 16, 54, -4, 0, 0, 0, 0 },
    { -2, 10, 58, -2, 0, 0, 0, 0 } };

/**
*******************************************************************************
*
* @brief
*  Inter prediction CTB level function
*
* @par Description:
*  For a given CTB, Inter prediction followed by weighted  prediction is
* done for all the PUs present in the CTB
*
* @param[in] ps_ctb
*  Pointer to the CTB context
*
* @returns
*
* @remarks
*
*
*******************************************************************************
*/

void ihevcd_inter_pred_ctb(process_ctxt_t *ps_proc)
{
    UWORD8 *ref_pic_luma_l0, *ref_pic_chroma_l0;
    UWORD8 *ref_pic_luma_l1, *ref_pic_chroma_l1;

    UWORD8 *ref_pic_l0 = NULL, *ref_pic_l1 = NULL;

    slice_header_t *ps_slice_hdr;
    sps_t *ps_sps;
    pps_t *ps_pps;
    pu_t *ps_pu;
    codec_t *ps_codec;
    WORD32 pu_indx;
    WORD32 pu_x, pu_y;
    WORD32 pu_wd, pu_ht;
    WORD32 i4_pu_cnt;
    WORD32 cur_ctb_idx;

    WORD32 clr_indx;
    WORD32 ntaps;



    WORD32 ai2_xint[2] = { 0, 0 }, ai2_yint[2] = { 0, 0 };
    WORD32 ai2_xfrac[2] = { 0, 0 }, ai2_yfrac[2] = { 0, 0 };

    WORD32 weighted_pred, bi_pred;

    WORD32 ref_strd;
    UWORD8 *pu1_dst_luma, *pu1_dst_chroma;

    UWORD8 *pu1_dst;

    WORD16 *pi2_tmp1, *pi2_tmp2;

    WORD32 luma_weight_l0, luma_weight_l1;
    WORD32 chroma_weight_l0_cb, chroma_weight_l1_cb, chroma_weight_l0_cr, chroma_weight_l1_cr;
    WORD32 luma_offset_l0, luma_offset_l1;
    WORD32 chroma_offset_l0_cb, chroma_offset_l1_cb, chroma_offset_l0_cr, chroma_offset_l1_cr;
    WORD32 shift, lvl_shift1, lvl_shift2;

    pf_inter_pred func_ptr1, func_ptr2, func_ptr3, func_ptr4;
    WORD32 func_indx1, func_indx2, func_indx3, func_indx4;
    void *func_src;
    void *func_dst;
    WORD32 func_src_strd;
    WORD32 func_dst_strd;
    WORD8 *func_coeff;
    WORD32 func_wd;
    WORD32 func_ht;
    WORD32 next_ctb_idx;
    WORD8(*coeff)[8];
    WORD32  chroma_yuv420sp_vu;

    PROFILE_DISABLE_INTER_PRED();
    ps_codec = ps_proc->ps_codec;
    ps_slice_hdr = ps_proc->ps_slice_hdr;
    ps_pps = ps_proc->ps_pps;
    ps_sps = ps_proc->ps_sps;
    cur_ctb_idx = ps_proc->i4_ctb_x
                    + ps_proc->i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);
    /*
     * In case of tiles, the next ctb belonging to the same tile must be used to get the PU index
     */

    next_ctb_idx = ps_proc->i4_next_pu_ctb_cnt;
    i4_pu_cnt = ps_proc->pu4_pic_pu_idx[next_ctb_idx] - ps_proc->pu4_pic_pu_idx[cur_ctb_idx];

    ps_pu = ps_proc->ps_pu;
    ref_strd = ps_codec->i4_strd;
    pi2_tmp1 = ps_proc->pi2_inter_pred_tmp_buf1;
    pi2_tmp2 = ps_proc->pi2_inter_pred_tmp_buf2;
    pu1_dst_luma = ps_proc->pu1_cur_pic_luma;
    pu1_dst_chroma = ps_proc->pu1_cur_pic_chroma;

    chroma_yuv420sp_vu = (ps_codec->e_ref_chroma_fmt == IV_YUV_420SP_VU);

    ASSERT(PSLICE == ps_slice_hdr->i1_slice_type || BSLICE == ps_slice_hdr->i1_slice_type);

    ref_pic_luma_l0 = NULL;
    ref_pic_chroma_l0 = NULL;

    luma_weight_l0 = 0;
    chroma_weight_l0_cb = 0;
    chroma_weight_l0_cr = 0;

    luma_offset_l0 = 0;
    chroma_offset_l0_cb = 0;
    chroma_offset_l0_cr = 0;

    ref_pic_luma_l1 = NULL;
    ref_pic_chroma_l1 = NULL;

    luma_weight_l1 = 0;
    chroma_weight_l1_cb = 0;
    chroma_weight_l1_cr = 0;

    luma_offset_l1 = 0;
    chroma_offset_l1_cb = 0;
    chroma_offset_l1_cr = 0;

    for(pu_indx = 0; pu_indx < i4_pu_cnt; pu_indx++, ps_pu++)
    {
        /* If the PU is intra then proceed to the next */
        if(1 == ps_pu->b1_intra_flag)
            continue;
        pu_x = (ps_proc->i4_ctb_x << ps_sps->i1_log2_ctb_size) + (ps_pu->b4_pos_x << 2);
        pu_y = (ps_proc->i4_ctb_y << ps_sps->i1_log2_ctb_size) + (ps_pu->b4_pos_y << 2);

        pu_wd = (ps_pu->b4_wd + 1) << 2;
        pu_ht = (ps_pu->b4_ht + 1) << 2;

        weighted_pred = (ps_slice_hdr->i1_slice_type == PSLICE) ? ps_pps->i1_weighted_pred_flag :
                        ps_pps->i1_weighted_bipred_flag;
        bi_pred = (ps_pu->b2_pred_mode == PRED_BI);

        if(ps_pu->b2_pred_mode != PRED_L1)
        {
            pic_buf_t *ps_pic_buf_l0;

            ps_pic_buf_l0 = (pic_buf_t *)((ps_slice_hdr->as_ref_pic_list0[ps_pu->mv.i1_l0_ref_idx].pv_pic_buf));

            ref_pic_luma_l0 = ps_pic_buf_l0->pu1_luma;
            ref_pic_chroma_l0 = ps_pic_buf_l0->pu1_chroma;

            luma_weight_l0 = ps_slice_hdr->s_wt_ofst.i2_luma_weight_l0[ps_pu->mv.i1_l0_ref_idx];
            chroma_weight_l0_cb = ps_slice_hdr->s_wt_ofst.i2_chroma_weight_l0_cb[ps_pu->mv.i1_l0_ref_idx];
            chroma_weight_l0_cr = ps_slice_hdr->s_wt_ofst.i2_chroma_weight_l0_cr[ps_pu->mv.i1_l0_ref_idx];

            luma_offset_l0 = ps_slice_hdr->s_wt_ofst.i2_luma_offset_l0[ps_pu->mv.i1_l0_ref_idx];
            chroma_offset_l0_cb = ps_slice_hdr->s_wt_ofst.i2_chroma_offset_l0_cb[ps_pu->mv.i1_l0_ref_idx];
            chroma_offset_l0_cr = ps_slice_hdr->s_wt_ofst.i2_chroma_offset_l0_cr[ps_pu->mv.i1_l0_ref_idx];
        }

        if(ps_pu->b2_pred_mode != PRED_L0)
        {
            pic_buf_t *ps_pic_buf_l1;
            ps_pic_buf_l1 = (pic_buf_t *)((ps_slice_hdr->as_ref_pic_list1[ps_pu->mv.i1_l1_ref_idx].pv_pic_buf));
            ref_pic_luma_l1 = ps_pic_buf_l1->pu1_luma;
            ref_pic_chroma_l1 = ps_pic_buf_l1->pu1_chroma;

            luma_weight_l1 = ps_slice_hdr->s_wt_ofst.i2_luma_weight_l1[ps_pu->mv.i1_l1_ref_idx];
            chroma_weight_l1_cb = ps_slice_hdr->s_wt_ofst.i2_chroma_weight_l1_cb[ps_pu->mv.i1_l1_ref_idx];
            chroma_weight_l1_cr = ps_slice_hdr->s_wt_ofst.i2_chroma_weight_l1_cr[ps_pu->mv.i1_l1_ref_idx];

            luma_offset_l1 = ps_slice_hdr->s_wt_ofst.i2_luma_offset_l1[ps_pu->mv.i1_l1_ref_idx];
            chroma_offset_l1_cb = ps_slice_hdr->s_wt_ofst.i2_chroma_offset_l1_cb[ps_pu->mv.i1_l1_ref_idx];
            chroma_offset_l1_cr = ps_slice_hdr->s_wt_ofst.i2_chroma_offset_l1_cr[ps_pu->mv.i1_l1_ref_idx];
        }

        /*luma and chroma components*/
        for(clr_indx = 0; clr_indx < 2; clr_indx++)
        {
            PROFILE_DISABLE_INTER_PRED_LUMA(clr_indx);
            PROFILE_DISABLE_INTER_PRED_CHROMA(clr_indx);

            if(clr_indx == 0)
            {
                WORD32 mv;
                if(ps_pu->b2_pred_mode != PRED_L1)
                {
                    mv = CLIP3(ps_pu->mv.s_l0_mv.i2_mvx, (-((MAX_CTB_SIZE + pu_x + 7) << 2)), ((ps_sps->i2_pic_width_in_luma_samples - pu_x + 7) << 2));
                    ai2_xint[0] = pu_x + (mv >> 2);
                    ai2_xfrac[0] = mv & 3;

                    mv = CLIP3(ps_pu->mv.s_l0_mv.i2_mvy, (-((MAX_CTB_SIZE + pu_y + 7) << 2)), ((ps_sps->i2_pic_height_in_luma_samples - pu_y + 7) << 2));
                    ai2_yint[0] = pu_y + (mv >> 2);
                    ai2_yfrac[0] = mv & 3;

                    ai2_xfrac[0] &= ps_codec->i4_mv_frac_mask;
                    ai2_yfrac[0] &= ps_codec->i4_mv_frac_mask;


                    ref_pic_l0 = ref_pic_luma_l0 + ai2_yint[0] * ref_strd
                                    + ai2_xint[0];
                }

                if(ps_pu->b2_pred_mode != PRED_L0)
                {

                    mv = CLIP3(ps_pu->mv.s_l1_mv.i2_mvx, (-((MAX_CTB_SIZE + pu_x + 7) << 2)), ((ps_sps->i2_pic_width_in_luma_samples - pu_x + 7) << 2));
                    ai2_xint[1] = pu_x + (mv >> 2);
                    ai2_xfrac[1] = mv & 3;

                    mv = CLIP3(ps_pu->mv.s_l1_mv.i2_mvy, (-((MAX_CTB_SIZE + pu_y + 7) << 2)), ((ps_sps->i2_pic_height_in_luma_samples - pu_y + 7) << 2));
                    ai2_yint[1] = pu_y + (mv >> 2);
                    ai2_yfrac[1] = mv & 3;

                    ref_pic_l1 = ref_pic_luma_l1 + ai2_yint[1] * ref_strd
                                    + ai2_xint[1];
                    ai2_xfrac[1] &= ps_codec->i4_mv_frac_mask;
                    ai2_yfrac[1] &= ps_codec->i4_mv_frac_mask;

                }

                pu1_dst = pu1_dst_luma + pu_y * ref_strd + pu_x;

                ntaps = NTAPS_LUMA;
                coeff = gai1_luma_filter;
            }

            else
            {
                WORD32 mv;
                /* xint is upshifted by 1 because the chroma components are  */
                /* interleaved which is not the assumption made by standard  */
                if(ps_pu->b2_pred_mode != PRED_L1)
                {
                    mv = CLIP3(ps_pu->mv.s_l0_mv.i2_mvx, (-((MAX_CTB_SIZE + pu_x + 7) << 2)), ((ps_sps->i2_pic_width_in_luma_samples - pu_x + 7) << 2));
                    ai2_xint[0] = (pu_x / 2 + (mv >> 3)) << 1;
                    ai2_xfrac[0] = mv & 7;

                    mv = CLIP3(ps_pu->mv.s_l0_mv.i2_mvy, (-((MAX_CTB_SIZE + pu_y + 7) << 2)), ((ps_sps->i2_pic_height_in_luma_samples - pu_y + 7) << 2));
                    ai2_yint[0] = pu_y / 2 + (mv >> 3);
                    ai2_yfrac[0] = mv & 7;

                    ref_pic_l0 = ref_pic_chroma_l0 + ai2_yint[0] * ref_strd
                                    + ai2_xint[0];

                    ai2_xfrac[0] &= ps_codec->i4_mv_frac_mask;
                    ai2_yfrac[0] &= ps_codec->i4_mv_frac_mask;

                }

                if(ps_pu->b2_pred_mode != PRED_L0)
                {
                    mv = CLIP3(ps_pu->mv.s_l1_mv.i2_mvx, (-((MAX_CTB_SIZE + pu_x + 7) << 2)), ((ps_sps->i2_pic_width_in_luma_samples - pu_x + 7) << 2));
                    ai2_xint[1] = (pu_x / 2 + (mv >> 3)) << 1;
                    ai2_xfrac[1] = mv & 7;

                    mv = CLIP3(ps_pu->mv.s_l1_mv.i2_mvy, (-((MAX_CTB_SIZE + pu_y + 7) << 2)), ((ps_sps->i2_pic_height_in_luma_samples - pu_y + 7) << 2));
                    ai2_yint[1] = pu_y / 2 + (mv >> 3);
                    ai2_yfrac[1] = mv & 7;

                    ref_pic_l1 = ref_pic_chroma_l1 + ai2_yint[1] * ref_strd
                                    + ai2_xint[1];
                    ai2_xfrac[1] &= ps_codec->i4_mv_frac_mask;
                    ai2_yfrac[1] &= ps_codec->i4_mv_frac_mask;

                }

                pu1_dst = pu1_dst_chroma + pu_y * ref_strd / 2 + pu_x;

                ntaps = NTAPS_CHROMA;
                coeff = gai1_chroma_filter;
            }

            if(ps_pu->b2_pred_mode != PRED_L1)
            {
                func_indx1 = 4 * (weighted_pred || bi_pred) + 1 + 11 * clr_indx;
                func_indx1 += ai2_xfrac[0] ? 2 : 0;
                func_indx1 += ai2_yfrac[0] ? 1 : 0;

                func_indx2 = (ai2_xfrac[0] && ai2_yfrac[0])
                                * (9 + (weighted_pred || bi_pred)) + 11 * clr_indx;

                func_ptr1 = ps_codec->apf_inter_pred[func_indx1];
                func_ptr2 = ps_codec->apf_inter_pred[func_indx2];
            }
            else
            {
                func_ptr1 = NULL;
                func_ptr2 = NULL;
            }
            if(ps_pu->b2_pred_mode != PRED_L0)
            {
                func_indx3 = 4 * (weighted_pred || bi_pred) + 1 + 11 * clr_indx;
                func_indx3 += ai2_xfrac[1] ? 2 : 0;
                func_indx3 += ai2_yfrac[1] ? 1 : 0;

                func_indx4 = (ai2_xfrac[1] && ai2_yfrac[1])
                                * (9 + (weighted_pred || bi_pred)) + 11 * clr_indx;

                func_ptr3 = ps_codec->apf_inter_pred[func_indx3];
                func_ptr4 = ps_codec->apf_inter_pred[func_indx4];
            }
            else
            {
                func_ptr3 = NULL;
                func_ptr4 = NULL;
            }

            /*Function 1*/
            if(func_ptr1 != NULL)
            {
                func_src_strd = ref_strd;
                func_src = (ai2_xfrac[0] && ai2_yfrac[0]) ?
                                ref_pic_l0 - (ntaps / 2 - 1) * func_src_strd :
                                ref_pic_l0;
                func_dst = (weighted_pred || bi_pred) ?
                                (void *)pi2_tmp1 : (void *)pu1_dst;
                if(ai2_xfrac[0] && ai2_yfrac[0])
                {
                    func_dst = pi2_tmp1;
                }

                func_dst_strd = (weighted_pred || bi_pred
                                || (ai2_xfrac[0] && ai2_yfrac[0])) ?
                                pu_wd : ref_strd;
                func_coeff = ai2_xfrac[0] ?
                                coeff[ai2_xfrac[0]] : coeff[ai2_yfrac[0]];
                func_wd = pu_wd >> clr_indx;
                func_ht = pu_ht >> clr_indx;
                func_ht += (ai2_xfrac[0] && ai2_yfrac[0]) ? ntaps - 1 : 0;
                func_ptr1(func_src, func_dst, func_src_strd, func_dst_strd,
                          func_coeff, func_ht, func_wd);
            }

            /*Function 2*/
            if(func_ptr2 != NULL)
            {
                func_src_strd = pu_wd;
                func_src = pi2_tmp1 + (ntaps / 2 - 1) * func_src_strd;
                func_dst = (weighted_pred || bi_pred) ?
                                (void *)pi2_tmp1 : (void *)pu1_dst;

                func_dst_strd = (weighted_pred || bi_pred) ?
                                pu_wd : ref_strd;
                func_coeff = coeff[ai2_yfrac[0]];
                func_wd = pu_wd >> clr_indx;
                func_ht = pu_ht >> clr_indx;
                func_ptr2(func_src, func_dst, func_src_strd, func_dst_strd,
                          func_coeff, func_ht, func_wd);
            }

            if(func_ptr3 != NULL)
            {
                func_src_strd = ref_strd;
                func_src = (ai2_xfrac[1] && ai2_yfrac[1]) ?
                                ref_pic_l1 - (ntaps / 2 - 1) * func_src_strd :
                                ref_pic_l1;

                func_dst = (weighted_pred || bi_pred) ?
                                (void *)pi2_tmp2 : (void *)pu1_dst;
                if(ai2_xfrac[1] && ai2_yfrac[1])
                {
                    func_dst = pi2_tmp2;
                }
                func_dst_strd = (weighted_pred || bi_pred
                                || (ai2_xfrac[1] && ai2_yfrac[1])) ?
                                pu_wd : ref_strd;
                func_coeff = ai2_xfrac[1] ?
                                coeff[ai2_xfrac[1]] : coeff[ai2_yfrac[1]];
                func_wd = pu_wd >> clr_indx;
                func_ht = pu_ht >> clr_indx;
                func_ht += (ai2_xfrac[1] && ai2_yfrac[1]) ? ntaps - 1 : 0;
                func_ptr3(func_src, func_dst, func_src_strd, func_dst_strd,
                          func_coeff, func_ht, func_wd);

            }

            if(func_ptr4 != NULL)
            {
                func_src_strd = pu_wd;
                func_src = pi2_tmp2 + (ntaps / 2 - 1) * func_src_strd;

                func_dst = (weighted_pred || bi_pred) ?
                                (void *)pi2_tmp2 : (void *)pu1_dst;
                func_dst_strd = (weighted_pred || bi_pred) ?
                                pu_wd : ref_strd;
                func_coeff = coeff[ai2_yfrac[1]];
                func_wd = pu_wd >> clr_indx;
                func_ht = pu_ht >> clr_indx;
                func_ptr4(func_src, func_dst, func_src_strd, func_dst_strd,
                          func_coeff, func_ht, func_wd);

            }

            PROFILE_DISABLE_INTER_PRED_LUMA_AVERAGING(clr_indx);
            PROFILE_DISABLE_INTER_PRED_CHROMA_AVERAGING(clr_indx);


            if((weighted_pred != 0) && (bi_pred != 0))
            {
                lvl_shift1 = 0;
                lvl_shift2 = 0;
                if((0 == clr_indx) && (ai2_xfrac[0] && ai2_yfrac[0]))
                    lvl_shift1 = (1 << 13);

                if((0 == clr_indx) && (ai2_xfrac[1] && ai2_yfrac[1]))
                    lvl_shift2 = (1 << 13);


                if(0 == clr_indx)
                {
                    shift = ps_slice_hdr->s_wt_ofst.i1_luma_log2_weight_denom
                                    + SHIFT_14_MINUS_BIT_DEPTH + 1;

                    ps_codec->s_func_selector.ihevc_weighted_pred_bi_fptr(pi2_tmp1,
                                                                          pi2_tmp2,
                                                                          pu1_dst,
                                                                          pu_wd,
                                                                          pu_wd,
                                                                          ref_strd,
                                                                          luma_weight_l0,
                                                                          luma_offset_l0,
                                                                          luma_weight_l1,
                                                                          luma_offset_l1,
                                                                          shift,
                                                                          lvl_shift1,
                                                                          lvl_shift2,
                                                                          pu_ht,
                                                                          pu_wd);
                }
                else
                {
                    shift = ps_slice_hdr->s_wt_ofst.i1_chroma_log2_weight_denom
                                    + SHIFT_14_MINUS_BIT_DEPTH + 1;

                    if(chroma_yuv420sp_vu)
                    {
                        ps_codec->s_func_selector.ihevc_weighted_pred_chroma_bi_fptr(pi2_tmp1,
                                                                                     pi2_tmp2,
                                                                                     pu1_dst,
                                                                                     pu_wd,
                                                                                     pu_wd,
                                                                                     ref_strd,
                                                                                     chroma_weight_l0_cr,
                                                                                     chroma_weight_l0_cb,
                                                                                     chroma_offset_l0_cr,
                                                                                     chroma_offset_l0_cb,
                                                                                     chroma_weight_l1_cr,
                                                                                     chroma_weight_l1_cb,
                                                                                     chroma_offset_l1_cr,
                                                                                     chroma_offset_l1_cb,
                                                                                     shift,
                                                                                     lvl_shift1,
                                                                                     lvl_shift2,
                                                                                     pu_ht >> 1,
                                                                                     pu_wd >> 1);
                    }
                    else
                    {
                        ps_codec->s_func_selector.ihevc_weighted_pred_chroma_bi_fptr(pi2_tmp1,
                                                                                     pi2_tmp2,
                                                                                     pu1_dst,
                                                                                     pu_wd,
                                                                                     pu_wd,
                                                                                     ref_strd,
                                                                                     chroma_weight_l0_cb,
                                                                                     chroma_weight_l0_cr,
                                                                                     chroma_offset_l0_cb,
                                                                                     chroma_offset_l0_cr,
                                                                                     chroma_weight_l1_cb,
                                                                                     chroma_weight_l1_cr,
                                                                                     chroma_offset_l1_cb,
                                                                                     chroma_offset_l1_cr,
                                                                                     shift,
                                                                                     lvl_shift1,
                                                                                     lvl_shift2,
                                                                                     pu_ht >> 1,
                                                                                     pu_wd >> 1);
                    }
                }
            }

            else if((weighted_pred != 0) && (bi_pred == 0))
            {
                lvl_shift1 = 0;
                if(ps_pu->b2_pred_mode == PRED_L0)
                {
                    if((0 == clr_indx) && (ai2_xfrac[0] && ai2_yfrac[0]))
                        lvl_shift1 = (1 << 13);
                }
                else
                {
                    if((0 == clr_indx) && (ai2_xfrac[1] && ai2_yfrac[1]))
                        lvl_shift1 = (1 << 13);
                }

                if(0 == clr_indx)
                {
                    shift = ps_slice_hdr->s_wt_ofst.i1_luma_log2_weight_denom
                                    + SHIFT_14_MINUS_BIT_DEPTH;

                    ps_codec->s_func_selector.ihevc_weighted_pred_uni_fptr(ps_pu->b2_pred_mode == PRED_L0 ? pi2_tmp1 : pi2_tmp2,
                                                                           pu1_dst,
                                                                           pu_wd,
                                                                           ref_strd,
                                                                           ps_pu->b2_pred_mode == PRED_L0 ? luma_weight_l0 : luma_weight_l1,
                                                                           ps_pu->b2_pred_mode == PRED_L0 ? luma_offset_l0 : luma_offset_l1,
                                                                           shift,
                                                                           lvl_shift1,
                                                                           pu_ht,
                                                                           pu_wd);
                }
                else
                {
                    shift = ps_slice_hdr->s_wt_ofst.i1_chroma_log2_weight_denom
                                    + SHIFT_14_MINUS_BIT_DEPTH;

                    if(chroma_yuv420sp_vu)
                    {
                        ps_codec->s_func_selector.ihevc_weighted_pred_chroma_uni_fptr(ps_pu->b2_pred_mode == PRED_L0 ? pi2_tmp1 : pi2_tmp2,
                                                                                      pu1_dst,
                                                                                      pu_wd,
                                                                                      ref_strd,
                                                                                      ps_pu->b2_pred_mode == PRED_L0 ? chroma_weight_l0_cr : chroma_weight_l1_cr,
                                                                                      ps_pu->b2_pred_mode == PRED_L0 ? chroma_weight_l0_cb : chroma_weight_l1_cb,
                                                                                      ps_pu->b2_pred_mode == PRED_L0 ? chroma_offset_l0_cr : chroma_offset_l1_cr,
                                                                                      ps_pu->b2_pred_mode == PRED_L0 ? chroma_offset_l0_cb : chroma_offset_l1_cb,
                                                                                      shift,
                                                                                      lvl_shift1,
                                                                                      pu_ht >> 1,
                                                                                      pu_wd >> 1);
                    }
                    else
                    {
                        ps_codec->s_func_selector.ihevc_weighted_pred_chroma_uni_fptr(ps_pu->b2_pred_mode == PRED_L0 ? pi2_tmp1 : pi2_tmp2,
                                                                                      pu1_dst,
                                                                                      pu_wd,
                                                                                      ref_strd,
                                                                                      ps_pu->b2_pred_mode == PRED_L0 ? chroma_weight_l0_cb : chroma_weight_l1_cb,
                                                                                      ps_pu->b2_pred_mode == PRED_L0 ? chroma_weight_l0_cr : chroma_weight_l1_cr,
                                                                                      ps_pu->b2_pred_mode == PRED_L0 ? chroma_offset_l0_cb : chroma_offset_l1_cb,
                                                                                      ps_pu->b2_pred_mode == PRED_L0 ? chroma_offset_l0_cr : chroma_offset_l1_cr,
                                                                                      shift,
                                                                                      lvl_shift1,
                                                                                      pu_ht >> 1,
                                                                                      pu_wd >> 1);
                    }
                }
            }

            else if((weighted_pred == 0) && (bi_pred != 0))
            {
                lvl_shift1 = 0;
                lvl_shift2 = 0;
                if((0 == clr_indx) && (ai2_xfrac[0] && ai2_yfrac[0]))
                    lvl_shift1 = (1 << 13);

                if((0 == clr_indx) && (ai2_xfrac[1] && ai2_yfrac[1]))
                    lvl_shift2 = (1 << 13);

                if(clr_indx != 0)
                {
                    pu_ht = (pu_ht >> 1);
                }
                ps_codec->s_func_selector.ihevc_weighted_pred_bi_default_fptr(pi2_tmp1,
                                                                              pi2_tmp2,
                                                                              pu1_dst,
                                                                              pu_wd,
                                                                              pu_wd,
                                                                              ref_strd,
                                                                              lvl_shift1,
                                                                              lvl_shift2,
                                                                              pu_ht,
                                                                              pu_wd);

            }
        }
    }
}
