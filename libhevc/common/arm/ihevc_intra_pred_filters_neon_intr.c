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
*  ihevc_intra_pred_filters_neon_intr.c
*
* @brief
*  Contains function Definition for intra prediction  interpolation filters
*
*
* @author
*  Yogeswaran RS
*
* @par List of Functions:
*  - ihevc_intra_pred_luma_planar()
*  - ihevc_intra_pred_luma_dc()
*  - ihevc_intra_pred_luma_horz()
*  - ihevc_intra_pred_luma_ver()
*  - ihevc_intra_pred_luma_mode2()
*  - ihevc_intra_pred_luma_mode_18_34()
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

#include "ihevc_typedefs.h"
#include "ihevc_intra_pred.h"
#include "ihevc_macros.h"
#include "ihevc_func_selector.h"
#include "arm_neon.h"
#include "ihevc_platform_macros.h"
#include "ihevc_common_tables.h"

/****************************************************************************/
/* Constant Macros                                                          */
/****************************************************************************/
#define MAX_CU_SIZE 64
#define BIT_DEPTH 8
#define T32_4NT 128
#define T16_4NT 64



/*****************************************************************************/
/* Table Look-up                                                             */
/*****************************************************************************/

#define GET_BITS(y,x) ((y) & (1 << x)) && (1 << x)

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


void ihevc_intra_pred_luma_ref_substitution_neonintr(UWORD8 *pu1_top_left,
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
        UWORD8 *pu1_dst_tmp2 = pu1_dst;
        UWORD8 *pu1_top_tmp = pu1_top;
        pu1_dst_tmp2 += two_nt + 1;

        for(i = 0; i < two_nt; i++)
            pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];

        uint8x8_t src;
        for(i = two_nt; i > 0; i -= 8)
        {
            src = vld1_u8(pu1_top_tmp);
            pu1_top_tmp += 8;
            vst1_u8(pu1_dst_tmp2, src);
            pu1_dst_tmp2 += 8;
        }

        if(nt <= 8)
        {
            /* 1 bit extraction for all the neighboring blocks */
            tp_left = (nbr_flags & 0x10000) >> 16;
            bot_left = nbr_flags & 0x1;
            left = (nbr_flags & 0x10) >> 4;
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
                uint8x8_t dup_pu1_dst1;
                UWORD8 *pu1_dst_const_nt = pu1_dst;
                pu1_dst_const_nt += nt;

                if(0 == (nt & 7))
                {
                    dup_pu1_dst1 = vdup_n_u8(pu1_dst[nt - 1]);
                    for(i = nt; i > 0; i -= 8)
                    {
                        vst1_u8(pu1_dst_const_nt, dup_pu1_dst1);
                        pu1_dst_const_nt += 8;

                    }
                }
                else
                {
                    //uint32x2_t dup_pu1_dst4;
                    dup_pu1_dst1 = vdup_n_u8(pu1_dst[nt - 1]);
                    //dup_pu1_dst4 = vdup_n_u32((uint32_t) pu1_dst[nt - 1]);
                    for(i = nt; i > 0; i -= 4)
                    {
                        vst1_lane_u32((uint32_t *)pu1_dst_const_nt, vreinterpret_u32_u8(dup_pu1_dst1), 0);
                        pu1_dst_const_nt += 4;

                    }

                }

            }
            if(tp_left == 0)
                pu1_dst[two_nt] = pu1_dst[two_nt - 1];
            if(top == 0)
            {

                if(0 == (nt & 7))
                {
                    uint8x8_t dup_pu1_dst2;
                    UWORD8 *pu1_dst_const_two_nt_1 = pu1_dst;
                    pu1_dst_const_two_nt_1 += (two_nt + 1);
                    dup_pu1_dst2 = vdup_n_u8(pu1_dst[two_nt]);
                    for(i = nt; i > 0; i -= 8)
                    {
                        vst1_u8(pu1_dst_const_two_nt_1, dup_pu1_dst2);
                        pu1_dst_const_two_nt_1 += 8;

                    }
                }
                else
                {
                    for(i = 0; i < nt; i++)
                        pu1_dst[two_nt + 1 + i] = pu1_dst[two_nt];
                }
            }
            if(tp_right == 0)
            {
                uint8x8_t dup_pu1_dst3;
                UWORD8 *pu1_dst_const_three_nt_1 = pu1_dst;
                pu1_dst_const_three_nt_1 += (three_nt + 1);
                dup_pu1_dst3 = vdup_n_u8(pu1_dst[two_nt]);
                if(0 == (nt & 7))
                {
                    for(i = nt; i > 0; i -= 8)
                    {
                        vst1_u8(pu1_dst_const_three_nt_1, dup_pu1_dst3);
                        pu1_dst_const_three_nt_1 += 8;

                    }
                }
                else
                {
                    for(i = nt; i > 0; i -= 4)
                    {
                        vst1_lane_u32((uint32_t *)pu1_dst_const_three_nt_1, vreinterpret_u32_u8(dup_pu1_dst3), 0);
                        pu1_dst_const_three_nt_1 += 4;
                    }

                }

            }
        }
        if(nt == 16)
        {
            WORD32 nbr_flags_temp = 0;
            nbr_flags_temp = (nbr_flags & 0x3) + ((nbr_flags & 0x30) >> 2)
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
            while(nbr_id_from_bl < ((T32_4NT)+1))
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


void ihevc_intra_pred_ref_filtering_neonintr(UWORD8 *pu1_src,
                                             WORD32 nt,
                                             UWORD8 *pu1_dst,
                                             WORD32 mode,
                                             WORD32 strong_intra_smoothing_enable_flag)
{
    WORD32 filter_flag;
    WORD32 i = 0;
    WORD32 four_nt = 4 * nt;

    WORD32 src_4nt;
    WORD32 src_0nt;
    /* Naming has been made as per the functionlity it has, For eg. pu1_src_tmp_1 is denoting pu1_src + 1   */
    /* src_val_1 to load value from pointer pu1_src_tmp_1, add_res has the result of adding 2 values        */
    UWORD8 *pu1_src_tmp_0 = pu1_src;
    UWORD8 *pu1_src_tmp_1;
    UWORD8 *pu1_src_tmp_2;
    UWORD8 *pu1_dst_tmp_0 = pu1_dst;
    UWORD8 *pu1_dst_tmp_1;

    uint8x8_t src_val_0, src_val_2;
    uint8x8_t src_val_1, shift_res;
    uint8x8_t dup_const_2;
    uint16x8_t mul_res, add_res;
    WORD32 bi_linear_int_flag = 0;
    WORD32 abs_cond_left_flag = 0;
    WORD32 abs_cond_top_flag = 0;
    WORD32 dc_val = 1 << (BIT_DEPTH - 5);
    shift_res = vdup_n_u8(0);

    filter_flag = gau1_intra_pred_ref_filter[mode] & (1 << (CTZ(nt) - 2));

    if(0 == filter_flag)
    {
        if(pu1_src == pu1_dst)
        {
            return;
        }
        else
        {
            for(i = four_nt; i > 0; i -= 8)
            {
                src_val_0 = vld1_u8(pu1_src_tmp_0);
                pu1_src_tmp_0 += 8;
                vst1_u8(pu1_dst_tmp_0, src_val_0);
                pu1_dst_tmp_0 += 8;
            }
            pu1_dst[four_nt] = pu1_src[four_nt];
        }
    }

    else
    {
        /* If strong intra smoothin is enabled and transform size is 32 */
        if((1 == strong_intra_smoothing_enable_flag) && (32 == nt))
        {
            /*Strong Intra Filtering*/
            abs_cond_top_flag = (ABS(pu1_src[2 * nt] + pu1_src[4 * nt]
                            - (2 * pu1_src[3 * nt]))) < dc_val;
            abs_cond_left_flag = (ABS(pu1_src[2 * nt] + pu1_src[0]
                            - (2 * pu1_src[nt]))) < dc_val;

            bi_linear_int_flag = ((1 == abs_cond_left_flag)
                            && (1 == abs_cond_top_flag));
        }

        src_4nt = pu1_src[4 * nt];
        src_0nt = pu1_src[0];
        /* Strong filtering of reference samples */
        if(1 == bi_linear_int_flag)
        {
            WORD32 two_nt = four_nt >> 1;

            WORD32 pu1_src_0_val = pu1_src[0];
            WORD32 pu1_src_2_nt_val = pu1_src[2 * nt];
            WORD32 pu1_src_4_nt_val = pu1_src[4 * nt];

            WORD32 prod_two_nt_src_0_val = two_nt * pu1_src_0_val;
            uint16x8_t prod_two_nt_src_0_val_t = vdupq_n_u16(prod_two_nt_src_0_val);

            WORD32 prod_two_nt_src_2_nt_val = two_nt * pu1_src_2_nt_val;
            uint16x8_t prod_two_nt_src_2_nt_val_t = vdupq_n_u16(prod_two_nt_src_2_nt_val);

            const UWORD8 *const_col_i;
            uint8x8_t const_col_i_val;
            uint16x8_t prod_val_1;
            uint16x8_t prod_val_2;
            uint16x8_t prod_val_3;
            uint16x8_t prod_val_4;
            uint8x8_t res_val_1;
            uint8x8_t res_val_2;
            uint8x8_t pu1_src_0_val_t = vdup_n_u8(pu1_src_0_val);
            uint8x8_t pu1_src_2_nt_val_t = vdup_n_u8(pu1_src_2_nt_val);
            uint8x8_t pu1_src_4_nt_val_t = vdup_n_u8(pu1_src_4_nt_val);
            pu1_dst_tmp_0 = pu1_dst + 1;
            pu1_dst_tmp_1 = pu1_dst + two_nt + 1;

            const_col_i = gau1_ihevc_planar_factor + 1;

            for(i = two_nt; i > 0; i -= 8)
            {
                const_col_i_val = vld1_u8(const_col_i);
                const_col_i += 8;

                prod_val_1 = vmlsl_u8(prod_two_nt_src_0_val_t, const_col_i_val, pu1_src_0_val_t);
                prod_val_2 = vmlal_u8(prod_val_1, const_col_i_val, pu1_src_2_nt_val_t);

                res_val_1 = vrshrn_n_u16(prod_val_2, 6);
                prod_val_3 = vmlsl_u8(prod_two_nt_src_2_nt_val_t, const_col_i_val, pu1_src_2_nt_val_t);

                vst1_u8(pu1_dst_tmp_0, res_val_1);
                pu1_dst_tmp_0 += 8;
                prod_val_4 = vmlal_u8(prod_val_3, const_col_i_val, pu1_src_4_nt_val_t);

                res_val_2 = vrshrn_n_u16(prod_val_4, 6);
                vst1_u8(pu1_dst_tmp_1, res_val_2);
                pu1_dst_tmp_1 += 8;
            }
            pu1_dst[2 * nt] = pu1_src[2 * nt];
        }
        else
        {
            pu1_src_tmp_1 = pu1_src + 1;
            pu1_src_tmp_2 = pu1_src + 2;
            pu1_dst_tmp_0 += 1;

            dup_const_2 = vdup_n_u8(2);

            /* Extremities Untouched*/
            pu1_dst[0] = pu1_src[0];

            /* To avoid the issue when the dest and src has the same pointer this load has been done
             * outside and the 2nd consecutive load is done before the store of the 1st */

            /* Perform bilinear filtering of Reference Samples */
            for(i = (four_nt - 1); i > 0; i -= 8)
            {
                src_val_0 = vld1_u8(pu1_src_tmp_0);
                pu1_src_tmp_0 += 8;

                src_val_2 = vld1_u8(pu1_src_tmp_2);
                pu1_src_tmp_2 += 8;

                src_val_1 = vld1_u8(pu1_src_tmp_1);
                pu1_src_tmp_1 += 8;

                if(i < four_nt - 1)
                {
                    vst1_u8(pu1_dst_tmp_0, shift_res);
                    pu1_dst_tmp_0 += 8;
                }

                add_res = vaddl_u8(src_val_0, src_val_2);

                mul_res = vmlal_u8(add_res, src_val_1, dup_const_2);
                shift_res = vrshrn_n_u16(mul_res, 2);

            }
            vst1_u8(pu1_dst_tmp_0, shift_res);
            pu1_dst_tmp_0 += 8;
        }
        pu1_dst[4 * nt] = src_4nt;
        pu1_dst[0] = src_0nt;
    }

}



/**
 *******************************************************************************
 *
 * @brief
*   Intra prediction interpolation filter for luma planar
*
* @par Description:
*      Planar Intraprediction with reference neighboring samples  location
*      pointed by 'pu1_ref' to the TU block location  pointed by 'pu1_dst'
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

void ihevc_intra_pred_luma_planar_neonintr(UWORD8 *pu1_ref,
                                           WORD32 src_strd,
                                           UWORD8 *pu1_dst,
                                           WORD32 dst_strd,
                                           WORD32 nt,
                                           WORD32 mode)
{
    /* named it in the way (nt - 1 - col) --> const_nt_1_col(const denotes g_ihevc_planar_factor)   */
    /* load const_nt_1_col values into a d register                                                 */
    /* named it in the way pu1_ref[nt - 1] --> pu1_ref_nt_1                                         */
    /* the value of pu1_ref_nt_1 is duplicated to d register hence pu1_ref_nt_1_dup                 */
    /* log2nt + 1 is taken care while assigning the values itself                                   */
    /* In width multiple of 4 case the row also has been unrolled by 2 and store has been taken care*/

    WORD32 row, col = 0;
    WORD32 log2nt_plus1 = 6;
    WORD32 two_nt, three_nt;
    UWORD8 *pu1_ref_two_nt_1;
    UWORD8 *pu1_dst_tmp;
    const UWORD8 *const_nt_1_col;
    uint8x8_t const_nt_1_col_t;
    const UWORD8 *const_col_1;
    uint8x8_t const_col_1_t;
    uint8_t const_nt_1_row;
    uint8x8_t const_nt_1_row_dup;
    uint8_t const_row_1;
    uint8x8_t const_row_1_dup;
    uint8_t const_nt = nt;
    uint16x8_t const_nt_dup;
    uint8_t pu1_ref_nt_1 = pu1_ref[nt - 1];
    uint8x8_t pu1_ref_nt_1_dup;
    uint8_t pu1_ref_two_nt_1_row;
    uint8_t pu1_ref_three_nt_1;
    uint8x8_t pu1_ref_two_nt_1_row_dup;
    uint8x8_t pu1_ref_two_nt_1_t;
    uint8x8_t pu1_ref_three_nt_1_dup;
    uint16x8_t prod_t1;
    uint16x8_t prod_t2;
    uint16x8_t sto_res_tmp;
    uint8x8_t sto_res;
    int16x8_t log2nt_dup;
    UNUSED(src_strd);
    UNUSED(mode);
    log2nt_plus1 = 32 - CLZ(nt);
    two_nt = 2 * nt;
    three_nt = 3 * nt;
    /* loops have been unrolld considering the fact width is multiple of 8  */
    if(0 == (nt & 7))
    {
        pu1_dst_tmp = pu1_dst;
        const_nt_1_col = gau1_ihevc_planar_factor + nt - 8;

        const_col_1 = gau1_ihevc_planar_factor + 1;
        pu1_ref_three_nt_1 = pu1_ref[three_nt + 1];

        pu1_ref_nt_1_dup = vdup_n_u8(pu1_ref_nt_1);
        const_nt_dup = vdupq_n_u16(const_nt);

        log2nt_dup = vdupq_n_s16(log2nt_plus1);
        log2nt_dup = vnegq_s16(log2nt_dup);

        pu1_ref_three_nt_1_dup = vdup_n_u8(pu1_ref_three_nt_1);

        for(row = 0; row < nt; row++)
        {
            pu1_ref_two_nt_1_row = pu1_ref[two_nt - 1 - row];
            pu1_ref_two_nt_1_row_dup = vdup_n_u8(pu1_ref_two_nt_1_row);

            const_nt_1_row = nt - 1 - row;
            const_nt_1_row_dup = vdup_n_u8(const_nt_1_row);

            const_row_1 = row + 1;
            const_row_1_dup = vdup_n_u8(const_row_1);

            const_nt_1_col = gau1_ihevc_planar_factor + nt - 8;

            const_col_1 = gau1_ihevc_planar_factor + 1;
            pu1_ref_two_nt_1 = pu1_ref + two_nt + 1;

            for(col = nt; col > 0; col -= 8)
            {
                const_nt_1_col_t = vld1_u8(const_nt_1_col);
                const_nt_1_col -= 8;
                const_nt_1_col_t = vrev64_u8(const_nt_1_col_t);

                const_col_1_t = vld1_u8(const_col_1);
                const_col_1 += 8;
                prod_t1 = vmull_u8(const_nt_1_col_t, pu1_ref_two_nt_1_row_dup);

                pu1_ref_two_nt_1_t = vld1_u8(pu1_ref_two_nt_1);
                pu1_ref_two_nt_1 += 8;
                prod_t2 = vmull_u8(const_col_1_t, pu1_ref_three_nt_1_dup);

                prod_t1 = vmlal_u8(prod_t1, const_nt_1_row_dup, pu1_ref_two_nt_1_t);
                prod_t2 = vmlal_u8(prod_t2, const_row_1_dup, pu1_ref_nt_1_dup);
                prod_t1 = vaddq_u16(prod_t1, const_nt_dup);
                prod_t1 = vaddq_u16(prod_t1, prod_t2);

                sto_res_tmp = vreinterpretq_u16_s16(vshlq_s16(vreinterpretq_s16_u16(prod_t1), log2nt_dup));
                sto_res = vmovn_u16(sto_res_tmp);
                vst1_u8(pu1_dst_tmp, sto_res);
                pu1_dst_tmp += 8;
            }
            pu1_dst_tmp += dst_strd - nt;
        }
    }
    /* loops have been unrolld considering the fact width is multiple of 4  */
    /* If column is multiple of 4 then height should be multiple of 2       */
    else
    {
        uint8x8_t const_row_1_dup1;
        uint8x8_t pu1_ref_two_nt_1_t1;
        uint8x8_t const_nt_1_col_t1;
        uint8x8_t const_col_1_t1;
        uint8x8_t pu1_ref_two_nt_1_row_dup1;
        uint8x8_t const_nt_1_row_dup1;

        pu1_ref_three_nt_1 = pu1_ref[three_nt + 1];

        pu1_ref_nt_1_dup = vdup_n_u8(pu1_ref_nt_1);
        const_nt_dup = vdupq_n_u16(const_nt);

        log2nt_dup = vdupq_n_s16(log2nt_plus1);
        log2nt_dup = vnegq_s16(log2nt_dup);

        pu1_ref_three_nt_1_dup = vdup_n_u8(pu1_ref_three_nt_1);

        for(row = 0; row < nt; row += 2)
        {
            pu1_ref_two_nt_1_row = pu1_ref[two_nt - 1 - row];
            pu1_ref_two_nt_1_row_dup = vdup_n_u8(pu1_ref_two_nt_1_row);
            pu1_ref_two_nt_1_row = pu1_ref[two_nt - 2 - row];
            pu1_ref_two_nt_1_row_dup1 = vdup_n_u8(pu1_ref_two_nt_1_row);
            pu1_ref_two_nt_1_row_dup = vext_u8(pu1_ref_two_nt_1_row_dup, pu1_ref_two_nt_1_row_dup1, 4);

            const_nt_1_row = nt - 1 - row;
            const_nt_1_row_dup = vdup_n_u8(const_nt_1_row);
            const_nt_1_row = nt - 2 - row;
            const_nt_1_row_dup1 = vdup_n_u8(const_nt_1_row);
            const_nt_1_row_dup = vext_u8(const_nt_1_row_dup, const_nt_1_row_dup1, 4);

            const_row_1 = row + 1;
            const_row_1_dup = vdup_n_u8(const_row_1);
            const_row_1 = row + 2;
            const_row_1_dup1 = vdup_n_u8(const_row_1);
            const_row_1_dup = vext_u8(const_row_1_dup, const_row_1_dup1, 4);

            const_nt_1_col = gau1_ihevc_planar_factor + nt - 4;

            const_col_1 = gau1_ihevc_planar_factor + 1;

            pu1_ref_two_nt_1 = pu1_ref + two_nt + 1;

            for(col = nt; col > 0; col -= 4)
            {
                const_nt_1_col_t = vld1_u8(const_nt_1_col);
                const_nt_1_col -= 4;
                const_nt_1_col_t = vrev64_u8(const_nt_1_col_t);

                const_col_1_t = vld1_u8(const_col_1);
                const_col_1 += 4;
                const_nt_1_col_t1 = vreinterpret_u8_u64(vshr_n_u64(vreinterpret_u64_u8(const_nt_1_col_t), 32));

                pu1_dst_tmp = pu1_dst;
                const_nt_1_col_t = vext_u8(const_nt_1_col_t, const_nt_1_col_t1, 4);

                const_col_1_t1 = vreinterpret_u8_u64(vshl_n_u64(vreinterpret_u64_u8(const_col_1_t), 32));
                prod_t1 = vmull_u8(const_nt_1_col_t, pu1_ref_two_nt_1_row_dup);

                pu1_ref_two_nt_1_t = vld1_u8(pu1_ref_two_nt_1);
                pu1_ref_two_nt_1 += 4;
                const_col_1_t = vext_u8(const_col_1_t1, const_col_1_t, 4);

                pu1_ref_two_nt_1_t1 = vreinterpret_u8_u64(vshl_n_u64(vreinterpret_u64_u8(pu1_ref_two_nt_1_t), 32));
                prod_t2 = vmull_u8(const_col_1_t, pu1_ref_three_nt_1_dup);

                pu1_ref_two_nt_1_t = vext_u8(pu1_ref_two_nt_1_t1, pu1_ref_two_nt_1_t, 4);
                prod_t2 = vmlal_u8(prod_t2, const_row_1_dup, pu1_ref_nt_1_dup);

                prod_t1 = vmlal_u8(prod_t1, const_nt_1_row_dup, pu1_ref_two_nt_1_t);
                prod_t1 = vaddq_u16(prod_t1, const_nt_dup);
                prod_t1 = vaddq_u16(prod_t1, prod_t2);

                sto_res_tmp = vreinterpretq_u16_s16(vshlq_s16(vreinterpretq_s16_u16(prod_t1), log2nt_dup));
                sto_res = vmovn_u16(sto_res_tmp);

                vst1_lane_u32((uint32_t *)pu1_dst, vreinterpret_u32_u8(sto_res), 0);
                pu1_dst_tmp += dst_strd;

                vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(sto_res), 1);
                pu1_dst += 4;
            }
            pu1_dst += 2 * dst_strd - nt;
        }
    }

}
/* INTRA_PRED_LUMA_PLANAR */

/**
*******************************************************************************
*
* @brief
*    Intra prediction interpolation filter for luma dc
*
* @par Description:
*    Intraprediction for DC mode with reference neighboring  samples location
*    pointed by 'pu1_ref' to the TU block  location pointed by 'pu1_dst'
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

void ihevc_intra_pred_luma_dc_neonintr(UWORD8 *pu1_ref,
                                       WORD32 src_strd,
                                       UWORD8 *pu1_dst,
                                       WORD32 dst_strd,
                                       WORD32 nt,
                                       WORD32 mode)
{
    WORD32 dc_val = 0, two_dc_val = 0, three_dc_val = 0;
    WORD32 i = 0;
    WORD32 row = 0, col = 0, col_count;
    WORD32 log2nt_plus1 = 6;
    WORD32 two_nt = 0;
    uint16x8_t ref_load_q;
    uint16x8_t three_dc_val_t;
    uint8x8_t sto_res_tmp;
    uint8x8_t sto_res_tmp1;
    uint8x8_t sto_res_tmp2;
    uint8x8_t sto_res_tmp3;
    uint8x8_t sto_res_tmp4;
    uint8x8_t dc_val_t;

    UWORD8 *pu1_ref_tmp;
    UWORD8 *pu1_ref_tmp1;
    UWORD8 *pu1_dst_tmp;
    UWORD8 *pu1_dst_tmp1;
    UWORD8 *pu1_dst_tmp2;
    UNUSED(src_strd);
    UNUSED(mode);

    /* log2nt + 1 is taken care while assigning the values itself.          */
    log2nt_plus1 = 32 - CLZ(nt);

    /* loops have been unrolld considering the fact width is multiple of 8  */
    if(0 == (nt & 7))
    {
        uint8x8_t ref_load1;
        uint8x8_t ref_load2;
        uint16x4_t acc_dc_pair1;
        uint32x2_t acc_dc_pair2;
        uint64x1_t acc_dc = vdup_n_u64(col);

        two_nt = 2 * nt;
        pu1_ref_tmp = pu1_ref + nt;
        pu1_ref_tmp1 = pu1_ref + two_nt + 1;

        for(i = two_nt; i > nt; i -= 8)
        {
            ref_load1 = vld1_u8(pu1_ref_tmp);
            pu1_ref_tmp += 8;
            acc_dc_pair1 = vpaddl_u8(ref_load1);

            ref_load2 = vld1_u8(pu1_ref_tmp1);
            pu1_ref_tmp1 += 8;

            acc_dc_pair2 = vpaddl_u16(acc_dc_pair1);
            acc_dc = vpadal_u32(acc_dc, acc_dc_pair2);

            acc_dc_pair1 = vpaddl_u8(ref_load2);
            acc_dc_pair2 = vpaddl_u16(acc_dc_pair1);
            acc_dc = vpadal_u32(acc_dc, acc_dc_pair2);
        }

        dc_val = (vget_lane_u32(vreinterpret_u32_u64(acc_dc), 0) + nt) >> (log2nt_plus1);
        dc_val_t = vdup_n_u8(dc_val);
        two_dc_val = 2 * dc_val;
        three_dc_val = 3 * dc_val;
        three_dc_val += 2;

        three_dc_val_t = vdupq_n_u16((WORD16)three_dc_val);
        pu1_ref_tmp = pu1_ref + two_nt + 1 + 0;
        pu1_dst_tmp = pu1_dst;


        if(nt == 32)
        {
            for(row = 0; row < nt; row++)
            {
                for(col = nt; col > 0; col -= 8)
                {
                    vst1_u8(pu1_dst_tmp, dc_val_t);
                    pu1_dst_tmp += 8;
                }
                pu1_dst_tmp += dst_strd - nt;
            }
        }
        else

        {
            for(col = nt; col > 0; col -= 8)
            {
                ref_load1 = vld1_u8(pu1_ref_tmp);
                pu1_ref_tmp += 8;
                ref_load_q = vmovl_u8(ref_load1);
                ref_load_q = vaddq_u16(ref_load_q, three_dc_val_t);
                ref_load_q = vshrq_n_u16(ref_load_q, 2);
                sto_res_tmp = vmovn_u16(ref_load_q);
                vst1_u8(pu1_dst_tmp, sto_res_tmp);
                pu1_dst_tmp += 8;
            }

            pu1_ref_tmp = pu1_ref + two_nt - 9;
            pu1_dst_tmp = pu1_dst + dst_strd;
            col_count = nt - 8;

            /* Except the first row the remaining rows are done here                            */
            /* Both column and row has been unrolled by 8                                       */
            /* Store has been taken care for the unrolling                                      */
            /* Except the 1st column of the remaining rows(other than 1st row), the values are  */
            /* constant hence it is extracted with an constant value and stored                 */
            /* If the column is greater than 8, then the remaining values are constant which is */
            /* taken care in the inner for loop                                                 */

            for(row = nt; row > 0; row -= 8)
            {
                pu1_dst_tmp1 = pu1_dst_tmp + 8;
                ref_load1 = vld1_u8(pu1_ref_tmp);
                pu1_ref_tmp -= 8;
                ref_load_q = vmovl_u8(ref_load1);
                ref_load_q = vaddq_u16(ref_load_q, three_dc_val_t);
                ref_load_q = vshrq_n_u16(ref_load_q, 2);
                sto_res_tmp = vmovn_u16(ref_load_q);

                sto_res_tmp1 = vext_u8(sto_res_tmp, dc_val_t, 7);

                sto_res_tmp2 = vreinterpret_u8_u64(vshl_n_u64(vreinterpret_u64_u8(sto_res_tmp), 8));
                sto_res_tmp2 = vext_u8(sto_res_tmp2, dc_val_t, 7);
                vst1_u8(pu1_dst_tmp, sto_res_tmp1);
                pu1_dst_tmp += dst_strd;

                sto_res_tmp3 = vreinterpret_u8_u64(vshl_n_u64(vreinterpret_u64_u8(sto_res_tmp), 16));
                sto_res_tmp3 = vext_u8(sto_res_tmp3, dc_val_t, 7);
                vst1_u8(pu1_dst_tmp, sto_res_tmp2);
                pu1_dst_tmp += dst_strd;

                sto_res_tmp4 = vreinterpret_u8_u64(vshl_n_u64(vreinterpret_u64_u8(sto_res_tmp), 24));
                sto_res_tmp4 = vext_u8(sto_res_tmp4, dc_val_t, 7);
                vst1_u8(pu1_dst_tmp, sto_res_tmp3);
                pu1_dst_tmp += dst_strd;

                sto_res_tmp1 = vreinterpret_u8_u64(vshl_n_u64(vreinterpret_u64_u8(sto_res_tmp), 32));
                sto_res_tmp1 = vext_u8(sto_res_tmp1, dc_val_t, 7);
                vst1_u8(pu1_dst_tmp, sto_res_tmp4);
                pu1_dst_tmp += dst_strd;

                sto_res_tmp2 = vreinterpret_u8_u64(vshl_n_u64(vreinterpret_u64_u8(sto_res_tmp), 40));
                sto_res_tmp2 = vext_u8(sto_res_tmp2, dc_val_t, 7);
                vst1_u8(pu1_dst_tmp, sto_res_tmp1);
                pu1_dst_tmp += dst_strd;

                sto_res_tmp3 = vreinterpret_u8_u64(vshl_n_u64(vreinterpret_u64_u8(sto_res_tmp), 48));
                sto_res_tmp3 = vext_u8(sto_res_tmp3, dc_val_t, 7);
                vst1_u8(pu1_dst_tmp, sto_res_tmp2);
                pu1_dst_tmp += dst_strd;

                sto_res_tmp4 = vreinterpret_u8_u64(vshl_n_u64(vreinterpret_u64_u8(sto_res_tmp), 56));
                sto_res_tmp4 = vext_u8(sto_res_tmp4, dc_val_t, 7);
                vst1_u8(pu1_dst_tmp, sto_res_tmp3);
                pu1_dst_tmp += dst_strd;
                /* For last set of 8 rows only 7 rows need to be updated since first row is already written */
                if(row != 8)
                    vst1_u8(pu1_dst_tmp, sto_res_tmp4);
                pu1_dst_tmp += dst_strd;

                for(col = col_count; col > 0; col -= 8)
                {
                    pu1_dst_tmp2 = pu1_dst_tmp1;
                    vst1_u8(pu1_dst_tmp1, dc_val_t);
                    pu1_dst_tmp1 += dst_strd;
                    vst1_u8(pu1_dst_tmp1, dc_val_t);
                    pu1_dst_tmp1 += dst_strd;
                    vst1_u8(pu1_dst_tmp1, dc_val_t);
                    pu1_dst_tmp1 += dst_strd;
                    vst1_u8(pu1_dst_tmp1, dc_val_t);
                    pu1_dst_tmp1 += dst_strd;
                    vst1_u8(pu1_dst_tmp1, dc_val_t);
                    pu1_dst_tmp1 += dst_strd;
                    vst1_u8(pu1_dst_tmp1, dc_val_t);
                    pu1_dst_tmp1 += dst_strd;
                    vst1_u8(pu1_dst_tmp1, dc_val_t);
                    pu1_dst_tmp1 += dst_strd;

                    /* For last set of 8 rows only 7 rows need to be updated since first row is already written */
                    if(row != 8)
                        vst1_u8(pu1_dst_tmp1, dc_val_t);
                    pu1_dst_tmp1 = pu1_dst_tmp2 + 8;
                }
            }
            pu1_dst[0] = (pu1_ref[two_nt - 1] + two_dc_val + pu1_ref[two_nt + 1] + 2) >> 2;
        }
    }
    /* loops have been unrolld considering the fact width is multiple of 4  */
    else
    {
        WORD32 acc_dc;
        two_nt = 2 * nt;

        acc_dc = 0;
        pu1_ref_tmp = pu1_ref + nt + 1;
        for(i = nt; i < two_nt; i++)
        {
            acc_dc += pu1_ref[i];
            acc_dc += pu1_ref_tmp[i];
        }
        dc_val = (acc_dc + nt) >> (log2nt_plus1);
        two_dc_val = 2 * dc_val;
        three_dc_val = 3 * dc_val;
        three_dc_val = three_dc_val + 2;
        dc_val_t = vdup_n_u8(dc_val);

        if(nt == 32)
        {
            pu1_dst_tmp = pu1_dst;
            for(row = 0; row < nt; row++)
            {
                for(col = nt; col > 0; col -= 4)
                {
                    vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(dc_val_t), 0);
                    pu1_dst_tmp += 4;
                }
                pu1_dst_tmp += dst_strd - nt;
            }
        }
        else

        {
            for(col = 1; col < nt; col++)
            {
                pu1_dst[col] = (pu1_ref[two_nt + 1 + col] + three_dc_val) >> 2;
            }

            pu1_dst_tmp = pu1_dst + dst_strd + 0;
            /* Since first row is already updated before, loop count is nt-1 */
            for(row = nt - 1; row > 0; row -= 1)
            {
                for(col = nt; col > 0; col -= 4)
                {
                    vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(dc_val_t), 0);
                    pu1_dst_tmp += 4;
                }
                pu1_dst_tmp += dst_strd - nt;
            }

            for(row = 1; row < nt; row++)
            {
                pu1_dst[row * dst_strd] = (pu1_ref[two_nt - 1 - row] + three_dc_val) >> 2;
            }
            pu1_dst[0] = (pu1_ref[two_nt - 1] + two_dc_val + pu1_ref[two_nt + 1] + 2) >> 2;
        }
    }
}
/* INTRA_PRED_LUMA_DC */

/**
*******************************************************************************
*
* @brief
 *   Intra prediction interpolation filter for horizontal luma variable.
 *
 * @par Description:
 *   Horizontal intraprediction with reference neighboring  samples location
 *   pointed by 'pu1_ref' to the TU block  location pointed by 'pu1_dst'
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

void ihevc_intra_pred_luma_horz_neonintr(UWORD8 *pu1_ref,
                                         WORD32 src_strd,
                                         UWORD8 *pu1_dst,
                                         WORD32 dst_strd,
                                         WORD32 nt,
                                         WORD32 mode)
{

    WORD32 row, col;
    WORD32 two_nt;
    UNUSED(src_strd);
    UNUSED(mode);

    two_nt = 2 * nt;


    UWORD8 *pu1_dst_tmp = pu1_dst;
    UWORD32 pu1_val;
    uint8x8_t pu1_val_two_nt_1_row;
    if(nt == 32)
    {
        pu1_dst_tmp = pu1_dst;
        for(row = 0; row < nt; row++)
        {
            pu1_val = pu1_ref[two_nt - 1 - row];
            pu1_val_two_nt_1_row = vdup_n_u8(pu1_val);
            for(col = nt; col > 0; col -= 8)
            {
                vst1_u8(pu1_dst_tmp, pu1_val_two_nt_1_row);
                pu1_dst_tmp += 8;
            }
            pu1_dst_tmp += dst_strd - nt;
        }
    }
    else


    /* row loop has been unrolled, hence had pu1_ref_val1 and pu1_ref_val2 variables*/
    /* naming of variables made according to the operation(instructions) it performs*/
    /* (eg. shift_val which contains the shifted value,                             */
    /* add_sat which has add and saturated value)                                   */
    /* Loops are unrolled by 4 and 8 considering the fact the input width is either multiple of 4 or 8  */
    /* rows and columns are unrolled by 4, when the width is multiple of 4                              */
    {
        if(0 != (nt & 7))      /* cond for multiple of 4 */
        {
            UWORD8 *pu1_ref_4_two_nt_plus1 = pu1_ref;
            UWORD8 *pu1_ref_4_two_nt_minus_nt = pu1_ref;
            UWORD8 *pu1_dst_4 = pu1_dst;
            UWORD8 *pu1_dst_4_tmp = pu1_dst;

            uint32x2_t pu1_ref_val1, pu1_ref_val2;
            uint8x8_t dup_sub, round_val, dup_val;
            uint16x8_t dup_add, sub_val;
            int16x8_t shift_val, add_sat;

            pu1_ref_val1 = vdup_n_u32(0);
            pu1_ref_val2 = vdup_n_u32(0);

            dup_sub = vdup_n_u8(pu1_ref[two_nt]);

            dup_add = vdupq_n_u16(pu1_ref[two_nt - 1]);

            pu1_ref_4_two_nt_plus1 += (two_nt + 1);

            pu1_ref_4_two_nt_minus_nt += (two_nt - nt);

            for(row = nt; row > 0; row -= 4)
            {
                for(col = nt; col > 0; col -= 4)
                {
                    pu1_ref_val1 = vld1_lane_u32((uint32_t *)pu1_ref_4_two_nt_plus1, pu1_ref_val1, 0);
                    sub_val = vsubl_u8(vreinterpret_u8_u32(pu1_ref_val1), dup_sub);
                    shift_val  = vshrq_n_s16(vreinterpretq_s16_u16(sub_val), 1);

                    add_sat = vqaddq_s16(shift_val, vreinterpretq_s16_u16(dup_add));
                    round_val = vqmovun_s16(add_sat);
                    vst1_lane_u32((uint32_t *)pu1_dst_4, vreinterpret_u32_u8(round_val), 0);
                    pu1_dst_4 += dst_strd;

                    pu1_ref_val2 = vld1_lane_u32((uint32_t *)pu1_ref_4_two_nt_minus_nt, pu1_ref_val2, 0);
                    dup_val = vdup_lane_u8(vreinterpret_u8_u32(pu1_ref_val2), 2);
                    vst1_lane_u32((uint32_t *)pu1_dst_4, vreinterpret_u32_u8(dup_val), 0);
                    pu1_dst_4 += dst_strd;

                    dup_val = vdup_lane_u8(vreinterpret_u8_u32(pu1_ref_val2), 1);
                    vst1_lane_u32((uint32_t *)pu1_dst_4, vreinterpret_u32_u8(dup_val), 0);
                    pu1_dst_4 += dst_strd;

                    dup_val = vdup_lane_u8(vreinterpret_u8_u32(pu1_ref_val2), 0);
                    vst1_lane_u32((uint32_t *)pu1_dst_4, vreinterpret_u32_u8(dup_val), 0);
                    pu1_dst_4 += dst_strd;


                }
                /* worst cases */
                pu1_ref_4_two_nt_minus_nt += 3;
                pu1_ref_4_two_nt_plus1 += 4;
                pu1_dst_4 = (pu1_dst_4_tmp + 4);
            }

        }

        /* dup_1 - dup_8 are variables to load the duplicated values from the loaded source */
        /* naming of variables made according to the operation(instructions) it performs    */
        /* Loops are unrolled by 4 and 8 considering the fact the input width is either multiple of 4 or 8  */
        /* rows and columns are unrolled by 8, when the width is multiple of 8                              */

        else
        {
            UWORD8 *pu1_ref_tmp_1 = pu1_ref;
            UWORD8 *pu1_ref_tmp_2 = pu1_ref;

            UWORD8 *pu1_dst_tmp_1 = pu1_dst;
            UWORD8 *pu1_dst_tmp_2 = pu1_dst + dst_strd;
            UWORD8 *pu1_dst_tmp_3 = pu1_dst + dst_strd;

            uint8x8_t dup_sub, src_tmp, src_tmp_1, round_val, dup_1, dup_2, dup_3, dup_4, dup_5, dup_6, dup_7, dup_8, rev_res;
            uint16x8_t sub_res, dup_add;
            int16x8_t shift_res, add_res;

            dup_sub = vdup_n_u8(pu1_ref[two_nt]);
            dup_add = vdupq_n_u16(pu1_ref[two_nt - 1]);

            pu1_ref_tmp_1 += (two_nt + 1);
            pu1_ref_tmp_2 += (two_nt - 1);

            for(col = nt; col > 0; col -= 8)
            {
                src_tmp = vld1_u8(pu1_ref_tmp_1);
                pu1_ref_tmp_1 += 8;

                sub_res = vsubl_u8(src_tmp, dup_sub);
                shift_res  = vshrq_n_s16(vreinterpretq_s16_u16(sub_res), 1);
                add_res = vqaddq_s16(shift_res, vreinterpretq_s16_u16(dup_add));
                round_val = vqmovun_s16(add_res);
                vst1_u8(pu1_dst_tmp_1, round_val);
                pu1_dst_tmp_1 += 8;
            }

            for(row = nt; row > 0; row -= 8)
            {
                pu1_ref_tmp_2 -= 8;

                src_tmp_1 = vld1_u8(pu1_ref_tmp_2);
                rev_res = vrev64_u8(src_tmp_1); /* Reversing the loaded values */

                dup_1 = vdup_lane_u8(rev_res, 0);
                dup_2 = vdup_lane_u8(rev_res, 1);
                dup_3 = vdup_lane_u8(rev_res, 2);
                dup_4 = vdup_lane_u8(rev_res, 3);
                dup_5 = vdup_lane_u8(rev_res, 4);
                dup_6 = vdup_lane_u8(rev_res, 5);
                dup_7 = vdup_lane_u8(rev_res, 6);
                dup_8 = vdup_lane_u8(rev_res, 7);

                for(col = nt; col > 0; col -= 8)
                {
                    pu1_dst_tmp_2 = pu1_dst_tmp_3;

                    vst1_u8(pu1_dst_tmp_2, dup_1);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, dup_2);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, dup_3);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, dup_4);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, dup_5);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, dup_6);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, dup_7);
                    pu1_dst_tmp_2 += dst_strd;

                    /* For last set of 8 rows only 7 rows need to be updated since first row is already written */
                    if(row != 8)
                        vst1_u8(pu1_dst_tmp_2, dup_8);
                    pu1_dst_tmp_2 += dst_strd;

                    pu1_dst_tmp_3 += 8;
                }
                pu1_dst_tmp_2 -= (nt - 8);
                pu1_dst_tmp_3 = pu1_dst_tmp_2;
            }
        }
    }
}
/* INTRA_PRED_LUMA_HORZ */

/**
*******************************************************************************
*
* @brief
*    Intra prediction interpolation filter for vertical luma variable.
*
* @par Description:
*    Horizontal intraprediction with reference neighboring  samples location
*    pointed by 'pu1_ref' to the TU block  location pointed by 'pu1_dst'
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

void ihevc_intra_pred_luma_ver_neonintr(UWORD8 *pu1_ref,
                                        WORD32 src_strd,
                                        UWORD8 *pu1_dst,
                                        WORD32 dst_strd,
                                        WORD32 nt,
                                        WORD32 mode)
{
    WORD32 row, col;
    WORD32 two_nt;
    UNUSED(src_strd);
    UNUSED(mode);

    two_nt = 2 * nt;

    UWORD8 *pu1_dst_tmp = pu1_dst;
    UWORD8 *pu1_ref_tmp_1 = pu1_ref + two_nt + 1;
    uint8x8_t pu1_val_two_nt_1_col;
    if(nt == 32)
    {
        pu1_dst_tmp = pu1_dst;
        for(row = 0; row < nt; row++)
        {
            for(col = nt; col > 0; col -= 8)
            {
                pu1_val_two_nt_1_col = vld1_u8(pu1_ref_tmp_1);
                pu1_ref_tmp_1 += 8;
                vst1_u8(pu1_dst_tmp, pu1_val_two_nt_1_col);
                pu1_dst_tmp += 8;
            }
            pu1_ref_tmp_1 -= nt;
            pu1_dst_tmp += dst_strd - nt;
        }
    }
    else

    {
        /* naming of variables made according to the operation(instructions) it performs                    */
        /* (eg. shift_val which contains the shifted value,                                                 */
        /* add_sat which has add and saturated value)                                                       */
        /* Loops are unrolled by 4 and 8 considering the fact the input width is either multiple of 4 or 8  */
        /* rows and columns are unrolled by 4, when the width is multiple of 4                              */

        if(0 != (nt & 7))
        {
            WORD32 cond_4 = 0;
            UWORD8 *pu1_ref_val1 = pu1_ref;
            UWORD8 *pu1_ref_val2 = pu1_ref;
            UWORD8 *pu1_ref_val3 = pu1_ref;

            UWORD8 *pu1_dst_val1 = pu1_dst;
            UWORD8 *pu1_dst_val2 = pu1_dst;
            UWORD8 *pu1_dst_val3 = pu1_dst;

            uint8x8_t dup_2_sub, round_val, vext_val;
            uint16x8_t dup_2_add;
            uint32x2_t src_val1, src_val2, src_val3;
            uint16x8_t sub_val;
            int16x8_t shift_val1, add_sat;
            uint64x1_t shift_val2;

            src_val1 = vdup_n_u32(0);
            src_val2 = vdup_n_u32(0);
            src_val3 = vdup_n_u32(0);
            pu1_ref_val1 += (two_nt - nt);
            pu1_ref_val3 += (two_nt + 2);
            pu1_ref_val2 += (two_nt + 1);

            dup_2_sub = vdup_n_u8(pu1_ref[two_nt]);
            dup_2_add = vdupq_n_u16(pu1_ref[two_nt + 1]);

            /* loops to store the first nt sets of values in the destination */

            for(row = nt; row > 0; row -= 4)
            {
                for(col = nt; (col > 0) && (cond_4 == 0); col -= 4)
                {
                    /*  unrolling s2_predpixel = pu1_ref[two_nt + 1] + ((pu1_ref[two_nt - 1 - row] - pu1_ref[two_nt]) >> 1); here*/
                    src_val1 = vld1_lane_u32((uint32_t *)pu1_ref_val1, src_val1, 1);
                    sub_val = vsubl_u8(vreinterpret_u8_u32(src_val1), dup_2_sub);
                    shift_val1  = vshrq_n_s16(vreinterpretq_s16_u16(sub_val), 1);
                    add_sat = vqaddq_s16(shift_val1, vreinterpretq_s16_u16(dup_2_add));
                    round_val = vqmovun_s16(add_sat);

                    /* unrolling pu1_dst[row * dst_strd + col] = pu1_ref[two_nt + 1 + col]; here*/
                    src_val2 = vld1_lane_u32((uint32_t *)pu1_ref_val3, src_val2, 0);
                    vext_val = vext_u8(round_val, vreinterpret_u8_u32(src_val2), 7);
                    vst1_lane_u32((uint32_t *)pu1_dst_val1, vreinterpret_u32_u8(vext_val), 0);
                    pu1_dst_val1 += dst_strd;

                    shift_val2 = vshl_n_u64(vreinterpret_u64_u8(round_val), 8);

                    vext_val = vext_u8(vreinterpret_u8_u64(shift_val2), vreinterpret_u8_u32(src_val2), 7);
                    vst1_lane_u32((uint32_t *)pu1_dst_val1, vreinterpret_u32_u8(vext_val), 0);
                    pu1_dst_val1 += dst_strd;

                    shift_val2 = vshl_n_u64(vreinterpret_u64_u8(round_val), 16);

                    vext_val = vext_u8(vreinterpret_u8_u64(shift_val2), vreinterpret_u8_u32(src_val2), 7);
                    vst1_lane_u32((uint32_t *)pu1_dst_val1, vreinterpret_u32_u8(vext_val), 0);
                    pu1_dst_val1 += dst_strd;

                    shift_val2 = vshl_n_u64(vreinterpret_u64_u8(round_val), 24);

                    vext_val = vext_u8(vreinterpret_u8_u64(shift_val2), vreinterpret_u8_u32(src_val2), 7);
                    vst1_lane_u32((uint32_t *)pu1_dst_val1, vreinterpret_u32_u8(vext_val), 0);
                    pu1_dst_val1 += dst_strd;

                    pu1_ref_val1  -= 4;
                }

                /* loop to store next sets of eight values in the destination */

                for(col = nt - 3; (col > 0) && (cond_4 == 1); col -= 4)
                {
                    src_val3 = vld1_lane_u32((uint32_t *)pu1_ref_val2, src_val3, 0);

                    vst1_u8(pu1_dst_val2, vreinterpret_u8_u32(src_val3));
                    pu1_dst_val2 += dst_strd;

                    vst1_u8(pu1_dst_val2, vreinterpret_u8_u32(src_val3));
                    pu1_dst_val2 += dst_strd;

                    vst1_u8(pu1_dst_val2, vreinterpret_u8_u32(src_val3));
                    pu1_dst_val2 += dst_strd;

                    vst1_u8(pu1_dst_val2, vreinterpret_u8_u32(src_val3));
                    pu1_dst_val2 += dst_strd;
                }
                pu1_ref_val2 += 4;
                pu1_dst_val3 += 4;
                pu1_dst_val2 = pu1_dst_val3;
                cond_4 = 1;
            }
        }

        /* rows and columns are unrolled by 8, when the width is multiple of 8          */
        else
        {
            WORD32 cond = 0, col_1;
            UWORD8 *pu1_dst_tmp_1 = pu1_dst;
            UWORD8 *pu1_dst_tmp_2 = pu1_dst;
            UWORD8 *pu1_dst_tmp_3 = pu1_dst;

            UWORD8 *pu1_ref_tmp_1 = pu1_ref;
            UWORD8 *pu1_ref_tmp_2 = pu1_ref;
            UWORD8 *pu1_ref_tmp_3 = pu1_ref;

            uint8x8_t pu1_src_tmp1;
            uint8x8_t pu1_src_tmp2;

            uint8x8_t dup_sub;
            uint16x8_t dup_add;
            int16x8_t subsh_val;
            int16x8_t addsat_val;
            uint16x8_t sub_val;
            uint8x8_t round_val;
            uint8x8_t vext_t;
            uint64x1_t shift_64;

            dup_sub = vdup_n_u8(pu1_ref[two_nt]);
            dup_add = vdupq_n_u16(pu1_ref[two_nt + 1]);

            pu1_ref_tmp_1 += (two_nt);
            pu1_ref_tmp_1 -= 8;
            pu1_ref_tmp_2 += (two_nt + 2);
            pu1_ref_tmp_3 += (two_nt + 1);

            /* loops to store the first nt sets of values in the destination */

            for(row = nt; row > 0; row -= 8)
            {
                for(col = (nt - 1); (col > 0) && (cond == 0); col -= 8)
                {
                    pu1_src_tmp1 = vld1_u8(pu1_ref_tmp_1);

                    sub_val = vsubl_u8(pu1_src_tmp1, dup_sub);
                    subsh_val  = vshrq_n_s16(vreinterpretq_s16_u16(sub_val), 1);
                    addsat_val = vqaddq_s16(subsh_val, vreinterpretq_s16_u16(dup_add));
                    round_val = vqmovun_s16(addsat_val);

                    /* unrolling pu1_dst[row * dst_strd + col] = pu1_ref[two_nt + 1 + col]; here*/

                    pu1_src_tmp2 = vld1_u8(pu1_ref_tmp_2);
                    vext_t = vext_u8(round_val, pu1_src_tmp2, 7);
                    vst1_u8(pu1_dst_tmp_1, vext_t);
                    pu1_dst_tmp_1 += dst_strd;

                    shift_64 = vshl_n_u64(vreinterpret_u64_u8(round_val), 8);

                    vext_t = vext_u8(vreinterpret_u8_u64(shift_64), pu1_src_tmp2, 7);
                    vst1_u8(pu1_dst_tmp_1, vext_t);
                    pu1_dst_tmp_1 += dst_strd;

                    shift_64 = vshl_n_u64(vreinterpret_u64_u8(round_val), 16);
                    vext_t = vext_u8(vreinterpret_u8_u64(shift_64), pu1_src_tmp2, 7);
                    vst1_u8(pu1_dst_tmp_1, vext_t);
                    pu1_dst_tmp_1 += dst_strd;

                    shift_64 = vshl_n_u64(vreinterpret_u64_u8(round_val), 24);
                    vext_t = vext_u8(vreinterpret_u8_u64(shift_64), pu1_src_tmp2, 7);
                    vst1_u8(pu1_dst_tmp_1, vext_t);
                    pu1_dst_tmp_1 += dst_strd;

                    shift_64 = vshl_n_u64(vreinterpret_u64_u8(round_val), 32);
                    vext_t = vext_u8(vreinterpret_u8_u64(shift_64), pu1_src_tmp2, 7);
                    vst1_u8(pu1_dst_tmp_1, vext_t);
                    pu1_dst_tmp_1 += dst_strd;

                    shift_64 = vshl_n_u64(vreinterpret_u64_u8(round_val), 40);
                    vext_t = vext_u8(vreinterpret_u8_u64(shift_64), pu1_src_tmp2, 7);
                    vst1_u8(pu1_dst_tmp_1, vext_t);
                    pu1_dst_tmp_1 += dst_strd;

                    shift_64 = vshl_n_u64(vreinterpret_u64_u8(round_val), 48);
                    vext_t = vext_u8(vreinterpret_u8_u64(shift_64), pu1_src_tmp2, 7);
                    vst1_u8(pu1_dst_tmp_1, vext_t);
                    pu1_dst_tmp_1 += dst_strd;

                    shift_64 = vshl_n_u64(vreinterpret_u64_u8(round_val), 56);
                    vext_t = vext_u8(vreinterpret_u8_u64(shift_64), pu1_src_tmp2, 7);
                    vst1_u8(pu1_dst_tmp_1, vext_t);
                    pu1_dst_tmp_1 += dst_strd;

                    pu1_ref_tmp_1 -= 8;
                }

                /* loop to store next sets of eight values in the destination */

                for(col_1 = nt - 7; (col_1 > 0) && (cond == 1); col_1 -= 8)
                {
                    pu1_src_tmp2 = vld1_u8(pu1_ref_tmp_3);

                    vst1_u8(pu1_dst_tmp_2, pu1_src_tmp2);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, pu1_src_tmp2);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, pu1_src_tmp2);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, pu1_src_tmp2);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, pu1_src_tmp2);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, pu1_src_tmp2);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, pu1_src_tmp2);
                    pu1_dst_tmp_2 += dst_strd;

                    vst1_u8(pu1_dst_tmp_2, pu1_src_tmp2);
                    pu1_dst_tmp_2 += dst_strd;
                }
                pu1_ref_tmp_3 += 8;
                pu1_dst_tmp_3 += 8;
                pu1_dst_tmp_2 = pu1_dst_tmp_3;
                cond = 1;
            }
        }
    }
}
/* INTRA_PRED_LUMA_VER */

/**
*******************************************************************************
*
* @brief
*    Intra prediction interpolation filter for luma mode2.
*
* @par Description:
*    Intraprediction for mode 2 (sw angle) with reference  neighboring samples
*    location pointed by 'pu1_ref' to the  TU block location pointed by
*    'pu1_dst'
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

void ihevc_intra_pred_luma_mode2_neonintr(UWORD8 *pu1_ref,
                                          WORD32 src_strd,
                                          UWORD8 *pu1_dst,
                                          WORD32 dst_strd,
                                          WORD32 nt,
                                          WORD32 mode)
{

    WORD32 row, col;
    WORD32 two_nt;
    UNUSED(src_strd);
    UNUSED(mode);

    /* rev_res naming has been made to have the reverse result value in it                              */
    /* Loops are unrolled by 4 and 8 considering the fact the input width is either multiple of 4 or 8  */
    /* rows and columns are unrolled by 4, when the width is multiple of 4                              */

    if(0 != (nt & 7))
    {
        UWORD8 *pu1_ref_tmp = pu1_ref;
        UWORD8 *pu1_dst_tmp = pu1_dst;
        uint8x8_t pu1_src_val, rev_res;
        uint64x1_t shift_res;

        for(col = nt; col > 0; col -= 4)
        {
            for(row = nt; row > 0; row -= 4)
            {
                /* unrolling all col & rows for pu1_dst[row + (col * dst_strd)] = pu1_ref[two_nt - col - idx - 1]; */

                pu1_src_val = vld1_u8(pu1_ref_tmp);
                shift_res = vshl_n_u64(vreinterpret_u64_u8(pu1_src_val), 8);
                rev_res = vrev64_u8(vreinterpret_u8_u64(shift_res));

                vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(rev_res), 0);
                pu1_dst_tmp += dst_strd;

                shift_res = vshr_n_u64(vreinterpret_u64_u8(rev_res), 8);
                vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u64(shift_res), 0);
                pu1_dst_tmp += dst_strd;

                shift_res = vshr_n_u64(shift_res, 8);
                vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u64(shift_res), 0);
                pu1_dst_tmp += dst_strd;

                shift_res = vshr_n_u64(shift_res, 8);
                vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u64(shift_res), 0);
                pu1_dst_tmp += dst_strd;
            }
        }
    }

    /* rev_val_second, rev_val_first  to reverse the loaded values in order to get the values in right order */
    /* shift_64 to shift the reversed 2nd values to get the value what we need                               */
    /* rows and columns are unrolled by 8, when the width is multiple of 8                              */

    else
    {
        UWORD8 *pu1_ref_two_nt_minus2 = pu1_ref;
        UWORD8 *pu1_dst_tmp = pu1_dst;
        UWORD8 *pu1_dst_tmp_plus8 = pu1_dst;

        uint8x8_t pu1_src_val1, pu1_src_val2, vext_t, rev_val_second, rev_val_first;
        uint64x1_t shift_val;

        two_nt = 2 * nt;
        pu1_ref_two_nt_minus2 += (two_nt);
        pu1_ref_two_nt_minus2 -= 8;

        for(col = nt; col > 0; col -= 8)
        {
            for(row = nt; row > 0; row -= 8)
            {
                pu1_src_val2 = vld1_u8(pu1_ref_two_nt_minus2);
                rev_val_first = vrev64_u8(pu1_src_val2);

                pu1_ref_two_nt_minus2 -= 8;
                pu1_src_val1 = vld1_u8(pu1_ref_two_nt_minus2);
                rev_val_second = vrev64_u8(pu1_src_val1);

                vext_t = vext_u8(rev_val_first, rev_val_second, 1);
                vst1_u8(pu1_dst_tmp, vext_t);
                pu1_dst_tmp += dst_strd;

                shift_val = vshr_n_u64(vreinterpret_u64_u8(rev_val_second), 8);
                vext_t = vext_u8(vext_t, vreinterpret_u8_u64(shift_val), 1);
                vst1_u8(pu1_dst_tmp, vext_t);
                pu1_dst_tmp += dst_strd;

                shift_val = vshr_n_u64(vreinterpret_u64_u8(rev_val_second), 16);
                vext_t = vext_u8(vext_t, vreinterpret_u8_u64(shift_val), 1);
                vst1_u8(pu1_dst_tmp, vext_t);
                pu1_dst_tmp += dst_strd;

                shift_val = vshr_n_u64(vreinterpret_u64_u8(rev_val_second), 24);
                vext_t = vext_u8(vext_t, vreinterpret_u8_u64(shift_val), 1);
                vst1_u8(pu1_dst_tmp, vext_t);
                pu1_dst_tmp += dst_strd;

                shift_val = vshr_n_u64(vreinterpret_u64_u8(rev_val_second), 32);
                vext_t = vext_u8(vext_t, vreinterpret_u8_u64(shift_val), 1);
                vst1_u8(pu1_dst_tmp, vext_t);
                pu1_dst_tmp += dst_strd;

                shift_val = vshr_n_u64(vreinterpret_u64_u8(rev_val_second), 40);
                vext_t = vext_u8(vext_t, vreinterpret_u8_u64(shift_val), 1);
                vst1_u8(pu1_dst_tmp, vext_t);
                pu1_dst_tmp += dst_strd;

                shift_val = vshr_n_u64(vreinterpret_u64_u8(rev_val_second), 48);
                vext_t = vext_u8(vext_t, vreinterpret_u8_u64(shift_val), 1);
                vst1_u8(pu1_dst_tmp, vext_t);
                pu1_dst_tmp += dst_strd;

                shift_val = vshr_n_u64(vreinterpret_u64_u8(rev_val_second), 56);
                vext_t = vext_u8(vext_t, vreinterpret_u8_u64(shift_val), 1);
                vst1_u8(pu1_dst_tmp, vext_t);
                pu1_dst_tmp += dst_strd;
            }
            pu1_dst_tmp_plus8 += 8;
            pu1_dst_tmp = pu1_dst_tmp_plus8;
            pu1_ref_two_nt_minus2 += (nt - 8);
        }
    }
}
/* INTRA_PRED_LUMA_MODE2 */

/**
*******************************************************************************
*
* @brief
*   Intra prediction interpolation filter for luma mode 18 & mode 34.
*
* @par Description:
*    Intraprediction for mode 34 (ne angle) with reference  neighboring
*    samples location pointed by 'pu1_ref' to the  TU block location pointed by
*    'pu1_dst'
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

void ihevc_intra_pred_luma_mode_18_34_neonintr(UWORD8 *pu1_ref,
                                               WORD32 src_strd,
                                               UWORD8 *pu1_dst,
                                               WORD32 dst_strd,
                                               WORD32 nt,
                                               WORD32 mode)
{

    WORD32 row, col, idx;
    WORD32 intraPredAngle = 32;
    WORD32 two_nt;
    UNUSED(src_strd);
    two_nt = 2 * nt;

    UWORD8 *pu1_ref_tmp = pu1_ref;
    UWORD8 *pu1_ref_tmp1 = pu1_ref;
    UWORD8 *pu1_dst_tmp = pu1_dst;
    UWORD8 *pu1_dst_tmp_plus8 = pu1_dst;

    uint8x8_t src_tmp_1st, src_tmp_2nd, vext1, vext2, vext3, vext4, vext5, vext6, vext7;

    /* src_tmp_1st, src_tmp_2nd are named as to load the 1st eight and next 8 values from source(pu1_ref)   */
    /* vext1 - vext7 are named to do vext operation between 2 loaded values and to handle dual issue        */
    /* Loops are unrolled by 4 and 8 considering the fact the input width is either multiple of 4 or 8      */
    /* rows and columns are unrolled by 8, when the width is multiple of 8                                  */
    /* loops are maintained separately for mode18 and mode34                                                */

    /* cond to allow multiples of 8 */
    if(0 == (nt & 7))
    {
        if(mode == 34)
        {
            pu1_ref_tmp += (two_nt + 2);

            for(row = nt; row > 0; row -= 8)
            {
                for(col = nt; col > 0; col -= 8)
                {
                    /* Loading 1st eight values */
                    src_tmp_1st = vld1_u8(pu1_ref_tmp);
                    pu1_ref_tmp += 8;

                    /* Loading next eight values */
                    src_tmp_2nd = vld1_u8(pu1_ref_tmp);

                    /* UNROLLED  pu1_dst[col + (row * dst_strd)] = pu1_ref[two_nt + col + idx + 1] */
                    vext1 = vext_u8(src_tmp_1st, src_tmp_2nd, 1);
                    vst1_u8(pu1_dst_tmp, src_tmp_1st);
                    pu1_dst_tmp += dst_strd;

                    vext2 = vext_u8(src_tmp_1st, src_tmp_2nd, 2);
                    vst1_u8(pu1_dst_tmp, vext1);
                    pu1_dst_tmp += dst_strd;

                    vext3 = vext_u8(src_tmp_1st, src_tmp_2nd, 3);
                    vst1_u8(pu1_dst_tmp, vext2);
                    pu1_dst_tmp += dst_strd;

                    vext4 = vext_u8(src_tmp_1st, src_tmp_2nd, 4);
                    vst1_u8(pu1_dst_tmp, vext3);
                    pu1_dst_tmp += dst_strd;

                    vext5 = vext_u8(src_tmp_1st, src_tmp_2nd, 5);
                    vst1_u8(pu1_dst_tmp, vext4);
                    pu1_dst_tmp += dst_strd;

                    vext6 = vext_u8(src_tmp_1st, src_tmp_2nd, 6);
                    vst1_u8(pu1_dst_tmp, vext5);
                    pu1_dst_tmp += dst_strd;

                    vext7 = vext_u8(src_tmp_1st, src_tmp_2nd, 7);
                    vst1_u8(pu1_dst_tmp, vext6);
                    pu1_dst_tmp += dst_strd;

                    vst1_u8(pu1_dst_tmp, vext7);
                    pu1_dst_tmp += dst_strd;
                }

                pu1_dst_tmp_plus8 += 8;
                pu1_dst_tmp = pu1_dst_tmp_plus8;
                pu1_ref_tmp -= (nt - 8);
            }
        }
        else /* Loop for mode 18 */
        {
            pu1_ref_tmp += (two_nt);

            for(row = nt; row > 0; row -= 8)
            {
                for(col = nt; col > 0; col -= 8)
                {
                    /* Loading 1st eight values */
                    src_tmp_1st = vld1_u8(pu1_ref_tmp);
                    pu1_ref_tmp -= 8;

                    /* Loading next eight values */
                    src_tmp_2nd = vld1_u8(pu1_ref_tmp);

                    /* UNROLLED  pu1_dst[col + (row * dst_strd)] = pu1_ref[two_nt + col + idx + 1] */
                    vext1 = vext_u8(src_tmp_2nd, src_tmp_1st, 7);
                    vst1_u8(pu1_dst_tmp, src_tmp_1st);
                    pu1_dst_tmp += dst_strd;

                    vext2 = vext_u8(src_tmp_2nd, src_tmp_1st, 6);
                    vst1_u8(pu1_dst_tmp, vext1);
                    pu1_dst_tmp += dst_strd;

                    vext3 = vext_u8(src_tmp_2nd, src_tmp_1st, 5);
                    vst1_u8(pu1_dst_tmp, vext2);
                    pu1_dst_tmp += dst_strd;

                    vext4 = vext_u8(src_tmp_2nd, src_tmp_1st, 4);
                    vst1_u8(pu1_dst_tmp, vext3);
                    pu1_dst_tmp += dst_strd;

                    vext5 = vext_u8(src_tmp_2nd, src_tmp_1st, 3);
                    vst1_u8(pu1_dst_tmp, vext4);
                    pu1_dst_tmp += dst_strd;

                    vext6 = vext_u8(src_tmp_2nd, src_tmp_1st, 2);
                    vst1_u8(pu1_dst_tmp, vext5);
                    pu1_dst_tmp += dst_strd;

                    vext7 = vext_u8(src_tmp_2nd, src_tmp_1st, 1);
                    vst1_u8(pu1_dst_tmp, vext6);
                    pu1_dst_tmp += dst_strd;

                    vst1_u8(pu1_dst_tmp, vext7);
                    pu1_dst_tmp += dst_strd;
                }
                pu1_dst_tmp_plus8 += 8;
                pu1_dst_tmp = pu1_dst_tmp_plus8;
                pu1_ref_tmp += (nt + 8);
            }
        }
    }

    /* rows and columns are unrolled by 4, when the width is multiple of 4  */

    else /* loop for multiples of 4 */
    {
        uint8x8_t src_val1;
        uint8x8_t src_val2;

        if(mode == 18)
            intraPredAngle = -32;
        else if(mode == 34)
            intraPredAngle = 32;

        for(row = 0; row < nt; row += 2)
        {
            /* unrolling 2 rows */
            idx = ((row + 1) * intraPredAngle) >> 5;
            pu1_ref_tmp = pu1_ref + two_nt + idx + 1;
            src_val1 = vld1_u8(pu1_ref_tmp);

            idx = ((row + 2) * intraPredAngle) >> 5;
            pu1_ref_tmp1 = pu1_ref + two_nt + idx + 1;
            src_val2 = vld1_u8(pu1_ref_tmp1);

            /* unrolling 4 col */
            for(col = nt; col > 0; col -= 4)
            {
                pu1_dst_tmp = pu1_dst;
                vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(src_val1), 0);
                pu1_dst_tmp += dst_strd;
                vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(src_val2), 0);
                pu1_dst += 4;
            }
            pu1_dst += 2 * dst_strd - nt;
        }
    }
}
/* INTRA_PRED_LUMA_MODE_18_34 */

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


void ihevc_intra_pred_luma_mode_3_to_9_neonintr(UWORD8 *pu1_ref,
                                                WORD32 src_strd,
                                                UWORD8 *pu1_dst,
                                                WORD32 dst_strd,
                                                WORD32 nt,
                                                WORD32 mode)
{

    WORD32 row, col;
    WORD32 intra_pred_ang;
    WORD32 pos, fract = 100, fract_prev;
    UNUSED(src_strd);
    if(0 == (nt & 7))
    {

        UWORD8 *pu1_ref_main_idx = pu1_ref;
        UWORD8 *pu1_ref_main_idx_1 = pu1_ref;

        UWORD8 *pu1_dst_tmp1 = pu1_dst;
        UWORD8 *pu1_dst_tmp2 = pu1_dst;

        WORD32 two_nt = 2 * nt;

        pu1_ref_main_idx += two_nt;
        pu1_ref_main_idx_1 += two_nt - 1;

        uint8x8_t dup_const_fract, dup_const_32_fract, ref_main_idx, ref_main_idx_1;
        uint8x8_t shift_res;
        uint16x8_t mul_res1, mul_res2, add_res;

        /* Intra Pred Angle according to the mode */
        intra_pred_ang = gai4_ihevc_ang_table[mode];

        pu1_ref_main_idx -= 8;
        pu1_ref_main_idx_1 -= 8;

        for(col = 0; col < nt; col++)
        {
            fract_prev = fract;

            pos = ((col + 1) * intra_pred_ang);
            fract = pos & (31);

            if(fract_prev < fract)
            {
                pu1_ref_main_idx += 1;
                pu1_ref_main_idx_1 += 1;
            }

            dup_const_fract = vdup_n_u8((uint8_t)fract);
            dup_const_32_fract = vdup_n_u8((uint8_t)(32 - fract));

            for(row = nt; row > 0; row -= 8)
            {
                ref_main_idx = vld1_u8(pu1_ref_main_idx);
                ref_main_idx_1 = vld1_u8(pu1_ref_main_idx_1);

                mul_res1 = vmull_u8(ref_main_idx, dup_const_32_fract);
                mul_res2 = vmull_u8(ref_main_idx_1, dup_const_fract);

                add_res = vaddq_u16(mul_res1, mul_res2);

                shift_res = vrshrn_n_u16(add_res, 5);

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 7);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 6);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 5);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 4);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 3);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 2);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 1);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 0);
                pu1_dst_tmp1 += dst_strd;

                pu1_ref_main_idx -= 8;
                pu1_ref_main_idx_1 -= 8;

            }
            pu1_dst_tmp2 += 1;
            pu1_dst_tmp1 = pu1_dst_tmp2;

            pu1_ref_main_idx += nt;
            pu1_ref_main_idx_1 += nt;

            pu1_ref_main_idx -= 1;
            pu1_ref_main_idx_1 -= 1;

        }
    }
    else
    {
        UWORD8 *pu1_ref_tmp1 = pu1_ref;
        UWORD8 *pu1_ref_tmp2 = pu1_ref;
        UWORD8 *pu1_dst_tmp1 = pu1_dst;
        UWORD8 *pu1_dst_tmp2 = pu1_dst;

        pu1_ref_tmp1 += nt;
        pu1_ref_tmp2 += (nt - 1);

        uint8x8_t dup_fract, dup_32_fract, shift_res;
        uint16x8_t mul_res1, mul_res2, add_res;
        uint32x2_t  pu1_ref_val1, pu1_ref_val2;

        pu1_ref_val1 = vdup_n_u32(0);
        pu1_ref_val2 = vdup_n_u32(0);

        /* Intra Pred Angle according to the mode */
        intra_pred_ang = gai4_ihevc_ang_table[mode];


        for(col = 0; col < nt; col++)
        {
            fract_prev = fract;
            pos = ((col + 1) * intra_pred_ang);
            fract = pos & (31);
            if(fract_prev < fract)
            {
                pu1_ref_tmp1 += 1;
                pu1_ref_tmp2 += 1;
            }
            dup_fract = vdup_n_u8((uint8_t)fract);
            dup_32_fract = vdup_n_u8((uint8_t)(32 - fract));

            for(row = nt; row > 0; row -= 4)
            {
                pu1_ref_val1 = vld1_lane_u32((uint32_t *)pu1_ref_tmp1, pu1_ref_val1, 0);
                pu1_ref_val2 = vld1_lane_u32((uint32_t *)pu1_ref_tmp2, pu1_ref_val2, 0);

                mul_res1 = vmull_u8(vreinterpret_u8_u32(pu1_ref_val1), dup_32_fract);
                mul_res2 = vmull_u8(vreinterpret_u8_u32(pu1_ref_val2), dup_fract);

                add_res = vaddq_u16(mul_res1, mul_res2);

                shift_res = vrshrn_n_u16(add_res, 5);

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 3);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 2);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 1);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 0);

            }
            pu1_ref_tmp1 -= 1;
            pu1_ref_tmp2 -= 1;

            pu1_dst_tmp2 += 1;
            pu1_dst_tmp1 = pu1_dst_tmp2;

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


void ihevc_intra_pred_luma_mode_11_to_17_neonintr(UWORD8 *pu1_ref,
                                                  WORD32 src_strd,
                                                  UWORD8 *pu1_dst,
                                                  WORD32 dst_strd,
                                                  WORD32 nt,
                                                  WORD32 mode)
{

    WORD32 row, col, k;
    WORD32 two_nt;
    WORD32 intra_pred_ang, inv_ang, inv_ang_sum;
    WORD32 pos, fract = 1000, fract_prev;
    WORD32  ref_idx;

    UWORD8 *ref_main;
    UWORD8 *ref_main_tmp;

    UWORD8 *pu1_ref_tmp1 = pu1_ref;
    UWORD8 *pu1_ref_tmp2 = pu1_ref;
    UWORD8 *pu1_dst_tmp1 = pu1_dst;
    UWORD8 *pu1_dst_tmp2 = pu1_dst;

    UWORD8 ref_temp[2 * MAX_CU_SIZE + 1];

    uint16x8_t mul_res1, mul_res2, add_res;
    uint8x8_t dup_const_fract, dup_const_32_fract;
    uint8x8_t ref_main_idx, ref_main_idx_1, shift_res;
    uint8x8_t ref_left_t;
    uint32x2_t  ref_left_tmp;
    UNUSED(src_strd);
    ref_left_tmp = vdup_n_u32(0);

    inv_ang_sum = 128;
    two_nt = 2 * nt;

    intra_pred_ang = gai4_ihevc_ang_table[mode];

    inv_ang = gai4_ihevc_inv_ang_table[mode - 11];

    pu1_ref_tmp1 += two_nt;

    ref_main = ref_temp + (nt - 1);
    ref_main_tmp = ref_main;

    if(0 == (nt & 7))
    {
        pu1_ref_tmp2 += (two_nt - 7);

        for(k = nt - 1; k >= 0; k -= 8)
        {

            ref_left_t = vld1_u8(pu1_ref_tmp2);

            ref_left_t = vrev64_u8(ref_left_t);
            vst1_u8(ref_main_tmp, ref_left_t);
            ref_main_tmp += 8;
            pu1_ref_tmp2 -= 8;

        }

    }
    else
    {
        uint8x8_t rev_val;
        pu1_ref_tmp2 += (two_nt - (nt - 1));

        for(k = nt - 1; k >= 0; k -= 8)
        {

            ref_left_tmp = vld1_lane_u32((uint32_t *)pu1_ref_tmp2, ref_left_tmp, 1);

            rev_val = vrev64_u8(vreinterpret_u8_u32(ref_left_tmp));
            vst1_lane_u32((uint32_t *)ref_main_tmp, vreinterpret_u32_u8(rev_val), 0);

        }

    }

    ref_main[nt] = pu1_ref[two_nt - nt];

    /* For horizontal modes, (ref main = ref left) (ref side = ref above) */

    ref_idx = (nt * intra_pred_ang) >> 5;

    /* SIMD Optimization can be done using look-up table for the loop */
    /* For negative angled derive the main reference samples from side */
    /*  reference samples refer to section 8.4.4.2.6 */
    for(k = -1; k > ref_idx; k--)
    {
        inv_ang_sum += inv_ang;
        ref_main[k] = pu1_ref[two_nt + (inv_ang_sum >> 8)];
    }

    UWORD8 *ref_main_tmp1 = ref_main;
    UWORD8 *ref_main_tmp2 = ref_main;

    ref_main_tmp2 += 1;

    if(0 == (nt & 7))
    {
        /* For the angles other then 45 degree, interpolation btw 2 neighboring */
        /* samples dependent on distance to obtain destination sample */
        for(col = 0; col < nt; col++)
        {

            fract_prev = fract;
            pos = ((col + 1) * intra_pred_ang);
            fract = pos & (31);

            if(fract_prev < fract)
            {
                ref_main_tmp1 -= 1;
                ref_main_tmp2 -= 1;
            }

            dup_const_fract = vdup_n_u8((uint8_t)fract);
            dup_const_32_fract = vdup_n_u8((uint8_t)(32 - fract));

            // Do linear filtering
            for(row = nt; row > 0; row -= 8)
            {
                ref_main_idx = vld1_u8(ref_main_tmp1);

                ref_main_idx_1 = vld1_u8(ref_main_tmp2);

                mul_res1 = vmull_u8(ref_main_idx, dup_const_32_fract);
                mul_res2 = vmull_u8(ref_main_idx_1, dup_const_fract);

                add_res = vaddq_u16(mul_res1, mul_res2);

                shift_res = vrshrn_n_u16(add_res, 5);

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 0);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 1);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 2);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 3);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 4);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 5);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 6);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 7);
                pu1_dst_tmp1 += dst_strd;

                ref_main_tmp1 += 8;
                ref_main_tmp2 += 8;
            }

            ref_main_tmp1 -= nt;
            ref_main_tmp2 -= nt;

            pu1_dst_tmp2 += 1;
            pu1_dst_tmp1 = pu1_dst_tmp2;
        }
    }
    else
    {
        uint32x2_t ref_main_idx1, ref_main_idx2;

        ref_main_idx1 = vdup_n_u32(0);
        ref_main_idx2 = vdup_n_u32(0);

        for(col = 0; col < nt; col++)
        {
            fract_prev = fract;
            pos = ((col + 1) * intra_pred_ang);
            fract = pos & (31);

            if(fract_prev < fract)
            {
                ref_main_tmp1 -= 1;
                ref_main_tmp2 -= 1;
            }

            dup_const_fract = vdup_n_u8((uint8_t)fract);
            dup_const_32_fract = vdup_n_u8((uint8_t)(32 - fract));

            for(row = nt; row > 0; row -= 4)
            {

                ref_main_idx1 = vld1_lane_u32((uint32_t *)ref_main_tmp1, ref_main_idx1, 0);
                ref_main_idx2 = vld1_lane_u32((uint32_t *)ref_main_tmp2, ref_main_idx2, 0);

                mul_res1 = vmull_u8(vreinterpret_u8_u32(ref_main_idx1), dup_const_32_fract);
                mul_res2 = vmull_u8(vreinterpret_u8_u32(ref_main_idx2), dup_const_fract);

                add_res = vaddq_u16(mul_res1, mul_res2);

                shift_res = vrshrn_n_u16(add_res, 5);

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 0);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 1);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 2);
                pu1_dst_tmp1 += dst_strd;

                vst1_lane_u8(pu1_dst_tmp1, shift_res, 3);
                pu1_dst_tmp1 += dst_strd;

            }

            pu1_dst_tmp2 += 1;
            pu1_dst_tmp1 = pu1_dst_tmp2;

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


void ihevc_intra_pred_luma_mode_19_to_25_neonintr(UWORD8 *pu1_ref,
                                                  WORD32 src_strd,
                                                  UWORD8 *pu1_dst,
                                                  WORD32 dst_strd,
                                                  WORD32 nt,
                                                  WORD32 mode)
{

    WORD32 row, col, k;
    WORD32 two_nt, intra_pred_ang;
    WORD32 inv_ang, inv_ang_sum, pos, fract = 1000, fract_prev;;
    WORD32 ref_idx;
    UWORD8 *ref_main;
    UWORD8 *ref_main_tmp;
    UWORD8 ref_temp[(2 * MAX_CU_SIZE) + 1];

    UWORD8 *pu1_ref_tmp1 = pu1_ref;
    UWORD8 *pu1_ref_tmp2 = pu1_ref;
    UWORD8 *pu1_dst_tmp1 = pu1_dst;

    uint16x8_t mul_res1, mul_res2, add_res;
    uint8x8_t dup_const_fract, dup_const_32_fract;
    uint8x8_t ref_main_idx, ref_main_idx_1, shift_res;
    uint8x8_t ref_above_t;
    uint32x2_t ref_above_tmp;
    UNUSED(src_strd);
    ref_above_tmp = vdup_n_u32(0);

    two_nt = 2 * nt;
    intra_pred_ang = gai4_ihevc_ang_table[mode];
    inv_ang = gai4_ihevc_inv_ang_table[mode - 12];

    /* Intermediate reference samples for negative angle modes */
    /* This have to be removed during optimization*/
    pu1_ref_tmp1 += two_nt;


    ref_main = ref_temp + (nt - 1);
    ref_main_tmp = ref_main;

    if(0 == (nt & 7))
    {
        pu1_ref_tmp2 += (two_nt - 7);
        for(k = nt - 1; k >= 0; k -= 8)
        {

            ref_above_t = vld1_u8(pu1_ref_tmp1);
            vst1_u8(ref_main_tmp, ref_above_t);
            ref_main_tmp += 8;
            pu1_ref_tmp1 += 8;

        }

    }
    else
    {
        pu1_ref_tmp2 += (two_nt - (nt - 1));

        for(k = nt - 1; k >= 0; k -= 4)
        {

            ref_above_tmp = vld1_lane_u32((uint32_t *)pu1_ref_tmp1, ref_above_tmp, 0);
            vst1_lane_u32((uint32_t *)ref_main_tmp, ref_above_tmp, 0);

        }

    }

    ref_main[nt] = pu1_ref[two_nt + nt];

    /* For horizontal modes, (ref main = ref above) (ref side = ref left) */

    ref_idx = (nt * intra_pred_ang) >> 5;
    inv_ang_sum = 128;

    /* SIMD Optimization can be done using look-up table for the loop */
    /* For negative angled derive the main reference samples from side */
    /*  reference samples refer to section 8.4.4.2.6 */
    for(k = -1; k > ref_idx; k--)
    {
        inv_ang_sum += inv_ang;
        ref_main[k] = pu1_ref[two_nt - (inv_ang_sum >> 8)];
    }

    UWORD8 *ref_main_tmp1 = ref_main;
    UWORD8 *ref_main_tmp2 = ref_main;

    ref_main_tmp2 += 1;

    if(0 == (nt & 7))
    {
        /* For the angles other then 45 degree, interpolation btw 2 neighboring */
        /* samples dependent on distance to obtain destination sample */
        for(row = 0; row < nt; row++)
        {

            fract_prev = fract;
            pos = ((row + 1) * intra_pred_ang);
            fract = pos & (31);

            if(fract_prev < fract)
            {
                ref_main_tmp1 -= 1;
                ref_main_tmp2 -= 1;
            }

            dup_const_fract = vdup_n_u8((uint8_t)fract);
            dup_const_32_fract = vdup_n_u8((uint8_t)(32 - fract));

            // Do linear filtering
            for(col = nt; col > 0; col -= 8)
            {
                ref_main_idx = vld1_u8(ref_main_tmp1);

                ref_main_idx_1 = vld1_u8(ref_main_tmp2);

                mul_res1 = vmull_u8(ref_main_idx, dup_const_32_fract);
                mul_res2 = vmull_u8(ref_main_idx_1, dup_const_fract);

                add_res = vaddq_u16(mul_res1, mul_res2);

                shift_res = vrshrn_n_u16(add_res, 5);

                vst1_u8(pu1_dst_tmp1, shift_res);
                pu1_dst_tmp1 += 8;

                ref_main_tmp1 += 8;
                ref_main_tmp2 += 8;
            }

            ref_main_tmp1 -= nt;
            ref_main_tmp2 -= nt;

            pu1_dst_tmp1 += (dst_strd - nt);
        }
    }
    else
    {
        uint32x2_t ref_main_idx1, ref_main_idx2;

        ref_main_idx1 = vdup_n_u32(0);
        ref_main_idx2 = vdup_n_u32(0);

        for(row = 0; row < nt; row++)
        {
            fract_prev = fract;
            pos = ((row + 1) * intra_pred_ang);
            fract = pos & (31);

            if(fract_prev < fract)
            {
                ref_main_tmp1 -= 1;
                ref_main_tmp2 -= 1;
            }

            dup_const_fract = vdup_n_u8((uint8_t)fract);
            dup_const_32_fract = vdup_n_u8((uint8_t)(32 - fract));

            for(col = nt; col > 0; col -= 4)
            {

                ref_main_idx1 = vld1_lane_u32((uint32_t *)ref_main_tmp1, ref_main_idx1, 0);
                ref_main_idx2 = vld1_lane_u32((uint32_t *)ref_main_tmp2, ref_main_idx2, 0);

                mul_res1 = vmull_u8(vreinterpret_u8_u32(ref_main_idx1), dup_const_32_fract);
                mul_res2 = vmull_u8(vreinterpret_u8_u32(ref_main_idx2), dup_const_fract);

                add_res = vaddq_u16(mul_res1, mul_res2);

                shift_res = vrshrn_n_u16(add_res, 5);

                vst1_lane_u32((uint32_t *)pu1_dst_tmp1, vreinterpret_u32_u8(shift_res), 0);
                pu1_dst_tmp1 += 4;

            }
            pu1_dst_tmp1 += (dst_strd - nt);
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


void ihevc_intra_pred_luma_mode_27_to_33_neonintr(UWORD8 *pu1_ref,
                                                  WORD32 src_strd,
                                                  UWORD8 *pu1_dst,
                                                  WORD32 dst_strd,
                                                  WORD32 nt,
                                                  WORD32 mode)
{

    WORD32 row, col;
    WORD32 intra_pred_ang;
    WORD32 pos, fract = 0, fract_prev;

    WORD32 two_nt = 2 * nt;
    UNUSED(src_strd);
    if(0 == (nt & 7))
    {

        UWORD8 *pu1_ref_main_idx = pu1_ref;
        UWORD8 *pu1_ref_main_idx_1 = pu1_ref;

        UWORD8 *pu1_dst_tmp1 = pu1_dst;
        pu1_ref_main_idx += (two_nt + 1);
        pu1_ref_main_idx_1 += (two_nt + 2);

        uint8x8_t dup_const_fract, dup_const_32_fract, ref_main_idx, ref_main_idx_1;
        uint8x8_t shift_res;
        uint16x8_t mul_res1, mul_res2, add_res;

        /* Intra Pred Angle according to the mode */
        intra_pred_ang = gai4_ihevc_ang_table[mode];

        for(row = 0; row < nt; row++)
        {
            fract_prev = fract;

            pos = ((row + 1) * intra_pred_ang);
            fract = pos & (31);

            if(fract_prev > fract)
            {
                pu1_ref_main_idx += 1;
                pu1_ref_main_idx_1 += 1;
            }

            dup_const_fract = vdup_n_u8((uint8_t)fract);
            dup_const_32_fract = vdup_n_u8((uint8_t)(32 - fract));

            for(col = nt; col > 0; col -= 8)
            {
                ref_main_idx = vld1_u8(pu1_ref_main_idx);
                ref_main_idx_1 = vld1_u8(pu1_ref_main_idx_1);

                mul_res1 = vmull_u8(ref_main_idx, dup_const_32_fract);
                mul_res2 = vmull_u8(ref_main_idx_1, dup_const_fract);

                add_res = vaddq_u16(mul_res1, mul_res2);

                shift_res = vrshrn_n_u16(add_res, 5);

                vst1_u8(pu1_dst_tmp1, shift_res);
                pu1_dst_tmp1 += 8;

                pu1_ref_main_idx += 8;
                pu1_ref_main_idx_1 += 8;
            }

            pu1_ref_main_idx -= nt;
            pu1_ref_main_idx_1 -= nt;

            pu1_dst_tmp1 += (dst_strd - nt);
        }

    }
    else
    {
        UWORD8 *pu1_ref_tmp1 = pu1_ref;
        UWORD8 *pu1_ref_tmp2 = pu1_ref;
        UWORD8 *pu1_dst_tmp1 = pu1_dst;

        pu1_ref_tmp1 += (two_nt + 1);;
        pu1_ref_tmp2 += (two_nt + 2);;

        uint8x8_t dup_fract, dup_32_fract, shift_res;
        uint16x8_t mul_res1, mul_res2, add_res;
        uint32x2_t  pu1_ref_val1, pu1_ref_val2;

        pu1_ref_val1 = vdup_n_u32(0);
        pu1_ref_val2 = vdup_n_u32(0);

        /* Intra Pred Angle according to the mode */
        intra_pred_ang = gai4_ihevc_ang_table[mode];

        for(row = 0; row < nt; row++)
        {
            fract_prev = fract;
            pos = ((row + 1) * intra_pred_ang);
            fract = pos & (31);
            if(fract_prev > fract)
            {
                pu1_ref_tmp1 += 1;
                pu1_ref_tmp2 += 1;
            }
            dup_fract = vdup_n_u8((uint8_t)fract);
            dup_32_fract = vdup_n_u8((uint8_t)(32 - fract));

            for(col = nt; col > 0; col -= 4)
            {
                pu1_ref_val1 = vld1_lane_u32((uint32_t *)pu1_ref_tmp1, pu1_ref_val1, 0);
                pu1_ref_val2 = vld1_lane_u32((uint32_t *)pu1_ref_tmp2, pu1_ref_val2, 0);

                mul_res1 = vmull_u8(vreinterpret_u8_u32(pu1_ref_val1), dup_32_fract);
                mul_res2 = vmull_u8(vreinterpret_u8_u32(pu1_ref_val2), dup_fract);

                add_res = vaddq_u16(mul_res1, mul_res2);

                shift_res = vrshrn_n_u16(add_res, 5);

                vst1_lane_u32((uint32_t *)pu1_dst_tmp1, vreinterpret_u32_u8(shift_res), 0);
                pu1_dst_tmp1 += 4;

            }

            pu1_dst_tmp1 += (dst_strd - nt);

        }


    }

}
