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
*  ihevc_chroma_intra_pred_filters_x86_intr.c
*
* @brief
*  Contains function Definition for intra prediction  interpolation filters
*
*
* @author
*  Ittiam
*
* @par List of Functions:
*  ihevc_intra_pred_chroma_planar_sse42()
*
*  ihevc_intra_pred_chroma_dc_sse42()
*
* @remarks
*  None
*
*******************************************************************************
*/


/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/

#include "ihevc_typedefs.h"
#include "ihevc_macros.h"
#include "ihevc_func_selector.h"
#include "ihevc_platform_macros.h"
#include "ihevc_intra_pred.h"
#include "ihevc_chroma_intra_pred.h"
#include "ihevc_common_tables.h"
#include "ihevc_tables_x86_intr.h"

#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>


/****************************************************************************/
/* Constant Macros                                                          */
/****************************************************************************/
#define MAX_CU_SIZE 64
#define BIT_DEPTH 8
#define T32_4NT 128
#define T16_4NT 64
#define T16C_4NT 64
#define T8C_4NT 32
/****************************************************************************/
/* Function Macros                                                          */
/****************************************************************************/

#define GET_BIT(y,x) ((y) & (1 << x)) && (1 << x)

/* tables to shuffle 8-bit values */

/*****************************************************************************/
/* Function Definition                                                      */
/*****************************************************************************/



/**
*******************************************************************************
*
* @brief
*  Planar Intraprediction with reference neighboring samples location
* pointed by 'pu1_ref' to the TU block location  pointed by 'pu1_dst'  Refer
* to section 8.4.4.2.4 in the standard
*
* @par Description:
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[in] pu1_dst
*  UWORD8 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] nt
*  integer Transform Block size
*
* @param[in] mode
*  integer intraprediction mode
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_intra_pred_chroma_planar_sse42(UWORD8 *pu1_ref,
                                          WORD32 src_strd,
                                          UWORD8 *pu1_dst,
                                          WORD32 dst_strd,
                                          WORD32 nt,
                                          WORD32 mode)
{

    WORD32 row, col;
    WORD32 log2nt = 5;
    WORD32 two_nt, three_nt;

    __m128i const_temp_4x32b, const_temp1_4x32b, const_temp2_4x32b, const_temp3_4x32b, const_temp4_4x32b;
    __m128i col_8x16b, const_temp5_4x32b, const_temp6_4x32b, zero_8x16b, const_temp7_4x32b;
    UNUSED(src_strd);
    UNUSED(mode);

    switch(nt)
    {
        case 16:
            log2nt = 4;
            break;
        case 8:
            log2nt = 3;
            break;
        case 4:
            log2nt = 2;
            break;
        default:
            break;
    }
    two_nt = 2 * nt;
    three_nt = 3 * nt;

    /* Planar filtering */

/* setting vallues in  registera*/

//  pu1_ref[2*(two_nt - 1 - row)]
//  pu1_ref[2 * (three_nt + 1)]
//  pu1_ref[2 * (two_nt + 1) + col]
//  pu1_ref[2 * (nt - 1)]

    const_temp_4x32b  = _mm_set_epi16(pu1_ref[2 * (three_nt + 1) + 1], pu1_ref[2 * (three_nt + 1)], pu1_ref[2 * (three_nt + 1) + 1],
                                      pu1_ref[2 * (three_nt + 1)], pu1_ref[2 * (three_nt + 1) + 1], pu1_ref[2 * (three_nt + 1)],
                                      pu1_ref[2 * (three_nt + 1) + 1], pu1_ref[2 * (three_nt + 1)]);

    const_temp1_4x32b = _mm_set_epi16(pu1_ref[2 * (nt - 1) + 1], pu1_ref[2 * (nt - 1)], pu1_ref[2 * (nt - 1) + 1], pu1_ref[2 * (nt - 1)],
                                      pu1_ref[2 * (nt - 1) + 1], pu1_ref[2 * (nt - 1)], pu1_ref[2 * (nt - 1) + 1], pu1_ref[2 * (nt - 1)]);

    const_temp4_4x32b = _mm_set1_epi16(nt - 1);
    const_temp6_4x32b = _mm_set1_epi16(nt);
    const_temp7_4x32b = _mm_set1_epi16(4);

    zero_8x16b = _mm_set1_epi32(0);

    if(nt % 4 == 0)
    {
        const_temp7_4x32b = _mm_set1_epi16(4);

        for(row = 0; row < nt; row++)
        {
            __m128i res_temp_8x16b, row_8x16b, res_temp1_8x16b, res_temp2_8x16b;
            __m128i res_temp3_8x16b;

            const_temp2_4x32b  = _mm_set_epi16(pu1_ref[2 * (two_nt - 1 - row) + 1], pu1_ref[2 * (two_nt - 1 - row)], pu1_ref[2 * (two_nt - 1 - row) + 1],
                                               pu1_ref[2 * (two_nt - 1 - row)], pu1_ref[2 * (two_nt - 1 - row) + 1], pu1_ref[2 * (two_nt - 1 - row)],
                                               pu1_ref[2 * (two_nt - 1 - row) + 1], pu1_ref[2 * (two_nt - 1 - row)]);

            const_temp3_4x32b  = _mm_set1_epi16((row + 1));
            row_8x16b = _mm_set1_epi16((nt - 1 - row));

            const_temp5_4x32b = _mm_set_epi16(3, 3, 2, 2, 1, 1, 0, 0);
            col_8x16b = _mm_set_epi16(4, 4, 3, 3, 2, 2, 1, 1);

            const_temp5_4x32b = _mm_sub_epi16(const_temp4_4x32b, const_temp5_4x32b);

            /*(row + 1) * pu1_ref[nt - 1]*/
            res_temp_8x16b  = _mm_mullo_epi16(const_temp3_4x32b,  const_temp1_4x32b);

            /*(row + 1) * pu1_ref[nt - 1] + nt)*/
            res_temp_8x16b = _mm_add_epi16(res_temp_8x16b, const_temp6_4x32b);

            for(col = 0; col < 2 * nt; col += 8)
            {
                __m128i src_temp_8x16b;

                /* loding 8bit 16 pixles*/
                src_temp_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + 2 * (two_nt + 1) + col));

                src_temp_8x16b =  _mm_cvtepu8_epi16(src_temp_8x16b); /* row=0*/

                /* (nt - 1 - row) * pu1_ref[two_nt + 1 + col] */
                res_temp1_8x16b  = _mm_mullo_epi16(src_temp_8x16b,  row_8x16b);

                /*(col + 1) * pu1_ref[three_nt + 1]*/
                res_temp2_8x16b  = _mm_mullo_epi16(const_temp_4x32b,  col_8x16b);

                /*(nt - 1 - col)* pu1_ref[two_nt - 1 - row]*/
                res_temp3_8x16b  = _mm_mullo_epi16(const_temp2_4x32b,  const_temp5_4x32b);

                res_temp1_8x16b = _mm_add_epi16(res_temp_8x16b, res_temp1_8x16b);
                res_temp1_8x16b = _mm_add_epi16(res_temp1_8x16b, res_temp2_8x16b);
                res_temp1_8x16b = _mm_add_epi16(res_temp1_8x16b, res_temp3_8x16b);

                res_temp1_8x16b = _mm_srli_epi16(res_temp1_8x16b, (log2nt + 1));
                res_temp1_8x16b = _mm_packus_epi16(res_temp1_8x16b, zero_8x16b);

                _mm_storel_epi64((__m128i *)(pu1_dst + (row * dst_strd) + col), res_temp1_8x16b);

                const_temp5_4x32b = _mm_sub_epi16(const_temp5_4x32b, const_temp7_4x32b);
                col_8x16b = _mm_add_epi16(col_8x16b, const_temp7_4x32b);
            } /* inner loop ends here */
        }
    }
}

/**
*******************************************************************************
*
* @brief
*  Intraprediction for DC mode with reference neighboring  samples location
* pointed by 'pu1_ref' to the TU block  location pointed by 'pu1_dst'  Refer
* to section 8.4.4.2.5 in the standard
*
* @par Description:
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[in] pu1_dst
*  UWORD8 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] nt
*  integer Transform Block size (Chroma)
*
* @param[in] mode
*  integer intraprediction mode
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_intra_pred_chroma_dc_sse42(UWORD8 *pu1_ref,
                                      WORD32 src_strd,
                                      UWORD8 *pu1_dst,
                                      WORD32 dst_strd,
                                      WORD32 nt,
                                      WORD32 mode)
{

    WORD32 acc_dc_u, acc_dc_v;
    WORD32 dc_val_u, dc_val_v;
    WORD32 row;
    WORD32 log2nt = 5;
    __m128i src_temp1, src_temp3, src_temp4, src_temp5, src_temp6, m_mask;
    __m128i src_temp7, src_temp8, src_temp9, src_temp10;
    __m128i m_zero = _mm_set1_epi32(0);
    UNUSED(src_strd);
    UNUSED(mode);

    switch(nt)
    {
        case 32:
            log2nt = 5;
            break;
        case 16:
            log2nt = 4;
            break;
        case 8:
            log2nt = 3;
            break;
        case 4:
            log2nt = 2;
            break;
        default:
            break;
    }

    acc_dc_u = 0;
    acc_dc_v = 0;

    /* Calculate DC value for the transform block */

    m_mask = _mm_loadu_si128((__m128i *)&IHEVCE_SHUFFLEMASKY9[0]);

    if(nt == 16)
    {
        __m128i temp_sad;

        src_temp3 =  _mm_loadu_si128((__m128i *)(pu1_ref + (2 * nt)));
        src_temp4 =  _mm_loadu_si128((__m128i *)(pu1_ref + (2 * nt) + 16));
        src_temp7 =  _mm_loadu_si128((__m128i *)(pu1_ref + (2 * nt) + 32));
        src_temp8 =  _mm_loadu_si128((__m128i *)(pu1_ref + (2 * nt) + 48));

        src_temp5 =  _mm_cvtepu8_epi16(src_temp3);
        src_temp6 =  _mm_cvtepu8_epi16(src_temp4);
        src_temp9 =  _mm_cvtepu8_epi16(src_temp7);
        src_temp10 =  _mm_cvtepu8_epi16(src_temp8);

        src_temp3 = _mm_srli_si128(src_temp3, 8);
        src_temp4 = _mm_srli_si128(src_temp4, 8);
        src_temp7 = _mm_srli_si128(src_temp7, 8);
        src_temp8 = _mm_srli_si128(src_temp8, 8);

        src_temp3 =  _mm_cvtepu8_epi16(src_temp3);
        src_temp4 =  _mm_cvtepu8_epi16(src_temp4);
        src_temp7 =  _mm_cvtepu8_epi16(src_temp7);
        src_temp8 =  _mm_cvtepu8_epi16(src_temp8);

        src_temp4 = _mm_add_epi16(src_temp4, src_temp6);
        src_temp6 = _mm_add_epi16(src_temp3, src_temp5);
        src_temp8 = _mm_add_epi16(src_temp7, src_temp8);
        src_temp10 = _mm_add_epi16(src_temp9, src_temp10);

        src_temp4 = _mm_add_epi16(src_temp4, src_temp6);
        src_temp8 = _mm_add_epi16(src_temp8, src_temp10);

        src_temp4 = _mm_add_epi16(src_temp4, src_temp8);
        src_temp4 = _mm_shuffle_epi8(src_temp4, m_mask);
        src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);
        src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);

        src_temp4 = _mm_cvtepi16_epi32(src_temp4);
        temp_sad  = _mm_srli_si128(src_temp4, 4); /* Next 32 bits */
        acc_dc_u  = _mm_cvtsi128_si32(src_temp4);
        acc_dc_v  = _mm_cvtsi128_si32(temp_sad);
    }

    else if(nt == 8)
    {
        __m128i temp_sad;
        src_temp3 =  _mm_loadu_si128((__m128i *)(pu1_ref + (2 * nt)));
        src_temp4 =  _mm_loadu_si128((__m128i *)(pu1_ref + (2 * nt) + 16));

        src_temp5 =  _mm_cvtepu8_epi16(src_temp3);
        src_temp6 =  _mm_cvtepu8_epi16(src_temp4);

        src_temp3 = _mm_srli_si128(src_temp3, 8);
        src_temp4 = _mm_srli_si128(src_temp4, 8);

        src_temp3 =  _mm_cvtepu8_epi16(src_temp3);
        src_temp4 =  _mm_cvtepu8_epi16(src_temp4);

        src_temp4 = _mm_add_epi16(src_temp4, src_temp6);
        src_temp6 = _mm_add_epi16(src_temp3, src_temp5);

        src_temp4 = _mm_add_epi16(src_temp4, src_temp6);
        src_temp4 = _mm_shuffle_epi8(src_temp4, m_mask);
        src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);
        src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);

        src_temp4 = _mm_cvtepi16_epi32(src_temp4);
        temp_sad  = _mm_srli_si128(src_temp4, 4); /* Next 32 bits */
        acc_dc_u  = _mm_cvtsi128_si32(src_temp4);
        acc_dc_v  = _mm_cvtsi128_si32(temp_sad);
    }

    else if(nt == 4)
    {
        __m128i temp_sad;
        src_temp3 =  _mm_loadu_si128((__m128i *)(pu1_ref + (2 * nt)));

        src_temp5 =  _mm_cvtepu8_epi16(src_temp3);
        src_temp4 = _mm_srli_si128(src_temp3, 8);
        src_temp4 =  _mm_cvtepu8_epi16(src_temp4);

        src_temp4 = _mm_add_epi16(src_temp4, src_temp5);

        src_temp4 = _mm_shuffle_epi8(src_temp4, m_mask);
        src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);
        src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);

        src_temp4 = _mm_cvtepi16_epi32(src_temp4);
        temp_sad  = _mm_srli_si128(src_temp4, 4); /* Next 32 bits */
        acc_dc_u  = _mm_cvtsi128_si32(src_temp4);
        acc_dc_v  = _mm_cvtsi128_si32(temp_sad);
    }


    acc_dc_u += pu1_ref[6 * nt];
    acc_dc_v += pu1_ref[6 * nt + 1];

    acc_dc_u -= pu1_ref[4 * nt];
    acc_dc_v -= pu1_ref[4 * nt + 1];

    dc_val_u = (acc_dc_u + nt) >> (log2nt + 1);
    dc_val_v = (acc_dc_v + nt) >> (log2nt + 1);

    dc_val_u = dc_val_u | (dc_val_v << 8);

    /* Fill the remaining rows with DC value*/

    if(nt == 4)
    {
        src_temp1 = _mm_set1_epi16(dc_val_u);

        /*  pu1_dst[(row * dst_strd) + col] = dc_val;*/
        _mm_storel_epi64((__m128i *)(pu1_dst + (0 * dst_strd)), src_temp1);
        _mm_storel_epi64((__m128i *)(pu1_dst + (1 * dst_strd)), src_temp1);
        _mm_storel_epi64((__m128i *)(pu1_dst + (2 * dst_strd)), src_temp1);
        _mm_storel_epi64((__m128i *)(pu1_dst + (3 * dst_strd)), src_temp1);

    }
    else if(nt == 8)
    {
        src_temp1 = _mm_set1_epi16(dc_val_u);

        /*  pu1_dst[(row * dst_strd) + col] = dc_val;*/
        _mm_storeu_si128((__m128i *)(pu1_dst + (0 * dst_strd)), src_temp1);
        _mm_storeu_si128((__m128i *)(pu1_dst + (1 * dst_strd)), src_temp1);
        _mm_storeu_si128((__m128i *)(pu1_dst + (2 * dst_strd)), src_temp1);
        _mm_storeu_si128((__m128i *)(pu1_dst + (3 * dst_strd)), src_temp1);

        _mm_storeu_si128((__m128i *)(pu1_dst + (4 * dst_strd)), src_temp1);
        _mm_storeu_si128((__m128i *)(pu1_dst + (5 * dst_strd)), src_temp1);
        _mm_storeu_si128((__m128i *)(pu1_dst + (6 * dst_strd)), src_temp1);
        _mm_storeu_si128((__m128i *)(pu1_dst + (7 * dst_strd)), src_temp1);

    }

    else /* nt == 16 */
    {

        src_temp1 = _mm_set1_epi16(dc_val_u);

        for(row = 0; row < nt; row += 8)
        {
            /*  pu1_dst[(row * dst_strd) + col] = dc_val;*/
            _mm_storeu_si128((__m128i *)(pu1_dst + (0 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + (1 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + (2 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + (3 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (0 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (1 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (2 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (3 * dst_strd)), src_temp1);

            _mm_storeu_si128((__m128i *)(pu1_dst + (4 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + (5 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + (6 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + (7 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (4 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (5 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (6 * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (7 * dst_strd)), src_temp1);

            pu1_dst += 8 * dst_strd;
        }


    }

}
