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
*  ihevcd_intra_ref_substitution.c
*
* @brief
*  Contains ref substitution functions
*
* @author
*  Naveen
*
* @par List of Functions:
* @remarks
*  None
*
*******************************************************************************
*/
/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ihevc_typedefs.h"
#include "ihevc_platform_macros.h"
#include "ihevc_intra_pred.h"
#include "ihevc_mem_fns.h"
#include "ihevc_chroma_intra_pred.h"
#include "ihevc_common_tables.h"
#include "ihevc_defs.h"
#include "ihevc_mem_fns.h"
#include "ihevc_macros.h"

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
#define GET_BITS(y,x) ((y) & (1 << x)) && (1 << x)
/**
*******************************************************************************
*
* @brief
*  Reference substitution process for samples unavailable  for prediction
* Refer to section 8.4.4.2.2
*
* @par Description:
*
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

void ihevc_intra_pred_chroma_ref_substitution_a9q(UWORD8 *pu1_top_left,
                                                  UWORD8 *pu1_top,
                                                  UWORD8 *pu1_left,
                                                  WORD32 src_strd,
                                                  WORD32 nt,
                                                  WORD32 nbr_flags,
                                                  UWORD8 *pu1_dst,
                                                  WORD32 dst_strd)
{
    UWORD8 pu1_ref_u, pu1_ref_v;
    WORD32 dc_val, i, j;
    WORD32 total_samples = (4 * nt) + 1;
    WORD32 get_bits;
    WORD32 next;
    WORD32 bot_left, left, top, tp_right, tp_left;
    WORD32 idx, nbr_id_from_bl, frwd_nbr_flag;
    WORD32 a_nbr_flag[5];
    UNUSED(dst_strd);
    /* Neighbor Flag Structure*/
    /* WORD32 nbr_flags MSB-->LSB   TOP LEFT | TOP-RIGHT |  TOP   | LEFT    | BOTTOM LEFT*/
    /*                              (1 bit)     (4 bits)  (4 bits) (4 bits)  (4 bits)  */

    if(nbr_flags == 0)
    {
/* If no neighbor flags are present, fill the neighbor samples with DC value */
        /*dc_val = 1 << (BIT_DEPTH - 1);*/
        dc_val = 1 << (8 - 1);
        for(i = 0; i < (2 * total_samples); i++)
        {
            pu1_dst[i] = dc_val;
        }
    }
    else
    {
        /* Else fill the corresponding samples */

        /* Check for the neighbors availibility */
        tp_left     = (nbr_flags & 0x10000);
        tp_right    = (nbr_flags & 0x0f000);
        top         = (nbr_flags & 0x00f00);
        left        = (nbr_flags & 0x000f0);
        bot_left    = (nbr_flags & 0x0000f);

        /* Fill nbrs depending on avalibility */
        /* Top -Left nbrs  */
        if(0 != tp_left)
        {
            pu1_dst[(4 * nt)] = *pu1_top_left; // U top-left sample
            pu1_dst[(4 * nt) + 1] = *(pu1_top_left + 1); // V top-left sample
        }
        /* Left nbrs  */
        if(0 != left)
        {
            for(i = 0, j = 0; i < (2 * nt); i += 2)
            {
                pu1_dst[(4 * nt) - 2 - i] = pu1_left[j * src_strd]; // U left samples
                pu1_dst[(4 * nt) - 1 - i] = pu1_left[(j * src_strd) + 1]; // V left samples
                j++;
            }
        }
        /* Bottom - Left nbrs  */
        if(0 != bot_left)
        {
            for(i = (2 * nt), j = nt; i < (4 * nt); i += 2)
            {
                pu1_dst[(4 * nt) - 2 - i] = pu1_left[j * src_strd]; // U left samples
                pu1_dst[(4 * nt) - 1 - i] = pu1_left[(j * src_strd) + 1]; // V left samples
                j++;
            }
        }
        /* Top nbrs  */
        if(0 != top)
        {
            ihevc_memcpy_mul_8_a9q(&pu1_dst[(4 * nt) + 2], pu1_top, 2 * nt);
            // U-V interleaved Top-top right samples
        }

        /* Top - Right nbrs  */
        if(0 != tp_right)
        {
            ihevc_memcpy_mul_8_a9q(&pu1_dst[(4 * nt) + 2 + 2 * nt], pu1_top + 2 * nt, 2 * nt);
            // U-V interleaved Top-top right samples
        }

        if(nt == 4)
        {
            /* 1 bit extraction for all the neighboring blocks */
            tp_left = (nbr_flags & 0x10000) >> 16;
            bot_left = (nbr_flags & 0x8) >> 3;
            left = (nbr_flags & 0x80) >> 7;
            top = (nbr_flags & 0x100) >> 8;
            tp_right = (nbr_flags & 0x1000) >> 12;

            next = 1;
            a_nbr_flag[0] = bot_left;
            a_nbr_flag[1] = left;
            a_nbr_flag[2] = tp_left;
            a_nbr_flag[3] = top;
            a_nbr_flag[4] = tp_right;

            /* If bottom -left is not available, reverse substitution process*/
            if(bot_left == 0)
            {
                /* Check for the 1st available sample from bottom-left*/
                while(!a_nbr_flag[next])
                    next++;

                /* If Left, top-left are available*/
                if(next <= 2)
                {
                    UWORD16 *pu2_dst;
                    idx = (nt * next);
                    pu2_dst = (UWORD16 *)&pu1_dst[2 * idx];
                    ihevc_memset_16bit_a9q((UWORD16 *)pu1_dst, pu2_dst[0], idx);
                }
                else /* If top, top-right are available */
                {
                    UWORD16 *pu2_dst;
                    /* Idx is changed to copy 1 pixel value for top-left ,if top-left is not available*/
                    idx = (nt * (next - 1)) + 1;
                    pu2_dst = (UWORD16 *)&pu1_dst[2 * idx];
                    ihevc_memset_16bit_a9q((UWORD16 *)pu1_dst, pu2_dst[0], idx);
                }
            }

            if(left == 0)
            {
                UWORD16 *pu2_dst = (UWORD16 *)&pu1_dst[(2 * nt) - 2];
                ihevc_memset_16bit_a9q((UWORD16 *)&pu1_dst[(2 * nt)], pu2_dst[0], nt);


            }
            if(tp_left == 0)
            {
                pu1_dst[4 * nt] = pu1_dst[(4 * nt) - 2];
                pu1_dst[(4 * nt) + 1] = pu1_dst[(4 * nt) - 1];
            }
            if(top == 0)
            {
                UWORD16 *pu2_dst = (UWORD16 *)&pu1_dst[(4 * nt)];
                ihevc_memset_16bit_a9q((UWORD16 *)&pu1_dst[(4 * nt) + 2], pu2_dst[0], nt);


            }
            if(tp_right == 0)
            {
                UWORD16 *pu2_dst = (UWORD16 *)&pu1_dst[(6 * nt)];
                ihevc_memset_16bit_a9q((UWORD16 *)&pu1_dst[(6 * nt) + 2], pu2_dst[0], nt);


            }
        }
        else if(nt == 8)
        {
            WORD32 nbr_flags_temp = 0;
            nbr_flags_temp = ((nbr_flags & 0xC) >> 2) + ((nbr_flags & 0xC0) >> 4)
                            + ((nbr_flags & 0x300) >> 4)
                            + ((nbr_flags & 0x3000) >> 6)
                            + ((nbr_flags & 0x10000) >> 8);

            /* compute trailing zeors based on nbr_flag for substitution process of below left see section .*/
            /* as each bit in nbr flags corresponds to 8 pels for bot_left, left, top and topright but 1 pel for topleft */
            {
                nbr_id_from_bl = look_up_trailing_zeros(nbr_flags_temp & 0XF) * 4; /* for bottom left and left */
                if(nbr_id_from_bl == 32)
                    nbr_id_from_bl = 16;
                if(nbr_id_from_bl == 16)
                {
                    /* for top left : 1 pel per nbr bit */
                    if(!((nbr_flags_temp >> 8) & 0x1))
                    {
                        nbr_id_from_bl++;
                        nbr_id_from_bl += look_up_trailing_zeros((nbr_flags_temp >> 4) & 0xF) * 4; /* top and top right;  8 pels per nbr bit */

                    }
                }
                /* Reverse Substitution Process*/
                if(nbr_id_from_bl)
                {
                    /* Replicate the bottom-left and subsequent unavailable pixels with the 1st available pixel above */
                    pu1_ref_u = pu1_dst[2 * nbr_id_from_bl];
                    pu1_ref_v = pu1_dst[(2 * nbr_id_from_bl) + 1];
                    for(i = 2 * (nbr_id_from_bl - 1); i >= 0; i -= 2)
                    {
                        pu1_dst[i] = pu1_ref_u;
                        pu1_dst[i + 1] = pu1_ref_v;
                    }
                }
            }

            /* for the loop of 4*Nt+1 pixels (excluding pixels computed from reverse substitution) */
            while(nbr_id_from_bl < ((T8C_4NT)+1))
            {
                /* To Obtain the next unavailable idx flag after reverse neighbor substitution  */
                /* Divide by 8 to obtain the original index */
                frwd_nbr_flag = (nbr_id_from_bl >> 2); /*+ (nbr_id_from_bl & 0x1);*/

                /* The Top-left flag is at the last bit location of nbr_flags*/
                if(nbr_id_from_bl == (T8C_4NT / 2))
                {
                    get_bits = GET_BIT(nbr_flags_temp, 8);

                    /* only pel substitution for TL */
                    if(!get_bits)
                    {
                        pu1_dst[2 * nbr_id_from_bl] = pu1_dst[(2 * nbr_id_from_bl) - 2];
                        pu1_dst[(2 * nbr_id_from_bl) + 1] = pu1_dst[(2 * nbr_id_from_bl) - 1];
                    }
                }
                else
                {
                    get_bits = GET_BIT(nbr_flags_temp, frwd_nbr_flag);
                    if(!get_bits)
                    {
                        UWORD16 *pu2_dst;
                        /* 8 pel substitution (other than TL) */
                        pu2_dst = (UWORD16 *)&pu1_dst[(2 * nbr_id_from_bl) - 2];
                        ihevc_memset_16bit_a9q((UWORD16 *)(pu1_dst + (2 * nbr_id_from_bl)), pu2_dst[0], 4);
                    }

                }
                nbr_id_from_bl += (nbr_id_from_bl == (T8C_4NT / 2)) ? 1 : 4;
            }

        }
        else if(nt == 16)
        {
            /* compute trailing ones based on mbr_flag for substitution process of below left see section .*/
            /* as each bit in nbr flags corresponds to 4 pels for bot_left, left, top and topright but 1 pel for topleft */
            {
                nbr_id_from_bl = look_up_trailing_zeros((nbr_flags & 0XFF)) * 4; /* for bottom left and left */

                if(nbr_id_from_bl == 32)
                {
                    /* for top left : 1 pel per nbr bit */
                    if(!((nbr_flags >> 16) & 0x1))
                    {
                        /* top left not available */
                        nbr_id_from_bl++;
                        /* top and top right;  4 pels per nbr bit */
                        nbr_id_from_bl += look_up_trailing_zeros((nbr_flags >> 8) & 0xFF) * 4;
                    }
                }
                /* Reverse Substitution Process*/
                if(nbr_id_from_bl)
                {
                    /* Replicate the bottom-left and subsequent unavailable pixels with the 1st available pixel above */
                    pu1_ref_u = pu1_dst[2 * nbr_id_from_bl];
                    pu1_ref_v = pu1_dst[2 * nbr_id_from_bl + 1];
                    for(i = (2 * (nbr_id_from_bl - 1)); i >= 0; i -= 2)
                    {
                        pu1_dst[i] = pu1_ref_u;
                        pu1_dst[i + 1] = pu1_ref_v;
                    }
                }
            }

            /* for the loop of 4*Nt+1 pixels (excluding pixels computed from reverse substitution) */
            while(nbr_id_from_bl < ((T16C_4NT)+1))
            {
                /* To Obtain the next unavailable idx flag after reverse neighbor substitution  */
                /* Devide by 4 to obtain the original index */
                frwd_nbr_flag = (nbr_id_from_bl >> 2); /*+ (nbr_id_from_bl & 0x1);*/

                /* The Top-left flag is at the last bit location of nbr_flags*/
                if(nbr_id_from_bl == (T16C_4NT / 2))
                {
                    get_bits = GET_BIT(nbr_flags, 16);
                    /* only pel substitution for TL */
                    if(!get_bits)
                    {
                        pu1_dst[2 * nbr_id_from_bl] = pu1_dst[(2 * nbr_id_from_bl) - 2];
                        pu1_dst[(2 * nbr_id_from_bl) + 1] = pu1_dst[(2 * nbr_id_from_bl) - 1];
                    }
                }
                else
                {
                    get_bits = GET_BIT(nbr_flags, frwd_nbr_flag);
                    if(!get_bits)
                    {
                        UWORD16 *pu2_dst;
                        /* 4 pel substitution (other than TL) */
                        pu2_dst = (UWORD16 *)&pu1_dst[(2 * nbr_id_from_bl) - 2];
                        ihevc_memset_16bit_a9q((UWORD16 *)(pu1_dst + (2 * nbr_id_from_bl)), pu2_dst[0], 4);
                    }

                }
                nbr_id_from_bl += (nbr_id_from_bl == (T16C_4NT / 2)) ? 1 : 4;
            }
        }
    }
}


void ihevc_intra_pred_luma_ref_substitution_a9q(UWORD8 *pu1_top_left,
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
                ihevc_memset_a9q(&pu1_dst[two_nt - 1 - (nt - 1)], 0, nt);
            }


            if(bot_left)
            {
                for(i = nt; i < two_nt; i++)
                    pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
            }
            else
            {
                ihevc_memset_a9q(&pu1_dst[two_nt - 1 - (two_nt - 1)], 0, nt);
            }


            if(top)
            {
                ihevc_memcpy_a9q(&pu1_dst[two_nt + 1], pu1_top, nt);
            }
            else
            {
                ihevc_memset_a9q(&pu1_dst[two_nt + 1], 0, nt);
            }

            if(tp_right)
            {
                ihevc_memcpy_a9q(&pu1_dst[two_nt + 1 + nt], pu1_top + nt, nt);
            }
            else
            {
                ihevc_memset_a9q(&pu1_dst[two_nt + 1 + nt], 0, nt);
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
                ihevc_memset_a9q(&pu1_dst[nt], pu1_dst[nt - 1], nt);

            }
            /* If top-left is Unavailable, copy the last left value */
            if(tp_left == 0)
                pu1_dst[two_nt] = pu1_dst[two_nt - 1];
            /* If top is Unavailable, copy the last top-left value */
            if(top == 0)
            {
                ihevc_memset_a9q(&pu1_dst[two_nt + 1], pu1_dst[two_nt], nt);
            }
            /* If to right is Unavailable, copy the last top value */
            if(tp_right == 0)
            {
                ihevc_memset_a9q(&pu1_dst[three_nt + 1], pu1_dst[three_nt], nt);

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
                ihevc_memset_mul_8_a9q(&pu1_dst[two_nt - 1 - (nt - 1)], 0, nt);
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
                    ihevc_memset_mul_8_a9q(&pu1_dst[nt - 8], 0, 8);
                }

                if(nbr_flags & 0x4)
                {
                    for(i = (nt + 8); i < two_nt; i++)
                        pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
                }
                else
                {
                    ihevc_memset_mul_8_a9q(&pu1_dst[0], 0, 8);
                }
            }


            if(nbr_flags & 0x300)
            {
                ihevc_memcpy_mul_8_a9q(&pu1_dst[two_nt + 1], pu1_top, nt);
            }
            else
            {
                ihevc_memset_mul_8_a9q(&pu1_dst[two_nt + 1], 0, nt);
            }

            if(nbr_flags & 0x3000)
            {
                ihevc_memcpy_mul_8_a9q(&pu1_dst[two_nt + 1 + nt], pu1_top + nt, nt);
            }
            else
            {
                ihevc_memset_mul_8_a9q(&pu1_dst[two_nt + 1 + nt], 0, nt);
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
                        ihevc_memset_mul_8_a9q(pu1_dst + nbr_id_from_bl, pu1_ref, 8);


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
                ihevc_memset_mul_8_a9q(&pu1_dst[two_nt - 1 - (nt - 1)], 0, nt);
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
                    ihevc_memset_mul_8_a9q(&pu1_dst[24], 0, 8);
                }

                if(nbr_flags & 0x4)
                {
                    for(i = (nt + 8); i < (nt + 16); i++)
                        pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
                }
                else
                {
                    ihevc_memset_mul_8_a9q(&pu1_dst[16], 0, 8);
                }

                if(nbr_flags & 0x2)
                {
                    for(i = (nt + 16); i < (nt + 24); i++)
                        pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
                }
                else
                {
                    ihevc_memset_mul_8_a9q(&pu1_dst[8], 0, 8);
                }

                if(nbr_flags & 0x1)
                {
                    for(i = (nt + 24); i < (two_nt); i++)
                        pu1_dst[two_nt - 1 - i] = pu1_left[i * src_strd];
                }
                else
                {
                    ihevc_memset_mul_8_a9q(&pu1_dst[0], 0, 8);
                }
            }

            if(nbr_flags & 0xF00)
            {
                ihevc_memcpy_mul_8_a9q(&pu1_dst[two_nt + 1], pu1_top, nt);
            }
            else
            {
                ihevc_memset_mul_8_a9q(&pu1_dst[two_nt + 1], 0, nt);
            }

            if(nbr_flags & 0xF000)
            {
                ihevc_memcpy_mul_8_a9q(&pu1_dst[two_nt + 1 + nt], pu1_top + nt, nt);
            }
            else
            {
                ihevc_memset_mul_8_a9q(&pu1_dst[two_nt + 1 + nt], 0, nt);
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
                        ihevc_memset_mul_8_a9q(&pu1_dst[nbr_id_from_bl], pu1_ref, 8);

                    }

                }
                nbr_id_from_bl += (nbr_id_from_bl == (T32_4NT / 2)) ? 1 : 8;
            }
        }

    }
}
