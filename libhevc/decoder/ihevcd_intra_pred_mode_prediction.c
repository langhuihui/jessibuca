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
 *  ihevcd_intra_pred_mode_prediction.c.c
 *
 * @brief
 *  Contains functions for intra pred mode prediction
 *
 * @author
 *  Ittiam
 *
 * @par List of Functions:
 * - ihevcd_intra_pred_mode_prediction()
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ihevc_typedefs.h"
#include "iv.h"
#include "ivd.h"
#include "ihevcd_cxa.h"

#include "ihevc_defs.h"
#include "ihevc_debug.h"
#include "ihevc_structs.h"
#include "ihevc_macros.h"
#include "ihevc_mem_fns.h"
#include "ihevc_platform_macros.h"

#include "ihevcd_defs.h"
#include "ihevc_cabac_tables.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"

#include "ihevcd_bitstream.h"


/*****************************************************************************/
/* Function Prototypes                                                       */
/*****************************************************************************/

/*****************************************************************************/
/* Availability check is not done inside the function                        */
/* Whenever the top and left are not available, it is assumed that Intra DC  */
/*                  mode will initialized in place of non available          */
/*                  neighbors                                                */
/*****************************************************************************/

/**
*******************************************************************************
*
* @brief Computes intra prediction mode for a CU
*
* @par   Description
* Computes intra prediction mode for a CU
*
* @param[in,out] ps_cu
* Codic unit context
*
* @param[in] ps_parse
* parse context
*
* @param[in] ps_codec
* codec context
*
* @param[in] log2_cb_size
* log of cb size base 2
*
* @returns none
*
* @remarks
* Availability check is moved to CTB level. If the neighbors are
*  not available or if the pred mode of neighbor is not MODE_INTRA,
*  INTRA_DC mode will be updated in top and left buffers.
*******************************************************************************
*/
void ihevcd_intra_pred_mode_prediction(codec_t *ps_codec,
                                       WORD32 log2_cb_size,
                                       WORD32 x0,
                                       WORD32 y0)
{
    WORD32 i, j, num_pred_blocks;
    WORD32 available_l, available_t;
    WORD32 cand_intra_pred_mode_l, cand_intra_pred_mode_t;
    WORD32 cand_mode_list[3];
    WORD32 cb_size, block_offset_in_min_pu;
    UWORD8 *pu1_luma_intra_pred_mode_top;
    UWORD8 *pu1_luma_intra_pred_mode_left;

    parse_ctxt_t *ps_parse = &ps_codec->s_parse;
    parse_cu_t *ps_cu = &ps_codec->s_parse.s_cu;
    sps_t *ps_sps = ps_parse->ps_sps;


    available_t = 1;
    available_l = 1;
    /* i4_pos_x and i4_pos_y are in minCu units (8x8), convert them to 4x4 units by multiplying by 2 */
    pu1_luma_intra_pred_mode_top = ps_parse->pu1_luma_intra_pred_mode_top
                    + (ps_cu->i4_pos_x * 2);

    pu1_luma_intra_pred_mode_left = ps_parse->pu1_luma_intra_pred_mode_left
                    + (ps_cu->i4_pos_y * 2);

/*
    if(0 == ps_cu->i4_pos_y)
    {
        memset(pu1_luma_intra_pred_mode_top, INTRA_DC, 16);
    }

    if(0 == ps_cu->i4_pos_x)
    {
        memset(pu1_luma_intra_pred_mode_left, INTRA_DC, 16);
    }
*/
    if(ps_cu->i4_pos_y)
    {
        UWORD8 *pu1_pic_intra_flag = ps_codec->s_parse.pu1_pic_intra_flag;
        WORD32 top_intra_flag;

        WORD32 numbytes_row =  (ps_sps->i2_pic_width_in_luma_samples + 63) / 64;
        pu1_pic_intra_flag += ((y0 - 8) / 8) * numbytes_row;
        pu1_pic_intra_flag += (x0 / 64);
        top_intra_flag = *pu1_pic_intra_flag;
        top_intra_flag &= (1 << ((x0 / 8) % 8));

        if(0 == top_intra_flag)
        {
            available_t = 0;
        }
    }
    else
        available_t = 0;


    if((0 == ps_cu->i4_pos_x) && (((0 == ps_codec->s_parse.i4_ctb_slice_x) && (0 == ps_codec->s_parse.i4_ctb_slice_y)) ||
                                  (0 == ps_codec->s_parse.i4_ctb_tile_x)))
    {
        available_l = 0;
    }

    if(available_l)
    {
        UWORD8 *pu1_pic_intra_flag = ps_codec->s_parse.pu1_pic_intra_flag;
        WORD32 left_intra_flag;
        WORD32 numbytes_row =  (ps_sps->i2_pic_width_in_luma_samples + 63) / 64;
        pu1_pic_intra_flag += (y0 / 8) * numbytes_row;
        pu1_pic_intra_flag += ((x0 - 8) / 64);
        left_intra_flag = *pu1_pic_intra_flag;
        left_intra_flag &= (1 << (((x0 - 8) / 8) % 8));

        if(0 == left_intra_flag)
        {
            available_l = 0;
        }
    }

    cb_size = (1 << log2_cb_size);

    block_offset_in_min_pu = (cb_size / 2) / MIN_PU_SIZE;

    num_pred_blocks = (ps_cu->i4_part_mode == PART_NxN) ? 2 : 1;

    for(i = 0; i < num_pred_blocks; i++)
    {
        WORD32 available_l_tmp;
        available_l_tmp = available_l;
        for(j = 0; j < num_pred_blocks; j++)
        {
            /* Computing Candidate intra pred mode left */
            {
                WORD32 block_offset;

                block_offset = i * block_offset_in_min_pu;
                cand_intra_pred_mode_l = INTRA_DC;
                if(available_l_tmp)
                {
                    cand_intra_pred_mode_l =
                                    pu1_luma_intra_pred_mode_left[block_offset];
                }

            }

            {
                WORD32 block_offset;
                block_offset = j * block_offset_in_min_pu;
                cand_intra_pred_mode_t = INTRA_DC;
                if(available_t)
                {
                    cand_intra_pred_mode_t =
                                    pu1_luma_intra_pred_mode_top[block_offset];
                }
            }

            /* Computing Candidate mode list */
            if(cand_intra_pred_mode_l == cand_intra_pred_mode_t)
            {
                if(cand_intra_pred_mode_l < 2)
                {
                    cand_mode_list[0] = INTRA_PLANAR;
                    cand_mode_list[1] = INTRA_DC;
                    cand_mode_list[2] = INTRA_ANGULAR(26); /* angular 26 = Vertical */
                }
                else
                {
                    cand_mode_list[0] = cand_intra_pred_mode_l;
                    cand_mode_list[1] = 2
                                    + ((cand_intra_pred_mode_l + 29) % 32);
                    cand_mode_list[2] = 2
                                    + ((cand_intra_pred_mode_l - 2 + 1) % 32);
                }
            }
            else
            {
                cand_mode_list[0] = cand_intra_pred_mode_l;
                cand_mode_list[1] = cand_intra_pred_mode_t;

                if((cand_intra_pred_mode_l != INTRA_PLANAR)
                                && (cand_intra_pred_mode_t != INTRA_PLANAR))
                {
                    cand_mode_list[2] = INTRA_PLANAR;
                }
                else if((cand_intra_pred_mode_l != INTRA_DC)
                                && (cand_intra_pred_mode_t != INTRA_DC))
                {
                    cand_mode_list[2] = INTRA_DC;
                }
                else
                {
                    cand_mode_list[2] = INTRA_ANGULAR(26);
                }
            }

            /* Computing Intra pred mode */
            if(ps_cu->ai4_prev_intra_luma_pred_flag[2 * i + j] == 1)
            {
                ps_cu->ai4_intra_luma_pred_mode[2 * i + j] =
                                cand_mode_list[ps_cu->ai4_mpm_idx[2 * i + j]];
            }
            else
            {
                WORD32 intra_pred_mode;
                /* Arranging cand_mode_list in increasing order */
                if(cand_mode_list[0] > cand_mode_list[1])
                {
                    SWAP(cand_mode_list[0], cand_mode_list[1]);
                }
                if(cand_mode_list[0] > cand_mode_list[2])
                {
                    SWAP(cand_mode_list[0], cand_mode_list[2]);
                }
                if(cand_mode_list[1] > cand_mode_list[2])
                {
                    SWAP(cand_mode_list[1], cand_mode_list[2]);
                }

                intra_pred_mode = ps_cu->ai4_rem_intra_luma_pred_mode[2 * i + j];

                if(intra_pred_mode >= cand_mode_list[0])
                    intra_pred_mode++;

                if(intra_pred_mode >= cand_mode_list[1])
                    intra_pred_mode++;

                if(intra_pred_mode >= cand_mode_list[2])
                    intra_pred_mode++;

                ps_cu->ai4_intra_luma_pred_mode[2 * i + j] = intra_pred_mode;
            }
            /* Update Top and Left intra pred mode */
            {
                WORD32 intra_pred_mode;

                intra_pred_mode = ps_cu->ai4_intra_luma_pred_mode[2 * i + j];

                ps_codec->s_func_selector.ihevc_memset_fptr(pu1_luma_intra_pred_mode_left + i * block_offset_in_min_pu, intra_pred_mode, (cb_size / num_pred_blocks) / MIN_PU_SIZE);
                ps_codec->s_func_selector.ihevc_memset_fptr(pu1_luma_intra_pred_mode_top + j * block_offset_in_min_pu, intra_pred_mode, (cb_size / num_pred_blocks) / MIN_PU_SIZE);

            }
            /* If partition is PART_NxN, then left is available for second column always */
            available_l_tmp = 1;

        }
        /* If partition is PART_NxN, then top is available for bottom row always */
        available_t = 1;
    }

    /* In case it is PART_2Nx2N partition, replicate intra pred mode in other three entries */
    if(ps_cu->i4_part_mode == PART_2Nx2N)
    {
        ps_cu->ai4_intra_luma_pred_mode[1] = ps_cu->ai4_intra_luma_pred_mode[0];
        ps_cu->ai4_intra_luma_pred_mode[2] = ps_cu->ai4_intra_luma_pred_mode[0];
        ps_cu->ai4_intra_luma_pred_mode[3] = ps_cu->ai4_intra_luma_pred_mode[0];
    }
}

