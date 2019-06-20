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
 *  ihevc_chroma_itrans_recon_8x8.c
 *
 * @brief
 *  Contains function definitions for 8x8 inverse transform  and reconstruction
 * of chroma interleaved data.
 *
 * @author
 *  100470
 *
 * @par List of Functions:
 *  - ihevc_chroma_itrans_recon_8x8()
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include "ihevc_typedefs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_defs.h"
#include "ihevc_trans_tables.h"
#include "ihevc_chroma_itrans_recon.h"
#include "ihevc_func_selector.h"
#include "ihevc_trans_macros.h"

/* All the functions work one component(U or V) of interleaved data depending upon pointers passed to it */
/* Data visualization */
/* U V U V U V U V */
/* U V U V U V U V */
/* U V U V U V U V */
/* U V U V U V U V */
/* If the pointer points to first byte of above stream (U) , functions will operate on U component */
/* If the pointer points to second byte of above stream (V) , functions will operate on V component */

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs Inverse transform  and reconstruction for 8x8
 * input block
 *
 * @par Description:
 *  Performs inverse transform and adds the prediction  data and clips output
 * to 8 bit
 *
 * @param[in] pi2_src
 *  Input 8x8 coefficients
 *
 * @param[in] pi2_tmp
 *  Temporary 8x8 buffer for storing inverse transform
 *  1st stage output
 *
 * @param[in] pu1_pred
 *  Prediction 8x8 block
 *
 * @param[out] pu1_dst
 *  Output 8x8 block
 *
 * @param[in] src_strd
 *  Input stride
 *
 * @param[in] pred_strd
 *  Prediction stride
 *
 * @param[in] dst_strd
 *  Output Stride
 *
 * @param[in] shift
 *  Output shift
 *
 * @param[in] zero_cols
 *  Zero columns in pi2_src
 *
 * @returns  Void
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */


void ihevc_chroma_itrans_recon_8x8(WORD16 *pi2_src,
                                   WORD16 *pi2_tmp,
                                   UWORD8 *pu1_pred,
                                   UWORD8 *pu1_dst,
                                   WORD32 src_strd,
                                   WORD32 pred_strd,
                                   WORD32 dst_strd,
                                   WORD32 zero_cols,
                                   WORD32 zero_rows)
{
    WORD32 j, k;
    WORD32 e[4], o[4];
    WORD32 ee[2], eo[2];
    WORD32 add;
    WORD32 shift;
    WORD16 *pi2_tmp_orig;
    WORD32 trans_size;
    WORD32 zero_rows_2nd_stage = zero_cols;
    WORD32 row_limit_2nd_stage;
    UNUSED(zero_rows);
    trans_size = TRANS_SIZE_8;

    pi2_tmp_orig = pi2_tmp;

    if((zero_cols & 0xF0) == 0xF0)
        row_limit_2nd_stage = 4;
    else
        row_limit_2nd_stage = TRANS_SIZE_8;

    /* Inverse Transform 1st stage */
    shift = IT_SHIFT_STAGE_1;
    add = 1 << (shift - 1);
    {
        /************************************************************************************************/
        /**********************************START - IT_RECON_8x8******************************************/
        /************************************************************************************************/

        for(j = 0; j < row_limit_2nd_stage; j++)
        {
            /* Checking for Zero Cols */
            if((zero_cols & 1) == 1)
            {
                memset(pi2_tmp, 0, trans_size * sizeof(WORD16));
            }
            else
            {
                /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
                for(k = 0; k < 4; k++)
                {
                    o[k] = g_ai2_ihevc_trans_8[1][k] * pi2_src[src_strd]
                                    + g_ai2_ihevc_trans_8[3][k]
                                                    * pi2_src[3 * src_strd]
                                    + g_ai2_ihevc_trans_8[5][k]
                                                    * pi2_src[5 * src_strd]
                                    + g_ai2_ihevc_trans_8[7][k]
                                                    * pi2_src[7 * src_strd];
                }

                eo[0] = g_ai2_ihevc_trans_8[2][0] * pi2_src[2 * src_strd]
                                + g_ai2_ihevc_trans_8[6][0] * pi2_src[6 * src_strd];
                eo[1] = g_ai2_ihevc_trans_8[2][1] * pi2_src[2 * src_strd]
                                + g_ai2_ihevc_trans_8[6][1] * pi2_src[6 * src_strd];
                ee[0] = g_ai2_ihevc_trans_8[0][0] * pi2_src[0]
                                + g_ai2_ihevc_trans_8[4][0] * pi2_src[4 * src_strd];
                ee[1] = g_ai2_ihevc_trans_8[0][1] * pi2_src[0]
                                + g_ai2_ihevc_trans_8[4][1] * pi2_src[4 * src_strd];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                e[0] = ee[0] + eo[0];
                e[3] = ee[0] - eo[0];
                e[1] = ee[1] + eo[1];
                e[2] = ee[1] - eo[1];
                for(k = 0; k < 4; k++)
                {
                    pi2_tmp[k] =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pi2_tmp[k + 4] =
                                    CLIP_S16(((e[3 - k] - o[3 - k] + add) >> shift));
                }
            }
            pi2_src++;
            pi2_tmp += trans_size;
            zero_cols = zero_cols >> 1;
        }

        pi2_tmp = pi2_tmp_orig;

        /* Inverse Transform 2nd stage */
        shift = IT_SHIFT_STAGE_2;
        add = 1 << (shift - 1);

        if((zero_rows_2nd_stage & 0xF0) == 0xF0) /* First 4 rows of output of 1st stage are non-zero */
        {
            for(j = 0; j < trans_size; j++)
            {
                /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
                for(k = 0; k < 4; k++)
                {
                    o[k] = g_ai2_ihevc_trans_8[1][k] * pi2_tmp[trans_size]
                                    + g_ai2_ihevc_trans_8[3][k]
                                                    * pi2_tmp[3 * trans_size];
                }
                eo[0] = g_ai2_ihevc_trans_8[2][0] * pi2_tmp[2 * trans_size];
                eo[1] = g_ai2_ihevc_trans_8[2][1] * pi2_tmp[2 * trans_size];
                ee[0] = g_ai2_ihevc_trans_8[0][0] * pi2_tmp[0];
                ee[1] = g_ai2_ihevc_trans_8[0][1] * pi2_tmp[0];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                e[0] = ee[0] + eo[0];
                e[3] = ee[0] - eo[0];
                e[1] = ee[1] + eo[1];
                e[2] = ee[1] - eo[1];
                for(k = 0; k < 4; k++)
                {
                    WORD32 itrans_out;
                    itrans_out =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pu1_dst[k * 2] = CLIP_U8((itrans_out + pu1_pred[k * 2]));
                    itrans_out =
                                    CLIP_S16(((e[3 - k] - o[3 - k] + add) >> shift));
                    pu1_dst[(k + 4) * 2] =
                                    CLIP_U8((itrans_out + pu1_pred[(k + 4) * 2]));
                }
                pi2_tmp++;
                pu1_pred += pred_strd;
                pu1_dst += dst_strd;
            }
        }
        else /* All rows of output of 1st stage are non-zero */
        {
            for(j = 0; j < trans_size; j++)
            {
                /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
                for(k = 0; k < 4; k++)
                {
                    o[k] = g_ai2_ihevc_trans_8[1][k] * pi2_tmp[trans_size]
                                    + g_ai2_ihevc_trans_8[3][k]
                                                    * pi2_tmp[3 * trans_size]
                                    + g_ai2_ihevc_trans_8[5][k]
                                                    * pi2_tmp[5 * trans_size]
                                    + g_ai2_ihevc_trans_8[7][k]
                                                    * pi2_tmp[7 * trans_size];
                }

                eo[0] = g_ai2_ihevc_trans_8[2][0] * pi2_tmp[2 * trans_size]
                                + g_ai2_ihevc_trans_8[6][0] * pi2_tmp[6 * trans_size];
                eo[1] = g_ai2_ihevc_trans_8[2][1] * pi2_tmp[2 * trans_size]
                                + g_ai2_ihevc_trans_8[6][1] * pi2_tmp[6 * trans_size];
                ee[0] = g_ai2_ihevc_trans_8[0][0] * pi2_tmp[0]
                                + g_ai2_ihevc_trans_8[4][0] * pi2_tmp[4 * trans_size];
                ee[1] = g_ai2_ihevc_trans_8[0][1] * pi2_tmp[0]
                                + g_ai2_ihevc_trans_8[4][1] * pi2_tmp[4 * trans_size];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                e[0] = ee[0] + eo[0];
                e[3] = ee[0] - eo[0];
                e[1] = ee[1] + eo[1];
                e[2] = ee[1] - eo[1];
                for(k = 0; k < 4; k++)
                {
                    WORD32 itrans_out;
                    itrans_out =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pu1_dst[k * 2] = CLIP_U8((itrans_out + pu1_pred[k * 2]));
                    itrans_out =
                                    CLIP_S16(((e[3 - k] - o[3 - k] + add) >> shift));
                    pu1_dst[(k + 4) * 2] =
                                    CLIP_U8((itrans_out + pu1_pred[(k + 4) * 2]));
                }
                pi2_tmp++;
                pu1_pred += pred_strd;
                pu1_dst += dst_strd;
            }
        }
        /************************************************************************************************/
        /************************************END - IT_RECON_8x8******************************************/
        /************************************************************************************************/
    }
}
