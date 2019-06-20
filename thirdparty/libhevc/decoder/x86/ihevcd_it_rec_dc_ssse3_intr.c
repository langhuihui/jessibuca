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
*  ihevcd_it_rec_dc_atom_intr.c
*
* @brief
*  Platform specific intrinsic implementation of certain functions
*
* @author
*  Ittiam
* @par List of Functions:
*  - ihevcd_itrans_recon_dc
*  - ihevcd_fmt_conv_420sp_to_420p
*
* @remarks
*  None
*
*******************************************************************************
*/

#include "ihevc_typedefs.h"
#include "ihevc_defs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevcd_function_selector.h"

#include <immintrin.h>




void ihevcd_itrans_recon_dc_luma_ssse3(UWORD8 *pu1_pred, UWORD8 *pu1_dst, WORD32 pred_strd, WORD32 dst_strd,
                                       WORD32 log2_trans_size, WORD16 i2_coeff_value)
{
    __m128i m_temp_reg_0;
    __m128i m_temp_reg_1;
    __m128i m_temp_reg_2;
    __m128i m_temp_reg_3;
    __m128i m_temp_reg_4;
    __m128i m_temp_reg_5;
    __m128i m_temp_reg_6;
    __m128i m_temp_reg_7;
    __m128i m_temp_reg_8;
    __m128i m_temp_reg_9;
    __m128i m_temp_reg_10;
    __m128i m_temp_reg_11;
    __m128i m_temp_reg_12;
    __m128i m_temp_reg_13;
    __m128i m_temp_reg_14;
    __m128i m_temp_reg_15;
    __m128i m_temp_reg_20, zero_8x16b;
    __m128i *pi4_dst = (__m128i *)pu1_dst;



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

    /*Replicate the DC value within 16 bits in 128 bit register*/
    m_temp_reg_20 = _mm_set1_epi16(dc_value);
    zero_8x16b = _mm_setzero_si128();

    if(trans_size == 4)
    {
        WORD32 *pi4_dst = (WORD32 *)pu1_dst;

        m_temp_reg_0 = _mm_loadl_epi64((__m128i *)(pu1_pred));
        m_temp_reg_1 = _mm_loadl_epi64((__m128i *)(pu1_pred + pred_strd));
        m_temp_reg_2 = _mm_loadl_epi64((__m128i *)(pu1_pred + 2 * pred_strd));
        m_temp_reg_3 = _mm_loadl_epi64((__m128i *)(pu1_pred + 3 * pred_strd));

        m_temp_reg_4 = _mm_unpacklo_epi32(m_temp_reg_0, m_temp_reg_1);
        m_temp_reg_5 = _mm_unpacklo_epi32(m_temp_reg_2, m_temp_reg_3);

        m_temp_reg_4 = _mm_unpacklo_epi8(m_temp_reg_4, zero_8x16b);
        m_temp_reg_5 = _mm_unpacklo_epi8(m_temp_reg_5, zero_8x16b);

        m_temp_reg_6 = _mm_add_epi16(m_temp_reg_4, m_temp_reg_20);
        m_temp_reg_7 = _mm_add_epi16(m_temp_reg_5, m_temp_reg_20);

        m_temp_reg_8 = _mm_packus_epi16(m_temp_reg_6, m_temp_reg_7);


        *pi4_dst = _mm_cvtsi128_si32(m_temp_reg_8);
        m_temp_reg_1 = _mm_srli_si128(m_temp_reg_8, 4);
        m_temp_reg_2 = _mm_srli_si128(m_temp_reg_8, 8);
        m_temp_reg_3 = _mm_srli_si128(m_temp_reg_8, 12);
        pu1_dst += dst_strd;
        pi4_dst = (WORD32 *)(pu1_dst);

        *pi4_dst = _mm_cvtsi128_si32(m_temp_reg_1);
        pu1_dst += dst_strd;
        pi4_dst = (WORD32 *)(pu1_dst);

        *pi4_dst = _mm_cvtsi128_si32(m_temp_reg_2);
        pu1_dst += dst_strd;
        pi4_dst = (WORD32 *)(pu1_dst);

        *pi4_dst = _mm_cvtsi128_si32(m_temp_reg_3);
    }
    else
    {
        WORD32 i, j;

        for(i = 1; i <= trans_size; i += 4)
        {
            for(j = 1; j <= trans_size; j += 8)
            {

                m_temp_reg_0 = _mm_loadl_epi64((__m128i *)pu1_pred);
                m_temp_reg_1 = _mm_loadl_epi64((__m128i *)(pu1_pred + pred_strd));
                m_temp_reg_2 = _mm_loadl_epi64((__m128i *)(pu1_pred + 2 * pred_strd));
                m_temp_reg_3 = _mm_loadl_epi64((__m128i *)(pu1_pred + 3 * pred_strd));


                m_temp_reg_4 = _mm_unpacklo_epi8(m_temp_reg_0, zero_8x16b);
                m_temp_reg_5 = _mm_unpacklo_epi8(m_temp_reg_1, zero_8x16b);
                m_temp_reg_6 = _mm_unpacklo_epi8(m_temp_reg_2, zero_8x16b);
                m_temp_reg_7 = _mm_unpacklo_epi8(m_temp_reg_3, zero_8x16b);

                m_temp_reg_8 = _mm_add_epi16(m_temp_reg_4, m_temp_reg_20);
                m_temp_reg_9 = _mm_add_epi16(m_temp_reg_5, m_temp_reg_20);
                m_temp_reg_10 = _mm_add_epi16(m_temp_reg_6, m_temp_reg_20);
                m_temp_reg_11 = _mm_add_epi16(m_temp_reg_7, m_temp_reg_20);

                pi4_dst = (__m128i *)(pu1_dst);

                m_temp_reg_12 = _mm_packus_epi16(m_temp_reg_8, m_temp_reg_9);
                _mm_storel_epi64(pi4_dst, m_temp_reg_12);

                pi4_dst = (__m128i *)(pu1_dst + dst_strd);

                m_temp_reg_13 = _mm_srli_si128(m_temp_reg_12, 8);
                _mm_storel_epi64(pi4_dst, m_temp_reg_13);

                pi4_dst = (__m128i *)(pu1_dst + 2 * dst_strd);

                m_temp_reg_14 = _mm_packus_epi16(m_temp_reg_10, m_temp_reg_11);
                _mm_storel_epi64(pi4_dst, m_temp_reg_14);

                pi4_dst = (__m128i *)(pu1_dst + 3 * dst_strd);

                m_temp_reg_15 = _mm_srli_si128(m_temp_reg_14, 8);
                _mm_storel_epi64(pi4_dst, m_temp_reg_15);

                pu1_pred += 8;
                pu1_dst += 8;
            }
            pu1_pred += 4 * pred_strd - trans_size;
            pu1_dst += 4 * dst_strd - trans_size;
        }
    }


}

void ihevcd_itrans_recon_dc_chroma_ssse3(UWORD8 *pu1_pred, UWORD8 *pu1_dst, WORD32 pred_strd, WORD32 dst_strd,
                                         WORD32 log2_trans_size, WORD16 i2_coeff_value)
{
    __m128i m_temp_reg_0;
    __m128i m_temp_reg_1;
    __m128i m_temp_reg_2;
    __m128i m_temp_reg_3;
    __m128i m_temp_reg_4;
    __m128i m_temp_reg_5;
    __m128i m_temp_reg_6;
    __m128i m_temp_reg_7;
    __m128i m_temp_reg_8;
    __m128i m_temp_reg_9;
    __m128i m_temp_reg_10;
    __m128i m_temp_reg_11;
    __m128i m_temp_reg_12;
    __m128i m_temp_reg_13;
    __m128i m_temp_reg_14;
    __m128i m_temp_reg_15;
    __m128i m_temp_reg_20, zero_8x16b;
    __m128i *pi4_dst = (__m128i *)pu1_dst;


    WORD32 add, shift;
    WORD32 dc_value, quant_out;
    WORD32 trans_size;


    WORD32 shuffle_mask_4x4 = 0x06040200;
    WORD32 unchanged_mask_4x4 = 0x07050301;
    LWORD64 shuffle_mask = 0x0E0C0A0806040200LL;
    LWORD64 unchanged_mask = 0x0F0D0B0907050301LL;

    trans_size = (1 << log2_trans_size);

    quant_out = i2_coeff_value;

    shift = IT_SHIFT_STAGE_1;
    add = 1 << (shift - 1);
    dc_value = CLIP_S16((quant_out * 64 + add) >> shift);
    shift = IT_SHIFT_STAGE_2;
    add = 1 << (shift - 1);
    dc_value = CLIP_S16((dc_value * 64 + add) >> shift);

    /*Replicate the DC value within 16 bits in 128 bit register*/
    m_temp_reg_20 = _mm_set1_epi16(dc_value);
    zero_8x16b = _mm_setzero_si128();

    if(trans_size == 4)
    {
        __m128i chroma_shuffle_mask_16x8b;
        __m128i chroma_unchanged_mask_16x8b;
        chroma_shuffle_mask_16x8b = _mm_cvtsi32_si128(shuffle_mask_4x4);
        chroma_unchanged_mask_16x8b = _mm_cvtsi32_si128(unchanged_mask_4x4);

        /*Load the prediction data*/
        m_temp_reg_0 = _mm_loadl_epi64((__m128i *)(pu1_pred));
        m_temp_reg_1 = _mm_loadl_epi64((__m128i *)(pu1_pred + pred_strd));
        m_temp_reg_2 = _mm_loadl_epi64((__m128i *)(pu1_pred + 2 * pred_strd));
        m_temp_reg_3 = _mm_loadl_epi64((__m128i *)(pu1_pred + 3 * pred_strd));

        m_temp_reg_10  = _mm_shuffle_epi8(m_temp_reg_0, chroma_shuffle_mask_16x8b);
        m_temp_reg_11  = _mm_shuffle_epi8(m_temp_reg_1, chroma_shuffle_mask_16x8b);
        m_temp_reg_12  = _mm_shuffle_epi8(m_temp_reg_2, chroma_shuffle_mask_16x8b);
        m_temp_reg_13  = _mm_shuffle_epi8(m_temp_reg_3, chroma_shuffle_mask_16x8b);

        m_temp_reg_14 = _mm_unpacklo_epi32(m_temp_reg_10, m_temp_reg_11);
        m_temp_reg_15 = _mm_unpacklo_epi32(m_temp_reg_12, m_temp_reg_13);

        m_temp_reg_4 = _mm_unpacklo_epi8(m_temp_reg_14, zero_8x16b);
        m_temp_reg_5 = _mm_unpacklo_epi8(m_temp_reg_15, zero_8x16b);

        m_temp_reg_6 = _mm_add_epi16(m_temp_reg_4, m_temp_reg_20);
        m_temp_reg_7 = _mm_add_epi16(m_temp_reg_5, m_temp_reg_20);

        /*Load the recon data to make sure that 'v' is not corrupted when 'u' is called and vice versa*/
        m_temp_reg_0 = _mm_loadl_epi64((__m128i *)pu1_dst);
        m_temp_reg_1 = _mm_loadl_epi64((__m128i *)(pu1_dst + dst_strd));
        m_temp_reg_2 = _mm_loadl_epi64((__m128i *)(pu1_dst + 2 * dst_strd));
        m_temp_reg_3 = _mm_loadl_epi64((__m128i *)(pu1_dst + 3 * dst_strd));

        m_temp_reg_0  = _mm_shuffle_epi8(m_temp_reg_0, chroma_unchanged_mask_16x8b);
        m_temp_reg_1  = _mm_shuffle_epi8(m_temp_reg_1, chroma_unchanged_mask_16x8b);
        m_temp_reg_2  = _mm_shuffle_epi8(m_temp_reg_2, chroma_unchanged_mask_16x8b);
        m_temp_reg_3  = _mm_shuffle_epi8(m_temp_reg_3, chroma_unchanged_mask_16x8b);


        m_temp_reg_8 = _mm_packus_epi16(m_temp_reg_6, m_temp_reg_7);
        m_temp_reg_9 = _mm_unpacklo_epi8(m_temp_reg_8, m_temp_reg_0);
        m_temp_reg_8 = _mm_srli_si128(m_temp_reg_8, 4);
        m_temp_reg_10 = _mm_unpacklo_epi8(m_temp_reg_8, m_temp_reg_1);
        m_temp_reg_8 = _mm_srli_si128(m_temp_reg_8, 4);
        m_temp_reg_11 = _mm_unpacklo_epi8(m_temp_reg_8, m_temp_reg_2);
        m_temp_reg_8 = _mm_srli_si128(m_temp_reg_8, 4);
        m_temp_reg_12 = _mm_unpacklo_epi8(m_temp_reg_8, m_temp_reg_3);

        /*Store the result in the destination*/
        _mm_storel_epi64(pi4_dst, m_temp_reg_9);
        pu1_dst += dst_strd;
        pi4_dst = (__m128i *)(pu1_dst);


        _mm_storel_epi64(pi4_dst, m_temp_reg_10);
        pu1_dst += dst_strd;
        pi4_dst = (__m128i *)(pu1_dst);

        _mm_storel_epi64(pi4_dst, m_temp_reg_11);
        pu1_dst += dst_strd;
        pi4_dst = (__m128i *)(pu1_dst);

        _mm_storel_epi64(pi4_dst, m_temp_reg_12);
    }
    else
    {
        WORD32 i, j;
        __m128i chroma_shuffle_mask_16x8b;
        __m128i chroma_unchanged_mask_16x8b;
        chroma_shuffle_mask_16x8b = _mm_loadl_epi64((__m128i *)(&shuffle_mask));
        chroma_unchanged_mask_16x8b =
                        _mm_loadl_epi64((__m128i *)(&unchanged_mask));

        for(i = 0; i < trans_size; i += 4)
        {
            for(j = 0; j < trans_size; j += 8)
            {

                m_temp_reg_0 = _mm_loadu_si128((__m128i *)pu1_pred);
                m_temp_reg_1 = _mm_loadu_si128((__m128i *)(pu1_pred + pred_strd));
                m_temp_reg_2 = _mm_loadu_si128((__m128i *)(pu1_pred + 2 * pred_strd));
                m_temp_reg_3 = _mm_loadu_si128((__m128i *)(pu1_pred + 3 * pred_strd));

                /*Retain only one chroma component*/
                m_temp_reg_4  = _mm_shuffle_epi8(m_temp_reg_0, chroma_shuffle_mask_16x8b);
                m_temp_reg_5  = _mm_shuffle_epi8(m_temp_reg_1, chroma_shuffle_mask_16x8b);
                m_temp_reg_6  = _mm_shuffle_epi8(m_temp_reg_2, chroma_shuffle_mask_16x8b);
                m_temp_reg_7  = _mm_shuffle_epi8(m_temp_reg_3, chroma_shuffle_mask_16x8b);

                m_temp_reg_4 = _mm_unpacklo_epi8(m_temp_reg_4, zero_8x16b);
                m_temp_reg_5 = _mm_unpacklo_epi8(m_temp_reg_5, zero_8x16b);
                m_temp_reg_6 = _mm_unpacklo_epi8(m_temp_reg_6, zero_8x16b);
                m_temp_reg_7 = _mm_unpacklo_epi8(m_temp_reg_7, zero_8x16b);

                m_temp_reg_8 = _mm_add_epi16(m_temp_reg_4, m_temp_reg_20);
                m_temp_reg_9 = _mm_add_epi16(m_temp_reg_5, m_temp_reg_20);
                m_temp_reg_10 = _mm_add_epi16(m_temp_reg_6, m_temp_reg_20);
                m_temp_reg_11 = _mm_add_epi16(m_temp_reg_7, m_temp_reg_20);


                /*Load the recon data to make sure that 'v' is not corrupted when 'u' is called and vice versa*/
                m_temp_reg_0 = _mm_loadu_si128((__m128i *)pu1_dst);
                m_temp_reg_1 = _mm_loadu_si128((__m128i *)(pu1_dst + dst_strd));
                m_temp_reg_2 = _mm_loadu_si128((__m128i *)(pu1_dst + 2 * dst_strd));
                m_temp_reg_3 = _mm_loadu_si128((__m128i *)(pu1_dst + 3 * dst_strd));

                m_temp_reg_0  = _mm_shuffle_epi8(m_temp_reg_0, chroma_unchanged_mask_16x8b);
                m_temp_reg_1  = _mm_shuffle_epi8(m_temp_reg_1, chroma_unchanged_mask_16x8b);
                m_temp_reg_2  = _mm_shuffle_epi8(m_temp_reg_2, chroma_unchanged_mask_16x8b);
                m_temp_reg_3  = _mm_shuffle_epi8(m_temp_reg_3, chroma_unchanged_mask_16x8b);

                m_temp_reg_4 = _mm_packus_epi16(m_temp_reg_8, m_temp_reg_9);
                m_temp_reg_5 = _mm_packus_epi16(m_temp_reg_10, m_temp_reg_11);

                m_temp_reg_12 = _mm_unpacklo_epi8(m_temp_reg_4, m_temp_reg_0);
                m_temp_reg_4 = _mm_srli_si128(m_temp_reg_4, 8);
                m_temp_reg_13 = _mm_unpacklo_epi8(m_temp_reg_4, m_temp_reg_1);

                m_temp_reg_14 = _mm_unpacklo_epi8(m_temp_reg_5, m_temp_reg_2);
                m_temp_reg_5 = _mm_srli_si128(m_temp_reg_5, 8);
                m_temp_reg_15 = _mm_unpacklo_epi8(m_temp_reg_5, m_temp_reg_3);

                /*Store the result in the destination*/
                pi4_dst = (__m128i *)(pu1_dst);

                _mm_storel_epi64(pi4_dst, m_temp_reg_12);
                m_temp_reg_8 = _mm_srli_si128(m_temp_reg_12, 8);

                pi4_dst = (__m128i *)(pu1_dst + 8);
                _mm_storel_epi64(pi4_dst, m_temp_reg_8);

                pi4_dst = (__m128i *)(pu1_dst + dst_strd);

                _mm_storel_epi64(pi4_dst, m_temp_reg_13);
                m_temp_reg_9 = _mm_srli_si128(m_temp_reg_13, 8);

                pi4_dst = (__m128i *)(pu1_dst + dst_strd + 8);
                _mm_storel_epi64(pi4_dst, m_temp_reg_9);

                pi4_dst = (__m128i *)(pu1_dst + 2 * dst_strd);

                _mm_storel_epi64(pi4_dst, m_temp_reg_14);
                m_temp_reg_10 = _mm_srli_si128(m_temp_reg_14, 8);

                pi4_dst = (__m128i *)(pu1_dst + 2 * dst_strd + 8);
                _mm_storel_epi64(pi4_dst, m_temp_reg_10);

                pi4_dst = (__m128i *)(pu1_dst + 3 * dst_strd);

                _mm_storel_epi64(pi4_dst, m_temp_reg_15);
                m_temp_reg_11 = _mm_srli_si128(m_temp_reg_15, 8);

                pi4_dst = (__m128i *)(pu1_dst + 3 * dst_strd + 8);
                _mm_storel_epi64(pi4_dst, m_temp_reg_11);

                pu1_pred += 16;
                pu1_dst += 16;
            }

            pu1_pred += 4 * pred_strd - 2 * trans_size;
            pu1_dst += 4 * dst_strd - 2 * trans_size;
        }
    }


}
