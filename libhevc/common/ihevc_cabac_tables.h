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
******************************************************************************
* @file ihevc_cabac_tables.h
*
* @brief
*  This file contains enumerations, macros and extern declarations of HEVC
*  cabac tables
*
* @author
*  Ittiam
******************************************************************************
*/

#ifndef _IHEVC_CABAC_TABLES_H_
#define _IHEVC_CABAC_TABLES_H_

/*****************************************************************************/
/* Constant Macros                                                           */
/*****************************************************************************/

/**
******************************************************************************
 *  @brief  maximum range of cabac_init_idc (0-2)
******************************************************************************
 */
#define IHEVC_NUM_CAB_IDC   3

/**
******************************************************************************
 *  @brief  max range of qps in HEVC (0-51)
******************************************************************************
 */
#define IHEVC_MAX_QP      52


/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/

/**
 *****************************************************************************
 *  @brief   start offsets of cabac contexts for various syntax elements
 *
 *  @remarks Init ContextModel generation and these offsets are tightly coupled
             See the cabac table generation utility for these offsets
 *****************************************************************************
 */
typedef enum
{
    IHEVC_CAB_SAO_MERGE             = 0,
    IHEVC_CAB_SAO_TYPE              = IHEVC_CAB_SAO_MERGE               + 1,
    IHEVC_CAB_SPLIT_CU_FLAG         = IHEVC_CAB_SAO_TYPE                + 1,
    IHEVC_CAB_CU_TQ_BYPASS_FLAG     = IHEVC_CAB_SPLIT_CU_FLAG           + 3,
    IHEVC_CAB_SKIP_FLAG             = IHEVC_CAB_CU_TQ_BYPASS_FLAG       + 1,
    IHEVC_CAB_QP_DELTA_ABS          = IHEVC_CAB_SKIP_FLAG               + 3,
    IHEVC_CAB_PRED_MODE             = IHEVC_CAB_QP_DELTA_ABS            + 2,
    IHEVC_CAB_PART_MODE             = IHEVC_CAB_PRED_MODE               + 1,
    IHEVC_CAB_INTRA_LUMA_PRED_FLAG  = IHEVC_CAB_PART_MODE               + 4,
    IHEVC_CAB_CHROMA_PRED_MODE      = IHEVC_CAB_INTRA_LUMA_PRED_FLAG    + 1,
    IHEVC_CAB_MERGE_FLAG_EXT        = IHEVC_CAB_CHROMA_PRED_MODE        + 1,
    IHEVC_CAB_MERGE_IDX_EXT         = IHEVC_CAB_MERGE_FLAG_EXT          + 1,
    IHEVC_CAB_INTER_PRED_IDC        = IHEVC_CAB_MERGE_IDX_EXT           + 1,
    IHEVC_CAB_INTER_REF_IDX         = IHEVC_CAB_INTER_PRED_IDC          + 5,
    IHEVC_CAB_MVD_GRT0              = IHEVC_CAB_INTER_REF_IDX           + 2,
    IHEVC_CAB_MVD_GRT1              = IHEVC_CAB_MVD_GRT0                + 1,
    IHEVC_CAB_MVP_L0L1              = IHEVC_CAB_MVD_GRT1                + 1,
    IHEVC_CAB_NORES_IDX             = IHEVC_CAB_MVP_L0L1                + 1,
    IHEVC_CAB_SPLIT_TFM             = IHEVC_CAB_NORES_IDX               + 1,
    IHEVC_CAB_CBF_LUMA_IDX          = IHEVC_CAB_SPLIT_TFM               + 3,
    IHEVC_CAB_CBCR_IDX              = IHEVC_CAB_CBF_LUMA_IDX            + 2,
    IHEVC_CAB_TFM_SKIP0             = IHEVC_CAB_CBCR_IDX                + 4,
    IHEVC_CAB_TFM_SKIP12            = IHEVC_CAB_TFM_SKIP0               + 1,
    IHEVC_CAB_COEFFX_PREFIX         = IHEVC_CAB_TFM_SKIP12              + 1,
    IHEVC_CAB_COEFFY_PREFIX         = IHEVC_CAB_COEFFX_PREFIX           + 18,
    IHEVC_CAB_CODED_SUBLK_IDX       = IHEVC_CAB_COEFFY_PREFIX           + 18,
    IHEVC_CAB_COEFF_FLAG            = IHEVC_CAB_CODED_SUBLK_IDX         + 4,
    IHEVC_CAB_COEFABS_GRTR1_FLAG    = IHEVC_CAB_COEFF_FLAG              + 42,
    IHEVC_CAB_COEFABS_GRTR2_FLAG    = IHEVC_CAB_COEFABS_GRTR1_FLAG      + 24,
    IHEVC_CAB_CTXT_END              = IHEVC_CAB_COEFABS_GRTR2_FLAG      + 6
}IHEVC_CABAC_CTXT_OFFSETS;


/*****************************************************************************/
/* Extern global declarations                                                */
/*****************************************************************************/

/**
 ******************************************************************************
 * @brief  Table for rangeTabLPS depending on pStateIdx and qCodIRangeIdx
 * input   : pStateIdx(0-63) and qCodIRangeIdx(0-3) [(Range >> 6) & 0x3]
 * output  : RLps
 *
 * @remarks See Table 9-40 of HEVC spec for rangeTabLPS
 *******************************************************************************
 */
extern const UWORD8 gau1_ihevc_cabac_rlps[64][4];


/**
 ******************************************************************************
 * @brief  probaility+MPS state transition tables based on cur State and bin
 * input  : curpState[bits7-2]  | curMPS[bit1] | decodedBin[bit0]
 * output : nextpState[bits6-1] | nextMPS[bit0]
 * @remarks Modified form of Table-9-41 State Transition table in HEVC spec
 ******************************************************************************
 */
extern const UWORD8 gau1_ihevc_next_state[128*2];

/**
 ******************************************************************************
 * @brief  Init context tables for all combinations of qp and cabac_init_idc
 * @remarks Packing format MPS in lsb and pState in bits[1-6]
 ******************************************************************************
 */
extern const UWORD8 gau1_ihevc_cab_ctxts[IHEVC_NUM_CAB_IDC][IHEVC_MAX_QP][IHEVC_CAB_CTXT_END];



#endif /* _IHEVC_CABAC_TABLES_H_ */
