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
 *  ihevc_itrans_recon_16x16.c
 *
 * @brief
 *  Contains function definitions for inverse transform  and reconstruction 16x16
 *
 *
 * @author
 *  100470
 *
 * @par List of Functions:
 *  - ihevc_itrans_recon_16x16()
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
#include "ihevc_itrans_recon.h"
#include "ihevc_func_selector.h"
#include "ihevc_trans_macros.h"

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs Inverse transform  and reconstruction for 16x16
 * input block
 *
 * @par Description:
 *  Performs inverse transform and adds the prediction  data and clips output
 * to 8 bit
 *
 * @param[in] pi2_src
 *  Input 16x16 coefficients
 *
 * @param[in] pi2_tmp
 *  Temporary 16x16 buffer for storing inverse
 *
 *  transform
 *  1st stage output
 *
 * @param[in] pu1_pred
 *  Prediction 16x16 block
 *
 * @param[out] pu1_dst
 *  Output 16x16 block
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

void ihevc_itrans_recon_16x16(WORD16 *pi2_src,
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
    WORD32 e[8], o[8];
    WORD32 ee[4], eo[4];
    WORD32 eee[2], eeo[2];
    WORD32 add;
    WORD32 shift;
    WORD16 *pi2_tmp_orig;
    WORD32 trans_size;
    WORD32 zero_rows_2nd_stage = zero_cols;
    WORD32 row_limit_2nd_stage;

    if((zero_cols & 0xFFF0) == 0xFFF0)
        row_limit_2nd_stage = 4;
    else if((zero_cols & 0xFF00) == 0xFF00)
        row_limit_2nd_stage = 8;
    else
        row_limit_2nd_stage = TRANS_SIZE_16;

    trans_size = TRANS_SIZE_16;
    pi2_tmp_orig = pi2_tmp;
    if((zero_rows & 0xFFF0) == 0xFFF0)  /* First 4 rows of input are non-zero */
    {
        /* Inverse Transform 1st stage */
        /************************************************************************************************/
        /**********************************START - IT_RECON_16x16****************************************/
        /************************************************************************************************/

        shift = IT_SHIFT_STAGE_1;
        add = 1 << (shift - 1);

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
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_src[src_strd]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_src[3 * src_strd];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_src[2 * src_strd];
                }
                eeo[0] = 0;
                eee[0] = g_ai2_ihevc_trans_16[0][0] * pi2_src[0];
                eeo[1] = 0;
                eee[1] = g_ai2_ihevc_trans_16[0][1] * pi2_src[0];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    pi2_tmp[k] =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pi2_tmp[k + 8] =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
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

        if((zero_rows_2nd_stage & 0xFFF0) == 0xFFF0) /* First 4 rows of output of 1st stage are non-zero */
        {
            for(j = 0; j < trans_size; j++)
            {
                /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_tmp[trans_size]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_tmp[3 * trans_size];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_tmp[2 * trans_size];
                }
                eeo[0] = 0;
                eee[0] = g_ai2_ihevc_trans_16[0][0] * pi2_tmp[0];
                eeo[1] = 0;
                eee[1] = g_ai2_ihevc_trans_16[0][1] * pi2_tmp[0];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    WORD32 itrans_out;
                    itrans_out =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pu1_dst[k] = CLIP_U8((itrans_out + pu1_pred[k]));
                    itrans_out =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
                    pu1_dst[k + 8] = CLIP_U8((itrans_out + pu1_pred[k + 8]));
                }
                pi2_tmp++;
                pu1_pred += pred_strd;
                pu1_dst += dst_strd;
            }
        }
        else if((zero_rows_2nd_stage & 0xFF00) == 0xFF00) /* First 4 rows of output of 1st stage are non-zero */
        {
            for(j = 0; j < trans_size; j++)
            {
                /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_tmp[trans_size]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_tmp[3 * trans_size]
                                    + g_ai2_ihevc_trans_16[5][k]
                                                    * pi2_tmp[5 * trans_size]
                                    + g_ai2_ihevc_trans_16[7][k]
                                                    * pi2_tmp[7 * trans_size];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_tmp[2 * trans_size]
                                    + g_ai2_ihevc_trans_16[6][k]
                                                    * pi2_tmp[6 * trans_size];
                }
                eeo[0] = g_ai2_ihevc_trans_16[4][0] * pi2_tmp[4 * trans_size];
                eee[0] = g_ai2_ihevc_trans_16[0][0] * pi2_tmp[0];
                eeo[1] = g_ai2_ihevc_trans_16[4][1] * pi2_tmp[4 * trans_size];
                eee[1] = g_ai2_ihevc_trans_16[0][1] * pi2_tmp[0];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    WORD32 itrans_out;
                    itrans_out =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pu1_dst[k] = CLIP_U8((itrans_out + pu1_pred[k]));
                    itrans_out =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
                    pu1_dst[k + 8] = CLIP_U8((itrans_out + pu1_pred[k + 8]));
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
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_tmp[trans_size]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_tmp[3 * trans_size]
                                    + g_ai2_ihevc_trans_16[5][k]
                                                    * pi2_tmp[5 * trans_size]
                                    + g_ai2_ihevc_trans_16[7][k]
                                                    * pi2_tmp[7 * trans_size]
                                    + g_ai2_ihevc_trans_16[9][k]
                                                    * pi2_tmp[9 * trans_size]
                                    + g_ai2_ihevc_trans_16[11][k]
                                                    * pi2_tmp[11 * trans_size]
                                    + g_ai2_ihevc_trans_16[13][k]
                                                    * pi2_tmp[13 * trans_size]
                                    + g_ai2_ihevc_trans_16[15][k]
                                                    * pi2_tmp[15 * trans_size];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_tmp[2 * trans_size]
                                    + g_ai2_ihevc_trans_16[6][k]
                                                    * pi2_tmp[6 * trans_size]
                                    + g_ai2_ihevc_trans_16[10][k]
                                                    * pi2_tmp[10 * trans_size]
                                    + g_ai2_ihevc_trans_16[14][k]
                                                    * pi2_tmp[14 * trans_size];
                }
                eeo[0] =
                                g_ai2_ihevc_trans_16[4][0] * pi2_tmp[4 * trans_size]
                                                + g_ai2_ihevc_trans_16[12][0]
                                                                * pi2_tmp[12
                                                                                * trans_size];
                eee[0] = g_ai2_ihevc_trans_16[0][0] * pi2_tmp[0]
                                + g_ai2_ihevc_trans_16[8][0] * pi2_tmp[8 * trans_size];
                eeo[1] =
                                g_ai2_ihevc_trans_16[4][1] * pi2_tmp[4 * trans_size]
                                                + g_ai2_ihevc_trans_16[12][1]
                                                                * pi2_tmp[12
                                                                                * trans_size];
                eee[1] = g_ai2_ihevc_trans_16[0][1] * pi2_tmp[0]
                                + g_ai2_ihevc_trans_16[8][1] * pi2_tmp[8 * trans_size];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    WORD32 itrans_out;
                    itrans_out =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pu1_dst[k] = CLIP_U8((itrans_out + pu1_pred[k]));
                    itrans_out =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
                    pu1_dst[k + 8] = CLIP_U8((itrans_out + pu1_pred[k + 8]));
                }
                pi2_tmp++;
                pu1_pred += pred_strd;
                pu1_dst += dst_strd;
            }
        }
        /************************************************************************************************/
        /************************************END - IT_RECON_16x16****************************************/
        /************************************************************************************************/
    }
    else if((zero_rows & 0xFF00) == 0xFF00)  /* First 8 rows of input are non-zero */
    {
        /* Inverse Transform 1st stage */
        /************************************************************************************************/
        /**********************************START - IT_RECON_16x16****************************************/
        /************************************************************************************************/

        shift = IT_SHIFT_STAGE_1;
        add = 1 << (shift - 1);

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
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_src[src_strd]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_src[3 * src_strd]
                                    + g_ai2_ihevc_trans_16[5][k]
                                                    * pi2_src[5 * src_strd]
                                    + g_ai2_ihevc_trans_16[7][k]
                                                    * pi2_src[7 * src_strd];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_src[2 * src_strd]
                                    + g_ai2_ihevc_trans_16[6][k]
                                                    * pi2_src[6 * src_strd];
                }
                eeo[0] = g_ai2_ihevc_trans_16[4][0] * pi2_src[4 * src_strd];
                eee[0] = g_ai2_ihevc_trans_16[0][0] * pi2_src[0];
                eeo[1] = g_ai2_ihevc_trans_16[4][1] * pi2_src[4 * src_strd];
                eee[1] = g_ai2_ihevc_trans_16[0][1] * pi2_src[0];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    pi2_tmp[k] =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pi2_tmp[k + 8] =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
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

        if((zero_rows_2nd_stage & 0xFFF0) == 0xFFF0) /* First 4 rows of output of 1st stage are non-zero */
        {
            for(j = 0; j < trans_size; j++)
            {
                /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_tmp[trans_size]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_tmp[3 * trans_size];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_tmp[2 * trans_size];
                }
                eeo[0] = 0;
                eee[0] = g_ai2_ihevc_trans_16[0][0] * pi2_tmp[0];
                eeo[1] = 0;
                eee[1] = g_ai2_ihevc_trans_16[0][1] * pi2_tmp[0];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    WORD32 itrans_out;
                    itrans_out =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pu1_dst[k] = CLIP_U8((itrans_out + pu1_pred[k]));
                    itrans_out =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
                    pu1_dst[k + 8] = CLIP_U8((itrans_out + pu1_pred[k + 8]));
                }
                pi2_tmp++;
                pu1_pred += pred_strd;
                pu1_dst += dst_strd;
            }
        }
        else if((zero_rows_2nd_stage & 0xFF00) == 0xFF00) /* First 4 rows of output of 1st stage are non-zero */
        {
            for(j = 0; j < trans_size; j++)
            {
                /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_tmp[trans_size]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_tmp[3 * trans_size]
                                    + g_ai2_ihevc_trans_16[5][k]
                                                    * pi2_tmp[5 * trans_size]
                                    + g_ai2_ihevc_trans_16[7][k]
                                                    * pi2_tmp[7 * trans_size];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_tmp[2 * trans_size]
                                    + g_ai2_ihevc_trans_16[6][k]
                                                    * pi2_tmp[6 * trans_size];
                }
                eeo[0] = g_ai2_ihevc_trans_16[4][0] * pi2_tmp[4 * trans_size];
                eee[0] = g_ai2_ihevc_trans_16[0][0] * pi2_tmp[0];
                eeo[1] = g_ai2_ihevc_trans_16[4][1] * pi2_tmp[4 * trans_size];
                eee[1] = g_ai2_ihevc_trans_16[0][1] * pi2_tmp[0];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    WORD32 itrans_out;
                    itrans_out =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pu1_dst[k] = CLIP_U8((itrans_out + pu1_pred[k]));
                    itrans_out =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
                    pu1_dst[k + 8] = CLIP_U8((itrans_out + pu1_pred[k + 8]));
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
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_tmp[trans_size]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_tmp[3 * trans_size]
                                    + g_ai2_ihevc_trans_16[5][k]
                                                    * pi2_tmp[5 * trans_size]
                                    + g_ai2_ihevc_trans_16[7][k]
                                                    * pi2_tmp[7 * trans_size]
                                    + g_ai2_ihevc_trans_16[9][k]
                                                    * pi2_tmp[9 * trans_size]
                                    + g_ai2_ihevc_trans_16[11][k]
                                                    * pi2_tmp[11 * trans_size]
                                    + g_ai2_ihevc_trans_16[13][k]
                                                    * pi2_tmp[13 * trans_size]
                                    + g_ai2_ihevc_trans_16[15][k]
                                                    * pi2_tmp[15 * trans_size];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_tmp[2 * trans_size]
                                    + g_ai2_ihevc_trans_16[6][k]
                                                    * pi2_tmp[6 * trans_size]
                                    + g_ai2_ihevc_trans_16[10][k]
                                                    * pi2_tmp[10 * trans_size]
                                    + g_ai2_ihevc_trans_16[14][k]
                                                    * pi2_tmp[14 * trans_size];
                }
                eeo[0] =
                                g_ai2_ihevc_trans_16[4][0] * pi2_tmp[4 * trans_size]
                                                + g_ai2_ihevc_trans_16[12][0]
                                                                * pi2_tmp[12
                                                                                * trans_size];
                eee[0] = g_ai2_ihevc_trans_16[0][0] * pi2_tmp[0]
                                + g_ai2_ihevc_trans_16[8][0] * pi2_tmp[8 * trans_size];
                eeo[1] =
                                g_ai2_ihevc_trans_16[4][1] * pi2_tmp[4 * trans_size]
                                                + g_ai2_ihevc_trans_16[12][1]
                                                                * pi2_tmp[12
                                                                                * trans_size];
                eee[1] = g_ai2_ihevc_trans_16[0][1] * pi2_tmp[0]
                                + g_ai2_ihevc_trans_16[8][1] * pi2_tmp[8 * trans_size];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    WORD32 itrans_out;
                    itrans_out =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pu1_dst[k] = CLIP_U8((itrans_out + pu1_pred[k]));
                    itrans_out =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
                    pu1_dst[k + 8] = CLIP_U8((itrans_out + pu1_pred[k + 8]));
                }
                pi2_tmp++;
                pu1_pred += pred_strd;
                pu1_dst += dst_strd;
            }
        }
        /************************************************************************************************/
        /************************************END - IT_RECON_16x16****************************************/
        /************************************************************************************************/
    }
    else  /* All rows of input are non-zero */
    {
        /* Inverse Transform 1st stage */
        /************************************************************************************************/
        /**********************************START - IT_RECON_16x16****************************************/
        /************************************************************************************************/

        shift = IT_SHIFT_STAGE_1;
        add = 1 << (shift - 1);

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
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_src[src_strd]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_src[3 * src_strd]
                                    + g_ai2_ihevc_trans_16[5][k]
                                                    * pi2_src[5 * src_strd]
                                    + g_ai2_ihevc_trans_16[7][k]
                                                    * pi2_src[7 * src_strd]
                                    + g_ai2_ihevc_trans_16[9][k]
                                                    * pi2_src[9 * src_strd]
                                    + g_ai2_ihevc_trans_16[11][k]
                                                    * pi2_src[11 * src_strd]
                                    + g_ai2_ihevc_trans_16[13][k]
                                                    * pi2_src[13 * src_strd]
                                    + g_ai2_ihevc_trans_16[15][k]
                                                    * pi2_src[15 * src_strd];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_src[2 * src_strd]
                                    + g_ai2_ihevc_trans_16[6][k]
                                                    * pi2_src[6 * src_strd]
                                    + g_ai2_ihevc_trans_16[10][k]
                                                    * pi2_src[10 * src_strd]
                                    + g_ai2_ihevc_trans_16[14][k]
                                                    * pi2_src[14 * src_strd];
                }
                eeo[0] = g_ai2_ihevc_trans_16[4][0] * pi2_src[4 * src_strd]
                                + g_ai2_ihevc_trans_16[12][0]
                                                * pi2_src[12 * src_strd];
                eee[0] =
                                g_ai2_ihevc_trans_16[0][0] * pi2_src[0]
                                                + g_ai2_ihevc_trans_16[8][0]
                                                                * pi2_src[8
                                                                                * src_strd];
                eeo[1] = g_ai2_ihevc_trans_16[4][1] * pi2_src[4 * src_strd]
                                + g_ai2_ihevc_trans_16[12][1]
                                                * pi2_src[12 * src_strd];
                eee[1] =
                                g_ai2_ihevc_trans_16[0][1] * pi2_src[0]
                                                + g_ai2_ihevc_trans_16[8][1]
                                                                * pi2_src[8
                                                                                * src_strd];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    pi2_tmp[k] =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pi2_tmp[k + 8] =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
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

        if((zero_rows_2nd_stage & 0xFFF0) == 0xFFF0) /* First 4 rows of output of 1st stage are non-zero */
        {
            for(j = 0; j < trans_size; j++)
            {
                /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_tmp[trans_size]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_tmp[3 * trans_size];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_tmp[2 * trans_size];
                }
                eeo[0] = 0;
                eee[0] = g_ai2_ihevc_trans_16[0][0] * pi2_tmp[0];
                eeo[1] = 0;
                eee[1] = g_ai2_ihevc_trans_16[0][1] * pi2_tmp[0];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    WORD32 itrans_out;
                    itrans_out =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pu1_dst[k] = CLIP_U8((itrans_out + pu1_pred[k]));
                    itrans_out =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
                    pu1_dst[k + 8] = CLIP_U8((itrans_out + pu1_pred[k + 8]));
                }
                pi2_tmp++;
                pu1_pred += pred_strd;
                pu1_dst += dst_strd;
            }
        }
        else if((zero_rows_2nd_stage & 0xFF00) == 0xFF00) /* First 4 rows of output of 1st stage are non-zero */
        {
            for(j = 0; j < trans_size; j++)
            {
                /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_tmp[trans_size]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_tmp[3 * trans_size]
                                    + g_ai2_ihevc_trans_16[5][k]
                                                    * pi2_tmp[5 * trans_size]
                                    + g_ai2_ihevc_trans_16[7][k]
                                                    * pi2_tmp[7 * trans_size];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_tmp[2 * trans_size]
                                    + g_ai2_ihevc_trans_16[6][k]
                                                    * pi2_tmp[6 * trans_size];
                }
                eeo[0] = g_ai2_ihevc_trans_16[4][0] * pi2_tmp[4 * trans_size];
                eee[0] = g_ai2_ihevc_trans_16[0][0] * pi2_tmp[0];
                eeo[1] = g_ai2_ihevc_trans_16[4][1] * pi2_tmp[4 * trans_size];
                eee[1] = g_ai2_ihevc_trans_16[0][1] * pi2_tmp[0];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    WORD32 itrans_out;
                    itrans_out =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pu1_dst[k] = CLIP_U8((itrans_out + pu1_pred[k]));
                    itrans_out =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
                    pu1_dst[k + 8] = CLIP_U8((itrans_out + pu1_pred[k + 8]));
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
                for(k = 0; k < 8; k++)
                {
                    o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_tmp[trans_size]
                                    + g_ai2_ihevc_trans_16[3][k]
                                                    * pi2_tmp[3 * trans_size]
                                    + g_ai2_ihevc_trans_16[5][k]
                                                    * pi2_tmp[5 * trans_size]
                                    + g_ai2_ihevc_trans_16[7][k]
                                                    * pi2_tmp[7 * trans_size]
                                    + g_ai2_ihevc_trans_16[9][k]
                                                    * pi2_tmp[9 * trans_size]
                                    + g_ai2_ihevc_trans_16[11][k]
                                                    * pi2_tmp[11 * trans_size]
                                    + g_ai2_ihevc_trans_16[13][k]
                                                    * pi2_tmp[13 * trans_size]
                                    + g_ai2_ihevc_trans_16[15][k]
                                                    * pi2_tmp[15 * trans_size];
                }
                for(k = 0; k < 4; k++)
                {
                    eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_tmp[2 * trans_size]
                                    + g_ai2_ihevc_trans_16[6][k]
                                                    * pi2_tmp[6 * trans_size]
                                    + g_ai2_ihevc_trans_16[10][k]
                                                    * pi2_tmp[10 * trans_size]
                                    + g_ai2_ihevc_trans_16[14][k]
                                                    * pi2_tmp[14 * trans_size];
                }
                eeo[0] =
                                g_ai2_ihevc_trans_16[4][0] * pi2_tmp[4 * trans_size]
                                                + g_ai2_ihevc_trans_16[12][0]
                                                                * pi2_tmp[12
                                                                                * trans_size];
                eee[0] = g_ai2_ihevc_trans_16[0][0] * pi2_tmp[0]
                                + g_ai2_ihevc_trans_16[8][0] * pi2_tmp[8 * trans_size];
                eeo[1] =
                                g_ai2_ihevc_trans_16[4][1] * pi2_tmp[4 * trans_size]
                                                + g_ai2_ihevc_trans_16[12][1]
                                                                * pi2_tmp[12
                                                                                * trans_size];
                eee[1] = g_ai2_ihevc_trans_16[0][1] * pi2_tmp[0]
                                + g_ai2_ihevc_trans_16[8][1] * pi2_tmp[8 * trans_size];

                /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
                for(k = 0; k < 2; k++)
                {
                    ee[k] = eee[k] + eeo[k];
                    ee[k + 2] = eee[1 - k] - eeo[1 - k];
                }
                for(k = 0; k < 4; k++)
                {
                    e[k] = ee[k] + eo[k];
                    e[k + 4] = ee[3 - k] - eo[3 - k];
                }
                for(k = 0; k < 8; k++)
                {
                    WORD32 itrans_out;
                    itrans_out =
                                    CLIP_S16(((e[k] + o[k] + add) >> shift));
                    pu1_dst[k] = CLIP_U8((itrans_out + pu1_pred[k]));
                    itrans_out =
                                    CLIP_S16(((e[7 - k] - o[7 - k] + add) >> shift));
                    pu1_dst[k + 8] = CLIP_U8((itrans_out + pu1_pred[k + 8]));
                }
                pi2_tmp++;
                pu1_pred += pred_strd;
                pu1_dst += dst_strd;
            }
        }
        /************************************************************************************************/
        /************************************END - IT_RECON_16x16****************************************/
        /************************************************************************************************/
    }

}

