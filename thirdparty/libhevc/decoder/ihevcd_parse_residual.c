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
*  ihevcd_parse_residual.c
*
* @brief
*  Contains functions for parsing residual data at TU level
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

#include "ihevc_defs.h"
#include "ihevc_debug.h"
#include "ihevc_structs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"

#include "ihevc_common_tables.h"
#include "ihevc_error.h"
#include "ihevc_cabac_tables.h"

#include "ihevcd_trace.h"
#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_nal.h"
#include "ihevcd_bitstream.h"
#include "ihevcd_utils.h"
#include "ihevcd_parse_residual.h"
#include "ihevcd_cabac.h"

/**
  *****************************************************************************
  * @brief  returns context increment for sig coeff based on csbf neigbour
  *         flags (bottom and right) and current coeff postion in 4x4 block
  *         See section 9.3.3.1.4 for details on this context increment
  *
  * input   : neigbour csbf flags(bit0:rightcsbf, bit1:bottom csbf)
  *           coeff idx in raster order (0-15)
  *
  * output  : context increment for sig coeff flag
  *
  *****************************************************************************
  */
const UWORD8 gau1_ihevcd_sigcoeff_ctxtinc[3][4][16] =
{

    {
        /* nbr csbf = 0:  sigCtx = (xP+yP == 0) ? 2 : (xP+yP < 3) ? 1: 0 */
        { 2,    1,    1,    1,    1,    1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },
        /* nbr csbf = 1:  sigCtx = (yP == 0) ? 2 : (yP == 1) ? 1: 0      */
        { 2,    1,    2,    0,    1,    2,    0,    0,    1,    2,    0,    0,    1,    0,    0,    0 },
        /* nbr csbf = 2:  sigCtx = (xP == 0) ? 2 : (xP == 1) ? 1: 0      */
        { 2,    2,    1,    2,    1,    0,    2,    1,    0,    0,    1,    0,    0,    0,    0,    0 },
        /* nbr csbf = 3:  sigCtx = 2                                     */
        { 2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2 },
    },
    {
        /* nbr csbf = 0:  sigCtx = (xP+yP == 0) ? 2 : (xP+yP < 3) ? 1: 0 */
        { 2,    1,    1,    0,    1,    1,    0,    0,    1,    0,    0,    0,    0,    0,    0,    0 },
        /* nbr csbf = 1:  sigCtx = (yP == 0) ? 2 : (yP == 1) ? 1: 0      */
        { 2,    2,    2,    2,    1,    1,    1,    1,    0,    0,    0,    0,    0,    0,    0,    0 },
        /* nbr csbf = 2:  sigCtx = (xP == 0) ? 2 : (xP == 1) ? 1: 0      */
        { 2,    1,    0,    0,    2,    1,    0,    0,    2,    1,    0,    0,    2,    1,    0,    0 },
        /* nbr csbf = 3:  sigCtx = 2                                     */
        { 2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2 },
    },
    {
        /* nbr csbf = 0:  sigCtx = (xP+yP == 0) ? 2 : (xP+yP < 3) ? 1: 0 */
        { 2,    1,    1,    0,    1,    1,    0,    0,    1,    0,    0,    0,    0,    0,    0,    0 },
        /* nbr csbf = 1:  sigCtx = (yP == 0) ? 2 : (yP == 1) ? 1: 0      */
        { 2,    1,    0,    0,    2,    1,    0,    0,    2,    1,    0,    0,    2,    1,    0,    0 },
        /* nbr csbf = 2:  sigCtx = (xP == 0) ? 2 : (xP == 1) ? 1: 0      */
        { 2,    2,    2,    2,    1,    1,    1,    1,    0,    0,    0,    0,    0,    0,    0,    0 },
        /* nbr csbf = 3:  sigCtx = 2                                     */
        { 2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2 },
    },


};



/**
  *****************************************************************************
  * @brief  returns context increment for sig coeff for 4x4 tranform size as
  *         per Table 9-39 in section 9.3.3.1.4
  *
  * input   : coeff idx in raster order (0-15)
  *
  * output  : context increment for sig coeff flag
  *
  *****************************************************************************
  */
const UWORD8 gau1_ihevcd_sigcoeff_ctxtinc_tr4[3][16] =
{
    /* Upright diagonal scan */
    {
        0,    2,    1,    6,
        3,    4,    7,    6,
        4,    5,    7,    8,
        5,    8,    8,    8,
    },
    /* Horizontal scan */
    {
        0,    1,    4,    5,
        2,    3,    4,    5,
        6,    6,    8,    8,
        7,    7,    8,    8,
    },
    /* Vertical scan */
    {
        0,    2,    6,    7,
        1,    3,    6,    7,
        4,    4,    8,    8,
        5,    5,    8,    8,
    },
};


/**
*******************************************************************************
*
* @brief
*  Parses Residual coding
*
* @par Description:
*  Parses Residual coding as per  Section:7.3.13
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns  error code from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/

WORD32 ihevcd_parse_residual_coding(codec_t *ps_codec,
                                    WORD32 x0, WORD32 y0,
                                    WORD32 log2_trafo_size,
                                    WORD32 c_idx,
                                    WORD32 intra_pred_mode)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 transform_skip_flag;
    WORD32 value;
    pps_t *ps_pps;
    WORD32 last_scan_pos, last_sub_blk;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    WORD32 last_significant_coeff_x_prefix, last_significant_coeff_y_prefix;
    WORD32 last_significant_coeff_x, last_significant_coeff_y;
    const UWORD8 *pu1_scan_blk = NULL, *pu1_scan_coeff;
    WORD32 scan_idx;
    WORD32 i;
    WORD32 sign_data_hiding_flag;
    cab_ctxt_t *ps_cabac = &ps_codec->s_parse.s_cabac;
    WORD32 gt1_ctxt = 1;
    WORD32 c_max;
    UWORD16 au2_csbf[9];
    tu_sblk_coeff_data_t *ps_tu_sblk_coeff_data;
    WORD8 *pi1_num_coded_subblks;
    WORD32 num_subblks;
    WORD32 sig_coeff_base_ctxt, abs_gt1_base_ctxt;
    UNUSED(x0);
    UNUSED(y0);
    ps_pps = ps_codec->s_parse.ps_pps;

    sign_data_hiding_flag = ps_pps->i1_sign_data_hiding_flag;
    transform_skip_flag = 0;
    if(ps_pps->i1_transform_skip_enabled_flag &&
       !ps_codec->s_parse.s_cu.i4_cu_transquant_bypass &&
       (log2_trafo_size == 2))
    {
        WORD32 ctxt_idx;

        if(!c_idx)
        {
            ctxt_idx = IHEVC_CAB_TFM_SKIP0;
        }
        else
        {
            ctxt_idx = IHEVC_CAB_TFM_SKIP12;
        }
        TRACE_CABAC_CTXT("transform_skip_flag", ps_cabac->u4_range, ctxt_idx);
        value = ihevcd_cabac_decode_bin(ps_cabac,
                                        ps_bitstrm,
                                        ctxt_idx);
        AEV_TRACE("transform_skip_flag", value, ps_cabac->u4_range);
        transform_skip_flag = value;
    }

    /* code the last_coeff_x_prefix as tunary binarized code */
    {
        WORD32 ctxt_idx_x, ctxt_idx_y, ctx_shift;
        WORD32 ctx_offset;
        c_max = (log2_trafo_size << 1) - 1;

        if(!c_idx)
        {
            ctx_offset = (3 * (log2_trafo_size - 2)) + ((log2_trafo_size - 1) >> 2);
            ctxt_idx_x = IHEVC_CAB_COEFFX_PREFIX + ctx_offset;
            ctxt_idx_y = IHEVC_CAB_COEFFY_PREFIX + ctx_offset;
            ctx_shift  = (log2_trafo_size + 1) >> 2;
        }
        else
        {
            ctxt_idx_x = IHEVC_CAB_COEFFX_PREFIX + 15;
            ctxt_idx_y = IHEVC_CAB_COEFFY_PREFIX + 15;
            ctx_shift  = log2_trafo_size  - 2;
        }

        TRACE_CABAC_CTXT("last_coeff_x_prefix", ps_cabac->u4_range, ctxt_idx_x);
        last_significant_coeff_x_prefix = ihevcd_cabac_decode_bins_tunary(ps_cabac,
                                                                          ps_bitstrm,
                                                                          c_max,
                                                                          ctxt_idx_x,
                                                                          ctx_shift,
                                                                          c_max);

        AEV_TRACE("last_coeff_x_prefix", last_significant_coeff_x_prefix, ps_cabac->u4_range);

        TRACE_CABAC_CTXT("last_coeff_y_prefix", ps_cabac->u4_range, ctxt_idx_y);
        last_significant_coeff_y_prefix = ihevcd_cabac_decode_bins_tunary(ps_cabac,
                                                                          ps_bitstrm,
                                                                          c_max,
                                                                          ctxt_idx_y,
                                                                          ctx_shift,
                                                                          c_max);

        AEV_TRACE("last_coeff_y_prefix", last_significant_coeff_y_prefix, ps_cabac->u4_range);


        last_significant_coeff_x = last_significant_coeff_x_prefix;
        if(last_significant_coeff_x_prefix > 3)
        {
            WORD32 suf_length = ((last_significant_coeff_x_prefix - 2) >> 1);

            value = ihevcd_cabac_decode_bypass_bins(ps_cabac,
                                                    ps_bitstrm,
                                                    suf_length);

            AEV_TRACE("last_coeff_x_suffix", value, ps_cabac->u4_range);


            last_significant_coeff_x =
                            (1 << ((last_significant_coeff_x_prefix >> 1) - 1)) *
                            (2 + (last_significant_coeff_x_prefix & 1)) + value;
        }


        last_significant_coeff_y = last_significant_coeff_y_prefix;
        if(last_significant_coeff_y_prefix > 3)
        {
            WORD32 suf_length = ((last_significant_coeff_y_prefix - 2) >> 1);
            value = ihevcd_cabac_decode_bypass_bins(ps_cabac,
                                                    ps_bitstrm,
                                                    suf_length);

            AEV_TRACE("last_coeff_y_suffix", value, ps_cabac->u4_range);
            last_significant_coeff_y =
                            (1 << ((last_significant_coeff_y_prefix >> 1) - 1)) *
                            (2 + (last_significant_coeff_y_prefix & 1)) + value;
        }

    }

    /* Choose a scan matrix based on intra flag, intra pred mode, transform size
     and luma/chroma */
    scan_idx = SCAN_DIAG_UPRIGHT;
    if(PRED_MODE_INTRA == ps_codec->s_parse.s_cu.i4_pred_mode)
    {
        if((2 == log2_trafo_size) || ((3 == log2_trafo_size) && (0 == c_idx)))
        {
            if((6 <= intra_pred_mode) &&
               (14 >= intra_pred_mode))
            {
                scan_idx = SCAN_VERT;
            }
            else if((22 <= intra_pred_mode) &&
                    (30 >= intra_pred_mode))
            {
                scan_idx = SCAN_HORZ;
            }
        }
    }

    /* In case the scan is vertical, then swap  X and Y positions */
    if(SCAN_VERT == scan_idx)
    {
        SWAP(last_significant_coeff_x, last_significant_coeff_y);
    }

    {
        WORD8 *pi1_scan_idx;
        WORD8 *pi1_buf = (WORD8 *)ps_codec->s_parse.pv_tu_coeff_data;

        /* First WORD8 gives number of coded subblocks */
        pi1_num_coded_subblks = pi1_buf++;

        /* Set number of coded subblocks in the current TU to zero */
        /* This will be updated later */
        *pi1_num_coded_subblks = 0;

        /* Second WORD8 gives (scan idx << 1) | trans_skip */
        pi1_scan_idx = pi1_buf++;
        *pi1_scan_idx = (scan_idx << 1) | transform_skip_flag;

        /* Store the incremented pointer in pv_tu_coeff_data */
        ps_codec->s_parse.pv_tu_coeff_data = pi1_buf;

    }
    /**
     * Given last_significant_coeff_y and last_significant_coeff_x find last sub block
     * This is done by ignoring lower two bits of last_significant_coeff_y and last_significant_coeff_x
     * and using scan matrix for lookup
     */

    /* If transform is 4x4, last_sub_blk is zero */
    last_sub_blk = 0;

    /* If transform is larger than 4x4, then based on scan_idx and transform size, choose a scan table */

    if(log2_trafo_size > 2)
    {
        WORD32 scan_pos;
        WORD32 scan_mat_size;
        pu1_scan_blk = (UWORD8 *)gapv_ihevc_scan[scan_idx * 3 + (log2_trafo_size - 2 - 1)];


        /* Divide the current transform to 4x4 subblocks and count number of 4x4 in the first row */
        /* This will be size of scan matrix to be used for subblock scanning */
        scan_mat_size = 1 << (log2_trafo_size - 2);
        scan_pos = ((last_significant_coeff_y >> 2) * scan_mat_size) +
                        (last_significant_coeff_x >> 2);

        last_sub_blk = pu1_scan_blk[scan_pos];
    }
    pu1_scan_coeff  = &gau1_ihevc_scan4x4[scan_idx][0];

    {
        WORD32 scan_pos;

        scan_pos = ((last_significant_coeff_y & 3) << 2) +
                        (last_significant_coeff_x & 3);

        last_scan_pos = pu1_scan_coeff[scan_pos];
    }
    if(log2_trafo_size > 2)
        pu1_scan_blk = (UWORD8 *)gapv_ihevc_invscan[scan_idx * 3 + (log2_trafo_size - 2 - 1)];
    pu1_scan_coeff  = &gau1_ihevc_invscan4x4[scan_idx][0];

    /* Set CSBF array to zero */
    {
        UWORD32 *pu4_csbf;
        pu4_csbf = (void *)au2_csbf;
        *pu4_csbf++ = 0;
        *pu4_csbf++ = 0;
        *pu4_csbf++ = 0;
        *pu4_csbf = 0;
        /* To avoid a check for y pos, 9th WORD16 in the array is set to zero */
        au2_csbf[8] = 0;
    }

    /*************************************************************************/
    /* derive base context index for sig coeff as per section 9.3.3.1.4      */
    /* TODO; convert to look up based on luma/chroma, scan type and tfr size */
    /*************************************************************************/
    if(!c_idx)
    {
        sig_coeff_base_ctxt = IHEVC_CAB_COEFF_FLAG;
        abs_gt1_base_ctxt = IHEVC_CAB_COEFABS_GRTR1_FLAG;

        if(3 == log2_trafo_size)
        {
            /* 8x8 transform size */
            sig_coeff_base_ctxt += (scan_idx == SCAN_DIAG_UPRIGHT) ? 9 : 15;
        }
        else  if(3 < log2_trafo_size)
        {
            /* larger transform sizes */
            sig_coeff_base_ctxt += 21;
        }
    }
    else
    {
        /* chroma context initializations */
        sig_coeff_base_ctxt = IHEVC_CAB_COEFF_FLAG + 27;
        abs_gt1_base_ctxt = IHEVC_CAB_COEFABS_GRTR1_FLAG + 16;

        if(3 == log2_trafo_size)
        {
            /* 8x8 transform size */
            sig_coeff_base_ctxt += 9;
        }
        else  if(3 < log2_trafo_size)
        {
            /* larger transform sizes */
            sig_coeff_base_ctxt += 12;
        }
    }
    num_subblks = 0;
    /* Parse each 4x4 subblocks */
    for(i = last_sub_blk; i >= 0; i--)
    {
        WORD32 sub_blk_pos;
        WORD32 infer_sig_coeff_flag;
        WORD32 cur_csbf;

        WORD32 n;
        WORD32 num_coeff;
        /* Sig coeff map for 16 entries in raster scan order. Upper 16 bits are used.
         * MSB gives sig coeff flag for 0th coeff and so on
         * UWORD16 would have been enough but kept as UWORD32 for code optimizations
         * In arm unnecessary masking operations are saved
         */
        UWORD32 u4_sig_coeff_map_raster;
        WORD32 sign_hidden;

        /* Sig coeff map in scan order */
        UWORD32 u4_sig_coeff_map;
        WORD32 coeff_abs_level_greater2_flag;
        UWORD32 u4_coeff_abs_level_greater1_map;
        UWORD32 u4_coeff_abs_level_greater2_map;
        UWORD32 u4_coeff_sign_map;
        WORD32 first_sig_scan_pos, last_sig_scan_pos, num_greater1_flag, first_greater1_scan_pos;
        WORD32  num_sig_coeff, sum_abs_level;
        WORD32 nbr_csbf;


        WORD32 ctxt_set;
        WORD32 rice_param;
        WORD32 xs, ys;


        sub_blk_pos  = 0;
        if(i && (log2_trafo_size > 2))
            sub_blk_pos = pu1_scan_blk[i];

        /* Get xs and ys from scan position */
        /* This is needed for context modelling of significant coeff flag */
        xs = sub_blk_pos & ((1 << (log2_trafo_size - 2)) - 1);
        ys = sub_blk_pos >> (log2_trafo_size - 2);


        /* Check if neighbor subblocks are coded */
        {

            nbr_csbf = 0;

            /* Get Bottom sub blocks CSBF */
            nbr_csbf |= (au2_csbf[ys + 1] >> xs) & 1;
            nbr_csbf <<= 1;

            /* Get Right sub blocks CSBF */
            /* Even if xs is equal to (1 << (log2_trafo_size - 2 )) - 1,
               since au2_csbf is set to zero at the beginning, csbf for
               neighbor will be read as 0 */

            nbr_csbf |= (au2_csbf[ys] >> (xs + 1)) & 1;


        }
        cur_csbf = 0;

        /* DC coeff is inferred, only if coded_sub_block is explicitly parsed as 1 */
        /* i.e. it is not inferred for first and last subblock */
        infer_sig_coeff_flag = 0;
        if((i < last_sub_blk) && (i > 0))
        {
            WORD32 ctxt_idx  = IHEVC_CAB_CODED_SUBLK_IDX;

            /* ctxt based on right / bottom avail csbf, section 9.3.3.1.3 */
            ctxt_idx += (nbr_csbf) ? 1 : 0;

            /* Ctxt based on luma or chroma */
            ctxt_idx += c_idx  ? 2 : 0;
            TRACE_CABAC_CTXT("coded_sub_block_flag", ps_cabac->u4_range, ctxt_idx);
            IHEVCD_CABAC_DECODE_BIN(cur_csbf, ps_cabac, ps_bitstrm, ctxt_idx);
            AEV_TRACE("coded_sub_block_flag", cur_csbf, ps_cabac->u4_range);

            infer_sig_coeff_flag = 1;
        }
        else /* if((i == last_sub_blk) || (sub_blk_pos == 0)) */
        {
            /* CSBF is set to 1 for first and last subblock */
            /* Note for these subblocks sig_coeff_map is not inferred but instead parsed */
            cur_csbf = 1;
        }

        /* Set current sub blocks CSBF */
        {
            UWORD32 u4_mask = 1 << xs;
            if(cur_csbf)
                au2_csbf[ys] |= u4_mask;
            else
                au2_csbf[ys] &= ~u4_mask;

        }

        /* If current subblock is not coded, proceed to the next subblock */
        if(0 == cur_csbf)
            continue;

        n = 15;
        u4_sig_coeff_map_raster = 0;
        u4_sig_coeff_map = 0;
        num_coeff = 0;
        if(i == last_sub_blk)
        {
            WORD32 pos = ((last_significant_coeff_y & 3) << 2) +
                            (last_significant_coeff_x & 3);
            n = (last_scan_pos - 1);
            /* Set Significant coeff map for last significant coeff flag as 1 */
            u4_sig_coeff_map_raster = 1 << pos;
            u4_sig_coeff_map = 1 << last_scan_pos;
            num_coeff = 1;
        }

        for(; n >= 0; n--)
        {
            WORD32 significant_coeff_flag;

            if((n > 0 || !infer_sig_coeff_flag))
            {
                //WORD32 coeff_pos;
                WORD32 sig_ctxinc;
                WORD32 ctxt_idx;

                /* Coefficient position is needed for deriving context index for significant_coeff_flag */
                //coeff_pos = pu1_scan_coeff[n];
                /* derive the context inc as per section 9.3.3.1.4 */
                sig_ctxinc = 0;
                if(2 == log2_trafo_size)
                {

                    /* 4x4 transform size increment uses lookup */
                    sig_ctxinc = gau1_ihevcd_sigcoeff_ctxtinc_tr4[scan_idx][n];
                }
                else if(n || i)
                {
                    /* ctxt for AC coeff depends on curpos and neigbour csbf */
                    sig_ctxinc = gau1_ihevcd_sigcoeff_ctxtinc[scan_idx][nbr_csbf][n];

                    /* based on luma subblock pos */
                    sig_ctxinc += (i && (!c_idx)) ? 3 : 0;

                }
                else
                {
                    /* DC coeff has fixed context for luma and chroma */
                    sig_coeff_base_ctxt = (0 == c_idx) ? IHEVC_CAB_COEFF_FLAG :
                                                         (IHEVC_CAB_COEFF_FLAG + 27);
                }

                ctxt_idx = sig_ctxinc + sig_coeff_base_ctxt;
                TRACE_CABAC_CTXT("significant_coeff_flag", ps_cabac->u4_range, ctxt_idx);
                IHEVCD_CABAC_DECODE_BIN(significant_coeff_flag, ps_cabac,
                                        ps_bitstrm,
                                        ctxt_idx);
                AEV_TRACE("significant_coeff_flag", significant_coeff_flag, ps_cabac->u4_range);


                /* If at least one non-zero coeff is signalled then do not infer sig coeff map */
                /* for (0,0) coeff in the current sub block */
                if(significant_coeff_flag)
                    infer_sig_coeff_flag = 0;

//                u4_sig_coeff_map_raster |= significant_coeff_flag
//                              << coeff_pos;
                u4_sig_coeff_map |= significant_coeff_flag << n;
                num_coeff += significant_coeff_flag;
            }


        }
        /*********************************************************************/
        /* If infer_sig_coeff_flag is 1 then treat the 0th coeff as non zero */
        /* If infer_sig_coeff_flag is zero, then last significant_coeff_flag */
        /* is parsed in the above loop                                       */
        /*********************************************************************/
        if(infer_sig_coeff_flag)
        {
            u4_sig_coeff_map_raster |= 1;
            u4_sig_coeff_map |= 1;
            num_coeff++;
        }

        /*********************************************************************/
        /* First subblock does not get an explicit csbf. It is assumed to    */
        /* be 1. For this subblock there is chance of getting all            */
        /* sig_coeff_flags to be zero. In such a case proceed to the next    */
        /* subblock(which is end of parsing for the current transform block) */
        /*********************************************************************/

        if(0 == num_coeff)
            continue;

        /* Increment number of coded subblocks for the current TU */
        num_subblks++;

        /* Set sig coeff map and subblock position */
        ps_tu_sblk_coeff_data = (tu_sblk_coeff_data_t *)ps_codec->s_parse.pv_tu_coeff_data;
        ps_tu_sblk_coeff_data->u2_sig_coeff_map = u4_sig_coeff_map;
        ps_tu_sblk_coeff_data->u2_subblk_pos = (ys << 8) | xs;

        first_sig_scan_pos = 16;
        last_sig_scan_pos = -1;
        num_greater1_flag = 0;
        first_greater1_scan_pos = -1;
        u4_coeff_abs_level_greater1_map = 0;


        /* context set based on luma subblock pos */
        ctxt_set = (i && (!c_idx)) ? 2 : 0;

        /* See section 9.3.3.1.5           */
        ctxt_set += (0 == gt1_ctxt) ? 1 : 0;

        gt1_ctxt = 1;
        /* Instead of initializing n to 15, set it to 31-CLZ(sig coeff map) */
        {
            UWORD32 u4_sig_coeff_map_shift;
            UWORD32 clz;
            clz = CLZ(u4_sig_coeff_map);
            n = 31 - clz;
            u4_sig_coeff_map_shift = u4_sig_coeff_map << clz;
            /* For loop for n changed to do while to break early if sig_coeff_map_shift becomes zero */
            do
            {
                //WORD32 coeff_pos;
                WORD32 ctxt_idx;

                //TODO: Scan lookup will be removed later and instead u4_sig_coeff_map will be used
                //coeff_pos = pu1_scan_coeff[n];

                if((u4_sig_coeff_map_shift >> 31) & 1)
                {

                    /* abs_level_greater1_flag is sent for only first 8 non-zero levels in a subblock */
                    if(num_greater1_flag < 8)
                    {
                        WORD32 coeff_abs_level_greater1_flag;

                        ctxt_idx = (ctxt_set * 4) + abs_gt1_base_ctxt + gt1_ctxt;

                        TRACE_CABAC_CTXT("coeff_abs_level_greater1_flag", ps_cabac->u4_range, ctxt_idx);
                        IHEVCD_CABAC_DECODE_BIN(coeff_abs_level_greater1_flag, ps_cabac, ps_bitstrm, ctxt_idx);
                        AEV_TRACE("coeff_abs_level_greater1_flag", coeff_abs_level_greater1_flag, ps_cabac->u4_range);

                        u4_coeff_abs_level_greater1_map |= coeff_abs_level_greater1_flag << n;
                        num_greater1_flag++;

                        /* first_greater1_scan_pos is obtained using CLZ on u4_coeff_abs_level_greater1_map*/
                        /*  outside the loop instead of the following check inside the loop                */
                        /* if( coeff_abs_level_greater1_flag && first_greater1_scan_pos == -1) */
                        /*    first_greater1_scan_pos = n;                                     */

                        if(coeff_abs_level_greater1_flag)
                        {
                            gt1_ctxt = 0;
                        }
                        else if(gt1_ctxt && (gt1_ctxt < 3))
                        {
                            gt1_ctxt++;
                        }

                    }
                    else
                        break;

                    /* instead of computing last and first significan scan position using checks below */
                    /* They are computed outside the loop using CLZ and CTZ on sig_coeff_map */
                    /* if(last_sig_scan_pos == -1)                          */
                    /*    last_sig_scan_pos = n;                            */
                    /*  first_sig_scan_pos = n;                             */
                }
                u4_sig_coeff_map_shift <<= 1;
                n--;
                /* If there are zero coeffs, then shift by as many zero coeffs and decrement n */
                clz = CLZ(u4_sig_coeff_map_shift);
                u4_sig_coeff_map_shift <<= clz;
                n -= clz;
            }while(u4_sig_coeff_map_shift);
        }
        /* At this level u4_sig_coeff_map is non-zero i.e. has atleast one non-zero coeff */
        last_sig_scan_pos = (31 - CLZ(u4_sig_coeff_map));
        first_sig_scan_pos = CTZ(u4_sig_coeff_map);
        sign_hidden = (((last_sig_scan_pos - first_sig_scan_pos) > 3) && !ps_codec->s_parse.s_cu.i4_cu_transquant_bypass);

        u4_coeff_abs_level_greater2_map = 0;

        if(u4_coeff_abs_level_greater1_map)
        {
            /* Check if the first level > 1 is greater than 2 */
            WORD32 ctxt_idx;
            first_greater1_scan_pos = (31 - CLZ(u4_coeff_abs_level_greater1_map));


            ctxt_idx = IHEVC_CAB_COEFABS_GRTR2_FLAG;

            ctxt_idx += (!c_idx) ? ctxt_set : (ctxt_set + 4);
            TRACE_CABAC_CTXT("coeff_abs_level_greater2_flag", ps_cabac->u4_range, ctxt_idx);
            IHEVCD_CABAC_DECODE_BIN(coeff_abs_level_greater2_flag, ps_cabac, ps_bitstrm, ctxt_idx);
            AEV_TRACE("coeff_abs_level_greater2_flag", coeff_abs_level_greater2_flag, ps_cabac->u4_range);
            u4_coeff_abs_level_greater2_map = coeff_abs_level_greater2_flag << first_greater1_scan_pos;
        }


        u4_coeff_sign_map = 0;

        /* Parse sign flags */
        if(!sign_data_hiding_flag || !sign_hidden)
        {
            IHEVCD_CABAC_DECODE_BYPASS_BINS(value, ps_cabac, ps_bitstrm, num_coeff);
            AEV_TRACE("sign_flags", value, ps_cabac->u4_range);
            u4_coeff_sign_map = value << (32 - num_coeff);
        }
        else
        {
            IHEVCD_CABAC_DECODE_BYPASS_BINS(value, ps_cabac, ps_bitstrm, (num_coeff - 1));
            AEV_TRACE("sign_flags", value, ps_cabac->u4_range);
            u4_coeff_sign_map = value << (32 - (num_coeff - 1));
        }

        num_sig_coeff = 0;
        sum_abs_level = 0;
        rice_param = 0;
        {
            UWORD32 clz;
            UWORD32 u4_sig_coeff_map_shift;
            clz = CLZ(u4_sig_coeff_map);
            n = 31 - clz;
            u4_sig_coeff_map_shift = u4_sig_coeff_map << clz;
            /* For loop for n changed to do while to break early if sig_coeff_map_shift becomes zero */
            do
            {

                if((u4_sig_coeff_map_shift >> 31) & 1)
                {
                    WORD32 base_lvl;
                    WORD32 coeff_abs_level_remaining;
                    WORD32 level;
                    base_lvl = 1;

                    /* Update base_lvl if it is greater than 1 */
                    if((u4_coeff_abs_level_greater1_map >> n) & 1)
                        base_lvl++;

                    /* Update base_lvl if it is greater than 2 */
                    if((u4_coeff_abs_level_greater2_map >> n) & 1)
                        base_lvl++;

                    /* If level is greater than 3/2/1 based on the greater1 and greater2 maps,
                     * decode remaining level (level - base_lvl) will be signalled as bypass bins
                     */
                    coeff_abs_level_remaining = 0;
                    if(base_lvl == ((num_sig_coeff < 8) ? ((n == first_greater1_scan_pos) ? 3 : 2) : 1))
                    {
                        UWORD32 u4_prefix;
                        WORD32 bin;

                        u4_prefix = 0;

                        do
                        {
                            IHEVCD_CABAC_DECODE_BYPASS_BIN(bin, ps_cabac, ps_bitstrm);
                            u4_prefix++;

                            if((WORD32)u4_prefix == 19 - rice_param)
                            {
                                bin = 1;
                                break;
                            }

                        }while(bin);

                        u4_prefix = u4_prefix - 1;
                        if(u4_prefix < 3)
                        {
                            UWORD32 u4_suffix;

                            coeff_abs_level_remaining = (u4_prefix << rice_param);
                            if(rice_param)
                            {
                                IHEVCD_CABAC_DECODE_BYPASS_BINS(u4_suffix, ps_cabac, ps_bitstrm, rice_param);

                                coeff_abs_level_remaining |= u4_suffix;
                            }
                        }
                        else
                        {
                            UWORD32 u4_suffix;
                            UWORD32 u4_numbins;

                            //u4_prefix = CLIP3(u4_prefix, 0, 19 - rice_param);

                            u4_numbins = (u4_prefix - 3 + rice_param);
                            coeff_abs_level_remaining = (((1 << (u4_prefix - 3)) + 3 - 1) << rice_param);
                            if(u4_numbins)
                            {
                                IHEVCD_CABAC_DECODE_BYPASS_BINS(u4_suffix, ps_cabac, ps_bitstrm, u4_numbins);
                                coeff_abs_level_remaining += u4_suffix;
                            }
                        }


                        AEV_TRACE("coeff_abs_level_remaining", coeff_abs_level_remaining, ps_cabac->u4_range);
                        base_lvl += coeff_abs_level_remaining;

                    }

                    /* update the rice param based on coeff level */
                    if((base_lvl > (3 << rice_param)) && (rice_param < 4))
                    {
                        rice_param++;
                    }

                    /* Compute absolute level */
                    level = base_lvl;

                    /* Update level with the sign */
                    if((u4_coeff_sign_map >> 31) & 1)
                        level = -level;

                    u4_coeff_sign_map <<= 1;
                    /* Update sign in case sign is hidden */
                    if(sign_data_hiding_flag && sign_hidden)
                    {
                        sum_abs_level += base_lvl;

                        if(n == first_sig_scan_pos && ((sum_abs_level % 2) == 1))
                            level = -level;
                    }

                    /* Store the resulting level in non-zero level array */
                    ps_tu_sblk_coeff_data->ai2_level[num_sig_coeff++] = level;
                    //AEV_TRACE("level", level, 0);
                }
                u4_sig_coeff_map_shift <<= 1;
                n--;
                /* If there are zero coeffs, then shift by as many zero coeffs and decrement n */
                clz = CLZ(u4_sig_coeff_map_shift);
                u4_sig_coeff_map_shift <<= clz;
                n -= clz;


            }while(u4_sig_coeff_map_shift);
        }

        /* Increment the pv_tu_sblk_coeff_data */
        {
            UWORD8 *pu1_buf = (UWORD8 *)ps_codec->s_parse.pv_tu_coeff_data;
            pu1_buf += sizeof(tu_sblk_coeff_data_t) - SUBBLK_COEFF_CNT * sizeof(WORD16);
            pu1_buf += num_coeff * sizeof(WORD16);
            ps_codec->s_parse.pv_tu_coeff_data = pu1_buf;

        }

    }
    /* Set number of coded sub blocks in the current TU */
    *pi1_num_coded_subblks = num_subblks;

    return ret;
}
