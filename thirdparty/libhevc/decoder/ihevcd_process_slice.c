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
 *  ihevcd_process_slice.c
 *
 * @brief
 *  Contains functions for processing slice data
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
#include "ihevc_defs.h"
#include "ihevc_structs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_cabac_tables.h"
#include "ihevc_padding.h"
#include "ihevc_iquant_itrans_recon.h"
#include "ihevc_chroma_iquant_itrans_recon.h"
#include "ihevc_recon.h"
#include "ihevc_chroma_recon.h"
#include "ihevc_iquant_recon.h"
#include "ihevc_chroma_iquant_recon.h"
#include "ihevc_intra_pred.h"

#include "ihevc_error.h"
#include "ihevc_common_tables.h"
#include "ihevc_quant_tables.h"
#include "ihevcd_common_tables.h"

#include "ihevcd_profile.h"
#include "ihevcd_trace.h"
#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_nal.h"
#include "ihevcd_bitstream.h"
#include "ihevcd_job_queue.h"
#include "ihevcd_utils.h"
#include "ihevcd_debug.h"
#include "ihevcd_get_mv.h"
#include "ihevcd_inter_pred.h"
#include "ihevcd_iquant_itrans_recon_ctb.h"
#include "ihevcd_boundary_strength.h"
#include "ihevcd_deblk.h"
#include "ihevcd_fmt_conv.h"
#include "ihevcd_sao.h"
#include "ihevcd_profile.h"

IHEVCD_ERROR_T ihevcd_fmt_conv(codec_t *ps_codec,
                               process_ctxt_t *ps_proc,
                               UWORD8 *pu1_y_dst,
                               UWORD8 *pu1_u_dst,
                               UWORD8 *pu1_v_dst,
                               WORD32 cur_row,
                               WORD32 num_rows);

typedef enum
{
    PROC_ALL,
    PROC_INTER_PRED,
    PROC_RECON,
    PROC_DEBLK,
    PROC_SAO
}proc_type_t;

void ihevcd_proc_map_check(process_ctxt_t *ps_proc, proc_type_t proc_type, WORD32 nctb)
{
    tile_t *ps_tile = ps_proc->ps_tile;
    sps_t *ps_sps = ps_proc->ps_sps;
    pps_t *ps_pps = ps_proc->ps_pps;
    codec_t *ps_codec = ps_proc->ps_codec;
    WORD32 idx;
    WORD32 nop_cnt;
    WORD32 bit_pos = proc_type;
    WORD32 bit_mask = (1 << bit_pos);

    if(ps_proc->i4_check_proc_status)
    {
        nop_cnt = PROC_NOP_CNT;
        while(1)
        {
            volatile UWORD8 *pu1_buf;
            volatile WORD32 status;
            status = 1;
            /* Check if all dependencies for the next nCTBs are met */
            {
                WORD32 x_pos;

                {
                    /* Check if the top right of next nCTBs are processed */
                    if(ps_proc->i4_ctb_y > 0)
                    {
                        x_pos = (ps_proc->i4_ctb_tile_x + nctb);
                        idx = MIN(x_pos, (ps_tile->u2_wd - 1));

                        /* Check if top-right CTB for the last CTB in nCTB is within the tile */
                        {
                            idx += ps_tile->u1_pos_x;
                            idx += ((ps_proc->i4_ctb_y - 1)
                                            * ps_sps->i2_pic_wd_in_ctb);
                            pu1_buf = (ps_codec->pu1_proc_map + idx);
                            status = *pu1_buf & bit_mask;
                        }
                    }
                }

                /* If tiles are enabled, then test left and top-left as well */
                ps_pps = ps_proc->ps_pps;
                if(ps_pps->i1_tiles_enabled_flag)
                {
                    /*Check if left ctb is processed*/
                    if((ps_proc->i4_ctb_x > 0) && ((0 != status)))
                    {
                        x_pos   = ps_tile->u1_pos_x + ps_proc->i4_ctb_tile_x - 1;
                        idx     = x_pos + (ps_proc->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb);
                        pu1_buf = (ps_codec->pu1_proc_map + idx);
                        status  = *pu1_buf & bit_mask;
                    }

                    /*Check if top left ctb is processed*/
                    if((ps_proc->i4_ctb_x > 0) && (0 != status) && (ps_proc->i4_ctb_y > 0))
                    {
                        x_pos   = ps_tile->u1_pos_x + ps_proc->i4_ctb_tile_x - 1;
                        idx     = x_pos + ((ps_proc->i4_ctb_y - 1) * ps_sps->i2_pic_wd_in_ctb);
                        pu1_buf = (ps_codec->pu1_proc_map + idx);
                        status  = *pu1_buf & bit_mask;
                    }
                }
            }

            if(status)
                break;

            /* if dependencies are not met, then wait for few cycles.
             * Even after few iterations, if the dependencies are not met then yield
             */
            if(nop_cnt > 0)
            {
                NOP(128);
                nop_cnt -= 128;
            }
            else
            {
                nop_cnt = PROC_NOP_CNT;
                ithread_yield();
                //NOP(128 * 16);
            }
        }
        DATA_SYNC();
    }
}

void ihevcd_proc_map_update(process_ctxt_t *ps_proc, proc_type_t proc_type, WORD32 nctb)
{
    codec_t *ps_codec = ps_proc->ps_codec;
    WORD32 i, idx;
    WORD32 bit_pos = proc_type;
    WORD32 bit_mask = (1 << bit_pos);

    /* Update the current CTBs processing status */
    if(ps_proc->i4_check_proc_status)
    {
        DATA_SYNC();
        for(i = 0; i < nctb; i++)
        {
            sps_t *ps_sps = ps_proc->ps_sps;
            UWORD8 *pu1_buf;
            idx = (ps_proc->i4_ctb_x + i);
            idx += ((ps_proc->i4_ctb_y) * ps_sps->i2_pic_wd_in_ctb);
            pu1_buf = (ps_codec->pu1_proc_map + idx);
            *pu1_buf = *pu1_buf | bit_mask;
        }
    }
}


void ihevcd_slice_hdr_update(process_ctxt_t *ps_proc)
{

    /* Slice x and y are initialized in proc_init. But initialize slice x and y count here
     *  if a new slice begins at the middle of a row since proc_init is invoked only at the beginning of each row */
    if(!((ps_proc->i4_ctb_x == 0) && (ps_proc->i4_ctb_y == 0)))
    {
        slice_header_t *ps_slice_hdr_next = ps_proc->ps_codec->ps_slice_hdr_base + ((ps_proc->i4_cur_slice_idx + 1) & (MAX_SLICE_HDR_CNT - 1));

        if((ps_slice_hdr_next->i2_ctb_x == ps_proc->i4_ctb_x)
                        && (ps_slice_hdr_next->i2_ctb_y == ps_proc->i4_ctb_y))
        {
            if(0 == ps_slice_hdr_next->i1_dependent_slice_flag)
            {
                ps_proc->i4_ctb_slice_x = 0;
                ps_proc->i4_ctb_slice_y = 0;
            }

            ps_proc->i4_cur_slice_idx++;
            ps_proc->ps_slice_hdr = ps_slice_hdr_next;
        }

    }
}

void ihevcd_ctb_pos_update(process_ctxt_t *ps_proc, WORD32 nctb)
{
    WORD32 tile_start_ctb_idx, slice_start_ctb_idx;
    slice_header_t *ps_slice_hdr = ps_proc->ps_slice_hdr;
    tile_t *ps_tile = ps_proc->ps_tile;
    sps_t *ps_sps = ps_proc->ps_sps;

    /* Update x and y positions */
    ps_proc->i4_ctb_tile_x += nctb;
    ps_proc->i4_ctb_x += nctb;

    ps_proc->i4_ctb_slice_x += nctb;
    /*If tile are enabled, then handle the tile & slice counters differently*/
    if(ps_proc->ps_pps->i1_tiles_enabled_flag)
    {
        /* Update slice counters*/
        slice_start_ctb_idx = ps_slice_hdr->i2_ctb_x + (ps_slice_hdr->i2_ctb_y * ps_sps->i2_pic_wd_in_ctb);
        tile_start_ctb_idx = ps_tile->u1_pos_x + (ps_tile->u1_pos_y * ps_sps->i2_pic_wd_in_ctb);
        /*
         * There can be 2 cases where slice counters must be handled differently.
         * 1 - Multiple tiles span across a single/one of the many slice.
         * 2 - Multiple slices span across a single/one of the many tiles.
         */

        /*Case 1 */
        if(slice_start_ctb_idx < tile_start_ctb_idx)
        {
            /*End of tile row*/
            if(ps_proc->i4_ctb_x > ps_slice_hdr->i2_ctb_x)
            {
                if(ps_proc->i4_ctb_slice_x >= (ps_tile->u2_wd + ps_tile->u1_pos_x))
                {
                    ps_proc->i4_ctb_slice_y++;
                    ps_proc->i4_ctb_slice_x = ps_proc->i4_ctb_slice_x
                                    - ps_tile->u2_wd;
                }
            }
            else
            {
                WORD32 temp_stride = (ps_sps->i2_pic_wd_in_ctb - ps_slice_hdr->i2_ctb_x);
                if(ps_proc->i4_ctb_slice_x >= (temp_stride + ps_tile->u2_wd + ps_tile->u1_pos_x))
                {
                    ps_proc->i4_ctb_slice_y++;
                    ps_proc->i4_ctb_slice_x = ps_proc->i4_ctb_slice_x
                                    - ps_tile->u2_wd;
                }
            }
        }
        /*Case 2*/
        else if(ps_proc->i4_ctb_slice_x >= (ps_tile->u2_wd))
        {
            /*End of tile row*/
            ps_proc->i4_ctb_slice_y++;
            ps_proc->i4_ctb_slice_x = 0;
        }
    }
    else
    {
        if(ps_proc->i4_ctb_slice_x >= ps_tile->u2_wd)
        {
            ps_proc->i4_ctb_slice_y++;
            ps_proc->i4_ctb_slice_x = ps_proc->i4_ctb_slice_x
                            - ps_tile->u2_wd;
        }
    }
}

void ihevcd_ctb_avail_update(process_ctxt_t *ps_proc)
{
    slice_header_t *ps_slice_hdr = ps_proc->ps_slice_hdr;
    sps_t *ps_sps = ps_proc->ps_sps;
    tile_t *ps_tile_prev;
    tile_t *ps_tile = ps_proc->ps_tile;
    WORD32 cur_pu_idx;
    WORD32 tile_start_ctb_idx, slice_start_ctb_idx;
    WORD16 i2_wd_in_ctb;
    WORD32 continuous_tiles = 0;
    WORD32 cur_ctb_idx;
    WORD32 check_tile_wd;

    if((0 != ps_tile->u1_pos_x) && (0 != ps_tile->u1_pos_y))
    {
        ps_tile_prev = ps_tile - 1;
    }
    else
    {
        ps_tile_prev = ps_tile;
    }


    check_tile_wd = ps_slice_hdr->i2_ctb_x + ps_tile_prev->u2_wd;
    if(!(((check_tile_wd >= ps_sps->i2_pic_wd_in_ctb) && (check_tile_wd % ps_sps->i2_pic_wd_in_ctb == ps_tile->u1_pos_x))
                                    || ((ps_slice_hdr->i2_ctb_x == ps_tile->u1_pos_x))))
    {
        continuous_tiles = 1;
    }

    slice_start_ctb_idx = ps_slice_hdr->i2_ctb_x + (ps_slice_hdr->i2_ctb_y * ps_sps->i2_pic_wd_in_ctb);
    tile_start_ctb_idx = ps_tile->u1_pos_x + (ps_tile->u1_pos_y * ps_sps->i2_pic_wd_in_ctb);

    if((slice_start_ctb_idx < tile_start_ctb_idx) && (continuous_tiles))
    {
        //Slices span across multiple tiles.
        i2_wd_in_ctb = ps_sps->i2_pic_wd_in_ctb;
    }
    else
    {
        i2_wd_in_ctb = ps_tile->u2_wd;
    }
    cur_ctb_idx = ps_proc->i4_ctb_x
                    + ps_proc->i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);

    /* Ctb level availability */
    /* Bottom left will not be available at a CTB level, no need to pass this */
    ps_proc->u1_top_ctb_avail = 1;
    ps_proc->u1_left_ctb_avail = 1;
    ps_proc->u1_top_lt_ctb_avail = 1;
    ps_proc->u1_top_rt_ctb_avail = 1;
    /* slice and tile boundaries */

    if((0 == ps_proc->i4_ctb_y) || (0 == ps_proc->i4_ctb_tile_y))
    {
        ps_proc->u1_top_ctb_avail = 0;
        ps_proc->u1_top_lt_ctb_avail = 0;
        ps_proc->u1_top_rt_ctb_avail = 0;
    }

    if((0 == ps_proc->i4_ctb_x) || (0 == ps_proc->i4_ctb_tile_x))
    {
        ps_proc->u1_left_ctb_avail = 0;
        ps_proc->u1_top_lt_ctb_avail = 0;
        if((0 == ps_proc->i4_ctb_slice_y) || (0 == ps_proc->i4_ctb_tile_y))
        {
            ps_proc->u1_top_ctb_avail = 0;
            if((i2_wd_in_ctb - 1) != ps_proc->i4_ctb_slice_x)
            {
                ps_proc->u1_top_rt_ctb_avail = 0;
            }
        }
    }
    /*For slices not beginning at start of a ctb row*/
    else if(ps_proc->i4_ctb_x > 0)
    {
        if((0 == ps_proc->i4_ctb_slice_y) || (0 == ps_proc->i4_ctb_tile_y))
        {
            ps_proc->u1_top_ctb_avail = 0;
            ps_proc->u1_top_lt_ctb_avail = 0;
            if(0 == ps_proc->i4_ctb_slice_x)
            {
                ps_proc->u1_left_ctb_avail = 0;
            }
            if((i2_wd_in_ctb - 1) != ps_proc->i4_ctb_slice_x)
            {
                ps_proc->u1_top_rt_ctb_avail = 0;
            }
        }
        else if((1 == ps_proc->i4_ctb_slice_y) && (0 == ps_proc->i4_ctb_slice_x))
        {
            ps_proc->u1_top_lt_ctb_avail = 0;
        }
    }

    if((ps_proc->i4_ctb_x == (ps_sps->i2_pic_wd_in_ctb - 1)) || ((ps_tile->u2_wd - 1) == ps_proc->i4_ctb_tile_x))
    {
        ps_proc->u1_top_rt_ctb_avail = 0;
    }


    {
        WORD32 next_ctb_idx;
        next_ctb_idx = cur_ctb_idx + 1;

        if(ps_tile->u2_wd == (ps_proc->i4_ctb_tile_x + 1))
        {
            if((ps_proc->i4_ctb_tile_y + 1) == ps_tile->u2_ht)
            {
                //Last tile
                if(((ps_proc->i4_ctb_tile_y + 1 + ps_tile->u1_pos_y) == ps_sps->i2_pic_ht_in_ctb) && ((ps_proc->i4_ctb_tile_x + 1 + ps_tile->u1_pos_x) == ps_sps->i2_pic_wd_in_ctb))
                {
                    next_ctb_idx = cur_ctb_idx + 1;
                }
                else //Not last tile, but new tile
                {
                    tile_t *ps_tile_next = ps_proc->ps_tile + 1;
                    next_ctb_idx = ps_tile_next->u1_pos_x + (ps_tile_next->u1_pos_y * ps_sps->i2_pic_wd_in_ctb);
                }
            }
            else //End of each tile row
            {
                next_ctb_idx = ((ps_tile->u1_pos_y + ps_proc->i4_ctb_tile_y + 1) * ps_sps->i2_pic_wd_in_ctb) + ps_tile->u1_pos_x;
            }
        }
        ps_proc->i4_next_pu_ctb_cnt = next_ctb_idx;
        ps_proc->i4_ctb_pu_cnt =
                        ps_proc->pu4_pic_pu_idx[next_ctb_idx]
                        - ps_proc->pu4_pic_pu_idx[cur_ctb_idx];
        cur_pu_idx = ps_proc->pu4_pic_pu_idx[cur_ctb_idx];
        ps_proc->i4_ctb_start_pu_idx = cur_pu_idx;
        ps_proc->ps_pu = &ps_proc->ps_pic_pu[cur_pu_idx];
    }
}

void ihevcd_update_ctb_tu_cnt(process_ctxt_t *ps_proc)
{
    sps_t *ps_sps = ps_proc->ps_sps;
    codec_t *ps_codec = ps_proc->ps_codec;
    WORD32 cur_ctb_idx;

    cur_ctb_idx = ps_proc->i4_ctb_x
                    + ps_proc->i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);

    {
        tile_t *ps_tile;
        WORD32 next_ctb_tu_idx;
        ps_tile = ps_proc->ps_tile;


        if(1 == ps_codec->i4_num_cores)
        {
            next_ctb_tu_idx = cur_ctb_idx % RESET_TU_BUF_NCTB + 1;
            if(ps_tile->u2_wd == (ps_proc->i4_ctb_tile_x + 1))
            {
                if((ps_proc->i4_ctb_tile_y + 1) == ps_tile->u2_ht)
                {
                    //Last tile
                    if(((ps_proc->i4_ctb_tile_y + 1 + ps_tile->u1_pos_y) == ps_sps->i2_pic_ht_in_ctb) && ((ps_proc->i4_ctb_tile_x + 1 + ps_tile->u1_pos_x) == ps_sps->i2_pic_wd_in_ctb))
                    {
                        next_ctb_tu_idx = (cur_ctb_idx % RESET_TU_BUF_NCTB) + 1;
                    }
                    else //Not last tile, but new tile
                    {
                        tile_t *ps_tile_next = ps_proc->ps_tile + 1;
                        next_ctb_tu_idx = ps_tile_next->u1_pos_x + (ps_tile_next->u1_pos_y * ps_sps->i2_pic_wd_in_ctb);
                    }
                }
                else //End of each tile row
                {
                    next_ctb_tu_idx = ((ps_tile->u1_pos_y + ps_proc->i4_ctb_tile_y + 1) * ps_sps->i2_pic_wd_in_ctb) + ps_tile->u1_pos_x;
                }
            }
            ps_proc->i4_next_tu_ctb_cnt = next_ctb_tu_idx;
            ps_proc->i4_ctb_tu_cnt = ps_proc->pu4_pic_tu_idx[next_ctb_tu_idx] - ps_proc->pu4_pic_tu_idx[cur_ctb_idx % RESET_TU_BUF_NCTB];
        }
        else
        {
            next_ctb_tu_idx = cur_ctb_idx + 1;
            if(ps_tile->u2_wd == (ps_proc->i4_ctb_tile_x + 1))
            {
                if((ps_proc->i4_ctb_tile_y + 1) == ps_tile->u2_ht)
                {
                    //Last tile
                    if(((ps_proc->i4_ctb_tile_y + 1 + ps_tile->u1_pos_y) == ps_sps->i2_pic_ht_in_ctb) && ((ps_proc->i4_ctb_tile_x + 1 + ps_tile->u1_pos_x) == ps_sps->i2_pic_wd_in_ctb))
                    {
                        next_ctb_tu_idx = (cur_ctb_idx % RESET_TU_BUF_NCTB) + 1;
                    }
                    else //Not last tile, but new tile
                    {
                        tile_t *ps_tile_next = ps_proc->ps_tile + 1;
                        next_ctb_tu_idx = ps_tile_next->u1_pos_x + (ps_tile_next->u1_pos_y * ps_sps->i2_pic_wd_in_ctb);
                    }
                }
                else //End of each tile row
                {
                    next_ctb_tu_idx = ((ps_tile->u1_pos_y + ps_proc->i4_ctb_tile_y + 1) * ps_sps->i2_pic_wd_in_ctb) + ps_tile->u1_pos_x;
                }
            }
            ps_proc->i4_next_tu_ctb_cnt = next_ctb_tu_idx;
            ps_proc->i4_ctb_tu_cnt = ps_proc->pu4_pic_tu_idx[next_ctb_tu_idx] -
                            ps_proc->pu4_pic_tu_idx[cur_ctb_idx];
        }
    }
}

IHEVCD_ERROR_T ihevcd_process(process_ctxt_t *ps_proc)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    codec_t *ps_codec;
    sps_t *ps_sps = ps_proc->ps_sps;

    WORD32 nctb;
    WORD32 i;
    WORD32 idx;
    WORD32 nop_cnt;
    WORD32 num_minpu_in_ctb;
    WORD32 cur_slice_idx, cur_ctb_tile_x, cur_ctb_slice_x, cur_ctb_tile_y, cur_ctb_slice_y;
    WORD32 nxt_ctb_slice_y, nxt_ctb_slice_x;
    tu_t *ps_tu_cur, *ps_tu_nxt;
    UWORD8 *pu1_pu_map_cur, *pu1_pu_map_nxt;
    WORD32 num_ctb, num_ctb_tmp;
    proc_type_t proc_type;


    WORD32 ctb_size = 1 << ps_sps->i1_log2_ctb_size;

    PROFILE_DISABLE_PROCESS_CTB();

    ps_codec = ps_proc->ps_codec;
    num_minpu_in_ctb = (ctb_size / MIN_PU_SIZE) * (ctb_size / MIN_PU_SIZE);

    nctb = MIN(ps_codec->i4_proc_nctb, ps_proc->i4_ctb_cnt);
    nctb = MIN(nctb, (ps_proc->ps_tile->u2_wd - ps_proc->i4_ctb_tile_x));

    if(ps_proc->i4_cur_slice_idx > (MAX_SLICE_HDR_CNT - 2 * ps_sps->i2_pic_wd_in_ctb))
    {
        num_ctb = 1;
    }
    else
    {
        num_ctb = ps_proc->i4_nctb;
    }
    nxt_ctb_slice_y = ps_proc->i4_ctb_slice_y;
    nxt_ctb_slice_x = ps_proc->i4_ctb_slice_x;
    pu1_pu_map_nxt = ps_proc->pu1_pu_map;
    ps_tu_nxt = ps_proc->ps_tu;

    while(ps_proc->i4_ctb_cnt)
    {
        ps_proc->i4_ctb_slice_y = nxt_ctb_slice_y;
        ps_proc->i4_ctb_slice_x = nxt_ctb_slice_x;
        ps_proc->pu1_pu_map = pu1_pu_map_nxt;
        ps_proc->ps_tu = ps_tu_nxt;

        cur_ctb_tile_x = ps_proc->i4_ctb_tile_x;
        cur_ctb_tile_y = ps_proc->i4_ctb_tile_y;
        cur_ctb_slice_x = ps_proc->i4_ctb_slice_x;
        cur_ctb_slice_y = ps_proc->i4_ctb_slice_y;
        cur_slice_idx = ps_proc->i4_cur_slice_idx;
        ps_tu_cur = ps_proc->ps_tu;
        pu1_pu_map_cur = ps_proc->pu1_pu_map;
        proc_type = PROC_INTER_PRED;

        if(ps_proc->i4_ctb_cnt < num_ctb)
        {
            num_ctb = ps_proc->i4_ctb_cnt;
        }
        num_ctb_tmp = num_ctb;

        while(num_ctb_tmp)
        {
            slice_header_t *ps_slice_hdr;
            tile_t *ps_tile = ps_proc->ps_tile;

            /* Waiting for Parsing to be done*/
            {


                nop_cnt = PROC_NOP_CNT;
                if(ps_proc->i4_check_parse_status || ps_proc->i4_check_proc_status)
                {
                    while(1)
                    {
                        volatile UWORD8 *pu1_buf;
                        volatile WORD32 status;
                        status = 1;
                        /* Check if all dependencies for the next nCTBs are met */
                        /* Check if the next nCTBs are parsed */
                        if(ps_proc->i4_check_parse_status)
                        {
                            idx = (ps_proc->i4_ctb_x + nctb - 1);
                            idx += (ps_proc->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb);
                            pu1_buf = (ps_codec->pu1_parse_map + idx);
                            status = *pu1_buf;
                        }

                        if(status)
                            break;

                        /* if dependencies are not met, then wait for few cycles.
                         * Even after few iterations, if the dependencies are not met then yield
                         */
                        if(nop_cnt > 0)
                        {
                            NOP(128);
                            nop_cnt -= 128;
                        }
                        else
                        {
                            nop_cnt = PROC_NOP_CNT;
                            ithread_yield();
                        }
                    }
                }
            }

            /* Check proc map to ensure dependencies for recon are met */
            ihevcd_proc_map_check(ps_proc, proc_type, nctb);

            ihevcd_slice_hdr_update(ps_proc);
            ps_slice_hdr = ps_proc->ps_slice_hdr;

            //ihevcd_mv_prediction();
            //ihevcd_lvl_unpack();
            //ihevcd_inter_iq_it_recon();
            //Following does prediction, iq, it and recon on a TU by TU basis for intra TUs
            //ihevcd_intra_process();
            //ihevcd_ctb_boundary_strength_islice(ps_proc, ctb_size);
            //ihevcd_deblk_ctb(ps_proc);

            /* iq,it recon of Intra TU */
            {
                UWORD32 *pu4_ctb_top_pu_idx, *pu4_ctb_left_pu_idx, *pu4_ctb_top_left_pu_idx;
                WORD32 cur_ctb_idx;

                ihevcd_ctb_avail_update(ps_proc);

#if DEBUG_DUMP_FRAME_BUFFERS_INFO
                au1_pic_avail_ctb_flags[ps_proc->i4_ctb_x + ps_proc->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb] =
                                ((ps_proc->u1_top_ctb_avail << 3) | (ps_proc->u1_left_ctb_avail << 2) | (ps_proc->u1_top_lt_ctb_avail << 1) | (ps_proc->u1_top_rt_ctb_avail));
                au4_pic_ctb_slice_xy[ps_proc->i4_ctb_x + ps_proc->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb] =
                                (((UWORD16)ps_proc->i4_ctb_slice_x << 16) | ((UWORD16)ps_proc->i4_ctb_slice_y << 16));
#endif

                /*************************************************/
                /****************   MV pred **********************/
                /*************************************************/
                if(PSLICE == ps_slice_hdr->i1_slice_type
                                || BSLICE == ps_slice_hdr->i1_slice_type)
                {
                    mv_ctxt_t s_mv_ctxt;

                    pu4_ctb_top_pu_idx = ps_proc->pu4_pic_pu_idx_top
                                    + (ps_proc->i4_ctb_x * ctb_size / MIN_PU_SIZE);
                    pu4_ctb_left_pu_idx = ps_proc->pu4_pic_pu_idx_left;
                    pu4_ctb_top_left_pu_idx = &ps_proc->u4_ctb_top_left_pu_idx;

                    /* Initializing s_mv_ctxt */
                    if(ps_codec->i4_num_cores > MV_PRED_NUM_CORES_THRESHOLD)
                    {
                        s_mv_ctxt.ps_pps = ps_proc->ps_pps;
                        s_mv_ctxt.ps_sps = ps_proc->ps_sps;
                        s_mv_ctxt.ps_slice_hdr = ps_proc->ps_slice_hdr;
                        s_mv_ctxt.i4_ctb_x = ps_proc->i4_ctb_x;
                        s_mv_ctxt.i4_ctb_y = ps_proc->i4_ctb_y;
                        s_mv_ctxt.ps_pu = ps_proc->ps_pu;
                        s_mv_ctxt.ps_pic_pu = ps_proc->ps_pic_pu;
                        s_mv_ctxt.ps_tile = ps_tile;
                        s_mv_ctxt.pu4_pic_pu_idx_map = ps_proc->pu4_pic_pu_idx_map;
                        s_mv_ctxt.pu4_pic_pu_idx = ps_proc->pu4_pic_pu_idx;
                        s_mv_ctxt.pu1_pic_pu_map = ps_proc->pu1_pic_pu_map;
                        s_mv_ctxt.i4_ctb_pu_cnt = ps_proc->i4_ctb_pu_cnt;
                        s_mv_ctxt.i4_ctb_start_pu_idx = ps_proc->i4_ctb_start_pu_idx;
                        s_mv_ctxt.u1_top_ctb_avail = ps_proc->u1_top_ctb_avail;
                        s_mv_ctxt.u1_top_rt_ctb_avail = ps_proc->u1_top_rt_ctb_avail;
                        s_mv_ctxt.u1_top_lt_ctb_avail = ps_proc->u1_top_lt_ctb_avail;
                        s_mv_ctxt.u1_left_ctb_avail = ps_proc->u1_left_ctb_avail;

                        ihevcd_get_mv_ctb(&s_mv_ctxt, pu4_ctb_top_pu_idx,
                                          pu4_ctb_left_pu_idx, pu4_ctb_top_left_pu_idx);
                    }

                    ihevcd_inter_pred_ctb(ps_proc);
                }
                else if(ps_codec->i4_num_cores > MV_PRED_NUM_CORES_THRESHOLD)
                {
                    WORD32 next_ctb_idx, num_pu_per_ctb, ctb_start_pu_idx, pu_cnt;
                    pu_t *ps_pu;
                    WORD32 num_minpu_in_ctb = (ctb_size / MIN_PU_SIZE) * (ctb_size / MIN_PU_SIZE);
                    UWORD8 *pu1_pic_pu_map_ctb = ps_proc->pu1_pic_pu_map +
                                    (ps_proc->i4_ctb_x + ps_proc->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb) * num_minpu_in_ctb;
                    WORD32 row, col;
                    UWORD32 *pu4_nbr_pu_idx = ps_proc->pu4_pic_pu_idx_map;
                    WORD32 nbr_pu_idx_strd = MAX_CTB_SIZE / MIN_PU_SIZE + 2;
                    WORD32 ctb_size_in_min_pu = (ctb_size / MIN_PU_SIZE);

                    /* Neighbor PU idx update inside CTB */
                    /* 1byte per 4x4. Indicates the PU idx that 4x4 block belongs to */

                    cur_ctb_idx = ps_proc->i4_ctb_x
                                    + ps_proc->i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);
                    next_ctb_idx = ps_proc->i4_next_pu_ctb_cnt;
                    num_pu_per_ctb = ps_proc->pu4_pic_pu_idx[next_ctb_idx]
                                    - ps_proc->pu4_pic_pu_idx[cur_ctb_idx];
                    ctb_start_pu_idx = ps_proc->pu4_pic_pu_idx[cur_ctb_idx];
                    ps_pu = &ps_proc->ps_pic_pu[ctb_start_pu_idx];

                    for(pu_cnt = 0; pu_cnt < num_pu_per_ctb; pu_cnt++, ps_pu++)
                    {
                        UWORD32 cur_pu_idx;
                        WORD32 pu_ht = (ps_pu->b4_ht + 1) << 2;
                        WORD32 pu_wd = (ps_pu->b4_wd + 1) << 2;

                        cur_pu_idx = ctb_start_pu_idx + pu_cnt;

                        for(row = 0; row < pu_ht / MIN_PU_SIZE; row++)
                            for(col = 0; col < pu_wd / MIN_PU_SIZE; col++)
                                pu4_nbr_pu_idx[(1 + ps_pu->b4_pos_x + col)
                                                + (1 + ps_pu->b4_pos_y + row)
                                                * nbr_pu_idx_strd] =
                                                cur_pu_idx;
                    }

                    /* Updating Top and Left pointers */
                    {
                        WORD32 rows_remaining = ps_sps->i2_pic_height_in_luma_samples
                                        - (ps_proc->i4_ctb_y << ps_sps->i1_log2_ctb_size);
                        WORD32 ctb_size_left = MIN(ctb_size, rows_remaining);

                        /* Top Left */
                        /* saving top left before updating top ptr, as updating top ptr will overwrite the top left for the next ctb */
                        ps_proc->u4_ctb_top_left_pu_idx = ps_proc->pu4_pic_pu_idx_top[((ps_proc->i4_ctb_x + 1) * ctb_size / MIN_PU_SIZE) - 1];
                        for(i = 0; i < ctb_size / MIN_PU_SIZE; i++)
                        {
                            /* Left */
                            /* Last column of au4_nbr_pu_idx */
                            ps_proc->pu4_pic_pu_idx_left[i] =
                                            pu4_nbr_pu_idx[(ctb_size / MIN_PU_SIZE) + (i + 1) * nbr_pu_idx_strd];
                            /* Top */
                            /* Last row of au4_nbr_pu_idx */
                            ps_proc->pu4_pic_pu_idx_top[(ps_proc->i4_ctb_x * ctb_size / MIN_PU_SIZE) + i] =
                                            pu4_nbr_pu_idx[(ctb_size_left / MIN_PU_SIZE) * nbr_pu_idx_strd + i + 1];

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
                    }
                }
            }

            if(ps_proc->ps_pps->i1_tiles_enabled_flag)
            {
                /*Update the tile index buffer with tile information for the current ctb*/
                UWORD16 *pu1_tile_idx = ps_proc->pu1_tile_idx;
                pu1_tile_idx[(ps_proc->i4_ctb_x + (ps_proc->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb))]
                                = ps_proc->i4_cur_tile_idx;
            }

            /*************************************************/
            /*********** BS, QP and Deblocking  **************/
            /*************************************************/
            /* Boundary strength call has to be after IQ IT recon since QP population needs ps_proc->i4_qp_const_inc_ctb flag */

            {
                slice_header_t *ps_slice_hdr;
                ps_slice_hdr = ps_proc->ps_slice_hdr;


                /* Check if deblock is disabled for the current slice or if it is disabled for the current picture
                 * because of disable deblock api
                 */
                if(0 == ps_codec->i4_disable_deblk_pic)
                {
                    if(ps_codec->i4_num_cores > MV_PRED_NUM_CORES_THRESHOLD)
                    {
                        /* Boundary strength calculation is done irrespective of whether deblocking is disabled
                         * in the slice or not, to handle deblocking slice boundaries */
                        if((0 == ps_codec->i4_slice_error))
                        {
                            ihevcd_update_ctb_tu_cnt(ps_proc);
                            ps_proc->s_bs_ctxt.ps_pps = ps_proc->ps_pps;
                            ps_proc->s_bs_ctxt.ps_sps = ps_proc->ps_sps;
                            ps_proc->s_bs_ctxt.ps_codec = ps_proc->ps_codec;
                            ps_proc->s_bs_ctxt.i4_ctb_tu_cnt = ps_proc->i4_ctb_tu_cnt;
                            ps_proc->s_bs_ctxt.i4_ctb_x = ps_proc->i4_ctb_x;
                            ps_proc->s_bs_ctxt.i4_ctb_y = ps_proc->i4_ctb_y;
                            ps_proc->s_bs_ctxt.i4_ctb_tile_x = ps_proc->i4_ctb_tile_x;
                            ps_proc->s_bs_ctxt.i4_ctb_tile_y = ps_proc->i4_ctb_tile_y;
                            ps_proc->s_bs_ctxt.i4_ctb_slice_x = ps_proc->i4_ctb_slice_x;
                            ps_proc->s_bs_ctxt.i4_ctb_slice_y = ps_proc->i4_ctb_slice_y;
                            ps_proc->s_bs_ctxt.ps_tu = ps_proc->ps_tu;
                            ps_proc->s_bs_ctxt.ps_pu = ps_proc->ps_pu;
                            ps_proc->s_bs_ctxt.pu4_pic_pu_idx_map = ps_proc->pu4_pic_pu_idx_map;
                            ps_proc->s_bs_ctxt.i4_next_pu_ctb_cnt = ps_proc->i4_next_pu_ctb_cnt;
                            ps_proc->s_bs_ctxt.i4_next_tu_ctb_cnt = ps_proc->i4_next_tu_ctb_cnt;
                            ps_proc->s_bs_ctxt.pu1_slice_idx = ps_proc->pu1_slice_idx;
                            ps_proc->s_bs_ctxt.ps_slice_hdr = ps_proc->ps_slice_hdr;
                            ps_proc->s_bs_ctxt.ps_tile = ps_proc->ps_tile;

                            if(ISLICE == ps_slice_hdr->i1_slice_type)
                            {
                                ihevcd_ctb_boundary_strength_islice(&ps_proc->s_bs_ctxt);
                            }
                            else
                            {
                                ihevcd_ctb_boundary_strength_pbslice(&ps_proc->s_bs_ctxt);
                            }
                        }

                        /* Boundary strength is set to zero if deblocking is disabled for the current slice */
                        if((0 != ps_slice_hdr->i1_slice_disable_deblocking_filter_flag))
                        {
                            WORD32 bs_strd = (ps_sps->i2_pic_wd_in_ctb + 1) * (ctb_size * ctb_size / 8 / 16);

                            UWORD32 *pu4_vert_bs = (UWORD32 *)((UWORD8 *)ps_proc->s_bs_ctxt.pu4_pic_vert_bs +
                                            ps_proc->i4_ctb_x * (ctb_size * ctb_size / 8 / 16) +
                                            ps_proc->i4_ctb_y * bs_strd);
                            UWORD32 *pu4_horz_bs = (UWORD32 *)((UWORD8 *)ps_proc->s_bs_ctxt.pu4_pic_horz_bs +
                                            ps_proc->i4_ctb_x * (ctb_size * ctb_size / 8 / 16) +
                                            ps_proc->i4_ctb_y * bs_strd);

                            memset(pu4_vert_bs, 0, (ctb_size / 8) * (ctb_size / 4) / 8 * 2);
                            memset(pu4_horz_bs, 0, (ctb_size / 8) * (ctb_size / 4) / 8 * 2);
                        }
                    }
                }
            }

            /* Per CTB update the following */
            {
                WORD32 cur_ctb_idx = ps_proc->i4_ctb_x
                                + ps_proc->i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);
                cur_ctb_idx++;

                ps_proc->pu1_pu_map += nctb * num_minpu_in_ctb;
                ps_proc->ps_tu += ps_proc->i4_ctb_tu_cnt;
                if((1 == ps_codec->i4_num_cores) &&
                                (0 == cur_ctb_idx % RESET_TU_BUF_NCTB))
                {
                    ps_proc->ps_tu = ps_proc->ps_pic_tu;
                }
                ps_proc->ps_pu += ps_proc->i4_ctb_pu_cnt;
            }

            /* Update proc map for recon*/
            ihevcd_proc_map_update(ps_proc, proc_type, nctb);

            num_ctb_tmp -= nctb;
            ihevcd_ctb_pos_update(ps_proc, nctb);

        }

        if(cur_slice_idx != ps_proc->i4_cur_slice_idx)
        {
            ps_proc->ps_slice_hdr = ps_codec->ps_slice_hdr_base + ((cur_slice_idx)&(MAX_SLICE_HDR_CNT - 1));
            ps_proc->i4_cur_slice_idx = cur_slice_idx;
        }
        /* Restore the saved variables  */
        num_ctb_tmp = num_ctb;
        ps_proc->i4_ctb_x -= num_ctb;
        ps_proc->i4_ctb_tile_x = cur_ctb_tile_x;
        ps_proc->i4_ctb_slice_x = cur_ctb_slice_x;
        ps_proc->i4_ctb_tile_y = cur_ctb_tile_y;
        ps_proc->i4_ctb_slice_y = cur_ctb_slice_y;
        ps_proc->pu1_pu_map = pu1_pu_map_cur;
        ps_proc->ps_tu = ps_tu_cur;
        proc_type = PROC_RECON;

        while(num_ctb_tmp)
        {

            /* Check proc map to ensure dependencies for recon are met */
            ihevcd_proc_map_check(ps_proc, proc_type, nctb);

            ihevcd_slice_hdr_update(ps_proc);

            {

                ihevcd_ctb_avail_update(ps_proc);

                /*************************************************/
                /**************** IQ IT RECON  *******************/
                /*************************************************/

                ihevcd_update_ctb_tu_cnt(ps_proc);

                /* When scaling matrix is not to be used(scaling_list_enable_flag is zero in SPS),
                 * default value of 16 has to be used. Since the value is same for all sizes,
                 * same table is used for all cases.
                 */
                if(0 == ps_sps->i1_scaling_list_enable_flag)
                {
                    ps_proc->api2_dequant_intra_matrix[0] =
                                    (WORD16 *)gi2_flat_scale_mat_32x32;
                    ps_proc->api2_dequant_intra_matrix[1] =
                                    (WORD16 *)gi2_flat_scale_mat_32x32;
                    ps_proc->api2_dequant_intra_matrix[2] =
                                    (WORD16 *)gi2_flat_scale_mat_32x32;
                    ps_proc->api2_dequant_intra_matrix[3] =
                                    (WORD16 *)gi2_flat_scale_mat_32x32;

                    ps_proc->api2_dequant_inter_matrix[0] =
                                    (WORD16 *)gi2_flat_scale_mat_32x32;
                    ps_proc->api2_dequant_inter_matrix[1] =
                                    (WORD16 *)gi2_flat_scale_mat_32x32;
                    ps_proc->api2_dequant_inter_matrix[2] =
                                    (WORD16 *)gi2_flat_scale_mat_32x32;
                    ps_proc->api2_dequant_inter_matrix[3] =
                                    (WORD16 *)gi2_flat_scale_mat_32x32;
                }
                else
                {
                    if(0 == ps_sps->i1_sps_scaling_list_data_present_flag)
                    {
                        ps_proc->api2_dequant_intra_matrix[0] =
                                        (WORD16 *)gi2_flat_scale_mat_32x32;
                        ps_proc->api2_dequant_intra_matrix[1] =
                                        (WORD16 *)gi2_intra_default_scale_mat_8x8;
                        ps_proc->api2_dequant_intra_matrix[2] =
                                        (WORD16 *)gi2_intra_default_scale_mat_16x16;
                        ps_proc->api2_dequant_intra_matrix[3] =
                                        (WORD16 *)gi2_intra_default_scale_mat_32x32;

                        ps_proc->api2_dequant_inter_matrix[0] =
                                        (WORD16 *)gi2_flat_scale_mat_32x32;
                        ps_proc->api2_dequant_inter_matrix[1] =
                                        (WORD16 *)gi2_inter_default_scale_mat_8x8;
                        ps_proc->api2_dequant_inter_matrix[2] =
                                        (WORD16 *)gi2_inter_default_scale_mat_16x16;
                        ps_proc->api2_dequant_inter_matrix[3] =
                                        (WORD16 *)gi2_inter_default_scale_mat_32x32;
                    }
                    /*TODO: Add support for custom scaling matrices */
                }


                /* CTB Level pointers */
                ps_proc->pu1_cur_ctb_luma = ps_proc->pu1_cur_pic_luma
                                + (ps_proc->i4_ctb_x * ctb_size
                                + ps_proc->i4_ctb_y * ctb_size
                                * ps_codec->i4_strd);
                ps_proc->pu1_cur_ctb_chroma = ps_proc->pu1_cur_pic_chroma
                                + ps_proc->i4_ctb_x * ctb_size
                                + (ps_proc->i4_ctb_y * ctb_size * ps_codec->i4_strd / 2);

                ihevcd_iquant_itrans_recon_ctb(ps_proc);
            }

            /* Per CTB update the following */
            {
                WORD32 cur_ctb_idx = ps_proc->i4_ctb_x
                                + ps_proc->i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);
                cur_ctb_idx++;

                ps_proc->pu1_pu_map += nctb * num_minpu_in_ctb;
                ps_proc->ps_tu += ps_proc->i4_ctb_tu_cnt;
                if((1 == ps_codec->i4_num_cores) &&
                                (0 == cur_ctb_idx % RESET_TU_BUF_NCTB))
                {
                    ps_proc->ps_tu = ps_proc->ps_pic_tu;
                }
                ps_proc->ps_pu += ps_proc->i4_ctb_pu_cnt;
            }


            /* Update proc map for recon*/
            ihevcd_proc_map_update(ps_proc, proc_type, nctb);

            num_ctb_tmp -= nctb;
            ihevcd_ctb_pos_update(ps_proc, nctb);
        }

        if(cur_slice_idx != ps_proc->i4_cur_slice_idx)
        {
            ps_proc->ps_slice_hdr = ps_codec->ps_slice_hdr_base + ((cur_slice_idx)&(MAX_SLICE_HDR_CNT - 1));
            ps_proc->i4_cur_slice_idx = cur_slice_idx;
        }
        /* Restore the saved variables  */
        num_ctb_tmp = num_ctb;
        ps_proc->i4_ctb_x -= num_ctb;
        ps_proc->i4_ctb_tile_x = cur_ctb_tile_x;
        ps_proc->i4_ctb_slice_x = cur_ctb_slice_x;
        ps_proc->i4_ctb_tile_y = cur_ctb_tile_y;
        ps_proc->i4_ctb_slice_y = cur_ctb_slice_y;
        pu1_pu_map_nxt = ps_proc->pu1_pu_map;
        ps_tu_nxt = ps_proc->ps_tu;
        ps_proc->pu1_pu_map = pu1_pu_map_cur;
        ps_proc->ps_tu = ps_tu_cur;
        proc_type = PROC_DEBLK;

        while(num_ctb_tmp)
        {


            /* Check proc map to ensure dependencies for deblk are met */
            ihevcd_proc_map_check(ps_proc, proc_type, nctb);

            ihevcd_slice_hdr_update(ps_proc);


            if(((0 == FRAME_ILF_PAD || ps_codec->i4_num_cores != 1)) &&
               (0 == ps_codec->i4_disable_deblk_pic))
            {
                WORD32 i4_is_last_ctb_x = 0;
                WORD32 i4_is_last_ctb_y = 0;


                /* Deblocking is done irrespective of whether it is disabled in the slice or not,
                 * to handle deblocking the slice boundaries */
                {
                    ps_proc->s_deblk_ctxt.ps_pps = ps_proc->ps_pps;
                    ps_proc->s_deblk_ctxt.ps_sps = ps_proc->ps_sps;
                    ps_proc->s_deblk_ctxt.ps_codec = ps_proc->ps_codec;
                    ps_proc->s_deblk_ctxt.ps_slice_hdr = ps_proc->ps_slice_hdr;
                    ps_proc->s_deblk_ctxt.i4_ctb_x = ps_proc->i4_ctb_x;
                    ps_proc->s_deblk_ctxt.i4_ctb_y = ps_proc->i4_ctb_y;
                    ps_proc->s_deblk_ctxt.pu1_slice_idx = ps_proc->pu1_slice_idx;
                    ps_proc->s_deblk_ctxt.is_chroma_yuv420sp_vu = (ps_codec->e_ref_chroma_fmt == IV_YUV_420SP_VU);

                    /* Populating Current CTB's no_loop_filter flags */
                    {
                        WORD32 row;
                        WORD32 log2_ctb_size = ps_sps->i1_log2_ctb_size;

                        /* Loop filter strd in units of num bits */
                        WORD32 loop_filter_strd = ((ps_sps->i2_pic_width_in_luma_samples + 63) >> 6) << 3;
                        /* Bit position is the current 8x8 bit offset wrt pic_no_loop_filter
                         * bit_pos has to be a WOR32 so that when it is negative, the downshift still retains it to be a negative value */
                        WORD32 bit_pos = ((ps_proc->i4_ctb_y << (log2_ctb_size - 3)) - 1) * loop_filter_strd + (ps_proc->i4_ctb_x << (log2_ctb_size - 3)) - 1;

                        for(row = 0; row < (ctb_size >> 3) + 1; row++)
                        {
                            /* Go to the corresponding byte - read 32 bits and downshift */
                            ps_proc->s_deblk_ctxt.au2_ctb_no_loop_filter_flag[row] = (*(UWORD32 *)(ps_proc->pu1_pic_no_loop_filter_flag + (bit_pos >> 3))) >> (bit_pos & 7);
                            bit_pos += loop_filter_strd;
                        }
                    }

                    ihevcd_deblk_ctb(&ps_proc->s_deblk_ctxt, i4_is_last_ctb_x, i4_is_last_ctb_y);

                    /* If the last CTB in the row was a complete CTB then deblocking has to be called from remaining pixels, since deblocking
                     * is applied on a shifted CTB structure
                     */
                    if(ps_proc->i4_ctb_x == ps_sps->i2_pic_wd_in_ctb - 1)
                    {
                        WORD32 i4_is_last_ctb_x = 1;
                        WORD32 i4_is_last_ctb_y = 0;

                        WORD32 last_x_pos;
                        last_x_pos = (ps_sps->i2_pic_wd_in_ctb << ps_sps->i1_log2_ctb_size);
                        if(last_x_pos  ==  ps_sps->i2_pic_width_in_luma_samples)
                        {
                            ihevcd_deblk_ctb(&ps_proc->s_deblk_ctxt, i4_is_last_ctb_x, i4_is_last_ctb_y);
                        }
                    }


                    /* If the last CTB in the column was a complete CTB then deblocking has to be called from remaining pixels, since deblocking
                     * is applied on a shifted CTB structure
                     */
                    if(ps_proc->i4_ctb_y == ps_sps->i2_pic_ht_in_ctb - 1)
                    {
                        WORD32 i4_is_last_ctb_x = 0;
                        WORD32 i4_is_last_ctb_y = 1;
                        WORD32 last_y_pos;
                        last_y_pos = (ps_sps->i2_pic_ht_in_ctb << ps_sps->i1_log2_ctb_size);
                        if(last_y_pos == ps_sps->i2_pic_height_in_luma_samples)
                        {
                            ihevcd_deblk_ctb(&ps_proc->s_deblk_ctxt, i4_is_last_ctb_x, i4_is_last_ctb_y);
                        }
                    }
                }
            }

            /* Update proc map for deblk*/
            ihevcd_proc_map_update(ps_proc, proc_type, nctb);

            num_ctb_tmp -= nctb;
            ihevcd_ctb_pos_update(ps_proc, nctb);
        }

        if(cur_slice_idx != ps_proc->i4_cur_slice_idx)
        {
            ps_proc->ps_slice_hdr = ps_codec->ps_slice_hdr_base + ((cur_slice_idx)&(MAX_SLICE_HDR_CNT - 1));
            ps_proc->i4_cur_slice_idx = cur_slice_idx;
        }
        /* Restore the saved variables  */
        num_ctb_tmp = num_ctb;
        ps_proc->i4_ctb_x -= num_ctb;
        ps_proc->i4_ctb_tile_x = cur_ctb_tile_x;
        ps_proc->i4_ctb_tile_y = cur_ctb_tile_y;
        ps_proc->pu1_pu_map = pu1_pu_map_cur;
        ps_proc->ps_tu = ps_tu_cur;
        nxt_ctb_slice_y = ps_proc->i4_ctb_slice_y;
        nxt_ctb_slice_x = ps_proc->i4_ctb_slice_x;
        ps_proc->i4_ctb_slice_y = cur_ctb_slice_y;
        ps_proc->i4_ctb_slice_x = cur_ctb_slice_x;
        proc_type = PROC_SAO;

        while(num_ctb_tmp)
        {


            /* Check proc map to ensure dependencies for SAO are met */
            ihevcd_proc_map_check(ps_proc, proc_type, nctb);

            ihevcd_slice_hdr_update(ps_proc);


            if(0 == FRAME_ILF_PAD || ps_codec->i4_num_cores != 1)
            {
                /* SAO is done even when it is disabled in the current slice, because
                 * it is performed on a shifted CTB and the neighbor CTBs can belong
                 * to different slices with SAO enabled */
                if(0 == ps_codec->i4_disable_sao_pic)
                {
                    ps_proc->s_sao_ctxt.ps_pps = ps_proc->ps_pps;
                    ps_proc->s_sao_ctxt.ps_sps = ps_proc->ps_sps;
                    ps_proc->s_sao_ctxt.ps_tile = ps_proc->ps_tile;
                    ps_proc->s_sao_ctxt.ps_codec = ps_proc->ps_codec;
                    ps_proc->s_sao_ctxt.ps_slice_hdr = ps_proc->ps_slice_hdr;
                    ps_proc->s_sao_ctxt.i4_cur_slice_idx = ps_proc->i4_cur_slice_idx;


#if SAO_PROCESS_SHIFT_CTB
                    ps_proc->s_sao_ctxt.i4_ctb_x = ps_proc->i4_ctb_x;
                    ps_proc->s_sao_ctxt.i4_ctb_y = ps_proc->i4_ctb_y;
                    ps_proc->s_sao_ctxt.is_chroma_yuv420sp_vu = (ps_codec->e_ref_chroma_fmt == IV_YUV_420SP_VU);

                    ihevcd_sao_shift_ctb(&ps_proc->s_sao_ctxt);
#else
                    if(ps_proc->i4_ctb_x > 1 && ps_proc->i4_ctb_y > 0)
                    {
                        ps_proc->s_sao_ctxt.i4_ctb_x = ps_proc->i4_ctb_x - 2;
                        ps_proc->s_sao_ctxt.i4_ctb_y = ps_proc->i4_ctb_y - 1;

                        ihevcd_sao_ctb(&ps_proc->s_sao_ctxt);
                    }

                    if(ps_sps->i2_pic_wd_in_ctb - 1 == ps_proc->i4_ctb_x && ps_proc->i4_ctb_y > 0)
                    {
                        ps_proc->s_sao_ctxt.i4_ctb_x = ps_proc->i4_ctb_x - 1;
                        ps_proc->s_sao_ctxt.i4_ctb_y = ps_proc->i4_ctb_y - 1;

                        ihevcd_sao_ctb(&ps_proc->s_sao_ctxt);

                        ps_proc->s_sao_ctxt.i4_ctb_x = ps_proc->i4_ctb_x;
                        ps_proc->s_sao_ctxt.i4_ctb_y = ps_proc->i4_ctb_y - 1;

                        ihevcd_sao_ctb(&ps_proc->s_sao_ctxt);

                        if(ps_sps->i2_pic_ht_in_ctb - 1 == ps_proc->i4_ctb_y)
                        {
                            WORD32 i4_ctb_x;
                            ps_proc->s_sao_ctxt.i4_ctb_y = ps_proc->i4_ctb_y;
                            for(i4_ctb_x = 0; i4_ctb_x < ps_sps->i2_pic_wd_in_ctb; i4_ctb_x++)
                            {
                                ps_proc->s_sao_ctxt.i4_ctb_x = i4_ctb_x;
                                ihevcd_sao_ctb(&ps_proc->s_sao_ctxt);
                            }
                        }
                    }
#endif
                }


                /* Call padding if required */
                {
#if SAO_PROCESS_SHIFT_CTB

                    if(0 == ps_proc->i4_ctb_x)
                    {
                        WORD32 pad_ht_luma;
                        WORD32 pad_ht_chroma;

                        ps_proc->pu1_cur_ctb_luma = ps_proc->pu1_cur_pic_luma
                                        + (ps_proc->i4_ctb_x * ctb_size
                                        + ps_proc->i4_ctb_y * ctb_size
                                        * ps_codec->i4_strd);
                        ps_proc->pu1_cur_ctb_chroma = ps_proc->pu1_cur_pic_chroma
                                        + ps_proc->i4_ctb_x * ctb_size
                                        + (ps_proc->i4_ctb_y * ctb_size * ps_codec->i4_strd / 2);

                        pad_ht_luma = ctb_size;
                        pad_ht_luma += (ps_sps->i2_pic_ht_in_ctb - 1) == ps_proc->i4_ctb_y ? 8 : 0;
                        pad_ht_chroma = ctb_size / 2;
                        /* Pad left after 1st CTB is processed */
                        ps_codec->s_func_selector.ihevc_pad_left_luma_fptr(ps_proc->pu1_cur_ctb_luma - 8 * ps_codec->i4_strd, ps_codec->i4_strd, pad_ht_luma, PAD_LEFT);
                        ps_codec->s_func_selector.ihevc_pad_left_chroma_fptr(ps_proc->pu1_cur_ctb_chroma - 16 * ps_codec->i4_strd, ps_codec->i4_strd, pad_ht_chroma, PAD_LEFT);
                    }

                    if((ps_sps->i2_pic_wd_in_ctb - 1) == ps_proc->i4_ctb_x)
                    {
                        WORD32 pad_ht_luma;
                        WORD32 pad_ht_chroma;
                        WORD32 cols_remaining = ps_sps->i2_pic_width_in_luma_samples - (ps_proc->i4_ctb_x << ps_sps->i1_log2_ctb_size);

                        ps_proc->pu1_cur_ctb_luma = ps_proc->pu1_cur_pic_luma
                                        + (ps_proc->i4_ctb_x * ctb_size
                                        + ps_proc->i4_ctb_y * ctb_size
                                        * ps_codec->i4_strd);
                        ps_proc->pu1_cur_ctb_chroma = ps_proc->pu1_cur_pic_chroma
                                        + ps_proc->i4_ctb_x * ctb_size
                                        + (ps_proc->i4_ctb_y * ctb_size * ps_codec->i4_strd / 2);

                        pad_ht_luma = ctb_size;
                        pad_ht_chroma = ctb_size / 2;
                        if((ps_sps->i2_pic_ht_in_ctb - 1) == ps_proc->i4_ctb_y)
                        {
                            pad_ht_luma += 8;
                            pad_ht_chroma += 16;
                            ps_codec->s_func_selector.ihevc_pad_left_chroma_fptr(ps_proc->pu1_cur_pic_chroma + (ps_sps->i2_pic_height_in_luma_samples / 2 - 16) * ps_codec->i4_strd,
                                                                                 ps_codec->i4_strd, 16, PAD_LEFT);
                        }
                        /* Pad right after last CTB in the current row is processed */
                        ps_codec->s_func_selector.ihevc_pad_right_luma_fptr(ps_proc->pu1_cur_ctb_luma + cols_remaining - 8 * ps_codec->i4_strd, ps_codec->i4_strd, pad_ht_luma, PAD_RIGHT);
                        ps_codec->s_func_selector.ihevc_pad_right_chroma_fptr(ps_proc->pu1_cur_ctb_chroma + cols_remaining - 16 * ps_codec->i4_strd, ps_codec->i4_strd, pad_ht_chroma, PAD_RIGHT);

                        if((ps_sps->i2_pic_ht_in_ctb - 1) == ps_proc->i4_ctb_y)
                        {
                            UWORD8 *pu1_buf;
                            /* Since SAO is shifted by 8x8, chroma padding can not be done till second row is processed */
                            /* Hence moving top padding to to end of frame, Moving it to second row also results in problems when there is only one row */
                            /* Pad top after padding left and right for current rows after processing 1st CTB row */
                            ihevc_pad_top(ps_proc->pu1_cur_pic_luma - PAD_LEFT, ps_codec->i4_strd, ps_sps->i2_pic_width_in_luma_samples + PAD_WD, PAD_TOP);
                            ihevc_pad_top(ps_proc->pu1_cur_pic_chroma - PAD_LEFT, ps_codec->i4_strd, ps_sps->i2_pic_width_in_luma_samples + PAD_WD, PAD_TOP / 2);

                            pu1_buf = ps_proc->pu1_cur_pic_luma + ps_codec->i4_strd * ps_sps->i2_pic_height_in_luma_samples - PAD_LEFT;
                            /* Pad top after padding left and right for current rows after processing 1st CTB row */
                            ihevc_pad_bottom(pu1_buf, ps_codec->i4_strd, ps_sps->i2_pic_width_in_luma_samples + PAD_WD, PAD_BOT);

                            pu1_buf = ps_proc->pu1_cur_pic_chroma + ps_codec->i4_strd * (ps_sps->i2_pic_height_in_luma_samples / 2) - PAD_LEFT;
                            ihevc_pad_bottom(pu1_buf, ps_codec->i4_strd, ps_sps->i2_pic_width_in_luma_samples + PAD_WD, PAD_BOT / 2);
                        }
                    }
#else
                    if(ps_proc->i4_ctb_y > 1)
                    {
                        if(0 == ps_proc->i4_ctb_x)
                        {
                            WORD32 pad_ht_luma;
                            WORD32 pad_ht_chroma;

                            pad_ht_luma = ctb_size;
                            pad_ht_chroma = ctb_size / 2;
                            /* Pad left after 1st CTB is processed */
                            ps_codec->s_func_selector.ihevc_pad_left_luma_fptr(ps_proc->pu1_cur_ctb_luma - 2 * ctb_size * ps_codec->i4_strd, ps_codec->i4_strd, pad_ht_luma, PAD_LEFT);
                            ps_codec->s_func_selector.ihevc_pad_left_chroma_fptr(ps_proc->pu1_cur_ctb_chroma - ctb_size * ps_codec->i4_strd, ps_codec->i4_strd, pad_ht_chroma, PAD_LEFT);
                        }
                        else if((ps_sps->i2_pic_wd_in_ctb - 1) == ps_proc->i4_ctb_x)
                        {
                            WORD32 pad_ht_luma;
                            WORD32 pad_ht_chroma;
                            WORD32 cols_remaining = ps_sps->i2_pic_width_in_luma_samples - (ps_proc->i4_ctb_x << ps_sps->i1_log2_ctb_size);

                            pad_ht_luma = ((ps_sps->i2_pic_ht_in_ctb - 1) == ps_proc->i4_ctb_y) ? 3 * ctb_size : ctb_size;
                            pad_ht_chroma = ((ps_sps->i2_pic_ht_in_ctb - 1) == ps_proc->i4_ctb_y) ? 3 * ctb_size / 2 : ctb_size / 2;
                            /* Pad right after last CTB in the current row is processed */
                            ps_codec->s_func_selector.ihevc_pad_right_luma_fptr(ps_proc->pu1_cur_ctb_luma + cols_remaining - 2 * ctb_size * ps_codec->i4_strd, ps_codec->i4_strd, pad_ht_luma, PAD_RIGHT);
                            ps_codec->s_func_selector.ihevc_pad_right_chroma_fptr(ps_proc->pu1_cur_ctb_chroma + cols_remaining - ctb_size * ps_codec->i4_strd, ps_codec->i4_strd, pad_ht_chroma, PAD_RIGHT);

                            if((ps_sps->i2_pic_ht_in_ctb - 1) == ps_proc->i4_ctb_y)
                            {
                                UWORD8 *pu1_buf;
                                WORD32 pad_ht_luma;
                                WORD32 pad_ht_chroma;

                                pad_ht_luma = 2 * ctb_size;
                                pad_ht_chroma = ctb_size;

                                ps_codec->s_func_selector.ihevc_pad_left_luma_fptr(ps_proc->pu1_cur_pic_luma + ps_codec->i4_strd * (ps_sps->i2_pic_height_in_luma_samples - 2 * ctb_size),
                                                                                   ps_codec->i4_strd, pad_ht_luma, PAD_LEFT);
                                ps_codec->s_func_selector.ihevc_pad_left_chroma_fptr(ps_proc->pu1_cur_pic_chroma + ps_codec->i4_strd * (ps_sps->i2_pic_height_in_luma_samples / 2 - ctb_size),
                                                                                     ps_codec->i4_strd, pad_ht_chroma, PAD_LEFT);

                                /* Since SAO is shifted by 8x8, chroma padding can not be done till second row is processed */
                                /* Hence moving top padding to to end of frame, Moving it to second row also results in problems when there is only one row */
                                /* Pad top after padding left and right for current rows after processing 1st CTB row */
                                ihevc_pad_top(ps_proc->pu1_cur_pic_luma - PAD_LEFT, ps_codec->i4_strd, ps_sps->i2_pic_width_in_luma_samples + PAD_WD, PAD_TOP);
                                ihevc_pad_top(ps_proc->pu1_cur_pic_chroma - PAD_LEFT, ps_codec->i4_strd, ps_sps->i2_pic_width_in_luma_samples + PAD_WD, PAD_TOP / 2);

                                pu1_buf = ps_proc->pu1_cur_pic_luma + ps_codec->i4_strd * ps_sps->i2_pic_height_in_luma_samples - PAD_LEFT;
                                /* Pad top after padding left and right for current rows after processing 1st CTB row */
                                ihevc_pad_bottom(pu1_buf, ps_codec->i4_strd, ps_sps->i2_pic_width_in_luma_samples + PAD_WD, PAD_BOT);

                                pu1_buf = ps_proc->pu1_cur_pic_chroma + ps_codec->i4_strd * (ps_sps->i2_pic_height_in_luma_samples / 2) - PAD_LEFT;
                                ihevc_pad_bottom(pu1_buf, ps_codec->i4_strd, ps_sps->i2_pic_width_in_luma_samples + PAD_WD, PAD_BOT / 2);
                            }
                        }
                    }
#endif
                }
            }


            /* Update proc map for SAO*/
            ihevcd_proc_map_update(ps_proc, proc_type, nctb);
            /* Update proc map for Completion of CTB*/
            ihevcd_proc_map_update(ps_proc, PROC_ALL, nctb);
            {
                tile_t *ps_tile;

                ps_tile = ps_proc->ps_tile;
                num_ctb_tmp -= nctb;

                ps_proc->i4_ctb_tile_x += nctb;
                ps_proc->i4_ctb_x += nctb;

                ps_proc->i4_ctb_slice_x += nctb;


                /* Update tile counters */
                if(ps_proc->i4_ctb_tile_x >= (ps_tile->u2_wd))
                {
                    /*End of tile row*/
                    ps_proc->i4_ctb_tile_x = 0;
                    ps_proc->i4_ctb_x = ps_tile->u1_pos_x;

                    ps_proc->i4_ctb_tile_y++;
                    ps_proc->i4_ctb_y++;
                    if(ps_proc->i4_ctb_tile_y == ps_tile->u2_ht)
                    {
                        /* Reached End of Tile */
                        ps_proc->i4_ctb_tile_y = 0;
                        ps_proc->i4_ctb_tile_x = 0;
                        ps_proc->ps_tile++;
                        //End of picture
                        if(!((ps_tile->u2_ht + ps_tile->u1_pos_y  ==  ps_sps->i2_pic_ht_in_ctb) && (ps_tile->u2_wd + ps_tile->u1_pos_x  ==  ps_sps->i2_pic_wd_in_ctb)))
                        {
                            ps_tile = ps_proc->ps_tile;
                            ps_proc->i4_ctb_x = ps_tile->u1_pos_x;
                            ps_proc->i4_ctb_y = ps_tile->u1_pos_y;

                        }
                    }
                }
            }
        }

        ps_proc->i4_ctb_cnt -= num_ctb;
    }
    return ret;
}

void ihevcd_init_proc_ctxt(process_ctxt_t *ps_proc, WORD32 tu_coeff_data_ofst)
{
    codec_t *ps_codec;
    slice_header_t *ps_slice_hdr;
    pps_t *ps_pps;
    sps_t *ps_sps;
    tile_t *ps_tile, *ps_tile_prev;
    WORD32 tile_idx;
    WORD32 ctb_size;
    WORD32 num_minpu_in_ctb;
    WORD32 num_ctb_in_row;
    WORD32 ctb_addr;
    WORD32 i4_wd_in_ctb;
    WORD32 tile_start_ctb_idx;
    WORD32 slice_start_ctb_idx;
    WORD32 check_tile_wd;
    WORD32 continuous_tiles = 0; //Refers to tiles that are continuous, within a slice, horizontally

    ps_codec = ps_proc->ps_codec;

    ps_slice_hdr = ps_codec->ps_slice_hdr_base + ((ps_proc->i4_cur_slice_idx) & (MAX_SLICE_HDR_CNT - 1));
    ps_proc->ps_slice_hdr = ps_slice_hdr;
    ps_proc->ps_pps = ps_codec->ps_pps_base + ps_slice_hdr->i1_pps_id;
    ps_pps = ps_proc->ps_pps;
    ps_proc->ps_sps = ps_codec->ps_sps_base + ps_pps->i1_sps_id;
    ps_sps = ps_proc->ps_sps;
    ps_proc->i4_init_done = 1;
    ctb_size = 1 << ps_sps->i1_log2_ctb_size;
    num_minpu_in_ctb = (ctb_size / MIN_PU_SIZE) * (ctb_size / MIN_PU_SIZE);
    num_ctb_in_row = ps_sps->i2_pic_wd_in_ctb;

    ps_proc->s_sao_ctxt.pu1_slice_idx = ps_proc->pu1_slice_idx;

    ihevcd_get_tile_pos(ps_pps, ps_sps, ps_proc->i4_ctb_x, ps_proc->i4_ctb_y,
                        &ps_proc->i4_ctb_tile_x, &ps_proc->i4_ctb_tile_y,
                        &tile_idx);

    ps_proc->ps_tile = ps_pps->ps_tile + tile_idx;
    ps_proc->i4_cur_tile_idx = tile_idx;
    ps_tile = ps_proc->ps_tile;

    if(ps_pps->i1_tiles_enabled_flag)
    {
        if(tile_idx)
            ps_tile_prev = ps_tile - 1;
        else
            ps_tile_prev = ps_tile;

        slice_start_ctb_idx = ps_slice_hdr->i2_ctb_x + (ps_slice_hdr->i2_ctb_y * ps_sps->i2_pic_wd_in_ctb);
        tile_start_ctb_idx = ps_tile->u1_pos_x + (ps_tile->u1_pos_y * ps_sps->i2_pic_wd_in_ctb);

        /*Check if
         * 1. Last tile that ends in frame boundary and 1st tile in next row belongs to same slice
         * 1.1. If it does, check if the slice that has these tiles spans across the frame row.
         * 2. Vertical tiles are present within a slice */
        if(((ps_slice_hdr->i2_ctb_x == ps_tile->u1_pos_x) && (ps_slice_hdr->i2_ctb_y != ps_tile->u1_pos_y)))
        {
            continuous_tiles = 1;
        }
        else
        {
            check_tile_wd = ps_slice_hdr->i2_ctb_x + ps_tile_prev->u2_wd;
            if(!(((check_tile_wd >= ps_sps->i2_pic_wd_in_ctb) && (check_tile_wd % ps_sps->i2_pic_wd_in_ctb == ps_tile->u1_pos_x))
                                            || ((ps_slice_hdr->i2_ctb_x == ps_tile->u1_pos_x))))
            {
                continuous_tiles = 1;
            }
        }

        {
            WORD32 i2_independent_ctb_x = ps_slice_hdr->i2_independent_ctb_x;
            WORD32 i2_independent_ctb_y = ps_slice_hdr->i2_independent_ctb_y;

            /* Handles cases where
             * 1. Slices begin at the start of each tile
             * 2. Tiles lie in the same slice row.i.e, starting tile_x > slice_x, but tile_y == slice_y
             * */
            if(ps_proc->i4_ctb_x >= i2_independent_ctb_x)
            {
                ps_proc->i4_ctb_slice_x = ps_proc->i4_ctb_x - i2_independent_ctb_x;
            }
            else
            {
                /* Indicates multiple tiles in a slice case where
                 * The new tile belongs to an older slice that started in the previous rows-not the present row
                 * & (tile_y > slice_y and tile_x < slice_x)
                 */
                if((slice_start_ctb_idx < tile_start_ctb_idx) && (continuous_tiles))
                {
                    i4_wd_in_ctb = ps_sps->i2_pic_wd_in_ctb;
                }
                /* Indicates many-tiles-in-one-slice case, for slices that end without spanning the frame width*/
                else
                {
                    i4_wd_in_ctb = ps_tile->u2_wd;
                }

                if(continuous_tiles)
                {
                    ps_proc->i4_ctb_slice_x = i4_wd_in_ctb
                                    - (i2_independent_ctb_x - ps_proc->i4_ctb_x);
                }
                else
                {
                    ps_proc->i4_ctb_slice_x = ps_proc->i4_ctb_x - ps_tile->u1_pos_x;
                }
            }
            /* Initialize ctb slice y to zero and at the start of slice row initialize it
        to difference between ctb_y and slice's start ctb y */

            ps_proc->i4_ctb_slice_y = ps_proc->i4_ctb_y - i2_independent_ctb_y;

            /*If beginning of tile, check if slice counters are set correctly*/
            if((0 == ps_proc->i4_ctb_tile_x) && (0 == ps_proc->i4_ctb_tile_y))
            {
                if(ps_slice_hdr->i1_dependent_slice_flag)
                {
                    ps_proc->i4_ctb_slice_x = 0;
                    ps_proc->i4_ctb_slice_y = 0;
                }
                /*For slices that span across multiple tiles*/
                else if(slice_start_ctb_idx < tile_start_ctb_idx)
                {
                    ps_proc->i4_ctb_slice_y = ps_tile->u1_pos_y - i2_independent_ctb_y;
                    /* Two Cases
                     * 1 - slice spans across frame-width- but dose not start from 1st column
                     * 2 - Slice spans across multiple tiles anywhere is a frame
                     */
                    /*TODO:In a multiple slice clip,  if an independent slice span across more than 2 tiles in a row, it is not supported*/
                    if(continuous_tiles) //Case 2-implemented for slices that span not more than 2 tiles
                    {
                        if(i2_independent_ctb_y <= ps_tile->u1_pos_y)
                        {
                            //Check if ctb x is before or after
                            if(i2_independent_ctb_x > ps_tile->u1_pos_x)
                            {
                                ps_proc->i4_ctb_slice_y -= 1;
                            }
                        }
                    }
                }
            }
            //Slice starts from a column which is not the starting tile-column, but is within the tile
            if(((i2_independent_ctb_x - ps_tile->u1_pos_x) != 0) && ((ps_proc->i4_ctb_slice_y != 0))
                            && ((i2_independent_ctb_x >= ps_tile->u1_pos_x) && (i2_independent_ctb_x < ps_tile->u1_pos_x + ps_tile->u2_wd)))
            {
                ps_proc->i4_ctb_slice_y -= 1;
            }
        }
    }
    else
    {
        WORD32 i2_independent_ctb_x = ps_slice_hdr->i2_independent_ctb_x;
        WORD32 i2_independent_ctb_y = ps_slice_hdr->i2_independent_ctb_y;


        {
            ps_proc->i4_ctb_slice_x = ps_proc->i4_ctb_x - i2_independent_ctb_x;
            ps_proc->i4_ctb_slice_y = ps_proc->i4_ctb_y - i2_independent_ctb_y;
            if(ps_proc->i4_ctb_slice_x < 0)
            {
                ps_proc->i4_ctb_slice_x += ps_sps->i2_pic_wd_in_ctb;
                ps_proc->i4_ctb_slice_y -= 1;
            }

            /* Initialize ctb slice y to zero and at the start of slice row initialize it
            to difference between ctb_y and slice's start ctb y */
        }
    }

    /* Compute TU offset for the current CTB set */
    {

        WORD32 ctb_luma_min_tu_cnt;
        WORD32 ctb_addr;

        ctb_addr = ps_proc->i4_ctb_y * num_ctb_in_row + ps_proc->i4_ctb_x;

        ctb_luma_min_tu_cnt = (1 << ps_sps->i1_log2_ctb_size) / MIN_TU_SIZE;
        ctb_luma_min_tu_cnt *= ctb_luma_min_tu_cnt;

        ps_proc->pu1_tu_map = ps_proc->pu1_pic_tu_map
                        + ctb_luma_min_tu_cnt * ctb_addr;
        if(1 == ps_codec->i4_num_cores)
        {
            ps_proc->ps_tu = ps_proc->ps_pic_tu + ps_proc->pu4_pic_tu_idx[ctb_addr % RESET_TU_BUF_NCTB];
        }
        else
        {
            ps_proc->ps_tu = ps_proc->ps_pic_tu + ps_proc->pu4_pic_tu_idx[ctb_addr];
        }
        ps_proc->pv_tu_coeff_data = (UWORD8 *)ps_proc->pv_pic_tu_coeff_data
                        + tu_coeff_data_ofst;

    }

    /* Compute PU related elements for the current CTB set */
    {
        WORD32 pu_idx;
        ctb_addr = ps_proc->i4_ctb_y * num_ctb_in_row + ps_proc->i4_ctb_x;
        pu_idx = ps_proc->pu4_pic_pu_idx[ctb_addr];
        ps_proc->pu1_pu_map = ps_proc->pu1_pic_pu_map
                        + ctb_addr * num_minpu_in_ctb;
        ps_proc->ps_pu = ps_proc->ps_pic_pu + pu_idx;
    }

    /* Number of ctbs processed in one loop of process function */
    {
        ps_proc->i4_nctb = MIN(ps_codec->u4_nctb, ps_tile->u2_wd);
    }

}
void ihevcd_process_thread(process_ctxt_t *ps_proc)
{
    {
        ithread_set_affinity(ps_proc->i4_id + 1);
    }
    while(1)
    {
        IHEVCD_ERROR_T ret;
        proc_job_t s_job;

        ret = ihevcd_jobq_dequeue((jobq_t *)ps_proc->pv_proc_jobq, &s_job,
                                  sizeof(proc_job_t), 1);
        if((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret)
            break;

        ps_proc->i4_ctb_cnt = s_job.i2_ctb_cnt;
        ps_proc->i4_ctb_x = s_job.i2_ctb_x;
        ps_proc->i4_ctb_y = s_job.i2_ctb_y;
        ps_proc->i4_cur_slice_idx = s_job.i2_slice_idx;



        if(CMD_PROCESS == s_job.i4_cmd)
        {
            ihevcd_init_proc_ctxt(ps_proc, s_job.i4_tu_coeff_data_ofst);
            ihevcd_process(ps_proc);
        }
        else if(CMD_FMTCONV == s_job.i4_cmd)
        {
            sps_t *ps_sps;
            codec_t *ps_codec;
            ivd_out_bufdesc_t *ps_out_buffer;
            WORD32 num_rows;

            if(0 == ps_proc->i4_init_done)
            {
                ihevcd_init_proc_ctxt(ps_proc, 0);
            }
            ps_sps = ps_proc->ps_sps;
            ps_codec = ps_proc->ps_codec;
            ps_out_buffer = ps_proc->ps_out_buffer;
            num_rows = 1 << ps_sps->i1_log2_ctb_size;

            num_rows = MIN(num_rows, (ps_codec->i4_disp_ht - (s_job.i2_ctb_y << ps_sps->i1_log2_ctb_size)));

            if(num_rows < 0)
                num_rows = 0;

            ihevcd_fmt_conv(ps_proc->ps_codec, ps_proc, ps_out_buffer->pu1_bufs[0], ps_out_buffer->pu1_bufs[1], ps_out_buffer->pu1_bufs[2],
                            s_job.i2_ctb_y << ps_sps->i1_log2_ctb_size, num_rows);
        }
    }
    //ithread_exit(0);
    return;
}

