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
 *  ihevcd_iquant_itrans_recon_ctb.h
 *
 * @brief
 *  Definitions related to inverse transform functions
 *
 * @author
 *  Naveen S R
 *
 * @par List of Functions:
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

#ifndef _IHEVCD_IQUANT_ITRANS_RECON_CTB_H_
#define _IHEVCD_IQUANT_ITRANS_RECON_CTB_H_

#define MAX_NUM_IP_MODES        35

typedef enum
{
    IP_FUNC_MODE_0 = 1,
    IP_FUNC_MODE_1,
    IP_FUNC_MODE_2,
    IP_FUNC_MODE_3TO9,
    IP_FUNC_MODE_10,
    IP_FUNC_MODE_11TO17,
    IP_FUNC_MODE_18_34,
    IP_FUNC_MODE_19TO25,
    IP_FUNC_MODE_26,
    IP_FUNC_MODE_27TO33,

    NUM_IP_FUNCS

}IP_FUNCS_T;


typedef enum
{
    DST_4x4, DCT_4x4, DCT_8x8, DCT_16x16, DCT_32x32, SKIP_64x64
}TRANSFORM_TYPE;

WORD32 ihevcd_iquant_itrans_recon_ctb(process_ctxt_t *ps_proc);

#endif /* _IHEVCD_IQUANT_ITRANS_RECON_CTB_H_ */
