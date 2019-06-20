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
*  ihevc_deblk.c
*
* @brief
*  Contains definition for the ctb level deblk function
*
* @author
*  Srinivas T
*
* @par List of Functions:
*   - ihevc_deblk()
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
#include "ihevcd_debug.h"

#include "ihevc_deblk.h"
#include "ihevc_deblk_tables.h"
#include "ihevcd_profile.h"
/**
*******************************************************************************
*
* @brief
*     Deblock CTB level function.
*
* @par Description:
*     For a given CTB, deblocking on both vertical and
*     horizontal edges is done. Both the luma and chroma
*     blocks are processed
*
* @param[in] ps_deblk
*  Pointer to the deblock context
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevcd_deblk_ctb(deblk_ctxt_t *ps_deblk,
                      WORD32 i4_is_last_ctb_x,
                      WORD32 i4_is_last_ctb_y)
{
    WORD32 ctb_size;
    WORD32 log2_ctb_size;
    UWORD32 u4_bs;
    WORD32 bs_tz; /*Leading zeros in boundary strength*/
    WORD32 qp_p, qp_q;

    WORD32 filter_p, filter_q;

    UWORD8 *pu1_src;
    WORD32 qp_strd;
    UWORD32 *pu4_vert_bs, *pu4_horz_bs;
    UWORD32 *pu4_ctb_vert_bs, *pu4_ctb_horz_bs;
    WORD32 bs_strd;
    WORD32 src_strd;
    UWORD8 *pu1_qp;
    UWORD16 *pu2_ctb_no_loop_filter_flag;
    UWORD16 au2_ctb_no_loop_filter_flag[9];

    WORD32 col, row;

    /* Flag to indicate if QP is constant in CTB
     * 0 - top_left, 1 - top, 2 - left, 3 - current */
    UWORD32 u4_qp_const_in_ctb[4] = { 0, 0, 0, 0 };
    WORD32 ctb_indx;
    WORD32  chroma_yuv420sp_vu = ps_deblk->is_chroma_yuv420sp_vu;
    sps_t *ps_sps;
    pps_t *ps_pps;
    codec_t *ps_codec;
    slice_header_t *ps_slice_hdr;

    PROFILE_DISABLE_DEBLK();

    ps_sps = ps_deblk->ps_sps;
    ps_pps = ps_deblk->ps_pps;
    ps_codec = ps_deblk->ps_codec;
    ps_slice_hdr = ps_deblk->ps_slice_hdr;

    log2_ctb_size = ps_sps->i1_log2_ctb_size;
    ctb_size = (1 << ps_sps->i1_log2_ctb_size);

    /* strides are in units of number of bytes */
    /* ctb_size * ctb_size / 8 / 16 is the number of bytes needed per CTB */
    bs_strd = (ps_sps->i2_pic_wd_in_ctb + 1) << (2 * log2_ctb_size - 7);

    pu4_vert_bs = (UWORD32 *)((UWORD8 *)ps_deblk->s_bs_ctxt.pu4_pic_vert_bs +
                    (ps_deblk->i4_ctb_x << (2 * log2_ctb_size - 7)) +
                    ps_deblk->i4_ctb_y * bs_strd);
    pu4_ctb_vert_bs = pu4_vert_bs;

    pu4_horz_bs = (UWORD32 *)((UWORD8 *)ps_deblk->s_bs_ctxt.pu4_pic_horz_bs +
                    (ps_deblk->i4_ctb_x << (2 * log2_ctb_size - 7)) +
                    ps_deblk->i4_ctb_y * bs_strd);
    pu4_ctb_horz_bs = pu4_horz_bs;

    qp_strd = ps_sps->i2_pic_wd_in_ctb << (log2_ctb_size - 3);
    pu1_qp = ps_deblk->s_bs_ctxt.pu1_pic_qp + ((ps_deblk->i4_ctb_x + ps_deblk->i4_ctb_y * qp_strd) << (log2_ctb_size - 3));

    pu2_ctb_no_loop_filter_flag = ps_deblk->au2_ctb_no_loop_filter_flag;

    ctb_indx = ps_deblk->i4_ctb_x + ps_sps->i2_pic_wd_in_ctb * ps_deblk->i4_ctb_y;
    if(i4_is_last_ctb_y)
    {
        pu4_vert_bs = (UWORD32 *)((UWORD8 *)pu4_vert_bs + bs_strd);
        pu4_ctb_vert_bs = pu4_vert_bs;
        /* ctb_size/8 is the number of edges per CTB
         * ctb_size/4 is the number of BS values needed per edge
         * divided by 8 for the number of bytes
         * 2 is the number of bits needed for each BS value */
        memset(pu4_vert_bs, 0, 1 << (2 * log2_ctb_size - 7));

        pu1_qp += (qp_strd << (log2_ctb_size - 3));
        pu2_ctb_no_loop_filter_flag += (ctb_size >> 3);
        ctb_indx += ps_sps->i2_pic_wd_in_ctb;
    }

    if(i4_is_last_ctb_x)
    {
        pu4_horz_bs = (UWORD32 *)((UWORD8 *)pu4_horz_bs + (1 << (2 * log2_ctb_size - 7)));
        pu4_ctb_horz_bs = pu4_horz_bs;
        memset(pu4_horz_bs, 0, 1 << (2 * log2_ctb_size - 7));

        pu1_qp += (ctb_size >> 3);

        for(row = 0; row < (ctb_size >> 3) + 1; row++)
            au2_ctb_no_loop_filter_flag[row] = ps_deblk->au2_ctb_no_loop_filter_flag[row] >> (ctb_size >> 3);
        pu2_ctb_no_loop_filter_flag = au2_ctb_no_loop_filter_flag;
        ctb_indx += 1;
    }

    u4_qp_const_in_ctb[3] = ps_deblk->s_bs_ctxt.pu1_pic_qp_const_in_ctb[(ctb_indx) >> 3] & (1 << (ctb_indx & 7));

    if(ps_deblk->i4_ctb_x || i4_is_last_ctb_x)
    {
        u4_qp_const_in_ctb[2] = ps_deblk->s_bs_ctxt.pu1_pic_qp_const_in_ctb[(ctb_indx - 1) >> 3] & (1 << ((ctb_indx - 1) & 7));
    }

    if((ps_deblk->i4_ctb_x || i4_is_last_ctb_x) && (ps_deblk->i4_ctb_y || i4_is_last_ctb_y))
    {
        u4_qp_const_in_ctb[0] =
                        ps_deblk->s_bs_ctxt.pu1_pic_qp_const_in_ctb[(ctb_indx - ps_sps->i2_pic_wd_in_ctb - 1) >> 3] &
                        (1 << ((ctb_indx - ps_sps->i2_pic_wd_in_ctb - 1) & 7));
    }



    if(ps_deblk->i4_ctb_y || i4_is_last_ctb_y)
    {
        u4_qp_const_in_ctb[1] =
                        ps_deblk->s_bs_ctxt.pu1_pic_qp_const_in_ctb[(ctb_indx - ps_sps->i2_pic_wd_in_ctb) >> 3] &
                        (1 << ((ctb_indx - ps_sps->i2_pic_wd_in_ctb) & 7));
    }

    src_strd = ps_codec->i4_strd;

    /* Luma Vertical Edge */

    if(0 == i4_is_last_ctb_x)
    {
        /* Top CTB's slice header */
        slice_header_t *ps_slice_hdr_top;
        {
            WORD32 cur_ctb_indx = ps_deblk->i4_ctb_x + ps_deblk->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb;
            if(i4_is_last_ctb_y)
                cur_ctb_indx += ps_sps->i2_pic_wd_in_ctb;
            ps_slice_hdr_top = ps_codec->ps_slice_hdr_base + ps_deblk->pu1_slice_idx[cur_ctb_indx - ps_sps->i2_pic_wd_in_ctb];
        }

        pu1_src = ps_deblk->pu1_cur_pic_luma + ((ps_deblk->i4_ctb_x + ps_deblk->i4_ctb_y * ps_deblk->ps_codec->i4_strd) << (log2_ctb_size));
        pu1_src += i4_is_last_ctb_y ? ps_deblk->ps_codec->i4_strd << log2_ctb_size : 0;

        /** Deblocking is done on a shifted CTB -
         *  Vertical edge processing is done by shifting the CTB up by four pixels */
        pu1_src -= 4 * src_strd;

        for(col = 0; col < ctb_size / 8; col++)
        {
            WORD32 shift = 0;

            /*  downshift vert_bs by ctb_size/2 for each column
             *  shift = (col & ((MAX_CTB_SIZE >> log2_ctb_size) - 1)) << (log2_ctb_size - 1);
             *  which will reduce to the following assuming ctb size is one of 16, 32 and 64
             *  and deblocking is done on 8x8 grid
             */
            if(6 != log2_ctb_size)
                shift = (col & 1) << (log2_ctb_size - 1);

            /* BS for the column - Last row is excluded and the top row is included*/
            u4_bs = (pu4_vert_bs[0] >> shift) << 2;

            if(ps_deblk->i4_ctb_y || i4_is_last_ctb_y)
            {
                /* Picking the last BS of the previous CTB corresponding to the same column */
                UWORD32 *pu4_vert_bs_top = (UWORD32 *)((UWORD8 *)pu4_vert_bs - bs_strd);
                UWORD32 u4_top_bs = (*pu4_vert_bs_top) >> (shift + (1 << (log2_ctb_size - 1)) - 2);
                u4_bs |= u4_top_bs & 3;
            }

            for(row = 0; row < ctb_size / 4;)
            {
                WORD8 i1_beta_offset_div2 = ps_slice_hdr->i1_beta_offset_div2;
                WORD8 i1_tc_offset_div2 = ps_slice_hdr->i1_tc_offset_div2;

                /* Trailing zeros are computed and the corresponding rows are not processed */
                bs_tz = CTZ(u4_bs) >> 1;
                if(0 != bs_tz)
                {
                    u4_bs = u4_bs >> (bs_tz << 1);
                    if((row + bs_tz) >= (ctb_size / 4))
                        pu1_src += 4 * (ctb_size / 4 - row) * src_strd;
                    else
                        pu1_src += 4 * bs_tz  * src_strd;

                    row += bs_tz;
                    continue;
                }

                if(0 == row)
                {
                    i1_beta_offset_div2 = ps_slice_hdr_top->i1_beta_offset_div2;
                    i1_tc_offset_div2 = ps_slice_hdr_top->i1_tc_offset_div2;

                    if(0 == col)
                    {
                        qp_p = u4_qp_const_in_ctb[0] ?
                                        pu1_qp[-ctb_size / 8 * qp_strd - ctb_size / 8] :
                                        pu1_qp[-qp_strd - 1];
                    }
                    else
                    {
                        qp_p = u4_qp_const_in_ctb[1] ?
                                        pu1_qp[-ctb_size / 8 * qp_strd] :
                                        pu1_qp[col - 1 - qp_strd];
                    }

                    qp_q = u4_qp_const_in_ctb[1] ?
                                    pu1_qp[-ctb_size / 8 * qp_strd] :
                                    pu1_qp[col - qp_strd];
                }
                else
                {
                    if(0 == col)
                    {
                        qp_p = u4_qp_const_in_ctb[2] ?
                                        pu1_qp[-ctb_size / 8] :
                                        pu1_qp[((row - 1) >> 1) * qp_strd - 1];
                    }
                    else
                    {
                        qp_p = u4_qp_const_in_ctb[3] ?
                                        pu1_qp[0] :
                                        pu1_qp[((row - 1) >> 1) * qp_strd + col - 1];
                    }

                    qp_q = u4_qp_const_in_ctb[3] ?
                                    pu1_qp[0] :
                                    pu1_qp[((row - 1) >> 1) * qp_strd + col];
                }

                filter_p = (pu2_ctb_no_loop_filter_flag[(row + 1) >> 1] >> col) & 1;
                filter_q = (pu2_ctb_no_loop_filter_flag[(row + 1) >> 1] >> col) & 2;
                /* filter_p and filter_q are inverted as they are calculated using no_loop_filter_flags */
                filter_p = !filter_p;
                filter_q = !filter_q;

                if(filter_p || filter_q)
                {
                    DUMP_DEBLK_LUMA_VERT(pu1_src, src_strd,
                                         u4_bs & 3, qp_p, qp_q,
                                         ps_slice_hdr->i1_beta_offset_div2,
                                         ps_slice_hdr->i1_tc_offset_div2,
                                         filter_p, filter_q);
                    ps_codec->s_func_selector.ihevc_deblk_luma_vert_fptr(pu1_src, src_strd,
                                                                         u4_bs & 3, qp_p, qp_q,
                                                                         i1_beta_offset_div2,
                                                                         i1_tc_offset_div2,
                                                                         filter_p, filter_q);
                }

                pu1_src += 4 * src_strd;
                u4_bs = u4_bs >> 2;
                row++;
            }

            if((64 == ctb_size) ||
                            ((32 == ctb_size) && (col & 1)))
            {
                pu4_vert_bs++;
            }
            pu1_src -= (src_strd << log2_ctb_size);
            pu1_src += 8;
        }
        pu4_vert_bs = pu4_ctb_vert_bs;
    }


    /* Luma Horizontal Edge */

    if(0 == i4_is_last_ctb_y)
    {

        /* Left CTB's slice header */
        slice_header_t *ps_slice_hdr_left;
        {
            WORD32 cur_ctb_indx = ps_deblk->i4_ctb_x + ps_deblk->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb;
            if(i4_is_last_ctb_x)
                cur_ctb_indx += 1;
            ps_slice_hdr_left = ps_codec->ps_slice_hdr_base + ps_deblk->pu1_slice_idx[cur_ctb_indx - 1];
        }
        pu1_src = ps_deblk->pu1_cur_pic_luma + ((ps_deblk->i4_ctb_x + ps_deblk->i4_ctb_y * ps_deblk->ps_codec->i4_strd) << log2_ctb_size);
        pu1_src += i4_is_last_ctb_x ? ctb_size : 0;

        /** Deblocking is done on a shifted CTB -
         *  Horizontal edge processing is done by shifting the CTB left by four pixels */
        pu1_src -= 4;
        for(row = 0; row < ctb_size / 8; row++)
        {
            WORD32 shift = 0;

            /* downshift vert_bs by ctb_size/2 for each column
             *  shift = (row & (MAX_CTB_SIZE / ctb_size - 1)) * ctb_size / 2;
             *  which will reduce to the following assuming ctb size is one of 16, 32 and 64
             *  and deblocking is done on 8x8 grid
             */
            if(6 != log2_ctb_size)
                shift = (row & 1) << (log2_ctb_size - 1);

            /* BS for the row - Last column is excluded and the left column is included*/
            u4_bs = (pu4_horz_bs[0] >> shift) << 2;

            if(ps_deblk->i4_ctb_x || i4_is_last_ctb_x)
            {
                /** Picking the last BS of the previous CTB corresponding to the same row
                * UWORD32 *pu4_horz_bs_left = (UWORD32 *)((UWORD8 *)pu4_horz_bs - (ctb_size / 8) * (ctb_size / 4) / 8 * 2);
                */
                UWORD32 *pu4_horz_bs_left = (UWORD32 *)((UWORD8 *)pu4_horz_bs - (1 << (2 * log2_ctb_size - 7)));
                UWORD32 u4_left_bs = (*pu4_horz_bs_left) >> (shift + (1 << (log2_ctb_size - 1)) - 2);
                u4_bs |= u4_left_bs & 3;
            }

            for(col = 0; col < ctb_size / 4;)
            {
                WORD8 i1_beta_offset_div2 = ps_slice_hdr->i1_beta_offset_div2;
                WORD8 i1_tc_offset_div2 = ps_slice_hdr->i1_tc_offset_div2;

                bs_tz = CTZ(u4_bs) >> 1;
                if(0 != bs_tz)
                {
                    u4_bs = u4_bs >> (bs_tz << 1);

                    if((col + bs_tz) >= (ctb_size / 4))
                        pu1_src += 4 * (ctb_size / 4 - col);
                    else
                        pu1_src += 4 * bs_tz;

                    col += bs_tz;
                    continue;
                }

                if(0 == col)
                {
                    i1_beta_offset_div2 = ps_slice_hdr_left->i1_beta_offset_div2;
                    i1_tc_offset_div2 = ps_slice_hdr_left->i1_tc_offset_div2;

                    if(0 == row)
                    {
                        qp_p = u4_qp_const_in_ctb[0] ?
                                        pu1_qp[-ctb_size / 8 * qp_strd - ctb_size / 8] :
                                        pu1_qp[-qp_strd - 1];
                    }
                    else
                    {
                        qp_p = u4_qp_const_in_ctb[2] ?
                                        pu1_qp[-ctb_size / 8] :
                                        pu1_qp[(row - 1) * qp_strd - 1];
                    }

                    qp_q = u4_qp_const_in_ctb[2] ?
                                    pu1_qp[-ctb_size / 8] :
                                    pu1_qp[row * qp_strd - 1];
                }
                else
                {
                    if(0 == row)
                    {
                        qp_p = u4_qp_const_in_ctb[1] ?
                                        pu1_qp[-ctb_size / 8 * qp_strd] :
                                        pu1_qp[((col - 1) >> 1) - qp_strd];
                    }
                    else
                    {
                        qp_p = u4_qp_const_in_ctb[3] ?
                                        pu1_qp[0] :
                                        pu1_qp[((col - 1) >> 1) + (row - 1) * qp_strd];
                    }

                    qp_q = u4_qp_const_in_ctb[3] ?
                                    pu1_qp[0] :
                                    pu1_qp[((col - 1) >> 1) + row * qp_strd];
                }

                filter_p = (pu2_ctb_no_loop_filter_flag[row] >> ((col + 1) >> 1)) & 1;
                filter_q = (pu2_ctb_no_loop_filter_flag[row + 1] >> ((col + 1) >> 1)) & 1;
                /* filter_p and filter_q are inverted as they are calculated using no_loop_filter_flags */
                filter_p = !filter_p;
                filter_q = !filter_q;

                if(filter_p || filter_q)
                {
                    DUMP_DEBLK_LUMA_HORZ(pu1_src, src_strd,
                                         u4_bs & 3, qp_p, qp_q,
                                         ps_slice_hdr->i1_beta_offset_div2,
                                         ps_slice_hdr->i1_tc_offset_div2,
                                         filter_p, filter_q);
                    ps_codec->s_func_selector.ihevc_deblk_luma_horz_fptr(pu1_src, src_strd,
                                                                         u4_bs & 3, qp_p, qp_q,
                                                                         i1_beta_offset_div2,
                                                                         i1_tc_offset_div2, filter_p, filter_q);
                }

                pu1_src += 4;
                u4_bs = u4_bs >> 2;
                col++;
            }

            if((64 == ctb_size) ||
                            ((32 == ctb_size) && (row & 1)))
            {
                pu4_horz_bs++;
            }
            pu1_src -= ctb_size;
            pu1_src += (src_strd << 3);
        }
        pu4_horz_bs = pu4_ctb_horz_bs;
    }


    /* Chroma Veritcal Edge */

    if(0 == i4_is_last_ctb_x)
    {

        /* Top CTB's slice header */
        slice_header_t *ps_slice_hdr_top;
        {
            WORD32 cur_ctb_indx = ps_deblk->i4_ctb_x + ps_deblk->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb;
            if(i4_is_last_ctb_y)
                cur_ctb_indx += ps_sps->i2_pic_wd_in_ctb;
            ps_slice_hdr_top = ps_codec->ps_slice_hdr_base + ps_deblk->pu1_slice_idx[cur_ctb_indx - ps_sps->i2_pic_wd_in_ctb];
        }

        pu1_src = ps_deblk->pu1_cur_pic_chroma + ((ps_deblk->i4_ctb_x + ps_deblk->i4_ctb_y * ps_deblk->ps_codec->i4_strd / 2) << log2_ctb_size);
        pu1_src += i4_is_last_ctb_y ? (ps_deblk->ps_codec->i4_strd / 2) << log2_ctb_size : 0;

        /** Deblocking is done on a shifted CTB -
         *  Vertical edge processing is done by shifting the CTB up by four pixels */
        pu1_src -= 4 * src_strd;

        for(col = 0; col < ctb_size / 16; col++)
        {

            /* BS for the column - Last row is excluded and the top row is included*/
            u4_bs = pu4_vert_bs[0] << 2;

            if(ps_deblk->i4_ctb_y || i4_is_last_ctb_y)
            {
                /* Picking the last BS of the previous CTB corresponding to the same column */
                UWORD32 *pu4_vert_bs_top = (UWORD32 *)((UWORD8 *)pu4_vert_bs - bs_strd);
                UWORD32 u4_top_bs = (*pu4_vert_bs_top) >> ((1 << (log2_ctb_size - 1)) - 2);
                u4_bs |= u4_top_bs & 3;
            }

            /* Every alternate boundary strength value is used for chroma */
            u4_bs &= 0x22222222;

            for(row = 0; row < ctb_size / 8;)
            {
                WORD8 i1_tc_offset_div2 = ps_slice_hdr->i1_tc_offset_div2;

                bs_tz = CTZ(u4_bs) >> 2;
                if(0 != bs_tz)
                {
                    if((row + bs_tz) >= (ctb_size / 8))
                        pu1_src += 4 * (ctb_size / 8 - row) * src_strd;
                    else
                        pu1_src += 4 * bs_tz  * src_strd;
                    row += bs_tz;
                    u4_bs = u4_bs >> (bs_tz << 2);
                    continue;
                }

                if(0 == row)
                {
                    i1_tc_offset_div2 = ps_slice_hdr_top->i1_tc_offset_div2;

                    if(0 == col)
                    {
                        qp_p = u4_qp_const_in_ctb[0] ?
                                        pu1_qp[-ctb_size / 8 * qp_strd - ctb_size / 8] :
                                        pu1_qp[-qp_strd - 1];
                    }
                    else
                    {
                        qp_p = u4_qp_const_in_ctb[1] ?
                                        pu1_qp[-ctb_size / 8 * qp_strd] :
                                        pu1_qp[2 * col - 1 - qp_strd];
                    }

                    qp_q = u4_qp_const_in_ctb[1] ?
                                    pu1_qp[-ctb_size / 8 * qp_strd] :
                                    pu1_qp[2 * col - qp_strd];
                }
                else
                {
                    if(0 == col)
                    {
                        qp_p = u4_qp_const_in_ctb[2] ?
                                        pu1_qp[-ctb_size / 8] :
                                        pu1_qp[(row - 1) * qp_strd - 1];
                    }
                    else
                    {
                        qp_p = u4_qp_const_in_ctb[3] ?
                                        pu1_qp[0] :
                                        pu1_qp[(row - 1) * qp_strd + 2 * col - 1];
                    }

                    qp_q = u4_qp_const_in_ctb[3] ?
                                    pu1_qp[0] :
                                    pu1_qp[(row - 1) * qp_strd + 2 * col];
                }

                filter_p = (pu2_ctb_no_loop_filter_flag[row] >> (col << 1)) & 1;
                filter_q = (pu2_ctb_no_loop_filter_flag[row] >> (col << 1)) & 2;
                /* filter_p and filter_q are inverted as they are calculated using no_loop_filter_flags */
                filter_p = !filter_p;
                filter_q = !filter_q;

                if(filter_p || filter_q)
                {
                    ASSERT(1 == ((u4_bs & 3) >> 1));
                    DUMP_DEBLK_CHROMA_VERT(pu1_src, src_strd,
                                           u4_bs & 3, qp_p, qp_q,
                                           ps_pps->i1_pic_cb_qp_offset,
                                           ps_pps->i1_pic_cr_qp_offset,
                                           ps_slice_hdr->i1_tc_offset_div2,
                                           filter_p, filter_q);
                    if(chroma_yuv420sp_vu)
                    {
                        ps_codec->s_func_selector.ihevc_deblk_chroma_vert_fptr(pu1_src,
                                                                               src_strd,
                                                                               qp_q,
                                                                               qp_p,
                                                                               ps_pps->i1_pic_cr_qp_offset,
                                                                               ps_pps->i1_pic_cb_qp_offset,
                                                                               i1_tc_offset_div2,
                                                                               filter_q,
                                                                               filter_p);
                    }
                    else
                    {
                        ps_codec->s_func_selector.ihevc_deblk_chroma_vert_fptr(pu1_src,
                                                                               src_strd,
                                                                               qp_p,
                                                                               qp_q,
                                                                               ps_pps->i1_pic_cb_qp_offset,
                                                                               ps_pps->i1_pic_cr_qp_offset,
                                                                               i1_tc_offset_div2,
                                                                               filter_p,
                                                                               filter_q);
                    }
                }

                pu1_src += 4 * src_strd;
                u4_bs = u4_bs >> 4;
                row++;
            }

            pu4_vert_bs += (64 == ctb_size) ? 2 : 1;
            pu1_src -= ((src_strd / 2) << log2_ctb_size);
            pu1_src += 16;
        }
    }

    /* Chroma Horizontal Edge */

    if(0 == i4_is_last_ctb_y)
    {

        /* Left CTB's slice header */
        slice_header_t *ps_slice_hdr_left;
        {
            WORD32 cur_ctb_indx = ps_deblk->i4_ctb_x + ps_deblk->i4_ctb_y * ps_sps->i2_pic_wd_in_ctb;
            if(i4_is_last_ctb_x)
                cur_ctb_indx += 1;
            ps_slice_hdr_left = ps_codec->ps_slice_hdr_base + ps_deblk->pu1_slice_idx[cur_ctb_indx - 1];
        }

        pu1_src = ps_deblk->pu1_cur_pic_chroma + ((ps_deblk->i4_ctb_x + ps_deblk->i4_ctb_y * ps_deblk->ps_codec->i4_strd / 2) << log2_ctb_size);
        pu1_src += i4_is_last_ctb_x ? ctb_size : 0;

        /** Deblocking is done on a shifted CTB -
         * Vertical edge processing is done by shifting the CTB up by four pixels (8 here beacuse UV are interleaved) */
        pu1_src -= 8;
        for(row = 0; row < ctb_size / 16; row++)
        {
            /* BS for the row - Last column is excluded and the left column is included*/
            u4_bs = pu4_horz_bs[0] << 2;

            if(ps_deblk->i4_ctb_x || i4_is_last_ctb_x)
            {
                /** Picking the last BS of the previous CTB corresponding to the same row
                * UWORD32 *pu4_horz_bs_left = (UWORD32 *)((UWORD8 *)pu4_horz_bs - (ctb_size / 8) * (ctb_size / 4) / 8 * 2);
                */
                UWORD32 *pu4_horz_bs_left = (UWORD32 *)((UWORD8 *)pu4_horz_bs - (1 << (2 * log2_ctb_size - 7)));
                UWORD32 u4_left_bs = (*pu4_horz_bs_left) >> ((1 << (log2_ctb_size - 1)) - 2);
                u4_bs |= u4_left_bs & 3;
            }

            /* Every alternate boundary strength value is used for chroma */
            u4_bs &= 0x22222222;

            for(col = 0; col < ctb_size / 8;)
            {
                WORD8 i1_tc_offset_div2 = ps_slice_hdr->i1_tc_offset_div2;

                bs_tz = CTZ(u4_bs) >> 2;
                if(0 != bs_tz)
                {
                    u4_bs = u4_bs >> (bs_tz << 2);

                    if((col + bs_tz) >= (ctb_size / 8))
                        pu1_src += 8 * (ctb_size / 8 - col);
                    else
                        pu1_src += 8 * bs_tz;

                    col += bs_tz;
                    continue;
                }

                if(0 == col)
                {
                    i1_tc_offset_div2 = ps_slice_hdr_left->i1_tc_offset_div2;

                    if(0 == row)
                    {
                        qp_p = u4_qp_const_in_ctb[0] ?
                                        pu1_qp[-ctb_size / 8 * qp_strd - ctb_size / 8] :
                                        pu1_qp[-qp_strd - 1];
                    }
                    else
                    {
                        qp_p = u4_qp_const_in_ctb[2] ?
                                        pu1_qp[-ctb_size / 8] :
                                        pu1_qp[(2 * row - 1) * qp_strd - 1];
                    }

                    qp_q = u4_qp_const_in_ctb[2] ?
                                    pu1_qp[-ctb_size / 8] :
                                    pu1_qp[(2 * row) * qp_strd - 1];
                }
                else
                {
                    if(0 == row)
                    {
                        qp_p = u4_qp_const_in_ctb[1] ?
                                        pu1_qp[-ctb_size / 8 * qp_strd] :
                                        pu1_qp[col - 1 - qp_strd];
                    }
                    else
                    {
                        qp_p = u4_qp_const_in_ctb[3] ?
                                        pu1_qp[0] :
                                        pu1_qp[(col - 1) +  (2 * row - 1) * qp_strd];
                    }

                    qp_q = u4_qp_const_in_ctb[3] ?
                                    pu1_qp[0] :
                                    pu1_qp[(col - 1) + 2 * row * qp_strd];
                }

                filter_p = (pu2_ctb_no_loop_filter_flag[row << 1] >> col) & 1;
                filter_q = (pu2_ctb_no_loop_filter_flag[(row << 1) + 1] >> col) & 1;
                /* filter_p and filter_q are inverted as they are calculated using no_loop_filter_flags */
                filter_p = !filter_p;
                filter_q = !filter_q;

                if(filter_p || filter_q)
                {
                    ASSERT(1 == ((u4_bs & 3) >> 1));
                    DUMP_DEBLK_CHROMA_HORZ(pu1_src, src_strd,
                                           u4_bs & 3, qp_p, qp_q,
                                           ps_pps->i1_pic_cb_qp_offset,
                                           ps_pps->i1_pic_cr_qp_offset,
                                           ps_slice_hdr->i1_tc_offset_div2,
                                           filter_p, filter_q);
                    if(chroma_yuv420sp_vu)
                    {
                        ps_codec->s_func_selector.ihevc_deblk_chroma_horz_fptr(pu1_src,
                                                                               src_strd,
                                                                               qp_q,
                                                                               qp_p,
                                                                               ps_pps->i1_pic_cr_qp_offset,
                                                                               ps_pps->i1_pic_cb_qp_offset,
                                                                               i1_tc_offset_div2,
                                                                               filter_q,
                                                                               filter_p);
                    }
                    else
                    {
                        ps_codec->s_func_selector.ihevc_deblk_chroma_horz_fptr(pu1_src,
                                                                               src_strd,
                                                                               qp_p,
                                                                               qp_q,
                                                                               ps_pps->i1_pic_cb_qp_offset,
                                                                               ps_pps->i1_pic_cr_qp_offset,
                                                                               i1_tc_offset_div2,
                                                                               filter_p,
                                                                               filter_q);
                    }
                }

                pu1_src += 8;
                u4_bs = u4_bs >> 4;
                col++;
            }

            pu4_horz_bs += (64 == ctb_size) ? 2 : 1;
            pu1_src -= ctb_size;
            pu1_src += 8 * src_strd;

        }
    }
}
