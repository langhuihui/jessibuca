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
 *  ihevcd_parse_slice.c
 *
 * @brief
 *  Contains functions for parsing slice data
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
#include "ihevc_structs.h"
#include "ihevc_macros.h"
#include "ihevc_mem_fns.h"
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
#include "ihevcd_parse_slice.h"
#include "ihevcd_parse_residual.h"
#include "ihevcd_cabac.h"
#include "ihevcd_job_queue.h"
#include "ihevcd_intra_pred_mode_prediction.h"
#include "ihevcd_common_tables.h"
#include "ihevcd_process_slice.h"
#include "ihevcd_debug.h"
#include "ihevcd_get_mv.h"
#include "ihevcd_boundary_strength.h"
#include "ihevcd_ilf_padding.h"
#include "ihevcd_statistics.h"
/* Bit stream offset threshold */
#define BITSTRM_OFF_THRS 8

#define MIN_CU_QP_DELTA_ABS(x) (-26 + ((x) * 6) / 2)
#define MAX_CU_QP_DELTA_ABS(x) (25 + ((x) * 6) / 2)

/**
 * Table used to decode part_mode if AMP is enabled and current CU is not min CU
 */
const UWORD8 gau1_part_mode_amp[] = { PART_nLx2N, PART_nRx2N, PART_Nx2N, 0xFF, PART_2NxnU, PART_2NxnD, PART_2NxN, 0xFF };

const UWORD32 gau4_ct_depth_mask[] = { 0x0, 0x55555555, 0xAAAAAAAA, 0xFFFFFFFF };



/**
 *******************************************************************************
 *
 * @brief
 *  Parses Transform tree syntax
 *
 * @par Description:
 *  Parses Transform tree syntax as per Section:7.3.9.8
 *
 * @param[in] ps_codec
 *  Pointer to codec context
 *
 * @returns  Status
 *
 * @remarks
 *
 *
 *******************************************************************************
 */

WORD32 ihevcd_parse_transform_tree(codec_t *ps_codec,
                                   WORD32 x0, WORD32 y0,
                                   WORD32 cu_x_base, WORD32 cu_y_base,
                                   WORD32 log2_trafo_size,
                                   WORD32 trafo_depth,
                                   WORD32 blk_idx,
                                   WORD32 intra_pred_mode)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    sps_t *ps_sps;
    pps_t *ps_pps;
    WORD32 value;
    WORD32 x1, y1;
    WORD32 max_trafo_depth;

    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    WORD32 intra_split_flag;
    WORD32 split_transform_flag;
    WORD32 ctxt_idx;
    cab_ctxt_t *ps_cabac = &ps_codec->s_parse.s_cabac;

    max_trafo_depth = ps_codec->s_parse.s_cu.i4_max_trafo_depth;
    ps_sps = ps_codec->s_parse.ps_sps;
    ps_pps = ps_codec->s_parse.ps_pps;
    intra_split_flag = ps_codec->s_parse.s_cu.i4_intra_split_flag;

    {
        split_transform_flag = 0;
        if((log2_trafo_size <= ps_sps->i1_log2_max_transform_block_size) &&
                        (log2_trafo_size > ps_sps->i1_log2_min_transform_block_size) &&
                        (trafo_depth < max_trafo_depth) &&
                        !(intra_split_flag && (trafo_depth == 0)))
        {
            /* encode the split transform flag, context derived as per Table9-37 */
            ctxt_idx = IHEVC_CAB_SPLIT_TFM + (5 - log2_trafo_size);

            TRACE_CABAC_CTXT("split_transform_flag", ps_cabac->u4_range, ctxt_idx);
            split_transform_flag = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
            AEV_TRACE("split_transform_flag", split_transform_flag,
                      ps_cabac->u4_range);

        }
        else
        {
            WORD32 inter_split_flag = 0;

            if((0 == ps_sps->i1_max_transform_hierarchy_depth_inter) &&
                            (PRED_MODE_INTER == ps_codec->s_parse.s_cu.i4_pred_mode) &&
                            (PART_2Nx2N != ps_codec->s_parse.s_cu.i4_part_mode) &&
                            (0 == trafo_depth))
            {
                inter_split_flag = 1;
            }

            if((log2_trafo_size > ps_sps->i1_log2_max_transform_block_size) ||
                            ((1 == intra_split_flag) && (0 == trafo_depth)) ||
                            (1 == inter_split_flag))
            {
                split_transform_flag = 1;
            }
        }

        if(0 == trafo_depth)
        {
            ps_codec->s_parse.s_cu.ai1_cbf_cr[trafo_depth] = 0;
            ps_codec->s_parse.s_cu.ai1_cbf_cb[trafo_depth] = 0;
        }
        else
        {
            ps_codec->s_parse.s_cu.ai1_cbf_cb[trafo_depth] = ps_codec->s_parse.s_cu.ai1_cbf_cb[trafo_depth - 1];
            ps_codec->s_parse.s_cu.ai1_cbf_cr[trafo_depth] = ps_codec->s_parse.s_cu.ai1_cbf_cr[trafo_depth - 1];
        }
        if(trafo_depth == 0 || log2_trafo_size > 2)
        {
            ctxt_idx = IHEVC_CAB_CBCR_IDX + trafo_depth;
            /* CBF for Cb/Cr is sent only if the parent CBF for Cb/Cr is non-zero */
            if((trafo_depth == 0) || ps_codec->s_parse.s_cu.ai1_cbf_cb[trafo_depth - 1])
            {
                TRACE_CABAC_CTXT("cbf_cb", ps_cabac->u4_range, ctxt_idx);
                value = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
                AEV_TRACE("cbf_cb", value, ps_cabac->u4_range);
                ps_codec->s_parse.s_cu.ai1_cbf_cb[trafo_depth] = value;
            }

            if((trafo_depth == 0) || ps_codec->s_parse.s_cu.ai1_cbf_cr[trafo_depth - 1])
            {
                TRACE_CABAC_CTXT("cbf_cr", ps_cabac->u4_range, ctxt_idx);
                value = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
                AEV_TRACE("cbf_cr", value, ps_cabac->u4_range);
                ps_codec->s_parse.s_cu.ai1_cbf_cr[trafo_depth] = value;
            }
        }
        if(split_transform_flag)
        {
            WORD32 intra_pred_mode_tmp;
            x1 = x0 + ((1 << log2_trafo_size) >> 1);
            y1 = y0 + ((1 << log2_trafo_size) >> 1);

            /* For transform depth of zero, intra pred mode as decoded at CU */
            /* level is sent to the transform tree nodes */
            /* When depth is non-zero intra pred mode of parent node is sent */
            /* This takes care of passing correct mode to all the child nodes */
            intra_pred_mode_tmp = trafo_depth ? intra_pred_mode : ps_codec->s_parse.s_cu.ai4_intra_luma_pred_mode[0];
            ret = ihevcd_parse_transform_tree(ps_codec, x0, y0, x0, y0, log2_trafo_size - 1, trafo_depth + 1, 0, intra_pred_mode_tmp);
            RETURN_IF((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret, ret);

            intra_pred_mode_tmp = trafo_depth ? intra_pred_mode : ps_codec->s_parse.s_cu.ai4_intra_luma_pred_mode[1];
            ret = ihevcd_parse_transform_tree(ps_codec, x1, y0, x0, y0, log2_trafo_size - 1, trafo_depth + 1, 1, intra_pred_mode_tmp);
            RETURN_IF((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret, ret);

            intra_pred_mode_tmp = trafo_depth ? intra_pred_mode : ps_codec->s_parse.s_cu.ai4_intra_luma_pred_mode[2];
            ret = ihevcd_parse_transform_tree(ps_codec, x0, y1, x0, y0, log2_trafo_size - 1, trafo_depth + 1, 2, intra_pred_mode_tmp);
            RETURN_IF((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret, ret);

            intra_pred_mode_tmp = trafo_depth ? intra_pred_mode : ps_codec->s_parse.s_cu.ai4_intra_luma_pred_mode[3];
            ret = ihevcd_parse_transform_tree(ps_codec, x1, y1, x0, y0, log2_trafo_size - 1, trafo_depth + 1, 3, intra_pred_mode_tmp);
            RETURN_IF((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret, ret);

        }
        else
        {
            WORD32 ctb_x_base;
            WORD32 ctb_y_base;
            WORD32 cu_qp_delta_abs;



            tu_t *ps_tu = ps_codec->s_parse.ps_tu;
            cu_qp_delta_abs = 0;
            ctb_x_base = ps_codec->s_parse.i4_ctb_x << ps_sps->i1_log2_ctb_size;
            ctb_y_base = ps_codec->s_parse.i4_ctb_y << ps_sps->i1_log2_ctb_size;

            if((ps_codec->s_parse.s_cu.i4_pred_mode == PRED_MODE_INTRA) ||
                            (trafo_depth != 0) ||
                            (ps_codec->s_parse.s_cu.ai1_cbf_cb[trafo_depth]) ||
                            (ps_codec->s_parse.s_cu.ai1_cbf_cr[trafo_depth]))
            {
                ctxt_idx = IHEVC_CAB_CBF_LUMA_IDX;
                ctxt_idx += (trafo_depth == 0) ? 1 : 0;

                TRACE_CABAC_CTXT("cbf_luma", ps_cabac->u4_range, ctxt_idx);
                value = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
                AEV_TRACE("cbf_luma", value, ps_cabac->u4_range);

                ps_codec->s_parse.s_cu.i1_cbf_luma = value;
            }
            else
            {
                ps_codec->s_parse.s_cu.i1_cbf_luma = 1;
            }

            /* Initialize ps_tu to default values */
            /* If required change this to WORD32 packed write */
            ps_tu->b1_cb_cbf = 0;
            ps_tu->b1_cr_cbf = 0;
            ps_tu->b1_y_cbf = 0;
            ps_tu->b4_pos_x = ((x0 - ctb_x_base) >> 2);
            ps_tu->b4_pos_y = ((y0 - ctb_y_base) >> 2);
            ps_tu->b1_transquant_bypass = ps_codec->s_parse.s_cu.i4_cu_transquant_bypass;
            ps_tu->b3_size = (log2_trafo_size - 2);
            ps_tu->b7_qp = ps_codec->s_parse.u4_qp;

            ps_tu->b6_luma_intra_mode = intra_pred_mode;
            ps_tu->b3_chroma_intra_mode_idx = ps_codec->s_parse.s_cu.i4_intra_chroma_pred_mode_idx;

            /* Section:7.3.12  Transform unit syntax inlined here */
            if(ps_codec->s_parse.s_cu.i1_cbf_luma ||
                            ps_codec->s_parse.s_cu.ai1_cbf_cb[trafo_depth] ||
                            ps_codec->s_parse.s_cu.ai1_cbf_cr[trafo_depth])
            {
                WORD32 intra_pred_mode_chroma;
                if(ps_pps->i1_cu_qp_delta_enabled_flag && !ps_codec->s_parse.i4_is_cu_qp_delta_coded)
                {


                    WORD32 c_max        = TU_MAX_QP_DELTA_ABS;
                    WORD32 ctxt_inc     = IHEVC_CAB_QP_DELTA_ABS;
                    WORD32 ctxt_inc_max = CTXT_MAX_QP_DELTA_ABS;

                    TRACE_CABAC_CTXT("cu_qp_delta_abs", ps_cabac->u4_range, ctxt_inc);
                    /* qp_delta_abs is coded as combination of tunary and eg0 code  */
                    /* See Table 9-32 and Table 9-37 for details on cu_qp_delta_abs */
                    cu_qp_delta_abs = ihevcd_cabac_decode_bins_tunary(ps_cabac,
                                                                      ps_bitstrm,
                                                                      c_max,
                                                                      ctxt_inc,
                                                                      0,
                                                                      ctxt_inc_max);
                    if(cu_qp_delta_abs >= c_max)
                    {
                        value = ihevcd_cabac_decode_bypass_bins_egk(ps_cabac, ps_bitstrm, 0);
                        cu_qp_delta_abs += value;
                    }
                    AEV_TRACE("cu_qp_delta_abs", cu_qp_delta_abs, ps_cabac->u4_range);

                    ps_codec->s_parse.i4_is_cu_qp_delta_coded = 1;


                    if(cu_qp_delta_abs)
                    {
                        value = ihevcd_cabac_decode_bypass_bin(ps_cabac, ps_bitstrm);
                        AEV_TRACE("cu_qp_delta_sign", value, ps_cabac->u4_range);

                        if(value)
                            cu_qp_delta_abs = -cu_qp_delta_abs;

                    }

                    if (cu_qp_delta_abs < MIN_CU_QP_DELTA_ABS(ps_sps->i1_bit_depth_luma_minus8)
                                    || cu_qp_delta_abs > MAX_CU_QP_DELTA_ABS(ps_sps->i1_bit_depth_luma_minus8))
                    {
                        return IHEVCD_INVALID_PARAMETER;
                    }

                    ps_codec->s_parse.s_cu.i4_cu_qp_delta = cu_qp_delta_abs;

                }

                if(ps_codec->s_parse.s_cu.i1_cbf_luma)
                {
                    ps_tu->b1_y_cbf = 1;
                    ihevcd_parse_residual_coding(ps_codec, x0, y0, log2_trafo_size, 0, intra_pred_mode);
                }

                if(4 == ps_codec->s_parse.s_cu.i4_intra_chroma_pred_mode_idx)
                    intra_pred_mode_chroma = ps_codec->s_parse.s_cu.ai4_intra_luma_pred_mode[0];
                else
                {
                    intra_pred_mode_chroma = gau1_intra_pred_chroma_modes[ps_codec->s_parse.s_cu.i4_intra_chroma_pred_mode_idx];

                    if(intra_pred_mode_chroma ==
                                    ps_codec->s_parse.s_cu.ai4_intra_luma_pred_mode[0])
                    {
                        intra_pred_mode_chroma = INTRA_ANGULAR(34);
                    }

                }
                if(log2_trafo_size > 2)
                {
                    if(ps_codec->s_parse.s_cu.ai1_cbf_cb[trafo_depth])
                    {
                        ps_tu->b1_cb_cbf = 1;
                        ihevcd_parse_residual_coding(ps_codec, x0, y0, log2_trafo_size - 1, 1, intra_pred_mode_chroma);
                    }

                    if(ps_codec->s_parse.s_cu.ai1_cbf_cr[trafo_depth])
                    {
                        ps_tu->b1_cr_cbf = 1;
                        ihevcd_parse_residual_coding(ps_codec, x0, y0, log2_trafo_size - 1, 2, intra_pred_mode_chroma);
                    }
                }
                else if(blk_idx == 3)
                {
                    if(ps_codec->s_parse.s_cu.ai1_cbf_cb[trafo_depth])
                    {
                        ps_tu->b1_cb_cbf = 1;
                        ihevcd_parse_residual_coding(ps_codec, cu_x_base, cu_y_base, log2_trafo_size, 1, intra_pred_mode_chroma);
                    }

                    if(ps_codec->s_parse.s_cu.ai1_cbf_cr[trafo_depth])
                    {
                        ps_tu->b1_cr_cbf = 1;
                        ihevcd_parse_residual_coding(ps_codec, cu_x_base, cu_y_base, log2_trafo_size, 2, intra_pred_mode_chroma);
                    }
                }
                else
                {
                    //ps_tu->b1_chroma_present = 0;
                    ps_tu->b3_chroma_intra_mode_idx = INTRA_PRED_CHROMA_IDX_NONE;
                }
            }
            else
            {
                if((3 != blk_idx) && (2 == log2_trafo_size))
                {
                    ps_tu->b3_chroma_intra_mode_idx = INTRA_PRED_CHROMA_IDX_NONE;
                }
            }

            /* Set the first TU in CU flag */
            {
                if((ps_codec->s_parse.s_cu.i4_pos_x << 3) == (ps_tu->b4_pos_x << 2) &&
                                (ps_codec->s_parse.s_cu.i4_pos_y << 3) == (ps_tu->b4_pos_y << 2))
                {
                    ps_tu->b1_first_tu_in_cu = 1;
                }
                else
                {
                    ps_tu->b1_first_tu_in_cu = 0;
                }
            }
            ps_codec->s_parse.ps_tu++;
            ps_codec->s_parse.s_cu.i4_tu_cnt++;
            ps_codec->s_parse.i4_pic_tu_idx++;
        }
    }
    return ret;
}
/**
 *******************************************************************************
 *
 * @brief
 *  Parses Motion vector difference
 *
 * @par Description:
 *  Parses Motion vector difference as per Section:7.3.9.9
 *
 * @param[in] ps_codec
 *  Pointer to codec context
 *
 * @returns  Error from IHEVCD_ERROR_T
 *
 * @remarks
 *
 *
 *******************************************************************************
 */
IHEVCD_ERROR_T ihevcd_parse_mvd(codec_t *ps_codec, mv_t *ps_mv)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 value;
    WORD32 abs_mvd;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    WORD32 abs_mvd_greater0_flag[2];
    WORD32 abs_mvd_greater1_flag[2];
    WORD32 ctxt_idx;
    cab_ctxt_t *ps_cabac = &ps_codec->s_parse.s_cabac;


    ctxt_idx  = IHEVC_CAB_MVD_GRT0;
    /* encode absmvd_x > 0 */
    TRACE_CABAC_CTXT("abs_mvd_greater0_flag[0]", ps_cabac->u4_range, ctxt_idx);
    abs_mvd_greater0_flag[0] = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
    AEV_TRACE("abs_mvd_greater0_flag[0]", abs_mvd_greater0_flag[0], ps_cabac->u4_range);

    /* encode absmvd_y > 0 */
    TRACE_CABAC_CTXT("abs_mvd_greater0_flag[1]", ps_cabac->u4_range, ctxt_idx);
    abs_mvd_greater0_flag[1] = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
    AEV_TRACE("abs_mvd_greater0_flag[1]", abs_mvd_greater0_flag[1], ps_cabac->u4_range);

    ctxt_idx  = IHEVC_CAB_MVD_GRT1;
    abs_mvd_greater1_flag[0] = 0;
    abs_mvd_greater1_flag[1] = 0;

    if(abs_mvd_greater0_flag[0])
    {
        TRACE_CABAC_CTXT("abs_mvd_greater1_flag[0]", ps_cabac->u4_range, ctxt_idx);
        abs_mvd_greater1_flag[0] = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
        AEV_TRACE("abs_mvd_greater1_flag[0]", abs_mvd_greater1_flag[0], ps_cabac->u4_range);
    }
    if(abs_mvd_greater0_flag[1])
    {
        TRACE_CABAC_CTXT("abs_mvd_greater1_flag[1]", ps_cabac->u4_range, ctxt_idx);
        abs_mvd_greater1_flag[1] = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
        AEV_TRACE("abs_mvd_greater1_flag[1]", abs_mvd_greater1_flag[1], ps_cabac->u4_range);
    }
    abs_mvd = 0;
    if(abs_mvd_greater0_flag[0])
    {
        abs_mvd = 1;
        if(abs_mvd_greater1_flag[0])
        {
            value = ihevcd_cabac_decode_bypass_bins_egk(ps_cabac, ps_bitstrm, 1);
            AEV_TRACE("abs_mvd_minus2[0]", value, ps_cabac->u4_range);
            abs_mvd = value + 2;
        }
        value = ihevcd_cabac_decode_bypass_bin(ps_cabac, ps_bitstrm);
        AEV_TRACE("mvd_sign_flag[0]", value, ps_cabac->u4_range);
        if(value)
        {
            abs_mvd = -abs_mvd;
        }

    }
    ps_mv->i2_mvx = abs_mvd;
    abs_mvd = 0;
    if(abs_mvd_greater0_flag[1])
    {
        abs_mvd = 1;
        if(abs_mvd_greater1_flag[1])
        {
            value = ihevcd_cabac_decode_bypass_bins_egk(ps_cabac, ps_bitstrm, 1);
            AEV_TRACE("abs_mvd_minus2[1]", value, ps_cabac->u4_range);
            abs_mvd = value + 2;

        }
        value = ihevcd_cabac_decode_bypass_bin(ps_cabac, ps_bitstrm);
        AEV_TRACE("mvd_sign_flag[1]", value, ps_cabac->u4_range);

        if(value)
        {
            abs_mvd = -abs_mvd;
        }
    }
    ps_mv->i2_mvy = abs_mvd;

    return ret;
}

/**
 *******************************************************************************
 *
 * @brief
 *  Parses PCM sample
 *
 *
 * @par Description:
 *  Parses PCM sample as per Section:7.3.9.7 Pcm sample syntax
 *
 * @param[in] ps_codec
 *  Pointer to codec context
 *
 * @returns  Error from IHEVCD_ERROR_T
 *
 * @remarks
 *
 *
 *******************************************************************************
 */

IHEVCD_ERROR_T  ihevcd_parse_pcm_sample(codec_t *ps_codec,
                                        WORD32 x0,
                                        WORD32 y0,
                                        WORD32 log2_cb_size)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    cab_ctxt_t *ps_cabac = &ps_codec->s_parse.s_cabac;
    sps_t *ps_sps;

    WORD32 value;
    WORD32 i;

    WORD32 num_bits;
    UWORD32 u4_sig_coeff_map;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    tu_t *ps_tu = ps_codec->s_parse.ps_tu;
    tu_sblk_coeff_data_t *ps_tu_sblk_coeff_data;
    UWORD8 *pu1_coeff_data;
    ps_sps = ps_codec->s_parse.ps_sps;

    UNUSED(value);
    UNUSED(ps_tu);
    UNUSED(ps_cabac);
    UNUSED(x0);
    UNUSED(y0);

    {
        WORD8 *pi1_scan_idx;
        WORD8 *pi1_buf = (WORD8 *)ps_codec->s_parse.pv_tu_coeff_data;
        WORD8 *pi1_num_coded_subblks;

        /* First WORD8 gives number of coded subblocks */
        pi1_num_coded_subblks = pi1_buf++;

        /* Set number of coded subblocks in the current TU to zero */
        /* For PCM there will be only one subblock which is the same size as CU */
        *pi1_num_coded_subblks = 1;

        /* Second WORD8 gives (scan idx << 1) | trans_skip */
        pi1_scan_idx = pi1_buf++;
        *pi1_scan_idx = (0 << 1) | 1;

        /* Store the incremented pointer in pv_tu_coeff_data */
        ps_codec->s_parse.pv_tu_coeff_data = pi1_buf;

    }

    u4_sig_coeff_map = 0xFFFFFFFF;
    ps_tu_sblk_coeff_data = (tu_sblk_coeff_data_t *)ps_codec->s_parse.pv_tu_coeff_data;
    ps_tu_sblk_coeff_data->u2_sig_coeff_map = u4_sig_coeff_map;
    ps_tu_sblk_coeff_data->u2_subblk_pos = 0;

    pu1_coeff_data = (UWORD8 *)&ps_tu_sblk_coeff_data->ai2_level[0];

    num_bits = ps_sps->i1_pcm_sample_bit_depth_luma;

    for(i = 0; i < 1 << (log2_cb_size << 1); i++)
    {
        TRACE_CABAC_CTXT("pcm_sample_luma", ps_cabac->u4_range, 0);
        BITS_PARSE("pcm_sample_luma", value, ps_bitstrm, num_bits);

        //ps_pcmsample_t->i1_pcm_sample_luma[i] = value;
        *pu1_coeff_data++ = value << (BIT_DEPTH_LUMA - num_bits);
    }

    num_bits = ps_sps->i1_pcm_sample_bit_depth_chroma;

    for(i = 0; i < (1 << (log2_cb_size << 1)) >> 1; i++)
    {
        TRACE_CABAC_CTXT("pcm_sample_chroma", ps_cabac->u4_range, 0);
        BITS_PARSE("pcm_sample_chroma", value, ps_bitstrm, num_bits);

        // ps_pcmsample_t->i1_pcm_sample_chroma[i] = value;
        *pu1_coeff_data++ = value << (BIT_DEPTH_CHROMA - num_bits);
    }

    ps_codec->s_parse.pv_tu_coeff_data = pu1_coeff_data;

    return ret;
}
/**
 *******************************************************************************
 *
 * @brief
 *  Parses Prediction unit
 *
 * @par Description:
 *  Parses Prediction unit as per Section:7.3.9.6
 *
 * @param[in] ps_codec
 *  Pointer to codec context
 *
 * @returns  Error from IHEVCD_ERROR_T
 *
 * @remarks
 *
 *
 *******************************************************************************
 */

IHEVCD_ERROR_T  ihevcd_parse_pu_mvp(codec_t *ps_codec, pu_t *ps_pu)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 value;
    slice_header_t *ps_slice_hdr;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    cab_ctxt_t *ps_cabac = &ps_codec->s_parse.s_cabac;
    WORD32 inter_pred_idc;

    ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr;

    if(ps_slice_hdr->i1_slice_type == BSLICE)
    {
        WORD32 pu_w_plus_pu_h;
        WORD32 ctxt_idx;
        /* required to check if w+h==12 case */
        pu_w_plus_pu_h = ((ps_pu->b4_wd + 1) << 2) + ((ps_pu->b4_ht + 1) << 2);
        if(12 == pu_w_plus_pu_h)
        {
            ctxt_idx = IHEVC_CAB_INTER_PRED_IDC + 4;
            TRACE_CABAC_CTXT("inter_pred_idc", ps_cabac->u4_range, ctxt_idx);
            inter_pred_idc = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm,
                                                     ctxt_idx);
        }
        else
        {
            /* larger PUs can be encoded as bi_pred/l0/l1 inter_pred_idc */
            WORD32 is_bipred;

            ctxt_idx = IHEVC_CAB_INTER_PRED_IDC + ps_codec->s_parse.i4_ct_depth;
            TRACE_CABAC_CTXT("inter_pred_idc", ps_cabac->u4_range, ctxt_idx);
            is_bipred = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
            inter_pred_idc = PRED_BI;
            if(!is_bipred)
            {
                ctxt_idx = IHEVC_CAB_INTER_PRED_IDC + 4;
                inter_pred_idc = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm,
                                                         ctxt_idx);
            }
        }

        AEV_TRACE("inter_pred_idc", inter_pred_idc, ps_cabac->u4_range);
    }
    else
        inter_pred_idc = PRED_L0;
    ps_pu->mv.i1_l0_ref_idx = 0;
    ps_pu->mv.i1_l1_ref_idx = 0;
    /* Decode MVD for L0 for PRED_L0 or PRED_BI */
    if(inter_pred_idc != PRED_L1)
    {
        WORD32 active_refs = ps_slice_hdr->i1_num_ref_idx_l0_active;
        WORD32 ref_idx = 0;
        WORD32 ctxt_idx;

        if(active_refs > 1)
        {
            ctxt_idx = IHEVC_CAB_INTER_REF_IDX;
            /* encode the context modelled first bin */
            TRACE_CABAC_CTXT("ref_idx", ps_cabac->u4_range, ctxt_idx);
            ref_idx = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);

            if((active_refs > 2) && ref_idx)
            {
                WORD32 value;
                /* encode the context modelled second bin */
                ctxt_idx++;
                value = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
                ref_idx += value;
                if((active_refs > 3) && value)
                {
                    /* encode remaining bypass bins */
                    ref_idx = ihevcd_cabac_decode_bypass_bins_tunary(ps_cabac,
                                                                     ps_bitstrm,
                                                                     (active_refs - 3)
                    );
                    ref_idx += 2;
                }
            }
            AEV_TRACE("ref_idx", ref_idx, ps_cabac->u4_range);
        }

        ref_idx = CLIP3(ref_idx, 0, MAX_DPB_SIZE - 1);
        ps_pu->mv.i1_l0_ref_idx = ref_idx;

        ihevcd_parse_mvd(ps_codec, &ps_pu->mv.s_l0_mv);

        ctxt_idx = IHEVC_CAB_MVP_L0L1;
        value = ihevcd_cabac_decode_bin(ps_cabac,
                                        ps_bitstrm,
                                        ctxt_idx);

        AEV_TRACE("mvp_l0/l1_flag", value, ps_cabac->u4_range);

        ps_pu->b1_l0_mvp_idx = value;

    }
    /* Decode MVD for L1 for PRED_L1 or PRED_BI */
    if(inter_pred_idc != PRED_L0)
    {
        WORD32 active_refs = ps_slice_hdr->i1_num_ref_idx_l1_active;
        WORD32 ref_idx = 0;
        WORD32 ctxt_idx;

        if(active_refs > 1)
        {

            ctxt_idx = IHEVC_CAB_INTER_REF_IDX;
            TRACE_CABAC_CTXT("ref_idx", ps_cabac->u4_range, ctxt_idx);
            /* encode the context modelled first bin */
            ref_idx = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);

            if((active_refs > 2) && ref_idx)
            {
                WORD32 value;
                /* encode the context modelled second bin */
                ctxt_idx++;
                value = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
                ref_idx += value;
                if((active_refs > 3) && value)
                {
                    /* encode remaining bypass bins */
                    ref_idx = ihevcd_cabac_decode_bypass_bins_tunary(ps_cabac,
                                                                     ps_bitstrm,
                                                                     (active_refs - 3)
                    );
                    ref_idx += 2;
                }
            }

            AEV_TRACE("ref_idx", ref_idx, ps_cabac->u4_range);
        }

        ref_idx = CLIP3(ref_idx, 0, MAX_DPB_SIZE - 1);
        ps_pu->mv.i1_l1_ref_idx = ref_idx;

        if(ps_slice_hdr->i1_mvd_l1_zero_flag && inter_pred_idc == PRED_BI)
        {
            ps_pu->mv.s_l1_mv.i2_mvx = 0;
            ps_pu->mv.s_l1_mv.i2_mvy = 0;
        }
        else
        {
            ihevcd_parse_mvd(ps_codec, &ps_pu->mv.s_l1_mv);
        }

        ctxt_idx = IHEVC_CAB_MVP_L0L1;
        value = ihevcd_cabac_decode_bin(ps_cabac,
                                        ps_bitstrm,
                                        ctxt_idx);

        AEV_TRACE("mvp_l0/l1_flag", value, ps_cabac->u4_range);
        ps_pu->b1_l1_mvp_idx = value;

    }

    ps_pu->b2_pred_mode = inter_pred_idc;
    return ret;
}
/**
 *******************************************************************************
 *
 * @brief
 *  Parses Prediction unit
 *
 * @par Description:
 *  Parses Prediction unit as per Section:7.3.9.6
 *
 * @param[in] ps_codec
 *  Pointer to codec context
 *
 * @returns  Error from IHEVCD_ERROR_T
 *
 * @remarks
 *
 *
 *******************************************************************************
 */

IHEVCD_ERROR_T  ihevcd_parse_prediction_unit(codec_t *ps_codec,
                                             WORD32 x0,
                                             WORD32 y0,
                                             WORD32 wd,
                                             WORD32 ht)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    slice_header_t *ps_slice_hdr;
    sps_t *ps_sps;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    WORD32 ctb_x_base;
    WORD32 ctb_y_base;

    pu_t *ps_pu = ps_codec->s_parse.ps_pu;
    cab_ctxt_t *ps_cabac = &ps_codec->s_parse.s_cabac;

    ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr;

    /* Set PU structure to default values */
    memset(ps_pu, 0, sizeof(pu_t));

    ps_sps = ps_codec->s_parse.ps_sps;
    ctb_x_base = ps_codec->s_parse.i4_ctb_x << ps_sps->i1_log2_ctb_size;
    ctb_y_base = ps_codec->s_parse.i4_ctb_y << ps_sps->i1_log2_ctb_size;

    ps_pu->b4_pos_x = (x0 - ctb_x_base) >> 2;
    ps_pu->b4_pos_y = (y0 - ctb_y_base) >> 2;
    ps_pu->b4_wd = (wd >> 2) - 1;
    ps_pu->b4_ht = (ht >> 2) - 1;

    ps_pu->b1_intra_flag = 0;
    ps_pu->b3_part_mode = ps_codec->s_parse.s_cu.i4_part_mode;

    if(PRED_MODE_SKIP == ps_codec->s_parse.s_cu.i4_pred_mode)
    {
        WORD32 merge_idx = 0;
        if(ps_slice_hdr->i1_max_num_merge_cand > 1)
        {
            WORD32 ctxt_idx = IHEVC_CAB_MERGE_IDX_EXT;
            WORD32 bin;

            TRACE_CABAC_CTXT("merge_idx", ps_cabac->u4_range, ctxt_idx);
            bin = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
            if(bin)
            {
                if(ps_slice_hdr->i1_max_num_merge_cand > 2)
                {
                    merge_idx = ihevcd_cabac_decode_bypass_bins_tunary(
                                    ps_cabac, ps_bitstrm,
                                    (ps_slice_hdr->i1_max_num_merge_cand - 2));
                }
                merge_idx++;
            }
            AEV_TRACE("merge_idx", merge_idx, ps_cabac->u4_range);
        }
        ps_pu->b1_merge_flag = 1;
        ps_pu->b3_merge_idx = merge_idx;

    }
    else
    {
        /* MODE_INTER */
        WORD32 merge_flag;
        WORD32 ctxt_idx = IHEVC_CAB_MERGE_FLAG_EXT;
        TRACE_CABAC_CTXT("merge_flag", ps_cabac->u4_range, ctxt_idx);
        merge_flag = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
        AEV_TRACE("merge_flag", merge_flag, ps_cabac->u4_range);

        ps_pu->b1_merge_flag = merge_flag;

        if(merge_flag)
        {
            WORD32 merge_idx = 0;
            if(ps_slice_hdr->i1_max_num_merge_cand > 1)
            {
                WORD32 ctxt_idx = IHEVC_CAB_MERGE_IDX_EXT;
                WORD32 bin;
                TRACE_CABAC_CTXT("merge_idx", ps_cabac->u4_range, ctxt_idx);
                bin = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
                if(bin)
                {
                    if(ps_slice_hdr->i1_max_num_merge_cand > 2)
                    {
                        merge_idx = ihevcd_cabac_decode_bypass_bins_tunary(
                                        ps_cabac, ps_bitstrm,
                                        (ps_slice_hdr->i1_max_num_merge_cand - 2));
                    }
                    merge_idx++;
                }
                AEV_TRACE("merge_idx", merge_idx, ps_cabac->u4_range);
            }

            ps_pu->b3_merge_idx = merge_idx;
        }
        else
        {
            ihevcd_parse_pu_mvp(ps_codec, ps_pu);
        }

    }
    STATS_UPDATE_PU_SIZE(ps_pu);
    /* Increment PU pointer */
    ps_codec->s_parse.ps_pu++;
    ps_codec->s_parse.i4_pic_pu_idx++;
    return ret;
}


WORD32 ihevcd_parse_part_mode_amp(cab_ctxt_t *ps_cabac, bitstrm_t *ps_bitstrm)
{
    WORD32 ctxt_idx = IHEVC_CAB_PART_MODE;
    WORD32 part_mode_idx;
    WORD32 part_mode;
    WORD32 bin;

    part_mode = 0;
    TRACE_CABAC_CTXT("part_mode", ps_cabac->u4_range, ctxt_idx);
    bin = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx++);

    if(!bin)
    {
        bin = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx++);
        part_mode_idx = bin;
        part_mode_idx <<= 1;

        /* Following takes of handling context increment for 3rd bin in part_mode */
        /* When AMP is enabled and the current is not min CB */
        /* Context for 3rd bin is 3 and not 2 */
        ctxt_idx += 1;

        bin = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
        part_mode_idx |= bin;

        part_mode_idx <<= 1;
        if(!bin)
        {

            bin = ihevcd_cabac_decode_bypass_bin(ps_cabac, ps_bitstrm);
            part_mode_idx |= bin;
        }
        part_mode = gau1_part_mode_amp[part_mode_idx];
    }
    return part_mode;
}
IHEVCD_ERROR_T ihevcd_parse_coding_unit_intra(codec_t *ps_codec,
                                              WORD32 x0,
                                              WORD32 y0,
                                              WORD32 log2_cb_size)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    sps_t *ps_sps;
    cab_ctxt_t *ps_cabac = &ps_codec->s_parse.s_cabac;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    WORD32 pcm_flag;
    WORD32 value;
    WORD32 cb_size = 1 << log2_cb_size;
    WORD32 part_mode =  ps_codec->s_parse.s_cu.i4_part_mode;
    tu_t *ps_tu = ps_codec->s_parse.ps_tu;
    pu_t *ps_pu = ps_codec->s_parse.ps_pu;
    WORD32 ctb_x_base;
    WORD32 ctb_y_base;
    ps_sps = ps_codec->s_parse.ps_sps;
    ctb_x_base = ps_codec->s_parse.i4_ctb_x << ps_sps->i1_log2_ctb_size;
    ctb_y_base = ps_codec->s_parse.i4_ctb_y << ps_sps->i1_log2_ctb_size;

    memset(ps_pu, 0, sizeof(pu_t));
    ps_pu->b1_intra_flag = 1;
    ps_pu->b4_wd = (cb_size >> 2) - 1;
    ps_pu->b4_ht = (cb_size >> 2) - 1;
    ps_pu->b4_pos_x = (x0 - ctb_x_base) >> 2;
    ps_pu->b4_pos_y = (y0 - ctb_y_base) >> 2;

    pcm_flag = 0;
    if((PART_2Nx2N == part_mode) && (ps_sps->i1_pcm_enabled_flag)
                    && (log2_cb_size
                                    >= ps_sps->i1_log2_min_pcm_coding_block_size)
                    && (log2_cb_size
                                    <= (ps_sps->i1_log2_min_pcm_coding_block_size + ps_sps->i1_log2_diff_max_min_pcm_coding_block_size)))
    {
        TRACE_CABAC_CTXT("pcm_flag", ps_cabac->u4_range, 0);
        pcm_flag = ihevcd_cabac_decode_terminate(ps_cabac, ps_bitstrm);
        AEV_TRACE("pcm_flag", pcm_flag, ps_cabac->u4_range);
    }

    ps_codec->s_parse.i4_cu_pcm_flag = pcm_flag;
    if(pcm_flag)
    {
        UWORD8 *pu1_luma_intra_pred_mode_top, *pu1_luma_intra_pred_mode_left;
        WORD32 i,  num_pred_blocks;

        if(ps_codec->s_parse.s_bitstrm.u4_bit_ofst % 8)
        {
            TRACE_CABAC_CTXT("pcm_alignment_zero_bit", ps_cabac->u4_range, 0);
            ihevcd_bits_flush_to_byte_boundary(&ps_codec->s_parse.s_bitstrm);
            AEV_TRACE("pcm_alignment_zero_bit", 0, ps_cabac->u4_range);
        }

        ihevcd_parse_pcm_sample(ps_codec, x0, y0, log2_cb_size);

        ihevcd_cabac_reset(&ps_codec->s_parse.s_cabac,
                           &ps_codec->s_parse.s_bitstrm);

        ps_tu = ps_codec->s_parse.ps_tu;
        ps_tu->b1_cb_cbf = 1;
        ps_tu->b1_cr_cbf = 1;
        ps_tu->b1_y_cbf = 1;
        ps_tu->b4_pos_x = ((x0 - ctb_x_base) >> 2);
        ps_tu->b4_pos_y = ((y0 - ctb_y_base) >> 2);
        ps_tu->b1_transquant_bypass = 1;
        ps_tu->b3_size = (log2_cb_size - 2);
        ps_tu->b7_qp = ps_codec->s_parse.u4_qp;
        ps_tu->b3_chroma_intra_mode_idx = INTRA_PRED_CHROMA_IDX_NONE;
        ps_tu->b6_luma_intra_mode   = INTRA_PRED_NONE;

        /* Set the first TU in CU flag */
        {
            if((ps_codec->s_parse.s_cu.i4_pos_x << 3) == (ps_tu->b4_pos_x << 2) &&
                            (ps_codec->s_parse.s_cu.i4_pos_y << 3) == (ps_tu->b4_pos_y << 2))
            {
                ps_tu->b1_first_tu_in_cu = 1;
            }
            else
            {
                ps_tu->b1_first_tu_in_cu = 0;
            }
        }

        /* Update the intra pred mode for PCM to INTRA_DC(default mode) */
        pu1_luma_intra_pred_mode_top = ps_codec->s_parse.pu1_luma_intra_pred_mode_top
                        + (ps_codec->s_parse.s_cu.i4_pos_x * 2);

        pu1_luma_intra_pred_mode_left = ps_codec->s_parse.pu1_luma_intra_pred_mode_left
                        + (ps_codec->s_parse.s_cu.i4_pos_y * 2);

        num_pred_blocks = 1; /* Because PCM part mode will be 2Nx2N */

        ps_codec->s_func_selector.ihevc_memset_fptr(pu1_luma_intra_pred_mode_left, INTRA_DC, (cb_size / num_pred_blocks) / MIN_PU_SIZE);
        ps_codec->s_func_selector.ihevc_memset_fptr(pu1_luma_intra_pred_mode_top, INTRA_DC, (cb_size / num_pred_blocks) / MIN_PU_SIZE);


        /* Set no_loop_filter appropriately */
        if(1 == ps_sps->i1_pcm_loop_filter_disable_flag)
        {
            UWORD8 *pu1_pic_no_loop_filter_flag;
            WORD32 numbytes_row;
            UWORD32 u4_mask;

            pu1_pic_no_loop_filter_flag = ps_codec->s_parse.pu1_pic_no_loop_filter_flag;
            numbytes_row =  (ps_sps->i2_pic_width_in_luma_samples + 63) / 64;
            pu1_pic_no_loop_filter_flag += (y0 / 8) * numbytes_row;
            pu1_pic_no_loop_filter_flag += (x0 / 64);
            /* Generate (cb_size / 8) number of 1s */
            /* i.e (log2_cb_size - 2) number of 1s */
            u4_mask = LSB_ONES((cb_size >> 3));
            for(i = 0; i < (cb_size / 8); i++)
            {
                *pu1_pic_no_loop_filter_flag |= (u4_mask << (((x0) / 8) % 8));
                pu1_pic_no_loop_filter_flag += numbytes_row;
            }
        }
        /* Increment ps_tu and tu_idx */
        ps_codec->s_parse.ps_tu++;
        ps_codec->s_parse.s_cu.i4_tu_cnt++;
        ps_codec->s_parse.i4_pic_tu_idx++;

    }
    else
    {
        WORD32 cnt = 0;
        WORD32 i;
        WORD32 part_cnt;

        part_cnt = (part_mode == PART_NxN) ? 4 : 1;

        for(i = 0; i < part_cnt; i++)
        {
            TRACE_CABAC_CTXT("prev_intra_pred_luma_flag", ps_cabac->u4_range, IHEVC_CAB_INTRA_LUMA_PRED_FLAG);
            value = ihevcd_cabac_decode_bin(ps_cabac,
                                            ps_bitstrm,
                                            IHEVC_CAB_INTRA_LUMA_PRED_FLAG);

            ps_codec->s_parse.s_cu.ai4_prev_intra_luma_pred_flag[i] =
                            value;
            AEV_TRACE("prev_intra_pred_luma_flag", value, ps_cabac->u4_range);
        }

        for(i = 0; i < part_cnt; i++)
        {
            if(ps_codec->s_parse.s_cu.ai4_prev_intra_luma_pred_flag[cnt])
            {
                value = ihevcd_cabac_decode_bypass_bins_tunary(ps_cabac, ps_bitstrm, 2);
                AEV_TRACE("mpm_idx", value, ps_cabac->u4_range);
                ps_codec->s_parse.s_cu.ai4_mpm_idx[cnt] = value;
            }
            else
            {
                value = ihevcd_cabac_decode_bypass_bins(ps_cabac, ps_bitstrm, 5);
                AEV_TRACE("rem_intra_luma_pred_mode", value,
                          ps_cabac->u4_range);
                ps_codec->s_parse.s_cu.ai4_rem_intra_luma_pred_mode[cnt] =
                                value;
            }
            cnt++;
        }
        TRACE_CABAC_CTXT("intra_chroma_pred_mode", ps_cabac->u4_range, IHEVC_CAB_CHROMA_PRED_MODE);
        value = ihevcd_cabac_decode_bin(ps_cabac,
                                        ps_bitstrm,
                                        IHEVC_CAB_CHROMA_PRED_MODE);
        ps_codec->s_parse.s_cu.i4_intra_chroma_pred_mode_idx = 4;
        if(value)
        {
            ps_codec->s_parse.s_cu.i4_intra_chroma_pred_mode_idx =
                            ihevcd_cabac_decode_bypass_bins(ps_cabac,
                                                            ps_bitstrm, 2);
        }
        AEV_TRACE("intra_chroma_pred_mode",
                  ps_codec->s_parse.s_cu.i4_intra_chroma_pred_mode_idx,
                  ps_cabac->u4_range);


        ihevcd_intra_pred_mode_prediction(ps_codec, log2_cb_size, x0, y0);
    }
    STATS_UPDATE_PU_SIZE(ps_pu);
    /* Increment PU pointer */
    ps_codec->s_parse.ps_pu++;
    ps_codec->s_parse.i4_pic_pu_idx++;

    return ret;
}
/**
 *******************************************************************************
 *
 * @brief
 *  Parses coding unit
 *
 * @par Description:
 *  Parses coding unit as per Section:7.3.9.5
 *
 * @param[in] ps_codec
 *  Pointer to codec context
 *
 * @returns  Error from IHEVCD_ERROR_T
 *
 * @remarks
 *
 *
 *******************************************************************************
 */

IHEVCD_ERROR_T  ihevcd_parse_coding_unit(codec_t *ps_codec,
                                         WORD32 x0,
                                         WORD32 y0,
                                         WORD32 log2_cb_size)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    sps_t *ps_sps;
    pps_t *ps_pps;
    WORD32 cb_size;
    slice_header_t *ps_slice_hdr;
    WORD32 skip_flag;
    WORD32 pcm_flag;
    UWORD32 *pu4_skip_top = ps_codec->s_parse.pu4_skip_cu_top;
    UWORD32 u4_skip_left = ps_codec->s_parse.u4_skip_cu_left;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    tu_t *ps_tu = ps_codec->s_parse.ps_tu;

    WORD32 cu_pos_x;
    WORD32 cu_pos_y;
    cab_ctxt_t *ps_cabac = &ps_codec->s_parse.s_cabac;

    ASSERT(0 == (x0 % 8));
    ASSERT(0 == (y0 % 8));

    ps_codec->s_parse.s_cu.i4_tu_cnt = 0;
    ps_sps = ps_codec->s_parse.ps_sps;
    ps_pps = ps_codec->s_parse.ps_pps;

    cu_pos_x = ps_codec->s_parse.s_cu.i4_pos_x;
    cu_pos_y = ps_codec->s_parse.s_cu.i4_pos_y;



    ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr;


    cb_size = 1 << log2_cb_size;

    ps_codec->s_parse.s_cu.i4_cu_transquant_bypass = 0;

    if(ps_pps->i1_transquant_bypass_enable_flag)
    {
        TRACE_CABAC_CTXT("cu_transquant_bypass_flag", ps_cabac->u4_range, IHEVC_CAB_CU_TQ_BYPASS_FLAG);
        ps_codec->s_parse.s_cu.i4_cu_transquant_bypass =
                        ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm,
                                                IHEVC_CAB_CU_TQ_BYPASS_FLAG);
        /* Update transquant_bypass in ps_tu */

        AEV_TRACE("cu_transquant_bypass_flag", ps_codec->s_parse.s_cu.i4_cu_transquant_bypass,
                  ps_cabac->u4_range);

        if(ps_codec->s_parse.s_cu.i4_cu_transquant_bypass)
        {
            UWORD8 *pu1_pic_no_loop_filter_flag = ps_codec->s_parse.pu1_pic_no_loop_filter_flag;
            UWORD32 u4_mask;
            WORD32 i;
            WORD32 numbytes_row;
            numbytes_row =  (ps_sps->i2_pic_width_in_luma_samples + 63) / 64;
            pu1_pic_no_loop_filter_flag += (y0 / 8) * numbytes_row;
            pu1_pic_no_loop_filter_flag += (x0 / 64);

            /* Generate (cb_size / 8) number of 1s */
            /* i.e (log2_cb_size - 2) number of 1s */
            u4_mask = LSB_ONES((cb_size >> 3));
            for(i = 0; i < (cb_size / 8); i++)
            {
                *pu1_pic_no_loop_filter_flag |= (u4_mask << (((x0) / 8) % 8));
                pu1_pic_no_loop_filter_flag += numbytes_row;
            }
        }
    }

    {
        UWORD32 u4_skip_top = 0;
        UWORD32 u4_mask;
        UWORD32 u4_top_mask, u4_left_mask;
        UWORD32 u4_min_cu_x = x0 / 8;
        UWORD32 u4_min_cu_y = y0 / 8;

        pu4_skip_top += (u4_min_cu_x / 32);


        if(ps_slice_hdr->i1_slice_type != ISLICE)
        {
            WORD32 ctx_idx_inc;
            ctx_idx_inc = 0;

            if((0 != cu_pos_y) ||
                            ((0 != ps_codec->s_parse.i4_ctb_slice_y) &&
                                            (0 != ps_codec->s_parse.i4_ctb_tile_y)))
            {
                u4_skip_top = *pu4_skip_top;
                u4_skip_top >>= (u4_min_cu_x % 32);
                if(u4_skip_top & 1)
                    ctx_idx_inc++;
            }

            /*****************************************************************/
            /* If cu_pos_x is non-zero then left is available                */
            /* If cu_pos_x is zero then ensure both the following are true   */
            /*    Current CTB is not the first CTB in a tile row             */
            /*    Current CTB is not the first CTB in a slice                */
            /*****************************************************************/
            if((0 != cu_pos_x) ||
                            (((0 != ps_codec->s_parse.i4_ctb_slice_x) || (0 != ps_codec->s_parse.i4_ctb_slice_y)) &&
                                            (0 != ps_codec->s_parse.i4_ctb_tile_x)))
            {
                u4_skip_left >>= (u4_min_cu_y % 32);
                if(u4_skip_left & 1)
                    ctx_idx_inc++;
            }
            TRACE_CABAC_CTXT("cu_skip_flag", ps_cabac->u4_range, (IHEVC_CAB_SKIP_FLAG + ctx_idx_inc));
            skip_flag = ihevcd_cabac_decode_bin(ps_cabac,
                                                ps_bitstrm,
                                                (IHEVC_CAB_SKIP_FLAG + ctx_idx_inc));

            AEV_TRACE("cu_skip_flag", skip_flag, ps_cabac->u4_range);
        }
        else
            skip_flag = 0;

        /* Update top skip_flag */
        u4_skip_top = *pu4_skip_top;
        /* Since Max cb_size is 64, maximum of 8 bits will be set or reset */
        /* Also since Coding block will be within 64x64 grid, only 8bits within a WORD32
         * need to be updated. These 8 bits will not cross 8 bit boundaries
         */
        u4_mask = LSB_ONES(cb_size / 8);
        u4_top_mask = u4_mask << (u4_min_cu_x % 32);


        if(skip_flag)
        {
            u4_skip_top |= u4_top_mask;
        }
        else
        {
            u4_skip_top &= ~u4_top_mask;
        }
        *pu4_skip_top = u4_skip_top;

        /* Update left skip_flag */
        u4_skip_left = ps_codec->s_parse.u4_skip_cu_left;
        u4_mask = LSB_ONES(cb_size / 8);
        u4_left_mask = u4_mask << (u4_min_cu_y % 32);

        if(skip_flag)
        {
            u4_skip_left |= u4_left_mask;
        }
        else
        {
            u4_skip_left &= ~u4_left_mask;
        }
        ps_codec->s_parse.u4_skip_cu_left = u4_skip_left;
    }
    ps_codec->s_parse.i4_cu_pcm_flag = 0;

    if(skip_flag)
    {
        WORD32 ctb_x_base;
        WORD32 ctb_y_base;

        ctb_x_base = ps_codec->s_parse.i4_ctb_x << ps_sps->i1_log2_ctb_size;
        ctb_y_base = ps_codec->s_parse.i4_ctb_y << ps_sps->i1_log2_ctb_size;

        ps_tu->b1_cb_cbf = 0;
        ps_tu->b1_cr_cbf = 0;
        ps_tu->b1_y_cbf = 0;
        ps_tu->b4_pos_x = ((x0 - ctb_x_base) >> 2);
        ps_tu->b4_pos_y = ((y0 - ctb_y_base) >> 2);
        ps_tu->b1_transquant_bypass = 0;
        ps_tu->b3_size = (log2_cb_size - 2);
        ps_tu->b7_qp = ps_codec->s_parse.u4_qp;
        ps_tu->b3_chroma_intra_mode_idx = INTRA_PRED_CHROMA_IDX_NONE;
        ps_tu->b6_luma_intra_mode   = INTRA_PRED_NONE;

        /* Set the first TU in CU flag */
        {
            if((ps_codec->s_parse.s_cu.i4_pos_x << 3) == (ps_tu->b4_pos_x << 2) &&
                            (ps_codec->s_parse.s_cu.i4_pos_y << 3) == (ps_tu->b4_pos_y << 2))
            {
                ps_tu->b1_first_tu_in_cu = 1;
            }
            else
            {
                ps_tu->b1_first_tu_in_cu = 0;
            }
        }

        ps_codec->s_parse.ps_tu++;
        ps_codec->s_parse.s_cu.i4_tu_cnt++;
        ps_codec->s_parse.i4_pic_tu_idx++;

        ps_codec->s_parse.s_cu.i4_pred_mode = PRED_MODE_SKIP;
        ps_codec->s_parse.s_cu.i4_part_mode = PART_2Nx2N;
        {
            pu_t *ps_pu = ps_codec->s_parse.ps_pu;
            ps_pu->b2_part_idx = 0;
            ihevcd_parse_prediction_unit(ps_codec, x0, y0, cb_size, cb_size);
            STATS_UPDATE_PU_SKIP_SIZE(ps_pu);
        }
    }
    else
    {
        WORD32 pred_mode;
        WORD32 part_mode;
        WORD32 intra_split_flag;
        WORD32 is_mincb;
        cb_size = (1 << log2_cb_size);
        is_mincb = (cb_size == (1 << ps_sps->i1_log2_min_coding_block_size));
        pcm_flag = 0;
        if(ps_slice_hdr->i1_slice_type != ISLICE)
        {
            TRACE_CABAC_CTXT("pred_mode_flag", ps_cabac->u4_range, IHEVC_CAB_PRED_MODE);
            pred_mode = ihevcd_cabac_decode_bin(ps_cabac,
                                                ps_bitstrm,
                                                IHEVC_CAB_PRED_MODE);

            AEV_TRACE("pred_mode_flag", pred_mode, ps_cabac->u4_range);
        }
        else
        {
            pred_mode = PRED_MODE_INTRA;
        }

        /* If current CU is intra then set corresponging bit in picture level intra map */
        if(PRED_MODE_INTRA == pred_mode)
        {
            UWORD8 *pu1_pic_intra_flag = ps_codec->s_parse.pu1_pic_intra_flag;
            UWORD32 u4_mask;
            WORD32 i;
            WORD32 numbytes_row;
            numbytes_row =  (ps_sps->i2_pic_width_in_luma_samples + 63) / 64;
            pu1_pic_intra_flag += (y0 / 8) * numbytes_row;
            pu1_pic_intra_flag += (x0 / 64);

            /* Generate (cb_size / 8) number of 1s */
            /* i.e (log2_cb_size - 2) number of 1s */
            u4_mask = LSB_ONES((cb_size >> 3));
            for(i = 0; i < (cb_size / 8); i++)
            {
                *pu1_pic_intra_flag |= (u4_mask << (((x0) / 8) % 8));
                pu1_pic_intra_flag += numbytes_row;
            }
        }

        ps_codec->s_parse.s_cu.i4_pred_mode = pred_mode;
        intra_split_flag = 0;
        if((PRED_MODE_INTRA != pred_mode) ||
                        is_mincb)
        {
            UWORD32 bin;
            if(PRED_MODE_INTRA == pred_mode)
            {
                TRACE_CABAC_CTXT("part_mode", ps_cabac->u4_range, IHEVC_CAB_PART_MODE);
                bin = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, IHEVC_CAB_PART_MODE);
                part_mode = (bin) ? PART_2Nx2N : PART_NxN;
            }
            else
            {
                WORD32 amp_enabled = ps_sps->i1_amp_enabled_flag;

                UWORD32 u4_max_bin_cnt = 0;



                if(amp_enabled && !is_mincb)
                {
                    part_mode = ihevcd_parse_part_mode_amp(ps_cabac, ps_bitstrm);
                }
                else
                {
                    WORD32 ctxt_inc = IHEVC_CAB_PART_MODE;

                    u4_max_bin_cnt = 2;
                    if((is_mincb) && (cb_size > 8))
                    {
                        u4_max_bin_cnt++;
                    }

                    part_mode = -1;
                    TRACE_CABAC_CTXT("part_mode", ps_cabac->u4_range, IHEVC_CAB_PART_MODE);
                    do
                    {
                        bin = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm,
                                                      ctxt_inc++);
                        part_mode++;
                    }while(--u4_max_bin_cnt && !bin);

                    /* If the last bin was zero, then increment part mode by 1 */
                    if(!bin)
                        part_mode++;
                }


            }

            AEV_TRACE("part_mode", part_mode, ps_cabac->u4_range);

        }
        else
        {
            part_mode = 0;
            intra_split_flag = 0;
        }
        ps_codec->s_parse.s_cu.i4_part_mode = part_mode;

        if((PRED_MODE_INTRA == ps_codec->s_parse.s_cu.i4_pred_mode) &&
                        (PART_NxN == ps_codec->s_parse.s_cu.i4_part_mode))
        {
            intra_split_flag = 1;
        }
        ps_codec->s_parse.s_cu.i4_part_mode = part_mode;
        ps_codec->s_parse.s_cu.i4_intra_split_flag = intra_split_flag;
        if(pred_mode == PRED_MODE_INTRA)
        {
            ps_codec->s_parse.i4_cu_pcm_flag = 0;
            ihevcd_parse_coding_unit_intra(ps_codec, x0, y0, log2_cb_size);
            pcm_flag = ps_codec->s_parse.i4_cu_pcm_flag;

        }
        else
        {
            if(part_mode == PART_2Nx2N)
            {
                pu_t *ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0, y0, cb_size, cb_size);
                ps_pu->b2_part_idx = 0;
            }
            else if(part_mode == PART_2NxN)
            {
                pu_t *ps_pu = ps_codec->s_parse.ps_pu;

                ihevcd_parse_prediction_unit(ps_codec, x0, y0, cb_size, cb_size / 2);
                ps_pu->b2_part_idx = 0;

                ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0, y0 + (cb_size / 2), cb_size, cb_size / 2);

                ps_pu->b2_part_idx = 1;
            }
            else if(part_mode == PART_Nx2N)
            {
                pu_t *ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0, y0, cb_size / 2, cb_size);
                ps_pu->b2_part_idx = 0;
                ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0 + (cb_size / 2), y0, cb_size / 2, cb_size);

                ps_pu->b2_part_idx = 1;
            }
            else if(part_mode == PART_2NxnU)
            {
                pu_t *ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0, y0, cb_size, cb_size / 4);
                ps_pu->b2_part_idx = 0;
                ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0, y0 + (cb_size / 4), cb_size, cb_size * 3 / 4);

                ps_pu->b2_part_idx = 1;
            }
            else if(part_mode == PART_2NxnD)
            {
                pu_t *ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0, y0, cb_size, cb_size * 3 / 4);
                ps_pu->b2_part_idx = 0;
                ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0, y0 + (cb_size * 3 / 4), cb_size, cb_size / 4);

                ps_pu->b2_part_idx = 1;
            }
            else if(part_mode == PART_nLx2N)
            {
                pu_t *ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0, y0, cb_size / 4, cb_size);
                ps_pu->b2_part_idx = 0;
                ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0 + (cb_size / 4), y0, cb_size * 3 / 4, cb_size);

                ps_pu->b2_part_idx = 1;
            }
            else if(part_mode == PART_nRx2N)
            {
                pu_t *ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0, y0, cb_size * 3 / 4, cb_size);
                ps_pu->b2_part_idx = 0;
                ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0 + (cb_size * 3 / 4), y0, cb_size / 4, cb_size);
                ps_pu->b2_part_idx = 1;
            }
            else
            { /* PART_NxN */
                pu_t *ps_pu = ps_codec->s_parse.ps_pu;

                ihevcd_parse_prediction_unit(ps_codec, x0, y0, cb_size / 2, cb_size / 2);
                ps_pu->b2_part_idx = 0;
                ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0 + (cb_size / 2), y0, cb_size / 2, cb_size / 2);

                ps_pu->b2_part_idx = 1;
                ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0, y0 + (cb_size / 2), cb_size / 2, cb_size / 2);

                ps_pu->b2_part_idx = 2;
                ps_pu = ps_codec->s_parse.ps_pu;
                ihevcd_parse_prediction_unit(ps_codec, x0 + (cb_size / 2), y0 + (cb_size / 2), cb_size / 2, cb_size / 2);

                ps_pu->b2_part_idx = 3;
            }
        }

        if(!pcm_flag)
        {
            WORD32 no_residual_syntax_flag = 0;
            pu_t *ps_pu;
            /* Since ps_pu is incremented for each PU parsed, decrement by 1 to
             *  access last decoded PU
             */
            ps_pu = ps_codec->s_parse.ps_pu - 1;
            if((PRED_MODE_INTRA != pred_mode) &&
                            (!((part_mode == PART_2Nx2N) && ps_pu->b1_merge_flag)))
            {

                TRACE_CABAC_CTXT("rqt_root_cbf", ps_cabac->u4_range, IHEVC_CAB_NORES_IDX);
                no_residual_syntax_flag = ihevcd_cabac_decode_bin(ps_cabac,
                                                                  ps_bitstrm,
                                                                  IHEVC_CAB_NORES_IDX);

                AEV_TRACE("rqt_root_cbf", no_residual_syntax_flag,
                          ps_cabac->u4_range);
                /* TODO: HACK FOR COMPLIANCE WITH HM REFERENCE DECODER */
                /*********************************************************/
                /* currently the HM decoder expects qtroot cbf instead of */
                /* no_residue_flag which has opposite meaning             */
                /* This will be fixed once the software / spec is fixed   */
                /*********************************************************/
                no_residual_syntax_flag = 1 - no_residual_syntax_flag;
            }

            if(!no_residual_syntax_flag)
            {

                ps_codec->s_parse.s_cu.i4_max_trafo_depth = (pred_mode == PRED_MODE_INTRA) ?
                                (ps_sps->i1_max_transform_hierarchy_depth_intra + intra_split_flag) :
                                (ps_sps->i1_max_transform_hierarchy_depth_inter);
                ret = ihevcd_parse_transform_tree(ps_codec, x0, y0, x0, y0,
                                                  log2_cb_size, 0, 0,
                                                  ps_codec->s_parse.s_cu.ai4_intra_luma_pred_mode[0]);
                RETURN_IF((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret, ret);
            }
            else
            {
                WORD32 ctb_x_base;
                WORD32 ctb_y_base;

                ctb_x_base = ps_codec->s_parse.i4_ctb_x << ps_sps->i1_log2_ctb_size;
                ctb_y_base = ps_codec->s_parse.i4_ctb_y << ps_sps->i1_log2_ctb_size;

                ps_tu = ps_codec->s_parse.ps_tu;
                ps_tu->b1_cb_cbf = 0;
                ps_tu->b1_cr_cbf = 0;
                ps_tu->b1_y_cbf = 0;
                ps_tu->b4_pos_x = ((x0 - ctb_x_base) >> 2);
                ps_tu->b4_pos_y = ((y0 - ctb_y_base) >> 2);
                ps_tu->b1_transquant_bypass = 0;
                ps_tu->b3_size = (log2_cb_size - 2);
                ps_tu->b7_qp = ps_codec->s_parse.u4_qp;
                ps_tu->b3_chroma_intra_mode_idx = INTRA_PRED_CHROMA_IDX_NONE;
                ps_tu->b6_luma_intra_mode   = ps_codec->s_parse.s_cu.ai4_intra_luma_pred_mode[0];

                /* Set the first TU in CU flag */
                {
                    if((ps_codec->s_parse.s_cu.i4_pos_x << 3) == (ps_tu->b4_pos_x << 2) &&
                                    (ps_codec->s_parse.s_cu.i4_pos_y << 3) == (ps_tu->b4_pos_y << 2))
                    {
                        ps_tu->b1_first_tu_in_cu = 1;
                    }
                    else
                    {
                        ps_tu->b1_first_tu_in_cu = 0;
                    }
                }
                ps_codec->s_parse.ps_tu++;
                ps_codec->s_parse.s_cu.i4_tu_cnt++;
                ps_codec->s_parse.i4_pic_tu_idx++;

            }
        }

    }




    return ret;
}




/**
 *******************************************************************************
 *
 * @brief
 *  Parses Coding Quad Tree
 *
 * @par Description:
 *  Parses Coding Quad Tree as per Section:7.3.9.4
 *
 * @param[in] ps_codec
 *  Pointer to codec context
 *
 * @returns  Error from IHEVCD_ERROR_T
 *
 * @remarks
 *
 *
 *******************************************************************************
 */
IHEVCD_ERROR_T ihevcd_parse_coding_quadtree(codec_t *ps_codec,
                                            WORD32 x0,
                                            WORD32 y0,
                                            WORD32 log2_cb_size,
                                            WORD32 ct_depth)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    sps_t *ps_sps;
    pps_t *ps_pps;
    WORD32 split_cu_flag;
    WORD32 x1, y1;
    WORD32 cu_pos_x;
    WORD32 cu_pos_y;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    cab_ctxt_t *ps_cabac = &ps_codec->s_parse.s_cabac;
    WORD32 cb_size = 1 << log2_cb_size;
    ps_sps = ps_codec->s_parse.ps_sps;
    ps_pps = ps_codec->s_parse.ps_pps;

    /* Compute CU position with respect to current CTB in (8x8) units */
    cu_pos_x = (x0 - (ps_codec->s_parse.i4_ctb_x << ps_sps->i1_log2_ctb_size)) >> 3;
    cu_pos_y = (y0 - (ps_codec->s_parse.i4_ctb_y << ps_sps->i1_log2_ctb_size)) >> 3;

    ps_codec->s_parse.s_cu.i4_pos_x = cu_pos_x;
    ps_codec->s_parse.s_cu.i4_pos_y = cu_pos_y;

    ps_codec->s_parse.s_cu.i4_log2_cb_size = log2_cb_size;

    ps_codec->s_parse.i4_ct_depth = ct_depth;
    {
        UWORD32 *pu4_ct_depth_top = ps_codec->s_parse.pu4_ct_depth_top;
        UWORD32 u4_ct_depth_left = ps_codec->s_parse.u4_ct_depth_left;
        UWORD32 u4_ct_depth_top = 0;
        UWORD32 u4_mask;
        UWORD32 u4_top_mask, u4_left_mask;
        WORD32  ctxt_idx;
        UWORD32 u4_min_cu_x = x0 / 8;
        UWORD32 u4_min_cu_y = y0 / 8;

        pu4_ct_depth_top += (u4_min_cu_x / 16);




        if(((x0 + (1 << log2_cb_size)) <= ps_sps->i2_pic_width_in_luma_samples) &&
                        ((y0 + (1 << log2_cb_size)) <= ps_sps->i2_pic_height_in_luma_samples) &&
                        (log2_cb_size > ps_sps->i1_log2_min_coding_block_size))
        {

            ctxt_idx = IHEVC_CAB_SPLIT_CU_FLAG;
            /* Split cu context increment is decided based on left and top Coding tree
             * depth which is stored at frame level
             */
            /* Check if the CTB is in first row in the current slice or tile */
            if((0 != cu_pos_y) ||
                            ((0 != ps_codec->s_parse.i4_ctb_slice_y) &&
                                            (0 != ps_codec->s_parse.i4_ctb_tile_y)))
            {
                u4_ct_depth_top = *pu4_ct_depth_top;
                u4_ct_depth_top >>= ((u4_min_cu_x % 16) * 2);
                u4_ct_depth_top &= 3;

                if((WORD32)u4_ct_depth_top > ct_depth)
                    ctxt_idx++;
            }

            /* Check if the CTB is in first column in the current slice or tile */
            /*****************************************************************/
            /* If cu_pos_x is non-zero then left is available                */
            /* If cu_pos_x is zero then ensure both the following are true   */
            /*    Current CTB is not the first CTB in a tile row             */
            /*    Current CTB is not the first CTB in a slice                */
            /*****************************************************************/
            if((0 != cu_pos_x) ||
                            (((0 != ps_codec->s_parse.i4_ctb_slice_x) || (0 != ps_codec->s_parse.i4_ctb_slice_y)) &&
                                            (0 != ps_codec->s_parse.i4_ctb_tile_x)))
            {
                u4_ct_depth_left >>= ((u4_min_cu_y % 16) * 2);
                u4_ct_depth_left &= 3;
                if((WORD32)u4_ct_depth_left > ct_depth)
                    ctxt_idx++;
            }
            TRACE_CABAC_CTXT("split_cu_flag", ps_cabac->u4_range, ctxt_idx);
            split_cu_flag = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
            AEV_TRACE("split_cu_flag", split_cu_flag, ps_cabac->u4_range);
        }
        else
        {
            if(log2_cb_size > ps_sps->i1_log2_min_coding_block_size)
                split_cu_flag = 1;
            else
                split_cu_flag = 0;
        }

        if(0 == split_cu_flag)
        {
            /* Update top ct_depth */
            u4_ct_depth_top = *pu4_ct_depth_top;
            /* Since Max cb_size is 64, maximum of 8 bits will be set or reset */
            /* Also since Coding block will be within 64x64 grid, only 8bits within a WORD32
             * need to be updated. These 8 bits will not cross 8 bit boundaries
             */
            u4_mask = DUP_LSB_11(cb_size / 8);

            u4_top_mask = u4_mask << ((u4_min_cu_x % 16) * 2);
            u4_ct_depth_top &= ~u4_top_mask;

            if(ct_depth)
            {
                u4_top_mask = gau4_ct_depth_mask[ct_depth] & u4_mask;

                u4_top_mask = u4_top_mask << ((u4_min_cu_x % 16) * 2);
                u4_ct_depth_top |= u4_top_mask;
            }

            *pu4_ct_depth_top = u4_ct_depth_top;

            /* Update left ct_depth */
            u4_ct_depth_left = ps_codec->s_parse.u4_ct_depth_left;

            u4_left_mask = u4_mask << ((u4_min_cu_y % 16) * 2);

            u4_ct_depth_left &= ~u4_left_mask;
            if(ct_depth)
            {
                u4_left_mask = gau4_ct_depth_mask[ct_depth] & u4_mask;

                u4_left_mask = u4_left_mask << ((u4_min_cu_y % 16) * 2);
                u4_ct_depth_left |= u4_left_mask;
            }

            ps_codec->s_parse.u4_ct_depth_left = u4_ct_depth_left;
        }
    }
    if((ps_pps->i1_cu_qp_delta_enabled_flag) &&
                    (log2_cb_size >= ps_pps->i1_log2_min_cu_qp_delta_size))
    {
        ps_codec->s_parse.i4_is_cu_qp_delta_coded = 0;
        ps_codec->s_parse.i4_cu_qp_delta = 0;
    }
    if(split_cu_flag)
    {
        x1 = x0 + ((1 << log2_cb_size) >> 1);
        y1 = y0 + ((1 << log2_cb_size) >> 1);

        ret = ihevcd_parse_coding_quadtree(ps_codec, x0, y0, log2_cb_size - 1, ct_depth + 1);
        RETURN_IF((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret, ret);

        /* At frame boundaries coding quadtree nodes are sent only if they fall within the frame */
        if(x1 < ps_sps->i2_pic_width_in_luma_samples)
        {
            ret = ihevcd_parse_coding_quadtree(ps_codec, x1, y0, log2_cb_size - 1, ct_depth + 1);
            RETURN_IF((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret, ret);
        }

        if(y1 < ps_sps->i2_pic_height_in_luma_samples)
        {
            ret = ihevcd_parse_coding_quadtree(ps_codec, x0, y1, log2_cb_size - 1, ct_depth + 1);
            RETURN_IF((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret, ret);
        }

        if((x1 < ps_sps->i2_pic_width_in_luma_samples) &&
                        (y1 < ps_sps->i2_pic_height_in_luma_samples))
        {
            ret = ihevcd_parse_coding_quadtree(ps_codec, x1, y1, log2_cb_size - 1, ct_depth + 1);
            RETURN_IF((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret, ret);
        }
    }
    else
    {
        /* Set current group QP if current CU is aligned with the group */
        {
            WORD32 cu_pos_x = ps_codec->s_parse.s_cu.i4_pos_x << 3;
            WORD32 cu_pos_y = ps_codec->s_parse.s_cu.i4_pos_y << 3;

            WORD32 qpg_x = (cu_pos_x - (cu_pos_x & ((1 << ps_pps->i1_log2_min_cu_qp_delta_size) - 1)));
            WORD32 qpg_y = (cu_pos_y - (cu_pos_y & ((1 << ps_pps->i1_log2_min_cu_qp_delta_size) - 1)));

            if((cu_pos_x == qpg_x) &&
                            (cu_pos_y == qpg_y))
            {
                ps_codec->s_parse.u4_qpg = ps_codec->s_parse.u4_qp;

                ps_codec->s_parse.s_cu.i4_cu_qp_delta = 0;

            }
        }

        ret = ihevcd_parse_coding_unit(ps_codec, x0, y0, log2_cb_size);
        RETURN_IF((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret, ret);

        if(ps_pps->i1_cu_qp_delta_enabled_flag)
        {
            WORD32 qp_pred, qp_left, qp_top;
            WORD32 cu_pos_x;
            WORD32 cu_pos_y;
            WORD32 qpg_x;
            WORD32 qpg_y;
            WORD32 i, j;
            WORD32 qp;
            WORD32 cur_cu_offset;
            tu_t *ps_tu = ps_codec->s_parse.ps_tu;
            WORD32 cb_size = 1 << ps_codec->s_parse.s_cu.i4_log2_cb_size;

            cu_pos_x = ps_codec->s_parse.s_cu.i4_pos_x << 3;
            cu_pos_y = ps_codec->s_parse.s_cu.i4_pos_y << 3;

            qpg_x = (cu_pos_x - (cu_pos_x & ((1 << ps_pps->i1_log2_min_cu_qp_delta_size) - 1))) >> 3;
            qpg_y = (cu_pos_y - (cu_pos_y & ((1 << ps_pps->i1_log2_min_cu_qp_delta_size) - 1))) >> 3;

            /*previous coded Qp*/
            qp_left = ps_codec->s_parse.u4_qpg;
            qp_top = ps_codec->s_parse.u4_qpg;

            if(qpg_x > 0)
            {
                qp_left = ps_codec->s_parse.ai1_8x8_cu_qp[qpg_x - 1 + (qpg_y * 8)];
            }
            if(qpg_y > 0)
            {
                qp_top = ps_codec->s_parse.ai1_8x8_cu_qp[qpg_x + ((qpg_y - 1) * 8)];
            }

            qp_pred = (qp_left + qp_top + 1) >> 1;
            /* Since qp_pred + ps_codec->s_parse.s_cu.i4_cu_qp_delta can be negative,
            52 is added before taking modulo 52 */
            qp = (qp_pred + ps_codec->s_parse.s_cu.i4_cu_qp_delta + 52) % 52;

            cur_cu_offset = (cu_pos_x >> 3) + cu_pos_y;
            for(i = 0; i < (cb_size >> 3); i++)
            {
                for(j = 0; j < (cb_size >> 3); j++)
                {
                    ps_codec->s_parse.ai1_8x8_cu_qp[cur_cu_offset + (i * 8) + j] = qp;
                }
            }

            ps_codec->s_parse.u4_qp = qp;
            ps_codec->s_parse.s_cu.i4_qp = qp;


            /* When change in QP is signaled, update the QP in TUs that are already parsed in the CU */
            {
                tu_t *ps_tu_tmp;
                ps_tu_tmp = ps_tu - ps_codec->s_parse.s_cu.i4_tu_cnt;
                ps_tu->b7_qp = ps_codec->s_parse.u4_qp;
                while(ps_tu_tmp != ps_tu)
                {
                    ps_tu_tmp->b7_qp = ps_codec->s_parse.u4_qp;

                    ps_tu_tmp++;
                }
            }
            if(ps_codec->s_parse.s_cu.i4_cu_qp_delta)
            {
                WORD32 ctb_indx;
                ctb_indx = ps_codec->s_parse.i4_ctb_x + ps_sps->i2_pic_wd_in_ctb * ps_codec->s_parse.i4_ctb_y;
                ps_codec->s_parse.s_bs_ctxt.pu1_pic_qp_const_in_ctb[ctb_indx >> 3] &= (~(1 << (ctb_indx & 7)));
            }

        }

    }




    return ret;
}


/**
 *******************************************************************************
 *
 * @brief
 *  Parses SAO (Sample adaptive offset syntax)
 *
 * @par Description:
 *  Parses SAO (Sample adaptive offset syntax) as per  Section:7.3.9.3
 *
 * @param[in] ps_codec
 *  Pointer to codec context
 *
 * @returns  Error from IHEVCD_ERROR_T
 *
 * @remarks
 *
 *
 *******************************************************************************
 */
IHEVCD_ERROR_T  ihevcd_parse_sao(codec_t *ps_codec)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    sps_t *ps_sps;
    sao_t *ps_sao;
    WORD32 rx;
    WORD32 ry;
    WORD32 value;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    WORD32 sao_merge_left_flag;
    WORD32 sao_merge_up_flag;
    slice_header_t *ps_slice_hdr;
    cab_ctxt_t *ps_cabac = &ps_codec->s_parse.s_cabac;
    WORD32 ctxt_idx;

    ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr_base;
    ps_slice_hdr += (ps_codec->s_parse.i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1));

    ps_sps = (ps_codec->s_parse.ps_sps);
    rx = ps_codec->s_parse.i4_ctb_x;
    ry = ps_codec->s_parse.i4_ctb_y;

    ps_sao = ps_codec->s_parse.ps_pic_sao + rx + ry * ps_sps->i2_pic_wd_in_ctb;

    /* Default values */
    ps_sao->b3_y_type_idx = 0;
    ps_sao->b3_cb_type_idx = 0;
    ps_sao->b3_cr_type_idx = 0;

    UNUSED(value);
    ctxt_idx = IHEVC_CAB_SAO_MERGE;
    sao_merge_left_flag = 0;
    sao_merge_up_flag = 0;
    if(rx > 0)
    {
        /*TODO:Implemented only for slice. condition for tile is not tested*/
        if(((0 != ps_codec->s_parse.i4_ctb_slice_x) || (0 != ps_codec->s_parse.i4_ctb_slice_y)) &&
                        (0 != ps_codec->s_parse.i4_ctb_tile_x))
        {

            TRACE_CABAC_CTXT("sao_merge_flag", ps_cabac->u4_range, ctxt_idx);
            sao_merge_left_flag = ihevcd_cabac_decode_bin(ps_cabac,
                                                          ps_bitstrm,
                                                          ctxt_idx);
            AEV_TRACE("sao_merge_flag", sao_merge_left_flag, ps_cabac->u4_range);
        }

    }
    if(ry > 0 && !sao_merge_left_flag)
    {
        if((ps_codec->s_parse.i4_ctb_slice_y > 0) && (ps_codec->s_parse.i4_ctb_tile_y > 0))
        {
            TRACE_CABAC_CTXT("sao_merge_flag", ps_cabac->u4_range, ctxt_idx);
            sao_merge_up_flag = ihevcd_cabac_decode_bin(ps_cabac,
                                                        ps_bitstrm,
                                                        ctxt_idx);
            AEV_TRACE("sao_merge_flag", sao_merge_up_flag, ps_cabac->u4_range);
        }
    }
    ctxt_idx = IHEVC_CAB_SAO_TYPE;

    if(sao_merge_left_flag)
    {
        *ps_sao = *(ps_sao - 1);
    }
    else if(sao_merge_up_flag)
    {
        *ps_sao = *(ps_sao - ps_sps->i2_pic_wd_in_ctb);
    }
    else // if(!sao_merge_up_flag && !sao_merge_left_flag)
    {
        WORD32 c_idx;
        WORD32 sao_type_idx = 0;
        for(c_idx = 0; c_idx < 3; c_idx++)
        {
            if((ps_slice_hdr->i1_slice_sao_luma_flag && c_idx == 0) || (ps_slice_hdr->i1_slice_sao_chroma_flag && c_idx > 0))
            {


                /* sao_type_idx will be same for c_idx == 1 and c_idx == 2 - hence not initialized to zero for c_idx == 2*/

                if(c_idx == 0)
                {
                    sao_type_idx = 0;
                    TRACE_CABAC_CTXT("sao_type_idx", ps_cabac->u4_range, ctxt_idx);
                    sao_type_idx = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);

                    if(sao_type_idx)
                    {
                        sao_type_idx += ihevcd_cabac_decode_bypass_bin(ps_cabac, ps_bitstrm);
                    }
                    AEV_TRACE("sao_type_idx", sao_type_idx,  ps_cabac->u4_range);

                    ps_sao->b3_y_type_idx = sao_type_idx;
                }
                if(c_idx == 1)
                {
                    sao_type_idx = 0;
                    TRACE_CABAC_CTXT("sao_type_idx", ps_cabac->u4_range, ctxt_idx);
                    sao_type_idx = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_idx);
                    if(sao_type_idx)
                    {
                        sao_type_idx += ihevcd_cabac_decode_bypass_bin(ps_cabac, ps_bitstrm);
                    }

                    AEV_TRACE("sao_type_idx", sao_type_idx,  ps_cabac->u4_range);

                    ps_sao->b3_cb_type_idx = sao_type_idx;
                    ps_sao->b3_cr_type_idx = sao_type_idx;
                }

                if(sao_type_idx != 0)
                {
                    WORD32 i;
                    WORD32 sao_offset[4];
                    WORD32 sao_band_position = 0;
                    WORD32 c_max =  (1 << (MIN(BIT_DEPTH, 10) - 5)) - 1;
                    for(i = 0; i < 4; i++)
                    {
                        sao_offset[i] = ihevcd_cabac_decode_bypass_bins_tunary(ps_cabac, ps_bitstrm, c_max);
                        AEV_TRACE("sao_offset_abs", sao_offset[i], ps_cabac->u4_range);

                        if((2 == sao_type_idx) && (i > 1))
                        {
                            sao_offset[i] = -sao_offset[i];
                        }
                    }

                    if(sao_type_idx == 1)
                    {
                        for(i = 0; i < 4; i++)
                        {
                            if(sao_offset[i] != 0)
                            {
                                value = ihevcd_cabac_decode_bypass_bin(ps_cabac, ps_bitstrm);
                                AEV_TRACE("sao_offset_sign", value, ps_cabac->u4_range);

                                if(value)
                                {
                                    sao_offset[i] = -sao_offset[i];
                                }
                            }
                        }
                        value = ihevcd_cabac_decode_bypass_bins(ps_cabac, ps_bitstrm, 5);
                        AEV_TRACE("sao_band_position", value, ps_cabac->u4_range);

                        sao_band_position = value;
                    }
                    else
                    {
                        if(c_idx == 0)
                        {
                            value = ihevcd_cabac_decode_bypass_bins(ps_cabac, ps_bitstrm, 2);
                            AEV_TRACE("sao_eo_class", value, ps_cabac->u4_range);

                            ps_sao->b3_y_type_idx += value;
                        }

                        if(c_idx == 1)
                        {
                            value = ihevcd_cabac_decode_bypass_bins(ps_cabac, ps_bitstrm, 2);
                            AEV_TRACE("sao_eo_class", value, ps_cabac->u4_range);

                            ps_sao->b3_cb_type_idx += value;
                            ps_sao->b3_cr_type_idx += value;
                        }
                    }

                    if(0 == c_idx)
                    {
                        ps_sao->b4_y_offset_1 = sao_offset[0];
                        ps_sao->b4_y_offset_2 = sao_offset[1];
                        ps_sao->b4_y_offset_3 = sao_offset[2];
                        ps_sao->b4_y_offset_4 = sao_offset[3];

                        ps_sao->b5_y_band_pos = sao_band_position;
                    }
                    else if(1 == c_idx)
                    {
                        ps_sao->b4_cb_offset_1 = sao_offset[0];
                        ps_sao->b4_cb_offset_2 = sao_offset[1];
                        ps_sao->b4_cb_offset_3 = sao_offset[2];
                        ps_sao->b4_cb_offset_4 = sao_offset[3];

                        ps_sao->b5_cb_band_pos = sao_band_position;
                    }
                    else // 2 == c_idx
                    {
                        ps_sao->b4_cr_offset_1 = sao_offset[0];
                        ps_sao->b4_cr_offset_2 = sao_offset[1];
                        ps_sao->b4_cr_offset_3 = sao_offset[2];
                        ps_sao->b4_cr_offset_4 = sao_offset[3];

                        ps_sao->b5_cr_band_pos = sao_band_position;
                    }
                }
            }
        }
    }

    return ret;
}
/**
 *******************************************************************************
 *
 * @brief
 *  Set ctb skip
 *
 * @par Description:
 *  During error, sets tu and pu params of a ctb as skip.
 *
 * @param[in] ps_codec
 *  Pointer to codec context
 *
 * @returns  None
 *
 * @remarks
 *
 *
 *******************************************************************************
 */
void ihevcd_set_ctb_skip(codec_t *ps_codec)
{
    tu_t *ps_tu;
    pu_t *ps_pu;
    sps_t *ps_sps = ps_codec->s_parse.ps_sps;
    WORD32 ctb_size = 1 << ps_sps->i1_log2_ctb_size;
    WORD32 ctb_skip_wd, ctb_skip_ht;
    WORD32 rows_remaining, cols_remaining;
    WORD32 tu_abs_x, tu_abs_y;
    WORD32 numbytes_row =  (ps_sps->i2_pic_width_in_luma_samples + 63) / 64;
    UWORD8 *pu1_pic_intra_flag;
    UWORD32 u4_mask;
    WORD32 pu_x,pu_y;

    /* Set pu wd and ht based on whether the ctb is complete or not */
    rows_remaining = ps_sps->i2_pic_height_in_luma_samples
                    - (ps_codec->s_parse.i4_ctb_y << ps_sps->i1_log2_ctb_size);
    ctb_skip_ht = MIN(ctb_size, rows_remaining);

    cols_remaining = ps_sps->i2_pic_width_in_luma_samples
                    - (ps_codec->s_parse.i4_ctb_x << ps_sps->i1_log2_ctb_size);
    ctb_skip_wd = MIN(ctb_size, cols_remaining);

    ps_codec->s_parse.s_cu.i4_pred_mode = PRED_MODE_SKIP;
    ps_codec->s_parse.s_cu.i4_part_mode = PART_2Nx2N;

    for (pu_y = 0; pu_y < ctb_skip_ht ; pu_y += MIN_CU_SIZE)
    {
        for (pu_x = 0; pu_x < ctb_skip_wd ; pu_x += MIN_CU_SIZE)
        {
            ps_tu = ps_codec->s_parse.ps_tu;
            ps_tu->b1_cb_cbf = 0;
            ps_tu->b1_cr_cbf = 0;
            ps_tu->b1_y_cbf = 0;
            ps_tu->b4_pos_x = pu_x >> 2;
            ps_tu->b4_pos_y = pu_y >> 2;
            ps_tu->b1_transquant_bypass = 0;
            ps_tu->b3_size = 1;
            ps_tu->b7_qp = ps_codec->s_parse.u4_qp;
            ps_tu->b3_chroma_intra_mode_idx = INTRA_PRED_CHROMA_IDX_NONE;
            ps_tu->b6_luma_intra_mode   = INTRA_PRED_NONE;
            ps_tu->b1_first_tu_in_cu = 1;

            ps_codec->s_parse.ps_tu++;
            ps_codec->s_parse.s_cu.i4_tu_cnt++;
            ps_codec->s_parse.i4_pic_tu_idx++;

            tu_abs_x = (ps_codec->s_parse.i4_ctb_x << ps_sps->i1_log2_ctb_size) + pu_x;
            tu_abs_y = (ps_codec->s_parse.i4_ctb_y << ps_sps->i1_log2_ctb_size) + pu_y;
            pu1_pic_intra_flag = ps_codec->s_parse.pu1_pic_intra_flag;
            pu1_pic_intra_flag += (tu_abs_y >> 3) * numbytes_row;
            pu1_pic_intra_flag += (tu_abs_x >> 6);
            u4_mask = (LSB_ONES((MIN_CU_SIZE >> 3)) << (((tu_abs_x) / 8) % 8));
            u4_mask = ~u4_mask;
            *pu1_pic_intra_flag &= u4_mask;

            ps_pu = ps_codec->s_parse.ps_pu;
            ps_pu->b2_part_idx = 0;
            ps_pu->b4_pos_x = pu_x >> 2;
            ps_pu->b4_pos_y = pu_y >> 2;
            ps_pu->b4_wd = 1;
            ps_pu->b4_ht = 1;
            ps_pu->b1_intra_flag = 0;
            ps_pu->b3_part_mode = ps_codec->s_parse.s_cu.i4_part_mode;
            ps_pu->b1_merge_flag = 1;
            ps_pu->b3_merge_idx = 0;

            ps_codec->s_parse.ps_pu++;
            ps_codec->s_parse.i4_pic_pu_idx++;
        }
    }
}

/**
 *******************************************************************************
 *
 * @brief
 *  Parses Slice data syntax
 *
 * @par Description:
 *  Parses Slice data syntax as per Section:7.3.9.1
 *
 * @param[in] ps_codec
 *  Pointer to codec context
 *
 * @returns  Error from IHEVCD_ERROR_T
 *
 * @remarks
 *
 *
 *******************************************************************************
 */
IHEVCD_ERROR_T ihevcd_parse_slice_data(codec_t *ps_codec)
{

    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 end_of_slice_flag = 0;
    sps_t *ps_sps;
    pps_t *ps_pps;
    slice_header_t *ps_slice_hdr;
    WORD32 end_of_pic;
    tile_t *ps_tile, *ps_tile_prev;
    WORD32 i;
    WORD32 ctb_addr;
    WORD32 tile_idx;
    WORD32 cabac_init_idc;
    WORD32 ctb_size;
    WORD32 num_ctb_in_row;
    WORD32 num_min4x4_in_ctb;
    WORD32 slice_qp;
    WORD32 slice_start_ctb_idx;
    WORD32 tile_start_ctb_idx;


    ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr_base;
    ps_pps = ps_codec->s_parse.ps_pps_base;
    ps_sps = ps_codec->s_parse.ps_sps_base;

    /* Get current slice header, pps and sps */
    ps_slice_hdr += (ps_codec->s_parse.i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1));
    ps_pps  += ps_slice_hdr->i1_pps_id;
    ps_sps  += ps_pps->i1_sps_id;

    if(0 != ps_codec->s_parse.i4_cur_slice_idx)
    {
        if(!ps_slice_hdr->i1_dependent_slice_flag)
        {
            ps_codec->s_parse.i4_cur_independent_slice_idx =
                    ps_codec->s_parse.i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1);
        }
    }


    ctb_size = 1 << ps_sps->i1_log2_ctb_size;
    num_min4x4_in_ctb = (ctb_size / 4) * (ctb_size / 4);
    num_ctb_in_row = ps_sps->i2_pic_wd_in_ctb;

    /* Update the parse context */
    if(0 == ps_codec->i4_slice_error)
    {
        ps_codec->s_parse.i4_ctb_x = ps_slice_hdr->i2_ctb_x;
        ps_codec->s_parse.i4_ctb_y = ps_slice_hdr->i2_ctb_y;
    }
    ps_codec->s_parse.ps_pps = ps_pps;
    ps_codec->s_parse.ps_sps = ps_sps;
    ps_codec->s_parse.ps_slice_hdr = ps_slice_hdr;

    /* Derive Tile positions for the current CTB */
    /* Change this to lookup if required */
    ihevcd_get_tile_pos(ps_pps, ps_sps, ps_codec->s_parse.i4_ctb_x,
                        ps_codec->s_parse.i4_ctb_y,
                        &ps_codec->s_parse.i4_ctb_tile_x,
                        &ps_codec->s_parse.i4_ctb_tile_y,
                        &tile_idx);
    ps_codec->s_parse.ps_tile = ps_pps->ps_tile + tile_idx;
    ps_codec->s_parse.i4_cur_tile_idx = tile_idx;
    ps_tile = ps_codec->s_parse.ps_tile;
    if(tile_idx)
        ps_tile_prev = ps_tile - 1;
    else
        ps_tile_prev = ps_tile;

    /* If the present slice is dependent, then store the previous
     * independent slices' ctb x and y values for decoding process */
    if(0 == ps_codec->i4_slice_error)
    {
        if(1 == ps_slice_hdr->i1_dependent_slice_flag)
        {
            /*If slice is present at the start of a new tile*/
            if((0 == ps_codec->s_parse.i4_ctb_tile_x) && (0 == ps_codec->s_parse.i4_ctb_tile_y))
            {
                ps_codec->s_parse.i4_ctb_slice_x = 0;
                ps_codec->s_parse.i4_ctb_slice_y = 0;
            }
        }

        if(!ps_slice_hdr->i1_dependent_slice_flag)
        {
            ps_codec->s_parse.i4_ctb_slice_x = 0;
            ps_codec->s_parse.i4_ctb_slice_y = 0;
        }
    }

    /* Frame level initializations */
    if((0 == ps_codec->s_parse.i4_ctb_y) &&
                    (0 == ps_codec->s_parse.i4_ctb_x))
    {
        ret = ihevcd_parse_pic_init(ps_codec);
        RETURN_IF((ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS), ret);

        ps_codec->s_parse.pu4_pic_tu_idx[0] = 0;
        ps_codec->s_parse.pu4_pic_pu_idx[0] = 0;
        ps_codec->s_parse.i4_cur_independent_slice_idx = 0;
        ps_codec->s_parse.i4_ctb_tile_x = 0;
        ps_codec->s_parse.i4_ctb_tile_y = 0;
    }

    {
        /* Updating the poc list of current slice to ps_mv_buf */
        mv_buf_t *ps_mv_buf = ps_codec->s_parse.ps_cur_mv_buf;

        if(ps_slice_hdr->i1_num_ref_idx_l1_active != 0)
        {
            for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l1_active; i++)
            {
                ps_mv_buf->ai4_l1_collocated_poc[(ps_codec->s_parse.i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1))][i] = ((pic_buf_t *)ps_slice_hdr->as_ref_pic_list1[i].pv_pic_buf)->i4_abs_poc;
                ps_mv_buf->ai1_l1_collocated_poc_lt[(ps_codec->s_parse.i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1))][i] = ((pic_buf_t *)ps_slice_hdr->as_ref_pic_list1[i].pv_pic_buf)->u1_used_as_ref;
            }
        }

        if(ps_slice_hdr->i1_num_ref_idx_l0_active != 0)
        {
            for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l0_active; i++)
            {
                ps_mv_buf->ai4_l0_collocated_poc[(ps_codec->s_parse.i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1))][i] = ((pic_buf_t *)ps_slice_hdr->as_ref_pic_list0[i].pv_pic_buf)->i4_abs_poc;
                ps_mv_buf->ai1_l0_collocated_poc_lt[(ps_codec->s_parse.i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1))][i] = ((pic_buf_t *)ps_slice_hdr->as_ref_pic_list0[i].pv_pic_buf)->u1_used_as_ref;
            }
        }
    }

    /*Initialize the low delay flag at the beginning of every slice*/
    if((0 == ps_codec->s_parse.i4_ctb_slice_x) || (0 == ps_codec->s_parse.i4_ctb_slice_y))
    {
        /* Lowdelay flag */
        WORD32 cur_poc, ref_list_poc, flag = 1;
        cur_poc = ps_slice_hdr->i4_abs_pic_order_cnt;
        for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l0_active; i++)
        {
            ref_list_poc = ((mv_buf_t *)ps_slice_hdr->as_ref_pic_list0[i].pv_mv_buf)->i4_abs_poc;
            if(ref_list_poc > cur_poc)
            {
                flag = 0;
                break;
            }
        }
        if(flag && (ps_slice_hdr->i1_slice_type == BSLICE))
        {
            for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l1_active; i++)
            {
                ref_list_poc = ((mv_buf_t *)ps_slice_hdr->as_ref_pic_list1[i].pv_mv_buf)->i4_abs_poc;
                if(ref_list_poc > cur_poc)
                {
                    flag = 0;
                    break;
                }
            }
        }
        ps_slice_hdr->i1_low_delay_flag = flag;
    }

    /* initialize the cabac init idc based on slice type */
    if(ps_slice_hdr->i1_slice_type == ISLICE)
    {
        cabac_init_idc = 0;
    }
    else if(ps_slice_hdr->i1_slice_type == PSLICE)
    {
        cabac_init_idc = ps_slice_hdr->i1_cabac_init_flag ? 2 : 1;
    }
    else
    {
        cabac_init_idc = ps_slice_hdr->i1_cabac_init_flag ? 1 : 2;
    }

    slice_qp = ps_slice_hdr->i1_slice_qp_delta + ps_pps->i1_pic_init_qp;
    slice_qp = CLIP3(slice_qp, 0, 51);

    /*Update QP value for every indepndent slice or for every dependent slice that begins at the start of a new tile*/
    if((0 == ps_slice_hdr->i1_dependent_slice_flag) ||
                    ((1 == ps_slice_hdr->i1_dependent_slice_flag) && ((0 == ps_codec->s_parse.i4_ctb_tile_x) && (0 == ps_codec->s_parse.i4_ctb_tile_y))))
    {
        ps_codec->s_parse.u4_qp = slice_qp;
    }

    /*Cabac init at the beginning of a slice*/
    //If the slice is a dependent slice, not present at the start of a tile
    if(0 == ps_codec->i4_slice_error)
    {
        if((1 == ps_slice_hdr->i1_dependent_slice_flag) && (!((ps_codec->s_parse.i4_ctb_tile_x == 0) && (ps_codec->s_parse.i4_ctb_tile_y == 0))))
        {
            if((0 == ps_pps->i1_entropy_coding_sync_enabled_flag) || (ps_pps->i1_entropy_coding_sync_enabled_flag && (0 != ps_codec->s_parse.i4_ctb_x)))
            {
                ihevcd_cabac_reset(&ps_codec->s_parse.s_cabac,
                                   &ps_codec->s_parse.s_bitstrm);
            }
        }
        else if((0 == ps_pps->i1_entropy_coding_sync_enabled_flag) || (ps_pps->i1_entropy_coding_sync_enabled_flag && (0 != ps_codec->s_parse.i4_ctb_x)))
        {
            ret = ihevcd_cabac_init(&ps_codec->s_parse.s_cabac,
                                    &ps_codec->s_parse.s_bitstrm,
                                    slice_qp,
                                    cabac_init_idc,
                                    &gau1_ihevc_cab_ctxts[cabac_init_idc][slice_qp][0]);
            if(ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS)
            {
                ps_codec->i4_slice_error = 1;
                end_of_slice_flag = 1;
                ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
            }
        }
    }


    do
    {

        {
            WORD32 cur_ctb_idx = ps_codec->s_parse.i4_ctb_x
                            + ps_codec->s_parse.i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);
            if(1 == ps_codec->i4_num_cores && 0 == cur_ctb_idx % RESET_TU_BUF_NCTB)
            {
                ps_codec->s_parse.ps_tu = ps_codec->s_parse.ps_pic_tu;
                ps_codec->s_parse.i4_pic_tu_idx = 0;
            }
        }

        end_of_pic = 0;
        /* Section:7.3.7 Coding tree unit syntax */
        /* coding_tree_unit() inlined here */
        /* If number of cores is greater than 1, then add job to the queue */
        //TODO: Dual core implementation might need a different algo for better load balancing
        /* At the start of ctb row parsing in a tile, queue a job for processing the current tile row */
        ps_codec->s_parse.i4_ctb_num_pcm_blks = 0;


        /*At the beginning of each tile-which is not the beginning of a slice, cabac context must be initialized.
         * Hence, check for the tile beginning here */
        if(((0 == ps_codec->s_parse.i4_ctb_tile_x) && (0 == ps_codec->s_parse.i4_ctb_tile_y))
                        && (!((ps_tile->u1_pos_x == 0) && (ps_tile->u1_pos_y == 0)))
                        && (!((0 == ps_codec->s_parse.i4_ctb_slice_x) && (0 == ps_codec->s_parse.i4_ctb_slice_y))))
        {
            slice_qp = ps_slice_hdr->i1_slice_qp_delta + ps_pps->i1_pic_init_qp;
            slice_qp = CLIP3(slice_qp, 0, 51);
            ps_codec->s_parse.u4_qp = slice_qp;

            ihevcd_get_tile_pos(ps_pps, ps_sps, ps_codec->s_parse.i4_ctb_x,
                                ps_codec->s_parse.i4_ctb_y,
                                &ps_codec->s_parse.i4_ctb_tile_x,
                                &ps_codec->s_parse.i4_ctb_tile_y,
                                &tile_idx);

            ps_codec->s_parse.ps_tile = ps_pps->ps_tile + tile_idx;
            ps_codec->s_parse.i4_cur_tile_idx = tile_idx;
            ps_tile_prev = ps_tile - 1;

            tile_start_ctb_idx = ps_tile->u1_pos_x
                            + ps_tile->u1_pos_y * (ps_sps->i2_pic_wd_in_ctb);

            slice_start_ctb_idx =  ps_slice_hdr->i2_ctb_x
                            + ps_slice_hdr->i2_ctb_y * (ps_sps->i2_pic_wd_in_ctb);

            /*For slices that span across multiple tiles*/
            if(slice_start_ctb_idx < tile_start_ctb_idx)
            {       /* 2 Cases
             * 1 - slice spans across frame-width- but does not start from 1st column
             * 2 - Slice spans across multiple tiles anywhere is a frame
             */
                ps_codec->s_parse.i4_ctb_slice_y = ps_tile->u1_pos_y - ps_slice_hdr->i2_ctb_y;
                if(!(((ps_slice_hdr->i2_ctb_x + ps_tile_prev->u2_wd) % ps_sps->i2_pic_wd_in_ctb) == ps_tile->u1_pos_x)) //Case 2
                {
                    if(ps_slice_hdr->i2_ctb_y <= ps_tile->u1_pos_y)
                    {
                        //Check if ctb x is before or after
                        if(ps_slice_hdr->i2_ctb_x > ps_tile->u1_pos_x)
                        {
                            ps_codec->s_parse.i4_ctb_slice_y -= 1;
                        }
                    }
                }
                /*ps_codec->s_parse.i4_ctb_slice_y = ps_tile->u1_pos_y - ps_slice_hdr->i2_ctb_y;
                if (ps_slice_hdr->i2_ctb_y <= ps_tile->u1_pos_y)
                {
                    //Check if ctb x is before or after
                    if (ps_slice_hdr->i2_ctb_x > ps_tile->u1_pos_x )
                    {
                        ps_codec->s_parse.i4_ctb_slice_y -= 1 ;
                    }
                }*/
            }

            /* Cabac init is done unconditionally at the start of the tile irrespective
             * of whether it is a dependent or an independent slice */
            if(0 == ps_codec->i4_slice_error)
            {
                ret = ihevcd_cabac_init(&ps_codec->s_parse.s_cabac,
                                        &ps_codec->s_parse.s_bitstrm,
                                        slice_qp,
                                        cabac_init_idc,
                                        &gau1_ihevc_cab_ctxts[cabac_init_idc][slice_qp][0]);
                if(ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS)
                {
                    ps_codec->i4_slice_error = 1;
                    end_of_slice_flag = 1;
                    ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
                }

            }
        }
        /* If number of cores is greater than 1, then add job to the queue */
        //TODO: Dual core implementation might need a different algo for better load balancing
        /* At the start of ctb row parsing in a tile, queue a job for processing the current tile row */

        if(0 == ps_codec->s_parse.i4_ctb_tile_x)
        {

            if(1 < ps_codec->i4_num_cores)
            {
                proc_job_t s_job;
                IHEVCD_ERROR_T ret;
                s_job.i4_cmd    = CMD_PROCESS;
                s_job.i2_ctb_cnt = (WORD16)ps_tile->u2_wd;
                s_job.i2_ctb_x = (WORD16)ps_codec->s_parse.i4_ctb_x;
                s_job.i2_ctb_y = (WORD16)ps_codec->s_parse.i4_ctb_y;
                s_job.i2_slice_idx = (WORD16)ps_codec->s_parse.i4_cur_slice_idx;
                s_job.i4_tu_coeff_data_ofst = (UWORD8 *)ps_codec->s_parse.pv_tu_coeff_data -
                                (UWORD8 *)ps_codec->s_parse.pv_pic_tu_coeff_data;
                ret = ihevcd_jobq_queue((jobq_t *)ps_codec->s_parse.pv_proc_jobq, &s_job, sizeof(proc_job_t), 1);

                if(ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS)
                    return ret;
            }
            else
            {
                process_ctxt_t *ps_proc = &ps_codec->as_process[0];
                WORD32 tu_coeff_data_ofst = (UWORD8 *)ps_codec->s_parse.pv_tu_coeff_data -
                                (UWORD8 *)ps_codec->s_parse.pv_pic_tu_coeff_data;

                /* If the codec is running in single core mode,
                 * initialize zeroth process context
                 * TODO: Dual core mode might need a different implementation instead of jobq
                 */

                ps_proc->i4_ctb_cnt = ps_tile->u2_wd;
                ps_proc->i4_ctb_x   = ps_codec->s_parse.i4_ctb_x;
                ps_proc->i4_ctb_y   = ps_codec->s_parse.i4_ctb_y;
                ps_proc->i4_cur_slice_idx = ps_codec->s_parse.i4_cur_slice_idx;

                ihevcd_init_proc_ctxt(ps_proc, tu_coeff_data_ofst);
            }
        }


        /* Restore cabac context model from top right CTB if entropy sync is enabled */
        if(ps_pps->i1_entropy_coding_sync_enabled_flag)
        {
            /*TODO Handle single CTB and top-right belonging to a different slice */
            if(0 == ps_codec->s_parse.i4_ctb_x && 0 == ps_codec->i4_slice_error)
            {
                //WORD32 size = sizeof(ps_codec->s_parse.s_cabac.au1_ctxt_models);
                WORD32 default_ctxt = 0;

                if((0 == ps_codec->s_parse.i4_ctb_slice_y) && (!ps_slice_hdr->i1_dependent_slice_flag))
                    default_ctxt = 1;
                if(1 == ps_sps->i2_pic_wd_in_ctb)
                    default_ctxt = 1;

                ps_codec->s_parse.u4_qp = slice_qp;
                if(default_ctxt)
                {
                    //memcpy(&ps_codec->s_parse.s_cabac.au1_ctxt_models, &gau1_ihevc_cab_ctxts[cabac_init_idc][slice_qp][0], size);
                    ret = ihevcd_cabac_init(&ps_codec->s_parse.s_cabac,
                                            &ps_codec->s_parse.s_bitstrm,
                                            slice_qp,
                                            cabac_init_idc,
                                            &gau1_ihevc_cab_ctxts[cabac_init_idc][slice_qp][0]);

                    if(ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS)
                    {
                        ps_codec->i4_slice_error = 1;
                        end_of_slice_flag = 1;
                        ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
                    }
                }
                else
                {
                    //memcpy(&ps_codec->s_parse.s_cabac.au1_ctxt_models, &ps_codec->s_parse.s_cabac.au1_ctxt_models_sync, size);
                    ret = ihevcd_cabac_init(&ps_codec->s_parse.s_cabac,
                                            &ps_codec->s_parse.s_bitstrm,
                                            slice_qp,
                                            cabac_init_idc,
                                            (const UWORD8 *)&ps_codec->s_parse.s_cabac.au1_ctxt_models_sync);

                    if(ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS)
                    {
                        ps_codec->i4_slice_error = 1;
                        end_of_slice_flag = 1;
                        ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
                    }
                }
            }
        }



        if(0 == ps_codec->i4_slice_error)
        {
            if(ps_slice_hdr->i1_slice_sao_luma_flag || ps_slice_hdr->i1_slice_sao_chroma_flag)
                ihevcd_parse_sao(ps_codec);
        }
        else
        {
            sao_t *ps_sao = ps_codec->s_parse.ps_pic_sao +
                            ps_codec->s_parse.i4_ctb_x +
                            ps_codec->s_parse.i4_ctb_y * ps_sps->i2_pic_wd_in_ctb;

            /* Default values */
            ps_sao->b3_y_type_idx = 0;
            ps_sao->b3_cb_type_idx = 0;
            ps_sao->b3_cr_type_idx = 0;
        }

        //AEV_TRACE("CTB x", ps_codec->s_parse.i4_ctb_x, 0);
        //AEV_TRACE("CTB y", ps_codec->s_parse.i4_ctb_y, 0);

        {
            WORD32 ctb_indx;
            ctb_indx = ps_codec->s_parse.i4_ctb_x + ps_sps->i2_pic_wd_in_ctb * ps_codec->s_parse.i4_ctb_y;
            ps_codec->s_parse.s_bs_ctxt.pu1_pic_qp_const_in_ctb[ctb_indx >> 3] |= (1 << (ctb_indx & 7));
            {
                UWORD16 *pu1_slice_idx = ps_codec->s_parse.pu1_slice_idx;
                pu1_slice_idx[ctb_indx] = ps_codec->s_parse.i4_cur_independent_slice_idx;
            }
        }

        if(0 == ps_codec->i4_slice_error)
        {
            tu_t *ps_tu = ps_codec->s_parse.ps_tu;
            WORD32 i4_tu_cnt = ps_codec->s_parse.s_cu.i4_tu_cnt;
            WORD32 i4_pic_tu_idx = ps_codec->s_parse.i4_pic_tu_idx;

            pu_t *ps_pu = ps_codec->s_parse.ps_pu;
            WORD32 i4_pic_pu_idx = ps_codec->s_parse.i4_pic_pu_idx;

            UWORD8 *pu1_tu_coeff_data = (UWORD8 *)ps_codec->s_parse.pv_tu_coeff_data;

            ret = ihevcd_parse_coding_quadtree(ps_codec,
                                               (ps_codec->s_parse.i4_ctb_x << ps_sps->i1_log2_ctb_size),
                                               (ps_codec->s_parse.i4_ctb_y << ps_sps->i1_log2_ctb_size),
                                               ps_sps->i1_log2_ctb_size,
                                               0);
            /* Check for error */
            if (ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS)
            {
                /* Reset tu and pu parameters, and signal current ctb as skip */
                WORD32 tu_coeff_data_reset_size;

                ps_codec->s_parse.ps_tu = ps_tu;
                ps_codec->s_parse.s_cu.i4_tu_cnt = i4_tu_cnt;
                ps_codec->s_parse.i4_pic_tu_idx = i4_pic_tu_idx;

                ps_codec->s_parse.ps_pu = ps_pu;
                ps_codec->s_parse.i4_pic_pu_idx = i4_pic_pu_idx;

                tu_coeff_data_reset_size = (UWORD8 *)ps_codec->s_parse.pv_tu_coeff_data - pu1_tu_coeff_data;
                memset(pu1_tu_coeff_data, 0, tu_coeff_data_reset_size);
                ps_codec->s_parse.pv_tu_coeff_data = (void *)pu1_tu_coeff_data;

                ihevcd_set_ctb_skip(ps_codec);

                /* Set slice error to suppress further parsing and
                 * signal end of slice.
                 */
                ps_codec->i4_slice_error = 1;
                end_of_slice_flag = 1;
                ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
            }
        }
        else
        {
            ihevcd_set_ctb_skip(ps_codec);
        }

        if(0 == ps_codec->i4_slice_error)
            end_of_slice_flag = ihevcd_cabac_decode_terminate(&ps_codec->s_parse.s_cabac, &ps_codec->s_parse.s_bitstrm);

        AEV_TRACE("end_of_slice_flag", end_of_slice_flag, ps_codec->s_parse.s_cabac.u4_range);


        /* In case of tiles or entropy sync, terminate cabac and copy cabac context backed up at the end of top-right CTB */
        if(ps_pps->i1_tiles_enabled_flag || ps_pps->i1_entropy_coding_sync_enabled_flag)
        {
            WORD32 end_of_tile = 0;
            WORD32 end_of_tile_row = 0;

            /* Take a back up of cabac context models if entropy sync is enabled */
            if(ps_pps->i1_entropy_coding_sync_enabled_flag || ps_pps->i1_tiles_enabled_flag)
            {
                if(1 == ps_codec->s_parse.i4_ctb_x)
                {
                    WORD32 size = sizeof(ps_codec->s_parse.s_cabac.au1_ctxt_models);
                    memcpy(&ps_codec->s_parse.s_cabac.au1_ctxt_models_sync, &ps_codec->s_parse.s_cabac.au1_ctxt_models, size);
                }
            }

            /* Since tiles and entropy sync are not enabled simultaneously, the following will not result in any problems */
            if((ps_codec->s_parse.i4_ctb_tile_x + 1) == (ps_tile->u2_wd))
            {
                end_of_tile_row = 1;
                if((ps_codec->s_parse.i4_ctb_tile_y + 1) == ps_tile->u2_ht)
                    end_of_tile = 1;
            }
            if((0 == end_of_slice_flag) && (0 == ps_codec->i4_slice_error) &&
                            ((ps_pps->i1_tiles_enabled_flag && end_of_tile) ||
                                            (ps_pps->i1_entropy_coding_sync_enabled_flag && end_of_tile_row)))
            {
                WORD32 end_of_sub_stream_one_bit;
                end_of_sub_stream_one_bit = ihevcd_cabac_decode_terminate(&ps_codec->s_parse.s_cabac, &ps_codec->s_parse.s_bitstrm);
                AEV_TRACE("end_of_sub_stream_one_bit", end_of_sub_stream_one_bit, ps_codec->s_parse.s_cabac.u4_range);

                /* TODO: Remove the check for offset when HM is updated to include a byte unconditionally even for aligned location */
                /* For Ittiam streams this check should not be there, for HM9.1 streams this should be there */
                if(ps_codec->s_parse.s_bitstrm.u4_bit_ofst % 8)
                    ihevcd_bits_flush_to_byte_boundary(&ps_codec->s_parse.s_bitstrm);

                UNUSED(end_of_sub_stream_one_bit);
            }
        }
        {
            WORD32 ctb_indx;

            ctb_addr = ps_codec->s_parse.i4_ctb_y * num_ctb_in_row + ps_codec->s_parse.i4_ctb_x;

            ctb_indx = ++ctb_addr;

            /* Store pu_idx for next CTB in frame level pu_idx array */

            //In case of multiple tiles, if end-of-tile row is reached
            if((ps_tile->u2_wd == (ps_codec->s_parse.i4_ctb_tile_x + 1)) && (ps_tile->u2_wd != ps_sps->i2_pic_wd_in_ctb))
            {
                ctb_indx = (ps_sps->i2_pic_wd_in_ctb * (ps_codec->s_parse.i4_ctb_tile_y + 1 + ps_tile->u1_pos_y)) + ps_tile->u1_pos_x; //idx is the beginning of next row in current tile.
                if(ps_tile->u2_ht == (ps_codec->s_parse.i4_ctb_tile_y + 1))
                {
                    //If the current ctb is the last tile's last ctb
                    if((ps_tile->u2_wd + ps_tile->u1_pos_x == ps_sps->i2_pic_wd_in_ctb) && ((ps_tile->u2_ht + ps_tile->u1_pos_y == ps_sps->i2_pic_ht_in_ctb)))
                    {
                        ctb_indx = ctb_addr; //Next continuous ctb address
                    }
                    else //Not last tile's end , but a tile end
                    {
                        tile_t *ps_next_tile = ps_codec->s_parse.ps_tile + 1;
                        ctb_indx = ps_next_tile->u1_pos_x + (ps_next_tile->u1_pos_y * ps_sps->i2_pic_wd_in_ctb); //idx is the beginning of first row in next tile.
                    }
                }
            }

            ps_codec->s_parse.pu4_pic_pu_idx[ctb_indx] = ps_codec->s_parse.i4_pic_pu_idx;
            ps_codec->s_parse.i4_next_pu_ctb_cnt = ctb_indx;

            ps_codec->s_parse.pu1_pu_map += num_min4x4_in_ctb;

            /* Store tu_idx for next CTB in frame level tu_idx array */
            if(1 == ps_codec->i4_num_cores)
            {
                ctb_indx = (0 == ctb_addr % RESET_TU_BUF_NCTB) ?
                                RESET_TU_BUF_NCTB : ctb_addr % RESET_TU_BUF_NCTB;

                //In case of multiple tiles, if end-of-tile row is reached
                if((ps_tile->u2_wd == (ps_codec->s_parse.i4_ctb_tile_x + 1)) && (ps_tile->u2_wd != ps_sps->i2_pic_wd_in_ctb))
                {
                    ctb_indx = (ps_sps->i2_pic_wd_in_ctb * (ps_codec->s_parse.i4_ctb_tile_y + 1 + ps_tile->u1_pos_y)) + ps_tile->u1_pos_x; //idx is the beginning of next row in current tile.
                    if(ps_tile->u2_ht == (ps_codec->s_parse.i4_ctb_tile_y + 1))
                    {
                        //If the current ctb is the last tile's last ctb
                        if((ps_tile->u2_wd + ps_tile->u1_pos_x == ps_sps->i2_pic_wd_in_ctb) && ((ps_tile->u2_ht + ps_tile->u1_pos_y == ps_sps->i2_pic_ht_in_ctb)))
                        {
                            ctb_indx = (0 == ctb_addr % RESET_TU_BUF_NCTB) ?
                                            RESET_TU_BUF_NCTB : ctb_addr % RESET_TU_BUF_NCTB;
                        }
                        else  //Not last tile's end , but a tile end
                        {
                            tile_t *ps_next_tile = ps_codec->s_parse.ps_tile + 1;
                            ctb_indx =  ps_next_tile->u1_pos_x + (ps_next_tile->u1_pos_y * ps_sps->i2_pic_wd_in_ctb); //idx is the beginning of first row in next tile.
                        }
                    }
                }
                ps_codec->s_parse.i4_next_tu_ctb_cnt = ctb_indx;
                ps_codec->s_parse.pu4_pic_tu_idx[ctb_indx] = ps_codec->s_parse.i4_pic_tu_idx;
            }
            else
            {
                ctb_indx = ctb_addr;
                if((ps_tile->u2_wd == (ps_codec->s_parse.i4_ctb_tile_x + 1)) && (ps_tile->u2_wd != ps_sps->i2_pic_wd_in_ctb))
                {
                    ctb_indx = (ps_sps->i2_pic_wd_in_ctb * (ps_codec->s_parse.i4_ctb_tile_y + 1 + ps_tile->u1_pos_y)) + ps_tile->u1_pos_x; //idx is the beginning of next row in current tile.
                    if(ps_tile->u2_ht == (ps_codec->s_parse.i4_ctb_tile_y + 1))
                    {
                        //If the current ctb is the last tile's last ctb
                        if((ps_tile->u2_wd + ps_tile->u1_pos_x == ps_sps->i2_pic_wd_in_ctb) && ((ps_tile->u2_ht + ps_tile->u1_pos_y == ps_sps->i2_pic_ht_in_ctb)))
                        {
                            ctb_indx = ctb_addr;
                        }
                        else  //Not last tile's end , but a tile end
                        {
                            tile_t *ps_next_tile = ps_codec->s_parse.ps_tile + 1;
                            ctb_indx =  ps_next_tile->u1_pos_x + (ps_next_tile->u1_pos_y * ps_sps->i2_pic_wd_in_ctb); //idx is the beginning of first row in next tile.
                        }
                    }
                }
                ps_codec->s_parse.i4_next_tu_ctb_cnt = ctb_indx;
                ps_codec->s_parse.pu4_pic_tu_idx[ctb_indx] = ps_codec->s_parse.i4_pic_tu_idx;
            }
            ps_codec->s_parse.pu1_tu_map += num_min4x4_in_ctb;
        }

        /* QP array population has to be done if deblocking is enabled in the picture
         * but some of the slices in the pic have it disabled */
        if((0 != ps_codec->i4_disable_deblk_pic) &&
                (1 == ps_slice_hdr->i1_slice_disable_deblocking_filter_flag))
        {
            bs_ctxt_t *ps_bs_ctxt = &ps_codec->s_parse.s_bs_ctxt;
            WORD32 log2_ctb_size = ps_sps->i1_log2_ctb_size;
            UWORD8 *pu1_qp;
            WORD32 qp_strd;
            WORD32 u4_qp_const_in_ctb;
            WORD32 cur_ctb_idx;
            WORD32 next_ctb_idx;
            WORD32 cur_tu_idx;
            WORD32 i4_ctb_tu_cnt;
            tu_t *ps_tu;

            cur_ctb_idx = ps_codec->s_parse.i4_ctb_x + ps_sps->i2_pic_wd_in_ctb * ps_codec->s_parse.i4_ctb_y;
            /* ctb_size/8 elements per CTB */
            qp_strd = ps_sps->i2_pic_wd_in_ctb << (log2_ctb_size - 3);
            pu1_qp = ps_bs_ctxt->pu1_pic_qp + ((ps_codec->s_parse.i4_ctb_x + ps_codec->s_parse.i4_ctb_y * qp_strd) << (log2_ctb_size - 3));

            u4_qp_const_in_ctb = ps_bs_ctxt->pu1_pic_qp_const_in_ctb[cur_ctb_idx >> 3] & (1 << (cur_ctb_idx & 7));

            next_ctb_idx = ps_codec->s_parse.i4_next_tu_ctb_cnt;
            if(1 == ps_codec->i4_num_cores)
            {
                i4_ctb_tu_cnt = ps_codec->s_parse.pu4_pic_tu_idx[next_ctb_idx] -
                                ps_codec->s_parse.pu4_pic_tu_idx[cur_ctb_idx % RESET_TU_BUF_NCTB];

                cur_tu_idx = ps_codec->s_parse.pu4_pic_tu_idx[cur_ctb_idx % RESET_TU_BUF_NCTB];
            }
            else
            {
                i4_ctb_tu_cnt = ps_codec->s_parse.pu4_pic_tu_idx[next_ctb_idx] -
                                ps_codec->s_parse.pu4_pic_tu_idx[cur_ctb_idx];

                cur_tu_idx = ps_codec->s_parse.pu4_pic_tu_idx[cur_ctb_idx];
            }

            ps_tu = &ps_codec->s_parse.ps_pic_tu[cur_tu_idx];

            if(u4_qp_const_in_ctb)
            {
                pu1_qp[0] = ps_tu->b7_qp;
            }
            else
            {
                for(i = 0; i < i4_ctb_tu_cnt; i++, ps_tu++)
                {
                    WORD32 start_pos_x;
                    WORD32 start_pos_y;
                    WORD32 tu_size;

                    /* start_pos_x and start_pos_y are in units of min TU size (4x4) */
                    start_pos_x = ps_tu->b4_pos_x;
                    start_pos_y = ps_tu->b4_pos_y;

                    tu_size = 1 << (ps_tu->b3_size + 2);
                    tu_size >>= 2; /* TU size divided by 4 */

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
        }

        if(ps_codec->i4_num_cores <= MV_PRED_NUM_CORES_THRESHOLD)
        {
            /*************************************************/
            /****************   MV pred **********************/
            /*************************************************/
            WORD8 u1_top_ctb_avail = 1;
            WORD8 u1_left_ctb_avail = 1;
            WORD8 u1_top_lt_ctb_avail = 1;
            WORD8 u1_top_rt_ctb_avail = 1;
            WORD16 i2_wd_in_ctb;

            tile_start_ctb_idx = ps_tile->u1_pos_x
                            + ps_tile->u1_pos_y * (ps_sps->i2_pic_wd_in_ctb);

            slice_start_ctb_idx =  ps_slice_hdr->i2_ctb_x
                            + ps_slice_hdr->i2_ctb_y * (ps_sps->i2_pic_wd_in_ctb);

            if((slice_start_ctb_idx < tile_start_ctb_idx))
            {
                //Slices span across multiple tiles.
                i2_wd_in_ctb = ps_sps->i2_pic_wd_in_ctb;
            }
            else
            {
                i2_wd_in_ctb = ps_tile->u2_wd;
            }
            /* slice and tile boundaries */
            if((0 == ps_codec->s_parse.i4_ctb_y) || (0 == ps_codec->s_parse.i4_ctb_tile_y))
            {
                u1_top_ctb_avail = 0;
                u1_top_lt_ctb_avail = 0;
                u1_top_rt_ctb_avail = 0;
            }

            if((0 == ps_codec->s_parse.i4_ctb_x) || (0 == ps_codec->s_parse.i4_ctb_tile_x))
            {
                u1_left_ctb_avail = 0;
                u1_top_lt_ctb_avail = 0;
                if((0 == ps_codec->s_parse.i4_ctb_slice_y) || (0 == ps_codec->s_parse.i4_ctb_tile_y))
                {
                    u1_top_ctb_avail = 0;
                    if((i2_wd_in_ctb - 1) != ps_codec->s_parse.i4_ctb_slice_x) //TODO: For tile, not implemented
                    {
                        u1_top_rt_ctb_avail = 0;
                    }
                }
            }
            /*For slices not beginning at start of a ctb row*/
            else if(ps_codec->s_parse.i4_ctb_x > 0)
            {
                if((0 == ps_codec->s_parse.i4_ctb_slice_y) || (0 == ps_codec->s_parse.i4_ctb_tile_y))
                {
                    u1_top_ctb_avail = 0;
                    u1_top_lt_ctb_avail = 0;
                    if(0 == ps_codec->s_parse.i4_ctb_slice_x)
                    {
                        u1_left_ctb_avail = 0;
                    }
                    if((i2_wd_in_ctb - 1) != ps_codec->s_parse.i4_ctb_slice_x)
                    {
                        u1_top_rt_ctb_avail = 0;
                    }
                }
                else if((1 == ps_codec->s_parse.i4_ctb_slice_y) && (0 == ps_codec->s_parse.i4_ctb_slice_x))
                {
                    u1_top_lt_ctb_avail = 0;
                }
            }

            if(((ps_sps->i2_pic_wd_in_ctb - 1) == ps_codec->s_parse.i4_ctb_x) || ((ps_tile->u2_wd - 1) == ps_codec->s_parse.i4_ctb_tile_x))
            {
                u1_top_rt_ctb_avail = 0;
            }

            if(PSLICE == ps_slice_hdr->i1_slice_type
                            || BSLICE == ps_slice_hdr->i1_slice_type)
            {
                mv_ctxt_t s_mv_ctxt;
                process_ctxt_t *ps_proc;
                UWORD32 *pu4_ctb_top_pu_idx;
                UWORD32 *pu4_ctb_left_pu_idx;
                UWORD32 *pu4_ctb_top_left_pu_idx;
                WORD32 i4_ctb_pu_cnt;
                WORD32 cur_ctb_idx;
                WORD32 next_ctb_idx;
                WORD32 cur_pu_idx;
                ps_proc = &ps_codec->as_process[(ps_codec->i4_num_cores == 1) ? 1 : (ps_codec->i4_num_cores - 1)];
                cur_ctb_idx = ps_codec->s_parse.i4_ctb_x
                                + ps_codec->s_parse.i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);
                next_ctb_idx = ps_codec->s_parse.i4_next_pu_ctb_cnt;
                i4_ctb_pu_cnt = ps_codec->s_parse.pu4_pic_pu_idx[next_ctb_idx]
                                - ps_codec->s_parse.pu4_pic_pu_idx[cur_ctb_idx];

                cur_pu_idx = ps_codec->s_parse.pu4_pic_pu_idx[cur_ctb_idx];

                pu4_ctb_top_pu_idx = ps_proc->pu4_pic_pu_idx_top
                                + (ps_codec->s_parse.i4_ctb_x * ctb_size / MIN_PU_SIZE);
                pu4_ctb_left_pu_idx = ps_proc->pu4_pic_pu_idx_left;
                pu4_ctb_top_left_pu_idx = &ps_proc->u4_ctb_top_left_pu_idx;

                /* Initializing s_mv_ctxt */
                {
                    s_mv_ctxt.ps_pps = ps_pps;
                    s_mv_ctxt.ps_sps = ps_sps;
                    s_mv_ctxt.ps_slice_hdr = ps_slice_hdr;
                    s_mv_ctxt.i4_ctb_x = ps_codec->s_parse.i4_ctb_x;
                    s_mv_ctxt.i4_ctb_y = ps_codec->s_parse.i4_ctb_y;
                    s_mv_ctxt.ps_pu = &ps_codec->s_parse.ps_pic_pu[cur_pu_idx];
                    s_mv_ctxt.ps_pic_pu = ps_codec->s_parse.ps_pic_pu;
                    s_mv_ctxt.ps_tile = ps_tile;
                    s_mv_ctxt.pu4_pic_pu_idx_map = ps_proc->pu4_pic_pu_idx_map;
                    s_mv_ctxt.pu4_pic_pu_idx = ps_codec->s_parse.pu4_pic_pu_idx;
                    s_mv_ctxt.pu1_pic_pu_map = ps_codec->s_parse.pu1_pic_pu_map;
                    s_mv_ctxt.i4_ctb_pu_cnt = i4_ctb_pu_cnt;
                    s_mv_ctxt.i4_ctb_start_pu_idx = cur_pu_idx;
                    s_mv_ctxt.u1_top_ctb_avail = u1_top_ctb_avail;
                    s_mv_ctxt.u1_top_rt_ctb_avail = u1_top_rt_ctb_avail;
                    s_mv_ctxt.u1_top_lt_ctb_avail = u1_top_lt_ctb_avail;
                    s_mv_ctxt.u1_left_ctb_avail = u1_left_ctb_avail;
                }

                ihevcd_get_mv_ctb(&s_mv_ctxt, pu4_ctb_top_pu_idx,
                                  pu4_ctb_left_pu_idx, pu4_ctb_top_left_pu_idx);

            }
            else
            {
                WORD32 num_minpu_in_ctb = (ctb_size / MIN_PU_SIZE) * (ctb_size / MIN_PU_SIZE);
                UWORD8 *pu1_pic_pu_map_ctb = ps_codec->s_parse.pu1_pic_pu_map +
                                (ps_codec->s_parse.i4_ctb_x + ps_codec->s_parse.i4_ctb_y * ps_sps->i2_pic_wd_in_ctb) * num_minpu_in_ctb;
                process_ctxt_t *ps_proc = &ps_codec->as_process[(ps_codec->i4_num_cores == 1) ? 1 : (ps_codec->i4_num_cores - 1)];
                WORD32 row, col;
                WORD32 pu_cnt;
                WORD32 num_pu_per_ctb;
                WORD32 cur_ctb_idx;
                WORD32 next_ctb_idx;
                WORD32 ctb_start_pu_idx;
                UWORD32 *pu4_nbr_pu_idx = ps_proc->pu4_pic_pu_idx_map;
                WORD32 nbr_pu_idx_strd = MAX_CTB_SIZE / MIN_PU_SIZE + 2;
                pu_t *ps_pu;
                WORD32 ctb_size_in_min_pu = (ctb_size / MIN_PU_SIZE);

                /* Neighbor PU idx update inside CTB */
                /* 1byte per 4x4. Indicates the PU idx that 4x4 block belongs to */

                cur_ctb_idx = ps_codec->s_parse.i4_ctb_x
                                + ps_codec->s_parse.i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);
                next_ctb_idx = ps_codec->s_parse.i4_next_pu_ctb_cnt;
                num_pu_per_ctb = ps_codec->s_parse.pu4_pic_pu_idx[next_ctb_idx]
                                - ps_codec->s_parse.pu4_pic_pu_idx[cur_ctb_idx];
                ctb_start_pu_idx = ps_codec->s_parse.pu4_pic_pu_idx[cur_ctb_idx];
                ps_pu = &ps_codec->s_parse.ps_pic_pu[ctb_start_pu_idx];

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
                                    - (ps_codec->s_parse.i4_ctb_y << ps_sps->i1_log2_ctb_size);
                    WORD32 ctb_size_left = MIN(ctb_size, rows_remaining);

                    /* Top Left */
                    /* saving top left before updating top ptr, as updating top ptr will overwrite the top left for the next ctb */
                    ps_proc->u4_ctb_top_left_pu_idx = ps_proc->pu4_pic_pu_idx_top[(ps_codec->s_parse.i4_ctb_x * ctb_size / MIN_PU_SIZE) + ctb_size / MIN_PU_SIZE - 1];
                    for(i = 0; i < ctb_size / MIN_PU_SIZE; i++)
                    {
                        /* Left */
                        /* Last column of au4_nbr_pu_idx */
                        ps_proc->pu4_pic_pu_idx_left[i] = pu4_nbr_pu_idx[(ctb_size / MIN_PU_SIZE)
                                        + (i + 1) * nbr_pu_idx_strd];
                        /* Top */
                        /* Last row of au4_nbr_pu_idx */
                        ps_proc->pu4_pic_pu_idx_top[(ps_codec->s_parse.i4_ctb_x * ctb_size / MIN_PU_SIZE) + i] =
                                        pu4_nbr_pu_idx[(ctb_size_left / MIN_PU_SIZE) * nbr_pu_idx_strd + i + 1];

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
            }

            /*************************************************/
            /******************  BS, QP  *********************/
            /*************************************************/
            /* Check if deblock is disabled for the current slice or if it is disabled for the current picture
             * because of disable deblock api
             */
            if(0 == ps_codec->i4_disable_deblk_pic)
            {
                /* Boundary strength calculation is done irrespective of whether deblocking is disabled
                 * in the slice or not, to handle deblocking slice boundaries */
                if((0 == ps_codec->i4_slice_error))
                {
                    WORD32 i4_ctb_tu_cnt;
                    WORD32 cur_ctb_idx, next_ctb_idx;
                    WORD32 cur_pu_idx;
                    WORD32 cur_tu_idx;
                    process_ctxt_t *ps_proc;

                    ps_proc = &ps_codec->as_process[(ps_codec->i4_num_cores == 1) ? 1 : (ps_codec->i4_num_cores - 1)];
                    cur_ctb_idx = ps_codec->s_parse.i4_ctb_x
                                    + ps_codec->s_parse.i4_ctb_y * (ps_sps->i2_pic_wd_in_ctb);

                    cur_pu_idx = ps_codec->s_parse.pu4_pic_pu_idx[cur_ctb_idx];
                    next_ctb_idx = ps_codec->s_parse.i4_next_tu_ctb_cnt;
                    if(1 == ps_codec->i4_num_cores)
                    {
                        i4_ctb_tu_cnt = ps_codec->s_parse.pu4_pic_tu_idx[next_ctb_idx] -
                                        ps_codec->s_parse.pu4_pic_tu_idx[cur_ctb_idx % RESET_TU_BUF_NCTB];

                        cur_tu_idx = ps_codec->s_parse.pu4_pic_tu_idx[cur_ctb_idx % RESET_TU_BUF_NCTB];
                    }
                    else
                    {
                        i4_ctb_tu_cnt = ps_codec->s_parse.pu4_pic_tu_idx[next_ctb_idx] -
                                        ps_codec->s_parse.pu4_pic_tu_idx[cur_ctb_idx];

                        cur_tu_idx = ps_codec->s_parse.pu4_pic_tu_idx[cur_ctb_idx];
                    }

                    ps_codec->s_parse.s_bs_ctxt.ps_pps = ps_codec->s_parse.ps_pps;
                    ps_codec->s_parse.s_bs_ctxt.ps_sps = ps_codec->s_parse.ps_sps;
                    ps_codec->s_parse.s_bs_ctxt.ps_codec = ps_codec;
                    ps_codec->s_parse.s_bs_ctxt.i4_ctb_tu_cnt = i4_ctb_tu_cnt;
                    ps_codec->s_parse.s_bs_ctxt.i4_ctb_x = ps_codec->s_parse.i4_ctb_x;
                    ps_codec->s_parse.s_bs_ctxt.i4_ctb_y = ps_codec->s_parse.i4_ctb_y;
                    ps_codec->s_parse.s_bs_ctxt.i4_ctb_tile_x = ps_codec->s_parse.i4_ctb_tile_x;
                    ps_codec->s_parse.s_bs_ctxt.i4_ctb_tile_y = ps_codec->s_parse.i4_ctb_tile_y;
                    ps_codec->s_parse.s_bs_ctxt.i4_ctb_slice_x = ps_codec->s_parse.i4_ctb_slice_x;
                    ps_codec->s_parse.s_bs_ctxt.i4_ctb_slice_y = ps_codec->s_parse.i4_ctb_slice_y;
                    ps_codec->s_parse.s_bs_ctxt.ps_tu = &ps_codec->s_parse.ps_pic_tu[cur_tu_idx];
                    ps_codec->s_parse.s_bs_ctxt.ps_pu = &ps_codec->s_parse.ps_pic_pu[cur_pu_idx];
                    ps_codec->s_parse.s_bs_ctxt.pu4_pic_pu_idx_map = ps_proc->pu4_pic_pu_idx_map;
                    ps_codec->s_parse.s_bs_ctxt.i4_next_pu_ctb_cnt = ps_codec->s_parse.i4_next_pu_ctb_cnt;
                    ps_codec->s_parse.s_bs_ctxt.i4_next_tu_ctb_cnt = ps_codec->s_parse.i4_next_tu_ctb_cnt;
                    ps_codec->s_parse.s_bs_ctxt.pu1_slice_idx = ps_codec->s_parse.pu1_slice_idx;
                    ps_codec->s_parse.s_bs_ctxt.ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr;
                    ps_codec->s_parse.s_bs_ctxt.ps_tile = ps_codec->s_parse.ps_tile;

                    if(ISLICE == ps_slice_hdr->i1_slice_type)
                    {
                        ihevcd_ctb_boundary_strength_islice(&ps_codec->s_parse.s_bs_ctxt);
                    }
                    else
                    {
                        ihevcd_ctb_boundary_strength_pbslice(&ps_codec->s_parse.s_bs_ctxt);
                    }
                }

                /* Boundary strength is set to zero if deblocking is disabled for the current slice */
                if(0 != ps_slice_hdr->i1_slice_disable_deblocking_filter_flag)
                {
                    WORD32 bs_strd = (ps_sps->i2_pic_wd_in_ctb + 1) * (ctb_size * ctb_size / 8 / 16);

                    UWORD32 *pu4_vert_bs = (UWORD32 *)((UWORD8 *)ps_codec->s_parse.s_bs_ctxt.pu4_pic_vert_bs +
                                    ps_codec->s_parse.i4_ctb_x * (ctb_size * ctb_size / 8 / 16) +
                                    ps_codec->s_parse.i4_ctb_y * bs_strd);
                    UWORD32 *pu4_horz_bs = (UWORD32 *)((UWORD8 *)ps_codec->s_parse.s_bs_ctxt.pu4_pic_horz_bs +
                                    ps_codec->s_parse.i4_ctb_x * (ctb_size * ctb_size / 8 / 16) +
                                    ps_codec->s_parse.i4_ctb_y * bs_strd);

                    memset(pu4_vert_bs, 0, (ctb_size / 8) * (ctb_size / 4) / 8 * 2);
                    memset(pu4_horz_bs, 0, (ctb_size / 8) * (ctb_size / 4) / 8 * 2);
                }
            }

        }

        DATA_SYNC();

        /* Update the parse status map */
        {
            sps_t *ps_sps = ps_codec->s_parse.ps_sps;
            UWORD8 *pu1_buf;
            WORD32 idx;
            idx = (ps_codec->s_parse.i4_ctb_x);
            idx += ((ps_codec->s_parse.i4_ctb_y) * ps_sps->i2_pic_wd_in_ctb);
            pu1_buf = (ps_codec->pu1_parse_map + idx);
            *pu1_buf = 1;
        }

        /* Increment CTB x and y positions */
        ps_codec->s_parse.i4_ctb_tile_x++;
        ps_codec->s_parse.i4_ctb_x++;
        ps_codec->s_parse.i4_ctb_slice_x++;

        /*If tiles are enabled, handle the slice counters differently*/
        if(ps_pps->i1_tiles_enabled_flag)
        {
            //Indicates multiple tiles in a slice case
            tile_start_ctb_idx = ps_tile->u1_pos_x
                            + ps_tile->u1_pos_y * (ps_sps->i2_pic_wd_in_ctb);

            slice_start_ctb_idx =  ps_slice_hdr->i2_ctb_x
                            + ps_slice_hdr->i2_ctb_y * (ps_sps->i2_pic_wd_in_ctb);

            if((slice_start_ctb_idx < tile_start_ctb_idx))
            {
                if(ps_codec->s_parse.i4_ctb_slice_x == (ps_tile->u1_pos_x + ps_tile->u2_wd))
                {
                    /* Reached end of slice row within a tile /frame */
                    ps_codec->s_parse.i4_ctb_slice_y++;
                    ps_codec->s_parse.i4_ctb_slice_x = ps_tile->u1_pos_x; //todo:Check
                }
            }
            //Indicates multiple slices in a tile case - hence, reset slice_x
            else if(ps_codec->s_parse.i4_ctb_slice_x == (ps_tile->u2_wd))
            {
                ps_codec->s_parse.i4_ctb_slice_y++;
                ps_codec->s_parse.i4_ctb_slice_x = 0;
            }
        }
        else
        {
            if(ps_codec->s_parse.i4_ctb_slice_x == ps_tile->u2_wd)
            {
                /* Reached end of slice row within a tile /frame */
                ps_codec->s_parse.i4_ctb_slice_y++;
                ps_codec->s_parse.i4_ctb_slice_x = 0;
            }
        }


        if(ps_codec->s_parse.i4_ctb_tile_x == (ps_tile->u2_wd))
        {
            /* Reached end of tile row */
            ps_codec->s_parse.i4_ctb_tile_x = 0;
            ps_codec->s_parse.i4_ctb_x = ps_tile->u1_pos_x;

            ps_codec->s_parse.i4_ctb_tile_y++;
            ps_codec->s_parse.i4_ctb_y++;

            if(ps_codec->s_parse.i4_ctb_tile_y == (ps_tile->u2_ht))
            {
                /* Reached End of Tile */
                ps_codec->s_parse.i4_ctb_tile_y = 0;
                ps_codec->s_parse.i4_ctb_tile_x = 0;
                ps_codec->s_parse.ps_tile++;

                if((ps_tile->u2_ht + ps_tile->u1_pos_y  ==  ps_sps->i2_pic_ht_in_ctb) && (ps_tile->u2_wd + ps_tile->u1_pos_x  ==  ps_sps->i2_pic_wd_in_ctb))
                {
                    /* Reached end of frame */
                    end_of_pic = 1;
                    ps_codec->s_parse.i4_ctb_x = 0;
                    ps_codec->s_parse.i4_ctb_y = ps_sps->i2_pic_ht_in_ctb;
                }
                else
                {
                    /* Initialize ctb_x and ctb_y to start of next tile */
                    ps_tile = ps_codec->s_parse.ps_tile;
                    ps_codec->s_parse.i4_ctb_x = ps_tile->u1_pos_x;
                    ps_codec->s_parse.i4_ctb_y = ps_tile->u1_pos_y;
                    ps_codec->s_parse.i4_ctb_tile_y = 0;
                    ps_codec->s_parse.i4_ctb_tile_x = 0;
                    ps_codec->s_parse.i4_ctb_slice_x = ps_tile->u1_pos_x;
                    ps_codec->s_parse.i4_ctb_slice_y = ps_tile->u1_pos_y;

                }
            }

        }

        ps_codec->s_parse.i4_next_ctb_indx = ps_codec->s_parse.i4_ctb_x +
                        ps_codec->s_parse.i4_ctb_y * ps_sps->i2_pic_wd_in_ctb;

        /* If the current slice is in error, check if the next slice's address
         * is reached and mark the end_of_slice flag */
        if(ps_codec->i4_slice_error)
        {
            slice_header_t *ps_slice_hdr_next = ps_slice_hdr + 1;
            WORD32 next_slice_addr = ps_slice_hdr_next->i2_ctb_x +
                            ps_slice_hdr_next->i2_ctb_y * ps_sps->i2_pic_wd_in_ctb;

            if(ps_codec->s_parse.i4_next_ctb_indx == next_slice_addr)
                end_of_slice_flag = 1;
        }

        /* If the codec is running in single core mode
         * then call process function for current CTB
         */
        if((1 == ps_codec->i4_num_cores) && (ps_codec->s_parse.i4_ctb_tile_x == 0))
        {
            process_ctxt_t *ps_proc = &ps_codec->as_process[0];
//          ps_proc->i4_ctb_cnt = ihevcd_nctb_cnt(ps_codec, ps_sps);
            ps_proc->i4_ctb_cnt = ps_proc->ps_tile->u2_wd;
            ihevcd_process(ps_proc);
        }

        /* If the bytes for the current slice are exhausted
         * set end_of_slice flag to 1
         * This slice will be treated as incomplete */
        if((UWORD8 *)ps_codec->s_parse.s_bitstrm.pu1_buf_max + BITSTRM_OFF_THRS <
                                        ((UWORD8 *)ps_codec->s_parse.s_bitstrm.pu4_buf + (ps_codec->s_parse.s_bitstrm.u4_bit_ofst / 8)))
        {
            // end_of_slice_flag = ps_codec->i4_slice_error ? 0 : 1;

            if(0 == ps_codec->i4_slice_error)
                end_of_slice_flag = 1;
        }


        if(end_of_pic)
            break;
    } while(!end_of_slice_flag);

    /* Reset slice error */
    ps_codec->i4_slice_error = 0;

    /* Increment the slice index for parsing next slice */
    if(0 == end_of_pic)
    {
        while(1)
        {

            WORD32 parse_slice_idx;
            parse_slice_idx = ps_codec->s_parse.i4_cur_slice_idx;
            parse_slice_idx++;

            {
                /* If the next slice header is not initialized, update cur_slice_idx and break */
                if((1 == ps_codec->i4_num_cores) || (0 != (parse_slice_idx & (MAX_SLICE_HDR_CNT - 1))))
                {
                    ps_codec->s_parse.i4_cur_slice_idx = parse_slice_idx;
                    break;
                }

                /* If the next slice header is initialised, wait for the parsed slices to be processed */
                else
                {
                    WORD32 ctb_indx = 0;

                    while(ctb_indx != ps_sps->i4_pic_size_in_ctb)
                    {
                        WORD32 parse_status = *(ps_codec->pu1_parse_map + ctb_indx);
                        volatile WORD32 proc_status = *(ps_codec->pu1_proc_map + ctb_indx) & 1;

                        if(parse_status == proc_status)
                            ctb_indx++;
                    }
                    ps_codec->s_parse.i4_cur_slice_idx = parse_slice_idx;
                    break;
                }

            }
        }

    }
    else
    {
#if FRAME_ILF_PAD
        if(FRAME_ILF_PAD && 1 == ps_codec->i4_num_cores)
        {
            if(ps_slice_hdr->i4_abs_pic_order_cnt == 0)
            {
                DUMP_PRE_ILF(ps_codec->as_process[0].pu1_cur_pic_luma,
                             ps_codec->as_process[0].pu1_cur_pic_chroma,
                             ps_sps->i2_pic_width_in_luma_samples,
                             ps_sps->i2_pic_height_in_luma_samples,
                             ps_codec->i4_strd);

                DUMP_BS(ps_codec->as_process[0].s_bs_ctxt.pu4_pic_vert_bs,
                        ps_codec->as_process[0].s_bs_ctxt.pu4_pic_horz_bs,
                        ps_sps->i2_pic_wd_in_ctb * (ctb_size * ctb_size / 8 / 16) * ps_sps->i2_pic_ht_in_ctb,
                        (ps_sps->i2_pic_wd_in_ctb + 1) * (ctb_size * ctb_size / 8 / 16) * ps_sps->i2_pic_ht_in_ctb);

                DUMP_QP(ps_codec->as_process[0].s_bs_ctxt.pu1_pic_qp,
                        (ps_sps->i2_pic_height_in_luma_samples * ps_sps->i2_pic_width_in_luma_samples) / (MIN_CU_SIZE * MIN_CU_SIZE));

                DUMP_QP_CONST_IN_CTB(ps_codec->as_process[0].s_bs_ctxt.pu1_pic_qp_const_in_ctb,
                                     (ps_sps->i2_pic_height_in_luma_samples * ps_sps->i2_pic_width_in_luma_samples) / (MIN_CTB_SIZE * MIN_CTB_SIZE) / 8);

                DUMP_NO_LOOP_FILTER(ps_codec->as_process[0].pu1_pic_no_loop_filter_flag,
                                    (ps_sps->i2_pic_width_in_luma_samples / MIN_CU_SIZE) * (ps_sps->i2_pic_height_in_luma_samples / MIN_CU_SIZE) / 8);

                DUMP_OFFSETS(ps_slice_hdr->i1_beta_offset_div2,
                             ps_slice_hdr->i1_tc_offset_div2,
                             ps_pps->i1_pic_cb_qp_offset,
                             ps_pps->i1_pic_cr_qp_offset);
            }
            ps_codec->s_parse.s_deblk_ctxt.ps_pps = ps_codec->s_parse.ps_pps;
            ps_codec->s_parse.s_deblk_ctxt.ps_sps = ps_codec->s_parse.ps_sps;
            ps_codec->s_parse.s_deblk_ctxt.ps_codec = ps_codec;
            ps_codec->s_parse.s_deblk_ctxt.ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr;
            ps_codec->s_parse.s_deblk_ctxt.is_chroma_yuv420sp_vu = (ps_codec->e_ref_chroma_fmt == IV_YUV_420SP_VU);

            ps_codec->s_parse.s_sao_ctxt.ps_pps = ps_codec->s_parse.ps_pps;
            ps_codec->s_parse.s_sao_ctxt.ps_sps = ps_codec->s_parse.ps_sps;
            ps_codec->s_parse.s_sao_ctxt.ps_codec = ps_codec;
            ps_codec->s_parse.s_sao_ctxt.ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr;

            ihevcd_ilf_pad_frame(&ps_codec->s_parse.s_deblk_ctxt, &ps_codec->s_parse.s_sao_ctxt);

        }
#endif
        ps_codec->s_parse.i4_end_of_frame = 1;
    }
    return ret;
}








