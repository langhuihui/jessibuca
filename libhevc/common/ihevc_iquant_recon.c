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
 *  ihevc_iquant_recon.c
 *
 * @brief
 *  Contains function definitions for inverse  quantization and
 * reconstruction
 *
 * @author
 *  100470
 *
 * @par List of Functions:
 *  - ihevc_iquant_recon_4x4_ttype1()
 *  - ihevc_iquant_recon_4x4()
 *  - ihevc_iquant_recon_8x8()
 *  - ihevc_iquant_recon_16x16()
 *  - ihevc_iquant_recon_32x32()
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
#include "ihevc_iquant_recon.h"
#include "ihevc_func_selector.h"
#include "ihevc_trans_macros.h"

/* All the functions here are replicated from ihevc_iquant_itrans_recon.c and modified to */
/* include reconstruction */

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs inverse quantization type 1 and  reconstruction
 * for 4x4 input block
 *
 * @par Description:
 *  This function performs inverse quantization and  reconstruction for 4x4
 * input block
 *
 * @param[in] pi2_src
 *  Input 4x4 coefficients
 *
 * @param[in] pu1_pred
 *  Prediction 4x4 block
 *
 * @param[in] pi2_dequant_coeff
 *  Dequant Coeffs
 *
 * @param[out] pu1_dst
 *  Output 4x4 block
 *
 * @param[in] qp_div
 *  Quantization parameter / 6
 *
 * @param[in] qp_rem
 *  Quantization parameter % 6
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

void ihevc_iquant_recon_4x4_ttype1(WORD16 *pi2_src,
                                   UWORD8 *pu1_pred,
                                   WORD16 *pi2_dequant_coeff,
                                   UWORD8 *pu1_dst,
                                   WORD32 qp_div, /* qpscaled / 6 */
                                   WORD32 qp_rem, /* qpscaled % 6 */
                                   WORD32 src_strd,
                                   WORD32 pred_strd,
                                   WORD32 dst_strd,
                                   WORD32 zero_cols)
{

    {
        /* Inverse Quant and recon */
        {
            WORD32 i, j;
            WORD32 shift_iq;
            WORD32 trans_size;
            /* Inverse Quantization constants */
            {
                WORD32 log2_trans_size, bit_depth;

                log2_trans_size = 2;
                bit_depth = 8 + 0;
                shift_iq = bit_depth + log2_trans_size - 5;
            }

            trans_size = TRANS_SIZE_4;

            for(i = 0; i < trans_size; i++)
            {
                /* Checking for Zero Cols */
                if((zero_cols & 1) == 1)
                {
                    for(j = 0; j < trans_size; j++)
                        pu1_dst[j * dst_strd] = pu1_pred[j * pred_strd];
                }
                else
                {
                    for(j = 0; j < trans_size; j++)
                    {
                        WORD32 iquant_out;
                        IQUANT_4x4(iquant_out,
                                   pi2_src[j * src_strd],
                                   pi2_dequant_coeff[j * trans_size] * g_ihevc_iquant_scales[qp_rem],
                                   shift_iq, qp_div);

                        iquant_out = (iquant_out + 16) >> 5;
                        pu1_dst[j * dst_strd] =
                                        CLIP_U8(iquant_out + pu1_pred[j * pred_strd]);
                    }
                }
                pi2_src++;
                pi2_dequant_coeff++;
                pu1_pred++;
                pu1_dst++;

                zero_cols = zero_cols >> 1;
            }
        }
    }
}

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs inverse quantization and  reconstruction for 4x4
 * input block
 *
 * @par Description:
 *  This function performs inverse quantization and  reconstruction for 4x4
 * input block
 *
 * @param[in] pi2_src
 *  Input 4x4 coefficients
 *
 * @param[in] pu1_pred
 *  Prediction 4x4 block
 *
 * @param[in] pi2_dequant_coeff
 *  Dequant Coeffs
 *
 * @param[out] pu1_dst
 *  Output 4x4 block
 *
 * @param[in] qp_div
 *  Quantization parameter / 6
 *
 * @param[in] qp_rem
 *  Quantization parameter % 6
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

void ihevc_iquant_recon_4x4(WORD16 *pi2_src,
                            UWORD8 *pu1_pred,
                            WORD16 *pi2_dequant_coeff,
                            UWORD8 *pu1_dst,
                            WORD32 qp_div, /* qpscaled / 6 */
                            WORD32 qp_rem, /* qpscaled % 6 */
                            WORD32 src_strd,
                            WORD32 pred_strd,
                            WORD32 dst_strd,
                            WORD32 zero_cols)
{

    {
        /* Inverse Quant and recon */
        {
            WORD32 i, j;
            WORD32 shift_iq;
            WORD32 trans_size;
            /* Inverse Quantization constants */
            {
                WORD32 log2_trans_size, bit_depth;

                log2_trans_size = 2;
                bit_depth = 8 + 0;
                shift_iq = bit_depth + log2_trans_size - 5;
            }

            trans_size = TRANS_SIZE_4;

            for(i = 0; i < trans_size; i++)
            {
                /* Checking for Zero Cols */
                if((zero_cols & 1) == 1)
                {
                    for(j = 0; j < trans_size; j++)
                        pu1_dst[j * dst_strd] = pu1_pred[j * pred_strd];
                }
                else
                {
                    for(j = 0; j < trans_size; j++)
                    {
                        WORD32 iquant_out;
                        IQUANT_4x4(iquant_out,
                                   pi2_src[j * src_strd],
                                   pi2_dequant_coeff[j * trans_size] * g_ihevc_iquant_scales[qp_rem],
                                   shift_iq, qp_div);
                        iquant_out = (iquant_out + 16) >> 5;
                        pu1_dst[j * dst_strd] =
                                        CLIP_U8(iquant_out + pu1_pred[j * pred_strd]);
                    }
                }
                pi2_src++;
                pi2_dequant_coeff++;
                pu1_pred++;
                pu1_dst++;

                zero_cols = zero_cols >> 1;
            }
        }
    }
}

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs inverse quantization and  reconstruction for 8x8
 * input block
 *
 * @par Description:
 *  This function performs inverse quantization and  reconstruction for 8x8
 * input block
 *
 * @param[in] pi2_src
 *  Input 8x8 coefficients
 *
 * @param[in] pu1_pred
 *  Prediction 8x8 block
 *
 * @param[in] pi2_dequant_coeff
 *  Dequant Coeffs
 *
 * @param[out] pu1_dst
 *  Output 8x8 block
 *
 * @param[in] qp_div
 *  Quantization parameter / 6
 *
 * @param[in] qp_rem
 *  Quantization parameter % 6
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

void ihevc_iquant_recon_8x8(WORD16 *pi2_src,
                            UWORD8 *pu1_pred,
                            WORD16 *pi2_dequant_coeff,
                            UWORD8 *pu1_dst,
                            WORD32 qp_div, /* qpscaled / 6 */
                            WORD32 qp_rem, /* qpscaled % 6 */
                            WORD32 src_strd,
                            WORD32 pred_strd,
                            WORD32 dst_strd,
                            WORD32 zero_cols)
{

    {
        /* Inverse Quant and recon */
        {
            WORD32 i, j;
            WORD32 shift_iq;
            WORD32 trans_size;
            /* Inverse Quantization constants */
            {
                WORD32 log2_trans_size, bit_depth;

                log2_trans_size = 3;
                bit_depth = 8 + 0;
                shift_iq = bit_depth + log2_trans_size - 5;
            }

            trans_size = TRANS_SIZE_8;

            for(i = 0; i < trans_size; i++)
            {
                /* Checking for Zero Cols */
                if((zero_cols & 1) == 1)
                {
                    for(j = 0; j < trans_size; j++)
                        pu1_dst[j * dst_strd] = pu1_pred[j * pred_strd];
                }
                else
                {
                    for(j = 0; j < trans_size; j++)
                    {
                        WORD32 iquant_out;
                        IQUANT(iquant_out,
                               pi2_src[j * src_strd],
                               pi2_dequant_coeff[j * trans_size] * g_ihevc_iquant_scales[qp_rem],
                               shift_iq, qp_div);
                        iquant_out = (iquant_out + 16) >> 5;
                        pu1_dst[j * dst_strd] =
                                        CLIP_U8(iquant_out + pu1_pred[j * pred_strd]);
                    }
                }
                pi2_src++;
                pi2_dequant_coeff++;
                pu1_pred++;
                pu1_dst++;

                zero_cols = zero_cols >> 1;
            }
        }
    }
}

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs inverse quantization and  reconstruction for 16x16
 * input block
 *
 * @par Description:
 *  This function performs inverse quantization and  reconstruction for 16x16
 * input block
 *
 * @param[in] pi2_src
 *  Input 16x16 coefficients
 *
 * @param[in] pu1_pred
 *  Prediction 16x16 block
 *
 * @param[in] pi2_dequant_coeff
 *  Dequant Coeffs
 *
 * @param[out] pu1_dst
 *  Output 16x16 block
 *
 * @param[in] qp_div
 *  Quantization parameter / 6
 *
 * @param[in] qp_rem
 *  Quantization parameter % 6
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

void ihevc_iquant_recon_16x16(WORD16 *pi2_src,
                              UWORD8 *pu1_pred,
                              WORD16 *pi2_dequant_coeff,
                              UWORD8 *pu1_dst,
                              WORD32 qp_div, /* qpscaled / 6 */
                              WORD32 qp_rem, /* qpscaled % 6 */
                              WORD32 src_strd,
                              WORD32 pred_strd,
                              WORD32 dst_strd,
                              WORD32 zero_cols)

{

    {
        /* Inverse Quant and recon */
        {
            WORD32 i, j;
            WORD32 shift_iq;
            WORD32 trans_size;
            /* Inverse Quantization constants */
            {
                WORD32 log2_trans_size, bit_depth;

                log2_trans_size = 4;
                bit_depth = 8 + 0;
                shift_iq = bit_depth + log2_trans_size - 5;
            }

            trans_size = TRANS_SIZE_16;

            for(i = 0; i < trans_size; i++)
            {
                /* Checking for Zero Cols */
                if((zero_cols & 1) == 1)
                {
                    for(j = 0; j < trans_size; j++)
                        pu1_dst[j * dst_strd] = pu1_pred[j * pred_strd];
                }
                else
                {
                    for(j = 0; j < trans_size; j++)
                    {
                        WORD32 iquant_out;
                        IQUANT(iquant_out,
                               pi2_src[j * src_strd],
                               pi2_dequant_coeff[j * trans_size] * g_ihevc_iquant_scales[qp_rem],
                               shift_iq, qp_div);
                        iquant_out = (iquant_out + 16) >> 5;
                        pu1_dst[j * dst_strd] =
                                        CLIP_U8(iquant_out + pu1_pred[j * pred_strd]);
                    }
                }
                pi2_src++;
                pi2_dequant_coeff++;
                pu1_pred++;
                pu1_dst++;

                zero_cols = zero_cols >> 1;
            }
        }
    }
}

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs inverse quantization and  reconstruction for 32x32
 * input block
 *
 * @par Description:
 *  This function performs inverse quantization and  reconstruction for 32x32
 * input block
 *
 * @param[in] pi2_src
 *  Input 32x32 coefficients
 *
 * @param[in] pu1_pred
 *  Prediction 32x32 block
 *
 * @param[in] pi2_dequant_coeff
 *  Dequant Coeffs
 *
 * @param[out] pu1_dst
 *  Output 32x32 block
 *
 * @param[in] qp_div
 *  Quantization parameter / 6
 *
 * @param[in] qp_rem
 *  Quantization parameter % 6
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

void ihevc_iquant_recon_32x32(WORD16 *pi2_src,
                              UWORD8 *pu1_pred,
                              WORD16 *pi2_dequant_coeff,
                              UWORD8 *pu1_dst,
                              WORD32 qp_div, /* qpscaled / 6 */
                              WORD32 qp_rem, /* qpscaled % 6 */
                              WORD32 src_strd,
                              WORD32 pred_strd,
                              WORD32 dst_strd,
                              WORD32 zero_cols)
{

    {
        /* Inverse Quant and recon */
        {
            WORD32 i, j;
            WORD32 shift_iq;
            WORD32 trans_size;
            /* Inverse Quantization constants */
            {
                WORD32 log2_trans_size, bit_depth;

                log2_trans_size = 5;
                bit_depth = 8 + 0;
                shift_iq = bit_depth + log2_trans_size - 5;
            }

            trans_size = TRANS_SIZE_32;

            for(i = 0; i < trans_size; i++)
            {
                /* Checking for Zero Cols */
                if((zero_cols & 1) == 1)
                {
                    for(j = 0; j < trans_size; j++)
                        pu1_dst[j * dst_strd] = pu1_pred[j * pred_strd];
                }
                else
                {
                    for(j = 0; j < trans_size; j++)
                    {
                        WORD32 iquant_out;
                        IQUANT(iquant_out,
                               pi2_src[j * src_strd],
                               pi2_dequant_coeff[j * trans_size] * g_ihevc_iquant_scales[qp_rem],
                               shift_iq, qp_div);
                        iquant_out = (iquant_out + 16) >> 5;
                        pu1_dst[j * dst_strd] =
                                        CLIP_U8(iquant_out + pu1_pred[j * pred_strd]);
                    }
                }
                pi2_src++;
                pi2_dequant_coeff++;
                pu1_pred++;
                pu1_dst++;

                zero_cols = zero_cols >> 1;
            }
        }
    }
}

