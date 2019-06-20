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
*  ihevcd_parse_headers.c
*
* @brief
*  Contains functions for parsing headers
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
#include "ihevc_defs.h"
#include "ihevc_structs.h"
#include "ihevc_buf_mgr.h"
#include "ihevc_dpb_mgr.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_cabac_tables.h"
#include "ihevc_common_tables.h"
#include "ihevc_quant_tables.h"

#include "ihevcd_trace.h"
#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_debug.h"
#include "ihevcd_nal.h"
#include "ihevcd_bitstream.h"
#include "ihevcd_parse_headers.h"
#include "ihevcd_ref_list.h"

#define COPY_DEFAULT_SCALING_LIST(pi2_scaling_mat)                                                                                      \
{                                                                                                                                       \
    WORD32 scaling_mat_offset[]={0, 16, 32, 48, 64, 80, 96, 160, 224, 288, 352, 416, 480, 736, 992, 1248, 1504, 1760, 2016, 3040};      \
                                                                                                                                        \
    /* scaling matrix for 4x4 */                                                                                                        \
    memcpy(pi2_scaling_mat, gi2_flat_scale_mat_32x32, 6*16*sizeof(WORD16));                                                             \
/* scaling matrix for 8x8 */                                                                                                            \
    memcpy(pi2_scaling_mat + scaling_mat_offset[6], gi2_intra_default_scale_mat_8x8, 64*sizeof(WORD16));                                \
    memcpy(pi2_scaling_mat + scaling_mat_offset[7], gi2_intra_default_scale_mat_8x8, 64*sizeof(WORD16));                                \
    memcpy(pi2_scaling_mat + scaling_mat_offset[8], gi2_intra_default_scale_mat_8x8, 64*sizeof(WORD16));                                \
    memcpy(pi2_scaling_mat + scaling_mat_offset[9], gi2_inter_default_scale_mat_8x8, 64*sizeof(WORD16));                                \
    memcpy(pi2_scaling_mat + scaling_mat_offset[10], gi2_inter_default_scale_mat_8x8, 64*sizeof(WORD16));                               \
    memcpy(pi2_scaling_mat + scaling_mat_offset[11], gi2_inter_default_scale_mat_8x8, 64*sizeof(WORD16));                               \
    /* scaling matrix for 16x16 */                                                                                                      \
    memcpy(pi2_scaling_mat + scaling_mat_offset[12], gi2_intra_default_scale_mat_16x16, 256*sizeof(WORD16));                            \
    memcpy(pi2_scaling_mat + scaling_mat_offset[13], gi2_intra_default_scale_mat_16x16, 256*sizeof(WORD16));                            \
    memcpy(pi2_scaling_mat + scaling_mat_offset[14], gi2_intra_default_scale_mat_16x16, 256*sizeof(WORD16));                            \
    memcpy(pi2_scaling_mat + scaling_mat_offset[15], gi2_inter_default_scale_mat_16x16, 256*sizeof(WORD16));                            \
    memcpy(pi2_scaling_mat + scaling_mat_offset[16], gi2_inter_default_scale_mat_16x16, 256*sizeof(WORD16));                            \
    memcpy(pi2_scaling_mat + scaling_mat_offset[17], gi2_inter_default_scale_mat_16x16, 256*sizeof(WORD16));                            \
    /* scaling matrix for 32x32 */                                                                                                      \
    memcpy(pi2_scaling_mat + scaling_mat_offset[18], gi2_intra_default_scale_mat_32x32, 1024*sizeof(WORD16));                           \
    memcpy(pi2_scaling_mat + scaling_mat_offset[19], gi2_inter_default_scale_mat_32x32, 1024*sizeof(WORD16));                           \
}

#define COPY_FLAT_SCALING_LIST(pi2_scaling_mat)                                                                                         \
{                                                                                                                                       \
    WORD32 scaling_mat_offset[]={0, 16, 32, 48, 64, 80, 96, 160, 224, 288, 352, 416, 480, 736, 992, 1248, 1504, 1760, 2016, 3040};      \
                                                                                                                                        \
    /* scaling matrix for 4x4 */                                                                                                        \
    memcpy(pi2_scaling_mat, gi2_flat_scale_mat_32x32, 6*16*sizeof(WORD16));                                                             \
    /* scaling matrix for 8x8 */                                                                                                        \
    memcpy(pi2_scaling_mat + scaling_mat_offset[6], gi2_flat_scale_mat_32x32, 6*64*sizeof(WORD16));                                     \
    /* scaling matrix for 16x16 */                                                                                                      \
    memcpy(pi2_scaling_mat + scaling_mat_offset[12], gi2_flat_scale_mat_32x32, 3*256*sizeof(WORD16));                                   \
    memcpy(pi2_scaling_mat + scaling_mat_offset[15], gi2_flat_scale_mat_32x32, 3*256*sizeof(WORD16));                                   \
    /* scaling matrix for 32x32 */                                                                                                      \
    memcpy(pi2_scaling_mat + scaling_mat_offset[18], gi2_flat_scale_mat_32x32, 1024*sizeof(WORD16));                                    \
    memcpy(pi2_scaling_mat + scaling_mat_offset[19], gi2_flat_scale_mat_32x32, 1024*sizeof(WORD16));                                    \
}

/* Function declarations */

/**
*******************************************************************************
*
* @brief
*  Parses Prediction weight table syntax
*
* @par Description:
*  Parse Prediction weight table syntax as per Section: 7.3.8.4
*
* @param[in] ps_bitstrm
*  Pointer to bitstream context
*
* @param[in] ps_sps
*  Current SPS
*
* @param[in] ps_pps
*  Current PPS
*
* @param[in] ps_slice_hdr
*  Current Slice header
*
* @returns  Error code from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/

WORD32 ihevcd_parse_pred_wt_ofst(bitstrm_t *ps_bitstrm,
                                 sps_t *ps_sps,
                                 pps_t *ps_pps,
                                 slice_header_t *ps_slice_hdr)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 value;
    WORD32 i;

    pred_wt_ofst_t *ps_wt_ofst = &ps_slice_hdr->s_wt_ofst;
    UNUSED(ps_pps);

    UEV_PARSE("luma_log2_weight_denom", value, ps_bitstrm);
    ps_wt_ofst->i1_luma_log2_weight_denom = value;

    if(ps_sps->i1_chroma_format_idc != 0)
    {
        SEV_PARSE("delta_chroma_log2_weight_denom", value, ps_bitstrm);
        ps_wt_ofst->i1_chroma_log2_weight_denom = ps_wt_ofst->i1_luma_log2_weight_denom + value;
    }

    for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l0_active; i++)
    {
        BITS_PARSE("luma_weight_l0_flag[ i ]", value, ps_bitstrm, 1);
        ps_wt_ofst->i1_luma_weight_l0_flag[i] = value;
    }



    if(ps_sps->i1_chroma_format_idc != 0)
    {
        for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l0_active; i++)
        {
            BITS_PARSE("chroma_weight_l0_flag[ i ]", value, ps_bitstrm, 1);
            ps_wt_ofst->i1_chroma_weight_l0_flag[i] = value;
        }
    }
    else
    {
        for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l0_active; i++)
        {
            ps_wt_ofst->i1_chroma_weight_l0_flag[i] = 0;
        }
    }


    for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l0_active; i++)
    {
        if(ps_wt_ofst->i1_luma_weight_l0_flag[i])
        {
            SEV_PARSE("delta_luma_weight_l0[ i ]", value, ps_bitstrm);


            ps_wt_ofst->i2_luma_weight_l0[i] = (1 << ps_wt_ofst->i1_luma_log2_weight_denom) + value;

            SEV_PARSE("luma_offset_l0[ i ]", value, ps_bitstrm);
            ps_wt_ofst->i2_luma_offset_l0[i] = value;

        }
        else
        {
            ps_wt_ofst->i2_luma_weight_l0[i] = (1 << ps_wt_ofst->i1_luma_log2_weight_denom);
            ps_wt_ofst->i2_luma_offset_l0[i] = 0;
        }
        if(ps_wt_ofst->i1_chroma_weight_l0_flag[i])
        {
            WORD32 ofst;
            WORD32 shift = (1 << (BIT_DEPTH_CHROMA - 1));
            SEV_PARSE("delta_chroma_weight_l0[ i ][ j ]", value, ps_bitstrm);
            ps_wt_ofst->i2_chroma_weight_l0_cb[i] = (1 << ps_wt_ofst->i1_chroma_log2_weight_denom) + value;


            SEV_PARSE("delta_chroma_offset_l0[ i ][ j ]", value, ps_bitstrm);
            ofst = ((shift * ps_wt_ofst->i2_chroma_weight_l0_cb[i]) >> ps_wt_ofst->i1_chroma_log2_weight_denom);
            ofst = value - ofst + shift;

            ps_wt_ofst->i2_chroma_offset_l0_cb[i] = CLIP_S8(ofst);

            SEV_PARSE("delta_chroma_weight_l0[ i ][ j ]", value, ps_bitstrm);
            ps_wt_ofst->i2_chroma_weight_l0_cr[i] = (1 << ps_wt_ofst->i1_chroma_log2_weight_denom) + value;


            SEV_PARSE("delta_chroma_offset_l0[ i ][ j ]", value, ps_bitstrm);
            ofst = ((shift * ps_wt_ofst->i2_chroma_weight_l0_cr[i]) >> ps_wt_ofst->i1_chroma_log2_weight_denom);
            ofst = value - ofst + shift;

            ps_wt_ofst->i2_chroma_offset_l0_cr[i] = CLIP_S8(ofst);

        }
        else
        {
            ps_wt_ofst->i2_chroma_weight_l0_cb[i] = (1 << ps_wt_ofst->i1_chroma_log2_weight_denom);
            ps_wt_ofst->i2_chroma_weight_l0_cr[i] = (1 << ps_wt_ofst->i1_chroma_log2_weight_denom);

            ps_wt_ofst->i2_chroma_offset_l0_cb[i] = 0;
            ps_wt_ofst->i2_chroma_offset_l0_cr[i] = 0;
        }
    }
    if(BSLICE == ps_slice_hdr->i1_slice_type)
    {
        for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l1_active; i++)
        {
            BITS_PARSE("luma_weight_l1_flag[ i ]", value, ps_bitstrm, 1);
            ps_wt_ofst->i1_luma_weight_l1_flag[i] = value;
        }

        if(ps_sps->i1_chroma_format_idc != 0)
        {
            for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l1_active; i++)
            {
                BITS_PARSE("chroma_weight_l1_flag[ i ]", value, ps_bitstrm, 1);
                ps_wt_ofst->i1_chroma_weight_l1_flag[i] = value;
            }
        }
        else
        {
            for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l1_active; i++)
            {
                ps_wt_ofst->i1_chroma_weight_l1_flag[i] = 0;
            }
        }

        for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l1_active; i++)
        {
            if(ps_wt_ofst->i1_luma_weight_l1_flag[i])
            {
                SEV_PARSE("delta_luma_weight_l1[ i ]", value, ps_bitstrm);


                ps_wt_ofst->i2_luma_weight_l1[i] = (1 << ps_wt_ofst->i1_luma_log2_weight_denom) + value;

                SEV_PARSE("luma_offset_l1[ i ]", value, ps_bitstrm);
                ps_wt_ofst->i2_luma_offset_l1[i] = value;

            }
            else
            {
                ps_wt_ofst->i2_luma_weight_l1[i] = (1 << ps_wt_ofst->i1_luma_log2_weight_denom);
                ps_wt_ofst->i2_luma_offset_l1[i] = 0;
            }

            if(ps_wt_ofst->i1_chroma_weight_l1_flag[i])
            {
                WORD32 ofst;
                WORD32 shift = (1 << (BIT_DEPTH_CHROMA - 1));
                SEV_PARSE("delta_chroma_weight_l1[ i ][ j ]", value, ps_bitstrm);
                ps_wt_ofst->i2_chroma_weight_l1_cb[i] = (1 << ps_wt_ofst->i1_chroma_log2_weight_denom) + value;;


                SEV_PARSE("delta_chroma_offset_l1[ i ][ j ]", value, ps_bitstrm);
                ofst = ((shift * ps_wt_ofst->i2_chroma_weight_l1_cb[i]) >> ps_wt_ofst->i1_chroma_log2_weight_denom);
                ofst = value - ofst + shift;

                ps_wt_ofst->i2_chroma_offset_l1_cb[i] = CLIP_S8(ofst);;

                SEV_PARSE("delta_chroma_weight_l1[ i ][ j ]", value, ps_bitstrm);
                ps_wt_ofst->i2_chroma_weight_l1_cr[i] = (1 << ps_wt_ofst->i1_chroma_log2_weight_denom) + value;


                SEV_PARSE("delta_chroma_offset_l1[ i ][ j ]", value, ps_bitstrm);
                ofst = ((shift * ps_wt_ofst->i2_chroma_weight_l1_cr[i]) >> ps_wt_ofst->i1_chroma_log2_weight_denom);
                ofst = value - ofst + shift;

                ps_wt_ofst->i2_chroma_offset_l1_cr[i] = CLIP_S8(ofst);;

            }
            else
            {
                ps_wt_ofst->i2_chroma_weight_l1_cb[i] = (1 << ps_wt_ofst->i1_chroma_log2_weight_denom);
                ps_wt_ofst->i2_chroma_weight_l1_cr[i] = (1 << ps_wt_ofst->i1_chroma_log2_weight_denom);

                ps_wt_ofst->i2_chroma_offset_l1_cb[i] = 0;
                ps_wt_ofst->i2_chroma_offset_l1_cr[i] = 0;

            }
        }
    }
    return ret;
}

/**
*******************************************************************************
*
* @brief
*  Parses short term reference picture set
*
* @par   Description
*  Parses short term reference picture set as per section 7.3.8.2.
* Can be called by either SPS or Slice header parsing modules.
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @param[out] ps_stref_picset_base
*  Pointer to first short term ref pic set structure
*
* @param[in] num_short_term_ref_pic_sets
*  Number of short term reference pic sets
*
* @param[in] idx
*  Current short term ref pic set id
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_short_term_ref_pic_set(bitstrm_t *ps_bitstrm,
                                             stref_picset_t *ps_stref_picset_base,
                                             WORD32 num_short_term_ref_pic_sets,
                                             WORD32 idx,
                                             stref_picset_t *ps_stref_picset)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 value;
    stref_picset_t *ps_stref_picset_ref;
    WORD32 delta_idx, delta_rps;
    WORD32 r_idx;
    WORD32 i;
    WORD32 j, k, temp;
    if(idx > 0)
    {
        BITS_PARSE("inter_ref_pic_set_prediction_flag", value, ps_bitstrm, 1);
        ps_stref_picset->i1_inter_ref_pic_set_prediction_flag = value;
    }
    else
        ps_stref_picset->i1_inter_ref_pic_set_prediction_flag = 0;

    if(ps_stref_picset->i1_inter_ref_pic_set_prediction_flag)
    {
        WORD32 delta_rps_sign;
        WORD32 abs_delta_rps;
        WORD32 num_neg_pics = 0;
        WORD32 num_pos_pics = 0;
        WORD32 num_pics = 0;

        if(idx == num_short_term_ref_pic_sets)
        {
            UEV_PARSE("delta_idx_minus1", value, ps_bitstrm);
            delta_idx = value + 1;
        }
        else
        {
            delta_idx = 1;
        }
        r_idx = idx - delta_idx;
        r_idx = CLIP3(r_idx, 0, idx - 1);

        ps_stref_picset_ref = ps_stref_picset_base + r_idx;

        BITS_PARSE("delta_rps_sign", value, ps_bitstrm, 1);
        delta_rps_sign = value;

        UEV_PARSE("abs_delta_rps_minus1", value, ps_bitstrm);
        abs_delta_rps = value + 1;

        delta_rps = (1 - 2 * delta_rps_sign) * (abs_delta_rps);



        for(i = 0; i <= ps_stref_picset_ref->i1_num_delta_pocs; i++)
        {
            WORD32 ref_idc;

            /*****************************************************************/
            /* ref_idc is parsed as below                                    */
            /* bits "1" ref_idc 1                                            */
            /* bits "01" ref_idc 2                                           */
            /* bits "00" ref_idc 0                                           */
            /*****************************************************************/
            BITS_PARSE("used_by_curr_pic_flag", value, ps_bitstrm, 1);
            ref_idc = value;
            ps_stref_picset->ai1_used[num_pics] = value;
            /* If ref_idc is zero check for next bit */
            if(0 == ref_idc)
            {
                BITS_PARSE("use_delta_flag", value, ps_bitstrm, 1);
                ref_idc = value << 1;
            }
            if((ref_idc == 1) || (ref_idc == 2))
            {
                WORD32 delta_poc;
                delta_poc = delta_rps;
                delta_poc +=
                                ((i < ps_stref_picset_ref->i1_num_delta_pocs) ?
                                ps_stref_picset_ref->ai2_delta_poc[i] :
                                0);

                ps_stref_picset->ai2_delta_poc[num_pics] = delta_poc;

                if(delta_poc < 0)
                {
                    num_neg_pics++;
                }
                else
                {
                    num_pos_pics++;
                }
                num_pics++;
            }
            ps_stref_picset->ai1_ref_idc[i] = ref_idc;
        }

        num_neg_pics = CLIP3(num_neg_pics, 0, MAX_DPB_SIZE - 1);
        num_pos_pics = CLIP3(num_pos_pics, 0, (MAX_DPB_SIZE - 1 - num_neg_pics));
        num_pics = num_neg_pics + num_pos_pics;

        ps_stref_picset->i1_num_ref_idc =
                        ps_stref_picset_ref->i1_num_delta_pocs + 1;
        ps_stref_picset->i1_num_delta_pocs = num_pics;
        ps_stref_picset->i1_num_pos_pics = num_pos_pics;
        ps_stref_picset->i1_num_neg_pics = num_neg_pics;


        for(j = 1; j < num_pics; j++)
        {
            WORD32 delta_poc = ps_stref_picset->ai2_delta_poc[j];
            WORD8 i1_used = ps_stref_picset->ai1_used[j];
            for(k = j - 1; k >= 0; k--)
            {
                temp = ps_stref_picset->ai2_delta_poc[k];
                if(delta_poc < temp)
                {
                    ps_stref_picset->ai2_delta_poc[k + 1] = temp;
                    ps_stref_picset->ai1_used[k + 1] = ps_stref_picset->ai1_used[k];
                    ps_stref_picset->ai2_delta_poc[k] = delta_poc;
                    ps_stref_picset->ai1_used[k] = i1_used;
                }
            }
        }
        // flip the negative values to largest first
        for(j = 0, k = num_neg_pics - 1; j < num_neg_pics >> 1; j++, k--)
        {
            WORD32 delta_poc = ps_stref_picset->ai2_delta_poc[j];
            WORD8 i1_used = ps_stref_picset->ai1_used[j];
            ps_stref_picset->ai2_delta_poc[j] = ps_stref_picset->ai2_delta_poc[k];
            ps_stref_picset->ai1_used[j] = ps_stref_picset->ai1_used[k];
            ps_stref_picset->ai2_delta_poc[k] = delta_poc;
            ps_stref_picset->ai1_used[k] = i1_used;
        }

    }
    else
    {
        WORD32 prev_poc = 0;
        WORD32 poc;

        UEV_PARSE("num_negative_pics", value, ps_bitstrm);
        ps_stref_picset->i1_num_neg_pics = value;
        ps_stref_picset->i1_num_neg_pics = CLIP3(ps_stref_picset->i1_num_neg_pics,
                                                 0,
                                                 MAX_DPB_SIZE - 1);

        UEV_PARSE("num_positive_pics", value, ps_bitstrm);
        ps_stref_picset->i1_num_pos_pics = value;
        ps_stref_picset->i1_num_pos_pics = CLIP3(ps_stref_picset->i1_num_pos_pics,
                                                 0,
                                                 (MAX_DPB_SIZE - 1 - ps_stref_picset->i1_num_neg_pics));

        ps_stref_picset->i1_num_delta_pocs =
                        ps_stref_picset->i1_num_neg_pics +
                        ps_stref_picset->i1_num_pos_pics;


        for(i = 0; i < ps_stref_picset->i1_num_neg_pics; i++)
        {
            UEV_PARSE("delta_poc_s0_minus1", value, ps_bitstrm);
            poc = prev_poc - (value + 1);
            prev_poc = poc;
            ps_stref_picset->ai2_delta_poc[i] = poc;

            BITS_PARSE("used_by_curr_pic_s0_flag", value, ps_bitstrm, 1);
            ps_stref_picset->ai1_used[i] = value;

        }
        prev_poc = 0;
        for(i = ps_stref_picset->i1_num_neg_pics;
                        i < ps_stref_picset->i1_num_delta_pocs;
                        i++)
        {
            UEV_PARSE("delta_poc_s1_minus1", value, ps_bitstrm);
            poc = prev_poc + (value + 1);
            prev_poc = poc;
            ps_stref_picset->ai2_delta_poc[i] = poc;

            BITS_PARSE("used_by_curr_pic_s1_flag", value, ps_bitstrm, 1);
            ps_stref_picset->ai1_used[i] = value;

        }

    }

    return ret;
}


static WORD32 ihevcd_parse_sub_layer_hrd_parameters(bitstrm_t *ps_bitstrm,
                                                    sub_lyr_hrd_params_t *ps_sub_layer_hrd_params,
                                                    WORD32 cpb_cnt,
                                                    WORD32 sub_pic_cpb_params_present_flag)
{
    WORD32 ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 i;

    for(i = 0; i <= cpb_cnt; i++)
    {
        UEV_PARSE("bit_rate_value_minus1[ i ]", ps_sub_layer_hrd_params->au4_bit_rate_value_minus1[i], ps_bitstrm);
        UEV_PARSE("cpb_size_value_minus1[ i ]", ps_sub_layer_hrd_params->au4_cpb_size_value_minus1[i], ps_bitstrm);

        if(sub_pic_cpb_params_present_flag)
        {
            UEV_PARSE("cpb_size_du_value_minus1[ i ]", ps_sub_layer_hrd_params->au4_cpb_size_du_value_minus1[i], ps_bitstrm);
            UEV_PARSE("bit_rate_du_value_minus1[ i ]", ps_sub_layer_hrd_params->au4_bit_rate_du_value_minus1[i], ps_bitstrm);
        }
        BITS_PARSE("cbr_flag[ i ]", ps_sub_layer_hrd_params->au1_cbr_flag[i], ps_bitstrm, 1);
    }

    return ret;
}


static WORD32 ihevcd_parse_hrd_parameters(bitstrm_t *ps_bitstrm,
                                          hrd_params_t *ps_hrd,
                                          WORD32 common_info_present_flag,
                                          WORD32 max_num_sub_layers_minus1)
{
    WORD32 ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 i;

    ps_hrd->u1_nal_hrd_parameters_present_flag = 0;
    ps_hrd->u1_vcl_hrd_parameters_present_flag = 0;

    ps_hrd->u1_sub_pic_cpb_params_present_flag = 0;

    ps_hrd->u1_tick_divisor_minus2 = 0;
    ps_hrd->u1_du_cpb_removal_delay_increment_length_minus1 = 0;
    ps_hrd->u1_sub_pic_cpb_params_in_pic_timing_sei_flag = 0;
    ps_hrd->u1_dpb_output_delay_du_length_minus1 = 0;

    ps_hrd->u4_bit_rate_scale = 0;
    ps_hrd->u4_cpb_size_scale = 0;
    ps_hrd->u4_cpb_size_du_scale = 0;

    ps_hrd->u1_initial_cpb_removal_delay_length_minus1 = 23;
    ps_hrd->u1_au_cpb_removal_delay_length_minus1 = 23;
    ps_hrd->u1_dpb_output_delay_length_minus1 = 23;

    if(common_info_present_flag)
    {
        BITS_PARSE("nal_hrd_parameters_present_flag", ps_hrd->u1_nal_hrd_parameters_present_flag, ps_bitstrm, 1);
        BITS_PARSE("vcl_hrd_parameters_present_flag", ps_hrd->u1_vcl_hrd_parameters_present_flag, ps_bitstrm, 1);

        if(ps_hrd->u1_nal_hrd_parameters_present_flag  ||  ps_hrd->u1_vcl_hrd_parameters_present_flag)
        {
            BITS_PARSE("sub_pic_cpb_params_present_flag", ps_hrd->u1_sub_pic_cpb_params_present_flag, ps_bitstrm, 1);
            if(ps_hrd->u1_sub_pic_cpb_params_present_flag)
            {
                BITS_PARSE("tick_divisor_minus2", ps_hrd->u1_tick_divisor_minus2, ps_bitstrm, 8);
                BITS_PARSE("du_cpb_removal_delay_increment_length_minus1", ps_hrd->u1_du_cpb_removal_delay_increment_length_minus1, ps_bitstrm, 5);
                BITS_PARSE("sub_pic_cpb_params_in_pic_timing_sei_flag", ps_hrd->u1_sub_pic_cpb_params_in_pic_timing_sei_flag, ps_bitstrm, 1);
                BITS_PARSE("dpb_output_delay_du_length_minus1", ps_hrd->u1_dpb_output_delay_du_length_minus1, ps_bitstrm, 5);
            }

            BITS_PARSE("bit_rate_scale", ps_hrd->u4_bit_rate_scale, ps_bitstrm, 4);
            BITS_PARSE("cpb_size_scale", ps_hrd->u4_cpb_size_scale, ps_bitstrm, 4);
            if(ps_hrd->u1_sub_pic_cpb_params_present_flag)
                BITS_PARSE("cpb_size_du_scale", ps_hrd->u4_cpb_size_du_scale, ps_bitstrm, 4);

            BITS_PARSE("initial_cpb_removal_delay_length_minus1", ps_hrd->u1_initial_cpb_removal_delay_length_minus1, ps_bitstrm, 5);
            BITS_PARSE("au_cpb_removal_delay_length_minus1", ps_hrd->u1_au_cpb_removal_delay_length_minus1, ps_bitstrm, 5);
            BITS_PARSE("dpb_output_delay_length_minus1", ps_hrd->u1_dpb_output_delay_length_minus1, ps_bitstrm, 5);
        }
    }


    for(i = 0; i <= max_num_sub_layers_minus1; i++)
    {
        BITS_PARSE("fixed_pic_rate_general_flag[ i ]", ps_hrd->au1_fixed_pic_rate_general_flag[i], ps_bitstrm, 1);

        ps_hrd->au1_fixed_pic_rate_within_cvs_flag[i] = 1;
        ps_hrd->au1_elemental_duration_in_tc_minus1[i] = 0;
        ps_hrd->au1_low_delay_hrd_flag[i] = 0;
        ps_hrd->au1_cpb_cnt_minus1[i] = 0;

        if(!ps_hrd->au1_fixed_pic_rate_general_flag[i])
            BITS_PARSE("fixed_pic_rate_within_cvs_flag[ i ]", ps_hrd->au1_fixed_pic_rate_within_cvs_flag[i], ps_bitstrm, 1);

        if(ps_hrd->au1_fixed_pic_rate_within_cvs_flag[i])
        {
            UEV_PARSE("elemental_duration_in_tc_minus1[ i ]", ps_hrd->au1_elemental_duration_in_tc_minus1[i], ps_bitstrm);
        }
        else
        {
            BITS_PARSE("low_delay_hrd_flag[ i ]", ps_hrd->au1_low_delay_hrd_flag[i], ps_bitstrm, 1);
        }

        if(!ps_hrd->au1_low_delay_hrd_flag[i])
            UEV_PARSE("cpb_cnt_minus1[ i ]", ps_hrd->au1_cpb_cnt_minus1[i], ps_bitstrm);

        if(ps_hrd->au1_cpb_cnt_minus1[i] >= (MAX_CPB_CNT - 1))
            return IHEVCD_INVALID_PARAMETER;

        if(ps_hrd->u1_nal_hrd_parameters_present_flag)
            ihevcd_parse_sub_layer_hrd_parameters(ps_bitstrm,
                                                  &ps_hrd->as_sub_layer_hrd_params[i],
                                                  ps_hrd->au1_cpb_cnt_minus1[i],
                                                  ps_hrd->u1_sub_pic_cpb_params_present_flag);

        if(ps_hrd->u1_vcl_hrd_parameters_present_flag)
            ihevcd_parse_sub_layer_hrd_parameters(ps_bitstrm,
                                                  &ps_hrd->as_sub_layer_hrd_params[i],
                                                  ps_hrd->au1_cpb_cnt_minus1[i],
                                                  ps_hrd->u1_sub_pic_cpb_params_present_flag);
    }

    return ret;
}


static WORD32 ihevcd_parse_vui_parameters(bitstrm_t *ps_bitstrm,
                                          vui_t *ps_vui,
                                          WORD32 sps_max_sub_layers_minus1)
{
    WORD32 ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    UWORD16 u2_sar_width = 0;
    UWORD16 u2_sar_height = 0;

    BITS_PARSE("aspect_ratio_info_present_flag", ps_vui->u1_aspect_ratio_info_present_flag, ps_bitstrm, 1);

    ps_vui->u1_aspect_ratio_idc = SAR_UNUSED;
    u2_sar_width = 0;
    u2_sar_height = 0;
    if(ps_vui->u1_aspect_ratio_info_present_flag)
    {
        BITS_PARSE("aspect_ratio_idc", ps_vui->u1_aspect_ratio_idc, ps_bitstrm, 8);
        switch(ps_vui->u1_aspect_ratio_idc)
        {
            case SAR_1_1:
                u2_sar_width = 1;
                u2_sar_height = 1;
                break;
            case SAR_12_11:
                u2_sar_width = 12;
                u2_sar_height = 11;
                break;
            case SAR_10_11:
                u2_sar_width = 10;
                u2_sar_height = 11;
                break;
            case SAR_16_11:
                u2_sar_width = 16;
                u2_sar_height = 11;
                break;
            case SAR_40_33:
                u2_sar_width = 40;
                u2_sar_height = 33;
                break;
            case SAR_24_11:
                u2_sar_width = 24;
                u2_sar_height = 11;
                break;
            case SAR_20_11:
                u2_sar_width = 20;
                u2_sar_height = 11;
                break;
            case SAR_32_11:
                u2_sar_width = 32;
                u2_sar_height = 11;
                break;
            case SAR_80_33:
                u2_sar_width = 80;
                u2_sar_height = 33;
                break;
            case SAR_18_11:
                u2_sar_width = 18;
                u2_sar_height = 11;
                break;
            case SAR_15_11:
                u2_sar_width = 15;
                u2_sar_height = 11;
                break;
            case SAR_64_33:
                u2_sar_width = 64;
                u2_sar_height = 33;
                break;
            case SAR_160_99:
                u2_sar_width = 160;
                u2_sar_height = 99;
                break;
            case SAR_4_3:
                u2_sar_width = 4;
                u2_sar_height = 3;
                break;
            case SAR_3_2:
                u2_sar_width = 3;
                u2_sar_height = 2;
                break;
            case SAR_2_1:
                u2_sar_width = 2;
                u2_sar_height = 1;
                break;
            case EXTENDED_SAR:
                BITS_PARSE("sar_width", u2_sar_width, ps_bitstrm, 16);
                BITS_PARSE("sar_height", u2_sar_height, ps_bitstrm, 16);
                break;
            default:
                u2_sar_width = 0;
                u2_sar_height = 0;
                break;
        }
    }

    ps_vui->u2_sar_width    = u2_sar_width;
    ps_vui->u2_sar_height   = u2_sar_height;

    BITS_PARSE("overscan_info_present_flag", ps_vui->u1_overscan_info_present_flag, ps_bitstrm, 1);
    ps_vui->u1_overscan_appropriate_flag = 0;
    if(ps_vui->u1_overscan_info_present_flag)
        BITS_PARSE("overscan_appropriate_flag", ps_vui->u1_overscan_appropriate_flag, ps_bitstrm, 1);

    BITS_PARSE("video_signal_type_present_flag", ps_vui->u1_video_signal_type_present_flag, ps_bitstrm, 1);
    ps_vui->u1_video_format = VID_FMT_UNSPECIFIED;
    ps_vui->u1_video_full_range_flag = 0;
    ps_vui->u1_colour_description_present_flag = 0;
    ps_vui->u1_colour_primaries = 2;
    ps_vui->u1_transfer_characteristics = 2;
    ps_vui->u1_matrix_coefficients = 2;

    if(ps_vui->u1_video_signal_type_present_flag)
    {
        BITS_PARSE("video_format", ps_vui->u1_video_format, ps_bitstrm, 3);
        BITS_PARSE("video_full_range_flag", ps_vui->u1_video_full_range_flag, ps_bitstrm, 1);
        BITS_PARSE("colour_description_present_flag", ps_vui->u1_colour_description_present_flag, ps_bitstrm, 1);
        if(ps_vui->u1_colour_description_present_flag)
        {
            BITS_PARSE("colour_primaries", ps_vui->u1_colour_primaries, ps_bitstrm, 8);
            BITS_PARSE("transfer_characteristics", ps_vui->u1_transfer_characteristics, ps_bitstrm, 8);
            BITS_PARSE("matrix_coeffs", ps_vui->u1_matrix_coefficients, ps_bitstrm, 8);
        }
    }

    BITS_PARSE("chroma_loc_info_present_flag", ps_vui->u1_chroma_loc_info_present_flag, ps_bitstrm, 1);
    ps_vui->u1_chroma_sample_loc_type_top_field = 0;
    ps_vui->u1_chroma_sample_loc_type_bottom_field = 0;
    if(ps_vui->u1_chroma_loc_info_present_flag)
    {
        UEV_PARSE("chroma_sample_loc_type_top_field", ps_vui->u1_chroma_sample_loc_type_top_field, ps_bitstrm);
        UEV_PARSE("chroma_sample_loc_type_bottom_field", ps_vui->u1_chroma_sample_loc_type_bottom_field, ps_bitstrm);
    }

    BITS_PARSE("neutral_chroma_indication_flag", ps_vui->u1_neutral_chroma_indication_flag, ps_bitstrm, 1);
    BITS_PARSE("field_seq_flag", ps_vui->u1_field_seq_flag, ps_bitstrm, 1);
    BITS_PARSE("frame_field_info_present_flag", ps_vui->u1_frame_field_info_present_flag, ps_bitstrm, 1);
    BITS_PARSE("default_display_window_flag", ps_vui->u1_default_display_window_flag, ps_bitstrm, 1);
    ps_vui->u4_def_disp_win_left_offset = 0;
    ps_vui->u4_def_disp_win_right_offset = 0;
    ps_vui->u4_def_disp_win_top_offset = 0;
    ps_vui->u4_def_disp_win_bottom_offset = 0;
    if(ps_vui->u1_default_display_window_flag)
    {
        UEV_PARSE("def_disp_win_left_offset", ps_vui->u4_def_disp_win_left_offset, ps_bitstrm);
        UEV_PARSE("def_disp_win_right_offset", ps_vui->u4_def_disp_win_right_offset, ps_bitstrm);
        UEV_PARSE("def_disp_win_top_offset", ps_vui->u4_def_disp_win_top_offset, ps_bitstrm);
        UEV_PARSE("def_disp_win_bottom_offset", ps_vui->u4_def_disp_win_bottom_offset, ps_bitstrm);
    }

    BITS_PARSE("vui_timing_info_present_flag", ps_vui->u1_vui_timing_info_present_flag, ps_bitstrm, 1);
    if(ps_vui->u1_vui_timing_info_present_flag)
    {
        BITS_PARSE("vui_num_units_in_tick", ps_vui->u4_vui_num_units_in_tick, ps_bitstrm, 32);
        BITS_PARSE("vui_time_scale", ps_vui->u4_vui_time_scale, ps_bitstrm, 32);
        BITS_PARSE("vui_poc_proportional_to_timing_flag", ps_vui->u1_poc_proportional_to_timing_flag, ps_bitstrm, 1);
        if(ps_vui->u1_poc_proportional_to_timing_flag)
            UEV_PARSE("vui_num_ticks_poc_diff_one_minus1", ps_vui->u1_num_ticks_poc_diff_one_minus1, ps_bitstrm);

        BITS_PARSE("vui_hrd_parameters_present_flag", ps_vui->u1_vui_hrd_parameters_present_flag, ps_bitstrm, 1);
        if(ps_vui->u1_vui_hrd_parameters_present_flag)
        {
            ret = ihevcd_parse_hrd_parameters(ps_bitstrm, &ps_vui->s_vui_hrd_parameters, 1, sps_max_sub_layers_minus1);
            RETURN_IF((ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS), ret);
        }
    }

    BITS_PARSE("bitstream_restriction_flag", ps_vui->u1_bitstream_restriction_flag, ps_bitstrm, 1);
    ps_vui->u1_tiles_fixed_structure_flag = 0;
    ps_vui->u1_motion_vectors_over_pic_boundaries_flag = 1;
    ps_vui->u1_restricted_ref_pic_lists_flag = 0;
    ps_vui->u4_min_spatial_segmentation_idc = 0;
    ps_vui->u1_max_bytes_per_pic_denom = 2;
    ps_vui->u1_max_bits_per_mincu_denom = 1;
    ps_vui->u1_log2_max_mv_length_horizontal = 15;
    ps_vui->u1_log2_max_mv_length_vertical = 15;
    if(ps_vui->u1_bitstream_restriction_flag)
    {
        BITS_PARSE("tiles_fixed_structure_flag", ps_vui->u1_tiles_fixed_structure_flag, ps_bitstrm, 1);
        BITS_PARSE("motion_vectors_over_pic_boundaries_flag", ps_vui->u1_motion_vectors_over_pic_boundaries_flag, ps_bitstrm, 1);
        BITS_PARSE("restricted_ref_pic_lists_flag", ps_vui->u1_restricted_ref_pic_lists_flag, ps_bitstrm, 1);

        UEV_PARSE("min_spatial_segmentation_idc", ps_vui->u4_min_spatial_segmentation_idc, ps_bitstrm);
        UEV_PARSE("max_bytes_per_pic_denom", ps_vui->u1_max_bytes_per_pic_denom, ps_bitstrm);
        UEV_PARSE("max_bits_per_min_cu_denom", ps_vui->u1_max_bits_per_mincu_denom, ps_bitstrm);
        UEV_PARSE("log2_max_mv_length_horizontal", ps_vui->u1_log2_max_mv_length_horizontal, ps_bitstrm);
        UEV_PARSE("log2_max_mv_length_vertical", ps_vui->u1_log2_max_mv_length_vertical, ps_bitstrm);
    }

    return ret;
}

/**
*******************************************************************************
*
* @brief
*  Parses profile tier and level info for either general layer of sub_layer
*
* @par   Description
*  Parses profile tier and level info for either general layer of sub_layer
* as per section 7.3.3
*
* Since the same function is called for parsing general_profile and
* sub_layer_profile etc, variables do not specify whether the syntax is
* for general or sub_layer. Similarly trace functions also do not differentiate
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @param[out] ps_ptl
*  Pointer to profile, tier level structure
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*
*******************************************************************************
*/

static IHEVCD_ERROR_T ihevcd_parse_profile_tier_level_layer(bitstrm_t *ps_bitstrm,
                                                            profile_tier_lvl_t *ps_ptl)
{
    WORD32 value;
    WORD32 i;
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;

    BITS_PARSE("XXX_profile_space[]", value, ps_bitstrm, 2);
    ps_ptl->i1_profile_space = value;

    BITS_PARSE("XXX_tier_flag[]", value, ps_bitstrm, 1);
    ps_ptl->i1_tier_flag = value;

    BITS_PARSE("XXX_profile_idc[]", value, ps_bitstrm, 5);
    ps_ptl->i1_profile_idc = value;

    for(i = 0; i < MAX_PROFILE_COMPATBLTY; i++)
    {
        BITS_PARSE("XXX_profile_compatibility_flag[][j]", value, ps_bitstrm, 1);
        ps_ptl->ai1_profile_compatibility_flag[i] = value;
    }

    BITS_PARSE("general_progressive_source_flag", value, ps_bitstrm, 1);
    ps_ptl->i1_general_progressive_source_flag = value;

    BITS_PARSE("general_interlaced_source_flag", value, ps_bitstrm, 1);
    ps_ptl->i1_general_interlaced_source_flag = value;

    BITS_PARSE("general_non_packed_constraint_flag", value, ps_bitstrm, 1);
    ps_ptl->i1_general_non_packed_constraint_flag = value;

    BITS_PARSE("general_frame_only_constraint_flag", value, ps_bitstrm, 1);
    ps_ptl->i1_frame_only_constraint_flag = value;

    BITS_PARSE("XXX_reserved_zero_44bits[0..15]", value, ps_bitstrm, 16);

    BITS_PARSE("XXX_reserved_zero_44bits[16..31]", value, ps_bitstrm, 16);

    BITS_PARSE("XXX_reserved_zero_44bits[32..43]", value, ps_bitstrm, 12);
    return ret;
}


/**
*******************************************************************************
*
* @brief
*  Parses profile tier and level info
*
* @par   Description
*  Parses profile tier and level info as per section 7.3.3
* Called during VPS and SPS parsing
* calls ihevcd_parse_profile_tier_level() for general layer and each sub_layers
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @param[out] ps_ptl
*  Pointer to structure that contains profile, tier level for each layers
*
* @param[in] profile_present
*  Flag to indicate if profile data is present
*
* @param[in] max_num_sub_layers
*  Number of sub layers present
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*
*******************************************************************************
*/

static IHEVCD_ERROR_T ihevcd_profile_tier_level(bitstrm_t *ps_bitstrm,
                                                profile_tier_lvl_info_t *ps_ptl,
                                                WORD32 profile_present,
                                                WORD32 max_num_sub_layers)
{
    WORD32 value;
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 i;

    if(profile_present)
    {
        ret = ihevcd_parse_profile_tier_level_layer(ps_bitstrm, &ps_ptl->s_ptl_gen);
        if((IHEVCD_ERROR_T)IHEVCD_SUCCESS != ret)
        {
            return ret;
        }
    }

    BITS_PARSE("general_level_idc", value, ps_bitstrm, 8);
    ps_ptl->s_ptl_gen.u1_level_idc = value;


    for(i = 0; i < max_num_sub_layers; i++)
    {
        BITS_PARSE("sub_layer_profile_present_flag[i]", value, ps_bitstrm, 1);
        ps_ptl->ai1_sub_layer_profile_present_flag[i] = value;

        BITS_PARSE("sub_layer_level_present_flag[i]", value, ps_bitstrm, 1);
        ps_ptl->ai1_sub_layer_level_present_flag[i] = value;
    }

    if(max_num_sub_layers > 0)
    {
        for(i = max_num_sub_layers; i < 8; i++)
        {
            BITS_PARSE("reserved_zero_2bits", value, ps_bitstrm, 2);
        }
    }

    for(i = 0; i < max_num_sub_layers; i++)
    {
        if(ps_ptl->ai1_sub_layer_profile_present_flag[i])
        {
            ret = ihevcd_parse_profile_tier_level_layer(ps_bitstrm,
                                                        &ps_ptl->as_ptl_sub[i]);
        }
        if(ps_ptl->ai1_sub_layer_level_present_flag[i])
        {
            BITS_PARSE("sub_layer_level_idc[i]", value, ps_bitstrm, 8);
            ps_ptl->as_ptl_sub[i].u1_level_idc = value;

        }
    }



    return ret;
}

/**
*******************************************************************************
*
* @brief
*  Parses Scaling List Data syntax
*
* @par Description:
*  Parses Scaling List Data syntax as per Section: 7.3.6
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns  Error code from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
IHEVCD_ERROR_T  ihevcd_scaling_list_data(codec_t *ps_codec, WORD16 *pi2_scaling_mat)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 size_id;
    WORD32 matrix_id;
    WORD32 value, dc_value = 0;
    WORD32 next_coef;
    WORD32 coef_num;
    WORD32 i, j, offset;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    WORD16 *pi2_scaling_mat_offset;
    WORD32 scaling_mat_offset[] = { 0, 16, 32, 48, 64, 80, 96, 160, 224, 288, 352, 416, 480, 736, 992, 1248, 1504, 1760, 2016, 3040 };
    UWORD8 *scan_table;

    for(size_id = 0; size_id < 4; size_id++)
    {
        for(matrix_id = 0; matrix_id < ((size_id == 3) ? 2 : 6); matrix_id++)
        {
            WORD32 scaling_list_pred_mode_flag;
            WORD32 scaling_list_delta_coef;
            BITS_PARSE("scaling_list_pred_mode_flag", scaling_list_pred_mode_flag, ps_bitstrm, 1);

            offset = size_id * 6 + matrix_id;
            pi2_scaling_mat_offset = pi2_scaling_mat + scaling_mat_offset[offset];

            if(!scaling_list_pred_mode_flag)
            {
                WORD32 num_elements;
                UEV_PARSE("scaling_list_pred_matrix_id_delta", value,
                          ps_bitstrm);
                value = CLIP3(value, 0, matrix_id);

                num_elements = (1 << (4 + (size_id << 1)));
                if(0 != value)
                    memcpy(pi2_scaling_mat_offset, pi2_scaling_mat_offset - value * num_elements, num_elements * sizeof(WORD16));
            }
            else
            {
                next_coef = 8;
                coef_num = MIN(64, (1 << (4 + (size_id << 1))));

                if(size_id > 1)
                {
                    SEV_PARSE("scaling_list_dc_coef_minus8", value,
                              ps_bitstrm);

                    next_coef = value + 8;
                    dc_value = next_coef;
                }
                if(size_id < 2)
                {
                    scan_table = (UWORD8 *)gapv_ihevc_invscan[size_id + 1];

                    for(i = 0; i < coef_num; i++)
                    {
                        SEV_PARSE("scaling_list_delta_coef",
                                  scaling_list_delta_coef, ps_bitstrm);
                        next_coef = (next_coef + scaling_list_delta_coef + 256)
                                        % 256;
                        pi2_scaling_mat_offset[scan_table[i]] = next_coef;
                    }
                }
                else if(size_id == 2)
                {
                    scan_table = (UWORD8 *)gapv_ihevc_invscan[2];

                    for(i = 0; i < coef_num; i++)
                    {
                        SEV_PARSE("scaling_list_delta_coef",
                                  scaling_list_delta_coef, ps_bitstrm);
                        next_coef = (next_coef + scaling_list_delta_coef + 256)
                                        % 256;

                        offset = scan_table[i];
                        offset = (offset >> 3) * 16 * 2 + (offset & 0x7) * 2;
                        pi2_scaling_mat_offset[offset] = next_coef;
                        pi2_scaling_mat_offset[offset + 1] = next_coef;
                        pi2_scaling_mat_offset[offset + 16] = next_coef;
                        pi2_scaling_mat_offset[offset + 16 + 1] = next_coef;
                    }
                    pi2_scaling_mat_offset[0] = dc_value;
                }
                else
                {
                    scan_table = (UWORD8 *)gapv_ihevc_invscan[2];

                    for(i = 0; i < coef_num; i++)
                    {
                        SEV_PARSE("scaling_list_delta_coef",
                                  scaling_list_delta_coef, ps_bitstrm);
                        next_coef = (next_coef + scaling_list_delta_coef + 256)
                                        % 256;

                        offset = scan_table[i];
                        offset = (offset >> 3) * 32 * 4 + (offset & 0x7) * 4;

                        for(j = 0; j < 4; j++)
                        {
                            pi2_scaling_mat_offset[offset + j * 32] = next_coef;
                            pi2_scaling_mat_offset[offset + 1 + j * 32] = next_coef;
                            pi2_scaling_mat_offset[offset + 2 + j * 32] = next_coef;
                            pi2_scaling_mat_offset[offset + 3 + j * 32] = next_coef;
                        }
                        pi2_scaling_mat_offset[0] = dc_value;
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
*  Parses VPS (Video Parameter Set)
*
* @par Description:
*  Parse Video Parameter Set as per Section 7.3.2.1
* update vps structure corresponding to vps ID
* Till parsing VPS id, the elements are stored in local variables and are copied
* later
*
* @param[in] ps_codec
*  Pointer to codec context.
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_parse_vps(codec_t *ps_codec)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 i;
    WORD32 value;
    WORD32 vps_id;
    vps_t *ps_vps;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    BITS_PARSE("vps_video_parameter_set_id", value, ps_bitstrm, 4);
    vps_id = value;

    if(vps_id >= MAX_VPS_CNT)
    {
        ps_codec->s_parse.i4_error_code = IHEVCD_UNSUPPORTED_VPS_ID;
        return IHEVCD_UNSUPPORTED_VPS_ID;
    }


    ps_vps = (ps_codec->s_parse.ps_vps_base + vps_id);

    ps_vps->i1_vps_id = vps_id;

    BITS_PARSE("vps_reserved_three_2bits", value, ps_bitstrm, 2);
    ASSERT(value == 3);

    BITS_PARSE("vps_max_layers_minus1", value, ps_bitstrm, 6);
    //ps_vps->i1_vps_max_layers = value + 1;



    BITS_PARSE("vps_max_sub_layers_minus1", value, ps_bitstrm, 3);
    ps_vps->i1_vps_max_sub_layers = value + 1;

    ASSERT(ps_vps->i1_vps_max_sub_layers < VPS_MAX_SUB_LAYERS);

    BITS_PARSE("vps_temporal_id_nesting_flag", value, ps_bitstrm, 1);
    ps_vps->i1_vps_temporal_id_nesting_flag = value;

    BITS_PARSE("vps_reserved_ffff_16bits", value, ps_bitstrm, 16);
    ASSERT(value == 0xFFFF);
    // profile_and_level( 1, vps_max_sub_layers_minus1 )
    ret = ihevcd_profile_tier_level(ps_bitstrm, &(ps_vps->s_ptl),
                                    1, (ps_vps->i1_vps_max_sub_layers - 1));

    BITS_PARSE("vps_sub_layer_ordering_info_present_flag", value, ps_bitstrm, 1);
    ps_vps->i1_sub_layer_ordering_info_present_flag = value;
    i = (ps_vps->i1_sub_layer_ordering_info_present_flag ?
                    0 : (ps_vps->i1_vps_max_sub_layers - 1));
    for(; i < ps_vps->i1_vps_max_sub_layers; i++)
    {
        UEV_PARSE("vps_max_dec_pic_buffering[i]", value, ps_bitstrm);
        ps_vps->ai1_vps_max_dec_pic_buffering[i] = value;

        /* vps_num_reorder_pics (no max) used in print in order to match with HM */
        UEV_PARSE("vps_num_reorder_pics[i]", value, ps_bitstrm);
        ps_vps->ai1_vps_max_num_reorder_pics[i] = value;

        UEV_PARSE("vps_max_latency_increase[i]", value, ps_bitstrm);
        ps_vps->ai1_vps_max_latency_increase[i] = value;
    }



    BITS_PARSE("vps_max_layer_id", value, ps_bitstrm, 6);
    //ps_vps->i1_vps_max_layer_id  = value;

    UEV_PARSE("vps_num_layer_sets_minus1", value, ps_bitstrm);
    //ps_vps->i1_vps_num_layer_sets  = value + 1;

    BITS_PARSE("vps_timing_info_present_flag", value, ps_bitstrm, 1);
    //ps_vps->i1_vps_timing_info_present_flag  = value;



    return ret;
}

/**
*******************************************************************************
*
* @brief
*  Parses SPS (Sequence Parameter Set)
* sequence_parameter_set_rbsp()
*
* @par Description:
*  Parse Sequence Parameter Set as per section  Section: 7.3.2.2
* The sps is written to a temporary buffer and copied later to the
* appropriate location
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_parse_sps(codec_t *ps_codec)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 value;

    WORD32 i;
    WORD32 vps_id;
    WORD32 sps_max_sub_layers;
    WORD32 sps_id;
    WORD32 sps_temporal_id_nesting_flag;
    sps_t *ps_sps;
    profile_tier_lvl_info_t s_ptl;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    WORD32 ctb_log2_size_y = 0;


    BITS_PARSE("video_parameter_set_id", value, ps_bitstrm, 4);
    vps_id = value;
    vps_id = CLIP3(vps_id, 0, MAX_VPS_CNT - 1);

    BITS_PARSE("sps_max_sub_layers_minus1", value, ps_bitstrm, 3);
    sps_max_sub_layers = value + 1;
    sps_max_sub_layers = CLIP3(sps_max_sub_layers, 1, 7);

    BITS_PARSE("sps_temporal_id_nesting_flag", value, ps_bitstrm, 1);
    sps_temporal_id_nesting_flag = value;

    //profile_and_level( 1, sps_max_sub_layers_minus1 )
    ret = ihevcd_profile_tier_level(ps_bitstrm, &(s_ptl), 1,
                                    (sps_max_sub_layers - 1));

    UEV_PARSE("seq_parameter_set_id", value, ps_bitstrm);
    sps_id = value;

    if((sps_id >= MAX_SPS_CNT) || (sps_id < 0))
    {
        if(ps_codec->i4_sps_done)
            return IHEVCD_UNSUPPORTED_SPS_ID;
        else
            sps_id = 0;
    }


    ps_sps = (ps_codec->s_parse.ps_sps_base + MAX_SPS_CNT - 1);
    /* Reset SPS to zero */
    {
        WORD16 *pi2_scaling_mat = ps_sps->pi2_scaling_mat;
        memset(ps_sps, 0, sizeof(sps_t));
        ps_sps->pi2_scaling_mat = pi2_scaling_mat;
    }
    ps_sps->i1_sps_id = sps_id;
    ps_sps->i1_vps_id = vps_id;
    ps_sps->i1_sps_max_sub_layers = sps_max_sub_layers;
    ps_sps->i1_sps_temporal_id_nesting_flag = sps_temporal_id_nesting_flag;

    memcpy(&ps_sps->s_ptl, &s_ptl, sizeof(profile_tier_lvl_info_t));

    UEV_PARSE("chroma_format_idc", value, ps_bitstrm);
    ps_sps->i1_chroma_format_idc = value;

    if(ps_sps->i1_chroma_format_idc != CHROMA_FMT_IDC_YUV420)
    {
        ps_codec->s_parse.i4_error_code = IHEVCD_UNSUPPORTED_CHROMA_FMT_IDC;
        return (IHEVCD_ERROR_T)IHEVCD_UNSUPPORTED_CHROMA_FMT_IDC;
    }

    if(CHROMA_FMT_IDC_YUV444_PLANES == ps_sps->i1_chroma_format_idc)
    {
        BITS_PARSE("separate_colour_plane_flag", value, ps_bitstrm, 1);
        ps_sps->i1_separate_colour_plane_flag = value;
    }
    else
    {
        ps_sps->i1_separate_colour_plane_flag = 0;
    }

    UEV_PARSE("pic_width_in_luma_samples", value, ps_bitstrm);
    ps_sps->i2_pic_width_in_luma_samples = value;

    UEV_PARSE("pic_height_in_luma_samples", value, ps_bitstrm);
    ps_sps->i2_pic_height_in_luma_samples = value;

    if((0 >= ps_sps->i2_pic_width_in_luma_samples) || (0 >= ps_sps->i2_pic_height_in_luma_samples))
        return IHEVCD_INVALID_PARAMETER;

    BITS_PARSE("pic_cropping_flag", value, ps_bitstrm, 1);
    ps_sps->i1_pic_cropping_flag = value;

    if(ps_sps->i1_pic_cropping_flag)
    {

        UEV_PARSE("pic_crop_left_offset", value, ps_bitstrm);
        if (value < 0 || value >= ps_sps->i2_pic_width_in_luma_samples)
        {
            return IHEVCD_INVALID_PARAMETER;
        }
        ps_sps->i2_pic_crop_left_offset = value;

        UEV_PARSE("pic_crop_right_offset", value, ps_bitstrm);
        if (value < 0 || value >= ps_sps->i2_pic_width_in_luma_samples)
        {
            return IHEVCD_INVALID_PARAMETER;
        }
        ps_sps->i2_pic_crop_right_offset = value;

        UEV_PARSE("pic_crop_top_offset", value, ps_bitstrm);
        if (value < 0 || value >= ps_sps->i2_pic_height_in_luma_samples)
        {
            return IHEVCD_INVALID_PARAMETER;
        }
        ps_sps->i2_pic_crop_top_offset = value;

        UEV_PARSE("pic_crop_bottom_offset", value, ps_bitstrm);
        if (value < 0 || value >= ps_sps->i2_pic_height_in_luma_samples)
        {
            return IHEVCD_INVALID_PARAMETER;
        }
        ps_sps->i2_pic_crop_bottom_offset = value;
    }
    else
    {
        ps_sps->i2_pic_crop_left_offset = 0;
        ps_sps->i2_pic_crop_right_offset = 0;
        ps_sps->i2_pic_crop_top_offset = 0;
        ps_sps->i2_pic_crop_bottom_offset = 0;
    }


    UEV_PARSE("bit_depth_luma_minus8", value, ps_bitstrm);
    if(0 != value)
        return IHEVCD_UNSUPPORTED_BIT_DEPTH;

    UEV_PARSE("bit_depth_chroma_minus8", value, ps_bitstrm);
    if(0 != value)
        return IHEVCD_UNSUPPORTED_BIT_DEPTH;

    UEV_PARSE("log2_max_pic_order_cnt_lsb_minus4", value, ps_bitstrm);
    if(value < 0 || value > 12)
        return IHEVCD_INVALID_PARAMETER;
    ps_sps->i1_log2_max_pic_order_cnt_lsb = value + 4;

    BITS_PARSE("sps_sub_layer_ordering_info_present_flag", value, ps_bitstrm, 1);
    ps_sps->i1_sps_sub_layer_ordering_info_present_flag = value;


    i = (ps_sps->i1_sps_sub_layer_ordering_info_present_flag ? 0 : (ps_sps->i1_sps_max_sub_layers - 1));
    for(; i < ps_sps->i1_sps_max_sub_layers; i++)
    {
        UEV_PARSE("max_dec_pic_buffering", value, ps_bitstrm);
        if(value < 0 || (value + 1) > MAX_DPB_SIZE)
        {
            return IHEVCD_INVALID_PARAMETER;
        }
        ps_sps->ai1_sps_max_dec_pic_buffering[i] = value + 1;

        UEV_PARSE("num_reorder_pics", value, ps_bitstrm);
        if(value < 0 || value > ps_sps->ai1_sps_max_dec_pic_buffering[i])
        {
            return IHEVCD_INVALID_PARAMETER;
        }
        ps_sps->ai1_sps_max_num_reorder_pics[i] = value;

        UEV_PARSE("max_latency_increase", value, ps_bitstrm);
        ps_sps->ai1_sps_max_latency_increase[i] = value;
    }

    /* Check if sps_max_dec_pic_buffering or sps_max_num_reorder_pics
       has changed */
    if(0 != ps_codec->u4_allocate_dynamic_done)
    {
        sps_t *ps_sps_old = ps_codec->s_parse.ps_sps;
        if(ps_sps_old->ai1_sps_max_dec_pic_buffering[ps_sps_old->i1_sps_max_sub_layers - 1] !=
                    ps_sps->ai1_sps_max_dec_pic_buffering[ps_sps->i1_sps_max_sub_layers - 1])
        {
            if(0 == ps_codec->i4_first_pic_done)
            {
                return IHEVCD_INVALID_PARAMETER;
            }
            ps_codec->i4_reset_flag = 1;
            return (IHEVCD_ERROR_T)IVD_RES_CHANGED;
        }

        if(ps_sps_old->ai1_sps_max_num_reorder_pics[ps_sps_old->i1_sps_max_sub_layers - 1] !=
                    ps_sps->ai1_sps_max_num_reorder_pics[ps_sps->i1_sps_max_sub_layers - 1])
        {
            if(0 == ps_codec->i4_first_pic_done)
            {
                return IHEVCD_INVALID_PARAMETER;
            }
            ps_codec->i4_reset_flag = 1;
            return (IHEVCD_ERROR_T)IVD_RES_CHANGED;
        }
    }

    UEV_PARSE("log2_min_coding_block_size_minus3", value, ps_bitstrm);
    ps_sps->i1_log2_min_coding_block_size = value + 3;

    UEV_PARSE("log2_diff_max_min_coding_block_size", value, ps_bitstrm);
    ps_sps->i1_log2_diff_max_min_coding_block_size = value;

    ctb_log2_size_y = ps_sps->i1_log2_min_coding_block_size + ps_sps->i1_log2_diff_max_min_coding_block_size;

    UEV_PARSE("log2_min_transform_block_size_minus2", value, ps_bitstrm);
    ps_sps->i1_log2_min_transform_block_size = value + 2;

    UEV_PARSE("log2_diff_max_min_transform_block_size", value, ps_bitstrm);
    ps_sps->i1_log2_diff_max_min_transform_block_size = value;

    ps_sps->i1_log2_max_transform_block_size = ps_sps->i1_log2_min_transform_block_size +
                    ps_sps->i1_log2_diff_max_min_transform_block_size;

    if ((ps_sps->i1_log2_max_transform_block_size < 0) ||
                    (ps_sps->i1_log2_max_transform_block_size > MIN(ctb_log2_size_y, 5)))
    {
        return IHEVCD_INVALID_PARAMETER;
    }

    ps_sps->i1_log2_ctb_size = ps_sps->i1_log2_min_coding_block_size +
                    ps_sps->i1_log2_diff_max_min_coding_block_size;

    if((ps_sps->i1_log2_min_coding_block_size < 3) ||
                    (ps_sps->i1_log2_min_transform_block_size < 2) ||
                    (ps_sps->i1_log2_diff_max_min_transform_block_size < 0) ||
                    (ps_sps->i1_log2_max_transform_block_size > ps_sps->i1_log2_ctb_size) ||
                    (ps_sps->i1_log2_ctb_size < 4) ||
                    (ps_sps->i1_log2_ctb_size > 6) ||
                    (ps_sps->i2_pic_width_in_luma_samples % (1 << ps_sps->i1_log2_min_coding_block_size) != 0) ||
                    (ps_sps->i2_pic_height_in_luma_samples % (1 << ps_sps->i1_log2_min_coding_block_size) != 0))
    {
        return IHEVCD_INVALID_PARAMETER;
    }

    ps_sps->i1_log2_min_pcm_coding_block_size = 0;
    ps_sps->i1_log2_diff_max_min_pcm_coding_block_size = 0;

    UEV_PARSE("max_transform_hierarchy_depth_inter", value, ps_bitstrm);
    if(value < 0 || value > (ps_sps->i1_log2_ctb_size - ps_sps->i1_log2_min_transform_block_size))
    {
        return IHEVCD_INVALID_PARAMETER;
    }
    ps_sps->i1_max_transform_hierarchy_depth_inter = value;

    UEV_PARSE("max_transform_hierarchy_depth_intra", value, ps_bitstrm);
    if(value < 0 || value > (ps_sps->i1_log2_ctb_size - ps_sps->i1_log2_min_transform_block_size))
    {
        return IHEVCD_INVALID_PARAMETER;
    }
    ps_sps->i1_max_transform_hierarchy_depth_intra = value;

    /* String has a d (enabled) in order to match with HM */
    BITS_PARSE("scaling_list_enabled_flag", value, ps_bitstrm, 1);
    ps_sps->i1_scaling_list_enable_flag = value;

    if(ps_sps->i1_scaling_list_enable_flag)
    {
        COPY_DEFAULT_SCALING_LIST(ps_sps->pi2_scaling_mat);
        BITS_PARSE("sps_scaling_list_data_present_flag", value, ps_bitstrm, 1);
        ps_sps->i1_sps_scaling_list_data_present_flag = value;

        if(ps_sps->i1_sps_scaling_list_data_present_flag)
            ihevcd_scaling_list_data(ps_codec, ps_sps->pi2_scaling_mat);
    }
    else
    {
        COPY_FLAT_SCALING_LIST(ps_sps->pi2_scaling_mat);
    }
    /* String is asymmetric_motion_partitions_enabled_flag instead of amp_enabled_flag in order to match with HM */
    BITS_PARSE("asymmetric_motion_partitions_enabled_flag", value, ps_bitstrm, 1);
    ps_sps->i1_amp_enabled_flag = value;

    BITS_PARSE("sample_adaptive_offset_enabled_flag", value, ps_bitstrm, 1);
    ps_sps->i1_sample_adaptive_offset_enabled_flag = value;

    BITS_PARSE("pcm_enabled_flag", value, ps_bitstrm, 1);
    ps_sps->i1_pcm_enabled_flag = value;

    if(ps_sps->i1_pcm_enabled_flag)
    {
        BITS_PARSE("pcm_sample_bit_depth_luma", value, ps_bitstrm, 4);
        ps_sps->i1_pcm_sample_bit_depth_luma = value + 1;

        BITS_PARSE("pcm_sample_bit_depth_chroma", value, ps_bitstrm, 4);
        ps_sps->i1_pcm_sample_bit_depth_chroma = value + 1;

        UEV_PARSE("log2_min_pcm_coding_block_size_minus3", value, ps_bitstrm);
        ps_sps->i1_log2_min_pcm_coding_block_size = value + 3;

        UEV_PARSE("log2_diff_max_min_pcm_coding_block_size", value, ps_bitstrm);
        ps_sps->i1_log2_diff_max_min_pcm_coding_block_size = value;
        BITS_PARSE("pcm_loop_filter_disable_flag", value, ps_bitstrm, 1);
        ps_sps->i1_pcm_loop_filter_disable_flag = value;

    }
    UEV_PARSE("num_short_term_ref_pic_sets", value, ps_bitstrm);
    if(value < 0 || value > MAX_STREF_PICS_SPS)
    {
        return IHEVCD_INVALID_PARAMETER;
    }
    ps_sps->i1_num_short_term_ref_pic_sets = value;

    for(i = 0; i < ps_sps->i1_num_short_term_ref_pic_sets; i++)
        ihevcd_short_term_ref_pic_set(ps_bitstrm, &ps_sps->as_stref_picset[0], ps_sps->i1_num_short_term_ref_pic_sets, i, &ps_sps->as_stref_picset[i]);

    BITS_PARSE("long_term_ref_pics_present_flag", value, ps_bitstrm, 1);
    ps_sps->i1_long_term_ref_pics_present_flag = value;

    if(ps_sps->i1_long_term_ref_pics_present_flag)
    {
        UEV_PARSE("num_long_term_ref_pics_sps", value, ps_bitstrm);
        if(value < 0 || value > MAX_LTREF_PICS_SPS)
        {
            return IHEVCD_INVALID_PARAMETER;
        }
        ps_sps->i1_num_long_term_ref_pics_sps = value;

        for(i = 0; i < ps_sps->i1_num_long_term_ref_pics_sps; i++)
        {
            BITS_PARSE("lt_ref_pic_poc_lsb_sps[ i ]", value, ps_bitstrm, ps_sps->i1_log2_max_pic_order_cnt_lsb);
            ps_sps->au2_lt_ref_pic_poc_lsb_sps[i] = value;

            BITS_PARSE("used_by_curr_pic_lt_sps_flag[ i ]", value, ps_bitstrm, 1);
            ps_sps->ai1_used_by_curr_pic_lt_sps_flag[i] = value;
        }
    }

    BITS_PARSE("sps_temporal_mvp_enable_flag", value, ps_bitstrm, 1);
    ps_sps->i1_sps_temporal_mvp_enable_flag = value;

    /* Print matches HM 8-2 */
    BITS_PARSE("sps_strong_intra_smoothing_enable_flag", value, ps_bitstrm, 1);
    ps_sps->i1_strong_intra_smoothing_enable_flag = value;

    BITS_PARSE("vui_parameters_present_flag", value, ps_bitstrm, 1);
    ps_sps->i1_vui_parameters_present_flag = value;

    if(ps_sps->i1_vui_parameters_present_flag)
    {
        ret = ihevcd_parse_vui_parameters(ps_bitstrm,
                                          &ps_sps->s_vui_parameters,
                                          ps_sps->i1_sps_max_sub_layers - 1);
        RETURN_IF((ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS), ret);
    }

    BITS_PARSE("sps_extension_flag", value, ps_bitstrm, 1);

    if((UWORD8 *)ps_bitstrm->pu4_buf > ps_bitstrm->pu1_buf_max)
    {
        return IHEVCD_INVALID_PARAMETER;
    }

    {
        WORD32 numerator;
        WORD32 ceil_offset;

        ceil_offset = (1 << ps_sps->i1_log2_ctb_size) - 1;
        numerator = ps_sps->i2_pic_width_in_luma_samples;

        ps_sps->i2_pic_wd_in_ctb = ((numerator + ceil_offset) /
                        (1 << ps_sps->i1_log2_ctb_size));

        numerator = ps_sps->i2_pic_height_in_luma_samples;
        ps_sps->i2_pic_ht_in_ctb = ((numerator + ceil_offset) /
                        (1 << ps_sps->i1_log2_ctb_size));

        ps_sps->i4_pic_size_in_ctb = ps_sps->i2_pic_ht_in_ctb *
                        ps_sps->i2_pic_wd_in_ctb;

        if(0 == ps_codec->i4_sps_done)
            ps_codec->s_parse.i4_next_ctb_indx = ps_sps->i4_pic_size_in_ctb;

        numerator = ps_sps->i2_pic_width_in_luma_samples;
        ps_sps->i2_pic_wd_in_min_cb = numerator  /
                        (1 << ps_sps->i1_log2_min_coding_block_size);

        numerator = ps_sps->i2_pic_height_in_luma_samples;
        ps_sps->i2_pic_ht_in_min_cb = numerator  /
                        (1 << ps_sps->i1_log2_min_coding_block_size);
    }
    if((0 != ps_codec->u4_allocate_dynamic_done) &&
                    ((ps_codec->i4_wd != ps_sps->i2_pic_width_in_luma_samples) ||
                    (ps_codec->i4_ht != ps_sps->i2_pic_height_in_luma_samples)))
    {
        if(0 == ps_codec->i4_first_pic_done)
        {
            return IHEVCD_INVALID_PARAMETER;
        }
        ps_codec->i4_reset_flag = 1;
        return (IHEVCD_ERROR_T)IVD_RES_CHANGED;
    }

    if((ps_sps->i2_pic_width_in_luma_samples > MAX_WD) ||
                    ((ps_sps->i2_pic_width_in_luma_samples * ps_sps->i2_pic_height_in_luma_samples) >
                    (MAX_WD * MAX_HT)))
    {
        return (IHEVCD_ERROR_T)IVD_STREAM_WIDTH_HEIGHT_NOT_SUPPORTED;
    }

    /* Update display width and display height */
    {
        WORD32 disp_wd, disp_ht;
        WORD32 crop_unit_x, crop_unit_y;
        crop_unit_x = 1;
        crop_unit_y = 1;

        if(CHROMA_FMT_IDC_YUV420 == ps_sps->i1_chroma_format_idc)
        {
            crop_unit_x = 2;
            crop_unit_y = 2;
        }

        disp_wd = ps_sps->i2_pic_width_in_luma_samples;
        disp_wd -= ps_sps->i2_pic_crop_left_offset * crop_unit_x;
        disp_wd -= ps_sps->i2_pic_crop_right_offset * crop_unit_x;


        disp_ht = ps_sps->i2_pic_height_in_luma_samples;
        disp_ht -= ps_sps->i2_pic_crop_top_offset * crop_unit_y;
        disp_ht -= ps_sps->i2_pic_crop_bottom_offset * crop_unit_y;

        if((0 >= disp_wd) || (0 >= disp_ht))
            return IHEVCD_INVALID_PARAMETER;

        if((0 != ps_codec->u4_allocate_dynamic_done) &&
                            ((ps_codec->i4_disp_wd != disp_wd) ||
                            (ps_codec->i4_disp_ht != disp_ht)))
        {
            if(0 == ps_codec->i4_first_pic_done)
            {
                return IHEVCD_INVALID_PARAMETER;
            }
            ps_codec->i4_reset_flag = 1;
            return (IHEVCD_ERROR_T)IVD_RES_CHANGED;
        }

        ps_codec->i4_disp_wd = disp_wd;
        ps_codec->i4_disp_ht = disp_ht;


        ps_codec->i4_wd = ps_sps->i2_pic_width_in_luma_samples;
        ps_codec->i4_ht = ps_sps->i2_pic_height_in_luma_samples;

        {
            WORD32 ref_strd;
            ref_strd = ALIGN32(ps_sps->i2_pic_width_in_luma_samples + PAD_WD);
            if(ps_codec->i4_strd < ref_strd)
            {
                ps_codec->i4_strd = ref_strd;
            }
        }

        if(0 == ps_codec->i4_share_disp_buf)
        {
            if(ps_codec->i4_disp_strd < ps_codec->i4_disp_wd)
            {
                ps_codec->i4_disp_strd = ps_codec->i4_disp_wd;
            }
        }
        else
        {
            if(ps_codec->i4_disp_strd < ps_codec->i4_strd)
            {
                ps_codec->i4_disp_strd = ps_codec->i4_strd;
            }
        }
    }

    /* This is used only during initialization to get reorder count etc */
    ps_codec->i4_sps_id = sps_id;

    ps_codec->i4_sps_done = 1;
    return ret;
}


void ihevcd_unmark_pps(codec_t *ps_codec, WORD32 sps_id)
{
    WORD32 pps_id = 0;
    pps_t *ps_pps = ps_codec->ps_pps_base;

    for(pps_id = 0; pps_id < MAX_PPS_CNT - 1; pps_id++, ps_pps++)
    {
        if((ps_pps->i1_pps_valid) &&
                        (ps_pps->i1_sps_id == sps_id))
            ps_pps->i1_pps_valid = 0;
    }
}


void ihevcd_copy_sps(codec_t *ps_codec, WORD32 sps_id, WORD32 sps_id_ref)
{
    sps_t *ps_sps, *ps_sps_ref;
    WORD16 *pi2_scaling_mat_backup;
    WORD32 scaling_mat_size;

    SCALING_MAT_SIZE(scaling_mat_size);
    ps_sps_ref = ps_codec->ps_sps_base + sps_id_ref;
    ps_sps = ps_codec->ps_sps_base + sps_id;

    if(ps_sps->i1_sps_valid)
    {
        if((ps_sps->i1_log2_ctb_size != ps_sps_ref->i1_log2_ctb_size) ||
                        (ps_sps->i2_pic_wd_in_ctb != ps_sps_ref->i2_pic_wd_in_ctb) ||
                        (ps_sps->i2_pic_ht_in_ctb != ps_sps_ref->i2_pic_ht_in_ctb))
        {
            ihevcd_unmark_pps(ps_codec, sps_id);
        }
    }

    pi2_scaling_mat_backup = ps_sps->pi2_scaling_mat;

    memcpy(ps_sps, ps_sps_ref, sizeof(sps_t));
    ps_sps->pi2_scaling_mat = pi2_scaling_mat_backup;
    memcpy(ps_sps->pi2_scaling_mat, ps_sps_ref->pi2_scaling_mat, scaling_mat_size * sizeof(WORD16));
    ps_sps->i1_sps_valid = 1;

    ps_codec->s_parse.ps_sps = ps_sps;
}


/**
*******************************************************************************
*
* @brief
*  Parses PPS (Picture Parameter Set)
*
* @par Description:
*  Parse Picture Parameter Set as per section  Section: 7.3.2.3
* The pps is written to a temporary buffer and copied later to the
* appropriate location
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_parse_pps(codec_t *ps_codec)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 value;
    WORD32 pps_id;

    pps_t *ps_pps;
    sps_t *ps_sps;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;


    if(0 == ps_codec->i4_sps_done)
        return IHEVCD_INVALID_HEADER;

    UEV_PARSE("pic_parameter_set_id", value, ps_bitstrm);

    pps_id = value;
    if((pps_id >= MAX_PPS_CNT) || (pps_id < 0))
    {
        if(ps_codec->i4_pps_done)
            return IHEVCD_UNSUPPORTED_PPS_ID;
        else
            pps_id = 0;
    }


    ps_pps = (ps_codec->s_parse.ps_pps_base + MAX_PPS_CNT - 1);

    ps_pps->i1_pps_id = pps_id;

    UEV_PARSE("seq_parameter_set_id", value, ps_bitstrm);
    ps_pps->i1_sps_id = value;
    ps_pps->i1_sps_id = CLIP3(ps_pps->i1_sps_id, 0, MAX_SPS_CNT - 2);

    ps_sps = (ps_codec->s_parse.ps_sps_base + ps_pps->i1_sps_id);

    /* If the SPS that is being referred to has not been parsed,
     * copy an existing SPS to the current location */
    if(0 == ps_sps->i1_sps_valid)
    {
        return IHEVCD_INVALID_HEADER;

/*
        sps_t *ps_sps_ref = ps_codec->ps_sps_base;
        while(0 == ps_sps_ref->i1_sps_valid)
            ps_sps_ref++;
        ihevcd_copy_sps(ps_codec, ps_pps->i1_sps_id, ps_sps_ref->i1_sps_id);
*/
    }

    BITS_PARSE("dependent_slices_enabled_flag", value, ps_bitstrm, 1);
    ps_pps->i1_dependent_slice_enabled_flag = value;

    BITS_PARSE("output_flag_present_flag", value, ps_bitstrm, 1);
    ps_pps->i1_output_flag_present_flag = value;

    BITS_PARSE("num_extra_slice_header_bits", value, ps_bitstrm, 3);
    ps_pps->i1_num_extra_slice_header_bits = value;


    BITS_PARSE("sign_data_hiding_flag", value, ps_bitstrm, 1);
    ps_pps->i1_sign_data_hiding_flag = value;

    BITS_PARSE("cabac_init_present_flag", value, ps_bitstrm, 1);
    ps_pps->i1_cabac_init_present_flag = value;

    UEV_PARSE("num_ref_idx_l0_default_active_minus1", value, ps_bitstrm);
    ps_pps->i1_num_ref_idx_l0_default_active = value + 1;

    UEV_PARSE("num_ref_idx_l1_default_active_minus1", value, ps_bitstrm);
    ps_pps->i1_num_ref_idx_l1_default_active = value + 1;

    SEV_PARSE("pic_init_qp_minus26", value, ps_bitstrm);
    ps_pps->i1_pic_init_qp = value + 26;

    BITS_PARSE("constrained_intra_pred_flag", value, ps_bitstrm, 1);
    ps_pps->i1_constrained_intra_pred_flag = value;

    BITS_PARSE("transform_skip_enabled_flag", value, ps_bitstrm, 1);
    ps_pps->i1_transform_skip_enabled_flag = value;

    BITS_PARSE("cu_qp_delta_enabled_flag", value, ps_bitstrm, 1);
    ps_pps->i1_cu_qp_delta_enabled_flag = value;

    if(ps_pps->i1_cu_qp_delta_enabled_flag)
    {
        UEV_PARSE("diff_cu_qp_delta_depth", value, ps_bitstrm);
        ps_pps->i1_diff_cu_qp_delta_depth = value;
    }
    else
    {
        ps_pps->i1_diff_cu_qp_delta_depth = 0;
    }
    ps_pps->i1_log2_min_cu_qp_delta_size = ps_sps->i1_log2_ctb_size - ps_pps->i1_diff_cu_qp_delta_depth;
    /* Print different */
    SEV_PARSE("cb_qp_offset", value, ps_bitstrm);
    ps_pps->i1_pic_cb_qp_offset = value;

    /* Print different */
    SEV_PARSE("cr_qp_offset", value, ps_bitstrm);
    ps_pps->i1_pic_cr_qp_offset = value;

    /* Print different */
    BITS_PARSE("slicelevel_chroma_qp_flag", value, ps_bitstrm, 1);
    ps_pps->i1_pic_slice_level_chroma_qp_offsets_present_flag = value;

    BITS_PARSE("weighted_pred_flag", value, ps_bitstrm, 1);
    ps_pps->i1_weighted_pred_flag = value;

    BITS_PARSE("weighted_bipred_flag", value, ps_bitstrm, 1);
    ps_pps->i1_weighted_bipred_flag = value;

    BITS_PARSE("transquant_bypass_enable_flag", value, ps_bitstrm, 1);
    ps_pps->i1_transquant_bypass_enable_flag = value;

    BITS_PARSE("tiles_enabled_flag", value, ps_bitstrm, 1);
    ps_pps->i1_tiles_enabled_flag = value;

    /* When tiles are enabled and width or height is >= 4096,
     * CTB Size should at least be 32 while if width or height is >= 8192,
     * CTB Size should at least be 64 and so on. 16x16 CTBs can result
     * in tile position greater than 255 for 4096 while 32x32 CTBs can result
     * in tile position greater than 255 for 8192,
     * which decoder does not support.
     */
    if (ps_pps->i1_tiles_enabled_flag)
    {
        if((ps_sps->i1_log2_ctb_size == 4) &&
            ((ps_sps->i2_pic_width_in_luma_samples >= 4096) ||
            (ps_sps->i2_pic_height_in_luma_samples >= 4096)))
        {
            return IHEVCD_INVALID_HEADER;
        }
        if((ps_sps->i1_log2_ctb_size == 5) &&
            ((ps_sps->i2_pic_width_in_luma_samples >= 8192) ||
            (ps_sps->i2_pic_height_in_luma_samples >= 8192)))
        {
            return IHEVCD_INVALID_HEADER;
        }
        if((ps_sps->i1_log2_ctb_size == 6) &&
            ((ps_sps->i2_pic_width_in_luma_samples >= 16384) ||
            (ps_sps->i2_pic_height_in_luma_samples >= 16384)))
        {
            return IHEVCD_INVALID_HEADER;
        }
    }

    BITS_PARSE("entropy_coding_sync_enabled_flag", value, ps_bitstrm, 1);
    ps_pps->i1_entropy_coding_sync_enabled_flag = value;

    ps_pps->i1_loop_filter_across_tiles_enabled_flag = 0;
    if(ps_pps->i1_tiles_enabled_flag)
    {
        WORD32 wd = ALIGN64(ps_codec->i4_wd);
        WORD32 ht = ALIGN64(ps_codec->i4_ht);

        WORD32 max_tile_cols = (wd + MIN_TILE_WD - 1) / MIN_TILE_WD;
        WORD32 max_tile_rows = (ht + MIN_TILE_HT - 1) / MIN_TILE_HT;

        UEV_PARSE("num_tile_columns_minus1", value, ps_bitstrm);
        ps_pps->i1_num_tile_columns = value + 1;

        UEV_PARSE("num_tile_rows_minus1", value, ps_bitstrm);
        ps_pps->i1_num_tile_rows = value + 1;

        if((ps_pps->i1_num_tile_columns < 1) ||
                        (ps_pps->i1_num_tile_columns > max_tile_cols) ||
                        (ps_pps->i1_num_tile_rows < 1) ||
                        (ps_pps->i1_num_tile_rows > max_tile_rows))
            return IHEVCD_INVALID_HEADER;

        BITS_PARSE("uniform_spacing_flag", value, ps_bitstrm, 1);
        ps_pps->i1_uniform_spacing_flag = value;


        {

            WORD32 start;
            WORD32 i, j;


            start = 0;
            for(i = 0; i < ps_pps->i1_num_tile_columns; i++)
            {
                tile_t *ps_tile;
                if(!ps_pps->i1_uniform_spacing_flag)
                {
                    if(i < (ps_pps->i1_num_tile_columns - 1))
                    {
                        UEV_PARSE("column_width_minus1[ i ]", value, ps_bitstrm);
                        value += 1;
                    }
                    else
                    {
                        value = ps_sps->i2_pic_wd_in_ctb - start;
                    }
                }
                else
                {
                    value = ((i + 1) * ps_sps->i2_pic_wd_in_ctb) / ps_pps->i1_num_tile_columns -
                                    (i * ps_sps->i2_pic_wd_in_ctb) / ps_pps->i1_num_tile_columns;
                }

                for(j = 0; j < ps_pps->i1_num_tile_rows; j++)
                {
                    ps_tile = ps_pps->ps_tile + j * ps_pps->i1_num_tile_columns + i;
                    ps_tile->u1_pos_x = start;
                    ps_tile->u2_wd = value;
                }
                start += value;

                if((start > ps_sps->i2_pic_wd_in_ctb) ||
                                (value <= 0))
                    return IHEVCD_INVALID_HEADER;
            }

            start = 0;
            for(i = 0; i < (ps_pps->i1_num_tile_rows); i++)
            {
                tile_t *ps_tile;
                if(!ps_pps->i1_uniform_spacing_flag)
                {
                    if(i < (ps_pps->i1_num_tile_rows - 1))
                    {

                        UEV_PARSE("row_height_minus1[ i ]", value, ps_bitstrm);
                        value += 1;
                    }
                    else
                    {
                        value = ps_sps->i2_pic_ht_in_ctb - start;
                    }
                }
                else
                {
                    value = ((i + 1) * ps_sps->i2_pic_ht_in_ctb) / ps_pps->i1_num_tile_rows -
                                    (i * ps_sps->i2_pic_ht_in_ctb) / ps_pps->i1_num_tile_rows;
                }

                for(j = 0; j < ps_pps->i1_num_tile_columns; j++)
                {
                    ps_tile = ps_pps->ps_tile + i * ps_pps->i1_num_tile_columns + j;
                    ps_tile->u1_pos_y = start;
                    ps_tile->u2_ht = value;
                }
                start += value;

                if((start > ps_sps->i2_pic_ht_in_ctb) ||
                                (value <= 0))
                    return IHEVCD_INVALID_HEADER;
            }
        }


        BITS_PARSE("loop_filter_across_tiles_enabled_flag", value, ps_bitstrm, 1);
        ps_pps->i1_loop_filter_across_tiles_enabled_flag = value;

    }
    else
    {
        /* If tiles are not present, set first tile in each PPS to have tile
        width and height equal to picture width and height */
        ps_pps->i1_num_tile_columns = 1;
        ps_pps->i1_num_tile_rows = 1;
        ps_pps->i1_uniform_spacing_flag = 1;

        ps_pps->ps_tile->u1_pos_x = 0;
        ps_pps->ps_tile->u1_pos_y = 0;
        ps_pps->ps_tile->u2_wd = ps_sps->i2_pic_wd_in_ctb;
        ps_pps->ps_tile->u2_ht = ps_sps->i2_pic_ht_in_ctb;
    }

    BITS_PARSE("loop_filter_across_slices_enabled_flag", value, ps_bitstrm, 1);
    ps_pps->i1_loop_filter_across_slices_enabled_flag = value;

    BITS_PARSE("deblocking_filter_control_present_flag", value, ps_bitstrm, 1);
    ps_pps->i1_deblocking_filter_control_present_flag = value;

    /* Default values */
    ps_pps->i1_pic_disable_deblocking_filter_flag = 0;
    ps_pps->i1_deblocking_filter_override_enabled_flag = 0;
    ps_pps->i1_beta_offset_div2 = 0;
    ps_pps->i1_tc_offset_div2 = 0;

    if(ps_pps->i1_deblocking_filter_control_present_flag)
    {

        BITS_PARSE("deblocking_filter_override_enabled_flag", value, ps_bitstrm, 1);
        ps_pps->i1_deblocking_filter_override_enabled_flag = value;

        BITS_PARSE("pic_disable_deblocking_filter_flag", value, ps_bitstrm, 1);
        ps_pps->i1_pic_disable_deblocking_filter_flag = value;

        if(!ps_pps->i1_pic_disable_deblocking_filter_flag)
        {

            SEV_PARSE("pps_beta_offset_div2", value, ps_bitstrm);
            ps_pps->i1_beta_offset_div2 = value;

            SEV_PARSE("pps_tc_offset_div2", value, ps_bitstrm);
            ps_pps->i1_tc_offset_div2 = value;

        }
    }

    BITS_PARSE("pps_scaling_list_data_present_flag", value, ps_bitstrm, 1);
    ps_pps->i1_pps_scaling_list_data_present_flag = value;

    if(ps_pps->i1_pps_scaling_list_data_present_flag)
    {
        COPY_DEFAULT_SCALING_LIST(ps_pps->pi2_scaling_mat);
        ihevcd_scaling_list_data(ps_codec, ps_pps->pi2_scaling_mat);
    }

    BITS_PARSE("lists_modification_present_flag", value, ps_bitstrm, 1);
    ps_pps->i1_lists_modification_present_flag = value;
    UEV_PARSE("log2_parallel_merge_level_minus2", value, ps_bitstrm);
    ps_pps->i1_log2_parallel_merge_level = value + 2;

    BITS_PARSE("slice_header_extension_present_flag", value, ps_bitstrm, 1);
    ps_pps->i1_slice_header_extension_present_flag = value;
    /* Not present in HM */
    BITS_PARSE("pps_extension_flag", value, ps_bitstrm, 1);

    if((UWORD8 *)ps_bitstrm->pu4_buf > ps_bitstrm->pu1_buf_max)
        return IHEVCD_INVALID_PARAMETER;

    ps_codec->i4_pps_done = 1;
    return ret;
}


void ihevcd_copy_pps(codec_t *ps_codec, WORD32 pps_id, WORD32 pps_id_ref)
{
    pps_t *ps_pps, *ps_pps_ref;
    WORD16 *pi2_scaling_mat_backup;
    WORD32 scaling_mat_size;
    tile_t *ps_tile_backup;
    WORD32 max_tile_cols, max_tile_rows;
    WORD32 wd, ht;
    wd = ALIGN64(ps_codec->i4_wd);
    ht = ALIGN64(ps_codec->i4_ht);

    SCALING_MAT_SIZE(scaling_mat_size);
    max_tile_cols = (wd + MIN_TILE_WD - 1) / MIN_TILE_WD;
    max_tile_rows = (ht + MIN_TILE_HT - 1) / MIN_TILE_HT;

    ps_pps_ref = ps_codec->ps_pps_base + pps_id_ref;
    ps_pps = ps_codec->ps_pps_base + pps_id;

    pi2_scaling_mat_backup = ps_pps->pi2_scaling_mat;
    ps_tile_backup = ps_pps->ps_tile;

    memcpy(ps_pps, ps_pps_ref, sizeof(pps_t));
    ps_pps->pi2_scaling_mat = pi2_scaling_mat_backup;
    ps_pps->ps_tile = ps_tile_backup;
    memcpy(ps_pps->pi2_scaling_mat, ps_pps_ref->pi2_scaling_mat, scaling_mat_size * sizeof(WORD16));
    memcpy(ps_pps->ps_tile, ps_pps_ref->ps_tile, max_tile_cols * max_tile_rows * sizeof(tile_t));

    ps_pps->i1_pps_valid = 1;

    ps_codec->s_parse.ps_pps = ps_pps;
}


IHEVCD_ERROR_T ihevcd_parse_buffering_period_sei(codec_t *ps_codec,
                                                 sps_t *ps_sps)
{
    parse_ctxt_t *ps_parse = &ps_codec->s_parse;
    bitstrm_t *ps_bitstrm = &ps_parse->s_bitstrm;
    UWORD32 value;
    vui_t *ps_vui;
    buf_period_sei_params_t *ps_buf_period_sei_params;
    UWORD32 i;
    hrd_params_t *ps_vui_hdr;
    UWORD32 u4_cpb_cnt;

    ps_vui = &ps_sps->s_vui_parameters;
    ps_vui_hdr = &ps_vui->s_vui_hrd_parameters;

    ps_buf_period_sei_params = &ps_parse->s_sei_params.s_buf_period_sei_params;

    ps_parse->s_sei_params.i1_buf_period_params_present_flag = 1;

    UEV_PARSE("bp_seq_parameter_set_id", value, ps_bitstrm);
    ps_buf_period_sei_params->u1_bp_seq_parameter_set_id = value;

    if(!ps_vui_hdr->u1_sub_pic_cpb_params_present_flag)
    {
        BITS_PARSE("irap_cpb_params_present_flag", value, ps_bitstrm, 1);
        ps_buf_period_sei_params->u1_rap_cpb_params_present_flag = value;
    }

    if(ps_buf_period_sei_params->u1_rap_cpb_params_present_flag)
    {
        BITS_PARSE("cpb_delay_offset",
                   value,
                   ps_bitstrm,
                   (ps_vui_hdr->u1_au_cpb_removal_delay_length_minus1
                                   + 1));
        ps_buf_period_sei_params->u4_cpb_delay_offset = value;

        BITS_PARSE("dpb_delay_offset",
                   value,
                   ps_bitstrm,
                   (ps_vui_hdr->u1_dpb_output_delay_length_minus1
                                   + 1));
        ps_buf_period_sei_params->u4_dpb_delay_offset = value;
    }
    else
    {
        ps_buf_period_sei_params->u4_cpb_delay_offset = 0;
        ps_buf_period_sei_params->u4_dpb_delay_offset = 0;
    }

    BITS_PARSE("concatenation_flag", value, ps_bitstrm, 1);
    ps_buf_period_sei_params->u1_concatenation_flag = value;

    BITS_PARSE("au_cpb_removal_delay_delta_minus1",
               value,
               ps_bitstrm,
               (ps_vui_hdr->u1_au_cpb_removal_delay_length_minus1
                               + 1));
    ps_buf_period_sei_params->u4_au_cpb_removal_delay_delta_minus1 = value;

    if(ps_vui_hdr->u1_nal_hrd_parameters_present_flag)
    {
        u4_cpb_cnt = ps_vui_hdr->au1_cpb_cnt_minus1[0];

        for(i = 0; i <= u4_cpb_cnt; i++)
        {
            BITS_PARSE("nal_initial_cpb_removal_delay[i]",
                       value,
                       ps_bitstrm,
                       (ps_vui_hdr->u1_initial_cpb_removal_delay_length_minus1
                                       + 1));
            ps_buf_period_sei_params->au4_nal_initial_cpb_removal_delay[i] =
                            value;

            BITS_PARSE("nal_initial_cpb_removal_delay_offset",
                       value,
                       ps_bitstrm,
                       (ps_vui_hdr->u1_initial_cpb_removal_delay_length_minus1
                                       + 1));
            ps_buf_period_sei_params->au4_nal_initial_cpb_removal_delay_offset[i] =
                            value;

            if(ps_vui_hdr->u1_sub_pic_cpb_params_present_flag
                            || ps_buf_period_sei_params->u1_rap_cpb_params_present_flag)
            {
                BITS_PARSE("nal_initial_alt_cpb_removal_delay[i]",
                           value,
                           ps_bitstrm,
                           (ps_vui_hdr->u1_initial_cpb_removal_delay_length_minus1
                                           + 1));
                ps_buf_period_sei_params->au4_nal_initial_alt_cpb_removal_delay[i] =
                                value;

                BITS_PARSE("nal_initial_alt_cpb_removal_delay_offset",
                           value,
                           ps_bitstrm,
                           (ps_vui_hdr->u1_initial_cpb_removal_delay_length_minus1
                                           + 1));
                ps_buf_period_sei_params->au4_nal_initial_alt_cpb_removal_delay_offset[i] =
                                value;
            }
        }
    }

    if(ps_vui_hdr->u1_vcl_hrd_parameters_present_flag)
    {
        u4_cpb_cnt = ps_vui_hdr->au1_cpb_cnt_minus1[0];

        for(i = 0; i <= u4_cpb_cnt; i++)
        {
            BITS_PARSE("vcl_initial_cpb_removal_delay[i]",
                       value,
                       ps_bitstrm,
                       (ps_vui_hdr->u1_initial_cpb_removal_delay_length_minus1
                                       + 1));
            ps_buf_period_sei_params->au4_vcl_initial_cpb_removal_delay[i] =
                            value;

            BITS_PARSE("vcl_initial_cpb_removal_delay_offset",
                       value,
                       ps_bitstrm,
                       (ps_vui_hdr->u1_initial_cpb_removal_delay_length_minus1
                                       + 1));
            ps_buf_period_sei_params->au4_vcl_initial_cpb_removal_delay_offset[i] =
                            value;

            if(ps_vui_hdr->u1_sub_pic_cpb_params_present_flag
                            || ps_buf_period_sei_params->u1_rap_cpb_params_present_flag)
            {
                BITS_PARSE("vcl_initial_alt_cpb_removal_delay[i]",
                           value,
                           ps_bitstrm,
                           (ps_vui_hdr->u1_initial_cpb_removal_delay_length_minus1
                                           + 1));
                ps_buf_period_sei_params->au4_vcl_initial_alt_cpb_removal_delay[i] =
                                value;

                BITS_PARSE("vcl_initial_alt_cpb_removal_delay_offset",
                           value,
                           ps_bitstrm,
                           (ps_vui_hdr->u1_initial_cpb_removal_delay_length_minus1
                                           + 1));
                ps_buf_period_sei_params->au4_vcl_initial_alt_cpb_removal_delay_offset[i] =
                                value;
            }
        }
    }

    return (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
}

IHEVCD_ERROR_T ihevcd_parse_pic_timing_sei(codec_t *ps_codec, sps_t *ps_sps)
{
    parse_ctxt_t *ps_parse = &ps_codec->s_parse;
    bitstrm_t *ps_bitstrm = &ps_parse->s_bitstrm;
    UWORD32 value;
    vui_t *ps_vui;
    UWORD32 i;
    hrd_params_t *ps_vui_hdr;
    UWORD32 u4_cpb_dpb_delays_present_flag = 0;
    pic_timing_sei_params_t *ps_pic_timing;

    ps_pic_timing = &ps_parse->s_sei_params.s_pic_timing_sei_params;
    ps_vui = &ps_sps->s_vui_parameters;
    ps_vui_hdr = &ps_vui->s_vui_hrd_parameters;
    ps_parse->s_sei_params.i1_pic_timing_params_present_flag = 1;
    if(ps_vui->u1_frame_field_info_present_flag)
    {
        BITS_PARSE("pic_struct", value, ps_bitstrm, 4);
        ps_pic_timing->u4_pic_struct = value;

        BITS_PARSE("source_scan_type", value, ps_bitstrm, 2);
        ps_pic_timing->u4_source_scan_type = value;

        BITS_PARSE("duplicate_flag", value, ps_bitstrm, 1);
        ps_pic_timing->u1_duplicate_flag = value;
    }

    if(ps_vui_hdr->u1_nal_hrd_parameters_present_flag
                    || ps_vui_hdr->u1_vcl_hrd_parameters_present_flag)
    {
        u4_cpb_dpb_delays_present_flag = 1;
    }
    else
    {
        u4_cpb_dpb_delays_present_flag = 0;
    }

    if(u4_cpb_dpb_delays_present_flag)
    {
        BITS_PARSE("au_cpb_removal_delay_minus1", value, ps_bitstrm,
                   (ps_vui_hdr->u1_au_cpb_removal_delay_length_minus1 + 1));
        ps_pic_timing->u4_au_cpb_removal_delay_minus1 = value;

        BITS_PARSE("pic_dpb_output_delay", value, ps_bitstrm,
                   (ps_vui_hdr->u1_dpb_output_delay_length_minus1 + 1));
        ps_pic_timing->u4_pic_dpb_output_delay = value;

        if(ps_vui_hdr->u1_sub_pic_cpb_params_present_flag)
        {
            BITS_PARSE("pic_dpb_output_du_delay", value, ps_bitstrm,
                       (ps_vui_hdr->u1_dpb_output_delay_du_length_minus1 + 1));
            ps_pic_timing->u4_pic_dpb_output_du_delay = value;
        }

        if(ps_vui_hdr->u1_sub_pic_cpb_params_present_flag
                        && ps_vui_hdr->u1_sub_pic_cpb_params_in_pic_timing_sei_flag)
        {
            UWORD32 num_units_minus1;
            UWORD32 array_size;

            UEV_PARSE("num_decoding_units_minus1", value, ps_bitstrm);
            ps_pic_timing->u4_num_decoding_units_minus1 = value;

            num_units_minus1 = ps_pic_timing->u4_num_decoding_units_minus1;
            array_size = (sizeof(ps_pic_timing->au4_num_nalus_in_du_minus1)
                       / sizeof(ps_pic_timing->au4_num_nalus_in_du_minus1[0]));
            num_units_minus1 = CLIP3(num_units_minus1, 0,(array_size - 1));
            ps_pic_timing->u4_num_decoding_units_minus1 = num_units_minus1;

            BITS_PARSE("du_common_cpb_removal_delay_flag", value, ps_bitstrm, 1);
            ps_pic_timing->u1_du_common_cpb_removal_delay_flag = value;

            if(ps_pic_timing->u1_du_common_cpb_removal_delay_flag)
            {
                BITS_PARSE("du_common_cpb_removal_delay_increment_minus1",
                           value,
                           ps_bitstrm,
                           (ps_vui_hdr->u1_du_cpb_removal_delay_increment_length_minus1
                                           + 1));
                ps_pic_timing->u4_du_common_cpb_removal_delay_increment_minus1 =
                                value;
            }

            for(i = 0; i <= ps_pic_timing->u4_num_decoding_units_minus1; i++)
            {
                UEV_PARSE("num_nalus_in_du_minus1", value, ps_bitstrm);
                ps_pic_timing->au4_num_nalus_in_du_minus1[i] = value;

                if((!ps_pic_timing->u1_du_common_cpb_removal_delay_flag)
                                && (i < ps_pic_timing->u4_num_decoding_units_minus1))
                {
                    BITS_PARSE("du_common_cpb_removal_delay_increment_minus1",
                               value,
                               ps_bitstrm,
                               (ps_vui_hdr->u1_du_cpb_removal_delay_increment_length_minus1
                                               + 1));
                    ps_pic_timing->au4_du_cpb_removal_delay_increment_minus1[i] =
                                    value;
                }
            }
        }
    }

    return (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
}

IHEVCD_ERROR_T ihevcd_parse_time_code_sei(codec_t *ps_codec)
{
    parse_ctxt_t *ps_parse = &ps_codec->s_parse;
    bitstrm_t *ps_bitstrm = &ps_parse->s_bitstrm;
    UWORD32 value;
    time_code_t *ps_time_code;
    WORD32 i;

    ps_parse->s_sei_params.i1_time_code_present_flag = 1;
    ps_time_code = &ps_parse->s_sei_params.s_time_code;

    BITS_PARSE("num_clock_ts", value, ps_bitstrm, 2);
    ps_time_code->u1_num_clock_ts = value;

    for(i = 0; i < ps_time_code->u1_num_clock_ts; i++)
    {
        BITS_PARSE("clock_timestamp_flag[i]", value, ps_bitstrm, 1);
        ps_time_code->au1_clock_timestamp_flag[i] = value;

        if(ps_time_code->au1_clock_timestamp_flag[i])
        {
            BITS_PARSE("units_field_based_flag[i]", value, ps_bitstrm, 1);
            ps_time_code->au1_units_field_based_flag[i] = value;

            BITS_PARSE("counting_type[i]", value, ps_bitstrm, 5);
            ps_time_code->au1_counting_type[i] = value;

            BITS_PARSE("full_timestamp_flag[i]", value, ps_bitstrm, 1);
            ps_time_code->au1_full_timestamp_flag[i] = value;

            BITS_PARSE("discontinuity_flag[i]", value, ps_bitstrm, 1);
            ps_time_code->au1_discontinuity_flag[i] = value;

            BITS_PARSE("cnt_dropped_flag[i]", value, ps_bitstrm, 1);
            ps_time_code->au1_cnt_dropped_flag[i] = value;

            BITS_PARSE("n_frames[i]", value, ps_bitstrm, 9);
            ps_time_code->au2_n_frames[i] = value;

            if(ps_time_code->au1_full_timestamp_flag[i])
            {
                BITS_PARSE("seconds_value[i]", value, ps_bitstrm, 6);
                ps_time_code->au1_seconds_value[i] = value;

                BITS_PARSE("minutes_value[i]", value, ps_bitstrm, 6);
                ps_time_code->au1_minutes_value[i] = value;

                BITS_PARSE("hours_value[i]", value, ps_bitstrm, 5);
                ps_time_code->au1_hours_value[i] = value;
            }
            else
            {
                BITS_PARSE("seconds_flag[i]", value, ps_bitstrm, 1);
                ps_time_code->au1_seconds_flag[i] = value;

                if(ps_time_code->au1_seconds_flag[i])
                {
                    BITS_PARSE("seconds_value[i]", value, ps_bitstrm, 6);
                    ps_time_code->au1_seconds_value[i] = value;

                    BITS_PARSE("minutes_flag[i]", value, ps_bitstrm, 1);
                    ps_time_code->au1_minutes_flag[i] = value;

                    if(ps_time_code->au1_minutes_flag[i])
                    {
                        BITS_PARSE("minutes_value[i]", value, ps_bitstrm, 6);
                        ps_time_code->au1_minutes_value[i] = value;

                        BITS_PARSE("hours_flag[i]", value, ps_bitstrm, 1);
                        ps_time_code->au1_hours_flag[i] = value;

                        if(ps_time_code->au1_hours_flag[i])
                        {
                            BITS_PARSE("hours_value[i]", value, ps_bitstrm, 5);
                            ps_time_code->au1_hours_value[i] = value;
                        }
                    }
                }
            }

            BITS_PARSE("time_offset_length[i]", value, ps_bitstrm, 5);
            ps_time_code->au1_time_offset_length[i] = value;

            if(ps_time_code->au1_time_offset_length[i] > 0)
            {
                BITS_PARSE("time_offset_value[i]", value, ps_bitstrm,
                           ps_time_code->au1_time_offset_length[i]);
                ps_time_code->au1_time_offset_value[i] = value;
            }
            else
            {
                ps_time_code->au1_time_offset_value[i] = 0;
            }
        }
    }

    return (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
}

IHEVCD_ERROR_T ihevcd_parse_mastering_disp_params_sei(codec_t *ps_codec)
{
    parse_ctxt_t *ps_parse = &ps_codec->s_parse;
    bitstrm_t *ps_bitstrm = &ps_parse->s_bitstrm;
    UWORD32 value;
    mastering_dis_col_vol_sei_params_t *ps_mastering_dis_col_vol;
    WORD32 i;

    ps_parse->s_sei_params.i4_sei_mastering_disp_colour_vol_params_present_flags = 1;

    ps_mastering_dis_col_vol = &ps_parse->s_sei_params.s_mastering_dis_col_vol_sei_params;

    for(i = 0; i < 3; i++)
    {
        BITS_PARSE("display_primaries_x[c]", value, ps_bitstrm, 16);
        ps_mastering_dis_col_vol->au2_display_primaries_x[i] = value;

        BITS_PARSE("display_primaries_y[c]", value, ps_bitstrm, 16);
        ps_mastering_dis_col_vol->au2_display_primaries_y[i] = value;
    }

    BITS_PARSE("white_point_x", value, ps_bitstrm, 16);
    ps_mastering_dis_col_vol->u2_white_point_x = value;

    BITS_PARSE("white_point_y", value, ps_bitstrm, 16);
    ps_mastering_dis_col_vol->u2_white_point_y = value;

    BITS_PARSE("max_display_mastering_luminance", value, ps_bitstrm, 32);
    ps_mastering_dis_col_vol->u4_max_display_mastering_luminance = value;

    BITS_PARSE("min_display_mastering_luminance", value, ps_bitstrm, 32);
    ps_mastering_dis_col_vol->u4_min_display_mastering_luminance = value;

    return (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
}

IHEVCD_ERROR_T ihevcd_parse_user_data_registered_itu_t_t35(codec_t *ps_codec,
                                                           UWORD32 u4_payload_size)
{
    parse_ctxt_t *ps_parse = &ps_codec->s_parse;
    bitstrm_t *ps_bitstrm = &ps_parse->s_bitstrm;
    UWORD32 value;
    user_data_registered_itu_t_t35_t *ps_user_data_registered_itu_t_t35;
    UWORD32 i;
    UWORD32 j = 0;

    ps_parse->s_sei_params.i1_user_data_registered_present_flag = 1;
    ps_user_data_registered_itu_t_t35 =
                    &ps_parse->s_sei_params.as_user_data_registered_itu_t_t35[ps_parse->s_sei_params.i4_sei_user_data_cnt];
    ps_parse->s_sei_params.i4_sei_user_data_cnt++;

    ps_user_data_registered_itu_t_t35->i4_payload_size = u4_payload_size;

    if(u4_payload_size > MAX_USERDATA_PAYLOAD)
    {
        u4_payload_size = MAX_USERDATA_PAYLOAD;
    }

    ps_user_data_registered_itu_t_t35->i4_valid_payload_size = u4_payload_size;

    BITS_PARSE("itu_t_t35_country_code", value, ps_bitstrm, 8);
    ps_user_data_registered_itu_t_t35->u1_itu_t_t35_country_code = value;

    if(0xFF != ps_user_data_registered_itu_t_t35->u1_itu_t_t35_country_code)
    {
        i = 1;
    }
    else
    {
        BITS_PARSE("itu_t_t35_country_code_extension_byte", value, ps_bitstrm,
                   8);
        ps_user_data_registered_itu_t_t35->u1_itu_t_t35_country_code_extension_byte =
                        value;

        i = 2;
    }

    do
    {
        BITS_PARSE("itu_t_t35_payload_byte", value, ps_bitstrm, 8);
        ps_user_data_registered_itu_t_t35->u1_itu_t_t35_payload_byte[j++] =
                        value;

        i++;
    }while(i < u4_payload_size);

    return (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
}

void ihevcd_parse_sei_payload(codec_t *ps_codec,
                              UWORD32 u4_payload_type,
                              UWORD32 u4_payload_size,
                              WORD8 i1_nal_type)
{
    parse_ctxt_t *ps_parse = &ps_codec->s_parse;
    bitstrm_t *ps_bitstrm = &ps_parse->s_bitstrm;
    WORD32 payload_bits_remaining = 0;
    sps_t *ps_sps;

    UWORD32 i;

    for(i = 0; i < MAX_SPS_CNT; i++)
    {
        ps_sps = ps_codec->ps_sps_base + i;
        if(ps_sps->i1_sps_valid)
        {
            break;
        }
    }
    if(NULL == ps_sps)
    {
        return;
    }

    if(NAL_PREFIX_SEI == i1_nal_type)
    {
        switch(u4_payload_type)
        {
            case SEI_BUFFERING_PERIOD:
                ps_parse->s_sei_params.i1_sei_parameters_present_flag = 1;
                ihevcd_parse_buffering_period_sei(ps_codec, ps_sps);
                break;

            case SEI_PICTURE_TIMING:
                ps_parse->s_sei_params.i1_sei_parameters_present_flag = 1;
                ihevcd_parse_pic_timing_sei(ps_codec, ps_sps);
                break;

            case SEI_TIME_CODE:
                ps_parse->s_sei_params.i1_sei_parameters_present_flag = 1;
                ihevcd_parse_time_code_sei(ps_codec);
                break;

            case SEI_MASTERING_DISPLAY_COLOUR_VOLUME:
                ps_parse->s_sei_params.i4_sei_mastering_disp_colour_vol_params_present_flags = 1;
                ihevcd_parse_mastering_disp_params_sei(ps_codec);
                break;

            case SEI_USER_DATA_REGISTERED_ITU_T_T35:
                ps_parse->s_sei_params.i1_sei_parameters_present_flag = 1;
                if(ps_parse->s_sei_params.i4_sei_user_data_cnt >= USER_DATA_MAX)
                {
                    for(i = 0; i < u4_payload_size / 4; i++)
                    {
                        ihevcd_bits_flush(ps_bitstrm, 4 * 8);
                    }

                    ihevcd_bits_flush(ps_bitstrm, (u4_payload_size - i * 4) * 8);
                }
                else
                {
                    ihevcd_parse_user_data_registered_itu_t_t35(ps_codec,
                                                                u4_payload_size);
                }
                break;

            default:
                for(i = 0; i < u4_payload_size; i++)
                {
                    ihevcd_bits_flush(ps_bitstrm, 8);
                }
                break;
        }
    }
    else /* NAL_SUFFIX_SEI */
    {
        switch(u4_payload_type)
        {
            case SEI_USER_DATA_REGISTERED_ITU_T_T35:
                ps_parse->s_sei_params.i1_sei_parameters_present_flag = 1;
                if(ps_parse->s_sei_params.i4_sei_user_data_cnt >= USER_DATA_MAX)
                {
                    for(i = 0; i < u4_payload_size / 4; i++)
                    {
                        ihevcd_bits_flush(ps_bitstrm, 4 * 8);
                    }

                    ihevcd_bits_flush(ps_bitstrm, (u4_payload_size - i * 4) * 8);
                }
                else
                {
                    ihevcd_parse_user_data_registered_itu_t_t35(ps_codec,
                                                                u4_payload_size);
                }
                break;

            default:
                for(i = 0; i < u4_payload_size; i++)
                {
                    ihevcd_bits_flush(ps_bitstrm, 8);
                }
                break;
        }
    }

    /**
     * By definition the underlying bitstream terminates in a byte-aligned manner.
     * 1. Extract all bar the last MIN(bitsremaining,nine) bits as reserved_payload_extension_data
     * 2. Examine the final 8 bits to determine the payload_bit_equal_to_one marker
     * 3. Extract the remainingreserved_payload_extension_data bits.
     *
     * If there are fewer than 9 bits available, extract them.
     */

    payload_bits_remaining = ihevcd_bits_num_bits_remaining(ps_bitstrm);
    if(payload_bits_remaining) /* more_data_in_payload() */
    {
        WORD32 final_bits;
        WORD32 final_payload_bits = 0;
        WORD32 mask = 0xFF;
        UWORD32 u4_dummy;
        UWORD32 u4_reserved_payload_extension_data;
        UNUSED(u4_dummy);
        UNUSED(u4_reserved_payload_extension_data);

        while(payload_bits_remaining > 9)
        {
            BITS_PARSE("reserved_payload_extension_data",
                       u4_reserved_payload_extension_data, ps_bitstrm, 1);
            payload_bits_remaining--;
        }

        final_bits = ihevcd_bits_nxt(ps_bitstrm, payload_bits_remaining);

        while(final_bits & (mask >> final_payload_bits))
        {
            final_payload_bits++;
            continue;
        }

        while(payload_bits_remaining > (9 - final_payload_bits))
        {
            BITS_PARSE("reserved_payload_extension_data",
                       u4_reserved_payload_extension_data, ps_bitstrm, 1);
            payload_bits_remaining--;
        }

        BITS_PARSE("payload_bit_equal_to_one", u4_dummy, ps_bitstrm, 1);
        payload_bits_remaining--;
        while(payload_bits_remaining)
        {
            BITS_PARSE("payload_bit_equal_to_zero", u4_dummy, ps_bitstrm, 1);
            payload_bits_remaining--;
        }
    }

    return;
}

IHEVCD_ERROR_T ihevcd_read_rbsp_trailing_bits(codec_t *ps_codec,
                                              UWORD32 u4_bits_left)
{
    parse_ctxt_t *ps_parse = &ps_codec->s_parse;
    UWORD32 value;
    WORD32 cnt = 0;
    BITS_PARSE("rbsp_stop_one_bit", value, &ps_parse->s_bitstrm, 1);
    u4_bits_left--;
    if(value != 1)
    {
        return (IHEVCD_ERROR_T)IHEVCD_FAIL;
    }
    while(u4_bits_left)
    {
        BITS_PARSE("rbsp_alignment_zero_bit", value, &ps_parse->s_bitstrm, 1);
        u4_bits_left--;
        cnt++;
    }
    ASSERT(cnt < 8);

    return (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
}
/**
*******************************************************************************
*
* @brief
*  Parses SEI (Supplemental Enhancement Information)
*
* @par Description:
*  Parses SEI (Supplemental Enhancement Information) as per Section: 7.3.7
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_parse_sei(codec_t *ps_codec, nal_header_t *ps_nal)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    parse_ctxt_t *ps_parse = &ps_codec->s_parse;
    UWORD32 u4_payload_type = 0, u4_last_payload_type_byte = 0;
    UWORD32 u4_payload_size = 0, u4_last_payload_size_byte = 0;
    UWORD32 value;
    bitstrm_t *ps_bitstrm = &ps_parse->s_bitstrm;
    UWORD32 u4_bits_left;

    u4_bits_left = ihevcd_bits_num_bits_remaining(ps_bitstrm);

    while(u4_bits_left > 8)
    {
        while(ihevcd_bits_nxt(ps_bitstrm, 8) == 0xFF)
        {
            ihevcd_bits_flush(ps_bitstrm, 8); /* equal to 0xFF */
            u4_payload_type += 255;
        }

        BITS_PARSE("last_payload_type_byte", value, ps_bitstrm, 8);
        u4_last_payload_type_byte = value;

        u4_payload_type += u4_last_payload_type_byte;

        while(ihevcd_bits_nxt(ps_bitstrm, 8) == 0xFF)
        {
            ihevcd_bits_flush(ps_bitstrm, 8); /* equal to 0xFF */
            u4_payload_size += 255;
        }

        BITS_PARSE("last_payload_size_byte", value, ps_bitstrm, 8);
        u4_last_payload_size_byte = value;

        u4_payload_size += u4_last_payload_size_byte;
        u4_bits_left = ihevcd_bits_num_bits_remaining(ps_bitstrm);
        u4_payload_size = MIN(u4_payload_size, u4_bits_left / 8);
        ihevcd_parse_sei_payload(ps_codec, u4_payload_type, u4_payload_size,
                                 ps_nal->i1_nal_unit_type);

        /* Calculate the bits left in the current payload */
        u4_bits_left = ihevcd_bits_num_bits_remaining(ps_bitstrm);
    }

    // read rbsp_trailing_bits
    if(u4_bits_left)
    {
        ihevcd_read_rbsp_trailing_bits(ps_codec, u4_bits_left);
    }

    return ret;
}

/**
*******************************************************************************
*
* @brief
*  Parses Access unit delimiter
*
* @par Description:
*  Parses Access unit delimiter as per section  Section: 7.3.2.5
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_parse_aud(codec_t *ps_codec)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    UNUSED(ps_codec);
    return ret;
}

WORD32 ihevcd_extend_sign_bit(WORD32 value, WORD32 num_bits)
{
    WORD32 ret_value = value;
    if(value >> (num_bits - 1))
    {
        ret_value |= (0xFFFFFFFF << num_bits);
    }
    return ret_value;
}

/**
*******************************************************************************
*
* @brief
*  Calculate POC of the current slice
*
* @par Description:
*  Calculates the current POC using the previous POC lsb and previous POC msb
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @param[in] i1_pic_order_cnt_lsb
*  Current POC lsb
*
* @returns  Current absolute POC
*
* @remarks
*
*
*******************************************************************************
*/

WORD32 ihevcd_calc_poc(codec_t *ps_codec, nal_header_t *ps_nal, WORD8 i1_log2_max_poc_lsb, WORD32 i2_poc_lsb)
{
    WORD32 i4_abs_poc, i4_poc_msb;
    WORD32 max_poc_lsb;
    WORD8 i1_nal_unit_type = ps_nal->i1_nal_unit_type;
    max_poc_lsb = (1 << i1_log2_max_poc_lsb);

    if((!ps_codec->i4_first_pic_done) && (!ps_codec->i4_pic_present))
        ps_codec->i4_prev_poc_msb = -2 * max_poc_lsb;

    if(NAL_IDR_N_LP == i1_nal_unit_type
                    || NAL_IDR_W_LP == i1_nal_unit_type
                    || NAL_BLA_N_LP == i1_nal_unit_type
                    || NAL_BLA_W_DLP == i1_nal_unit_type
                    || NAL_BLA_W_LP == i1_nal_unit_type
                    || (NAL_CRA == i1_nal_unit_type && !ps_codec->i4_first_pic_done))
    {
        i4_poc_msb = ps_codec->i4_prev_poc_msb + 2 * max_poc_lsb;
        ps_codec->i4_prev_poc_lsb = 0;
        ps_codec->i4_max_prev_poc_lsb = 0;
//        ps_codec->i4_prev_poc_msb = 0;
    }
    else
    {

        if((i2_poc_lsb < ps_codec->i4_prev_poc_lsb)
                        && ((ps_codec->i4_prev_poc_lsb - i2_poc_lsb) >= max_poc_lsb / 2))
        {
            i4_poc_msb = ps_codec->i4_prev_poc_msb + max_poc_lsb;
        }
        else if((i2_poc_lsb > ps_codec->i4_prev_poc_lsb)
                        && ((i2_poc_lsb - ps_codec->i4_prev_poc_lsb) > max_poc_lsb / 2))
        {
            i4_poc_msb = ps_codec->i4_prev_poc_msb - max_poc_lsb;
        }
        else
        {
            i4_poc_msb = ps_codec->i4_prev_poc_msb;
        }


    }

    i4_abs_poc = i4_poc_msb + i2_poc_lsb;
    ps_codec->i4_max_prev_poc_lsb = MAX(ps_codec->i4_max_prev_poc_lsb, i2_poc_lsb);

    {
        WORD32 is_reference_nal = ((i1_nal_unit_type <= NAL_RSV_VCL_R15) && (i1_nal_unit_type % 2 != 0)) || ((i1_nal_unit_type >= NAL_BLA_W_LP) && (i1_nal_unit_type <= NAL_RSV_RAP_VCL23));
        WORD32 update_prev_poc = ((is_reference_nal) && ((i1_nal_unit_type < NAL_RADL_N) || (i1_nal_unit_type > NAL_RASL_R)));

        if((0 == ps_nal->i1_nuh_temporal_id) &&
                        (update_prev_poc))
        {
            ps_codec->i4_prev_poc_lsb = i2_poc_lsb;
            ps_codec->i4_prev_poc_msb = i4_poc_msb;
        }
    }

    return i4_abs_poc;
}


void ihevcd_copy_slice_hdr(codec_t *ps_codec, WORD32 slice_idx, WORD32 slice_idx_ref)
{
    slice_header_t *ps_slice_hdr, *ps_slice_hdr_ref;
    WORD32 *pu4_entry_offset_backup;

    ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr_base + slice_idx;
    ps_slice_hdr_ref = ps_codec->s_parse.ps_slice_hdr_base + slice_idx_ref;

    pu4_entry_offset_backup = ps_slice_hdr->pu4_entry_point_offset;
    memcpy(ps_slice_hdr, ps_slice_hdr_ref, sizeof(slice_header_t));
    ps_slice_hdr->pu4_entry_point_offset = pu4_entry_offset_backup;
}



