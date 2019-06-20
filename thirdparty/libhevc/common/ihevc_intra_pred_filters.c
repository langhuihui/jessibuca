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
*  ihevc_intra_pred_filters.c
*
* @brief
*  Contains function Definition for intra prediction  interpolation filters
*
*
* @author
*  Srinivas T
*
* @par List of Functions:
*  - ihevc_intra_pred_luma_planar()
*  - ihevc_intra_pred_luma_dc()
*  - ihevc_intra_pred_luma_horz()
*  - ihevc_intra_pred_luma_ver()
*  - ihevc_intra_pred_luma_mode2()
*  - ihevc_intra_pred_luma_mode_18_34()
*  - ihevc_intra_pred_luma_mode_3_to_9()
*  - ihevc_intra_pred_luma_mode_11_to_17()
*  - ihevc_intra_pred_luma_mode_19_to_25()
*  - ihevc_intra_pred_luma_mode_27_to_33()
*  - ihevc_intra_pred_luma_ref_substitution()
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
#include "ihevc_typedefs.h"
#include "ihevc_intra_pred.h"
#include "ihevc_macros.h"
#include "ihevc_func_selector.h"
#include "ihevc_platform_macros.h"
#include "ihevc_common_tables.h"
#include "ihevc_defs.h"
#include "ihevc_mem_fns.h"
#include "ihevc_debug.h"

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
void ihevc_intra_pred_luma_ref_subst_all_avlble(UWORD8 *pu1_top_left,
                                                UWORD8 *pu1_top,
                                                UWORD8 *pu1_left,
                                                WORD32 src_strd,
                                                WORD32 nt,
                                                WORD32 nbr_flags,
                                                UWORD8 *pu1_dst,
                                                WORD32 dst_strd)
{

    WORD32 i;
    WORD32 two_nt = 2 * nt;
    UNUSED(nbr_flags);
    UNUSED(dst_strd);

    /* Neighbor Flag Structure*/
    /* MSB ---> LSB */
    /*    Top-Left | Top-Right | Top | Left | Bottom-Left
              1         4         4     4         4
     */
    ASSERT((nbr_flags == 0x11188) || (nbr_flags == 0x133CC) || (nbr_flags == 0x1FFFF));
    {

        if(nt == 4)
        {
            /* 1 bit extraction for all the neighboring blocks */


            /* Else fill the corresponding samples */
            pu1_dst[two_nt] = *pu1_top_left;
            //if(left)
            {
                for(i = 0; i < nt; i++)
                    pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
            }
//            if(bot_left)
            {
                for(i = nt; i < two_nt; i++)
                    pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
            }
//            if(top)
            {
                ihevc_memcpy(&pu1_dst[two_nt + 1], pu1_top, nt);
            }
//            if(tp_right)
            {
                ihevc_memcpy(&pu1_dst[two_nt + 1 + nt], pu1_top + nt, nt);
            }


        }
        else

        {

            /* Else fill the corresponding samples */
            ASSERT((nt == 8) || (nt == 16) || (nt == 32));
            pu1_dst[two_nt] = *pu1_top_left;

            for(i = 0; i < nt; i++)
                pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];

            for(i = nt; i < two_nt; i++)
                pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];

            ihevc_memcpy_mul_8(&pu1_dst[two_nt + 1], pu1_top, nt);

            ihevc_memcpy_mul_8(&pu1_dst[two_nt + 1 + nt], pu1_top + nt, nt);
        }

    }
}


void ihevc_intra_pred_luma_ref_substitution(UWORD8 *pu1_top_left,
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
    /*dc_val = 1 << (BIT_DEPTH - 1);*/
    dc_val = 1 << (8 - 1);


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
        if(nt <= 8)
        {
            /* 1 bit extraction for all the neighboring blocks */
            tp_left = (nbr_flags & 0x10000) >> 16;
            bot_left = (nbr_flags & 0x8) >> 3;
            left = (nbr_flags & 0x80) >> 7;
            top = (nbr_flags & 0x100) >> 8;
            tp_right = (nbr_flags & 0x1000) >> 12;

            /* Else fill the corresponding samples */
            if(tp_left)
                pu1_dst[two_nt] = *pu1_top_left;
            else
                pu1_dst[two_nt] = 0;


            if(left)
            {
                for(i = 0; i < nt; i++)
                    pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
            }
            else
            {
                ihevc_memset(&pu1_dst[two_nt - 1 - (nt - 1)], 0, nt);
            }


            if(bot_left)
            {
                for(i = nt; i < two_nt; i++)
                    pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
            }
            else
            {
                ihevc_memset(&pu1_dst[two_nt - 1 - (two_nt - 1)], 0, nt);
            }


            if(top)
            {
                ihevc_memcpy(&pu1_dst[two_nt + 1], pu1_top, nt);
            }
            else
            {
                ihevc_memset(&pu1_dst[two_nt + 1], 0, nt);
            }

            if(tp_right)
            {
                ihevc_memcpy(&pu1_dst[two_nt + 1 + nt], pu1_top + nt, nt);
            }
            else
            {
                ihevc_memset(&pu1_dst[two_nt + 1 + nt], 0, nt);
            }
            next = 1;

            /* If bottom -left is not available, reverse substitution process*/
            if(bot_left == 0)
            {
                WORD32 a_nbr_flag[5];
                a_nbr_flag[0] = bot_left;
                a_nbr_flag[1] = left;
                a_nbr_flag[2] = tp_left;
                a_nbr_flag[3] = top;
                a_nbr_flag[4] = tp_right;

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
                ihevc_memset(&pu1_dst[nt], pu1_dst[nt - 1], nt);

            }
            /* If top-left is Unavailable, copy the last left value */
            if(tp_left == 0)
                pu1_dst[two_nt] = pu1_dst[two_nt - 1];
            /* If top is Unavailable, copy the last top-left value */
            if(top == 0)
            {
                ihevc_memset(&pu1_dst[two_nt + 1], pu1_dst[two_nt], nt);
            }
            /* If to right is Unavailable, copy the last top value */
            if(tp_right == 0)
            {
                ihevc_memset(&pu1_dst[three_nt + 1], pu1_dst[three_nt], nt);

            }
        }

        if(nt == 16)
        {
            WORD32 nbr_flags_temp = 0;
            nbr_flags_temp = ((nbr_flags & 0xC) >> 2) + ((nbr_flags & 0xC0) >> 4)
                            + ((nbr_flags & 0x300) >> 4)
                            + ((nbr_flags & 0x3000) >> 6)
                            + ((nbr_flags & 0x10000) >> 8);

            /* Else fill the corresponding samples */
            if(nbr_flags & 0x10000)
                pu1_dst[two_nt] = *pu1_top_left;
            else
                pu1_dst[two_nt] = 0;

            if(nbr_flags & 0xC0)
            {
                for(i = 0; i < nt; i++)
                    pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
            }
            else
            {
                ihevc_memset_mul_8(&pu1_dst[two_nt - 1 - (nt - 1)], 0, nt);
            }

            /* Bottom - left availability is checked for every 8x8 TU position and set accordingly */
            {
                if(nbr_flags & 0x8)
                {
                    for(i = nt; i < (nt + 8); i++)
                    pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
                }
                else
                {
                    ihevc_memset_mul_8(&pu1_dst[nt - 8], 0, 8);
                }

                if(nbr_flags & 0x4)
                {
                    for(i = (nt + 8); i < two_nt; i++)
                        pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
                }
                else
                {
                    ihevc_memset_mul_8(&pu1_dst[0], 0, 8);
                }
            }

            if(nbr_flags & 0x300)
            {
                ihevc_memcpy_mul_8(&pu1_dst[two_nt + 1], pu1_top, nt);
            }
            else
            {
                ihevc_memset_mul_8(&pu1_dst[two_nt + 1], 0, nt);
            }

            if(nbr_flags & 0x3000)
            {
                ihevc_memcpy_mul_8(&pu1_dst[two_nt + 1 + nt], pu1_top + nt, nt);
            }
            else
            {
                ihevc_memset_mul_8(&pu1_dst[two_nt + 1 + nt], 0, nt);
            }
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
            while(nbr_id_from_bl < ((T16_4NT)+1))
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
                        ihevc_memset_mul_8(pu1_dst + nbr_id_from_bl, pu1_ref, 8);


                    }

                }
                nbr_id_from_bl += (nbr_id_from_bl == (T16_4NT / 2)) ? 1 : 8;
            }


        }

        if(nt == 32)
        {
            /* Else fill the corresponding samples */
            if(nbr_flags & 0x10000)
                pu1_dst[two_nt] = *pu1_top_left;
            else
                pu1_dst[two_nt] = 0;

            if(nbr_flags & 0xF0)
            {
                for(i = 0; i < nt; i++)
                    pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
            }
            else
            {
                ihevc_memset_mul_8(&pu1_dst[two_nt - 1 - (nt - 1)], 0, nt);
            }

            /* Bottom - left availability is checked for every 8x8 TU position and set accordingly */
            {
                if(nbr_flags & 0x8)
                {
                    for(i = nt; i < (nt + 8); i++)
                    pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
                }
                else
                {
                    ihevc_memset_mul_8(&pu1_dst[24], 0, 8);
                }

                if(nbr_flags & 0x4)
                {
                    for(i = (nt + 8); i < (nt + 16); i++)
                        pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
                }
                else
                {
                    ihevc_memset_mul_8(&pu1_dst[16], 0, 8);
                }

                if(nbr_flags & 0x2)
                {
                    for(i = (nt + 16); i < (nt + 24); i++)
                        pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
                }
                else
                {
                    ihevc_memset_mul_8(&pu1_dst[8], 0, 8);
                }

                if(nbr_flags & 0x1)
                {
                    for(i = (nt + 24); i < (two_nt); i++)
                        pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
                }
                else
                {
                    ihevc_memset_mul_8(&pu1_dst[0], 0, 8);
                }
            }


            if(nbr_flags & 0xF00)
            {
                ihevc_memcpy_mul_8(&pu1_dst[two_nt + 1], pu1_top, nt);
            }
            else
            {
                ihevc_memset_mul_8(&pu1_dst[two_nt + 1], 0, nt);
            }

            if(nbr_flags & 0xF000)
            {
                ihevc_memcpy_mul_8(&pu1_dst[two_nt + 1 + nt], pu1_top + nt, nt);
            }
            else
            {
                ihevc_memset_mul_8(&pu1_dst[two_nt + 1 + nt], 0, nt);
            }
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
                        ihevc_memset_mul_8(&pu1_dst[nbr_id_from_bl], pu1_ref, 8);

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


void ihevc_intra_pred_ref_filtering(UWORD8 *pu1_src,
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
    /*WORD32 dc_val = 1 << (BIT_DEPTH - 5);*/
    WORD32 dc_val = 1 << (8 - 5);
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
            for(i = 0; i < (four_nt + 1); i++)
                pu1_dst[i] = pu1_src[i];
        }
    }

    else
    {
        /* If strong intra smoothin is enabled and transform size is 32 */
        if((1 == strong_intra_smoothing_enable_flag) && (32 == nt))
        {
            /* Strong Intra Filtering */
            abs_cond_top_flag = (ABS(pu1_src[2 * nt] + pu1_src[4 * nt]
                            - (2 * pu1_src[3 * nt]))) < dc_val;
            abs_cond_left_flag = (ABS(pu1_src[2 * nt] + pu1_src[0]
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
            /* Perform bilinear filtering of Reference Samples */
            for(i = 0; i < (four_nt - 1); i++)
            {
                au1_flt[i + 1] = (pu1_src[i] + 2 * pu1_src[i + 1]
                                + pu1_src[i + 2] + 2) >> 2;
            }
        }


        for(i = 0; i < (four_nt + 1); i++)
            pu1_dst[i] = au1_flt[i];
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


void ihevc_intra_pred_luma_planar(UWORD8 *pu1_ref,
                                  WORD32 src_strd,
                                  UWORD8 *pu1_dst,
                                  WORD32 dst_strd,
                                  WORD32 nt,
                                  WORD32 mode)
{


    WORD32 row, col;
    WORD32 log2nt = 5;
    WORD32 two_nt, three_nt;
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
    /* Planar filtering */
    for(row = 0; row < nt; row++)
    {
        for(col = 0; col < nt; col++)
        {
            pu1_dst[row * dst_strd + col] = ((nt - 1 - col)
                            * pu1_ref[two_nt - 1 - row]
                            + (col + 1) * pu1_ref[three_nt + 1]
                            + (nt - 1 - row) * pu1_ref[two_nt + 1 + col]
                            + (row + 1) * pu1_ref[nt - 1] + nt) >> (log2nt + 1);
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


void ihevc_intra_pred_luma_dc(UWORD8 *pu1_ref,
                              WORD32 src_strd,
                              UWORD8 *pu1_dst,
                              WORD32 dst_strd,
                              WORD32 nt,
                              WORD32 mode)
{

    WORD32 acc_dc;
    WORD32 dc_val, two_dc_val, three_dc_val;
    WORD32 i;
    WORD32 row, col;
    WORD32 log2nt = 5;
    WORD32 two_nt, three_nt;
    UNUSED(mode);
    UNUSED(src_strd);
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
    for(i = nt; i < two_nt; i++)
        acc_dc += pu1_ref[i];

    for(i = (two_nt + 1); i <= three_nt; i++)
        acc_dc += pu1_ref[i];

    dc_val = (acc_dc + nt) >> (log2nt + 1);

    two_dc_val = 2 * dc_val;
    three_dc_val = 3 * dc_val;


    if(nt == 32)
    {
        for(row = 0; row < nt; row++)
            for(col = 0; col < nt; col++)
                pu1_dst[(row * dst_strd) + col] = dc_val;
    }
    else
    {
        /* DC filtering for the first top row and first left column */
        pu1_dst[0] = ((pu1_ref[two_nt - 1] + two_dc_val + pu1_ref[two_nt + 1] + 2)
                        >> 2);

        for(col = 1; col < nt; col++)
            pu1_dst[col] = (pu1_ref[two_nt + 1 + col] + three_dc_val + 2) >> 2;

        for(row = 1; row < nt; row++)
            pu1_dst[row * dst_strd] = (pu1_ref[two_nt - 1 - row] + three_dc_val + 2)
                            >> 2;

        /* Fill the remaining rows with DC value*/
        for(row = 1; row < nt; row++)
            for(col = 1; col < nt; col++)
                pu1_dst[(row * dst_strd) + col] = dc_val;
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


void ihevc_intra_pred_luma_horz(UWORD8 *pu1_ref,
                                WORD32 src_strd,
                                UWORD8 *pu1_dst,
                                WORD32 dst_strd,
                                WORD32 nt,
                                WORD32 mode)
{

    WORD32 row, col;
    WORD32 two_nt;
    WORD16 s2_predpixel;
    UNUSED(mode);
    UNUSED(src_strd);
    two_nt = 2 * nt;

    if(nt == 32)
    {
        for(row = 0; row < nt; row++)
            for(col = 0; col < nt; col++)
                pu1_dst[(row * dst_strd) + col] = pu1_ref[two_nt - 1 - row];
    }
    else
    {
        /*Filtering done for the 1st row */
        for(col = 0; col < nt; col++)
        {
            s2_predpixel = pu1_ref[two_nt - 1]
                            + ((pu1_ref[two_nt + 1 + col] - pu1_ref[two_nt]) >> 1);
            pu1_dst[col] = CLIP_U8(s2_predpixel);
        }

        /* Replication to next rows*/
        for(row = 1; row < nt; row++)
            for(col = 0; col < nt; col++)
                pu1_dst[(row * dst_strd) + col] = pu1_ref[two_nt - 1 - row];
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


void ihevc_intra_pred_luma_ver(UWORD8 *pu1_ref,
                               WORD32 src_strd,
                               UWORD8 *pu1_dst,
                               WORD32 dst_strd,
                               WORD32 nt,
                               WORD32 mode)
{
    WORD32 row, col;
    WORD16 s2_predpixel;
    WORD32 two_nt = 2 * nt;
    UNUSED(mode);
    UNUSED(src_strd);

    if(nt == 32)
    {
        /* Replication to next columns*/
        for(row = 0; row < nt; row++)
            for(col = 0; col < nt; col++)
                pu1_dst[(row * dst_strd) + col] = pu1_ref[two_nt + 1 + col];
    }
    else
    {
        /*Filtering done for the 1st column */
        for(row = 0; row < nt; row++)
        {
            s2_predpixel = pu1_ref[two_nt + 1]
                            + ((pu1_ref[two_nt - 1 - row] - pu1_ref[two_nt]) >> 1);
            pu1_dst[row * dst_strd] = CLIP_U8(s2_predpixel);
        }

        /* Replication to next columns*/
        for(row = 0; row < nt; row++)
            for(col = 1; col < nt; col++)
                pu1_dst[(row * dst_strd) + col] = pu1_ref[two_nt + 1 + col];
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


void ihevc_intra_pred_luma_mode2(UWORD8 *pu1_ref,
                                 WORD32 src_strd,
                                 UWORD8 *pu1_dst,
                                 WORD32 dst_strd,
                                 WORD32 nt,
                                 WORD32 mode)
{
    WORD32 row, col;
    WORD32 two_nt = 2 * nt;
    WORD32 intra_pred_ang = 32;
    WORD32 idx = 0;
    UNUSED(mode);
    UNUSED(src_strd);
    /* For the angle 45, replication is done from the corresponding angle */
    /* intra_pred_ang = tan(angle) in q5 format */
    for(col = 0; col < nt; col++)
    {
        idx = ((col + 1) * intra_pred_ang) >> 5; /* Use idx++ */

        for(row = 0; row < nt; row++)
            pu1_dst[col + (row * dst_strd)] = pu1_ref[two_nt - row - idx - 1];
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


void ihevc_intra_pred_luma_mode_18_34(UWORD8 *pu1_ref,
                                      WORD32 src_strd,
                                      UWORD8 *pu1_dst,
                                      WORD32 dst_strd,
                                      WORD32 nt,
                                      WORD32 mode)
{
    WORD32 row, col;
    WORD32 intra_pred_ang;
    WORD32 idx = 0;
    WORD32 two_nt = 2 * nt;
    UNUSED(src_strd);
    intra_pred_ang = 32;    /*Default value*/

    /* For mode 18, angle is -45degree */
    if(mode == 18)
        intra_pred_ang = -32;
    /* For mode 34, angle is 45degree */
    else if(mode == 34)
        intra_pred_ang = 32;
    /* For the angle 45 and -45, replication is done from the corresponding angle */
    /* No interpolation is done for 45 degree*/
    for(row = 0; row < nt; row++)
    {
        idx = ((row + 1) * intra_pred_ang) >> 5;
#if OPT
        if(mode == 18)
            idx--;
        if(mode == 34)
            idx++;
#endif
        for(col = 0; col < nt; col++)
            pu1_dst[col + (row * dst_strd)] = pu1_ref[two_nt + col + idx + 1];

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


void ihevc_intra_pred_luma_mode_3_to_9(UWORD8 *pu1_ref,
                                       WORD32 src_strd,
                                       UWORD8 *pu1_dst,
                                       WORD32 dst_strd,
                                       WORD32 nt,
                                       WORD32 mode)
{
    WORD32 row, col;
    WORD32 two_nt = 2 * nt;
    WORD32 intra_pred_ang;
    WORD32 idx, ref_main_idx;
    WORD32 pos, fract;
    UNUSED(src_strd);
    /* Intra Pred Angle according to the mode */
    intra_pred_ang = gai4_ihevc_ang_table[mode];

    /* For the angles other then 45 degree, interpolation btw 2 neighboring */
    /* samples dependent on distance to obtain destination sample */

    for(col = 0; col < nt; col++)
    {
        pos = ((col + 1) * intra_pred_ang);
        idx = pos >> 5;
        fract = pos & (31);

        // Do linear filtering
        for(row = 0; row < nt; row++)
        {
            ref_main_idx = two_nt - row - idx - 1;
            pu1_dst[col + (row * dst_strd)] = (((32 - fract)
                            * pu1_ref[ref_main_idx]
                            + fract * pu1_ref[ref_main_idx - 1] + 16) >> 5);
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


void ihevc_intra_pred_luma_mode_11_to_17(UWORD8 *pu1_ref,
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
    WORD32 idx, ref_main_idx, ref_idx;
    WORD32 pos, fract;

    UWORD8 ref_temp[2 * MAX_CU_SIZE + 1];
    UWORD8 *ref_main;
    UNUSED(src_strd);
    inv_ang_sum = 128;
    two_nt    = 2 * nt;

    intra_pred_ang = gai4_ihevc_ang_table[mode];

    inv_ang = gai4_ihevc_inv_ang_table[mode - 11];
    /* Intermediate reference samples for negative angle modes */
    /* This have to be removed during optimization*/
    /* For horizontal modes, (ref main = ref left) (ref side = ref above) */

    ref_main = ref_temp + nt - 1;
    for(k = 0; k < nt + 1; k++)
        ref_temp[k + nt - 1] = pu1_ref[two_nt - k];

    ref_main = ref_temp + nt - 1;
    ref_idx = (nt * intra_pred_ang) >> 5;

    /* SIMD Optimization can be done using look-up table for the loop */
    /* For negative angled derive the main reference samples from side */
    /*  reference samples refer to section 8.4.4.2.6 */
    for(k = -1; k > ref_idx; k--)
    {
        inv_ang_sum += inv_ang;
        ref_main[k] = pu1_ref[two_nt + (inv_ang_sum >> 8)];
    }

    /* For the angles other then 45 degree, interpolation btw 2 neighboring */
    /* samples dependent on distance to obtain destination sample */
    for(col = 0; col < nt; col++)
    {
        pos = ((col + 1) * intra_pred_ang);
        idx = pos >> 5;
        fract = pos & (31);

        // Do linear filtering
        for(row = 0; row < nt; row++)
        {
            ref_main_idx = row + idx + 1;
            pu1_dst[col + (dst_strd * row)] = (UWORD8)(((32 - fract)
                            * ref_main[ref_main_idx]
                            + fract * ref_main[ref_main_idx + 1] + 16) >> 5);

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


void ihevc_intra_pred_luma_mode_19_to_25(UWORD8 *pu1_ref,
                                         WORD32 src_strd,
                                         UWORD8 *pu1_dst,
                                         WORD32 dst_strd,
                                         WORD32 nt,
                                         WORD32 mode)
{

    WORD32 row, col, k;
    WORD32 two_nt, intra_pred_ang, idx;
    WORD32 inv_ang, inv_ang_sum, pos, fract;
    WORD32 ref_main_idx, ref_idx;
    UWORD8 ref_temp[(2 * MAX_CU_SIZE) + 1];
    UWORD8 *ref_main;
    UNUSED(src_strd);
    two_nt = 2 * nt;
    intra_pred_ang = gai4_ihevc_ang_table[mode];
    inv_ang = gai4_ihevc_inv_ang_table[mode - 12];

    /* Intermediate reference samples for negative angle modes */
    /* This have to be removed during optimization*/
    /* For horizontal modes, (ref main = ref above) (ref side = ref left) */
    ref_main = ref_temp + nt - 1;
    for(k = 0; k < (nt + 1); k++)
        ref_temp[k + nt - 1] = pu1_ref[two_nt + k];

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

    for(row = 0; row < nt; row++)
    {
        pos = ((row + 1) * intra_pred_ang);
        idx = pos >> 5;
        fract = pos & (31);

        // Do linear filtering
        for(col = 0; col < nt; col++)
        {
            ref_main_idx = col + idx + 1;
            pu1_dst[(row * dst_strd) + col] = (UWORD8)(((32 - fract)
                            * ref_main[ref_main_idx]
                            + fract * ref_main[ref_main_idx + 1] + 16) >> 5);

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


void ihevc_intra_pred_luma_mode_27_to_33(UWORD8 *pu1_ref,
                                         WORD32 src_strd,
                                         UWORD8 *pu1_dst,
                                         WORD32 dst_strd,
                                         WORD32 nt,
                                         WORD32 mode)
{
    WORD32 row, col;
    WORD32 two_nt, pos, fract;
    WORD32 intra_pred_ang;
    WORD32 idx, ref_main_idx;
    UNUSED(src_strd);
    two_nt = 2 * nt;
    intra_pred_ang = gai4_ihevc_ang_table[mode];

    for(row = 0; row < nt; row++)
    {
        pos = ((row + 1) * intra_pred_ang);
        idx = pos >> 5;
        fract = pos & (31);

        // Do linear filtering
        for(col = 0; col < nt; col++)
        {
            ref_main_idx = two_nt + col + idx + 1;
            pu1_dst[col + (row * dst_strd)] = (((32 - fract)
                            * pu1_ref[ref_main_idx]
                            + fract * pu1_ref[ref_main_idx + 1] + 16) >> 5);
        }

    }

}

