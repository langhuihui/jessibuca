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
*  ihevcd_parse_headers.h
*
* @brief
*  Parsing of various headers like VPS, SPS, PPS etc
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/

#ifndef _IHEVCD_PARSE_HEADERS_H_
#define _IHEVCD_PARSE_HEADERS_H_

void ihevcd_copy_sps(codec_t *ps_codec, WORD32 sps_id, WORD32 sps_id_ref);
void ihevcd_copy_pps(codec_t *ps_codec, WORD32 pps_id, WORD32 pps_id_ref);
void ihevcd_copy_slice_hdr(codec_t *ps_codec, WORD32 slice_idx, WORD32 slice_idx_ref);

IHEVCD_ERROR_T ihevcd_parse_vps(codec_t *ps_codec);
IHEVCD_ERROR_T ihevcd_parse_sps(codec_t *ps_codec);
IHEVCD_ERROR_T ihevcd_parse_pps(codec_t *ps_codec);
IHEVCD_ERROR_T ihevcd_parse_sei(codec_t *ps_codec, nal_header_t *ps_nal);
IHEVCD_ERROR_T ihevcd_parse_pic_timing_sei(codec_t *ps_codec, sps_t *ps_sps);
IHEVCD_ERROR_T ihevcd_parse_buffering_period_sei(codec_t *ps_codec, sps_t *ps_sps);
IHEVCD_ERROR_T ihevcd_parse_time_code_sei(codec_t *ps_codec);
IHEVCD_ERROR_T ihevcd_parse_user_data_registered_itu_t_t35(codec_t *ps_codec, UWORD32 u4_payload_size);
IHEVCD_ERROR_T ihevcd_parse_active_parameter_sets_sei(codec_t *ps_codec, sps_t *ps_sps);
IHEVCD_ERROR_T ihevcd_read_rbsp_trailing_bits(codec_t *ps_codec, UWORD32 u4_bits_left);
IHEVCD_ERROR_T ihevcd_parse_slice_header(codec_t *ps_codec,
                                         nal_header_t *ps_nal);

#endif /* _IHEVCD_PARSE_HEADERS_H_ */
