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
 *  ihevcd_mv_pred.c
 *
 * @brief
 *  Contains functions for motion vector prediction
 *
 * @author
 *  Ittiam
 *
 * @par List of Functions:
 * - ihevcd_scale_mv()
 * - ihevcd_mv_pred()
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
#include "ihevc_buf_mgr.h"
#include "ihevc_dpb_mgr.h"

#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_nal.h"
#include "ihevcd_bitstream.h"
#include "ihevcd_fmt_conv.h"
#include "ihevcd_job_queue.h"
#include "ihevcd_debug.h"
#include "ihevcd_mv_merge.h"

/**
 *******************************************************************************
 *
 * @brief Function scaling motion vector
 *
 *
 * @par Description:
 *   Scales mv based on difference between current POC and current
 *   reference POC and neighbour reference poc
 *
 * @param[inout] mv
 *   motion vector to be scaled
 *
 * @param[in] cur_ref_poc
 *   Current PU refernce pic poc
 *
 * @param[in] nbr_ref_poc
 *   Neighbor PU reference pic poc
 *
 * @param[in] cur_poc
 *   Picture order count of current pic
 *
 * @returns
 *  None
 *
 * @remarks
 *
 *******************************************************************************
 */
void ihevcd_scale_mv(mv_t *ps_mv,
                     WORD32 cur_ref_poc,
                     WORD32 nbr_ref_poc,
                     WORD32 cur_poc)
{
    WORD32 td, tb, tx;
    WORD32 dist_scale_factor;
    WORD32 mvx, mvy;

    td = CLIP_S8(cur_poc - nbr_ref_poc);
    tb = CLIP_S8(cur_poc - cur_ref_poc);

    if(0 != td)
    {
        tx = (16384 + (abs(td) >> 1)) / td;

        dist_scale_factor = (tb * tx + 32) >> 6;
        dist_scale_factor = CLIP3(dist_scale_factor, -4096, 4095);

        mvx = ps_mv->i2_mvx;
        mvy = ps_mv->i2_mvy;

        mvx = SIGN(dist_scale_factor * mvx)
                        * ((abs(dist_scale_factor * mvx) + 127) >> 8);
        mvy = SIGN(dist_scale_factor * mvy)
                        * ((abs(dist_scale_factor * mvy) + 127) >> 8);

        ps_mv->i2_mvx = CLIP_S16(mvx);
        ps_mv->i2_mvy = CLIP_S16(mvy);
    }
}

/**
 *******************************************************************************
 *
 * @brief Function scaling temporal motion vector
 *
 *
 * @par Description:
 *   Scales mv based on difference between current POC and current
 *   reference POC and neighbour reference poc
 *
 * @param[inout] mv
 *   motion vector to be scaled
 *
 * @param[in] cur_ref_poc
 *   Current PU refernce pic poc
 *
 * @param[in] nbr_ref_poc
 *   Neighbor PU reference pic poc
 *
 * @param[in] cur_poc
 *   Picture order count of current pic
 *
 * @returns
 *  None
 *
 * @remarks
 *
 *******************************************************************************
 */
void ihevcd_scale_collocated_mv(mv_t *ps_mv,
                                WORD32 cur_ref_poc,
                                WORD32 col_ref_poc,
                                WORD32 col_poc,
                                WORD32 cur_poc)
{
    WORD32 td, tb, tx;
    WORD32 dist_scale_factor;
    WORD32 mvx, mvy;

    td = CLIP_S8(col_poc - col_ref_poc);
    tb = CLIP_S8(cur_poc - cur_ref_poc);

    tx = (16384 + (abs(td) >> 1)) / td;

    dist_scale_factor = (tb * tx + 32) >> 6;
    dist_scale_factor = CLIP3(dist_scale_factor, -4096, 4095);

    mvx = ps_mv->i2_mvx;
    mvy = ps_mv->i2_mvy;

    mvx = SIGN(dist_scale_factor * mvx)
                    * ((abs(dist_scale_factor * mvx) + 127) >> 8);
    mvy = SIGN(dist_scale_factor * mvy)
                    * ((abs(dist_scale_factor * mvy) + 127) >> 8);

    ps_mv->i2_mvx = CLIP_S16(mvx);
    ps_mv->i2_mvy = CLIP_S16(mvy);
}

#define CHECK_NBR_MV_ST(pi4_avail_flag, cur_ref_poc, u1_nbr_pred_flag, nbr_ref_poc,         \
                        ps_mv, ps_nbr_mv )                                                  \
{                                                                                           \
    if((u1_nbr_pred_flag) && (cur_ref_poc == nbr_ref_poc))                                  \
    {                                                                                       \
        *pi4_avail_flag = 1;                                                                \
        *ps_mv = *ps_nbr_mv;                                                                \
        break ;                                                                             \
    }                                                                                       \
}
#define CHECK_NBR_MV_LT(pi4_avail_flag, u1_cur_ref_lt, cur_poc, cur_ref_poc,                 \
                      u1_nbr_pred_flag, u1_nbr_ref_lt, nbr_ref_poc,                          \
                      ps_mv, ps_nbr_mv )                                                     \
{                                                                                            \
    WORD32 cur_lt, nbr_lt;                                                                   \
    cur_lt = (LONG_TERM_REF == (u1_cur_ref_lt));                                             \
    nbr_lt = (LONG_TERM_REF == (u1_nbr_ref_lt));                                             \
    if((u1_nbr_pred_flag) && (cur_lt == nbr_lt))                                             \
    {                                                                                        \
        *pi4_avail_flag = 1;                                                                 \
        *ps_mv = *ps_nbr_mv;                                                                 \
        if(SHORT_TERM_REF == u1_nbr_ref_lt)                                                  \
        {                                                                                    \
            ihevcd_scale_mv(ps_mv, cur_ref_poc, nbr_ref_poc,                                 \
                                                cur_poc);                                    \
        }                                                                                    \
        break ;                                                                              \
    }                                                                                        \
}


void GET_MV_NBR_ST(ref_list_t **ps_ref_pic_list, WORD32 *pi4_avail_flag, pic_buf_t *ps_cur_pic_buf_lx, pu_t **aps_nbr_pu, mv_t *ps_mv, WORD32 num_nbrs, WORD32 lx)
{
    WORD32 i, nbr_pred_lx;
    pic_buf_t *ps_nbr_pic_buf_lx;
    /* Short Term */
    /* L0 */
    if(0 == lx)
    {
        for(i = 0; i < num_nbrs; i++)
        {
            nbr_pred_lx = (PRED_L1 != aps_nbr_pu[i]->b2_pred_mode);
            ps_nbr_pic_buf_lx = (pic_buf_t *)((ps_ref_pic_list[0][aps_nbr_pu[i]->mv.i1_l0_ref_idx].pv_pic_buf));
            CHECK_NBR_MV_ST(pi4_avail_flag, ps_cur_pic_buf_lx->i4_abs_poc, nbr_pred_lx,
                            ps_nbr_pic_buf_lx->i4_abs_poc, ps_mv, &aps_nbr_pu[i]->mv.s_l0_mv);
            nbr_pred_lx = (PRED_L0 != aps_nbr_pu[i]->b2_pred_mode);

            nbr_pred_lx = (PRED_L0 != aps_nbr_pu[i]->b2_pred_mode);
            ps_nbr_pic_buf_lx = (pic_buf_t *)((ps_ref_pic_list[1][aps_nbr_pu[i]->mv.i1_l1_ref_idx].pv_pic_buf));
            CHECK_NBR_MV_ST(pi4_avail_flag, ps_cur_pic_buf_lx->i4_abs_poc, nbr_pred_lx,
                            ps_nbr_pic_buf_lx->i4_abs_poc, ps_mv, &aps_nbr_pu[i]->mv.s_l1_mv);
        }
    }
    /* L1 */
    else
    {
        for(i = 0; i < num_nbrs; i++)
        {
            nbr_pred_lx = (PRED_L0 != aps_nbr_pu[i]->b2_pred_mode);
            ps_nbr_pic_buf_lx = (pic_buf_t *)((ps_ref_pic_list[1][aps_nbr_pu[i]->mv.i1_l1_ref_idx].pv_pic_buf));
            CHECK_NBR_MV_ST(pi4_avail_flag, ps_cur_pic_buf_lx->i4_abs_poc, nbr_pred_lx,
                            ps_nbr_pic_buf_lx->i4_abs_poc, ps_mv, &aps_nbr_pu[i]->mv.s_l1_mv);

            nbr_pred_lx = (PRED_L1 != aps_nbr_pu[i]->b2_pred_mode);
            ps_nbr_pic_buf_lx = (pic_buf_t *)((ps_ref_pic_list[0][aps_nbr_pu[i]->mv.i1_l0_ref_idx].pv_pic_buf));
            CHECK_NBR_MV_ST(pi4_avail_flag, ps_cur_pic_buf_lx->i4_abs_poc, nbr_pred_lx,
                            ps_nbr_pic_buf_lx->i4_abs_poc, ps_mv, &aps_nbr_pu[i]->mv.s_l0_mv);
        }
    }
}

void GET_MV_NBR_LT(ref_list_t **ps_ref_pic_list, slice_header_t *ps_slice_hdr, WORD32 *pi4_avail_flag, pic_buf_t *ps_cur_pic_buf_lx, pu_t **aps_nbr_pu, mv_t *ps_mv, WORD32 num_nbrs, WORD32 lx)
{
    WORD32 i, nbr_pred_lx;
    pic_buf_t *ps_nbr_pic_buf_lx;
    /* Long Term*/
    /* L0 */
    if(0 == lx)
    {
        for(i = 0; i < num_nbrs; i++)
        {
            nbr_pred_lx = (PRED_L1 != aps_nbr_pu[i]->b2_pred_mode);
            ps_nbr_pic_buf_lx = (pic_buf_t *)((ps_ref_pic_list[0][aps_nbr_pu[i]->mv.i1_l0_ref_idx].pv_pic_buf));
            CHECK_NBR_MV_LT(pi4_avail_flag, ps_cur_pic_buf_lx->u1_used_as_ref, ps_slice_hdr->i4_abs_pic_order_cnt, ps_cur_pic_buf_lx->i4_abs_poc,
                            nbr_pred_lx,
                            ps_nbr_pic_buf_lx->u1_used_as_ref, ps_nbr_pic_buf_lx->i4_abs_poc,
                            ps_mv, &aps_nbr_pu[i]->mv.s_l0_mv);

            nbr_pred_lx = (PRED_L0 != aps_nbr_pu[i]->b2_pred_mode);
            ps_nbr_pic_buf_lx = (pic_buf_t *)((ps_ref_pic_list[1][aps_nbr_pu[i]->mv.i1_l1_ref_idx].pv_pic_buf));
            CHECK_NBR_MV_LT(pi4_avail_flag, ps_cur_pic_buf_lx->u1_used_as_ref, ps_slice_hdr->i4_abs_pic_order_cnt, ps_cur_pic_buf_lx->i4_abs_poc,
                            nbr_pred_lx,
                            ps_nbr_pic_buf_lx->u1_used_as_ref, ps_nbr_pic_buf_lx->i4_abs_poc,
                            ps_mv, &aps_nbr_pu[i]->mv.s_l1_mv);
        }
    }
    /* L1 */
    else
    {
        for(i = 0; i < num_nbrs; i++)
        {
            nbr_pred_lx = (PRED_L0 != aps_nbr_pu[i]->b2_pred_mode);
            ps_nbr_pic_buf_lx = (pic_buf_t *)((ps_ref_pic_list[1][aps_nbr_pu[i]->mv.i1_l1_ref_idx].pv_pic_buf));
            CHECK_NBR_MV_LT(pi4_avail_flag, ps_cur_pic_buf_lx->u1_used_as_ref, ps_slice_hdr->i4_abs_pic_order_cnt, ps_cur_pic_buf_lx->i4_abs_poc,
                            nbr_pred_lx,
                            ps_nbr_pic_buf_lx->u1_used_as_ref, ps_nbr_pic_buf_lx->i4_abs_poc,
                            ps_mv, &aps_nbr_pu[i]->mv.s_l1_mv);

            nbr_pred_lx = (PRED_L1 != aps_nbr_pu[i]->b2_pred_mode);
            ps_nbr_pic_buf_lx = (pic_buf_t *)((ps_ref_pic_list[0][aps_nbr_pu[i]->mv.i1_l0_ref_idx].pv_pic_buf));
            CHECK_NBR_MV_LT(pi4_avail_flag, ps_cur_pic_buf_lx->u1_used_as_ref, ps_slice_hdr->i4_abs_pic_order_cnt, ps_cur_pic_buf_lx->i4_abs_poc,
                            nbr_pred_lx,
                            ps_nbr_pic_buf_lx->u1_used_as_ref, ps_nbr_pic_buf_lx->i4_abs_poc,
                            ps_mv, &aps_nbr_pu[i]->mv.s_l0_mv);
        }
    }
}
/**
 *******************************************************************************
 *
 * @brief
 * This function performs Motion Vector prediction and return a list of mv
 *
 * @par Description:
 *  MV predictor list is computed using neighbor mvs and colocated mv
 *
 * @param[in] ps_ctxt
 * pointer to mv predictor context
 *
 * @param[in] ps_top_nbr_4x4
 * pointer to top 4x4 nbr structure
 *
 * @param[in] ps_left_nbr_4x4
 * pointer to left 4x4 nbr structure
 *
 * @param[in] ps_top_left_nbr_4x4
 * pointer to top left 4x4 nbr structure
 *
 * @param[in] left_nbr_4x4_strd
 * left nbr buffer stride in terms of 4x4 units
 *
 * @param[in] ps_avail_flags
 * Neighbor availability flags container
 *
 * @param[in] ps_col_mv
 * Colocated MV pointer
 *
 * @param[in] ps_pu
 * Current Partition PU strucrture pointer
 *
 * @param[inout] ps_pred_mv
 * pointer to store predicted MV list
 *
 * @returns
 * None
 * @remarks
 *
 *
 *******************************************************************************
 */
void ihevcd_mv_pred(mv_ctxt_t *ps_mv_ctxt,
                    UWORD32 *pu4_top_pu_idx,
                    UWORD32 *pu4_left_pu_idx,
                    UWORD32 *pu4_top_left_pu_idx,
                    WORD32 left_nbr_4x4_strd,
                    pu_t *ps_pu,
                    WORD32 lb_avail,
                    WORD32 l_avail,
                    WORD32 tr_avail,
                    WORD32 t_avail,
                    WORD32 tl_avail,
                    pu_mv_t *ps_pred_mv)
{
    slice_header_t *ps_slice_hdr;
    ref_list_t *ps_ref_pic_list[2];
    pu_t *ps_pic_pu;
    WORD32 max_l0_mvp_cand, max_l1_mvp_cand;
    WORD32 l0_done_flag, l1_done_flag;
    WORD32 num_l0_mvp_cand, num_l1_mvp_cand;
    WORD32 is_scaled_flag_list /* Indicates whether A0 or A1 is available */;
    WORD32 avail_a_flag[2];
    mv_t as_mv_a[2];
    WORD32 part_pos_x;
    WORD32 part_pos_y;
    WORD32 part_wd;
    WORD32 part_ht;
    pic_buf_t *ps_cur_pic_buf_l0, *ps_cur_pic_buf_l1;
    WORD32 nbr_avail[3]; /*[A0/A1] */ /* [B0/B1/B2] */
    pu_t *aps_nbr_pu[3];  /*[A0/A1] */ /* [B0/B1/B2] */
    WORD32 num_nbrs = 0;

    /*******************************************/
    /* Neighbor location: Graphical indication */
    /*                                         */
    /*          B2 _____________B1 B0          */
    /*            |               |            */
    /*            |               |            */
    /*            |               |            */
    /*            |      PU     ht|            */
    /*            |               |            */
    /*            |               |            */
    /*          A1|______wd_______|            */
    /*          A0                             */
    /*                                         */
    /*******************************************/

    ps_slice_hdr = ps_mv_ctxt->ps_slice_hdr;
    ps_pic_pu = ps_mv_ctxt->ps_pic_pu;
    max_l0_mvp_cand = ps_pu->b1_l0_mvp_idx + 1;
    max_l1_mvp_cand = ps_pu->b1_l1_mvp_idx + 1;
    num_l0_mvp_cand = 0;
    num_l1_mvp_cand = 0;

    /* Initializing reference list */
    ps_ref_pic_list[0] = ps_slice_hdr->as_ref_pic_list0;
    ps_ref_pic_list[1] = ps_slice_hdr->as_ref_pic_list1;
    if(PSLICE == ps_slice_hdr->i1_slice_type)
        ps_ref_pic_list[1] = ps_slice_hdr->as_ref_pic_list0;

    ps_cur_pic_buf_l0 = (pic_buf_t *)((ps_ref_pic_list[0][ps_pu->mv.i1_l0_ref_idx].pv_pic_buf));
    ps_cur_pic_buf_l1 = (pic_buf_t *)((ps_ref_pic_list[1][ps_pu->mv.i1_l1_ref_idx].pv_pic_buf));

    is_scaled_flag_list = 0;

    part_pos_x = ps_pu->b4_pos_x << 2;
    part_pos_y = ps_pu->b4_pos_y << 2;
    part_wd = (ps_pu->b4_wd + 1) << 2;
    part_ht = (ps_pu->b4_ht + 1) << 2;

    /************************************************************/
    /* Calculating of motion vector A from neighbors A0 and A1  */
    /************************************************************/
    {
        nbr_avail[0] = 0;
        nbr_avail[1] = 0;

        /* Pointers to A0 and A1 */
        {
            WORD32 y_a0, y_a1;
            WORD32 pu_idx_a0, pu_idx_a1;

            /* TODO: y_a0, y_a1 is coded assuming left nbr pointer starts at PU */
            y_a0 = (part_ht >> 2);
            y_a1 = ((part_ht - 1) >> 2);

            pu_idx_a0 = *(pu4_left_pu_idx + (y_a0 * left_nbr_4x4_strd));
            pu_idx_a1 = *(pu4_left_pu_idx + (y_a1 * left_nbr_4x4_strd));

            if(lb_avail && (!ps_pic_pu[pu_idx_a0].b1_intra_flag))
            {
                aps_nbr_pu[num_nbrs] = &ps_pic_pu[pu_idx_a0];
                num_nbrs++;
                nbr_avail[0] = 1;
            }
            if(l_avail && (!ps_pic_pu[pu_idx_a1].b1_intra_flag))
            {
                aps_nbr_pu[num_nbrs] = &ps_pic_pu[pu_idx_a1];
                num_nbrs++;
                nbr_avail[1] = 1;
            }
        }
        /* Setting is scaled flag based on availability of A0 and A1 */
        if((nbr_avail[0] == 1) || (nbr_avail[1]))
        {
            is_scaled_flag_list = 1;
        }

        avail_a_flag[0] = 0;
        avail_a_flag[1] = 0;

        /* L0 */
        GET_MV_NBR_ST(ps_ref_pic_list, &avail_a_flag[0], ps_cur_pic_buf_l0, aps_nbr_pu, &as_mv_a[0], num_nbrs, 0);
        if(0 == avail_a_flag[0])
        {
            GET_MV_NBR_LT(ps_ref_pic_list, ps_slice_hdr, &avail_a_flag[0], ps_cur_pic_buf_l0, aps_nbr_pu, &as_mv_a[0], num_nbrs, 0);
        }

        /* L1 */
        if(PRED_L0 != ps_pu->b2_pred_mode)
        {
            GET_MV_NBR_ST(ps_ref_pic_list, &avail_a_flag[1], ps_cur_pic_buf_l1, aps_nbr_pu, &as_mv_a[1], num_nbrs, 1);
            if(0 == avail_a_flag[1])
            {
                GET_MV_NBR_LT(ps_ref_pic_list, ps_slice_hdr, &avail_a_flag[1], ps_cur_pic_buf_l1, aps_nbr_pu, &as_mv_a[1], num_nbrs, 1);
            }
        }

        l0_done_flag = (PRED_L1 == ps_pu->b2_pred_mode);
        l1_done_flag = (PRED_L0 == ps_pu->b2_pred_mode);

        if(avail_a_flag[0])
        {
            num_l0_mvp_cand++;
            if(max_l0_mvp_cand == num_l0_mvp_cand)
            {
                ps_pred_mv->s_l0_mv = as_mv_a[0];
                l0_done_flag = 1;
            }
        }
        if(avail_a_flag[1])
        {
            num_l1_mvp_cand++;
            if(max_l1_mvp_cand == num_l1_mvp_cand)
            {
                ps_pred_mv->s_l1_mv = as_mv_a[1];
                l1_done_flag = 1;
            }
        }
        if(l0_done_flag && l1_done_flag)
            return;
    }

    /************************************************************/
    /* Calculating of motion vector B from neighbors B0 and B1  */
    /************************************************************/
    {
        WORD32 avail_b_flag[2];
        mv_t as_mv_b[2];

        /* Pointers to B0, B1 and B2 */
        {
            WORD32 x_b0, x_b1, x_b2;
            WORD32 pu_idx_b0, pu_idx_b1, pu_idx_b2;

            /* Relative co-ordiante of Xp,Yp w.r.t CTB start will work */
            /* as long as minCTB = 16                                  */
            x_b0 = (part_pos_x + part_wd);
            x_b1 = (part_pos_x + part_wd - 1);
            x_b2 = (part_pos_x - 1);
            /* Getting offset back to given pointer */
            x_b0 = x_b0 - part_pos_x;
            x_b1 = x_b1 - part_pos_x;
            x_b2 = x_b2 - part_pos_x;

            /* Below derivation are based on top pointer */
            /* is pointing first pixel of PU             */
            pu_idx_b0 = *(pu4_top_pu_idx + (x_b0 >> 2));
            pu_idx_b0 = pu_idx_b0 * tr_avail;
            pu_idx_b1 = *(pu4_top_pu_idx + (x_b1 >> 2));
            pu_idx_b1 = pu_idx_b1 * t_avail;
            /* At CTB boundary, use top-left passed in */
            if(part_pos_y)
            {
                pu_idx_b2 = *pu4_top_left_pu_idx;
            }
            else
            {
                /* Not at CTB boundary, use top and  */
                /* add correction to go to top-left */
                pu_idx_b2 = *((pu4_top_pu_idx)+(x_b2 >> 2));
            }
            pu_idx_b2 = pu_idx_b2 * tl_avail;

            num_nbrs = 0;
            nbr_avail[0] = 0;
            nbr_avail[1] = 0;
            nbr_avail[2] = 0;

            if(tr_avail && (!ps_pic_pu[pu_idx_b0].b1_intra_flag))
            {
                aps_nbr_pu[num_nbrs] = &ps_pic_pu[pu_idx_b0];
                num_nbrs++;
                nbr_avail[0] = 1;
            }
            if(t_avail && (!ps_pic_pu[pu_idx_b1].b1_intra_flag))
            {
                aps_nbr_pu[num_nbrs] = &ps_pic_pu[pu_idx_b1];
                num_nbrs++;
                nbr_avail[1] = 1;
            }
            if(tl_avail && (!ps_pic_pu[pu_idx_b2].b1_intra_flag))
            {
                aps_nbr_pu[num_nbrs] = &ps_pic_pu[pu_idx_b2];
                num_nbrs++;
                nbr_avail[2] = 1;
            }
        }

        /* L0 */
        avail_b_flag[0] = 0;
        avail_b_flag[1] = 0;

        GET_MV_NBR_ST(ps_ref_pic_list, &avail_b_flag[0], ps_cur_pic_buf_l0, aps_nbr_pu, &as_mv_b[0], num_nbrs, 0);

        /* L1 */
        if(PRED_L0 != ps_pu->b2_pred_mode)
        {
            /* B0 Short Term */
            GET_MV_NBR_ST(ps_ref_pic_list, &avail_b_flag[1], ps_cur_pic_buf_l1, aps_nbr_pu, &as_mv_b[1], num_nbrs, 1);
        }

        if(avail_b_flag[0])
        {
            if(((0 == num_l0_mvp_cand)
                            || (as_mv_a[0].i2_mvx != as_mv_b[0].i2_mvx)
                            || (as_mv_a[0].i2_mvy != as_mv_b[0].i2_mvy)))
            {
                num_l0_mvp_cand++;
                if(max_l0_mvp_cand == num_l0_mvp_cand)
                {
                    ps_pred_mv->s_l0_mv = as_mv_b[0];
                    l0_done_flag = 1;
                }
            }
        }
        if(avail_b_flag[1])
        {
            if(((0 == num_l1_mvp_cand)
                            || (as_mv_a[1].i2_mvx != as_mv_b[1].i2_mvx)
                            || (as_mv_a[1].i2_mvy != as_mv_b[1].i2_mvy)))
            {
                num_l1_mvp_cand++;
                if(max_l1_mvp_cand == num_l1_mvp_cand)
                {
                    ps_pred_mv->s_l1_mv = as_mv_b[1];
                    l1_done_flag = 1;
                }
            }
        }
        if(l0_done_flag && l1_done_flag)
            return;

        if((is_scaled_flag_list == 0) && (avail_b_flag[0] == 1))
        {
            avail_a_flag[0] = 1;
            as_mv_a[0] = as_mv_b[0];
        }
        if((is_scaled_flag_list == 0) && (avail_b_flag[1] == 1))
        {
            avail_a_flag[1] = 1;
            as_mv_a[1] = as_mv_b[1];
        }

        if(0 == is_scaled_flag_list)
        {
            avail_b_flag[0] = avail_b_flag[1] = 0;

            GET_MV_NBR_LT(ps_ref_pic_list, ps_slice_hdr, &avail_b_flag[0], ps_cur_pic_buf_l0, aps_nbr_pu, &as_mv_b[0], num_nbrs, 0);

            if(PRED_L0 != ps_pu->b2_pred_mode)
            {
                GET_MV_NBR_LT(ps_ref_pic_list, ps_slice_hdr, &avail_b_flag[1], ps_cur_pic_buf_l1, aps_nbr_pu, &as_mv_b[1], num_nbrs, 1);
            }

            if(avail_b_flag[0])
            {
                if(((0 == num_l0_mvp_cand)
                                || (as_mv_a[0].i2_mvx != as_mv_b[0].i2_mvx)
                                || (as_mv_a[0].i2_mvy != as_mv_b[0].i2_mvy)))
                {
                    num_l0_mvp_cand++;
                    if(max_l0_mvp_cand == num_l0_mvp_cand)
                    {
                        ps_pred_mv->s_l0_mv = as_mv_b[0];
                        l0_done_flag = 1;
                    }
                }
            }
            if(avail_b_flag[1])
            {
                if(((0 == num_l1_mvp_cand)
                                || (as_mv_a[1].i2_mvx != as_mv_b[1].i2_mvx)
                                || (as_mv_a[1].i2_mvy != as_mv_b[1].i2_mvy)))
                {
                    num_l1_mvp_cand++;
                    if(max_l1_mvp_cand == num_l1_mvp_cand)
                    {
                        ps_pred_mv->s_l1_mv = as_mv_b[1];
                        l1_done_flag = 1;
                    }
                }
            }
            if(l0_done_flag && l1_done_flag)
                return;
        }
        /***********************************************************/
        /*          Collocated MV prediction                       */
        /***********************************************************/
        if((2 != num_l0_mvp_cand) || (2 != num_l1_mvp_cand))
        {
            mv_t as_mv_col[2], s_mv_col_l0, s_mv_col_l1;
            WORD32 avail_col_flag[2] = { 0 };
            WORD32 x_col, y_col, avail_col_l0, avail_col_l1;
//            ihevcd_collocated_mvp((mv_ctxt_t *)ps_mv_ctxt,ps_pu,part_pos_x,part_pos_y,part_wd,part_ht,as_mv_col,avail_col_flag,1);
            x_col = part_pos_x + part_wd;
            y_col = part_pos_y + part_ht;
            ihevcd_collocated_mvp(ps_mv_ctxt, ps_pu, as_mv_col, avail_col_flag, 1, x_col, y_col);

            avail_col_l0 = avail_col_flag[0];
            avail_col_l1 = avail_col_flag[1];
            if(avail_col_l0 || avail_col_l1)
            {
                s_mv_col_l0 = as_mv_col[0];
                s_mv_col_l1 = as_mv_col[1];
            }

            if(avail_col_l0 == 0 || avail_col_l1 == 0)
            {
                /* Checking Collocated MV availability at Center of PU */
                x_col = part_pos_x + (part_wd >> 1);
                y_col = part_pos_y + (part_ht >> 1);
                ihevcd_collocated_mvp(ps_mv_ctxt, ps_pu, as_mv_col, avail_col_flag, 1, x_col, y_col);

                if(avail_col_l0 == 0)
                {
                    s_mv_col_l0 = as_mv_col[0];
                }
                if(avail_col_l1 == 0)
                {
                    s_mv_col_l1 = as_mv_col[1];
                }

                avail_col_l0 |= avail_col_flag[0];
                avail_col_l1 |= avail_col_flag[1];
            }

            /* Checking if mvp index matches collocated mv */
            if(avail_col_l0)
            {
                if(2 != num_l0_mvp_cand)
                {
                    num_l0_mvp_cand++;
                    if(max_l0_mvp_cand == num_l0_mvp_cand)
                    {
                        ps_pred_mv->s_l0_mv = s_mv_col_l0;
                        l0_done_flag = 1;
                    }
                }
            }
            if(avail_col_l1)
            {
                if(2 != num_l1_mvp_cand)
                {
                    num_l1_mvp_cand++;
                    if(max_l1_mvp_cand == num_l1_mvp_cand)
                    {
                        ps_pred_mv->s_l1_mv = s_mv_col_l1;
                        l1_done_flag = 1;
                    }
                }
            }
            if(l0_done_flag && l1_done_flag)
                return;
        }

        if(0 == l0_done_flag)
        {
            ps_pred_mv->s_l0_mv.i2_mvx = 0;
            ps_pred_mv->s_l0_mv.i2_mvy = 0;
        }
        if(0 == l1_done_flag)
        {
            ps_pred_mv->s_l1_mv.i2_mvx = 0;
            ps_pred_mv->s_l1_mv.i2_mvy = 0;
        }
    }
}
