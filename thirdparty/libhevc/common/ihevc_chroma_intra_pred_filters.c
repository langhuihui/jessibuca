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
*  ihevc_chroma_intra_pred_filters.c
*
* @brief
*  Contains function Definition for intra prediction  interpolation filters
*
*
* @author
*  Ittiam
*
* @par List of Functions:
*  ihevc_intra_pred_chroma_planar()
*
*  ihevc_intra_pred_chroma_dc()
*
*  ihevc_intra_pred_chroma_horz()
*
*  ihevc_intra_pred_chroma_ver()
*
*  ihevc_intra_pred_chroma_mode2()
*
*  ihevc_intra_pred_chroma_mode_18_34()
*
*  ihevc_intra_pred_chroma_mode_3_to_9()
*
*  ihevc_intra_pred_chroma_mode_11_to_17()
*
*  ihevc_intra_pred_chroma_mode_19_to_25()
*
*  ihevc_intra_pred_chroma_mode_27_to_33()
*
*  ihevc_intra_pred_chroma_ref_substitution()
*
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
#include "ihevc_mem_fns.h"
#include "ihevc_chroma_intra_pred.h"
#include "ihevc_common_tables.h"


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


/*****************************************************************************/
/* Function Definition                                                      */
/*****************************************************************************/

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


void ihevc_intra_pred_chroma_ref_substitution(UWORD8 *pu1_top_left,
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
            ihevc_memcpy_mul_8(&pu1_dst[(4 * nt) + 2], pu1_top, 2 * nt);
            // U-V interleaved Top-top right samples
        }

        /* Top - Right nbrs  */
        if(0 != tp_right)
        {
            ihevc_memcpy_mul_8(&pu1_dst[(4 * nt) + 2 + 2 * nt], pu1_top + 2 * nt, 2 * nt);
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
                    ihevc_memset_16bit((UWORD16 *)pu1_dst, pu2_dst[0], idx);
                }
                else /* If top, top-right are available */
                {
                    UWORD16 *pu2_dst;
                    /* Idx is changed to copy 1 pixel value for top-left ,if top-left is not available*/
                    idx = (nt * (next - 1)) + 1;
                    pu2_dst = (UWORD16 *)&pu1_dst[2 * idx];
                    ihevc_memset_16bit((UWORD16 *)pu1_dst, pu2_dst[0], idx);
                }
            }

            if(left == 0)
            {
                UWORD16 *pu2_dst = (UWORD16 *)&pu1_dst[(2 * nt) - 2];
                ihevc_memset_16bit((UWORD16 *)&pu1_dst[(2 * nt)], pu2_dst[0], nt);


            }
            if(tp_left == 0)
            {
                pu1_dst[4 * nt] = pu1_dst[(4 * nt) - 2];
                pu1_dst[(4 * nt) + 1] = pu1_dst[(4 * nt) - 1];
            }
            if(top == 0)
            {
                UWORD16 *pu2_dst = (UWORD16 *)&pu1_dst[(4 * nt)];
                ihevc_memset_16bit((UWORD16 *)&pu1_dst[(4 * nt) + 2], pu2_dst[0], nt);


            }
            if(tp_right == 0)
            {
                UWORD16 *pu2_dst = (UWORD16 *)&pu1_dst[(6 * nt)];
                ihevc_memset_16bit((UWORD16 *)&pu1_dst[(6 * nt) + 2], pu2_dst[0], nt);


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
                        ihevc_memset_16bit((UWORD16 *)(pu1_dst + (2 * nbr_id_from_bl)), pu2_dst[0], 4);
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
                        ihevc_memset_16bit((UWORD16 *)(pu1_dst + (2 * nbr_id_from_bl)), pu2_dst[0], 4);
                    }

                }
                nbr_id_from_bl += (nbr_id_from_bl == (T16C_4NT / 2)) ? 1 : 4;
            }
        }
    }
}


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


void ihevc_intra_pred_chroma_planar(UWORD8 *pu1_ref,
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
        for(col = 0; col < (2 * nt); col += 2)
        {
            pu1_dst[row * dst_strd + col] = ((nt - 1 - col / 2)
                            * pu1_ref[2 * (two_nt - 1 - row)]
                            + (col / 2 + 1) * pu1_ref[2 * (three_nt + 1)]
                            + (nt - 1 - row) * pu1_ref[2 * (two_nt + 1) + col]
                            + (row + 1) * pu1_ref[2 * (nt - 1)] + nt) >> (log2nt + 1);

            pu1_dst[row * dst_strd + col + 1] = ((nt - 1 - col / 2)
                            * pu1_ref[2 * (two_nt - 1 - row) + 1]
                            + (col / 2 + 1) * pu1_ref[2 * (three_nt + 1) + 1]
                            + (nt - 1 - row) * pu1_ref[2 * (two_nt + 1) + col + 1]
                            + (row + 1) * pu1_ref[2 * (nt - 1) + 1] + nt) >> (log2nt + 1);
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


void ihevc_intra_pred_chroma_dc(UWORD8 *pu1_ref,
                                WORD32 src_strd,
                                UWORD8 *pu1_dst,
                                WORD32 dst_strd,
                                WORD32 nt,
                                WORD32 mode)
{

    WORD32 acc_dc_u, acc_dc_v;
    WORD32 dc_val_u, dc_val_v;
    WORD32 i;
    WORD32 row, col;
    WORD32 log2nt = 5;
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


    acc_dc_u = 0;
    acc_dc_v = 0;
    /* Calculate DC value for the transform block */
    for(i = (2 * nt); i < (4 * nt); i += 2)
    {
        acc_dc_u += pu1_ref[i];
        acc_dc_v += pu1_ref[i + 1];
    }
    for(i = ((4 * nt) + 2); i < ((6 * nt) + 2); i += 2)
    {
        acc_dc_u += pu1_ref[i];
        acc_dc_v += pu1_ref[i + 1];
    }


    dc_val_u = (acc_dc_u + nt) >> (log2nt + 1);
    dc_val_v = (acc_dc_v + nt) >> (log2nt + 1);


    /* Fill the remaining rows with DC value*/
    for(row = 0; row < nt; row++)
    {
        for(col = 0; col < (2 * nt); col += 2)
        {
            pu1_dst[(row * dst_strd) + col] = dc_val_u;
            pu1_dst[(row * dst_strd) + col + 1] = dc_val_v;
        }
    }

}


/**
*******************************************************************************
*
* @brief
*  Horizontal intraprediction(mode 10) with reference  samples location
* pointed by 'pu1_ref' to the TU block  location pointed by 'pu1_dst'  Refer
* to section 8.4.4.2.6 in the standard (Special case)
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


void ihevc_intra_pred_chroma_horz(UWORD8 *pu1_ref,
                                  WORD32 src_strd,
                                  UWORD8 *pu1_dst,
                                  WORD32 dst_strd,
                                  WORD32 nt,
                                  WORD32 mode)
{

    WORD32 row, col;
    UNUSED(mode);
    UNUSED(src_strd);
    /* Replication to next rows*/
    for(row = 0; row < nt; row++)
    {
        for(col = 0; col < (2 * nt); col += 2)
        {
            pu1_dst[(row * dst_strd) + col] = pu1_ref[(4 * nt) - 2 - 2 * row];
            pu1_dst[(row * dst_strd) + col + 1] = pu1_ref[(4 * nt) - 1 - 2 * row];
        }
    }
}



/**
*******************************************************************************
*
* @brief
*  Horizontal intraprediction with reference neighboring  samples location
* pointed by 'pu1_ref' to the TU block  location pointed by 'pu1_dst'  Refer
* to section 8.4.4.2.6 in the standard (Special case)
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


void ihevc_intra_pred_chroma_ver(UWORD8 *pu1_ref,
                                 WORD32 src_strd,
                                 UWORD8 *pu1_dst,
                                 WORD32 dst_strd,
                                 WORD32 nt,
                                 WORD32 mode)
{
    WORD32 row, col;
    UNUSED(mode);
    UNUSED(src_strd);
    /* Replication to next columns*/
    for(row = 0; row < nt; row++)
    {
        for(col = 0; col < (2 * nt); col += 2)
        {
            pu1_dst[(row * dst_strd) + col] = pu1_ref[(4 * nt) + 2 + col];
            pu1_dst[(row * dst_strd) + col + 1] = pu1_ref[(4 * nt) + 3 + col];
        }
    }
}


/**
*******************************************************************************
*
* @brief
*  Intraprediction for mode 2 (sw angle) with reference  neighboring samples
* location pointed by 'pu1_ref' to the  TU block location pointed by
* 'pu1_dst'  Refer to section 8.4.4.2.6 in the standard
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


void ihevc_intra_pred_chroma_mode2(UWORD8 *pu1_ref,
                                   WORD32 src_strd,
                                   UWORD8 *pu1_dst,
                                   WORD32 dst_strd,
                                   WORD32 nt,
                                   WORD32 mode)
{
    WORD32 row, col;

    WORD32 intra_pred_ang = 32;
    WORD32 idx_u, idx_v;
    UNUSED(src_strd);
    UNUSED(mode);
    /* For the angle 45, replication is done from the corresponding angle */
    /* intra_pred_ang = tan(angle) in q5 format */
    for(col = 0; col < (2 * nt); col += 2)
    {
        idx_u = ((col + 1) * intra_pred_ang) >> 5; /* Use idx++ */
        idx_v = (((col + 1) + 1) * intra_pred_ang) >> 5; /* Use idx++ */
        for(row = 0; row < nt; row++)
        {
            pu1_dst[col + (row * dst_strd)] = pu1_ref[(4 * nt) - 2 * row - idx_u - 3];
            pu1_dst[(col + 1) + (row * dst_strd)] = pu1_ref[(4 * nt) - 2 * row - idx_v - 1];
        }
    }

}


/**
*******************************************************************************
*
* @brief
*  Intraprediction for mode 34 (ne angle) and  mode 18 (nw angle) with
* reference  neighboring samples location pointed by 'pu1_ref' to the  TU
* block location pointed by 'pu1_dst'
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


void ihevc_intra_pred_chroma_mode_18_34(UWORD8 *pu1_ref,
                                        WORD32 src_strd,
                                        UWORD8 *pu1_dst,
                                        WORD32 dst_strd,
                                        WORD32 nt,
                                        WORD32 mode)
{
    WORD32 row, col;
    WORD32 intra_pred_ang;
    WORD32 idx = 0;
    UNUSED(src_strd);
    intra_pred_ang = 32; /*Default value*/
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

        for(col = 0; col < (2 * nt); col += 2)
        {
            pu1_dst[col + (row * dst_strd)] = pu1_ref[(4 * nt) + col + 2 * idx + 2];
            pu1_dst[(col + 1) + (row * dst_strd)] = pu1_ref[(4 * nt) + (col + 1) + 2 * idx + 2];
        }

    }

}


/**
*******************************************************************************
*
* @brief
*  Intraprediction for mode 3 to 9  (positive angle, horizontal mode ) with
* reference  neighboring samples location pointed by 'pu1_ref' to the  TU
* block location pointed by 'pu1_dst'
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


void ihevc_intra_pred_chroma_mode_3_to_9(UWORD8 *pu1_ref,
                                         WORD32 src_strd,
                                         UWORD8 *pu1_dst,
                                         WORD32 dst_strd,
                                         WORD32 nt,
                                         WORD32 mode)
{
    WORD32 row, col;

    WORD32 intra_pred_ang;
    WORD32 idx_u, ref_main_idx_u;
    WORD32 idx_v, ref_main_idx_v;
    WORD32 pos_u, fract_u;
    WORD32 pos_v, fract_v;
    UNUSED(src_strd);
    /* Intra Pred Angle according to the mode */
    intra_pred_ang = gai4_ihevc_ang_table[mode];

    /* For the angles other then 45 degree, interpolation btw 2 neighboring */
    /* samples dependent on distance to obtain destination sample */

    for(col = 0; col < (2 * nt); col += 2)
    {
        pos_u = ((col / 2 + 1) * intra_pred_ang);
        pos_v = ((col / 2 + 1) * intra_pred_ang);

        idx_u = pos_u >> 5;
        fract_u = pos_u & (31);

        idx_v = pos_v >> 5;
        fract_v = pos_v & (31);
        // Do linear filtering
        for(row = 0; row < nt; row++)
        {
            ref_main_idx_u = (4 * nt) - 2 * row - 2 * idx_u - 2;
            ref_main_idx_v = (4 * nt) - 2 * row - 2 * idx_v - 1;

            pu1_dst[col + (row * dst_strd)] = (((32 - fract_u)
                            * pu1_ref[ref_main_idx_u]
                            + fract_u * pu1_ref[ref_main_idx_u - 2] + 16) >> 5);

            pu1_dst[(col + 1) + (row * dst_strd)] = (((32 - fract_v)
                            * pu1_ref[ref_main_idx_v]
                            + fract_v * pu1_ref[ref_main_idx_v - 2] + 16) >> 5);
        }

    }

}


/**
*******************************************************************************
*
* @brief
*  Intraprediction for mode 11 to 17  (negative angle, horizontal mode )
* with reference  neighboring samples location pointed by 'pu1_ref' to the
* TU block location pointed by 'pu1_dst'
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


void ihevc_intra_pred_chroma_mode_11_to_17(UWORD8 *pu1_ref,
                                           WORD32 src_strd,
                                           UWORD8 *pu1_dst,
                                           WORD32 dst_strd,
                                           WORD32 nt,
                                           WORD32 mode)
{
    /* This function and ihevc_intra_pred_CHROMA_mode_19_to_25 are same except*/
    /* for ref main & side samples assignment,can be combined for */
    /* optimzation*/

    WORD32 row, col, k;
    WORD32 intra_pred_ang, inv_ang, inv_ang_sum;
    WORD32 idx_u, idx_v, ref_main_idx_u, ref_main_idx_v, ref_idx;
    WORD32 pos_u, pos_v, fract_u, fract_v;

    UWORD8 ref_temp[2 * MAX_CU_SIZE + 2];
    UWORD8 *ref_main;
    UNUSED(src_strd);
    inv_ang_sum = 128;

    intra_pred_ang = gai4_ihevc_ang_table[mode];

    inv_ang = gai4_ihevc_inv_ang_table[mode - 11];
    /* Intermediate reference samples for negative angle modes */
    /* This have to be removed during optimization*/

    /* For horizontal modes, (ref main = ref left) (ref side = ref above) */


    ref_main = ref_temp + 2 * nt;
    for(k = 0; k < (2 * (nt + 1)); k += 2)
    {
        ref_temp[k + (2 * (nt - 1))] = pu1_ref[(4 * nt) - k];
        ref_temp[k + 1 + (2 * (nt - 1))] = pu1_ref[(4 * nt) - k + 1];
    }

    ref_main = ref_temp + (2 * (nt - 1));
    ref_idx = (nt * intra_pred_ang) >> 5;

    /* SIMD Optimization can be done using look-up table for the loop */
    /* For negative angled derive the main reference samples from side */
    /*  reference samples refer to section 8.4.4.2.6 */
    for(k = -2; k > (2 * ref_idx); k -= 2)
    {
        inv_ang_sum += inv_ang;
        ref_main[k] = pu1_ref[(4 * nt) + ((inv_ang_sum >> 8) << 1)];
        ref_main[k + 1] = pu1_ref[((4 * nt) + 1) + ((inv_ang_sum >> 8) << 1)];
    }

    /* For the angles other then 45 degree, interpolation btw 2 neighboring */
    /* samples dependent on distance to obtain destination sample */
    for(col = 0; col < (2 * nt); col += 2)
    {
        pos_u = ((col / 2 + 1) * intra_pred_ang);
        pos_v = ((col / 2 + 1) * intra_pred_ang);
        idx_u = pos_u >> 5;
        idx_v = pos_v >> 5;
        fract_u = pos_u & (31);
        fract_v = pos_v & (31);

        // Do linear filtering
        for(row = 0; row < nt; row++)
        {
            ref_main_idx_u = 2 * (row + idx_u + 1);
            ref_main_idx_v = 2 * (row + idx_v + 1) + 1;

            pu1_dst[col + (dst_strd * row)] = (UWORD8)(((32 - fract_u)
                            * ref_main[ref_main_idx_u]
                            + fract_u * ref_main[ref_main_idx_u + 2] + 16) >> 5);
            pu1_dst[(col + 1) + (dst_strd * row)] = (UWORD8)(((32 - fract_v)
                            * ref_main[ref_main_idx_v]
                            + fract_v * ref_main[ref_main_idx_v + 2] + 16) >> 5);

        }

    }

}



/**
*******************************************************************************
*
* @brief
*  Intraprediction for mode 19 to 25  (negative angle, vertical mode ) with
* reference  neighboring samples location pointed by 'pu1_ref' to the  TU
* block location pointed by 'pu1_dst'
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


void ihevc_intra_pred_chroma_mode_19_to_25(UWORD8 *pu1_ref,
                                           WORD32 src_strd,
                                           UWORD8 *pu1_dst,
                                           WORD32 dst_strd,
                                           WORD32 nt,
                                           WORD32 mode)
{

    WORD32 row, col, k;
    WORD32 intra_pred_ang, idx;
    WORD32 inv_ang, inv_ang_sum, pos, fract;
    WORD32 ref_main_idx_u, ref_main_idx_v, ref_idx;
    UWORD8 ref_temp[(2 * MAX_CU_SIZE) + 2];
    UWORD8 *ref_main;
    UNUSED(src_strd);


    intra_pred_ang = gai4_ihevc_ang_table_chroma[mode];
    inv_ang = gai4_ihevc_inv_ang_table_chroma[mode - 12];

    /* Intermediate reference samples for negative angle modes */
    /* This have to be removed during optimization*/
    /* For horizontal modes, (ref main = ref above) (ref side = ref left) */
    ref_main = ref_temp + 2 * nt;
    for(k = 0; k < (2 * (nt + 1)); k += 2)
    {
        ref_temp[k + (2 * (nt - 1))] = pu1_ref[(4 * nt) + k];
        ref_temp[k + 1 + (2 * (nt - 1))] = pu1_ref[(4 * nt) + k + 1];
    }


    ref_idx = (nt * intra_pred_ang) >> 5;
    inv_ang_sum = 128;
    ref_main = ref_temp + (2 * (nt - 1));
    /* SIMD Optimization can be done using look-up table for the loop */
    /* For negative angled derive the main reference samples from side */
    /*  reference samples refer to section 8.4.4.2.6 */
    for(k = -2; k > (2 * ref_idx); k -= 2)
    {
        inv_ang_sum += inv_ang;
        ref_main[k] = pu1_ref[(4 * nt) - (inv_ang_sum >> 8) * 2];
        ref_main[k + 1] = pu1_ref[((4 * nt) + 1) - (inv_ang_sum >> 8) * 2];
    }

    for(row = 0; row < nt; row++)
    {
        pos = ((row + 1) * intra_pred_ang);
        idx = pos >> 5;
        fract = pos & (31);

        // Do linear filtering
        for(col = 0; col < (2 * nt); col += 2)
        {
            ref_main_idx_u = col + 2 * idx + 2;
            ref_main_idx_v = (col + 1) + 2 * idx + 2;
            pu1_dst[(row * dst_strd) + col] = (UWORD8)(((32 - fract)
                            * ref_main[ref_main_idx_u]
                            + fract * ref_main[ref_main_idx_u + 2] + 16) >> 5);
            pu1_dst[(row * dst_strd) + (col + 1)] = (UWORD8)(((32 - fract)
                            * ref_main[ref_main_idx_v]
                            + fract * ref_main[ref_main_idx_v + 2] + 16) >> 5);

        }

    }

}



/**
*******************************************************************************
*
* @brief
*  Intraprediction for mode 27 to 33  (positive angle, vertical mode ) with
* reference  neighboring samples location pointed by 'pu1_ref' to the  TU
* block location pointed by 'pu1_dst'
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


void ihevc_intra_pred_chroma_mode_27_to_33(UWORD8 *pu1_ref,
                                           WORD32 src_strd,
                                           UWORD8 *pu1_dst,
                                           WORD32 dst_strd,
                                           WORD32 nt,
                                           WORD32 mode)
{
    WORD32 row, col;
    WORD32 pos, fract;
    WORD32 intra_pred_ang;
    WORD32 idx, ref_main_idx_u, ref_main_idx_v;
    UNUSED(src_strd);


    intra_pred_ang = gai4_ihevc_ang_table_chroma[mode];

    for(row = 0; row < nt; row++)
    {
        pos = ((row + 1) * intra_pred_ang);
        idx = pos >> 5;
        fract = pos & (31);


        // Do linear filtering
        for(col = 0; col < (2 * nt); col += 2)
        {
            ref_main_idx_u = (4 * nt) + col + 2 * idx + 2;
            ref_main_idx_v = (4 * nt) + (col + 1) + 2 * idx + 2;
            pu1_dst[col + (row * dst_strd)] = (((32 - fract)
                            * pu1_ref[ref_main_idx_u]
                            + fract * pu1_ref[ref_main_idx_u + 2] + 16) >> 5);
            pu1_dst[(col + 1) + (row * dst_strd)] = (((32 - fract)
                            * pu1_ref[ref_main_idx_v]
                            + fract * pu1_ref[ref_main_idx_v + 2] + 16) >> 5);

        }
    }

}

