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
*  ihevc_trans_macros.h
*
* @brief
*  Macros used in the forward transform and inverse transform functions
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef IHEVC_TRANS_MACROS_H_
#define IHEVC_TRANS_MACROS_H_

#define QUANT(out, inp, quant_coeff, qp_div, log2_trans_size, q_add) \
{                                                                                                                                                                \
    LWORD64 tmp;                                                                                                                                                  \
    WORD32 sign;                                                                                                                                                 \
    WORD32 bit_depth,transform_shift;                                                                                                                            \
    WORD32  q_bits, quant_multiplier;                                                                                                                            \
                                                                                                                                                                 \
    /* q_bits and q_add calculation*/                                                                                                                            \
    /* To be moved outside in neon. To be computer once per transform call */                                                                                    \
    bit_depth = 8;                                                                                                                                               \
    transform_shift = MAX_TR_DYNAMIC_RANGE - bit_depth - log2_trans_size;                                                                                        \
    quant_multiplier = 4 ; /* because quant_coeff are multiplied by 16. Instead of multiplying, we can reduce the division factor q_bits by 4 */                 \
    q_bits = QUANT_SHIFT + qp_div + transform_shift + SCALING_Q_SHIFT - quant_multiplier ;                                                                       \
                                                                                                                                                                 \
    sign = (inp)<0 ? -1:1;                                                                                                                                       \
                                                                                                                                                                 \
    tmp = (LWORD64)(abs(inp));                                                                                                                                    \
    tmp = tmp * (quant_coeff);                                                                                                                                   \
    tmp = tmp + (((LWORD64)q_add) << (q_bits - QUANT_ROUND_FACTOR_Q));                                                                                            \
    tmp = tmp >> q_bits;                                                                                                                                         \
                                                                                                                                                                 \
    tmp = tmp * sign;                                                                                                                                            \
    out = (WORD16) CLIP_S16(tmp);                                                                                                                                \
}                                                                                                                                                                \

#define QUANT_HBD(out, inp, quant_coeff, qp_div, log2_trans_size, q_add, bit_depth) \
{                                                                                                                                                                \
    LWORD64 tmp;                                                                                                                                                  \
    WORD32 sign;                                                                                                                                                 \
    WORD32 transform_shift;                                                                                                                                      \
    WORD32  q_bits, quant_multiplier;                                                                                                                            \
                                                                                                                                                                 \
    /* q_bits and q_add calculation*/                                                                                                                            \
    /* To be moved outside in neon. To be computer once per transform call */                                                                                    \
                                                                                                                                                                 \
    transform_shift = MAX_TR_DYNAMIC_RANGE - bit_depth - log2_trans_size;                                                                                        \
    quant_multiplier = 4 ; /* because quant_coeff are multiplied by 16. Instead of multiplying, we can reduce the division factor q_bits by 4 */                 \
    q_bits = QUANT_SHIFT + qp_div + transform_shift + SCALING_Q_SHIFT - quant_multiplier ;                                                                       \
                                                                                                                                                                 \
    sign = (inp)<0 ? -1:1;                                                                                                                                       \
                                                                                                                                                                 \
    tmp = (LWORD64)(abs(inp));                                                                                                                                    \
    tmp = tmp * (quant_coeff);                                                                                                                                   \
    tmp = tmp + (((LWORD64)q_add) << (q_bits - QUANT_ROUND_FACTOR_Q));                                                                                            \
    tmp = tmp >> q_bits;                                                                                                                                         \
                                                                                                                                                                 \
    tmp = tmp * sign;                                                                                                                                            \
    out = (WORD16) CLIP_S16(tmp);                                                                                                                                \
}
/* added by 100028 */
#define QUANT_NO_WEIGHTMAT(out, inp, quant_coeff, qp_div, log2_trans_size, q_add) \
{                                                                                                                                                                \
    WORD32 tmp;                                                                                                                                                  \
    WORD32 sign;                                                                                                                                                 \
    WORD32 bit_depth,transform_shift;                                                                                                                            \
    WORD32  q_bits, quant_multiplier;                                                                                                                            \
                                                                                                                                                                 \
    /* q_bits and q_add calculation*/                                                                                                                            \
    /* To be moved outside in neon. To be computer once per transform call */                                                                                    \
    bit_depth = 8;                                                                                                                                               \
    transform_shift = MAX_TR_DYNAMIC_RANGE - bit_depth - log2_trans_size;                                                                                        \
    quant_multiplier = 4 ; /* because quant_coeff are multiplied by 16. Instead of multiplying, we can reduce the division factor q_bits by 4 */                 \
    q_bits = QUANT_SHIFT + qp_div + transform_shift + SCALING_Q_SHIFT - quant_multiplier - FLAT_RESCALE_MAT_Q_SHIFT /* 2048 */;                                                                       \
                                                                                                                                                                 \
    sign = (inp)<0 ? -1:1;                                                                                                                                       \
                                                                                                                                                                 \
    tmp = (WORD32)(abs(inp));                                                                                                                                    \
    tmp = tmp * (quant_coeff);                                                                                                                                   \
    tmp = tmp + (((WORD32)q_add) << (q_bits - QUANT_ROUND_FACTOR_Q));                                                                                            \
    tmp = tmp >> q_bits;                                                                                                                                         \
                                                                                                                                                                 \
    tmp = tmp * sign;                                                                                                                                            \
    out = (WORD16) CLIP_S16(tmp);                                                                                                                                \
}

#define QUANT_NO_WEIGHTMAT_HBD(out, inp, quant_coeff, qp_div, log2_trans_size, q_add, bit_depth) \
{                                                                                                                                                                \
    WORD32 tmp;                                                                                                                                                  \
    WORD32 sign;                                                                                                                                                 \
    WORD32 transform_shift;                                                                                                                                      \
    WORD32  q_bits, quant_multiplier;                                                                                                                            \
                                                                                                                                                                 \
    /* q_bits and q_add calculation*/                                                                                                                            \
    /* To be moved outside in neon. To be computer once per transform call */                                                                                    \
                                                                                                                                                                 \
    transform_shift = MAX_TR_DYNAMIC_RANGE - bit_depth - log2_trans_size;                                                                                        \
    quant_multiplier = 4 ; /* because quant_coeff are multiplied by 16. Instead of multiplying, we can reduce the division factor q_bits by 4 */                 \
    q_bits = QUANT_SHIFT + qp_div + transform_shift + SCALING_Q_SHIFT - quant_multiplier - FLAT_RESCALE_MAT_Q_SHIFT /* 2048 */;                                                                       \
                                                                                                                                                                 \
    sign = (inp)<0 ? -1:1;                                                                                                                                       \
                                                                                                                                                                 \
    tmp = (WORD32)(abs(inp));                                                                                                                                    \
    tmp = tmp * (quant_coeff);                                                                                                                                   \
    tmp = tmp + (((WORD32)q_add) << (q_bits - QUANT_ROUND_FACTOR_Q));                                                                                            \
    tmp = tmp >> q_bits;                                                                                                                                         \
                                                                                                                                                                 \
    tmp = tmp * sign;                                                                                                                                            \
    out = (WORD16) CLIP_S16(tmp);                                                                                                                                \
}
/* Reference Inverse Quantization: "pi2_src"(Coefficients) will be clipped to 15 or 14 bits when (qp_div > shift_iq). Spec doesn't have any clip mentioned  */

/* Inverse quantization other than 4x4 */
/* No clipping is needed for "pi2_src"(coefficients) */
#define IQUANT(res, coeff /*pi2_src[index*src_strd]*/, dequant_coeff /*pi2_dequant_coeff[index*trans_size] * g_ihevc_iquant_scales[qp_rem] */, shift_iq, qp_div)       \
{                                                                                                                                              \
    WORD32 tmp, add_iq;                                                                                                                        \
                                                                                                                                               \
    add_iq = SHL_NEG(1 , (shift_iq - qp_div - 1));  /* To be moved outside in neon. To be computed once per transform call */                  \
                                                                                                                                               \
    tmp = coeff * dequant_coeff ;                                                                                                              \
    tmp = tmp + add_iq;                                                                                                                        \
    tmp = SHR_NEG(tmp,(shift_iq - qp_div));                                                                                                    \
                                                                                                                                               \
    res = CLIP_S16(tmp);                                                                                                                       \
}

/* 4x4 inverse quantization */
/* Options : */
/* 1. Clip "pi2_src"(coefficients) to 10 bits if "(qp_div >= shift_iq)" or 16 bits if "(qp_div < shift_iq)"*/
/* 2. Increasing precision of "pi2_src"(coefficients) to 64 bits */

#define IQUANT_4x4(res, coeff /*pi2_src[index*src_strd]*/, dequant_coeff /*pi2_dequant_coeff[index*trans_size] * g_ihevc_iquant_scales[qp_rem] */, shift_iq, qp_div)   \
{                                                                                                                                              \
    WORD32 clip_coeff, tmp;                                                                                                                    \
    WORD32 coeff_min,coeff_max;                                                                                                                \
    WORD32 coeff_bit_range;                                                                                                                    \
    WORD32 add_iq;                                                                                                                             \
    add_iq = SHL_NEG(1 , (shift_iq - qp_div - 1));  /* To be moved outside in neon. To be computed once per transform call */                  \
                                                                                                                                               \
    coeff_bit_range = 16;                                                                                                                      \
    if(qp_div > shift_iq)                                                                                                                      \
        coeff_bit_range = 10;                                                                                                                  \
                                                                                                                                               \
    coeff_min = -(1<<(coeff_bit_range-1));                                                                                                     \
    coeff_max = (1<<(coeff_bit_range-1)) - 1;                                                                                                  \
                                                                                                                                               \
    clip_coeff = CLIP3(coeff,coeff_min,coeff_max);                                                                                             \
                                                                                                                                               \
    tmp = clip_coeff * dequant_coeff ;                                                                                                         \
    tmp = tmp + add_iq;                                                                                                                        \
    tmp = SHR_NEG(tmp,(shift_iq - qp_div));                                                                                                    \
                                                                                                                                               \
    res = CLIP_S16(tmp);                                                                                                                       \
}

#endif /* IHEVC_TRANS_MACROS_H_ */
