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
 *  ihevcd_mv_merge.c
 *
 * @brief
 *  Contains functions for motion vector merge candidates derivation
 *
 * @author
 *  Ittiam
 *
 * @par List of Functions:
 * - ihevcd_compare_pu_mv_t()
 * - ihevcd_mv_pred_merge()
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
 * @brief Compare Motion vectors function
 *
 *
 * @par Description:
 *   Checks if MVs and Reference idx are excatly matching.
 *
 * @param[inout] ps_1
 *   motion vector 1 to be compared
 *
 * @param[in] ps_2
 *   motion vector 2 to be compared
 *
 * @returns
 *  0 : if not matching 1 : if matching
 *
 * @remarks
 *
 *******************************************************************************
 */
WORD32 ihevcd_compare_pu_t(pu_t *ps_pu_1, pu_t *ps_pu_2)
{
    WORD32 l0_match = 0, l1_match = 0;
    pu_mv_t *ps_mv_1, *ps_mv_2;
    WORD32 pred_mode_1, pred_mode_2;

    ps_mv_1 = &ps_pu_1->mv;
    ps_mv_2 = &ps_pu_2->mv;

    pred_mode_1 = ps_pu_1->b2_pred_mode;
    pred_mode_2 = ps_pu_2->b2_pred_mode;

    if(pred_mode_1 == pred_mode_2)
    {
        if(pred_mode_1 != PRED_L1)
        {
            if(ps_mv_1->i1_l0_ref_idx == ps_mv_2->i1_l0_ref_idx)
            {
                if(0 == memcmp(&ps_mv_1->s_l0_mv, &ps_mv_2->s_l0_mv, sizeof(mv_t)))
                {
                    l0_match = 1;
                }
            }
        }
        if(pred_mode_1 != PRED_L0)
        {
            if(ps_mv_1->i1_l1_ref_idx == ps_mv_2->i1_l1_ref_idx)
            {
                if(0 == memcmp(&ps_mv_1->s_l1_mv, &ps_mv_2->s_l1_mv, sizeof(mv_t)))
                {
                    l1_match = 1;
                }
            }
        }
        if(pred_mode_1 == PRED_BI)
            return (l1_match && l0_match);
        else if(pred_mode_1 == PRED_L0)
            return l0_match;
        else
            return l1_match;
    }

    return 0;
}

void ihevcd_collocated_mvp(mv_ctxt_t *ps_mv_ctxt,
                           pu_t *ps_pu,
                           mv_t *ps_mv_col,
                           WORD32 *pu4_avail_col_flag,
                           WORD32 use_pu_ref_idx,
                           WORD32 x_col,
                           WORD32 y_col)
{
    sps_t *ps_sps = ps_mv_ctxt->ps_sps;
    slice_header_t *ps_slice_hdr = ps_mv_ctxt->ps_slice_hdr;
    ref_list_t *ps_ref_list[2];
    mv_buf_t *ps_mv_buf_col;
    WORD32 xp_col, yp_col;
    WORD32 col_ctb_x, col_ctb_y;
    mv_t as_mv_col[2];
    WORD32 log2_ctb_size;
    WORD32 ctb_size;
    WORD32 avail_col;
    WORD32 col_ctb_idx, pu_cnt;
    WORD32 au4_list_col[2];
    WORD32 num_minpu_in_ctb;
    UWORD8 *pu1_pic_pu_map_ctb;
    pu_t *ps_col_pu;
    WORD32 part_pos_y;


    part_pos_y = ps_pu->b4_pos_y << 2;

    log2_ctb_size = ps_sps->i1_log2_ctb_size;
    ctb_size = (1 << log2_ctb_size);

    avail_col = 1;

    /* Initializing reference list */
    ps_ref_list[0] = ps_slice_hdr->as_ref_pic_list0;
    ps_ref_list[1] = ps_slice_hdr->as_ref_pic_list1;
    if(PSLICE == ps_slice_hdr->i1_slice_type)
        ps_ref_list[1] = ps_slice_hdr->as_ref_pic_list0;

    if((ps_slice_hdr->i1_slice_type == BSLICE) && (ps_slice_hdr->i1_collocated_from_l0_flag == 0))
    {
        /* L1 */
        ps_mv_buf_col = (mv_buf_t *)ps_ref_list[1][ps_slice_hdr->i1_collocated_ref_idx].pv_mv_buf;

    }
    else
    {
        /* L0 */
        ps_mv_buf_col = (mv_buf_t *)ps_ref_list[0][ps_slice_hdr->i1_collocated_ref_idx].pv_mv_buf;

    }
    num_minpu_in_ctb = (ctb_size / MIN_PU_SIZE) * (ctb_size / MIN_PU_SIZE);

    if(((part_pos_y >> log2_ctb_size) == (y_col >> log2_ctb_size))
                    && ((x_col + (ps_mv_ctxt->i4_ctb_x << log2_ctb_size)) < ps_sps->i2_pic_width_in_luma_samples)
                    && (((y_col + (ps_mv_ctxt->i4_ctb_y << log2_ctb_size))
                                    < ps_sps->i2_pic_height_in_luma_samples)))
    {
        xp_col = ((x_col >> 4) << 4);
        yp_col = ((y_col >> 4) << 4);
        col_ctb_x = ps_mv_ctxt->i4_ctb_x + (xp_col >> log2_ctb_size);
        col_ctb_y = ps_mv_ctxt->i4_ctb_y + (yp_col >> log2_ctb_size);
        col_ctb_idx = col_ctb_x + (col_ctb_y)*(ps_sps->i2_pic_wd_in_ctb);
        pu_cnt = ps_mv_buf_col->pu4_pic_pu_idx[col_ctb_idx];
        pu1_pic_pu_map_ctb = ps_mv_buf_col->pu1_pic_pu_map
                        + col_ctb_idx * num_minpu_in_ctb;
        if(xp_col == ctb_size)
            xp_col = 0;
        pu_cnt += pu1_pic_pu_map_ctb[(yp_col >> 2)
                        * (ctb_size / MIN_PU_SIZE) + (xp_col >> 2)];
        ps_col_pu = &ps_mv_buf_col->ps_pic_pu[pu_cnt];
    }
    else
        avail_col = 0;

    if((avail_col == 0) || (ps_col_pu->b1_intra_flag == 1)
                    || (ps_slice_hdr->i1_slice_temporal_mvp_enable_flag == 0))
    {
        pu4_avail_col_flag[0] = 0;
        pu4_avail_col_flag[1] = 0;
        ps_mv_col[0].i2_mvx = 0;
        ps_mv_col[0].i2_mvy = 0;
        ps_mv_col[1].i2_mvx = 0;
        ps_mv_col[1].i2_mvy = 0;
    }
    else
    {
        WORD32 au4_ref_idx_col[2];
        WORD32 pred_flag_l0, pred_flag_l1;
        pred_flag_l0 = (ps_col_pu->b2_pred_mode != PRED_L1);
        pred_flag_l1 = (ps_col_pu->b2_pred_mode != PRED_L0);

        if(pred_flag_l0 == 0)
        {
            as_mv_col[0] = ps_col_pu->mv.s_l1_mv;
            au4_ref_idx_col[0] = ps_col_pu->mv.i1_l1_ref_idx;
            au4_list_col[0] = 1; /* L1 */

            as_mv_col[1] = ps_col_pu->mv.s_l1_mv;
            au4_ref_idx_col[1] = ps_col_pu->mv.i1_l1_ref_idx;
            au4_list_col[1] = 1; /* L1 */
        }
        else
        {
            if(pred_flag_l1 == 0)
            {
                as_mv_col[0] = ps_col_pu->mv.s_l0_mv;
                au4_ref_idx_col[0] = ps_col_pu->mv.i1_l0_ref_idx;
                au4_list_col[0] = 0; /* L1 */

                as_mv_col[1] = ps_col_pu->mv.s_l0_mv;
                au4_ref_idx_col[1] = ps_col_pu->mv.i1_l0_ref_idx;
                au4_list_col[1] = 0; /* L1 */
            }
            else
            {
                if(1 == ps_slice_hdr->i1_low_delay_flag)
                {
                    as_mv_col[0] = ps_col_pu->mv.s_l0_mv;
                    au4_ref_idx_col[0] = ps_col_pu->mv.i1_l0_ref_idx;
                    au4_list_col[0] = 0; /* L0 */

                    as_mv_col[1] = ps_col_pu->mv.s_l1_mv;
                    au4_ref_idx_col[1] = ps_col_pu->mv.i1_l1_ref_idx;
                    au4_list_col[1] = 1; /* L1 */
                }
                else
                {
                    if(0 == ps_slice_hdr->i1_collocated_from_l0_flag)
                    {
                        as_mv_col[0] = ps_col_pu->mv.s_l0_mv;
                        au4_ref_idx_col[0] = ps_col_pu->mv.i1_l0_ref_idx;

                        as_mv_col[1] = ps_col_pu->mv.s_l0_mv;
                        au4_ref_idx_col[1] = ps_col_pu->mv.i1_l0_ref_idx;
                    }
                    else
                    {
                        as_mv_col[0] = ps_col_pu->mv.s_l1_mv;
                        au4_ref_idx_col[0] = ps_col_pu->mv.i1_l1_ref_idx;

                        as_mv_col[1] = ps_col_pu->mv.s_l1_mv;
                        au4_ref_idx_col[1] = ps_col_pu->mv.i1_l1_ref_idx;
                    }

                    au4_list_col[0] = ps_slice_hdr->i1_collocated_from_l0_flag; /* L"collocated_from_l0_flag" */
                    au4_list_col[1] = ps_slice_hdr->i1_collocated_from_l0_flag; /* L"collocated_from_l0_flag" */
                }
            }
        }
        avail_col = 1;
        {
            WORD32 cur_poc, col_poc, col_ref_poc_l0, cur_ref_poc;
            WORD32 col_ref_poc_l0_lt, cur_ref_poc_lt;
            WORD32 ref_idx_l0, ref_idx_l1;
            WORD32 slice_idx;
            pic_buf_t *ps_pic_buf;

            if(use_pu_ref_idx)
            {
                ref_idx_l0 = ps_pu->mv.i1_l0_ref_idx;
                ref_idx_l1 = ps_pu->mv.i1_l1_ref_idx;
            }
            else
            {
                ref_idx_l0 = 0;
                ref_idx_l1 = 0;
            }

            col_poc = ps_mv_buf_col->i4_abs_poc;
            cur_poc = ps_slice_hdr->i4_abs_pic_order_cnt;

            slice_idx = *(ps_mv_buf_col->pu1_pic_slice_map + col_ctb_x + col_ctb_y * ps_sps->i2_pic_wd_in_ctb);
            slice_idx &= (MAX_SLICE_HDR_CNT - 1);
            if(au4_list_col[0] == 0)
            {
                col_ref_poc_l0 =
                                ps_mv_buf_col->ai4_l0_collocated_poc[slice_idx][au4_ref_idx_col[0]];
                col_ref_poc_l0_lt =
                                (ps_mv_buf_col->ai1_l0_collocated_poc_lt[slice_idx][au4_ref_idx_col[0]] == LONG_TERM_REF);
            }
            else
            {
                col_ref_poc_l0 =
                                ps_mv_buf_col->ai4_l1_collocated_poc[slice_idx][au4_ref_idx_col[0]];
                col_ref_poc_l0_lt =
                                (ps_mv_buf_col->ai1_l1_collocated_poc_lt[slice_idx][au4_ref_idx_col[0]] == LONG_TERM_REF);
            }
            /* L0 collocated mv */
            ps_pic_buf = (pic_buf_t *)((ps_ref_list[0][ref_idx_l0].pv_pic_buf));
            cur_ref_poc = ps_pic_buf->i4_abs_poc;
            cur_ref_poc_lt = (ps_pic_buf->u1_used_as_ref == LONG_TERM_REF);

            if(cur_ref_poc_lt == col_ref_poc_l0_lt)
            {
                pu4_avail_col_flag[0] = 1;

                if(cur_ref_poc_lt || ((col_poc - col_ref_poc_l0) == (cur_poc - cur_ref_poc)))
                {
                    ps_mv_col[0] = as_mv_col[0];
                }
                else
                {
                    ps_mv_col[0] = as_mv_col[0];
                    if(col_ref_poc_l0 != col_poc)
                        ihevcd_scale_collocated_mv((mv_t *)(&ps_mv_col[0]), cur_ref_poc,
                                                   col_ref_poc_l0, col_poc, cur_poc);
                }
            }
            else
            {
                pu4_avail_col_flag[0] = 0;
                ps_mv_col[0].i2_mvx = 0;
                ps_mv_col[0].i2_mvy = 0;
            }
            if((BSLICE == ps_slice_hdr->i1_slice_type))
            {
                WORD32 col_ref_poc_l1_lt, col_ref_poc_l1;

                if(au4_list_col[1] == 0)
                {
                    col_ref_poc_l1 =
                                    ps_mv_buf_col->ai4_l0_collocated_poc[slice_idx][au4_ref_idx_col[1]];
                    col_ref_poc_l1_lt =
                                    (ps_mv_buf_col->ai1_l0_collocated_poc_lt[slice_idx][au4_ref_idx_col[1]] == LONG_TERM_REF);
                }
                else
                {
                    col_ref_poc_l1 =
                                    ps_mv_buf_col->ai4_l1_collocated_poc[slice_idx][au4_ref_idx_col[1]];
                    col_ref_poc_l1_lt =
                                    (ps_mv_buf_col->ai1_l1_collocated_poc_lt[slice_idx][au4_ref_idx_col[1]] == LONG_TERM_REF);
                }

                /* L1 collocated mv */
                ps_pic_buf = (pic_buf_t *)((ps_ref_list[1][ref_idx_l1].pv_pic_buf));
                cur_ref_poc = ps_pic_buf->i4_abs_poc;
                cur_ref_poc_lt = (ps_pic_buf->u1_used_as_ref == LONG_TERM_REF);

                if(cur_ref_poc_lt == col_ref_poc_l1_lt)
                {
                    pu4_avail_col_flag[1] = 1;

                    if(cur_ref_poc_lt || ((col_poc - col_ref_poc_l1) == (cur_poc - cur_ref_poc)))
                    {
                        ps_mv_col[1] = as_mv_col[1];
                    }
                    else
                    {
                        ps_mv_col[1] = as_mv_col[1];
                        if(col_ref_poc_l1 != col_poc)
                            ihevcd_scale_collocated_mv((mv_t *)&ps_mv_col[1], cur_ref_poc,
                                                       col_ref_poc_l1, col_poc, cur_poc);
                    }
                }
                else
                {
                    pu4_avail_col_flag[1] = 0;
                    ps_mv_col[1].i2_mvx = 0;
                    ps_mv_col[1].i2_mvy = 0;
                }
            }
            else
            {
                pu4_avail_col_flag[1] = 0;
            }
        }
    }
}


/**
 *******************************************************************************
 *
 * @brief
 * This function performs Motion Vector Merge candidates derivation
 *
 * @par Description:
 *  MV merge list is computed using neighbor mvs and colocated mv
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
 * @param[in] part_mode
 * Partition mode @sa PART_SIZE_E
 *
 * @param[in] part_idx
 * Partition idx of current partition inside CU
 *
 * @param[in] single_mcl_flag
 * Single MCL flag based on 8x8 CU and Parallel merge value
 *
 * @param[out] ps_merge_cand_list
 * pointer to store MV merge candidates list
 *
 * @returns
 * None
 * @remarks
 *
 *
 *******************************************************************************
 */
void ihevcd_mv_merge(mv_ctxt_t *ps_mv_ctxt,
                     UWORD32 *pu4_top_pu_idx,
                     UWORD32 *pu4_left_pu_idx,
                     WORD32 left_nbr_4x4_strd,
                     pu_t *ps_pu,
                     WORD32 part_mode,
                     WORD32 part_idx,
                     WORD32 part_wd,
                     WORD32 part_ht,
                     WORD32 part_pos_x,
                     WORD32 part_pos_y,
                     WORD32 single_mcl_flag,
                     WORD32 lb_avail,
                     WORD32 l_avail,
                     WORD32 tr_avail,
                     WORD32 t_avail,
                     WORD32 tl_avail)
{
    /******************************************************/
    /*      Spatial Merge Candidates                      */
    /******************************************************/
    slice_header_t *ps_slice_hdr;
    pu_t as_pu_merge_list[MAX_NUM_MERGE_CAND];
    pps_t *ps_pps;
    ref_list_t *ps_ref_list[2];
    WORD32 sum_avail_a0_a1_b0_b1 = 0; /*Sum of availability of A0, A1, B0, B1*/
    WORD32 nbr_x, nbr_y;
    WORD32 nbr_avail[MAX_NUM_MV_NBR];
    WORD32 merge_shift;
    WORD32 nbr_pu_idx;
    pu_t *ps_nbr_pu[MAX_NUM_MV_NBR];
    WORD32 max_num_merge_cand;
    WORD32 candidate_cnt;
    WORD32 pos_x_merge_shift, pos_y_merge_shift;

    ps_slice_hdr = ps_mv_ctxt->ps_slice_hdr;
    ps_pps = ps_mv_ctxt->ps_pps;
    /* Initializing reference list */
    ps_ref_list[0] = ps_slice_hdr->as_ref_pic_list0;
    ps_ref_list[1] = ps_slice_hdr->as_ref_pic_list1;
    if(PSLICE == ps_slice_hdr->i1_slice_type)
        ps_ref_list[1] = ps_slice_hdr->as_ref_pic_list0;

    candidate_cnt = 0;
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

    merge_shift = ps_pps->i1_log2_parallel_merge_level;

    /* Availability check */
    /* A1 */
    nbr_x = part_pos_x - 1;
    nbr_y = part_pos_y + part_ht - 1; /* A1 */

    nbr_pu_idx = *(pu4_left_pu_idx + ((nbr_y - part_pos_y) >> 2) * left_nbr_4x4_strd);
    ps_nbr_pu[NBR_A1] = ps_mv_ctxt->ps_pic_pu + nbr_pu_idx;

    nbr_avail[NBR_A1] = l_avail
                    && (!ps_nbr_pu[NBR_A1]->b1_intra_flag); /* A1 */

    pos_x_merge_shift = (part_pos_x >> merge_shift);
    pos_y_merge_shift = (part_pos_y >> merge_shift);
    max_num_merge_cand = ps_pu->b3_merge_idx + 1;

    {
        if(nbr_avail[NBR_A1])
        {
            /* if at same merge level */
            if(pos_x_merge_shift == (nbr_x >> merge_shift) &&
               (pos_y_merge_shift == (nbr_y >> merge_shift)))
            {
                nbr_avail[NBR_A1] = 0;
            }

            /* SPEC JCTVC-K1003_v9 version has a different way using not available       */
            /* candidates compared to software. for non square part and seconf part case */
            /* ideally nothing from the 1st partition should be used as per spec but     */
            /* HM 8.2 dev verison does not adhere to this. currenlty code fllows HM      */

            /* if single MCL is 0 , second part of 2 part in CU */
            else if((single_mcl_flag == 0) && (part_idx == 1) &&
                            ((part_mode == PART_Nx2N) || (part_mode == PART_nLx2N) ||
                                            (part_mode == PART_nRx2N)))
            {
                nbr_avail[NBR_A1] = 0;
            }
            sum_avail_a0_a1_b0_b1 += nbr_avail[NBR_A1];
            if(nbr_avail[NBR_A1])
            {
                as_pu_merge_list[candidate_cnt] = *ps_nbr_pu[NBR_A1];
                candidate_cnt++;
                if(candidate_cnt == max_num_merge_cand)
                {
                    ps_pu[0].mv = as_pu_merge_list[candidate_cnt - 1].mv;
                    ps_pu[0].b2_pred_mode = as_pu_merge_list[candidate_cnt - 1].b2_pred_mode;
                    return;
                }
            }
        }
    }

    /* B1 */
    nbr_x = part_pos_x + part_wd - 1;
    nbr_y = part_pos_y - 1;

    nbr_pu_idx = *(pu4_top_pu_idx + ((nbr_x - part_pos_x) >> 2));
    ps_nbr_pu[NBR_B1] = ps_mv_ctxt->ps_pic_pu + nbr_pu_idx;

    nbr_avail[NBR_B1] = t_avail
                    && (!ps_nbr_pu[NBR_B1]->b1_intra_flag); /* B1 */

    {
        WORD32 avail_flag;
        avail_flag = nbr_avail[NBR_B1];

        if(nbr_avail[NBR_B1])
        {
            /* if at same merge level */
            if(pos_x_merge_shift == (nbr_x >> merge_shift) &&
               (pos_y_merge_shift == (nbr_y >> merge_shift)))
            {
                nbr_avail[NBR_B1] = 0;
                avail_flag = 0;
            }

            /* if single MCL is 0 , second part of 2 part in CU */
            else if((single_mcl_flag == 0) && (part_idx == 1) &&
                            ((part_mode == PART_2NxN) || (part_mode == PART_2NxnU) ||
                                            (part_mode == PART_2NxnD)))
            {
                nbr_avail[NBR_B1] = 0;
                avail_flag = 0;
            }

            else if(nbr_avail[NBR_A1])
            {
                avail_flag = !ihevcd_compare_pu_t(ps_nbr_pu[NBR_A1], ps_nbr_pu[NBR_B1]);
            }

            sum_avail_a0_a1_b0_b1 += avail_flag;
            if(avail_flag)
            {
                as_pu_merge_list[candidate_cnt] = *ps_nbr_pu[NBR_B1];
                candidate_cnt++;
                if(candidate_cnt == max_num_merge_cand)
                {
                    ps_pu[0].mv = as_pu_merge_list[candidate_cnt - 1].mv;
                    ps_pu[0].b2_pred_mode = as_pu_merge_list[candidate_cnt - 1].b2_pred_mode;
                    return;
                }
            }
        }
    }
    /* B0 */
    nbr_x = part_pos_x + part_wd;
    nbr_y = part_pos_y - 1;

    nbr_pu_idx = *(pu4_top_pu_idx + ((nbr_x - part_pos_x) >> 2));
    ps_nbr_pu[NBR_B0] = ps_mv_ctxt->ps_pic_pu + nbr_pu_idx;

    nbr_avail[NBR_B0] = tr_avail
                    && (!ps_nbr_pu[NBR_B0]->b1_intra_flag); /* B0 */

    {
        WORD32 avail_flag;
        avail_flag = nbr_avail[NBR_B0];

        /* if at same merge level */
        if(nbr_avail[NBR_B0])
        {
            if(pos_x_merge_shift == (nbr_x >> merge_shift) &&
               (pos_y_merge_shift == (nbr_y >> merge_shift)))
            {
                nbr_avail[NBR_B0] = 0;
                avail_flag = 0;
            }
            else if(nbr_avail[NBR_B1])
            {
                avail_flag = !ihevcd_compare_pu_t(ps_nbr_pu[NBR_B1], ps_nbr_pu[NBR_B0]);
            }

            sum_avail_a0_a1_b0_b1 += avail_flag;
            if(avail_flag)
            {
                as_pu_merge_list[candidate_cnt] = *ps_nbr_pu[NBR_B0];
                candidate_cnt++;
                if(candidate_cnt == max_num_merge_cand)
                {
                    ps_pu[0].mv = as_pu_merge_list[candidate_cnt - 1].mv;
                    ps_pu[0].b2_pred_mode = as_pu_merge_list[candidate_cnt - 1].b2_pred_mode;
                    return;
                }
            }
        }
    }
    /* A0 */
    nbr_x = part_pos_x - 1;
    nbr_y = part_pos_y + part_ht; /* A0 */

    nbr_pu_idx = *(pu4_left_pu_idx + ((nbr_y - part_pos_y) >> 2) * left_nbr_4x4_strd);
    ps_nbr_pu[NBR_A0] = ps_mv_ctxt->ps_pic_pu + nbr_pu_idx;

    nbr_avail[NBR_A0] = lb_avail
                    && (!ps_nbr_pu[NBR_A0]->b1_intra_flag); /* A0 */
    {
        WORD32 avail_flag;
        avail_flag = nbr_avail[NBR_A0];

        if(nbr_avail[NBR_A0])
        {
            /* if at same merge level */
            if(pos_x_merge_shift == (nbr_x >> merge_shift) &&
                            (pos_y_merge_shift == (nbr_y >> merge_shift)))
            {
                nbr_avail[NBR_A0] = 0;
                avail_flag = 0;
            }
            else if(nbr_avail[NBR_A1])
            {
                avail_flag = !ihevcd_compare_pu_t(ps_nbr_pu[NBR_A1], ps_nbr_pu[NBR_A0]);
            }

            sum_avail_a0_a1_b0_b1 += avail_flag;
            if(avail_flag)
            {
                as_pu_merge_list[candidate_cnt] = *ps_nbr_pu[NBR_A0];
                candidate_cnt++;
                if(candidate_cnt == max_num_merge_cand)
                {
                    ps_pu[0].mv = as_pu_merge_list[candidate_cnt - 1].mv;
                    ps_pu[0].b2_pred_mode = as_pu_merge_list[candidate_cnt - 1].b2_pred_mode;
                    return;
                }
            }
        }
    }
    /* B2 */

    nbr_x = part_pos_x - 1;
    nbr_y = part_pos_y - 1; /* B2 */

    nbr_pu_idx = *(pu4_top_pu_idx + ((nbr_x - part_pos_x) >> 2));
    ps_nbr_pu[NBR_B2] = ps_mv_ctxt->ps_pic_pu + nbr_pu_idx;

    nbr_avail[NBR_B2] = tl_avail
                    && (!ps_nbr_pu[NBR_B2]->b1_intra_flag); /* B2 */

    {
        WORD32 avail_flag;
        avail_flag = nbr_avail[NBR_B2];

        if(nbr_avail[NBR_B2])
        {
            /* if at same merge level */
            if(pos_x_merge_shift == (nbr_x >> merge_shift) &&
                            (pos_y_merge_shift == (nbr_y >> merge_shift)))
            {
                nbr_avail[NBR_B2] = 0;
                avail_flag = 0;
            }
            else if(4 == sum_avail_a0_a1_b0_b1)
            {
                avail_flag = 0;
            }

            else
            {
                if(nbr_avail[NBR_A1])
                {
                    avail_flag = !ihevcd_compare_pu_t(ps_nbr_pu[NBR_A1], ps_nbr_pu[NBR_B2]);
                }

                if(avail_flag && nbr_avail[NBR_B1])
                {
                    avail_flag = !ihevcd_compare_pu_t(ps_nbr_pu[NBR_B1], ps_nbr_pu[NBR_B2]);
                }
            }

            if(avail_flag)
            {
                as_pu_merge_list[candidate_cnt] = *ps_nbr_pu[NBR_B2];
                candidate_cnt++;
                if(candidate_cnt == max_num_merge_cand)
                {
                    ps_pu[0].mv = as_pu_merge_list[candidate_cnt - 1].mv;
                    ps_pu[0].b2_pred_mode = as_pu_merge_list[candidate_cnt - 1].b2_pred_mode;
                    return;
                }
            }
        }
    }

    /***********************************************************/
    /*          Collocated MV prediction                       */
    /***********************************************************/
    {
        mv_t as_mv_col[2];
        WORD32 avail_col_flag[2] = { 0 }, x_col, y_col;
        WORD32 avail_col_l0, avail_col_l1;
//        ihevcd_collocated_mvp(ps_mv_ctxt,ps_pu,part_pos_x,part_pos_y,part_wd,part_ht,as_mv_col,avail_col_flag,0);

        /* Checking Collocated MV availability at Bottom right of PU*/
        x_col = part_pos_x + part_wd;
        y_col = part_pos_y + part_ht;
        ihevcd_collocated_mvp(ps_mv_ctxt, ps_pu, as_mv_col, avail_col_flag, 0, x_col, y_col);

        avail_col_l0 = avail_col_flag[0];
        avail_col_l1 = avail_col_flag[1];

        if(avail_col_l0 || avail_col_l1)
        {
            as_pu_merge_list[candidate_cnt].mv.s_l0_mv = as_mv_col[0];
            as_pu_merge_list[candidate_cnt].mv.s_l1_mv = as_mv_col[1];
        }

        if(avail_col_l0 == 0 || avail_col_l1 == 0)
        {
            /* Checking Collocated MV availability at Center of PU */
            x_col = part_pos_x + (part_wd >> 1);
            y_col = part_pos_y + (part_ht >> 1);
            ihevcd_collocated_mvp(ps_mv_ctxt, ps_pu, as_mv_col, avail_col_flag, 0, x_col, y_col);

            if(avail_col_l0 == 0)
            {
                as_pu_merge_list[candidate_cnt].mv.s_l0_mv = as_mv_col[0];
            }
            if(avail_col_l1 == 0)
            {
                as_pu_merge_list[candidate_cnt].mv.s_l1_mv = as_mv_col[1];
            }

            avail_col_l0 |= avail_col_flag[0];
            avail_col_l1 |= avail_col_flag[1];
        }

        as_pu_merge_list[candidate_cnt].mv.i1_l0_ref_idx = 0;
        as_pu_merge_list[candidate_cnt].mv.i1_l1_ref_idx = 0;
        as_pu_merge_list[candidate_cnt].b2_pred_mode = avail_col_l0 ? (avail_col_l1 ? PRED_BI : PRED_L0) : PRED_L1;

        candidate_cnt += (avail_col_l0 || avail_col_l1);

        if(candidate_cnt == max_num_merge_cand)
        {
            ps_pu[0].mv = as_pu_merge_list[candidate_cnt - 1].mv;
            ps_pu[0].b2_pred_mode = as_pu_merge_list[candidate_cnt - 1].b2_pred_mode;
            return;
        }

    }
    {
        WORD32 slice_type;

        slice_type = ps_slice_hdr->i1_slice_type;
        /* Colocated mv has to be added to list, if available */

        /******************************************************/
        /*      Bi pred merge candidates                      */
        /******************************************************/
        if(slice_type == BSLICE)
        {
            if((candidate_cnt > 1) && (candidate_cnt < MAX_NUM_MERGE_CAND))
            {
                WORD32 priority_list0[12] =
                    { 0, 1, 0, 2, 1, 2, 0, 3, 1, 3, 2, 3 };
                WORD32 priority_list1[12] =
                    { 1, 0, 2, 0, 2, 1, 3, 0, 3, 1, 3, 2 };
                WORD32 l0_cand, l1_cand;
                WORD32 bi_pred_idx = 0;
                WORD32 total_bi_pred_cand =
                                candidate_cnt * (candidate_cnt - 1);

                while(bi_pred_idx < total_bi_pred_cand)
                {
                    l0_cand = priority_list0[bi_pred_idx];
                    l1_cand = priority_list1[bi_pred_idx];

                    if((as_pu_merge_list[l0_cand].b2_pred_mode != PRED_L1)
                                    && (as_pu_merge_list[l1_cand].b2_pred_mode
                                                    != PRED_L0))
                    {
                        WORD8 i1_l0_ref_idx, i1_l1_ref_idx;
                        mv_t s_l0_mv, s_l1_mv;
                        pic_buf_t *ps_pic_buf_l0, *ps_pic_buf_l1;

                        i1_l0_ref_idx = as_pu_merge_list[l0_cand].mv.i1_l0_ref_idx;
                        i1_l1_ref_idx = as_pu_merge_list[l1_cand].mv.i1_l1_ref_idx;
                        ps_pic_buf_l0 = (pic_buf_t *)((ps_ref_list[0][i1_l0_ref_idx].pv_pic_buf));
                        ps_pic_buf_l1 = (pic_buf_t *)((ps_ref_list[1][i1_l1_ref_idx].pv_pic_buf));
                        s_l0_mv = as_pu_merge_list[l0_cand].mv.s_l0_mv;
                        s_l1_mv = as_pu_merge_list[l1_cand].mv.s_l1_mv;

                        if((ps_pic_buf_l0->i4_abs_poc != ps_pic_buf_l1->i4_abs_poc)
                                        || (s_l0_mv.i2_mvx != s_l1_mv.i2_mvx)
                                        || (s_l0_mv.i2_mvy != s_l1_mv.i2_mvy))
                        {
                            candidate_cnt++;
                            if(candidate_cnt == max_num_merge_cand)
                            {
                                ps_pu[0].mv.s_l0_mv = s_l0_mv;
                                ps_pu[0].mv.s_l1_mv = s_l1_mv;
                                ps_pu[0].mv.i1_l0_ref_idx = i1_l0_ref_idx;
                                ps_pu[0].mv.i1_l1_ref_idx = i1_l1_ref_idx;
                                ps_pu[0].b2_pred_mode = PRED_BI;
                                return;
                            }
                        }
                    }

                    bi_pred_idx++;

                    if((bi_pred_idx == total_bi_pred_cand)
                                    || (candidate_cnt == MAX_NUM_MERGE_CAND))
                    {
                        break;
                    }
                }
            }
        }

        /******************************************************/
        /*      Zero merge candidates                         */
        /******************************************************/
//        if(candidate_cnt < max_num_merge_cand)
        {
            WORD32 num_ref_idx;
            WORD32 zero_idx;

            zero_idx = max_num_merge_cand - candidate_cnt - 1;

            if(slice_type == PSLICE)
                num_ref_idx = ps_slice_hdr->i1_num_ref_idx_l0_active;
            else
                /* Slice type B */
                num_ref_idx = MIN(ps_slice_hdr->i1_num_ref_idx_l0_active, ps_slice_hdr->i1_num_ref_idx_l1_active);

            if(zero_idx >= num_ref_idx)
                zero_idx = 0;

            ps_pu[0].mv.i1_l0_ref_idx = zero_idx;
            if(slice_type == PSLICE)
            {
                ps_pu[0].mv.i1_l1_ref_idx = 0;
                ps_pu[0].b2_pred_mode = PRED_L0;
            }
            else /* Slice type B */
            {
                ps_pu[0].mv.i1_l1_ref_idx = zero_idx;
                ps_pu[0].b2_pred_mode = PRED_BI;
            }

            ps_pu[0].mv.s_l0_mv.i2_mvx = 0;
            ps_pu[0].mv.s_l0_mv.i2_mvy = 0;
            ps_pu[0].mv.s_l1_mv.i2_mvx = 0;
            ps_pu[0].mv.s_l1_mv.i2_mvy = 0;

            candidate_cnt++;
        }
    }

    return;
}


