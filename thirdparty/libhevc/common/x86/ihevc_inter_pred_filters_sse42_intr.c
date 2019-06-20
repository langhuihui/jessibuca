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
*  ihevc_inter_pred_filters_x86_intr.c
*
* @brief
*  Contains function definitions for inter prediction  interpolation filters
*  coded in x86 intrinsics
*
*
* @author
*
*
* @par List of Functions:
*  - ihevc_inter_pred_luma_copy_w16out_sse42()
*  - ihevc_inter_pred_chroma_copy_sse42()
*  - ihevc_inter_pred_chroma_copy_w16out_sse42()
*
* @remarks
*  None
*
*******************************************************************************
*/


/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/
#include <assert.h>

#include "ihevc_debug.h"
#include "ihevc_typedefs.h"
#include "ihevc_defs.h"
#include "ihevc_inter_pred.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_func_selector.h"

#include <immintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <tmmintrin.h>

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/



/**
*******************************************************************************
*
* @brief
*       Interprediction luma filter for copy 16bit output
*
* @par Description:
*    Copies the array of width 'wd' and height 'ht' from the  location pointed
*    by 'src' to the location pointed by 'dst' The output is upshifted by 6
*    bits and is used as input for vertical filtering or weighted prediction
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pi2_dst
*  WORD16 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_inter_pred_luma_copy_w16out_sse42(UWORD8 *pu1_src,
                                             WORD16 *pi2_dst,
                                             WORD32 src_strd,
                                             WORD32 dst_strd,
                                             WORD8 *pi1_coeff,
                                             WORD32 ht,
                                             WORD32 wd)
{
    WORD32 row, col;
    __m128i  src0_16x8b, src1_16x8b, src2_16x8b, src3_16x8b;
    UNUSED(pi1_coeff);
    ASSERT(wd % 4 == 0); /* checking assumption*/
    ASSERT(ht % 4 == 0); /* checking assumption*/

    if(0 == (wd & 7)) /* multiple of 8 case */
    {
        for(row = 0; row < ht; row += 4)
        {
            for(col = 0; col < wd; col += 8)
            {
                /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                src0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src));                /* row =0 */
                src1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */
                src2_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 2 * src_strd)); /* row =2 */
                src3_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 3 * src_strd)); /* row =3 */

                src0_16x8b = _mm_cvtepu8_epi16(src0_16x8b);
                src1_16x8b = _mm_cvtepu8_epi16(src1_16x8b);
                src2_16x8b = _mm_cvtepu8_epi16(src2_16x8b);
                src3_16x8b = _mm_cvtepu8_epi16(src3_16x8b);

                src0_16x8b = _mm_slli_epi16(src0_16x8b,  SHIFT_14_MINUS_BIT_DEPTH); /* (pu1_src[col] << SHIFT_14_MINUS_BIT_DEPTH */
                src1_16x8b = _mm_slli_epi16(src1_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);
                src2_16x8b = _mm_slli_epi16(src2_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);
                src3_16x8b = _mm_slli_epi16(src3_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);

                /* storing 16 8-bit output values */
                _mm_storeu_si128((__m128i *)(pi2_dst), src0_16x8b);                 /* row =0 */
                _mm_storeu_si128((__m128i *)(pi2_dst + 1 * dst_strd), src1_16x8b);  /* row =1 */
                _mm_storeu_si128((__m128i *)(pi2_dst + 2 * dst_strd), src2_16x8b);  /* row =2 */
                _mm_storeu_si128((__m128i *)(pi2_dst + 3 * dst_strd), src3_16x8b);  /* row =3 */

                pu1_src += 8; /* pointer update */
                pi2_dst += 8; /* pointer update */
            } /* inner for loop ends here(8-output values in single iteration) */

            pu1_src += 4 * src_strd - wd; /* pointer update */
            pi2_dst += 4 * dst_strd - wd; /* pointer update */
        }
    }
    else /* wd = multiple of 4 case */
    {
        for(row = 0; row < ht; row += 4)
        {
            for(col = 0; col < wd; col += 4)
            {
                /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                src0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src));                /* row =0 */
                src1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */
                src2_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 2 * src_strd)); /* row =2 */
                src3_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 3 * src_strd)); /* row =3 */

                src0_16x8b = _mm_cvtepu8_epi16(src0_16x8b);
                src1_16x8b = _mm_cvtepu8_epi16(src1_16x8b);
                src2_16x8b = _mm_cvtepu8_epi16(src2_16x8b);
                src3_16x8b = _mm_cvtepu8_epi16(src3_16x8b);

                src0_16x8b = _mm_slli_epi16(src0_16x8b,  SHIFT_14_MINUS_BIT_DEPTH); /* (pu1_src[col] << SHIFT_14_MINUS_BIT_DEPTH */
                src1_16x8b = _mm_slli_epi16(src1_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);
                src2_16x8b = _mm_slli_epi16(src2_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);
                src3_16x8b = _mm_slli_epi16(src3_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);

                /* storing 16 8-bit output values */
                _mm_storel_epi64((__m128i *)(pi2_dst), src0_16x8b);                 /* row =0 */
                _mm_storel_epi64((__m128i *)(pi2_dst + 1 * dst_strd), src1_16x8b);  /* row =1 */
                _mm_storel_epi64((__m128i *)(pi2_dst + 2 * dst_strd), src2_16x8b);  /* row =2 */
                _mm_storel_epi64((__m128i *)(pi2_dst + 3 * dst_strd), src3_16x8b);  /* row =3 */

                pu1_src += 4; /* pointer update */
                pi2_dst += 4; /* pointer update */
            } /* inner for loop ends here(4-output values in single iteration) */

            pu1_src += 4 * src_strd - wd; /* pointer update */
            pi2_dst += 4 * dst_strd - wd; /* pointer update */
        }
    }
}

/**
*******************************************************************************
*
* @brief
*      Chroma interprediction filter for copy
*
* @par Description:
*    Copies the array of width 'wd' and height 'ht' from the  location pointed
*    by 'src' to the location pointed by 'dst'
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pu1_dst
*  UWORD8 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_inter_pred_chroma_copy_sse42(UWORD8 *pu1_src,
                                        UWORD8 *pu1_dst,
                                        WORD32 src_strd,
                                        WORD32 dst_strd,
                                        WORD8 *pi1_coeff,
                                        WORD32 ht,
                                        WORD32 wd)
{
    WORD32 row, col, wdx2;
    __m128i  src0_16x8b, src1_16x8b, src2_16x8b, src3_16x8b;

    ASSERT(wd % 2 == 0); /* checking assumption*/
    ASSERT(ht % 2 == 0); /* checking assumption*/
    UNUSED(pi1_coeff);
    wdx2 = wd * 2;

    if(0 == (ht & 3)) /* ht multiple of 4 */
    {
        if(0 == (wdx2 & 15)) /* wdx2 multiple of 16 case */
        {
            for(row = 0; row < ht; row += 4)
            {
                for(col = 0; col < wdx2; col += 16)
                {
                    /*load 16 pixel values from 15:0 pos. relative to cur. pos.*/
                    src0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src));                /* row =0 */
                    src1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */
                    src2_16x8b = _mm_loadu_si128((__m128i *)(pu1_src + 2 * src_strd)); /* row =2 */
                    src3_16x8b = _mm_loadu_si128((__m128i *)(pu1_src + 3 * src_strd)); /* row =3 */

                    /* storing 16 8-bit output values */
                    _mm_storeu_si128((__m128i *)(pu1_dst), src0_16x8b);                 /* row =0 */
                    _mm_storeu_si128((__m128i *)(pu1_dst + 1 * dst_strd), src1_16x8b);  /* row =1 */
                    _mm_storeu_si128((__m128i *)(pu1_dst + 2 * dst_strd), src2_16x8b);  /* row =2 */
                    _mm_storeu_si128((__m128i *)(pu1_dst + 3 * dst_strd), src3_16x8b);  /* row =3 */

                    pu1_src += 16; /* pointer update */
                    pu1_dst += 16; /* pointer update */
                } /* inner for loop ends here(16-output values in single iteration) */

                pu1_src += 4 * src_strd - wdx2; /* pointer update */
                pu1_dst += 4 * dst_strd - wdx2; /* pointer update */
            }

        }
        else if(0 == (wdx2 & 7)) /* multiple of 8 case */
        {
            for(row = 0; row < ht; row += 4)
            {
                for(col = 0; col < wdx2; col += 8)
                {
                    /*load 16 pixel values from 15:0 pos. relative to cur. pos.*/
                    src0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src));                /* row =0 */
                    src1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */
                    src2_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 2 * src_strd)); /* row =2 */
                    src3_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 3 * src_strd)); /* row =3 */

                    /* storing 16 8-bit output values */
                    _mm_storel_epi64((__m128i *)(pu1_dst), src0_16x8b);                 /* row =0 */
                    _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), src1_16x8b);  /* row =1 */
                    _mm_storel_epi64((__m128i *)(pu1_dst + 2 * dst_strd), src2_16x8b);  /* row =2 */
                    _mm_storel_epi64((__m128i *)(pu1_dst + 3 * dst_strd), src3_16x8b);  /* row =3 */

                    pu1_src += 8; /* pointer update */
                    pu1_dst += 8; /* pointer update */
                } /*  inner for loop ends here(8-output values in single iteration) */

                pu1_src += 4 * src_strd - wdx2; /* pointer update */
                pu1_dst += 4 * dst_strd - wdx2; /* pointer update */
            }
        }
        else /* wdx2 = multiple of 4 case */
        {
            WORD32 dst0, dst1, dst2, dst3;
            for(row = 0; row < ht; row += 4)
            {
                for(col = 0; col < wdx2; col += 4)
                {
                    /*load 16 pixel values from 15:0 pos. relative to cur. pos.*/
                    src0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src));                /* row =0 */
                    src1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */
                    src2_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 2 * src_strd)); /* row =2 */
                    src3_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 3 * src_strd)); /* row =3 */

                    dst0 = _mm_cvtsi128_si32(src0_16x8b);
                    dst1 = _mm_cvtsi128_si32(src1_16x8b);
                    dst2 = _mm_cvtsi128_si32(src2_16x8b);
                    dst3 = _mm_cvtsi128_si32(src3_16x8b);

                    /* storing 4 8-bit output values */
                    *(WORD32 *)(&pu1_dst[0 * dst_strd]) = dst0; /* row =0 */
                    *(WORD32 *)(&pu1_dst[1 * dst_strd]) = dst1; /* row =1 */
                    *(WORD32 *)(&pu1_dst[2 * dst_strd]) = dst2; /* row =2 */
                    *(WORD32 *)(&pu1_dst[3 * dst_strd]) = dst3; /* row =3 */

                    pu1_src += 4; /* pointer update */
                    pu1_dst += 4; /* pointer update */
                } /*  inner for loop ends here(4- output values in single iteration) */

                pu1_src += 4 * src_strd - wdx2; /* pointer update */
                pu1_dst += 4 * dst_strd - wdx2; /* pointer update */
            }
        }
    }
    else /* ht multiple of 2 */
    {
        if(0 == (wdx2 & 15)) /* wdx2 multiple of 16 case */
        {
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wdx2; col += 16)
                {
                    /*load 16 pixel values from 15:0 pos. relative to cur. pos.*/
                    src0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src));                /* row =0 */
                    src1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */

                    /* storing 16 8-bit output values */
                    _mm_storeu_si128((__m128i *)(pu1_dst), src0_16x8b);                 /* row =0 */
                    _mm_storeu_si128((__m128i *)(pu1_dst + 1 * dst_strd), src1_16x8b);  /* row =1 */

                    pu1_src += 16; /* pointer update */
                    pu1_dst += 16; /* pointer update */
                } /* inner for loop ends here(16-output values in single iteration) */

                pu1_src += 2 * src_strd - wdx2; /* pointer update */
                pu1_dst += 2 * dst_strd - wdx2; /* pointer update */
            }

        }
        else if(0 == (wdx2 & 7)) /* multiple of 8 case */
        {
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wdx2; col += 8)
                {
                    /*load 16 pixel values from 15:0 pos. relative to cur. pos.*/
                    src0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src));                /* row =0 */
                    src1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */

                    /* storing 16 8-bit output values */
                    _mm_storel_epi64((__m128i *)(pu1_dst), src0_16x8b);                 /* row =0 */
                    _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), src1_16x8b);  /* row =1 */

                    pu1_src += 8; /* pointer update */
                    pu1_dst += 8; /* pointer update */
                } /*  inner for loop ends here(8-output values in single iteration) */

                pu1_src += 2 * src_strd - wdx2; /* pointer update */
                pu1_dst += 2 * dst_strd - wdx2; /* pointer update */
            }
        }
        else /* wdx2 = multiple of 4 case */
        {
            WORD32 dst0, dst1;
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wdx2; col += 4)
                {
                    /*load 16 pixel values from 15:0 pos. relative to cur. pos.*/
                    src0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src));                /* row =0 */
                    src1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */

                    dst0 = _mm_cvtsi128_si32(src0_16x8b);
                    dst1 = _mm_cvtsi128_si32(src1_16x8b);


                    /* storing 4 8-bit output values */
                    *(WORD32 *)(&pu1_dst[0 * dst_strd]) = dst0; /* row =0 */
                    *(WORD32 *)(&pu1_dst[1 * dst_strd]) = dst1; /* row =1 */

                    pu1_src += 4; /* pointer update */
                    pu1_dst += 4; /* pointer update */
                } /*  inner for loop ends here(4- output values in single iteration) */

                pu1_src += 2 * src_strd - wdx2; /* pointer update */
                pu1_dst += 2 * dst_strd - wdx2; /* pointer update */
            }
        }
    }
}

/**
*******************************************************************************
*
* @brief
*       chroma interprediction filter for copying 16bit output
*
* @par Description:
*    Copies the array of width 'wd' and height 'ht' from the  location pointed
*    by 'src' to the location pointed by 'dst' The output is upshifted by 6
*    bits and is used as input for vertical filtering or weighted prediction
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pi2_dst
*  WORD16 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_inter_pred_chroma_copy_w16out_sse42(UWORD8 *pu1_src,
                                               WORD16 *pi2_dst,
                                               WORD32 src_strd,
                                               WORD32 dst_strd,
                                               WORD8 *pi1_coeff,
                                               WORD32 ht,
                                               WORD32 wd)
{
    WORD32 row, col, wdx2;
    __m128i  src0_16x8b, src1_16x8b, src2_16x8b, src3_16x8b;

    ASSERT(wd % 2 == 0); /* checking assumption*/
    ASSERT(ht % 2 == 0); /* checking assumption*/
    UNUSED(pi1_coeff);
    wdx2 = wd * 2;

    if(0 == (ht & 3)) /* multiple of 4 case */
    {
        if(0 == (wdx2 & 7)) /* multiple of 8 case */
        {
            for(row = 0; row < ht; row += 4)
            {
                for(col = 0; col < wdx2; col += 8)
                {
                    /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                    src0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src));                /* row =0 */
                    src1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */
                    src2_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 2 * src_strd)); /* row =2 */
                    src3_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 3 * src_strd)); /* row =3 */

                    src0_16x8b = _mm_cvtepu8_epi16(src0_16x8b);
                    src1_16x8b = _mm_cvtepu8_epi16(src1_16x8b);
                    src2_16x8b = _mm_cvtepu8_epi16(src2_16x8b);
                    src3_16x8b = _mm_cvtepu8_epi16(src3_16x8b);

                    src0_16x8b = _mm_slli_epi16(src0_16x8b,  SHIFT_14_MINUS_BIT_DEPTH); /* (pu1_src[col] << SHIFT_14_MINUS_BIT_DEPTH */
                    src1_16x8b = _mm_slli_epi16(src1_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);
                    src2_16x8b = _mm_slli_epi16(src2_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);
                    src3_16x8b = _mm_slli_epi16(src3_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);

                    /* storing 16 8-bit output values */
                    _mm_storeu_si128((__m128i *)(pi2_dst), src0_16x8b);                 /* row =0 */
                    _mm_storeu_si128((__m128i *)(pi2_dst + 1 * dst_strd), src1_16x8b);  /* row =1 */
                    _mm_storeu_si128((__m128i *)(pi2_dst + 2 * dst_strd), src2_16x8b);  /* row =2 */
                    _mm_storeu_si128((__m128i *)(pi2_dst + 3 * dst_strd), src3_16x8b);  /* row =3 */

                    pu1_src += 8; /* pointer update */
                    pi2_dst += 8; /* pointer update */
                } /* inner for loop ends here(8-output values in single iteration) */

                pu1_src += 4 * src_strd - wdx2; /* pointer update */
                pi2_dst += 4 * dst_strd - wdx2; /* pointer update */
            }
        }
        else /* wdx2 = multiple of 4 case */
        {
            for(row = 0; row < ht; row += 4)
            {
                for(col = 0; col < wdx2; col += 4)
                {
                    /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                    src0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src));                /* row =0 */
                    src1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */
                    src2_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 2 * src_strd)); /* row =2 */
                    src3_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 3 * src_strd)); /* row =3 */

                    src0_16x8b = _mm_cvtepu8_epi16(src0_16x8b);
                    src1_16x8b = _mm_cvtepu8_epi16(src1_16x8b);
                    src2_16x8b = _mm_cvtepu8_epi16(src2_16x8b);
                    src3_16x8b = _mm_cvtepu8_epi16(src3_16x8b);

                    src0_16x8b = _mm_slli_epi16(src0_16x8b,  SHIFT_14_MINUS_BIT_DEPTH); /* (pu1_src[col] << SHIFT_14_MINUS_BIT_DEPTH */
                    src1_16x8b = _mm_slli_epi16(src1_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);
                    src2_16x8b = _mm_slli_epi16(src2_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);
                    src3_16x8b = _mm_slli_epi16(src3_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);

                    /* storing 16 8-bit output values */
                    _mm_storel_epi64((__m128i *)(pi2_dst), src0_16x8b);                 /* row =0 */
                    _mm_storel_epi64((__m128i *)(pi2_dst + 1 * dst_strd), src1_16x8b);  /* row =1 */
                    _mm_storel_epi64((__m128i *)(pi2_dst + 2 * dst_strd), src2_16x8b);  /* row =2 */
                    _mm_storel_epi64((__m128i *)(pi2_dst + 3 * dst_strd), src3_16x8b);  /* row =3 */

                    pu1_src += 4; /* pointer update */
                    pi2_dst += 4; /* pointer update */
                } /* inner for loop ends here(4-output values in single iteration) */

                pu1_src += 4 * src_strd - wdx2; /* pointer update */
                pi2_dst += 4 * dst_strd - wdx2; /* pointer update */
            }
        }
    }
    else  /* ht multiple of 2 case */
    {
        if(0 == (wdx2 & 7)) /* multiple of 8 case */
        {
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wdx2; col += 8)
                {
                    /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                    src0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src));                /* row =0 */
                    src1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */

                    src0_16x8b = _mm_cvtepu8_epi16(src0_16x8b);
                    src1_16x8b = _mm_cvtepu8_epi16(src1_16x8b);

                    src0_16x8b = _mm_slli_epi16(src0_16x8b,  SHIFT_14_MINUS_BIT_DEPTH); /* (pu1_src[col] << SHIFT_14_MINUS_BIT_DEPTH */
                    src1_16x8b = _mm_slli_epi16(src1_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);

                    /* storing 16 8-bit output values */
                    _mm_storeu_si128((__m128i *)(pi2_dst), src0_16x8b);                 /* row =0 */
                    _mm_storeu_si128((__m128i *)(pi2_dst + 1 * dst_strd), src1_16x8b);  /* row =1 */

                    pu1_src += 8; /* pointer update */
                    pi2_dst += 8; /* pointer update */
                } /* inner for loop ends here(8-output values in single iteration) */

                pu1_src += 2 * src_strd - wdx2; /* pointer update */
                pi2_dst += 2 * dst_strd - wdx2; /* pointer update */
            }
        }
        else /* wdx2 = multiple of 4 case */
        {
            for(row = 0; row < ht; row += 2)
            {
                for(col = 0; col < wdx2; col += 4)
                {
                    /*load 8 pixel values from 7:0 pos. relative to cur. pos.*/
                    src0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src));                /* row =0 */
                    src1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 1 * src_strd)); /* row =1 */

                    src0_16x8b = _mm_cvtepu8_epi16(src0_16x8b);
                    src1_16x8b = _mm_cvtepu8_epi16(src1_16x8b);

                    src0_16x8b = _mm_slli_epi16(src0_16x8b,  SHIFT_14_MINUS_BIT_DEPTH); /* (pu1_src[col] << SHIFT_14_MINUS_BIT_DEPTH */
                    src1_16x8b = _mm_slli_epi16(src1_16x8b,  SHIFT_14_MINUS_BIT_DEPTH);

                    /* storing 16 8-bit output values */
                    _mm_storel_epi64((__m128i *)(pi2_dst), src0_16x8b);                 /* row =0 */
                    _mm_storel_epi64((__m128i *)(pi2_dst + 1 * dst_strd), src1_16x8b);  /* row =1 */

                    pu1_src += 4; /* pointer update */
                    pi2_dst += 4; /* pointer update */
                } /* inner for loop ends here(4-output values in single iteration) */

                pu1_src += 2 * src_strd - wdx2; /* pointer update */
                pi2_dst += 2 * dst_strd - wdx2; /* pointer update */
            }
        }
    }
}
