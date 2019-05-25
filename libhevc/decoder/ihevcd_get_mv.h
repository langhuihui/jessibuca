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
*  Processing of slice level data
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

#ifndef IHEVCD_GET_MV_H_
#define IHEVCD_GET_MV_H_

WORD32 ihevcd_get_mv_ctb(mv_ctxt_t *ps_mv_ctxt,
                         UWORD32 *pu4_ctb_top_pu_idx,
                         UWORD32 *pu4_ctb_left_pu_idx,
                         UWORD32 *pu4_ctb_top_left_pu_idx);


#endif /* IHEVCD_GET_MV_H_ */
