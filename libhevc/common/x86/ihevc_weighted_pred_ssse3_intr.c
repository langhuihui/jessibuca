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
*  ihevc_weighted_pred_atom_intr.c
*
* @brief
*  Contains function definitions for weighted prediction used in inter
* prediction
*
* @author
*
*
* @par List of Functions:
*   - ihevc_weighted_pred_uni_ssse3()
*   - ihevc_weighted_pred_bi_ssse3()
*   - ihevc_weighted_pred_bi_default_ssse3()
*   - ihevc_weighted_pred_chroma_uni_ssse3()
*   - ihevc_weighted_pred_chroma_bi_ssse3()
*   - ihevc_weighted_pred_chroma_bi_default_ssse3()
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
#include <assert.h>

#include "ihevc_debug.h"
#include "ihevc_typedefs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_func_selector.h"
#include "ihevc_defs.h"
#include "ihevc_weighted_pred.h"
#include "ihevc_inter_pred.h"


#include <immintrin.h>

/**
*******************************************************************************
*
* @brief
*  Does uni-weighted prediction on the array pointed by  pi2_src and stores
* it at the location pointed by pi2_dst
*
* @par Description:
*  dst = ( (src + lvl_shift) * wgt0 + (1 << (shift - 1)) )  >> shift +
* offset
*
* @param[in] pi2_src
*  Pointer to the source
*
* @param[out] pu1_dst
*  Pointer to the destination
*
* @param[in] src_strd
*  Source stride
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] wgt0
*  weight to be multiplied to the source
*
* @param[in] off0
*  offset to be added after rounding and
*
* @param[in] shifting
*
*
* @param[in] shift
*  (14 Bit depth) + log2_weight_denominator
*
* @param[in] lvl_shift
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_weighted_pred_uni_ssse3(WORD16 *pi2_src,
                                   UWORD8 *pu1_dst,
                                   WORD32 src_strd,
                                   WORD32 dst_strd,
                                   WORD32 wgt0,
                                   WORD32 off0,
                                   WORD32 shift,
                                   WORD32 lvl_shift,
                                   WORD32 ht,
                                   WORD32 wd)
{
    WORD32 row, col, temp;

    /* all 128 bit registers are named with a suffix mxnb, where m is the */
    /* number of n bits packed in the register                            */
    __m128i src_temp0_8x16b, src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b;
    __m128i const_temp_4x32b, lvl_shift_4x32b, wgt0_8x16b, off0_4x32b;
    __m128i res_temp0_4x32b, res_temp1_4x32b, res_temp2_4x32b, res_temp3_4x32b;

    ASSERT(wd % 4 == 0); /* checking assumption*/
    ASSERT(ht % 4 == 0); /* checking assumption*/

    temp = 1 << (shift - 1);

    // seting values in register
    lvl_shift_4x32b = _mm_set1_epi16(lvl_shift);
    wgt0_8x16b = _mm_set1_epi16(wgt0);

    /* lvl_shift * wgt0 */
    res_temp0_4x32b = _mm_mullo_epi16(lvl_shift_4x32b, wgt0_8x16b);
    res_temp1_4x32b = _mm_mulhi_epi16(lvl_shift_4x32b, wgt0_8x16b);

    const_temp_4x32b = _mm_set1_epi32(temp);
    off0_4x32b = _mm_set1_epi32(off0);


    /* lvl_shift * wgt0 */
    lvl_shift_4x32b = _mm_unpacklo_epi16(res_temp0_4x32b, res_temp1_4x32b);
    /* lvl_shift * wgt0 + 1 << (shift - 1) */
    lvl_shift_4x32b = _mm_add_epi32(lvl_shift_4x32b, const_temp_4x32b);

    if(0 == (wd & 7)) /* wd multiple of 8 case */
    {
        __m128i res_temp4_4x32b, res_temp5_4x32b, res_temp6_4x32b, res_temp7_4x32b;

        /*  outer for loop starts from here */
        for(row = 0; row < ht; row += 4)
        {
            for(col = 0; col < wd; col += 8)
            {   /* for row =0 ,1,2,3*/

                /* row = 0 */ /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                src_temp0_8x16b = _mm_loadu_si128((__m128i *)(pi2_src));
                /* row = 1 */
                src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pi2_src + src_strd));
                /* row = 2 */
                src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pi2_src + 2 * src_strd));
                /* row = 3 */
                src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pi2_src + 3 * src_strd));

                /*i4_tmp = (pi2_src[col]) * wgt0*/ /* Lower 16 bit */
                res_temp0_4x32b  = _mm_mullo_epi16(src_temp0_8x16b, wgt0_8x16b);
                res_temp1_4x32b  = _mm_mullo_epi16(src_temp1_8x16b, wgt0_8x16b);
                res_temp2_4x32b  = _mm_mullo_epi16(src_temp2_8x16b, wgt0_8x16b);
                res_temp3_4x32b  = _mm_mullo_epi16(src_temp3_8x16b, wgt0_8x16b);

                /*i4_tmp = (pi2_src[col] ) * wgt0*/ /* Higher 16 bit */
                src_temp0_8x16b  = _mm_mulhi_epi16(src_temp0_8x16b, wgt0_8x16b);
                src_temp1_8x16b  = _mm_mulhi_epi16(src_temp1_8x16b, wgt0_8x16b);
                src_temp2_8x16b  = _mm_mulhi_epi16(src_temp2_8x16b, wgt0_8x16b);
                src_temp3_8x16b  = _mm_mulhi_epi16(src_temp3_8x16b, wgt0_8x16b);

                /* Get 32 bit Result */
                res_temp4_4x32b = _mm_unpackhi_epi16(res_temp0_4x32b, src_temp0_8x16b);
                res_temp5_4x32b = _mm_unpackhi_epi16(res_temp1_4x32b, src_temp1_8x16b);
                res_temp6_4x32b = _mm_unpackhi_epi16(res_temp2_4x32b, src_temp2_8x16b);
                res_temp7_4x32b = _mm_unpackhi_epi16(res_temp3_4x32b, src_temp3_8x16b);

                res_temp0_4x32b = _mm_unpacklo_epi16(res_temp0_4x32b, src_temp0_8x16b);
                res_temp1_4x32b = _mm_unpacklo_epi16(res_temp1_4x32b, src_temp1_8x16b);
                res_temp2_4x32b = _mm_unpacklo_epi16(res_temp2_4x32b, src_temp2_8x16b);
                res_temp3_4x32b = _mm_unpacklo_epi16(res_temp3_4x32b, src_temp3_8x16b);

                /* i4_tmp = (pi2_src[col] + lvl_shift) * wgt0 + 1 << (shift - 1) */
                res_temp4_4x32b = _mm_add_epi32(res_temp4_4x32b, lvl_shift_4x32b);
                res_temp5_4x32b = _mm_add_epi32(res_temp5_4x32b, lvl_shift_4x32b);
                res_temp6_4x32b = _mm_add_epi32(res_temp6_4x32b, lvl_shift_4x32b);
                res_temp7_4x32b = _mm_add_epi32(res_temp7_4x32b, lvl_shift_4x32b);
                res_temp0_4x32b = _mm_add_epi32(res_temp0_4x32b, lvl_shift_4x32b);
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, lvl_shift_4x32b);
                res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, lvl_shift_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, lvl_shift_4x32b);

                /* (i4_tmp >> shift) */ /* First 4 pixels */
                res_temp0_4x32b = _mm_srai_epi32(res_temp0_4x32b, shift);
                res_temp1_4x32b = _mm_srai_epi32(res_temp1_4x32b, shift);
                res_temp2_4x32b = _mm_srai_epi32(res_temp2_4x32b, shift);
                res_temp3_4x32b = _mm_srai_epi32(res_temp3_4x32b, shift);

                /* (i4_tmp >> shift) */ /* Last 4 pixels */
                res_temp4_4x32b = _mm_srai_epi32(res_temp4_4x32b, shift);
                res_temp5_4x32b = _mm_srai_epi32(res_temp5_4x32b, shift);
                res_temp6_4x32b = _mm_srai_epi32(res_temp6_4x32b, shift);
                res_temp7_4x32b = _mm_srai_epi32(res_temp7_4x32b, shift);

                /*i4_tmp = (i4_tmp >> shift) + off0; */ /* First 4 pixels */
                res_temp0_4x32b = _mm_add_epi32(res_temp0_4x32b, off0_4x32b);
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, off0_4x32b);
                res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, off0_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, off0_4x32b);

                /*i4_tmp = (i4_tmp >> shift) + off0; */ /* Last 4 pixels */
                res_temp4_4x32b = _mm_add_epi32(res_temp4_4x32b, off0_4x32b);
                res_temp5_4x32b = _mm_add_epi32(res_temp5_4x32b, off0_4x32b);
                res_temp6_4x32b = _mm_add_epi32(res_temp6_4x32b, off0_4x32b);
                res_temp7_4x32b = _mm_add_epi32(res_temp7_4x32b, off0_4x32b);

                res_temp0_4x32b = _mm_packs_epi32(res_temp0_4x32b, res_temp4_4x32b);
                res_temp1_4x32b = _mm_packs_epi32(res_temp1_4x32b, res_temp5_4x32b);
                res_temp2_4x32b = _mm_packs_epi32(res_temp2_4x32b, res_temp6_4x32b);
                res_temp3_4x32b = _mm_packs_epi32(res_temp3_4x32b, res_temp7_4x32b);
                /* pu1_dst[col] = CLIP_U8(i4_tmp); */
                res_temp0_4x32b = _mm_packus_epi16(res_temp0_4x32b, res_temp0_4x32b);
                res_temp1_4x32b = _mm_packus_epi16(res_temp1_4x32b, res_temp1_4x32b);
                res_temp2_4x32b = _mm_packus_epi16(res_temp2_4x32b, res_temp2_4x32b);
                res_temp3_4x32b = _mm_packus_epi16(res_temp3_4x32b, res_temp3_4x32b);

                /* store four 8-bit output values  */
                _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd), res_temp0_4x32b); /* row = 0*/
                _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), res_temp1_4x32b); /* row = 2*/
                _mm_storel_epi64((__m128i *)(pu1_dst + 2 * dst_strd), res_temp2_4x32b); /* row = 1*/
                _mm_storel_epi64((__m128i *)(pu1_dst + 3 * dst_strd), res_temp3_4x32b); /* row = 3*/

                /* To update pointer */
                pi2_src += 8;
                pu1_dst += 8;

            } /* inner loop ends here(4-output values in single iteration) */

            pi2_src = pi2_src - wd + 4 * src_strd;    /* Pointer update */
            pu1_dst = pu1_dst - wd + 4 * dst_strd; /* Pointer update */

        }
    }
    else  /* wd multiple of 4 case */
    {
        WORD32 dst0, dst1, dst2, dst3;
        /*  outer for loop starts from here */
        for(row = 0; row < ht; row += 4)
        {
            for(col = 0; col < wd; col += 4)
            {   /* for row =0 ,1,2,3*/

                /* row = 0 */ /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                src_temp0_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src));
                /* row = 1 */
                src_temp1_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src + src_strd));
                /* row = 2 */
                src_temp2_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src + 2 * src_strd));
                /* row = 3 */
                src_temp3_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src + 3 * src_strd));

                /* 2 rows together */
                src_temp0_8x16b = _mm_unpacklo_epi64(src_temp0_8x16b, src_temp2_8x16b);
                src_temp1_8x16b = _mm_unpacklo_epi64(src_temp1_8x16b, src_temp3_8x16b);

                /*i4_tmp = (pi2_src[col]) * wgt0*/ /* Lower 16 bit */
                res_temp0_4x32b  = _mm_mullo_epi16(src_temp0_8x16b, wgt0_8x16b);
                res_temp1_4x32b  = _mm_mullo_epi16(src_temp1_8x16b, wgt0_8x16b);
                /*i4_tmp = (pi2_src[col]) * wgt0*/ /* Higher 16 bit */
                src_temp0_8x16b  = _mm_mulhi_epi16(src_temp0_8x16b, wgt0_8x16b);
                src_temp1_8x16b  = _mm_mulhi_epi16(src_temp1_8x16b, wgt0_8x16b);

                /* Get 32 bit Result */
                res_temp2_4x32b = _mm_unpackhi_epi16(res_temp0_4x32b, src_temp0_8x16b);
                res_temp3_4x32b = _mm_unpackhi_epi16(res_temp1_4x32b, src_temp1_8x16b);

                res_temp0_4x32b = _mm_unpacklo_epi16(res_temp0_4x32b, src_temp0_8x16b);
                res_temp1_4x32b = _mm_unpacklo_epi16(res_temp1_4x32b, src_temp1_8x16b);

                /* i4_tmp = (pi2_src[col] + lvl_shift) * wgt0 + 1 << (shift - 1) */
                res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, lvl_shift_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, lvl_shift_4x32b);
                res_temp0_4x32b = _mm_add_epi32(res_temp0_4x32b, lvl_shift_4x32b);
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, lvl_shift_4x32b);

                /* (i4_tmp >> shift) */
                res_temp0_4x32b = _mm_srai_epi32(res_temp0_4x32b, shift);
                res_temp1_4x32b = _mm_srai_epi32(res_temp1_4x32b, shift);
                res_temp2_4x32b = _mm_srai_epi32(res_temp2_4x32b, shift);
                res_temp3_4x32b = _mm_srai_epi32(res_temp3_4x32b, shift);

                /*i4_tmp = (i4_tmp >> shift) + off0; */
                res_temp0_4x32b = _mm_add_epi32(res_temp0_4x32b, off0_4x32b);
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, off0_4x32b);
                res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, off0_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, off0_4x32b);

                res_temp0_4x32b = _mm_packs_epi32(res_temp0_4x32b, res_temp1_4x32b);
                res_temp2_4x32b = _mm_packs_epi32(res_temp2_4x32b, res_temp3_4x32b);

                /* pu1_dst[col] = CLIP_U8(i4_tmp); */
                res_temp0_4x32b = _mm_packus_epi16(res_temp0_4x32b, res_temp2_4x32b);

                dst0 = _mm_cvtsi128_si32(res_temp0_4x32b);
                /* dst row = 1 to 3 */
                res_temp1_4x32b = _mm_shuffle_epi32(res_temp0_4x32b, 1);
                res_temp2_4x32b = _mm_shuffle_epi32(res_temp0_4x32b, 2);
                res_temp3_4x32b = _mm_shuffle_epi32(res_temp0_4x32b, 3);

                /* store four 8-bit output values  */
                *(WORD32 *)(&pu1_dst[0 * dst_strd]) = dst0;

                dst1 = _mm_cvtsi128_si32(res_temp1_4x32b);
                dst2 = _mm_cvtsi128_si32(res_temp2_4x32b);
                dst3 = _mm_cvtsi128_si32(res_temp3_4x32b);

                /* row = 1 to row = 3 */
                *(WORD32 *)(&pu1_dst[1 * dst_strd]) = dst1;
                *(WORD32 *)(&pu1_dst[2 * dst_strd]) = dst2;
                *(WORD32 *)(&pu1_dst[3 * dst_strd]) = dst3;

                /* To update pointer */
                pi2_src += 4;
                pu1_dst += 4;

            } /* inner loop ends here(4-output values in single iteration) */

            pi2_src = pi2_src - wd + 4 * src_strd;    /* Pointer update */
            pu1_dst = pu1_dst - wd + 4 * dst_strd; /* Pointer update */

        }
    }
}

/**
*******************************************************************************
*
* @brief
* Does chroma uni-weighted prediction on array pointed by pi2_src and stores
* it at the location pointed by pi2_dst
*
* @par Description:
*  dst = ( (src + lvl_shift) * wgt0 + (1 << (shift - 1)) )  >> shift +
* offset
*
* @param[in] pi2_src
*  Pointer to the source
*
* @param[out] pu1_dst
*  Pointer to the destination
*
* @param[in] src_strd
*  Source stride
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] wgt0
*  weight to be multiplied to the source
*
* @param[in] off0
*  offset to be added after rounding and
*
* @param[in] shifting
*
*
* @param[in] shift
*  (14 Bit depth) + log2_weight_denominator
*
* @param[in] lvl_shift
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source (each colour component)
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/


void ihevc_weighted_pred_chroma_uni_ssse3(WORD16 *pi2_src,
                                          UWORD8 *pu1_dst,
                                          WORD32 src_strd,
                                          WORD32 dst_strd,
                                          WORD32 wgt0_cb,
                                          WORD32 wgt0_cr,
                                          WORD32 off0_cb,
                                          WORD32 off0_cr,
                                          WORD32 shift,
                                          WORD32 lvl_shift,
                                          WORD32 ht,
                                          WORD32 wd)
{
    WORD32 row, col, temp, wdx2;
    /* all 128 bit registers are named with a suffix mxnb, where m is the */
    /* number of n bits packed in the register                            */

    __m128i src_temp0_8x16b, src_temp1_8x16b;
    __m128i const_temp_4x32b, lvl_shift_4x32b, wgt0_8x16b, off0_4x32b;
    __m128i res_temp0_4x32b, res_temp1_4x32b;

    ASSERT(wd % 2 == 0); /* checking assumption*/
    ASSERT(ht % 2 == 0); /* checking assumption*/

    temp = 1 << (shift - 1);
    wdx2 = 2 * wd;

    // seting values in register
    lvl_shift_4x32b = _mm_set1_epi16(lvl_shift);
    wgt0_8x16b = _mm_set_epi16(wgt0_cr, wgt0_cb, wgt0_cr, wgt0_cb, wgt0_cr, wgt0_cb, wgt0_cr, wgt0_cb);

    /* lvl_shift * wgt0 */
    res_temp0_4x32b = _mm_mullo_epi16(lvl_shift_4x32b, wgt0_8x16b);
    res_temp1_4x32b = _mm_mulhi_epi16(lvl_shift_4x32b, wgt0_8x16b);

    const_temp_4x32b = _mm_set1_epi32(temp);
    off0_4x32b = _mm_set_epi32(off0_cr, off0_cb, off0_cr, off0_cb);

    /* lvl_shift * wgt0 */
    lvl_shift_4x32b = _mm_unpacklo_epi16(res_temp0_4x32b, res_temp1_4x32b);
    /* lvl_shift * wgt0 + 1 << (shift - 1) */
    lvl_shift_4x32b = _mm_add_epi32(lvl_shift_4x32b, const_temp_4x32b);

    {
        if(0 == (wdx2 & 15)) /* 2*wd multiple of 16 case */
        {
            __m128i src_temp2_8x16b, src_temp3_8x16b;
            __m128i res_temp2_4x32b, res_temp3_4x32b;
            __m128i res_temp4_4x32b, res_temp5_4x32b, res_temp6_4x32b, res_temp7_4x32b;

            /*  outer for loop starts from here */
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wdx2; col += 16)
                {
                    /* row = 0 */ /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                    src_temp0_8x16b = _mm_loadu_si128((__m128i *)(pi2_src));
                    /* row = 1 */
                    src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pi2_src + src_strd));
                    /* row = 0 */ /* Next 8 pixels */
                    src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pi2_src + 8));
                    /* row = 1 */
                    src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pi2_src + src_strd + 8));

                    /*i4_tmp = (pi2_src[col]) * wgt0*/ /* Lower 16 bit */
                    res_temp0_4x32b  = _mm_mullo_epi16(src_temp0_8x16b, wgt0_8x16b);
                    res_temp1_4x32b  = _mm_mullo_epi16(src_temp1_8x16b, wgt0_8x16b);
                    res_temp4_4x32b  = _mm_mullo_epi16(src_temp2_8x16b, wgt0_8x16b);
                    res_temp5_4x32b  = _mm_mullo_epi16(src_temp3_8x16b, wgt0_8x16b);

                    /*i4_tmp = (pi2_src[col] ) * wgt0*/ /* Higher 16 bit */
                    src_temp0_8x16b  = _mm_mulhi_epi16(src_temp0_8x16b, wgt0_8x16b);
                    src_temp1_8x16b  = _mm_mulhi_epi16(src_temp1_8x16b, wgt0_8x16b);
                    src_temp2_8x16b  = _mm_mulhi_epi16(src_temp2_8x16b, wgt0_8x16b);
                    src_temp3_8x16b  = _mm_mulhi_epi16(src_temp3_8x16b, wgt0_8x16b);

                    /* Get 32 bit Result */
                    res_temp2_4x32b = _mm_unpackhi_epi16(res_temp0_4x32b, src_temp0_8x16b);
                    res_temp3_4x32b = _mm_unpackhi_epi16(res_temp1_4x32b, src_temp1_8x16b);
                    res_temp6_4x32b = _mm_unpackhi_epi16(res_temp4_4x32b, src_temp2_8x16b);
                    res_temp7_4x32b = _mm_unpackhi_epi16(res_temp5_4x32b, src_temp3_8x16b);

                    res_temp0_4x32b = _mm_unpacklo_epi16(res_temp0_4x32b, src_temp0_8x16b);
                    res_temp1_4x32b = _mm_unpacklo_epi16(res_temp1_4x32b, src_temp1_8x16b);
                    res_temp4_4x32b = _mm_unpacklo_epi16(res_temp4_4x32b, src_temp2_8x16b);
                    res_temp5_4x32b = _mm_unpacklo_epi16(res_temp5_4x32b, src_temp3_8x16b);

                    /*i4_tmp = (pi2_src[col] + lvl_shift) * wgt0 + 1 << (shift - 1) */
                    res_temp0_4x32b = _mm_add_epi32(res_temp0_4x32b, lvl_shift_4x32b);
                    res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, lvl_shift_4x32b);
                    res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, lvl_shift_4x32b);
                    res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, lvl_shift_4x32b);
                    res_temp4_4x32b = _mm_add_epi32(res_temp4_4x32b, lvl_shift_4x32b);
                    res_temp5_4x32b = _mm_add_epi32(res_temp5_4x32b, lvl_shift_4x32b);
                    res_temp6_4x32b = _mm_add_epi32(res_temp6_4x32b, lvl_shift_4x32b);
                    res_temp7_4x32b = _mm_add_epi32(res_temp7_4x32b, lvl_shift_4x32b);

                    /* (i4_tmp >> shift) */
                    res_temp0_4x32b = _mm_srai_epi32(res_temp0_4x32b,  shift);
                    res_temp1_4x32b = _mm_srai_epi32(res_temp1_4x32b,  shift);
                    res_temp2_4x32b = _mm_srai_epi32(res_temp2_4x32b,  shift);
                    res_temp3_4x32b = _mm_srai_epi32(res_temp3_4x32b,  shift);
                    /*i4_tmp = (i4_tmp >> shift) + off0; */
                    res_temp0_4x32b = _mm_add_epi32(res_temp0_4x32b, off0_4x32b);
                    res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, off0_4x32b);
                    /*i4_tmp = (i4_tmp >> shift) + off0; */ /* Second 4 pixels */
                    res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, off0_4x32b);
                    res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, off0_4x32b);

                    /* (i4_tmp >> shift) */
                    res_temp4_4x32b = _mm_srai_epi32(res_temp4_4x32b,  shift);
                    res_temp5_4x32b = _mm_srai_epi32(res_temp5_4x32b,  shift);
                    res_temp6_4x32b = _mm_srai_epi32(res_temp6_4x32b,  shift);
                    res_temp7_4x32b = _mm_srai_epi32(res_temp7_4x32b,  shift);
                    /*i4_tmp = (i4_tmp >> shift) + off0; */ /* Third 4 pixels */
                    res_temp4_4x32b = _mm_add_epi32(res_temp4_4x32b, off0_4x32b);
                    res_temp5_4x32b = _mm_add_epi32(res_temp5_4x32b, off0_4x32b);
                    /*i4_tmp = (i4_tmp >> shift) + off0; */ /* Last 4 pixels */
                    res_temp6_4x32b = _mm_add_epi32(res_temp6_4x32b, off0_4x32b);
                    res_temp7_4x32b = _mm_add_epi32(res_temp7_4x32b, off0_4x32b);

                    res_temp0_4x32b = _mm_packs_epi32(res_temp0_4x32b, res_temp2_4x32b);
                    res_temp1_4x32b = _mm_packs_epi32(res_temp1_4x32b, res_temp3_4x32b);
                    res_temp4_4x32b = _mm_packs_epi32(res_temp4_4x32b, res_temp6_4x32b);
                    res_temp5_4x32b = _mm_packs_epi32(res_temp5_4x32b, res_temp7_4x32b);
                    /* pu1_dst[col] = CLIP_U8(i4_tmp); */
                    res_temp0_4x32b = _mm_packus_epi16(res_temp0_4x32b, res_temp4_4x32b);
                    res_temp1_4x32b = _mm_packus_epi16(res_temp1_4x32b, res_temp5_4x32b);

                    /* store 16 8-bit output values  */
                    _mm_storeu_si128((__m128i *)(pu1_dst + 0 * dst_strd), res_temp0_4x32b); /* row = 0*/
                    _mm_storeu_si128((__m128i *)(pu1_dst + 1 * dst_strd), res_temp1_4x32b); /* row = 1*/

                    pi2_src += 16;  /* Pointer update */
                    pu1_dst += 16; /* Pointer update */

                } /* inner loop ends here(4-output values in single iteration) */
                pi2_src = pi2_src - wdx2 + 2 * src_strd;  /* Pointer update */
                pu1_dst = pu1_dst - wdx2 + 2 * dst_strd; /* Pointer update */
            }
        }
        else if(0 == (wdx2 & 7)) /* 2*wd multiple of 8 case */
        {
            __m128i res_temp2_4x32b, res_temp3_4x32b;
            /*  outer for loop starts from here */
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wdx2; col += 8)
                {
                    /* row = 0 */ /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                    src_temp0_8x16b = _mm_loadu_si128((__m128i *)(pi2_src));
                    /* row = 1 */
                    src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pi2_src + src_strd));

                    /*i4_tmp = (pi2_src[col]) * wgt0*/ /* Lower 16 bit */
                    res_temp0_4x32b  = _mm_mullo_epi16(src_temp0_8x16b, wgt0_8x16b);
                    res_temp1_4x32b  = _mm_mullo_epi16(src_temp1_8x16b, wgt0_8x16b);
                    /*i4_tmp = (pi2_src[col] ) * wgt0*/ /* Higher 16 bit */
                    src_temp0_8x16b  = _mm_mulhi_epi16(src_temp0_8x16b, wgt0_8x16b);
                    src_temp1_8x16b  = _mm_mulhi_epi16(src_temp1_8x16b, wgt0_8x16b);

                    /* Get 32 bit Result */
                    res_temp2_4x32b = _mm_unpackhi_epi16(res_temp0_4x32b, src_temp0_8x16b);
                    res_temp3_4x32b = _mm_unpackhi_epi16(res_temp1_4x32b, src_temp1_8x16b);

                    res_temp0_4x32b = _mm_unpacklo_epi16(res_temp0_4x32b, src_temp0_8x16b);
                    res_temp1_4x32b = _mm_unpacklo_epi16(res_temp1_4x32b, src_temp1_8x16b);

                    /*i4_tmp = (pi2_src[col] + lvl_shift) * wgt0 + 1 << (shift - 1) */
                    res_temp0_4x32b = _mm_add_epi32(res_temp0_4x32b, lvl_shift_4x32b);
                    res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, lvl_shift_4x32b);
                    res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, lvl_shift_4x32b);
                    res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, lvl_shift_4x32b);

                    /* (i4_tmp >> shift) */
                    res_temp0_4x32b = _mm_srai_epi32(res_temp0_4x32b,  shift);
                    res_temp1_4x32b = _mm_srai_epi32(res_temp1_4x32b,  shift);
                    res_temp2_4x32b = _mm_srai_epi32(res_temp2_4x32b,  shift);
                    res_temp3_4x32b = _mm_srai_epi32(res_temp3_4x32b,  shift);

                    /*i4_tmp = (i4_tmp >> shift) + off0; */
                    res_temp0_4x32b = _mm_add_epi32(res_temp0_4x32b, off0_4x32b);
                    res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, off0_4x32b);
                    /*i4_tmp = (i4_tmp >> shift) + off0; */ /* Last 4 pixels */
                    res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, off0_4x32b);
                    res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, off0_4x32b);

                    res_temp0_4x32b = _mm_packs_epi32(res_temp0_4x32b, res_temp2_4x32b);
                    res_temp1_4x32b = _mm_packs_epi32(res_temp1_4x32b, res_temp3_4x32b);

                    /* pu1_dst[col] = CLIP_U8(i4_tmp); */
                    res_temp0_4x32b = _mm_packus_epi16(res_temp0_4x32b, res_temp0_4x32b);
                    res_temp1_4x32b = _mm_packus_epi16(res_temp1_4x32b, res_temp1_4x32b);

                    /* store four 8-bit output values  */
                    _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd), res_temp0_4x32b); /* row = 0*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), res_temp1_4x32b); /* row = 1*/

                    pi2_src += 8;   /* Pointer update */
                    pu1_dst += 8; /* Pointer update */

                } /* inner loop ends here(4-output values in single iteration) */
                pi2_src = pi2_src - wdx2 + 2 * src_strd;  /* Pointer update */
                pu1_dst = pu1_dst - wdx2 + 2 * dst_strd; /* Pointer update */
            }
        }
        else /* 2*wd multiple of 4 case */
        {
            WORD32 dst0, dst1;
            /*  outer for loop starts from here */
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wdx2; col += 4)
                {
                    /* row = 0 */ /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                    src_temp0_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src));
                    /* row = 1 */
                    src_temp1_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src + src_strd));

                    /* 2 rows together */
                    src_temp0_8x16b = _mm_unpacklo_epi64(src_temp0_8x16b, src_temp1_8x16b);

                    /*i4_tmp = (pi2_src[col]) * wgt0*/ /* Lower 16 bit */
                    res_temp0_4x32b  = _mm_mullo_epi16(src_temp0_8x16b, wgt0_8x16b);
                    /*i4_tmp = (pi2_src[col] ) * wgt0*/ /* Higher 16 bit */
                    src_temp0_8x16b  = _mm_mulhi_epi16(src_temp0_8x16b, wgt0_8x16b);

                    /* Get 32 bit Result */
                    res_temp1_4x32b = _mm_unpackhi_epi16(res_temp0_4x32b, src_temp0_8x16b);
                    res_temp0_4x32b = _mm_unpacklo_epi16(res_temp0_4x32b, src_temp0_8x16b);

                    /*i4_tmp = (pi2_src[col] + lvl_shift) * wgt0 + 1 << (shift - 1) */
                    res_temp0_4x32b = _mm_add_epi32(res_temp0_4x32b, lvl_shift_4x32b);
                    res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, lvl_shift_4x32b);

                    /* (i4_tmp >> shift) */
                    res_temp0_4x32b = _mm_srai_epi32(res_temp0_4x32b,  shift);
                    res_temp1_4x32b = _mm_srai_epi32(res_temp1_4x32b,  shift);

                    /*i4_tmp = (i4_tmp >> shift) + off0; */
                    res_temp0_4x32b = _mm_add_epi32(res_temp0_4x32b, off0_4x32b);
                    res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, off0_4x32b);

                    res_temp0_4x32b = _mm_packs_epi32(res_temp0_4x32b, res_temp1_4x32b);

                    /* pu1_dst[col] = CLIP_U8(i4_tmp); */
                    res_temp0_4x32b = _mm_packus_epi16(res_temp0_4x32b, res_temp0_4x32b);

                    dst0 = _mm_cvtsi128_si32(res_temp0_4x32b);
                    /* dst row = 1 to 3 */
                    res_temp1_4x32b = _mm_shuffle_epi32(res_temp0_4x32b, 1);

                    /* store four 8-bit output values  */
                    *(WORD32 *)(&pu1_dst[0 * dst_strd]) = dst0;

                    dst1 = _mm_cvtsi128_si32(res_temp1_4x32b);
                    /* row = 1 */
                    *(WORD32 *)(&pu1_dst[1 * dst_strd]) = dst1;

                    pi2_src += 4;   /* Pointer update */
                    pu1_dst += 4; /* Pointer update */

                } /* inner loop ends here(4-output values in single iteration) */
                pi2_src = pi2_src - wdx2 + 2 * src_strd;  /* Pointer update */
                pu1_dst = pu1_dst - wdx2 + 2 * dst_strd; /* Pointer update */
            }
        }
    }
}

/**
*******************************************************************************
*
* @brief
*  Does bi-weighted prediction on the arrays pointed by  pi2_src1 and
* pi2_src2 and stores it at location pointed  by pi2_dst
*
* @par Description:
*  dst = ( (src1 + lvl_shift1)*wgt0 +  (src2 + lvl_shift2)*wgt1 +  (off0 +
* off1 + 1) << (shift - 1) ) >> shift
*
* @param[in] pi2_src1
*  Pointer to source 1
*
* @param[in] pi2_src2
*  Pointer to source 2
*
* @param[out] pu1_dst
*  Pointer to destination
*
* @param[in] src_strd1
*  Source stride 1
*
* @param[in] src_strd2
*  Source stride 2
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] wgt0
*  weight to be multiplied to source 1
*
* @param[in] off0
*  offset 0
*
* @param[in] wgt1
*  weight to be multiplied to source 2
*
* @param[in] off1
*  offset 1
*
* @param[in] shift
*  (14 Bit depth) + log2_weight_denominator
*
* @param[in] lvl_shift1
*  added before shift and offset
*
* @param[in] lvl_shift2
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/


void ihevc_weighted_pred_bi_ssse3(WORD16 *pi2_src1,
                                  WORD16 *pi2_src2,
                                  UWORD8 *pu1_dst,
                                  WORD32 src_strd1,
                                  WORD32 src_strd2,
                                  WORD32 dst_strd,
                                  WORD32 wgt0,
                                  WORD32 off0,
                                  WORD32 wgt1,
                                  WORD32 off1,
                                  WORD32 shift,
                                  WORD32 lvl_shift1,
                                  WORD32 lvl_shift2,
                                  WORD32 ht,
                                  WORD32 wd)
{
    WORD32 row, col, temp;

    __m128i src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b, src_temp4_8x16b;
    __m128i const_temp_4x32b, lvl_shift1_4x32b, lvl_shift2_4x32b, wgt0_8x16b, wgt1_8x16b;
    __m128i res_temp1_4x32b, res_temp2_4x32b, res_temp3_4x32b, res_temp4_4x32b;

#include <assert.h>
    ASSERT(wd % 4 == 0); /* checking assumption*/
    ASSERT(ht % 4 == 0); /* checking assumption*/

    temp = (off0 + off1 + 1) << (shift - 1);

    // seting values in register
    lvl_shift1_4x32b = _mm_set1_epi16(lvl_shift1);
    wgt0_8x16b = _mm_set1_epi16(wgt0);
    lvl_shift2_4x32b = _mm_set1_epi16(lvl_shift2);
    wgt1_8x16b = _mm_set1_epi16(wgt1);

    /* lvl_shift1 * wgt0 */
    res_temp1_4x32b = _mm_mullo_epi16(lvl_shift1_4x32b, wgt0_8x16b);
    res_temp2_4x32b = _mm_mulhi_epi16(lvl_shift1_4x32b, wgt0_8x16b);
    /* lvl_shift2 * wgt1 */
    res_temp3_4x32b = _mm_mullo_epi16(lvl_shift2_4x32b, wgt1_8x16b);
    res_temp4_4x32b = _mm_mulhi_epi16(lvl_shift2_4x32b, wgt1_8x16b);

    const_temp_4x32b = _mm_set1_epi32(temp);

    /* lvl_shift1 * wgt0 */
    lvl_shift1_4x32b = _mm_unpacklo_epi16(res_temp1_4x32b, res_temp2_4x32b);
    /* lvl_shift2 * wgt1 */
    lvl_shift2_4x32b = _mm_unpacklo_epi16(res_temp3_4x32b, res_temp4_4x32b);

    if(0 == (wd & 7)) /* wd multiple of 8 case */
    {
        __m128i res_temp5_4x32b, res_temp6_4x32b, res_temp7_4x32b, res_temp8_4x32b;
        /*  outer for loop starts from here */
        for(row = 0; row < ht; row += 2)
        {
            for(col = 0; col < wd; col += 8)
            {
                /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1)); /* row = 0 */
                src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2)); /* row = 0 */
                src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 1 * src_strd1)); /* row = 1 */
                src_temp4_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 1 * src_strd2)); /* row = 1 */

                /*i4_tmp = (pi2_src[col]) * wgt*/ /* Lower 16 bit */
                res_temp1_4x32b  = _mm_mullo_epi16(src_temp1_8x16b, wgt0_8x16b);
                res_temp2_4x32b  = _mm_mullo_epi16(src_temp2_8x16b, wgt1_8x16b);
                res_temp3_4x32b  = _mm_mullo_epi16(src_temp3_8x16b, wgt0_8x16b);
                res_temp4_4x32b  = _mm_mullo_epi16(src_temp4_8x16b, wgt1_8x16b);
                /*i4_tmp = (pi2_src[col] ) * wgt*/ /* Higher 16 bit */
                src_temp1_8x16b  = _mm_mulhi_epi16(src_temp1_8x16b, wgt0_8x16b);
                src_temp2_8x16b  = _mm_mulhi_epi16(src_temp2_8x16b, wgt1_8x16b);
                src_temp3_8x16b  = _mm_mulhi_epi16(src_temp3_8x16b, wgt0_8x16b);
                src_temp4_8x16b  = _mm_mulhi_epi16(src_temp4_8x16b, wgt1_8x16b);

                /* Get 32 bit Result */
                res_temp5_4x32b = _mm_unpackhi_epi16(res_temp1_4x32b, src_temp1_8x16b);
                res_temp6_4x32b = _mm_unpackhi_epi16(res_temp2_4x32b, src_temp2_8x16b);
                res_temp7_4x32b = _mm_unpackhi_epi16(res_temp3_4x32b, src_temp3_8x16b);
                res_temp8_4x32b = _mm_unpackhi_epi16(res_temp4_4x32b, src_temp4_8x16b);

                res_temp1_4x32b = _mm_unpacklo_epi16(res_temp1_4x32b, src_temp1_8x16b);
                res_temp2_4x32b = _mm_unpacklo_epi16(res_temp2_4x32b, src_temp2_8x16b);
                res_temp3_4x32b = _mm_unpacklo_epi16(res_temp3_4x32b, src_temp3_8x16b);
                res_temp4_4x32b = _mm_unpacklo_epi16(res_temp4_4x32b, src_temp4_8x16b);

                /* (pi2_src[col] + lvl_shift) * wgt */
                res_temp5_4x32b = _mm_add_epi32(res_temp5_4x32b, lvl_shift1_4x32b);
                res_temp6_4x32b = _mm_add_epi32(res_temp6_4x32b, lvl_shift2_4x32b);
                res_temp7_4x32b = _mm_add_epi32(res_temp7_4x32b, lvl_shift1_4x32b);
                res_temp8_4x32b = _mm_add_epi32(res_temp8_4x32b, lvl_shift2_4x32b);
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, lvl_shift1_4x32b);
                res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, lvl_shift2_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, lvl_shift1_4x32b);
                res_temp4_4x32b = _mm_add_epi32(res_temp4_4x32b, lvl_shift2_4x32b);

                /* (pi2_src1[col] + lvl_shift1) * wgt0 + (pi2_src2[col] + lvl_shift2) * wgt1 */
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, res_temp2_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, res_temp4_4x32b);
                /* i4_tmp += (off0 + off1 + 1) << (shift - 1); */
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, const_temp_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, const_temp_4x32b);
                /* (i4_tmp >> shift) */
                res_temp1_4x32b = _mm_srai_epi32(res_temp1_4x32b,  shift);
                res_temp3_4x32b = _mm_srai_epi32(res_temp3_4x32b,  shift);

                /* Next 4 Pixels */
                res_temp5_4x32b = _mm_add_epi32(res_temp5_4x32b, res_temp6_4x32b);
                res_temp7_4x32b = _mm_add_epi32(res_temp7_4x32b, res_temp8_4x32b);
                res_temp5_4x32b = _mm_add_epi32(res_temp5_4x32b, const_temp_4x32b);
                res_temp7_4x32b = _mm_add_epi32(res_temp7_4x32b, const_temp_4x32b);
                res_temp5_4x32b = _mm_srai_epi32(res_temp5_4x32b,  shift);
                res_temp7_4x32b = _mm_srai_epi32(res_temp7_4x32b,  shift);

                res_temp1_4x32b = _mm_packs_epi32(res_temp1_4x32b, res_temp5_4x32b);
                res_temp3_4x32b = _mm_packs_epi32(res_temp3_4x32b, res_temp7_4x32b);

                /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */
                res_temp1_4x32b = _mm_packus_epi16(res_temp1_4x32b, res_temp1_4x32b);
                res_temp3_4x32b = _mm_packus_epi16(res_temp3_4x32b, res_temp3_4x32b);

                /* store four 8-bit output values  */
                _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd), res_temp1_4x32b); /* row = 0*/
                _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), res_temp3_4x32b); /* row = 1*/

                pi2_src1 += 8;  /* Pointer update */
                pi2_src2 += 8;  /* Pointer update */
                pu1_dst  += 8;  /* Pointer update */

            } /* inner loop ends here(4-output values in single iteration) */

            pi2_src1 = pi2_src1 - wd + 2 * src_strd1;  /* Pointer update */
            pi2_src2 = pi2_src2 - wd + 2 * src_strd2;  /* Pointer update */
            pu1_dst  = pu1_dst  - wd + 2 * dst_strd;   /* Pointer update */

        } /* outer loop ends */
    }
    else /* wd multiple of 4 case */
    {
        WORD32 dst0, dst1;
        /*  outer for loop starts from here */
        for(row = 0; row < ht; row += 2)
        {
            for(col = 0; col < wd; col += 4)
            {
                /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                src_temp1_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1)); /* row = 0 */
                src_temp2_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2)); /* row = 0 */
                src_temp3_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1 + 1 * src_strd1)); /* row = 1 */
                src_temp4_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2 + 1 * src_strd2)); /* row = 1 */

                /* 2 rows together */
                src_temp1_8x16b = _mm_unpacklo_epi64(src_temp1_8x16b, src_temp3_8x16b);
                src_temp2_8x16b = _mm_unpacklo_epi64(src_temp2_8x16b, src_temp4_8x16b);

                /*i4_tmp = (pi2_src[col]) * wgt*/ /* Lower 16 bit */
                res_temp1_4x32b  = _mm_mullo_epi16(src_temp1_8x16b, wgt0_8x16b);
                res_temp2_4x32b  = _mm_mullo_epi16(src_temp2_8x16b, wgt1_8x16b);
                /*i4_tmp = (pi2_src[col] ) * wgt*/ /* Higher 16 bit */
                src_temp1_8x16b  = _mm_mulhi_epi16(src_temp1_8x16b, wgt0_8x16b);
                src_temp2_8x16b  = _mm_mulhi_epi16(src_temp2_8x16b, wgt1_8x16b);

                /* Get 32 bit Result */
                res_temp3_4x32b = _mm_unpackhi_epi16(res_temp1_4x32b, src_temp1_8x16b);
                res_temp4_4x32b = _mm_unpackhi_epi16(res_temp2_4x32b, src_temp2_8x16b);

                res_temp1_4x32b = _mm_unpacklo_epi16(res_temp1_4x32b, src_temp1_8x16b);
                res_temp2_4x32b = _mm_unpacklo_epi16(res_temp2_4x32b, src_temp2_8x16b);

                /* (pi2_src[col] + lvl_shift) * wgt */
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, lvl_shift1_4x32b);
                res_temp4_4x32b = _mm_add_epi32(res_temp4_4x32b, lvl_shift2_4x32b);
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, lvl_shift1_4x32b);
                res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, lvl_shift2_4x32b);

                /* (pi2_src1[col] + lvl_shift1) * wgt0 + (pi2_src2[col] + lvl_shift2) * wgt1 */
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, res_temp2_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, res_temp4_4x32b);

                /* i4_tmp += (off0 + off1 + 1) << (shift - 1); */
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, const_temp_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, const_temp_4x32b);

                /* (i4_tmp >> shift) */
                res_temp1_4x32b = _mm_srai_epi32(res_temp1_4x32b,  shift);
                res_temp3_4x32b = _mm_srai_epi32(res_temp3_4x32b,  shift);

                res_temp1_4x32b = _mm_packs_epi32(res_temp1_4x32b, res_temp3_4x32b);

                /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */
                res_temp1_4x32b = _mm_packus_epi16(res_temp1_4x32b, res_temp1_4x32b);

                dst0 = _mm_cvtsi128_si32(res_temp1_4x32b);

                /* dst row = 1 to 3 */
                res_temp2_4x32b = _mm_shuffle_epi32(res_temp1_4x32b, 1);

                /* store four 8-bit output values  */
                *(WORD32 *)(&pu1_dst[0 * dst_strd]) = dst0;

                dst1 = _mm_cvtsi128_si32(res_temp2_4x32b);

                /* row = 1 */
                *(WORD32 *)(&pu1_dst[1 * dst_strd]) = dst1;

                pi2_src1 += 4;  /* Pointer update */
                pi2_src2 += 4;  /* Pointer update */
                pu1_dst  += 4;  /* Pointer update */

            } /* inner loop ends here(4-output values in single iteration) */

            pi2_src1 = pi2_src1 - wd + 2 * src_strd1;  /* Pointer update */
            pi2_src2 = pi2_src2 - wd + 2 * src_strd2;  /* Pointer update */
            pu1_dst  = pu1_dst  - wd + 2 * dst_strd;   /* Pointer update */

        } /* outer loop ends */
    }

}

/**
*******************************************************************************
*
* @brief
* Does chroma bi-weighted prediction on the arrays pointed by  pi2_src1 and
* pi2_src2 and stores it at location pointed  by pi2_dst
*
* @par Description:
*  dst = ( (src1 + lvl_shift1)*wgt0 +  (src2 + lvl_shift2)*wgt1 +  (off0 +
* off1 + 1) << (shift - 1) ) >> shift
*
* @param[in] pi2_src1
*  Pointer to source 1
*
* @param[in] pi2_src2
*  Pointer to source 2
*
* @param[out] pu1_dst
*  Pointer to destination
*
* @param[in] src_strd1
*  Source stride 1
*
* @param[in] src_strd2
*  Source stride 2
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] wgt0
*  weight to be multiplied to source 1
*
* @param[in] off0
*  offset 0
*
* @param[in] wgt1
*  weight to be multiplied to source 2
*
* @param[in] off1
*  offset 1
*
* @param[in] shift
*  (14 Bit depth) + log2_weight_denominator
*
* @param[in] lvl_shift1
*  added before shift and offset
*
* @param[in] lvl_shift2
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source (each colour component)
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/


void ihevc_weighted_pred_chroma_bi_ssse3(WORD16 *pi2_src1,
                                         WORD16 *pi2_src2,
                                         UWORD8 *pu1_dst,
                                         WORD32 src_strd1,
                                         WORD32 src_strd2,
                                         WORD32 dst_strd,
                                         WORD32 wgt0_cb,
                                         WORD32 wgt0_cr,
                                         WORD32 off0_cb,
                                         WORD32 off0_cr,
                                         WORD32 wgt1_cb,
                                         WORD32 wgt1_cr,
                                         WORD32 off1_cb,
                                         WORD32 off1_cr,
                                         WORD32 shift,
                                         WORD32 lvl_shift1,
                                         WORD32 lvl_shift2,
                                         WORD32 ht,
                                         WORD32 wd)
{
    WORD32 row, col, temp1, temp2;
    WORD32 wdx2;

    __m128i src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b, src_temp4_8x16b;
    __m128i const_temp_4x32b, lvl_shift1_4x32b, lvl_shift2_4x32b, wgt0_8x16b, wgt1_8x16b;
    __m128i res_temp1_4x32b, res_temp2_4x32b, res_temp3_4x32b, res_temp4_4x32b;

    ASSERT(wd % 2 == 0); /* checking assumption*/
    ASSERT(ht % 2 == 0); /* checking assumption*/

    temp1 = (off0_cb + off1_cb + 1) << (shift - 1);
    temp2 = (off0_cr + off1_cr + 1) << (shift - 1);

    // seting values in register
    lvl_shift1_4x32b = _mm_set1_epi16(lvl_shift1);
    wgt0_8x16b = _mm_set_epi16(wgt0_cr, wgt0_cb, wgt0_cr, wgt0_cb, wgt0_cr, wgt0_cb, wgt0_cr, wgt0_cb);
    lvl_shift2_4x32b = _mm_set1_epi16(lvl_shift2);
    wgt1_8x16b = _mm_set_epi16(wgt1_cr, wgt1_cb, wgt1_cr, wgt1_cb, wgt1_cr, wgt1_cb, wgt1_cr, wgt1_cb);

    /* lvl_shift1 * wgt0 */
    res_temp1_4x32b = _mm_mullo_epi16(lvl_shift1_4x32b, wgt0_8x16b);
    res_temp2_4x32b = _mm_mulhi_epi16(lvl_shift1_4x32b, wgt0_8x16b);
    /* lvl_shift2 * wgt1 */
    res_temp3_4x32b = _mm_mullo_epi16(lvl_shift2_4x32b, wgt1_8x16b);
    res_temp4_4x32b = _mm_mulhi_epi16(lvl_shift2_4x32b, wgt1_8x16b);

    const_temp_4x32b = _mm_set_epi32(temp2, temp1, temp2, temp1);
    wdx2 = wd * 2;

    /* lvl_shift1 * wgt0 */
    lvl_shift1_4x32b = _mm_unpacklo_epi16(res_temp1_4x32b, res_temp2_4x32b);
    /* lvl_shift2 * wgt1 */
    lvl_shift2_4x32b = _mm_unpacklo_epi16(res_temp3_4x32b, res_temp4_4x32b);

    if(0 == (wdx2 & 7)) /* wdx2 multiple of 8 case */
    {
        __m128i res_temp5_4x32b, res_temp6_4x32b, res_temp7_4x32b, res_temp8_4x32b;
        /*  outer for loop starts from here */
        for(row = 0; row < ht; row += 2)
        {
            for(col = 0; col < wdx2; col += 8)
            {
                /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1)); /* row = 0 */
                src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2)); /* row = 0 */
                src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 1 * src_strd1)); /* row = 1 */
                src_temp4_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 1 * src_strd2)); /* row = 1 */

                /*i4_tmp = (pi2_src[col]) * wgt*/ /* Lower 16 bit */
                res_temp1_4x32b  = _mm_mullo_epi16(src_temp1_8x16b, wgt0_8x16b);
                res_temp2_4x32b  = _mm_mullo_epi16(src_temp2_8x16b, wgt1_8x16b);
                res_temp3_4x32b  = _mm_mullo_epi16(src_temp3_8x16b, wgt0_8x16b);
                res_temp4_4x32b  = _mm_mullo_epi16(src_temp4_8x16b, wgt1_8x16b);
                /*i4_tmp = (pi2_src[col] ) * wgt*/ /* Higher 16 bit */
                src_temp1_8x16b  = _mm_mulhi_epi16(src_temp1_8x16b, wgt0_8x16b);
                src_temp2_8x16b  = _mm_mulhi_epi16(src_temp2_8x16b, wgt1_8x16b);
                src_temp3_8x16b  = _mm_mulhi_epi16(src_temp3_8x16b, wgt0_8x16b);
                src_temp4_8x16b  = _mm_mulhi_epi16(src_temp4_8x16b, wgt1_8x16b);

                /* Get 32 bit Result */
                res_temp5_4x32b = _mm_unpackhi_epi16(res_temp1_4x32b, src_temp1_8x16b);
                res_temp6_4x32b = _mm_unpackhi_epi16(res_temp2_4x32b, src_temp2_8x16b);
                res_temp7_4x32b = _mm_unpackhi_epi16(res_temp3_4x32b, src_temp3_8x16b);
                res_temp8_4x32b = _mm_unpackhi_epi16(res_temp4_4x32b, src_temp4_8x16b);

                res_temp1_4x32b = _mm_unpacklo_epi16(res_temp1_4x32b, src_temp1_8x16b);
                res_temp2_4x32b = _mm_unpacklo_epi16(res_temp2_4x32b, src_temp2_8x16b);
                res_temp3_4x32b = _mm_unpacklo_epi16(res_temp3_4x32b, src_temp3_8x16b);
                res_temp4_4x32b = _mm_unpacklo_epi16(res_temp4_4x32b, src_temp4_8x16b);

                /* (pi2_src[col] + lvl_shift) * wgt */
                res_temp5_4x32b = _mm_add_epi32(res_temp5_4x32b, lvl_shift1_4x32b);
                res_temp6_4x32b = _mm_add_epi32(res_temp6_4x32b, lvl_shift2_4x32b);
                res_temp7_4x32b = _mm_add_epi32(res_temp7_4x32b, lvl_shift1_4x32b);
                res_temp8_4x32b = _mm_add_epi32(res_temp8_4x32b, lvl_shift2_4x32b);
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, lvl_shift1_4x32b);
                res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, lvl_shift2_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, lvl_shift1_4x32b);
                res_temp4_4x32b = _mm_add_epi32(res_temp4_4x32b, lvl_shift2_4x32b);

                /* (pi2_src1[col] + lvl_shift1) * wgt0 + (pi2_src2[col] + lvl_shift2) * wgt1 */
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, res_temp2_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, res_temp4_4x32b);
                /* i4_tmp += (off0 + off1 + 1) << (shift - 1); */
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, const_temp_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, const_temp_4x32b);
                /* (i4_tmp >> shift) */
                res_temp1_4x32b = _mm_srai_epi32(res_temp1_4x32b,  shift);
                res_temp3_4x32b = _mm_srai_epi32(res_temp3_4x32b,  shift);

                /* Next 4 Pixels */
                res_temp5_4x32b = _mm_add_epi32(res_temp5_4x32b, res_temp6_4x32b);
                res_temp7_4x32b = _mm_add_epi32(res_temp7_4x32b, res_temp8_4x32b);
                res_temp5_4x32b = _mm_add_epi32(res_temp5_4x32b, const_temp_4x32b);
                res_temp7_4x32b = _mm_add_epi32(res_temp7_4x32b, const_temp_4x32b);
                res_temp5_4x32b = _mm_srai_epi32(res_temp5_4x32b,  shift);
                res_temp7_4x32b = _mm_srai_epi32(res_temp7_4x32b,  shift);

                res_temp1_4x32b = _mm_packs_epi32(res_temp1_4x32b, res_temp5_4x32b);
                res_temp3_4x32b = _mm_packs_epi32(res_temp3_4x32b, res_temp7_4x32b);

                /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */
                res_temp1_4x32b = _mm_packus_epi16(res_temp1_4x32b, res_temp1_4x32b);
                res_temp3_4x32b = _mm_packus_epi16(res_temp3_4x32b, res_temp3_4x32b);

                /* store four 8-bit output values  */
                _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd), res_temp1_4x32b); /* row = 0*/
                _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), res_temp3_4x32b); /* row = 1*/

                pi2_src1 += 8;  /* Pointer update */
                pi2_src2 += 8;  /* Pointer update */
                pu1_dst  += 8;  /* Pointer update */

            } /* inner loop ends here(4-output values in single iteration) */

            pi2_src1 = pi2_src1 - wdx2 + 2 * src_strd1;    /* Pointer update */
            pi2_src2 = pi2_src2 - wdx2 + 2 * src_strd2;    /* Pointer update */
            pu1_dst  = pu1_dst  - wdx2 + 2 * dst_strd;   /* Pointer update */

        } /* outer loop ends */
    }
    else /* wdx2 multiple of 4 case */
    {
        WORD32 dst0, dst1;
        /*  outer for loop starts from here */
        for(row = 0; row < ht; row += 2)
        {
            for(col = 0; col < wdx2; col += 4)
            {
                /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                src_temp1_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1)); /* row = 0 */
                src_temp2_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2)); /* row = 0 */
                src_temp3_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1 + 1 * src_strd1)); /* row = 1 */
                src_temp4_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2 + 1 * src_strd2)); /* row = 1 */

                /* 2 rows together */
                src_temp1_8x16b = _mm_unpacklo_epi64(src_temp1_8x16b, src_temp3_8x16b);
                src_temp2_8x16b = _mm_unpacklo_epi64(src_temp2_8x16b, src_temp4_8x16b);

                /*i4_tmp = (pi2_src[col]) * wgt*/ /* Lower 16 bit */
                res_temp1_4x32b  = _mm_mullo_epi16(src_temp1_8x16b, wgt0_8x16b);
                res_temp2_4x32b  = _mm_mullo_epi16(src_temp2_8x16b, wgt1_8x16b);
                /*i4_tmp = (pi2_src[col] ) * wgt*/ /* Higher 16 bit */
                src_temp1_8x16b  = _mm_mulhi_epi16(src_temp1_8x16b, wgt0_8x16b);
                src_temp2_8x16b  = _mm_mulhi_epi16(src_temp2_8x16b, wgt1_8x16b);

                /* Get 32 bit Result */
                res_temp3_4x32b = _mm_unpackhi_epi16(res_temp1_4x32b, src_temp1_8x16b);
                res_temp4_4x32b = _mm_unpackhi_epi16(res_temp2_4x32b, src_temp2_8x16b);

                res_temp1_4x32b = _mm_unpacklo_epi16(res_temp1_4x32b, src_temp1_8x16b);
                res_temp2_4x32b = _mm_unpacklo_epi16(res_temp2_4x32b, src_temp2_8x16b);

                /* (pi2_src[col] + lvl_shift) * wgt */
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, lvl_shift1_4x32b);
                res_temp4_4x32b = _mm_add_epi32(res_temp4_4x32b, lvl_shift2_4x32b);
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, lvl_shift1_4x32b);
                res_temp2_4x32b = _mm_add_epi32(res_temp2_4x32b, lvl_shift2_4x32b);

                /* (pi2_src1[col] + lvl_shift1) * wgt0 + (pi2_src2[col] + lvl_shift2) * wgt1 */
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, res_temp2_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, res_temp4_4x32b);

                /* i4_tmp += (off0 + off1 + 1) << (shift - 1); */
                res_temp1_4x32b = _mm_add_epi32(res_temp1_4x32b, const_temp_4x32b);
                res_temp3_4x32b = _mm_add_epi32(res_temp3_4x32b, const_temp_4x32b);

                /* (i4_tmp >> shift) */
                res_temp1_4x32b = _mm_srai_epi32(res_temp1_4x32b,  shift);
                res_temp3_4x32b = _mm_srai_epi32(res_temp3_4x32b,  shift);

                res_temp1_4x32b = _mm_packs_epi32(res_temp1_4x32b, res_temp3_4x32b);

                /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */
                res_temp1_4x32b = _mm_packus_epi16(res_temp1_4x32b, res_temp1_4x32b);

                dst0 = _mm_cvtsi128_si32(res_temp1_4x32b);

                /* dst row = 1 to 3 */
                res_temp2_4x32b = _mm_shuffle_epi32(res_temp1_4x32b, 1);

                /* store four 8-bit output values  */
                *(WORD32 *)(&pu1_dst[0 * dst_strd]) = dst0;

                dst1 = _mm_cvtsi128_si32(res_temp2_4x32b);

                /* row = 1 */
                *(WORD32 *)(&pu1_dst[1 * dst_strd]) = dst1;

                pi2_src1 += 4;  /* Pointer update */
                pi2_src2 += 4;  /* Pointer update */
                pu1_dst  += 4;  /* Pointer update */

            } /* inner loop ends here(4-output values in single iteration) */

            pi2_src1 = pi2_src1 - wdx2 + 2 * src_strd1;    /* Pointer update */
            pi2_src2 = pi2_src2 - wdx2 + 2 * src_strd2;    /* Pointer update */
            pu1_dst  = pu1_dst  - wdx2 + 2 * dst_strd;   /* Pointer update */
        }
    }

}

/**
*******************************************************************************
*
* @brief
*  Does default bi-weighted prediction on the arrays pointed by pi2_src1 and
* pi2_src2 and stores it at location  pointed by pi2_dst
*
* @par Description:
*  dst = ( (src1 + lvl_shift1) +  (src2 + lvl_shift2) +  1 << (shift - 1) )
* >> shift  where shift = 15 - BitDepth
*
* @param[in] pi2_src1
*  Pointer to source 1
*
* @param[in] pi2_src2
*  Pointer to source 2
*
* @param[out] pu1_dst
*  Pointer to destination
*
* @param[in] src_strd1
*  Source stride 1
*
* @param[in] src_strd2
*  Source stride 2
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] lvl_shift1
*  added before shift and offset
*
* @param[in] lvl_shift2
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source
*
* @returns
*
* @remarks
*  None
*
* Assumption : ht%4 == 0, wd%4 == 0
* shift == 7, (lvl_shift1+lvl_shift2) can take {0, 8K, 16K}. In that case,
* final result will match even if intermediate precision is in 16 bit.
*
*******************************************************************************
*/
void ihevc_weighted_pred_bi_default_ssse3(WORD16 *pi2_src1,
                                          WORD16 *pi2_src2,
                                          UWORD8 *pu1_dst,
                                          WORD32 src_strd1,
                                          WORD32 src_strd2,
                                          WORD32 dst_strd,
                                          WORD32 lvl_shift1,
                                          WORD32 lvl_shift2,
                                          WORD32 ht,
                                          WORD32 wd)
{
    {
        WORD32 row, col, temp;
        WORD32 shift;

        __m128i src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b, src_temp4_8x16b;
        __m128i const_temp_8x16b, lvl_shift1_8x16b, lvl_shift2_8x16b;
        __m128i src_temp5_8x16b, src_temp6_8x16b, src_temp7_8x16b, src_temp8_8x16b;

        ASSERT(wd % 4 == 0); /* checking assumption*/
        ASSERT(ht % 2 == 0); /* checking assumption*/

        shift = SHIFT_14_MINUS_BIT_DEPTH + 1;
        temp = 1 << (shift - 1);

        // seting values in register
        lvl_shift1_8x16b = _mm_set1_epi16(lvl_shift1);
        lvl_shift2_8x16b = _mm_set1_epi16(lvl_shift2);
        const_temp_8x16b = _mm_set1_epi16(temp);

        lvl_shift1_8x16b = _mm_adds_epi16(lvl_shift1_8x16b, lvl_shift2_8x16b);
        lvl_shift1_8x16b = _mm_adds_epi16(lvl_shift1_8x16b, const_temp_8x16b);

        if(0 == (ht & 3)) /* ht multiple of 4*/
        {
            if(0 == (wd & 15)) /* wd multiple of 16 case */
            {
                __m128i src_temp9_8x16b,  src_temp10_8x16b, src_temp11_8x16b, src_temp12_8x16b;
                __m128i src_temp13_8x16b, src_temp14_8x16b, src_temp15_8x16b, src_temp16_8x16b;
                /*  outer for loop starts from here */
                for(row = 0; row < ht; row += 4)
                {
                    for(col = 0; col < wd; col += 16)
                    {
                        /*load 8 pixel values */ /* First 8 Values */
                        src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1));
                        src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2));
                        /* row = 1 */
                        src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + src_strd1));
                        src_temp4_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + src_strd2));
                        /* row = 2 */
                        src_temp5_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 2 * src_strd1));
                        src_temp6_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 2 * src_strd2));
                        /* row = 3 */
                        src_temp7_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 3 * src_strd1));
                        src_temp8_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 3 * src_strd2));

                        /*load 8 pixel values */ /* Second 8 Values */
                        src_temp9_8x16b  = _mm_loadu_si128((__m128i *)(pi2_src1 + 8));
                        src_temp10_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 8));
                        /* row = 1 */
                        src_temp11_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + src_strd1 + 8));
                        src_temp12_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + src_strd2 + 8));
                        /* row = 2 */
                        src_temp13_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 2 * src_strd1 + 8));
                        src_temp14_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 2 * src_strd2 + 8));

                        /* (pi2_src1[col] + pi2_src2[col]) */ /* First 8 Values */
                        src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, src_temp2_8x16b);
                        src_temp3_8x16b = _mm_adds_epi16(src_temp3_8x16b, src_temp4_8x16b);
                        src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, src_temp6_8x16b);
                        src_temp7_8x16b = _mm_adds_epi16(src_temp7_8x16b, src_temp8_8x16b);

                        /*load 8 pixel values */ /* Second 8 Values */
                        /* row = 3 */
                        src_temp15_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 3 * src_strd1 + 8));
                        src_temp16_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 3 * src_strd2 + 8));

                        /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */ /* First 8 Values */
                        src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, lvl_shift1_8x16b);
                        src_temp3_8x16b = _mm_adds_epi16(src_temp3_8x16b, lvl_shift1_8x16b);
                        src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, lvl_shift1_8x16b);
                        src_temp7_8x16b = _mm_adds_epi16(src_temp7_8x16b, lvl_shift1_8x16b);

                        /* (pi2_src1[col] + pi2_src2[col]) */ /* Second 8 Values */
                        src_temp9_8x16b  = _mm_adds_epi16(src_temp9_8x16b,  src_temp10_8x16b);
                        src_temp11_8x16b = _mm_adds_epi16(src_temp11_8x16b, src_temp12_8x16b);
                        src_temp13_8x16b = _mm_adds_epi16(src_temp13_8x16b, src_temp14_8x16b);
                        src_temp15_8x16b = _mm_adds_epi16(src_temp15_8x16b, src_temp16_8x16b);

                        /* (i4_tmp >> shift) */ /* First 8 Values */
                        src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  shift);
                        src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  shift);
                        src_temp5_8x16b = _mm_srai_epi16(src_temp5_8x16b,  shift);
                        src_temp7_8x16b = _mm_srai_epi16(src_temp7_8x16b,  shift);

                        /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */ /* Second 8 Values */
                        src_temp9_8x16b  = _mm_adds_epi16(src_temp9_8x16b, lvl_shift1_8x16b);
                        src_temp11_8x16b = _mm_adds_epi16(src_temp11_8x16b, lvl_shift1_8x16b);
                        src_temp13_8x16b = _mm_adds_epi16(src_temp13_8x16b, lvl_shift1_8x16b);
                        src_temp15_8x16b = _mm_adds_epi16(src_temp15_8x16b, lvl_shift1_8x16b);

                        /* (i4_tmp >> shift) */ /* Second 8 Values */
                        src_temp9_8x16b  = _mm_srai_epi16(src_temp9_8x16b,  shift);
                        src_temp11_8x16b = _mm_srai_epi16(src_temp11_8x16b,  shift);
                        src_temp13_8x16b = _mm_srai_epi16(src_temp13_8x16b,  shift);
                        src_temp15_8x16b = _mm_srai_epi16(src_temp15_8x16b,  shift);

                        /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */ /* 16 8 Values */
                        src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp9_8x16b);
                        src_temp3_8x16b = _mm_packus_epi16(src_temp3_8x16b, src_temp11_8x16b);
                        src_temp5_8x16b = _mm_packus_epi16(src_temp5_8x16b, src_temp13_8x16b);
                        src_temp7_8x16b = _mm_packus_epi16(src_temp7_8x16b, src_temp15_8x16b);

                        /* store four 8-bit output values  */ /* 16 8 Values */
                        _mm_storeu_si128((__m128i *)(pu1_dst + 0 * dst_strd), src_temp1_8x16b); /* row = 0*/
                        _mm_storeu_si128((__m128i *)(pu1_dst + 1 * dst_strd), src_temp3_8x16b); /* row = 2*/
                        _mm_storeu_si128((__m128i *)(pu1_dst + 2 * dst_strd), src_temp5_8x16b); /* row = 1*/
                        _mm_storeu_si128((__m128i *)(pu1_dst + 3 * dst_strd), src_temp7_8x16b); /* row = 3*/

                        /* To update pointer */
                        pi2_src1 += 16;
                        pi2_src2 += 16;
                        pu1_dst  += 16;

                    } /* inner loop ends here(8-output values in single iteration) */

                    pi2_src1 = pi2_src1 - wd + 4 * src_strd1;  /* Pointer update */
                    pi2_src2 = pi2_src2 - wd + 4 * src_strd2;  /* Pointer update */
                    pu1_dst  = pu1_dst - wd + 4 * dst_strd;   /* Pointer update */

                }
            }
            else if(0 == (wd & 7)) /* multiple of 8 case */
            {
                /*  outer for loop starts from here */
                for(row = 0; row < ht; row += 4)
                {
                    for(col = 0; col < wd; col += 8)
                    {
                        /*load 8 pixel values */
                        src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1));
                        src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2));
                        /* row = 1 */
                        src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + src_strd1));
                        src_temp4_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + src_strd2));
                        /* row = 2 */
                        src_temp5_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 2 * src_strd1));
                        src_temp6_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 2 * src_strd2));
                        /* row = 3 */
                        src_temp7_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 3 * src_strd1));
                        src_temp8_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 3 * src_strd2));

                        /* (pi2_src1[col] + pi2_src2[col]) */
                        src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, src_temp2_8x16b);
                        src_temp3_8x16b = _mm_adds_epi16(src_temp3_8x16b, src_temp4_8x16b);
                        src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, src_temp6_8x16b);
                        src_temp7_8x16b = _mm_adds_epi16(src_temp7_8x16b, src_temp8_8x16b);

                        /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */
                        src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, lvl_shift1_8x16b);
                        src_temp3_8x16b = _mm_adds_epi16(src_temp3_8x16b, lvl_shift1_8x16b);
                        src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, lvl_shift1_8x16b);
                        src_temp7_8x16b = _mm_adds_epi16(src_temp7_8x16b, lvl_shift1_8x16b);

                        /* (i4_tmp >> shift) */
                        src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  shift);
                        src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  shift);
                        src_temp5_8x16b = _mm_srai_epi16(src_temp5_8x16b,  shift);
                        src_temp7_8x16b = _mm_srai_epi16(src_temp7_8x16b,  shift);

                        /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */
                        src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp1_8x16b);
                        src_temp3_8x16b = _mm_packus_epi16(src_temp3_8x16b, src_temp3_8x16b);
                        src_temp5_8x16b = _mm_packus_epi16(src_temp5_8x16b, src_temp5_8x16b);
                        src_temp7_8x16b = _mm_packus_epi16(src_temp7_8x16b, src_temp7_8x16b);

                        /* store four 8-bit output values  */
                        _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd), src_temp1_8x16b); /* row = 0*/
                        _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), src_temp3_8x16b); /* row = 2*/
                        _mm_storel_epi64((__m128i *)(pu1_dst + 2 * dst_strd), src_temp5_8x16b); /* row = 1*/
                        _mm_storel_epi64((__m128i *)(pu1_dst + 3 * dst_strd), src_temp7_8x16b); /* row = 3*/

                        /* To update pointer */
                        pi2_src1 += 8;
                        pi2_src2 += 8;
                        pu1_dst  += 8;

                    } /* inner loop ends here(8-output values in single iteration) */

                    pi2_src1 = pi2_src1 - wd + 4 * src_strd1;  /* Pointer update */
                    pi2_src2 = pi2_src2 - wd + 4 * src_strd2;  /* Pointer update */
                    pu1_dst  = pu1_dst - wd + 4 * dst_strd;   /* Pointer update */

                }
            }
            else /* wd multiple of 4 case*/
            {
                WORD32 dst0, dst1, dst2, dst3;

                /*  outer for loop starts from here */
                for(row = 0; row < ht; row += 4)
                {
                    for(col = 0; col < wd; col += 4)
                    {
                        /*load 4 pixel values from 7:0 pos. relative to cur. pos.*/
                        src_temp1_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1));
                        /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                        src_temp2_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2));

                        /* row = 1 */
                        src_temp3_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1 + src_strd1));
                        src_temp4_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2 + src_strd2));
                        /* row = 2 */
                        src_temp5_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1 + 2 * src_strd1));
                        src_temp6_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2 + 2 * src_strd2));
                        /* row = 3 */
                        src_temp7_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1 + 3 * src_strd1));
                        src_temp8_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2 + 3 * src_strd2));

                        /* Pack two rows together */
                        src_temp1_8x16b = _mm_unpacklo_epi64(src_temp1_8x16b, src_temp3_8x16b);
                        src_temp2_8x16b = _mm_unpacklo_epi64(src_temp2_8x16b, src_temp4_8x16b);
                        src_temp5_8x16b = _mm_unpacklo_epi64(src_temp5_8x16b, src_temp7_8x16b);
                        src_temp6_8x16b = _mm_unpacklo_epi64(src_temp6_8x16b, src_temp8_8x16b);

                        /* (pi2_src1[col] + pi2_src2[col]) */
                        src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, src_temp2_8x16b);
                        src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, src_temp6_8x16b);

                        /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */
                        src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, lvl_shift1_8x16b);
                        src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, lvl_shift1_8x16b);

                        /* (i4_tmp >> shift) */
                        src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  shift);
                        src_temp5_8x16b = _mm_srai_epi16(src_temp5_8x16b,  shift);

                        /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */
                        src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp1_8x16b);
                        src_temp5_8x16b = _mm_packus_epi16(src_temp5_8x16b, src_temp5_8x16b);

                        dst0 = _mm_cvtsi128_si32(src_temp1_8x16b);
                        /* dst row = 1 to 3 */
                        src_temp2_8x16b = _mm_shuffle_epi32(src_temp1_8x16b, 1);
                        src_temp4_8x16b = _mm_shuffle_epi32(src_temp5_8x16b, 1);

                        /* store four 8-bit output values  */
                        *(WORD32 *)(&pu1_dst[0 * dst_strd]) = dst0;

                        dst1 = _mm_cvtsi128_si32(src_temp2_8x16b);
                        dst2 = _mm_cvtsi128_si32(src_temp5_8x16b);
                        dst3 = _mm_cvtsi128_si32(src_temp4_8x16b);

                        /* row = 1 to row = 3 */
                        *(WORD32 *)(&pu1_dst[1 * dst_strd]) = dst1;
                        *(WORD32 *)(&pu1_dst[2 * dst_strd]) = dst2;
                        *(WORD32 *)(&pu1_dst[3 * dst_strd]) = dst3;

                        /* To update pointer */
                        pi2_src1 += 4;
                        pi2_src2 += 4;
                        pu1_dst  += 4;

                    } /* inner loop ends here(4-output values in single iteration) */

                    pi2_src1 = pi2_src1 - wd + 4 * src_strd1; /* Pointer update */
                    pi2_src2 = pi2_src2 - wd + 4 * src_strd2; /* Pointer update */
                    pu1_dst  = pu1_dst  - wd + 4 * dst_strd;  /* Pointer update */

                }
            }
        }
        else /* ht multiple of 2 case and wd multiple of 4 case*/
        {

            WORD32 dst0, dst1;

            /*  outer for loop starts from here */
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wd; col += 4)
                {
                    /*load 4 pixel values from 7:0 pos. relative to cur. pos.*/
                    src_temp1_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1));
                    /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                    src_temp2_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2));

                    /* row = 1 */
                    src_temp3_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1 + src_strd1));
                    src_temp4_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2 + src_strd2));

                    /* Pack two rows together */
                    src_temp1_8x16b = _mm_unpacklo_epi64(src_temp1_8x16b, src_temp3_8x16b);
                    src_temp2_8x16b = _mm_unpacklo_epi64(src_temp2_8x16b, src_temp4_8x16b);

                    /* (pi2_src1[col] + pi2_src2[col]) */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, src_temp2_8x16b);

                    /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, lvl_shift1_8x16b);

                    /* (i4_tmp >> shift) */
                    src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  shift);

                    /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */
                    src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp1_8x16b);

                    dst0 = _mm_cvtsi128_si32(src_temp1_8x16b);
                    /* dst row = 1 to 3 */
                    src_temp2_8x16b = _mm_shuffle_epi32(src_temp1_8x16b, 1);

                    /* store four 8-bit output values  */
                    *(WORD32 *)(&pu1_dst[0 * dst_strd]) = dst0;

                    dst1 = _mm_cvtsi128_si32(src_temp2_8x16b);

                    /* row = 1 to row = 3 */
                    *(WORD32 *)(&pu1_dst[1 * dst_strd]) = dst1;

                    /* To update pointer */
                    pi2_src1 += 4;
                    pi2_src2 += 4;
                    pu1_dst  += 4;

                } /* inner loop ends here(4-output values in single iteration) */

                pi2_src1 = pi2_src1 - wd + 2 * src_strd1; /* Pointer update */
                pi2_src2 = pi2_src2 - wd + 2 * src_strd2; /* Pointer update */
                pu1_dst  = pu1_dst  - wd + 2 * dst_strd;  /* Pointer update */

            }

        }

    }
}


/**
*******************************************************************************
*
* @brief
*  Does chroma default bi-weighted prediction on arrays pointed by pi2_src1 and
* pi2_src2 and stores it at location  pointed by pi2_dst
*
* @par Description:
*  dst = ( (src1 + lvl_shift1) +  (src2 + lvl_shift2) +  1 << (shift - 1) )
* >> shift  where shift = 15 - BitDepth
*
* @param[in] pi2_src1
*  Pointer to source 1
*
* @param[in] pi2_src2
*  Pointer to source 2
*
* @param[out] pu1_dst
*  Pointer to destination
*
* @param[in] src_strd1
*  Source stride 1
*
* @param[in] src_strd2
*  Source stride 2
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] lvl_shift1
*  added before shift and offset
*
* @param[in] lvl_shift2
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source (each colour component)
*
* @returns
*
* @remarks
*  None
*
* Assumption : ht%2 == 0, wd%2 == 0, lvl_shift1==0, lvl_shift2==0.
* shift == 7, (lvl_shift1+lvl_shift2) can take {0, 8K, 16K}. In that case,
* final result will match even if intermediate precision is in 16 bit.
*******************************************************************************
*/

void ihevc_weighted_pred_chroma_bi_default_ssse3(WORD16 *pi2_src1,
                                                 WORD16 *pi2_src2,
                                                 UWORD8 *pu1_dst,
                                                 WORD32 src_strd1,
                                                 WORD32 src_strd2,
                                                 WORD32 dst_strd,
                                                 WORD32 lvl_shift1,
                                                 WORD32 lvl_shift2,
                                                 WORD32 ht,
                                                 WORD32 wd)
{
    WORD32 row, col, temp;
    WORD32 shift, wdx2;

    __m128i src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b, src_temp4_8x16b;
    __m128i lvl_shift1_8x16b;
    __m128i src_temp5_8x16b, src_temp6_8x16b, src_temp7_8x16b, src_temp8_8x16b;

    ASSERT(wd % 2 == 0); /* checking assumption*/
    ASSERT(ht % 2 == 0); /* checking assumption*/
    UNUSED(lvl_shift1);
    UNUSED(lvl_shift2);
    shift = SHIFT_14_MINUS_BIT_DEPTH + 1;
    temp = 1 << (shift - 1);
    wdx2 = wd * 2;

    // seting values in register
    lvl_shift1_8x16b = _mm_set1_epi16(temp);

    if(0 == (ht & 3)) /* ht multiple of 4 case */
    {
        if(0 == (wdx2 & 15)) /* 2*wd multiple of 16 case */
        {
            __m128i src_temp9_8x16b,  src_temp10_8x16b, src_temp11_8x16b, src_temp12_8x16b;
            __m128i src_temp13_8x16b, src_temp14_8x16b, src_temp15_8x16b, src_temp16_8x16b;
            /*  outer for loop starts from here */
            for(row = 0; row < ht; row += 4)
            {
                for(col = 0; col < wdx2; col += 16)
                {
                    /*load 8 pixel values */ /* First 8 Values */
                    src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1));
                    src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2));
                    /* row = 1 */
                    src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + src_strd1));
                    src_temp4_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + src_strd2));
                    /* row = 2 */
                    src_temp5_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 2 * src_strd1));
                    src_temp6_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 2 * src_strd2));
                    /* row = 3 */
                    src_temp7_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 3 * src_strd1));
                    src_temp8_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 3 * src_strd2));

                    /*load 8 pixel values */ /* Second 8 Values */
                    src_temp9_8x16b  = _mm_loadu_si128((__m128i *)(pi2_src1 + 8));
                    src_temp10_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 8));
                    /* row = 1 */
                    src_temp11_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + src_strd1 + 8));
                    src_temp12_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + src_strd2 + 8));
                    /* row = 2 */
                    src_temp13_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 2 * src_strd1 + 8));
                    src_temp14_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 2 * src_strd2 + 8));

                    /* (pi2_src1[col] + pi2_src2[col]) */ /* First 8 Values */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, src_temp2_8x16b);
                    src_temp3_8x16b = _mm_adds_epi16(src_temp3_8x16b, src_temp4_8x16b);
                    src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, src_temp6_8x16b);
                    src_temp7_8x16b = _mm_adds_epi16(src_temp7_8x16b, src_temp8_8x16b);

                    /*load 8 pixel values */ /* Second 8 Values */
                    /* row = 3 */
                    src_temp15_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 3 * src_strd1 + 8));
                    src_temp16_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 3 * src_strd2 + 8));

                    /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */ /* First 8 Values */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, lvl_shift1_8x16b);
                    src_temp3_8x16b = _mm_adds_epi16(src_temp3_8x16b, lvl_shift1_8x16b);
                    src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, lvl_shift1_8x16b);
                    src_temp7_8x16b = _mm_adds_epi16(src_temp7_8x16b, lvl_shift1_8x16b);

                    /* (pi2_src1[col] + pi2_src2[col]) */ /* Second 8 Values */
                    src_temp9_8x16b  = _mm_adds_epi16(src_temp9_8x16b,  src_temp10_8x16b);
                    src_temp11_8x16b = _mm_adds_epi16(src_temp11_8x16b, src_temp12_8x16b);
                    src_temp13_8x16b = _mm_adds_epi16(src_temp13_8x16b, src_temp14_8x16b);
                    src_temp15_8x16b = _mm_adds_epi16(src_temp15_8x16b, src_temp16_8x16b);

                    /* (i4_tmp >> shift) */ /* First 8 Values */
                    src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  shift);
                    src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  shift);
                    src_temp5_8x16b = _mm_srai_epi16(src_temp5_8x16b,  shift);
                    src_temp7_8x16b = _mm_srai_epi16(src_temp7_8x16b,  shift);

                    /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */ /* Second 8 Values */
                    src_temp9_8x16b  = _mm_adds_epi16(src_temp9_8x16b, lvl_shift1_8x16b);
                    src_temp11_8x16b = _mm_adds_epi16(src_temp11_8x16b, lvl_shift1_8x16b);
                    src_temp13_8x16b = _mm_adds_epi16(src_temp13_8x16b, lvl_shift1_8x16b);
                    src_temp15_8x16b = _mm_adds_epi16(src_temp15_8x16b, lvl_shift1_8x16b);

                    /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */ /* First 8 Values */
                    src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp1_8x16b);
                    src_temp3_8x16b = _mm_packus_epi16(src_temp3_8x16b, src_temp3_8x16b);
                    src_temp5_8x16b = _mm_packus_epi16(src_temp5_8x16b, src_temp5_8x16b);
                    src_temp7_8x16b = _mm_packus_epi16(src_temp7_8x16b, src_temp7_8x16b);

                    /* (i4_tmp >> shift) */ /* Second 8 Values */
                    src_temp9_8x16b  = _mm_srai_epi16(src_temp9_8x16b,  shift);
                    src_temp11_8x16b = _mm_srai_epi16(src_temp11_8x16b,  shift);
                    src_temp13_8x16b = _mm_srai_epi16(src_temp13_8x16b,  shift);
                    src_temp15_8x16b = _mm_srai_epi16(src_temp15_8x16b,  shift);

                    /* store four 8-bit output values  */ /* First 8 Values */
                    _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd), src_temp1_8x16b); /* row = 0*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), src_temp3_8x16b); /* row = 2*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 2 * dst_strd), src_temp5_8x16b); /* row = 1*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 3 * dst_strd), src_temp7_8x16b); /* row = 3*/

                    /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */ /* Second 8 Values */
                    src_temp9_8x16b  = _mm_packus_epi16(src_temp9_8x16b, src_temp9_8x16b);
                    src_temp11_8x16b = _mm_packus_epi16(src_temp11_8x16b, src_temp11_8x16b);
                    src_temp13_8x16b = _mm_packus_epi16(src_temp13_8x16b, src_temp13_8x16b);
                    src_temp15_8x16b = _mm_packus_epi16(src_temp15_8x16b, src_temp15_8x16b);

                    /* store four 8-bit output values  */ /* Second 8 Values */
                    _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd + 8), src_temp9_8x16b); /* row = 0*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd + 8), src_temp11_8x16b); /* row = 2*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 2 * dst_strd + 8), src_temp13_8x16b); /* row = 1*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 3 * dst_strd + 8), src_temp15_8x16b); /* row = 3*/

                    /* To update pointer */
                    pi2_src1 += 16;
                    pi2_src2 += 16;
                    pu1_dst  += 16;

                } /* inner loop ends here(8-output values in single iteration) */

                pi2_src1 = pi2_src1 - wdx2 + 4 * src_strd1;    /* Pointer update */
                pi2_src2 = pi2_src2 - wdx2 + 4 * src_strd2;    /* Pointer update */
                pu1_dst  = pu1_dst - wdx2 + 4 * dst_strd; /* Pointer update */

            }
        }
        else if(0 == (wdx2 & 7)) /* multiple of 8 case */
        {
            /*  outer for loop starts from here */
            for(row = 0; row < ht; row += 4)
            {
                for(col = 0; col < wdx2; col += 8)
                {
                    /*load 8 pixel values */
                    src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1));
                    src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2));
                    /* row = 1 */
                    src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + src_strd1));
                    src_temp4_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + src_strd2));
                    /* row = 2 */
                    src_temp5_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 2 * src_strd1));
                    src_temp6_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 2 * src_strd2));
                    /* row = 3 */
                    src_temp7_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + 3 * src_strd1));
                    src_temp8_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 3 * src_strd2));

                    /* (pi2_src1[col] + pi2_src2[col]) */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, src_temp2_8x16b);
                    src_temp3_8x16b = _mm_adds_epi16(src_temp3_8x16b, src_temp4_8x16b);
                    src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, src_temp6_8x16b);
                    src_temp7_8x16b = _mm_adds_epi16(src_temp7_8x16b, src_temp8_8x16b);

                    /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, lvl_shift1_8x16b);
                    src_temp3_8x16b = _mm_adds_epi16(src_temp3_8x16b, lvl_shift1_8x16b);
                    src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, lvl_shift1_8x16b);
                    src_temp7_8x16b = _mm_adds_epi16(src_temp7_8x16b, lvl_shift1_8x16b);

                    /* (i4_tmp >> shift) */
                    src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  shift);
                    src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  shift);
                    src_temp5_8x16b = _mm_srai_epi16(src_temp5_8x16b,  shift);
                    src_temp7_8x16b = _mm_srai_epi16(src_temp7_8x16b,  shift);

                    /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */
                    src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp1_8x16b);
                    src_temp3_8x16b = _mm_packus_epi16(src_temp3_8x16b, src_temp3_8x16b);
                    src_temp5_8x16b = _mm_packus_epi16(src_temp5_8x16b, src_temp5_8x16b);
                    src_temp7_8x16b = _mm_packus_epi16(src_temp7_8x16b, src_temp7_8x16b);

                    /* store four 8-bit output values  */
                    _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd), src_temp1_8x16b); /* row = 0*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), src_temp3_8x16b); /* row = 2*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 2 * dst_strd), src_temp5_8x16b); /* row = 1*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 3 * dst_strd), src_temp7_8x16b); /* row = 3*/

                    /* To update pointer */
                    pi2_src1 += 8;
                    pi2_src2 += 8;
                    pu1_dst  += 8;

                } /* inner loop ends here(8-output values in single iteration) */

                pi2_src1 = pi2_src1 - wdx2 + 4 * src_strd1;    /* Pointer update */
                pi2_src2 = pi2_src2 - wdx2 + 4 * src_strd2;    /* Pointer update */
                pu1_dst  = pu1_dst - wdx2 + 4 * dst_strd; /* Pointer update */

            }
        }
        else /* 2*wd multiple of 4 case */
        {
            WORD32 dst0, dst1, dst2, dst3;
            /*  outer for loop starts from here */
            for(row = 0; row < ht; row += 4)
            {
                for(col = 0; col < wdx2; col += 4)
                {
                    /*load 4 pixel values from 7:0 pos. relative to cur. pos.*/
                    src_temp1_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1));
                    /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                    src_temp2_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2));

                    /* row = 1 */
                    src_temp3_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1 + src_strd1));
                    src_temp4_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2 + src_strd2));
                    /* row = 2 */
                    src_temp5_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1 + 2 * src_strd1));
                    src_temp6_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2 + 2 * src_strd2));
                    /* row = 3 */
                    src_temp7_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1 + 3 * src_strd1));
                    src_temp8_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2 + 3 * src_strd2));

                    /* Pack two rows together */
                    src_temp1_8x16b = _mm_unpacklo_epi64(src_temp1_8x16b, src_temp3_8x16b);
                    src_temp2_8x16b = _mm_unpacklo_epi64(src_temp2_8x16b, src_temp4_8x16b);
                    src_temp5_8x16b = _mm_unpacklo_epi64(src_temp5_8x16b, src_temp7_8x16b);
                    src_temp6_8x16b = _mm_unpacklo_epi64(src_temp6_8x16b, src_temp8_8x16b);

                    /* (pi2_src1[col] + pi2_src2[col]) */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, src_temp2_8x16b);
                    src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, src_temp6_8x16b);

                    /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, lvl_shift1_8x16b);
                    src_temp5_8x16b = _mm_adds_epi16(src_temp5_8x16b, lvl_shift1_8x16b);

                    /* (i4_tmp >> shift) */
                    src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  shift);
                    src_temp5_8x16b = _mm_srai_epi16(src_temp5_8x16b,  shift);

                    /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */
                    src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp1_8x16b);
                    src_temp5_8x16b = _mm_packus_epi16(src_temp5_8x16b, src_temp5_8x16b);

                    dst0 = _mm_cvtsi128_si32(src_temp1_8x16b);
                    /* dst row = 1 to 3 */
                    src_temp2_8x16b = _mm_shuffle_epi32(src_temp1_8x16b, 1);
                    src_temp4_8x16b = _mm_shuffle_epi32(src_temp5_8x16b, 1);

                    /* store four 8-bit output values  */
                    *(WORD32 *)(&pu1_dst[0 * dst_strd]) = dst0;

                    dst1 = _mm_cvtsi128_si32(src_temp2_8x16b);
                    dst2 = _mm_cvtsi128_si32(src_temp5_8x16b);
                    dst3 = _mm_cvtsi128_si32(src_temp4_8x16b);

                    /* row = 1 to row = 3 */
                    *(WORD32 *)(&pu1_dst[1 * dst_strd]) = dst1;
                    *(WORD32 *)(&pu1_dst[2 * dst_strd]) = dst2;
                    *(WORD32 *)(&pu1_dst[3 * dst_strd]) = dst3;

                    /* To update pointer */
                    pi2_src1 += 4;
                    pi2_src2 += 4;
                    pu1_dst  += 4;

                } /* inner loop ends here(4-output values in single iteration) */

                pi2_src1 = pi2_src1 - wdx2 + 4 * src_strd1;   /* Pointer update */
                pi2_src2 = pi2_src2 - wdx2 + 4 * src_strd2;   /* Pointer update */
                pu1_dst  = pu1_dst  - wdx2 + 4 * dst_strd;    /* Pointer update */

            }
        }
    }
    else /* ht multiple of 2 case */
    {
        if(0 == (wdx2 & 15)) /* 2*wd multiple of 16 case */
        {
            __m128i src_temp9_8x16b,  src_temp10_8x16b, src_temp11_8x16b, src_temp12_8x16b;
            /*  outer for loop starts from here */
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wdx2; col += 16)
                {
                    /*load 8 pixel values */ /* First 8 Values */
                    src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1));
                    src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2));
                    /* row = 1 */
                    src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + src_strd1));
                    src_temp4_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + src_strd2));

                    /*load 8 pixel values */ /* Second 8 Values */
                    src_temp9_8x16b  = _mm_loadu_si128((__m128i *)(pi2_src1 + 8));
                    src_temp10_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + 8));
                    /* row = 1 */
                    src_temp11_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + src_strd1 + 8));
                    src_temp12_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + src_strd2 + 8));

                    /* (pi2_src1[col] + pi2_src2[col]) */ /* First 8 Values */
                    src_temp1_8x16b  = _mm_adds_epi16(src_temp1_8x16b,  src_temp2_8x16b);
                    src_temp3_8x16b  = _mm_adds_epi16(src_temp3_8x16b,  src_temp4_8x16b);

                    /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */ /* First 8 Values */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, lvl_shift1_8x16b);
                    src_temp3_8x16b = _mm_adds_epi16(src_temp3_8x16b, lvl_shift1_8x16b);

                    /* (pi2_src1[col] + pi2_src2[col]) */ /* Second 8 Values */
                    src_temp9_8x16b  = _mm_adds_epi16(src_temp9_8x16b,  src_temp10_8x16b);
                    src_temp11_8x16b = _mm_adds_epi16(src_temp11_8x16b, src_temp12_8x16b);

                    /* (i4_tmp >> shift) */ /* First 8 Values */
                    src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  shift);
                    src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  shift);

                    /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */ /* Second 8 Values */
                    src_temp9_8x16b  = _mm_adds_epi16(src_temp9_8x16b, lvl_shift1_8x16b);
                    src_temp11_8x16b = _mm_adds_epi16(src_temp11_8x16b, lvl_shift1_8x16b);

                    /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */ /* First 8 Values */
                    src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp1_8x16b);
                    src_temp3_8x16b = _mm_packus_epi16(src_temp3_8x16b, src_temp3_8x16b);

                    /* (i4_tmp >> shift) */ /* Second 8 Values */
                    src_temp9_8x16b  = _mm_srai_epi16(src_temp9_8x16b,  shift);
                    src_temp11_8x16b = _mm_srai_epi16(src_temp11_8x16b,  shift);

                    /* store four 8-bit output values  */ /* First 8 Values */
                    _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd), src_temp1_8x16b); /* row = 0*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), src_temp3_8x16b); /* row = 2*/

                    /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */ /* Second 8 Values */
                    src_temp9_8x16b  = _mm_packus_epi16(src_temp9_8x16b, src_temp9_8x16b);
                    src_temp11_8x16b = _mm_packus_epi16(src_temp11_8x16b, src_temp11_8x16b);

                    /* store four 8-bit output values  */ /* Second 8 Values */
                    _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd + 8), src_temp9_8x16b); /* row = 0*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd + 8), src_temp11_8x16b); /* row = 2*/

                    /* To update pointer */
                    pi2_src1 += 16;
                    pi2_src2 += 16;
                    pu1_dst  += 16;

                } /* inner loop ends here(8-output values in single iteration) */

                pi2_src1 = pi2_src1 - wdx2 + 2 * src_strd1;    /* Pointer update */
                pi2_src2 = pi2_src2 - wdx2 + 2 * src_strd2;    /* Pointer update */
                pu1_dst  = pu1_dst - wdx2 + 2 * dst_strd; /* Pointer update */

            }
        }
        else if(0 == (wdx2 & 7)) /* multiple of 8 case */
        {
            /*  outer for loop starts from here */
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wdx2; col += 8)
                {
                    /*load 8 pixel values */
                    src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1));
                    src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2));
                    /* row = 1 */
                    src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pi2_src1 + src_strd1));
                    src_temp4_8x16b = _mm_loadu_si128((__m128i *)(pi2_src2 + src_strd2));

                    /* (pi2_src1[col] + pi2_src2[col]) */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, src_temp2_8x16b);
                    src_temp3_8x16b = _mm_adds_epi16(src_temp3_8x16b, src_temp4_8x16b);

                    /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, lvl_shift1_8x16b);
                    src_temp3_8x16b = _mm_adds_epi16(src_temp3_8x16b, lvl_shift1_8x16b);

                    /* (i4_tmp >> shift) */
                    src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  shift);
                    src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  shift);

                    /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */
                    src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp1_8x16b);
                    src_temp3_8x16b = _mm_packus_epi16(src_temp3_8x16b, src_temp3_8x16b);

                    /* store four 8-bit output values  */
                    _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd), src_temp1_8x16b); /* row = 0*/
                    _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), src_temp3_8x16b); /* row = 1*/

                    /* To update pointer */
                    pi2_src1 += 8;
                    pi2_src2 += 8;
                    pu1_dst  += 8;

                } /* inner loop ends here(8-output values in single iteration) */

                pi2_src1 = pi2_src1 - wdx2 + 2 * src_strd1;    /* Pointer update */
                pi2_src2 = pi2_src2 - wdx2 + 2 * src_strd2;    /* Pointer update */
                pu1_dst  = pu1_dst - wdx2 + 2 * dst_strd; /* Pointer update */

            }
        }
        else /* 2*wd multiple of 4 case */
        {
            WORD32 dst0, dst1;
            /*  outer for loop starts from here */
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wdx2; col += 4)
                {
                    /*load 4 pixel values from 7:0 pos. relative to cur. pos.*/
                    src_temp1_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1));
                    /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                    src_temp2_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2));
                    /* row = 1 */
                    src_temp3_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src1 + src_strd1));
                    src_temp4_8x16b = _mm_loadl_epi64((__m128i *)(pi2_src2 + src_strd2));

                    /* Pack two rows together */
                    src_temp1_8x16b = _mm_unpacklo_epi64(src_temp1_8x16b, src_temp3_8x16b);
                    src_temp2_8x16b = _mm_unpacklo_epi64(src_temp2_8x16b, src_temp4_8x16b);

                    /* (pi2_src1[col] + pi2_src2[col]) */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, src_temp2_8x16b);
                    /* i4_tmp = (pi2_src1[col] + pi2_src2[col] + lvl_shift1 + lvl_shift2 + shift_value) */
                    src_temp1_8x16b = _mm_adds_epi16(src_temp1_8x16b, lvl_shift1_8x16b);

                    /* (i4_tmp >> shift) */
                    src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  shift);
                    /* pu1_dst[col] = CLIP_U8(i4_tmp >> shift); */
                    src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp1_8x16b);

                    dst0 = _mm_cvtsi128_si32(src_temp1_8x16b);
                    /* dst row = 1 */
                    src_temp2_8x16b = _mm_shuffle_epi32(src_temp1_8x16b, 1);

                    /* store four 8-bit output values  */
                    *(WORD32 *)(&pu1_dst[0 * dst_strd]) = dst0;

                    dst1 = _mm_cvtsi128_si32(src_temp2_8x16b);
                    /* row = 1 */
                    *(WORD32 *)(&pu1_dst[1 * dst_strd]) = dst1;

                    /* To update pointer */
                    pi2_src1 += 4;
                    pi2_src2 += 4;
                    pu1_dst  += 4;
                } /* inner loop ends here(4-output values in single iteration) */

                pi2_src1 = pi2_src1 - wdx2 + 2 * src_strd1;   /* Pointer update */
                pi2_src2 = pi2_src2 - wdx2 + 2 * src_strd2;   /* Pointer update */
                pu1_dst  = pu1_dst  - wdx2 + 2 * dst_strd;    /* Pointer update */

            }
        }
    }
}
