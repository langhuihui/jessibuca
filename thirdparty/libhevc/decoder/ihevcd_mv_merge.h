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
*  ihevce_mv_merge.h
*
* @brief
*    This file contains function prototypes of MV Merge candidates list
*    derivation functions and corresponding structure and macrso definations
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

#ifndef _IHEVCD_MV_MERGE_H_
#define _IHEVCD_MV_MERGE_H_

/*****************************************************************************/
/* Constant Macros                                                           */
/*****************************************************************************/

#define MAX_NUM_MV_NBR     5

/*****************************************************************************/
/* Function Macros                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Typedefs                                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/
typedef enum
{
    NBR_A0 = 0,
    NBR_A1 = 1,
    NBR_B0 = 2,
    NBR_B1 = 3,
    NBR_B2 = 4,

    /* should be last */
    MAX_NUM_NBRS
}MV_MERGE_NBRS_T;

/*****************************************************************************/
/* Structure                                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Extern Variable Declarations                                              */
/*****************************************************************************/

/*****************************************************************************/
/* Extern Function Declarations                                              */
/*****************************************************************************/
void ihevcd_mv_merge(mv_ctxt_t *ps_mv_ctxt,
                     UWORD32 *pu4_top_pu_idx,
                     UWORD32 *pu4_left_pu_idx,
                     WORD32 left_nbr_4x4_strd,
                     pu_t *ps_pu,
                     WORD32 part_mode,
                     WORD32 part_idx,
                     WORD32 part_wd,
                     WORD32 part_ht,
                     WORD32 part_pos_x,
                     WORD32 part_pos_y,
                     WORD32 single_mcl_flag,
                     WORD32 lb_avail,
                     WORD32 l_avail,
                     WORD32 tr_avail,
                     WORD32 t_avail,
                     WORD32 tl_avail);
void ihevcd_collocated_mvp(mv_ctxt_t *ps_mv_ctxt,
                           pu_t *ps_pu,
                           mv_t *ps_mv_col,
                           WORD32 *pu4_avail_col_flag,
                           WORD32 use_pu_ref_idx,
                           WORD32 x_col,
                           WORD32 y_col);

void ihevcd_scale_collocated_mv(mv_t *ps_mv,
                                WORD32 cur_ref_poc,
                                WORD32 col_ref_poc,
                                WORD32 col_poc,
                                WORD32 cur_poc);
#endif  /* _IHEVCD_MV_MERGE_H_ */
