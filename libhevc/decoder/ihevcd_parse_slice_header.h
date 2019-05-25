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
*  ihevcd_parse_slice_header.h
*
* @brief
*  Parsing of slice header
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/

#ifndef _IHEVCD_PARSE_SLICE_HEADER_H_
#define _IHEVCD_PARSE_SLICE_HEADER_H_

IHEVCD_ERROR_T ihevcd_short_term_ref_pic_set(bitstrm_t *ps_bitstrm,
                                             stref_picset_t *ps_stref_picset_base,
                                             WORD32 num_short_term_ref_pic_sets,
                                             WORD32 idx,
                                             stref_picset_t *ps_stref_picset);

WORD32 ihevcd_parse_pred_wt_ofst(bitstrm_t *ps_bitstrm,
                                 sps_t *ps_sps,
                                 pps_t *ps_pps,
                                 slice_header_t *ps_slice_hdr);

WORD32 ihevcd_calc_poc(codec_t *ps_codec, nal_header_t *ps_nal, WORD8 i1_log2_max_poc_lsb, WORD32 i2_poc_lsb);



#endif /* _IHEVCD_PARSE_SLICE_HEADER_H_ */
