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
 *  ihevcd_get_mv.c
 *
 * @brief
 *  Contains functions to compute motion vectors
 *
 * @author
 *  Ittiam
 *
 * @par List of Functions:
 * - ihevcd_get_mv_ctb()
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
#include "ihevcd_mv_pred.h"
#include "ihevcd_profile.h"
/**
 *******************************************************************************
 *
 * @brief
 * This function computes and stores MV's of all the PU's in CTB
 *
 * @par Description:
 * MV's of a PU will be stored in PU structure. MV computation can be merge or mv pred
 *
 * @param[in] ps_proc
 * processor context
 *
 * @param[in] pi4_ctb_top_pu_idx
 * Pointer to ctb top PU indices
 *
 * @param[in] pi4_ctb_left_pu_idx
 * Pointer to ctb left PU indices
 *
 * @param[in] pi4_ctb_top_left_pu_idx
 * Pointer to ctb top left PU indices
 *
 * @returns
 * number of PU's per ctb
 *
 * @remarks
 *
 *
 *******************************************************************************
 */

WORD32 ihevcd_get_mv_ctb(mv_ctxt_t *ps_mv_ctxt,
                         UWORD32 *pu4_ctb_top_pu_idx,
                         UWORD32 *pu4_ctb_left_pu_idx,
                         UWORD32 *pu4_ctb_top_left_pu_idx)
{

    WORD32 i;
    sps_t *ps_sps;
    pps_t *ps_pps;
    pu_t *ps_pu;
    tile_t *ps_tile;
    UWORD8 *pu1_pic_pu_map_ctb;
    WORD32 num_minpu_in_ctb;
    WORD32 ctb_start_pu_idx;
    UWORD32 *pu4_top_pu_idx, *pu4_left_pu_idx, *pu4_top_left_pu_idx;
    WORD32 pu_x_in_4x4, pu_y_in_4x4;
    WORD32 pu_x_in_4x4_single_mcl, pu_y_in_4x4_single_mcl;
    pu_mv_t s_pred_mv;
    WORD32 ctb_size, ctb_size_in_min_pu;
    WORD32 num_pu_per_ctb, pu_wd, pu_ht, pu_cnt;
    WORD32  pu_wd_single_mcl, pu_ht_single_mcl;
    UWORD32 au4_nbr_avail[MAX_CTB_SIZE / MIN_PU_SIZE
                    + 2 /* Top nbr + bot nbr */];
    UWORD32 *pu4_nbr_pu_idx/* (Left + ctb_size + right ) * (top + ctb_size + bottom) */;
    WORD32 top_avail_bits;
    UWORD8 u1_lb_avail, u1_l_avail, u1_t_avail, u1_tr_avail, u1_tl_avail;
    WORD32 nbr_pu_idx_strd;
    WORD32 cb_size;
    WORD32 single_mcl_flag;

    PROFILE_DISABLE_MV_PREDICTION();
    ps_sps = ps_mv_ctxt->ps_sps;
    ps_pps = ps_mv_ctxt->ps_pps;
    ps_pu = ps_mv_ctxt->ps_pu;
    ps_tile = ps_mv_ctxt->ps_tile;

    pu4_nbr_pu_idx = ps_mv_ctxt->pu4_pic_pu_idx_map;

    ctb_size = (1 << ps_sps->i1_log2_ctb_size);

    ctb_size_in_min_pu = (ctb_size / MIN_PU_SIZE);

    num_minpu_in_ctb = ctb_size_in_min_pu * ctb_size_in_min_pu;
    pu1_pic_pu_map_ctb = ps_mv_ctxt->pu1_pic_pu_map + (ps_mv_ctxt->i4_ctb_x + ps_mv_ctxt->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb) * num_minpu_in_ctb;

    num_pu_per_ctb = ps_mv_ctxt->i4_ctb_pu_cnt;
    ctb_start_pu_idx = ps_mv_ctxt->i4_ctb_start_pu_idx;
    nbr_pu_idx_strd = MAX_CTB_SIZE / MIN_PU_SIZE + 2;

    {
        /* Updating the initial availability map */
        WORD32 i;
        UWORD32 u4_left_ctb_avail, u4_top_lt_ctb_avail, u4_top_rt_ctb_avail,
                        u4_top_ctb_avail;

        u4_left_ctb_avail = ps_mv_ctxt->u1_left_ctb_avail;
        u4_top_lt_ctb_avail = ps_mv_ctxt->u1_top_lt_ctb_avail;
        u4_top_ctb_avail = ps_mv_ctxt->u1_top_ctb_avail;
        u4_top_rt_ctb_avail = ps_mv_ctxt->u1_top_rt_ctb_avail;

        /* Initializing the availability array */
        memset(au4_nbr_avail, 0,
               (MAX_CTB_SIZE / MIN_PU_SIZE + 2) * sizeof(UWORD32));
        /* Initializing the availability array with CTB level availability flags */
        {
            WORD32 rows_remaining = ps_sps->i2_pic_height_in_luma_samples
                            - (ps_mv_ctxt->i4_ctb_y << ps_sps->i1_log2_ctb_size);
            WORD32 ctb_size_left = MIN(ctb_size, rows_remaining);
            for(i = 0; i < ctb_size_left / MIN_PU_SIZE; i++)
            {
                au4_nbr_avail[i + 1] = (u4_left_ctb_avail << 31);
            }
        }
        au4_nbr_avail[0] |= ((u4_top_rt_ctb_avail << 31)
                        >> (1 + ctb_size_in_min_pu)); /* 1+ctb_size/4 position bit pos from msb */

        au4_nbr_avail[0] |= (u4_top_lt_ctb_avail << 31);
        {
            WORD32 cols_remaining = ps_sps->i2_pic_width_in_luma_samples
                            - (ps_mv_ctxt->i4_ctb_x << ps_sps->i1_log2_ctb_size);
            WORD32 ctb_size_top = MIN(ctb_size, cols_remaining);
            WORD32 shift = (31 - (ctb_size / MIN_TU_SIZE));

            /* ctb_size_top gives number of valid pixels remaining in the current row */
            /* Since we need pattern of 1's starting from the MSB, an additional shift */
            /* is needed */
            shift += ((ctb_size - ctb_size_top) / MIN_TU_SIZE);

            top_avail_bits = ((1 << (ctb_size_top / MIN_PU_SIZE)) - 1) << shift;
        }

        au4_nbr_avail[0] |= ((u4_top_ctb_avail == 1) ? top_avail_bits : 0x0);
        /* Starting from msb 2nd bit to (1+ctb_size/4) bit, set 1 if top avail,or 0 */

    }

    {
        /* In case of a  tile boundary, left and top arrays must change*/
        /*Left*/
        /* If start of tile row*/
        if(((ps_tile->u1_pos_x) == (ps_mv_ctxt->i4_ctb_x)) && (ps_mv_ctxt->i4_ctb_x != 0))
        {
            WORD32 index_pic_map;
            WORD32 ctb_pu_idx;
            UWORD8 *pu1_pic_pu_map;

            /* Goto the left ctb which belongs to another tile */
            index_pic_map = ((ps_mv_ctxt->i4_ctb_x - 1) + ps_mv_ctxt->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb);
            ctb_pu_idx = ps_mv_ctxt->pu4_pic_pu_idx[index_pic_map];
            index_pic_map *= num_minpu_in_ctb;

            /*Replicate the PUs of the last column of the left ctb*/
            pu1_pic_pu_map = ps_mv_ctxt->pu1_pic_pu_map + index_pic_map + ctb_size_in_min_pu - 1;
            for(i = 0; i < ctb_size_in_min_pu; i++)
            {
                /* Left neighbors change*/
                pu4_ctb_left_pu_idx[i] = ctb_pu_idx + (WORD32)*pu1_pic_pu_map;
                pu1_pic_pu_map = pu1_pic_pu_map + ctb_size_in_min_pu;
            }


            index_pic_map = ((ps_mv_ctxt->i4_ctb_x - 1) + (ps_mv_ctxt->i4_ctb_y - 1) * ps_sps->i2_pic_wd_in_ctb);
            ctb_pu_idx = ps_mv_ctxt->pu4_pic_pu_idx[index_pic_map];
            index_pic_map *= num_minpu_in_ctb;
            index_pic_map += (num_minpu_in_ctb - 1);
            pu4_ctb_top_left_pu_idx[0] = ctb_pu_idx + pu1_pic_pu_map[index_pic_map];
        }
        /*Top*/
        /* If start of tile column*/
        if(((ps_tile->u1_pos_y) == (ps_mv_ctxt->i4_ctb_y)) && (ps_mv_ctxt->i4_ctb_y != 0))
        {
            WORD32 index_pic_map;
            WORD32 ctb_pu_idx;
            UWORD8 *pu1_pic_pu_map;

            /* Goto the top ctb which belongs to another tile */
            index_pic_map =  (ps_mv_ctxt->i4_ctb_x) + ((ps_mv_ctxt->i4_ctb_y - 1) * ps_sps->i2_pic_wd_in_ctb);
            ctb_pu_idx = ps_mv_ctxt->pu4_pic_pu_idx[index_pic_map];
            index_pic_map *= num_minpu_in_ctb;

            /*Replicate the PUs of the last row of the top ctb*/
            pu1_pic_pu_map = ps_mv_ctxt->pu1_pic_pu_map + index_pic_map + (ctb_size_in_min_pu * (ctb_size_in_min_pu - 1));
            for(i = 0; i < ctb_size_in_min_pu; i++)
            {
                /* Top neighbors change*/
                pu4_ctb_top_pu_idx[i] = ctb_pu_idx + (WORD32)*pu1_pic_pu_map;
                pu1_pic_pu_map++;
            }
        }

        /* Updating the initial neighbor pu idx map */
        /* Initializing the availability array with CTB level availability flags */
        /* 16x16 array for holding pu info of the ctb, wrt the frame pu count*/
        for(i = 0; i < ctb_size_in_min_pu; i++)
        {
            /* Left */
            pu4_nbr_pu_idx[(i + 1) * nbr_pu_idx_strd] = pu4_ctb_left_pu_idx[i];
            /* Top */
            pu4_nbr_pu_idx[i + 1] = pu4_ctb_top_pu_idx[i];
        }
        /* Top right */
        pu4_nbr_pu_idx[1 + ctb_size_in_min_pu] = pu4_ctb_top_pu_idx[ctb_size_in_min_pu];

        /* Top left */
        pu4_nbr_pu_idx[0] = pu4_ctb_top_left_pu_idx[0];

    }

    /* CTB level MV pred */
    for(pu_cnt = 0; pu_cnt < num_pu_per_ctb; pu_cnt++, ps_pu++)
    {
        pu_ht = (ps_pu->b4_ht + 1) << 2;
        pu_wd = (ps_pu->b4_wd + 1) << 2;

        pu_ht_single_mcl = pu_ht;
        pu_wd_single_mcl = pu_wd;

        pu_x_in_4x4 = ps_pu->b4_pos_x;
        pu_y_in_4x4 = ps_pu->b4_pos_y;

        pu_x_in_4x4_single_mcl = pu_x_in_4x4;
        pu_y_in_4x4_single_mcl = pu_y_in_4x4;

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
        /* Below code is for merge mode, where if single_mcl_flag == 1,
         * all the prediction units of the current coding unit share a
         * single merge candidate list, which is identical to the
         * merge candidate list of the 2Nx2N prediction unit.
         */
        single_mcl_flag = 0;
        if(1 == ps_pu->b1_merge_flag)
        {
            cb_size = MAX(pu_wd_single_mcl, pu_ht_single_mcl);
            cb_size = MAX(cb_size,
                          (1 << ps_sps->i1_log2_min_coding_block_size));
            if((ps_pps->i1_log2_parallel_merge_level > 2) && cb_size == 8 && (pu_wd_single_mcl != pu_ht_single_mcl))
            {
                single_mcl_flag = 1;
                if((PART_Nx2N == ps_pu->b3_part_mode) && (1 == ps_pu->b2_part_idx))
                {
                    pu_x_in_4x4_single_mcl = pu_x_in_4x4_single_mcl - 1;
                }
                else if((PART_2NxN == ps_pu->b3_part_mode) && (1 == ps_pu->b2_part_idx))
                {
                    pu_y_in_4x4_single_mcl = pu_y_in_4x4_single_mcl - 1;
                }
                pu_ht_single_mcl = 8;
                pu_wd_single_mcl = 8;
            }
        }
        pu4_top_pu_idx = &pu4_nbr_pu_idx[(1 + pu_x_in_4x4_single_mcl)
                        + (1 + pu_y_in_4x4_single_mcl - 1) * nbr_pu_idx_strd];
        pu4_top_left_pu_idx = pu4_top_pu_idx - 1;
        pu4_left_pu_idx = pu4_top_pu_idx - 1 + nbr_pu_idx_strd;

        /* Get neibhbor availability */
        {
            u1_lb_avail = (au4_nbr_avail[1 + pu_y_in_4x4_single_mcl + pu_ht_single_mcl / MIN_PU_SIZE]
                            >> (31 - (1 + pu_x_in_4x4_single_mcl - 1))) & 1;
            u1_l_avail = (au4_nbr_avail[1 + pu_y_in_4x4_single_mcl]
                            >> (31 - (1 + pu_x_in_4x4_single_mcl - 1))) & 1;
            u1_t_avail = (au4_nbr_avail[1 + pu_y_in_4x4_single_mcl - 1]
                            >> (31 - (1 + pu_x_in_4x4_single_mcl))) & 1;
            u1_tr_avail = (au4_nbr_avail[1 + pu_y_in_4x4_single_mcl - 1]
                            >> (31 - (1 + pu_x_in_4x4_single_mcl + pu_wd_single_mcl / MIN_PU_SIZE)))
                            & 1;
            u1_tl_avail = (au4_nbr_avail[1 + pu_y_in_4x4_single_mcl - 1]
                            >> (31 - (1 + pu_x_in_4x4_single_mcl - 1))) & 1;
        }
        if(ps_pu->b1_intra_flag == 0)
        {
            if(ps_pu->b1_merge_flag == 0)
            {
                WORD32 pred_flag_l0, pred_flag_l1;
                WORD32 tmp_x, tmp_y, mvd_x, mvd_y, mvp_x, mvp_y;
                WORD32 two_pow_16, two_pow_15;

                ihevcd_mv_pred(ps_mv_ctxt, pu4_top_pu_idx, pu4_left_pu_idx,
                               pu4_top_left_pu_idx, nbr_pu_idx_strd,
                               ps_pu, u1_lb_avail, u1_l_avail,
                               u1_tr_avail, u1_t_avail, u1_tl_avail,
                               &s_pred_mv);

                pred_flag_l0 = (ps_pu->b2_pred_mode != PRED_L1);
                pred_flag_l1 = (ps_pu->b2_pred_mode != PRED_L0);

                two_pow_16 = (1 << 16);
                two_pow_15 = (1 << 15);

                /* L0 MV */
                if(pred_flag_l0)
                {
                    mvp_x = s_pred_mv.s_l0_mv.i2_mvx;
                    mvp_y = s_pred_mv.s_l0_mv.i2_mvy;
                    mvd_x = ps_pu->mv.s_l0_mv.i2_mvx;
                    mvd_y = ps_pu->mv.s_l0_mv.i2_mvy;

                    tmp_x = (mvp_x + mvd_x + two_pow_16) & (two_pow_16 - 1);
                    tmp_x = tmp_x >= two_pow_15 ?
                                    (tmp_x - two_pow_16) : tmp_x;
                    ps_pu->mv.s_l0_mv.i2_mvx = tmp_x;
                    tmp_y = (mvp_y + mvd_y + two_pow_16) & (two_pow_16 - 1);
                    tmp_y = tmp_y >= two_pow_15 ?
                                    (tmp_y - two_pow_16) : tmp_y;
                    ps_pu->mv.s_l0_mv.i2_mvy = tmp_y;
                }
                /* L1 MV */
                if(pred_flag_l1)
                {
                    mvp_x = s_pred_mv.s_l1_mv.i2_mvx;
                    mvp_y = s_pred_mv.s_l1_mv.i2_mvy;
                    mvd_x = ps_pu->mv.s_l1_mv.i2_mvx;
                    mvd_y = ps_pu->mv.s_l1_mv.i2_mvy;

                    tmp_x = (mvp_x + mvd_x + two_pow_16) & (two_pow_16 - 1);
                    tmp_x = tmp_x >= two_pow_15 ?
                                    (tmp_x - two_pow_16) : tmp_x;
                    ps_pu->mv.s_l1_mv.i2_mvx = tmp_x;
                    tmp_y = (mvp_y + mvd_y + two_pow_16) & (two_pow_16 - 1);
                    tmp_y = tmp_y >= two_pow_15 ?
                                    (tmp_y - two_pow_16) : tmp_y;
                    ps_pu->mv.s_l1_mv.i2_mvy = tmp_y;
                }
            }
            else
            {
                WORD32 part_mode;
                WORD32 part_idx;
                part_mode = ps_pu->b3_part_mode;
                //TODO: Get part_idx
                part_idx = ps_pu->b2_part_idx;

                ihevcd_mv_merge(ps_mv_ctxt, pu4_top_pu_idx, pu4_left_pu_idx,
                                nbr_pu_idx_strd, ps_pu, part_mode,
                                part_idx, pu_wd_single_mcl, pu_ht_single_mcl,
                                pu_x_in_4x4_single_mcl << 2, pu_y_in_4x4_single_mcl << 2,
                                single_mcl_flag, u1_lb_avail, u1_l_avail, u1_tr_avail,
                                u1_t_avail, u1_tl_avail);

                if(PRED_BI == ps_pu->b2_pred_mode)
                {
                    if(((ps_pu->b3_part_mode == PART_2NxN) && (pu_wd == 8))
                                    || ((ps_pu->b3_part_mode == PART_Nx2N)
                                                    && (pu_ht == 8)))
                    {
                        ps_pu->b2_pred_mode = PRED_L0;
                    }
                }
            }
        }

        {
            slice_header_t *ps_slice_hdr;
            pic_buf_t *ps_pic_buf_l0, *ps_pic_buf_l1;
            ps_slice_hdr = ps_mv_ctxt->ps_slice_hdr;
            ps_pic_buf_l0 = (pic_buf_t *)((ps_slice_hdr->as_ref_pic_list0[ps_pu->mv.i1_l0_ref_idx].pv_pic_buf));
            ps_pic_buf_l1 = (pic_buf_t *)((ps_slice_hdr->as_ref_pic_list1[ps_pu->mv.i1_l1_ref_idx].pv_pic_buf));
            ps_pu->mv.i1_l0_ref_pic_buf_id = ps_pic_buf_l0->u1_buf_id;
            if(BSLICE == ps_slice_hdr->i1_slice_type)
            {
                ps_pu->mv.i1_l1_ref_pic_buf_id = ps_pic_buf_l1->u1_buf_id;
            }
        }

        /* Neighbor availability inside CTB */
        /* 1bit per 4x4. Indicates whether that 4x4 block has been reconstructed(avialable) */
        /* Used for neighbor availability in intra pred */
        {
            WORD32 trans_in_min_tu;
            UWORD32 cur_tu_in_bits;
            UWORD32 cur_tu_avail_flag;

            trans_in_min_tu = pu_wd / MIN_PU_SIZE;
            cur_tu_in_bits = (1 << trans_in_min_tu) - 1;
            cur_tu_in_bits = cur_tu_in_bits << (32 - trans_in_min_tu);

            cur_tu_avail_flag = cur_tu_in_bits >> (pu_x_in_4x4 + 1);

            for(i = 0; i < pu_ht / MIN_PU_SIZE; i++)
                au4_nbr_avail[1 + pu_y_in_4x4 + i] |= cur_tu_avail_flag;
        }

        /* Neighbor PU idx update inside CTB */
        /* 1byte per 4x4. Indicates the PU idx that 4x4 block belongs to */

        {
            WORD32 row, col;
            UWORD32 cur_pu_idx;
            WORD32 offset;
            cur_pu_idx = ctb_start_pu_idx + pu_cnt;

            offset = (1 + pu_x_in_4x4 + 0) + (1 + pu_y_in_4x4 + 0) * nbr_pu_idx_strd;

            for(row = 0; row < pu_ht / MIN_PU_SIZE; row++)
            {
                for(col = 0; col < pu_wd / MIN_PU_SIZE; col++)
                {
                    pu4_nbr_pu_idx[offset + col] = cur_pu_idx;
                }
                offset += nbr_pu_idx_strd;
            }
        }

    }

    /* Updating Top and Left pointers */
    {
        WORD32 offset_top, offset_left;

        offset_left = ctb_size_in_min_pu + (0 + 1) * nbr_pu_idx_strd;
        offset_top = ctb_size_in_min_pu * nbr_pu_idx_strd + 0 + 1;

        /* Top Left */
        /* saving top left before updating top ptr, as updating top ptr will overwrite the top left for the next ctb */
        pu4_ctb_top_left_pu_idx[0] = pu4_ctb_top_pu_idx[ctb_size_in_min_pu - 1];

        for(i = 0; i < ctb_size_in_min_pu; i++)
        {
            /* Left */
            /* Last column of au4_nbr_pu_idx */
            pu4_ctb_left_pu_idx[i] = pu4_nbr_pu_idx[offset_left];
            /* Top */
            /* Last row of au4_nbr_pu_idx */
            pu4_ctb_top_pu_idx[i] = pu4_nbr_pu_idx[offset_top];

            offset_left += nbr_pu_idx_strd;
            offset_top += 1;
        }
    }

    /* Updating the CTB level PU idx (Used for collocated MV pred)*/
    {
        WORD32 ctb_row, ctb_col, index_pic_map, index_nbr_map;
        WORD32 first_pu_of_ctb;
        first_pu_of_ctb = pu4_nbr_pu_idx[1 + nbr_pu_idx_strd];

        index_pic_map = 0 * ctb_size_in_min_pu + 0;
        index_nbr_map = (0 + 1) * nbr_pu_idx_strd + (0 + 1);

        for(ctb_row = 0; ctb_row < ctb_size_in_min_pu; ctb_row++)
        {
            for(ctb_col = 0; ctb_col < ctb_size_in_min_pu; ctb_col++)
            {
                pu1_pic_pu_map_ctb[index_pic_map + ctb_col] = pu4_nbr_pu_idx[index_nbr_map + ctb_col]
                                - first_pu_of_ctb;
            }
            index_pic_map += ctb_size_in_min_pu;
            index_nbr_map += nbr_pu_idx_strd;
        }
    }
    return num_pu_per_ctb;
}
