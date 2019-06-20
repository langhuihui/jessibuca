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
 *  ihevcd_iquant_itrans_recon_ctb.c
 *
 * @brief
 *  Contains functions for inverse quantization, inverse transform and recon
 *
 * @author
 *  Ittiam
 *
 * @par List of Functions:
 * - ihevcd_iquant_itrans_recon_ctb()
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

#include "ihevc_defs.h"
#include "ihevc_debug.h"
#include "ihevc_structs.h"
#include "ihevc_cabac_tables.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"

#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_bitstream.h"
#include "ihevc_common_tables.h"

/* Intra pred includes */
#include "ihevc_intra_pred.h"

/* Inverse transform common module includes */
#include "ihevc_trans_tables.h"
#include "ihevc_trans_macros.h"
#include "ihevc_itrans_recon.h"
#include "ihevc_recon.h"
#include "ihevc_chroma_itrans_recon.h"
#include "ihevc_chroma_recon.h"

/* Decoder includes */
#include "ihevcd_common_tables.h"
#include "ihevcd_iquant_itrans_recon_ctb.h"
#include "ihevcd_debug.h"
#include "ihevcd_profile.h"
#include "ihevcd_statistics.h"
#include "ihevcd_itrans_recon_dc.h"

static const UWORD32 gau4_ihevcd_4_bit_reverse[] = { 0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15 };


/* Globals */
static const WORD32 g_i4_ip_funcs[MAX_NUM_IP_MODES] =
  { IP_FUNC_MODE_0, /* Mode 0 */
    IP_FUNC_MODE_1, /* Mode 1 */
    IP_FUNC_MODE_2, /* Mode 2 */
    IP_FUNC_MODE_3TO9, /* Mode 3 */
    IP_FUNC_MODE_3TO9, /* Mode 4 */
    IP_FUNC_MODE_3TO9, /* Mode 5 */
    IP_FUNC_MODE_3TO9, /* Mode 6 */
    IP_FUNC_MODE_3TO9, /* Mode 7 */
    IP_FUNC_MODE_3TO9, /* Mode 8 */
    IP_FUNC_MODE_3TO9, /* Mode 9 */
    IP_FUNC_MODE_10, /* Mode 10 */
    IP_FUNC_MODE_11TO17, /* Mode 11 */
    IP_FUNC_MODE_11TO17, /* Mode 12 */
    IP_FUNC_MODE_11TO17, /* Mode 13 */
    IP_FUNC_MODE_11TO17, /* Mode 14 */
    IP_FUNC_MODE_11TO17, /* Mode 15 */
    IP_FUNC_MODE_11TO17, /* Mode 16 */
    IP_FUNC_MODE_11TO17, /* Mode 17 */
    IP_FUNC_MODE_18_34, /* Mode 18 */
    IP_FUNC_MODE_19TO25, /* Mode 19 */
    IP_FUNC_MODE_19TO25, /* Mode 20 */
    IP_FUNC_MODE_19TO25, /* Mode 21 */
    IP_FUNC_MODE_19TO25, /* Mode 22 */
    IP_FUNC_MODE_19TO25, /* Mode 23 */
    IP_FUNC_MODE_19TO25, /* Mode 24 */
    IP_FUNC_MODE_19TO25, /* Mode 25 */
    IP_FUNC_MODE_26, /* Mode 26 */
    IP_FUNC_MODE_27TO33, /* Mode 27 */
    IP_FUNC_MODE_27TO33, /* Mode 26 */
    IP_FUNC_MODE_27TO33, /* Mode 29 */
    IP_FUNC_MODE_27TO33, /* Mode 30 */
    IP_FUNC_MODE_27TO33, /* Mode 31 */
    IP_FUNC_MODE_27TO33, /* Mode 32 */
    IP_FUNC_MODE_27TO33, /* Mode 33 */
    IP_FUNC_MODE_18_34, /* Mode 34 */
};


const WORD16 *g_ai2_ihevc_trans_tables[] =
  { &g_ai2_ihevc_trans_dst_4[0][0],
    &g_ai2_ihevc_trans_4[0][0],
    &g_ai2_ihevc_trans_8[0][0],
    &g_ai2_ihevc_trans_16[0][0],
    &g_ai2_ihevc_trans_32[0][0]
};


/*****************************************************************************/
/* Function Prototypes                                                       */
/*****************************************************************************/
/* Returns number of ai2_level read from ps_sblk_coeff */
UWORD8* ihevcd_unpack_coeffs(WORD16 *pi2_tu_coeff,
                             WORD32 log2_trans_size,
                             UWORD8 *pu1_tu_coeff_data,
                             WORD16 *pi2_dequant_matrix,
                             WORD32 qp_rem,
                             WORD32 qp_div,
                             TRANSFORM_TYPE e_trans_type,
                             WORD32 trans_quant_bypass,
                             UWORD32 *pu4_zero_cols,
                             UWORD32 *pu4_zero_rows,
                             UWORD32 *pu4_coeff_type,
                             WORD16 *pi2_coeff_value)
{
    /* Generating coeffs from coeff-map */
    WORD32 i;
    WORD16 *pi2_sblk_ptr;
    WORD32 subblk_pos_x, subblk_pos_y;
    WORD32 sblk_scan_idx, coeff_raster_idx;
    WORD32 sblk_non_zero_coeff_idx;
    tu_sblk_coeff_data_t *ps_tu_sblk_coeff_data;
    UWORD8 u1_num_coded_sblks, u1_scan_type;
    UWORD8 *pu1_new_tu_coeff_data;
    WORD32 trans_size;
    WORD32 xs, ys;
    WORD32 trans_skip;
    WORD16 iquant_out;
    WORD32 shift_iq;
    {
        WORD32 bit_depth;

        bit_depth = 8 + 0;
        shift_iq = bit_depth + log2_trans_size - 5;
    }
    trans_size = (1 << log2_trans_size);

    /* First byte points to number of coded blocks */
    u1_num_coded_sblks = *pu1_tu_coeff_data++;

    /* Next byte points to scan type */
    u1_scan_type = *pu1_tu_coeff_data++;
    /* 0th bit has trans_skip */
    trans_skip = u1_scan_type & 1;
    u1_scan_type >>= 1;

    pi2_sblk_ptr = pi2_tu_coeff;

    /* Initially all columns are assumed to be zero */
    *pu4_zero_cols = 0xFFFFFFFF;
    /* Initially all rows are assumed to be zero */
    *pu4_zero_rows = 0xFFFFFFFF;

    ps_tu_sblk_coeff_data = (tu_sblk_coeff_data_t *)(pu1_tu_coeff_data);

    if(trans_skip)
        memset(pi2_tu_coeff, 0, trans_size * trans_size * sizeof(WORD16));

    STATS_INIT_SBLK_AND_COEFF_POS();

    /* DC only case */
    if((e_trans_type != DST_4x4) && (1 == u1_num_coded_sblks)
                    && (0 == ps_tu_sblk_coeff_data->u2_subblk_pos)
                    && (1 == ps_tu_sblk_coeff_data->u2_sig_coeff_map))
    {
        *pu4_coeff_type = 1;

        if(!trans_quant_bypass)
        {
            if(4 == trans_size)
            {
                IQUANT_4x4(iquant_out,
                           ps_tu_sblk_coeff_data->ai2_level[0],
                           pi2_dequant_matrix[0]
                                           * g_ihevc_iquant_scales[qp_rem],
                           shift_iq, qp_div);
            }
            else
            {
                IQUANT(iquant_out, ps_tu_sblk_coeff_data->ai2_level[0],
                       pi2_dequant_matrix[0] * g_ihevc_iquant_scales[qp_rem],
                       shift_iq, qp_div);
            }
            if(trans_skip)
                iquant_out = (iquant_out + 16) >> 5;
        }
        else
        {
            /* setting the column to zero */
            for(i = 0; i < trans_size; i++)
                *(pi2_tu_coeff + i * trans_size) = 0;

            iquant_out = ps_tu_sblk_coeff_data->ai2_level[0];
        }
        *pi2_coeff_value = iquant_out;
        *pi2_tu_coeff = iquant_out;
        *pu4_zero_cols &= ~0x1;
        *pu4_zero_rows &= ~0x1;
        ps_tu_sblk_coeff_data =
                        (void *)&ps_tu_sblk_coeff_data->ai2_level[1];

        STATS_UPDATE_COEFF_COUNT();
        STATS_LAST_SBLK_POS_UPDATE(e_trans_type, (trans_skip || trans_quant_bypass),  0, 0);
        STATS_UPDATE_SBLK_AND_COEFF_HISTOGRAM(e_trans_type, (trans_quant_bypass || trans_skip));
        return ((UWORD8 *)ps_tu_sblk_coeff_data);
    }
    else
    {
        *pu4_coeff_type = 0;
        /* In case of trans skip, memset has already happened */
        if(!trans_skip)
            memset(pi2_tu_coeff, 0, trans_size * trans_size * sizeof(WORD16));
    }

    for(i = 0; i < u1_num_coded_sblks; i++)
    {
        UWORD32 u4_sig_coeff_map;
        subblk_pos_x = ps_tu_sblk_coeff_data->u2_subblk_pos & 0x00FF;
        subblk_pos_y = (ps_tu_sblk_coeff_data->u2_subblk_pos & 0xFF00) >> 8;

        STATS_LAST_SBLK_POS_UPDATE(e_trans_type, (trans_skip || trans_quant_bypass), subblk_pos_x, subblk_pos_y);

        subblk_pos_x = subblk_pos_x * MIN_TU_SIZE;
        subblk_pos_y = subblk_pos_y * MIN_TU_SIZE;

        pi2_sblk_ptr = pi2_tu_coeff + subblk_pos_y * trans_size
                        + subblk_pos_x;

        //*pu4_zero_cols &= ~(0xF << subblk_pos_x);

        sblk_non_zero_coeff_idx = 0;
        u4_sig_coeff_map = ps_tu_sblk_coeff_data->u2_sig_coeff_map;
        //for(sblk_scan_idx = (31 - CLZ(u4_sig_coeff_map)); sblk_scan_idx >= 0; sblk_scan_idx--)
        sblk_scan_idx = 31;
        do
        {
            WORD32 clz = CLZ(u4_sig_coeff_map);

            sblk_scan_idx -= clz;
            /* when clz is 31, u4_sig_coeff_map << (clz+1) might result in unknown behaviour in some cases */
            /* Hence either use SHL which takes care of handling these issues based on platform or shift in two stages */
            u4_sig_coeff_map = u4_sig_coeff_map << clz;
            /* Copying coeffs and storing in reverse order */
            {
                STATS_UPDATE_COEFF_COUNT();
                coeff_raster_idx =
                                gau1_ihevc_invscan4x4[u1_scan_type][sblk_scan_idx];

                xs = coeff_raster_idx & 0x3;
                ys = coeff_raster_idx >> 2;

                if(!trans_quant_bypass)
                {
                    if(4 == trans_size)
                    {
                        IQUANT_4x4(iquant_out,
                                   ps_tu_sblk_coeff_data->ai2_level[sblk_non_zero_coeff_idx],
                                   pi2_dequant_matrix[(subblk_pos_x + xs)
                                                   + (subblk_pos_y + ys)
                                                   * trans_size]
                                   * g_ihevc_iquant_scales[qp_rem],
                                   shift_iq, qp_div);
                        sblk_non_zero_coeff_idx++;
                    }
                    else
                    {
                        IQUANT(iquant_out,
                               ps_tu_sblk_coeff_data->ai2_level[sblk_non_zero_coeff_idx],
                               pi2_dequant_matrix[(subblk_pos_x + xs)
                                               + (subblk_pos_y + ys)
                                               * trans_size]
                               * g_ihevc_iquant_scales[qp_rem],
                               shift_iq, qp_div);
                        sblk_non_zero_coeff_idx++;
                    }

                    if(trans_skip)
                        iquant_out = (iquant_out + 16) >> 5;
                }
                else
                {
                    iquant_out = ps_tu_sblk_coeff_data->ai2_level[sblk_non_zero_coeff_idx++];
                }
                *pu4_zero_cols &= ~(0x1 << (subblk_pos_x + xs));
                *pu4_zero_rows &= ~(0x1 << (subblk_pos_y + ys));
                *(pi2_sblk_ptr + xs + ys * trans_size) = iquant_out;
            }
            sblk_scan_idx--;
            u4_sig_coeff_map <<= 1;

        }while(u4_sig_coeff_map);
        /* Updating the sblk pointer */
        ps_tu_sblk_coeff_data =
                        (void *)&ps_tu_sblk_coeff_data->ai2_level[sblk_non_zero_coeff_idx];
    }

    STATS_UPDATE_SBLK_AND_COEFF_HISTOGRAM(e_trans_type, (trans_quant_bypass || trans_skip));

    pu1_new_tu_coeff_data = (UWORD8 *)ps_tu_sblk_coeff_data;

    return pu1_new_tu_coeff_data;
}

WORD32 ihevcd_get_intra_nbr_flag(process_ctxt_t *ps_proc,
                                 tu_t *ps_tu,
                                 UWORD32 *pu4_intra_nbr_avail,
                                 WORD16 i2_pic_width_in_luma_samples,
                                 UWORD8 i1_constrained_intra_pred_flag,
                                 WORD32 trans_size,
                                 WORD32 ctb_size)
{
    sps_t *ps_sps;
    UWORD8 u1_bot_lt_avail, u1_left_avail, u1_top_avail, u1_top_rt_avail,
                    u1_top_lt_avail;
    WORD32 x_cur, y_cur, x_nbr, y_nbr;
    UWORD8 *pu1_nbr_intra_flag;
    UWORD8 *pu1_pic_intra_flag;
    UWORD8 top_right, top, top_left, left, bot_left;
    WORD32 intra_pos;
    WORD32 num_8_blks, num_8_blks_in_bits;
    WORD32 numbytes_row = (i2_pic_width_in_luma_samples + 63) / 64;
    WORD32 cur_x, cur_y;
    WORD32 i;
    WORD32 nbr_flags;

    ps_sps = ps_proc->ps_sps;
    cur_x = ps_tu->b4_pos_x;
    cur_y = ps_tu->b4_pos_y;

    u1_bot_lt_avail = (pu4_intra_nbr_avail[1 + cur_y + trans_size / MIN_TU_SIZE]
                    >> (31 - (1 + cur_x - 1))) & 1;
    u1_left_avail = (pu4_intra_nbr_avail[1 + cur_y] >> (31 - (1 + cur_x - 1)))
                    & 1;
    u1_top_avail = (pu4_intra_nbr_avail[1 + cur_y - 1] >> (31 - (1 + cur_x)))
                    & 1;
    u1_top_rt_avail = (pu4_intra_nbr_avail[1 + cur_y - 1]
                    >> (31 - (1 + cur_x + trans_size / MIN_TU_SIZE))) & 1;
    u1_top_lt_avail = (pu4_intra_nbr_avail[1 + cur_y - 1]
                    >> (31 - (1 + cur_x - 1))) & 1;

    x_cur = ps_proc->i4_ctb_x * ctb_size + cur_x * MIN_TU_SIZE;
    y_cur = ps_proc->i4_ctb_y * ctb_size + cur_y * MIN_TU_SIZE;

    pu1_pic_intra_flag = ps_proc->pu1_pic_intra_flag;

    /* WORD32 nbr_flags as below  MSB --> LSB */
    /*    Top-Left | Top-Right | Top | Left | Bottom-Left
     *       1         4         4     4         4
     */
    bot_left = 0;
    left = 0;
    top_right = 0;
    top = 0;
    top_left = 0;

    num_8_blks = trans_size > 4 ? trans_size / 8 : 1;
    num_8_blks_in_bits = ((1 << num_8_blks) - 1);

    if(i1_constrained_intra_pred_flag)
    {
        /* TODO: constrained intra pred not tested */
        if(u1_bot_lt_avail)
        {
            x_nbr = x_cur - 1;
            y_nbr = y_cur + trans_size;

            pu1_nbr_intra_flag = pu1_pic_intra_flag + y_nbr / 8 * numbytes_row
                            + x_nbr / 64;
            intra_pos = ((x_nbr / 8) % 8);
            for(i = 0; i < num_8_blks; i++)
            {
                bot_left |= ((*(pu1_nbr_intra_flag + i * numbytes_row)
                                >> intra_pos) & 1) << i;
            }
            bot_left &= num_8_blks_in_bits;
        }
        if(u1_left_avail)
        {
            x_nbr = x_cur - 1;
            y_nbr = y_cur;

            pu1_nbr_intra_flag = pu1_pic_intra_flag + y_nbr / 8 * numbytes_row
                            + x_nbr / 64;
            intra_pos = ((x_nbr / 8) % 8);

            for(i = 0; i < num_8_blks; i++)
            {
                left |= ((*(pu1_nbr_intra_flag + i * numbytes_row) >> intra_pos)
                                & 1) << i;
            }
            left &= num_8_blks_in_bits;
        }
        if(u1_top_avail)
        {
            x_nbr = x_cur;
            y_nbr = y_cur - 1;

            pu1_nbr_intra_flag = pu1_pic_intra_flag + y_nbr / 8 * numbytes_row
                            + x_nbr / 64;
            intra_pos = ((x_nbr / 8) % 8);

            top = (*pu1_nbr_intra_flag >> intra_pos);
            top &= num_8_blks_in_bits;
            /*
             for(i=0;i<num_8_blks;i++)
             {
             top |= ( (*pu1_nbr_intra_flag >> (intra_pos+i)) & 1) << i;
             }
             */
        }
        if(u1_top_rt_avail)
        {
            x_nbr = x_cur + trans_size;
            y_nbr = y_cur - 1;

            pu1_nbr_intra_flag = pu1_pic_intra_flag + y_nbr / 8 * numbytes_row
                            + x_nbr / 64;
            intra_pos = ((x_nbr / 8) % 8);

            top_right = (*pu1_nbr_intra_flag >> intra_pos);
            top_right &= num_8_blks_in_bits;
            /*
             for(i=0;i<num_8_blks;i++)
             {
             top_right |= ( (*pu1_nbr_intra_flag >> (intra_pos+i)) & 1) << i;
             }
             */
        }
        if(u1_top_lt_avail)
        {
            x_nbr = x_cur - 1;
            y_nbr = y_cur - 1;

            pu1_nbr_intra_flag = pu1_pic_intra_flag + y_nbr / 8 * numbytes_row
                            + x_nbr / 64;
            intra_pos = ((x_nbr / 8) % 8);

            top_left = (*pu1_nbr_intra_flag >> intra_pos) & 1;
        }
    }
    else
    {
        if(u1_top_avail)
            top = 0xF;
        if(u1_top_rt_avail)
            top_right = 0xF;
        if(u1_bot_lt_avail)
            bot_left = 0xF;
        if(u1_left_avail)
            left = 0xF;
        if(u1_top_lt_avail)
            top_left = 0x1;
    }

    /* Handling incomplete CTBs */
    {
        WORD32 pu_size_limit = MIN(trans_size, 8);
        WORD32 cols_remaining = ps_sps->i2_pic_width_in_luma_samples
                        - (ps_proc->i4_ctb_x << ps_sps->i1_log2_ctb_size)
                        - (ps_tu->b4_pos_x * MIN_TU_SIZE)
                        - (1 << (ps_tu->b3_size + 2));
        /* ctb_size_top gives number of valid pixels remaining in the current row */
        WORD32 ctb_size_top = MIN(ctb_size, cols_remaining);
        WORD32 ctb_size_top_bits = (1 << (ctb_size_top / pu_size_limit)) - 1;

        WORD32 rows_remaining = ps_sps->i2_pic_height_in_luma_samples
                        - (ps_proc->i4_ctb_y << ps_sps->i1_log2_ctb_size)
                        - (ps_tu->b4_pos_y * MIN_TU_SIZE)
                        - (1 << (ps_tu->b3_size + 2));
        /* ctb_size_bot gives number of valid pixels remaining in the current column */
        WORD32 ctb_size_bot = MIN(ctb_size, rows_remaining);
        WORD32 ctb_size_bot_bits = (1 << (ctb_size_bot / pu_size_limit)) - 1;

        top_right &= ctb_size_top_bits;
        bot_left &= ctb_size_bot_bits;
    }

    /*    Top-Left | Top-Right | Top | Left | Bottom-Left
     *      1         4         4     4         4
     */

    /*
     nbr_flags = (top_left << 16) | (gau4_ihevcd_4_bit_reverse[top_right] << 12) | (gau4_ihevcd_4_bit_reverse[top] << 8) | (gau4_ihevcd_4_bit_reverse[left] << 4)
     | gau4_ihevcd_4_bit_reverse[bot_left];
     */
    nbr_flags = (top_left << 16) | (top_right << 12) | (top << 8) | (gau4_ihevcd_4_bit_reverse[left] << 4)
                    | gau4_ihevcd_4_bit_reverse[bot_left];


    return nbr_flags;

}

WORD32 ihevcd_iquant_itrans_recon_ctb(process_ctxt_t *ps_proc)
{
    WORD16 *pi2_scaling_mat;
    UWORD8 *pu1_y_dst_ctb;
    UWORD8 *pu1_uv_dst_ctb;
    WORD32 ctb_size;
    codec_t *ps_codec;
    slice_header_t *ps_slice_hdr;
    tu_t *ps_tu;
    WORD16 *pi2_ctb_coeff;
    WORD32 tu_cnt;
    WORD16 *pi2_tu_coeff;
    WORD16 *pi2_tmp;
    WORD32 pic_strd;
    WORD32 luma_nbr_flags;
    WORD32 chroma_nbr_flags = 0;
    UWORD8 u1_luma_pred_mode_first_tu = 0;
    /* Pointers for generating 2d coeffs from coeff-map */
    UWORD8 *pu1_tu_coeff_data;
    /* nbr avail map for CTB */
    /* 1st bit points to neighbor (left/top_left/bot_left) */
    /* 1Tb starts at 2nd bit from msb of 2nd value in array, followed by number of min_tu's in that ctb */
    UWORD32 au4_intra_nbr_avail[MAX_CTB_SIZE / MIN_TU_SIZE
                    + 2 /* Top nbr + bot nbr */]; UWORD32
                    top_avail_bits;
    sps_t *ps_sps;
    pps_t *ps_pps;
    WORD32 intra_flag;
    UWORD8 *pu1_pic_intra_flag;
    /*************************************************************************/
    /* Contanis scaling matrix offset in the following order in a 1D buffer  */
    /* Intra 4 x 4 Y, 4 x 4 U, 4 x 4 V                                       */
    /* Inter 4 x 4 Y, 4 x 4 U, 4 x 4 V                                       */
    /* Intra 8 x 8 Y, 8 x 8 U, 8 x 8 V                                       */
    /* Inter 8 x 8 Y, 8 x 8 U, 8 x 8 V                                       */
    /* Intra 16x16 Y, 16x16 U, 16x16 V                                       */
    /* Inter 16x16 Y, 16x16 U, 16x16 V                                       */
    /* Intra 32x32 Y                                                         */
    /* Inter 32x32 Y                                                         */
    /*************************************************************************/
    /* Only first 20 entries are used. Array is extended to avoid out of bound
       reads. Skip CUs (64x64) read this table, but don't really use the value */
    static const WORD32 scaling_mat_offset[] =
      { 0, 16, 32, 48, 64, 80, 96, 160, 224, 288, 352, 416, 480, 736, 992,
        1248, 1504, 1760, 2016, 3040, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    PROFILE_DISABLE_IQ_IT_RECON_INTRA_PRED();

    ps_sps = ps_proc->ps_sps;
    ps_pps = ps_proc->ps_pps;
    ps_slice_hdr = ps_proc->ps_slice_hdr;
    ps_codec = ps_proc->ps_codec;

    pu1_y_dst_ctb = ps_proc->pu1_cur_ctb_luma;
    pu1_uv_dst_ctb = ps_proc->pu1_cur_ctb_chroma;

    pi2_ctb_coeff = ps_proc->pi2_invscan_out;

    ctb_size = (1 << ps_sps->i1_log2_ctb_size);
    pu1_tu_coeff_data = (UWORD8 *)ps_proc->pv_tu_coeff_data;

    pic_strd = ps_codec->i4_strd;

    pi2_tmp = ps_proc->pi2_itrans_intrmd_buf;

    pi2_tu_coeff = pi2_ctb_coeff;

    ps_tu = ps_proc->ps_tu;

    if((1 == ps_sps->i1_scaling_list_enable_flag) && (1 == ps_pps->i1_pps_scaling_list_data_present_flag))
    {
        pi2_scaling_mat = ps_pps->pi2_scaling_mat;
    }
    else
    {
        pi2_scaling_mat = ps_sps->pi2_scaling_mat;
    }

    {
        /* Updating the initial availability map */
        WORD32 i;
        UWORD8 u1_left_ctb_avail, u1_top_lt_ctb_avail, u1_top_rt_ctb_avail,
                        u1_top_ctb_avail;

        u1_left_ctb_avail = ps_proc->u1_left_ctb_avail;
        u1_top_lt_ctb_avail = ps_proc->u1_top_lt_ctb_avail;
        u1_top_ctb_avail = ps_proc->u1_top_ctb_avail;
        u1_top_rt_ctb_avail = ps_proc->u1_top_rt_ctb_avail;

        /* Initializing the availability array */
        memset(au4_intra_nbr_avail, 0,
               (MAX_CTB_SIZE / MIN_TU_SIZE + 2) * sizeof(UWORD32));
        /* Initializing the availability array with CTB level availability flags */
        {
            WORD32 rows_remaining = ps_sps->i2_pic_height_in_luma_samples - (ps_proc->i4_ctb_y << ps_sps->i1_log2_ctb_size);
            WORD32 ctb_size_left = MIN(ctb_size, rows_remaining);
            for(i = 0; i < ctb_size_left / MIN_TU_SIZE; i++)
            {
                au4_intra_nbr_avail[i + 1] = ((UWORD32)u1_left_ctb_avail << 31);
            }
        }
        au4_intra_nbr_avail[0] |= (((UWORD32)u1_top_rt_ctb_avail << 31)
                        >> (1 + ctb_size / MIN_TU_SIZE)); /* 1+ctb_size/4 position bit pos from msb */

        au4_intra_nbr_avail[0] |= ((UWORD32)u1_top_lt_ctb_avail << 31);

        {
            WORD32 cols_remaining = ps_sps->i2_pic_width_in_luma_samples - (ps_proc->i4_ctb_x << ps_sps->i1_log2_ctb_size);
            WORD32 ctb_size_top = MIN(ctb_size, cols_remaining);
            WORD32 shift = (31 - (ctb_size / MIN_TU_SIZE));

            /* ctb_size_top gives number of valid pixels remaining in the current row */
            /* Since we need pattern of 1's starting from the MSB, an additional shift */
            /* is needed */
            shift += ((ctb_size - ctb_size_top) / MIN_TU_SIZE);

            top_avail_bits = ((1 << (ctb_size_top / MIN_TU_SIZE)) - 1)
                            << shift;
        }
        au4_intra_nbr_avail[0] |= (
                        (u1_top_ctb_avail == 1) ? top_avail_bits : 0x0);
        /* Starting from msb 2nd bit to (1+ctb_size/4) bit, set 1 if top avail,or 0 */

    }

    /* Applying Inverse transform on all the TU's in CTB */
    for(tu_cnt = 0; tu_cnt < ps_proc->i4_ctb_tu_cnt; tu_cnt++, ps_tu++)
    {
        WORD32 transform_skip_flag = 0;
        WORD32 transform_skip_flag_v = 0;
        WORD32 num_comp, c_idx, func_idx;
        WORD32 src_strd, pred_strd, dst_strd;
        WORD32 qp_div = 0, qp_rem = 0;
        WORD32 qp_div_v = 0, qp_rem_v = 0;
        UWORD32 zero_cols = 0, zero_cols_v = 0;
        UWORD32 zero_rows = 0, zero_rows_v = 0;
        UWORD32 coeff_type = 0, coeff_type_v = 0;
        WORD16 i2_coeff_value, i2_coeff_value_v;
        WORD32 trans_size = 0;
        TRANSFORM_TYPE e_trans_type;
        WORD32 log2_y_trans_size_minus_2, log2_uv_trans_size_minus_2;
        WORD32 log2_trans_size;
        WORD32 chroma_qp_idx;
        WORD16 *pi2_src = NULL, *pi2_src_v = NULL;
        UWORD8 *pu1_pred = NULL, *pu1_pred_v = NULL;
        UWORD8 *pu1_dst = NULL, *pu1_dst_v = NULL;
        WORD16 *pi2_dequant_matrix = NULL, *pi2_dequant_matrix_v = NULL;
        WORD32 tu_x, tu_y;
        WORD32 tu_y_offset, tu_uv_offset;
        WORD8 i1_chroma_pic_qp_offset, i1_chroma_slice_qp_offset;
        UWORD8 u1_cbf = 0, u1_cbf_v = 0, u1_luma_pred_mode, u1_chroma_pred_mode;
        WORD32 luma_nbr_flags_4x4[4];
        WORD32 offset;
        WORD32 pcm_flag;
        WORD32  chroma_yuv420sp_vu = (ps_codec->e_ref_chroma_fmt == IV_YUV_420SP_VU);
        /* If 420SP_VU is chroma format, pred and dst pointer   */
        /* will be added +1 to point to U                       */
        WORD32 chroma_yuv420sp_vu_u_offset = 1 * chroma_yuv420sp_vu;
        /* If 420SP_VU is chroma format, pred and dst pointer   */
        /* will be added U offset of +1 and subtracted 2        */
        /* to point to V                                        */
        WORD32 chroma_yuv420sp_vu_v_offset = -2 * chroma_yuv420sp_vu;

        tu_x = ps_tu->b4_pos_x * 4; /* Converting minTU unit to pixel unit */
        tu_y = ps_tu->b4_pos_y * 4; /* Converting minTU unit to pixel unit */
        {
            WORD32 tu_abs_x = (ps_proc->i4_ctb_x << ps_sps->i1_log2_ctb_size) + (tu_x);
            WORD32 tu_abs_y = (ps_proc->i4_ctb_y << ps_sps->i1_log2_ctb_size) + (tu_y);

            WORD32 numbytes_row =  (ps_sps->i2_pic_width_in_luma_samples + 63) / 64;

            pu1_pic_intra_flag = ps_proc->pu1_pic_intra_flag;
            pu1_pic_intra_flag += (tu_abs_y >> 3) * numbytes_row;
            pu1_pic_intra_flag += (tu_abs_x >> 6);

            intra_flag = *pu1_pic_intra_flag;
            intra_flag &= (1 << ((tu_abs_x >> 3) % 8));
        }

        u1_luma_pred_mode = ps_tu->b6_luma_intra_mode;
        u1_chroma_pred_mode = ps_tu->b3_chroma_intra_mode_idx;

        if(u1_chroma_pred_mode != 7)
            num_comp = 2; /* Y and UV */
        else
            num_comp = 1; /* Y */


        pcm_flag = 0;

        if((intra_flag) && (u1_luma_pred_mode == INTRA_PRED_NONE))
        {
            UWORD8 *pu1_buf;
            UWORD8 *pu1_y_dst = pu1_y_dst_ctb;
            UWORD8 *pu1_uv_dst = pu1_uv_dst_ctb;
            WORD32 i, j;
            tu_sblk_coeff_data_t *ps_tu_sblk_coeff_data;
            WORD32 cb_size = 1 << (ps_tu->b3_size + 2);

            /* trans_size is used to update availability after reconstruction */
            trans_size = cb_size;

            pcm_flag = 1;

            tu_y_offset = tu_x + tu_y * pic_strd;
            pu1_y_dst += tu_x + tu_y * pic_strd;
            pu1_uv_dst += tu_x + (tu_y >> 1) * pic_strd;

            /* First byte points to number of coded blocks */
            pu1_tu_coeff_data++;

            /* Next byte points to scan type */
            pu1_tu_coeff_data++;

            ps_tu_sblk_coeff_data = (tu_sblk_coeff_data_t *)pu1_tu_coeff_data;

            pu1_buf = (UWORD8 *)&ps_tu_sblk_coeff_data->ai2_level[0];
            {

                for(i = 0; i < cb_size; i++)
                {
                    //pu1_y_dst[i * pic_strd + j] = *pu1_buf++;
                    memcpy(&pu1_y_dst[i * pic_strd], pu1_buf, cb_size);
                    pu1_buf += cb_size;
                }

                pu1_uv_dst = pu1_uv_dst + chroma_yuv420sp_vu_u_offset;

                /* U */
                for(i = 0; i < cb_size / 2; i++)
                {
                    for(j = 0; j < cb_size / 2; j++)
                    {
                        pu1_uv_dst[i * pic_strd + 2 * j] = *pu1_buf++;
                    }
                }

                pu1_uv_dst = pu1_uv_dst + 1 + chroma_yuv420sp_vu_v_offset;

                /* V */
                for(i = 0; i < cb_size / 2; i++)
                {
                    for(j = 0; j < cb_size / 2; j++)
                    {
                        pu1_uv_dst[i * pic_strd + 2 * j] = *pu1_buf++;
                    }
                }
            }

            pu1_tu_coeff_data = pu1_buf;

        }





        for(c_idx = 0; c_idx < num_comp; c_idx++)
        {
            if(0 == pcm_flag)
            {
                /* Initializing variables */
                pred_strd = pic_strd;
                dst_strd = pic_strd;

                if(c_idx == 0) /* Y */
                {
                    log2_y_trans_size_minus_2 = ps_tu->b3_size;
                    trans_size = 1 << (log2_y_trans_size_minus_2 + 2);
                    log2_trans_size = log2_y_trans_size_minus_2 + 2;

                    tu_y_offset = tu_x + tu_y * pic_strd;

                    pi2_src = pi2_tu_coeff;
                    pu1_pred = pu1_y_dst_ctb + tu_y_offset;
                    pu1_dst = pu1_y_dst_ctb + tu_y_offset;

                    /* Calculating scaling matrix offset */
                    offset = log2_y_trans_size_minus_2 * 6
                                    + (!intra_flag)
                                    * ((log2_y_trans_size_minus_2
                                                    == 3) ? 1 : 3)
                                    + c_idx;
                    pi2_dequant_matrix = pi2_scaling_mat
                                    + scaling_mat_offset[offset];

                    src_strd = trans_size;

                    /* 4x4 transform Luma in INTRA mode is DST */
                    if(log2_y_trans_size_minus_2 == 0 && intra_flag)
                    {
                        func_idx = log2_y_trans_size_minus_2;
                        e_trans_type = DST_4x4;
                    }
                    else
                    {
                        func_idx = log2_y_trans_size_minus_2 + 1;
                        e_trans_type = (TRANSFORM_TYPE)(log2_y_trans_size_minus_2 + 1);
                    }

                    qp_div = ps_tu->b7_qp / 6;
                    qp_rem = ps_tu->b7_qp % 6;

                    u1_cbf = ps_tu->b1_y_cbf;

                    transform_skip_flag = pu1_tu_coeff_data[1] & 1;
                    /* Unpacking coeffs */
                    if(1 == u1_cbf)
                    {
                        pu1_tu_coeff_data = ihevcd_unpack_coeffs(
                                        pi2_src, log2_y_trans_size_minus_2 + 2,
                                        pu1_tu_coeff_data, pi2_dequant_matrix,
                                        qp_rem, qp_div, e_trans_type,
                                        ps_tu->b1_transquant_bypass, &zero_cols,
                                        &zero_rows, &coeff_type,
                                        &i2_coeff_value);
                    }
                }
                else /* UV interleaved */
                {
                    /* Chroma :If Transform size is 4x4, keep 4x4 else do transform on (trans_size/2 x trans_size/2) */
                    if(ps_tu->b3_size == 0)
                    {
                        /* Chroma 4x4 is present with 4th luma 4x4 block. For this case chroma postion has to be (luma pos x- 4,luma pos y- 4) */
                        log2_uv_trans_size_minus_2 = ps_tu->b3_size;
                        tu_uv_offset = (tu_x - 4) + ((tu_y - 4) / 2) * pic_strd;
                    }
                    else
                    {
                        log2_uv_trans_size_minus_2 = ps_tu->b3_size - 1;
                        tu_uv_offset = tu_x + (tu_y >> 1) * pic_strd;
                    }
                    trans_size = 1 << (log2_uv_trans_size_minus_2 + 2);
                    log2_trans_size = log2_uv_trans_size_minus_2 + 2;

                    pi2_src = pi2_tu_coeff;
                    pi2_src_v = pi2_tu_coeff + trans_size * trans_size;
                    pu1_pred = pu1_uv_dst_ctb + tu_uv_offset + chroma_yuv420sp_vu_u_offset; /* Pointing to start byte of U*/
                    pu1_pred_v = pu1_pred + 1 + chroma_yuv420sp_vu_v_offset; /* Pointing to start byte of V*/
                    pu1_dst = pu1_uv_dst_ctb + tu_uv_offset + chroma_yuv420sp_vu_u_offset; /* Pointing to start byte of U*/
                    pu1_dst_v = pu1_dst + 1 + chroma_yuv420sp_vu_v_offset; /* Pointing to start byte of V*/

                    /*TODO: Add support for choosing different tables for U and V,
                     * change this to a single array to handle flat/default/custom, intra/inter, luma/chroma and various sizes
                     */
                    /* Calculating scaling matrix offset */
                    /* ((log2_uv_trans_size_minus_2 == 3) ? 1:3) condition check is not needed, since
                     * max uv trans size is 16x16
                     */
                    offset = log2_uv_trans_size_minus_2 * 6
                                    + (!intra_flag) * 3 + c_idx;
                    pi2_dequant_matrix = pi2_scaling_mat
                                    + scaling_mat_offset[offset];
                    pi2_dequant_matrix_v = pi2_scaling_mat
                                    + scaling_mat_offset[offset + 1];

                    src_strd = trans_size;

                    func_idx = 1 + 4 + log2_uv_trans_size_minus_2; /* DST func + Y funcs + cur func index*/

                    /* Handle error cases where 64x64 TU is signalled which results in 32x32 chroma.
                     * By limiting func_idx to 7, max of 16x16 chroma is called */
                    func_idx = MIN(func_idx, 7);

                    e_trans_type = (TRANSFORM_TYPE)(log2_uv_trans_size_minus_2 + 1);
                    /* QP for U */
                    i1_chroma_pic_qp_offset = ps_pps->i1_pic_cb_qp_offset;
                    i1_chroma_slice_qp_offset = ps_slice_hdr->i1_slice_cb_qp_offset;
                    u1_cbf = ps_tu->b1_cb_cbf;

                    chroma_qp_idx = ps_tu->b7_qp + i1_chroma_pic_qp_offset
                                    + i1_chroma_slice_qp_offset;
                    chroma_qp_idx = CLIP3(chroma_qp_idx, 0, 57);
                    qp_div = gai2_ihevcd_chroma_qp[chroma_qp_idx] / 6;
                    qp_rem = gai2_ihevcd_chroma_qp[chroma_qp_idx] % 6;

                    /* QP for V */
                    i1_chroma_pic_qp_offset = ps_pps->i1_pic_cr_qp_offset;
                    i1_chroma_slice_qp_offset = ps_slice_hdr->i1_slice_cr_qp_offset;
                    u1_cbf_v = ps_tu->b1_cr_cbf;

                    chroma_qp_idx = ps_tu->b7_qp + i1_chroma_pic_qp_offset
                                    + i1_chroma_slice_qp_offset;
                    chroma_qp_idx = CLIP3(chroma_qp_idx, 0, 57);
                    qp_div_v = gai2_ihevcd_chroma_qp[chroma_qp_idx] / 6;
                    qp_rem_v = gai2_ihevcd_chroma_qp[chroma_qp_idx] % 6;

                    /* Unpacking coeffs */
                    transform_skip_flag = pu1_tu_coeff_data[1] & 1;
                    if(1 == u1_cbf)
                    {
                        pu1_tu_coeff_data = ihevcd_unpack_coeffs(
                                        pi2_src, log2_uv_trans_size_minus_2 + 2,
                                        pu1_tu_coeff_data, pi2_dequant_matrix,
                                        qp_rem, qp_div, e_trans_type,
                                        ps_tu->b1_transquant_bypass, &zero_cols,
                                        &zero_rows, &coeff_type,
                                        &i2_coeff_value);
                    }

                    transform_skip_flag_v = pu1_tu_coeff_data[1] & 1;
                    if(1 == u1_cbf_v)
                    {
                        pu1_tu_coeff_data = ihevcd_unpack_coeffs(
                                        pi2_src_v, log2_uv_trans_size_minus_2 + 2,
                                        pu1_tu_coeff_data, pi2_dequant_matrix_v,
                                        qp_rem_v, qp_div_v, e_trans_type,
                                        ps_tu->b1_transquant_bypass, &zero_cols_v,
                                        &zero_rows_v, &coeff_type_v, &i2_coeff_value_v);
                    }
                }
                /***************************************************************/
                /******************  Intra Prediction **************************/
                /***************************************************************/
                if(intra_flag) /* Intra */
                {
                    /* While (MAX_TU_SIZE * 2 * 2) + 1 is the actaul size needed,
                       au1_ref_sub_out size is kept as multiple of 8,
                       so that SIMD functions can load 64 bits */
                    UWORD8 au1_ref_sub_out[(MAX_TU_SIZE * 2 * 2) + 8];
                    UWORD8 *pu1_top_left, *pu1_top, *pu1_left;
                    WORD32 luma_pred_func_idx, chroma_pred_func_idx;

                    /* Get the neighbour availability flags */
                    /* Done for only Y */
                    if(c_idx == 0)
                    {
                        /* Get neighbor availability for Y only */
                        luma_nbr_flags = ihevcd_get_intra_nbr_flag(ps_proc,
                                                                   ps_tu,
                                                                   au4_intra_nbr_avail,
                                                                   ps_sps->i2_pic_width_in_luma_samples,
                                                                   ps_pps->i1_constrained_intra_pred_flag,
                                                                   trans_size,
                                                                   ctb_size);

                        if(trans_size == 4)
                            luma_nbr_flags_4x4[(ps_tu->b4_pos_x % 2) + (ps_tu->b4_pos_y % 2) * 2] = luma_nbr_flags;

                        if((ps_tu->b4_pos_x % 2 == 0) && (ps_tu->b4_pos_y % 2 == 0))
                        {
                            chroma_nbr_flags = luma_nbr_flags;
                        }

                        /* Initializing nbr pointers */
                        pu1_top = pu1_pred - pic_strd;
                        pu1_left = pu1_pred - 1;
                        pu1_top_left = pu1_pred - pic_strd - 1;

                        /* call reference array substitution */
                        if(luma_nbr_flags == 0x1ffff)
                            ps_codec->s_func_selector.ihevc_intra_pred_luma_ref_subst_all_avlble_fptr(
                                            pu1_top_left,
                                            pu1_top, pu1_left, pred_strd, trans_size, luma_nbr_flags, au1_ref_sub_out, 1);
                        else
                            ps_codec->s_func_selector.ihevc_intra_pred_luma_ref_substitution_fptr(
                                            pu1_top_left,
                                            pu1_top, pu1_left, pred_strd, trans_size, luma_nbr_flags, au1_ref_sub_out, 1);

                        /* call reference filtering */
                        ps_codec->s_func_selector.ihevc_intra_pred_ref_filtering_fptr(
                                        au1_ref_sub_out, trans_size,
                                        au1_ref_sub_out,
                                        u1_luma_pred_mode, ps_sps->i1_strong_intra_smoothing_enable_flag);

                        /* use the look up to get the function idx */
                        luma_pred_func_idx = g_i4_ip_funcs[u1_luma_pred_mode];

                        /* call the intra prediction function */
                        ps_codec->apf_intra_pred_luma[luma_pred_func_idx](au1_ref_sub_out, 1, pu1_pred, pred_strd, trans_size, u1_luma_pred_mode);
                    }
                    else
                    {
                        /* In case of yuv420sp_vu, prediction happens as usual.         */
                        /* So point the pu1_pred pointer to original prediction pointer */
                        UWORD8 *pu1_pred_orig = pu1_pred - chroma_yuv420sp_vu_u_offset;

                        /*    Top-Left | Top-Right | Top | Left | Bottom-Left
                         *      1         4         4     4         4
                         *
                         * Generating chroma_nbr_flags depending upon the transform size */
                        if(ps_tu->b3_size == 0)
                        {
                            /* Take TL,T,L flags of First luma 4x4 block */
                            chroma_nbr_flags = (luma_nbr_flags_4x4[0] & 0x10FF0);
                            /* Take TR flags of Second luma 4x4 block */
                            chroma_nbr_flags |= (luma_nbr_flags_4x4[1] & 0x0F000);
                            /* Take BL flags of Third luma 4x4 block */
                            chroma_nbr_flags |= (luma_nbr_flags_4x4[2] & 0x0000F);
                        }

                        /* Initializing nbr pointers */
                        pu1_top = pu1_pred_orig - pic_strd;
                        pu1_left = pu1_pred_orig - 2;
                        pu1_top_left = pu1_pred_orig - pic_strd - 2;

                        /* Chroma pred  mode derivation from luma pred mode */
                        {
                            tu_t *ps_tu_tmp = ps_tu;
                            while(!ps_tu_tmp->b1_first_tu_in_cu)
                            {
                                ps_tu_tmp--;
                            }
                            u1_luma_pred_mode_first_tu = ps_tu_tmp->b6_luma_intra_mode;
                        }
                        if(4 == u1_chroma_pred_mode)
                            u1_chroma_pred_mode = u1_luma_pred_mode_first_tu;
                        else
                        {
                            u1_chroma_pred_mode = gau1_intra_pred_chroma_modes[u1_chroma_pred_mode];

                            if(u1_chroma_pred_mode ==
                                                            u1_luma_pred_mode_first_tu)
                            {
                                u1_chroma_pred_mode = INTRA_ANGULAR(34);
                            }
                        }

                        /* call the chroma reference array substitution */
                        ps_codec->s_func_selector.ihevc_intra_pred_chroma_ref_substitution_fptr(
                                        pu1_top_left,
                                        pu1_top, pu1_left, pic_strd, trans_size, chroma_nbr_flags, au1_ref_sub_out, 1);

                        /* use the look up to get the function idx */
                        chroma_pred_func_idx =
                                        g_i4_ip_funcs[u1_chroma_pred_mode];

                        /* call the intra prediction function */
                        ps_codec->apf_intra_pred_chroma[chroma_pred_func_idx](au1_ref_sub_out, 1, pu1_pred_orig, pred_strd, trans_size, u1_chroma_pred_mode);
                    }
                }

                /* Updating number of transform types */
                STATS_UPDATE_ALL_TRANS(e_trans_type, c_idx);

                /* IQ, IT and Recon for Y if c_idx == 0, and U if c_idx !=0 */
                if(1 == u1_cbf)
                {
                    if(ps_tu->b1_transquant_bypass || transform_skip_flag)
                    {
                        /* Recon */
                        ps_codec->apf_recon[func_idx](pi2_src, pu1_pred, pu1_dst,
                                                      src_strd, pred_strd, dst_strd,
                                                      zero_cols);
                    }
                    else
                    {

                        /* Updating coded number of transform types(excluding trans skip and trans quant skip) */
                        STATS_UPDATE_CODED_TRANS(e_trans_type, c_idx, 0);

                        /* iQuant , iTrans and Recon */
                        if((0 == coeff_type))
                        {
                            ps_codec->apf_itrans_recon[func_idx](pi2_src, pi2_tmp,
                                                                 pu1_pred, pu1_dst,
                                                                 src_strd, pred_strd,
                                                                 dst_strd, zero_cols,
                                                                 zero_rows);
                        }
                        else /* DC only */
                        {
                            STATS_UPDATE_CODED_TRANS(e_trans_type, c_idx, 1);
                            ps_codec->apf_itrans_recon_dc[c_idx](pu1_pred, pu1_dst,
                                                                 pred_strd, dst_strd,
                                                                 log2_trans_size,
                                                                 i2_coeff_value);
                        }
                    }
                }
                /* IQ, IT and Recon for V */
                if(c_idx != 0)
                {
                    if(1 == u1_cbf_v)
                    {
                        if(ps_tu->b1_transquant_bypass || transform_skip_flag_v)
                        {
                            /* Recon */
                            ps_codec->apf_recon[func_idx](pi2_src_v, pu1_pred_v,
                                                          pu1_dst_v, src_strd,
                                                          pred_strd, dst_strd,
                                                          zero_cols_v);
                        }
                        else
                        {
                            /* Updating number of transform types */
                            STATS_UPDATE_CODED_TRANS(e_trans_type, c_idx, 0);

                            /* iQuant , iTrans and Recon */
                            if((0 == coeff_type_v))
                            {
                                ps_codec->apf_itrans_recon[func_idx](pi2_src_v,
                                                                     pi2_tmp,
                                                                     pu1_pred_v,
                                                                     pu1_dst_v,
                                                                     src_strd,
                                                                     pred_strd,
                                                                     dst_strd,
                                                                     zero_cols_v,
                                                                     zero_rows_v);
                            }
                            else  /* DC only */
                            {
                                STATS_UPDATE_CODED_TRANS(e_trans_type, c_idx, 1);
                                ps_codec->apf_itrans_recon_dc[c_idx](pu1_pred_v, pu1_dst_v,
                                                                     pred_strd, dst_strd,
                                                                     log2_trans_size,
                                                                     i2_coeff_value_v);
                            }
                        }
                    }
                }
            }

            /* Neighbor availability inside CTB */
            /* 1bit per 4x4. Indicates whether that 4x4 block has been reconstructed(avialable) */
            /* Used for neighbor availability in intra pred */
            if(c_idx == 0)
            {
                WORD32 i;
                WORD32 trans_in_min_tu;
                UWORD32 cur_tu_in_bits;
                UWORD32 cur_tu_avail_flag;

                trans_in_min_tu = trans_size / MIN_TU_SIZE;
                cur_tu_in_bits = (1 << trans_in_min_tu) - 1;
                cur_tu_in_bits = cur_tu_in_bits << (32 - trans_in_min_tu);

                cur_tu_avail_flag = cur_tu_in_bits >> (ps_tu->b4_pos_x + 1);

                for(i = 0; i < trans_in_min_tu; i++)
                    au4_intra_nbr_avail[1 + ps_tu->b4_pos_y + i] |=
                                    cur_tu_avail_flag;
            }
        }
    }
    ps_proc->pv_tu_coeff_data = pu1_tu_coeff_data;

    return ps_proc->i4_ctb_tu_cnt;
}

