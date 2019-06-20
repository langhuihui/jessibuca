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
#include "ihevcd_parse_slice_header.h"
#include "ihevcd_ref_list.h"

mv_buf_t* ihevcd_mv_mgr_get_poc(buf_mgr_t *ps_mv_buf_mgr, UWORD32 abs_poc);

/**
*******************************************************************************
*
* @brief
*  Parses VPS operation point
*
* @par   Description
* Parses VPS operation point as per section 7.3.5
*
* @param[out] ps_vps
*  Pointer to VPS structure
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @param[in] ops_idx
*  Operating point index
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_operation_point_set(vps_t *ps_vps, bitstrm_t *ps_bitstrm, WORD32 ops_idx)
{
    WORD32 i;
    WORD32 value;

    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    UNUSED(ops_idx);
    for(i = 0; i <= ps_vps->i1_vps_max_nuh_reserved_zero_layer_id; i++)
    {
        BITS_PARSE("list_entry_l0[ i ]", value, ps_bitstrm, 1);
        //ps_vps->ai1_layer_id_included_flag[ops_idx][i] = value;

    }
    UNUSED(value);

    return ret;
}

/**
*******************************************************************************
*
* @brief
*  Parses pic_lismod_t (picture list mod syntax)  Section:7.3.8.3 Reference
* picture list mod syntax
*
* @par Description:
*  Parse pict list mod synt and update pic_lismod_t struct
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

WORD32 ihevcd_ref_pic_list_modification(bitstrm_t *ps_bitstrm,
                                        slice_header_t *ps_slice_hdr,
                                        WORD32 num_poc_total_curr)
{
    WORD32 ret = IHEVCD_SUCCESS;
    WORD32 value;
    WORD32 i;
    rplm_t *ps_rplm;
    WORD32 num_bits_list_entry;

    ps_rplm = &(ps_slice_hdr->s_rplm);

    /* Calculate Ceil(Log2(num_poc_total_curr)) */
    {
        num_bits_list_entry = 32 - CLZ(num_poc_total_curr);
        /* Check if num_poc_total_curr is power of 2 */
        if(0 == (num_poc_total_curr & (num_poc_total_curr - 1)))
        {
            num_bits_list_entry--;
        }
    }

    if(ps_slice_hdr->i1_slice_type  == PSLICE || ps_slice_hdr->i1_slice_type  == BSLICE)
    {
        BITS_PARSE("ref_pic_list_modification_flag_l0", value, ps_bitstrm, 1);
        ps_rplm->i1_ref_pic_list_modification_flag_l0 = value;

        if(ps_rplm->i1_ref_pic_list_modification_flag_l0)
            for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l0_active; i++)
            {
                BITS_PARSE("list_entry_l0", value, ps_bitstrm, num_bits_list_entry);
                ps_rplm->i1_list_entry_l0[i] = value;

                ps_rplm->i1_list_entry_l0[i] = CLIP3(ps_rplm->i1_list_entry_l0[i], 0, num_poc_total_curr - 1);
            }
    }

    if(ps_slice_hdr->i1_slice_type  == BSLICE)
    {
        BITS_PARSE("ref_pic_list_modification_flag_l1", value, ps_bitstrm, 1);
        ps_rplm->i1_ref_pic_list_modification_flag_l1 = value;

        if(ps_rplm->i1_ref_pic_list_modification_flag_l1)
            for(i = 0; i < ps_slice_hdr->i1_num_ref_idx_l1_active; i++)
            {
                BITS_PARSE("list_entry_l1", value, ps_bitstrm, num_bits_list_entry);
                ps_rplm->i1_list_entry_l1[i] = value;

                ps_rplm->i1_list_entry_l1[i] = CLIP3(ps_rplm->i1_list_entry_l1[i], 0, num_poc_total_curr - 1);
            }

    }

    return ret;
}

/**
*******************************************************************************
*
* @brief
*  Parse Slice Header
* slice_header_syntax()
*
* @par Description:
*  Parse Slice Header as per  Section: 7.3.8
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

IHEVCD_ERROR_T ihevcd_parse_slice_header(codec_t *ps_codec,
                                         nal_header_t *ps_nal)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 value;
    WORD32 i, j;
    WORD32 sps_id;

    pps_t *ps_pps;
    sps_t *ps_sps;
    slice_header_t *ps_slice_hdr;
    WORD32 disable_deblocking_filter_flag;
    bitstrm_t *ps_bitstrm = &ps_codec->s_parse.s_bitstrm;
    WORD32 idr_pic_flag;
    WORD32 pps_id;
    WORD32 first_slice_in_pic_flag;
    WORD32 no_output_of_prior_pics_flag = 0;
    WORD8 i1_nal_unit_type = ps_nal->i1_nal_unit_type;
    WORD32 num_poc_total_curr = 0;
    WORD32 slice_address;
    WORD32 prev_slice_incomplete_flag = 0;

    if(ps_codec->i4_slice_error == 1)
        return ret;

    idr_pic_flag = (NAL_IDR_W_LP == i1_nal_unit_type) ||
                    (NAL_IDR_N_LP == i1_nal_unit_type);


    BITS_PARSE("first_slice_in_pic_flag", first_slice_in_pic_flag, ps_bitstrm, 1);
    if((NAL_BLA_W_LP <= i1_nal_unit_type) &&
       (NAL_RSV_RAP_VCL23          >= i1_nal_unit_type))
    {
        BITS_PARSE("no_output_of_prior_pics_flag", no_output_of_prior_pics_flag, ps_bitstrm, 1);
    }
    UEV_PARSE("pic_parameter_set_id", pps_id, ps_bitstrm);
    pps_id = CLIP3(pps_id, 0, MAX_PPS_CNT - 2);

    /* Get the current PPS structure */
    ps_pps = ps_codec->s_parse.ps_pps_base + pps_id;
    if(0 == ps_pps->i1_pps_valid)
    {
        pps_t *ps_pps_ref = ps_codec->ps_pps_base;
        while(0 == ps_pps_ref->i1_pps_valid)
        {
            ps_pps_ref++;
            if((ps_pps_ref - ps_codec->ps_pps_base >= MAX_PPS_CNT - 1))
                return IHEVCD_INVALID_HEADER;
        }

        ihevcd_copy_pps(ps_codec, pps_id, ps_pps_ref->i1_pps_id);
    }

    /* Get SPS id for the current PPS */
    sps_id = ps_pps->i1_sps_id;

    /* Get the current SPS structure */
    ps_sps = ps_codec->s_parse.ps_sps_base + sps_id;

    /* When the current slice is the first in a pic,
     *  check whether the previous frame is complete
     *  If the previous frame is incomplete -
     *  treat the remaining CTBs as skip */
    if((0 != ps_codec->u4_pic_cnt || ps_codec->i4_pic_present) &&
                    first_slice_in_pic_flag)
    {
        if(ps_codec->i4_pic_present)
        {
            slice_header_t *ps_slice_hdr_next;
            ps_codec->i4_slice_error = 1;
            ps_codec->s_parse.i4_cur_slice_idx--;
            if(ps_codec->s_parse.i4_cur_slice_idx < 0)
                ps_codec->s_parse.i4_cur_slice_idx = 0;

            ps_slice_hdr_next = ps_codec->s_parse.ps_slice_hdr_base + ((ps_codec->s_parse.i4_cur_slice_idx + 1) & (MAX_SLICE_HDR_CNT - 1));
            ps_slice_hdr_next->i2_ctb_x = 0;
            ps_slice_hdr_next->i2_ctb_y = ps_codec->s_parse.ps_sps->i2_pic_ht_in_ctb;
            return ret;
        }
        else
        {
            ps_codec->i4_slice_error = 0;
        }
    }

    if(first_slice_in_pic_flag)
    {
        ps_codec->s_parse.i4_cur_slice_idx = 0;
    }
    else
    {
        /* If the current slice is not the first slice in the pic,
         * but the first one to be parsed, set the current slice indx to 1
         * Treat the first slice to be missing and copy the current slice header
         * to the first one */
        if(0 == ps_codec->i4_pic_present)
            ps_codec->s_parse.i4_cur_slice_idx = 1;
    }

    ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr_base + (ps_codec->s_parse.i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1));


    if((ps_pps->i1_dependent_slice_enabled_flag) &&
       (!first_slice_in_pic_flag))
    {
        BITS_PARSE("dependent_slice_flag", value, ps_bitstrm, 1);

        /* If dependendent slice, copy slice header from previous slice */
        if(value && (ps_codec->s_parse.i4_cur_slice_idx > 0))
        {
            ihevcd_copy_slice_hdr(ps_codec,
                                  (ps_codec->s_parse.i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1)),
                                  ((ps_codec->s_parse.i4_cur_slice_idx - 1) & (MAX_SLICE_HDR_CNT - 1)));
        }
        ps_slice_hdr->i1_dependent_slice_flag = value;
    }
    else
    {
        ps_slice_hdr->i1_dependent_slice_flag = 0;
    }
    ps_slice_hdr->i1_nal_unit_type = i1_nal_unit_type;
    ps_slice_hdr->i1_pps_id = pps_id;
    ps_slice_hdr->i1_first_slice_in_pic_flag = first_slice_in_pic_flag;

    ps_slice_hdr->i1_no_output_of_prior_pics_flag = 1;
    if((NAL_BLA_W_LP <= i1_nal_unit_type) &&
                    (NAL_RSV_RAP_VCL23          >= i1_nal_unit_type))
    {
        ps_slice_hdr->i1_no_output_of_prior_pics_flag = no_output_of_prior_pics_flag;
    }
    ps_slice_hdr->i1_pps_id = pps_id;

    if(!ps_slice_hdr->i1_first_slice_in_pic_flag)
    {
        WORD32 num_bits;

        /* Use CLZ to compute Ceil( Log2( PicSizeInCtbsY ) ) */
        num_bits = 32 - CLZ(ps_sps->i4_pic_size_in_ctb - 1);
        BITS_PARSE("slice_address", value, ps_bitstrm, num_bits);

        slice_address = value;
        /* If slice address is greater than the number of CTBs in a picture,
         * ignore the slice */
        if(value >= ps_sps->i4_pic_size_in_ctb || value <= 0)
            return IHEVCD_IGNORE_SLICE;
    }
    else
    {
        slice_address = 0;
    }

    if(!ps_slice_hdr->i1_dependent_slice_flag)
    {
        ps_slice_hdr->i1_pic_output_flag = 1;
        ps_slice_hdr->i4_pic_order_cnt_lsb = 0;
        ps_slice_hdr->i1_num_long_term_sps = 0;
        ps_slice_hdr->i1_num_long_term_pics = 0;

        for(i = 0; i < ps_pps->i1_num_extra_slice_header_bits; i++)
        {
            BITS_PARSE("slice_reserved_undetermined_flag[ i ]", value, ps_bitstrm, 1);
            //slice_reserved_undetermined_flag[ i ]
        }
        UEV_PARSE("slice_type", value, ps_bitstrm);
        ps_slice_hdr->i1_slice_type = value;

        /* If the picture is IRAP, slice type must be equal to ISLICE */
        if((ps_slice_hdr->i1_nal_unit_type >= NAL_BLA_W_LP) &&
                        (ps_slice_hdr->i1_nal_unit_type <= NAL_RSV_RAP_VCL23))
            ps_slice_hdr->i1_slice_type = ISLICE;

        if((ps_slice_hdr->i1_slice_type < 0) ||
                        (ps_slice_hdr->i1_slice_type > 2))
            return IHEVCD_IGNORE_SLICE;

        if(ps_pps->i1_output_flag_present_flag)
        {
            BITS_PARSE("pic_output_flag", value, ps_bitstrm, 1);
            ps_slice_hdr->i1_pic_output_flag = value;
        }
        ps_slice_hdr->i1_colour_plane_id = 0;
        if(1 == ps_sps->i1_separate_colour_plane_flag)
        {
            BITS_PARSE("colour_plane_id", value, ps_bitstrm, 2);
            ps_slice_hdr->i1_colour_plane_id = value;
        }
        ps_slice_hdr->i1_slice_temporal_mvp_enable_flag = 0;

        if(!idr_pic_flag)
        {

            WORD32 st_rps_idx;
            WORD32 num_neg_pics;
            WORD32 num_pos_pics;
            WORD8 *pi1_used;

            BITS_PARSE("pic_order_cnt_lsb", value, ps_bitstrm, ps_sps->i1_log2_max_pic_order_cnt_lsb);
            //value = ihevcd_extend_sign_bit(value, ps_sps->i1_log2_max_pic_order_cnt_lsb);
            ps_slice_hdr->i4_pic_order_cnt_lsb = value;

            BITS_PARSE("short_term_ref_pic_set_sps_flag", value, ps_bitstrm, 1);
            ps_slice_hdr->i1_short_term_ref_pic_set_sps_flag = value;

            if(1 == ps_slice_hdr->i1_short_term_ref_pic_set_sps_flag)
            {
                WORD32 numbits;

                ps_slice_hdr->i1_short_term_ref_pic_set_idx = 0;
                if(ps_sps->i1_num_short_term_ref_pic_sets > 1)
                {
                    numbits = 32 - CLZ(ps_sps->i1_num_short_term_ref_pic_sets - 1);
                    BITS_PARSE("short_term_ref_pic_set_idx", value, ps_bitstrm, numbits);
                    ps_slice_hdr->i1_short_term_ref_pic_set_idx = value;
                }

                st_rps_idx = ps_slice_hdr->i1_short_term_ref_pic_set_idx;
                num_neg_pics = ps_sps->as_stref_picset[st_rps_idx].i1_num_neg_pics;
                num_pos_pics = ps_sps->as_stref_picset[st_rps_idx].i1_num_pos_pics;
                pi1_used = ps_sps->as_stref_picset[st_rps_idx].ai1_used;
            }
            else
            {
                ihevcd_short_term_ref_pic_set(ps_bitstrm,
                                              &ps_sps->as_stref_picset[0],
                                              ps_sps->i1_num_short_term_ref_pic_sets,
                                              ps_sps->i1_num_short_term_ref_pic_sets,
                                              &ps_slice_hdr->s_stref_picset);

                st_rps_idx = ps_sps->i1_num_short_term_ref_pic_sets;
                num_neg_pics = ps_slice_hdr->s_stref_picset.i1_num_neg_pics;
                num_pos_pics = ps_slice_hdr->s_stref_picset.i1_num_pos_pics;
                pi1_used = ps_slice_hdr->s_stref_picset.ai1_used;
            }

            if(ps_sps->i1_long_term_ref_pics_present_flag)
            {
                if(ps_sps->i1_num_long_term_ref_pics_sps > 0)
                {
                    UEV_PARSE("num_long_term_sps", value, ps_bitstrm);
                    ps_slice_hdr->i1_num_long_term_sps = value;

                    ps_slice_hdr->i1_num_long_term_sps = CLIP3(ps_slice_hdr->i1_num_long_term_sps,
                                                               0, MAX_DPB_SIZE - num_neg_pics - num_pos_pics);
                }
                UEV_PARSE("num_long_term_pics", value, ps_bitstrm);
                ps_slice_hdr->i1_num_long_term_pics = value;
                ps_slice_hdr->i1_num_long_term_pics = CLIP3(ps_slice_hdr->i1_num_long_term_pics,
                                                            0, MAX_DPB_SIZE - num_neg_pics - num_pos_pics -
                                                            ps_slice_hdr->i1_num_long_term_sps);

                for(i = 0; i < (ps_slice_hdr->i1_num_long_term_sps +
                                ps_slice_hdr->i1_num_long_term_pics); i++)
                {
                    if(i < ps_slice_hdr->i1_num_long_term_sps)
                    {
                        /* Use CLZ to compute Ceil( Log2( num_long_term_ref_pics_sps ) ) */
                        if (ps_sps->i1_num_long_term_ref_pics_sps > 1)
                        {
                            WORD32 num_bits = 32 - CLZ(ps_sps->i1_num_long_term_ref_pics_sps - 1);
                            BITS_PARSE("lt_idx_sps[ i ]", value, ps_bitstrm, num_bits);
                        }
                        else
                        {
                            value = 0;
                        }
                        ps_slice_hdr->ai4_poc_lsb_lt[i] = ps_sps->au2_lt_ref_pic_poc_lsb_sps[value];
                        ps_slice_hdr->ai1_used_by_curr_pic_lt_flag[i] = ps_sps->ai1_used_by_curr_pic_lt_sps_flag[value];

                    }
                    else
                    {
                        BITS_PARSE("poc_lsb_lt[ i ]", value, ps_bitstrm, ps_sps->i1_log2_max_pic_order_cnt_lsb);
                        ps_slice_hdr->ai4_poc_lsb_lt[i] = value;

                        BITS_PARSE("used_by_curr_pic_lt_flag[ i ]", value, ps_bitstrm, 1);
                        ps_slice_hdr->ai1_used_by_curr_pic_lt_flag[i] = value;

                    }
                    BITS_PARSE("delta_poc_msb_present_flag[ i ]", value, ps_bitstrm, 1);
                    ps_slice_hdr->ai1_delta_poc_msb_present_flag[i] = value;


                    ps_slice_hdr->ai1_delta_poc_msb_cycle_lt[i] = 0;
                    if(ps_slice_hdr->ai1_delta_poc_msb_present_flag[i])
                    {

                        UEV_PARSE("delata_poc_msb_cycle_lt[ i ]", value, ps_bitstrm);
                        ps_slice_hdr->ai1_delta_poc_msb_cycle_lt[i] = value;
                    }

                    if((i != 0) && (i != ps_slice_hdr->i1_num_long_term_sps))
                    {
                        ps_slice_hdr->ai1_delta_poc_msb_cycle_lt[i] += ps_slice_hdr->ai1_delta_poc_msb_cycle_lt[i - 1];
                    }

                }
            }

            for(i = 0; i < num_neg_pics + num_pos_pics; i++)
            {
                if(pi1_used[i])
                {
                    num_poc_total_curr++;
                }
            }
            for(i = 0; i < ps_slice_hdr->i1_num_long_term_sps + ps_slice_hdr->i1_num_long_term_pics; i++)
            {
                if(ps_slice_hdr->ai1_used_by_curr_pic_lt_flag[i])
                {
                    num_poc_total_curr++;
                }
            }


            if(ps_sps->i1_sps_temporal_mvp_enable_flag)
            {
                BITS_PARSE("enable_temporal_mvp_flag", value, ps_bitstrm, 1);
                ps_slice_hdr->i1_slice_temporal_mvp_enable_flag = value;
            }

        }
        ps_slice_hdr->i1_slice_sao_luma_flag = 0;
        ps_slice_hdr->i1_slice_sao_chroma_flag = 0;
        if(ps_sps->i1_sample_adaptive_offset_enabled_flag)
        {
            BITS_PARSE("slice_sao_luma_flag", value, ps_bitstrm, 1);
            ps_slice_hdr->i1_slice_sao_luma_flag = value;

            BITS_PARSE("slice_sao_chroma_flag", value, ps_bitstrm, 1);
            ps_slice_hdr->i1_slice_sao_chroma_flag = value;

        }

        ps_slice_hdr->i1_max_num_merge_cand = 1;
        ps_slice_hdr->i1_cabac_init_flag = 0;

        ps_slice_hdr->i1_num_ref_idx_l0_active = 0;
        ps_slice_hdr->i1_num_ref_idx_l1_active = 0;
        ps_slice_hdr->i1_slice_cb_qp_offset = 0;
        ps_slice_hdr->i1_slice_cr_qp_offset = 0;
        if((PSLICE == ps_slice_hdr->i1_slice_type) ||
           (BSLICE == ps_slice_hdr->i1_slice_type))
        {
            BITS_PARSE("num_ref_idx_active_override_flag", value, ps_bitstrm, 1);
            ps_slice_hdr->i1_num_ref_idx_active_override_flag = value;

            if(ps_slice_hdr->i1_num_ref_idx_active_override_flag)
            {
                UEV_PARSE("num_ref_idx_l0_active_minus1", value, ps_bitstrm);
                ps_slice_hdr->i1_num_ref_idx_l0_active = value + 1;

                if(BSLICE == ps_slice_hdr->i1_slice_type)
                {
                    UEV_PARSE("num_ref_idx_l1_active_minus1", value, ps_bitstrm);
                    ps_slice_hdr->i1_num_ref_idx_l1_active = value + 1;
                }

            }
            else
            {
                ps_slice_hdr->i1_num_ref_idx_l0_active = ps_pps->i1_num_ref_idx_l0_default_active;

                if(BSLICE == ps_slice_hdr->i1_slice_type)
                {
                    ps_slice_hdr->i1_num_ref_idx_l1_active = ps_pps->i1_num_ref_idx_l1_default_active;
                }
            }

            ps_slice_hdr->i1_num_ref_idx_l0_active = CLIP3(ps_slice_hdr->i1_num_ref_idx_l0_active, 0, MAX_DPB_SIZE - 1);
            ps_slice_hdr->i1_num_ref_idx_l1_active = CLIP3(ps_slice_hdr->i1_num_ref_idx_l1_active, 0, MAX_DPB_SIZE - 1);

            if(0 == num_poc_total_curr)
                return IHEVCD_IGNORE_SLICE;
            if((ps_pps->i1_lists_modification_present_flag) && (num_poc_total_curr > 1))
            {
                ihevcd_ref_pic_list_modification(ps_bitstrm,
                                                 ps_slice_hdr, num_poc_total_curr);
            }
            else
            {
                ps_slice_hdr->s_rplm.i1_ref_pic_list_modification_flag_l0 = 0;
                ps_slice_hdr->s_rplm.i1_ref_pic_list_modification_flag_l1 = 0;
            }

            if(BSLICE == ps_slice_hdr->i1_slice_type)
            {
                BITS_PARSE("mvd_l1_zero_flag", value, ps_bitstrm, 1);
                ps_slice_hdr->i1_mvd_l1_zero_flag = value;
            }

            ps_slice_hdr->i1_cabac_init_flag = 0;
            if(ps_pps->i1_cabac_init_present_flag)
            {
                BITS_PARSE("cabac_init_flag", value, ps_bitstrm, 1);
                ps_slice_hdr->i1_cabac_init_flag = value;

            }
            ps_slice_hdr->i1_collocated_from_l0_flag = 1;
            ps_slice_hdr->i1_collocated_ref_idx = 0;
            if(ps_slice_hdr->i1_slice_temporal_mvp_enable_flag)
            {
                if(BSLICE == ps_slice_hdr->i1_slice_type)
                {
                    BITS_PARSE("collocated_from_l0_flag", value, ps_bitstrm, 1);
                    ps_slice_hdr->i1_collocated_from_l0_flag = value;
                }

                if((ps_slice_hdr->i1_collocated_from_l0_flag  &&  (ps_slice_hdr->i1_num_ref_idx_l0_active > 1)) ||
                   (!ps_slice_hdr->i1_collocated_from_l0_flag  && (ps_slice_hdr->i1_num_ref_idx_l1_active > 1)))
                {
                    UEV_PARSE("collocated_ref_idx", value, ps_bitstrm);
                    ps_slice_hdr->i1_collocated_ref_idx = value;
                }

            }
            ps_slice_hdr->i1_collocated_ref_idx = CLIP3(ps_slice_hdr->i1_collocated_ref_idx, 0, MAX_DPB_SIZE - 1);

            if((ps_pps->i1_weighted_pred_flag  &&   (PSLICE == ps_slice_hdr->i1_slice_type)) ||
               (ps_pps->i1_weighted_bipred_flag  &&  (BSLICE == ps_slice_hdr->i1_slice_type)))
            {
                ihevcd_parse_pred_wt_ofst(ps_bitstrm, ps_sps, ps_pps, ps_slice_hdr);
            }
            UEV_PARSE("five_minus_max_num_merge_cand", value, ps_bitstrm);
            ps_slice_hdr->i1_max_num_merge_cand = 5 - value;

        }
        ps_slice_hdr->i1_max_num_merge_cand = CLIP3(ps_slice_hdr->i1_max_num_merge_cand, 1, 5);
        SEV_PARSE("slice_qp_delta", value, ps_bitstrm);
        ps_slice_hdr->i1_slice_qp_delta = value;

        if(ps_pps->i1_pic_slice_level_chroma_qp_offsets_present_flag)
        {
            SEV_PARSE("slice_cb_qp_offset", value, ps_bitstrm);
            ps_slice_hdr->i1_slice_cb_qp_offset = value;

            SEV_PARSE("slice_cr_qp_offset", value, ps_bitstrm);
            ps_slice_hdr->i1_slice_cr_qp_offset = value;

        }
        ps_slice_hdr->i1_deblocking_filter_override_flag = 0;
        ps_slice_hdr->i1_slice_disable_deblocking_filter_flag  = ps_pps->i1_pic_disable_deblocking_filter_flag;
        ps_slice_hdr->i1_beta_offset_div2 = ps_pps->i1_beta_offset_div2;
        ps_slice_hdr->i1_tc_offset_div2 = ps_pps->i1_tc_offset_div2;

        disable_deblocking_filter_flag = ps_pps->i1_pic_disable_deblocking_filter_flag;

        if(ps_pps->i1_deblocking_filter_control_present_flag)
        {

            if(ps_pps->i1_deblocking_filter_override_enabled_flag)
            {
                BITS_PARSE("deblocking_filter_override_flag", value, ps_bitstrm, 1);
                ps_slice_hdr->i1_deblocking_filter_override_flag = value;
            }

            if(ps_slice_hdr->i1_deblocking_filter_override_flag)
            {
                BITS_PARSE("slice_disable_deblocking_filter_flag", value, ps_bitstrm, 1);
                ps_slice_hdr->i1_slice_disable_deblocking_filter_flag = value;
                disable_deblocking_filter_flag = ps_slice_hdr->i1_slice_disable_deblocking_filter_flag;

                if(!ps_slice_hdr->i1_slice_disable_deblocking_filter_flag)
                {
                    SEV_PARSE("beta_offset_div2", value, ps_bitstrm);
                    ps_slice_hdr->i1_beta_offset_div2 = value;

                    SEV_PARSE("tc_offset_div2", value, ps_bitstrm);
                    ps_slice_hdr->i1_tc_offset_div2 = value;

                }
            }
        }

        ps_slice_hdr->i1_slice_loop_filter_across_slices_enabled_flag = ps_pps->i1_loop_filter_across_slices_enabled_flag;
        if(ps_pps->i1_loop_filter_across_slices_enabled_flag  &&
                        (ps_slice_hdr->i1_slice_sao_luma_flag  ||  ps_slice_hdr->i1_slice_sao_chroma_flag  || !disable_deblocking_filter_flag))
        {
            BITS_PARSE("slice_loop_filter_across_slices_enabled_flag", value, ps_bitstrm, 1);
            ps_slice_hdr->i1_slice_loop_filter_across_slices_enabled_flag = value;
        }

    }

    /* Check sanity of slice */
    if((!first_slice_in_pic_flag) &&
                    (ps_codec->i4_pic_present))
    {
        slice_header_t *ps_slice_hdr_base = ps_codec->ps_slice_hdr_base;


        /* According to the standard, the above conditions must be satisfied - But for error resilience,
         * only the following conditions are checked */
        if((ps_slice_hdr_base->i1_pps_id != ps_slice_hdr->i1_pps_id) ||
                        (ps_slice_hdr_base->i4_pic_order_cnt_lsb != ps_slice_hdr->i4_pic_order_cnt_lsb))
        {
            return IHEVCD_IGNORE_SLICE;
        }

    }


    if(0 == ps_codec->i4_pic_present)
    {
        ps_slice_hdr->i4_abs_pic_order_cnt = ihevcd_calc_poc(ps_codec, ps_nal, ps_sps->i1_log2_max_pic_order_cnt_lsb, ps_slice_hdr->i4_pic_order_cnt_lsb);
    }
    else
    {
        ps_slice_hdr->i4_abs_pic_order_cnt = ps_codec->s_parse.i4_abs_pic_order_cnt;
    }


    if(!first_slice_in_pic_flag)
    {
        /* Check if the current slice belongs to the same pic (Pic being parsed) */
        if(ps_codec->s_parse.i4_abs_pic_order_cnt == ps_slice_hdr->i4_abs_pic_order_cnt)
        {

            /* If the Next CTB's index is less than the slice address,
             * the previous slice is incomplete.
             * Indicate slice error, and treat the remaining CTBs as skip */
            if(slice_address > ps_codec->s_parse.i4_next_ctb_indx)
            {
                if(ps_codec->i4_pic_present)
                {
                    prev_slice_incomplete_flag = 1;
                }
                else
                {
                    return IHEVCD_IGNORE_SLICE;
                }
            }
            /* If the slice address is less than the next CTB's index,
             * extra CTBs have been decoded in the previous slice.
             * Ignore the current slice. Treat it as incomplete */
            else if(slice_address < ps_codec->s_parse.i4_next_ctb_indx)
            {
                return IHEVCD_IGNORE_SLICE;
            }
            else
            {
                ps_codec->i4_slice_error = 0;
            }
        }

        /* The current slice does not belong to the pic that is being parsed */
        else
        {
            /* The previous pic is incomplete.
             * Treat the remaining CTBs as skip */
            if(ps_codec->i4_pic_present)
            {
                slice_header_t *ps_slice_hdr_next;
                ps_codec->i4_slice_error = 1;
                ps_codec->s_parse.i4_cur_slice_idx--;
                if(ps_codec->s_parse.i4_cur_slice_idx < 0)
                    ps_codec->s_parse.i4_cur_slice_idx = 0;

                ps_slice_hdr_next = ps_codec->s_parse.ps_slice_hdr_base + ((ps_codec->s_parse.i4_cur_slice_idx + 1) & (MAX_SLICE_HDR_CNT - 1));
                ps_slice_hdr_next->i2_ctb_x = 0;
                ps_slice_hdr_next->i2_ctb_y = ps_codec->s_parse.ps_sps->i2_pic_ht_in_ctb;
                return ret;
            }

            /* If the previous pic is complete,
             * return if the current slice is dependant
             * otherwise, update the parse context's POC */
            else
            {
                if(ps_slice_hdr->i1_dependent_slice_flag)
                    return IHEVCD_IGNORE_SLICE;

                ps_codec->s_parse.i4_abs_pic_order_cnt = ps_slice_hdr->i4_abs_pic_order_cnt;
            }
        }
    }

    /* If the slice is the first slice in the pic, update the parse context's POC */
    else
    {
        /* If the first slice is repeated, ignore the second occurrence
         * If any other slice is repeated, the CTB addr will be greater than the slice addr,
         * and hence the second occurrence is ignored */
        if(ps_codec->s_parse.i4_abs_pic_order_cnt == ps_slice_hdr->i4_abs_pic_order_cnt)
            return IHEVCD_IGNORE_SLICE;

        ps_codec->s_parse.i4_abs_pic_order_cnt = ps_slice_hdr->i4_abs_pic_order_cnt;
    }

    // printf("POC: %d\n", ps_slice_hdr->i4_abs_pic_order_cnt);
    // AEV_TRACE("POC", ps_slice_hdr->i4_abs_pic_order_cnt, 0);
    ps_slice_hdr->i4_num_entry_point_offsets = 0;
    if((ps_pps->i1_tiles_enabled_flag) ||
       (ps_pps->i1_entropy_coding_sync_enabled_flag))
    {
        UEV_PARSE("num_entry_point_offsets", value, ps_bitstrm);
        ps_slice_hdr->i4_num_entry_point_offsets = value;

        {
            WORD32 max_num_entry_point_offsets;
            if((ps_pps->i1_tiles_enabled_flag) &&
                            (ps_pps->i1_entropy_coding_sync_enabled_flag))
            {
                max_num_entry_point_offsets = ps_pps->i1_num_tile_columns * (ps_sps->i2_pic_ht_in_ctb - 1);
            }
            else if(ps_pps->i1_tiles_enabled_flag)
            {
                max_num_entry_point_offsets = ps_pps->i1_num_tile_columns * ps_pps->i1_num_tile_rows;
            }
            else
            {
                max_num_entry_point_offsets = (ps_sps->i2_pic_ht_in_ctb - 1);
            }

            ps_slice_hdr->i4_num_entry_point_offsets = CLIP3(ps_slice_hdr->i4_num_entry_point_offsets,
                                                             0, max_num_entry_point_offsets);
        }

        if(ps_slice_hdr->i4_num_entry_point_offsets > 0)
        {
            UEV_PARSE("offset_len_minus1", value, ps_bitstrm);
            ps_slice_hdr->i1_offset_len = value + 1;

            for(i = 0; i < ps_slice_hdr->i4_num_entry_point_offsets; i++)
            {
                BITS_PARSE("entry_point_offset", value, ps_bitstrm, ps_slice_hdr->i1_offset_len);

                /* TODO: pu4_entry_point_offset needs to be initialized */
                //ps_slice_hdr->pu4_entry_point_offset[i] = value;
            }

        }
    }

    if(ps_pps->i1_slice_header_extension_present_flag)
    {
        UEV_PARSE("slice_header_extension_length", value, ps_bitstrm);
        ps_slice_hdr->i2_slice_header_extension_length = value;


        for(i = 0; i < ps_slice_hdr->i2_slice_header_extension_length; i++)
        {
            BITS_PARSE("slice_header_extension_data_byte", value, ps_bitstrm, 8);
        }

    }

    ihevcd_bits_flush_to_byte_boundary(ps_bitstrm);

    if((UWORD8 *)ps_bitstrm->pu4_buf > ps_bitstrm->pu1_buf_max)
        return IHEVCD_INVALID_PARAMETER;

    {
        dpb_mgr_t *ps_dpb_mgr = (dpb_mgr_t *)ps_codec->pv_dpb_mgr;
        WORD32 r_idx;

        if((NAL_IDR_W_LP == ps_slice_hdr->i1_nal_unit_type) ||
                        (NAL_IDR_N_LP == ps_slice_hdr->i1_nal_unit_type)  ||
                        (NAL_BLA_N_LP == ps_slice_hdr->i1_nal_unit_type)  ||
                        (NAL_BLA_W_DLP == ps_slice_hdr->i1_nal_unit_type) ||
                        (NAL_BLA_W_LP == ps_slice_hdr->i1_nal_unit_type)  ||
                        (0 == ps_codec->u4_pic_cnt))
        {
            for(i = 0; i < MAX_DPB_BUFS; i++)
            {
                if(ps_dpb_mgr->as_dpb_info[i].ps_pic_buf)
                {
                    pic_buf_t *ps_pic_buf = ps_dpb_mgr->as_dpb_info[i].ps_pic_buf;
                    mv_buf_t *ps_mv_buf;

                    /* Long term index is set to MAX_DPB_BUFS to ensure it is not added as LT */
                    ihevc_dpb_mgr_del_ref((dpb_mgr_t *)ps_codec->pv_dpb_mgr, (buf_mgr_t *)ps_codec->pv_pic_buf_mgr, ps_pic_buf->i4_abs_poc);
                    /* Find buffer id of the MV bank corresponding to the buffer being freed (Buffer with POC of u4_abs_poc) */
                    ps_mv_buf = (mv_buf_t *)ps_codec->ps_mv_buf;
                    for(j = 0; j < ps_codec->i4_max_dpb_size; j++)
                    {
                        if(ps_mv_buf && ps_mv_buf->i4_abs_poc == ps_pic_buf->i4_abs_poc)
                        {
                            ihevc_buf_mgr_release((buf_mgr_t *)ps_codec->pv_mv_buf_mgr, j, BUF_MGR_REF);
                            break;
                        }
                        ps_mv_buf++;
                    }

                }

            }

            /* Initialize the reference lists to NULL
             * This is done to take care of the cases where the first pic is not IDR
             * but the reference list is not created for the first pic because
             * pic count is zero leaving the reference list uninitialised  */
            for(r_idx = 0; r_idx < MAX_DPB_SIZE; r_idx++)
            {
                ps_slice_hdr->as_ref_pic_list0[r_idx].pv_pic_buf = NULL;
                ps_slice_hdr->as_ref_pic_list0[r_idx].pv_mv_buf = NULL;

                ps_slice_hdr->as_ref_pic_list1[r_idx].pv_pic_buf = NULL;
                ps_slice_hdr->as_ref_pic_list1[r_idx].pv_mv_buf = NULL;
            }

        }
        else
        {
            ret = ihevcd_ref_list(ps_codec, ps_pps, ps_sps, ps_slice_hdr);
            if ((WORD32)IHEVCD_SUCCESS != ret)
            {
                return ret;
            }

        }

    }

    /* Fill the remaining entries of the reference lists with the nearest POC
     * This is done to handle cases where there is a corruption in the reference index */
    if(ps_codec->i4_pic_present)
    {
        pic_buf_t *ps_pic_buf_ref;
        mv_buf_t *ps_mv_buf_ref;
        WORD32 r_idx;
        dpb_mgr_t *ps_dpb_mgr = (dpb_mgr_t *)ps_codec->pv_dpb_mgr;
        buf_mgr_t *ps_mv_buf_mgr = (buf_mgr_t *)ps_codec->pv_mv_buf_mgr;

        ps_pic_buf_ref = ihevc_dpb_mgr_get_ref_by_nearest_poc(ps_dpb_mgr, ps_slice_hdr->i4_abs_pic_order_cnt);
        if(NULL == ps_pic_buf_ref)
        {
            ps_pic_buf_ref = ps_codec->as_process[0].ps_cur_pic;
            ps_mv_buf_ref = ps_codec->s_parse.ps_cur_mv_buf;
        }
        else
        {
            ps_mv_buf_ref = ihevcd_mv_mgr_get_poc(ps_mv_buf_mgr, ps_pic_buf_ref->i4_abs_poc);
        }

        for(r_idx = 0; r_idx < ps_slice_hdr->i1_num_ref_idx_l0_active; r_idx++)
        {
            if(NULL == ps_slice_hdr->as_ref_pic_list0[r_idx].pv_pic_buf)
            {
                ps_slice_hdr->as_ref_pic_list0[r_idx].pv_pic_buf = (void *)ps_pic_buf_ref;
                ps_slice_hdr->as_ref_pic_list0[r_idx].pv_mv_buf = (void *)ps_mv_buf_ref;
            }
        }

        for(r_idx = ps_slice_hdr->i1_num_ref_idx_l0_active; r_idx < MAX_DPB_SIZE; r_idx++)
        {
            ps_slice_hdr->as_ref_pic_list0[r_idx].pv_pic_buf = (void *)ps_pic_buf_ref;
            ps_slice_hdr->as_ref_pic_list0[r_idx].pv_mv_buf = (void *)ps_mv_buf_ref;
        }

        for(r_idx = 0; r_idx < ps_slice_hdr->i1_num_ref_idx_l1_active; r_idx++)
        {
            if(NULL == ps_slice_hdr->as_ref_pic_list1[r_idx].pv_pic_buf)
            {
                ps_slice_hdr->as_ref_pic_list1[r_idx].pv_pic_buf = (void *)ps_pic_buf_ref;
                ps_slice_hdr->as_ref_pic_list1[r_idx].pv_mv_buf = (void *)ps_mv_buf_ref;
            }
        }

        for(r_idx = ps_slice_hdr->i1_num_ref_idx_l1_active; r_idx < MAX_DPB_SIZE; r_idx++)
        {
            ps_slice_hdr->as_ref_pic_list1[r_idx].pv_pic_buf = (void *)ps_pic_buf_ref;
            ps_slice_hdr->as_ref_pic_list1[r_idx].pv_mv_buf = (void *)ps_mv_buf_ref;
        }
    }

    /* Update slice address in the header */
    if(!ps_slice_hdr->i1_first_slice_in_pic_flag)
    {
        ps_slice_hdr->i2_ctb_x = slice_address % ps_sps->i2_pic_wd_in_ctb;
        ps_slice_hdr->i2_ctb_y = slice_address / ps_sps->i2_pic_wd_in_ctb;

        if(!ps_slice_hdr->i1_dependent_slice_flag)
        {
            ps_slice_hdr->i2_independent_ctb_x = ps_slice_hdr->i2_ctb_x;
            ps_slice_hdr->i2_independent_ctb_y = ps_slice_hdr->i2_ctb_y;
        }
    }
    else
    {
        ps_slice_hdr->i2_ctb_x = 0;
        ps_slice_hdr->i2_ctb_y = 0;

        ps_slice_hdr->i2_independent_ctb_x = 0;
        ps_slice_hdr->i2_independent_ctb_y = 0;
    }

    /* If the first slice in the pic is missing, copy the current slice header to
     * the first slice's header */
    if((!first_slice_in_pic_flag) &&
                    (0 == ps_codec->i4_pic_present))
    {
        slice_header_t *ps_slice_hdr_prev = ps_codec->s_parse.ps_slice_hdr_base;
        ihevcd_copy_slice_hdr(ps_codec, 0, (ps_codec->s_parse.i4_cur_slice_idx & (MAX_SLICE_HDR_CNT - 1)));

        prev_slice_incomplete_flag = 1;
        ASSERT(ps_codec->s_parse.i4_cur_slice_idx == 1);

        ps_slice_hdr_prev->i2_ctb_x = 0;
        ps_slice_hdr_prev->i2_ctb_y = 0;

        ps_codec->s_parse.i4_ctb_x = 0;
        ps_codec->s_parse.i4_ctb_y = 0;

        ps_codec->s_parse.i4_cur_slice_idx = 0;

        if((ps_slice_hdr->i2_ctb_x == 0) &&
                        (ps_slice_hdr->i2_ctb_y == 0))
        {
            ps_slice_hdr->i2_ctb_x++;
        }
    }

    {
        /* If skip B is enabled,
         * ignore pictures that are non-reference
         * TODO: (i1_nal_unit_type < NAL_BLA_W_LP) && (i1_nal_unit_type % 2 == 0) only says it is
         * sub-layer non-reference slice. May need to find a way to detect actual non-reference pictures*/

        if((i1_nal_unit_type < NAL_BLA_W_LP) &&
                        (i1_nal_unit_type % 2 == 0))
        {
            if(IVD_SKIP_B == ps_codec->e_pic_skip_mode)
                return IHEVCD_IGNORE_SLICE;
        }

        /* If skip PB is enabled,
         * decode only I slices */
        if((IVD_SKIP_PB == ps_codec->e_pic_skip_mode) &&
                        (ISLICE != ps_slice_hdr->i1_slice_type))
        {
            return IHEVCD_IGNORE_SLICE;
        }
    }

    if(prev_slice_incomplete_flag)
    {
        ps_codec->i4_slice_error = 1;
        ps_codec->s_parse.i4_cur_slice_idx--;
        if(ps_codec->s_parse.i4_cur_slice_idx < 0)
            ps_codec->s_parse.i4_cur_slice_idx = 0;
    }

    return ret;
}
