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
*  ihevc_deblck_atom_intr.c
*
* @brief
*  Contains function definitions for deblocking filters
*
* @author
*  Rishab
*
* @par List of Functions:
*   - ihevc_deblk_luma_vert_ssse3()
*   - ihevc_deblk_luma_horz_ssse3()
*   - ihevc_deblk_chroma_vert_ssse3()
*   - ihevc_deblk_chroma_horz_ssse3()
*
* @remarks
*  None
*
*******************************************************************************
*/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "ihevc_typedefs.h"
#include "ihevc_platform_macros.h"
#include "ihevc_macros.h"
#include "ihevc_deblk.h"
#include "ihevc_deblk_tables.h"
#include "ihevc_debug.h"

#include "ihevc_tables_x86_intr.h"

#include <immintrin.h>
/**
*******************************************************************************
*
* @brief
*       Decision process and filtering for the luma block vertical edge.
*
* @par Description:
*     The decision process for the luma block vertical edge is  carried out and
*     an appropriate filter is applied. The  boundary filter strength, bs should
*     be greater than 0.  The pcm flags and the transquant bypass flags should
*     be  taken care of by the calling function.
*
* @param[in] pu1_src
*  Pointer to the src sample q(0,0)
*
* @param[in] src_strd
*  Source stride
*
* @param[in] bs
*  Boundary filter strength of q(0,0)
*
* @param[in] quant_param_p
*  quantization parameter of p block
*
* @param[in] quant_param_q
*  quantization parameter of p block
*
* @param[in] beta_offset_div2
*
*
* @param[in] tc_offset_div2
*
*
* @param[in] filter_flag_p
*  flag whether to filter the p block
*
* @param[in] filter_flag_q
*  flag whether to filter the q block
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_deblk_luma_vert_ssse3(UWORD8 *pu1_src,
                                 WORD32 src_strd,
                                 WORD32 bs,
                                 WORD32 quant_param_p,
                                 WORD32 quant_param_q,
                                 WORD32 beta_offset_div2,
                                 WORD32 tc_offset_div2,
                                 WORD32 filter_flag_p,
                                 WORD32 filter_flag_q)
{
    WORD32 qp_luma, beta_indx, tc_indx;
    WORD32 beta, tc;
    WORD32 d, dp, dq, d_sam0, d_sam3;

    WORD32 d3, d0, de_0, de_1, de_2, de_3;
    WORD32 de, dep, deq;
    __m128i src_row0_8x16b, src_row1_8x16b, src_row2_8x16b, src_row3_8x16b;


    {
        __m128i src_tmp_8x16b, coef_8x16b, mask_d_result_4x32b, mask_de_result_8x16b;
        __m128i mask_16x8b, temp_coef0_8x16b, temp_coef1_8x16b;



        ASSERT((bs > 0) && (bs <= 3));
        ASSERT(filter_flag_p || filter_flag_q);

        qp_luma = (quant_param_p + quant_param_q + 1) >> 1;
        beta_indx = CLIP3(qp_luma + (beta_offset_div2 << 1), 0, 51);

        /* BS based on implementation can take value 3 if it is intra/inter egde          */
        /* based on BS, tc index is calcuated by adding 2 * ( bs - 1) to QP and tc_offset */
        /* for BS = 1 adding factor is (0*2), BS = 2 or 3 adding factor is (1*2)          */
        /* the above desired functionallity is achieved by doing (2*(bs>>1))              */

        tc_indx = CLIP3(qp_luma + (2 * (bs >> 1)) + (tc_offset_div2 << 1), 0, 53);

        beta = gai4_ihevc_beta_table[beta_indx];
        tc = gai4_ihevc_tc_table[tc_indx];
        if(0 == tc)
        {
            return;
        }
        src_row0_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src - 4));
        src_row3_8x16b = _mm_loadl_epi64((__m128i *)((pu1_src - 4) + 3 * src_strd));

        coef_8x16b = _mm_load_si128((__m128i *)(coef_d));
        mask_16x8b =  _mm_load_si128((__m128i *)(shuffle_d));

        src_tmp_8x16b = _mm_unpacklo_epi64(src_row0_8x16b, src_row3_8x16b);
        mask_de_result_8x16b = _mm_shuffle_epi8(src_tmp_8x16b, mask_16x8b);

        mask_d_result_4x32b = _mm_maddubs_epi16(src_tmp_8x16b, coef_8x16b);


        //to get all 1's of 8 bit in (1)
        temp_coef0_8x16b = _mm_cmpeq_epi16(src_tmp_8x16b, src_tmp_8x16b);
        temp_coef1_8x16b = _mm_srli_epi16(temp_coef0_8x16b, 15);
        //accumulating values foe dp3 dq3 , dp0 dq0 values
        mask_d_result_4x32b = _mm_madd_epi16(mask_d_result_4x32b, temp_coef1_8x16b);

        temp_coef1_8x16b = _mm_packus_epi16(temp_coef1_8x16b, temp_coef1_8x16b);
        // to get all 1,-1 sets of 16 bits in (0)
        temp_coef0_8x16b = _mm_unpacklo_epi8(temp_coef0_8x16b, temp_coef1_8x16b);
        //q33-q30,p33-p30,q03-q00,p03-p00,0,q30-p30,0,q00-p00
        mask_de_result_8x16b = _mm_maddubs_epi16(mask_de_result_8x16b, temp_coef0_8x16b);
        //to get 16 bit 1's
        temp_coef0_8x16b = _mm_srli_epi16(temp_coef1_8x16b, 8);


        // dq3 dp3 dq0 dp0
        mask_d_result_4x32b = _mm_abs_epi32(mask_d_result_4x32b);
        mask_16x8b = _mm_shuffle_epi32(mask_d_result_4x32b, 0xec);
        mask_d_result_4x32b = _mm_shuffle_epi32(mask_d_result_4x32b, 0x49);
        // dq dp d3 d0
        mask_d_result_4x32b = _mm_add_epi32(mask_d_result_4x32b, mask_16x8b);
        //|q33-q30|,|p33-p30|,|q03-q00|,|p03-p00|,0,|q30-p30|,0,|q00-p00|
        mask_de_result_8x16b = _mm_abs_epi16(mask_de_result_8x16b);
        //|q33-q30|+|p33-p30|,|q03-q00|+|p03-p00|,0+|q30-p30|,0+|q00-p00|
        mask_de_result_8x16b = _mm_madd_epi16(mask_de_result_8x16b, temp_coef0_8x16b);

        ///store back in a single variable
        temp_coef0_8x16b = _mm_srli_si128(mask_d_result_4x32b, 4);
        temp_coef1_8x16b = _mm_srli_si128(mask_d_result_4x32b, 8);
        mask_16x8b = _mm_srli_si128(mask_d_result_4x32b, 12);

        d0 = _mm_cvtsi128_si32(mask_d_result_4x32b);
        d3 = _mm_cvtsi128_si32(temp_coef0_8x16b);
        dp = _mm_cvtsi128_si32(temp_coef1_8x16b);
        dq = _mm_cvtsi128_si32(mask_16x8b);
        //getting d
        d = d0 + d3;

        ///store back in a single variable
        temp_coef0_8x16b = _mm_srli_si128(mask_de_result_8x16b, 4);
        temp_coef1_8x16b = _mm_srli_si128(mask_de_result_8x16b, 8);
        mask_16x8b = _mm_srli_si128(mask_de_result_8x16b, 12);

        de_0 = _mm_cvtsi128_si32(mask_de_result_8x16b);
        de_1 = _mm_cvtsi128_si32(temp_coef0_8x16b);
        de_2 = _mm_cvtsi128_si32(temp_coef1_8x16b);
        de_3 = _mm_cvtsi128_si32(mask_16x8b);

        de = 0;
        dep = 0;
        deq = 0;
        if(d < beta)
        {
            d_sam0 = 0;
            if((2 * d0 < (beta >> 2))
                            && (de_2 < (beta >> 3))
                            && (de_0 < ((5 * tc + 1) >> 1)))
            {
                d_sam0 = 1;
            }

            d_sam3 = 0;
            if((2 * d3 < (beta >> 2))
                            && (de_3 < (beta >> 3))
                            && de_1 < ((5 * tc + 1) >> 1))
            {
                d_sam3 = 1;
            }

            de = (d_sam0 & d_sam3) + 1;
            dep = (dp < (beta + (beta >> 1)) >> 3) ? 1 : 0;
            deq = (dq < (beta + (beta >> 1)) >> 3) ? 1 : 0;
            if(tc <= 1)
            {
                dep = 0;
                deq = 0;
            }
        }

    }

    if(de != 0)
    {


        src_row1_8x16b = _mm_loadl_epi64((__m128i *)((pu1_src - 4) + src_strd));
        src_row2_8x16b = _mm_loadl_epi64((__m128i *)((pu1_src - 4) + 2 * src_strd));

        if(de == 2)
        {
            __m128i temp_pq_str0_16x8b;
            __m128i temp_pq1_str0_16x8b, temp_pq1_str1_16x8b;
            __m128i temp_pq2_str0_16x8b;
            __m128i temp_pq_str1_16x8b;
            __m128i temp_str0_16x8b, temp_str1_16x8b, temp_str2_16x8b, temp_str3_16x8b;
            __m128i temp_max0_16x8b, temp_max1_16x8b, temp_min0_16x8b, temp_min1_16x8b;
            __m128i const2_8x16b, const2tc_8x16b;
            LWORD64 mask, tc2;
            tc = tc << 1;
            mask = (((LWORD64)filter_flag_q) << 63) | (((LWORD64)filter_flag_p) << 31);
            tc2 = ((LWORD64)tc);

            const2_8x16b = _mm_cmpeq_epi16(src_row0_8x16b, src_row0_8x16b);
            //q'0-q'1-2 ,p'0-p'1-2
            src_row0_8x16b = _mm_unpacklo_epi64(src_row0_8x16b, src_row2_8x16b);
            src_row1_8x16b = _mm_unpacklo_epi64(src_row1_8x16b, src_row3_8x16b);

            const2_8x16b = _mm_srli_epi16(const2_8x16b, 15);
            temp_pq_str0_16x8b = _mm_srli_epi64(src_row0_8x16b, 16);
            temp_pq_str1_16x8b = _mm_srli_epi64(src_row1_8x16b, 16);
            //arranged x x x x x x x x q31 q30 q1 q10 p30 p31 p10 p11 , x x x x x x x x q21 q20 q01 q00 p20 p21 p00 p01
            temp_str0_16x8b = _mm_unpacklo_epi16(temp_pq_str0_16x8b, temp_pq_str1_16x8b);
            temp_str1_16x8b = _mm_unpackhi_epi16(temp_pq_str0_16x8b, temp_pq_str1_16x8b);

            const2_8x16b = _mm_packus_epi16(const2_8x16b, const2_8x16b);
            //arranged q31 q30 q21 q20 q1 q10 q01 q00 p30 p31 p20 p21 p10 p11 p00 p01
            temp_pq_str0_16x8b = _mm_unpacklo_epi32(temp_str0_16x8b, temp_str1_16x8b);

            temp_pq_str0_16x8b = _mm_maddubs_epi16(temp_pq_str0_16x8b, const2_8x16b);

            //q'1-2, p'1-2
            temp_pq1_str0_16x8b = _mm_srli_epi64(src_row0_8x16b, 8);
            temp_pq1_str1_16x8b = _mm_srli_epi64(src_row1_8x16b, 8);

            temp_str2_16x8b = _mm_unpacklo_epi16(temp_pq1_str0_16x8b, temp_pq1_str1_16x8b);
            temp_str3_16x8b = _mm_unpackhi_epi16(temp_pq1_str0_16x8b, temp_pq1_str1_16x8b);

            temp_str2_16x8b = _mm_shuffle_epi32(temp_str2_16x8b, 0x58);
            temp_str3_16x8b = _mm_shuffle_epi32(temp_str3_16x8b, 0x58);
            // q30 p30 q20 p20 q10 p10 q01 q00 p30 q20 p20 q10 p10 q01 q00 p00
            temp_pq1_str0_16x8b = _mm_unpackhi_epi32(temp_str2_16x8b, temp_str3_16x8b);
            // q32 q31 q22 q21 q12 q11 q02 q01 p32 p31 p22 p21 p12 p11 p02 p01
            temp_pq1_str1_16x8b = _mm_unpacklo_epi32(temp_str2_16x8b, temp_str3_16x8b);

            temp_pq1_str0_16x8b = _mm_maddubs_epi16(temp_pq1_str0_16x8b, const2_8x16b);
            temp_pq1_str1_16x8b = _mm_maddubs_epi16(temp_pq1_str1_16x8b, const2_8x16b);

            //clipping mask design
            temp_str1_16x8b = _mm_setzero_si128();
            temp_str0_16x8b = _mm_loadl_epi64((__m128i *)(&mask));
            const2tc_8x16b  = _mm_loadl_epi64((__m128i *)(&tc2));
            temp_str0_16x8b = _mm_shuffle_epi32(temp_str0_16x8b, 0x44);
            const2tc_8x16b  = _mm_shuffle_epi8(const2tc_8x16b, temp_str1_16x8b);

            //clipping mask design
            temp_str0_16x8b = _mm_srai_epi32(temp_str0_16x8b, 31);
            const2tc_8x16b = _mm_and_si128(const2tc_8x16b, temp_str0_16x8b);
            //calculating Clipping MAX for all pixel values.
            temp_max0_16x8b = _mm_adds_epu8(src_row0_8x16b, const2tc_8x16b);
            temp_max1_16x8b = _mm_adds_epu8(src_row1_8x16b, const2tc_8x16b);


            //q'2-q'0-2,p'2-p'0-2
            temp_pq2_str0_16x8b = _mm_unpacklo_epi16(src_row0_8x16b, src_row2_8x16b);
            temp_str3_16x8b     = _mm_unpacklo_epi16(src_row1_8x16b, src_row3_8x16b);

            temp_pq2_str0_16x8b = _mm_shuffle_epi32(temp_pq2_str0_16x8b, 0x5c);
            temp_str3_16x8b     = _mm_shuffle_epi32(temp_str3_16x8b, 0x5c);

            const2_8x16b = _mm_slli_epi16(const2_8x16b, 1);
            //arranged q33 q32 q23 q22 q13 q12 q03 q02 p33 p32 p23 p22 p13 p12 p03 p02
            temp_str3_16x8b = _mm_unpacklo_epi16(temp_pq2_str0_16x8b, temp_str3_16x8b);

            temp_pq2_str0_16x8b = _mm_maddubs_epi16(temp_str3_16x8b, const2_8x16b);

            //calculating Clipping MIN for all pixel values.
            temp_min0_16x8b = _mm_subs_epu8(src_row0_8x16b, const2tc_8x16b);
            temp_min1_16x8b = _mm_subs_epu8(src_row1_8x16b, const2tc_8x16b);
            //q'0-q'1-2 ,p'0-p'1-2
            temp_pq_str1_16x8b = _mm_shuffle_epi32(temp_pq_str0_16x8b, 0x4e);
            temp_pq_str0_16x8b = _mm_add_epi16(temp_pq_str0_16x8b, temp_pq_str1_16x8b);
            //q'1-2 p'1-2
            temp_pq1_str0_16x8b = _mm_add_epi16(temp_pq1_str0_16x8b, temp_pq1_str1_16x8b);
            //to get 2 in 16 bit
            const2_8x16b = _mm_srli_epi16(const2_8x16b, 8);
            //to get q33 q23 q13 q03, p33 p23 p13 p03
            temp_pq1_str1_16x8b = _mm_slli_epi16(temp_str3_16x8b, 8);
            temp_pq_str1_16x8b = _mm_srli_epi16(temp_str3_16x8b, 8);
            temp_pq1_str1_16x8b = _mm_srli_epi16(temp_pq1_str1_16x8b, 8);

            //q'1, p'1 (adding 2)
            temp_pq1_str0_16x8b = _mm_add_epi16(temp_pq1_str0_16x8b, const2_8x16b);
            //q'0-q'1,p'0-p'1
            temp_pq_str0_16x8b = _mm_add_epi16(temp_pq_str0_16x8b, const2_8x16b);
            //q'2-q'1,p'2-p'1
            temp_pq2_str0_16x8b = _mm_add_epi16(temp_pq2_str0_16x8b, const2_8x16b);
            //q'0 = (q'0-q'1)+q'1 ,p'0 = (p'0-p'1)+p'1;
            temp_pq_str0_16x8b = _mm_add_epi16(temp_pq1_str0_16x8b, temp_pq_str0_16x8b);
            //q'2 = (q'2-q'1)+q'1 ,p'2 = (p'2-p'1)+p'1;
            temp_pq2_str0_16x8b = _mm_add_epi16(temp_pq1_str0_16x8b, temp_pq2_str0_16x8b);

            //normalisation of all modified pixels
            temp_pq_str0_16x8b  = _mm_srai_epi16(temp_pq_str0_16x8b, 3);
            temp_pq1_str0_16x8b = _mm_srai_epi16(temp_pq1_str0_16x8b, 2);
            temp_pq2_str0_16x8b = _mm_srai_epi16(temp_pq2_str0_16x8b, 3);

            //getting p0 p1 together and p2 p3 together
            temp_str0_16x8b = _mm_unpacklo_epi16(temp_pq1_str0_16x8b, temp_pq_str0_16x8b);
            temp_str2_16x8b = _mm_unpacklo_epi16(temp_pq1_str1_16x8b, temp_pq2_str0_16x8b);
            //getting q1 q0 together and  q3 q2 together
            temp_pq_str0_16x8b = _mm_unpackhi_epi16(temp_pq_str0_16x8b, temp_pq1_str0_16x8b);
            temp_pq2_str0_16x8b = _mm_unpackhi_epi16(temp_pq2_str0_16x8b, temp_pq_str1_16x8b);
            //getting p's of row0 row1 together and of row2 row3 together
            temp_pq_str1_16x8b = _mm_unpacklo_epi32(temp_str2_16x8b, temp_str0_16x8b);
            temp_str2_16x8b    = _mm_unpackhi_epi32(temp_str2_16x8b, temp_str0_16x8b);
            //getting q's of row0 row1 together and of row2 row3 together
            temp_str0_16x8b    = _mm_unpacklo_epi32(temp_pq_str0_16x8b, temp_pq2_str0_16x8b);
            temp_pq_str0_16x8b = _mm_unpackhi_epi32(temp_pq_str0_16x8b, temp_pq2_str0_16x8b);
            //getting values for respective rows in 16 bit
            src_row0_8x16b = _mm_unpacklo_epi64(temp_pq_str1_16x8b, temp_str0_16x8b);
            src_row1_8x16b = _mm_unpackhi_epi64(temp_pq_str1_16x8b, temp_str0_16x8b);
            src_row2_8x16b = _mm_unpacklo_epi64(temp_str2_16x8b, temp_pq_str0_16x8b);
            src_row3_8x16b = _mm_unpackhi_epi64(temp_str2_16x8b, temp_pq_str0_16x8b);
            //packing values to 8 bit
            src_row0_8x16b = _mm_packus_epi16(src_row0_8x16b, src_row2_8x16b);
            src_row1_8x16b = _mm_packus_epi16(src_row1_8x16b, src_row3_8x16b);
            //Clipping MAX
            src_row0_8x16b = _mm_min_epu8(src_row0_8x16b, temp_max0_16x8b);
            src_row1_8x16b = _mm_min_epu8(src_row1_8x16b, temp_max1_16x8b);
            //Clipping MIN
            src_row0_8x16b = _mm_max_epu8(src_row0_8x16b, temp_min0_16x8b);
            src_row1_8x16b = _mm_max_epu8(src_row1_8x16b, temp_min1_16x8b);
            //separating row 2 and row 3
            src_row2_8x16b = _mm_srli_si128(src_row0_8x16b, 8);
            src_row3_8x16b = _mm_srli_si128(src_row1_8x16b, 8);

        }

        else
        {

            __m128i tmp_delta0_8x16b, tmp_delta1_8x16b, tmp_delta2_8x16b, tmp_delta3_8x16b;
            __m128i tmp0_const_8x16b, tmp1_const_8x16b, tmp2_const_8x16b, tmp3_const_8x16b;
            __m128i coefdelta_0_8x16b, mask_pq_8x16b;
            __m128i const2_8x16b, consttc_8x16b;

            LWORD64 mask1;
            mask1 = (((LWORD64)(filter_flag_q & deq)) << 63) | (((LWORD64)filter_flag_q) << 47) | (((LWORD64)filter_flag_p) << 31) | (((LWORD64)(filter_flag_p & dep)) << 15);

            consttc_8x16b = _mm_set1_epi32(tc);


            src_row0_8x16b = _mm_unpacklo_epi64(src_row0_8x16b, src_row1_8x16b);
            src_row2_8x16b = _mm_unpacklo_epi64(src_row2_8x16b, src_row3_8x16b);

            tmp_delta2_8x16b = _mm_srli_epi64(src_row0_8x16b, 16);
            tmp_delta3_8x16b = _mm_srli_epi64(src_row2_8x16b, 16);

            tmp_delta2_8x16b = _mm_shuffle_epi32(tmp_delta2_8x16b, 0x08);
            tmp_delta3_8x16b = _mm_shuffle_epi32(tmp_delta3_8x16b, 0x08);
            //arranged q31 q30 p30 p31  q21 q20 p20 p21  q1 q10 p10 p11 q01 q00 p00 p01
            tmp_delta2_8x16b = _mm_unpacklo_epi64(tmp_delta2_8x16b, tmp_delta3_8x16b);

            coefdelta_0_8x16b = _mm_load_si128((__m128i *)coef_de1);
            // (-3q1+9q0),(-9p0+3p1)
            tmp_delta3_8x16b = _mm_maddubs_epi16(tmp_delta2_8x16b, coefdelta_0_8x16b);
            //converting to 16 bit
            consttc_8x16b = _mm_packs_epi32(consttc_8x16b, consttc_8x16b);
            //getting -tc store
            tmp1_const_8x16b = _mm_cmpeq_epi32(consttc_8x16b, consttc_8x16b);
            //calc 10 *tc = 2*tc +8*tc ; 2*tc
            tmp2_const_8x16b = _mm_slli_epi16(consttc_8x16b, 1);
            //calc 10 *tc = 2*tc +8*tc ; 8*tc
            tmp0_const_8x16b = _mm_slli_epi16(consttc_8x16b, 3);
            //getting -tc store
            tmp3_const_8x16b = _mm_sign_epi16(consttc_8x16b, tmp1_const_8x16b);
            //calc 10 *tc
            tmp2_const_8x16b = _mm_add_epi16(tmp2_const_8x16b, tmp0_const_8x16b);
            //const 1
            const2_8x16b = _mm_srli_epi16(tmp1_const_8x16b, 15);
            tmp_delta0_8x16b = _mm_madd_epi16(tmp_delta3_8x16b, const2_8x16b);
            const2_8x16b = _mm_srli_epi32(tmp1_const_8x16b, 31);
            //getting the mask values
            mask_pq_8x16b = _mm_loadl_epi64((__m128i *)(&mask1));
            //loaded coef for delta1 calculation
            coefdelta_0_8x16b = _mm_load_si128((__m128i *)coef_dep1);
            //(-2q1+q0),(p0-2p1)
            tmp_delta3_8x16b = _mm_maddubs_epi16(tmp_delta2_8x16b, coefdelta_0_8x16b);
            //const 8
            const2_8x16b = _mm_slli_epi32(const2_8x16b, 3);
            //rearranging the mask values
            mask_pq_8x16b = _mm_unpacklo_epi64(mask_pq_8x16b, mask_pq_8x16b);
            //normalisation of the filter
            tmp_delta0_8x16b = _mm_add_epi32(tmp_delta0_8x16b, const2_8x16b);
            tmp_delta0_8x16b = _mm_srai_epi32(tmp_delta0_8x16b, 4);

            //getting deltaq0
            tmp_delta2_8x16b = _mm_sign_epi32(tmp_delta0_8x16b, tmp1_const_8x16b);
            //packing  d3q d2q d1q d0q d3p d2p d1p d0p
            tmp_delta0_8x16b = _mm_packs_epi32(tmp_delta0_8x16b, tmp_delta2_8x16b);
            //absolute delta
            tmp_delta2_8x16b = _mm_abs_epi16(tmp_delta0_8x16b);
            //Clipping of delta0
            tmp_delta0_8x16b = _mm_min_epi16(tmp_delta0_8x16b, consttc_8x16b);
            //mask for |delta| < 10*tc
            tmp0_const_8x16b = _mm_cmpgt_epi16(tmp2_const_8x16b, tmp_delta2_8x16b);
            //Clipping of delta0
            tmp_delta0_8x16b = _mm_max_epi16(tmp_delta0_8x16b, tmp3_const_8x16b);


            //delta 1 calc starts

            //getting q32 q22 q12 q02 p32 p12 p22 p02
            tmp2_const_8x16b = _mm_loadl_epi64((__m128i *)(shuffle0));
            tmp_delta2_8x16b = _mm_shuffle_epi8(src_row0_8x16b, tmp2_const_8x16b);
            tmp_delta1_8x16b =  _mm_shuffle_epi8(src_row2_8x16b, tmp2_const_8x16b);
            tmp_delta1_8x16b = _mm_unpacklo_epi32(tmp_delta2_8x16b, tmp_delta1_8x16b);
            //constant 1
            const2_8x16b = _mm_srli_epi16(tmp1_const_8x16b, 15);
            //tc>>1 16 bit
            consttc_8x16b = _mm_srai_epi16(consttc_8x16b, 1);

            //getting -tc>>1 store  16 bit
            tmp1_const_8x16b = _mm_sign_epi16(consttc_8x16b, tmp1_const_8x16b);
            //2*delta0
            tmp2_const_8x16b = _mm_add_epi16(tmp_delta0_8x16b, tmp_delta0_8x16b);

            //getting  all respective q's and p's together
            tmp3_const_8x16b = _mm_load_si128((__m128i *)(shuffle1));
            tmp_delta3_8x16b = _mm_shuffle_epi8(tmp_delta3_8x16b, tmp3_const_8x16b);
            //final adds for deltap1 and deltaq1
            tmp_delta3_8x16b = _mm_add_epi16(tmp_delta3_8x16b, const2_8x16b);
            tmp_delta1_8x16b = _mm_add_epi16(tmp_delta1_8x16b, tmp2_const_8x16b);
            tmp_delta1_8x16b = _mm_add_epi16(tmp_delta1_8x16b, tmp_delta3_8x16b);
            tmp2_const_8x16b = _mm_setzero_si128();
            tmp_delta1_8x16b = _mm_srai_epi16(tmp_delta1_8x16b, 2);

            // clipping delta1
            tmp_delta1_8x16b = _mm_min_epi16(tmp_delta1_8x16b, consttc_8x16b);
            // clipping delta1
            tmp_delta1_8x16b = _mm_max_epi16(tmp_delta1_8x16b, tmp1_const_8x16b);

            //getting the mask ready
            mask_pq_8x16b = _mm_srai_epi16(mask_pq_8x16b, 15);
            //masking of the delta values |delta|<10*tc
            tmp_delta1_8x16b = _mm_and_si128(tmp_delta1_8x16b, tmp0_const_8x16b);
            tmp_delta0_8x16b = _mm_and_si128(tmp_delta0_8x16b, tmp0_const_8x16b);
            //packing dq1 dq0 dp0 dp1
            tmp1_const_8x16b = _mm_unpacklo_epi16(tmp_delta1_8x16b, tmp_delta0_8x16b);
            tmp_delta0_8x16b = _mm_unpackhi_epi16(tmp_delta0_8x16b, tmp_delta1_8x16b);
            tmp_delta1_8x16b = _mm_unpackhi_epi32(tmp1_const_8x16b, tmp_delta0_8x16b);
            tmp_delta0_8x16b = _mm_unpacklo_epi32(tmp1_const_8x16b, tmp_delta0_8x16b);

            //masking of the delta values dep, deq , filter_p ,filter_q
            tmp_delta0_8x16b = _mm_and_si128(tmp_delta0_8x16b, mask_pq_8x16b);
            tmp_delta1_8x16b = _mm_and_si128(tmp_delta1_8x16b, mask_pq_8x16b);
            //converting 8bit to 16 bit
            src_row0_8x16b = _mm_unpacklo_epi8(src_row0_8x16b, tmp2_const_8x16b);
            src_row1_8x16b = _mm_unpacklo_epi8(src_row1_8x16b, tmp2_const_8x16b);
            src_row2_8x16b = _mm_unpacklo_epi8(src_row2_8x16b, tmp2_const_8x16b);
            src_row3_8x16b = _mm_unpacklo_epi8(src_row3_8x16b, tmp2_const_8x16b);
            //shuffle values loaded
            tmp0_const_8x16b = _mm_load_si128((__m128i *)shuffle2);
            tmp1_const_8x16b = _mm_load_si128((__m128i *)shuffle3);
            //arranging each row delta in different registers
            tmp_delta3_8x16b = _mm_shuffle_epi8(tmp_delta1_8x16b, tmp1_const_8x16b);
            tmp_delta2_8x16b = _mm_shuffle_epi8(tmp_delta1_8x16b, tmp0_const_8x16b);
            tmp_delta1_8x16b = _mm_shuffle_epi8(tmp_delta0_8x16b, tmp1_const_8x16b);
            tmp_delta0_8x16b = _mm_shuffle_epi8(tmp_delta0_8x16b, tmp0_const_8x16b);

            //adding the respective delta
            src_row3_8x16b = _mm_add_epi16(tmp_delta3_8x16b, src_row3_8x16b);
            src_row2_8x16b = _mm_add_epi16(tmp_delta2_8x16b, src_row2_8x16b);
            src_row1_8x16b = _mm_add_epi16(tmp_delta1_8x16b, src_row1_8x16b);
            src_row0_8x16b = _mm_add_epi16(tmp_delta0_8x16b, src_row0_8x16b);
            //saturating to 8 bit
            src_row2_8x16b = _mm_packus_epi16(src_row2_8x16b, src_row3_8x16b);
            src_row0_8x16b = _mm_packus_epi16(src_row0_8x16b, src_row1_8x16b);
            //separating different rows
            src_row1_8x16b = _mm_srli_si128(src_row0_8x16b, 8);
            src_row3_8x16b = _mm_srli_si128(src_row2_8x16b, 8);
        }

        _mm_storel_epi64((__m128i *)(pu1_src - 4), src_row0_8x16b);
        _mm_storel_epi64((__m128i *)((pu1_src - 4) + src_strd), src_row1_8x16b);
        _mm_storel_epi64((__m128i *)((pu1_src - 4) + 2 * src_strd), src_row2_8x16b);
        _mm_storel_epi64((__m128i *)((pu1_src - 4) + 3 * src_strd), src_row3_8x16b);
    }
}

void ihevc_deblk_luma_horz_ssse3(UWORD8 *pu1_src,
                                 WORD32 src_strd,
                                 WORD32 bs,
                                 WORD32 quant_param_p,
                                 WORD32 quant_param_q,
                                 WORD32 beta_offset_div2,
                                 WORD32 tc_offset_div2,
                                 WORD32 filter_flag_p,
                                 WORD32 filter_flag_q)
{
    WORD32 qp_luma, beta_indx, tc_indx;
    WORD32 beta, tc;

    WORD32 d0, d3, dp, dq, d;
    WORD32 de_0, de_1, de_2, de_3;
    WORD32 d_sam0, d_sam3;
    WORD32 de, dep, deq;

    __m128i src_q0_8x16b, src_q1_8x16b, src_p0_8x16b, src_p1_8x16b, src_q2_8x16b;
    __m128i tmp_pq_str1_8x16b, src_p2_8x16b, tmp_pq_str0_8x16b;




    {
        __m128i src_tmp_p_0_8x16b, src_tmp_p_1_8x16b, src_tmp_q_0_8x16b, src_tmp_q_1_8x16b;
        __m128i coef_8x16b, mask_d_result_4x32b, mask_de_result_8x16b;
        __m128i mask_16x8b, temp_coef0_8x16b, temp_coef1_8x16b;

        ASSERT((bs > 0));
        ASSERT(filter_flag_p || filter_flag_q);

        qp_luma = (quant_param_p + quant_param_q + 1) >> 1;
        beta_indx = CLIP3(qp_luma + (beta_offset_div2 << 1), 0, 51);

        /* BS based on implementation can take value 3 if it is intra/inter egde          */
        /* based on BS, tc index is calcuated by adding 2 * ( bs - 1) to QP and tc_offset */
        /* for BS = 1 adding factor is (0*2), BS = 2 or 3 adding factor is (1*2)          */
        /* the above desired functionallity is achieved by doing (2*(bs>>1))              */

        tc_indx = CLIP3(qp_luma + 2 * (bs >> 1) + (tc_offset_div2 << 1), 0, 53);

        beta = gai4_ihevc_beta_table[beta_indx];
        tc = gai4_ihevc_tc_table[tc_indx];
        if(0 == tc)
        {
            return;
        }
        src_q0_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src));
        src_q1_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src + src_strd));
        src_p0_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src - src_strd));
        src_p1_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src - 2 * src_strd));
        src_q2_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src + 2 * src_strd));
        tmp_pq_str1_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src + 3 * src_strd));
        src_p2_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src - 3 * src_strd));
        tmp_pq_str0_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src - 4 * src_strd));


        src_tmp_p_0_8x16b = _mm_unpacklo_epi8(src_p1_8x16b, src_p0_8x16b);
        src_tmp_p_1_8x16b = _mm_unpacklo_epi8(tmp_pq_str0_8x16b, src_p2_8x16b);

        src_tmp_q_0_8x16b = _mm_unpacklo_epi8(src_q0_8x16b, src_q1_8x16b);
        src_tmp_q_1_8x16b = _mm_unpacklo_epi8(src_q2_8x16b, tmp_pq_str1_8x16b);

        src_tmp_p_0_8x16b = _mm_unpacklo_epi16(src_tmp_p_1_8x16b, src_tmp_p_0_8x16b);
        src_tmp_q_0_8x16b = _mm_unpacklo_epi16(src_tmp_q_0_8x16b, src_tmp_q_1_8x16b);

        src_tmp_p_0_8x16b = _mm_shuffle_epi32(src_tmp_p_0_8x16b, 0x6c);
        src_tmp_q_0_8x16b = _mm_shuffle_epi32(src_tmp_q_0_8x16b, 0x6c);

        coef_8x16b = _mm_load_si128((__m128i *)(coef_d));
        mask_16x8b =  _mm_load_si128((__m128i *)(shuffle_d));

        src_tmp_p_0_8x16b = _mm_unpacklo_epi32(src_tmp_p_0_8x16b, src_tmp_q_0_8x16b);
        //WORD32 shuffle_d[4]={0x80800403,0x80800c0b,0x03000704,0x0b080f0c};
        mask_de_result_8x16b = _mm_shuffle_epi8(src_tmp_p_0_8x16b, mask_16x8b);

        mask_d_result_4x32b = _mm_maddubs_epi16(src_tmp_p_0_8x16b, coef_8x16b);


        //to get all 1's of 8 bit in (1)
        temp_coef0_8x16b = _mm_cmpeq_epi16(src_tmp_p_0_8x16b, src_tmp_p_0_8x16b);
        temp_coef1_8x16b = _mm_srli_epi16(temp_coef0_8x16b, 15);
        //accumulating values foe dp3 dq3 , dp0 dq0 values
        mask_d_result_4x32b = _mm_madd_epi16(mask_d_result_4x32b, temp_coef1_8x16b);

        temp_coef1_8x16b = _mm_packus_epi16(temp_coef1_8x16b, temp_coef1_8x16b);
        // to get all 1,-1 sets of 16 bits in (0)
        temp_coef0_8x16b = _mm_unpacklo_epi8(temp_coef0_8x16b, temp_coef1_8x16b);
        //q33-q30,p33-p30,q03-q00,p03-p00,0,q30-p30,0,q00-p00
        mask_de_result_8x16b = _mm_maddubs_epi16(mask_de_result_8x16b, temp_coef0_8x16b);
        //to get 16 bit 1's
        temp_coef0_8x16b = _mm_srli_epi16(temp_coef1_8x16b, 8);


        // dq3 dp3 dq0 dp0
        mask_d_result_4x32b = _mm_abs_epi32(mask_d_result_4x32b);
        mask_16x8b = _mm_shuffle_epi32(mask_d_result_4x32b, 0xec);
        mask_d_result_4x32b = _mm_shuffle_epi32(mask_d_result_4x32b, 0x49);
        // dq dp d3 d0
        mask_d_result_4x32b = _mm_add_epi32(mask_d_result_4x32b, mask_16x8b);
        //|q33-q30|,|p33-p30|,|q03-q00|,|p03-p00|,0,|q30-p30|,0,|q00-p00|
        mask_de_result_8x16b = _mm_abs_epi16(mask_de_result_8x16b);
        //|q33-q30|+|p33-p30|,|q03-q00|+|p03-p00|,0+|q30-p30|,0+|q00-p00|
        mask_de_result_8x16b = _mm_madd_epi16(mask_de_result_8x16b, temp_coef0_8x16b);

        ///store back in a single variable
        temp_coef0_8x16b = _mm_srli_si128(mask_d_result_4x32b, 4);
        temp_coef1_8x16b = _mm_srli_si128(mask_d_result_4x32b, 8);
        mask_16x8b = _mm_srli_si128(mask_d_result_4x32b, 12);

        d0 = _mm_cvtsi128_si32(mask_d_result_4x32b);
        d3 = _mm_cvtsi128_si32(temp_coef0_8x16b);
        dp = _mm_cvtsi128_si32(temp_coef1_8x16b);
        dq = _mm_cvtsi128_si32(mask_16x8b);
        //getting d
        d = d0 + d3;

        ///store back in a single variable
        temp_coef0_8x16b = _mm_srli_si128(mask_de_result_8x16b, 4);
        temp_coef1_8x16b = _mm_srli_si128(mask_de_result_8x16b, 8);
        mask_16x8b = _mm_srli_si128(mask_de_result_8x16b, 12);

        de_0 = _mm_cvtsi128_si32(mask_de_result_8x16b);
        de_1 = _mm_cvtsi128_si32(temp_coef0_8x16b);
        de_2 = _mm_cvtsi128_si32(temp_coef1_8x16b);
        de_3 = _mm_cvtsi128_si32(mask_16x8b);

        de = 0;
        dep = 0;
        deq = 0;
        if(d < beta)
        {
            d_sam0 = 0;
            if((2 * d0 < (beta >> 2))
                            && (de_2 < (beta >> 3))
                            && (de_0 < ((5 * tc + 1) >> 1)))
            {
                d_sam0 = 1;
            }

            d_sam3 = 0;
            if((2 * d3 < (beta >> 2))
                            && (de_3 < (beta >> 3))
                            && de_1 < ((5 * tc + 1) >> 1))
            {
                d_sam3 = 1;
            }

            de = (d_sam0 & d_sam3) + 1;
            dep = (dp < (beta + (beta >> 1)) >> 3) ? 1 : 0;
            deq = (dq < (beta + (beta >> 1)) >> 3) ? 1 : 0;
            if(tc <= 1)
            {
                dep = 0;
                deq = 0;
            }
        }

    }

    if(de != 0)
    {

        if(2 == de)
        {

            __m128i temp_pq0_str0_16x8b;
            __m128i temp_pq1_str0_16x8b, temp_pq1_str1_16x8b;
            __m128i temp_pq2_str0_16x8b;
            __m128i temp_str0_16x8b, temp_str1_16x8b;
            __m128i const2_8x16b, const2tc_8x16b;

            LWORD64 mask, tc2;
            tc = tc << 1;
            mask = (((LWORD64)filter_flag_q) << 63) | (((LWORD64)filter_flag_p) << 31);
            tc2 = ((LWORD64)tc);

            const2_8x16b = _mm_cmpeq_epi16(src_p1_8x16b, src_p1_8x16b);
            //q'0-q'1-2 ,p'0-p'1-2
            temp_pq0_str0_16x8b = _mm_unpacklo_epi8(src_p1_8x16b, src_p0_8x16b);
            temp_str0_16x8b   = _mm_unpacklo_epi8(src_q0_8x16b, src_q1_8x16b);
            const2_8x16b = _mm_srli_epi16(const2_8x16b, 15);
            //arranged q31 q30 q21 q20 q1 q10 q01 q00 p30 p31 p20 p21 p10 p11 p00 p01
            temp_pq0_str0_16x8b = _mm_unpacklo_epi64(temp_pq0_str0_16x8b, temp_str0_16x8b);

            const2_8x16b = _mm_packus_epi16(const2_8x16b, const2_8x16b);
            temp_pq0_str0_16x8b = _mm_maddubs_epi16(temp_pq0_str0_16x8b, const2_8x16b);

            //q'1-2, p'1-2
            temp_pq1_str0_16x8b = _mm_unpacklo_epi8(src_p0_8x16b, src_q0_8x16b);
            temp_pq1_str1_16x8b = _mm_unpacklo_epi8(src_q1_8x16b, src_q2_8x16b);
            temp_str1_16x8b = _mm_unpacklo_epi8(src_p1_8x16b, src_p2_8x16b);
            // q30 p30 q20 p20 q10 p10 q01 q00 p30 q20 p20 q10 p10 q01 q00 p00
            temp_pq1_str0_16x8b = _mm_unpacklo_epi64(temp_pq1_str0_16x8b, temp_pq1_str0_16x8b);
            // q32 q31 q22 q21 q12 q11 q02 q01 p32 p31 p22 p21 p12 p11 p02 p01
            temp_pq1_str1_16x8b = _mm_unpacklo_epi64(temp_str1_16x8b, temp_pq1_str1_16x8b);

            temp_pq1_str0_16x8b = _mm_maddubs_epi16(temp_pq1_str0_16x8b, const2_8x16b);
            temp_pq1_str1_16x8b = _mm_maddubs_epi16(temp_pq1_str1_16x8b, const2_8x16b);

            //clipping mask design
            temp_str1_16x8b = _mm_setzero_si128();
            temp_str0_16x8b = _mm_loadl_epi64((__m128i *)(&mask));
            const2tc_8x16b  = _mm_loadl_epi64((__m128i *)(&tc2));
            temp_str0_16x8b = _mm_shuffle_epi32(temp_str0_16x8b, 0x44);
            const2tc_8x16b  = _mm_shuffle_epi8(const2tc_8x16b, temp_str1_16x8b);

            //clipping mask design
            temp_str0_16x8b = _mm_srai_epi32(temp_str0_16x8b, 31);
            const2tc_8x16b = _mm_and_si128(const2tc_8x16b, temp_str0_16x8b);
            //calculating Clipping MAX for all pixel values.
            src_p0_8x16b = _mm_unpacklo_epi32(src_p0_8x16b, src_q0_8x16b);
            src_q0_8x16b = _mm_unpacklo_epi32(src_p1_8x16b, src_q1_8x16b);
            //for clipping calc
            src_p1_8x16b = _mm_unpacklo_epi64(src_p0_8x16b, src_q0_8x16b);
            //saving the unmodified data of q1 p1 q0 p0
            src_q1_8x16b = _mm_unpackhi_epi64(src_p0_8x16b, src_q0_8x16b);
            //CLIpping MAX and MIN for q1 p1 q0 p0
            src_p0_8x16b = _mm_adds_epu8(src_p1_8x16b, const2tc_8x16b);
            src_p1_8x16b = _mm_subs_epu8(src_p1_8x16b, const2tc_8x16b);


            //q'2-q'0-2,p'2-p'0-2
            tmp_pq_str0_8x16b = _mm_unpacklo_epi8(src_p2_8x16b, tmp_pq_str0_8x16b);
            temp_pq2_str0_16x8b = _mm_unpacklo_epi8(src_q2_8x16b, tmp_pq_str1_8x16b);
            const2_8x16b = _mm_slli_epi16(const2_8x16b, 1);
            //arranged q33 q32 q23 q22 q13 q12 q03 q02 p32 p33 p22 p23 p12 p13 p02 p03
            temp_pq2_str0_16x8b = _mm_unpacklo_epi64(tmp_pq_str0_8x16b, temp_pq2_str0_16x8b);
            src_p2_8x16b = _mm_unpacklo_epi32(src_p2_8x16b, src_q2_8x16b);
            temp_pq2_str0_16x8b = _mm_maddubs_epi16(temp_pq2_str0_16x8b, const2_8x16b);

            //calculating Clipping MAX and MIN for p2 and q2 .
            tmp_pq_str0_8x16b = _mm_adds_epu8(src_p2_8x16b, const2tc_8x16b);
            tmp_pq_str1_8x16b = _mm_subs_epu8(src_p2_8x16b, const2tc_8x16b);
            //q'0-q'1-2 ,p'0-p'1-2
            temp_str0_16x8b = _mm_shuffle_epi32(temp_pq0_str0_16x8b, 0x4e);
            temp_pq0_str0_16x8b = _mm_add_epi16(temp_pq0_str0_16x8b, temp_str0_16x8b);
            //q'1-2 p'1-2
            temp_pq1_str0_16x8b = _mm_add_epi16(temp_pq1_str0_16x8b, temp_pq1_str1_16x8b);
            //to get 2 in 16 bit
            const2_8x16b = _mm_srli_epi16(const2_8x16b, 8);


            //q'1, p'1 (adding 2)
            temp_pq1_str0_16x8b = _mm_add_epi16(temp_pq1_str0_16x8b, const2_8x16b);
            //q'0-q'1,p'0-p'1
            temp_pq0_str0_16x8b = _mm_add_epi16(temp_pq0_str0_16x8b, const2_8x16b);
            //q'2-q'1,p'2-p'1
            temp_pq2_str0_16x8b = _mm_add_epi16(temp_pq2_str0_16x8b, const2_8x16b);
            //q'0 = (q'0-q'1)+q'1 ,p'0 = (p'0-p'1)+p'1;
            temp_pq0_str0_16x8b = _mm_add_epi16(temp_pq1_str0_16x8b, temp_pq0_str0_16x8b);
            //q'2 = (q'2-q'1)+q'1 ,p'2 = (p'2-p'1)+p'1;
            temp_pq2_str0_16x8b = _mm_add_epi16(temp_pq1_str0_16x8b, temp_pq2_str0_16x8b);

            //normalisation of all modified pixels
            temp_pq0_str0_16x8b = _mm_srai_epi16(temp_pq0_str0_16x8b, 3);
            temp_pq1_str0_16x8b = _mm_srai_epi16(temp_pq1_str0_16x8b, 2);
            temp_pq2_str0_16x8b = _mm_srai_epi16(temp_pq2_str0_16x8b, 3);
            //q'1 p'1 q'0 p'0
            temp_pq0_str0_16x8b = _mm_packus_epi16(temp_pq0_str0_16x8b, temp_pq1_str0_16x8b);
            temp_pq2_str0_16x8b = _mm_packus_epi16(temp_pq2_str0_16x8b, temp_pq2_str0_16x8b);
            //pack with the unmodified data of q2 and p2
            src_p2_8x16b = _mm_unpackhi_epi64(temp_pq2_str0_16x8b, src_p2_8x16b);
            //Clipping MAX and MIN for q'1 p'1 q'0 p'0 and q'2  p'2
            temp_pq0_str0_16x8b = _mm_min_epu8(temp_pq0_str0_16x8b, src_p0_8x16b);
            src_p2_8x16b = _mm_min_epu8(src_p2_8x16b, tmp_pq_str0_8x16b);
            temp_pq0_str0_16x8b = _mm_max_epu8(temp_pq0_str0_16x8b, src_p1_8x16b);
            src_p2_8x16b = _mm_max_epu8(src_p2_8x16b, tmp_pq_str1_8x16b);
            //Reshuffling q'1 p'1 q'0 p'0 along with unmodified data
            src_p0_8x16b = _mm_unpacklo_epi32(temp_pq0_str0_16x8b, src_q1_8x16b);
            src_p1_8x16b = _mm_unpackhi_epi32(temp_pq0_str0_16x8b, src_q1_8x16b);
            src_p2_8x16b = _mm_shuffle_epi32(src_p2_8x16b, 0xd8);
            src_q0_8x16b = _mm_srli_si128(src_p0_8x16b, 8);
            src_q1_8x16b = _mm_srli_si128(src_p1_8x16b, 8);
            src_q2_8x16b = _mm_srli_si128(src_p2_8x16b, 8);

            _mm_storel_epi64((__m128i *)(pu1_src - 3 * src_strd), src_p2_8x16b);
            _mm_storel_epi64((__m128i *)(pu1_src - 2 * src_strd), src_p1_8x16b);
            _mm_storel_epi64((__m128i *)(pu1_src - src_strd), src_p0_8x16b);
            _mm_storel_epi64((__m128i *)(pu1_src), src_q0_8x16b);
            _mm_storel_epi64((__m128i *)(pu1_src + src_strd), src_q1_8x16b);
            _mm_storel_epi64((__m128i *)(pu1_src + 2 * src_strd), src_q2_8x16b);


        }

        else
        {

            __m128i tmp_delta0_8x16b, tmp_delta1_8x16b;
            __m128i tmp0_const_8x16b, tmp1_const_8x16b, tmp2_const_8x16b;
            __m128i coefdelta_0_8x16b;
            __m128i const2_8x16b, consttc_8x16b;

            LWORD64 maskp0, maskp1, maskq0, maskq1;
            maskp0 = (LWORD64)filter_flag_p;
            maskq0 = (LWORD64)filter_flag_q;
            maskp1 = (LWORD64)dep;
            maskq1 = (LWORD64)deq;
            consttc_8x16b = _mm_set1_epi32(tc);

            tmp_delta0_8x16b = _mm_unpacklo_epi8(src_p1_8x16b, src_p0_8x16b);
            tmp_delta1_8x16b = _mm_unpacklo_epi8(src_q0_8x16b, src_q1_8x16b);
            //arranged q31 q30 p30 p31  q21 q20 p20 p21  q1 q10 p10 p11 q01 q00 p00 p01
            tmp_delta1_8x16b = _mm_unpacklo_epi16(tmp_delta0_8x16b, tmp_delta1_8x16b);

            coefdelta_0_8x16b = _mm_load_si128((__m128i *)coef_de1);
            // (-3q1+9q0),(-9p0+3p1)
            tmp_delta0_8x16b = _mm_maddubs_epi16(tmp_delta1_8x16b, coefdelta_0_8x16b);

            //getting -tc store
            tmp2_const_8x16b = _mm_cmpeq_epi32(consttc_8x16b, consttc_8x16b);

            //getting tc in 16 bit
            consttc_8x16b = _mm_packs_epi32(consttc_8x16b, consttc_8x16b);
            //calc 10 *tc = 2*tc +8*tc ; 2*tc
            tmp_pq_str0_8x16b = _mm_slli_epi16(consttc_8x16b, 1);
            //calc 10 *tc = 2*tc +8*tc ; 8*tc
            tmp_pq_str1_8x16b = _mm_slli_epi16(consttc_8x16b, 3);

            //const 1
            const2_8x16b = _mm_srli_epi16(tmp2_const_8x16b, 15);
            //calc 10 *tc
            tmp_pq_str0_8x16b = _mm_add_epi16(tmp_pq_str0_8x16b, tmp_pq_str1_8x16b);
            //delta0 without normalisation and clipping
            tmp_delta0_8x16b = _mm_madd_epi16(tmp_delta0_8x16b, const2_8x16b);

            const2_8x16b = _mm_srli_epi32(tmp2_const_8x16b, 31);

            //loaded coef for delta1 calculation
            coefdelta_0_8x16b = _mm_load_si128((__m128i *)coef_dep1);
            //(-2q1+q0),(p0-2p1)
            tmp_delta1_8x16b = _mm_maddubs_epi16(tmp_delta1_8x16b, coefdelta_0_8x16b);
            //const 8
            const2_8x16b = _mm_slli_epi32(const2_8x16b, 3);

            //normalisation of the filter
            tmp_delta0_8x16b = _mm_add_epi32(tmp_delta0_8x16b, const2_8x16b);
            tmp_delta0_8x16b = _mm_srai_epi32(tmp_delta0_8x16b, 4);

            //getting deltaq0
            tmp_pq_str1_8x16b = _mm_sign_epi32(tmp_delta0_8x16b, tmp2_const_8x16b);
            //getting -tc
            tmp1_const_8x16b = _mm_sign_epi16(consttc_8x16b, tmp2_const_8x16b);
            //packing  d03q d02q d01q d0q d03p d02p d01p d00p
            tmp_delta0_8x16b = _mm_packs_epi32(tmp_delta0_8x16b, tmp_pq_str1_8x16b);
            //absolute delta
            tmp_pq_str1_8x16b = _mm_abs_epi16(tmp_delta0_8x16b);

            //Clipping of delta0
            tmp_delta0_8x16b = _mm_min_epi16(tmp_delta0_8x16b, consttc_8x16b);
            //tc>>1 16 bit
            consttc_8x16b = _mm_srai_epi16(consttc_8x16b, 1);
            //Clipping of delta0
            tmp_delta0_8x16b = _mm_max_epi16(tmp_delta0_8x16b, tmp1_const_8x16b);

            //(-tc)>>1 16 bit
            tmp1_const_8x16b = _mm_sign_epi16(consttc_8x16b, tmp2_const_8x16b);
            //mask for |delta| < 10*tc
            tmp_pq_str0_8x16b = _mm_cmpgt_epi16(tmp_pq_str0_8x16b, tmp_pq_str1_8x16b);
            //delta 1 calc starts

            //getting q32 q22 q12 q02 p32 p12 p22 p02
            tmp0_const_8x16b = _mm_setzero_si128();
            src_q2_8x16b = _mm_unpacklo_epi8(src_q2_8x16b, tmp0_const_8x16b);
            src_p2_8x16b = _mm_unpacklo_epi8(src_p2_8x16b, tmp0_const_8x16b);
            src_p2_8x16b = _mm_unpacklo_epi64(src_p2_8x16b, src_q2_8x16b);
            //constant 1
            const2_8x16b = _mm_srli_epi16(tmp2_const_8x16b, 15);
            //2*delta0
            tmp2_const_8x16b = _mm_add_epi16(tmp_delta0_8x16b, tmp_delta0_8x16b);
            //getting  all respective q's and p's together
            coefdelta_0_8x16b = _mm_load_si128((__m128i *)(shuffle1));
            tmp_delta1_8x16b = _mm_shuffle_epi8(tmp_delta1_8x16b, coefdelta_0_8x16b);
            //final adds for deltap1 and deltaq1
            tmp_delta1_8x16b = _mm_add_epi16(tmp_delta1_8x16b, const2_8x16b);
            src_p2_8x16b = _mm_add_epi16(src_p2_8x16b, tmp2_const_8x16b);
            tmp_delta1_8x16b = _mm_add_epi16(tmp_delta1_8x16b, src_p2_8x16b);
            tmp_delta1_8x16b = _mm_srai_epi16(tmp_delta1_8x16b, 2);

            //mask0= (((LWORD64)filter_flag_q)<<63)| (((LWORD64)filter_flag_p)<<31);
            tmp_pq_str1_8x16b = _mm_loadl_epi64((__m128i *)(&(maskq0)));
            src_p2_8x16b = _mm_loadl_epi64((__m128i *)(&(maskp0)));

            //   src_p2_8x16b = _mm_set_epi32(filter_flag_q,filter_flag_p,filter_flag_q,filter_flag_p);
            //mask1= (((LWORD64)(filter_flag_q&deq))<<63)|(((LWORD64)(filter_flag_p & dep))<<31);
            src_q2_8x16b = _mm_loadl_epi64((__m128i *)(&(maskq1)));
            coefdelta_0_8x16b = _mm_loadl_epi64((__m128i *)(&(maskp1)));

            src_p2_8x16b = _mm_unpacklo_epi32(src_p2_8x16b, tmp_pq_str1_8x16b);
            src_q2_8x16b = _mm_unpacklo_epi32(coefdelta_0_8x16b, src_q2_8x16b);
            //src_q2_8x16b = _mm_set_epi32(deq,dep,deq,dep);
            src_q2_8x16b = _mm_and_si128(src_q2_8x16b, src_p2_8x16b);

            //rearranging the mask values
            src_q2_8x16b = _mm_shuffle_epi32(src_q2_8x16b, 0x50);
            src_p2_8x16b = _mm_shuffle_epi32(src_p2_8x16b, 0x50);

            src_q2_8x16b = _mm_slli_epi32(src_q2_8x16b, 31);
            src_p2_8x16b = _mm_slli_epi32(src_p2_8x16b, 31);
            src_q2_8x16b = _mm_srai_epi32(src_q2_8x16b, 31);
            src_p2_8x16b = _mm_srai_epi32(src_p2_8x16b, 31);

            //combining mask delta1
            tmp_pq_str1_8x16b = _mm_and_si128(tmp_pq_str0_8x16b, src_q2_8x16b);
            // clipping delta1
            tmp_delta1_8x16b = _mm_min_epi16(tmp_delta1_8x16b, consttc_8x16b);
            //combining mask delat0
            tmp_pq_str0_8x16b = _mm_and_si128(tmp_pq_str0_8x16b, src_p2_8x16b);
            // clipping delta1
            tmp_delta1_8x16b = _mm_max_epi16(tmp_delta1_8x16b, tmp1_const_8x16b);


            //masking of the delta values |delta|<10*tc
            tmp_delta1_8x16b = _mm_and_si128(tmp_delta1_8x16b, tmp_pq_str1_8x16b);
            tmp_delta0_8x16b = _mm_and_si128(tmp_delta0_8x16b, tmp_pq_str0_8x16b);
            //separating p and q delta 0 and addinq p0 and q0
            tmp_pq_str0_8x16b = _mm_unpacklo_epi64(tmp_delta0_8x16b, tmp0_const_8x16b);
            tmp_pq_str1_8x16b = _mm_unpackhi_epi64(tmp_delta0_8x16b, tmp0_const_8x16b);
            src_p0_8x16b = _mm_unpacklo_epi8(src_p0_8x16b, tmp0_const_8x16b);
            src_q0_8x16b = _mm_unpacklo_epi8(src_q0_8x16b, tmp0_const_8x16b);
            src_p0_8x16b = _mm_add_epi16(src_p0_8x16b, tmp_pq_str0_8x16b);
            src_q0_8x16b = _mm_add_epi16(src_q0_8x16b, tmp_pq_str1_8x16b);
            //separating p and q delta 0 and addinq p0 and q0
            tmp_pq_str0_8x16b = _mm_unpacklo_epi64(tmp_delta1_8x16b, tmp0_const_8x16b);
            tmp_pq_str1_8x16b = _mm_unpackhi_epi64(tmp_delta1_8x16b, tmp0_const_8x16b);
            src_p1_8x16b = _mm_unpacklo_epi8(src_p1_8x16b, tmp0_const_8x16b);
            src_q1_8x16b = _mm_unpacklo_epi8(src_q1_8x16b, tmp0_const_8x16b);
            src_p1_8x16b = _mm_add_epi16(src_p1_8x16b, tmp_pq_str0_8x16b);
            src_q1_8x16b = _mm_add_epi16(src_q1_8x16b, tmp_pq_str1_8x16b);
            //packing p1 q1 and p0 q0 to 8 bit
            src_p1_8x16b = _mm_packus_epi16(src_p1_8x16b, src_q1_8x16b);
            src_p0_8x16b = _mm_packus_epi16(src_p0_8x16b, src_q0_8x16b);

            src_q1_8x16b = _mm_srli_si128(src_p1_8x16b, 8);
            src_q0_8x16b = _mm_srli_si128(src_p0_8x16b, 8);

            _mm_storel_epi64((__m128i *)(pu1_src - 2 * src_strd), src_p1_8x16b);
            _mm_storel_epi64((__m128i *)(pu1_src - src_strd), src_p0_8x16b);
            _mm_storel_epi64((__m128i *)(pu1_src), src_q0_8x16b);
            _mm_storel_epi64((__m128i *)(pu1_src + src_strd), src_q1_8x16b);


        }



    }

}

void ihevc_deblk_chroma_vert_ssse3(UWORD8 *pu1_src,
                                   WORD32 src_strd,
                                   WORD32 quant_param_p,
                                   WORD32 quant_param_q,
                                   WORD32 qp_offset_u,
                                   WORD32 qp_offset_v,
                                   WORD32 tc_offset_div2,
                                   WORD32 filter_flag_p,
                                   WORD32 filter_flag_q)
{
    WORD32 qp_indx_u, qp_chroma_u;
    WORD32 qp_indx_v, qp_chroma_v;
    WORD32 tc_indx_u, tc_u;
    WORD32 tc_indx_v, tc_v;

    __m128i src_row_0_16x8b, tmp_pxl_0_16x8b, src_row_2_16x8b, tmp_pxl_1_16x8b;
    ASSERT(filter_flag_p || filter_flag_q);

    /* chroma processing is done only if BS is 2             */
    /* this function is assumed to be called only if BS is 2 */
    qp_indx_u = qp_offset_u + ((quant_param_p + quant_param_q + 1) >> 1);
    qp_chroma_u = qp_indx_u < 0 ? qp_indx_u : (qp_indx_u > 57 ? qp_indx_u - 6 : gai4_ihevc_qp_table[qp_indx_u]);

    qp_indx_v = qp_offset_v + ((quant_param_p + quant_param_q + 1) >> 1);
    qp_chroma_v = qp_indx_v < 0 ? qp_indx_v : (qp_indx_v > 57 ? qp_indx_v - 6 : gai4_ihevc_qp_table[qp_indx_v]);

    tc_indx_u = CLIP3(qp_chroma_u + 2 + (tc_offset_div2 << 1), 0, 53);
    tc_u = gai4_ihevc_tc_table[tc_indx_u];

    tc_indx_v = CLIP3(qp_chroma_v + 2 + (tc_offset_div2 << 1), 0, 53);
    tc_v = gai4_ihevc_tc_table[tc_indx_v];

    if(0 == tc_u && 0 == tc_v)
    {
        return;
    }
    src_row_0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src - 4));
    tmp_pxl_0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + src_strd - 4));
    src_row_2_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 2 * src_strd - 4));
    tmp_pxl_1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + 3 * src_strd - 4));

    {
        LWORD64 mask_tc, mask_flag, mask;
        __m128i delta_vu0_16x8b, delta_vu1_16x8b;
        __m128i mask_tc_16x8, mask_16x8b, mask_flag_p_16x8b, mask_flag_q_16x8b;
        __m128i min_0_16x8b;
        __m128i const_16x8b;
        mask_flag = (((LWORD64)filter_flag_p) << 31) | (((LWORD64)filter_flag_q) << 63);
        mask_tc = (((LWORD64)tc_v) << 16) | ((LWORD64)tc_u);
        mask = 0xffff00000000ffffLL;

        src_row_0_16x8b = _mm_unpacklo_epi64(src_row_0_16x8b, tmp_pxl_0_16x8b);
        src_row_2_16x8b = _mm_unpacklo_epi64(src_row_2_16x8b, tmp_pxl_1_16x8b);

        mask_16x8b = _mm_load_si128((__m128i *)(shuffle_uv));
        // qv11 qu11 qv10 qu10 qv01 qu01 qv00 qu00 pv10 pu10 pv11 pu11 pv00 pu00 pv01 pu01
        // qv31 qu31 qv30 qu30 qv21 qu21 qv20 qu20 pv30 pu30 pv31 pu31 pv20 pu20 pv21 pu21
        delta_vu0_16x8b = _mm_shuffle_epi8(src_row_0_16x8b, mask_16x8b);
        delta_vu1_16x8b = _mm_shuffle_epi8(src_row_2_16x8b, mask_16x8b);

        tmp_pxl_0_16x8b = _mm_unpacklo_epi64(delta_vu0_16x8b, delta_vu1_16x8b);
        tmp_pxl_1_16x8b = _mm_unpackhi_epi64(delta_vu0_16x8b, delta_vu1_16x8b);
        // pv30 pv31 pu30 pu31 pv20 pv21 pu20 pu21 pv10 pv11 pu10 pu11 pv00 pv01 pu00 pu01
        // qv31 qv30 qu31 qu30 qv21 qv20 qu21 qu20 qv11 qv10 qu11 qu10 qv01 qv00 qu01 qu00
        delta_vu0_16x8b = _mm_load_si128((__m128i *)delta0);
        delta_vu1_16x8b = _mm_load_si128((__m128i *)delta1);

        delta_vu0_16x8b = _mm_maddubs_epi16(tmp_pxl_0_16x8b, delta_vu0_16x8b);
        delta_vu1_16x8b = _mm_maddubs_epi16(tmp_pxl_1_16x8b, delta_vu1_16x8b);

        //generating offset 4
        const_16x8b = _mm_cmpeq_epi16(tmp_pxl_0_16x8b, tmp_pxl_0_16x8b);
        // filter flag mask and tc mask
        mask_tc_16x8 = _mm_loadl_epi64((__m128i *)(&mask_tc));
        mask_flag_q_16x8b = _mm_loadl_epi64((__m128i *)(&mask_flag));

        mask_tc_16x8 = _mm_shuffle_epi32(mask_tc_16x8, 0x00);
        mask_flag_q_16x8b = _mm_srai_epi32(mask_flag_q_16x8b, 31);
        //-tc
        min_0_16x8b = _mm_sign_epi16(mask_tc_16x8, const_16x8b);
        //converting const 1
        const_16x8b = _mm_srli_epi16(const_16x8b, 15);

        //filterp and filterq flag
        mask_flag_p_16x8b = _mm_shuffle_epi32(mask_flag_q_16x8b, 0x00);
        mask_flag_q_16x8b = _mm_shuffle_epi32(mask_flag_q_16x8b, 0x55);

        //modified delta with a filter (1 -4 4 -1) available in 16 bit
        delta_vu0_16x8b = _mm_add_epi16(delta_vu0_16x8b, delta_vu1_16x8b);
        //converting const 4
        const_16x8b = _mm_slli_epi16(const_16x8b, 2);

        mask_16x8b = _mm_loadl_epi64((__m128i *)(&mask));
        //offset addition
        delta_vu0_16x8b = _mm_add_epi16(delta_vu0_16x8b, const_16x8b);
        //eliminating q1
        tmp_pxl_1_16x8b = _mm_slli_epi16(tmp_pxl_1_16x8b, 8);

        const_16x8b = _mm_setzero_si128();
        //filter after normalisation
        delta_vu0_16x8b = _mm_srai_epi16(delta_vu0_16x8b, 3);
        mask_16x8b = _mm_shuffle_epi32(mask_16x8b, 0x44);

        //clipping MAX
        delta_vu0_16x8b = _mm_min_epi16(delta_vu0_16x8b, mask_tc_16x8);
        //getting p0 and eliminating p1
        tmp_pxl_0_16x8b = _mm_srli_epi16(tmp_pxl_0_16x8b, 8);
        //clipping MIN
        delta_vu0_16x8b = _mm_max_epi16(delta_vu0_16x8b, min_0_16x8b);
        //getting q0
        tmp_pxl_1_16x8b = _mm_srli_epi16(tmp_pxl_1_16x8b, 8);
        //masking filter flag
        delta_vu1_16x8b = _mm_and_si128(delta_vu0_16x8b, mask_flag_q_16x8b);
        delta_vu0_16x8b = _mm_and_si128(delta_vu0_16x8b, mask_flag_p_16x8b);

        // q-delta ,p+delta
        tmp_pxl_1_16x8b = _mm_sub_epi16(tmp_pxl_1_16x8b, delta_vu1_16x8b);
        tmp_pxl_0_16x8b = _mm_add_epi16(tmp_pxl_0_16x8b, delta_vu0_16x8b);
        //merging q0 and p0 of respective rows
        delta_vu1_16x8b = _mm_unpackhi_epi32(tmp_pxl_0_16x8b, tmp_pxl_1_16x8b);
        delta_vu0_16x8b = _mm_unpacklo_epi32(tmp_pxl_0_16x8b, tmp_pxl_1_16x8b);
        // row 0 and row 1 packed , row2 and row3 packed
        delta_vu0_16x8b = _mm_packus_epi16(delta_vu0_16x8b, const_16x8b);
        delta_vu1_16x8b = _mm_packus_epi16(delta_vu1_16x8b, const_16x8b);
        //removing older pixel values
        src_row_0_16x8b = _mm_and_si128(src_row_0_16x8b, mask_16x8b);
        src_row_2_16x8b = _mm_and_si128(src_row_2_16x8b, mask_16x8b);
        //arranging modified pixels
        delta_vu0_16x8b = _mm_shuffle_epi32(delta_vu0_16x8b, 0xd8);
        delta_vu1_16x8b = _mm_shuffle_epi32(delta_vu1_16x8b, 0xd8);
        delta_vu0_16x8b = _mm_slli_epi64(delta_vu0_16x8b, 16);
        delta_vu1_16x8b = _mm_slli_epi64(delta_vu1_16x8b, 16);
        //plugging the modified values
        src_row_0_16x8b = _mm_or_si128(src_row_0_16x8b, delta_vu0_16x8b);
        src_row_2_16x8b = _mm_or_si128(src_row_2_16x8b, delta_vu1_16x8b);


        //geting values for row1 and row 3
        tmp_pxl_0_16x8b = _mm_srli_si128(src_row_0_16x8b, 8);
        tmp_pxl_1_16x8b = _mm_srli_si128(src_row_2_16x8b, 8);

        _mm_storel_epi64((__m128i *)(pu1_src - 4), src_row_0_16x8b);
        _mm_storel_epi64((__m128i *)((pu1_src - 4) + src_strd), tmp_pxl_0_16x8b);
        _mm_storel_epi64((__m128i *)((pu1_src - 4) + 2 * src_strd), src_row_2_16x8b);
        _mm_storel_epi64((__m128i *)((pu1_src - 4) + 3 * src_strd), tmp_pxl_1_16x8b);
    }



}

void ihevc_deblk_chroma_horz_ssse3(UWORD8 *pu1_src,
                                   WORD32 src_strd,
                                   WORD32 quant_param_p,
                                   WORD32 quant_param_q,
                                   WORD32 qp_offset_u,
                                   WORD32 qp_offset_v,
                                   WORD32 tc_offset_div2,
                                   WORD32 filter_flag_p,
                                   WORD32 filter_flag_q)
{
    WORD32 qp_indx_u, qp_chroma_u;
    WORD32 qp_indx_v, qp_chroma_v;
    WORD32 tc_indx_u, tc_u;
    WORD32 tc_indx_v, tc_v;


    __m128i tmp_p0_16x8b, src_p0_16x8b, src_q0_16x8b, tmp_q0_16x8b;

    ASSERT(filter_flag_p || filter_flag_q);

    /* chroma processing is done only if BS is 2             */
    /* this function is assumed to be called only if BS is 2 */
    qp_indx_u = qp_offset_u + ((quant_param_p + quant_param_q + 1) >> 1);
    qp_chroma_u = qp_indx_u < 0 ? qp_indx_u : (qp_indx_u > 57 ? qp_indx_u - 6 : gai4_ihevc_qp_table[qp_indx_u]);

    qp_indx_v = qp_offset_v + ((quant_param_p + quant_param_q + 1) >> 1);
    qp_chroma_v = qp_indx_v < 0 ? qp_indx_v : (qp_indx_v > 57 ? qp_indx_v - 6 : gai4_ihevc_qp_table[qp_indx_v]);

    tc_indx_u = CLIP3(qp_chroma_u + 2 + (tc_offset_div2 << 1), 0, 53);
    tc_u = gai4_ihevc_tc_table[tc_indx_u];

    tc_indx_v = CLIP3(qp_chroma_v + 2 + (tc_offset_div2 << 1), 0, 53);
    tc_v = gai4_ihevc_tc_table[tc_indx_v];

    if(0 == tc_u && 0 == tc_v)
    {
        return;
    }
    tmp_p0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src - 2 * src_strd));
    src_p0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src - src_strd));
    src_q0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src));
    tmp_q0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src + src_strd));

    {
        LWORD64 mask_tc, mask_flag;
        __m128i delta_vu0_16x8b, delta_vu1_16x8b;
        __m128i mask_tc_16x8, mask_16x8b, mask_flag_p_16x8b, mask_flag_q_16x8b;
        __m128i min_0_16x8b;
        __m128i const_16x8b;
        mask_flag = (((LWORD64)filter_flag_p) << 31) | (((LWORD64)filter_flag_q) << 63);
        mask_tc = (((LWORD64)tc_v) << 16) | ((LWORD64)tc_u);

        tmp_p0_16x8b = _mm_unpacklo_epi8(tmp_p0_16x8b, src_p0_16x8b);
        tmp_q0_16x8b = _mm_unpacklo_epi8(src_q0_16x8b, tmp_q0_16x8b);

        // pv30 pv31 pu30 pu31 pv20 pv21 pu20 pu21 pv10 pv11 pu10 pu11 pv00 pv01 pu00 pu01
        // qv31 qv30 qu31 qu30 qv21 qv20 qu21 qu20 qv11 qv10 qu11 qu10 qv01 qv00 qu01 qu00
        delta_vu0_16x8b = _mm_load_si128((__m128i *)delta0);
        delta_vu1_16x8b = _mm_load_si128((__m128i *)delta1);

        delta_vu0_16x8b = _mm_maddubs_epi16(tmp_p0_16x8b, delta_vu0_16x8b);
        delta_vu1_16x8b = _mm_maddubs_epi16(tmp_q0_16x8b, delta_vu1_16x8b);


        // filter flag mask and tc mask
        mask_tc_16x8 = _mm_loadl_epi64((__m128i *)(&mask_tc));
        mask_flag_q_16x8b = _mm_loadl_epi64((__m128i *)(&mask_flag));

        //generating offset 4
        const_16x8b = _mm_cmpeq_epi16(tmp_p0_16x8b, tmp_p0_16x8b);
        // filter flag mask and tc mask
        mask_tc_16x8 = _mm_shuffle_epi32(mask_tc_16x8, 0x00);
        mask_flag_q_16x8b = _mm_srai_epi32(mask_flag_q_16x8b, 31);
        //-tc
        min_0_16x8b = _mm_sign_epi16(mask_tc_16x8, const_16x8b);
        //converting const 1
        const_16x8b = _mm_srli_epi16(const_16x8b, 15);

        //filterp
        mask_flag_p_16x8b = _mm_shuffle_epi32(mask_flag_q_16x8b, 0x00);


        //converting const 4
        const_16x8b = _mm_slli_epi16(const_16x8b, 2);
        //modified delta with a filter (1 -4 4 -1) available in 16 bit
        delta_vu0_16x8b = _mm_add_epi16(delta_vu0_16x8b, delta_vu1_16x8b);

        //filterq flag
        mask_flag_q_16x8b = _mm_shuffle_epi32(mask_flag_q_16x8b, 0x55);
        //offset addition
        delta_vu0_16x8b = _mm_add_epi16(delta_vu0_16x8b, const_16x8b);
        mask_16x8b = _mm_setzero_si128();
        //filter after normalisation
        delta_vu0_16x8b = _mm_srai_epi16(delta_vu0_16x8b, 3);

        //converting p0 to 16bit
        src_p0_16x8b = _mm_unpacklo_epi8(src_p0_16x8b, mask_16x8b);
        //clipping MAX
        delta_vu0_16x8b = _mm_min_epi16(delta_vu0_16x8b, mask_tc_16x8);
        //converting q0 to 16bit
        src_q0_16x8b = _mm_unpacklo_epi8(src_q0_16x8b, mask_16x8b);
        //clipping MIN
        delta_vu0_16x8b = _mm_max_epi16(delta_vu0_16x8b, min_0_16x8b);

        //masking filter flag
        delta_vu1_16x8b = _mm_and_si128(delta_vu0_16x8b, mask_flag_q_16x8b);
        delta_vu0_16x8b = _mm_and_si128(delta_vu0_16x8b, mask_flag_p_16x8b);

        // q-delta ,p+delta
        src_q0_16x8b = _mm_sub_epi16(src_q0_16x8b, delta_vu1_16x8b);
        src_p0_16x8b = _mm_add_epi16(src_p0_16x8b, delta_vu0_16x8b);

        // p0 and q0 packed
        src_q0_16x8b = _mm_packus_epi16(src_q0_16x8b, mask_16x8b);
        src_p0_16x8b = _mm_packus_epi16(src_p0_16x8b, mask_16x8b);



        _mm_storel_epi64((__m128i *)(pu1_src - src_strd), src_p0_16x8b);
        _mm_storel_epi64((__m128i *)(pu1_src), src_q0_16x8b);

    }


}
