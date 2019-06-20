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
*  ihevcd_api.c
*
* @brief
*  Contains functions definitions for reference list generation
*
* @author
*  Srinivas T
*
* @par List of Functions:\
* - ihevcd_ref_pic
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
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_structs.h"
#include "ihevc_buf_mgr.h"
#include "ihevc_dpb_mgr.h"

#include "ihevcd_trace.h"
#include "ihevcd_defs.h"
#include "ihevc_cabac_tables.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_nal.h"
#include "ihevcd_bitstream.h"
#include "ihevcd_debug.h"
#include "ihevcd_error.h"


mv_buf_t* ihevcd_mv_mgr_get_poc(buf_mgr_t *ps_mv_buf_mgr, UWORD32 abs_poc)
{
    UWORD32 i;
    mv_buf_t *ps_mv_buf = NULL;



    for(i = 0; i < ps_mv_buf_mgr->u4_max_buf_cnt; i++)
    {
        ps_mv_buf = (mv_buf_t *)ps_mv_buf_mgr->apv_ptr[i];
        if(ps_mv_buf && (ps_mv_buf->i4_abs_poc == (WORD32)abs_poc))
        {
            break;
        }
    }

    return ps_mv_buf;
}


WORD32 ihevcd_ref_list(codec_t *ps_codec, pps_t *ps_pps, sps_t *ps_sps, slice_header_t *ps_slice_hdr)
{
    WORD32 i, j;
    WORD32 st_rps_idx;
    WORD32 num_neg_pics, num_pos_pics;
    WORD8 *pi1_used;
    WORD16 *pi2_delta_poc;
    UWORD32 u4_max_poc_lsb;
    pic_buf_t *ps_pic_buf;
    mv_buf_t *ps_mv_buf;
    UWORD32 r_idx;

    dpb_mgr_t *ps_dpb_mgr = (dpb_mgr_t *)ps_codec->pv_dpb_mgr;
    buf_mgr_t *ps_mv_buf_mgr = (buf_mgr_t *)ps_codec->pv_mv_buf_mgr;

    WORD32 ai4_poc_st_curr_before[MAX_DPB_SIZE], ai4_poc_st_foll[MAX_DPB_SIZE], ai4_poc_st_curr_after[MAX_DPB_SIZE];
    WORD32 ai4_poc_lt_curr[MAX_DPB_SIZE], ai4_poc_lt_foll[MAX_DPB_SIZE];
    UWORD32 u4_num_st_curr_before, u4_num_st_foll, u4_num_st_curr_after, u4_num_lt_curr, u4_num_lt_foll;
    UWORD32 u4_num_total_curr;

    WORD8 ai1_curr_delta_poc_msb_present_flag[MAX_DPB_SIZE], ai1_foll_delta_poc_msb_present_flag[MAX_DPB_SIZE];

    pic_buf_t *as_ref_pic_lt_curr[MAX_DPB_SIZE];
    pic_buf_t *as_ref_pic_lt_foll[MAX_DPB_SIZE];
    pic_buf_t *as_ref_pic_st_curr_after[MAX_DPB_SIZE];
    pic_buf_t *as_ref_pic_st_curr_before[MAX_DPB_SIZE];
    pic_buf_t *as_ref_pic_st_foll[MAX_DPB_SIZE];

    pic_buf_t *as_ref_pic_list_temp0[MAX_DPB_SIZE], *as_ref_pic_list_temp1[MAX_DPB_SIZE];

    UWORD32 u4_num_rps_curr_temp_list0, u4_num_rps_curr_temp_list1;

    WORD32 i4_pic_order_cnt_val;
    WORD32 i4_poc_lt;
    UNUSED(as_ref_pic_lt_foll);
    UNUSED(as_ref_pic_st_foll);
    UNUSED(ps_pps);

    RETURN_IF_NAL_INFO;

    u4_max_poc_lsb = (1 << ps_sps->i1_log2_max_pic_order_cnt_lsb);

    i4_pic_order_cnt_val = ps_slice_hdr->i4_abs_pic_order_cnt;

    if(1 == ps_slice_hdr->i1_short_term_ref_pic_set_sps_flag)
    {
        st_rps_idx = ps_slice_hdr->i1_short_term_ref_pic_set_idx;
        num_neg_pics = ps_sps->as_stref_picset[st_rps_idx].i1_num_neg_pics;
        num_pos_pics = ps_sps->as_stref_picset[st_rps_idx].i1_num_pos_pics;
        pi1_used = ps_sps->as_stref_picset[st_rps_idx].ai1_used;
        pi2_delta_poc = ps_sps->as_stref_picset[st_rps_idx].ai2_delta_poc;
    }
    else
    {
        st_rps_idx = ps_sps->i1_num_short_term_ref_pic_sets;
        num_neg_pics = ps_slice_hdr->s_stref_picset.i1_num_neg_pics;
        num_pos_pics = ps_slice_hdr->s_stref_picset.i1_num_pos_pics;
        pi1_used = ps_slice_hdr->s_stref_picset.ai1_used;
        pi2_delta_poc = ps_slice_hdr->s_stref_picset.ai2_delta_poc;
    }

    u4_num_st_curr_before = 0;
    u4_num_st_foll = 0;
    for(i = 0; i < num_neg_pics; i++)
    {
        if(pi1_used[i])
        {
            ai4_poc_st_curr_before[u4_num_st_curr_before] = i4_pic_order_cnt_val + pi2_delta_poc[i];
            u4_num_st_curr_before++;
        }
        else
        {
            ai4_poc_st_foll[u4_num_st_foll] = i4_pic_order_cnt_val + pi2_delta_poc[i];
            u4_num_st_foll++;
        }
    }
    u4_num_st_curr_after = 0;
    for(i = num_neg_pics; i < num_neg_pics + num_pos_pics; i++)
    {
        if(pi1_used[i])
        {
            ai4_poc_st_curr_after[u4_num_st_curr_after] = i4_pic_order_cnt_val + pi2_delta_poc[i];
            u4_num_st_curr_after++;
        }
        else
        {
            ai4_poc_st_foll[u4_num_st_foll] = i4_pic_order_cnt_val + pi2_delta_poc[i];
            u4_num_st_foll++;
        }
    }

    u4_num_lt_curr = 0;
    u4_num_lt_foll = 0;
    for(i = 0; i < ps_slice_hdr->i1_num_long_term_sps + ps_slice_hdr->i1_num_long_term_pics; i++)
    {
        i4_poc_lt = ps_slice_hdr->ai4_poc_lsb_lt[i];
        if(ps_slice_hdr->ai1_delta_poc_msb_present_flag[i])
        {
            i4_poc_lt += i4_pic_order_cnt_val - ps_slice_hdr->ai1_delta_poc_msb_cycle_lt[i] * u4_max_poc_lsb - ps_slice_hdr->i4_pic_order_cnt_lsb;
        }

        if(ps_slice_hdr->ai1_used_by_curr_pic_lt_flag[i])
        {
            ai4_poc_lt_curr[u4_num_lt_curr] = i4_poc_lt;
            ai1_curr_delta_poc_msb_present_flag[u4_num_lt_curr] = ps_slice_hdr->ai1_delta_poc_msb_present_flag[i];
            u4_num_lt_curr++;
        }
        else
        {
            ai4_poc_lt_foll[u4_num_lt_foll] = i4_poc_lt;
            ai1_foll_delta_poc_msb_present_flag[u4_num_lt_foll] = ps_slice_hdr->ai1_delta_poc_msb_present_flag[i];
            u4_num_lt_foll++;
        }
    }

    u4_num_total_curr = u4_num_lt_curr + u4_num_st_curr_after + u4_num_st_curr_before;

    /* Bit stream conformance tests */
/*
    for(i = 0; i < u4_num_lt_curr; i++)
    {
        int j;
        if(ai1_curr_delta_poc_msb_present_flag[i])
        {
            for(j = 0; j < u4_num_st_curr_before; j++)
            {
                ASSERT(ai4_poc_st_curr_before[j] != ai4_poc_lt_curr[i]);
            }
            for(j = 0; j < u4_num_st_curr_after; j++)
            {
                ASSERT(ai4_poc_st_curr_after[j] != ai4_poc_lt_curr[i]);
            }
            for(j = 0; j < u4_num_st_foll; j++)
            {
                ASSERT(ai4_poc_st_foll[j] != ai4_poc_lt_curr[i]);
            }
            for(j = 0; j < u4_num_lt_curr; j++)
            {
                ASSERT((ai4_poc_lt_curr[j] != ai4_poc_lt_curr[i]) || (j == i));
            }
        }
        else
        {
            for(j = 0; j < u4_num_st_curr_before; j++)
            {
                ASSERT((ai4_poc_st_curr_before[j] & (u4_max_poc_lsb - 1)) != ai4_poc_lt_curr[i]);
            }
            for(j = 0; j < u4_num_st_curr_after; j++)
            {
                ASSERT((ai4_poc_st_curr_after[j] & (u4_max_poc_lsb - 1)) != ai4_poc_lt_curr[i]);
            }
            for(j = 0; j < u4_num_st_foll; j++)
            {
                ASSERT((ai4_poc_st_foll[j] & (u4_max_poc_lsb - 1)) != ai4_poc_lt_curr[i]);
            }
            for(j = 0; j < u4_num_lt_curr; j++)
            {
                ASSERT(((ai4_poc_lt_curr[j] & (u4_max_poc_lsb - 1)) != ai4_poc_lt_curr[i]) || (j == i));
            }
        }
    }

    for(i = 0; i < u4_num_lt_foll; i++)
    {
        int j;
        if(ai1_foll_delta_poc_msb_present_flag[i])
        {
            for(j = 0; j < u4_num_st_curr_before; j++)
            {
                ASSERT(ai4_poc_st_curr_before[j] != ai4_poc_lt_foll[i]);
            }
            for(j = 0; j < u4_num_st_curr_after; j++)
            {
                ASSERT(ai4_poc_st_curr_after[j] != ai4_poc_lt_foll[i]);
            }
            for(j = 0; j < u4_num_st_foll; j++)
            {
                ASSERT(ai4_poc_st_foll[j] != ai4_poc_lt_foll[i]);
            }
            for(j = 0; j < u4_num_lt_curr; j++)
            {
                ASSERT(ai4_poc_lt_curr[j] != ai4_poc_lt_foll[i]);
            }
            for(j = 0; j < u4_num_lt_foll; j++)
            {
                ASSERT((ai4_poc_lt_foll[j] != ai4_poc_lt_foll[i]) || (j == i));
            }
        }
        else
        {
            for(j = 0; j < u4_num_st_curr_before; j++)
            {
                ASSERT((ai4_poc_st_curr_before[j] & (u4_max_poc_lsb - 1)) != ai4_poc_lt_foll[i]);
            }
            for(j = 0; j < u4_num_st_curr_after; j++)
            {
                ASSERT((ai4_poc_st_curr_after[j] & (u4_max_poc_lsb - 1)) != ai4_poc_lt_foll[i]);
            }
            for(j = 0; j < u4_num_st_foll; j++)
            {
                ASSERT((ai4_poc_st_foll[j] & (u4_max_poc_lsb - 1)) != ai4_poc_lt_foll[i]);
            }
            for(j = 0; j < u4_num_lt_curr; j++)
            {
                ASSERT((ai4_poc_lt_curr[j] & (u4_max_poc_lsb - 1)) != ai4_poc_lt_foll[i]);
            }
            for(j = 0; j < u4_num_lt_foll; j++)
            {
                ASSERT(((ai4_poc_lt_foll[j] & (u4_max_poc_lsb - 1)) != ai4_poc_lt_foll[i]) || (j == i));
            }
        }
    }
*/


    /* Reference Pic sets creation */

    /* Set all the DPB buffers to UNUSED_FOR_REF */
    if(0 == ps_codec->i4_pic_present)
    {
        for(i = 0; i < MAX_DPB_BUFS; i++)
        {
            if(ps_dpb_mgr->as_dpb_info[i].ps_pic_buf)
                ps_dpb_mgr->as_dpb_info[i].ps_pic_buf->u1_used_as_ref = UNUSED_FOR_REF;
        }
    }

    for(i = 0; i < (WORD32)u4_num_lt_curr; i++)
    {
        if(0 == ai1_curr_delta_poc_msb_present_flag[i])
        {
            ps_pic_buf = ihevc_dpb_mgr_get_ref_by_poc_lsb(ps_dpb_mgr, ai4_poc_lt_curr[i]);
            if(NULL != ps_pic_buf)
                ps_pic_buf->u1_used_as_ref = LONG_TERM_REF;

            as_ref_pic_lt_curr[i] = ps_pic_buf;
        }
        else
        {
            ps_pic_buf = ihevc_dpb_mgr_get_ref_by_poc(ps_dpb_mgr, ai4_poc_lt_curr[i]);
            if(NULL != ps_pic_buf)
                ps_pic_buf->u1_used_as_ref = LONG_TERM_REF;

            as_ref_pic_lt_curr[i] = ps_pic_buf;
        }
    }

    for(i = 0; i < (WORD32)u4_num_lt_foll; i++)
    {
        if(0 == ai1_foll_delta_poc_msb_present_flag[i])
        {
            ps_pic_buf = ihevc_dpb_mgr_get_ref_by_poc_lsb(ps_dpb_mgr, ai4_poc_lt_foll[i]);
            if(NULL != ps_pic_buf)
                ps_pic_buf->u1_used_as_ref = LONG_TERM_REF;

            as_ref_pic_lt_foll[i] = ps_pic_buf;
        }
        else
        {
            ps_pic_buf = ihevc_dpb_mgr_get_ref_by_poc(ps_dpb_mgr, ai4_poc_lt_foll[i]);
            if(NULL != ps_pic_buf)
                ps_pic_buf->u1_used_as_ref = LONG_TERM_REF;

            as_ref_pic_lt_foll[i] = ps_pic_buf;
        }
    }


    for(i = 0; i < (WORD32)u4_num_st_curr_before; i++)
    {

        ps_pic_buf = ihevc_dpb_mgr_get_ref_by_poc(ps_dpb_mgr, ai4_poc_st_curr_before[i]);
        if(NULL != ps_pic_buf)
            ps_pic_buf->u1_used_as_ref = SHORT_TERM_REF;

        as_ref_pic_st_curr_before[i] = ps_pic_buf;
    }

    for(i = 0; i < (WORD32)u4_num_st_curr_after; i++)
    {
        ps_pic_buf = ihevc_dpb_mgr_get_ref_by_poc(ps_dpb_mgr, ai4_poc_st_curr_after[i]);
        if(NULL != ps_pic_buf)
            ps_pic_buf->u1_used_as_ref = SHORT_TERM_REF;

        as_ref_pic_st_curr_after[i] = ps_pic_buf;
    }

    for(i = 0; i < (WORD32)u4_num_st_foll; i++)
    {
        ps_pic_buf = ihevc_dpb_mgr_get_ref_by_poc(ps_dpb_mgr, ai4_poc_st_foll[i]);
        if(NULL != ps_pic_buf)
            ps_pic_buf->u1_used_as_ref = SHORT_TERM_REF;

        as_ref_pic_st_foll[i] = ps_pic_buf;
    }

    //TODO: Bit stream conformance tests to be included

    u4_num_rps_curr_temp_list0 = (WORD32)u4_num_total_curr > ps_slice_hdr->i1_num_ref_idx_l0_active ? (WORD32)u4_num_total_curr : ps_slice_hdr->i1_num_ref_idx_l0_active;

    r_idx = 0;
    if((PSLICE == ps_slice_hdr->i1_slice_type) ||
       (BSLICE == ps_slice_hdr->i1_slice_type))
    {
        while(r_idx < u4_num_rps_curr_temp_list0)
        {
            for(i = 0; (i < (WORD32)u4_num_st_curr_before) && (r_idx < u4_num_rps_curr_temp_list0); r_idx++, i++)
            {
                if(NULL == as_ref_pic_st_curr_before[i])
                {
                    as_ref_pic_st_curr_before[i] = ihevc_dpb_mgr_get_ref_by_nearest_poc(ps_dpb_mgr, ai4_poc_st_curr_before[i]);
                }
                as_ref_pic_list_temp0[r_idx] = as_ref_pic_st_curr_before[i];
            }

            for(i = 0; (i < (WORD32)u4_num_st_curr_after) && (r_idx < u4_num_rps_curr_temp_list0); r_idx++, i++)
            {
                if(NULL == as_ref_pic_st_curr_after[i])
                {
                    as_ref_pic_st_curr_after[i] = ihevc_dpb_mgr_get_ref_by_nearest_poc(ps_dpb_mgr, ai4_poc_st_curr_after[i]);
                }
                as_ref_pic_list_temp0[r_idx] = as_ref_pic_st_curr_after[i];
            }

            for(i = 0; (i < (WORD32)u4_num_lt_curr) && (r_idx < u4_num_rps_curr_temp_list0); r_idx++, i++)
            {
                if(NULL == as_ref_pic_lt_curr[i])
                {
                    as_ref_pic_lt_curr[i] = ihevc_dpb_mgr_get_ref_by_nearest_poc(ps_dpb_mgr, ai4_poc_lt_curr[i]);
                }
                as_ref_pic_list_temp0[r_idx] = as_ref_pic_lt_curr[i];
            }
        }

        for(r_idx = 0; (WORD32)r_idx < ps_slice_hdr->i1_num_ref_idx_l0_active; r_idx++)
        {
            pic_buf_t *ps_pic_buf;
            ps_slice_hdr->as_ref_pic_list0[r_idx].pv_pic_buf = ps_slice_hdr->s_rplm.i1_ref_pic_list_modification_flag_l0 ?  (void *)as_ref_pic_list_temp0[ps_slice_hdr->s_rplm.i1_list_entry_l0[r_idx]] :  (void *)as_ref_pic_list_temp0[r_idx];
            ps_pic_buf = (pic_buf_t *)ps_slice_hdr->as_ref_pic_list0[r_idx].pv_pic_buf;

            if(ps_pic_buf == NULL)
                return IHEVCD_REF_PIC_NOT_FOUND;

            ps_mv_buf = ihevcd_mv_mgr_get_poc(ps_mv_buf_mgr, ps_pic_buf->i4_abs_poc);
            ps_slice_hdr->as_ref_pic_list0[r_idx].pv_mv_buf = ps_mv_buf;
        }


        if(ps_slice_hdr->i1_slice_type  == BSLICE)
        {
            u4_num_rps_curr_temp_list1 = (WORD32)u4_num_total_curr > ps_slice_hdr->i1_num_ref_idx_l1_active ? (WORD32)u4_num_total_curr : ps_slice_hdr->i1_num_ref_idx_l1_active;

            r_idx = 0;
            while(r_idx < u4_num_rps_curr_temp_list1)
            {
                for(i = 0; (i < (WORD32)u4_num_st_curr_after) && (r_idx < u4_num_rps_curr_temp_list1); r_idx++, i++)
                {
                    if(NULL == as_ref_pic_st_curr_after[i])
                    {
                        as_ref_pic_st_curr_after[i] = ihevc_dpb_mgr_get_ref_by_nearest_poc(ps_dpb_mgr, ai4_poc_st_curr_after[i]);
                    }
                    as_ref_pic_list_temp1[r_idx] = as_ref_pic_st_curr_after[i];
                }

                for(i = 0; (i < (WORD32)u4_num_st_curr_before) && (r_idx < u4_num_rps_curr_temp_list1); r_idx++, i++)
                {
                    if(NULL == as_ref_pic_st_curr_before[i])
                    {
                        as_ref_pic_st_curr_before[i] = ihevc_dpb_mgr_get_ref_by_nearest_poc(ps_dpb_mgr, ai4_poc_st_curr_before[i]);
                    }
                    as_ref_pic_list_temp1[r_idx] = as_ref_pic_st_curr_before[i];
                }

                for(i = 0; (i < (WORD32)u4_num_lt_curr) && (r_idx < u4_num_rps_curr_temp_list1); r_idx++, i++)
                {
                    if(NULL == as_ref_pic_lt_curr[i])
                    {
                        as_ref_pic_lt_curr[i] = ihevc_dpb_mgr_get_ref_by_nearest_poc(ps_dpb_mgr, ai4_poc_lt_curr[i]);
                    }
                    as_ref_pic_list_temp1[r_idx] = as_ref_pic_lt_curr[i];
                }
            }

            for(r_idx = 0; (WORD32)r_idx < ps_slice_hdr->i1_num_ref_idx_l1_active; r_idx++)
            {
                pic_buf_t *ps_pic_buf;
                ps_slice_hdr->as_ref_pic_list1[r_idx].pv_pic_buf = ps_slice_hdr->s_rplm.i1_ref_pic_list_modification_flag_l1 ?  (void *)as_ref_pic_list_temp1[ps_slice_hdr->s_rplm.i1_list_entry_l1[r_idx]] :  (void *)as_ref_pic_list_temp1[r_idx];
                ps_pic_buf = (pic_buf_t *)ps_slice_hdr->as_ref_pic_list1[r_idx].pv_pic_buf;

                if(ps_pic_buf == NULL)
                    return IHEVCD_REF_PIC_NOT_FOUND;

                ps_mv_buf = ihevcd_mv_mgr_get_poc(ps_mv_buf_mgr, ps_pic_buf->i4_abs_poc);
                ps_slice_hdr->as_ref_pic_list1[r_idx].pv_mv_buf = ps_mv_buf;
            }
        }
    }

    DEBUG_PRINT_REF_LIST_POCS(i4_pic_order_cnt_val, ps_slice_hdr, ps_dpb_mgr, u4_num_st_curr_before, u4_num_st_curr_after, u4_num_st_foll, u4_num_lt_curr, u4_num_lt_foll, ai4_poc_st_curr_before, ai4_poc_st_curr_after, ai4_poc_st_foll, ai4_poc_lt_curr, ai4_poc_lt_foll);
    /* Buffers that are still marked as UNUSED_FOR_REF are released from dpb (internally dpb calls release from pic buf manager)*/
    for(i = 0; i < MAX_DPB_BUFS; i++)
    {
        if((ps_dpb_mgr->as_dpb_info[i].ps_pic_buf) && (UNUSED_FOR_REF == ps_dpb_mgr->as_dpb_info[i].ps_pic_buf->u1_used_as_ref))
        {
            pic_buf_t *ps_pic_buf = ps_dpb_mgr->as_dpb_info[i].ps_pic_buf;
            mv_buf_t *ps_mv_buf;

            /* Long term index is set to MAX_DPB_BUFS to ensure it is not added as LT */
            ihevc_dpb_mgr_del_ref(ps_dpb_mgr, (buf_mgr_t *)ps_codec->pv_pic_buf_mgr, ps_pic_buf->i4_abs_poc);


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

    return IHEVCD_SUCCESS;
}
