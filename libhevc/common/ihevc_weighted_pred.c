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
*  ihevc_weighted_pred.c
*
* @brief
*  Contains function definitions for weighted prediction used in inter
* prediction
*
* @author
*  Srinivas T
*
* @par List of Functions:
*   - ihevc_weighted_pred_uni()
*   - ihevc_weighted_pred_bi()
*   - ihevc_weighted_pred_bi_default()
*   - ihevc_weighted_pred_chroma_uni()
*   - ihevc_weighted_pred_chroma_bi()
*   - ihevc_weighted_pred_chroma_bi_default()
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
#include "ihevc_defs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_func_selector.h"

#include "ihevc_inter_pred.h"

/**
*******************************************************************************
*
* @brief
*  Does uni-weighted prediction on the array pointed by  pi2_src and stores
* it at the location pointed by pi2_dst
*
* @par Description:
*  dst = ( (src + lvl_shift) * wgt0 + (1 << (shift - 1)) )  >> shift +
* offset
*
* @param[in] pi2_src
*  Pointer to the source
*
* @param[out] pu1_dst
*  Pointer to the destination
*
* @param[in] src_strd
*  Source stride
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] wgt0
*  weight to be multiplied to the source
*
* @param[in] off0
*  offset to be added after rounding and
*
* @param[in] shifting
*
*
* @param[in] shift
*  (14 Bit depth) + log2_weight_denominator
*
* @param[in] lvl_shift
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_weighted_pred_uni(WORD16 *pi2_src,
                             UWORD8 *pu1_dst,
                             WORD32 src_strd,
                             WORD32 dst_strd,
                             WORD32 wgt0,
                             WORD32 off0,
                             WORD32 shift,
                             WORD32 lvl_shift,
                             WORD32 ht,
                             WORD32 wd)
{
    WORD32 row, col;
    WORD32 i4_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            i4_tmp = (pi2_src[col] + lvl_shift) * wgt0;
            i4_tmp += 1 << (shift - 1);
            i4_tmp = (i4_tmp >> shift) + off0;

            pu1_dst[col] = CLIP_U8(i4_tmp);
        }

        pi2_src += src_strd;
        pu1_dst += dst_strd;
    }
}
//WEIGHTED_PRED_UNI

/**
*******************************************************************************
*
* @brief
* Does chroma uni-weighted prediction on array pointed by pi2_src and stores
* it at the location pointed by pi2_dst
*
* @par Description:
*  dst = ( (src + lvl_shift) * wgt0 + (1 << (shift - 1)) )  >> shift +
* offset
*
* @param[in] pi2_src
*  Pointer to the source
*
* @param[out] pu1_dst
*  Pointer to the destination
*
* @param[in] src_strd
*  Source stride
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] wgt0
*  weight to be multiplied to the source
*
* @param[in] off0
*  offset to be added after rounding and
*
* @param[in] shifting
*
*
* @param[in] shift
*  (14 Bit depth) + log2_weight_denominator
*
* @param[in] lvl_shift
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source (each colour component)
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_weighted_pred_chroma_uni(WORD16 *pi2_src,
                                    UWORD8 *pu1_dst,
                                    WORD32 src_strd,
                                    WORD32 dst_strd,
                                    WORD32 wgt0_cb,
                                    WORD32 wgt0_cr,
                                    WORD32 off0_cb,
                                    WORD32 off0_cr,
                                    WORD32 shift,
                                    WORD32 lvl_shift,
                                    WORD32 ht,
                                    WORD32 wd)
{
    WORD32 row, col;
    WORD32 i4_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < 2 * wd; col += 2)
        {
            i4_tmp = (pi2_src[col] + lvl_shift) * wgt0_cb;
            i4_tmp += 1 << (shift - 1);
            i4_tmp = (i4_tmp >> shift) + off0_cb;

            pu1_dst[col] = CLIP_U8(i4_tmp);

            i4_tmp = (pi2_src[col + 1] + lvl_shift) * wgt0_cr;
            i4_tmp += 1 << (shift - 1);
            i4_tmp = (i4_tmp >> shift) + off0_cr;

            pu1_dst[col + 1] = CLIP_U8(i4_tmp);
        }

        pi2_src += src_strd;
        pu1_dst += dst_strd;
    }
}
//WEIGHTED_PRED_CHROMA_UNI

/**
*******************************************************************************
*
* @brief
*  Does bi-weighted prediction on the arrays pointed by  pi2_src1 and
* pi2_src2 and stores it at location pointed  by pi2_dst
*
* @par Description:
*  dst = ( (src1 + lvl_shift1)*wgt0 +  (src2 + lvl_shift2)*wgt1 +  (off0 +
* off1 + 1) << (shift - 1) ) >> shift
*
* @param[in] pi2_src1
*  Pointer to source 1
*
* @param[in] pi2_src2
*  Pointer to source 2
*
* @param[out] pu1_dst
*  Pointer to destination
*
* @param[in] src_strd1
*  Source stride 1
*
* @param[in] src_strd2
*  Source stride 2
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] wgt0
*  weight to be multiplied to source 1
*
* @param[in] off0
*  offset 0
*
* @param[in] wgt1
*  weight to be multiplied to source 2
*
* @param[in] off1
*  offset 1
*
* @param[in] shift
*  (14 Bit depth) + log2_weight_denominator
*
* @param[in] lvl_shift1
*  added before shift and offset
*
* @param[in] lvl_shift2
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_weighted_pred_bi(WORD16 *pi2_src1,
                            WORD16 *pi2_src2,
                            UWORD8 *pu1_dst,
                            WORD32 src_strd1,
                            WORD32 src_strd2,
                            WORD32 dst_strd,
                            WORD32 wgt0,
                            WORD32 off0,
                            WORD32 wgt1,
                            WORD32 off1,
                            WORD32 shift,
                            WORD32 lvl_shift1,
                            WORD32 lvl_shift2,
                            WORD32 ht,
                            WORD32 wd)
{
    WORD32 row, col;
    WORD32 i4_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            i4_tmp = (pi2_src1[col] + lvl_shift1) * wgt0;
            i4_tmp += (pi2_src2[col] + lvl_shift2) * wgt1;
            i4_tmp += (off0 + off1 + 1) << (shift - 1);

            pu1_dst[col] = CLIP_U8(i4_tmp >> shift);
        }

        pi2_src1 += src_strd1;
        pi2_src2 += src_strd2;
        pu1_dst += dst_strd;
    }
}
//WEIGHTED_PRED_BI

/**
*******************************************************************************
*
* @brief
* Does chroma bi-weighted prediction on the arrays pointed by  pi2_src1 and
* pi2_src2 and stores it at location pointed  by pi2_dst
*
* @par Description:
*  dst = ( (src1 + lvl_shift1)*wgt0 +  (src2 + lvl_shift2)*wgt1 +  (off0 +
* off1 + 1) << (shift - 1) ) >> shift
*
* @param[in] pi2_src1
*  Pointer to source 1
*
* @param[in] pi2_src2
*  Pointer to source 2
*
* @param[out] pu1_dst
*  Pointer to destination
*
* @param[in] src_strd1
*  Source stride 1
*
* @param[in] src_strd2
*  Source stride 2
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] wgt0
*  weight to be multiplied to source 1
*
* @param[in] off0
*  offset 0
*
* @param[in] wgt1
*  weight to be multiplied to source 2
*
* @param[in] off1
*  offset 1
*
* @param[in] shift
*  (14 Bit depth) + log2_weight_denominator
*
* @param[in] lvl_shift1
*  added before shift and offset
*
* @param[in] lvl_shift2
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source (each colour component)
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_weighted_pred_chroma_bi(WORD16 *pi2_src1,
                                   WORD16 *pi2_src2,
                                   UWORD8 *pu1_dst,
                                   WORD32 src_strd1,
                                   WORD32 src_strd2,
                                   WORD32 dst_strd,
                                   WORD32 wgt0_cb,
                                   WORD32 wgt0_cr,
                                   WORD32 off0_cb,
                                   WORD32 off0_cr,
                                   WORD32 wgt1_cb,
                                   WORD32 wgt1_cr,
                                   WORD32 off1_cb,
                                   WORD32 off1_cr,
                                   WORD32 shift,
                                   WORD32 lvl_shift1,
                                   WORD32 lvl_shift2,
                                   WORD32 ht,
                                   WORD32 wd)
{
    WORD32 row, col;
    WORD32 i4_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < 2 * wd; col += 2)
        {
            i4_tmp = (pi2_src1[col] + lvl_shift1) * wgt0_cb;
            i4_tmp += (pi2_src2[col] + lvl_shift2) * wgt1_cb;
            i4_tmp += (off0_cb + off1_cb + 1) << (shift - 1);

            pu1_dst[col] = CLIP_U8(i4_tmp >> shift);

            i4_tmp = (pi2_src1[col + 1] + lvl_shift1) * wgt0_cr;
            i4_tmp += (pi2_src2[col + 1] + lvl_shift2) * wgt1_cr;
            i4_tmp += (off0_cr + off1_cr + 1) << (shift - 1);

            pu1_dst[col + 1] = CLIP_U8(i4_tmp >> shift);
        }

        pi2_src1 += src_strd1;
        pi2_src2 += src_strd2;
        pu1_dst += dst_strd;
    }
}
//WEIGHTED_PRED_CHROMA_BI

/**
*******************************************************************************
*
* @brief
*  Does default bi-weighted prediction on the arrays pointed by pi2_src1 and
* pi2_src2 and stores it at location  pointed by pi2_dst
*
* @par Description:
*  dst = ( (src1 + lvl_shift1) +  (src2 + lvl_shift2) +  1 << (shift - 1) )
* >> shift  where shift = 15 - BitDepth
*
* @param[in] pi2_src1
*  Pointer to source 1
*
* @param[in] pi2_src2
*  Pointer to source 2
*
* @param[out] pu1_dst
*  Pointer to destination
*
* @param[in] src_strd1
*  Source stride 1
*
* @param[in] src_strd2
*  Source stride 2
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] lvl_shift1
*  added before shift and offset
*
* @param[in] lvl_shift2
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_weighted_pred_bi_default(WORD16 *pi2_src1,
                                    WORD16 *pi2_src2,
                                    UWORD8 *pu1_dst,
                                    WORD32 src_strd1,
                                    WORD32 src_strd2,
                                    WORD32 dst_strd,
                                    WORD32 lvl_shift1,
                                    WORD32 lvl_shift2,
                                    WORD32 ht,
                                    WORD32 wd)
{
    WORD32 row, col;
    WORD32 i4_tmp;
    WORD32 shift;

    shift = SHIFT_14_MINUS_BIT_DEPTH + 1;
    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            i4_tmp = pi2_src1[col] + lvl_shift1;
            i4_tmp += pi2_src2[col] + lvl_shift2;
            i4_tmp += 1 << (shift - 1);

            pu1_dst[col] = CLIP_U8(i4_tmp >> shift);
        }

        pi2_src1 += src_strd1;
        pi2_src2 += src_strd2;
        pu1_dst += dst_strd;
    }
}
//WEIGHTED_PRED_BI_DEFAULT

/**
*******************************************************************************
*
* @brief
*  Does chroma default bi-weighted prediction on arrays pointed by pi2_src1 and
* pi2_src2 and stores it at location  pointed by pi2_dst
*
* @par Description:
*  dst = ( (src1 + lvl_shift1) +  (src2 + lvl_shift2) +  1 << (shift - 1) )
* >> shift  where shift = 15 - BitDepth
*
* @param[in] pi2_src1
*  Pointer to source 1
*
* @param[in] pi2_src2
*  Pointer to source 2
*
* @param[out] pu1_dst
*  Pointer to destination
*
* @param[in] src_strd1
*  Source stride 1
*
* @param[in] src_strd2
*  Source stride 2
*
* @param[in] dst_strd
*  Destination stride
*
* @param[in] lvl_shift1
*  added before shift and offset
*
* @param[in] lvl_shift2
*  added before shift and offset
*
* @param[in] ht
*  height of the source
*
* @param[in] wd
*  width of the source (each colour component)
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_weighted_pred_chroma_bi_default(WORD16 *pi2_src1,
                                           WORD16 *pi2_src2,
                                           UWORD8 *pu1_dst,
                                           WORD32 src_strd1,
                                           WORD32 src_strd2,
                                           WORD32 dst_strd,
                                           WORD32 lvl_shift1,
                                           WORD32 lvl_shift2,
                                           WORD32 ht,
                                           WORD32 wd)
{
    WORD32 row, col;
    WORD32 i4_tmp;
    WORD32 shift;

    shift = SHIFT_14_MINUS_BIT_DEPTH + 1;
    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < 2 * wd; col++)
        {
            i4_tmp = pi2_src1[col] + lvl_shift1;
            i4_tmp += pi2_src2[col] + lvl_shift2;
            i4_tmp += 1 << (shift - 1);

            pu1_dst[col] = CLIP_U8(i4_tmp >> shift);
        }

        pi2_src1 += src_strd1;
        pi2_src2 += src_strd2;
        pu1_dst += dst_strd;
    }
}
//WEIGHTED_PRED_CHROMA_BI_DEFAULT
