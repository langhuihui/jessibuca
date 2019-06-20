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
 *  ihevc_32x32_itrans_recon_x86_intr.c
 *
 * @brief
 *  Contains function definitions for inverse  quantization, inverse
 * transform and reconstruction
 *
 * @author
 *  100470
 *
 * @par List of Functions:
 *  - ihevc_itrans_recon_32x32_sse42()
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include "ihevc_typedefs.h"
#include "ihevc_platform_macros.h"
#include "ihevc_macros.h"
#include "ihevc_defs.h"
#include "ihevc_trans_tables.h"
#include "ihevc_iquant_itrans_recon.h"
#include "ihevc_func_selector.h"
#include "ihevc_trans_macros.h"

#include <emmintrin.h>
#include <smmintrin.h>
#include <tmmintrin.h>

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs inverse quantization, inverse  transform and
 * reconstruction for 16x16 input block
 *
 * @par Description:
 *  Performs inverse quantization , inverse transform  and adds the
 * prediction data and clips output to 8 bit
 *
 * @param[in] pi2_src
 *  Input 16x16 coefficients
 *
 * @param[in] pi2_tmp
 *  Temporary 16x16 buffer for storing inverse
 *  transform 1st stage output
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
/**/

void ihevc_itrans_recon_32x32_sse42(WORD16 *pi2_src,
                                    WORD16 *pi2_tmp,
                                    UWORD8 *pu1_pred,
                                    UWORD8 *pu1_dst,
                                    WORD32 src_strd,
                                    WORD32 pred_strd,
                                    WORD32 dst_strd,
                                    WORD32 zero_cols,
                                    WORD32 zero_rows)
{
    /* Inverse Transform */

    WORD32 j;


    WORD16 *pi2_tmp_orig;


    WORD16 *o_temp_ptr;
    WORD16 *temp_ptr;

    __m128i m_temp_reg_0;
    __m128i m_temp_reg_1;
    __m128i m_temp_reg_2;
    __m128i m_temp_reg_3;
    __m128i m_temp_reg_4;
    __m128i m_temp_reg_5;
    __m128i m_temp_reg_6;
    __m128i m_temp_reg_7;
    __m128i m_temp_reg_10;
    __m128i m_temp_reg_11;
    __m128i m_temp_reg_12;
    __m128i m_temp_reg_13;
    __m128i m_temp_reg_14;
    __m128i m_temp_reg_15;
    __m128i m_temp_reg_16;
    __m128i m_temp_reg_17;
    __m128i m_temp_reg_18;
    __m128i m_temp_reg_19;
    __m128i m_temp_reg_20;
    __m128i m_temp_reg_21;
    __m128i m_temp_reg_22;
    __m128i m_temp_reg_23;
    __m128i m_temp_reg_30;
    __m128i m_temp_reg_31;
    __m128i m_temp_reg_32;
    __m128i m_temp_reg_33;
    __m128i m_temp_reg_34;
    __m128i m_temp_reg_35;
    __m128i m_temp_reg_36;
    __m128i m_temp_reg_37;
    __m128i m_temp_reg_40;
    __m128i m_temp_reg_41;
    __m128i m_temp_reg_42;
    __m128i m_temp_reg_43;
    __m128i m_temp_reg_44;
    __m128i m_temp_reg_45;
    __m128i m_temp_reg_46;
    __m128i m_temp_reg_47;

    __m128i m_temp_reg_70;
    __m128i m_temp_reg_71;
    __m128i m_temp_reg_72;
    __m128i m_temp_reg_73;
    __m128i m_temp_reg_74;
    __m128i m_temp_reg_75;
    __m128i m_temp_reg_76;
    __m128i m_temp_reg_77;

    __m128i m_temp_reg_80;
    __m128i m_temp_reg_81;
    __m128i m_temp_reg_82;
    __m128i m_temp_reg_83;
    __m128i m_temp_reg_84;
    __m128i m_temp_reg_85;
    __m128i m_temp_reg_86;
    __m128i m_temp_reg_87;

    __m128i m_temp_reg_90;
    __m128i m_temp_reg_91;
    __m128i m_temp_reg_92;
    __m128i m_temp_reg_93;
    __m128i m_temp_reg_94;
    __m128i m_temp_reg_95;
    __m128i m_temp_reg_96;
    __m128i m_temp_reg_97;

    __m128i m_rdng_factor;
    __m128i m_count;
    __m128i m_coeff1, m_coeff2, m_coeff3, m_coeff4;
    __m128i m_coeff5, m_coeff6, m_coeff7, m_coeff8;

    __m128i temp1, temp2, temp3, temp4;
    __m128i temp5, temp6, temp7, temp8;

    __m128i all_zero_reg;
    WORD32 i;

    /*Lokesh*/
    WORD32  zero_last24_cols_stg1;
    WORD32  zero_last24_rows_stg1;
    WORD32  zero_last28_rows_stg1;

    WORD32  zero_last28_rows_stg2;
    WORD32  zero_last24_rows_stg2;

    WORD32  trans_size_stg1;

    WORD32 i4_shift = IT_SHIFT_STAGE_1;
    WORD32 trans_size = TRANS_SIZE_32;


    /* Last 8 cols of 16x16 block are skipped based on the below flag : Lokesh */
    zero_last24_cols_stg1 = ((zero_cols & 0xFFFFFF00) == 0xFFFFFF00) ? 1 : 0;
    zero_last24_rows_stg1 = ((zero_rows & 0xFFFFFF00) == 0xFFFFFF00) ? 1 : 0;
    zero_last28_rows_stg1 = ((zero_rows & 0xFFFFFFF0) == 0xFFFFFFF0) ? 1 : 0;

    zero_last28_rows_stg2 = ((zero_cols & 0xFFFFFFF0) == 0xFFFFFFF0) ? 1 : 0;
    zero_last24_rows_stg2 = zero_last24_cols_stg1;

    if((zero_last28_rows_stg2) || (zero_last24_cols_stg1))
    {
        trans_size_stg1 = 8;

    }
    else
    {
        trans_size_stg1 = 32;
    }

    all_zero_reg = _mm_setzero_si128();

    o_temp_ptr  = pi2_tmp;
    temp_ptr = (pi2_tmp + 1024);

    pi2_tmp += 2048;
    pi2_tmp_orig = pi2_tmp;

    for(i = 0; i < trans_size_stg1; i += 8)
    {

        {
            WORD16 *pi2_tmp_src = pi2_src;

            m_temp_reg_70 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_71 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_72 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_73 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_74 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_75 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_76 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_77 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);

            m_temp_reg_80 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_81 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_82 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_83 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_84 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_85 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_86 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_87 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
        }

        if(zero_last28_rows_stg1)
        {
            /* eeo */
            /* eeeo[0] stored in m_temp_reg_20 and m_temp_reg_21 */
            /* eeeo[1] stored in m_temp_reg_22 and m_temp_reg_23 */
            {
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[2][0]); //64

                m_temp_reg_1 = _mm_unpacklo_epi16(m_temp_reg_70, all_zero_reg);

                m_temp_reg_14 = _mm_madd_epi16(m_temp_reg_1, m_coeff3);

/* eeeo[0]= m_temp_reg_20  */
/* eeeo[1]= m_temp_reg_21  */
/* eeee[0]= m_temp_reg_22  */
/* eeee[1]= m_temp_reg_23  */

                /* eee[0] = eeee[0] + eeeo[0]; */
                m_temp_reg_40 = m_temp_reg_14;

                /* eee[3] = eeee[0] - eeeo[0]; */
                m_temp_reg_43 = m_temp_reg_14;

                /* eee[2] = eeee[1] - eeeo[1]; */
                m_temp_reg_42 = m_temp_reg_14; //m_temp_reg_16;

                /* eee[1] = eeee[1] + eeeo[1];*/
                m_temp_reg_41 = m_temp_reg_14; //m_temp_reg_16;

                m_temp_reg_70 = _mm_srli_si128(m_temp_reg_70, 8);

                m_temp_reg_1 = _mm_unpacklo_epi16(m_temp_reg_70, all_zero_reg);

                m_temp_reg_14 = _mm_madd_epi16(m_temp_reg_1, m_coeff3);

/* eeeo[0]= m_temp_reg_20  */
/* eeeo[1]= m_temp_reg_21  */
/* eeee[0]= m_temp_reg_22  */
/* eeee[1]= m_temp_reg_23  */

                /* eee[0] = eeee[0] + eeeo[0]; */
                m_temp_reg_44 = m_temp_reg_14;

                /* eee[3] = eeee[0] - eeeo[0]; */
                m_temp_reg_47 = m_temp_reg_14;

                /* eee[2] = eeee[1] - eeeo[1]; */
                m_temp_reg_46 = m_temp_reg_14; //m_temp_reg_16;

                /* eee[1] = eeee[1] + eeeo[1];*/
                m_temp_reg_45 = m_temp_reg_14; //m_temp_reg_16;


            }
            /* eo */
            {
                WORD16 *pi2_scratch = o_temp_ptr;

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[0][0]); //90
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[4][0]); //87
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[6][0]); //80
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[7][0]); //70
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[2][0]); //57
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[19][0]); //43
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[3][0]); //25
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[5][0]); //9

                m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_71, all_zero_reg);

                m_temp_reg_71 = _mm_srli_si128(m_temp_reg_71, 8);

                /* eo0[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_14 = _mm_unpacklo_epi16(m_temp_reg_71, all_zero_reg);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_40, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /* eo0[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_44, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_44, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }
                /* eo1[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff2);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_41, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_41, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /* eo1[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff2);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_45, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_45, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /* eo2[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff3);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_42, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /* eo2[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff3);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_46, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_46, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /**************************************************************************/


                /* eo3[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff4);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_43, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_43, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /* eo3[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff4);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_47, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_47, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo4[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff5);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_43, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_43, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }
                /* eo4[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_47, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_47, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /***********************************************************************/

                /* eo5[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff6);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_42, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo5[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff6);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_46, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_46, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /* eo6[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff7);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_41, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_41, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo6[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff7);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_45, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_45, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo7[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff8);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_40, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo7[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff8);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_44, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_44, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

            }
        }
        else if(zero_last24_rows_stg1)
        {
            {
                /* eeo */
                /* eeeo[0] stored in m_temp_reg_20 and m_temp_reg_21 */
                /* eeeo[1] stored in m_temp_reg_22 and m_temp_reg_23 */

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[0][0]); //83 36
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[1][0]); //36 -83

                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[2][0]); //64 64

                m_temp_reg_1 = _mm_unpacklo_epi16(m_temp_reg_70, all_zero_reg);

                m_temp_reg_14 = _mm_madd_epi16(m_temp_reg_1, m_coeff3);

                /* eeeo[0]= m_temp_reg_20  */
                /* eeeo[1]= m_temp_reg_21  */
                /* eeee[0]= m_temp_reg_22  */
                /* eeee[1]= m_temp_reg_23  */

                /* eee[0] = eeee[0] + eeeo[0]; */
                m_temp_reg_40 = m_temp_reg_14;

                /* eee[3] = eeee[0] - eeeo[0]; */
                m_temp_reg_43 = m_temp_reg_14;

                /* eee[2] = eeee[1] - eeeo[1]; */
                m_temp_reg_42 = m_temp_reg_14; //m_temp_reg_16;

                /* eee[1] = eeee[1] + eeeo[1];*/
                m_temp_reg_41 = m_temp_reg_14; //m_temp_reg_16;

                /* for row 4 to 7 */

                m_temp_reg_70 = _mm_srli_si128(m_temp_reg_70, 8);

                m_temp_reg_1 = _mm_unpacklo_epi16(m_temp_reg_70, all_zero_reg);

                m_temp_reg_14 = _mm_madd_epi16(m_temp_reg_1, m_coeff3);

                /* eeeo[0]= m_temp_reg_20  */
                /* eeeo[1]= m_temp_reg_21  */
                /* eeee[0]= m_temp_reg_22  */
                /* eeee[1]= m_temp_reg_23  */

                /* eee[0] = eeee[0] + eeeo[0]; */
                m_temp_reg_44 = m_temp_reg_14;

                /* eee[3] = eeee[0] - eeeo[0]; */
                m_temp_reg_47 = m_temp_reg_14;

                /* eee[2] = eeee[1] - eeeo[1]; */
                m_temp_reg_46 = m_temp_reg_14; //m_temp_reg_16;

                /* eee[1] = eeee[1] + eeeo[1];*/
                m_temp_reg_45 = m_temp_reg_14; //m_temp_reg_16;


                // eeo[]
                /* for(k = 0; k < 4; k++) */

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[4][0]); //89 75
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[6][0]); //75
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[9][0]); //18
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[5][0]); //50 18

                m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_72, all_zero_reg);

                m_temp_reg_72 = _mm_srli_si128(m_temp_reg_72, 8);

                m_temp_reg_14 = _mm_unpacklo_epi16(m_temp_reg_72, all_zero_reg);

                m_temp_reg_33 = _mm_setzero_si128();

                /* eeo */
                {
                    /* eeo0[0-3] */
                    {
                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_40, m_temp_reg_30);

                        m_temp_reg_90 = m_temp_reg_34;
                        m_temp_reg_97 = m_temp_reg_35;
                    }
                    /* eeo0[4-7] */
                    {
                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);

                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_44, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_44, m_temp_reg_30);

                        m_temp_reg_91 = m_temp_reg_34;
                        m_temp_reg_96 = m_temp_reg_35;

                    }

                    /* eeo1[0-3] */
                    {
                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff2);

                        /* e[1][0-3] stored in pi2_tmp[2][0-7] */
                        /* e[6][0-3] stored in pi2_tmp[2][8-15] */
                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_41, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_41, m_temp_reg_30);

                        m_temp_reg_92 = m_temp_reg_34;
                        m_temp_reg_95 = m_temp_reg_35;

                    }

                    /* eo1[4-7] */
                    {
                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff2);

                        /* e[1][4-7] stored in pi2_tmp[3][0-7] */
                        /* e[6][4-7] stored in pi2_tmp[3][8-15] */
                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_45, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_45, m_temp_reg_30);

                        m_temp_reg_93 = m_temp_reg_34;
                        m_temp_reg_94 = m_temp_reg_35;


                    }

                    /* eo2[0-3] */
                    {
                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff4);

                        /* e[2][0-3] stored in pi2_tmp[4][0-7] */
                        /* e[5][0-3] stored in pi2_tmp[4][8-15] */
                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_42, m_temp_reg_30);

                        temp1 = m_temp_reg_34;
                        temp7 = m_temp_reg_35;

                    }

                    /* eo2[4-7] */
                    {
                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff4);

                        /* e[2][4-7] stored in pi2_tmp[5][0-7] */
                        /* e[5][4-7] stored in pi2_tmp[5][8-15] */
                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_46, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_46, m_temp_reg_30);

                        temp2 = m_temp_reg_34;
                        temp6 = m_temp_reg_35;

                    }

                    /* eo3[0-3] */
                    {
                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff3);

                        /* e[3][0-3] stored in pi2_tmp[6][0-7] */
                        /* e[4][0-3] stored in pi2_tmp[6][8-15] */
                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_43, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_43, m_temp_reg_30);

                        temp3 = m_temp_reg_34;
                        temp5 = m_temp_reg_35;

                    }


                    /* eo3[4-7] */
                    {
                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff3);

                        /* e[3][4-7] stored in pi2_tmp[7][0-7] */
                        /* e[4][4-7] stored in pi2_tmp[7][8-15] */
                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_47, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_47, m_temp_reg_30);

                        temp4 = m_temp_reg_34;
                        temp8 = m_temp_reg_35;


                    }
                    /* All values of ee[] array in pi2_temp */

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[0][0]); //90 87
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[1][0]); //80 70
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[2][0]); //57 43
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[3][0]); //25 9

                    m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_71, m_temp_reg_73);

                    m_temp_reg_71 = _mm_srli_si128(m_temp_reg_71, 8);
                    m_temp_reg_73 = _mm_srli_si128(m_temp_reg_73, 8);

                }
            }
            /* eo */
            {

                WORD16 *pi2_scratch = o_temp_ptr;

                /* eo0[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_90, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_90, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo0[4-7] */
                {
                    m_temp_reg_14 = _mm_unpacklo_epi16(m_temp_reg_71, m_temp_reg_73);

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_91, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_91, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[4][0]); //87  57

                /* eo1[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_92, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_92, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo1[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_93, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_93, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[8][0]); //80  9

                /* eo2[0-3] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(temp1, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp1, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /* eo2[4-7] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(temp2, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp2, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /**************************************************************************/



                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[12][0]); //70  -43

                /* eo3[0-3] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(temp3, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp3, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo3[4-7] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(temp4, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp4, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[16][0]); //57  -80

                /* eo4[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(temp5, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp5, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }
                /* eo4[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(temp8, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp8, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /***********************************************************************/

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[20][0]); //43  -90

                /* eo5[0-3] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(temp7, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp7, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo5[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(temp6, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp6, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[24][0]); //25  -70

                /* eo6[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_95, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_95, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo6[4-7] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_94, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_94, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[28][0]); //9  -25

                /* eo7[0-3] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_97, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_97, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo7[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_96, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_96, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

            }

        }
        else
        {

            {
                /* eeo */
                /* eeeo[0] stored in m_temp_reg_20 and m_temp_reg_21 */
                /* eeeo[1] stored in m_temp_reg_22 and m_temp_reg_23 */

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[0][0]); //83 36
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[1][0]); //36 -83

                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[2][0]); //64 64
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[3][0]); //64 -64

                m_temp_reg_0 = _mm_unpacklo_epi16(m_temp_reg_74, m_temp_reg_84);

                m_temp_reg_1 = _mm_unpacklo_epi16(m_temp_reg_70, m_temp_reg_80);

                m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_0, m_coeff1);  /* eeeo[0] */
                m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_0, m_coeff2);  /* eeeo[1] */

                m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_1, m_coeff3);  /* eeee[0] */
                m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_1, m_coeff4);  /* eeee[1] */


                /* eeeo[0]= m_temp_reg_20  */
                /* eeeo[1]= m_temp_reg_21  */
                /* eeee[0]= m_temp_reg_22  */
                /* eeee[1]= m_temp_reg_23  */

                /* eee[0] = eeee[0] + eeeo[0]; */
                m_temp_reg_40 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);  /* eeeo[0] */

                /* eee[3] = eeee[0] - eeeo[0]; */
                m_temp_reg_43 = _mm_sub_epi32(m_temp_reg_21, m_temp_reg_20);  /* eeeo[1] */

                /* eee[2] = eeee[1] - eeeo[1]; */
                m_temp_reg_42 = _mm_sub_epi32(m_temp_reg_23, m_temp_reg_22);  /* eeee[1] */

                /* eee[1] = eeee[1] + eeeo[1];*/
                m_temp_reg_41 = _mm_add_epi32(m_temp_reg_23, m_temp_reg_22);  /* eeee[0] */

                /* for row 4 to 7 */

                m_temp_reg_74 = _mm_srli_si128(m_temp_reg_74, 8);
                m_temp_reg_84 = _mm_srli_si128(m_temp_reg_84, 8);

                /* Interleaving row 8 and row 24*/
                m_temp_reg_0 = _mm_unpacklo_epi16(m_temp_reg_74, m_temp_reg_84);

                m_temp_reg_70 = _mm_srli_si128(m_temp_reg_70, 8);
                m_temp_reg_80 = _mm_srli_si128(m_temp_reg_80, 8);

                m_temp_reg_1 = _mm_unpacklo_epi16(m_temp_reg_70, m_temp_reg_80);

                m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_0, m_coeff1);  /* eeeo[0] */
                m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_0, m_coeff2);  /* eeeo[1] */

                m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_1, m_coeff3);  /* eeee[0] */
                m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_1, m_coeff4);  /* eeee[1] */


                /* eeeo[0]= m_temp_reg_20  */
                /* eeeo[1]= m_temp_reg_21  */
                /* eeee[0]= m_temp_reg_22  */
                /* eeee[1]= m_temp_reg_23  */

                /* eee[0] = eeee[0] + eeeo[0]; */
                m_temp_reg_44 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);  /* eeeo[0] */

                /* eee[3] = eeee[0] - eeeo[0]; */
                m_temp_reg_47 = _mm_sub_epi32(m_temp_reg_21, m_temp_reg_20);  /* eeeo[1] */

                /* eee[2] = eeee[1] - eeeo[1]; */
                m_temp_reg_46 = _mm_sub_epi32(m_temp_reg_23, m_temp_reg_22);  /* eeee[1] */

                /* eee[1] = eeee[1] + eeeo[1];*/
                m_temp_reg_45 = _mm_add_epi32(m_temp_reg_23, m_temp_reg_22);  /* eeee[0] */


                // eeo[]
                /* for(k = 0; k < 4; k++) */

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[4][0]); //89 75
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[5][0]); //50 18

                /* eeo */
                {
                    /* eeo0[0-3] */
                    {
                        m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_72, m_temp_reg_76);
                        m_temp_reg_11 = _mm_unpacklo_epi16(m_temp_reg_82, m_temp_reg_86);

                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                        m_temp_reg_90 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_30);
                        m_temp_reg_97 = _mm_sub_epi32(m_temp_reg_40, m_temp_reg_30);

                    }

                    m_temp_reg_72 = _mm_srli_si128(m_temp_reg_72, 8);
                    m_temp_reg_76 = _mm_srli_si128(m_temp_reg_76, 8);
                    m_temp_reg_82 = _mm_srli_si128(m_temp_reg_82, 8);
                    m_temp_reg_86 = _mm_srli_si128(m_temp_reg_86, 8);

                    /* eeo0[4-7] */
                    {
                        m_temp_reg_14 = _mm_unpacklo_epi16(m_temp_reg_72, m_temp_reg_76);
                        m_temp_reg_15 = _mm_unpacklo_epi16(m_temp_reg_82, m_temp_reg_86);

                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);
                        m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff2);

                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                        m_temp_reg_91 = _mm_add_epi32(m_temp_reg_44, m_temp_reg_30);
                        m_temp_reg_96 = _mm_sub_epi32(m_temp_reg_44, m_temp_reg_30);

                    }


                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[6][0]); //75 -18
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[7][0]); //89  50

                    /* eeo1[0-3] */
                    {
                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff3);
                        m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff4);

                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_41, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_41, m_temp_reg_30);

                        m_temp_reg_92 = _mm_sub_epi32(m_temp_reg_34, m_temp_reg_31);
                        m_temp_reg_95 = _mm_add_epi32(m_temp_reg_35, m_temp_reg_31);

                    }

                    /* eeo1[4-7] */
                    {

                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff3);
                        m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff4);

                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_45, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_45, m_temp_reg_30);

                        m_temp_reg_93 = _mm_sub_epi32(m_temp_reg_34, m_temp_reg_31);
                        m_temp_reg_94 = _mm_add_epi32(m_temp_reg_35, m_temp_reg_31);


                    }

                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[8][0]); //50 -89
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[9][0]); //18  75

                    /* eeo2[0-3] */
                    {

                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff3);
                        m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff4);

                        /* e[2][0-3] stored in pi2_tmp[4][0-7] */
                        /* e[5][0-3] stored in pi2_tmp[4][8-15] */

                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_42, m_temp_reg_30);

                        temp1 = _mm_add_epi32(m_temp_reg_34, m_temp_reg_31);
                        temp7 = _mm_sub_epi32(m_temp_reg_35, m_temp_reg_31);

                    }

                    /* eeo2[4-7] */
                    {

                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff3);
                        m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff4);

                        /* e[2][4-7] stored in pi2_tmp[5][0-7] */
                        /* e[5][4-7] stored in pi2_tmp[5][8-15] */

                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_46, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_46, m_temp_reg_30);

                        temp2 = _mm_add_epi32(m_temp_reg_34, m_temp_reg_31);
                        temp6 = _mm_sub_epi32(m_temp_reg_35, m_temp_reg_31);

                    }

                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[10][0]); //18 -50
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[11][0]); //75  -89

                    /* eeo3[0-3] */
                    {

                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff3);
                        m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff4);

                        /* e[3][0-3] stored in pi2_tmp[6][0-7] */
                        /* e[4][0-3] stored in pi2_tmp[6][8-15] */

                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_43, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_43, m_temp_reg_30);

                        temp3 = _mm_add_epi32(m_temp_reg_34, m_temp_reg_31);
                        temp5 = _mm_sub_epi32(m_temp_reg_35, m_temp_reg_31);


                    }

                    /* eeo3[4-7] */
                    {

                        m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff3);
                        m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff4);

                        /* e[3][4-7] stored in pi2_tmp[7][0-7] */
                        /* e[4][4-7] stored in pi2_tmp[7][8-15] */

                        m_temp_reg_34 = _mm_add_epi32(m_temp_reg_47, m_temp_reg_30);
                        m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_47, m_temp_reg_30);
                        temp4 = _mm_add_epi32(m_temp_reg_34, m_temp_reg_31);
                        temp8 = _mm_sub_epi32(m_temp_reg_35, m_temp_reg_31);

                    }


                    /* All values of ee[] array in pi2_temp */

                    /* for(k = 0; k < 8; k++) */
                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[0][0]); //90 87
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[1][0]); //80 70
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[2][0]); //57 43
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[3][0]); //25 9
                }
            }
            /* eo */
            {

                WORD16 *pi2_scratch = o_temp_ptr;

                m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_71, m_temp_reg_73);
                m_temp_reg_11 = _mm_unpacklo_epi16(m_temp_reg_75, m_temp_reg_77);
                m_temp_reg_12 = _mm_unpacklo_epi16(m_temp_reg_81, m_temp_reg_83);
                m_temp_reg_13 = _mm_unpacklo_epi16(m_temp_reg_85, m_temp_reg_87);

                m_temp_reg_71 = _mm_srli_si128(m_temp_reg_71, 8);
                m_temp_reg_73 = _mm_srli_si128(m_temp_reg_73, 8);
                m_temp_reg_75 = _mm_srli_si128(m_temp_reg_75, 8);
                m_temp_reg_77 = _mm_srli_si128(m_temp_reg_77, 8);

                m_temp_reg_81 = _mm_srli_si128(m_temp_reg_81, 8);
                m_temp_reg_83 = _mm_srli_si128(m_temp_reg_83, 8);
                m_temp_reg_85 = _mm_srli_si128(m_temp_reg_85, 8);
                m_temp_reg_87 = _mm_srli_si128(m_temp_reg_87, 8);

                /* eo0[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_90, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_90, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }
                /* eo0[4-7] */
                {
                    m_temp_reg_14 = _mm_unpacklo_epi16(m_temp_reg_71, m_temp_reg_73);
                    m_temp_reg_15 = _mm_unpacklo_epi16(m_temp_reg_75, m_temp_reg_77);
                    m_temp_reg_16 = _mm_unpacklo_epi16(m_temp_reg_81, m_temp_reg_83);
                    m_temp_reg_17 = _mm_unpacklo_epi16(m_temp_reg_85, m_temp_reg_87);

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_16, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_17, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_91, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_91, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[4][0]); //87  57
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[5][0]); //0  -43
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[6][0]); //80  90
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[7][0]); //70  25

                /* eo1[0-3] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_92, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_92, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /* eo1[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_16, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_17, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_93, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_93, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[8][0]); //80  9
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[9][0]); //70  87
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[10][0]); //-25  57
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[11][0]); //90  43

                /* eo2[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(temp1, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp1, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo2[4-7] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff2);

                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_16, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_17, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(temp2, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp2, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }
                /**************************************************************************/

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[12][0]); //70  -43
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[13][0]); //-87  9
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[14][0]); //90  25
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[15][0]); //80  57

                /* eo3[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_sub_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(temp3, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp3, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo3[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_16, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_17, m_coeff4);

                    m_temp_reg_32 = _mm_sub_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(temp4, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp4, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[16][0]); //57  -80
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[17][0]); //-25  90
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[18][0]); //9  87
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[19][0]); //43  70

                /* eo4[0-3] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_sub_epi32(m_temp_reg_33, m_temp_reg_32);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(temp5, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp5, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo4[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_16, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_17, m_coeff4);

                    m_temp_reg_32 = _mm_sub_epi32(m_temp_reg_33, m_temp_reg_32);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(temp8, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp8, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                /***********************************************************************/

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[20][0]); //43  -90
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[21][0]); //57  25
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[22][0]); //-87  70
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[23][0]); //9  -80

                /* eo5[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(temp7, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp7, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo5[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_16, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_17, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(temp6, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(temp6, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[24][0]); //25  -70
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[25][0]); //90  -80
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[26][0]); //43  9
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[27][0]); //-57  87

                /* eo6[0-3] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_95, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_95, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo6[4-7] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_16, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_17, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_94, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_94, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[28][0]); //9  -25
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[29][0]); //43  -57
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[30][0]); //70  -80
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[31][0]); //87  -90

                /* eo7[0-3] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_97, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_97, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }


                /* eo7[4-7] */
                {

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_14, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_15, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_16, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_17, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                    m_temp_reg_34 = _mm_add_epi32(m_temp_reg_96, m_temp_reg_30);
                    m_temp_reg_35 = _mm_sub_epi32(m_temp_reg_96, m_temp_reg_30);

                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_34);
                    pi2_scratch += 8;
                    _mm_storeu_si128((__m128i *)pi2_scratch, m_temp_reg_35);
                    pi2_scratch += 8;

                }

            }

        }
        /*  All e[] are done */
        /****************************/

        {

            WORD16 *pi2_tmp_src = pi2_src + src_strd;

            m_temp_reg_70 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_71 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_72 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_73 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_74 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_75 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_76 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_77 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);

            m_temp_reg_80 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_81 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_82 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_83 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_84 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_85 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_86 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
            pi2_tmp_src += (src_strd << 1);
            m_temp_reg_87 = _mm_loadu_si128((__m128i *)pi2_tmp_src);
        }

        if(zero_last28_rows_stg1)
        {
            /* o & stage 1 out */
            {
                WORD32 j;
                WORD16 *pi2_src_scratch = o_temp_ptr;
                WORD16 *pi2_dst_scratch = temp_ptr;
                WORD32 out_stride = (trans_size << 1);
                WORD32 in_stride = trans_size;

                for(j = 0; j < 2; j++)
                {
                    if(j)
                    {
                        m_temp_reg_70 = _mm_srli_si128(m_temp_reg_70, 8);
                        m_temp_reg_71 = _mm_srli_si128(m_temp_reg_71, 8);
                    }

                    m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_70, m_temp_reg_71);

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[0][0]);

                    /* o0[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[8][0]);

                    /* o1[0-3] */
                    {

                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[16][0]);

                    /* o2[0-3] */
                    {

                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[24][0]);

                    /* o3[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[32][0]);

                    /* o4[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[40][0]);

                    /* o5[0-3] */
                    {

                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[48][0]);

                    /* o6[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[56][0]);

                    /* o7[0-3] */
                    {

                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += 8;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += 8;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[64][0]);

                    /* o8[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[72][0]);

                    /* o9[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[80][0]);

                    /* o10[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[88][0]);

                    /* o11[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[96][0]);

                    /* o12[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[104][0]);

                    /* o13[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[112][0]);

                    /* o14[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[120][0]);

                    /* o15[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += 8;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += 8;
                    }

                }
            }
        }
        else if(zero_last24_rows_stg1)
        {
            /* o & stage 1 out */
            {
                WORD32 j;

                WORD16 *pi2_src_scratch = o_temp_ptr;
                WORD16 *pi2_dst_scratch = temp_ptr;
                WORD32 out_stride = (trans_size << 1);

                WORD32 in_stride = trans_size;

                for(j = 0; j < 2; j++)
                {
                    if(j)
                    {
                        m_temp_reg_70 = _mm_srli_si128(m_temp_reg_70, 8);
                        m_temp_reg_71 = _mm_srli_si128(m_temp_reg_71, 8);
                        m_temp_reg_72 = _mm_srli_si128(m_temp_reg_72, 8);
                        m_temp_reg_73 = _mm_srli_si128(m_temp_reg_73, 8);
                    }

                    m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_70, m_temp_reg_71); //row 1 and row 3 interleaved
                    m_temp_reg_11 = _mm_unpacklo_epi16(m_temp_reg_72, m_temp_reg_73); //row 5 and row 7 interleaved

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[0][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[1][0]);

                    /* o0[0-3] */
                    {

                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[8][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[9][0]);

                    /* o1[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[16][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[17][0]);

                    /* o2[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_sub_epi32(m_temp_reg_21, m_temp_reg_20);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[24][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[25][0]);

                    /* o3[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_sub_epi32(m_temp_reg_21, m_temp_reg_20);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[32][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[33][0]);

                    /* o4[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[40][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[41][0]);

                    /* o5[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[48][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[49][0]);

                    /* o6[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[56][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[57][0]);

                    /* o7[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += 8;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += 8;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[64][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[65][0]);

                    /* o8[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[72][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[73][0]);

                    /* o9[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[80][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[81][0]);

                    /* o10[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[88][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[89][0]);

                    /* o11[0-3] */
                    {

                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[96][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[97][0]);

                    /* o12[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[104][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[105][0]);

                    /* o13[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[112][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[113][0]);

                    /* o14[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[120][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[121][0]);

                    /* o15[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += 8;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += 8;
                    }

                }
            }
        }
        else
        {
            /* o & stage 1 out */
            {
                WORD32 j;

                WORD16 *pi2_src_scratch = o_temp_ptr;
                WORD16 *pi2_dst_scratch = temp_ptr;
                WORD32 out_stride = (trans_size << 1);

                WORD32 in_stride = trans_size;


                for(j = 0; j < 2; j++)
                {
                    if(j)
                    {
                        m_temp_reg_70 = _mm_srli_si128(m_temp_reg_70, 8);
                        m_temp_reg_71 = _mm_srli_si128(m_temp_reg_71, 8);
                        m_temp_reg_72 = _mm_srli_si128(m_temp_reg_72, 8);
                        m_temp_reg_73 = _mm_srli_si128(m_temp_reg_73, 8);
                        m_temp_reg_74 = _mm_srli_si128(m_temp_reg_74, 8);
                        m_temp_reg_75 = _mm_srli_si128(m_temp_reg_75, 8);
                        m_temp_reg_76 = _mm_srli_si128(m_temp_reg_76, 8);
                        m_temp_reg_77 = _mm_srli_si128(m_temp_reg_77, 8);

                        m_temp_reg_80 = _mm_srli_si128(m_temp_reg_80, 8);
                        m_temp_reg_81 = _mm_srli_si128(m_temp_reg_81, 8);
                        m_temp_reg_82 = _mm_srli_si128(m_temp_reg_82, 8);
                        m_temp_reg_83 = _mm_srli_si128(m_temp_reg_83, 8);
                        m_temp_reg_84 = _mm_srli_si128(m_temp_reg_84, 8);
                        m_temp_reg_85 = _mm_srli_si128(m_temp_reg_85, 8);
                        m_temp_reg_86 = _mm_srli_si128(m_temp_reg_86, 8);
                        m_temp_reg_87 = _mm_srli_si128(m_temp_reg_87, 8);
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[0][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[1][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[2][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[3][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[4][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[5][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[6][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[7][0]);

                    m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_70, m_temp_reg_71); //row 1 and row 3 interleaved
                    m_temp_reg_11 = _mm_unpacklo_epi16(m_temp_reg_72, m_temp_reg_73); //row 5 and row 7 interleaved
                    m_temp_reg_12 = _mm_unpacklo_epi16(m_temp_reg_74, m_temp_reg_75); //row 9 and row 11 interleaved
                    m_temp_reg_13 = _mm_unpacklo_epi16(m_temp_reg_76, m_temp_reg_77); //row 13 and row 15 interleaved
                    temp1 = _mm_unpacklo_epi16(m_temp_reg_80, m_temp_reg_81); //row 17 and row 19 interleaved
                    temp2 = _mm_unpacklo_epi16(m_temp_reg_82, m_temp_reg_83); //row 21 and row 23 interleaved
                    temp3 = _mm_unpacklo_epi16(m_temp_reg_84, m_temp_reg_85); //row 25 and row 27 interleaved
                    temp4 = _mm_unpacklo_epi16(m_temp_reg_86, m_temp_reg_87); //row 29 and row 31 interleaved


                    /* o0[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[8][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[9][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[10][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[11][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[12][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[13][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[14][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[15][0]);


                    /* o1[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_sub_epi32(m_temp_reg_22, m_temp_reg_20);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[16][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[17][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[18][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[19][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[20][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[21][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[22][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[23][0]);

                    /* o2[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_sub_epi32(m_temp_reg_21, m_temp_reg_20);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_sub_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_sub_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }


                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[24][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[25][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[26][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[27][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[28][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[29][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[30][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[31][0]);

                    /* o3[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_sub_epi32(m_temp_reg_21, m_temp_reg_20);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_sub_epi32(m_temp_reg_41, m_temp_reg_40);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[32][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[33][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[34][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[35][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[36][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[37][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[38][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[39][0]);

                    /* o4[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }


                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[40][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[41][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[42][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[43][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[44][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[45][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[46][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[47][0]);

                    /* o5[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[48][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[49][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[50][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[51][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[52][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[53][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[54][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[55][0]);


                    /* o6[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[56][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[57][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[58][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[59][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[60][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[61][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[62][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[63][0]);

                    /* o7[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += 8;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += 8;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[64][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[65][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[66][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[67][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[68][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[69][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[70][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[71][0]);


                    /* o8[0-3] */
                    {

                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[72][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[73][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[74][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[75][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[76][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[77][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[78][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[79][0]);


                    /* o9[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[80][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[81][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[82][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[83][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[84][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[85][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[86][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[87][0]);

                    /* o10[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[88][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[89][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[90][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[91][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[92][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[93][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[94][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[95][0]);

                    /* o11[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[96][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[97][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[98][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[99][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[100][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[101][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[102][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[103][0]);


                    /* o12[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[104][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[105][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[106][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[107][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[108][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[109][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[110][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[111][0]);


                    /* o13[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;
                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[112][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[113][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[114][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[115][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[116][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[117][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[118][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[119][0]);


                    /* o14[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch -= in_stride;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch -= out_stride;

                    }

                    m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[120][0]);
                    m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[121][0]);
                    m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[122][0]);
                    m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[123][0]);
                    m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[124][0]);
                    m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[125][0]);
                    m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[126][0]);
                    m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[127][0]);

                    /* o15[0-3] */
                    {
                        m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                        m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                        m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                        m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                        m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                        m_temp_reg_40 = _mm_madd_epi16(temp1, m_coeff5);
                        m_temp_reg_41 = _mm_madd_epi16(temp2, m_coeff6);
                        m_temp_reg_42 = _mm_madd_epi16(temp3, m_coeff7);
                        m_temp_reg_43 = _mm_madd_epi16(temp4, m_coeff8);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                        m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                        m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                        m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                        m_temp_reg_30 = _mm_loadu_si128((__m128i *)pi2_src_scratch);
                        pi2_src_scratch += 8;

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_20);
                        m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_20);

                        m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                        m_count = _mm_cvtsi32_si128(i4_shift);
                        m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                        m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                        m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                        m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                        m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                        m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                        m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                        _mm_store_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                        pi2_dst_scratch += 8;
                    }

                }
            }
        }
        /* Transpose */
        {
            WORD16 *pi2_src_scratch = temp_ptr;
            WORD16 *pi2_dst_scratch = pi2_tmp;
            WORD32 in_stride = (trans_size << 1);

            for(j = 0; j < 2; j++)
            {
                m_temp_reg_30 =  _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch += in_stride;
                m_temp_reg_31 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch += in_stride;
                m_temp_reg_32 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch += in_stride;
                m_temp_reg_33 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch += in_stride;
                m_temp_reg_34 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch += in_stride;
                m_temp_reg_35 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch += in_stride;
                m_temp_reg_36 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch += in_stride;
                m_temp_reg_37 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch += 8;

                m_temp_reg_70 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch -= in_stride;
                m_temp_reg_71 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch -= in_stride;
                m_temp_reg_72 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch -= in_stride;
                m_temp_reg_73 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch -= in_stride;
                m_temp_reg_74 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch -= in_stride;
                m_temp_reg_75 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch -= in_stride;
                m_temp_reg_76 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch -= in_stride;
                m_temp_reg_77 = _mm_load_si128((__m128i *)pi2_src_scratch);
                pi2_src_scratch += 8;


                m_temp_reg_40 = _mm_unpacklo_epi16(m_temp_reg_30, m_temp_reg_31);
                m_temp_reg_41 = _mm_unpackhi_epi16(m_temp_reg_31, m_temp_reg_30);

                m_temp_reg_42 = _mm_unpacklo_epi16(m_temp_reg_32, m_temp_reg_33);
                m_temp_reg_43 = _mm_unpackhi_epi16(m_temp_reg_33, m_temp_reg_32);

                m_temp_reg_44 = _mm_unpacklo_epi16(m_temp_reg_34, m_temp_reg_35);
                m_temp_reg_45 = _mm_unpackhi_epi16(m_temp_reg_35, m_temp_reg_34);

                m_temp_reg_46 = _mm_unpacklo_epi16(m_temp_reg_36, m_temp_reg_37);
                m_temp_reg_47 = _mm_unpackhi_epi16(m_temp_reg_37, m_temp_reg_36);

                m_temp_reg_80 = _mm_unpacklo_epi16(m_temp_reg_70, m_temp_reg_71);
                m_temp_reg_81 = _mm_unpackhi_epi16(m_temp_reg_71, m_temp_reg_70);

                m_temp_reg_82 = _mm_unpacklo_epi16(m_temp_reg_72, m_temp_reg_73);
                m_temp_reg_83 = _mm_unpackhi_epi16(m_temp_reg_73, m_temp_reg_72);

                m_temp_reg_84 = _mm_unpacklo_epi16(m_temp_reg_74, m_temp_reg_75);
                m_temp_reg_85 = _mm_unpackhi_epi16(m_temp_reg_75, m_temp_reg_74);

                m_temp_reg_86 = _mm_unpacklo_epi16(m_temp_reg_76, m_temp_reg_77);
                m_temp_reg_87 = _mm_unpackhi_epi16(m_temp_reg_77, m_temp_reg_76);

                /****************/

                m_temp_reg_0 = _mm_unpacklo_epi32(m_temp_reg_40, m_temp_reg_42);
                m_temp_reg_1 = _mm_unpackhi_epi32(m_temp_reg_40, m_temp_reg_42);

                m_temp_reg_2 = _mm_unpacklo_epi32(m_temp_reg_44, m_temp_reg_46);
                m_temp_reg_3 = _mm_unpackhi_epi32(m_temp_reg_44, m_temp_reg_46);

                m_temp_reg_4 = _mm_unpacklo_epi32(m_temp_reg_80, m_temp_reg_82);
                m_temp_reg_5 = _mm_unpackhi_epi32(m_temp_reg_80, m_temp_reg_82);

                m_temp_reg_6 = _mm_unpacklo_epi32(m_temp_reg_84, m_temp_reg_86);
                m_temp_reg_7 = _mm_unpackhi_epi32(m_temp_reg_84, m_temp_reg_86);

                m_temp_reg_90 = _mm_unpacklo_epi32(m_temp_reg_43, m_temp_reg_41);
                m_temp_reg_91 = _mm_unpackhi_epi32(m_temp_reg_43, m_temp_reg_41);

                m_temp_reg_92 = _mm_unpacklo_epi32(m_temp_reg_47, m_temp_reg_45);
                m_temp_reg_93 = _mm_unpackhi_epi32(m_temp_reg_47, m_temp_reg_45);

                m_temp_reg_94 = _mm_unpacklo_epi32(m_temp_reg_83, m_temp_reg_81);
                m_temp_reg_95 = _mm_unpackhi_epi32(m_temp_reg_83, m_temp_reg_81);

                m_temp_reg_96 = _mm_unpacklo_epi32(m_temp_reg_87, m_temp_reg_85);
                m_temp_reg_97 = _mm_unpackhi_epi32(m_temp_reg_87, m_temp_reg_85);

                /******************/

                m_temp_reg_30 = _mm_unpacklo_epi64(m_temp_reg_0, m_temp_reg_2);
                m_temp_reg_31 = _mm_unpackhi_epi64(m_temp_reg_0, m_temp_reg_2);

                m_temp_reg_32 = _mm_unpacklo_epi64(m_temp_reg_92, m_temp_reg_90);
                m_temp_reg_33 = _mm_unpackhi_epi64(m_temp_reg_92, m_temp_reg_90);

                m_temp_reg_34 = _mm_unpacklo_epi64(m_temp_reg_4, m_temp_reg_6);
                m_temp_reg_35 = _mm_unpackhi_epi64(m_temp_reg_4, m_temp_reg_6);

                m_temp_reg_36 = _mm_unpacklo_epi64(m_temp_reg_96, m_temp_reg_94);
                m_temp_reg_37 = _mm_unpackhi_epi64(m_temp_reg_96, m_temp_reg_94);

                m_temp_reg_80 = _mm_unpacklo_epi64(m_temp_reg_1, m_temp_reg_3);
                m_temp_reg_81 = _mm_unpackhi_epi64(m_temp_reg_1, m_temp_reg_3);

                m_temp_reg_82 = _mm_unpacklo_epi64(m_temp_reg_93, m_temp_reg_91);
                m_temp_reg_83 = _mm_unpackhi_epi64(m_temp_reg_93, m_temp_reg_91);

                m_temp_reg_84 = _mm_unpacklo_epi64(m_temp_reg_5, m_temp_reg_7);
                m_temp_reg_85 = _mm_unpackhi_epi64(m_temp_reg_5, m_temp_reg_7);

                m_temp_reg_86 = _mm_unpacklo_epi64(m_temp_reg_97, m_temp_reg_95);
                m_temp_reg_87 = _mm_unpackhi_epi64(m_temp_reg_97, m_temp_reg_95);

                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 0 * trans_size), m_temp_reg_30);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 0 * trans_size + 8), m_temp_reg_34);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 0 * trans_size + 16), m_temp_reg_36);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 0 * trans_size + 24), m_temp_reg_32);

                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 1 * trans_size), m_temp_reg_31);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 1 * trans_size + 8), m_temp_reg_35);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 1 * trans_size + 16), m_temp_reg_37);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 1 * trans_size + 24), m_temp_reg_33);

                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 2 * trans_size), m_temp_reg_80);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 2 * trans_size + 8), m_temp_reg_84);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 2 * trans_size + 16), m_temp_reg_86);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 2 * trans_size + 24), m_temp_reg_82);

                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 3 * trans_size), m_temp_reg_81);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 3 * trans_size + 8), m_temp_reg_85);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 3 * trans_size + 16), m_temp_reg_87);
                _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 3 * trans_size + 24), m_temp_reg_83);

                pi2_dst_scratch += 4 * trans_size;
            }
        }
        pi2_src += 8;
//      pi2_dequant_coeff +=8;
        pi2_tmp += 8 * trans_size;
        zero_cols = zero_cols >> 1;
    }

    if(trans_size_stg1 != TRANS_SIZE_32)
    {
        m_temp_reg_10 = _mm_setzero_si128();

        for(i = trans_size_stg1; i < 32; i += 8)
        {
            WORD16 *pi2_dst_scratch = pi2_tmp;

            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 0 * trans_size), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 0 * trans_size + 8), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 0 * trans_size + 16), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 0 * trans_size + 24), m_temp_reg_10);

            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 1 * trans_size), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 1 * trans_size + 8), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 1 * trans_size + 16), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 1 * trans_size + 24), m_temp_reg_10);

            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 2 * trans_size), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 2 * trans_size + 8), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 2 * trans_size + 16), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 2 * trans_size + 24), m_temp_reg_10);

            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 3 * trans_size), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 3 * trans_size + 8), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 3 * trans_size + 16), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 3 * trans_size + 24), m_temp_reg_10);

            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 4 * trans_size), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 4 * trans_size + 8), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 4 * trans_size + 16), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 4 * trans_size + 24), m_temp_reg_10);

            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 5 * trans_size), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 5 * trans_size + 8), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 5 * trans_size + 16), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 5 * trans_size + 24), m_temp_reg_10);

            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 6 * trans_size), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 6 * trans_size + 8), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 6 * trans_size + 16), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 6 * trans_size + 24), m_temp_reg_10);

            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 7 * trans_size), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 7 * trans_size + 8), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 7 * trans_size + 16), m_temp_reg_10);
            _mm_storeu_si128((__m128i *)(pi2_dst_scratch + 7 * trans_size + 24), m_temp_reg_10);

            pi2_tmp += 8 * trans_size;
        }
    }

    pi2_tmp = pi2_tmp_orig;

    /* Inverse Transform 2nd stage */


    for(j = 0; j < trans_size; j += 4)
    {
        i4_shift = IT_SHIFT_STAGE_2;

        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        if(zero_last28_rows_stg2)
        {
            {

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[0][0]); //90 87
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[4][0]); //87
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[6][0]); //80
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[7][0]); //70
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[2][0]); //57
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[19][0]); //43
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[3][0]); //25
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[5][0]); //9

                m_temp_reg_10 = _mm_loadu_si128((__m128i *)&pi2_tmp[2 * trans_size]);

                m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_10, all_zero_reg);

                /* eo0[0-3] */
                {
                    m_temp_reg_90 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                }
                /* eo1[0-3] */
                {
                    m_temp_reg_91 = _mm_madd_epi16(m_temp_reg_10, m_coeff2);

                }
                /* eo2[0-3] */
                {
                    m_temp_reg_92 = _mm_madd_epi16(m_temp_reg_10, m_coeff3);
                }

                /* eo3[0-3] */
                {
                    m_temp_reg_93 = _mm_madd_epi16(m_temp_reg_10, m_coeff4);
                }
                /* eo4[0-3] */
                {
                    m_temp_reg_94 = _mm_madd_epi16(m_temp_reg_10, m_coeff5);
                }

                /* eo5[0-3] */
                {
                    m_temp_reg_95 = _mm_madd_epi16(m_temp_reg_10, m_coeff6);
                }

                /* eo6[0-3] */
                {
                    m_temp_reg_96 = _mm_madd_epi16(m_temp_reg_10, m_coeff7);
                }
                /* eo7[0-3] */
                {
                    m_temp_reg_97 = _mm_madd_epi16(m_temp_reg_10, m_coeff8);
                }
            }

            m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[2][0]); //64

            m_temp_reg_70 = _mm_loadu_si128((__m128i *)&pi2_tmp[0 * trans_size]);

            m_temp_reg_1 = _mm_unpacklo_epi16(m_temp_reg_70, all_zero_reg);

            m_temp_reg_14 = _mm_madd_epi16(m_temp_reg_1, m_coeff3);

            m_temp_reg_16 = _mm_madd_epi16(m_temp_reg_1, m_coeff3);

            /* e[]*/

            temp1 = _mm_add_epi32(m_temp_reg_14, m_temp_reg_90);  /* ee[0] */
            temp2 = _mm_sub_epi32(m_temp_reg_14, m_temp_reg_90);  /* ee[15] */

            temp3 = _mm_add_epi32(m_temp_reg_16, m_temp_reg_91);  /* ee[1] */
            temp4 = _mm_sub_epi32(m_temp_reg_16, m_temp_reg_91);  /* ee[14] */

            temp5 = _mm_add_epi32(m_temp_reg_16, m_temp_reg_92);  /* ee[2] */
            temp6 = _mm_sub_epi32(m_temp_reg_16, m_temp_reg_92);  /* ee[13] */

            temp7 = _mm_add_epi32(m_temp_reg_14, m_temp_reg_93);  /* ee[3] */
            temp8 = _mm_sub_epi32(m_temp_reg_14, m_temp_reg_93);  /* ee[12] */

            m_temp_reg_90 = _mm_add_epi32(m_temp_reg_14, m_temp_reg_94);  /* ee[4] */
            m_temp_reg_91 = _mm_sub_epi32(m_temp_reg_14, m_temp_reg_94);  /* ee[11] */

            m_temp_reg_92 = _mm_add_epi32(m_temp_reg_16, m_temp_reg_95);  /* ee[5] */
            m_temp_reg_93 = _mm_sub_epi32(m_temp_reg_16, m_temp_reg_95);  /* ee[10] */

            m_temp_reg_94 = _mm_add_epi32(m_temp_reg_16, m_temp_reg_96);  /* ee[6] */
            m_temp_reg_95 = _mm_sub_epi32(m_temp_reg_16, m_temp_reg_96);  /* ee[9] */

            m_temp_reg_96 = _mm_add_epi32(m_temp_reg_14, m_temp_reg_97);  /* ee[7] */
            m_temp_reg_97 = _mm_sub_epi32(m_temp_reg_14, m_temp_reg_97);  /* ee[8] */

            /*o[k]*/
            {

                WORD16 *pi2_dst_scratch = temp_ptr;
                WORD32 out_stride = 8;

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[0][0]);

                m_temp_reg_70 = _mm_loadu_si128((__m128i *)&pi2_tmp[trans_size]);
                m_temp_reg_71 = _mm_loadu_si128((__m128i *)&pi2_tmp[3 * trans_size]);

                m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_70, m_temp_reg_71); //row 1 and row 3 interleaved


                /* o0[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_sub_epi32(temp1, m_temp_reg_20);
                    m_temp_reg_30 = _mm_add_epi32(temp1, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[8][0]);

                /* o1[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_sub_epi32(temp3, m_temp_reg_20);
                    m_temp_reg_30 = _mm_add_epi32(temp3, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[16][0]);

                /* o2[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_sub_epi32(temp5, m_temp_reg_20);
                    m_temp_reg_30 = _mm_add_epi32(temp5, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[24][0]);

                /* o3[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_sub_epi32(temp7, m_temp_reg_20);
                    m_temp_reg_30 = _mm_add_epi32(temp7, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[32][0]);

                /* o4[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_90, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_90, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[40][0]);

                /* o5[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_92, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_92, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[48][0]);

                /* o6[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_94, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_94, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[56][0]);

                /* o7[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_96, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_96, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[64][0]);

                /* o8[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_97, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_97, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[72][0]);

                /* o9[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_95, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_95, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[80][0]);

                /* o10[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_93, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_93, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[88][0]);

                /* o11[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_91, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_91, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[96][0]);

                /* o12[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(temp8, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp8, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[104][0]);

                /* o13[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(temp6, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp6, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[112][0]);

                /* o14[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(temp4, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp4, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[120][0]);

                /* o15[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                    m_temp_reg_31 = _mm_add_epi32(temp2, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp2, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += 8;
                }

            }

        }
        else if(zero_last24_rows_stg2)
        {
            /* eo */
            {
                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[0][0]); //90 87

                m_temp_reg_10 = _mm_loadu_si128((__m128i *)&pi2_tmp[2 * trans_size]);
                m_temp_reg_11 = _mm_loadu_si128((__m128i *)&pi2_tmp[6 * trans_size]);

                m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_10, m_temp_reg_11);


                /* eo0[0-3] */
                {
                    m_temp_reg_90 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[4][0]); //87  57

                /* eo1[0-3] */
                {
                    m_temp_reg_91 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                }
                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[8][0]); //80  9

                /* eo2[0-3] */
                {
                    m_temp_reg_92 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[12][0]); //70  -43

                /* eo3[0-3] */
                {

                    m_temp_reg_93 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[16][0]); //57  -80

                /* eo4[0-3] */
                {
                    m_temp_reg_94 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[20][0]); //43  -90

                /* eo5[0-3] */
                {
                    m_temp_reg_95 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[24][0]); //25  -70
                /* eo6[0-3] */
                {
                    m_temp_reg_96 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[28][0]); //9  -25
                /* eo7[0-3] */
                {
                    m_temp_reg_97 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                }

            }

            /* eeo */
            {

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[4][0]); //89 75
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[6][0]); //75
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[9][0]); //18
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[8][0]); //50

                m_temp_reg_72 = _mm_loadu_si128((__m128i *)&pi2_tmp[4 * trans_size]);

                m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_72, all_zero_reg);

                /* eeo0[0-3] */
                {
                    temp1 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);

                }

                /* eeo1[0-3] */
                {
                    temp2 = _mm_madd_epi16(m_temp_reg_10, m_coeff2);

                }

                /* eo2[0-3] */
                {
                    temp3 = _mm_madd_epi16(m_temp_reg_10, m_coeff4);

                }


                /* eo3[0-3] */
                {
                    temp4 = _mm_madd_epi16(m_temp_reg_10, m_coeff3);

                }

            }

            m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[0][0]); //83
            m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[1][0]); //36
            m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[2][0]); //64

            m_temp_reg_70 = _mm_loadu_si128((__m128i *)&pi2_tmp[0 * trans_size]);

            //m_temp_reg_1 = _mm_cvtepi16_epi32(m_temp_reg_70);
            m_temp_reg_1 = _mm_unpacklo_epi16(m_temp_reg_70, all_zero_reg);

            m_temp_reg_14 = _mm_madd_epi16(m_temp_reg_1, m_coeff3);
            m_temp_reg_16 = _mm_madd_epi16(m_temp_reg_1, m_coeff3);

            m_temp_reg_70 = _mm_add_epi32(m_temp_reg_14, temp1);  /* ee[0] */
            m_temp_reg_71 = _mm_sub_epi32(m_temp_reg_14, temp1);  /* ee[7] */

            m_temp_reg_72 = _mm_add_epi32(m_temp_reg_16, temp2);  /* ee[1] */
            m_temp_reg_73 = _mm_sub_epi32(m_temp_reg_16, temp2);  /* ee[6] */

            m_temp_reg_74 = _mm_add_epi32(m_temp_reg_16, temp3);  /* ee[2] */
            m_temp_reg_75 = _mm_sub_epi32(m_temp_reg_16, temp3);  /* ee[5] */

            m_temp_reg_76 = _mm_add_epi32(m_temp_reg_14, temp4);  /* ee[3] */
            m_temp_reg_77 = _mm_sub_epi32(m_temp_reg_14, temp4);  /* ee[4] */

            /* e[]*/

            temp1 = _mm_add_epi32(m_temp_reg_70, m_temp_reg_90);  /* ee[0] */
            temp2 = _mm_sub_epi32(m_temp_reg_70, m_temp_reg_90);  /* ee[15] */

            temp3 = _mm_add_epi32(m_temp_reg_72, m_temp_reg_91);  /* ee[1] */
            temp4 = _mm_sub_epi32(m_temp_reg_72, m_temp_reg_91);  /* ee[14] */

            temp5 = _mm_add_epi32(m_temp_reg_74, m_temp_reg_92);  /* ee[2] */
            temp6 = _mm_sub_epi32(m_temp_reg_74, m_temp_reg_92);  /* ee[13] */

            temp7 = _mm_add_epi32(m_temp_reg_76, m_temp_reg_93);  /* ee[3] */
            temp8 = _mm_sub_epi32(m_temp_reg_76, m_temp_reg_93);  /* ee[12] */

            m_temp_reg_90 = _mm_add_epi32(m_temp_reg_77, m_temp_reg_94);  /* ee[4] */
            m_temp_reg_91 = _mm_sub_epi32(m_temp_reg_77, m_temp_reg_94);  /* ee[11] */

            m_temp_reg_92 = _mm_add_epi32(m_temp_reg_75, m_temp_reg_95);  /* ee[5] */
            m_temp_reg_93 = _mm_sub_epi32(m_temp_reg_75, m_temp_reg_95);  /* ee[10] */

            m_temp_reg_94 = _mm_add_epi32(m_temp_reg_73, m_temp_reg_96);  /* ee[6] */
            m_temp_reg_95 = _mm_sub_epi32(m_temp_reg_73, m_temp_reg_96);  /* ee[9] */

            m_temp_reg_96 = _mm_add_epi32(m_temp_reg_71, m_temp_reg_97);  /* ee[7] */
            m_temp_reg_97 = _mm_sub_epi32(m_temp_reg_71, m_temp_reg_97);  /* ee[8] */

            /*o[k] */
            {

                WORD16 *pi2_dst_scratch = temp_ptr;
                WORD32 out_stride = 8;

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[0][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[1][0]);

                m_temp_reg_70 = _mm_loadu_si128((__m128i *)&pi2_tmp[trans_size]);
                m_temp_reg_71 = _mm_loadu_si128((__m128i *)&pi2_tmp[3 * trans_size]);
                m_temp_reg_72 = _mm_loadu_si128((__m128i *)&pi2_tmp[5 * trans_size]);
                m_temp_reg_73 = _mm_loadu_si128((__m128i *)&pi2_tmp[7 * trans_size]);

                m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_70, m_temp_reg_71);
                m_temp_reg_11 = _mm_unpacklo_epi16(m_temp_reg_72, m_temp_reg_73);

                /* o0[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                    m_temp_reg_31 = _mm_sub_epi32(temp1, m_temp_reg_20);
                    m_temp_reg_30 = _mm_add_epi32(temp1, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }


                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[8][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[9][0]);

                /* o1[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);

                    m_temp_reg_31 = _mm_sub_epi32(temp3, m_temp_reg_20);
                    m_temp_reg_30 = _mm_add_epi32(temp3, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[16][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[17][0]);

                /* o2[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_sub_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(temp5, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp5, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[24][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[25][0]);

                /* o3[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_sub_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(temp7, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp7, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[32][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[33][0]);

                /* o4[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_90, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_90, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[40][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[41][0]);

                /* o5[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_92, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_92, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[48][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[49][0]);

                /* o6[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_94, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_94, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[56][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[57][0]);

                /* o7[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_96, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_96, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[64][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[65][0]);

                /* o8[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_97, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_97, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[72][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[73][0]);

                /* o9[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_95, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_95, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[80][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[81][0]);

                /* o10[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_93, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_93, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[88][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[89][0]);

                /* o11[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_91, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_91, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[96][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[97][0]);

                /* o12[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(temp8, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp8, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[104][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[105][0]);

                /* o13[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(temp6, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp6, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[112][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[113][0]);

                /* o14[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(temp4, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp4, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[120][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[121][0]);

                /* o15[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);

                    m_temp_reg_31 = _mm_add_epi32(temp2, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp2, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += 8;
                }

            }
        }
        else
        {
            /* eo */
            {

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[0][0]); //90 87
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[1][0]); //80 70
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[2][0]); //57 43
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[3][0]); //25 9


                m_temp_reg_10 = _mm_loadu_si128((__m128i *)&pi2_tmp[2 * trans_size]);
                m_temp_reg_11 = _mm_loadu_si128((__m128i *)&pi2_tmp[6 * trans_size]);
                m_temp_reg_12 = _mm_loadu_si128((__m128i *)&pi2_tmp[10 * trans_size]);
                m_temp_reg_13 = _mm_loadu_si128((__m128i *)&pi2_tmp[14 * trans_size]);
                m_temp_reg_18 = _mm_loadu_si128((__m128i *)&pi2_tmp[18 * trans_size]);
                m_temp_reg_19 = _mm_loadu_si128((__m128i *)&pi2_tmp[22 * trans_size]);
                m_temp_reg_20 = _mm_loadu_si128((__m128i *)&pi2_tmp[26 * trans_size]);
                m_temp_reg_21 = _mm_loadu_si128((__m128i *)&pi2_tmp[30 * trans_size]);

                m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_10, m_temp_reg_11);
                m_temp_reg_11 = _mm_unpacklo_epi16(m_temp_reg_12, m_temp_reg_13);
                m_temp_reg_12 = _mm_unpacklo_epi16(m_temp_reg_18, m_temp_reg_19);
                m_temp_reg_13 = _mm_unpacklo_epi16(m_temp_reg_20, m_temp_reg_21);

                /* eo0[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_90 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[4][0]); //87  57
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[5][0]); //0  -43
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[6][0]); //80  90
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[7][0]); //70  25

                /* eo1[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_91 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_32);

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[8][0]); //80  9
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[9][0]); //70  87
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[10][0]); //-25  57
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[11][0]); //90  43

                /* eo2[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_92 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[12][0]); //70  -43
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[13][0]); //-87  9
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[14][0]); //90  25
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[15][0]); //80  57

                /* eo3[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_sub_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_93 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[16][0]); //57  -80
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[17][0]); //-25  90
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[18][0]); //9  87
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[19][0]); //43  70


                /* eo4[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_sub_epi32(m_temp_reg_33, m_temp_reg_32);

                    m_temp_reg_94 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[20][0]); //43  -90
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[21][0]); //57  25
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[22][0]); //-87  70
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[23][0]); //9  -80

                /* eo5[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_95 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[24][0]); //25  -70
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[25][0]); //90  -80
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[26][0]); //43  9
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[27][0]); //-57  87

                /* eo6[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_96 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[28][0]); //9  -25
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[29][0]); //43  -57
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[30][0]); //70  -80
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_packed[31][0]); //87  -90

                /* eo7[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                    m_temp_reg_32 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_33 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_32 = _mm_add_epi32(m_temp_reg_32, m_temp_reg_33);

                    m_temp_reg_97 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_32);


                }

            }

            /* eeo */
            {
                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[4][0]); //89 75
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[5][0]); //50 18

                m_temp_reg_72 = _mm_loadu_si128((__m128i *)&pi2_tmp[4 * trans_size]);
                m_temp_reg_76 = _mm_loadu_si128((__m128i *)&pi2_tmp[12 * trans_size]);
                m_temp_reg_82 = _mm_loadu_si128((__m128i *)&pi2_tmp[20 * trans_size]);
                m_temp_reg_86 = _mm_loadu_si128((__m128i *)&pi2_tmp[28 * trans_size]);

                /* eeo0[0-3] */
                {

                    m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_72, m_temp_reg_76);
                    m_temp_reg_11 = _mm_unpacklo_epi16(m_temp_reg_82, m_temp_reg_86);

                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);

                    temp1 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                }

                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[6][0]); //75 -18
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[7][0]); //89  50

                /* eeo1[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff3);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff4);

                    temp2 = _mm_sub_epi32(m_temp_reg_30, m_temp_reg_31);

                }

                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[8][0]); //50 -89
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[9][0]); //18  75

                /* eo2[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff3);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff4);

                    temp3 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                }

                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[10][0]); //18 -50
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[11][0]); //75  -89

                /* eo3[0-3] */
                {
                    m_temp_reg_30 = _mm_madd_epi16(m_temp_reg_10, m_coeff3);
                    m_temp_reg_31 = _mm_madd_epi16(m_temp_reg_11, m_coeff4);

                    temp4 = _mm_add_epi32(m_temp_reg_30, m_temp_reg_31);

                }


            }

            m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[0][0]); //83 36
            m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[1][0]); //36 -83

            m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[2][0]); //64 64
            m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_16_even_packed[3][0]); //64 -64

            m_temp_reg_74 = _mm_loadu_si128((__m128i *)&pi2_tmp[8 * trans_size]);
            m_temp_reg_84 = _mm_loadu_si128((__m128i *)&pi2_tmp[24 * trans_size]);

            m_temp_reg_0 = _mm_unpacklo_epi16(m_temp_reg_74, m_temp_reg_84);

            m_temp_reg_70 = _mm_loadu_si128((__m128i *)&pi2_tmp[0 * trans_size]);
            m_temp_reg_80 = _mm_loadu_si128((__m128i *)&pi2_tmp[16 * trans_size]);

            m_temp_reg_1 = _mm_unpacklo_epi16(m_temp_reg_70, m_temp_reg_80);

            m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_0, m_coeff1);  /* eeeo[0] */
            m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_0, m_coeff2);  /* eeeo[1] */

            m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_1, m_coeff3);  /* eeee[0] */
            m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_1, m_coeff4);  /* eeee[1] */

/* eeeo[0]= m_temp_reg_20  */
/* eeeo[1]= m_temp_reg_21  */
/* eeee[0]= m_temp_reg_22  */
/* eeee[1]= m_temp_reg_23  */

            /* eee[0] = eeee[0] + eeeo[0]; */
            m_temp_reg_40 = _mm_add_epi32(m_temp_reg_21, m_temp_reg_20);  /* eeeo[0] */

            /* eee[3] = eeee[0] - eeeo[0]; */
            m_temp_reg_43 = _mm_sub_epi32(m_temp_reg_21, m_temp_reg_20);  /* eeeo[1] */

            /* eee[2] = eeee[1] - eeeo[1]; */
            m_temp_reg_42 = _mm_sub_epi32(m_temp_reg_23, m_temp_reg_22);  /* eeee[1] */

            /* eee[1] = eeee[1] + eeeo[1];*/
            m_temp_reg_41 = _mm_add_epi32(m_temp_reg_23, m_temp_reg_22);  /* eeee[0] */

            m_temp_reg_70 = _mm_add_epi32(m_temp_reg_40, temp1);  /* ee[0] */
            m_temp_reg_71 = _mm_sub_epi32(m_temp_reg_40, temp1);  /* ee[7] */

            m_temp_reg_72 = _mm_add_epi32(m_temp_reg_41, temp2);  /* ee[1] */
            m_temp_reg_73 = _mm_sub_epi32(m_temp_reg_41, temp2);  /* ee[6] */

            m_temp_reg_74 = _mm_add_epi32(m_temp_reg_42, temp3);  /* ee[2] */
            m_temp_reg_75 = _mm_sub_epi32(m_temp_reg_42, temp3);  /* ee[5] */

            m_temp_reg_76 = _mm_add_epi32(m_temp_reg_43, temp4);  /* ee[3] */
            m_temp_reg_77 = _mm_sub_epi32(m_temp_reg_43, temp4);  /* ee[4] */

/* e[]*/

            temp1 = _mm_add_epi32(m_temp_reg_70, m_temp_reg_90);  /* ee[0] */
            temp2 = _mm_sub_epi32(m_temp_reg_70, m_temp_reg_90);  /* ee[15] */

            temp3 = _mm_add_epi32(m_temp_reg_72, m_temp_reg_91);  /* ee[1] */
            temp4 = _mm_sub_epi32(m_temp_reg_72, m_temp_reg_91);  /* ee[14] */

            temp5 = _mm_add_epi32(m_temp_reg_74, m_temp_reg_92);  /* ee[2] */
            temp6 = _mm_sub_epi32(m_temp_reg_74, m_temp_reg_92);  /* ee[13] */

            temp7 = _mm_add_epi32(m_temp_reg_76, m_temp_reg_93);  /* ee[3] */
            temp8 = _mm_sub_epi32(m_temp_reg_76, m_temp_reg_93);  /* ee[12] */

            m_temp_reg_90 = _mm_add_epi32(m_temp_reg_77, m_temp_reg_94);  /* ee[4] */
            m_temp_reg_91 = _mm_sub_epi32(m_temp_reg_77, m_temp_reg_94);  /* ee[11] */

            m_temp_reg_92 = _mm_add_epi32(m_temp_reg_75, m_temp_reg_95);  /* ee[5] */
            m_temp_reg_93 = _mm_sub_epi32(m_temp_reg_75, m_temp_reg_95);  /* ee[10] */

            m_temp_reg_94 = _mm_add_epi32(m_temp_reg_73, m_temp_reg_96);  /* ee[6] */
            m_temp_reg_95 = _mm_sub_epi32(m_temp_reg_73, m_temp_reg_96);  /* ee[9] */

            m_temp_reg_96 = _mm_add_epi32(m_temp_reg_71, m_temp_reg_97);  /* ee[7] */
            m_temp_reg_97 = _mm_sub_epi32(m_temp_reg_71, m_temp_reg_97);  /* ee[8] */

/*o[k] */
            {

                WORD16 *pi2_dst_scratch = temp_ptr;
                WORD32 out_stride = 8;

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[0][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[1][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[2][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[3][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[4][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[5][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[6][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[7][0]);


                m_temp_reg_70 = _mm_loadu_si128((__m128i *)&pi2_tmp[trans_size]);
                m_temp_reg_71 = _mm_loadu_si128((__m128i *)&pi2_tmp[3 * trans_size]);
                m_temp_reg_72 = _mm_loadu_si128((__m128i *)&pi2_tmp[5 * trans_size]);
                m_temp_reg_73 = _mm_loadu_si128((__m128i *)&pi2_tmp[7 * trans_size]);
                m_temp_reg_74 = _mm_loadu_si128((__m128i *)&pi2_tmp[9 * trans_size]);
                m_temp_reg_75 = _mm_loadu_si128((__m128i *)&pi2_tmp[11 * trans_size]);
                m_temp_reg_76 = _mm_loadu_si128((__m128i *)&pi2_tmp[13 * trans_size]);
                m_temp_reg_77 = _mm_loadu_si128((__m128i *)&pi2_tmp[15 * trans_size]);

                m_temp_reg_80 = _mm_loadu_si128((__m128i *)&pi2_tmp[17 * trans_size]);
                m_temp_reg_81 = _mm_loadu_si128((__m128i *)&pi2_tmp[19 * trans_size]);
                m_temp_reg_82 = _mm_loadu_si128((__m128i *)&pi2_tmp[21 * trans_size]);
                m_temp_reg_83 = _mm_loadu_si128((__m128i *)&pi2_tmp[23 * trans_size]);
                m_temp_reg_84 = _mm_loadu_si128((__m128i *)&pi2_tmp[25 * trans_size]);
                m_temp_reg_85 = _mm_loadu_si128((__m128i *)&pi2_tmp[27 * trans_size]);
                m_temp_reg_86 = _mm_loadu_si128((__m128i *)&pi2_tmp[29 * trans_size]);
                m_temp_reg_87 = _mm_loadu_si128((__m128i *)&pi2_tmp[31 * trans_size]);

                m_temp_reg_10 = _mm_unpacklo_epi16(m_temp_reg_70, m_temp_reg_71); //row 1 and row 3 interleaved
                m_temp_reg_11 = _mm_unpacklo_epi16(m_temp_reg_72, m_temp_reg_73); //row 5 and row 7 interleaved
                m_temp_reg_12 = _mm_unpacklo_epi16(m_temp_reg_74, m_temp_reg_75); //row 9 and row 11 interleaved
                m_temp_reg_13 = _mm_unpacklo_epi16(m_temp_reg_76, m_temp_reg_77); //row 13 and row 15 interleaved
                m_temp_reg_14 = _mm_unpacklo_epi16(m_temp_reg_80, m_temp_reg_81); //row 17 and row 19 interleaved
                m_temp_reg_15 = _mm_unpacklo_epi16(m_temp_reg_82, m_temp_reg_83); //row 21 and row 23 interleaved
                m_temp_reg_16 = _mm_unpacklo_epi16(m_temp_reg_84, m_temp_reg_85); //row 25 and row 27 interleaved
                m_temp_reg_17 = _mm_unpacklo_epi16(m_temp_reg_86, m_temp_reg_87); //row 29 and row 31 interleaved

                /* o0[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_sub_epi32(temp1, m_temp_reg_20);
                    m_temp_reg_30 = _mm_add_epi32(temp1, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[8][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[9][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[10][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[11][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[12][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[13][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[14][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[15][0]);

                /* o1[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_sub_epi32(m_temp_reg_22, m_temp_reg_20);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(temp3, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp3, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[16][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[17][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[18][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[19][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[20][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[21][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[22][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[23][0]);

                /* o2[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_sub_epi32(m_temp_reg_21, m_temp_reg_20);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_sub_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_sub_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(temp5, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp5, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[24][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[25][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[26][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[27][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[28][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[29][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[30][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[31][0]);

                /* o3[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_sub_epi32(m_temp_reg_21, m_temp_reg_20);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_sub_epi32(m_temp_reg_41, m_temp_reg_40);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(temp7, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp7, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[32][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[33][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[34][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[35][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[36][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[37][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[38][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[39][0]);

                /* o4[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_90, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_90, m_temp_reg_20);
                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[40][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[41][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[42][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[43][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[44][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[45][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[46][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[47][0]);

                /* o5[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_92, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_92, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[48][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[49][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[50][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[51][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[52][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[53][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[54][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[55][0]);

                /* o6[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_94, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_94, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[56][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[57][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[58][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[59][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[60][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[61][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[62][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[63][0]);

                /* o7[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_96, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_96, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += 8;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[64][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[65][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[66][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[67][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[68][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[69][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[70][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[71][0]);

                /* o8[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_97, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_97, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[72][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[73][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[74][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[75][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[76][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[77][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[78][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[79][0]);

                /* o9[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_95, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_95, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[80][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[81][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[82][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[83][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[84][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[85][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[86][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[87][0]);

                /* o10[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_93, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_93, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }


                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[88][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[89][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[90][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[91][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[92][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[93][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[94][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[95][0]);

                /* o11[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_91, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(m_temp_reg_91, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[96][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[97][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[98][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[99][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[100][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[101][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[102][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[103][0]);

                /* o12[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(temp8, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp8, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[104][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[105][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[106][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[107][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[108][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[109][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[110][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[111][0]);

                /* o13[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(temp6, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp6, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;
                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[112][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[113][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[114][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[115][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[116][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[117][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[118][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[119][0]);

                /* o14[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(temp4, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp4, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += out_stride;

                }

                m_coeff1 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[120][0]);
                m_coeff2 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[121][0]);
                m_coeff3 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[122][0]);
                m_coeff4 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[123][0]);
                m_coeff5 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[124][0]);
                m_coeff6 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[125][0]);
                m_coeff7 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[126][0]);
                m_coeff8 = _mm_loadu_si128((__m128i *)&g_ai2_ihevc_trans_32_intr_odd_packed[127][0]);

                /* o15[0-3] */
                {
                    m_temp_reg_20 = _mm_madd_epi16(m_temp_reg_10, m_coeff1);
                    m_temp_reg_21 = _mm_madd_epi16(m_temp_reg_11, m_coeff2);
                    m_temp_reg_22 = _mm_madd_epi16(m_temp_reg_12, m_coeff3);
                    m_temp_reg_23 = _mm_madd_epi16(m_temp_reg_13, m_coeff4);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_21);
                    m_temp_reg_22 = _mm_add_epi32(m_temp_reg_22, m_temp_reg_23);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_22);

                    m_temp_reg_40 = _mm_madd_epi16(m_temp_reg_14, m_coeff5);
                    m_temp_reg_41 = _mm_madd_epi16(m_temp_reg_15, m_coeff6);
                    m_temp_reg_42 = _mm_madd_epi16(m_temp_reg_16, m_coeff7);
                    m_temp_reg_43 = _mm_madd_epi16(m_temp_reg_17, m_coeff8);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_41);
                    m_temp_reg_42 = _mm_add_epi32(m_temp_reg_42, m_temp_reg_43);

                    m_temp_reg_40 = _mm_add_epi32(m_temp_reg_40, m_temp_reg_42);

                    m_temp_reg_20 = _mm_add_epi32(m_temp_reg_20, m_temp_reg_40);

                    m_temp_reg_31 = _mm_add_epi32(temp2, m_temp_reg_20);
                    m_temp_reg_30 = _mm_sub_epi32(temp2, m_temp_reg_20);

                    m_rdng_factor = _mm_cvtsi32_si128((1 << (i4_shift - 1)));
                    m_count = _mm_cvtsi32_si128(i4_shift);
                    m_rdng_factor = _mm_unpacklo_epi32(m_rdng_factor, m_rdng_factor);
                    m_rdng_factor = _mm_unpacklo_epi64(m_rdng_factor, m_rdng_factor);

                    m_temp_reg_31 = _mm_add_epi32(m_temp_reg_31, m_rdng_factor);
                    m_temp_reg_30 = _mm_add_epi32(m_temp_reg_30, m_rdng_factor);
                    m_temp_reg_31 = _mm_sra_epi32(m_temp_reg_31, m_count);
                    m_temp_reg_30 = _mm_sra_epi32(m_temp_reg_30, m_count);

                    m_temp_reg_30 = _mm_packs_epi32(m_temp_reg_30, m_temp_reg_31);

                    _mm_storeu_si128((__m128i *)pi2_dst_scratch, m_temp_reg_30);
                    pi2_dst_scratch += 8;
                }

            }
        }

        /* Transpose */
        {

            WORD16 *pi2_src_scratch = temp_ptr;
            WORD32 out_stride = dst_strd;
            WORD32 in_stride = 8;

            m_temp_reg_30 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_31 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_32 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_33 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_34 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_35 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_36 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_37 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += 8;

            m_temp_reg_70 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_71 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_72 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_73 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_74 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_75 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_76 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += in_stride;
            m_temp_reg_77 = _mm_load_si128((__m128i *)pi2_src_scratch);
            pi2_src_scratch += 8;


            m_temp_reg_40 = _mm_unpacklo_epi16(m_temp_reg_30, m_temp_reg_31);
            m_temp_reg_41 = _mm_unpackhi_epi16(m_temp_reg_31, m_temp_reg_30);

            m_temp_reg_42 = _mm_unpacklo_epi16(m_temp_reg_32, m_temp_reg_33);
            m_temp_reg_43 = _mm_unpackhi_epi16(m_temp_reg_33, m_temp_reg_32);

            m_temp_reg_44 = _mm_unpacklo_epi16(m_temp_reg_34, m_temp_reg_35);
            m_temp_reg_45 = _mm_unpackhi_epi16(m_temp_reg_35, m_temp_reg_34);

            m_temp_reg_46 = _mm_unpacklo_epi16(m_temp_reg_36, m_temp_reg_37);
            m_temp_reg_47 = _mm_unpackhi_epi16(m_temp_reg_37, m_temp_reg_36);

            m_temp_reg_80 = _mm_unpacklo_epi16(m_temp_reg_70, m_temp_reg_71);
            m_temp_reg_81 = _mm_unpackhi_epi16(m_temp_reg_71, m_temp_reg_70);

            m_temp_reg_82 = _mm_unpacklo_epi16(m_temp_reg_72, m_temp_reg_73);
            m_temp_reg_83 = _mm_unpackhi_epi16(m_temp_reg_73, m_temp_reg_72);

            m_temp_reg_84 = _mm_unpacklo_epi16(m_temp_reg_74, m_temp_reg_75);
            m_temp_reg_85 = _mm_unpackhi_epi16(m_temp_reg_75, m_temp_reg_74);

            m_temp_reg_86 = _mm_unpacklo_epi16(m_temp_reg_76, m_temp_reg_77);
            m_temp_reg_87 = _mm_unpackhi_epi16(m_temp_reg_77, m_temp_reg_76);


            m_temp_reg_0 = _mm_unpacklo_epi32(m_temp_reg_40, m_temp_reg_42);
            m_temp_reg_1 = _mm_unpackhi_epi32(m_temp_reg_40, m_temp_reg_42);

            m_temp_reg_2 = _mm_unpacklo_epi32(m_temp_reg_44, m_temp_reg_46);
            m_temp_reg_3 = _mm_unpackhi_epi32(m_temp_reg_44, m_temp_reg_46);

            m_temp_reg_4 = _mm_unpacklo_epi32(m_temp_reg_80, m_temp_reg_82);
            m_temp_reg_5 = _mm_unpackhi_epi32(m_temp_reg_80, m_temp_reg_82);

            m_temp_reg_6 = _mm_unpacklo_epi32(m_temp_reg_84, m_temp_reg_86);
            m_temp_reg_7 = _mm_unpackhi_epi32(m_temp_reg_84, m_temp_reg_86);

            m_temp_reg_90 = _mm_unpacklo_epi32(m_temp_reg_43, m_temp_reg_41);
            m_temp_reg_91 = _mm_unpackhi_epi32(m_temp_reg_43, m_temp_reg_41);

            m_temp_reg_92 = _mm_unpacklo_epi32(m_temp_reg_47, m_temp_reg_45);
            m_temp_reg_93 = _mm_unpackhi_epi32(m_temp_reg_47, m_temp_reg_45);

            m_temp_reg_94 = _mm_unpacklo_epi32(m_temp_reg_83, m_temp_reg_81);
            m_temp_reg_95 = _mm_unpackhi_epi32(m_temp_reg_83, m_temp_reg_81);

            m_temp_reg_96 = _mm_unpacklo_epi32(m_temp_reg_87, m_temp_reg_85);
            m_temp_reg_97 = _mm_unpackhi_epi32(m_temp_reg_87, m_temp_reg_85);


            m_temp_reg_30 = _mm_unpacklo_epi64(m_temp_reg_0, m_temp_reg_2);       // row0 = 0-7
            m_temp_reg_31 = _mm_unpackhi_epi64(m_temp_reg_0, m_temp_reg_2);       // row1 = 0-7

            m_temp_reg_32 = _mm_unpacklo_epi64(m_temp_reg_92, m_temp_reg_90);     // row0=24-31
            m_temp_reg_33 = _mm_unpackhi_epi64(m_temp_reg_92, m_temp_reg_90);     // row1=24-31

            m_temp_reg_34 = _mm_unpacklo_epi64(m_temp_reg_4, m_temp_reg_6);       // row0=8-15
            m_temp_reg_35 = _mm_unpackhi_epi64(m_temp_reg_4, m_temp_reg_6);       // row1=8-15

            m_temp_reg_36 = _mm_unpacklo_epi64(m_temp_reg_96, m_temp_reg_94);     // row0=16-23
            m_temp_reg_37 = _mm_unpackhi_epi64(m_temp_reg_96, m_temp_reg_94);     // row1=16-23

            m_temp_reg_80 = _mm_unpacklo_epi64(m_temp_reg_1, m_temp_reg_3);      // row2 =0-7
            m_temp_reg_81 = _mm_unpackhi_epi64(m_temp_reg_1, m_temp_reg_3);      // row3 =0-7

            m_temp_reg_82 = _mm_unpacklo_epi64(m_temp_reg_93, m_temp_reg_91);    // row2=24-31
            m_temp_reg_83 = _mm_unpackhi_epi64(m_temp_reg_93, m_temp_reg_91);    // row3=24-31

            m_temp_reg_84 = _mm_unpacklo_epi64(m_temp_reg_5, m_temp_reg_7);      // row2=8-15
            m_temp_reg_85 = _mm_unpackhi_epi64(m_temp_reg_5, m_temp_reg_7);      // row3=8-15

            m_temp_reg_86 = _mm_unpacklo_epi64(m_temp_reg_97, m_temp_reg_95);    // row2=16-23
            m_temp_reg_87 = _mm_unpackhi_epi64(m_temp_reg_97, m_temp_reg_95);    // row3=16-23

            m_temp_reg_20 = _mm_loadu_si128((__m128i *)pu1_pred);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_20);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_20, all_zero_reg);

            m_temp_reg_40 = _mm_add_epi16(m_temp_reg_30, m_temp_reg_0);
            m_temp_reg_0 = _mm_srli_si128(m_temp_reg_20, 8);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_0);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_0, all_zero_reg);

            m_temp_reg_44 = _mm_add_epi16(m_temp_reg_34, m_temp_reg_0);
            m_temp_reg_20 = _mm_packus_epi16(m_temp_reg_40, m_temp_reg_44);

            _mm_storeu_si128((__m128i *)pu1_dst, m_temp_reg_20);

            m_temp_reg_20 = _mm_loadu_si128((__m128i *)(pu1_pred + 16));

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_20);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_20, all_zero_reg);

            m_temp_reg_40 = _mm_add_epi16(m_temp_reg_36, m_temp_reg_0);
            m_temp_reg_0 = _mm_srli_si128(m_temp_reg_20, 8);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_0);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_0, all_zero_reg);

            m_temp_reg_44 = _mm_add_epi16(m_temp_reg_32, m_temp_reg_0);
            m_temp_reg_20 = _mm_packus_epi16(m_temp_reg_40, m_temp_reg_44);

            _mm_storeu_si128((__m128i *)(pu1_dst + 16), m_temp_reg_20);
            pu1_dst += out_stride;
            pu1_pred += pred_strd;


            m_temp_reg_20 = _mm_loadu_si128((__m128i *)pu1_pred);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_20);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_20, all_zero_reg);

            m_temp_reg_40 = _mm_add_epi16(m_temp_reg_31, m_temp_reg_0);
            m_temp_reg_0 = _mm_srli_si128(m_temp_reg_20, 8);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_0);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_0, all_zero_reg);

            m_temp_reg_44 = _mm_add_epi16(m_temp_reg_35, m_temp_reg_0);
            m_temp_reg_20 = _mm_packus_epi16(m_temp_reg_40, m_temp_reg_44);

            _mm_storeu_si128((__m128i *)pu1_dst, m_temp_reg_20);

            m_temp_reg_20 = _mm_loadu_si128((__m128i *)(pu1_pred + 16));

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_20);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_20, all_zero_reg);

            m_temp_reg_40 = _mm_add_epi16(m_temp_reg_37, m_temp_reg_0);
            m_temp_reg_0 = _mm_srli_si128(m_temp_reg_20, 8);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_0);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_0, all_zero_reg);

            m_temp_reg_44 = _mm_add_epi16(m_temp_reg_33, m_temp_reg_0);
            m_temp_reg_20 = _mm_packus_epi16(m_temp_reg_40, m_temp_reg_44);

            _mm_storeu_si128((__m128i *)(pu1_dst + 16), m_temp_reg_20);
            pu1_dst += out_stride;
            pu1_pred += pred_strd;

            m_temp_reg_20 = _mm_loadu_si128((__m128i *)pu1_pred);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_20);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_20, all_zero_reg);

            m_temp_reg_40 = _mm_add_epi16(m_temp_reg_80, m_temp_reg_0);
            m_temp_reg_0 = _mm_srli_si128(m_temp_reg_20, 8);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_0);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_0, all_zero_reg);

            m_temp_reg_44 = _mm_add_epi16(m_temp_reg_84, m_temp_reg_0);
            m_temp_reg_20 = _mm_packus_epi16(m_temp_reg_40, m_temp_reg_44);

            _mm_storeu_si128((__m128i *)pu1_dst, m_temp_reg_20);

            m_temp_reg_20 = _mm_loadu_si128((__m128i *)(pu1_pred + 16));

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_20);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_20, all_zero_reg);

            m_temp_reg_40 = _mm_add_epi16(m_temp_reg_86, m_temp_reg_0);
            m_temp_reg_0 = _mm_srli_si128(m_temp_reg_20, 8);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_0);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_0, all_zero_reg);

            m_temp_reg_44 = _mm_add_epi16(m_temp_reg_82, m_temp_reg_0);
            m_temp_reg_20 = _mm_packus_epi16(m_temp_reg_40, m_temp_reg_44);

            _mm_storeu_si128((__m128i *)(pu1_dst + 16), m_temp_reg_20);
            pu1_dst += out_stride;
            pu1_pred += pred_strd;


            m_temp_reg_20 = _mm_loadu_si128((__m128i *)pu1_pred);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_20);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_20, all_zero_reg);

            m_temp_reg_40 = _mm_add_epi16(m_temp_reg_81, m_temp_reg_0);
            m_temp_reg_0 = _mm_srli_si128(m_temp_reg_20, 8);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_0);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_0, all_zero_reg);

            m_temp_reg_44 = _mm_add_epi16(m_temp_reg_85, m_temp_reg_0);
            m_temp_reg_20 = _mm_packus_epi16(m_temp_reg_40, m_temp_reg_44);

            _mm_storeu_si128((__m128i *)pu1_dst, m_temp_reg_20);

            m_temp_reg_20 = _mm_loadu_si128((__m128i *)(pu1_pred + 16));

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_20);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_20, all_zero_reg);

            m_temp_reg_40 = _mm_add_epi16(m_temp_reg_87, m_temp_reg_0);
            m_temp_reg_0 = _mm_srli_si128(m_temp_reg_20, 8);

            //m_temp_reg_0 = _mm_cvtepu8_epi16(m_temp_reg_0);
            m_temp_reg_0 = _mm_unpacklo_epi8(m_temp_reg_0, all_zero_reg);

            m_temp_reg_44 = _mm_add_epi16(m_temp_reg_83, m_temp_reg_0);
            m_temp_reg_20 = _mm_packus_epi16(m_temp_reg_40, m_temp_reg_44);

            _mm_storeu_si128((__m128i *)(pu1_dst + 16), m_temp_reg_20);
            pu1_dst += out_stride;
            pu1_pred += pred_strd;

        }
        pi2_tmp += 4;
    }
}

