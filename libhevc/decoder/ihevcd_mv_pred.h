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
 *  ihevcd_structs.h
 *
 * @brief
 *  Structure definitions used in the decoder
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

#ifndef IHEVCD_MV_PRED_H_
#define IHEVCD_MV_PRED_H_
void ihevcd_mv_pred(mv_ctxt_t *ps_mv_ctxt,
                    UWORD32 *pu4_top_pu_idx,
                    UWORD32 *pu4_left_pu_idx,
                    UWORD32 *pu4_top_left_pu_idx,
                    WORD32 left_nbr_4x4_strd,
                    pu_t *ps_pu,
                    WORD32 lb_avail,
                    WORD32 l_avail,
                    WORD32 tr_avail,
                    WORD32 t_avail,
                    WORD32 tl_avail,
                    pu_mv_t *ps_pred_mv);
void ihevcd_scale_mv(mv_t *ps_mv,
                     WORD32 cur_ref_poc,
                     WORD32 nbr_ref_poc,
                     WORD32 cur_poc);


#endif /* IHEVCD_MV_PRED_H_ */
