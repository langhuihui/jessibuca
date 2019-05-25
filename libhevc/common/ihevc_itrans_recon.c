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
 *  ihevc_itrans_recon.c
 *
 * @brief
 *  Contains function definitions for inverse transform  and reconstruction
 *
 *
 * @author
 *  100470
 *
 * @par List of Functions:
 *  - ihevc_itrans_recon_4x4_ttype1()
 *  - ihevc_itrans_recon_4x4()
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

/* All the functions here are replicated from ihevc_itrans.c and modified to */
/* include reconstruction */

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs Inverse transform type 1 (DST)  and reconstruction
 * for 4x4 input block
 *
 * @par Description:
 *  Performs inverse transform and adds the prediction  data and clips output
 * to 8 bit
 *
 * @param[in] pi2_src
 *  Input 4x4 coefficients
 *
 * @param[in] pi2_tmp
 *  Temporary 4x4 buffer for storing inverse
 *
 *  transform
 *  1st stage output
 *
 * @param[in] pu1_pred
 *  Prediction 4x4 block
 *
 * @param[out] pu1_dst
 *  Output 4x4 block
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

void ihevc_itrans_recon_4x4_ttype1(WORD16 *pi2_src,
                                   WORD16 *pi2_tmp,
                                   UWORD8 *pu1_pred,
                                   UWORD8 *pu1_dst,
                                   WORD32 src_strd,
                                   WORD32 pred_strd,
                                   WORD32 dst_strd,
                                   WORD32 zero_cols,
                                   WORD32 zero_rows)
{
    WORD32 i, c[4];
    WORD32 add;
    WORD32 shift;
    WORD16 *pi2_tmp_orig;
    WORD32 trans_size;
    UNUSED(zero_rows);
    trans_size = TRANS_SIZE_4;

    pi2_tmp_orig = pi2_tmp;

    /* Inverse Transform 1st stage */
    shift = IT_SHIFT_STAGE_1;
    add = 1 << (shift - 1);

    for(i = 0; i < trans_size; i++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            memset(pi2_tmp, 0, trans_size * sizeof(WORD16));
        }
        else
        {
            // Intermediate Variables
            c[0] = pi2_src[0] + pi2_src[2 * src_strd];
            c[1] = pi2_src[2 * src_strd] + pi2_src[3 * src_strd];
            c[2] = pi2_src[0] - pi2_src[3 * src_strd];
            c[3] = 74 * pi2_src[src_strd];

            pi2_tmp[0] =
                            CLIP_S16((29 * c[0] + 55 * c[1] + c[3] + add) >> shift);
            pi2_tmp[1] =
                            CLIP_S16((55 * c[2] - 29 * c[1] + c[3] + add) >> shift);
            pi2_tmp[2] =
                            CLIP_S16((74 * (pi2_src[0] - pi2_src[2 * src_strd] + pi2_src[3 * src_strd]) + add) >> shift);
            pi2_tmp[3] =
                            CLIP_S16((55 * c[0] + 29 * c[2] - c[3] + add) >> shift);
        }
        pi2_src++;
        pi2_tmp += trans_size;
        zero_cols = zero_cols >> 1;
    }

    pi2_tmp = pi2_tmp_orig;

    /* Inverse Transform 2nd stage */
    shift = IT_SHIFT_STAGE_2;
    add = 1 << (shift - 1);

    for(i = 0; i < trans_size; i++)
    {
        WORD32 itrans_out;
        // Intermediate Variables
        c[0] = pi2_tmp[0] + pi2_tmp[2 * trans_size];
        c[1] = pi2_tmp[2 * trans_size] + pi2_tmp[3 * trans_size];
        c[2] = pi2_tmp[0] - pi2_tmp[3 * trans_size];
        c[3] = 74 * pi2_tmp[trans_size];

        itrans_out =
                        CLIP_S16((29 * c[0] + 55 * c[1] + c[3] + add) >> shift);
        pu1_dst[0] = CLIP_U8((itrans_out + pu1_pred[0]));
        itrans_out =
                        CLIP_S16((55 * c[2] - 29 * c[1] + c[3] + add) >> shift);
        pu1_dst[1] = CLIP_U8((itrans_out + pu1_pred[1]));
        itrans_out =
                        CLIP_S16((74 * (pi2_tmp[0] - pi2_tmp[2 * trans_size] + pi2_tmp[3 * trans_size]) + add) >> shift);
        pu1_dst[2] = CLIP_U8((itrans_out + pu1_pred[2]));
        itrans_out =
                        CLIP_S16((55 * c[0] + 29 * c[2] - c[3] + add) >> shift);
        pu1_dst[3] = CLIP_U8((itrans_out + pu1_pred[3]));
        pi2_tmp++;
        pu1_pred += pred_strd;
        pu1_dst += dst_strd;
    }
}

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs Inverse transform  and reconstruction for 4x4
 * input block
 *
 * @par Description:
 *  Performs inverse transform and adds the prediction  data and clips output
 * to 8 bit
 *
 * @param[in] pi2_src
 *  Input 4x4 coefficients
 *
 * @param[in] pi2_tmp
 *  Temporary 4x4 buffer for storing inverse
 *
 *  transform
 *  1st stage output
 *
 * @param[in] pu1_pred
 *  Prediction 4x4 block
 *
 * @param[out] pu1_dst
 *  Output 4x4 block
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

void ihevc_itrans_recon_4x4(WORD16 *pi2_src,
                            WORD16 *pi2_tmp,
                            UWORD8 *pu1_pred,
                            UWORD8 *pu1_dst,
                            WORD32 src_strd,
                            WORD32 pred_strd,
                            WORD32 dst_strd,
                            WORD32 zero_cols,
                            WORD32 zero_rows)

{
    WORD32 j;
    WORD32 e[2], o[2];
    WORD32 add;
    WORD32 shift;
    WORD16 *pi2_tmp_orig;
    WORD32 trans_size;
    UNUSED(zero_rows);
    trans_size = TRANS_SIZE_4;

    pi2_tmp_orig = pi2_tmp;

    /* Inverse Transform 1st stage */
    shift = IT_SHIFT_STAGE_1;
    add = 1 << (shift - 1);

    for(j = 0; j < trans_size; j++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            memset(pi2_tmp, 0, trans_size * sizeof(WORD16));
        }
        else
        {

            /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
            o[0] = g_ai2_ihevc_trans_4[1][0] * pi2_src[src_strd]
                            + g_ai2_ihevc_trans_4[3][0] * pi2_src[3 * src_strd];
            o[1] = g_ai2_ihevc_trans_4[1][1] * pi2_src[src_strd]
                            + g_ai2_ihevc_trans_4[3][1] * pi2_src[3 * src_strd];
            e[0] = g_ai2_ihevc_trans_4[0][0] * pi2_src[0]
                            + g_ai2_ihevc_trans_4[2][0] * pi2_src[2 * src_strd];
            e[1] = g_ai2_ihevc_trans_4[0][1] * pi2_src[0]
                            + g_ai2_ihevc_trans_4[2][1] * pi2_src[2 * src_strd];

            pi2_tmp[0] =
                            CLIP_S16(((e[0] + o[0] + add) >> shift));
            pi2_tmp[1] =
                            CLIP_S16(((e[1] + o[1] + add) >> shift));
            pi2_tmp[2] =
                            CLIP_S16(((e[1] - o[1] + add) >> shift));
            pi2_tmp[3] =
                            CLIP_S16(((e[0] - o[0] + add) >> shift));

        }
        pi2_src++;
        pi2_tmp += trans_size;
        zero_cols = zero_cols >> 1;
    }

    pi2_tmp = pi2_tmp_orig;

    /* Inverse Transform 2nd stage */
    shift = IT_SHIFT_STAGE_2;
    add = 1 << (shift - 1);

    for(j = 0; j < trans_size; j++)
    {
        WORD32 itrans_out;
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        o[0] = g_ai2_ihevc_trans_4[1][0] * pi2_tmp[trans_size]
                        + g_ai2_ihevc_trans_4[3][0] * pi2_tmp[3 * trans_size];
        o[1] = g_ai2_ihevc_trans_4[1][1] * pi2_tmp[trans_size]
                        + g_ai2_ihevc_trans_4[3][1] * pi2_tmp[3 * trans_size];
        e[0] = g_ai2_ihevc_trans_4[0][0] * pi2_tmp[0]
                        + g_ai2_ihevc_trans_4[2][0] * pi2_tmp[2 * trans_size];
        e[1] = g_ai2_ihevc_trans_4[0][1] * pi2_tmp[0]
                        + g_ai2_ihevc_trans_4[2][1] * pi2_tmp[2 * trans_size];

        itrans_out =
                        CLIP_S16(((e[0] + o[0] + add) >> shift));
        pu1_dst[0] = CLIP_U8((itrans_out + pu1_pred[0]));
        itrans_out =
                        CLIP_S16(((e[1] + o[1] + add) >> shift));
        pu1_dst[1] = CLIP_U8((itrans_out + pu1_pred[1]));
        itrans_out =
                        CLIP_S16(((e[1] - o[1] + add) >> shift));
        pu1_dst[2] = CLIP_U8((itrans_out + pu1_pred[2]));
        itrans_out =
                        CLIP_S16(((e[0] - o[0] + add) >> shift));
        pu1_dst[3] = CLIP_U8((itrans_out + pu1_pred[3]));

        pi2_tmp++;
        pu1_pred += pred_strd;
        pu1_dst += dst_strd;

    }
}

