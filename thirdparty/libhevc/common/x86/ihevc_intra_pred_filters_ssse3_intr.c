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
*  ihevc_intra_pred_filters_atom_intr.c
*
* @brief
*  Contains function Definition for intra prediction  interpolation filters
*
*
* @author
* Ittiam
*
* @par List of Functions:
*  - ihevc_intra_pred_luma_planar_ssse3()
*  - ihevc_intra_pred_luma_dc_ssse3()
*  - ihevc_intra_pred_luma_horz_ssse3()
*  - ihevc_intra_pred_luma_ver_ssse3()
*  - ihevc_intra_pred_luma_mode2_ssse3()
*  - ihevc_intra_pred_luma_mode_18_34_ssse3()
*  - ihevc_intra_pred_luma_mode_3_to_9_ssse3()
*  - ihevc_intra_pred_luma_mode_11_to_17_ssse3()
*  - ihevc_intra_pred_luma_mode_19_to_25_ssse3()
*  - ihevc_intra_pred_luma_mode_27_to_33_ssse3()
*  - ihevc_intra_pred_luma_ref_substitution_ssse3()
*
* @remarks
*  None
*
*******************************************************************************
*/


/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/
#include <stdlib.h>

#include "ihevc_typedefs.h"
#include "ihevc_intra_pred.h"
#include "ihevc_platform_macros.h"
#include "ihevc_macros.h"
#include "ihevc_func_selector.h"
#include "ihevc_common_tables.h"
#include "ihevc_defs.h"
#include "ihevc_tables_x86_intr.h"

#include <immintrin.h>

/****************************************************************************/
/* Constant Macros                                                          */
/****************************************************************************/
#define MAX_CU_SIZE 64
#define BIT_DEPTH 8
#define T32_4NT 128
#define T16_4NT 64


/****************************************************************************/
/* Function Macros                                                          */
/****************************************************************************/
#define GET_BITS(y,x) ((y) & (1 << x)) && (1 << x)

/* tables to shuffle 8-bit values */


/*****************************************************************************/
/* global tables Definition                                                  */
/*****************************************************************************/


/*****************************************************************************/
/* Function Definition                                                      */
/*****************************************************************************/


/**
*******************************************************************************
*
* @brief
*    Intra prediction interpolation filter for pu1_ref substitution
*
*
* @par Description:
*    Reference substitution process for samples unavailable  for prediction
*    Refer to section 8.4.4.2.2
*
* @param[in] pu1_top_left
*  UWORD8 pointer to the top-left
*
* @param[in] pu1_top
*  UWORD8 pointer to the top
*
* @param[in] pu1_left
*  UWORD8 pointer to the left
*
* @param[in] src_strd
*  WORD32 Source stride
*
* @param[in] nbr_flags
*  WORD32 neighbor availability flags
*
* @param[in] nt
*  WORD32 transform Block size
*
* @param[in] dst_strd
*  WORD32 Destination stride
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_intra_pred_luma_ref_substitution_ssse3(UWORD8 *pu1_top_left,
                                                  UWORD8 *pu1_top,
                                                  UWORD8 *pu1_left,
                                                  WORD32 src_strd,
                                                  WORD32 nt,
                                                  WORD32 nbr_flags,
                                                  UWORD8 *pu1_dst,
                                                  WORD32 dst_strd)
{
    UWORD8 pu1_ref;
    WORD32 dc_val, i;
    WORD32 total_samples = (4 * nt) + 1;
    WORD32 two_nt = 2 * nt;

    WORD32 three_nt = 3 * nt;
    WORD32 get_bits;
    WORD32 next;
    WORD32 bot_left, left, top, tp_right, tp_left;

    WORD32 idx, nbr_id_from_bl, frwd_nbr_flag;
    UNUSED(dst_strd);

    dc_val = 1 << (BIT_DEPTH - 1);


    /* Neighbor Flag Structure*/
    /* MSB ---> LSB */
    /*    Top-Left | Top-Right | Top | Left | Bottom-Left
              1         4         4     4         4
     */
    /* If no neighbor flags are present, fill the neighbor samples with DC value */
    if(nbr_flags == 0)
    {
        for(i = 0; i < total_samples; i++)
        {
            pu1_dst[i] = dc_val;
        }
    }
    else
    {
        /* Else fill the corresponding samples */
        pu1_dst[two_nt] = *pu1_top_left;
        for(i = 0; i < two_nt; i++)
            pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
        for(i = 0; i < two_nt; i++)
            pu1_dst[two_nt + 1 + i] = pu1_top[i];

        if(nt <= 8)
        {
            /* 1 bit extraction for all the neighboring blocks */
            tp_left = (nbr_flags & 0x10000) >> 16;
            bot_left = (nbr_flags & 0x8) >> 3;
            left = (nbr_flags & 0x80) >> 7;
            top = (nbr_flags & 0x100) >> 8;
            tp_right = (nbr_flags & 0x1000) >> 12;

            next = 1;

            /* If bottom -left is not available, reverse substitution process*/
            if(bot_left == 0)
            {
                WORD32 a_nbr_flag[5] = { bot_left, left, tp_left, top, tp_right };

                /* Check for the 1st available sample from bottom-left*/
                while(!a_nbr_flag[next])
                    next++;

                /* If Left, top-left are available*/
                if(next <= 2)
                {
                    idx = nt * next;
                    pu1_ref = pu1_dst[idx];
                    for(i = 0; i < idx; i++)
                        pu1_dst[i] = pu1_ref;
                }
                else /* If top, top-right are available */
                {
                    /* Idx is changed to copy 1 pixel value for top-left ,if top-left is not available*/
                    idx = (nt * (next - 1)) + 1;
                    pu1_ref = pu1_dst[idx];
                    for(i = 0; i < idx; i++)
                        pu1_dst[i] = pu1_ref;
                }
            }

            /* Forward Substitution Process */
            /* If left is Unavailable, copy the last bottom-left value */
            if(left == 0)
            {
                for(i = 0; i < nt; i++)
                    pu1_dst[nt + i] = pu1_dst[nt - 1];
            }
            /* If top-left is Unavailable, copy the last left value */
            if(tp_left == 0)
                pu1_dst[two_nt] = pu1_dst[two_nt - 1];
            /* If top is Unavailable, copy the last top-left value */
            if(top == 0)
            {
                for(i = 0; i < nt; i++)
                    pu1_dst[two_nt + 1 + i] = pu1_dst[two_nt];
            }
            /* If to right is Unavailable, copy the last top value */
            if(tp_right == 0)
            {
                for(i = 0; i < nt; i++)
                    pu1_dst[three_nt + 1 + i] = pu1_dst[three_nt];
            }
        }

        if(nt == 16)
        {
            WORD32 nbr_flags_temp = 0;
            nbr_flags_temp = ((nbr_flags & 0xC) >> 2) + ((nbr_flags & 0xC0) >> 4)
                            + ((nbr_flags & 0x300) >> 4)
                            + ((nbr_flags & 0x3000) >> 6)
                            + ((nbr_flags & 0x10000) >> 8);

            /* compute trailing zeors based on nbr_flag for substitution process of below left see section .*/
            /* as each bit in nbr flags corresponds to 8 pels for bot_left, left, top and topright but 1 pel for topleft */
            {
                nbr_id_from_bl = look_up_trailing_zeros(nbr_flags_temp & 0XF) * 8; /* for below left and left */

                if(nbr_id_from_bl == 64)
                    nbr_id_from_bl = 32;

                if(nbr_id_from_bl == 32)
                {
                    /* for top left : 1 pel per nbr bit */
                    if(!((nbr_flags_temp >> 8) & 0x1))
                    {
                        nbr_id_from_bl++;
                        nbr_id_from_bl += look_up_trailing_zeros((nbr_flags_temp >> 4) & 0xF) * 8; /* top and top right;  8 pels per nbr bit */
                        //nbr_id_from_bl += idx * 8;
                    }
                }
                /* Reverse Substitution Process*/
                if(nbr_id_from_bl)
                {
                    /* Replicate the bottom-left and subsequent unavailable pixels with the 1st available pixel above */
                    pu1_ref = pu1_dst[nbr_id_from_bl];
                    for(i = (nbr_id_from_bl - 1); i >= 0; i--)
                    {
                        pu1_dst[i] = pu1_ref;
                    }
                }
            }

            /* for the loop of 4*Nt+1 pixels (excluding pixels computed from reverse substitution) */
            while(nbr_id_from_bl < ((T16_4NT) + 1))
            {
                /* To Obtain the next unavailable idx flag after reverse neighbor substitution  */
                /* Devide by 8 to obtain the original index */
                frwd_nbr_flag = (nbr_id_from_bl >> 3); /*+ (nbr_id_from_bl & 0x1);*/

                /* The Top-left flag is at the last bit location of nbr_flags*/
                if(nbr_id_from_bl == (T16_4NT / 2))
                {
                    get_bits = GET_BITS(nbr_flags_temp, 8);

                    /* only pel substitution for TL */
                    if(!get_bits)
                        pu1_dst[nbr_id_from_bl] = pu1_dst[nbr_id_from_bl - 1];
                }
                else
                {
                    get_bits = GET_BITS(nbr_flags_temp, frwd_nbr_flag);
                    if(!get_bits)
                    {
                        /* 8 pel substitution (other than TL) */
                        pu1_ref = pu1_dst[nbr_id_from_bl - 1];
                        for(i = 0; i < 8; i++)
                            pu1_dst[nbr_id_from_bl + i] = pu1_ref;
                    }

                }
                nbr_id_from_bl += (nbr_id_from_bl == (T16_4NT / 2)) ? 1 : 8;
            }


        }

        if(nt == 32)
        {
            /* compute trailing ones based on mbr_flag for substitution process of below left see section .*/
            /* as each bit in nbr flags corresponds to 8 pels for bot_left, left, top and topright but 1 pel for topleft */
            {
                nbr_id_from_bl = look_up_trailing_zeros((nbr_flags & 0XFF)) * 8; /* for below left and left */

                if(nbr_id_from_bl == 64)
                {
                    /* for top left : 1 pel per nbr bit */
                    if(!((nbr_flags >> 16) & 0x1))
                    {
                        /* top left not available */
                        nbr_id_from_bl++;
                        /* top and top right;  8 pels per nbr bit */
                        nbr_id_from_bl += look_up_trailing_zeros((nbr_flags >> 8) & 0xFF) * 8;
                    }
                }
                /* Reverse Substitution Process*/
                if(nbr_id_from_bl)
                {
                    /* Replicate the bottom-left and subsequent unavailable pixels with the 1st available pixel above */
                    pu1_ref = pu1_dst[nbr_id_from_bl];
                    for(i = (nbr_id_from_bl - 1); i >= 0; i--)
                        pu1_dst[i] = pu1_ref;
                }
            }

            /* for the loop of 4*Nt+1 pixels (excluding pixels computed from reverse substitution) */
            while(nbr_id_from_bl < ((T32_4NT) + 1))
            {
                /* To Obtain the next unavailable idx flag after reverse neighbor substitution  */
                /* Devide by 8 to obtain the original index */
                frwd_nbr_flag = (nbr_id_from_bl >> 3); /*+ (nbr_id_from_bl & 0x1);*/

                /* The Top-left flag is at the last bit location of nbr_flags*/
                if(nbr_id_from_bl == (T32_4NT / 2))
                {
                    get_bits = GET_BITS(nbr_flags, 16);
                    /* only pel substitution for TL */
                    if(!get_bits)
                        pu1_dst[nbr_id_from_bl] = pu1_dst[nbr_id_from_bl - 1];
                }
                else
                {
                    get_bits = GET_BITS(nbr_flags, frwd_nbr_flag);
                    if(!get_bits)
                    {
                        /* 8 pel substitution (other than TL) */
                        pu1_ref = pu1_dst[nbr_id_from_bl - 1];
                        for(i = 0; i < 8; i++)
                            pu1_dst[nbr_id_from_bl + i] = pu1_ref;
                    }

                }
                nbr_id_from_bl += (nbr_id_from_bl == (T32_4NT / 2)) ? 1 : 8;
            }
        }

    }
}

/**
*******************************************************************************
*
* @brief
*    Intra prediction interpolation filter for ref_filtering
*
*
* @par Description:
*    Reference DC filtering for neighboring samples dependent  on TU size and
*    mode  Refer to section 8.4.4.2.3 in the standard
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pu1_dst
*  UWORD8 pointer to the destination
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

void ihevc_intra_pred_ref_filtering_ssse3(UWORD8 *pu1_src,
                                          WORD32 nt,
                                          UWORD8 *pu1_dst,
                                          WORD32 mode,
                                          WORD32 strong_intra_smoothing_enable_flag)
{
    WORD32 filter_flag;
    WORD32 i; /* Generic indexing variable */
    WORD32 four_nt = 4 * nt;
    UWORD8 au1_flt[(4 * MAX_CU_SIZE) + 1];
    WORD32 bi_linear_int_flag = 0;
    WORD32 abs_cond_left_flag = 0;
    WORD32 abs_cond_top_flag = 0;
    WORD32 dc_val = 1 << (BIT_DEPTH - 5);
    __m128i src_temp1, src_temp2, src_temp3, src_temp7;
    __m128i src_temp4, src_temp5, src_temp6, src_temp8;

    //WORD32 strong_intra_smoothing_enable_flag  = 1;

    filter_flag = gau1_intra_pred_ref_filter[mode] & (1 << (CTZ(nt) - 2));
    if(0 == filter_flag)
    {
        if(pu1_src == pu1_dst)
        {
            return;
        }
        else
        {
            if(nt == 4)
            {
                src_temp1 = _mm_loadu_si128((__m128i *)(pu1_src));
                _mm_storeu_si128((__m128i *)(pu1_dst), src_temp1);
                pu1_dst[four_nt] = pu1_src[four_nt];

            }

            else if(nt == 8)
            {

                src_temp1 = _mm_loadu_si128((__m128i *)(pu1_src));
                src_temp2 = _mm_loadu_si128((__m128i *)(pu1_src + 16));

                _mm_storeu_si128((__m128i *)(pu1_dst), src_temp1);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16), src_temp2);


                pu1_dst[four_nt] = pu1_src[four_nt];
            }
            else if(nt == 16)
            {

                src_temp1 = _mm_loadu_si128((__m128i *)(pu1_src));
                src_temp2 = _mm_loadu_si128((__m128i *)(pu1_src + 16));
                src_temp3 = _mm_loadu_si128((__m128i *)(pu1_src + 32));
                src_temp4 = _mm_loadu_si128((__m128i *)(pu1_src + 48));

                _mm_storeu_si128((__m128i *)(pu1_dst), src_temp1);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16), src_temp2);
                _mm_storeu_si128((__m128i *)(pu1_dst + 32), src_temp3);
                _mm_storeu_si128((__m128i *)(pu1_dst + 48), src_temp4);

                pu1_dst[four_nt] = pu1_src[four_nt];
            }
            else if(nt == 32)
            {

                src_temp1 = _mm_loadu_si128((__m128i *)(pu1_src));
                src_temp2 = _mm_loadu_si128((__m128i *)(pu1_src + 16));
                src_temp3 = _mm_loadu_si128((__m128i *)(pu1_src + 32));
                src_temp4 = _mm_loadu_si128((__m128i *)(pu1_src + 48));

                src_temp5 = _mm_loadu_si128((__m128i *)(pu1_src + 64));
                src_temp6 = _mm_loadu_si128((__m128i *)(pu1_src + 80));
                src_temp7 = _mm_loadu_si128((__m128i *)(pu1_src + 96));
                src_temp8 = _mm_loadu_si128((__m128i *)(pu1_src + 112));

                _mm_storeu_si128((__m128i *)(pu1_dst), src_temp1);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16), src_temp2);
                _mm_storeu_si128((__m128i *)(pu1_dst + 32), src_temp3);
                _mm_storeu_si128((__m128i *)(pu1_dst + 48), src_temp4);

                _mm_storeu_si128((__m128i *)(pu1_dst + 64), src_temp5);
                _mm_storeu_si128((__m128i *)(pu1_dst + 80), src_temp6);
                _mm_storeu_si128((__m128i *)(pu1_dst + 96), src_temp7);
                _mm_storeu_si128((__m128i *)(pu1_dst + 112), src_temp8);

                pu1_dst[four_nt] = pu1_src[four_nt];
            }

        }
    }

    else
    {
        /* If strong intra smoothin is enabled and transform size is 32 */
        if((1 == strong_intra_smoothing_enable_flag) && (32 == nt))
        {
            /* Strong Intra Filtering */
            abs_cond_top_flag = (abs(pu1_src[2 * nt] + pu1_src[4 * nt]
                            - (2 * pu1_src[3 * nt]))) < dc_val;
            abs_cond_left_flag = (abs(pu1_src[2 * nt] + pu1_src[0]
                            - (2 * pu1_src[nt]))) < dc_val;

            bi_linear_int_flag = ((1 == abs_cond_left_flag)
                            && (1 == abs_cond_top_flag));
        }
        /* Extremities Untouched*/
        au1_flt[0] = pu1_src[0];
        au1_flt[4 * nt] = pu1_src[4 * nt];

        /* Strong filtering of reference samples */
        if(1 == bi_linear_int_flag)
        {
            au1_flt[2 * nt] = pu1_src[2 * nt];

            for(i = 1; i < (2 * nt); i++)
                au1_flt[i] = (((2 * nt) - i) * pu1_src[0] + i * pu1_src[2 * nt] + 32) >> 6;

            for(i = 1; i < (2 * nt); i++)
                au1_flt[i + (2 * nt)] = (((2 * nt) - i) * pu1_src[2 * nt] + i * pu1_src[4 * nt] + 32) >> 6;
        }
        else
        {
            __m128i const_value_8x16, zero_8x16b;

            const_value_8x16 = _mm_set1_epi16(2);

            au1_flt[0] = pu1_src[0];
            au1_flt[4 * nt] = pu1_src[4 * nt];

            zero_8x16b = _mm_setzero_si128();

            /* Perform bilinear filtering of Reference Samples */
            for(i = 0; i < (four_nt); i += 16)
            {
                src_temp1 = _mm_loadu_si128((__m128i *)(pu1_src + i));
                src_temp2 = _mm_srli_si128(src_temp1, 1);
                src_temp3 = _mm_srli_si128(src_temp2, 1);

                src_temp1 = _mm_unpacklo_epi8(src_temp1, zero_8x16b);
                src_temp2 = _mm_unpacklo_epi8(src_temp2, zero_8x16b);
                src_temp3 = _mm_unpacklo_epi8(src_temp3, zero_8x16b);

                src_temp2 = _mm_slli_epi16(src_temp2,  1);

                src_temp1 = _mm_add_epi16(src_temp1, src_temp2);
                src_temp1 = _mm_add_epi16(src_temp1, src_temp3);
                src_temp1 = _mm_add_epi16(src_temp1, const_value_8x16);

                src_temp1 = _mm_srai_epi16(src_temp1,  2);

                src_temp4 = _mm_loadu_si128((__m128i *)(pu1_src + 8 + i));
                src_temp5 = _mm_srli_si128(src_temp4, 1);
                src_temp6 = _mm_srli_si128(src_temp5, 1);

                src_temp4 = _mm_unpacklo_epi8(src_temp4, zero_8x16b);
                src_temp5 = _mm_unpacklo_epi8(src_temp5, zero_8x16b);
                src_temp6 = _mm_unpacklo_epi8(src_temp6, zero_8x16b);

                src_temp5 = _mm_slli_epi16(src_temp5,  1);

                src_temp4 = _mm_add_epi16(src_temp4, src_temp5);
                src_temp4 = _mm_add_epi16(src_temp4, src_temp6);
                src_temp4 = _mm_add_epi16(src_temp4, const_value_8x16);

                src_temp4 = _mm_srai_epi16(src_temp4,  2);

                /* converting 16 bit to 8 bit */
                src_temp1 = _mm_packus_epi16(src_temp1, src_temp4);

                _mm_storeu_si128((__m128i *)(au1_flt + 1 + i), src_temp1);
            }
            au1_flt[4 * nt] = pu1_src[4 * nt];
        }

        if(nt == 4)
        {
            src_temp1 = _mm_loadu_si128((__m128i *)(au1_flt));
            _mm_storeu_si128((__m128i *)(pu1_dst), src_temp1);
            pu1_dst[four_nt] = au1_flt[four_nt];
        }
        else if(nt == 8)
        {

            src_temp1 = _mm_loadu_si128((__m128i *)(au1_flt));
            src_temp2 = _mm_loadu_si128((__m128i *)(au1_flt + 16));

            _mm_storeu_si128((__m128i *)(pu1_dst), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16), src_temp2);

            pu1_dst[four_nt] = au1_flt[four_nt];
        }
        else if(nt == 16)
        {

            src_temp1 = _mm_loadu_si128((__m128i *)(au1_flt));
            src_temp2 = _mm_loadu_si128((__m128i *)(au1_flt + 16));
            src_temp3 = _mm_loadu_si128((__m128i *)(au1_flt + 32));
            src_temp4 = _mm_loadu_si128((__m128i *)(au1_flt + 48));

            _mm_storeu_si128((__m128i *)(pu1_dst), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16), src_temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 32), src_temp3);
            _mm_storeu_si128((__m128i *)(pu1_dst + 48), src_temp4);

            pu1_dst[four_nt] = au1_flt[four_nt];
        }

        else if(nt == 32)
        {

            src_temp1 = _mm_loadu_si128((__m128i *)(au1_flt));
            src_temp2 = _mm_loadu_si128((__m128i *)(au1_flt + 16));
            src_temp3 = _mm_loadu_si128((__m128i *)(au1_flt + 32));
            src_temp4 = _mm_loadu_si128((__m128i *)(au1_flt + 48));

            src_temp5 = _mm_loadu_si128((__m128i *)(au1_flt + 64));
            src_temp6 = _mm_loadu_si128((__m128i *)(au1_flt + 80));
            src_temp7 = _mm_loadu_si128((__m128i *)(au1_flt + 96));
            src_temp8 = _mm_loadu_si128((__m128i *)(au1_flt + 112));

            _mm_storeu_si128((__m128i *)(pu1_dst), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16), src_temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 32), src_temp3);
            _mm_storeu_si128((__m128i *)(pu1_dst + 48), src_temp4);

            _mm_storeu_si128((__m128i *)(pu1_dst + 64), src_temp5);
            _mm_storeu_si128((__m128i *)(pu1_dst + 80), src_temp6);
            _mm_storeu_si128((__m128i *)(pu1_dst + 96), src_temp7);
            _mm_storeu_si128((__m128i *)(pu1_dst + 112), src_temp8);

            pu1_dst[four_nt] = au1_flt[four_nt];
        }

    }
}

/**
*******************************************************************************
*
* @brief
*    Intra prediction interpolation filter for luma planar
*
* @par Description:
*    Planar Intraprediction with reference neighboring samples location
*    pointed by 'pu1_ref' to the TU block location  pointed by 'pu1_dst'  Refer
*    to section 8.4.4.2.4 in the standard
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


void ihevc_intra_pred_luma_planar_ssse3(UWORD8 *pu1_ref,
                                        WORD32 src_strd,
                                        UWORD8 *pu1_dst,
                                        WORD32 dst_strd,
                                        WORD32 nt,
                                        WORD32 mode)
{


    WORD32 row, col;
    WORD32 two_nt, three_nt;
    UWORD16 temp;

    __m128i pu1_ref_16x8b, const_temp_4x32b, const_temp1_4x32b, const_temp2_4x32b, const_temp3_4x32b, const_temp4_4x32b;
    __m128i col_8x16b, const_temp5_4x32b, const_temp6_4x32b, zero_8x16b, const_temp7_4x32b, const_temp8_4x32b;
    __m128i nt_row_16x8b, nt_row1_16x8b, nt_row2_16x8b, nt_row3_16x8b; //nt-1-row
    __m128i row_16x8b, row1_16x8b, row2_16x8b, row3_16x8b; //row+1
    UNUSED(src_strd);
    UNUSED(mode);

    two_nt = 2 * nt;
    three_nt = 3 * nt;

    /* Planar filtering */
    temp = pu1_ref[nt - 1];
    temp = (temp << 8) | ((UWORD16)pu1_ref[three_nt + 1]);
    /* setting vallues in  registera*/
    pu1_ref_16x8b  = _mm_set1_epi16(temp);
    const_temp6_4x32b = _mm_set1_epi16(nt);



    if(nt == 32) /* for nt multiple of 8*/
    {


        const_temp4_4x32b = _mm_set1_epi16(0x0400);
        const_temp1_4x32b = _mm_set1_epi16(0x0100);
        const_temp8_4x32b = _mm_set1_epi16(0x0008);
        //(nt-1-y) (nt-1-x) ; x= 0..15 , y = row
        //const_temp5_4x32b = _mm_set_epi8(nt_row, 0,nt_row, 1,nt_row, 2,nt_row, 3,nt_row, 4,nt_row, 5,nt_row, 6,nt_row, 7);
        nt_row_16x8b = _mm_set_epi16(0x1f18, 0x1f19, 0x1f1a, 0x1f1b, 0x1f1c, 0x1f1d, 0x1f1e, 0x1f1f);
        //(y+1) (x+1) ; x= 0..15 , y = row
        //const_temp3_4x32b = _mm_set_epi16(row1,8,row1, 7,row1, 6, row1, 5,row1, 4, row1, 3, row1, 2, row1, 1);
        row_16x8b = _mm_set_epi16(0x0108, 0x0107, 0x0106, 0x0105, 0x0104, 0x0103, 0x0102, 0x0101);

        for(row = 0; row < nt; row += 1)
        {
            __m128i res_temp_8x16b, res_temp1_8x16b, res_temp2_8x16b, res_temp3_8x16b;
            __m128i res_temp4_8x16b, res_temp5_8x16b, res_temp6_8x16b, res_temp7_8x16b;

            __m128i src_temp_8x16b, src_temp1_8x16b;


            res_temp1_8x16b  = _mm_set1_epi8(pu1_ref[two_nt - 1 - row]);

            nt_row1_16x8b = _mm_sub_epi16(nt_row_16x8b,  const_temp8_4x32b);
            row1_16x8b    = _mm_add_epi16(row_16x8b,     const_temp8_4x32b);
            nt_row2_16x8b = _mm_sub_epi16(nt_row1_16x8b, const_temp8_4x32b);
            row2_16x8b    = _mm_add_epi16(row1_16x8b,    const_temp8_4x32b);
            nt_row3_16x8b = _mm_sub_epi16(nt_row2_16x8b, const_temp8_4x32b);
            row3_16x8b    = _mm_add_epi16(row2_16x8b,    const_temp8_4x32b);
            /* loding 8bit 16 pixles*/
            src_temp_8x16b  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1));
            src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 17));

            res_temp4_8x16b =  _mm_unpacklo_epi8(res_temp1_8x16b, src_temp_8x16b); /* row=0*/
            res_temp5_8x16b =  _mm_unpackhi_epi8(res_temp1_8x16b, src_temp_8x16b); /* row=1*/
            res_temp6_8x16b =  _mm_unpacklo_epi8(res_temp1_8x16b, src_temp1_8x16b); /* row=2*/
            res_temp7_8x16b =  _mm_unpackhi_epi8(res_temp1_8x16b, src_temp1_8x16b); /* row=3*/

            /*(row + 1) * pu1_ref[nt - 1] + (col + 1) * pu1_ref[three_nt + 1] */
            res_temp_8x16b  = _mm_maddubs_epi16(pu1_ref_16x8b, row_16x8b);
            res_temp1_8x16b = _mm_maddubs_epi16(pu1_ref_16x8b, row1_16x8b);
            res_temp2_8x16b = _mm_maddubs_epi16(pu1_ref_16x8b, row2_16x8b);
            res_temp3_8x16b = _mm_maddubs_epi16(pu1_ref_16x8b, row3_16x8b);
            /* (nt - 1 - row) * pu1_ref[two_nt + 1 + col] + (nt - 1 - col)* pu1_ref[two_nt - 1 - row] */
            res_temp4_8x16b = _mm_maddubs_epi16(res_temp4_8x16b, nt_row_16x8b);
            res_temp5_8x16b = _mm_maddubs_epi16(res_temp5_8x16b, nt_row1_16x8b);
            res_temp6_8x16b = _mm_maddubs_epi16(res_temp6_8x16b, nt_row2_16x8b);
            res_temp7_8x16b = _mm_maddubs_epi16(res_temp7_8x16b, nt_row3_16x8b);

            res_temp_8x16b  = _mm_add_epi16(res_temp_8x16b, res_temp4_8x16b);
            res_temp1_8x16b = _mm_add_epi16(res_temp1_8x16b, res_temp5_8x16b);
            res_temp2_8x16b = _mm_add_epi16(res_temp2_8x16b, res_temp6_8x16b);
            res_temp3_8x16b = _mm_add_epi16(res_temp3_8x16b, res_temp7_8x16b);
            /*res_temp + nt)*/
            res_temp_8x16b  = _mm_add_epi16(res_temp_8x16b, const_temp6_4x32b);
            res_temp1_8x16b = _mm_add_epi16(res_temp1_8x16b, const_temp6_4x32b);
            res_temp2_8x16b = _mm_add_epi16(res_temp2_8x16b, const_temp6_4x32b);
            res_temp3_8x16b = _mm_add_epi16(res_temp3_8x16b, const_temp6_4x32b);

            res_temp_8x16b  = _mm_srli_epi16(res_temp_8x16b, 6); //log2(32)+1
            res_temp1_8x16b = _mm_srli_epi16(res_temp1_8x16b, 6);
            res_temp2_8x16b = _mm_srli_epi16(res_temp2_8x16b, 6);
            res_temp3_8x16b = _mm_srli_epi16(res_temp3_8x16b, 6);

            res_temp_8x16b  = _mm_packus_epi16(res_temp_8x16b, res_temp1_8x16b);
            res_temp1_8x16b = _mm_packus_epi16(res_temp2_8x16b, res_temp3_8x16b);


            _mm_storeu_si128((__m128i *)(pu1_dst + (row * dst_strd)), res_temp_8x16b);
            _mm_storeu_si128((__m128i *)(pu1_dst + (row * dst_strd) + 16), res_temp1_8x16b);


            nt_row_16x8b = _mm_sub_epi16(nt_row_16x8b, const_temp1_4x32b);
            row_16x8b    = _mm_add_epi16(row_16x8b,    const_temp1_4x32b);
        }
    }
    else if(nt == 16) /* for nt multiple of 8*/
    {

        const_temp4_4x32b = _mm_set1_epi16(0x0400);
        const_temp1_4x32b = _mm_set1_epi16(0x0100);
        const_temp8_4x32b = _mm_set1_epi16(0x0008);
        //(nt-1-y) (nt-1-x) ; x= 0..15 , y = row
        //const_temp5_4x32b = _mm_set_epi8(nt_row, 0,nt_row, 1,nt_row, 2,nt_row, 3,nt_row, 4,nt_row, 5,nt_row, 6,nt_row, 7);
        nt_row_16x8b = _mm_set_epi16(0x0f08, 0x0f09, 0x0f0a, 0x0f0b, 0x0f0c, 0x0f0d, 0x0f0e, 0x0f0f);
        //(y+1) (x+1) ; x= 0..15 , y = row
        //const_temp3_4x32b = _mm_set_epi16(row1,8,row1, 7,row1, 6, row1, 5,row1, 4, row1, 3, row1, 2, row1, 1);
        row_16x8b = _mm_set_epi16(0x0108, 0x0107, 0x0106, 0x0105, 0x0104, 0x0103, 0x0102, 0x0101);

        for(row = 0; row < nt; row += 2)
        {
            __m128i res_temp_8x16b, res_temp1_8x16b, res_temp2_8x16b, res_temp3_8x16b;
            __m128i res_temp4_8x16b, res_temp5_8x16b, res_temp6_8x16b, res_temp7_8x16b;

            __m128i src_temp_8x16b;


            res_temp1_8x16b  = _mm_set1_epi8(pu1_ref[two_nt - 1 - row]);
            res_temp2_8x16b  = _mm_set1_epi8(pu1_ref[two_nt - 2 - row]);


            nt_row1_16x8b = _mm_sub_epi16(nt_row_16x8b,  const_temp1_4x32b);
            row1_16x8b    = _mm_add_epi16(row_16x8b,     const_temp1_4x32b);
            nt_row2_16x8b = _mm_sub_epi16(nt_row_16x8b,  const_temp8_4x32b);
            row2_16x8b    = _mm_add_epi16(row_16x8b,     const_temp8_4x32b);
            nt_row3_16x8b = _mm_sub_epi16(nt_row1_16x8b, const_temp8_4x32b);
            row3_16x8b    = _mm_add_epi16(row1_16x8b,    const_temp8_4x32b);
            /* loding 8bit 16 pixles*/
            src_temp_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1));


            res_temp4_8x16b =  _mm_unpacklo_epi8(res_temp1_8x16b, src_temp_8x16b); /* row=0*/
            res_temp5_8x16b =  _mm_unpacklo_epi8(res_temp2_8x16b, src_temp_8x16b); /* row=1*/
            res_temp6_8x16b =  _mm_unpackhi_epi8(res_temp1_8x16b, src_temp_8x16b); /* row=2*/
            res_temp7_8x16b =  _mm_unpackhi_epi8(res_temp2_8x16b, src_temp_8x16b); /* row=3*/

            /*(row + 1) * pu1_ref[nt - 1] + (col + 1) * pu1_ref[three_nt + 1] */
            res_temp_8x16b  = _mm_maddubs_epi16(pu1_ref_16x8b, row_16x8b);
            res_temp1_8x16b = _mm_maddubs_epi16(pu1_ref_16x8b, row1_16x8b);
            res_temp2_8x16b = _mm_maddubs_epi16(pu1_ref_16x8b, row2_16x8b);
            res_temp3_8x16b = _mm_maddubs_epi16(pu1_ref_16x8b, row3_16x8b);
            /* (nt - 1 - row) * pu1_ref[two_nt + 1 + col] + (nt - 1 - col)* pu1_ref[two_nt - 1 - row] */
            res_temp4_8x16b = _mm_maddubs_epi16(res_temp4_8x16b, nt_row_16x8b);
            res_temp5_8x16b = _mm_maddubs_epi16(res_temp5_8x16b, nt_row1_16x8b);
            res_temp6_8x16b = _mm_maddubs_epi16(res_temp6_8x16b, nt_row2_16x8b);
            res_temp7_8x16b = _mm_maddubs_epi16(res_temp7_8x16b, nt_row3_16x8b);

            res_temp_8x16b  = _mm_add_epi16(res_temp_8x16b, res_temp4_8x16b);
            res_temp1_8x16b = _mm_add_epi16(res_temp1_8x16b, res_temp5_8x16b);
            res_temp2_8x16b = _mm_add_epi16(res_temp2_8x16b, res_temp6_8x16b);
            res_temp3_8x16b = _mm_add_epi16(res_temp3_8x16b, res_temp7_8x16b);
            /*res_temp + nt)*/
            res_temp_8x16b  = _mm_add_epi16(res_temp_8x16b, const_temp6_4x32b);
            res_temp1_8x16b = _mm_add_epi16(res_temp1_8x16b, const_temp6_4x32b);
            res_temp2_8x16b = _mm_add_epi16(res_temp2_8x16b, const_temp6_4x32b);
            res_temp3_8x16b = _mm_add_epi16(res_temp3_8x16b, const_temp6_4x32b);

            res_temp_8x16b  = _mm_srli_epi16(res_temp_8x16b, 5); //log2(16)+1
            res_temp1_8x16b = _mm_srli_epi16(res_temp1_8x16b, 5);
            res_temp2_8x16b = _mm_srli_epi16(res_temp2_8x16b, 5);
            res_temp3_8x16b = _mm_srli_epi16(res_temp3_8x16b, 5);

            res_temp_8x16b  = _mm_packus_epi16(res_temp_8x16b, res_temp2_8x16b);
            res_temp1_8x16b = _mm_packus_epi16(res_temp1_8x16b, res_temp3_8x16b);

            _mm_storeu_si128((__m128i *)(pu1_dst + (row * dst_strd)), res_temp_8x16b);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 1) * dst_strd)), res_temp1_8x16b);

            nt_row_16x8b = _mm_sub_epi16(nt_row1_16x8b, const_temp1_4x32b);
            row_16x8b    = _mm_add_epi16(row1_16x8b,    const_temp1_4x32b);
        }
    }
    else if(nt == 8)
    {


        const_temp4_4x32b = _mm_set1_epi16(0x0400);
        const_temp1_4x32b = _mm_set1_epi16(0x0100);
        zero_8x16b = _mm_set1_epi32(0);

        //(nt-1-y) (nt-1-x) ; x= 0..7 , y = row
        //const_temp5_4x32b = _mm_set_epi8(nt_row, 0,nt_row, 1,nt_row, 2,nt_row, 3,nt_row, 4,nt_row, 5,nt_row, 6,nt_row, 7);
        nt_row_16x8b = _mm_set_epi16(0x0700, 0x0701, 0x0702, 0x0703, 0x0704, 0x0705, 0x0706, 0x0707);
        //(y+1) (x+1) ; x= 0..7 , y = row
        //const_temp3_4x32b = _mm_set_epi16(row1,8,row1, 7,row1, 6, row1, 5,row1, 4, row1, 3, row1, 2, row1, 1);
        row_16x8b = _mm_set_epi16(0x0108, 0x0107, 0x0106, 0x0105, 0x0104, 0x0103, 0x0102, 0x0101);

        for(row = 0; row < nt; row += 4)
        {
            __m128i res_temp_8x16b, res_temp1_8x16b, res_temp2_8x16b, res_temp3_8x16b;
            __m128i res_temp4_8x16b, res_temp5_8x16b, res_temp6_8x16b, res_temp7_8x16b;

            __m128i src_temp_8x16b;


            res_temp4_8x16b  = _mm_set1_epi8(pu1_ref[two_nt - 1 - row]);
            res_temp5_8x16b  = _mm_set1_epi8(pu1_ref[two_nt - 2 - row]);
            res_temp6_8x16b  = _mm_set1_epi8(pu1_ref[two_nt - 3 - row]);
            res_temp7_8x16b  = _mm_set1_epi8(pu1_ref[two_nt - 4 - row]);

            nt_row1_16x8b = _mm_sub_epi16(nt_row_16x8b,  const_temp1_4x32b);
            row1_16x8b    = _mm_add_epi16(row_16x8b,     const_temp1_4x32b);
            nt_row2_16x8b = _mm_sub_epi16(nt_row1_16x8b, const_temp1_4x32b);
            row2_16x8b    = _mm_add_epi16(row1_16x8b,    const_temp1_4x32b);
            nt_row3_16x8b = _mm_sub_epi16(nt_row2_16x8b, const_temp1_4x32b);
            row3_16x8b    = _mm_add_epi16(row2_16x8b,    const_temp1_4x32b);
            /* loding 8bit 16 pixles*/
            src_temp_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1));

            res_temp4_8x16b =  _mm_unpacklo_epi8(res_temp4_8x16b, src_temp_8x16b); /* row=0*/
            res_temp5_8x16b =  _mm_unpacklo_epi8(res_temp5_8x16b, src_temp_8x16b); /* row=1*/
            res_temp6_8x16b =  _mm_unpacklo_epi8(res_temp6_8x16b, src_temp_8x16b); /* row=2*/
            res_temp7_8x16b =  _mm_unpacklo_epi8(res_temp7_8x16b, src_temp_8x16b); /* row=3*/

            /*(row + 1) * pu1_ref[nt - 1] + (col + 1) * pu1_ref[three_nt + 1] */
            res_temp_8x16b  = _mm_maddubs_epi16(pu1_ref_16x8b, row_16x8b);
            res_temp1_8x16b = _mm_maddubs_epi16(pu1_ref_16x8b, row1_16x8b);
            res_temp2_8x16b = _mm_maddubs_epi16(pu1_ref_16x8b, row2_16x8b);
            res_temp3_8x16b = _mm_maddubs_epi16(pu1_ref_16x8b, row3_16x8b);
            /* (nt - 1 - row) * pu1_ref[two_nt + 1 + col] + (nt - 1 - col)* pu1_ref[two_nt - 1 - row] */
            res_temp4_8x16b = _mm_maddubs_epi16(res_temp4_8x16b, nt_row_16x8b);
            res_temp5_8x16b = _mm_maddubs_epi16(res_temp5_8x16b, nt_row1_16x8b);
            res_temp6_8x16b = _mm_maddubs_epi16(res_temp6_8x16b, nt_row2_16x8b);
            res_temp7_8x16b = _mm_maddubs_epi16(res_temp7_8x16b, nt_row3_16x8b);

            res_temp_8x16b  = _mm_add_epi16(res_temp_8x16b, res_temp4_8x16b);
            res_temp1_8x16b = _mm_add_epi16(res_temp1_8x16b, res_temp5_8x16b);
            res_temp2_8x16b = _mm_add_epi16(res_temp2_8x16b, res_temp6_8x16b);
            res_temp3_8x16b = _mm_add_epi16(res_temp3_8x16b, res_temp7_8x16b);
            /*res_temp + nt)*/
            res_temp_8x16b  = _mm_add_epi16(res_temp_8x16b, const_temp6_4x32b);
            res_temp1_8x16b = _mm_add_epi16(res_temp1_8x16b, const_temp6_4x32b);
            res_temp2_8x16b = _mm_add_epi16(res_temp2_8x16b, const_temp6_4x32b);
            res_temp3_8x16b = _mm_add_epi16(res_temp3_8x16b, const_temp6_4x32b);

            res_temp_8x16b  = _mm_srli_epi16(res_temp_8x16b, 4); //log2(16)+1
            res_temp1_8x16b = _mm_srli_epi16(res_temp1_8x16b, 4);
            res_temp2_8x16b = _mm_srli_epi16(res_temp2_8x16b, 4);
            res_temp3_8x16b = _mm_srli_epi16(res_temp3_8x16b, 4);

            res_temp_8x16b  = _mm_packus_epi16(res_temp_8x16b, zero_8x16b);
            res_temp1_8x16b = _mm_packus_epi16(res_temp1_8x16b, zero_8x16b);
            res_temp2_8x16b = _mm_packus_epi16(res_temp2_8x16b, zero_8x16b);
            res_temp3_8x16b = _mm_packus_epi16(res_temp3_8x16b, zero_8x16b);

            _mm_storel_epi64((__m128i *)(pu1_dst + (row * dst_strd)), res_temp_8x16b);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((row + 1) * dst_strd)), res_temp1_8x16b);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((row + 2) * dst_strd)), res_temp2_8x16b);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((row + 3) * dst_strd)), res_temp3_8x16b);

            nt_row_16x8b = _mm_sub_epi16(nt_row3_16x8b, const_temp1_4x32b);
            row_16x8b    = _mm_add_epi16(row3_16x8b,    const_temp1_4x32b);
        }
    }
    else
    {

        /* for nt multiple of 4*/
        const_temp7_4x32b = _mm_set1_epi16(4);
        const_temp4_4x32b = _mm_set1_epi16(nt - 1);
        const_temp_4x32b  = _mm_set1_epi16(pu1_ref[three_nt + 1]);
        const_temp1_4x32b = _mm_set1_epi16(pu1_ref[nt - 1]);
        zero_8x16b = _mm_set1_epi32(0);

        for(row = 0; row < nt; row++)
        {
            __m128i res_temp_8x16b, row_8x16b, res_temp1_8x16b, res_temp2_8x16b;
            __m128i res_temp3_8x16b;

            const_temp2_4x32b  = _mm_set1_epi16(pu1_ref[two_nt - 1 - row]);
            const_temp3_4x32b  = _mm_set1_epi16((row + 1));


            row_8x16b = _mm_set1_epi16((nt - 1 - row));

            const_temp5_4x32b = _mm_set_epi16(7, 6, 5, 4, 3, 2, 1, 0);
            col_8x16b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);

            const_temp5_4x32b = _mm_sub_epi16(const_temp4_4x32b, const_temp5_4x32b);

            /*(row + 1) * pu1_ref[nt - 1]*/
            res_temp_8x16b  = _mm_mullo_epi16(const_temp3_4x32b,  const_temp1_4x32b);

            /*(row + 1) * pu1_ref[nt - 1] + nt)*/
            res_temp_8x16b = _mm_add_epi16(res_temp_8x16b, const_temp6_4x32b);

            for(col = 0; col < nt; col += 4)
            {
                __m128i src_temp_8x16b;
                int temp1;

                /* loding 8bit 16 pixles*/
                src_temp_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1 + col));

                src_temp_8x16b =  _mm_unpacklo_epi8(src_temp_8x16b, zero_8x16b); /* row=0*/

                /* (nt - 1 - row) * pu1_ref[two_nt + 1 + col] */
                res_temp1_8x16b  = _mm_mullo_epi16(src_temp_8x16b,  row_8x16b);

                /*(col + 1) * pu1_ref[three_nt + 1]*/
                res_temp2_8x16b  = _mm_mullo_epi16(const_temp_4x32b,  col_8x16b);

                /*(nt - 1 - col)* pu1_ref[two_nt - 1 - row]*/
                res_temp3_8x16b  = _mm_mullo_epi16(const_temp2_4x32b,  const_temp5_4x32b);

                res_temp1_8x16b = _mm_add_epi16(res_temp_8x16b, res_temp1_8x16b);
                res_temp1_8x16b = _mm_add_epi16(res_temp1_8x16b, res_temp2_8x16b);
                res_temp1_8x16b = _mm_add_epi16(res_temp1_8x16b, res_temp3_8x16b);

                res_temp1_8x16b = _mm_srli_epi16(res_temp1_8x16b, 3); //log2(16)+1
                res_temp1_8x16b = _mm_packus_epi16(res_temp1_8x16b, zero_8x16b);

                temp1 = _mm_cvtsi128_si32(res_temp1_8x16b);

                *(WORD32 *)(&pu1_dst[(row * dst_strd) + col]) = temp1;

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
*    Intra prediction interpolation filter for luma dc
*
* @par Description:
*   Intraprediction for DC mode with reference neighboring  samples location
*   pointed by 'pu1_ref' to the TU block  location pointed by 'pu1_dst'  Refer
*   to section 8.4.4.2.5 in the standard
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

void ihevc_intra_pred_luma_dc_ssse3(UWORD8 *pu1_ref,
                                    WORD32 src_strd,
                                    UWORD8 *pu1_dst,
                                    WORD32 dst_strd,
                                    WORD32 nt,
                                    WORD32 mode)
{

    WORD32 acc_dc;
    WORD32 dc_val, two_dc_val, three_dc_val;
    WORD32 row;
    WORD32 log2nt = 5;
    WORD32 two_nt, three_nt;
    __m128i src_temp1, src_temp7, src_temp3, src_temp4, src_temp5, src_temp6;
    __m128i src_temp8, src_temp10, src_temp2;
    __m128i m_zero = _mm_setzero_si128();
    __m128i sm = _mm_load_si128((__m128i *)&IHEVCE_SHUFFLEMASK5[0]);
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
    two_nt = 2 * nt;
    three_nt = 3 * nt;

    acc_dc = 0;
    /* Calculate DC value for the transform block */



    if(nt == 32)
    {
        __m128i temp;
        WORD32 itr_count;

        src_temp3 =  _mm_loadu_si128((__m128i *)(pu1_ref + nt));
        src_temp4 =  _mm_loadu_si128((__m128i *)(pu1_ref + nt + 16));
        src_temp7 =  _mm_loadu_si128((__m128i *)(pu1_ref + nt + 32));
        src_temp8 =  _mm_loadu_si128((__m128i *)(pu1_ref + nt + 48));

        src_temp3 = _mm_sad_epu8(src_temp3, m_zero);
        src_temp4 = _mm_sad_epu8(src_temp4, m_zero);
        src_temp7 = _mm_sad_epu8(src_temp7, m_zero);
        src_temp8 = _mm_sad_epu8(src_temp8, m_zero);

        src_temp4 = _mm_add_epi16(src_temp3, src_temp4);
        src_temp8 = _mm_add_epi16(src_temp7, src_temp8);
        src_temp4 = _mm_add_epi16(src_temp4, src_temp8);

        src_temp4 = _mm_shuffle_epi8(src_temp4, sm);
        src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);

        acc_dc = _mm_cvtsi128_si32(src_temp4);

        acc_dc += pu1_ref[three_nt];
        acc_dc -= pu1_ref[two_nt];

        /* computing acc_dc value */
        dc_val = (acc_dc + nt) >> (log2nt + 1);

        two_dc_val = 2 * dc_val;
        three_dc_val = 3 * dc_val;

        temp = _mm_set1_epi8(dc_val);

        for(itr_count = 0; itr_count < 2; itr_count++)
        {
            /*  pu1_dst[(row * dst_strd) + col] = dc_val;*/
            _mm_storeu_si128((__m128i *)(pu1_dst + ((0) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((1) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((2) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((3) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((4) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((5) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((6) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((7) * dst_strd)), temp);

            _mm_storeu_si128((__m128i *)(pu1_dst + ((8) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((9) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((10) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((11) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((12) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((13) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((14) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((15) * dst_strd)), temp);

            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((0) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((1) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((2) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((3) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((4) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((5) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((6) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((7) * dst_strd)), temp);

            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((8) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((9) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((10) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((11) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((12) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((13) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((14) * dst_strd)), temp);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((15) * dst_strd)), temp);

            pu1_dst += 16 * dst_strd;
        }
    }
    else

    {
        __m128i sm1 = _mm_load_si128((__m128i *)&IHEVCE_SHUFFLEMASK4[0]);

        /* DC filtering for the first top row and first left column */



        if(nt == 4) /* nt multiple of 4*/
        {
            WORD32 temp1, temp2, temp3;

            src_temp3 =  _mm_loadu_si128((__m128i *)(pu1_ref + nt));
            src_temp2 =  _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1));

            src_temp4 =  _mm_unpacklo_epi8(src_temp3, m_zero);
            src_temp2 =  _mm_unpacklo_epi8(src_temp2, m_zero);

            src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);
            src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);
            src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);

            acc_dc = _mm_cvtsi128_si32(src_temp4);
            acc_dc += pu1_ref[three_nt];
            acc_dc -= pu1_ref[two_nt];

/* computing acc_dc value */

            dc_val = (acc_dc + nt) >> (log2nt + 1);

            three_dc_val = 3 * dc_val;

            /* loding 8-bit 16 pixel */
            src_temp1 = _mm_set1_epi16(three_dc_val + 2);
            two_dc_val = 2 * dc_val;

            /*(pu1_ref[two_nt + 1 + col] + three_dc_val + 2 */
            src_temp2 = _mm_add_epi16(src_temp2, src_temp1);

            /*(pu1_ref[two_nt + 1 + col] + three_dc_val + 2) >> 2 */
            src_temp2 = _mm_srli_epi16(src_temp2, 2);

            src_temp2 = _mm_packus_epi16(src_temp2, m_zero);

            temp1 = _mm_cvtsi128_si32(src_temp2);

            *(WORD32 *)(&pu1_dst[0]) = temp1;

            src_temp2 = _mm_insert_epi16(src_temp2, dc_val, 0);

            src_temp2 =  _mm_shuffle_epi8(src_temp2, sm1);
            src_temp3 =  _mm_shuffle_epi8(src_temp2, sm1);
            src_temp4 =  _mm_shuffle_epi8(src_temp2, sm1);

            temp1 = _mm_cvtsi128_si32(src_temp2);
            temp2 = _mm_cvtsi128_si32(src_temp3);
            temp3 = _mm_cvtsi128_si32(src_temp4);

            *(WORD32 *)(&pu1_dst[(1 * dst_strd)]) = temp1;
            *(WORD32 *)(&pu1_dst[(2 * dst_strd)]) = temp2;
            *(WORD32 *)(&pu1_dst[(3 * dst_strd)]) = temp3;

            /*  retore  first value*/
            pu1_dst[0] = ((pu1_ref[two_nt - 1] + two_dc_val + pu1_ref[two_nt + 1] + 2)
                            >> 2);

            for(row = 1; row < nt; row++)
                pu1_dst[row * dst_strd] = (pu1_ref[two_nt - 1 - row] + three_dc_val + 2)
                                >> 2;

        }
        else if(nt == 8) /* if nt%8==0*/
        {

            src_temp3 =  _mm_loadu_si128((__m128i *)(pu1_ref + nt));

            src_temp4 = _mm_sad_epu8(src_temp3, m_zero);
            src_temp4 = _mm_shuffle_epi8(src_temp4, sm);
            src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);

            acc_dc = _mm_cvtsi128_si32(src_temp4);

            acc_dc += pu1_ref[three_nt];
            acc_dc -= pu1_ref[two_nt];

            /* computing acc_dc value */

            dc_val = (acc_dc + nt) >> (log2nt + 1);

            three_dc_val = 3 * dc_val;
            src_temp1 = _mm_set1_epi16(three_dc_val + 2);
            two_dc_val = 2 * dc_val;

            /* loding 8-bit 16 pixel */
            src_temp2 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1));
            src_temp2 =  _mm_unpacklo_epi8(src_temp2, m_zero);

            /*(pu1_ref[two_nt + 1 + col] + three_dc_val + 2 */
            src_temp2 = _mm_add_epi16(src_temp2, src_temp1);

            /*(pu1_ref[two_nt + 1 + col] + three_dc_val + 2)>>2 */
            src_temp2 = _mm_srli_epi16(src_temp2, 2);
            src_temp2 = _mm_packus_epi16(src_temp2, m_zero);

            _mm_storel_epi64((__m128i *)(pu1_dst), src_temp2);

            /* Fill the remaining rows with DC value*/

            src_temp1 = _mm_set1_epi8(dc_val);
            src_temp2 = _mm_set1_epi8(dc_val);
            src_temp3 = _mm_set1_epi8(dc_val);
            src_temp4 = _mm_set1_epi8(dc_val);
            src_temp5 = _mm_set1_epi8(dc_val);
            src_temp6 = _mm_set1_epi8(dc_val);
            src_temp7 = _mm_set1_epi8(dc_val);

            _mm_storel_epi64((__m128i *)(pu1_dst + ((1) * dst_strd)), src_temp1);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((2) * dst_strd)), src_temp2);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((3) * dst_strd)), src_temp3);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((4) * dst_strd)), src_temp4);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((5) * dst_strd)), src_temp5);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((6) * dst_strd)), src_temp6);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((7) * dst_strd)), src_temp7);

            /*  retore  first value*/
            pu1_dst[0] = ((pu1_ref[two_nt - 1] + two_dc_val + pu1_ref[two_nt + 1] + 2)
                            >> 2);

            for(row = 1; row < nt; row++)
                pu1_dst[row * dst_strd] = (pu1_ref[two_nt - 1 - row] + three_dc_val + 2)
                                >> 2;

        }
        else /* if nt == 16*/
        {

            src_temp3 =  _mm_loadu_si128((__m128i *)(pu1_ref + nt));
            src_temp4 =  _mm_loadu_si128((__m128i *)(pu1_ref + nt + 16));

            src_temp2 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1));
            src_temp10 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1 + 8));

            src_temp3 = _mm_sad_epu8(src_temp3, m_zero);
            src_temp4 = _mm_sad_epu8(src_temp4, m_zero);

            src_temp2  =  _mm_unpacklo_epi8(src_temp2, m_zero);
            src_temp10 =  _mm_unpacklo_epi8(src_temp10, m_zero);

            src_temp4 = _mm_add_epi16(src_temp3, src_temp4);
            src_temp4 = _mm_shuffle_epi8(src_temp4, sm);
            src_temp4 = _mm_hadd_epi16(src_temp4, m_zero);

            acc_dc = _mm_cvtsi128_si32(src_temp4);

            acc_dc += pu1_ref[three_nt];
            acc_dc -= pu1_ref[two_nt];

            /* computing acc_dc value */

            dc_val = (acc_dc + nt) >> (log2nt + 1);

            three_dc_val = 3 * dc_val;
            src_temp1 = _mm_set1_epi16(three_dc_val + 2);
            two_dc_val = 2 * dc_val;

            /*(pu1_ref[two_nt + 1 + col] + three_dc_val + 2 */
            src_temp2 = _mm_add_epi16(src_temp2, src_temp1);
            src_temp10 = _mm_add_epi16(src_temp10, src_temp1);
            /*(pu1_ref[two_nt + 1 + col] + three_dc_val + 2)>>2 */
            src_temp2 = _mm_srli_epi16(src_temp2, 2);
            src_temp10 = _mm_srli_epi16(src_temp10, 2);

            src_temp2 = _mm_packus_epi16(src_temp2, src_temp10);

            _mm_storeu_si128((__m128i *)(pu1_dst), src_temp2);

            /* Fill the remaining rows with DC value*/
            src_temp1 =  _mm_set1_epi8(dc_val);
            src_temp2 =  _mm_set1_epi8(dc_val);
            src_temp3 =  _mm_set1_epi8(dc_val);
            src_temp4 =  _mm_set1_epi8(dc_val);
            src_temp5 =  _mm_set1_epi8(dc_val);
            src_temp6 =  _mm_set1_epi8(dc_val);
            src_temp7 =  _mm_set1_epi8(dc_val);

            for(row = 1; row < nt; row += 8)
            {

                _mm_storeu_si128((__m128i *)(pu1_dst + ((1) * dst_strd)), src_temp1);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((2) * dst_strd)), src_temp2);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((3) * dst_strd)), src_temp3);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((4) * dst_strd)), src_temp4);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((5) * dst_strd)), src_temp5);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((6) * dst_strd)), src_temp6);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((7) * dst_strd)), src_temp7);

                _mm_storeu_si128((__m128i *)(pu1_dst + ((8) * dst_strd)), src_temp1);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((9) * dst_strd)), src_temp2);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((10) * dst_strd)), src_temp3);

                _mm_storeu_si128((__m128i *)(pu1_dst + ((11) * dst_strd)), src_temp4);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((12) * dst_strd)), src_temp5);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((13) * dst_strd)), src_temp6);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((14) * dst_strd)), src_temp7);

                _mm_storeu_si128((__m128i *)(pu1_dst + ((15) * dst_strd)), src_temp1);

            }

            /*  retore  first value*/
            pu1_dst[0] = ((pu1_ref[two_nt - 1] + two_dc_val + pu1_ref[two_nt + 1] + 2)
                            >> 2);

            for(row = 1; row < nt; row++)
                pu1_dst[row * dst_strd] = (pu1_ref[two_nt - 1 - row] + three_dc_val + 2)
                                >> 2;

        }
    }
}

/**
*******************************************************************************
*
* @brief
*     Intra prediction interpolation filter for horizontal luma variable.
*
* @par Description:
*      Horizontal intraprediction(mode 10) with reference  samples location
*      pointed by 'pu1_ref' to the TU block  location pointed by 'pu1_dst'  Refer
*      to section 8.4.4.2.6 in the standard (Special case)
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

void ihevc_intra_pred_luma_horz_ssse3(UWORD8 *pu1_ref,
                                      WORD32 src_strd,
                                      UWORD8 *pu1_dst,
                                      WORD32 dst_strd,
                                      WORD32 nt,
                                      WORD32 mode)
{

    WORD32 row;
    WORD32 two_nt;
    UNUSED(src_strd);
    UNUSED(mode);

    two_nt = 2 * nt;


    if(nt == 32)
    {
        __m128i src_temp1, src_temp2, src_temp3, src_temp4, src_temp5, src_temp6, src_temp7, src_temp8;
        __m128i src_temp9, src_temp10, src_temp11, src_temp12, src_temp13, src_temp14, src_temp15, src_temp16;
        __m128i sm = _mm_load_si128((__m128i *)&IHEVCE_SHUFFLEMASK4[0]);

        for(row = 0; row < nt; row += 16)
        {
            {
                src_temp1 =  _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 1 - row - 15));

                src_temp2 =  _mm_srli_si128(src_temp1, 1);
                src_temp3 =  _mm_srli_si128(src_temp1, 2);
                src_temp4 =  _mm_srli_si128(src_temp1, 3);
                src_temp5 =  _mm_srli_si128(src_temp1, 4);
                src_temp6 =  _mm_srli_si128(src_temp1, 5);
                src_temp7 =  _mm_srli_si128(src_temp1, 6);
                src_temp8 =  _mm_srli_si128(src_temp1, 7);

                src_temp9 =  _mm_srli_si128(src_temp1, 8);
                src_temp10 =  _mm_srli_si128(src_temp1, 9);
                src_temp11 =  _mm_srli_si128(src_temp1, 10);
                src_temp12 =  _mm_srli_si128(src_temp1, 11);
                src_temp13 =  _mm_srli_si128(src_temp1, 12);
                src_temp14 =  _mm_srli_si128(src_temp1, 13);
                src_temp15 =  _mm_srli_si128(src_temp1, 14);
                src_temp16 =  _mm_srli_si128(src_temp1, 15);

                src_temp8 =  _mm_shuffle_epi8(src_temp8, sm);
                src_temp7 =  _mm_shuffle_epi8(src_temp7, sm);
                src_temp6 =  _mm_shuffle_epi8(src_temp6, sm);
                src_temp5 =  _mm_shuffle_epi8(src_temp5, sm);
                src_temp4 =  _mm_shuffle_epi8(src_temp4, sm);
                src_temp3 =  _mm_shuffle_epi8(src_temp3, sm);
                src_temp2 =  _mm_shuffle_epi8(src_temp2, sm);
                src_temp1 =  _mm_shuffle_epi8(src_temp1, sm);

                src_temp16 =  _mm_shuffle_epi8(src_temp16, sm);
                src_temp15 =  _mm_shuffle_epi8(src_temp15, sm);
                src_temp14 =  _mm_shuffle_epi8(src_temp14, sm);
                src_temp13 =  _mm_shuffle_epi8(src_temp13, sm);
                src_temp12 =  _mm_shuffle_epi8(src_temp12, sm);
                src_temp11 =  _mm_shuffle_epi8(src_temp11, sm);
                src_temp10 =  _mm_shuffle_epi8(src_temp10, sm);
                src_temp9 =  _mm_shuffle_epi8(src_temp9, sm);

                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 0) * dst_strd)), src_temp16);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 1) * dst_strd)), src_temp15);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 2) * dst_strd)), src_temp14);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 3) * dst_strd)), src_temp13);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 4) * dst_strd)), src_temp12);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 5) * dst_strd)), src_temp11);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 6) * dst_strd)), src_temp10);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 7) * dst_strd)), src_temp9);

                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 8) * dst_strd)), src_temp8);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 9) * dst_strd)), src_temp7);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 10) * dst_strd)), src_temp6);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 11) * dst_strd)), src_temp5);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 12) * dst_strd)), src_temp4);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 13) * dst_strd)), src_temp3);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 14) * dst_strd)), src_temp2);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 15) * dst_strd)), src_temp1);

                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 0) * dst_strd)), src_temp16);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 1) * dst_strd)), src_temp15);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 2) * dst_strd)), src_temp14);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 3) * dst_strd)), src_temp13);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 4) * dst_strd)), src_temp12);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 5) * dst_strd)), src_temp11);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 6) * dst_strd)), src_temp10);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 7) * dst_strd)), src_temp9);

                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 8) * dst_strd)), src_temp8);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 9) * dst_strd)), src_temp7);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 10) * dst_strd)), src_temp6);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 11) * dst_strd)), src_temp5);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 12) * dst_strd)), src_temp4);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 13) * dst_strd)), src_temp3);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 14) * dst_strd)), src_temp2);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((row + 15) * dst_strd)), src_temp1);

            }

        }

    }
    else

    {
        __m128i src_temp1, src_temp2, src_temp3, src_temp4, src_temp5, src_temp6;
        __m128i src_temp10, zero_8x16b, src_temp7;

        /* DC filtering for the first top row and first left column */

        zero_8x16b = _mm_set1_epi16(0);

        /*Filtering done for the 1st row */

        src_temp2 =  _mm_set1_epi16(pu1_ref[two_nt - 1]);
        src_temp10 =  _mm_set1_epi16(pu1_ref[two_nt]);

        /*  loding 8-bit 16 pixels */
        src_temp4 =  _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1));

        src_temp4 =  _mm_unpacklo_epi8(src_temp4, zero_8x16b);

        /*(pu1_ref[two_nt + 1 + col] - pu1_ref[two_nt])*/
        src_temp3 = _mm_sub_epi16(src_temp4, src_temp10);

        /* ((pu1_ref[two_nt + 1 + col] - pu1_ref[two_nt]) >> 1)*/
        src_temp3 = _mm_srai_epi16(src_temp3, 1);

        /* pu1_ref[two_nt - 1]+((pu1_ref[two_nt + 1 + col] - pu1_ref[two_nt]) >> 1)*/
        src_temp3 = _mm_add_epi16(src_temp2, src_temp3);

        if(nt == 4)
        {
            int temp1, temp2, temp3;
            src_temp3 = _mm_packus_epi16(src_temp3, zero_8x16b);
            temp1 = _mm_cvtsi128_si32(src_temp3);

            *(WORD32 *)(&pu1_dst[0]) = temp1;

            src_temp2 =  _mm_set1_epi8(pu1_ref[two_nt - 2]);
            src_temp3 =  _mm_set1_epi8(pu1_ref[two_nt - 3]);
            src_temp4 =  _mm_set1_epi8(pu1_ref[two_nt - 4]);

            temp1 = _mm_cvtsi128_si32(src_temp2);
            temp2 = _mm_cvtsi128_si32(src_temp3);
            temp3 = _mm_cvtsi128_si32(src_temp4);

            /*pu1_dst[(row * dst_strd) + col] = pu1_ref[two_nt - 1 - row];*/
            *(WORD32 *)(&pu1_dst[(1 * dst_strd)]) = temp1;
            *(WORD32 *)(&pu1_dst[(2 * dst_strd)]) = temp2;
            *(WORD32 *)(&pu1_dst[(3 * dst_strd)]) = temp3;

        }
        else if(nt == 8)
        {
            src_temp10 = _mm_packus_epi16(src_temp3, zero_8x16b);


            src_temp1 =  _mm_set1_epi8(pu1_ref[two_nt - 2]);
            src_temp2 =  _mm_set1_epi8(pu1_ref[two_nt - 3]);
            src_temp3 =  _mm_set1_epi8(pu1_ref[two_nt - 4]);
            src_temp4 =  _mm_set1_epi8(pu1_ref[two_nt - 5]);
            src_temp5 =  _mm_set1_epi8(pu1_ref[two_nt - 6]);
            src_temp6 =  _mm_set1_epi8(pu1_ref[two_nt - 7]);
            src_temp7 =  _mm_set1_epi8(pu1_ref[two_nt - 8]);

            _mm_storel_epi64((__m128i *)(pu1_dst), src_temp10);

            /*pu1_dst[(row * dst_strd) + col] = pu1_ref[two_nt - 1 - row];*/
            _mm_storel_epi64((__m128i *)(pu1_dst + (1 * dst_strd)), src_temp1);
            _mm_storel_epi64((__m128i *)(pu1_dst + (2 * dst_strd)), src_temp2);
            _mm_storel_epi64((__m128i *)(pu1_dst + (3 * dst_strd)), src_temp3);
            _mm_storel_epi64((__m128i *)(pu1_dst + (4 * dst_strd)), src_temp4);
            _mm_storel_epi64((__m128i *)(pu1_dst + (5 * dst_strd)), src_temp5);
            _mm_storel_epi64((__m128i *)(pu1_dst + (6 * dst_strd)), src_temp6);
            _mm_storel_epi64((__m128i *)(pu1_dst + (7 * dst_strd)), src_temp7);

        }
        else if(nt == 16)
        {
            src_temp4 =  _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1 + 8));
            src_temp4 =  _mm_unpacklo_epi8(src_temp4, zero_8x16b);
            //src_temp4 =  _mm_cvtepu8_epi16 (src_temp4);

            src_temp10 = _mm_sub_epi16(src_temp4, src_temp10);
            src_temp10 = _mm_srai_epi16(src_temp10, 1);
            src_temp10 = _mm_add_epi16(src_temp2, src_temp10);

            src_temp3 = _mm_packus_epi16(src_temp3, src_temp10);
            _mm_storeu_si128((__m128i *)(pu1_dst), src_temp3);

            /*pu1_dst[(row * dst_strd) + col] = pu1_ref[two_nt - 1 - row];*/
            src_temp1 =  _mm_set1_epi8(pu1_ref[two_nt - 2]);
            src_temp2 =  _mm_set1_epi8(pu1_ref[two_nt - 3]);
            src_temp3 =  _mm_set1_epi8(pu1_ref[two_nt - 4]);
            src_temp4 =  _mm_set1_epi8(pu1_ref[two_nt - 5]);
            src_temp5 =  _mm_set1_epi8(pu1_ref[two_nt - 6]);
            src_temp6 =  _mm_set1_epi8(pu1_ref[two_nt - 7]);
            src_temp7 =  _mm_set1_epi8(pu1_ref[two_nt - 8]);
            src_temp10 =  _mm_set1_epi8(pu1_ref[two_nt - 9]);

            _mm_storeu_si128((__m128i *)(pu1_dst + ((1) * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((2) * dst_strd)), src_temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((3) * dst_strd)), src_temp3);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((4) * dst_strd)), src_temp4);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((5) * dst_strd)), src_temp5);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((6) * dst_strd)), src_temp6);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((7) * dst_strd)), src_temp7);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((8) * dst_strd)), src_temp10);

            src_temp1 =  _mm_set1_epi8(pu1_ref[two_nt - 10]);
            src_temp2 =  _mm_set1_epi8(pu1_ref[two_nt - 11]);
            src_temp3 =  _mm_set1_epi8(pu1_ref[two_nt - 12]);
            src_temp4 =  _mm_set1_epi8(pu1_ref[two_nt - 13]);
            src_temp5 =  _mm_set1_epi8(pu1_ref[two_nt - 14]);
            src_temp6 =  _mm_set1_epi8(pu1_ref[two_nt - 15]);
            src_temp7 =  _mm_set1_epi8(pu1_ref[two_nt - 16]);

            _mm_storeu_si128((__m128i *)(pu1_dst + ((9) * dst_strd)), src_temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((10) * dst_strd)), src_temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((11) * dst_strd)), src_temp3);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((12) * dst_strd)), src_temp4);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((13) * dst_strd)), src_temp5);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((14) * dst_strd)), src_temp6);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((15) * dst_strd)), src_temp7);

        }
    }
}


/**
*******************************************************************************
*
* @brief
*     Intra prediction interpolation filter for vertical luma variable.
*
* @par Description:
*    Horizontal intraprediction with reference neighboring  samples location
*    pointed by 'pu1_ref' to the TU block  location pointed by 'pu1_dst'  Refer
*    to section 8.4.4.2.6 in the standard (Special case)
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


void ihevc_intra_pred_luma_ver_ssse3(UWORD8 *pu1_ref,
                                     WORD32 src_strd,
                                     UWORD8 *pu1_dst,
                                     WORD32 dst_strd,
                                     WORD32 nt,
                                     WORD32 mode)
{
    WORD32 row;
    WORD16 s2_predpixel;
    WORD32 two_nt = 2 * nt;
    __m128i src_temp0, src_temp2;
    UNUSED(src_strd);
    UNUSED(mode);


    if(nt == 32)
    {
        __m128i temp1, temp2;
        WORD32 itr_count;

        temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1));
        temp2 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1 + 16));

        for(itr_count = 0; itr_count < 2; itr_count++)
        {
            /*  pu1_dst[(row * dst_strd) + col] = dc_val;*/
            _mm_storeu_si128((__m128i *)(pu1_dst + ((0) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((1) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((2) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((3) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((4) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((5) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((6) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((7) * dst_strd)), temp1);

            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((0) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((1) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((2) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((3) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((4) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((5) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((6) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((7) * dst_strd)), temp2);

            _mm_storeu_si128((__m128i *)(pu1_dst + ((8) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((9) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((10) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((11) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((12) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((13) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((14) * dst_strd)), temp1);
            _mm_storeu_si128((__m128i *)(pu1_dst + ((15) * dst_strd)), temp1);

            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((8) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((9) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((10) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((11) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((12) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((13) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((14) * dst_strd)), temp2);
            _mm_storeu_si128((__m128i *)(pu1_dst + 16 + ((15) * dst_strd)), temp2);

            pu1_dst += 16 * dst_strd;
        }
    }
    else
    {
        /* Replication to next columns*/

        if(nt == 4)
        {
            int temp1;

            src_temp2 =   _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1));

            temp1 = _mm_cvtsi128_si32(src_temp2);

            /* loding 4-bit 8 pixels values */
            *(WORD32 *)(&pu1_dst[(0 * dst_strd)]) = temp1;
            *(WORD32 *)(&pu1_dst[(1 * dst_strd)]) = temp1;
            *(WORD32 *)(&pu1_dst[(2 * dst_strd)]) = temp1;
            *(WORD32 *)(&pu1_dst[(3 * dst_strd)]) = temp1;

        }
        else if(nt == 8)
        {

            src_temp0 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1));

            _mm_storel_epi64((__m128i *)(pu1_dst + ((0) * dst_strd)), src_temp0);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((1) * dst_strd)), src_temp0);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((2) * dst_strd)), src_temp0);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((3) * dst_strd)), src_temp0);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((4) * dst_strd)), src_temp0);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((5) * dst_strd)), src_temp0);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((6) * dst_strd)), src_temp0);
            _mm_storel_epi64((__m128i *)(pu1_dst + ((7) * dst_strd)), src_temp0);


        }
        else if(nt == 16)
        {
            for(row = 0; row < nt; row += 8)
            {

                src_temp0 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1));

                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 0) * dst_strd)), src_temp0);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 1) * dst_strd)), src_temp0);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 2) * dst_strd)), src_temp0);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 3) * dst_strd)), src_temp0);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 4) * dst_strd)), src_temp0);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 5) * dst_strd)), src_temp0);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 6) * dst_strd)), src_temp0);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 7) * dst_strd)), src_temp0);

            }

        }

        /*Filtering done for the 1st column */
        for(row = nt - 1; row >= 0; row--)
        {
            s2_predpixel = pu1_ref[two_nt + 1]
                            + ((pu1_ref[two_nt - 1 - row] - pu1_ref[two_nt]) >> 1);
            pu1_dst[row * dst_strd] = CLIP_U8(s2_predpixel);
        }


    }
}

/**
*******************************************************************************
*
* @brief
*     Intra prediction interpolation filter for luma mode2.
*
* @par Description:
*    Intraprediction for mode 2 (sw angle) with reference  neighboring samples
*    location pointed by 'pu1_ref' to the  TU block location pointed by
*    'pu1_dst'  Refer to section 8.4.4.2.6 in the standard
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

void ihevc_intra_pred_luma_mode2_ssse3(UWORD8 *pu1_ref,
                                       WORD32 src_strd,
                                       UWORD8 *pu1_dst,
                                       WORD32 dst_strd,
                                       WORD32 nt,
                                       WORD32 mode)
{
    WORD32 row, col;
    WORD32 two_nt = 2 * nt;

    __m128i src_temp1, src_temp2, src_temp3, src_temp4, src_temp5, src_temp6, src_temp7, src_temp8;
    __m128i   sm1, sm2, sm3;
    UNUSED(src_strd);
    UNUSED(mode);


    sm1 = _mm_load_si128((__m128i *)&IHEVCE_SHUFFLEMASKY1[0]);
    sm2 = _mm_load_si128((__m128i *)&IHEVCE_SHUFFLEMASKY2[0]);
    sm3 = _mm_load_si128((__m128i *)&IHEVCE_SHUFFLEMASKY3[0]);

    /* For the angle 45, replication is done from the corresponding angle */
    /* intra_pred_ang = tan(angle) in q5 format */

    if(nt == 4)
    {
        int temp1, temp2, temp3, temp4;

        /*pu1_ref[two_nt - row - (col+1) - 1]*/
        src_temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 8));
        src_temp2 = _mm_srli_si128(src_temp1, 1);
        src_temp3 = _mm_srli_si128(src_temp1, 2);
        src_temp4 = _mm_srli_si128(src_temp1, 3);

        src_temp4 = _mm_shuffle_epi8(src_temp4, sm1);
        src_temp3 = _mm_shuffle_epi8(src_temp3, sm1);
        src_temp2 = _mm_shuffle_epi8(src_temp2, sm1);
        src_temp1 = _mm_shuffle_epi8(src_temp1, sm1);

        temp1 = _mm_cvtsi128_si32(src_temp4);
        temp2 = _mm_cvtsi128_si32(src_temp3);
        temp3 = _mm_cvtsi128_si32(src_temp2);
        temp4 = _mm_cvtsi128_si32(src_temp1);

        /*pu1_dst[(row * dst_strd) + col] = pu1_ref[two_nt - 1 - row];*/
        *(WORD32 *)(&pu1_dst[(0 * dst_strd)]) = temp1;
        *(WORD32 *)(&pu1_dst[(1 * dst_strd)]) = temp2;
        *(WORD32 *)(&pu1_dst[(2 * dst_strd)]) = temp3;
        *(WORD32 *)(&pu1_dst[(3 * dst_strd)]) = temp4;


    }
    else if(nt == 8)
    {
        /*pu1_ref[two_nt - row - (col+1) - 1]*/
        src_temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 16));
        src_temp2 = _mm_srli_si128(src_temp1, 1);
        src_temp3 = _mm_srli_si128(src_temp1, 2);
        src_temp4 = _mm_srli_si128(src_temp1, 3);
        src_temp5 = _mm_srli_si128(src_temp1, 4);
        src_temp6 = _mm_srli_si128(src_temp1, 5);
        src_temp7 = _mm_srli_si128(src_temp1, 6);
        src_temp8 = _mm_srli_si128(src_temp1, 7);

        src_temp1 = _mm_shuffle_epi8(src_temp1, sm2);
        src_temp2 = _mm_shuffle_epi8(src_temp2, sm2);
        src_temp3 = _mm_shuffle_epi8(src_temp3, sm2);
        src_temp4 = _mm_shuffle_epi8(src_temp4, sm2);
        src_temp5 = _mm_shuffle_epi8(src_temp5, sm2);
        src_temp6 = _mm_shuffle_epi8(src_temp6, sm2);
        src_temp7 = _mm_shuffle_epi8(src_temp7, sm2);
        src_temp8 = _mm_shuffle_epi8(src_temp8, sm2);

        _mm_storel_epi64((__m128i *)(pu1_dst + (0 * dst_strd)), src_temp8);
        _mm_storel_epi64((__m128i *)(pu1_dst + (1 * dst_strd)), src_temp7);
        _mm_storel_epi64((__m128i *)(pu1_dst + (2 * dst_strd)), src_temp6);
        _mm_storel_epi64((__m128i *)(pu1_dst + (3 * dst_strd)), src_temp5);
        _mm_storel_epi64((__m128i *)(pu1_dst + (4 * dst_strd)), src_temp4);
        _mm_storel_epi64((__m128i *)(pu1_dst + (5 * dst_strd)), src_temp3);
        _mm_storel_epi64((__m128i *)(pu1_dst + (6 * dst_strd)), src_temp2);
        _mm_storel_epi64((__m128i *)(pu1_dst + (7 * dst_strd)), src_temp1);

    }
    else
    {
        for(row = 0; row < nt; row += 8)
        {
            for(col = 0; col < nt; col += 16)
            {   /*pu1_ref[two_nt - row - (col+1) - 1]*/

                src_temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 0) - (col + 16) - 1));
                src_temp2 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 1) - (col + 16) - 1));
                src_temp3 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 2) - (col + 16) - 1));
                src_temp4 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 3) - (col + 16) - 1));
                src_temp5 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 4) - (col + 16) - 1));
                src_temp6 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 5) - (col + 16) - 1));
                src_temp7 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 6) - (col + 16) - 1));
                src_temp8 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 7) - (col + 16) - 1));

                src_temp1 = _mm_shuffle_epi8(src_temp1, sm3);
                src_temp2 = _mm_shuffle_epi8(src_temp2, sm3);
                src_temp3 = _mm_shuffle_epi8(src_temp3, sm3);
                src_temp4 = _mm_shuffle_epi8(src_temp4, sm3);
                src_temp5 = _mm_shuffle_epi8(src_temp5, sm3);
                src_temp6 = _mm_shuffle_epi8(src_temp6, sm3);
                src_temp7 = _mm_shuffle_epi8(src_temp7, sm3);
                src_temp8 = _mm_shuffle_epi8(src_temp8, sm3);

                _mm_storeu_si128((__m128i *)(pu1_dst + col + ((row + 0) * dst_strd)), src_temp1);
                _mm_storeu_si128((__m128i *)(pu1_dst + col + ((row + 1) * dst_strd)), src_temp2);
                _mm_storeu_si128((__m128i *)(pu1_dst + col + ((row + 2) * dst_strd)), src_temp3);
                _mm_storeu_si128((__m128i *)(pu1_dst + col + ((row + 3) * dst_strd)), src_temp4);
                _mm_storeu_si128((__m128i *)(pu1_dst + col + ((row + 4) * dst_strd)), src_temp5);
                _mm_storeu_si128((__m128i *)(pu1_dst + col + ((row + 5) * dst_strd)), src_temp6);
                _mm_storeu_si128((__m128i *)(pu1_dst + col + ((row + 6) * dst_strd)), src_temp7);
                _mm_storeu_si128((__m128i *)(pu1_dst + col + ((row + 7) * dst_strd)), src_temp8);
            }
        }
    }

}

/**
*******************************************************************************
*
* @brief
*    Intra prediction interpolation filter for luma mode 18 & mode 34.
*
* @par Description:
*    Intraprediction for mode 34 (ne angle) and  mode 18 (nw angle) with
*    reference  neighboring samples location pointed by 'pu1_ref' to the  TU
*    block location pointed by 'pu1_dst'
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

void ihevc_intra_pred_luma_mode_18_34_ssse3(UWORD8 *pu1_ref,
                                            WORD32 src_strd,
                                            UWORD8 *pu1_dst,
                                            WORD32 dst_strd,
                                            WORD32 nt,
                                            WORD32 mode)
{
    WORD32 row;
    WORD32 two_nt = 2 * nt;
    __m128i src_temp1, src_temp2, src_temp3, src_temp4, src_temp5, src_temp6, src_temp7, src_temp8;
    UNUSED(src_strd);
    if(mode == 34)
    {
        if(nt == 4)
        {

            int temp1, temp2, temp3, temp4;

            /*pu1_ref[two_nt + col + idx + 1]*/
            src_temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 2));
            src_temp2 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 3));
            src_temp3 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 4));
            src_temp4 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 5));

            temp1 = _mm_cvtsi128_si32(src_temp1);
            temp2 = _mm_cvtsi128_si32(src_temp2);
            temp3 = _mm_cvtsi128_si32(src_temp3);
            temp4 = _mm_cvtsi128_si32(src_temp4);

            /*pu1_dst[(row * dst_strd) + col] = pu1_ref[two_nt - 1 - row];*/
            *(WORD32 *)(&pu1_dst[(0 * dst_strd)]) = temp1;
            *(WORD32 *)(&pu1_dst[(1 * dst_strd)]) = temp2;
            *(WORD32 *)(&pu1_dst[(2 * dst_strd)]) = temp3;
            *(WORD32 *)(&pu1_dst[(3 * dst_strd)]) = temp4;

        }
        else if(nt == 8)
        {
            /*pu1_ref[two_nt + col + idx + 1]*/
            src_temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 2));
            src_temp2 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 3));
            src_temp3 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 4));
            src_temp4 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 5));
            src_temp5 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 6));
            src_temp6 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 7));
            src_temp7 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 8));
            src_temp8 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 9));

            _mm_storel_epi64((__m128i *)(pu1_dst + (0 * dst_strd)), src_temp1);
            _mm_storel_epi64((__m128i *)(pu1_dst + (1 * dst_strd)), src_temp2);
            _mm_storel_epi64((__m128i *)(pu1_dst + (2 * dst_strd)), src_temp3);
            _mm_storel_epi64((__m128i *)(pu1_dst + (3 * dst_strd)), src_temp4);
            _mm_storel_epi64((__m128i *)(pu1_dst + (4 * dst_strd)), src_temp5);
            _mm_storel_epi64((__m128i *)(pu1_dst + (5 * dst_strd)), src_temp6);
            _mm_storel_epi64((__m128i *)(pu1_dst + (6 * dst_strd)), src_temp7);
            _mm_storel_epi64((__m128i *)(pu1_dst + (7 * dst_strd)), src_temp8);

        }
        else if(nt == 16)
        {
            for(row = 0; row < nt; row += 8)
            {
                /*pu1_ref[two_nt + col + idx + 1]*/
                src_temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (row + 0) + 2));
                src_temp2 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (row + 1) + 2));
                src_temp3 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (row + 2) + 2));
                src_temp4 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (row + 3) + 2));
                src_temp5 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (row + 4) + 2));
                src_temp6 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (row + 5) + 2));
                src_temp7 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (row + 6) + 2));
                src_temp8 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (row + 7) + 2));

                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 0) * dst_strd)), src_temp1);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 1) * dst_strd)), src_temp2);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 2) * dst_strd)), src_temp3);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 3) * dst_strd)), src_temp4);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 4) * dst_strd)), src_temp5);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 5) * dst_strd)), src_temp6);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 6) * dst_strd)), src_temp7);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 7) * dst_strd)), src_temp8);


            }
        }
        else
        {
            __m128i src_temp9, src_temp10, src_temp11, src_temp12, src_temp13, src_temp14, src_temp15, src_temp16;
            for(row = 0; row < nt; row += 8)
            {
                /*pu1_ref[two_nt + col + idx + 1]*/
                src_temp1  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (0 + 0) + 2));
                src_temp9  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (0 + 16) + 2));
                src_temp2  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (1 + 0) + 2));
                src_temp10 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (1 + 16) + 2));
                src_temp3  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (2 + 0) + 2));
                src_temp11 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (2 + 16) + 2));
                src_temp4  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (3 + 0) + 2));
                src_temp12 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (3 + 16) + 2));

                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (0 * dst_strd)), src_temp1);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (0 * dst_strd)), src_temp9);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (1 * dst_strd)), src_temp2);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (1 * dst_strd)), src_temp10);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (2 * dst_strd)), src_temp3);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (2 * dst_strd)), src_temp11);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (3 * dst_strd)), src_temp4);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (3 * dst_strd)), src_temp12);

                src_temp5  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (4 + 0) + 2));
                src_temp13 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (4 + 16) + 2));
                src_temp6  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (5 + 0) + 2));
                src_temp14 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (5 + 16) + 2));
                src_temp7  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (6 + 0) + 2));
                src_temp15 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (6 + 16) + 2));
                src_temp8  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (7 + 0) + 2));
                src_temp16 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + (7 + 16) + 2));

                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (4 * dst_strd)), src_temp5);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (4 * dst_strd)), src_temp13);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (5 * dst_strd)), src_temp6);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (5 * dst_strd)), src_temp14);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (6 * dst_strd)), src_temp7);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (6 * dst_strd)), src_temp15);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (7 * dst_strd)), src_temp8);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (7 * dst_strd)), src_temp16);

                pu1_ref += 8;
                pu1_dst += 8 * dst_strd;
            }
        }
    }
    else
    {
        if(nt == 4)
        {
            int temp1, temp2, temp3, temp4;

            /*pu1_ref[two_nt + col + idx + 1]*/
            src_temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 3));
            src_temp2 = _mm_srli_si128(src_temp1, 1);
            src_temp3 = _mm_srli_si128(src_temp1, 2);
            src_temp4 = _mm_srli_si128(src_temp1, 3);

            temp1 = _mm_cvtsi128_si32(src_temp4);
            temp2 = _mm_cvtsi128_si32(src_temp3);
            temp3 = _mm_cvtsi128_si32(src_temp2);
            temp4 = _mm_cvtsi128_si32(src_temp1);

            /*pu1_dst[(row * dst_strd) + col] = pu1_ref[two_nt - 1 - row];*/
            *(WORD32 *)(&pu1_dst[(0 * dst_strd)]) = temp1;
            *(WORD32 *)(&pu1_dst[(1 * dst_strd)]) = temp2;
            *(WORD32 *)(&pu1_dst[(2 * dst_strd)]) = temp3;
            *(WORD32 *)(&pu1_dst[(3 * dst_strd)]) = temp4;

        }
        else if(nt == 8)
        {
            /*pu1_ref[two_nt + col + idx + 1]*/
            src_temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 7));
            src_temp2 = _mm_srli_si128(src_temp1, 1);
            src_temp3 = _mm_srli_si128(src_temp1, 2);
            src_temp4 = _mm_srli_si128(src_temp1, 3);
            src_temp5 = _mm_srli_si128(src_temp1, 4);
            src_temp6 = _mm_srli_si128(src_temp1, 5);
            src_temp7 = _mm_srli_si128(src_temp1, 6);
            src_temp8 = _mm_srli_si128(src_temp1, 7);

            _mm_storel_epi64((__m128i *)(pu1_dst + (0 * dst_strd)), src_temp8);
            _mm_storel_epi64((__m128i *)(pu1_dst + (1 * dst_strd)), src_temp7);
            _mm_storel_epi64((__m128i *)(pu1_dst + (2 * dst_strd)), src_temp6);
            _mm_storel_epi64((__m128i *)(pu1_dst + (3 * dst_strd)), src_temp5);
            _mm_storel_epi64((__m128i *)(pu1_dst + (4 * dst_strd)), src_temp4);
            _mm_storel_epi64((__m128i *)(pu1_dst + (5 * dst_strd)), src_temp3);
            _mm_storel_epi64((__m128i *)(pu1_dst + (6 * dst_strd)), src_temp2);
            _mm_storel_epi64((__m128i *)(pu1_dst + (7 * dst_strd)), src_temp1);


        }
        else if(nt == 16)
        {
            for(row = 0; row < nt; row += 8)
            {
                /*pu1_ref[two_nt + col + idx + 1]*/
                src_temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 0)));
                src_temp2 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 1)));
                src_temp3 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 2)));
                src_temp4 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 3)));
                src_temp5 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 4)));
                src_temp6 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 5)));
                src_temp7 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 6)));
                src_temp8 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - (row + 7)));

                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 0) * dst_strd)), src_temp1);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 1) * dst_strd)), src_temp2);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 2) * dst_strd)), src_temp3);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 3) * dst_strd)), src_temp4);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 4) * dst_strd)), src_temp5);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 5) * dst_strd)), src_temp6);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 6) * dst_strd)), src_temp7);
                _mm_storeu_si128((__m128i *)(pu1_dst + ((row + 7) * dst_strd)), src_temp8);

            }

        }
        else
        {
            __m128i src_temp9, src_temp10, src_temp11, src_temp12, src_temp13, src_temp14, src_temp15, src_temp16;
            for(row = 0; row < nt; row += 8)
            {
                /*pu1_ref[two_nt + col + idx + 1]*/
                src_temp1  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 0 + 0));
                src_temp9  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 0 + 16));
                src_temp2  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 1 + 0));
                src_temp10 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 1 + 16));
                src_temp3  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 2 + 0));
                src_temp11 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 2 + 16));
                src_temp4  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 3 + 0));
                src_temp12 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 3 + 16));

                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (0 * dst_strd)), src_temp1);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (0 * dst_strd)), src_temp9);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (1 * dst_strd)), src_temp2);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (1 * dst_strd)), src_temp10);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (2 * dst_strd)), src_temp3);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (2 * dst_strd)), src_temp11);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (3 * dst_strd)), src_temp4);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (3 * dst_strd)), src_temp12);

                src_temp5  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 4 + 0));
                src_temp13 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 4 + 16));
                src_temp6  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 5 + 0));
                src_temp14 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 5 + 16));
                src_temp7  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 6 + 0));
                src_temp15 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 6 + 16));
                src_temp8  = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 7 + 0));
                src_temp16 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 7 + 16));

                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (4 * dst_strd)), src_temp5);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (4 * dst_strd)), src_temp13);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (5 * dst_strd)), src_temp6);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (5 * dst_strd)), src_temp14);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (6 * dst_strd)), src_temp7);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (6 * dst_strd)), src_temp15);
                _mm_storeu_si128((__m128i *)(pu1_dst +  0 + (7 * dst_strd)), src_temp8);
                _mm_storeu_si128((__m128i *)(pu1_dst + 16 + (7 * dst_strd)), src_temp16);

                pu1_ref -= 8;
                pu1_dst += 8 * dst_strd;
            }
        }
    }
}


/**
*******************************************************************************
*
* @brief
*    Intra prediction interpolation filter for luma mode 3 to mode 9
*
* @par Description:
*    Intraprediction for mode 3 to 9  (positive angle, horizontal mode ) with
*    reference  neighboring samples location pointed by 'pu1_ref' to the  TU
*    block location pointed by 'pu1_dst'
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

void ihevc_intra_pred_luma_mode_3_to_9_ssse3(UWORD8 *pu1_ref,
                                             WORD32 src_strd,
                                             UWORD8 *pu1_dst,
                                             WORD32 dst_strd,
                                             WORD32 nt,
                                             WORD32 mode)
{
    WORD32 row, col;
    WORD32 two_nt = 2 * nt;
    WORD32 intra_pred_ang;


    __m128i const_temp_4x32b, const_temp2_4x32b, const_temp3_4x32b, const_temp4_4x32b, zero_8x16b;
    __m128i fract_4x32b, intra_pred_ang_4x32b;
    __m128i row_4x32b, two_nt_4x32b, ref_main_idx_4x32b, res_temp5_4x32b, sm3;
    UNUSED(src_strd);

    /* Intra Pred Angle according to the mode */
    intra_pred_ang = gai4_ihevc_ang_table[mode];

    /* For the angles other then 45 degree, interpolation btw 2 neighboring */
    /* samples dependent on distance to obtain destination sample */

    /* For the angles other then 45 degree, interpolation btw 2 neighboring */
    /* samples dependent on distance to obtain destination sample */

    const_temp_4x32b  = _mm_set1_epi16(16);
    const_temp2_4x32b = _mm_set1_epi32(31);
    const_temp3_4x32b = _mm_set1_epi32(32);
    const_temp4_4x32b = _mm_set1_epi32(4);

    two_nt_4x32b = _mm_set1_epi32(two_nt - nt);


    sm3 = _mm_load_si128((__m128i *)&IHEVCE_SHUFFLEMASKY11[0]);

    /* intra_pred_ang = gai4_ihevc_ang_table[mode]; */
    intra_pred_ang_4x32b = _mm_set1_epi32(intra_pred_ang);

    row_4x32b = _mm_set_epi32(4, 3, 2, 1);

    if(nt == 4)
    {

        WORD32 ref_main_idx1, ref_main_idx2, ref_main_idx3, ref_main_idx4;
        int temp11, temp21, temp31, temp41;
        // WORD8  ai1_fract_temp_val[16], ai1_row_temp_val[16];

        __m128i fract1_8x16b, fract2_8x16b, sign_8x16b;
        __m128i temp1_8x16b, temp2_8x16b, temp3_8x16b, temp4_8x16b;

        __m128i src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b, src_temp4_8x16b;
        __m128i src_temp5_8x16b, src_temp6_8x16b, src_temp7_8x16b; //, src_temp8_8x16b;
        __m128i ref_main_temp0, ref_main_temp1, ref_main_temp2;

        row_4x32b = _mm_set_epi16(4, 3, 2, 1, 4, 3, 2, 1);
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);

        /* pos = ((row + 1) * intra_pred_ang); */
        res_temp5_4x32b = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);
        zero_8x16b      = _mm_setzero_si128();
        sign_8x16b      = _mm_cmpgt_epi16(zero_8x16b, res_temp5_4x32b);
        res_temp5_4x32b = _mm_unpacklo_epi16(res_temp5_4x32b, sign_8x16b);

        /* idx = pos >> 5; */
        fract_4x32b = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

        /* fract = pos & (31); */
        ref_main_idx_4x32b = _mm_sub_epi32(two_nt_4x32b, _mm_srai_epi32(res_temp5_4x32b,  5));

        /*(32 - fract) */
        row_4x32b = _mm_sub_epi32(const_temp3_4x32b, fract_4x32b);

        fract1_8x16b = _mm_slli_epi16(fract_4x32b, 8);
        fract2_8x16b = _mm_slli_epi16(row_4x32b, 8);

        fract_4x32b = _mm_or_si128(fract_4x32b, fract1_8x16b);
        row_4x32b = _mm_or_si128(row_4x32b, fract2_8x16b); /*(32 - fract) */

        fract2_8x16b = _mm_unpackhi_epi8(row_4x32b, fract_4x32b);
        fract1_8x16b = _mm_unpacklo_epi8(row_4x32b, fract_4x32b);

        temp1_8x16b =  _mm_shuffle_epi32(fract1_8x16b, 0x00);
        temp2_8x16b =  _mm_shuffle_epi32(fract1_8x16b, 0xaa);
        temp3_8x16b =  _mm_shuffle_epi32(fract2_8x16b, 0x00);
        temp4_8x16b =  _mm_shuffle_epi32(fract2_8x16b, 0xaa);

        ref_main_temp0 = _mm_srli_si128(ref_main_idx_4x32b, 4);  /* next 32 bit values */
        ref_main_temp1 = _mm_srli_si128(ref_main_idx_4x32b, 8);  /* next 32 bit values */
        ref_main_temp2 = _mm_srli_si128(ref_main_idx_4x32b, 12); /* next 32 bit values */
        ref_main_idx1  = _mm_cvtsi128_si32(ref_main_idx_4x32b);    /* col=0*/
        ref_main_idx2  = _mm_cvtsi128_si32(ref_main_temp0);  /* col=1*/
        ref_main_idx3  = _mm_cvtsi128_si32(ref_main_temp1);  /* col=2*/
        ref_main_idx4  = _mm_cvtsi128_si32(ref_main_temp2);  /* col=3*/

        /* loding 8-bit 16 pixels */
        src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx1 - 1)); /* col=0*/
        src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx2 - 1)); /* col=1*/
        src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx3 - 1)); /* col=2*/
        src_temp4_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx4 - 1)); /* col=3*/

        src_temp1_8x16b =  _mm_shuffle_epi8(src_temp1_8x16b, sm3); /* col=0*/
        src_temp2_8x16b =  _mm_shuffle_epi8(src_temp2_8x16b, sm3); /* col=1*/
        src_temp3_8x16b =  _mm_shuffle_epi8(src_temp3_8x16b, sm3); /* col=2*/
        src_temp4_8x16b =  _mm_shuffle_epi8(src_temp4_8x16b, sm3); /* col=3*/

        /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
        src_temp1_8x16b = _mm_maddubs_epi16(src_temp1_8x16b, temp1_8x16b);
        src_temp2_8x16b = _mm_maddubs_epi16(src_temp2_8x16b, temp2_8x16b);
        src_temp3_8x16b = _mm_maddubs_epi16(src_temp3_8x16b, temp3_8x16b);
        src_temp4_8x16b = _mm_maddubs_epi16(src_temp4_8x16b, temp4_8x16b);

        /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
        src_temp1_8x16b = _mm_add_epi16(src_temp1_8x16b, const_temp_4x32b);
        src_temp2_8x16b = _mm_add_epi16(src_temp2_8x16b, const_temp_4x32b);
        src_temp3_8x16b = _mm_add_epi16(src_temp3_8x16b, const_temp_4x32b);
        src_temp4_8x16b = _mm_add_epi16(src_temp4_8x16b, const_temp_4x32b);

        /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
        src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  5);   /* col=0*/
        src_temp2_8x16b = _mm_srai_epi16(src_temp2_8x16b,  5);   /* col=1*/
        src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  5);   /* col=2*/
        src_temp4_8x16b = _mm_srai_epi16(src_temp4_8x16b,  5);   /* col=3*/

        /* converting 16 bit to 8 bit */
        src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp3_8x16b); /* col=0*/
        src_temp2_8x16b = _mm_packus_epi16(src_temp2_8x16b, src_temp4_8x16b); /* col=1*/


        src_temp5_8x16b = _mm_unpacklo_epi8(src_temp1_8x16b, src_temp2_8x16b);
        src_temp6_8x16b = _mm_unpackhi_epi8(src_temp1_8x16b, src_temp2_8x16b);

        src_temp3_8x16b = _mm_unpacklo_epi16(src_temp5_8x16b, src_temp6_8x16b);
        src_temp2_8x16b = _mm_srli_si128(src_temp3_8x16b, 4);
        src_temp1_8x16b = _mm_srli_si128(src_temp3_8x16b, 8);
        src_temp7_8x16b = _mm_srli_si128(src_temp3_8x16b, 12);

        temp11 = _mm_cvtsi128_si32(src_temp7_8x16b);
        temp21 = _mm_cvtsi128_si32(src_temp1_8x16b);
        temp31 = _mm_cvtsi128_si32(src_temp2_8x16b);
        temp41 = _mm_cvtsi128_si32(src_temp3_8x16b);

        /* loding 4-bit 8 pixels values */
        *(WORD32 *)(&pu1_dst[(0 * dst_strd)]) = temp11;
        *(WORD32 *)(&pu1_dst[(1 * dst_strd)]) = temp21;
        *(WORD32 *)(&pu1_dst[(2 * dst_strd)]) = temp31;
        *(WORD32 *)(&pu1_dst[(3 * dst_strd)]) = temp41;

    }

    else if(nt == 16 || nt == 32)
    {
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);
        row_4x32b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);
        const_temp2_4x32b = _mm_set1_epi16(31);
        const_temp4_4x32b = _mm_set1_epi16(8);
        const_temp3_4x32b = _mm_set1_epi16(32);
        two_nt_4x32b = _mm_set1_epi16(two_nt);

        for(col = 0; col < nt; col += 8)
        {
            WORD16 pi2_ref_main_idx1, pi2_ref_main_idx2, pi2_ref_main_idx3, pi2_ref_main_idx4;
            WORD16 pi2_ref_main_idx5, pi2_ref_main_idx6, pi2_ref_main_idx7, pi2_ref_main_idx8;
            //WORD8  ai1_fract_temp0_val[16], ai1_fract_temp1_val[16];

            __m128i fract1_8x16b, fract2_8x16b, fract3_8x16b, fract8_8x16b;

            __m128i temp1_8x16b, temp2_8x16b, temp3_8x16b, temp4_8x16b;
            __m128i temp11_8x16b, temp12_8x16b, temp13_8x16b, temp14_8x16b;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

            /* idx = pos >> 5; */
            fract_4x32b = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /*(32 - fract) */
            fract2_8x16b =  _mm_sub_epi16(const_temp3_4x32b, fract_4x32b);

            fract1_8x16b = _mm_slli_epi16(fract_4x32b, 8);
            fract3_8x16b = _mm_slli_epi16(fract2_8x16b, 8); /*(32 - fract) */

            fract_4x32b = _mm_or_si128(fract_4x32b, fract1_8x16b);
            fract2_8x16b = _mm_or_si128(fract2_8x16b, fract3_8x16b); /*(32 - fract) */


            fract8_8x16b = _mm_unpackhi_epi8(fract2_8x16b, fract_4x32b);
            fract_4x32b = _mm_unpacklo_epi8(fract2_8x16b, fract_4x32b);

            temp1_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0x00);
            temp2_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0x55);
            temp3_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0xaa);
            temp4_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0xff);

            temp11_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0x00);
            temp12_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0x55);
            temp13_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0xaa);
            temp14_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0xff);

            /* fract = pos & (31); */
            ref_main_idx_4x32b = _mm_sub_epi16(two_nt_4x32b, _mm_srai_epi16(res_temp5_4x32b,  5));

            row_4x32b = _mm_add_epi16(row_4x32b, const_temp4_4x32b);

            pi2_ref_main_idx1 = _mm_extract_epi16(ref_main_idx_4x32b, 0);    /* col=0*/
            pi2_ref_main_idx2 = _mm_extract_epi16(ref_main_idx_4x32b, 1);    /* col=1*/
            pi2_ref_main_idx3 = _mm_extract_epi16(ref_main_idx_4x32b, 2);    /* col=2*/
            pi2_ref_main_idx4 = _mm_extract_epi16(ref_main_idx_4x32b, 3);    /* col=3*/

            pi2_ref_main_idx5 = _mm_extract_epi16(ref_main_idx_4x32b, 4);    /* col=5*/
            pi2_ref_main_idx6 = _mm_extract_epi16(ref_main_idx_4x32b, 5);    /* col=6*/
            pi2_ref_main_idx7 = _mm_extract_epi16(ref_main_idx_4x32b, 6);    /* col=7*/
            pi2_ref_main_idx8 = _mm_extract_epi16(ref_main_idx_4x32b, 7);    /* col=8*/

            for(row = 0; row < nt; row += 8)
            {
                __m128i src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b, src_temp4_8x16b;
                __m128i src_temp5_8x16b, src_temp6_8x16b, src_temp7_8x16b, src_temp8_8x16b;


                __m128i src_temp11_8x16b, src_temp12_8x16b, src_temp13_8x16b, src_temp14_8x16b;
                __m128i src_temp15_8x16b, src_temp16_8x16b, src_temp17_8x16b, src_temp18_8x16b;

                /* loding 8-bit 16 pixels */
                src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx1 - 1 - (8 + row))); /* col=0*/
                src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx2 - 1 - (8 + row))); /* col=1*/
                src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx3 - 1 - (8 + row))); /* col=2*/
                src_temp4_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx4 - 1 - (8 + row))); /* col=3*/

                /* loding 8-bit 16 pixels */
                src_temp11_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx5 - 1 - (8 + row))); /* col=5*/
                src_temp12_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx6 - 1 - (8 + row))); /* col=6*/
                src_temp13_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx7 - 1 - (8 + row))); /* col=7*/
                src_temp14_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx8 - 1 - (8 + row))); /* col=8*/

                src_temp1_8x16b =  _mm_shuffle_epi8(src_temp1_8x16b, sm3); /* col=0*/
                src_temp2_8x16b =  _mm_shuffle_epi8(src_temp2_8x16b, sm3); /* col=1*/
                src_temp3_8x16b =  _mm_shuffle_epi8(src_temp3_8x16b, sm3); /* col=2*/
                src_temp4_8x16b =  _mm_shuffle_epi8(src_temp4_8x16b, sm3); /* col=3*/

                src_temp11_8x16b =  _mm_shuffle_epi8(src_temp11_8x16b, sm3); /* col=0*/
                src_temp12_8x16b =  _mm_shuffle_epi8(src_temp12_8x16b, sm3); /* col=1*/
                src_temp13_8x16b =  _mm_shuffle_epi8(src_temp13_8x16b, sm3); /* col=2*/
                src_temp14_8x16b =  _mm_shuffle_epi8(src_temp14_8x16b, sm3); /* col=3*/

                /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
                src_temp1_8x16b = _mm_maddubs_epi16(src_temp1_8x16b, temp1_8x16b);
                src_temp2_8x16b = _mm_maddubs_epi16(src_temp2_8x16b, temp2_8x16b);
                src_temp3_8x16b = _mm_maddubs_epi16(src_temp3_8x16b, temp3_8x16b);
                src_temp4_8x16b = _mm_maddubs_epi16(src_temp4_8x16b, temp4_8x16b);

                /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
                src_temp11_8x16b = _mm_maddubs_epi16(src_temp11_8x16b, temp11_8x16b);
                src_temp12_8x16b = _mm_maddubs_epi16(src_temp12_8x16b, temp12_8x16b);
                src_temp13_8x16b = _mm_maddubs_epi16(src_temp13_8x16b, temp13_8x16b);
                src_temp14_8x16b = _mm_maddubs_epi16(src_temp14_8x16b, temp14_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_temp1_8x16b = _mm_add_epi16(src_temp1_8x16b, const_temp_4x32b);
                src_temp2_8x16b = _mm_add_epi16(src_temp2_8x16b, const_temp_4x32b);
                src_temp3_8x16b = _mm_add_epi16(src_temp3_8x16b, const_temp_4x32b);
                src_temp4_8x16b = _mm_add_epi16(src_temp4_8x16b, const_temp_4x32b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  5);   /* col=0*/
                src_temp2_8x16b = _mm_srai_epi16(src_temp2_8x16b,  5);   /* col=1*/
                src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  5);   /* col=2*/
                src_temp4_8x16b = _mm_srai_epi16(src_temp4_8x16b,  5);   /* col=3*/

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_temp11_8x16b = _mm_add_epi16(src_temp11_8x16b, const_temp_4x32b);
                src_temp12_8x16b = _mm_add_epi16(src_temp12_8x16b, const_temp_4x32b);
                src_temp13_8x16b = _mm_add_epi16(src_temp13_8x16b, const_temp_4x32b);
                src_temp14_8x16b = _mm_add_epi16(src_temp14_8x16b, const_temp_4x32b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_temp11_8x16b = _mm_srai_epi16(src_temp11_8x16b,  5);   /* col=5*/
                src_temp12_8x16b = _mm_srai_epi16(src_temp12_8x16b,  5);   /* col=6*/
                src_temp13_8x16b = _mm_srai_epi16(src_temp13_8x16b,  5);   /* col=7*/
                src_temp14_8x16b = _mm_srai_epi16(src_temp14_8x16b,  5);   /* col=8*/

                /* converting 16 bit to 8 bit */
                src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp3_8x16b); /* col=0*/
                src_temp2_8x16b = _mm_packus_epi16(src_temp2_8x16b, src_temp4_8x16b); /* col=1*/

                /* converting 16 bit to 8 bit */
                src_temp11_8x16b = _mm_packus_epi16(src_temp11_8x16b, src_temp13_8x16b); /* col=5*/
                src_temp12_8x16b = _mm_packus_epi16(src_temp12_8x16b, src_temp14_8x16b); /* col=6*/

                src_temp5_8x16b = _mm_unpacklo_epi8(src_temp1_8x16b, src_temp2_8x16b);
                src_temp6_8x16b = _mm_unpackhi_epi8(src_temp1_8x16b, src_temp2_8x16b);

                src_temp15_8x16b = _mm_unpacklo_epi8(src_temp11_8x16b, src_temp12_8x16b);
                src_temp16_8x16b = _mm_unpackhi_epi8(src_temp11_8x16b, src_temp12_8x16b);

                src_temp7_8x16b = _mm_unpacklo_epi16(src_temp5_8x16b, src_temp6_8x16b);
                src_temp8_8x16b = _mm_unpackhi_epi16(src_temp5_8x16b, src_temp6_8x16b);

                src_temp17_8x16b = _mm_unpacklo_epi16(src_temp15_8x16b, src_temp16_8x16b);
                src_temp18_8x16b = _mm_unpackhi_epi16(src_temp15_8x16b, src_temp16_8x16b);

                src_temp1_8x16b = _mm_unpacklo_epi32(src_temp7_8x16b, src_temp17_8x16b);
                src_temp2_8x16b = _mm_unpackhi_epi32(src_temp7_8x16b, src_temp17_8x16b);

                src_temp5_8x16b = _mm_srli_si128(src_temp1_8x16b, 8);
                src_temp6_8x16b = _mm_srli_si128(src_temp2_8x16b, 8);

                src_temp3_8x16b = _mm_unpacklo_epi32(src_temp8_8x16b, src_temp18_8x16b);
                src_temp4_8x16b = _mm_unpackhi_epi32(src_temp8_8x16b, src_temp18_8x16b);

                src_temp7_8x16b = _mm_srli_si128(src_temp3_8x16b, 8);
                src_temp8_8x16b = _mm_srli_si128(src_temp4_8x16b, 8);

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 7))), src_temp1_8x16b);       /* row=7*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 6))), src_temp5_8x16b);       /* row=6*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 5))), src_temp2_8x16b);       /* row=5*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 4))), src_temp6_8x16b);       /* row=4*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 3))), src_temp3_8x16b);       /* row=3*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 2))), src_temp7_8x16b);       /* row=2*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 1))), src_temp4_8x16b);       /* row=1*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 0))), src_temp8_8x16b);       /* row=0*/

            }
        }
    }
    else
    {
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);
        row_4x32b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);
        const_temp2_4x32b = _mm_set1_epi16(31);
        const_temp4_4x32b = _mm_set1_epi16(8);
        const_temp3_4x32b = _mm_set1_epi16(32);
        two_nt_4x32b = _mm_set1_epi16(two_nt - nt);
        {
            WORD16 pi2_ref_main_idx1, pi2_ref_main_idx2, pi2_ref_main_idx3, pi2_ref_main_idx4;
            WORD16 pi2_ref_main_idx5, pi2_ref_main_idx6, pi2_ref_main_idx7, pi2_ref_main_idx8;

            __m128i fract1_8x16b, fract2_8x16b, fract3_8x16b, fract8_8x16b;

            __m128i temp1_8x16b, temp2_8x16b, temp3_8x16b, temp4_8x16b;
            __m128i temp11_8x16b, temp12_8x16b, temp13_8x16b, temp14_8x16b;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

            /* idx = pos >> 5; */
            fract_4x32b = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /* fract = pos & (31); */
            ref_main_idx_4x32b = _mm_sub_epi16(two_nt_4x32b, _mm_srai_epi16(res_temp5_4x32b,  5));

            /*(32 - fract) */
            fract2_8x16b =  _mm_sub_epi16(const_temp3_4x32b, fract_4x32b);

            fract1_8x16b = _mm_slli_epi16(fract_4x32b, 8);
            fract3_8x16b = _mm_slli_epi16(fract2_8x16b, 8); /*(32 - fract) */

            fract_4x32b = _mm_or_si128(fract_4x32b, fract1_8x16b);
            fract2_8x16b = _mm_or_si128(fract2_8x16b, fract3_8x16b); /*(32 - fract) */


            fract8_8x16b = _mm_unpackhi_epi8(fract2_8x16b, fract_4x32b);
            fract_4x32b = _mm_unpacklo_epi8(fract2_8x16b, fract_4x32b);

            temp1_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0x00);
            temp2_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0x55);
            temp3_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0xaa);
            temp4_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0xff);

            temp11_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0x00);
            temp12_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0x55);
            temp13_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0xaa);
            temp14_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0xff);

            pi2_ref_main_idx1 = _mm_extract_epi16(ref_main_idx_4x32b, 0);    /* col=0*/
            pi2_ref_main_idx2 = _mm_extract_epi16(ref_main_idx_4x32b, 1);    /* col=1*/
            pi2_ref_main_idx3 = _mm_extract_epi16(ref_main_idx_4x32b, 2);    /* col=2*/
            pi2_ref_main_idx4 = _mm_extract_epi16(ref_main_idx_4x32b, 3);    /* col=3*/

            pi2_ref_main_idx5 = _mm_extract_epi16(ref_main_idx_4x32b, 4);    /* col=5*/
            pi2_ref_main_idx6 = _mm_extract_epi16(ref_main_idx_4x32b, 5);    /* col=6*/
            pi2_ref_main_idx7 = _mm_extract_epi16(ref_main_idx_4x32b, 6);    /* col=7*/
            pi2_ref_main_idx8 = _mm_extract_epi16(ref_main_idx_4x32b, 7);    /* col=8*/

            {
                __m128i src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b, src_temp4_8x16b;
                __m128i src_temp5_8x16b, src_temp6_8x16b, src_temp7_8x16b, src_temp8_8x16b;

                __m128i src_temp11_8x16b, src_temp12_8x16b, src_temp13_8x16b, src_temp14_8x16b;
                __m128i src_temp15_8x16b, src_temp16_8x16b, src_temp17_8x16b, src_temp18_8x16b;

                /* loding 8-bit 16 pixels */
                src_temp1_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx1 - 1)); /* col=0*/
                src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx2 - 1)); /* col=1*/
                src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx3 - 1)); /* col=2*/
                src_temp4_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx4 - 1)); /* col=3*/

                /* loding 8-bit 16 pixels */
                src_temp11_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx5 - 1)); /* col=5*/
                src_temp12_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx6 - 1)); /* col=6*/
                src_temp13_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx7 - 1)); /* col=7*/
                src_temp14_8x16b = _mm_loadu_si128((__m128i *)(pu1_ref + pi2_ref_main_idx8 - 1)); /* col=8*/

                src_temp1_8x16b =  _mm_shuffle_epi8(src_temp1_8x16b, sm3); /* col=0*/
                src_temp2_8x16b =  _mm_shuffle_epi8(src_temp2_8x16b, sm3); /* col=1*/
                src_temp3_8x16b =  _mm_shuffle_epi8(src_temp3_8x16b, sm3); /* col=2*/
                src_temp4_8x16b =  _mm_shuffle_epi8(src_temp4_8x16b, sm3); /* col=3*/

                src_temp11_8x16b =  _mm_shuffle_epi8(src_temp11_8x16b, sm3); /* col=0*/
                src_temp12_8x16b =  _mm_shuffle_epi8(src_temp12_8x16b, sm3); /* col=1*/
                src_temp13_8x16b =  _mm_shuffle_epi8(src_temp13_8x16b, sm3); /* col=2*/
                src_temp14_8x16b =  _mm_shuffle_epi8(src_temp14_8x16b, sm3); /* col=3*/

                /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
                src_temp1_8x16b = _mm_maddubs_epi16(src_temp1_8x16b, temp1_8x16b);
                src_temp2_8x16b = _mm_maddubs_epi16(src_temp2_8x16b, temp2_8x16b);
                src_temp3_8x16b = _mm_maddubs_epi16(src_temp3_8x16b, temp3_8x16b);
                src_temp4_8x16b = _mm_maddubs_epi16(src_temp4_8x16b, temp4_8x16b);

                /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
                src_temp11_8x16b = _mm_maddubs_epi16(src_temp11_8x16b, temp11_8x16b);
                src_temp12_8x16b = _mm_maddubs_epi16(src_temp12_8x16b, temp12_8x16b);
                src_temp13_8x16b = _mm_maddubs_epi16(src_temp13_8x16b, temp13_8x16b);
                src_temp14_8x16b = _mm_maddubs_epi16(src_temp14_8x16b, temp14_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_temp1_8x16b = _mm_add_epi16(src_temp1_8x16b, const_temp_4x32b);
                src_temp2_8x16b = _mm_add_epi16(src_temp2_8x16b, const_temp_4x32b);
                src_temp3_8x16b = _mm_add_epi16(src_temp3_8x16b, const_temp_4x32b);
                src_temp4_8x16b = _mm_add_epi16(src_temp4_8x16b, const_temp_4x32b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  5);   /* col=0*/
                src_temp2_8x16b = _mm_srai_epi16(src_temp2_8x16b,  5);   /* col=1*/
                src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  5);   /* col=2*/
                src_temp4_8x16b = _mm_srai_epi16(src_temp4_8x16b,  5);   /* col=3*/

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_temp11_8x16b = _mm_add_epi16(src_temp11_8x16b, const_temp_4x32b);
                src_temp12_8x16b = _mm_add_epi16(src_temp12_8x16b, const_temp_4x32b);
                src_temp13_8x16b = _mm_add_epi16(src_temp13_8x16b, const_temp_4x32b);
                src_temp14_8x16b = _mm_add_epi16(src_temp14_8x16b, const_temp_4x32b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_temp11_8x16b = _mm_srai_epi16(src_temp11_8x16b,  5);   /* col=5*/
                src_temp12_8x16b = _mm_srai_epi16(src_temp12_8x16b,  5);   /* col=6*/
                src_temp13_8x16b = _mm_srai_epi16(src_temp13_8x16b,  5);   /* col=7*/
                src_temp14_8x16b = _mm_srai_epi16(src_temp14_8x16b,  5);   /* col=8*/

                /* converting 16 bit to 8 bit */
                src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp3_8x16b); /* col=0*/
                src_temp2_8x16b = _mm_packus_epi16(src_temp2_8x16b, src_temp4_8x16b); /* col=1*/

                /* converting 16 bit to 8 bit */
                src_temp11_8x16b = _mm_packus_epi16(src_temp11_8x16b, src_temp13_8x16b); /* col=5*/
                src_temp12_8x16b = _mm_packus_epi16(src_temp12_8x16b, src_temp14_8x16b); /* col=6*/

                src_temp5_8x16b = _mm_unpacklo_epi8(src_temp1_8x16b, src_temp2_8x16b);
                src_temp6_8x16b = _mm_unpackhi_epi8(src_temp1_8x16b, src_temp2_8x16b);

                src_temp15_8x16b = _mm_unpacklo_epi8(src_temp11_8x16b, src_temp12_8x16b);
                src_temp16_8x16b = _mm_unpackhi_epi8(src_temp11_8x16b, src_temp12_8x16b);

                src_temp7_8x16b = _mm_unpacklo_epi16(src_temp5_8x16b, src_temp6_8x16b);
                src_temp8_8x16b = _mm_unpackhi_epi16(src_temp5_8x16b, src_temp6_8x16b);

                src_temp17_8x16b = _mm_unpacklo_epi16(src_temp15_8x16b, src_temp16_8x16b);
                src_temp18_8x16b = _mm_unpackhi_epi16(src_temp15_8x16b, src_temp16_8x16b);

                src_temp1_8x16b = _mm_unpacklo_epi32(src_temp7_8x16b, src_temp17_8x16b);
                src_temp2_8x16b = _mm_unpackhi_epi32(src_temp7_8x16b, src_temp17_8x16b);

                src_temp5_8x16b = _mm_srli_si128(src_temp1_8x16b, 8);
                src_temp6_8x16b = _mm_srli_si128(src_temp2_8x16b, 8);

                src_temp3_8x16b = _mm_unpacklo_epi32(src_temp8_8x16b, src_temp18_8x16b);
                src_temp4_8x16b = _mm_unpackhi_epi32(src_temp8_8x16b, src_temp18_8x16b);

                src_temp7_8x16b = _mm_srli_si128(src_temp3_8x16b, 8);
                src_temp8_8x16b = _mm_srli_si128(src_temp4_8x16b, 8);

                _mm_storel_epi64((__m128i *)(pu1_dst), src_temp8_8x16b);       /* row=0*/
                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * 1)), src_temp4_8x16b);       /* row=1*/
                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * 2)), src_temp7_8x16b);       /* row=2*/
                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * 3)), src_temp3_8x16b);       /* row=3*/
                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * 4)), src_temp6_8x16b);       /* row=4*/
                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * 5)), src_temp2_8x16b);       /* row=5*/

                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * 6)), src_temp5_8x16b);       /* row=6*/
                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * 7)), src_temp1_8x16b);       /* row=7*/

            }
        }
    }

}



/**
*******************************************************************************
*
* @brief
*   Intra prediction interpolation filter for luma mode 11 to mode 17
*
* @par Description:
*    Intraprediction for mode 11 to 17  (negative angle, horizontal mode )
*    with reference  neighboring samples location pointed by 'pu1_ref' to the
*    TU block location pointed by 'pu1_dst'
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


void ihevc_intra_pred_luma_mode_11_to_17_ssse3(UWORD8 *pu1_ref,
                                               WORD32 src_strd,
                                               UWORD8 *pu1_dst,
                                               WORD32 dst_strd,
                                               WORD32 nt,
                                               WORD32 mode)
{

    /* This function and ihevc_intra_pred_luma_mode_19_to_25 are same except*/
    /* for ref main & side samples assignment,can be combined for */
    /* optimzation*/

    WORD32 row, col, k;
    WORD32 two_nt;
    WORD32 intra_pred_ang, inv_ang, inv_ang_sum;
    WORD32 ref_idx;

    __m128i const_temp_4x32b, const_temp2_4x32b, const_temp3_4x32b, const_temp4_4x32b;
    __m128i fract_4x32b,  intra_pred_ang_4x32b;
    __m128i row_4x32b, two_nt_4x32b, ref_main_idx_4x32b, res_temp5_4x32b, sm3;


    UWORD8 ref_tmp[2 * MAX_CU_SIZE + 2];
    UWORD8 *ref_main;
    UWORD8 *ref_temp;
    UNUSED(src_strd);
    inv_ang_sum = 128;
    two_nt    = 2 * nt;
    ref_temp = ref_tmp + 1;
    ref_main = ref_temp + nt - 1;
    intra_pred_ang = gai4_ihevc_ang_table[mode];

    /* For the angles other then 45 degree, interpolation btw 2 neighboring */
    /* samples dependent on distance to obtain destination sample */
    const_temp_4x32b  = _mm_set1_epi16(16);
    const_temp2_4x32b = _mm_set1_epi32(31);
    const_temp3_4x32b = _mm_set1_epi32(32);
    const_temp4_4x32b = _mm_set1_epi32(4);

    two_nt_4x32b = _mm_set1_epi32(1);


    sm3 = _mm_load_si128((__m128i *)&IHEVCE_SHUFFLEMASKY11[0]);

    /* intra_pred_ang = gai4_ihevc_ang_table[mode]; */
    intra_pred_ang_4x32b = _mm_set1_epi32(intra_pred_ang);

    row_4x32b = _mm_set_epi32(4, 3, 2, 1);

    if(nt == 4)
    {

        WORD32 ref_main_idx1, ref_main_idx2, ref_main_idx3, ref_main_idx4;
        int temp11, temp21, temp31, temp41;
//        WORD8  ai1_fract_temp_val[16], ai1_row_temp_val[16];

        __m128i fract1_8x16b, fract2_8x16b;
        __m128i temp1_8x16b, temp2_8x16b, temp3_8x16b, temp4_8x16b;

        __m128i src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b, src_temp4_8x16b;
        __m128i src_temp5_8x16b, src_temp6_8x16b, src_temp7_8x16b, src_temp8_8x16b;
        __m128i ref_main_temp0, ref_main_temp1, ref_main_temp2, zero_8x16b, sign_8x16b;

        /* Intermediate reference samples for negative angle modes */
        /* This have to be removed during optimization*/
        /* For horizontal modes, (ref main = ref left) (ref side = ref above) */
        inv_ang = gai4_ihevc_inv_ang_table[mode - 11];

        ref_main = ref_temp + nt - 1;
        for(k = 0; k < nt + 1; k++)
            ref_temp[k + nt - 1] = pu1_ref[two_nt - k];

        ref_main = ref_temp + nt - 1;
        ref_idx = (nt * intra_pred_ang) >> 5;
        zero_8x16b = _mm_setzero_si128();

        row_4x32b = _mm_set_epi16(4, 3, 2, 1, 4, 3, 2, 1);
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);
        /* SIMD Optimization can be done using look-up table for the loop */
        /* For negative angled derive the main reference samples from side */
        /*  reference samples refer to section 8.4.4.2.6 */
        for(k = -1; k > ref_idx; k--)
        {
            inv_ang_sum += inv_ang;
            ref_main[k] = pu1_ref[two_nt + (inv_ang_sum >> 8)];
        }


        /* pos = ((row + 1) * intra_pred_ang); */
        res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

        sign_8x16b      = _mm_cmpgt_epi16(zero_8x16b, res_temp5_4x32b);
        res_temp5_4x32b = _mm_unpacklo_epi16(res_temp5_4x32b, sign_8x16b);

        /* idx = pos >> 5; */
        fract_4x32b = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

        /* fract = pos & (31); */
        ref_main_idx_4x32b = _mm_add_epi32(two_nt_4x32b, _mm_srai_epi32(res_temp5_4x32b,  5));

        /*(32 - fract) */
        row_4x32b = _mm_sub_epi32(const_temp3_4x32b, fract_4x32b);

        fract1_8x16b = _mm_slli_epi16(fract_4x32b, 8);
        fract2_8x16b = _mm_slli_epi16(row_4x32b, 8);

        fract_4x32b = _mm_or_si128(fract_4x32b, fract1_8x16b);
        row_4x32b = _mm_or_si128(row_4x32b, fract2_8x16b); /*(32 - fract) */

        fract2_8x16b = _mm_unpackhi_epi8(fract_4x32b, row_4x32b);
        fract1_8x16b = _mm_unpacklo_epi8(fract_4x32b, row_4x32b);

        temp1_8x16b =  _mm_shuffle_epi32(fract1_8x16b, 0x00);
        temp2_8x16b =  _mm_shuffle_epi32(fract1_8x16b, 0xaa);
        temp3_8x16b =  _mm_shuffle_epi32(fract2_8x16b, 0x00);
        temp4_8x16b =  _mm_shuffle_epi32(fract2_8x16b, 0xaa);

        ref_main_temp0 = _mm_srli_si128(ref_main_idx_4x32b, 4);  /* next 32 bit values */
        ref_main_temp1 = _mm_srli_si128(ref_main_idx_4x32b, 8);  /* next 32 bit values */
        ref_main_temp2 = _mm_srli_si128(ref_main_idx_4x32b, 12); /* next 32 bit values */
        ref_main_idx1  = _mm_cvtsi128_si32(ref_main_idx_4x32b);    /* col=0*/
        ref_main_idx2  = _mm_cvtsi128_si32(ref_main_temp0);  /* col=1*/
        ref_main_idx3  = _mm_cvtsi128_si32(ref_main_temp1);  /* col=2*/
        ref_main_idx4  = _mm_cvtsi128_si32(ref_main_temp2);  /* col=3*/

        /* loding 8-bit 16 pixels */
        src_temp5_8x16b = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx1)); /* col=0*/
        src_temp6_8x16b = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx2)); /* col=1*/
        src_temp7_8x16b = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx3)); /* col=2*/
        src_temp8_8x16b = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx4)); /* col=3*/

        src_temp1_8x16b =  _mm_shuffle_epi8(src_temp5_8x16b, sm3); /* col=0*/
        src_temp2_8x16b =  _mm_shuffle_epi8(src_temp6_8x16b, sm3); /* col=1*/
        src_temp3_8x16b =  _mm_shuffle_epi8(src_temp7_8x16b, sm3); /* col=2*/
        src_temp4_8x16b =  _mm_shuffle_epi8(src_temp8_8x16b, sm3); /* col=3*/

        /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
        src_temp1_8x16b = _mm_maddubs_epi16(src_temp1_8x16b, temp1_8x16b);
        src_temp2_8x16b = _mm_maddubs_epi16(src_temp2_8x16b, temp2_8x16b);
        src_temp3_8x16b = _mm_maddubs_epi16(src_temp3_8x16b, temp3_8x16b);
        src_temp4_8x16b = _mm_maddubs_epi16(src_temp4_8x16b, temp4_8x16b);

        /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
        src_temp1_8x16b = _mm_add_epi16(src_temp1_8x16b, const_temp_4x32b);
        src_temp2_8x16b = _mm_add_epi16(src_temp2_8x16b, const_temp_4x32b);
        src_temp3_8x16b = _mm_add_epi16(src_temp3_8x16b, const_temp_4x32b);
        src_temp4_8x16b = _mm_add_epi16(src_temp4_8x16b, const_temp_4x32b);

        /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
        src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  5);   /* col=0*/
        src_temp2_8x16b = _mm_srai_epi16(src_temp2_8x16b,  5);   /* col=1*/
        src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  5);   /* col=2*/
        src_temp4_8x16b = _mm_srai_epi16(src_temp4_8x16b,  5);   /* col=3*/

        /* converting 16 bit to 8 bit */
        src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp3_8x16b); /* col=0*/
        src_temp2_8x16b = _mm_packus_epi16(src_temp2_8x16b, src_temp4_8x16b); /* col=1*/


        src_temp5_8x16b = _mm_unpacklo_epi8(src_temp1_8x16b, src_temp2_8x16b);
        src_temp6_8x16b = _mm_unpackhi_epi8(src_temp1_8x16b, src_temp2_8x16b);

        src_temp7_8x16b = _mm_unpacklo_epi16(src_temp5_8x16b, src_temp6_8x16b);
        src_temp1_8x16b = _mm_srli_si128(src_temp7_8x16b, 4);
        src_temp2_8x16b = _mm_srli_si128(src_temp7_8x16b, 8);
        src_temp3_8x16b = _mm_srli_si128(src_temp7_8x16b, 12);

        temp11 = _mm_cvtsi128_si32(src_temp7_8x16b);
        temp21 = _mm_cvtsi128_si32(src_temp1_8x16b);
        temp31 = _mm_cvtsi128_si32(src_temp2_8x16b);
        temp41 = _mm_cvtsi128_si32(src_temp3_8x16b);

        /* loding 8-bit 4 pixels values */
        *(WORD32 *)(&pu1_dst[(0 * dst_strd)]) = temp11;
        *(WORD32 *)(&pu1_dst[(1 * dst_strd)]) = temp21;
        *(WORD32 *)(&pu1_dst[(2 * dst_strd)]) = temp31;
        *(WORD32 *)(&pu1_dst[(3 * dst_strd)]) = temp41;
    }

    else if(nt == 32)
    {


        __m128i temp1, temp2, temp3, temp11, temp12;
        __m128i src_values0, src_values1;
        /* Intermediate reference samples for negative angle modes */

        ref_temp[two_nt - 1] = pu1_ref[two_nt - nt];
        temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + nt + 1));
        temp3 = _mm_loadu_si128((__m128i *)(pu1_ref + nt + 17));
        temp2 = _mm_loadu_si128((__m128i *)IHEVCE_SHUFFLEMASKY3);

        /* For negative angled derive the main reference samples from side */

        src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1)); /*nt-(nt+15)*/
        src_values1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 17)); /*(nt+16)-(two_nt-1)*/

        temp11 = _mm_loadu_si128((__m128i *)(inv_angle_shuffle[17 - mode]));
        temp12 = _mm_loadu_si128((__m128i *)(inv_angle_shuffle[17 - mode] + 16));

        src_values0 = _mm_shuffle_epi8(src_values0, temp2);
        src_values1 = _mm_shuffle_epi8(src_values1, temp2);
        src_values0 = _mm_shuffle_epi8(src_values0, temp12);
        src_values1 = _mm_shuffle_epi8(src_values1, temp11);

        temp1 = _mm_shuffle_epi8(temp1, temp2);
        temp3 = _mm_shuffle_epi8(temp3, temp2);

        _mm_storeu_si128((__m128i *)(ref_temp + nt - 1), temp3);
        _mm_storeu_si128((__m128i *)(ref_temp + nt - 1 + 16), temp1);
        _mm_storeu_si128((__m128i *)(ref_main - 16), src_values0);
        _mm_storeu_si128((__m128i *)(ref_main - nt + inv_angle_shuffle[17 - mode][0]), src_values1);


        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);
        row_4x32b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);
        const_temp2_4x32b = _mm_set1_epi16(31);
        const_temp4_4x32b = _mm_set1_epi16(8);
        const_temp3_4x32b = _mm_set1_epi16(32);
        two_nt_4x32b = _mm_set1_epi16(1);

        for(col = 0; col < nt; col += 8)
        {
            WORD16 pi2_ref_main_idx1, pi2_ref_main_idx2, pi2_ref_main_idx3, pi2_ref_main_idx4;
            WORD16 pi2_ref_main_idx5, pi2_ref_main_idx6, pi2_ref_main_idx7, pi2_ref_main_idx8;
            // WORD8  ai1_fract_temp0_val[16], ai1_fract_temp1_val[16];

            __m128i fract1_8x16b, fract2_8x16b, fract3_8x16b, fract8_8x16b;

            __m128i temp1_8x16b, temp2_8x16b, temp3_8x16b, temp4_8x16b;
            __m128i temp11_8x16b, temp12_8x16b, temp13_8x16b, temp14_8x16b;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

            /* idx = pos >> 5; */
            fract_4x32b = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /* fract = pos & (31); */
            ref_main_idx_4x32b = _mm_add_epi16(two_nt_4x32b, _mm_srai_epi16(res_temp5_4x32b,  5));

            row_4x32b = _mm_add_epi16(row_4x32b, const_temp4_4x32b);
            /*(32 - fract) */
            fract2_8x16b =  _mm_sub_epi16(const_temp3_4x32b, fract_4x32b);

            fract1_8x16b = _mm_slli_epi16(fract_4x32b, 8);
            fract3_8x16b = _mm_slli_epi16(fract2_8x16b, 8); /*(32 - fract) */

            fract_4x32b = _mm_or_si128(fract_4x32b, fract1_8x16b);
            fract2_8x16b = _mm_or_si128(fract2_8x16b, fract3_8x16b); /*(32 - fract) */


            fract8_8x16b = _mm_unpackhi_epi8(fract_4x32b, fract2_8x16b);
            fract_4x32b = _mm_unpacklo_epi8(fract_4x32b, fract2_8x16b);

            temp1_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0x00);
            temp2_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0x55);
            temp3_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0xaa);
            temp4_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0xff);

            temp11_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0x00);
            temp12_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0x55);
            temp13_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0xaa);
            temp14_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0xff);

            pi2_ref_main_idx1 = _mm_extract_epi16(ref_main_idx_4x32b, 0);    /* col=0*/
            pi2_ref_main_idx2 = _mm_extract_epi16(ref_main_idx_4x32b, 1);    /* col=1*/
            pi2_ref_main_idx3 = _mm_extract_epi16(ref_main_idx_4x32b, 2);    /* col=2*/
            pi2_ref_main_idx4 = _mm_extract_epi16(ref_main_idx_4x32b, 3);    /* col=3*/

            pi2_ref_main_idx5 = _mm_extract_epi16(ref_main_idx_4x32b, 4);    /* col=5*/
            pi2_ref_main_idx6 = _mm_extract_epi16(ref_main_idx_4x32b, 5);    /* col=6*/
            pi2_ref_main_idx7 = _mm_extract_epi16(ref_main_idx_4x32b, 6);    /* col=7*/
            pi2_ref_main_idx8 = _mm_extract_epi16(ref_main_idx_4x32b, 7);    /* col=8*/

            for(row = 0; row < nt; row += 8)
            {
                __m128i src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b, src_temp4_8x16b;
                __m128i src_temp5_8x16b, src_temp6_8x16b, src_temp7_8x16b, src_temp8_8x16b;


                __m128i src_temp11_8x16b, src_temp12_8x16b, src_temp13_8x16b, src_temp14_8x16b;
                __m128i src_temp15_8x16b, src_temp16_8x16b, src_temp17_8x16b, src_temp18_8x16b;

                /* loding 8-bit 16 pixels */
                src_temp5_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx1 + row)); /* col=0*/
                src_temp6_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx2 + row)); /* col=1*/
                src_temp7_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx3 + row)); /* col=2*/
                src_temp8_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx4 + row)); /* col=3*/

                src_temp1_8x16b = _mm_srli_si128(src_temp5_8x16b, 1); /* col=0*/
                src_temp2_8x16b = _mm_srli_si128(src_temp6_8x16b, 1); /* col=1*/
                src_temp3_8x16b = _mm_srli_si128(src_temp7_8x16b, 1); /* col=2*/
                src_temp4_8x16b = _mm_srli_si128(src_temp8_8x16b, 1); /* col=3*/

                /* loding 8-bit 16 pixels */
                src_temp15_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx5 + row)); /* col=5*/
                src_temp16_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx6 + row)); /* col=6*/
                src_temp17_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx7 + row)); /* col=7*/
                src_temp18_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx8 + row)); /* col=8*/

                src_temp1_8x16b =  _mm_shuffle_epi8(src_temp5_8x16b, sm3); /* col=0*/
                src_temp2_8x16b =  _mm_shuffle_epi8(src_temp6_8x16b, sm3); /* col=1*/
                src_temp3_8x16b =  _mm_shuffle_epi8(src_temp7_8x16b, sm3); /* col=2*/
                src_temp4_8x16b =  _mm_shuffle_epi8(src_temp8_8x16b, sm3); /* col=3*/

                src_temp11_8x16b =  _mm_shuffle_epi8(src_temp15_8x16b, sm3); /* col=0*/
                src_temp12_8x16b =  _mm_shuffle_epi8(src_temp16_8x16b, sm3); /* col=1*/
                src_temp13_8x16b =  _mm_shuffle_epi8(src_temp17_8x16b, sm3); /* col=2*/
                src_temp14_8x16b =  _mm_shuffle_epi8(src_temp18_8x16b, sm3); /* col=3*/

                /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
                src_temp1_8x16b = _mm_maddubs_epi16(src_temp1_8x16b, temp1_8x16b);
                src_temp2_8x16b = _mm_maddubs_epi16(src_temp2_8x16b, temp2_8x16b);
                src_temp3_8x16b = _mm_maddubs_epi16(src_temp3_8x16b, temp3_8x16b);
                src_temp4_8x16b = _mm_maddubs_epi16(src_temp4_8x16b, temp4_8x16b);

                /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
                src_temp11_8x16b = _mm_maddubs_epi16(src_temp11_8x16b, temp11_8x16b);
                src_temp12_8x16b = _mm_maddubs_epi16(src_temp12_8x16b, temp12_8x16b);
                src_temp13_8x16b = _mm_maddubs_epi16(src_temp13_8x16b, temp13_8x16b);
                src_temp14_8x16b = _mm_maddubs_epi16(src_temp14_8x16b, temp14_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_temp1_8x16b = _mm_add_epi16(src_temp1_8x16b, const_temp_4x32b);
                src_temp2_8x16b = _mm_add_epi16(src_temp2_8x16b, const_temp_4x32b);
                src_temp3_8x16b = _mm_add_epi16(src_temp3_8x16b, const_temp_4x32b);
                src_temp4_8x16b = _mm_add_epi16(src_temp4_8x16b, const_temp_4x32b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  5);   /* col=0*/
                src_temp2_8x16b = _mm_srai_epi16(src_temp2_8x16b,  5);   /* col=1*/
                src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  5);   /* col=2*/
                src_temp4_8x16b = _mm_srai_epi16(src_temp4_8x16b,  5);   /* col=3*/

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_temp11_8x16b = _mm_add_epi16(src_temp11_8x16b, const_temp_4x32b);
                src_temp12_8x16b = _mm_add_epi16(src_temp12_8x16b, const_temp_4x32b);
                src_temp13_8x16b = _mm_add_epi16(src_temp13_8x16b, const_temp_4x32b);
                src_temp14_8x16b = _mm_add_epi16(src_temp14_8x16b, const_temp_4x32b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_temp11_8x16b = _mm_srai_epi16(src_temp11_8x16b,  5);   /* col=5*/
                src_temp12_8x16b = _mm_srai_epi16(src_temp12_8x16b,  5);   /* col=6*/
                src_temp13_8x16b = _mm_srai_epi16(src_temp13_8x16b,  5);   /* col=7*/
                src_temp14_8x16b = _mm_srai_epi16(src_temp14_8x16b,  5);   /* col=8*/

                /* converting 16 bit to 8 bit */
                src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp3_8x16b); /* col=0*/
                src_temp2_8x16b = _mm_packus_epi16(src_temp2_8x16b, src_temp4_8x16b); /* col=1*/

                /* converting 16 bit to 8 bit */
                src_temp11_8x16b = _mm_packus_epi16(src_temp11_8x16b, src_temp13_8x16b); /* col=5*/
                src_temp12_8x16b = _mm_packus_epi16(src_temp12_8x16b, src_temp14_8x16b); /* col=6*/

                src_temp5_8x16b = _mm_unpacklo_epi8(src_temp1_8x16b, src_temp2_8x16b);
                src_temp6_8x16b = _mm_unpackhi_epi8(src_temp1_8x16b, src_temp2_8x16b);

                src_temp15_8x16b = _mm_unpacklo_epi8(src_temp11_8x16b, src_temp12_8x16b);
                src_temp16_8x16b = _mm_unpackhi_epi8(src_temp11_8x16b, src_temp12_8x16b);

                src_temp7_8x16b = _mm_unpacklo_epi16(src_temp5_8x16b, src_temp6_8x16b);
                src_temp8_8x16b = _mm_unpackhi_epi16(src_temp5_8x16b, src_temp6_8x16b);

                src_temp17_8x16b = _mm_unpacklo_epi16(src_temp15_8x16b, src_temp16_8x16b);
                src_temp18_8x16b = _mm_unpackhi_epi16(src_temp15_8x16b, src_temp16_8x16b);


                src_temp1_8x16b = _mm_unpacklo_epi32(src_temp7_8x16b, src_temp17_8x16b);
                src_temp2_8x16b = _mm_unpackhi_epi32(src_temp7_8x16b, src_temp17_8x16b);

                src_temp3_8x16b = _mm_unpacklo_epi32(src_temp8_8x16b, src_temp18_8x16b);
                src_temp4_8x16b = _mm_unpackhi_epi32(src_temp8_8x16b, src_temp18_8x16b);

                src_temp5_8x16b = _mm_srli_si128(src_temp1_8x16b, 8);
                src_temp6_8x16b = _mm_srli_si128(src_temp2_8x16b, 8);
                src_temp7_8x16b = _mm_srli_si128(src_temp3_8x16b, 8);
                src_temp8_8x16b = _mm_srli_si128(src_temp4_8x16b, 8);

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * row)), src_temp1_8x16b);          /* row=0*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 1))), src_temp5_8x16b);       /* row=1*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 2))), src_temp2_8x16b);       /* row=2*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 3))), src_temp6_8x16b);       /* row=4*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 4))), src_temp3_8x16b);       /* row=5*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 5))), src_temp7_8x16b);       /* row=6*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 6))), src_temp4_8x16b);       /* row=7*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 7))), src_temp8_8x16b);       /* row=8*/

            }
        }
    }
    else if(nt == 16)
    {

        __m128i temp1, temp2, temp11, src_values0;
        /* Intermediate reference samples for negative angle modes */
        /* For horizontal modes, (ref main = ref above) (ref side = ref left) */
        ref_temp[two_nt - 1] = pu1_ref[two_nt - nt];
        temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + nt + 1));
        temp2 = _mm_loadu_si128((__m128i *)IHEVCE_SHUFFLEMASKY3);
        src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1)); /*nt-(nt+15)*/

        temp11 = _mm_loadu_si128((__m128i *)(inv_angle_shuffle[17 - mode] + 16));

        src_values0 = _mm_shuffle_epi8(src_values0, temp2);
        temp1 = _mm_shuffle_epi8(temp1, temp2);
        src_values0 = _mm_shuffle_epi8(src_values0, temp11);

        _mm_storeu_si128((__m128i *)(ref_main - nt), src_values0);
        _mm_storeu_si128((__m128i *)(ref_temp + nt - 1), temp1);

        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);
        row_4x32b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);
        const_temp2_4x32b = _mm_set1_epi16(31);
        const_temp4_4x32b = _mm_set1_epi16(8);
        const_temp3_4x32b = _mm_set1_epi16(32);
        two_nt_4x32b = _mm_set1_epi16(1);

        for(col = 0; col < nt; col += 8)
        {
            WORD16 pi2_ref_main_idx1, pi2_ref_main_idx2, pi2_ref_main_idx3, pi2_ref_main_idx4;
            WORD16 pi2_ref_main_idx5, pi2_ref_main_idx6, pi2_ref_main_idx7, pi2_ref_main_idx8;
            // WORD8  ai1_fract_temp0_val[16], ai1_fract_temp1_val[16];

            __m128i fract1_8x16b, fract2_8x16b, fract3_8x16b, fract8_8x16b;

            __m128i temp1_8x16b, temp2_8x16b, temp3_8x16b, temp4_8x16b;
            __m128i temp11_8x16b, temp12_8x16b, temp13_8x16b, temp14_8x16b;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

            /* idx = pos >> 5; */
            fract_4x32b = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /* fract = pos & (31); */
            ref_main_idx_4x32b = _mm_add_epi16(two_nt_4x32b, _mm_srai_epi16(res_temp5_4x32b,  5));

            row_4x32b = _mm_add_epi16(row_4x32b, const_temp4_4x32b);
            /*(32 - fract) */
            fract2_8x16b =  _mm_sub_epi16(const_temp3_4x32b, fract_4x32b);

            fract1_8x16b = _mm_slli_epi16(fract_4x32b, 8);
            fract3_8x16b = _mm_slli_epi16(fract2_8x16b, 8); /*(32 - fract) */

            fract_4x32b = _mm_or_si128(fract_4x32b, fract1_8x16b);
            fract2_8x16b = _mm_or_si128(fract2_8x16b, fract3_8x16b); /*(32 - fract) */


            fract8_8x16b = _mm_unpackhi_epi8(fract_4x32b, fract2_8x16b);
            fract_4x32b = _mm_unpacklo_epi8(fract_4x32b, fract2_8x16b);

            temp1_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0x00);
            temp2_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0x55);
            temp3_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0xaa);
            temp4_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0xff);

            temp11_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0x00);
            temp12_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0x55);
            temp13_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0xaa);
            temp14_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0xff);

            pi2_ref_main_idx1 = _mm_extract_epi16(ref_main_idx_4x32b, 0);    /* col=0*/
            pi2_ref_main_idx2 = _mm_extract_epi16(ref_main_idx_4x32b, 1);    /* col=1*/
            pi2_ref_main_idx3 = _mm_extract_epi16(ref_main_idx_4x32b, 2);    /* col=2*/
            pi2_ref_main_idx4 = _mm_extract_epi16(ref_main_idx_4x32b, 3);    /* col=3*/

            pi2_ref_main_idx5 = _mm_extract_epi16(ref_main_idx_4x32b, 4);    /* col=5*/
            pi2_ref_main_idx6 = _mm_extract_epi16(ref_main_idx_4x32b, 5);    /* col=6*/
            pi2_ref_main_idx7 = _mm_extract_epi16(ref_main_idx_4x32b, 6);    /* col=7*/
            pi2_ref_main_idx8 = _mm_extract_epi16(ref_main_idx_4x32b, 7);    /* col=8*/

            for(row = 0; row < nt; row += 8)
            {
                __m128i src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b, src_temp4_8x16b;
                __m128i src_temp5_8x16b, src_temp6_8x16b, src_temp7_8x16b, src_temp8_8x16b;


                __m128i src_temp11_8x16b, src_temp12_8x16b, src_temp13_8x16b, src_temp14_8x16b;
                __m128i src_temp15_8x16b, src_temp16_8x16b, src_temp17_8x16b, src_temp18_8x16b;

                /* loding 8-bit 16 pixels */
                src_temp5_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx1 + row)); /* col=0*/
                src_temp6_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx2 + row)); /* col=1*/
                src_temp7_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx3 + row)); /* col=2*/
                src_temp8_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx4 + row)); /* col=3*/

                src_temp1_8x16b = _mm_srli_si128(src_temp5_8x16b, 1); /* col=0*/
                src_temp2_8x16b = _mm_srli_si128(src_temp6_8x16b, 1); /* col=1*/
                src_temp3_8x16b = _mm_srli_si128(src_temp7_8x16b, 1); /* col=2*/
                src_temp4_8x16b = _mm_srli_si128(src_temp8_8x16b, 1); /* col=3*/

                /* loding 8-bit 16 pixels */
                src_temp15_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx5 + row)); /* col=5*/
                src_temp16_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx6 + row)); /* col=6*/
                src_temp17_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx7 + row)); /* col=7*/
                src_temp18_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx8 + row)); /* col=8*/

                src_temp1_8x16b =  _mm_shuffle_epi8(src_temp5_8x16b, sm3); /* col=0*/
                src_temp2_8x16b =  _mm_shuffle_epi8(src_temp6_8x16b, sm3); /* col=1*/
                src_temp3_8x16b =  _mm_shuffle_epi8(src_temp7_8x16b, sm3); /* col=2*/
                src_temp4_8x16b =  _mm_shuffle_epi8(src_temp8_8x16b, sm3); /* col=3*/

                src_temp11_8x16b =  _mm_shuffle_epi8(src_temp15_8x16b, sm3); /* col=0*/
                src_temp12_8x16b =  _mm_shuffle_epi8(src_temp16_8x16b, sm3); /* col=1*/
                src_temp13_8x16b =  _mm_shuffle_epi8(src_temp17_8x16b, sm3); /* col=2*/
                src_temp14_8x16b =  _mm_shuffle_epi8(src_temp18_8x16b, sm3); /* col=3*/

                /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
                src_temp1_8x16b = _mm_maddubs_epi16(src_temp1_8x16b, temp1_8x16b);
                src_temp2_8x16b = _mm_maddubs_epi16(src_temp2_8x16b, temp2_8x16b);
                src_temp3_8x16b = _mm_maddubs_epi16(src_temp3_8x16b, temp3_8x16b);
                src_temp4_8x16b = _mm_maddubs_epi16(src_temp4_8x16b, temp4_8x16b);

                /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
                src_temp11_8x16b = _mm_maddubs_epi16(src_temp11_8x16b, temp11_8x16b);
                src_temp12_8x16b = _mm_maddubs_epi16(src_temp12_8x16b, temp12_8x16b);
                src_temp13_8x16b = _mm_maddubs_epi16(src_temp13_8x16b, temp13_8x16b);
                src_temp14_8x16b = _mm_maddubs_epi16(src_temp14_8x16b, temp14_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_temp1_8x16b = _mm_add_epi16(src_temp1_8x16b, const_temp_4x32b);
                src_temp2_8x16b = _mm_add_epi16(src_temp2_8x16b, const_temp_4x32b);
                src_temp3_8x16b = _mm_add_epi16(src_temp3_8x16b, const_temp_4x32b);
                src_temp4_8x16b = _mm_add_epi16(src_temp4_8x16b, const_temp_4x32b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  5);   /* col=0*/
                src_temp2_8x16b = _mm_srai_epi16(src_temp2_8x16b,  5);   /* col=1*/
                src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  5);   /* col=2*/
                src_temp4_8x16b = _mm_srai_epi16(src_temp4_8x16b,  5);   /* col=3*/

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_temp11_8x16b = _mm_add_epi16(src_temp11_8x16b, const_temp_4x32b);
                src_temp12_8x16b = _mm_add_epi16(src_temp12_8x16b, const_temp_4x32b);
                src_temp13_8x16b = _mm_add_epi16(src_temp13_8x16b, const_temp_4x32b);
                src_temp14_8x16b = _mm_add_epi16(src_temp14_8x16b, const_temp_4x32b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_temp11_8x16b = _mm_srai_epi16(src_temp11_8x16b,  5);   /* col=5*/
                src_temp12_8x16b = _mm_srai_epi16(src_temp12_8x16b,  5);   /* col=6*/
                src_temp13_8x16b = _mm_srai_epi16(src_temp13_8x16b,  5);   /* col=7*/
                src_temp14_8x16b = _mm_srai_epi16(src_temp14_8x16b,  5);   /* col=8*/

                /* converting 16 bit to 8 bit */
                src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp3_8x16b); /* col=0*/
                src_temp2_8x16b = _mm_packus_epi16(src_temp2_8x16b, src_temp4_8x16b); /* col=1*/

                /* converting 16 bit to 8 bit */
                src_temp11_8x16b = _mm_packus_epi16(src_temp11_8x16b, src_temp13_8x16b); /* col=5*/
                src_temp12_8x16b = _mm_packus_epi16(src_temp12_8x16b, src_temp14_8x16b); /* col=6*/

                src_temp5_8x16b = _mm_unpacklo_epi8(src_temp1_8x16b, src_temp2_8x16b);
                src_temp6_8x16b = _mm_unpackhi_epi8(src_temp1_8x16b, src_temp2_8x16b);

                src_temp15_8x16b = _mm_unpacklo_epi8(src_temp11_8x16b, src_temp12_8x16b);
                src_temp16_8x16b = _mm_unpackhi_epi8(src_temp11_8x16b, src_temp12_8x16b);

                src_temp7_8x16b = _mm_unpacklo_epi16(src_temp5_8x16b, src_temp6_8x16b);
                src_temp8_8x16b = _mm_unpackhi_epi16(src_temp5_8x16b, src_temp6_8x16b);

                src_temp17_8x16b = _mm_unpacklo_epi16(src_temp15_8x16b, src_temp16_8x16b);
                src_temp18_8x16b = _mm_unpackhi_epi16(src_temp15_8x16b, src_temp16_8x16b);


                src_temp1_8x16b = _mm_unpacklo_epi32(src_temp7_8x16b, src_temp17_8x16b);
                src_temp2_8x16b = _mm_unpackhi_epi32(src_temp7_8x16b, src_temp17_8x16b);

                src_temp3_8x16b = _mm_unpacklo_epi32(src_temp8_8x16b, src_temp18_8x16b);
                src_temp4_8x16b = _mm_unpackhi_epi32(src_temp8_8x16b, src_temp18_8x16b);

                src_temp5_8x16b = _mm_srli_si128(src_temp1_8x16b, 8);
                src_temp6_8x16b = _mm_srli_si128(src_temp2_8x16b, 8);
                src_temp7_8x16b = _mm_srli_si128(src_temp3_8x16b, 8);
                src_temp8_8x16b = _mm_srli_si128(src_temp4_8x16b, 8);

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * row)), src_temp1_8x16b);          /* row=0*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 1))), src_temp5_8x16b);       /* row=1*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 2))), src_temp2_8x16b);       /* row=2*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 3))), src_temp6_8x16b);       /* row=4*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 4))), src_temp3_8x16b);       /* row=5*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 5))), src_temp7_8x16b);       /* row=6*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 6))), src_temp4_8x16b);       /* row=7*/

                _mm_storel_epi64((__m128i *)(pu1_dst + col + (dst_strd * (row + 7))), src_temp8_8x16b);       /* row=8*/

            }
        }
    }
    else
    {


        __m128i temp1, temp2, temp11, src_values0;
        /* Intermediate reference samples for negative angle modes */
        /* For horizontal modes, (ref main = ref above) (ref side = ref left) */
        ref_temp[two_nt - 1] = pu1_ref[nt];
        temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + 1));

        /* For negative angled derive the main reference samples from side */

        src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 1)); /*nt-(nt+15)*/
        temp2 = _mm_loadu_si128((__m128i *)IHEVCE_SHUFFLEMASKY3);
        temp11 = _mm_loadu_si128((__m128i *)(inv_angle_shuffle[17 - mode] + 16));

        src_values0 = _mm_shuffle_epi8(src_values0, temp2);
        temp1 = _mm_shuffle_epi8(temp1, temp2);
        src_values0 = _mm_shuffle_epi8(src_values0, temp11);
        src_values0 = _mm_srli_si128(src_values0, 8);

        _mm_storel_epi64((__m128i *)(ref_temp + nt - 1), temp1);
        _mm_storel_epi64((__m128i *)(ref_main - nt), src_values0);


        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);
        row_4x32b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);
        const_temp2_4x32b = _mm_set1_epi16(31);
        const_temp4_4x32b = _mm_set1_epi16(8);
        const_temp3_4x32b = _mm_set1_epi16(32);
        two_nt_4x32b = _mm_set1_epi16(1);

        {
            WORD16 pi2_ref_main_idx1, pi2_ref_main_idx2, pi2_ref_main_idx3, pi2_ref_main_idx4;
            WORD16 pi2_ref_main_idx5, pi2_ref_main_idx6, pi2_ref_main_idx7, pi2_ref_main_idx8;
            //WORD8  ai1_fract_temp0_val[16], ai1_fract_temp1_val[16];

            __m128i fract1_8x16b, fract2_8x16b, fract3_8x16b, fract8_8x16b;

            __m128i temp1_8x16b, temp2_8x16b, temp3_8x16b, temp4_8x16b;
            __m128i temp11_8x16b, temp12_8x16b, temp13_8x16b, temp14_8x16b;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

            /* idx = pos >> 5; */
            fract_4x32b = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /* fract = pos & (31); */
            ref_main_idx_4x32b = _mm_add_epi16(two_nt_4x32b, _mm_srai_epi16(res_temp5_4x32b,  5));

            /*(32 - fract) */
            fract2_8x16b =  _mm_sub_epi16(const_temp3_4x32b, fract_4x32b);

            fract1_8x16b = _mm_slli_epi16(fract_4x32b, 8);
            fract3_8x16b = _mm_slli_epi16(fract2_8x16b, 8); /*(32 - fract) */

            fract_4x32b = _mm_or_si128(fract_4x32b, fract1_8x16b);
            fract2_8x16b = _mm_or_si128(fract2_8x16b, fract3_8x16b); /*(32 - fract) */

            fract8_8x16b = _mm_unpackhi_epi8(fract_4x32b, fract2_8x16b);
            fract_4x32b = _mm_unpacklo_epi8(fract_4x32b, fract2_8x16b);

            temp1_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0x00);
            temp2_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0x55);
            temp3_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0xaa);
            temp4_8x16b =  _mm_shuffle_epi32(fract_4x32b, 0xff);

            temp11_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0x00);
            temp12_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0x55);
            temp13_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0xaa);
            temp14_8x16b =  _mm_shuffle_epi32(fract8_8x16b, 0xff);

            pi2_ref_main_idx1 = _mm_extract_epi16(ref_main_idx_4x32b, 0);    /* col=0*/
            pi2_ref_main_idx2 = _mm_extract_epi16(ref_main_idx_4x32b, 1);    /* col=1*/
            pi2_ref_main_idx3 = _mm_extract_epi16(ref_main_idx_4x32b, 2);    /* col=2*/
            pi2_ref_main_idx4 = _mm_extract_epi16(ref_main_idx_4x32b, 3);    /* col=3*/

            pi2_ref_main_idx5 = _mm_extract_epi16(ref_main_idx_4x32b, 4);    /* col=5*/
            pi2_ref_main_idx6 = _mm_extract_epi16(ref_main_idx_4x32b, 5);    /* col=6*/
            pi2_ref_main_idx7 = _mm_extract_epi16(ref_main_idx_4x32b, 6);    /* col=7*/
            pi2_ref_main_idx8 = _mm_extract_epi16(ref_main_idx_4x32b, 7);    /* col=8*/

            {
                __m128i src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b, src_temp4_8x16b;
                __m128i src_temp5_8x16b, src_temp6_8x16b, src_temp7_8x16b, src_temp8_8x16b;

                __m128i src_temp11_8x16b, src_temp12_8x16b, src_temp13_8x16b, src_temp14_8x16b;
                __m128i src_temp15_8x16b, src_temp16_8x16b, src_temp17_8x16b, src_temp18_8x16b;

                /* loding 8-bit 16 pixels */
                src_temp5_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx1)); /* col=0*/
                src_temp6_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx2)); /* col=1*/
                src_temp7_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx3)); /* col=2*/
                src_temp8_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx4)); /* col=3*/

                /* loding 8-bit 16 pixels */
                src_temp15_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx5)); /* col=5*/
                src_temp16_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx6)); /* col=6*/
                src_temp17_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx7)); /* col=7*/
                src_temp18_8x16b = _mm_loadu_si128((__m128i *)(ref_main + pi2_ref_main_idx8)); /* col=8*/

                src_temp1_8x16b =  _mm_shuffle_epi8(src_temp5_8x16b, sm3); /* col=0*/
                src_temp2_8x16b =  _mm_shuffle_epi8(src_temp6_8x16b, sm3); /* col=1*/
                src_temp3_8x16b =  _mm_shuffle_epi8(src_temp7_8x16b, sm3); /* col=2*/
                src_temp4_8x16b =  _mm_shuffle_epi8(src_temp8_8x16b, sm3); /* col=3*/

                src_temp11_8x16b =  _mm_shuffle_epi8(src_temp15_8x16b, sm3); /* col=0*/
                src_temp12_8x16b =  _mm_shuffle_epi8(src_temp16_8x16b, sm3); /* col=1*/
                src_temp13_8x16b =  _mm_shuffle_epi8(src_temp17_8x16b, sm3); /* col=2*/
                src_temp14_8x16b =  _mm_shuffle_epi8(src_temp18_8x16b, sm3); /* col=3*/

                /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
                src_temp1_8x16b = _mm_maddubs_epi16(src_temp1_8x16b, temp1_8x16b);
                src_temp2_8x16b = _mm_maddubs_epi16(src_temp2_8x16b, temp2_8x16b);
                src_temp3_8x16b = _mm_maddubs_epi16(src_temp3_8x16b, temp3_8x16b);
                src_temp4_8x16b = _mm_maddubs_epi16(src_temp4_8x16b, temp4_8x16b);

                /* fract*(pu1_ref[ref_main_idx + 1]- pu1_ref[ref_main_idx]) */
                src_temp11_8x16b = _mm_maddubs_epi16(src_temp11_8x16b, temp11_8x16b);
                src_temp12_8x16b = _mm_maddubs_epi16(src_temp12_8x16b, temp12_8x16b);
                src_temp13_8x16b = _mm_maddubs_epi16(src_temp13_8x16b, temp13_8x16b);
                src_temp14_8x16b = _mm_maddubs_epi16(src_temp14_8x16b, temp14_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_temp1_8x16b = _mm_add_epi16(src_temp1_8x16b, const_temp_4x32b);
                src_temp2_8x16b = _mm_add_epi16(src_temp2_8x16b, const_temp_4x32b);
                src_temp3_8x16b = _mm_add_epi16(src_temp3_8x16b, const_temp_4x32b);
                src_temp4_8x16b = _mm_add_epi16(src_temp4_8x16b, const_temp_4x32b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_temp1_8x16b = _mm_srai_epi16(src_temp1_8x16b,  5);   /* row=0*/
                src_temp2_8x16b = _mm_srai_epi16(src_temp2_8x16b,  5);   /* row=1*/
                src_temp3_8x16b = _mm_srai_epi16(src_temp3_8x16b,  5);   /* row=2*/
                src_temp4_8x16b = _mm_srai_epi16(src_temp4_8x16b,  5);   /* row=3*/

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_temp11_8x16b = _mm_add_epi16(src_temp11_8x16b, const_temp_4x32b);
                src_temp12_8x16b = _mm_add_epi16(src_temp12_8x16b, const_temp_4x32b);
                src_temp13_8x16b = _mm_add_epi16(src_temp13_8x16b, const_temp_4x32b);
                src_temp14_8x16b = _mm_add_epi16(src_temp14_8x16b, const_temp_4x32b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_temp11_8x16b = _mm_srai_epi16(src_temp11_8x16b,  5);   /* col=5*/
                src_temp12_8x16b = _mm_srai_epi16(src_temp12_8x16b,  5);   /* col=6*/
                src_temp13_8x16b = _mm_srai_epi16(src_temp13_8x16b,  5);   /* col=7*/
                src_temp14_8x16b = _mm_srai_epi16(src_temp14_8x16b,  5);   /* col=8*/

                /* converting 16 bit to 8 bit */
                src_temp1_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp3_8x16b); /* col=0*/
                src_temp2_8x16b = _mm_packus_epi16(src_temp2_8x16b, src_temp4_8x16b); /* col=1*/

                /* converting 16 bit to 8 bit */
                src_temp11_8x16b = _mm_packus_epi16(src_temp11_8x16b, src_temp13_8x16b); /* col=4*/
                src_temp12_8x16b = _mm_packus_epi16(src_temp12_8x16b, src_temp14_8x16b); /* col=5*/

                src_temp5_8x16b = _mm_unpacklo_epi8(src_temp1_8x16b, src_temp2_8x16b);
                src_temp6_8x16b = _mm_unpackhi_epi8(src_temp1_8x16b, src_temp2_8x16b);

                src_temp15_8x16b = _mm_unpacklo_epi8(src_temp11_8x16b, src_temp12_8x16b);
                src_temp16_8x16b = _mm_unpackhi_epi8(src_temp11_8x16b, src_temp12_8x16b);

                src_temp7_8x16b = _mm_unpacklo_epi16(src_temp5_8x16b, src_temp6_8x16b);
                src_temp8_8x16b = _mm_unpackhi_epi16(src_temp5_8x16b, src_temp6_8x16b);

                src_temp17_8x16b = _mm_unpacklo_epi16(src_temp15_8x16b, src_temp16_8x16b);
                src_temp18_8x16b = _mm_unpackhi_epi16(src_temp15_8x16b, src_temp16_8x16b);


                src_temp1_8x16b = _mm_unpacklo_epi32(src_temp7_8x16b, src_temp17_8x16b);
                src_temp2_8x16b = _mm_unpackhi_epi32(src_temp7_8x16b, src_temp17_8x16b);

                src_temp3_8x16b = _mm_unpacklo_epi32(src_temp8_8x16b, src_temp18_8x16b);
                src_temp4_8x16b = _mm_unpackhi_epi32(src_temp8_8x16b, src_temp18_8x16b);

                src_temp5_8x16b = _mm_srli_si128(src_temp1_8x16b, 8);
                src_temp6_8x16b = _mm_srli_si128(src_temp2_8x16b, 8);
                src_temp7_8x16b = _mm_srli_si128(src_temp3_8x16b, 8);
                src_temp8_8x16b = _mm_srli_si128(src_temp4_8x16b, 8);

                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * 0)), src_temp1_8x16b);       /* row=0*/

                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * (1))), src_temp5_8x16b);       /* row=1*/

                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * (2))), src_temp2_8x16b);       /* row=2*/

                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * (3))), src_temp6_8x16b);       /* row=3*/

                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * (4))), src_temp3_8x16b);       /* row=4*/

                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * (5))), src_temp7_8x16b);       /* row=5*/

                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * (6))), src_temp4_8x16b);       /* row=6*/

                _mm_storel_epi64((__m128i *)(pu1_dst + (dst_strd * (7))), src_temp8_8x16b);       /* row=7*/

            }
        }
    }

}


/**
*******************************************************************************
*
* @brief
*   Intra prediction interpolation filter for luma mode 19 to mode 25
*
* @par Description:
*    Intraprediction for mode 19 to 25  (negative angle, vertical mode ) with
*    reference  neighboring samples location pointed by 'pu1_ref' to the  TU
*    block location pointed by 'pu1_dst'
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

void ihevc_intra_pred_luma_mode_19_to_25_ssse3(UWORD8 *pu1_ref,
                                               WORD32 src_strd,
                                               UWORD8 *pu1_dst,
                                               WORD32 dst_strd,
                                               WORD32 nt,
                                               WORD32 mode)
{

    WORD32 row, k;
    WORD32 two_nt, intra_pred_ang;
    WORD32 inv_ang, inv_ang_sum;
    //WORD32 ref_main_idx, pos, fract, idx;
    WORD32 ref_idx;
    UWORD8 ref_tmp[(2 * MAX_CU_SIZE) + 2];
    UWORD8 *ref_main, *ref_temp;

    __m128i  /*fract_8x16b,*/ const_temp_8x16b, sm3;
    __m128i temp1, temp2, temp3, temp4;
    __m128i temp11, temp12, temp13, temp14;
    UNUSED(src_strd);
    two_nt = 2 * nt;
    intra_pred_ang = gai4_ihevc_ang_table[mode];
    inv_ang = gai4_ihevc_inv_ang_table[mode - 12];

    /* Intermediate reference samples for negative angle modes */
    /* This have to be removed during optimization*/
    /* For horizontal modes, (ref main = ref above) (ref side = ref left) */
    ref_temp = ref_tmp + 1;
    ref_main = ref_temp + nt - 1;


    sm3 = _mm_load_si128((__m128i *)&IHEVCE_SHUFFLEMASKY11[0]);



    const_temp_8x16b = _mm_set1_epi16(16);

    if(nt == 32)
    {

        __m128i const_temp2_4x32b, const_temp3_4x32b, const_temp8_4x32b;
        __m128i src_values10, src_values11, intra_pred_ang_4x32b;
        __m128i row_4x32b, two_nt_4x32b, src_values12;

        __m128i src_values0, src_values1, src_values2, src_values3;
        __m128i  src_values4, src_values5, src_values6, src_values7;
        WORD32 col = 0;

        /* Intermediate reference samples for negative angle modes */
        /* This have to be removed during optimization*/
        /* For horizontal modes, (ref main = ref above) (ref side = ref left) */
        ref_temp[two_nt - 1] = pu1_ref[two_nt + nt];
        temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt));
        temp3 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt + 16));

        /* SIMD Optimization can be done using look-up table for the loop */
        /* For negative angled derive the main reference samples from side */
        /*  reference samples refer to section 8.4.4.2.6 */
        src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - nt)); /*nt-(nt+15)*/
        src_values1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - 16)); /*(nt+16)-(two_nt-1)*/

        temp11 = _mm_loadu_si128((__m128i *)(inv_angle_shuffle[mode - 19]));
        temp12 = _mm_loadu_si128((__m128i *)(inv_angle_shuffle[mode - 19] + 16));

        src_values0 = _mm_shuffle_epi8(src_values0, temp11);
        src_values1 = _mm_shuffle_epi8(src_values1, temp12);

        _mm_storeu_si128((__m128i *)(ref_temp + nt - 1), temp1);
        _mm_storeu_si128((__m128i *)(ref_temp + nt - 1 + 16), temp3);
        _mm_storeu_si128((__m128i *)(ref_main - 16), src_values1);
        _mm_storeu_si128((__m128i *)(ref_main - nt + inv_angle_shuffle[mode - 19][0]), src_values0);

        const_temp2_4x32b = _mm_set1_epi16(31);
        const_temp3_4x32b = _mm_set1_epi16(32);
        const_temp8_4x32b = _mm_set1_epi16(8);

        two_nt_4x32b = _mm_set1_epi16(1);

        /* intra_pred_ang = gai4_ihevc_ang_table[mode]; */
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);

        row_4x32b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);

        for(row = 0; row < nt; row += 8)
        {

            WORD16 ref_main_idx[9];

            __m128i res_temp5_4x32b;
            __m128i fract1_8x16b, fract2_8x16b;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

            /* fract = pos & (31); */
            src_values12 = _mm_add_epi16(two_nt_4x32b, _mm_srai_epi16(res_temp5_4x32b,  5));

            /* idx = pos >> 5; */
            src_values11 = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /*(32 - fract) */
            src_values10 = _mm_sub_epi16(const_temp3_4x32b, src_values11);

            fract1_8x16b = _mm_slli_epi16(src_values11, 8);
            fract2_8x16b = _mm_slli_epi16(src_values10, 8);

            src_values11 = _mm_or_si128(src_values11, fract1_8x16b);
            src_values10 = _mm_or_si128(src_values10, fract2_8x16b); /*(32 - fract) */

            fract2_8x16b = _mm_unpackhi_epi8(src_values11, src_values10);
            fract1_8x16b = _mm_unpacklo_epi8(src_values11, src_values10);

            temp1 =  _mm_shuffle_epi32(fract1_8x16b, 0x00);
            temp2 =  _mm_shuffle_epi32(fract1_8x16b, 0x55);
            temp3 =  _mm_shuffle_epi32(fract1_8x16b, 0xaa);
            temp4 =  _mm_shuffle_epi32(fract1_8x16b, 0xff);

            temp11 =  _mm_shuffle_epi32(fract2_8x16b, 0x00);
            temp12 =  _mm_shuffle_epi32(fract2_8x16b, 0x55);
            temp13 =  _mm_shuffle_epi32(fract2_8x16b, 0xaa);
            temp14 =  _mm_shuffle_epi32(fract2_8x16b, 0xff);

            row_4x32b = _mm_add_epi16(row_4x32b, const_temp8_4x32b);
            _mm_storeu_si128((__m128i *)ref_main_idx, src_values12);
            for(col = 0; col < nt; col += 16)
            {
                src_values0 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[0] + col));
                src_values1 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[1] + col));
                src_values2 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[2] + col));
                src_values3 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[3] + col));
                src_values4 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[0] + 8 + col));
                src_values5 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[1] + 8 + col));
                src_values6 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[2] + 8 + col));
                src_values7 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[3] + 8 + col));

                src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
                src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
                src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
                src_values3 =  _mm_shuffle_epi8(src_values3, sm3);
                src_values4 =  _mm_shuffle_epi8(src_values4, sm3);
                src_values5 =  _mm_shuffle_epi8(src_values5, sm3);
                src_values6 =  _mm_shuffle_epi8(src_values6, sm3);
                src_values7 =  _mm_shuffle_epi8(src_values7, sm3);


                src_values0 = _mm_maddubs_epi16(src_values0, temp1);
                src_values1 = _mm_maddubs_epi16(src_values1, temp2);
                src_values2 = _mm_maddubs_epi16(src_values2, temp3);
                src_values3 = _mm_maddubs_epi16(src_values3, temp4);
                src_values4 = _mm_maddubs_epi16(src_values4, temp1);
                src_values5 = _mm_maddubs_epi16(src_values5, temp2);
                src_values6 = _mm_maddubs_epi16(src_values6, temp3);
                src_values7 = _mm_maddubs_epi16(src_values7, temp4);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
                src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
                src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
                src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);
                src_values4 = _mm_add_epi16(src_values4, const_temp_8x16b);
                src_values5 = _mm_add_epi16(src_values5, const_temp_8x16b);
                src_values6 = _mm_add_epi16(src_values6, const_temp_8x16b);
                src_values7 = _mm_add_epi16(src_values7, const_temp_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_values0 = _mm_srai_epi16(src_values0,  5);
                src_values1 = _mm_srai_epi16(src_values1,  5);
                src_values2 = _mm_srai_epi16(src_values2,  5);
                src_values3 = _mm_srai_epi16(src_values3,  5);
                src_values4 = _mm_srai_epi16(src_values4,  5);
                src_values5 = _mm_srai_epi16(src_values5,  5);
                src_values6 = _mm_srai_epi16(src_values6,  5);
                src_values7 = _mm_srai_epi16(src_values7,  5);

                /* converting 16 bit to 8 bit */
                src_values0 = _mm_packus_epi16(src_values0, src_values4);
                src_values1 = _mm_packus_epi16(src_values1, src_values5);
                src_values2 = _mm_packus_epi16(src_values2, src_values6);
                src_values3 = _mm_packus_epi16(src_values3, src_values7);

                /* loading 8-bit 8 pixels values */
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (0) * dst_strd), src_values0);       /* row=0*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (1) * dst_strd), src_values1);   /* row=1*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (2) * dst_strd), src_values2);   /* row=2*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (3) * dst_strd), src_values3);   /* row=3*/


                src_values0 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[4] + col));
                src_values1 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[5] + col));
                src_values2 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[6] + col));
                src_values3 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[7] + col));
                src_values4 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[4] + 8 + col));
                src_values5 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[5] + 8 + col));
                src_values6 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[6] + 8 + col));
                src_values7 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[7] + 8 + col));

                src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
                src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
                src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
                src_values3 =  _mm_shuffle_epi8(src_values3, sm3);
                src_values4 =  _mm_shuffle_epi8(src_values4, sm3);
                src_values5 =  _mm_shuffle_epi8(src_values5, sm3);
                src_values6 =  _mm_shuffle_epi8(src_values6, sm3);
                src_values7 =  _mm_shuffle_epi8(src_values7, sm3);


                src_values0 = _mm_maddubs_epi16(src_values0, temp11);
                src_values1 = _mm_maddubs_epi16(src_values1, temp12);
                src_values2 = _mm_maddubs_epi16(src_values2, temp13);
                src_values3 = _mm_maddubs_epi16(src_values3, temp14);
                src_values4 = _mm_maddubs_epi16(src_values4, temp11);
                src_values5 = _mm_maddubs_epi16(src_values5, temp12);
                src_values6 = _mm_maddubs_epi16(src_values6, temp13);
                src_values7 = _mm_maddubs_epi16(src_values7, temp14);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
                src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
                src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
                src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);
                src_values4 = _mm_add_epi16(src_values4, const_temp_8x16b);
                src_values5 = _mm_add_epi16(src_values5, const_temp_8x16b);
                src_values6 = _mm_add_epi16(src_values6, const_temp_8x16b);
                src_values7 = _mm_add_epi16(src_values7, const_temp_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_values0 = _mm_srai_epi16(src_values0,  5);
                src_values1 = _mm_srai_epi16(src_values1,  5);
                src_values2 = _mm_srai_epi16(src_values2,  5);
                src_values3 = _mm_srai_epi16(src_values3,  5);
                src_values4 = _mm_srai_epi16(src_values4,  5);
                src_values5 = _mm_srai_epi16(src_values5,  5);
                src_values6 = _mm_srai_epi16(src_values6,  5);
                src_values7 = _mm_srai_epi16(src_values7,  5);

                /* converting 16 bit to 8 bit */
                src_values0 = _mm_packus_epi16(src_values0, src_values4);
                src_values1 = _mm_packus_epi16(src_values1, src_values5);
                src_values2 = _mm_packus_epi16(src_values2, src_values6);
                src_values3 = _mm_packus_epi16(src_values3, src_values7);

                /* loading 8-bit 8 pixels values */
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (4) * dst_strd), src_values0);   /* row=4*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (5) * dst_strd), src_values1);   /* row=5*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (6) * dst_strd), src_values2);   /* row=6*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (7) * dst_strd), src_values3);   /* row=7*/

            }
            pu1_dst += 8 * dst_strd;
        }

    }
    else if(nt == 16) /* for nt = 16 case */
    {

        __m128i const_temp2_4x32b, const_temp3_4x32b, const_temp8_4x32b;
        __m128i src_values10, src_values11, intra_pred_ang_4x32b;
        __m128i row_4x32b, two_nt_4x32b, src_values12;
        __m128i src_values0, src_values1, src_values2, src_values3;
        __m128i  src_values4, src_values5, src_values6, src_values7;


        /* Intermediate reference samples for negative angle modes */
        /* For horizontal modes, (ref main = ref above) (ref side = ref left) */
        ref_temp[two_nt - 1] = pu1_ref[two_nt + nt];
        temp1 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt));

        src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref + two_nt - nt)); /*nt-(nt+15)*/

        temp11 = _mm_loadu_si128((__m128i *)(inv_angle_shuffle[mode - 19] + 16));

        src_values0 = _mm_shuffle_epi8(src_values0, temp11);

        _mm_storeu_si128((__m128i *)(ref_main - nt), src_values0);
        _mm_storeu_si128((__m128i *)(ref_temp + nt - 1), temp1);

        const_temp2_4x32b = _mm_set1_epi16(31);
        const_temp3_4x32b = _mm_set1_epi16(32);
        const_temp8_4x32b = _mm_set1_epi16(8);

        two_nt_4x32b = _mm_set1_epi16(1);

        /* intra_pred_ang = gai4_ihevc_ang_table[mode]; */
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);

        row_4x32b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);

        for(row = 0; row < nt; row += 8)
        {

            WORD16 ref_main_idx[9];

            __m128i res_temp5_4x32b;
            __m128i fract1_8x16b, fract2_8x16b;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

            /* fract = pos & (31); */
            src_values12 = _mm_add_epi16(two_nt_4x32b, _mm_srai_epi16(res_temp5_4x32b,  5));

            /* idx = pos >> 5; */
            src_values11 = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /*(32 - fract) */
            src_values10 = _mm_sub_epi16(const_temp3_4x32b, src_values11);

            fract1_8x16b = _mm_slli_epi16(src_values11, 8);
            fract2_8x16b = _mm_slli_epi16(src_values10, 8);

            src_values11 = _mm_or_si128(src_values11, fract1_8x16b);
            src_values10 = _mm_or_si128(src_values10, fract2_8x16b); /*(32 - fract) */

            fract2_8x16b = _mm_unpackhi_epi8(src_values11, src_values10);
            fract1_8x16b = _mm_unpacklo_epi8(src_values11, src_values10);

            temp1 =  _mm_shuffle_epi32(fract1_8x16b, 0x00);
            temp2 =  _mm_shuffle_epi32(fract1_8x16b, 0x55);
            temp3 =  _mm_shuffle_epi32(fract1_8x16b, 0xaa);
            temp4 =  _mm_shuffle_epi32(fract1_8x16b, 0xff);

            temp11 =  _mm_shuffle_epi32(fract2_8x16b, 0x00);
            temp12 =  _mm_shuffle_epi32(fract2_8x16b, 0x55);
            temp13 =  _mm_shuffle_epi32(fract2_8x16b, 0xaa);
            temp14 =  _mm_shuffle_epi32(fract2_8x16b, 0xff);

            row_4x32b = _mm_add_epi16(row_4x32b, const_temp8_4x32b);
            _mm_storeu_si128((__m128i *)ref_main_idx, src_values12);

            {
                src_values0 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[0]));
                src_values1 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[1]));
                src_values2 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[2]));
                src_values3 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[3]));
                src_values4 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[0] + 8));
                src_values5 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[1] + 8));
                src_values6 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[2] + 8));
                src_values7 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[3] + 8));

                src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
                src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
                src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
                src_values3 =  _mm_shuffle_epi8(src_values3, sm3);
                src_values4 =  _mm_shuffle_epi8(src_values4, sm3);
                src_values5 =  _mm_shuffle_epi8(src_values5, sm3);
                src_values6 =  _mm_shuffle_epi8(src_values6, sm3);
                src_values7 =  _mm_shuffle_epi8(src_values7, sm3);


                src_values0 = _mm_maddubs_epi16(src_values0, temp1);
                src_values1 = _mm_maddubs_epi16(src_values1, temp2);
                src_values2 = _mm_maddubs_epi16(src_values2, temp3);
                src_values3 = _mm_maddubs_epi16(src_values3, temp4);
                src_values4 = _mm_maddubs_epi16(src_values4, temp1);
                src_values5 = _mm_maddubs_epi16(src_values5, temp2);
                src_values6 = _mm_maddubs_epi16(src_values6, temp3);
                src_values7 = _mm_maddubs_epi16(src_values7, temp4);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
                src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
                src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
                src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);
                src_values4 = _mm_add_epi16(src_values4, const_temp_8x16b);
                src_values5 = _mm_add_epi16(src_values5, const_temp_8x16b);
                src_values6 = _mm_add_epi16(src_values6, const_temp_8x16b);
                src_values7 = _mm_add_epi16(src_values7, const_temp_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_values0 = _mm_srai_epi16(src_values0,  5);
                src_values1 = _mm_srai_epi16(src_values1,  5);
                src_values2 = _mm_srai_epi16(src_values2,  5);
                src_values3 = _mm_srai_epi16(src_values3,  5);
                src_values4 = _mm_srai_epi16(src_values4,  5);
                src_values5 = _mm_srai_epi16(src_values5,  5);
                src_values6 = _mm_srai_epi16(src_values6,  5);
                src_values7 = _mm_srai_epi16(src_values7,  5);

                /* converting 16 bit to 8 bit */
                src_values0 = _mm_packus_epi16(src_values0, src_values4);
                src_values1 = _mm_packus_epi16(src_values1, src_values5);
                src_values2 = _mm_packus_epi16(src_values2, src_values6);
                src_values3 = _mm_packus_epi16(src_values3, src_values7);

                /* loading 8-bit 8 pixels values */
                _mm_storeu_si128((__m128i *)(pu1_dst + (0) * dst_strd), src_values0);       /* row=0*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (1) * dst_strd), src_values1);   /* row=1*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (2) * dst_strd), src_values2);   /* row=2*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (3) * dst_strd), src_values3);   /* row=3*/


                src_values0 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[4]));
                src_values1 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[5]));
                src_values2 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[6]));
                src_values3 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[7]));
                src_values4 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[4] + 8));
                src_values5 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[5] + 8));
                src_values6 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[6] + 8));
                src_values7 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[7] + 8));

                src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
                src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
                src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
                src_values3 =  _mm_shuffle_epi8(src_values3, sm3);
                src_values4 =  _mm_shuffle_epi8(src_values4, sm3);
                src_values5 =  _mm_shuffle_epi8(src_values5, sm3);
                src_values6 =  _mm_shuffle_epi8(src_values6, sm3);
                src_values7 =  _mm_shuffle_epi8(src_values7, sm3);


                src_values0 = _mm_maddubs_epi16(src_values0, temp11);
                src_values1 = _mm_maddubs_epi16(src_values1, temp12);
                src_values2 = _mm_maddubs_epi16(src_values2, temp13);
                src_values3 = _mm_maddubs_epi16(src_values3, temp14);
                src_values4 = _mm_maddubs_epi16(src_values4, temp11);
                src_values5 = _mm_maddubs_epi16(src_values5, temp12);
                src_values6 = _mm_maddubs_epi16(src_values6, temp13);
                src_values7 = _mm_maddubs_epi16(src_values7, temp14);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
                src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
                src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
                src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);
                src_values4 = _mm_add_epi16(src_values4, const_temp_8x16b);
                src_values5 = _mm_add_epi16(src_values5, const_temp_8x16b);
                src_values6 = _mm_add_epi16(src_values6, const_temp_8x16b);
                src_values7 = _mm_add_epi16(src_values7, const_temp_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_values0 = _mm_srai_epi16(src_values0,  5);
                src_values1 = _mm_srai_epi16(src_values1,  5);
                src_values2 = _mm_srai_epi16(src_values2,  5);
                src_values3 = _mm_srai_epi16(src_values3,  5);
                src_values4 = _mm_srai_epi16(src_values4,  5);
                src_values5 = _mm_srai_epi16(src_values5,  5);
                src_values6 = _mm_srai_epi16(src_values6,  5);
                src_values7 = _mm_srai_epi16(src_values7,  5);

                /* converting 16 bit to 8 bit */
                src_values0 = _mm_packus_epi16(src_values0, src_values4);
                src_values1 = _mm_packus_epi16(src_values1, src_values5);
                src_values2 = _mm_packus_epi16(src_values2, src_values6);
                src_values3 = _mm_packus_epi16(src_values3, src_values7);

                /* loading 8-bit 8 pixels values */
                _mm_storeu_si128((__m128i *)(pu1_dst + (4) * dst_strd), src_values0);   /* row=4*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (5) * dst_strd), src_values1);   /* row=5*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (6) * dst_strd), src_values2);   /* row=6*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (7) * dst_strd), src_values3);   /* row=7*/

            }
            pu1_dst += 8 * dst_strd;
        }
    }
    else if(nt == 8)
    {


        __m128i const_temp2_4x32b, const_temp3_4x32b;
        __m128i src_values10, src_values11, intra_pred_ang_4x32b;

        __m128i row_4x32b, two_nt_4x32b, src_values12;
        __m128i src_values0, src_values1, src_values2, src_values3;
        __m128i  src_values4, src_values5, src_values6, src_values7;


        /* Intermediate reference samples for negative angle modes */
        /* For horizontal modes, (ref main = ref above) (ref side = ref left) */
        ref_temp[two_nt - 1] = pu1_ref[two_nt + nt];
        temp1 = _mm_loadl_epi64((__m128i *)(pu1_ref + two_nt));

        /* For negative angled derive the main reference samples from side */

        src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref)); /*nt-(nt+15)*/

        temp11 = _mm_loadu_si128((__m128i *)(inv_angle_shuffle[mode - 19] + 16));

        src_values0 = _mm_shuffle_epi8(src_values0, temp11);
        src_values0 = _mm_srli_si128(src_values0, 8);
        _mm_storel_epi64((__m128i *)(ref_temp + nt - 1), temp1);
        _mm_storel_epi64((__m128i *)(ref_main - nt), src_values0);



        const_temp2_4x32b = _mm_set1_epi16(31);
        const_temp3_4x32b = _mm_set1_epi16(32);


        two_nt_4x32b = _mm_set1_epi16(1);


        /* intra_pred_ang = gai4_ihevc_ang_table[mode]; */
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);

        row_4x32b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);

        {

            WORD16 ref_main_idx[9];

            __m128i res_temp5_4x32b;
            __m128i fract1_8x16b, fract2_8x16b;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

            /* fract = pos & (31); */
            src_values12 = _mm_add_epi16(two_nt_4x32b, _mm_srai_epi16(res_temp5_4x32b,  5));

            /* idx = pos >> 5; */
            src_values11 = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /*(32 - fract) */
            src_values10 = _mm_sub_epi16(const_temp3_4x32b, src_values11);

            fract1_8x16b = _mm_slli_epi16(src_values11, 8);
            fract2_8x16b = _mm_slli_epi16(src_values10, 8);

            src_values11 = _mm_or_si128(src_values11, fract1_8x16b);
            src_values10 = _mm_or_si128(src_values10, fract2_8x16b); /*(32 - fract) */

            fract2_8x16b = _mm_unpackhi_epi8(src_values11, src_values10);
            fract1_8x16b = _mm_unpacklo_epi8(src_values11, src_values10);

            temp1 =  _mm_shuffle_epi32(fract1_8x16b, 0x00);
            temp2 =  _mm_shuffle_epi32(fract1_8x16b, 0x55);
            temp3 =  _mm_shuffle_epi32(fract1_8x16b, 0xaa);
            temp4 =  _mm_shuffle_epi32(fract1_8x16b, 0xff);

            temp11 =  _mm_shuffle_epi32(fract2_8x16b, 0x00);
            temp12 =  _mm_shuffle_epi32(fract2_8x16b, 0x55);
            temp13 =  _mm_shuffle_epi32(fract2_8x16b, 0xaa);
            temp14 =  _mm_shuffle_epi32(fract2_8x16b, 0xff);

            _mm_storeu_si128((__m128i *)ref_main_idx, src_values12);

            src_values0 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[0]));  /* col = 0-7   */
            src_values1 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[1]));  /* col = 8-15  */
            src_values2 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[2]));  /* col = 16-23 */
            src_values3 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[3]));  /* col = 24-31 */
            src_values4 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[4]));  /* col = 32-39   */
            src_values5 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[5]));  /* col = 40-47  */
            src_values6 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[6]));  /* col = 48-55 */
            src_values7 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx[7]));  /* col = 56-63*/

            src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
            src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
            src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
            src_values3 =  _mm_shuffle_epi8(src_values3, sm3);
            src_values4 =  _mm_shuffle_epi8(src_values4, sm3);
            src_values5 =  _mm_shuffle_epi8(src_values5, sm3);
            src_values6 =  _mm_shuffle_epi8(src_values6, sm3);
            src_values7 =  _mm_shuffle_epi8(src_values7, sm3);


            src_values0 = _mm_maddubs_epi16(src_values0, temp1);
            src_values1 = _mm_maddubs_epi16(src_values1, temp2);
            src_values2 = _mm_maddubs_epi16(src_values2, temp3);
            src_values3 = _mm_maddubs_epi16(src_values3, temp4);
            src_values4 = _mm_maddubs_epi16(src_values4, temp11);
            src_values5 = _mm_maddubs_epi16(src_values5, temp12);
            src_values6 = _mm_maddubs_epi16(src_values6, temp13);
            src_values7 = _mm_maddubs_epi16(src_values7, temp14);

            /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
            src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
            src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
            src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
            src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);
            src_values4 = _mm_add_epi16(src_values4, const_temp_8x16b);
            src_values5 = _mm_add_epi16(src_values5, const_temp_8x16b);
            src_values6 = _mm_add_epi16(src_values6, const_temp_8x16b);
            src_values7 = _mm_add_epi16(src_values7, const_temp_8x16b);

            /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
            src_values0 = _mm_srai_epi16(src_values0,  5);
            src_values1 = _mm_srai_epi16(src_values1,  5);
            src_values2 = _mm_srai_epi16(src_values2,  5);
            src_values3 = _mm_srai_epi16(src_values3,  5);
            src_values4 = _mm_srai_epi16(src_values4,  5);
            src_values5 = _mm_srai_epi16(src_values5,  5);
            src_values6 = _mm_srai_epi16(src_values6,  5);
            src_values7 = _mm_srai_epi16(src_values7,  5);

            /* converting 16 bit to 8 bit */
            src_values0 = _mm_packus_epi16(src_values0, src_values1);
            src_values2 = _mm_packus_epi16(src_values2, src_values3);
            src_values1 = _mm_srli_si128(src_values0, 8);
            src_values3 = _mm_srli_si128(src_values2, 8);
            src_values4 = _mm_packus_epi16(src_values4, src_values5);
            src_values6 = _mm_packus_epi16(src_values6, src_values7);
            src_values5 = _mm_srli_si128(src_values4, 8);
            src_values7 = _mm_srli_si128(src_values6, 8);

            /* loading 8-bit 8 pixels values */
            _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd), src_values0);       /* row=0*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), src_values1);   /* row=1*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 2 * dst_strd), src_values2);   /* row=2*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 3 * dst_strd), src_values3);   /* row=3*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 4 * dst_strd), src_values4);   /* row=4*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 5 * dst_strd), src_values5);   /* row=5*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 6 * dst_strd), src_values6);   /* row=6*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 7 * dst_strd), src_values7);   /* row=7*/
        }
    }
    else /* if nt =4*/
    {

        __m128i const_temp2_4x32b, const_temp3_4x32b, zero_8x16b;
        __m128i src_values10, src_values11, intra_pred_ang_4x32b, sign_8x16b;

        __m128i row_4x32b, two_nt_4x32b, src_values12;


        for(k = 0; k < (nt + 1); k++)
            ref_temp[k + nt - 1] = pu1_ref[two_nt + k];
        ref_idx = (nt * intra_pred_ang) >> 5;
        inv_ang_sum = 128;

        for(k = -1; k > ref_idx; k--)
        {
            inv_ang_sum += inv_ang;
            ref_main[k] = pu1_ref[two_nt - (inv_ang_sum >> 8)];
        }


        const_temp2_4x32b = _mm_set1_epi32(31);
        const_temp3_4x32b = _mm_set1_epi32(32);
        zero_8x16b = _mm_setzero_si128();
        two_nt_4x32b = _mm_set1_epi32(1);


        /* intra_pred_ang = gai4_ihevc_ang_table[mode]; */
        row_4x32b = _mm_set_epi16(4, 3, 2, 1, 4, 3, 2, 1);
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);

        {
            WORD32 ref_main_idx1, ref_main_idx2, ref_main_idx3, ref_main_idx4;
            int temp11, temp21, temp31, temp41;


            __m128i fract1_8x16b, fract2_8x16b,  res_temp5_4x32b;
            __m128i src_values0, src_values1, src_values2, src_values3;
            __m128i ref_main_temp0, ref_main_temp1, ref_main_temp2;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);
            sign_8x16b      = _mm_cmpgt_epi16(zero_8x16b, res_temp5_4x32b);
            res_temp5_4x32b = _mm_unpacklo_epi16(res_temp5_4x32b, sign_8x16b);

            /* fract = pos & (31); */
            src_values12 = _mm_add_epi32(two_nt_4x32b, _mm_srai_epi32(res_temp5_4x32b,  5));

            ref_main_temp0 = _mm_srli_si128(src_values12, 4);  /* next 32 bit values */
            ref_main_temp1 = _mm_srli_si128(src_values12, 8);  /* next 32 bit values */
            ref_main_temp2 = _mm_srli_si128(src_values12, 12); /* next 32 bit values */
            ref_main_idx1  = _mm_cvtsi128_si32(src_values12);    /* row=0*/
            ref_main_idx2  = _mm_cvtsi128_si32(ref_main_temp0);  /* row=1*/
            ref_main_idx3  = _mm_cvtsi128_si32(ref_main_temp1);  /* row=2*/
            ref_main_idx4  = _mm_cvtsi128_si32(ref_main_temp2);  /* row=3*/

            /* idx = pos >> 5; */
            src_values11 = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /*(32 - fract) */
            src_values10 = _mm_sub_epi32(const_temp3_4x32b, src_values11);

            fract1_8x16b = _mm_slli_epi16(src_values11, 8);
            fract2_8x16b = _mm_slli_epi16(src_values10, 8);

            src_values11 = _mm_or_si128(src_values11, fract1_8x16b);
            src_values10 = _mm_or_si128(src_values10, fract2_8x16b); /*(32 - fract) */

            fract2_8x16b = _mm_unpackhi_epi8(src_values11, src_values10);
            fract1_8x16b = _mm_unpacklo_epi8(src_values11, src_values10);

            temp1 =  _mm_shuffle_epi32(fract1_8x16b, 0x00);
            temp2 =  _mm_shuffle_epi32(fract1_8x16b, 0xaa);
            temp3 =  _mm_shuffle_epi32(fract2_8x16b, 0x00);
            temp4 =  _mm_shuffle_epi32(fract2_8x16b, 0xaa);

            src_values0 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx1));     /* col = 0-7   */
            src_values1 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx2));   /* col = 8-15  */
            src_values2 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx3));  /* col = 16-23 */
            src_values3 = _mm_loadu_si128((__m128i *)(ref_main + ref_main_idx4));  /* col = 24-31 */

            src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
            src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
            src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
            src_values3 =  _mm_shuffle_epi8(src_values3, sm3);


            src_values0 = _mm_maddubs_epi16(src_values0, temp1);
            src_values1 = _mm_maddubs_epi16(src_values1, temp2);
            src_values2 = _mm_maddubs_epi16(src_values2, temp3);
            src_values3 = _mm_maddubs_epi16(src_values3, temp4);

            /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
            src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
            src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
            src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
            src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);

            /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
            src_values0 = _mm_srai_epi16(src_values0,  5);
            src_values1 = _mm_srai_epi16(src_values1,  5);
            src_values2 = _mm_srai_epi16(src_values2,  5);
            src_values3 = _mm_srai_epi16(src_values3,  5);

            /* converting 16 bit to 8 bit */
            src_values0 = _mm_packus_epi16(src_values0, src_values1);
            src_values2 = _mm_packus_epi16(src_values2, src_values3);
            src_values1 = _mm_srli_si128(src_values0, 8);
            src_values3 = _mm_srli_si128(src_values2, 8);

            temp11 = _mm_cvtsi128_si32(src_values0);
            temp21 = _mm_cvtsi128_si32(src_values1);
            temp31 = _mm_cvtsi128_si32(src_values2);
            temp41 = _mm_cvtsi128_si32(src_values3);

            /* loding 4-bit 8 pixels values */
            *(WORD32 *)(&pu1_dst[(0 * dst_strd)]) = temp11;
            *(WORD32 *)(&pu1_dst[(1 * dst_strd)]) = temp21;
            *(WORD32 *)(&pu1_dst[(2 * dst_strd)]) = temp31;
            *(WORD32 *)(&pu1_dst[(3 * dst_strd)]) = temp41;

        }
    }
}




/**
*******************************************************************************
*
* @brief
*    Intra prediction interpolation filter for luma mode 27 to mode 33
*
* @par Description:
*    Intraprediction for mode 27 to 33  (positive angle, vertical mode ) with
*    reference  neighboring samples location pointed by 'pu1_ref' to the  TU
*    block location pointed by 'pu1_dst'
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


void ihevc_intra_pred_luma_mode_27_to_33_ssse3(UWORD8 *pu1_ref,
                                               WORD32 src_strd,
                                               UWORD8 *pu1_dst,
                                               WORD32 dst_strd,
                                               WORD32 nt,
                                               WORD32 mode)
{
    WORD32 row;
    WORD32 two_nt;
    WORD32 intra_pred_ang;

    __m128i temp11, temp12, temp13, temp14;

    __m128i     const_temp_8x16b;
    __m128i temp1, temp2, temp3, temp4, sm3;
    UNUSED(src_strd);
    two_nt = 2 * nt;
    intra_pred_ang = gai4_ihevc_ang_table[mode];

    const_temp_8x16b = _mm_set1_epi16(16);
    sm3 = _mm_load_si128((__m128i *)&IHEVCE_SHUFFLEMASKY11[0]);
    if(nt == 32)
    {

        __m128i const_temp2_4x32b, const_temp3_4x32b, const_temp8_4x32b;
        __m128i src_values10, src_values11, intra_pred_ang_4x32b;
        __m128i row_4x32b, two_nt_4x32b, src_values12;
        int col = 0;

        const_temp2_4x32b = _mm_set1_epi16(31);
        const_temp3_4x32b = _mm_set1_epi16(32);
        const_temp8_4x32b = _mm_set1_epi16(8);

        two_nt_4x32b = _mm_set1_epi16(two_nt + 1);

        /* intra_pred_ang = gai4_ihevc_ang_table[mode]; */
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);

        row_4x32b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);

        for(row = 0; row < nt; row += 8)
        {

            WORD16 ref_main_idx[9];

            __m128i res_temp5_4x32b;
            __m128i fract1_8x16b, fract2_8x16b;
            __m128i src_values0, src_values1, src_values2, src_values3;
            __m128i  src_values4, src_values5, src_values6, src_values7;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

            /* fract = pos & (31); */
            src_values12 = _mm_add_epi16(two_nt_4x32b, _mm_srai_epi16(res_temp5_4x32b,  5));

            /* idx = pos >> 5; */
            src_values11 = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /*(32 - fract) */
            src_values10 = _mm_sub_epi16(const_temp3_4x32b, src_values11);

            fract1_8x16b = _mm_slli_epi16(src_values11, 8);
            fract2_8x16b = _mm_slli_epi16(src_values10, 8);

            src_values11 = _mm_or_si128(src_values11, fract1_8x16b);
            src_values10 = _mm_or_si128(src_values10, fract2_8x16b); /*(32 - fract) */

            fract2_8x16b = _mm_unpackhi_epi8(src_values11, src_values10);
            fract1_8x16b = _mm_unpacklo_epi8(src_values11, src_values10);

            temp1 =  _mm_shuffle_epi32(fract1_8x16b, 0x00);
            temp2 =  _mm_shuffle_epi32(fract1_8x16b, 0x55);
            temp3 =  _mm_shuffle_epi32(fract1_8x16b, 0xaa);
            temp4 =  _mm_shuffle_epi32(fract1_8x16b, 0xff);

            temp11 =  _mm_shuffle_epi32(fract2_8x16b, 0x00);
            temp12 =  _mm_shuffle_epi32(fract2_8x16b, 0x55);
            temp13 =  _mm_shuffle_epi32(fract2_8x16b, 0xaa);
            temp14 =  _mm_shuffle_epi32(fract2_8x16b, 0xff);

            row_4x32b = _mm_add_epi16(row_4x32b, const_temp8_4x32b);
            _mm_storeu_si128((__m128i *)ref_main_idx, src_values12);
            for(col = 0; col < nt; col += 16)
            {
                src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[0] + col));
                src_values1 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[1] + col));
                src_values2 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[2] + col));
                src_values3 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[3] + col));
                src_values4 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[0] + 8 + col));
                src_values5 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[1] + 8 + col));
                src_values6 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[2] + 8 + col));
                src_values7 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[3] + 8 + col));

                src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
                src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
                src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
                src_values3 =  _mm_shuffle_epi8(src_values3, sm3);
                src_values4 =  _mm_shuffle_epi8(src_values4, sm3);
                src_values5 =  _mm_shuffle_epi8(src_values5, sm3);
                src_values6 =  _mm_shuffle_epi8(src_values6, sm3);
                src_values7 =  _mm_shuffle_epi8(src_values7, sm3);


                src_values0 = _mm_maddubs_epi16(src_values0, temp1);
                src_values1 = _mm_maddubs_epi16(src_values1, temp2);
                src_values2 = _mm_maddubs_epi16(src_values2, temp3);
                src_values3 = _mm_maddubs_epi16(src_values3, temp4);
                src_values4 = _mm_maddubs_epi16(src_values4, temp1);
                src_values5 = _mm_maddubs_epi16(src_values5, temp2);
                src_values6 = _mm_maddubs_epi16(src_values6, temp3);
                src_values7 = _mm_maddubs_epi16(src_values7, temp4);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
                src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
                src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
                src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);
                src_values4 = _mm_add_epi16(src_values4, const_temp_8x16b);
                src_values5 = _mm_add_epi16(src_values5, const_temp_8x16b);
                src_values6 = _mm_add_epi16(src_values6, const_temp_8x16b);
                src_values7 = _mm_add_epi16(src_values7, const_temp_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_values0 = _mm_srai_epi16(src_values0,  5);
                src_values1 = _mm_srai_epi16(src_values1,  5);
                src_values2 = _mm_srai_epi16(src_values2,  5);
                src_values3 = _mm_srai_epi16(src_values3,  5);
                src_values4 = _mm_srai_epi16(src_values4,  5);
                src_values5 = _mm_srai_epi16(src_values5,  5);
                src_values6 = _mm_srai_epi16(src_values6,  5);
                src_values7 = _mm_srai_epi16(src_values7,  5);

                /* converting 16 bit to 8 bit */
                src_values0 = _mm_packus_epi16(src_values0, src_values4);
                src_values1 = _mm_packus_epi16(src_values1, src_values5);
                src_values2 = _mm_packus_epi16(src_values2, src_values6);
                src_values3 = _mm_packus_epi16(src_values3, src_values7);

                /* loading 8-bit 8 pixels values */
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (0) * dst_strd), src_values0);       /* row=0*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (1) * dst_strd), src_values1);   /* row=1*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (2) * dst_strd), src_values2);   /* row=2*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (3) * dst_strd), src_values3);   /* row=3*/


                src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[4] + col));
                src_values1 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[5] + col));
                src_values2 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[6] + col));
                src_values3 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[7] + col));
                src_values4 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[4] + 8 + col));
                src_values5 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[5] + 8 + col));
                src_values6 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[6] + 8 + col));
                src_values7 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[7] + 8 + col));

                src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
                src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
                src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
                src_values3 =  _mm_shuffle_epi8(src_values3, sm3);
                src_values4 =  _mm_shuffle_epi8(src_values4, sm3);
                src_values5 =  _mm_shuffle_epi8(src_values5, sm3);
                src_values6 =  _mm_shuffle_epi8(src_values6, sm3);
                src_values7 =  _mm_shuffle_epi8(src_values7, sm3);


                src_values0 = _mm_maddubs_epi16(src_values0, temp11);
                src_values1 = _mm_maddubs_epi16(src_values1, temp12);
                src_values2 = _mm_maddubs_epi16(src_values2, temp13);
                src_values3 = _mm_maddubs_epi16(src_values3, temp14);
                src_values4 = _mm_maddubs_epi16(src_values4, temp11);
                src_values5 = _mm_maddubs_epi16(src_values5, temp12);
                src_values6 = _mm_maddubs_epi16(src_values6, temp13);
                src_values7 = _mm_maddubs_epi16(src_values7, temp14);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
                src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
                src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
                src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);
                src_values4 = _mm_add_epi16(src_values4, const_temp_8x16b);
                src_values5 = _mm_add_epi16(src_values5, const_temp_8x16b);
                src_values6 = _mm_add_epi16(src_values6, const_temp_8x16b);
                src_values7 = _mm_add_epi16(src_values7, const_temp_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_values0 = _mm_srai_epi16(src_values0,  5);
                src_values1 = _mm_srai_epi16(src_values1,  5);
                src_values2 = _mm_srai_epi16(src_values2,  5);
                src_values3 = _mm_srai_epi16(src_values3,  5);
                src_values4 = _mm_srai_epi16(src_values4,  5);
                src_values5 = _mm_srai_epi16(src_values5,  5);
                src_values6 = _mm_srai_epi16(src_values6,  5);
                src_values7 = _mm_srai_epi16(src_values7,  5);

                /* converting 16 bit to 8 bit */
                src_values0 = _mm_packus_epi16(src_values0, src_values4);
                src_values1 = _mm_packus_epi16(src_values1, src_values5);
                src_values2 = _mm_packus_epi16(src_values2, src_values6);
                src_values3 = _mm_packus_epi16(src_values3, src_values7);

                /* loading 8-bit 8 pixels values */
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (4) * dst_strd), src_values0);   /* row=4*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (5) * dst_strd), src_values1);   /* row=5*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (6) * dst_strd), src_values2);   /* row=6*/
                _mm_storeu_si128((__m128i *)(pu1_dst + col + (7) * dst_strd), src_values3);   /* row=7*/

            }
            pu1_dst += 8 * dst_strd;
        }

    }
    else if(nt == 16) /* for nt = 16 case */
    {

        __m128i const_temp2_4x32b, const_temp3_4x32b, const_temp8_4x32b;
        __m128i src_values10, src_values11, intra_pred_ang_4x32b;
        __m128i row_4x32b, two_nt_4x32b, src_values12;


        const_temp2_4x32b = _mm_set1_epi16(31);
        const_temp3_4x32b = _mm_set1_epi16(32);
        const_temp8_4x32b = _mm_set1_epi16(8);

        two_nt_4x32b = _mm_set1_epi16(two_nt + 1);

        /* intra_pred_ang = gai4_ihevc_ang_table[mode]; */
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);

        row_4x32b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);

        for(row = 0; row < nt; row += 8)
        {

            WORD16 ref_main_idx[9];

            __m128i res_temp5_4x32b;
            __m128i fract1_8x16b, fract2_8x16b;
            __m128i src_values0, src_values1, src_values2, src_values3;
            __m128i  src_values4, src_values5, src_values6, src_values7;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

            /* fract = pos & (31); */
            src_values12 = _mm_add_epi16(two_nt_4x32b, _mm_srai_epi16(res_temp5_4x32b,  5));

            /* idx = pos >> 5; */
            src_values11 = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /*(32 - fract) */
            src_values10 = _mm_sub_epi16(const_temp3_4x32b, src_values11);

            fract1_8x16b = _mm_slli_epi16(src_values11, 8);
            fract2_8x16b = _mm_slli_epi16(src_values10, 8);

            src_values11 = _mm_or_si128(src_values11, fract1_8x16b);
            src_values10 = _mm_or_si128(src_values10, fract2_8x16b); /*(32 - fract) */

            fract2_8x16b = _mm_unpackhi_epi8(src_values11, src_values10);
            fract1_8x16b = _mm_unpacklo_epi8(src_values11, src_values10);

            temp1 =  _mm_shuffle_epi32(fract1_8x16b, 0x00);
            temp2 =  _mm_shuffle_epi32(fract1_8x16b, 0x55);
            temp3 =  _mm_shuffle_epi32(fract1_8x16b, 0xaa);
            temp4 =  _mm_shuffle_epi32(fract1_8x16b, 0xff);

            temp11 =  _mm_shuffle_epi32(fract2_8x16b, 0x00);
            temp12 =  _mm_shuffle_epi32(fract2_8x16b, 0x55);
            temp13 =  _mm_shuffle_epi32(fract2_8x16b, 0xaa);
            temp14 =  _mm_shuffle_epi32(fract2_8x16b, 0xff);

            row_4x32b = _mm_add_epi16(row_4x32b, const_temp8_4x32b);
            _mm_storeu_si128((__m128i *)ref_main_idx, src_values12);

            {
                src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[0]));
                src_values1 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[1]));
                src_values2 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[2]));
                src_values3 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[3]));
                src_values4 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[0] + 8));
                src_values5 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[1] + 8));
                src_values6 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[2] + 8));
                src_values7 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[3] + 8));

                src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
                src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
                src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
                src_values3 =  _mm_shuffle_epi8(src_values3, sm3);
                src_values4 =  _mm_shuffle_epi8(src_values4, sm3);
                src_values5 =  _mm_shuffle_epi8(src_values5, sm3);
                src_values6 =  _mm_shuffle_epi8(src_values6, sm3);
                src_values7 =  _mm_shuffle_epi8(src_values7, sm3);


                src_values0 = _mm_maddubs_epi16(src_values0, temp1);
                src_values1 = _mm_maddubs_epi16(src_values1, temp2);
                src_values2 = _mm_maddubs_epi16(src_values2, temp3);
                src_values3 = _mm_maddubs_epi16(src_values3, temp4);
                src_values4 = _mm_maddubs_epi16(src_values4, temp1);
                src_values5 = _mm_maddubs_epi16(src_values5, temp2);
                src_values6 = _mm_maddubs_epi16(src_values6, temp3);
                src_values7 = _mm_maddubs_epi16(src_values7, temp4);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
                src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
                src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
                src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);
                src_values4 = _mm_add_epi16(src_values4, const_temp_8x16b);
                src_values5 = _mm_add_epi16(src_values5, const_temp_8x16b);
                src_values6 = _mm_add_epi16(src_values6, const_temp_8x16b);
                src_values7 = _mm_add_epi16(src_values7, const_temp_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_values0 = _mm_srai_epi16(src_values0,  5);
                src_values1 = _mm_srai_epi16(src_values1,  5);
                src_values2 = _mm_srai_epi16(src_values2,  5);
                src_values3 = _mm_srai_epi16(src_values3,  5);
                src_values4 = _mm_srai_epi16(src_values4,  5);
                src_values5 = _mm_srai_epi16(src_values5,  5);
                src_values6 = _mm_srai_epi16(src_values6,  5);
                src_values7 = _mm_srai_epi16(src_values7,  5);

                /* converting 16 bit to 8 bit */
                src_values0 = _mm_packus_epi16(src_values0, src_values4);
                src_values1 = _mm_packus_epi16(src_values1, src_values5);
                src_values2 = _mm_packus_epi16(src_values2, src_values6);
                src_values3 = _mm_packus_epi16(src_values3, src_values7);

                /* loading 8-bit 8 pixels values */
                _mm_storeu_si128((__m128i *)(pu1_dst + (0) * dst_strd), src_values0);       /* row=0*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (1) * dst_strd), src_values1);   /* row=1*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (2) * dst_strd), src_values2);   /* row=2*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (3) * dst_strd), src_values3);   /* row=3*/


                src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[4]));
                src_values1 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[5]));
                src_values2 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[6]));
                src_values3 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[7]));
                src_values4 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[4] + 8));
                src_values5 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[5] + 8));
                src_values6 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[6] + 8));
                src_values7 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[7] + 8));

                src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
                src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
                src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
                src_values3 =  _mm_shuffle_epi8(src_values3, sm3);
                src_values4 =  _mm_shuffle_epi8(src_values4, sm3);
                src_values5 =  _mm_shuffle_epi8(src_values5, sm3);
                src_values6 =  _mm_shuffle_epi8(src_values6, sm3);
                src_values7 =  _mm_shuffle_epi8(src_values7, sm3);


                src_values0 = _mm_maddubs_epi16(src_values0, temp11);
                src_values1 = _mm_maddubs_epi16(src_values1, temp12);
                src_values2 = _mm_maddubs_epi16(src_values2, temp13);
                src_values3 = _mm_maddubs_epi16(src_values3, temp14);
                src_values4 = _mm_maddubs_epi16(src_values4, temp11);
                src_values5 = _mm_maddubs_epi16(src_values5, temp12);
                src_values6 = _mm_maddubs_epi16(src_values6, temp13);
                src_values7 = _mm_maddubs_epi16(src_values7, temp14);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
                src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
                src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
                src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
                src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);
                src_values4 = _mm_add_epi16(src_values4, const_temp_8x16b);
                src_values5 = _mm_add_epi16(src_values5, const_temp_8x16b);
                src_values6 = _mm_add_epi16(src_values6, const_temp_8x16b);
                src_values7 = _mm_add_epi16(src_values7, const_temp_8x16b);

                /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
                src_values0 = _mm_srai_epi16(src_values0,  5);
                src_values1 = _mm_srai_epi16(src_values1,  5);
                src_values2 = _mm_srai_epi16(src_values2,  5);
                src_values3 = _mm_srai_epi16(src_values3,  5);
                src_values4 = _mm_srai_epi16(src_values4,  5);
                src_values5 = _mm_srai_epi16(src_values5,  5);
                src_values6 = _mm_srai_epi16(src_values6,  5);
                src_values7 = _mm_srai_epi16(src_values7,  5);

                /* converting 16 bit to 8 bit */
                src_values0 = _mm_packus_epi16(src_values0, src_values4);
                src_values1 = _mm_packus_epi16(src_values1, src_values5);
                src_values2 = _mm_packus_epi16(src_values2, src_values6);
                src_values3 = _mm_packus_epi16(src_values3, src_values7);

                /* loading 8-bit 8 pixels values */
                _mm_storeu_si128((__m128i *)(pu1_dst + (4) * dst_strd), src_values0);   /* row=4*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (5) * dst_strd), src_values1);   /* row=5*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (6) * dst_strd), src_values2);   /* row=6*/
                _mm_storeu_si128((__m128i *)(pu1_dst + (7) * dst_strd), src_values3);   /* row=7*/

            }
            pu1_dst += 8 * dst_strd;
        }

    }
    else if(nt == 8)
    {

        __m128i const_temp2_4x32b, const_temp3_4x32b;
        __m128i src_values10, src_values11, intra_pred_ang_4x32b;
        __m128i row_4x32b, two_nt_4x32b, src_values12;


        const_temp2_4x32b = _mm_set1_epi16(31);
        const_temp3_4x32b = _mm_set1_epi16(32);

        two_nt_4x32b = _mm_set1_epi16(two_nt + 1);


        /* intra_pred_ang = gai4_ihevc_ang_table[mode]; */
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);

        row_4x32b = _mm_set_epi16(8, 7, 6, 5, 4, 3, 2, 1);

        //for(row = 0; row < nt; row +=4)
        {

            WORD16 ref_main_idx[9];

            __m128i res_temp5_4x32b;
            __m128i fract1_8x16b, fract2_8x16b;
            __m128i src_values0, src_values1, src_values2, src_values3;
            __m128i  src_values4, src_values5, src_values6, src_values7;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);

            /* fract = pos & (31); */
            src_values12 = _mm_add_epi16(two_nt_4x32b, _mm_srai_epi16(res_temp5_4x32b,  5));

            /* idx = pos >> 5; */
            src_values11 = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /*(32 - fract) */
            src_values10 = _mm_sub_epi16(const_temp3_4x32b, src_values11);

            fract1_8x16b = _mm_slli_epi16(src_values11, 8);
            fract2_8x16b = _mm_slli_epi16(src_values10, 8);

            src_values11 = _mm_or_si128(src_values11, fract1_8x16b);
            src_values10 = _mm_or_si128(src_values10, fract2_8x16b); /*(32 - fract) */

            fract2_8x16b = _mm_unpackhi_epi8(src_values11, src_values10);
            fract1_8x16b = _mm_unpacklo_epi8(src_values11, src_values10);

            temp1 =  _mm_shuffle_epi32(fract1_8x16b, 0x00);
            temp2 =  _mm_shuffle_epi32(fract1_8x16b, 0x55);
            temp3 =  _mm_shuffle_epi32(fract1_8x16b, 0xaa);
            temp4 =  _mm_shuffle_epi32(fract1_8x16b, 0xff);

            temp11 =  _mm_shuffle_epi32(fract2_8x16b, 0x00);
            temp12 =  _mm_shuffle_epi32(fract2_8x16b, 0x55);
            temp13 =  _mm_shuffle_epi32(fract2_8x16b, 0xaa);
            temp14 =  _mm_shuffle_epi32(fract2_8x16b, 0xff);

            _mm_storeu_si128((__m128i *)ref_main_idx, src_values12);

            src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[0]));  /* col = 0-7   */
            src_values1 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[1]));  /* col = 8-15  */
            src_values2 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[2]));  /* col = 16-23 */
            src_values3 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[3]));  /* col = 24-31 */
            src_values4 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[4]));  /* col = 32-39   */
            src_values5 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[5]));  /* col = 40-47  */
            src_values6 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[6]));  /* col = 48-55 */
            src_values7 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx[7]));  /* col = 56-63*/

            src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
            src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
            src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
            src_values3 =  _mm_shuffle_epi8(src_values3, sm3);
            src_values4 =  _mm_shuffle_epi8(src_values4, sm3);
            src_values5 =  _mm_shuffle_epi8(src_values5, sm3);
            src_values6 =  _mm_shuffle_epi8(src_values6, sm3);
            src_values7 =  _mm_shuffle_epi8(src_values7, sm3);


            src_values0 = _mm_maddubs_epi16(src_values0, temp1);
            src_values1 = _mm_maddubs_epi16(src_values1, temp2);
            src_values2 = _mm_maddubs_epi16(src_values2, temp3);
            src_values3 = _mm_maddubs_epi16(src_values3, temp4);
            src_values4 = _mm_maddubs_epi16(src_values4, temp11);
            src_values5 = _mm_maddubs_epi16(src_values5, temp12);
            src_values6 = _mm_maddubs_epi16(src_values6, temp13);
            src_values7 = _mm_maddubs_epi16(src_values7, temp14);

            /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
            src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
            src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
            src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
            src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);
            src_values4 = _mm_add_epi16(src_values4, const_temp_8x16b);
            src_values5 = _mm_add_epi16(src_values5, const_temp_8x16b);
            src_values6 = _mm_add_epi16(src_values6, const_temp_8x16b);
            src_values7 = _mm_add_epi16(src_values7, const_temp_8x16b);

            /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
            src_values0 = _mm_srai_epi16(src_values0,  5);
            src_values1 = _mm_srai_epi16(src_values1,  5);
            src_values2 = _mm_srai_epi16(src_values2,  5);
            src_values3 = _mm_srai_epi16(src_values3,  5);
            src_values4 = _mm_srai_epi16(src_values4,  5);
            src_values5 = _mm_srai_epi16(src_values5,  5);
            src_values6 = _mm_srai_epi16(src_values6,  5);
            src_values7 = _mm_srai_epi16(src_values7,  5);

            /* converting 16 bit to 8 bit */
            src_values0 = _mm_packus_epi16(src_values0, src_values1);
            src_values2 = _mm_packus_epi16(src_values2, src_values3);
            src_values1 = _mm_srli_si128(src_values0, 8);
            src_values3 = _mm_srli_si128(src_values2, 8);
            src_values4 = _mm_packus_epi16(src_values4, src_values5);
            src_values6 = _mm_packus_epi16(src_values6, src_values7);
            src_values5 = _mm_srli_si128(src_values4, 8);
            src_values7 = _mm_srli_si128(src_values6, 8);

            /* loading 8-bit 8 pixels values */
            _mm_storel_epi64((__m128i *)(pu1_dst + 0 * dst_strd), src_values0);       /* row=0*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 1 * dst_strd), src_values1);   /* row=1*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 2 * dst_strd), src_values2);   /* row=2*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 3 * dst_strd), src_values3);   /* row=3*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 4 * dst_strd), src_values4);   /* row=4*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 5 * dst_strd), src_values5);   /* row=5*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 6 * dst_strd), src_values6);   /* row=6*/
            _mm_storel_epi64((__m128i *)(pu1_dst + 7 * dst_strd), src_values7);   /* row=7*/
        }

    }
    else /* if nt =4*/
    {

        __m128i const_temp2_4x32b, const_temp3_4x32b, zero_8x16b;
        __m128i src_values10, src_values11, intra_pred_ang_4x32b;

        __m128i row_4x32b, two_nt_4x32b, src_values12, sign_8x16b;


        const_temp2_4x32b = _mm_set1_epi32(31);
        const_temp3_4x32b = _mm_set1_epi32(32);
        zero_8x16b = _mm_setzero_si128();
        two_nt_4x32b = _mm_set1_epi32(two_nt + 1);


        /* intra_pred_ang = gai4_ihevc_ang_table[mode]; */
        row_4x32b = _mm_set_epi16(4, 3, 2, 1, 4, 3, 2, 1);
        intra_pred_ang_4x32b = _mm_set1_epi16(intra_pred_ang);
        {
            int temp11, temp21, temp31, temp41;

            WORD32 ref_main_idx1, ref_main_idx2, ref_main_idx3, ref_main_idx4;

            __m128i fract1_8x16b, fract2_8x16b, res_temp5_4x32b;
            __m128i src_values0, src_values1, src_values2, src_values3;
            __m128i ref_main_temp0, ref_main_temp1, ref_main_temp2;

            /* pos = ((row + 1) * intra_pred_ang); */
            res_temp5_4x32b  = _mm_mullo_epi16(row_4x32b, intra_pred_ang_4x32b);
            sign_8x16b      = _mm_cmpgt_epi16(zero_8x16b, res_temp5_4x32b);
            res_temp5_4x32b = _mm_unpacklo_epi16(res_temp5_4x32b, sign_8x16b);

            /* fract = pos & (31); */
            src_values12 = _mm_add_epi32(two_nt_4x32b, _mm_srai_epi32(res_temp5_4x32b,  5));

            ref_main_temp0 = _mm_srli_si128(src_values12, 4);  /* next 32 bit values */
            ref_main_temp1 = _mm_srli_si128(src_values12, 8);  /* next 32 bit values */
            ref_main_temp2 = _mm_srli_si128(src_values12, 12); /* next 32 bit values */
            ref_main_idx1  = _mm_cvtsi128_si32(src_values12);    /* row=0*/
            ref_main_idx2  = _mm_cvtsi128_si32(ref_main_temp0);  /* row=1*/
            ref_main_idx3  = _mm_cvtsi128_si32(ref_main_temp1);  /* row=2*/
            ref_main_idx4  = _mm_cvtsi128_si32(ref_main_temp2);  /* row=3*/

            /* idx = pos >> 5; */
            src_values11 = _mm_and_si128(res_temp5_4x32b, const_temp2_4x32b);

            /*(32 - fract) */
            src_values10 = _mm_sub_epi32(const_temp3_4x32b, src_values11);

            fract1_8x16b = _mm_slli_epi16(src_values11, 8);
            fract2_8x16b = _mm_slli_epi16(src_values10, 8);

            src_values11 = _mm_or_si128(src_values11, fract1_8x16b);
            src_values10 = _mm_or_si128(src_values10, fract2_8x16b); /*(32 - fract) */

            fract2_8x16b = _mm_unpackhi_epi8(src_values11, src_values10);
            fract1_8x16b = _mm_unpacklo_epi8(src_values11, src_values10);

            temp1 =  _mm_shuffle_epi32(fract1_8x16b, 0x00);
            temp2 =  _mm_shuffle_epi32(fract1_8x16b, 0xaa);
            temp3 =  _mm_shuffle_epi32(fract2_8x16b, 0x00);
            temp4 =  _mm_shuffle_epi32(fract2_8x16b, 0xaa);

            src_values0 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx1));     /* col = 0-7   */
            src_values1 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx2));   /* col = 8-15  */
            src_values2 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx3));  /* col = 16-23 */
            src_values3 = _mm_loadu_si128((__m128i *)(pu1_ref + ref_main_idx4));  /* col = 24-31 */

            src_values0 =  _mm_shuffle_epi8(src_values0, sm3);
            src_values1 =  _mm_shuffle_epi8(src_values1, sm3);
            src_values2 =  _mm_shuffle_epi8(src_values2, sm3);
            src_values3 =  _mm_shuffle_epi8(src_values3, sm3);

            src_values0 = _mm_maddubs_epi16(src_values0, temp1);
            src_values1 = _mm_maddubs_epi16(src_values1, temp2);
            src_values2 = _mm_maddubs_epi16(src_values2, temp3);
            src_values3 = _mm_maddubs_epi16(src_values3, temp4);

            /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16)*/
            src_values0 = _mm_add_epi16(src_values0, const_temp_8x16b);
            src_values1 = _mm_add_epi16(src_values1, const_temp_8x16b);
            src_values2 = _mm_add_epi16(src_values2, const_temp_8x16b);
            src_values3 = _mm_add_epi16(src_values3, const_temp_8x16b);

            /*((32 - fract)* pu1_ref[ref_main_idx]+ fract * pu1_ref[ref_main_idx + 1] + 16) >>5*/
            src_values0 = _mm_srai_epi16(src_values0,  5);
            src_values1 = _mm_srai_epi16(src_values1,  5);
            src_values2 = _mm_srai_epi16(src_values2,  5);
            src_values3 = _mm_srai_epi16(src_values3,  5);

            /* converting 16 bit to 8 bit */
            src_values0 = _mm_packus_epi16(src_values0, src_values1);
            src_values2 = _mm_packus_epi16(src_values2, src_values3);
            src_values1 = _mm_srli_si128(src_values0, 8);
            src_values3 = _mm_srli_si128(src_values2, 8);

            temp11 = _mm_cvtsi128_si32(src_values0);
            temp21 = _mm_cvtsi128_si32(src_values1);
            temp31 = _mm_cvtsi128_si32(src_values2);
            temp41 = _mm_cvtsi128_si32(src_values3);

            /* loding 4-bit 8 pixels values */
            *(WORD32 *)(&pu1_dst[(0 * dst_strd)]) = temp11;
            *(WORD32 *)(&pu1_dst[(1 * dst_strd)]) = temp21;
            *(WORD32 *)(&pu1_dst[(2 * dst_strd)]) = temp31;
            *(WORD32 *)(&pu1_dst[(3 * dst_strd)]) = temp41;

        }
    }
}
