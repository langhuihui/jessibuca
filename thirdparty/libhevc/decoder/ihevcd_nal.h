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
*  ihevcd_nal.h
*
* @brief
*  Header for NAL related function
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

#ifndef _IHEVCD_NAL_H_
#define _IHEVCD_NAL_H_
/**
 * Minimum size of start code including NAL type
 */

#define MIN_START_CODE_LEN          4
/**
 * Start code prefix byte - 1
 */
#define START_CODE_PREFIX_BYTE      1

/**
 * Emulation prevention byte - 3
 */

#define EMULATION_PREVENT_BYTE      3
/**
 * Minimum number of zeros before start code
 */
#define NUM_ZEROS_BEFORE_START_CODE 2


WORD32 ihevcd_nal_search_start_code(UWORD8 *pu1_buf, WORD32 bytes_remaining);

IHEVCD_ERROR_T ihevcd_nal_remv_emuln_bytes(UWORD8 *pu1_src,
                                           UWORD8 *pu1_dst,
                                           WORD32 bytes_remaining,
                                           WORD32 *pi4_nal_len,
                                           WORD32 *pi4_dst_len);

IHEVCD_ERROR_T ihevcd_nal_unit(codec_t *ps_codec);
#endif /* _IHEVCD_NAL_H_ */
