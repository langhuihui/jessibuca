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
 *  ihevc_boundary_strength.c
 *
 * @brief
 *  Contains functions for computing boundary strength
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
#include "ihevcd_profile.h"

/*****************************************************************************/
/* Function Prototypes                                                       */
/*****************************************************************************/


#define SET_NGBHR_ALL_AVAIL(avail)          avail = 0x1F;

#define SET_NGBHR_BOTLEFT_NOTAVAIL(avail)   avail &= ~0x10;
#define SET_NGBHR_LEFT_NOTAVAIL(avail)      avail &= ~0x8;
#define SET_NGBHR_TOPLEFT_NOTAVAIL(avail)   avail &= ~0x4;
#define SET_NGBHR_TOP_NOTAVAIL(avail)       avail &= ~0x2;
#define SET_NGBHR_TOPRIGHT_NOTAVAIL(avail)  avail &= ~0x1;

WORD32 ihevcd_pu_boundary_strength(pu_t *ps_pu,
                                   pu_t *ps_ngbr_pu)
{
    WORD32 i4_bs;
    UWORD32 l0_ref_pic_buf_id, l1_ref_pic_buf_id;
    UWORD32 ngbr_l0_ref_pic_buf_id, ngbr_l1_ref_pic_buf_id;

    WORD16 i2_mv_x0, i2_mv_y0, i2_mv_x1, i2_mv_y1;
    WORD16 i2_ngbr_mv_x0, i2_ngbr_mv_y0, i2_ngbr_mv_x1, i2_ngbr_mv_y1;

    WORD32 num_mv, ngbr_num_mv;

    num_mv = (PRED_BI == ps_pu->b2_pred_mode) ? 2 : 1;
    ngbr_num_mv = (PRED_BI == ps_ngbr_pu->b2_pred_mode) ? 2 : 1;

    l0_ref_pic_buf_id = ps_pu->mv.i1_l0_ref_pic_buf_id;
    l1_ref_pic_buf_id = ps_pu->mv.i1_l1_ref_pic_buf_id;
    ngbr_l0_ref_pic_buf_id = ps_ngbr_pu->mv.i1_l0_ref_pic_buf_id;
    ngbr_l1_ref_pic_buf_id = ps_ngbr_pu->mv.i1_l1_ref_pic_buf_id;


    i2_mv_x0 = ps_pu->mv.s_l0_mv.i2_mvx;
    i2_mv_y0 = ps_pu->mv.s_l0_mv.i2_mvy;
    i2_mv_x1 = ps_pu->mv.s_l1_mv.i2_mvx;
    i2_mv_y1 = ps_pu->mv.s_l1_mv.i2_mvy;

    i2_ngbr_mv_x0 = ps_ngbr_pu->mv.s_l0_mv.i2_mvx;
    i2_ngbr_mv_y0 = ps_ngbr_pu->mv.s_l0_mv.i2_mvy;
    i2_ngbr_mv_x1 = ps_ngbr_pu->mv.s_l1_mv.i2_mvx;
    i2_ngbr_mv_y1 = ps_ngbr_pu->mv.s_l1_mv.i2_mvy;


    /* If two motion vectors are used */
    if((2 == num_mv) &&
            (2 == ngbr_num_mv))
    {
        if((l0_ref_pic_buf_id == ngbr_l0_ref_pic_buf_id && l1_ref_pic_buf_id == ngbr_l1_ref_pic_buf_id) ||
                (l0_ref_pic_buf_id == ngbr_l1_ref_pic_buf_id && l1_ref_pic_buf_id == ngbr_l0_ref_pic_buf_id))
        {
            if(l0_ref_pic_buf_id != l1_ref_pic_buf_id) /* Different L0 and L1 */
            {
                if(l0_ref_pic_buf_id == ngbr_l0_ref_pic_buf_id)
                {
                    i4_bs = (ABS(i2_mv_x0 - i2_ngbr_mv_x0) < 4) &&
                            (ABS(i2_mv_y0 - i2_ngbr_mv_y0) < 4) &&
                            (ABS(i2_mv_x1 - i2_ngbr_mv_x1) < 4) &&
                            (ABS(i2_mv_y1 - i2_ngbr_mv_y1) < 4) ? 0 : 1;
                }
                else
                {
                    i4_bs = (ABS(i2_mv_x0 - i2_ngbr_mv_x1) < 4) &&
                            (ABS(i2_mv_y0 - i2_ngbr_mv_y1) < 4) &&
                            (ABS(i2_mv_x1 - i2_ngbr_mv_x0) < 4) &&
                            (ABS(i2_mv_y1 - i2_ngbr_mv_y0) < 4) ? 0 : 1;
                }
            }
            else /* Same L0 and L1 */
            {
                i4_bs = ((ABS(i2_mv_x0 - i2_ngbr_mv_x0) >= 4) ||
                         (ABS(i2_mv_y0 - i2_ngbr_mv_y0) >= 4) ||
                         (ABS(i2_mv_x1 - i2_ngbr_mv_x1) >= 4) ||
                         (ABS(i2_mv_y1 - i2_ngbr_mv_y1) >= 4)) &&
                                ((ABS(i2_mv_x0 - i2_ngbr_mv_x1) >= 4) ||
                                 (ABS(i2_mv_y0 - i2_ngbr_mv_y1) >= 4) ||
                                 (ABS(i2_mv_x1 - i2_ngbr_mv_x0) >= 4) ||
                                 (ABS(i2_mv_y1 - i2_ngbr_mv_y0) >= 4)) ? 1 : 0;
            }
        }
        else /* If the reference pictures used are different */
        {
            i4_bs = 1;
        }
    }

    /* If one motion vector is used in both PUs */
    else if((1 == num_mv) &&
            (1 == ngbr_num_mv))
    {
        WORD16 i2_mv_x, i2_mv_y;
        WORD16 i2_ngbr_mv_x, i2_ngbr_mv_y;
        UWORD32 ref_pic_buf_id, ngbr_ref_pic_buf_id;

        if(PRED_L0 == ps_pu->b2_pred_mode)
        {
            i2_mv_x = i2_mv_x0;
            i2_mv_y = i2_mv_y0;
            ref_pic_buf_id = l0_ref_pic_buf_id;
        }
        else
        {
            i2_mv_x = i2_mv_x1;
            i2_mv_y = i2_mv_y1;
            ref_pic_buf_id = l1_ref_pic_buf_id;
        }

        if(PRED_L0 == ps_ngbr_pu->b2_pred_mode)
        {
            i2_ngbr_mv_x = i2_ngbr_mv_x0;
            i2_ngbr_mv_y = i2_ngbr_mv_y0;
            ngbr_ref_pic_buf_id = ngbr_l0_ref_pic_buf_id;
        }
        else
        {
            i2_ngbr_mv_x = i2_ngbr_mv_x1;
            i2_ngbr_mv_y = i2_ngbr_mv_y1;
            ngbr_ref_pic_buf_id = ngbr_l1_ref_pic_buf_id;
        }

        i4_bs = (ref_pic_buf_id == ngbr_ref_pic_buf_id) &&
                (ABS(i2_mv_x - i2_ngbr_mv_x) < 4)  &&
                (ABS(i2_mv_y - i2_ngbr_mv_y) < 4) ? 0 : 1;
    }

    /* If the no. of motion vectors is not the same */
    else
    {
        i4_bs = 1;
    }


    return i4_bs;
}

/* QP is also populated in the same function */
WORD32 ihevcd_ctb_boundary_strength_islice(bs_ctxt_t *ps_bs_ctxt)
{
    pps_t *ps_pps;
    sps_t *ps_sps;
    tu_t *ps_tu;
    UWORD32 *pu4_vert_bs;
    UWORD32 *pu4_horz_bs;
    WORD32 bs_strd;
    WORD32 vert_bs0_tmp;
    WORD32 horz_bs0_tmp;
    UWORD8 *pu1_qp;
    WORD32 qp_strd;
    UWORD32 u4_qp_const_in_ctb;
    WORD32 ctb_indx;
    WORD32 i4_tu_cnt;
    WORD32 log2_ctb_size;
    WORD32 ctb_size;

    WORD8 i1_loop_filter_across_tiles_enabled_flag;
    WORD8 i1_loop_filter_across_slices_enabled_flag;

    WORD32 i;

    PROFILE_DISABLE_BOUNDARY_STRENGTH();

    ps_pps = ps_bs_ctxt->ps_pps;
    ps_sps = ps_bs_ctxt->ps_sps;
    i1_loop_filter_across_tiles_enabled_flag = ps_pps->i1_loop_filter_across_tiles_enabled_flag;
    i1_loop_filter_across_slices_enabled_flag = ps_bs_ctxt->ps_slice_hdr->i1_slice_loop_filter_across_slices_enabled_flag;
    i4_tu_cnt = ps_bs_ctxt->i4_ctb_tu_cnt;

    log2_ctb_size = ps_sps->i1_log2_ctb_size;
    ctb_size = (1 << log2_ctb_size);

    /* strides are in units of number of bytes */
    /* ctb_size * ctb_size / 8 / 16 is the number of bytes needed per CTB */
    bs_strd = (ps_sps->i2_pic_wd_in_ctb + 1) << (2 * log2_ctb_size - 7);

    pu4_vert_bs = (UWORD32 *)((UWORD8 *)ps_bs_ctxt->pu4_pic_vert_bs +
                    (ps_bs_ctxt->i4_ctb_x << (2 * log2_ctb_size - 7)) +
                    ps_bs_ctxt->i4_ctb_y * bs_strd);
    pu4_horz_bs = (UWORD32 *)((UWORD8 *)ps_bs_ctxt->pu4_pic_horz_bs +
                    (ps_bs_ctxt->i4_ctb_x << (2 * log2_ctb_size - 7)) +
                    ps_bs_ctxt->i4_ctb_y * bs_strd);

    /* ctb_size/8 elements per CTB */
    qp_strd = ps_sps->i2_pic_wd_in_ctb << (log2_ctb_size - 3);
    pu1_qp = ps_bs_ctxt->pu1_pic_qp + ((ps_bs_ctxt->i4_ctb_x + ps_bs_ctxt->i4_ctb_y * qp_strd) << (log2_ctb_size - 3));

    ctb_indx = ps_bs_ctxt->i4_ctb_x + ps_sps->i2_pic_wd_in_ctb * ps_bs_ctxt->i4_ctb_y;
    u4_qp_const_in_ctb = ps_bs_ctxt->pu1_pic_qp_const_in_ctb[ctb_indx >> 3] & (1 << (ctb_indx & 7));

    vert_bs0_tmp = pu4_vert_bs[0] & (0xFFFFFFFF >> (sizeof(UWORD32) * 8 - ctb_size / 2));
    horz_bs0_tmp = pu4_horz_bs[0] & (0xFFFFFFFF >> (sizeof(UWORD32) * 8 - ctb_size / 2));

    /* ctb_size/8 is the number of edges per CTB
     * ctb_size/4 is the number of BS values needed per edge
     * divided by 8 for the number of bytes
     * 2 is the number of bits needed for each BS value */
/*
    memset(pu4_vert_bs, 0, (ctb_size / 8 + 1) * (ctb_size / 4) / 8 * 2 );
    memset(pu4_horz_bs, 0, (ctb_size / 8) * (ctb_size / 4) / 8 * 2 );
*/
    memset(pu4_vert_bs, 0, (1 << (2 * log2_ctb_size - 7)) + ((ctb_size >> 5) << 1));
    memset(pu4_horz_bs, 0, (1 << (2 * log2_ctb_size - 7)));

    /* pu4_vert_bs[0] has information about the left CTB which is not required when ctb_x = 0 */
    if(0 != ps_bs_ctxt->i4_ctb_x)
    {
        pu4_vert_bs[0] |= vert_bs0_tmp;
    }

    /* pu4_horz_bs[0] has information about the top CTB which is not required when ctb_y = 0 */
    if(0 != ps_bs_ctxt->i4_ctb_y)
    {
        pu4_horz_bs[0] |= horz_bs0_tmp;
    }

    ps_tu = ps_bs_ctxt->ps_tu;

    /* Populating the QP array - if const_qp_in_ctb flag is one, set only the first element */
    if(u4_qp_const_in_ctb)
        pu1_qp[0] = ps_tu->b7_qp;

    for(i = 0; i < i4_tu_cnt; i++)
    {
        WORD32 start_pos_x;
        WORD32 start_pos_y;
        WORD32 tu_size;


        UWORD32 u4_bs;
        ps_tu = ps_bs_ctxt->ps_tu + i;

        /* start_pos_x and start_pos_y are in units of min TU size (4x4) */
        start_pos_x = ps_tu->b4_pos_x;
        start_pos_y = ps_tu->b4_pos_y;

        tu_size = 1 << (ps_tu->b3_size + 2);
        tu_size >>= 2; /* TU size divided by 4 */

        u4_bs = DUP_LSB_10(tu_size);

        /* Only if the current edge falls on 8 pixel grid set BS */
        if(0 == (start_pos_x & 1))
        {
            WORD32 shift;
            shift = start_pos_y * 2;
            /* shift += (((start_pos_x >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
             *  will reduce to the following assuming ctb size is one of 16, 32 and 64
             *  and deblocking is done on 8x8 grid
             */
            if(6 != log2_ctb_size)
                shift += ((start_pos_x & 2) << (log2_ctb_size - 2));
            pu4_vert_bs[start_pos_x >> (7 - log2_ctb_size)] |= (u4_bs << shift);
        }
        /* Only if the current edge falls on 8 pixel grid set BS */
        if(0 == (start_pos_y & 1))
        {
            WORD32 shift;
            shift = start_pos_x * 2;
            /* shift += (((start_pos_y >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
             *  will reduce to the following assuming ctb size is one of 16, 32 and 64
             *  and deblocking is done on 8x8 grid
             */
            if(6 != log2_ctb_size)
                shift += ((start_pos_y & 2) << (log2_ctb_size - 2));
            pu4_horz_bs[start_pos_y >> (7 - log2_ctb_size)] |= (u4_bs << shift);
        }

        /* Populating the QP array */
        if(0 == u4_qp_const_in_ctb)
        {
            if(0 == (start_pos_x & 1) && 0 == (start_pos_y & 1))
            {
                WORD32 row, col;
                for(row = start_pos_y; row < start_pos_y + tu_size; row += 2)
                {
                    for(col = start_pos_x; col < start_pos_x + tu_size; col += 2)
                    {
                        pu1_qp[(row >> 1) * qp_strd + (col >> 1)] = ps_tu->b7_qp;
                    }
                }
            }
        }

    }
    {
        /*Determine if the slice is dependent, and is its left neighbor belongs to the same slice, in a different tile*/
        UWORD32 ctb_addr;
        WORD32 slice_idx, left_slice_idx = -1, top_slice_idx = -1;
        /* If left neighbor is not available, then set BS for entire first column to zero */
        if(!ps_pps->i1_tiles_enabled_flag)
        {
            if((0 == i1_loop_filter_across_tiles_enabled_flag && 0 == ps_bs_ctxt->i4_ctb_tile_x) ||
                            (0 == i1_loop_filter_across_slices_enabled_flag && 0 == ps_bs_ctxt->i4_ctb_slice_x && 0 == ps_bs_ctxt->i4_ctb_slice_y) ||
                            (0 == ps_bs_ctxt->i4_ctb_x))
            {
                pu4_vert_bs[0] &= (64 == ctb_size) ? 0 : ((UWORD32)0xFFFFFFFF) << (ctb_size / 2);
            }
        }
        else
        {
            //If across-tiles is disabled
            if((0 == i1_loop_filter_across_tiles_enabled_flag && 0 == ps_bs_ctxt->i4_ctb_tile_x))
            {
                pu4_vert_bs[0] &= (64 == ctb_size) ? 0 : ((UWORD32)0xFFFFFFFF) << (ctb_size / 2);
            }
            else
            {
                ctb_addr = ps_bs_ctxt->i4_ctb_x + (ps_bs_ctxt->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb);
                slice_idx = ps_bs_ctxt->pu1_slice_idx[ctb_addr];
                if(ps_bs_ctxt->i4_ctb_x)
                {
                    ctb_addr = (ps_bs_ctxt->i4_ctb_x - 1) + (ps_bs_ctxt->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb);
                    left_slice_idx = ps_bs_ctxt->pu1_slice_idx[ctb_addr];
                }
                /*If the 1st slice in a new tile is a dependent slice*/
                if(!((ps_bs_ctxt->ps_slice_hdr->i1_dependent_slice_flag == 1) && (slice_idx == left_slice_idx)))
                {
                    /* Removed reduntant checks */
                    if((0 == i1_loop_filter_across_slices_enabled_flag && (
                                    ((slice_idx != left_slice_idx) && 0 == ps_bs_ctxt->i4_ctb_slice_y) ||
                                    ((0 == ps_bs_ctxt->i4_ctb_tile_x) && (slice_idx != left_slice_idx)))) ||
                                    (0 == ps_bs_ctxt->i4_ctb_x))
                    {
                        pu4_vert_bs[0] &= (64 == ctb_size) ? 0 : ((UWORD32)0xFFFFFFFF) << (ctb_size / 2);
                    }
                }
            }
        }

        ctb_addr = ps_bs_ctxt->i4_ctb_x + (ps_bs_ctxt->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb);
        slice_idx = ps_bs_ctxt->pu1_slice_idx[ctb_addr];
        if(ps_bs_ctxt->i4_ctb_y)
        {
            ctb_addr = (ps_bs_ctxt->i4_ctb_x) + ((ps_bs_ctxt->i4_ctb_y - 1) * ps_sps->i2_pic_wd_in_ctb);
            top_slice_idx = ps_bs_ctxt->pu1_slice_idx[ctb_addr];
        }

        /* If top neighbor is not available, then set BS for entire first row to zero */
        /* Removed reduntant checks */
        if((0 == i1_loop_filter_across_tiles_enabled_flag && 0 == ps_bs_ctxt->i4_ctb_tile_y)
                        || (0 == i1_loop_filter_across_slices_enabled_flag && ((slice_idx != top_slice_idx)))
                        || (0 == ps_bs_ctxt->i4_ctb_y))
        {
            pu4_horz_bs[0] &= (64 == ctb_size) ? 0 : ((UWORD32)0xFFFFFFFF) << (ctb_size / 2);
        }
    }

    /**
     *  Set BS of bottom and right frame boundaries to zero if it is an incomplete CTB
     *   (They might have been set to  non zero values because of CBF of the current CTB)
     *   This block might not be needed for I slices*/
    {
        WORD32 num_rows_remaining = (ps_sps->i2_pic_height_in_luma_samples - (ps_bs_ctxt->i4_ctb_y << log2_ctb_size)) >> 3;
        WORD32 num_cols_remaining = (ps_sps->i2_pic_width_in_luma_samples - (ps_bs_ctxt->i4_ctb_x << log2_ctb_size)) >> 3;
        if(num_rows_remaining < (ctb_size >> 3))
        {
            /* WORD32 offset = (((num_rows_remaining >> 3) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 4));
             *  will reduce to the following assuming ctb size is one of 16, 32 and 64
             *  and deblocking is done on 8x8 grid
             */
            WORD32 offset;
            offset = (num_rows_remaining >> (6 - log2_ctb_size)) << 2;
            if(6 != log2_ctb_size)
                offset += (num_rows_remaining & 1) << (log2_ctb_size - 4);

            memset(((UWORD8 *)pu4_horz_bs) + offset, 0, 1 << (log2_ctb_size - 4));
        }

        if(num_cols_remaining < (ctb_size >> 3))
        {
            /* WORD32 offset = (((num_cols_remaining >> 3) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 4));
             *  will reduce to the following assuming ctb size is one of 16, 32 and 64
             *  and deblocking is done on 8x8 grid
             */

            WORD32 offset;
            offset = (num_cols_remaining >> (6 - log2_ctb_size)) << 2;
            if(6 != log2_ctb_size)
                offset += (num_cols_remaining & 1) << (log2_ctb_size - 4);

            memset(((UWORD8 *)pu4_vert_bs) + offset, 0, 1 << (log2_ctb_size - 4));
        }
    }

    return 0;
}
WORD32 ihevcd_ctb_boundary_strength_pbslice(bs_ctxt_t *ps_bs_ctxt)
{
    sps_t *ps_sps;
    pps_t *ps_pps;
    WORD32 cur_ctb_idx, next_ctb_idx = 0;
    WORD32 i4_tu_cnt;
    WORD32 i4_pu_cnt;
    tu_t *ps_tu;

    UWORD32 *pu4_vert_bs;
    UWORD32 *pu4_horz_bs;
    WORD32 bs_strd;
    WORD32 vert_bs0_tmp;
    WORD32 horz_bs0_tmp;
    UWORD8 *pu1_qp;
    WORD32 qp_strd;
    UWORD32 u4_qp_const_in_ctb;
    WORD32 ctb_indx;
    WORD32 log2_ctb_size;
    WORD32 ctb_size;

    WORD32 i;
    WORD8 i1_loop_filter_across_tiles_enabled_flag;
    WORD8 i1_loop_filter_across_slices_enabled_flag;

    PROFILE_DISABLE_BOUNDARY_STRENGTH();

    ps_sps = ps_bs_ctxt->ps_sps;
    ps_pps = ps_bs_ctxt->ps_pps;

    log2_ctb_size = ps_sps->i1_log2_ctb_size;
    ctb_size = (1 << log2_ctb_size);

    /* strides are in units of number of bytes */
    /* ctb_size * ctb_size / 8 / 16 is the number of bytes needed per CTB */
    bs_strd = (ps_sps->i2_pic_wd_in_ctb + 1) << (2 * log2_ctb_size - 7);

    pu4_vert_bs = (UWORD32 *)((UWORD8 *)ps_bs_ctxt->pu4_pic_vert_bs +
                    (ps_bs_ctxt->i4_ctb_x << (2 * log2_ctb_size - 7)) +
                    ps_bs_ctxt->i4_ctb_y * bs_strd);
    pu4_horz_bs = (UWORD32 *)((UWORD8 *)ps_bs_ctxt->pu4_pic_horz_bs +
                    (ps_bs_ctxt->i4_ctb_x << (2 * log2_ctb_size - 7)) +
                    ps_bs_ctxt->i4_ctb_y * bs_strd);

    vert_bs0_tmp = pu4_vert_bs[0] & (0xFFFFFFFF >> (sizeof(UWORD32) * 8 - ctb_size / 2));
    horz_bs0_tmp = pu4_horz_bs[0] & (0xFFFFFFFF >> (sizeof(UWORD32) * 8 - ctb_size / 2));

    ps_tu = ps_bs_ctxt->ps_tu;

    /* ctb_size/8 elements per CTB */
    qp_strd = ps_sps->i2_pic_wd_in_ctb << (log2_ctb_size - 3);
    pu1_qp = ps_bs_ctxt->pu1_pic_qp + ((ps_bs_ctxt->i4_ctb_x + ps_bs_ctxt->i4_ctb_y * qp_strd) << (log2_ctb_size - 3));

    ctb_indx = ps_bs_ctxt->i4_ctb_x + ps_sps->i2_pic_wd_in_ctb * ps_bs_ctxt->i4_ctb_y;
    u4_qp_const_in_ctb = ps_bs_ctxt->pu1_pic_qp_const_in_ctb[ctb_indx >> 3] & (1 << (ctb_indx & 7));

    i1_loop_filter_across_tiles_enabled_flag = ps_pps->i1_loop_filter_across_tiles_enabled_flag;
    i1_loop_filter_across_slices_enabled_flag = ps_bs_ctxt->ps_slice_hdr->i1_slice_loop_filter_across_slices_enabled_flag;

    /* ctb_size/8 is the number of edges per CTB
     * ctb_size/4 is the number of BS values needed per edge
     * divided by 8 for the number of bytes
     * 2 is the number of bits needed for each BS value */
/*
    memset(pu4_vert_bs, 0, (ctb_size / 8 + 1) * (ctb_size / 4) * 2 / 8 );
    memset(pu4_horz_bs, 0, (ctb_size / 8) * (ctb_size / 4) * 2 / 8 );
*/
    memset(pu4_vert_bs, 0, (1 << (2 * log2_ctb_size - 7)) + (ctb_size >> 4));
    memset(pu4_horz_bs, 0, (1 << (2 * log2_ctb_size - 7)));

    /* pu4_vert_bs[0] has information about the left CTB which is not required when ctb_x = 0 */
    if(0 != ps_bs_ctxt->i4_ctb_x)
    {
        pu4_vert_bs[0] |= vert_bs0_tmp;
    }

    /* pu4_horz_bs[0] has information about the top CTB which is not required when ctb_y = 0 */
    if(0 != ps_bs_ctxt->i4_ctb_y)
    {
        pu4_horz_bs[0] |= horz_bs0_tmp;
    }
    /* pu4_horz_bs[bs_strd / 4] corresponds to pu4_horz_bs[0] of the bottom CTB */
    *(UWORD32 *)((UWORD8 *)pu4_horz_bs + bs_strd) = 0;

    cur_ctb_idx = ps_bs_ctxt->i4_ctb_x
                    + ps_bs_ctxt->i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);
    next_ctb_idx = ps_bs_ctxt->i4_next_tu_ctb_cnt;
    if(1 == ps_bs_ctxt->ps_codec->i4_num_cores)
    {
        i4_tu_cnt = ps_bs_ctxt->pu4_pic_tu_idx[next_ctb_idx] - ps_bs_ctxt->pu4_pic_tu_idx[cur_ctb_idx % RESET_TU_BUF_NCTB];
    }
    else
    {
        i4_tu_cnt = ps_bs_ctxt->pu4_pic_tu_idx[next_ctb_idx] - ps_bs_ctxt->pu4_pic_tu_idx[cur_ctb_idx];
    }

    ps_tu = ps_bs_ctxt->ps_tu;
    if(u4_qp_const_in_ctb)
        pu1_qp[0] = ps_tu->b7_qp;

    /* For all TUs in the CTB For left and top edges, check if there are coded coefficients on either sides of the edge */
    for(i = 0; i < i4_tu_cnt; i++)
    {
        WORD32 start_pos_x;
        WORD32 start_pos_y;
        WORD32 end_pos_x;
        WORD32 end_pos_y;
        WORD32 tu_size;
        UWORD32 u4_bs;
        WORD32 intra_flag;
        UWORD8 *pu1_pic_intra_flag;

        ps_tu = ps_bs_ctxt->ps_tu + i;

        start_pos_x = ps_tu->b4_pos_x;
        start_pos_y = ps_tu->b4_pos_y;

        tu_size = 1 << (ps_tu->b3_size + 2);
        tu_size >>= 2;

        end_pos_x = start_pos_x + tu_size;
        end_pos_y = start_pos_y + tu_size;

        {
            WORD32 tu_abs_x = (ps_bs_ctxt->i4_ctb_x << log2_ctb_size) + (start_pos_x << 2);
            WORD32 tu_abs_y = (ps_bs_ctxt->i4_ctb_y << log2_ctb_size) + (start_pos_y << 2);

            WORD32 numbytes_row =  (ps_sps->i2_pic_width_in_luma_samples + 63) / 64;

            pu1_pic_intra_flag = ps_bs_ctxt->ps_codec->pu1_pic_intra_flag;
            pu1_pic_intra_flag += (tu_abs_y >> 3) * numbytes_row;
            pu1_pic_intra_flag += (tu_abs_x >> 6);

            intra_flag = *pu1_pic_intra_flag;
            intra_flag &= (1 << ((tu_abs_x >> 3) % 8));
        }
        if(intra_flag)
        {
            u4_bs = DUP_LSB_10(tu_size);

            /* Only if the current edge falls on 8 pixel grid set BS */
            if(0 == (start_pos_x & 1))
            {
                WORD32 shift;
                shift = start_pos_y * 2;
                /* shift += (((start_pos_x >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
                 *  will reduce to the following assuming ctb size is one of 16, 32 and 64
                 *  and deblocking is done on 8x8 grid
                 */
                if(6 != log2_ctb_size)
                    shift += ((start_pos_x & 2) << (log2_ctb_size - 2));
                pu4_vert_bs[start_pos_x >> (7 - log2_ctb_size)] |= (u4_bs << shift);
            }
            /* Only if the current edge falls on 8 pixel grid set BS */
            if(0 == (start_pos_y & 1))
            {
                WORD32 shift;
                shift = start_pos_x * 2;
                /* shift += (((start_pos_y >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
                 *  will reduce to the following assuming ctb size is one of 16, 32 and 64
                 *  and deblocking is done on 8x8 grid
                 */
                if(6 != log2_ctb_size)
                    shift += ((start_pos_y & 2) << (log2_ctb_size - 2));
                pu4_horz_bs[start_pos_y >> (7 - log2_ctb_size)] |= (u4_bs << shift);
            }
        }


        /* If the current TU is coded then set both top edge and left edge BS to 1 and go to next TU */
        if(ps_tu->b1_y_cbf)
        {
            u4_bs = DUP_LSB_01(tu_size);

            /* Only if the current edge falls on 8 pixel grid set BS */
            if(0 == (start_pos_x & 1))
            {
                WORD32 shift;
                shift = start_pos_y * 2;
                /* shift += (((start_pos_x >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
                 *  will reduce to the following assuming ctb size is one of 16, 32 and 64
                 *  and deblocking is done on 8x8 grid
                 */
                if(6 != log2_ctb_size)
                    shift += ((start_pos_x & 2) << (log2_ctb_size - 2));
                pu4_vert_bs[start_pos_x >> (7 - log2_ctb_size)] |= (u4_bs << shift);
            }
            /* Only if the current edge falls on 8 pixel grid set BS */
            if(0 == (start_pos_y & 1))
            {
                WORD32 shift;
                shift = start_pos_x * 2;
                /* shift += (((start_pos_y >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
                 *  will reduce to the following assuming ctb size is one of 16, 32 and 64
                 *  and deblocking is done on 8x8 grid
                 */
                if(6 != log2_ctb_size)
                    shift += ((start_pos_y & 2) << (log2_ctb_size - 2));
                pu4_horz_bs[start_pos_y >> (7 - log2_ctb_size)] |= (u4_bs << shift);
            }
            /* Only if the current edge falls on 8 pixel grid set BS */
            if(0 == (end_pos_x & 1))
            {
                if(!(ctb_size / 8 == (end_pos_x >> 1) && ps_bs_ctxt->i4_ctb_x == ps_sps->i2_pic_wd_in_ctb - 1))
                {
                    WORD32 shift;
                    shift = start_pos_y * 2;
                    shift += (((end_pos_x >> 1) & ((MAX_CTB_SIZE >> log2_ctb_size) - 1)) << (log2_ctb_size - 1));
                    pu4_vert_bs[end_pos_x >> (7 - log2_ctb_size)] |= (u4_bs << shift);
                }
            }
            /* Only if the current edge falls on 8 pixel grid set BS */
            if(0 == (end_pos_y & 1))
            {
                /* If end_pos_y corresponds to the bottom of the CTB, write to pu4_horz_bs[0] of the bottom CTB */
                if(ctb_size / 8 == (end_pos_y >> 1))
                {
                    *(UWORD32 *)((UWORD8 *)pu4_horz_bs + bs_strd) |= (u4_bs << (start_pos_x * 2));
                }
                else
                {
                    WORD32 shift;
                    shift = start_pos_x * 2;
                    shift += (((end_pos_y >> 1) & ((MAX_CTB_SIZE >> log2_ctb_size) - 1)) << (log2_ctb_size - 1));
                    pu4_horz_bs[end_pos_y >> (7 - log2_ctb_size)] |= (u4_bs << shift);
                }
            }
        }

        if(0 == u4_qp_const_in_ctb)
        {
            if(0 == (start_pos_x & 1) && 0 == (start_pos_y & 1))
            {
                WORD32 row, col;
                for(row = start_pos_y; row < start_pos_y + tu_size; row += 2)
                {
                    for(col = start_pos_x; col < start_pos_x + tu_size; col += 2)
                    {
                        pu1_qp[(row >> 1) * qp_strd + (col >> 1)] = ps_tu->b7_qp;
                    }
                }
            }
        }
    }

    /* For all PUs in the CTB,
    For left and top edges, compute BS */

    cur_ctb_idx = ps_bs_ctxt->i4_ctb_x
                    + ps_bs_ctxt->i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);

    {
        WORD32 next_ctb_idx;
        next_ctb_idx = ps_bs_ctxt->i4_next_pu_ctb_cnt;
        i4_pu_cnt = ps_bs_ctxt->pu4_pic_pu_idx[next_ctb_idx] - ps_bs_ctxt->pu4_pic_pu_idx[cur_ctb_idx];
    }

    for(i = 0; i < i4_pu_cnt; i++)
    {
        WORD32 start_pos_x;
        WORD32 start_pos_y;
        WORD32 end_pos_x;
        WORD32 end_pos_y;
        WORD32 pu_wd, pu_ht;
        UWORD32 u4_bs;
        pu_t *ps_pu = ps_bs_ctxt->ps_pu + i;
        pu_t *ps_ngbr_pu;
        UWORD32 u4_ngbr_pu_indx;

        start_pos_x = ps_pu->b4_pos_x;
        start_pos_y = ps_pu->b4_pos_y;

        pu_wd = (ps_pu->b4_wd + 1);
        pu_ht = (ps_pu->b4_ht + 1);

        end_pos_x = start_pos_x + pu_wd;
        end_pos_y = start_pos_y + pu_ht;

        /* If the current PU is intra, set Boundary strength as 2 for both top and left edge */
        /* Need not mask the BS to zero even if it was set to 1 already since BS 2 and 3 are assumed to be the same in leaf level functions */
        if(ps_pu->b1_intra_flag)
        {
            u4_bs = DUP_LSB_10(pu_ht);

            /* Only if the current edge falls on 8 pixel grid set BS */
            if(0 == (start_pos_x & 1))
            {
                WORD32 shift;
                shift = start_pos_y * 2;
                /* shift += (((start_pos_x >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
                 *  will reduce to the following assuming ctb size is one of 16, 32 and 64
                 *  and deblocking is done on 8x8 grid
                 */
                if(6 != log2_ctb_size)
                    shift += ((start_pos_x & 2) << (log2_ctb_size - 2));
                pu4_vert_bs[start_pos_x >> (7 - log2_ctb_size)] |= (u4_bs << shift);
            }

            u4_bs = DUP_LSB_10(pu_wd);

            /* Only if the current edge falls on 8 pixel grid set BS */
            if(0 == (start_pos_y & 1))
            {
                WORD32 shift;
                shift = start_pos_x * 2;
                /* shift += (((start_pos_y >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
                 *  will reduce to the following assuming ctb size is one of 16, 32 and 64
                 *  and deblocking is done on 8x8 grid
                 */
                if(6 != log2_ctb_size)
                    shift += ((start_pos_y & 2) << (log2_ctb_size - 2));
                pu4_horz_bs[start_pos_y >> (7 - log2_ctb_size)] |= (u4_bs << shift);
            }
        }

        else
        {
            /* Vertical edge */
            /* Process only if the edge is not a frame edge */
            if(0 != ps_bs_ctxt->i4_ctb_x + start_pos_x)
            {
                do
                {
                    WORD32 pu_ngbr_ht;
                    WORD32 min_pu_ht;
                    WORD32 ngbr_end_pos_y;
                    UWORD32 ngbr_pu_idx_strd;
                    ngbr_pu_idx_strd = MAX_CTB_SIZE / MIN_PU_SIZE + 2;
                    u4_ngbr_pu_indx = ps_bs_ctxt->pu4_pic_pu_idx_map[(start_pos_y + 1) * ngbr_pu_idx_strd + (start_pos_x)];
                    ps_ngbr_pu = ps_bs_ctxt->ps_pic_pu + u4_ngbr_pu_indx;

                    pu_ngbr_ht = ps_ngbr_pu->b4_ht + 1;
                    ngbr_end_pos_y = ps_ngbr_pu->b4_pos_y + pu_ngbr_ht;

                    min_pu_ht = MIN(ngbr_end_pos_y, end_pos_y) - start_pos_y;

                    if(ps_ngbr_pu->b1_intra_flag)
                    {
                        u4_bs = DUP_LSB_10(min_pu_ht);

                        /* Only if the current edge falls on 8 pixel grid set BS */
                        if(0 == (start_pos_x & 1))
                        {
                            WORD32 shift;
                            shift = start_pos_y * 2;
                            /* shift += (((start_pos_x >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
                             *  will reduce to the following assuming ctb size is one of 16, 32 and 64
                             *  and deblocking is done on 8x8 grid
                             */
                            if(6 != log2_ctb_size)
                                shift += ((start_pos_x & 2) << (log2_ctb_size - 2));
                            pu4_vert_bs[start_pos_x >> (7 - log2_ctb_size)] |= (u4_bs << shift);
                        }
                    }
                    else
                    {
                        u4_bs = ihevcd_pu_boundary_strength(ps_pu, ps_ngbr_pu);
                        if(u4_bs)
                        {
                            u4_bs = DUP_LSB_01(min_pu_ht);
                            if(0 == (start_pos_x & 1))
                            {
                                WORD32 shift;
                                shift = start_pos_y * 2;
                                /* shift += (((start_pos_x >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
                                 *  will reduce to the following assuming ctb size is one of 16, 32 and 64
                                 *  and deblocking is done on 8x8 grid
                                 */
                                if(6 != log2_ctb_size)
                                    shift += ((start_pos_x & 2) << (log2_ctb_size - 2));
                                pu4_vert_bs[start_pos_x >> (7 - log2_ctb_size)] |= (u4_bs << shift);
                            }
                        }
                    }

                    pu_ht -= min_pu_ht;
                    start_pos_y += min_pu_ht;
                }while(pu_ht > 0);

                /* Reinitialising since the values are updated in the previous loop */
                pu_ht = ps_pu->b4_ht + 1;
                start_pos_y = ps_pu->b4_pos_y;
            }

            /* Horizontal edge */
            /* Process only if the edge is not a frame edge */
            if(0 != ps_bs_ctxt->i4_ctb_y + start_pos_y)
            {
                do
                {
                    WORD32 pu_ngbr_wd;
                    WORD32 min_pu_wd;
                    WORD32 ngbr_end_pos_x;
                    UWORD32 ngbr_pu_idx_strd = MAX_CTB_SIZE / MIN_PU_SIZE + 2;
                    u4_ngbr_pu_indx = ps_bs_ctxt->pu4_pic_pu_idx_map[(start_pos_y)*ngbr_pu_idx_strd + (start_pos_x + 1)];
                    ps_ngbr_pu = ps_bs_ctxt->ps_pic_pu + u4_ngbr_pu_indx;

                    pu_ngbr_wd = ps_ngbr_pu->b4_wd + 1;
                    ngbr_end_pos_x = ps_ngbr_pu->b4_pos_x + pu_ngbr_wd;

                    min_pu_wd = MIN(ngbr_end_pos_x, end_pos_x) - start_pos_x;

                    if(ps_ngbr_pu->b1_intra_flag)
                    {
                        u4_bs = DUP_LSB_10(min_pu_wd);

                        /* Only if the current edge falls on 8 pixel grid set BS */
                        if(0 == (start_pos_y & 1))
                        {
                            WORD32 shift;
                            shift = start_pos_x * 2;
                            /* shift += (((start_pos_y >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
                             *  will reduce to the following assuming ctb size is one of 16, 32 and 64
                             *  and deblocking is done on 8x8 grid
                             */
                            if(6 != log2_ctb_size)
                                shift += ((start_pos_y & 2) << (log2_ctb_size - 2));
                            pu4_horz_bs[start_pos_y >> (7 - log2_ctb_size)] |= (u4_bs << shift);
                        }
                    }
                    else
                    {
                        u4_bs = ihevcd_pu_boundary_strength(ps_pu, ps_ngbr_pu);
                        if(u4_bs)
                        {
                            u4_bs = DUP_LSB_01(min_pu_wd);

                            /* Only if the current edge falls on 8 pixel grid set BS */
                            if(0 == (start_pos_y & 1))
                            {
                                WORD32 shift;
                                shift = start_pos_x * 2;
                                /* shift += (((start_pos_y >> 1) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 1));
                                 *  will reduce to the following assuming ctb size is one of 16, 32 and 64
                                 *  and deblocking is done on 8x8 grid
                                 */
                                if(6 != log2_ctb_size)
                                    shift += ((start_pos_y & 2) << (log2_ctb_size - 2));
                                pu4_horz_bs[start_pos_y >> (7 - log2_ctb_size)] |= (u4_bs << shift);
                            }
                        }
                    }

                    pu_wd -= min_pu_wd;
                    start_pos_x += min_pu_wd;
                }while(pu_wd > 0);

                /* Reinitialising since the values are updated in the previous loop */
                pu_wd = ps_pu->b4_wd + 1;
                start_pos_x = ps_pu->b4_pos_x;
            }
        }
    }

    {
        /* If left neighbor is not available, then set BS for entire first column to zero */
        UWORD32 ctb_addr;
        WORD32 slice_idx, left_slice_idx = -1, top_slice_idx = -1;

        if(!ps_pps->i1_tiles_enabled_flag)
        {
            if((0 == i1_loop_filter_across_tiles_enabled_flag && 0 == ps_bs_ctxt->i4_ctb_tile_x) ||
                            (0 == i1_loop_filter_across_slices_enabled_flag && 0 == ps_bs_ctxt->i4_ctb_slice_x && 0 == ps_bs_ctxt->i4_ctb_slice_y) ||
                            (0 == ps_bs_ctxt->i4_ctb_x))
            {
                pu4_vert_bs[0] &= (64 == ctb_size) ? 0 : ((UWORD32)0xFFFFFFFF) << (ctb_size / 2);
            }
        }
        else
        {
            if((0 == i1_loop_filter_across_tiles_enabled_flag && 0 == ps_bs_ctxt->i4_ctb_tile_x))
            {
                pu4_vert_bs[0] &= (64 == ctb_size) ? 0 : ((UWORD32)0xFFFFFFFF) << (ctb_size / 2);
            }
            else
            {

                ctb_addr = ps_bs_ctxt->i4_ctb_x + (ps_bs_ctxt->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb);
                slice_idx = ps_bs_ctxt->pu1_slice_idx[ctb_addr];

                if(ps_bs_ctxt->i4_ctb_x)
                {
                    ctb_addr = (ps_bs_ctxt->i4_ctb_x - 1) + (ps_bs_ctxt->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb);
                    left_slice_idx = ps_bs_ctxt->pu1_slice_idx[ctb_addr];
                }

                if(!((ps_bs_ctxt->ps_slice_hdr->i1_dependent_slice_flag == 1) && (slice_idx == left_slice_idx)))
                {
                    /* Removed reduntant checks */
                    if((0 == i1_loop_filter_across_slices_enabled_flag && (
                                    (0 == ps_bs_ctxt->i4_ctb_slice_x && 0 == ps_bs_ctxt->i4_ctb_slice_y) ||
                                    ((0 == ps_bs_ctxt->i4_ctb_tile_x) && (slice_idx != left_slice_idx)))) || (0 == ps_bs_ctxt->i4_ctb_x))
                    {
                        pu4_vert_bs[0] &= (64 == ctb_size) ? 0 : ((UWORD32)0xFFFFFFFF) << (ctb_size / 2);
                    }
                }
            }
        }

        ctb_addr = ps_bs_ctxt->i4_ctb_x + (ps_bs_ctxt->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb);
        slice_idx = ps_bs_ctxt->pu1_slice_idx[ctb_addr];
        if(ps_bs_ctxt->i4_ctb_y)
        {
            ctb_addr = (ps_bs_ctxt->i4_ctb_x) + ((ps_bs_ctxt->i4_ctb_y - 1) * ps_sps->i2_pic_wd_in_ctb);
            top_slice_idx = ps_bs_ctxt->pu1_slice_idx[ctb_addr];
        }
        /* If top neighbor is not available, then set BS for entire first row to zero */
        /* Removed reduntant checks */
        if((0 == i1_loop_filter_across_tiles_enabled_flag && 0 == ps_bs_ctxt->i4_ctb_tile_y)
                        || (0 == i1_loop_filter_across_slices_enabled_flag && ((slice_idx != top_slice_idx)))
                        || (0 == ps_bs_ctxt->i4_ctb_y))
        {
            pu4_horz_bs[0] &= (64 == ctb_size) ? 0 : ((UWORD32)0xFFFFFFFF) << (ctb_size / 2);
        }
    }

    /**
     *  Set BS of bottom and right frame boundaries to zero if it is an incomplete CTB
     *   (They might have set to  non zero values because of CBF of the current CTB)*/
    {
        WORD32 num_rows_remaining = (ps_sps->i2_pic_height_in_luma_samples - (ps_bs_ctxt->i4_ctb_y << log2_ctb_size)) >> 3;
        WORD32 num_cols_remaining = (ps_sps->i2_pic_width_in_luma_samples - (ps_bs_ctxt->i4_ctb_x << log2_ctb_size)) >> 3;
        if(num_rows_remaining < (ctb_size >> 3))
        {
            /* WORD32 offset = (((num_rows_remaining >> 3) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 4));
             *  will reduce to the following assuming ctb size is one of 16, 32 and 64
             *  and deblocking is done on 8x8 grid
             */
            WORD32 offset;
            offset = (num_rows_remaining >> (6 - log2_ctb_size)) << 2;
            if(6 != log2_ctb_size)
                offset += (num_rows_remaining & 1) << (log2_ctb_size - 4);

            memset(((UWORD8 *)pu4_horz_bs) + offset, 0, 1 << (log2_ctb_size - 4));
        }

        if(num_cols_remaining < (ctb_size >> 3))
        {
            /* WORD32 offset = (((num_cols_remaining >> 3) & (MAX_CTB_SIZE / ctb_size - 1)) << (log2_ctb_size - 4));
             *  will reduce to the following assuming ctb size is one of 16, 32 and 64
             *  and deblocking is done on 8x8 grid
             */

            WORD32 offset;
            offset = (num_cols_remaining >> (6 - log2_ctb_size)) << 2;
            if(6 != log2_ctb_size)
                offset += (num_cols_remaining & 1) << (log2_ctb_size - 4);

            memset(((UWORD8 *)pu4_vert_bs) + offset, 0, 1 << (log2_ctb_size - 4));
        }
    }
    return 0;
}
