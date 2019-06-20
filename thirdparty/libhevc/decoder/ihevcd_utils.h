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
*  ihevcd_parse_slice.h
*
* @brief
*  Contains miscellaneous utility functions such as init() etc
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

#ifndef _IHEVCD_UTILS_H_
#define _IHEVCD_UTILS_H_

WORD32 ihevcd_get_lvl_idx(WORD32 level);
WORD32 ihevcd_get_dpb_size(WORD32 level, WORD32 pic_size);
WORD32 ihevcd_get_pic_mv_bank_size(WORD32 num_luma_samples);
WORD32 ihevcd_get_tu_data_size(WORD32 num_luma_samples);
WORD32 ihevcd_nctb_cnt(codec_t *ps_codec, sps_t *ps_sps);
WORD32 ihevcd_get_max_luma_samples(WORD32 level);
IHEVCD_ERROR_T ihevcd_get_tile_pos(pps_t *ps_pps,
                                   sps_t *ps_sps,
                                   WORD32 ctb_x,
                                   WORD32 ctb_y,
                                   WORD32 *pi4_ctb_tile_x,
                                   WORD32 *pi4_ctb_tile_y,
                                   WORD32 *pi4_tile_idx);
IHEVCD_ERROR_T ihevcd_parse_pic_init(codec_t *ps_codec);
WORD32 ihevcd_get_total_pic_buf_size(codec_t *ps_codec,
                                     WORD32 wd,
                                     WORD32 ht);
#endif /* _IHEVCD_UTILS_H_ */
