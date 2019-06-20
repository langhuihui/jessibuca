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
*  ihevcd_nal.c
*
* @brief
*  Contains functions for NAL level such as search start code etc
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
#include "ihevc_cabac_tables.h"


#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_nal.h"
#include "ihevcd_bitstream.h"
#include "ihevcd_parse_headers.h"
#include "ihevcd_parse_slice.h"
#include "ihevcd_debug.h"
/*****************************************************************************/
/* Function Prototypes                                                       */
/*****************************************************************************/

/**
*******************************************************************************
*
* @brief
*  Search start code from the given buffer pointer
*
* @par Description:
*  Search for start code  Return the offset of start code if start code is
* found  If no start code is found till end of given bitstream  then treat
* it as invalid NAL and return end of buffer as  offset
*
* @param[in] pu1_buf
*  Pointer to bitstream
*
* @param[in] bytes_remaining
*  Number of bytes remaining in the buffer
*
* @returns  Offset to the first byte in NAL after start code
*
* @remarks
*  Incomplete start code at the end of input bitstream is  not handled. This
* has to be taken care outside this func
*
*******************************************************************************
*/
WORD32 ihevcd_nal_search_start_code(UWORD8 *pu1_buf, WORD32 bytes_remaining)
{
    WORD32 ofst;

    WORD32 zero_byte_cnt;
    WORD32 start_code_found;

    ofst = -1;

    zero_byte_cnt = 0;
    start_code_found = 0;
    while(ofst < (bytes_remaining - 1))
    {
        ofst++;
        if(pu1_buf[ofst] != 0)
        {
            zero_byte_cnt = 0;
            continue;
        }

        zero_byte_cnt++;
        if((ofst < (bytes_remaining - 1)) &&
           (pu1_buf[ofst + 1] == START_CODE_PREFIX_BYTE) &&
           (zero_byte_cnt >= NUM_ZEROS_BEFORE_START_CODE))
        {
            /* Found the start code */
            ofst++;
            start_code_found = 1;
            break;
        }
    }
    if((0 == start_code_found) && (ofst < bytes_remaining))
    {
        if((START_CODE_PREFIX_BYTE == pu1_buf[ofst]) &&
           (zero_byte_cnt >= NUM_ZEROS_BEFORE_START_CODE))
        {
            /* Found a start code at the end*/
            ofst++;
        }
    }
    /* Since ofst started at -1, increment it by 1 */
    ofst++;

    return ofst;
}

/**
*******************************************************************************
*
* @brief
*  Remove emulation prevention byte present in the bitstream till next start
* code is found. Emulation prevention byte  removed data is stored in a
* different buffer
*
* @par Description:
*  Assumption is first start code is already found and  pu1_buf is pointing
* to a byte after the start code  Search for Next NAL's start code  Return
* if start code is found  Remove any emulation prevention byte present  Copy
* data to new buffer  If no start code is found, then treat complete buffer
* as  one nal.
*
* @param[in] pu1_src
*  Pointer to bitstream (excludes the initial the start code)
*
* @param[in] pu1_dst
*  Pointer to destination buffer
*
* @param[in] bytes_remaining
*  Number of bytes remaining
*
* @param[out] pi4_nal_len
*  NAL length (length of bitstream parsed)
*
* @param[out] pi4_dst_len
*  Destination bitstream size (length of bitstream parsed with emulation bytes
* removed)
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*  Incomplete start code at the end of input bitstream is  not handled. This
* has to be taken care outside this func
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_nal_remv_emuln_bytes(UWORD8 *pu1_src,
                                           UWORD8 *pu1_dst,
                                           WORD32 bytes_remaining,
                                           WORD32 *pi4_nal_len,
                                           WORD32 *pi4_dst_len)
{
    WORD32 src_cnt;
    WORD32 dst_cnt;
    WORD32 zero_byte_cnt;
    WORD32 start_code_found;
    UWORD8 u1_src;
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;

    src_cnt = 0;
    dst_cnt = 0;
    zero_byte_cnt = 0;
    start_code_found = 0;
    while(src_cnt < (bytes_remaining - 1))
    {
        u1_src = pu1_src[src_cnt++];

        pu1_dst[dst_cnt++] = u1_src;
        if(u1_src != 0)
        {
            zero_byte_cnt = 0;
            continue;
        }

        zero_byte_cnt++;
        if(zero_byte_cnt >= NUM_ZEROS_BEFORE_START_CODE)
        {
            u1_src = pu1_src[src_cnt];
            if(START_CODE_PREFIX_BYTE == u1_src)
            {
                /* Found the start code */
                src_cnt -= zero_byte_cnt;
                dst_cnt -= zero_byte_cnt;
                start_code_found = 1;
                break;
            }
            else if(EMULATION_PREVENT_BYTE == u1_src)
            {
                /* Found the emulation prevention byte */
                src_cnt++;
                zero_byte_cnt = 0;

                /* Decrement dst_cnt so that the next byte overwrites
                 * the emulation prevention byte already copied to dst above
                 */
            }
        }

    }

    if((0 == start_code_found) && (src_cnt < bytes_remaining))
    {
        u1_src = pu1_src[src_cnt++];
        if(zero_byte_cnt >= NUM_ZEROS_BEFORE_START_CODE)
        {

            if(START_CODE_PREFIX_BYTE == u1_src)
            {
                /* Found a start code at the end*/
                src_cnt -= zero_byte_cnt;
            }
            else if(EMULATION_PREVENT_BYTE == u1_src)
            {
                /* Found the emulation prevention byte at the end*/
                src_cnt++;
                /* Decrement dst_cnt so that the next byte overwrites
                 * the emulation prevention byte already copied to dst above
                 */
                dst_cnt--;
            }
        }
        else
        {
            pu1_dst[dst_cnt++] = u1_src;
        }


    }
    *pi4_nal_len = src_cnt;
    *pi4_dst_len = dst_cnt;
    return ret;
}
/**
*******************************************************************************
*
* @brief
*  Decode given NAL unit's header
*
* @par Description:
*  Call NAL unit's header decode  Section: 7.3.1.2
*
* @param[in] ps_bitstrm
*  Pointer to bitstream context
*
* @param[out] ps_nal
*  Pointer to NAL header
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_nal_unit_header(bitstrm_t *ps_bitstrm, nal_header_t *ps_nal)
{
    WORD32 unused;
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    UNUSED(unused);
    /* Syntax : forbidden_zero_bit */
    unused = ihevcd_bits_get(ps_bitstrm, 1);

    /* Syntax : nal_unit_type */
    ps_nal->i1_nal_unit_type = ihevcd_bits_get(ps_bitstrm, 6);

    /* Syntax : nuh_reserved_zero_6bits */
    unused = ihevcd_bits_get(ps_bitstrm, 6);

    /* Syntax : nuh_temporal_id_plus1 */
    ps_nal->i1_nuh_temporal_id = ihevcd_bits_get(ps_bitstrm, 3) - 1;

    return ret;

}

/**
*******************************************************************************
*
* @brief
*  Decode given NAL
*
* @par Description:
*  Based on the NAL type call appropriate decode function  Section: 7.3.1.1
*
*
* @param[in,out] ps_codec
*  Pointer to codec context (Functions called within will modify contents of
* ps_codec)
*
* @returns Error code from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_nal_unit(codec_t *ps_codec)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;

    /* NAL Header */
    nal_header_t s_nal;

    ret = ihevcd_nal_unit_header(&ps_codec->s_parse.s_bitstrm, &s_nal);
    RETURN_IF((ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS), ret);

    if(ps_codec->i4_slice_error)
        s_nal.i1_nal_unit_type = ps_codec->s_parse.ps_slice_hdr->i1_nal_unit_type;

    /* Setting RASL Output flag */
    switch(s_nal.i1_nal_unit_type)
    {
        case NAL_BLA_W_LP    :
        case NAL_BLA_W_DLP   :
        case NAL_BLA_N_LP    :
            ps_codec->i4_rasl_output_flag = 0;
            break;

        //TODO: After IDR, there is no case of open GOP
        //To be fixed appropriately by ignoring RASL only if the
        // required references are not found
        case NAL_IDR_W_LP    :
        case NAL_IDR_N_LP    :
            ps_codec->i4_rasl_output_flag = 1;
            break;

        case NAL_CRA         :
            ps_codec->i4_rasl_output_flag = (0 != ps_codec->i4_cra_as_first_pic) ? 0 : 1;
            break;

        default:
            break;
    }

    switch(s_nal.i1_nal_unit_type)
    {
        case NAL_BLA_W_LP    :
        case NAL_BLA_W_DLP   :
        case NAL_BLA_N_LP    :
        case NAL_IDR_W_LP    :
        case NAL_IDR_N_LP    :
        case NAL_CRA         :
        case NAL_TRAIL_N     :
        case NAL_TRAIL_R     :
        case NAL_TSA_N       :
        case NAL_TSA_R       :
        case NAL_STSA_N      :
        case NAL_STSA_R      :
        case NAL_RADL_N      :
        case NAL_RADL_R      :
        case NAL_RASL_N      :
        case NAL_RASL_R      :
            if(ps_codec->i4_header_mode)
                return IHEVCD_SLICE_IN_HEADER_MODE;

            if((0 == ps_codec->i4_sps_done) ||
                            (0 == ps_codec->i4_pps_done))
            {
                return IHEVCD_INVALID_HEADER;
            }

            ps_codec->i4_header_in_slice_mode = 0;
            ps_codec->i4_cra_as_first_pic = 0;

            ret = ihevcd_parse_slice_header(ps_codec, &s_nal);
            DEBUG_PRINT_NAL_INFO(ps_codec, s_nal.i1_nal_unit_type);
            if(ret == (IHEVCD_ERROR_T)IHEVCD_SUCCESS)
            {
                if((s_nal.i1_nal_unit_type != NAL_RASL_N && s_nal.i1_nal_unit_type != NAL_RASL_R) ||
                                ps_codec->i4_rasl_output_flag ||
                                ps_codec->i4_slice_error)
                    ret = ihevcd_parse_slice_data(ps_codec);
            }
            break;

        case NAL_VPS        :
            // ret = ihevcd_parse_vps(ps_codec);
            DEBUG_PRINT_NAL_INFO(ps_codec, s_nal.i1_nal_unit_type);
            break;

        case NAL_SPS        :
            if(0 == ps_codec->i4_header_mode)
            {
                ps_codec->i4_header_in_slice_mode = 1;
                if(ps_codec->i4_sps_done &&
                                ps_codec->i4_pic_present)
                    break;
            }

            ret = ihevcd_parse_sps(ps_codec);
            if(ret == (IHEVCD_ERROR_T)IHEVCD_SUCCESS)
            {
                sps_t *ps_sps = ps_codec->ps_sps_base + MAX_SPS_CNT - 1;
                ihevcd_copy_sps(ps_codec, ps_sps->i1_sps_id, MAX_SPS_CNT - 1);
            }
            ps_codec->i4_error_code = ret;

            DEBUG_PRINT_NAL_INFO(ps_codec, s_nal.i1_nal_unit_type);
            break;

        case NAL_PPS        :
            if(0 == ps_codec->i4_header_mode)
            {
                ps_codec->i4_header_in_slice_mode = 1;
                if(ps_codec->i4_pps_done &&
                                ps_codec->i4_pic_present)
                    break;
            }

            ret = ihevcd_parse_pps(ps_codec);
            if(ret == (IHEVCD_ERROR_T)IHEVCD_SUCCESS)
            {
                pps_t *ps_pps = ps_codec->ps_pps_base + MAX_PPS_CNT - 1;
                ihevcd_copy_pps(ps_codec, ps_pps->i1_pps_id, MAX_PPS_CNT - 1);
            }
            ps_codec->i4_error_code = ret;
            DEBUG_PRINT_NAL_INFO(ps_codec, s_nal.i1_nal_unit_type);
            break;

        case NAL_PREFIX_SEI:
        case NAL_SUFFIX_SEI:
            if(IVD_DECODE_HEADER == ps_codec->i4_header_mode)
            {
                return IHEVCD_SLICE_IN_HEADER_MODE;
            }

            ret = ihevcd_parse_sei(ps_codec, &s_nal);
            break;

        case NAL_EOS        :
            ps_codec->i4_cra_as_first_pic = 1;
            break;

        default:
            DEBUG_PRINT_NAL_INFO(ps_codec, s_nal.i1_nal_unit_type);
            break;
    }

    return ret;
}

