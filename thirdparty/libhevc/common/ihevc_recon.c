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
 *  ihevc_recon.c
 *
 * @brief
 *  Functions definitions reconstruction
 *
 * @author
 *  Ittiam
 *
 * @par List of Functions:
 *  - ihevc_recon_4x4_ttype1()
 *  - ihevc_recon_4x4()
 *  - ihevc_recon_8x8()
 *  - ihevc_recon_16x16()
 *  - ihevc_recon_32x32()
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
#include "ihevc_recon.h"
#include "ihevc_func_selector.h"
#include "ihevc_trans_macros.h"

/* All the functions here are replicated from ihevc.c and modified to */
/* include reconstruction */

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs reconstruction for  4x4 input block
 *
 * @par Description:
 *  Performs reconstruction of 4x4 input block by adding  adding prediction
 * data to input and clipping it to 8 bit
 *
 * @param[in] pi2_src
 *  Input 4x4 coefficients
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
 *  Zero columns in pi2_tmp
 *
 * @returns  Void
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

void ihevc_recon_4x4_ttype1(WORD16 *pi2_src,
                            UWORD8 *pu1_pred,
                            UWORD8 *pu1_dst,
                            WORD32 src_strd,
                            WORD32 pred_strd,
                            WORD32 dst_strd,
                            WORD32 zero_cols)
{
    WORD32 i, j;
    WORD32 trans_size;

    trans_size = TRANS_SIZE_4;

    /* Reconstruction */

    for(i = 0; i < trans_size; i++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            for(j = 0; j < trans_size; j++)
            {
                pu1_dst[j * dst_strd] = pu1_pred[j * pred_strd];
            }
        }
        else
        {
            for(j = 0; j < trans_size; j++)
            {
                pu1_dst[j * dst_strd] =
                                CLIP_U8(pi2_src[j * src_strd] + pu1_pred[j * pred_strd]);
            }
        }
        pi2_src++;
        pu1_dst++;
        pu1_pred++;
        zero_cols = zero_cols >> 1;
    }
}

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs reconstruction for  4x4 input block
 *
 * @par Description:
 *  Performs reconstruction of 4x4 input block by adding  adding prediction
 * data to input and clipping it to 8 bit
 *
 * @param[in] pi2_src
 *  Input 4x4 coefficients
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
 *  Zero columns in pi2_tmp
 *
 * @returns  Void
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

void ihevc_recon_4x4(WORD16 *pi2_src,
                     UWORD8 *pu1_pred,
                     UWORD8 *pu1_dst,
                     WORD32 src_strd,
                     WORD32 pred_strd,
                     WORD32 dst_strd,
                     WORD32 zero_cols)
{
    WORD32 i, j;
    WORD32 trans_size;

    trans_size = TRANS_SIZE_4;

    /* Reconstruction */

    for(i = 0; i < trans_size; i++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            for(j = 0; j < trans_size; j++)
            {
                pu1_dst[j * dst_strd] = pu1_pred[j * pred_strd];
            }
        }
        else
        {
            for(j = 0; j < trans_size; j++)
            {
                pu1_dst[j * dst_strd] =
                                CLIP_U8(pi2_src[j * src_strd] + pu1_pred[j * pred_strd]);
            }
        }
        pi2_src++;
        pu1_dst++;
        pu1_pred++;
        zero_cols = zero_cols >> 1;
    }
}

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs reconstruction for  8x8 input block
 *
 * @par Description:
 *  Performs reconstruction of 8x8 input block by adding  adding prediction
 * data to input and clipping it to 8 bit
 *
 * @param[in] pi2_src
 *  Input 8x8 coefficients
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
 *  Zero columns in pi2_tmp
 *
 * @returns  Void
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

void ihevc_recon_8x8(WORD16 *pi2_src,
                     UWORD8 *pu1_pred,
                     UWORD8 *pu1_dst,
                     WORD32 src_strd,
                     WORD32 pred_strd,
                     WORD32 dst_strd,
                     WORD32 zero_cols)
{
    WORD32 i, j;
    WORD32 trans_size;

    trans_size = TRANS_SIZE_8;

    /* Reconstruction */

    for(i = 0; i < trans_size; i++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            for(j = 0; j < trans_size; j++)
            {
                pu1_dst[j * dst_strd] = pu1_pred[j * pred_strd];
            }
        }
        else
        {
            for(j = 0; j < trans_size; j++)
            {
                pu1_dst[j * dst_strd] =
                                CLIP_U8(pi2_src[j * src_strd] + pu1_pred[j * pred_strd]);
            }
        }
        pi2_src++;
        pu1_dst++;
        pu1_pred++;
        zero_cols = zero_cols >> 1;
    }
}

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs reconstruction for  16x16 input block
 *
 * @par Description:
 *  Performs reconstruction of 16x16 input block by adding  adding prediction
 * data to input and clipping it to 8 bit
 *
 * @param[in] pi2_src
 *  Input 16x16 coefficients
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
 *  Zero columns in pi2_tmp
 *
 * @returns  Void
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

void ihevc_recon_16x16(WORD16 *pi2_src,
                       UWORD8 *pu1_pred,
                       UWORD8 *pu1_dst,
                       WORD32 src_strd,
                       WORD32 pred_strd,
                       WORD32 dst_strd,
                       WORD32 zero_cols)
{
    WORD32 i, j;
    WORD32 trans_size;

    trans_size = TRANS_SIZE_16;

    /* Reconstruction */

    for(i = 0; i < trans_size; i++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            for(j = 0; j < trans_size; j++)
            {
                pu1_dst[j * dst_strd] = pu1_pred[j * pred_strd];
            }
        }
        else
        {
            for(j = 0; j < trans_size; j++)
            {
                pu1_dst[j * dst_strd] =
                                CLIP_U8(pi2_src[j * src_strd] + pu1_pred[j * pred_strd]);
            }
        }
        pi2_src++;
        pu1_dst++;
        pu1_pred++;
        zero_cols = zero_cols >> 1;
    }
}

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs reconstruction for  32x32 input block
 *
 * @par Description:
 *  Performs reconstruction of 32x32 input block by adding  adding prediction
 * data to input and clipping it to 8 bit
 *
 * @param[in] pi2_src
 *  Input 32x32 coefficients
 *
 * @param[in] pu1_pred
 *  Prediction 32x32 block
 *
 * @param[out] pu1_dst
 *  Output 32x32 block
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
 *  Zero columns in pi2_tmp
 *
 * @returns  Void
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

void ihevc_recon_32x32(WORD16 *pi2_src,
                       UWORD8 *pu1_pred,
                       UWORD8 *pu1_dst,
                       WORD32 src_strd,
                       WORD32 pred_strd,
                       WORD32 dst_strd,
                       WORD32 zero_cols)
{
    WORD32 i, j;
    WORD32 trans_size;

    trans_size = TRANS_SIZE_32;

    /* Reconstruction */

    for(i = 0; i < trans_size; i++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            for(j = 0; j < trans_size; j++)
            {
                pu1_dst[j * dst_strd] = pu1_pred[j * pred_strd];
            }
        }
        else
        {
            for(j = 0; j < trans_size; j++)
            {
                pu1_dst[j * dst_strd] =
                                CLIP_U8(pi2_src[j * src_strd] + pu1_pred[j * pred_strd]);
            }
        }
        pi2_src++;
        pu1_dst++;
        pu1_pred++;
        zero_cols = zero_cols >> 1;
    }
}

