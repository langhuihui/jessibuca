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
*  ihevc_sao.c
*
* @brief
*  Contains leaf level function definitions for sample adaptive offset process
*
* @author
*  Srinivas T
*
* @par List of Functions:
*   - ihevc_sao_band_offset_luma()
*   - ihevc_sao_band_offset_chroma()
*   - ihevc_sao_edge_offset_class0()
*   - ihevc_sao_edge_offset_class0_chroma()
*   - ihevc_sao_edge_offset_class1()
*   - ihevc_sao_edge_offset_class1_chroma()
*   - ihevc_sao_edge_offset_class2()
*   - ihevc_sao_edge_offset_class2_chroma()
*   - ihevc_sao_edge_offset_class3()
*   - ihevc_sao_edge_offset_class3_chroma()
* @remarks
*  None
*
*******************************************************************************
*/
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ihevc_typedefs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_func_selector.h"
#include "ihevc_defs.h"
#include "ihevc_structs.h"
#include "ihevc_sao.h"

#define NUM_BAND_TABLE  32

const WORD32 gi4_ihevc_table_edge_idx[5] = { 1, 2, 0, 3, 4 };
/**
 * au4_avail is an array of flags - one for each neighboring block specifying if the block is available
 * au4_avail[0] - left
 * au4_avail[1] - right
 * au4_avail[2] - top
 * au4_avail[3] - bottom
 * au4_avail[4] - top-left
 * au4_avail[5] - top-right
 * au4_avail[6] - bottom-left
 * au4_avail[7] - bottom-right
 */


void ihevc_sao_band_offset_luma(UWORD8 *pu1_src,
                                WORD32 src_strd,
                                UWORD8 *pu1_src_left,
                                UWORD8 *pu1_src_top,
                                UWORD8 *pu1_src_top_left,
                                WORD32 sao_band_pos,
                                WORD8 *pi1_sao_offset,
                                WORD32 wd,
                                WORD32 ht)
{
    WORD32 band_shift;
    WORD32 band_table[NUM_BAND_TABLE];
    WORD32 i;
    WORD32 row, col;

    /* Updating left and top and top-left */
    for(row = 0; row < ht; row++)
    {
        pu1_src_left[row] = pu1_src[row * src_strd + (wd - 1)];
    }
    pu1_src_top_left[0] = pu1_src_top[wd - 1];
    for(col = 0; col < wd; col++)
    {
        pu1_src_top[col] = pu1_src[(ht - 1) * src_strd + col];
    }

    band_shift = BIT_DEPTH_LUMA - 5;
    for(i = 0; i < NUM_BAND_TABLE; i++)
    {
        band_table[i] = 0;
    }
    for(i = 0; i < 4; i++)
    {
        band_table[(i + sao_band_pos) & 31] = i + 1;
    }

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            WORD32 band_idx;

            band_idx = band_table[pu1_src[col] >> band_shift];
            pu1_src[col] = CLIP3(pu1_src[col] + pi1_sao_offset[band_idx], 0, (1 << (band_shift + 5)) - 1);
        }
        pu1_src += src_strd;
    }
}



/* input 'wd' has to be for the interleaved block and not for each color component */
void ihevc_sao_band_offset_chroma(UWORD8 *pu1_src,
                                  WORD32 src_strd,
                                  UWORD8 *pu1_src_left,
                                  UWORD8 *pu1_src_top,
                                  UWORD8 *pu1_src_top_left,
                                  WORD32 sao_band_pos_u,
                                  WORD32 sao_band_pos_v,
                                  WORD8 *pi1_sao_offset_u,
                                  WORD8 *pi1_sao_offset_v,
                                  WORD32 wd,
                                  WORD32 ht)
{
    WORD32 band_shift;
    WORD32 band_table_u[NUM_BAND_TABLE];
    WORD32 band_table_v[NUM_BAND_TABLE];
    WORD32 i;
    WORD32 row, col;

    /* Updating left and top and top-left */
    for(row = 0; row < ht; row++)
    {
        pu1_src_left[2 * row] = pu1_src[row * src_strd + (wd - 2)];
        pu1_src_left[2 * row + 1] = pu1_src[row * src_strd + (wd - 1)];
    }
    pu1_src_top_left[0] = pu1_src_top[wd - 2];
    pu1_src_top_left[1] = pu1_src_top[wd - 1];
    for(col = 0; col < wd; col++)
    {
        pu1_src_top[col] = pu1_src[(ht - 1) * src_strd + col];
    }


    band_shift = BIT_DEPTH_CHROMA - 5;
    for(i = 0; i < NUM_BAND_TABLE; i++)
    {
        band_table_u[i] = 0;
        band_table_v[i] = 0;
    }
    for(i = 0; i < 4; i++)
    {
        band_table_u[(i + sao_band_pos_u) & 31] = i + 1;
        band_table_v[(i + sao_band_pos_v) & 31] = i + 1;
    }

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            WORD32 band_idx;
            WORD8 *pi1_sao_offset;

            pi1_sao_offset = (0 == col % 2) ? pi1_sao_offset_u : pi1_sao_offset_v;
            band_idx = (0 == col % 2) ? band_table_u[pu1_src[col] >> band_shift] : band_table_v[pu1_src[col] >> band_shift];
            pu1_src[col] = CLIP3(pu1_src[col] + pi1_sao_offset[band_idx], 0, (1 << (band_shift + 5)) - 1);
        }
        pu1_src += src_strd;
    }
}



/* Horizontal filtering */
void ihevc_sao_edge_offset_class0(UWORD8 *pu1_src,
                                  WORD32 src_strd,
                                  UWORD8 *pu1_src_left,
                                  UWORD8 *pu1_src_top,
                                  UWORD8 *pu1_src_top_left,
                                  UWORD8 *pu1_src_top_right,
                                  UWORD8 *pu1_src_bot_left,
                                  UWORD8 *pu1_avail,
                                  WORD8 *pi1_sao_offset,
                                  WORD32 wd,
                                  WORD32 ht)
{
    WORD32 row, col;
    UWORD8 au1_mask[MAX_CTB_SIZE];
    UWORD8 au1_src_left_tmp[MAX_CTB_SIZE];
    WORD8 u1_sign_left, u1_sign_right;
    WORD32 bit_depth;
    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);
    bit_depth = BIT_DEPTH_LUMA;

    /* Initialize the mask values */
    memset(au1_mask, 0xFF, MAX_CTB_SIZE);

    /* Update top and top-left arrays */
    *pu1_src_top_left = pu1_src_top[wd - 1];
    for(row = 0; row < ht; row++)
    {
        au1_src_left_tmp[row] = pu1_src[row * src_strd + wd - 1];
    }
    for(col = 0; col < wd; col++)
    {
        pu1_src_top[col] = pu1_src[(ht - 1) * src_strd + col];
    }

    /* Update masks based on the availability flags */
    if(0 == pu1_avail[0])
    {
        au1_mask[0] = 0;
    }
    if(0 == pu1_avail[1])
    {
        au1_mask[wd - 1] = 0;
    }

    /* Processing is done on the intermediate buffer and the output is written to the source buffer */
    {
        for(row = 0; row < ht; row++)
        {
            u1_sign_left = SIGN(pu1_src[0] - pu1_src_left[row]);
            for(col = 0; col < wd; col++)
            {
                WORD32 edge_idx;

                u1_sign_right = SIGN(pu1_src[col] - pu1_src[col + 1]);
                edge_idx = 2 + u1_sign_left + u1_sign_right;
                u1_sign_left = -u1_sign_right;

                edge_idx = gi4_ihevc_table_edge_idx[edge_idx] & au1_mask[col];

                if(0 != edge_idx)
                {
                    pu1_src[col] = CLIP3(pu1_src[col] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
                }
            }

            pu1_src += src_strd;
        }
    }

    /* Update left array */
    for(row = 0; row < ht; row++)
    {
        pu1_src_left[row] = au1_src_left_tmp[row];
    }

}




/* input 'wd' has to be for the interleaved block and not for each color component */
void ihevc_sao_edge_offset_class0_chroma(UWORD8 *pu1_src,
                                         WORD32 src_strd,
                                         UWORD8 *pu1_src_left,
                                         UWORD8 *pu1_src_top,
                                         UWORD8 *pu1_src_top_left,
                                         UWORD8 *pu1_src_top_right,
                                         UWORD8 *pu1_src_bot_left,
                                         UWORD8 *pu1_avail,
                                         WORD8 *pi1_sao_offset_u,
                                         WORD8 *pi1_sao_offset_v,
                                         WORD32 wd,
                                         WORD32 ht)
{
    WORD32 row, col;
    UWORD8 au1_mask[MAX_CTB_SIZE];
    UWORD8 au1_src_left_tmp[2 * MAX_CTB_SIZE];
    WORD8 u1_sign_left_u, u1_sign_right_u;
    WORD8 u1_sign_left_v, u1_sign_right_v;
    WORD32 bit_depth;
    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);
    bit_depth = BIT_DEPTH_CHROMA;

    /* Initialize the mask values */
    memset(au1_mask, 0xFF, MAX_CTB_SIZE);

    /* Update left, top and top-left arrays */
    pu1_src_top_left[0] = pu1_src_top[wd - 2];
    pu1_src_top_left[1] = pu1_src_top[wd - 1];
    for(row = 0; row < ht; row++)
    {
        au1_src_left_tmp[2 * row] = pu1_src[row * src_strd + wd - 2];
        au1_src_left_tmp[2 * row + 1] = pu1_src[row * src_strd + wd - 1];
    }
    for(col = 0; col < wd; col++)
    {
        pu1_src_top[col] = pu1_src[(ht - 1) * src_strd + col];
    }

    /* Update masks based on the availability flags */
    if(0 == pu1_avail[0])
    {
        au1_mask[0] = 0;
    }
    if(0 == pu1_avail[1])
    {
        au1_mask[(wd - 1) >> 1] = 0;
    }

    /* Processing is done on the intermediate buffer and the output is written to the source buffer */
    {
        for(row = 0; row < ht; row++)
        {
            u1_sign_left_u = SIGN(pu1_src[0] - pu1_src_left[2 * row]);
            u1_sign_left_v = SIGN(pu1_src[1] - pu1_src_left[2 * row + 1]);
            for(col = 0; col < wd; col++)
            {
                WORD32 edge_idx;
                WORD8 *pi1_sao_offset;

                if(0 == col % 2)
                {
                    pi1_sao_offset = pi1_sao_offset_u;
                    u1_sign_right_u = SIGN(pu1_src[col] - pu1_src[col + 2]);
                    edge_idx = 2 + u1_sign_left_u + u1_sign_right_u;
                    u1_sign_left_u = -u1_sign_right_u;
                }
                else
                {
                    pi1_sao_offset = pi1_sao_offset_v;
                    u1_sign_right_v = SIGN(pu1_src[col] - pu1_src[col + 2]);
                    edge_idx = 2 + u1_sign_left_v + u1_sign_right_v;
                    u1_sign_left_v = -u1_sign_right_v;
                }

                edge_idx = gi4_ihevc_table_edge_idx[edge_idx] & au1_mask[col >> 1];

                if(0 != edge_idx)
                {
                    pu1_src[col] = CLIP3(pu1_src[col] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
                }
            }

            pu1_src += src_strd;
        }
    }

    for(row = 0; row < 2 * ht; row++)
    {
        pu1_src_left[row] = au1_src_left_tmp[row];
    }

}



/* Vertical filtering */
void ihevc_sao_edge_offset_class1(UWORD8 *pu1_src,
                                  WORD32 src_strd,
                                  UWORD8 *pu1_src_left,
                                  UWORD8 *pu1_src_top,
                                  UWORD8 *pu1_src_top_left,
                                  UWORD8 *pu1_src_top_right,
                                  UWORD8 *pu1_src_bot_left,
                                  UWORD8 *pu1_avail,
                                  WORD8 *pi1_sao_offset,
                                  WORD32 wd,
                                  WORD32 ht)
{
    WORD32 row, col;
    UWORD8 au1_mask[MAX_CTB_SIZE];
    UWORD8 au1_src_top_tmp[MAX_CTB_SIZE];
    WORD8 au1_sign_up[MAX_CTB_SIZE];
    WORD8 u1_sign_down;
    WORD32 bit_depth;
    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);

    bit_depth = BIT_DEPTH_LUMA;

    /* Initialize the mask values */
    memset(au1_mask, 0xFF, MAX_CTB_SIZE);

    /* Update left, top and top-left arrays */
    *pu1_src_top_left = pu1_src_top[wd - 1];
    for(row = 0; row < ht; row++)
    {
        pu1_src_left[row] = pu1_src[row * src_strd + wd - 1];
    }
    for(col = 0; col < wd; col++)
    {
        au1_src_top_tmp[col] = pu1_src[(ht - 1) * src_strd + col];
    }

    /* Update height and source pointers based on the availability flags */
    if(0 == pu1_avail[2])
    {
        pu1_src += src_strd;
        ht--;
        for(col = 0; col < wd; col++)
        {
            au1_sign_up[col] = SIGN(pu1_src[col] - pu1_src[col - src_strd]);
        }
    }
    else
    {
        for(col = 0; col < wd; col++)
        {
            au1_sign_up[col] = SIGN(pu1_src[col] - pu1_src_top[col]);
        }
    }
    if(0 == pu1_avail[3])
    {
        ht--;
    }

    /* Processing is done on the intermediate buffer and the output is written to the source buffer */
    {
        for(row = 0; row < ht; row++)
        {
            for(col = 0; col < wd; col++)
            {
                WORD32 edge_idx;

                u1_sign_down = SIGN(pu1_src[col] - pu1_src[col + src_strd]);
                edge_idx = 2 + au1_sign_up[col] + u1_sign_down;
                au1_sign_up[col] = -u1_sign_down;

                edge_idx = gi4_ihevc_table_edge_idx[edge_idx] & au1_mask[col];

                if(0 != edge_idx)
                {
                    pu1_src[col] = CLIP3(pu1_src[col] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
                }
            }

            pu1_src += src_strd;
        }
    }

    for(col = 0; col < wd; col++)
    {
        pu1_src_top[col] = au1_src_top_tmp[col];
    }

}



/* input 'wd' has to be for the interleaved block and not for each color component */
void ihevc_sao_edge_offset_class1_chroma(UWORD8 *pu1_src,
                                         WORD32 src_strd,
                                         UWORD8 *pu1_src_left,
                                         UWORD8 *pu1_src_top,
                                         UWORD8 *pu1_src_top_left,
                                         UWORD8 *pu1_src_top_right,
                                         UWORD8 *pu1_src_bot_left,
                                         UWORD8 *pu1_avail,
                                         WORD8 *pi1_sao_offset_u,
                                         WORD8 *pi1_sao_offset_v,
                                         WORD32 wd,
                                         WORD32 ht)
{
    WORD32 row, col;
    UWORD8 au1_mask[MAX_CTB_SIZE];
    UWORD8 au1_src_top_tmp[MAX_CTB_SIZE];
    WORD8 au1_sign_up[MAX_CTB_SIZE];
    WORD8 u1_sign_down;
    WORD32 bit_depth;
    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);

    bit_depth = BIT_DEPTH_CHROMA;

    /* Initialize the mask values */
    memset(au1_mask, 0xFF, MAX_CTB_SIZE);

    /* Update left, top and top-left arrays */
    pu1_src_top_left[0] = pu1_src_top[wd - 2];
    pu1_src_top_left[1] = pu1_src_top[wd - 1];
    for(row = 0; row < ht; row++)
    {
        pu1_src_left[2 * row] = pu1_src[row * src_strd + wd - 2];
        pu1_src_left[2 * row + 1] = pu1_src[row * src_strd + wd - 1];
    }
    for(col = 0; col < wd; col++)
    {
        au1_src_top_tmp[col] = pu1_src[(ht - 1) * src_strd + col];
    }

    /* Update height and source pointers based on the availability flags */
    if(0 == pu1_avail[2])
    {
        pu1_src += src_strd;
        ht--;
        for(col = 0; col < wd; col++)
        {
            au1_sign_up[col] = SIGN(pu1_src[col] - pu1_src[col - src_strd]);
        }
    }
    else
    {
        for(col = 0; col < wd; col++)
        {
            au1_sign_up[col] = SIGN(pu1_src[col] - pu1_src_top[col]);
        }
    }
    if(0 == pu1_avail[3])
    {
        ht--;
    }

    /* Processing is done on the intermediate buffer and the output is written to the source buffer */
    {
        for(row = 0; row < ht; row++)
        {
            for(col = 0; col < wd; col++)
            {
                WORD32 edge_idx;
                WORD8 *pi1_sao_offset;

                pi1_sao_offset = (0 == col % 2) ? pi1_sao_offset_u : pi1_sao_offset_v;

                u1_sign_down = SIGN(pu1_src[col] - pu1_src[col + src_strd]);
                edge_idx = 2 + au1_sign_up[col] + u1_sign_down;
                au1_sign_up[col] = -u1_sign_down;

                edge_idx = gi4_ihevc_table_edge_idx[edge_idx] & au1_mask[col >> 1];

                if(0 != edge_idx)
                {
                    pu1_src[col] = CLIP3(pu1_src[col] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
                }
            }

            pu1_src += src_strd;
        }
    }

    for(col = 0; col < wd; col++)
    {
        pu1_src_top[col] = au1_src_top_tmp[col];
    }

}



/* 135 degree filtering */
void ihevc_sao_edge_offset_class2(UWORD8 *pu1_src,
                                  WORD32 src_strd,
                                  UWORD8 *pu1_src_left,
                                  UWORD8 *pu1_src_top,
                                  UWORD8 *pu1_src_top_left,
                                  UWORD8 *pu1_src_top_right,
                                  UWORD8 *pu1_src_bot_left,
                                  UWORD8 *pu1_avail,
                                  WORD8 *pi1_sao_offset,
                                  WORD32 wd,
                                  WORD32 ht)
{
    WORD32 row, col;
    UWORD8 au1_mask[MAX_CTB_SIZE];
    UWORD8 au1_src_left_tmp[MAX_CTB_SIZE], au1_src_top_tmp[MAX_CTB_SIZE];
    UWORD8 u1_src_top_left_tmp;
    WORD8 au1_sign_up[MAX_CTB_SIZE + 1], au1_sign_up_tmp[MAX_CTB_SIZE + 1];
    WORD8 u1_sign_down;
    WORD8 *pu1_sign_up;
    WORD8 *pu1_sign_up_tmp;
    UWORD8 *pu1_src_left_cpy;

    WORD32 bit_depth;
    UWORD8 u1_pos_0_0_tmp;
    UWORD8 u1_pos_wd_ht_tmp;
    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);

    bit_depth = BIT_DEPTH_LUMA;
    pu1_sign_up = au1_sign_up;
    pu1_sign_up_tmp = au1_sign_up_tmp;
    pu1_src_left_cpy = pu1_src_left;

    /* Initialize the mask values */
    memset(au1_mask, 0xFF, MAX_CTB_SIZE);

    /* Update left, top and top-left arrays */
    u1_src_top_left_tmp = pu1_src_top[wd - 1];
    for(row = 0; row < ht; row++)
    {
        au1_src_left_tmp[row] = pu1_src[row * src_strd + wd - 1];
    }
    for(col = 0; col < wd; col++)
    {
        au1_src_top_tmp[col] = pu1_src[(ht - 1) * src_strd + col];
    }


    /* If top-left is available, process separately */
    if(0 != pu1_avail[4])
    {
        WORD32 edge_idx;

        edge_idx = 2 + SIGN(pu1_src[0] - pu1_src_top_left[0]) +
                        SIGN(pu1_src[0] - pu1_src[1 + src_strd]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_0_tmp = CLIP3(pu1_src[0] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_0_tmp = pu1_src[0];
        }
    }
    else
    {
        u1_pos_0_0_tmp = pu1_src[0];
    }

    /* If bottom-right is available, process separately */
    if(0 != pu1_avail[7])
    {
        WORD32 edge_idx;

        edge_idx = 2 + SIGN(pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd - 1 - src_strd]) +
                        SIGN(pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd + 1 + src_strd]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_ht_tmp = CLIP3(pu1_src[wd - 1 + (ht - 1) * src_strd] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_ht_tmp = pu1_src[wd - 1 + (ht - 1) * src_strd];
        }
    }
    else
    {
        u1_pos_wd_ht_tmp = pu1_src[wd - 1 + (ht - 1) * src_strd];
    }

    /* If Left is not available */
    if(0 == pu1_avail[0])
    {
        au1_mask[0] = 0;
    }

    /* If Top is not available */
    if(0 == pu1_avail[2])
    {
        pu1_src += src_strd;
        ht--;
        pu1_src_left_cpy += 1;
        for(col = 1; col < wd; col++)
        {
            pu1_sign_up[col] = SIGN(pu1_src[col] - pu1_src[col - 1 - src_strd]);
        }
    }
    else
    {
        for(col = 1; col < wd; col++)
        {
            pu1_sign_up[col] = SIGN(pu1_src[col] - pu1_src_top[col - 1]);
        }
    }

    /* If Right is not available */
    if(0 == pu1_avail[1])
    {
        au1_mask[wd - 1] = 0;
    }

    /* If Bottom is not available */
    if(0 == pu1_avail[3])
    {
        ht--;
    }

    /* Processing is done on the intermediate buffer and the output is written to the source buffer */
    {
        for(row = 0; row < ht; row++)
        {
            pu1_sign_up[0] = SIGN(pu1_src[0] - pu1_src_left_cpy[row - 1]);
            for(col = 0; col < wd; col++)
            {
                WORD32 edge_idx;

                u1_sign_down = SIGN(pu1_src[col] - pu1_src[col + 1 + src_strd]);
                edge_idx = 2 + pu1_sign_up[col] + u1_sign_down;
                pu1_sign_up_tmp[col + 1] = -u1_sign_down;

                edge_idx = gi4_ihevc_table_edge_idx[edge_idx] & au1_mask[col];

                if(0 != edge_idx)
                {
                    pu1_src[col] = CLIP3(pu1_src[col] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
                }
            }

            /* Swapping pu1_sign_up_tmp and pu1_sign_up */
            {
                WORD8 *pu1_swap_tmp = pu1_sign_up;
                pu1_sign_up = pu1_sign_up_tmp;
                pu1_sign_up_tmp = pu1_swap_tmp;
            }

            pu1_src += src_strd;
        }

        pu1_src[-(pu1_avail[2] ? ht : ht + 1) * src_strd] = u1_pos_0_0_tmp;
        pu1_src[(pu1_avail[3] ? wd - 1 - src_strd : wd - 1)] = u1_pos_wd_ht_tmp;
    }

    if(0 == pu1_avail[2])
        ht++;
    if(0 == pu1_avail[3])
        ht++;
    *pu1_src_top_left = u1_src_top_left_tmp;
    for(row = 0; row < ht; row++)
    {
        pu1_src_left[row] = au1_src_left_tmp[row];
    }
    for(col = 0; col < wd; col++)
    {
        pu1_src_top[col] = au1_src_top_tmp[col];
    }

}




/* 135 degree filtering */
void ihevc_sao_edge_offset_class2_chroma(UWORD8 *pu1_src,
                                         WORD32 src_strd,
                                         UWORD8 *pu1_src_left,
                                         UWORD8 *pu1_src_top,
                                         UWORD8 *pu1_src_top_left,
                                         UWORD8 *pu1_src_top_right,
                                         UWORD8 *pu1_src_bot_left,
                                         UWORD8 *pu1_avail,
                                         WORD8 *pi1_sao_offset_u,
                                         WORD8 *pi1_sao_offset_v,
                                         WORD32 wd,
                                         WORD32 ht)
{
    WORD32 row, col;
    UWORD8 au1_mask[MAX_CTB_SIZE];
    UWORD8 au1_src_left_tmp[2 * MAX_CTB_SIZE], au1_src_top_tmp[MAX_CTB_SIZE];
    UWORD8 au1_src_top_left_tmp[2];
    WORD8 au1_sign_up[MAX_CTB_SIZE + 2], au1_sign_up_tmp[MAX_CTB_SIZE + 2];
    WORD8 u1_sign_down;
    WORD8 *pu1_sign_up;
    WORD8 *pu1_sign_up_tmp;
    UWORD8 *pu1_src_left_cpy;

    WORD32 bit_depth;

    UWORD8 u1_pos_0_0_tmp_u;
    UWORD8 u1_pos_0_0_tmp_v;
    UWORD8 u1_pos_wd_ht_tmp_u;
    UWORD8 u1_pos_wd_ht_tmp_v;
    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);


    bit_depth = BIT_DEPTH_CHROMA;
    pu1_sign_up = au1_sign_up;
    pu1_sign_up_tmp = au1_sign_up_tmp;
    pu1_src_left_cpy = pu1_src_left;

    /* Initialize the mask values */
    memset(au1_mask, 0xFF, MAX_CTB_SIZE);

    /* Update left, top and top-left arrays */
    au1_src_top_left_tmp[0] = pu1_src_top[wd - 2];
    au1_src_top_left_tmp[1] = pu1_src_top[wd - 1];
    for(row = 0; row < ht; row++)
    {
        au1_src_left_tmp[2 * row] = pu1_src[row * src_strd + wd - 2];
        au1_src_left_tmp[2 * row + 1] = pu1_src[row * src_strd + wd - 1];
    }
    for(col = 0; col < wd; col++)
    {
        au1_src_top_tmp[col] = pu1_src[(ht - 1) * src_strd + col];
    }


    /* If top-left is available, process separately */
    if(0 != pu1_avail[4])
    {
        WORD32 edge_idx;

        /* U */
        edge_idx = 2 + SIGN(pu1_src[0] - pu1_src_top_left[0]) +
                        SIGN(pu1_src[0] - pu1_src[2 + src_strd]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_0_tmp_u = CLIP3(pu1_src[0] + pi1_sao_offset_u[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_0_tmp_u = pu1_src[0];
        }

        /* V */
        edge_idx = 2 + SIGN(pu1_src[1] - pu1_src_top_left[1]) +
                        SIGN(pu1_src[1] - pu1_src[1 + 2 + src_strd]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_0_tmp_v = CLIP3(pu1_src[1] + pi1_sao_offset_v[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_0_tmp_v = pu1_src[1];
        }
    }
    else
    {
        u1_pos_0_0_tmp_u = pu1_src[0];
        u1_pos_0_0_tmp_v = pu1_src[1];
    }

    /* If bottom-right is available, process separately */
    if(0 != pu1_avail[7])
    {
        WORD32 edge_idx;

        /* U */
        edge_idx = 2 + SIGN(pu1_src[wd - 2 + (ht - 1) * src_strd] - pu1_src[wd - 2 + (ht - 1) * src_strd - 2 - src_strd]) +
                        SIGN(pu1_src[wd - 2 + (ht - 1) * src_strd] - pu1_src[wd - 2 + (ht - 1) * src_strd + 2 + src_strd]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_ht_tmp_u = CLIP3(pu1_src[wd - 2 + (ht - 1) * src_strd] + pi1_sao_offset_u[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_ht_tmp_u = pu1_src[wd - 2 + (ht - 1) * src_strd];
        }

        /* V */
        edge_idx = 2 + SIGN(pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd - 2 - src_strd]) +
                        SIGN(pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd + 2 + src_strd]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_ht_tmp_v = CLIP3(pu1_src[wd - 1 + (ht - 1) * src_strd] + pi1_sao_offset_v[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_ht_tmp_v = pu1_src[wd - 1 + (ht - 1) * src_strd];
        }
    }
    else
    {
        u1_pos_wd_ht_tmp_u = pu1_src[wd - 2 + (ht - 1) * src_strd];
        u1_pos_wd_ht_tmp_v = pu1_src[wd - 1 + (ht - 1) * src_strd];
    }

    /* If Left is not available */
    if(0 == pu1_avail[0])
    {
        au1_mask[0] = 0;
    }

    /* If Top is not available */
    if(0 == pu1_avail[2])
    {
        pu1_src += src_strd;
        pu1_src_left_cpy += 2;
        ht--;
        for(col = 2; col < wd; col++)
        {
            pu1_sign_up[col] = SIGN(pu1_src[col] - pu1_src[col - 2 - src_strd]);
        }
    }
    else
    {
        for(col = 2; col < wd; col++)
        {
            pu1_sign_up[col] = SIGN(pu1_src[col] - pu1_src_top[col - 2]);
        }
    }

    /* If Right is not available */
    if(0 == pu1_avail[1])
    {
        au1_mask[(wd - 1) >> 1] = 0;
    }

    /* If Bottom is not available */
    if(0 == pu1_avail[3])
    {
        ht--;
    }

    /* Processing is done on the intermediate buffer and the output is written to the source buffer */
    {
        for(row = 0; row < ht; row++)
        {
            pu1_sign_up[0] = SIGN(pu1_src[0] - pu1_src_left_cpy[2 * (row - 1)]);
            pu1_sign_up[1] = SIGN(pu1_src[1] - pu1_src_left_cpy[2 * (row - 1) + 1]);
            for(col = 0; col < wd; col++)
            {
                WORD32 edge_idx;
                WORD8 *pi1_sao_offset;

                pi1_sao_offset = (0 == col % 2) ? pi1_sao_offset_u : pi1_sao_offset_v;

                u1_sign_down = SIGN(pu1_src[col] - pu1_src[col + 2 + src_strd]);
                edge_idx = 2 + pu1_sign_up[col] + u1_sign_down;
                pu1_sign_up_tmp[col + 2] = -u1_sign_down;

                edge_idx = gi4_ihevc_table_edge_idx[edge_idx] & au1_mask[col >> 1];

                if(0 != edge_idx)
                {
                    pu1_src[col] = CLIP3(pu1_src[col] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
                }
            }

            /* Swapping pu1_sign_up_tmp and pu1_sign_up */
            {
                WORD8 *pu1_swap_tmp = pu1_sign_up;
                pu1_sign_up = pu1_sign_up_tmp;
                pu1_sign_up_tmp = pu1_swap_tmp;
            }

            pu1_src += src_strd;
        }

        pu1_src[-(pu1_avail[2] ? ht : ht + 1) * src_strd] = u1_pos_0_0_tmp_u;
        pu1_src[-(pu1_avail[2] ? ht : ht + 1) * src_strd + 1] = u1_pos_0_0_tmp_v;
        pu1_src[(pu1_avail[3] ? wd - 2 - src_strd : wd - 2)] = u1_pos_wd_ht_tmp_u;
        pu1_src[(pu1_avail[3] ? wd - 1 - src_strd : wd - 1)] = u1_pos_wd_ht_tmp_v;
    }

    if(0 == pu1_avail[2])
        ht++;
    if(0 == pu1_avail[3])
        ht++;
    pu1_src_top_left[0] = au1_src_top_left_tmp[0];
    pu1_src_top_left[1] = au1_src_top_left_tmp[1];
    for(row = 0; row < 2 * ht; row++)
    {
        pu1_src_left[row] = au1_src_left_tmp[row];
    }
    for(col = 0; col < wd; col++)
    {
        pu1_src_top[col] = au1_src_top_tmp[col];
    }

}




/* 45 degree filtering */
void ihevc_sao_edge_offset_class3(UWORD8 *pu1_src,
                                  WORD32 src_strd,
                                  UWORD8 *pu1_src_left,
                                  UWORD8 *pu1_src_top,
                                  UWORD8 *pu1_src_top_left,
                                  UWORD8 *pu1_src_top_right,
                                  UWORD8 *pu1_src_bot_left,
                                  UWORD8 *pu1_avail,
                                  WORD8 *pi1_sao_offset,
                                  WORD32 wd,
                                  WORD32 ht)
{
    WORD32 row, col;
    UWORD8 au1_mask[MAX_CTB_SIZE];
    UWORD8 au1_src_top_tmp[MAX_CTB_SIZE];
    UWORD8 au1_src_left_tmp[MAX_CTB_SIZE];
    UWORD8 u1_src_top_left_tmp;
    WORD8 au1_sign_up[MAX_CTB_SIZE];
    UWORD8 *pu1_src_left_cpy;
    WORD8 u1_sign_down;
    WORD32 bit_depth;

    UWORD8 u1_pos_0_ht_tmp;
    UWORD8 u1_pos_wd_0_tmp;

    bit_depth = BIT_DEPTH_LUMA;
    pu1_src_left_cpy = pu1_src_left;

    /* Initialize the mask values */
    memset(au1_mask, 0xFF, MAX_CTB_SIZE);

    /* Update left, top and top-left arrays */
    u1_src_top_left_tmp = pu1_src_top[wd - 1];
    for(row = 0; row < ht; row++)
    {
        au1_src_left_tmp[row] = pu1_src[row * src_strd + wd - 1];
    }
    for(col = 0; col < wd; col++)
    {
        au1_src_top_tmp[col] = pu1_src[(ht - 1) * src_strd + col];
    }

    /* If top-right is available, process separately */
    if(0 != pu1_avail[5])
    {
        WORD32 edge_idx;

        edge_idx = 2 + SIGN(pu1_src[wd - 1] - pu1_src_top_right[0]) +
                        SIGN(pu1_src[wd - 1] - pu1_src[wd - 1 - 1 + src_strd]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_0_tmp = CLIP3(pu1_src[wd - 1] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_0_tmp = pu1_src[wd - 1];
        }
    }
    else
    {
        u1_pos_wd_0_tmp = pu1_src[wd - 1];
    }

    /* If bottom-left is available, process separately */
    if(0 != pu1_avail[6])
    {
        WORD32 edge_idx;

        edge_idx = 2 + SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src[(ht - 1) * src_strd + 1 - src_strd]) +
                        SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src_bot_left[0]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_ht_tmp = CLIP3(pu1_src[(ht - 1) * src_strd] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_ht_tmp = pu1_src[(ht - 1) * src_strd];
        }
    }
    else
    {
        u1_pos_0_ht_tmp = pu1_src[(ht - 1) * src_strd];
    }

    /* If Left is not available */
    if(0 == pu1_avail[0])
    {
        au1_mask[0] = 0;
    }

    /* If Top is not available */
    if(0 == pu1_avail[2])
    {
        pu1_src += src_strd;
        ht--;
        pu1_src_left_cpy += 1;
        for(col = 0; col < wd - 1; col++)
        {
            au1_sign_up[col] = SIGN(pu1_src[col] - pu1_src[col + 1 - src_strd]);
        }
    }
    else
    {
        for(col = 0; col < wd - 1; col++)
        {
            au1_sign_up[col] = SIGN(pu1_src[col] - pu1_src_top[col + 1]);
        }
    }

    /* If Right is not available */
    if(0 == pu1_avail[1])
    {
        au1_mask[wd - 1] = 0;
    }

    /* If Bottom is not available */
    if(0 == pu1_avail[3])
    {
        ht--;
    }

    /* Processing is done on the intermediate buffer and the output is written to the source buffer */
    {
        for(row = 0; row < ht; row++)
        {
            au1_sign_up[wd - 1] = SIGN(pu1_src[wd - 1] - pu1_src[wd - 1 + 1 - src_strd]);
            for(col = 0; col < wd; col++)
            {
                WORD32 edge_idx;

                u1_sign_down = SIGN(pu1_src[col] - ((col == 0) ? pu1_src_left_cpy[row + 1] :
                                                                 pu1_src[col - 1 + src_strd]));
                edge_idx = 2 + au1_sign_up[col] + u1_sign_down;
                if(col > 0)
                    au1_sign_up[col - 1] = -u1_sign_down;

                edge_idx = gi4_ihevc_table_edge_idx[edge_idx] & au1_mask[col];

                if(0 != edge_idx)
                {
                    pu1_src[col] = CLIP3(pu1_src[col] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
                }
            }

            pu1_src += src_strd;
        }

        pu1_src[-(pu1_avail[2] ? ht : ht + 1) * src_strd + wd - 1] = u1_pos_wd_0_tmp;
        pu1_src[(pu1_avail[3] ?  (-src_strd) : 0)] = u1_pos_0_ht_tmp;
    }

    if(0 == pu1_avail[2])
        ht++;
    if(0 == pu1_avail[3])
        ht++;
    *pu1_src_top_left = u1_src_top_left_tmp;
    for(row = 0; row < ht; row++)
    {
        pu1_src_left[row] = au1_src_left_tmp[row];
    }
    for(col = 0; col < wd; col++)
    {
        pu1_src_top[col] = au1_src_top_tmp[col];
    }

}




void ihevc_sao_edge_offset_class3_chroma(UWORD8 *pu1_src,
                                         WORD32 src_strd,
                                         UWORD8 *pu1_src_left,
                                         UWORD8 *pu1_src_top,
                                         UWORD8 *pu1_src_top_left,
                                         UWORD8 *pu1_src_top_right,
                                         UWORD8 *pu1_src_bot_left,
                                         UWORD8 *pu1_avail,
                                         WORD8 *pi1_sao_offset_u,
                                         WORD8 *pi1_sao_offset_v,
                                         WORD32 wd,
                                         WORD32 ht)
{
    WORD32 row, col;
    UWORD8 au1_mask[MAX_CTB_SIZE];
    UWORD8 au1_src_left_tmp[2 * MAX_CTB_SIZE], au1_src_top_tmp[MAX_CTB_SIZE];
    UWORD8 au1_src_top_left_tmp[2];
    WORD8 au1_sign_up[MAX_CTB_SIZE];
    UWORD8 *pu1_src_left_cpy;
    WORD8 u1_sign_down;
    WORD32 bit_depth;

    UWORD8 u1_pos_wd_0_tmp_u;
    UWORD8 u1_pos_wd_0_tmp_v;
    UWORD8 u1_pos_0_ht_tmp_u;
    UWORD8 u1_pos_0_ht_tmp_v;

    bit_depth = BIT_DEPTH_CHROMA;
    pu1_src_left_cpy = pu1_src_left;

    /* Initialize the mask values */
    memset(au1_mask, 0xFF, MAX_CTB_SIZE);

    /* Update left, top and top-left arrays */
    au1_src_top_left_tmp[0] = pu1_src_top[wd - 2];
    au1_src_top_left_tmp[1] = pu1_src_top[wd - 1];
    for(row = 0; row < ht; row++)
    {
        au1_src_left_tmp[2 * row] = pu1_src[row * src_strd + wd - 2];
        au1_src_left_tmp[2 * row + 1] = pu1_src[row * src_strd + wd - 1];
    }
    for(col = 0; col < wd; col++)
    {
        au1_src_top_tmp[col] = pu1_src[(ht - 1) * src_strd + col];
    }


    /* If top-right is available, process separately */
    if(0 != pu1_avail[5])
    {
        WORD32 edge_idx;

        /* U */
        edge_idx = 2 + SIGN(pu1_src[wd - 2] - pu1_src_top_right[0]) +
                        SIGN(pu1_src[wd - 2] - pu1_src[wd - 2 - 2 + src_strd]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_0_tmp_u = CLIP3(pu1_src[wd - 2] + pi1_sao_offset_u[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_0_tmp_u = pu1_src[wd - 2];
        }

        /* V */
        edge_idx = 2 + SIGN(pu1_src[wd - 1] - pu1_src_top_right[1]) +
                        SIGN(pu1_src[wd - 1] - pu1_src[wd - 1 - 2 + src_strd]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_0_tmp_v = CLIP3(pu1_src[wd - 1] + pi1_sao_offset_v[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_0_tmp_v = pu1_src[wd - 1];
        }
    }
    else
    {
        u1_pos_wd_0_tmp_u = pu1_src[wd - 2];
        u1_pos_wd_0_tmp_v = pu1_src[wd - 1];
    }

    /* If bottom-left is available, process separately */
    if(0 != pu1_avail[6])
    {
        WORD32 edge_idx;

        /* U */
        edge_idx = 2 + SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src[(ht - 1) * src_strd + 2 - src_strd]) +
                        SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src_bot_left[0]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_ht_tmp_u = CLIP3(pu1_src[(ht - 1) * src_strd] + pi1_sao_offset_u[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_ht_tmp_u = pu1_src[(ht - 1) * src_strd];
        }

        /* V */
        edge_idx = 2 + SIGN(pu1_src[(ht - 1) * src_strd + 1] - pu1_src[(ht - 1) * src_strd + 1 + 2 - src_strd]) +
                        SIGN(pu1_src[(ht - 1) * src_strd + 1] - pu1_src_bot_left[1]);

        edge_idx = gi4_ihevc_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_ht_tmp_v = CLIP3(pu1_src[(ht - 1) * src_strd + 1] + pi1_sao_offset_v[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_ht_tmp_v = pu1_src[(ht - 1) * src_strd + 1];
        }
    }
    else
    {
        u1_pos_0_ht_tmp_u = pu1_src[(ht - 1) * src_strd];
        u1_pos_0_ht_tmp_v = pu1_src[(ht - 1) * src_strd + 1];
    }

    /* If Left is not available */
    if(0 == pu1_avail[0])
    {
        au1_mask[0] = 0;
    }

    /* If Top is not available */
    if(0 == pu1_avail[2])
    {
        pu1_src += src_strd;
        ht--;
        pu1_src_left_cpy += 2;
        for(col = 0; col < wd - 2; col++)
        {
            au1_sign_up[col] = SIGN(pu1_src[col] - pu1_src[col + 2 - src_strd]);
        }
    }
    else
    {
        for(col = 0; col < wd - 2; col++)
        {
            au1_sign_up[col] = SIGN(pu1_src[col] - pu1_src_top[col + 2]);
        }
    }

    /* If Right is not available */
    if(0 == pu1_avail[1])
    {
        au1_mask[(wd - 1) >> 1] = 0;
    }

    /* If Bottom is not available */
    if(0 == pu1_avail[3])
    {
        ht--;
    }

    /* Processing is done on the intermediate buffer and the output is written to the source buffer */
    {
        for(row = 0; row < ht; row++)
        {
            au1_sign_up[wd - 2] = SIGN(pu1_src[wd - 2] - pu1_src[wd - 2 + 2 - src_strd]);
            au1_sign_up[wd - 1] = SIGN(pu1_src[wd - 1] - pu1_src[wd - 1 + 2 - src_strd]);
            for(col = 0; col < wd; col++)
            {
                WORD32 edge_idx;
                WORD8 *pi1_sao_offset;

                pi1_sao_offset = (0 == col % 2) ? pi1_sao_offset_u : pi1_sao_offset_v;

                u1_sign_down = SIGN(pu1_src[col] - ((col < 2) ? pu1_src_left_cpy[2 * (row + 1) + col] :
                                                                pu1_src[col - 2 + src_strd]));
                edge_idx = 2 + au1_sign_up[col] + u1_sign_down;
                if(col > 1)
                    au1_sign_up[col - 2] = -u1_sign_down;

                edge_idx = gi4_ihevc_table_edge_idx[edge_idx] & au1_mask[col >> 1];

                if(0 != edge_idx)
                {
                    pu1_src[col] = CLIP3(pu1_src[col] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
                }
            }

            pu1_src += src_strd;
        }

        pu1_src[-(pu1_avail[2] ? ht : ht + 1) * src_strd + wd - 2] = u1_pos_wd_0_tmp_u;
        pu1_src[-(pu1_avail[2] ? ht : ht + 1) * src_strd + wd - 1] = u1_pos_wd_0_tmp_v;
        pu1_src[(pu1_avail[3] ?  (-src_strd) : 0)] = u1_pos_0_ht_tmp_u;
        pu1_src[(pu1_avail[3] ?  (-src_strd) : 0) + 1] = u1_pos_0_ht_tmp_v;
    }

    if(0 == pu1_avail[2])
        ht++;
    if(0 == pu1_avail[3])
        ht++;
    pu1_src_top_left[0] = au1_src_top_left_tmp[0];
    pu1_src_top_left[1] = au1_src_top_left_tmp[1];
    for(row = 0; row < 2 * ht; row++)
    {
        pu1_src_left[row] = au1_src_left_tmp[row];
    }
    for(col = 0; col < wd; col++)
    {
        pu1_src_top[col] = au1_src_top_tmp[col];
    }

}
