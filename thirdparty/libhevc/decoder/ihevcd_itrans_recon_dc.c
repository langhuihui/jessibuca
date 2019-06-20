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
 *  ihevcd_itrans_recon_dc.c
 *
 * @brief
 *  Contains functions for DC inverse transform and reconstruction
 *
 * @author
 *  Ittiam
 *
 * @par List of Functions:
 * - ihevcd_itrans_recon_dc_luma()
 * - ihevcd_itrans_recon_dc_chroma()
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

#include "ihevc_typedefs.h"
#include "iv.h"
#include "ivd.h"
#include "ihevcd_cxa.h"

#include "ihevc_defs.h"
#include "ihevc_debug.h"
#include "ihevc_structs.h"
#include "ihevc_cabac_tables.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"

#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_bitstream.h"
#include "ihevc_common_tables.h"

/* Intra pred includes */
#include "ihevc_intra_pred.h"

/* Inverse transform common module includes */
#include "ihevc_trans_tables.h"
#include "ihevc_trans_macros.h"
#include "ihevc_itrans_recon.h"
#include "ihevc_recon.h"
#include "ihevc_chroma_itrans_recon.h"
#include "ihevc_chroma_recon.h"

/* Decoder includes */
#include "ihevcd_common_tables.h"
#include "ihevcd_iquant_itrans_recon_ctb.h"
#include "ihevcd_debug.h"
#include "ihevcd_profile.h"
#include "ihevcd_statistics.h"
#include "ihevcd_itrans_recon_dc.h"



void ihevcd_itrans_recon_dc_luma(UWORD8 *pu1_pred,
                                 UWORD8 *pu1_dst,
                                 WORD32 pred_strd,
                                 WORD32 dst_strd,
                                 WORD32 log2_trans_size,
                                 WORD16 i2_coeff_value)
{
    WORD32 row, col;
    WORD32 add, shift;
    WORD32 dc_value, quant_out;
    WORD32 trans_size;

    trans_size = (1 << log2_trans_size);

    quant_out = i2_coeff_value;

    shift = IT_SHIFT_STAGE_1;
    add = 1 << (shift - 1);
    dc_value = CLIP_S16((quant_out * 64 + add) >> shift);
    shift = IT_SHIFT_STAGE_2;
    add = 1 << (shift - 1);
    dc_value = CLIP_S16((dc_value * 64 + add) >> shift);

    for(row = 0; row < trans_size; row++)
        for(col = 0; col < trans_size; col++)
            pu1_dst[row * dst_strd + col] = CLIP_U8((pu1_pred[row * pred_strd + col] + dc_value));

}


void ihevcd_itrans_recon_dc_chroma(UWORD8 *pu1_pred,
                                   UWORD8 *pu1_dst,
                                   WORD32 pred_strd,
                                   WORD32 dst_strd,
                                   WORD32 log2_trans_size,
                                   WORD16 i2_coeff_value)
{
    WORD32 row, col;
    WORD32 add, shift;
    WORD32 dc_value, quant_out;
    WORD32 trans_size;


    trans_size = (1 << log2_trans_size);

    quant_out = i2_coeff_value;

    shift = IT_SHIFT_STAGE_1;
    add = 1 << (shift - 1);
    dc_value = CLIP_S16((quant_out * 64 + add) >> shift);
    shift = IT_SHIFT_STAGE_2;
    add = 1 << (shift - 1);
    dc_value = CLIP_S16((dc_value * 64 + add) >> shift);

    for(row = 0; row < trans_size; row++)
        for(col = 0; col < trans_size; col++)
            pu1_dst[row * dst_strd + (col << 1)] = CLIP_U8((pu1_pred[row * pred_strd + (col << 1)] + dc_value));

}


