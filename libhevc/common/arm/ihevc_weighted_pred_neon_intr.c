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
*  ihevc_weighted_pred_neon_intr.c
*
* @brief
*  Contains function definitions for weighted prediction used in inter
* prediction
*
* @author
*  Parthiban V
*
* @par List of Functions:
*  - ihevc_weighted_pred_uni()
*  - ihevc_weighted_pred_bi()
*  - ihevc_weighted_pred_bi_default()
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
#include "ihevc_func_selector.h"
#include "ihevc_inter_pred.h"
#include "arm_neon.h"


/**
*******************************************************************************
*
* @brief
*  Does uni-weighted prediction on the array pointed by  pi2_src and stores
* it at the location pointed by pi2_dst Assumptions : The function is
* optimized considering the fact Width and  height are multiple of 2.
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

void ihevc_weighted_pred_uni_neonintr(WORD16 *pi2_src,
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
    int16x4_t pi2_src_val1;
    int16x4_t pi2_src_val2;
    int32x4_t i4_tmp1_t;
    int32x4_t i4_tmp2_t;
    int32x4_t sto_res_tmp1;
    uint16x4_t sto_res_tmp2;
    uint16x8_t sto_res_tmp3;
    uint8x8_t sto_res;
    int32x4_t tmp_lvl_shift_t;
    WORD32 tmp_shift = 0 - shift;
    int32x4_t tmp_shift_t;
    WORD16 *pi2_src_tmp;
    UWORD8 *pu1_dst_tmp;

    WORD32 tmp_lvl_shift = lvl_shift * wgt0 + (off0 << shift);
    tmp_lvl_shift += (1 << (shift - 1));
    tmp_lvl_shift_t = vmovq_n_s32(tmp_lvl_shift);
    tmp_shift_t = vmovq_n_s32(tmp_shift);

    /* Used i4_tmp1_t & i4_tmp1_t to process 2 rows at a time.                                  */
    /* height has also been unrolled, hence 2 rows will processed at a time                     */
    /* store also has been taken care for two row process                                       */
    /* vcombine_u16 has been used since after narrowing we get 16x4 value which can't be        */
    /* saturated and narrowed                                                                   */

    for(row = ht; row > 0; row -= 2)
    {
        for(col = wd; col > 0; col -= 4)
        {
            pi2_src_tmp = pi2_src + src_strd;

            pu1_dst_tmp = pu1_dst + dst_strd;

            pi2_src_val1 = vld1_s16((int16_t *)pi2_src);
            pi2_src += 4;

            pi2_src_val2 = vld1_s16((int16_t *)pi2_src_tmp);
            i4_tmp1_t = vmull_n_s16(pi2_src_val1, (int16_t)wgt0);

            i4_tmp1_t = vaddq_s32(i4_tmp1_t, tmp_lvl_shift_t);
            i4_tmp2_t = vmull_n_s16(pi2_src_val2, (int16_t)wgt0);

            sto_res_tmp1 = vshlq_s32(i4_tmp1_t, tmp_shift_t);
            i4_tmp2_t = vaddq_s32(i4_tmp2_t, tmp_lvl_shift_t);

            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            sto_res_tmp1 = vshlq_s32(i4_tmp2_t, tmp_shift_t);
            sto_res = vqmovn_u16(sto_res_tmp3);

            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            vst1_lane_u32((uint32_t *)pu1_dst, vreinterpret_u32_u8(sto_res), 0);
            pu1_dst += 4;

            sto_res = vqmovn_u16(sto_res_tmp3);
            vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(sto_res), 0);
        }
        pi2_src += 2 * src_strd - wd;
        pu1_dst += 2 * dst_strd - wd;
    }
}
//WEIGHTED_PRED_UNI

/**
*******************************************************************************
*
* @brief
* Chroma uni-weighted prediction on the array pointed by  pi2_src and stores
* it at the location pointed by pi2_dst Assumptions : The function is
* optimized considering the fact Width and  height are multiple of 2.
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

void ihevc_weighted_pred_chroma_uni_neonintr(WORD16 *pi2_src,
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
    int16x4_t pi2_src_val1;
    int16x4_t pi2_src_val2;
    int32x4_t i4_tmp1_t;
    int32x4_t i4_tmp2_t;
    int32x4_t sto_res_tmp1;
    uint16x4_t sto_res_tmp2;
    uint16x8_t sto_res_tmp3;
    uint8x8_t sto_res;
    int32x4_t tmp_lvl_shift_t_u, tmp_lvl_shift_t_v;
    int32x4x2_t tmp_lvl_shift_t;
    WORD32 tmp_shift = 0 - shift;
    int32x4_t tmp_shift_t;
    int16x4_t tmp_wgt0_u, tmp_wgt0_v;
    int16x4x2_t wgt0;
    WORD16 *pi2_src_tmp;
    UWORD8 *pu1_dst_tmp;

    WORD32 tmp_lvl_shift = lvl_shift * wgt0_cb + (off0_cb << shift);
    tmp_lvl_shift += (1 << (shift - 1));
    tmp_lvl_shift_t_u = vmovq_n_s32(tmp_lvl_shift);

    tmp_lvl_shift = lvl_shift * wgt0_cr + (off0_cr << shift);
    tmp_lvl_shift += (1 << (shift - 1));
    tmp_lvl_shift_t_v = vmovq_n_s32(tmp_lvl_shift);

    tmp_lvl_shift_t = vzipq_s32(tmp_lvl_shift_t_u, tmp_lvl_shift_t_v);

    tmp_shift_t = vmovq_n_s32(tmp_shift);

    tmp_wgt0_u = vdup_n_s16(wgt0_cb);
    tmp_wgt0_v = vdup_n_s16(wgt0_cr);
    wgt0 = vzip_s16(tmp_wgt0_u, tmp_wgt0_v);

    /* Used i4_tmp1_t & i4_tmp1_t to process 2 rows at a time.                                  */
    /* height has also been unrolled, hence 2 rows will processed at a time                     */
    /* store also has been taken care for two row process                                       */
    /* vcombine_u16 has been used since after narrowing we get 16x4 value which can't be        */
    /* saturated and narrowed                                                                   */

    for(row = ht; row > 0; row -= 2)
    {
        for(col = 2 * wd; col > 0; col -= 4)
        {
            pi2_src_tmp = pi2_src + src_strd;

            pu1_dst_tmp = pu1_dst + dst_strd;

            pi2_src_val1 = vld1_s16((int16_t *)pi2_src);
            pi2_src += 4;

            pi2_src_val2 = vld1_s16((int16_t *)pi2_src_tmp);
            i4_tmp1_t = vmull_s16(pi2_src_val1, wgt0.val[0]);

            i4_tmp1_t = vaddq_s32(i4_tmp1_t, tmp_lvl_shift_t.val[0]);
            i4_tmp2_t = vmull_s16(pi2_src_val2, wgt0.val[0]);

            sto_res_tmp1 = vshlq_s32(i4_tmp1_t, tmp_shift_t);
            i4_tmp2_t = vaddq_s32(i4_tmp2_t, tmp_lvl_shift_t.val[0]);

            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            sto_res_tmp1 = vshlq_s32(i4_tmp2_t, tmp_shift_t);
            sto_res = vqmovn_u16(sto_res_tmp3);

            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            vst1_lane_u32((uint32_t *)pu1_dst, vreinterpret_u32_u8(sto_res), 0);
            pu1_dst += 4;

            sto_res = vqmovn_u16(sto_res_tmp3);
            vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(sto_res), 0);
        }
        pi2_src += 2 * src_strd - 2 * wd;
        pu1_dst += 2 * dst_strd - 2 * wd;
    }
}
//WEIGHTED_PRED_CHROMA_UNI

/**
*******************************************************************************
*
* @brief
*  Does bi-weighted prediction on the arrays pointed by  pi2_src1 and
* pi2_src2 and stores it at location pointed  by pi2_dst   Assumptions : The
* function is optimized considering the fact Width and  height are multiple
* of 2.
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

void ihevc_weighted_pred_bi_neonintr(WORD16 *pi2_src1,
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
    int16x4_t pi2_src1_val1;
    int16x4_t pi2_src1_val2;
    int16x4_t pi2_src2_val1;
    int16x4_t pi2_src2_val2;
    int32x4_t i4_tmp1_t1;
    int32x4_t i4_tmp1_t2;
    int32x4_t i4_tmp2_t1;
    int32x4_t i4_tmp2_t2;
    int32x4_t sto_res_tmp1;
    uint16x4_t sto_res_tmp2;
    uint16x8_t sto_res_tmp3;
    uint8x8_t sto_res;
    int32x4_t tmp_lvl_shift_t;
    WORD32 tmp_shift = 0 - shift;
    int32x4_t tmp_shift_t;
    WORD16 *pi2_src_tmp1;
    WORD16 *pi2_src_tmp2;
    UWORD8 *pu1_dst_tmp;

    WORD32 tmp_lvl_shift = (lvl_shift1 * wgt0) + (lvl_shift2 * wgt1);
    tmp_lvl_shift += ((off0 + off1 + 1) << (shift - 1));
    tmp_lvl_shift_t = vmovq_n_s32(tmp_lvl_shift);
    tmp_shift_t = vmovq_n_s32(tmp_shift);

    /* Used i4_tmp1_t & i4_tmp1_t to process 2 rows at a time.                                  */
    /* height has also been unrolled, hence 2 rows will processed at a time                     */
    /* store also has been taken care for two row process                                       */
    /* vcombine_u16 has been used since after narrowing we get 16x4 value which can't be        */
    /* saturated and narrowed                                                                   */

    for(row = ht; row > 0; row -= 2)
    {
        for(col = wd; col > 0; col -= 4)
        {
            pi2_src_tmp1 = pi2_src1 + src_strd1;
            pi2_src_tmp2 = pi2_src2 + src_strd2;

            pi2_src1_val1 = vld1_s16((int16_t *)pi2_src1);
            pi2_src1 += 4;
            pu1_dst_tmp = pu1_dst + dst_strd;

            pi2_src2_val1 = vld1_s16((int16_t *)pi2_src2);
            pi2_src2 += 4;
            i4_tmp1_t1 = vmull_n_s16(pi2_src1_val1, (int16_t)wgt0);

            pi2_src1_val2 = vld1_s16((int16_t *)pi2_src_tmp1);
            i4_tmp1_t2 = vmull_n_s16(pi2_src2_val1, (int16_t)wgt1);

            pi2_src2_val2 = vld1_s16((int16_t *)pi2_src_tmp2);
            i4_tmp1_t1 = vaddq_s32(i4_tmp1_t1, i4_tmp1_t2);

            i4_tmp2_t1 = vmull_n_s16(pi2_src1_val2, (int16_t)wgt0);
            i4_tmp1_t1 = vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t);

            i4_tmp2_t2 = vmull_n_s16(pi2_src2_val2, (int16_t)wgt1);
            sto_res_tmp1 = vshlq_s32(i4_tmp1_t1, tmp_shift_t);

            i4_tmp2_t1 = vaddq_s32(i4_tmp2_t1, i4_tmp2_t2);
            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);

            i4_tmp2_t1 = vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            sto_res_tmp1 = vshlq_s32(i4_tmp2_t1, tmp_shift_t);
            sto_res = vqmovn_u16(sto_res_tmp3);

            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            vst1_lane_u32((uint32_t *)pu1_dst, vreinterpret_u32_u8(sto_res), 0);
            pu1_dst += 4;

            sto_res = vqmovn_u16(sto_res_tmp3);
            vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(sto_res), 0);
        }
        pi2_src1 += 2 * src_strd1 - wd;
        pi2_src2 += 2 * src_strd2 - wd;
        pu1_dst += 2 * dst_strd - wd;
    }
}
//WEIGHTED_PRED_BI

/**
*******************************************************************************
*
* @brief
*  Chroma bi-weighted prediction on the arrays pointed by  pi2_src1 and
* pi2_src2 and stores it at location pointed  by pi2_dst   Assumptions : The
* function is optimized considering the fact Width and  height are multiple
* of 2.
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

void ihevc_weighted_pred_chroma_bi_neonintr(WORD16 *pi2_src1,
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
    int16x4_t pi2_src1_val1;
    int16x4_t pi2_src1_val2;
    int16x4_t pi2_src2_val1;
    int16x4_t pi2_src2_val2;
    int32x4_t i4_tmp1_t1;
    int32x4_t i4_tmp1_t2;
    int32x4_t i4_tmp2_t1;
    int32x4_t i4_tmp2_t2;
    int32x4_t sto_res_tmp1;
    uint16x4_t sto_res_tmp2;
    uint16x8_t sto_res_tmp3;
    uint8x8_t sto_res;
    int32x4_t tmp_lvl_shift_t_u, tmp_lvl_shift_t_v;
    int32x4x2_t tmp_lvl_shift_t;
    WORD32 tmp_shift = 0 - shift;
    int32x4_t tmp_shift_t;
    int16x4_t tmp_wgt0_u, tmp_wgt0_v, tmp_wgt1_u, tmp_wgt1_v;
    int16x4x2_t wgt0, wgt1;
    WORD16 *pi2_src_tmp1;
    WORD16 *pi2_src_tmp2;
    UWORD8 *pu1_dst_tmp;

    WORD32 tmp_lvl_shift = (lvl_shift1 * wgt0_cb) + (lvl_shift2 * wgt1_cb);
    tmp_lvl_shift += ((off0_cb + off1_cb + 1) << (shift - 1));
    tmp_lvl_shift_t_u = vmovq_n_s32(tmp_lvl_shift);

    tmp_lvl_shift = (lvl_shift1 * wgt0_cr) + (lvl_shift2 * wgt1_cr);
    tmp_lvl_shift += ((off0_cr + off1_cr + 1) << (shift - 1));
    tmp_lvl_shift_t_v = vmovq_n_s32(tmp_lvl_shift);

    tmp_lvl_shift_t = vzipq_s32(tmp_lvl_shift_t_u, tmp_lvl_shift_t_v);

    tmp_shift_t = vmovq_n_s32(tmp_shift);

    tmp_wgt0_u = vdup_n_s16(wgt0_cb);
    tmp_wgt0_v = vdup_n_s16(wgt0_cr);
    wgt0 = vzip_s16(tmp_wgt0_u, tmp_wgt0_v);
    tmp_wgt1_u = vdup_n_s16(wgt1_cb);
    tmp_wgt1_v = vdup_n_s16(wgt1_cr);
    wgt1 = vzip_s16(tmp_wgt1_u, tmp_wgt1_v);

    /* Used i4_tmp1_t & i4_tmp1_t to process 2 rows at a time.                                  */
    /* height has also been unrolled, hence 2 rows will processed at a time                     */
    /* store also has been taken care for two row process                                       */
    /* vcombine_u16 has been used since after narrowing we get 16x4 value which can't be        */
    /* saturated and narrowed                                                                   */

    for(row = ht; row > 0; row -= 2)
    {
        for(col = 2 * wd; col > 0; col -= 4)
        {
            pi2_src_tmp1 = pi2_src1 + src_strd1;
            pi2_src_tmp2 = pi2_src2 + src_strd2;

            pi2_src1_val1 = vld1_s16((int16_t *)pi2_src1);
            pi2_src1 += 4;
            pu1_dst_tmp = pu1_dst + dst_strd;

            pi2_src2_val1 = vld1_s16((int16_t *)pi2_src2);
            pi2_src2 += 4;
            i4_tmp1_t1 = vmull_s16(pi2_src1_val1, wgt0.val[0]);

            pi2_src1_val2 = vld1_s16((int16_t *)pi2_src_tmp1);
            i4_tmp1_t2 = vmull_s16(pi2_src2_val1, wgt1.val[0]);

            pi2_src2_val2 = vld1_s16((int16_t *)pi2_src_tmp2);
            i4_tmp1_t1 = vaddq_s32(i4_tmp1_t1, i4_tmp1_t2);

            i4_tmp2_t1 = vmull_s16(pi2_src1_val2, wgt0.val[0]);
            i4_tmp1_t1 = vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t.val[0]);

            i4_tmp2_t2 = vmull_s16(pi2_src2_val2, wgt1.val[0]);
            sto_res_tmp1 = vshlq_s32(i4_tmp1_t1, tmp_shift_t);

            i4_tmp2_t1 = vaddq_s32(i4_tmp2_t1, i4_tmp2_t2);
            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);

            i4_tmp2_t1 = vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t.val[0]);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            sto_res_tmp1 = vshlq_s32(i4_tmp2_t1, tmp_shift_t);
            sto_res = vqmovn_u16(sto_res_tmp3);

            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            vst1_lane_u32((uint32_t *)pu1_dst, vreinterpret_u32_u8(sto_res), 0);
            pu1_dst += 4;

            sto_res = vqmovn_u16(sto_res_tmp3);
            vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(sto_res), 0);
        }
        pi2_src1 += 2 * src_strd1 - 2 * wd;
        pi2_src2 += 2 * src_strd2 - 2 * wd;
        pu1_dst += 2 * dst_strd - 2 * wd;
    }
}
//WEIGHTED_PRED_CHROMA_BI

/**
*******************************************************************************
*
* @brief
*  Does default bi-weighted prediction on the arrays pointed by pi2_src1 and
* pi2_src2 and stores it at location  pointed by pi2_dst Assumptions : The
* function is optimized considering the fact Width and  height are multiple
* of 2.
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

void ihevc_weighted_pred_bi_default_neonintr(WORD16 *pi2_src1,
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
    int16x4_t pi2_src1_val1;
    int16x4_t pi2_src1_val2;
    int16x4_t pi2_src2_val1;
    int16x4_t pi2_src2_val2;
    int32x4_t i4_tmp1_t1;
    int32x4_t i4_tmp1_t2;
    int32x4_t i4_tmp2_t1;
    int32x4_t i4_tmp2_t2;
    int32x4_t sto_res_tmp1;
    uint16x4_t sto_res_tmp2;
    uint16x8_t sto_res_tmp3;
    uint8x8_t sto_res;
    int32x4_t tmp_lvl_shift_t;
    int32x4_t tmp_shift_t;
    WORD16 *pi2_src_tmp1;
    WORD16 *pi2_src_tmp2;
    UWORD8 *pu1_dst_tmp;
    WORD32 shift;

    shift = SHIFT_14_MINUS_BIT_DEPTH + 1;
    WORD32 tmp_shift = 0 - shift;
    WORD32 tmp_lvl_shift = 1 << (shift - 1);
    tmp_lvl_shift_t = vmovq_n_s32(tmp_lvl_shift);
    tmp_shift_t = vmovq_n_s32(tmp_shift);

    int16x4_t lvl_shift1_t = vmov_n_s16((int16_t)lvl_shift1);
    int16x4_t lvl_shift2_t = vmov_n_s16((int16_t)lvl_shift2);

    /* Used i4_tmp1_t & i4_tmp1_t to process 2 rows at a time.                                  */
    /* height has also been unrolled, hence 2 rows will processed at a time                     */
    /* store also has been taken care for two row process                                       */
    /* vcombine_u16 has been used since after narrowing we get 16x4 value which can't be        */
    /* saturated and narrowed                                                                   */

    for(row = ht; row > 0; row -= 2)
    {
        for(col = wd; col > 0; col -= 4)
        {
            pi2_src_tmp1 = pi2_src1 + src_strd1;
            pi2_src_tmp2 = pi2_src2 + src_strd2;

            pi2_src1_val1 = vld1_s16((int16_t *)pi2_src1);
            pi2_src1 += 4;
            pu1_dst_tmp = pu1_dst + dst_strd;

            pi2_src2_val1 = vld1_s16((int16_t *)pi2_src2);
            pi2_src2 += 4;
            i4_tmp1_t1 = vaddl_s16(pi2_src1_val1, lvl_shift1_t);

            pi2_src1_val2 = vld1_s16((int16_t *)pi2_src_tmp1);
            i4_tmp1_t2 = vaddl_s16(pi2_src2_val1, lvl_shift2_t);

            pi2_src2_val2 = vld1_s16((int16_t *)pi2_src_tmp2);
            i4_tmp1_t1 = vaddq_s32(i4_tmp1_t1, i4_tmp1_t2);

            i4_tmp2_t1 = vaddl_s16(pi2_src1_val2, lvl_shift1_t);
            i4_tmp1_t1 = vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t);

            i4_tmp2_t2 = vaddl_s16(pi2_src2_val2, lvl_shift2_t);
            sto_res_tmp1 = vshlq_s32(i4_tmp1_t1, tmp_shift_t);

            i4_tmp2_t1 = vaddq_s32(i4_tmp2_t1, i4_tmp2_t2);
            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);

            i4_tmp2_t1 = vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            sto_res_tmp1 = vshlq_s32(i4_tmp2_t1, tmp_shift_t);
            sto_res = vqmovn_u16(sto_res_tmp3);

            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            vst1_lane_u32((uint32_t *)pu1_dst, vreinterpret_u32_u8(sto_res), 0);
            pu1_dst += 4;

            sto_res = vqmovn_u16(sto_res_tmp3);
            vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(sto_res), 0);
        }
        pi2_src1 += 2 * src_strd1 - wd;
        pi2_src2 += 2 * src_strd2 - wd;
        pu1_dst += 2 * dst_strd - wd;
    }
}
//WEIGHTED_PRED_BI_DEFAULT

/**
*******************************************************************************
*
* @brief
*  Does default bi-weighted prediction on the arrays pointed by pi2_src1 and
* pi2_src2 and stores it at location  pointed by pi2_dst Assumptions : The
* function is optimized considering the fact Width and  height are multiple
* of 2.
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

void ihevc_weighted_pred_chroma_bi_default_neonintr(WORD16 *pi2_src1,
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
    int16x4_t pi2_src1_val1;
    int16x4_t pi2_src1_val2;
    int16x4_t pi2_src2_val1;
    int16x4_t pi2_src2_val2;
    int32x4_t i4_tmp1_t1;
    int32x4_t i4_tmp1_t2;
    int32x4_t i4_tmp2_t1;
    int32x4_t i4_tmp2_t2;
    int32x4_t sto_res_tmp1;
    uint16x4_t sto_res_tmp2;
    uint16x8_t sto_res_tmp3;
    uint8x8_t sto_res;
    int32x4_t tmp_lvl_shift_t;
    int32x4_t tmp_shift_t;
    WORD16 *pi2_src_tmp1;
    WORD16 *pi2_src_tmp2;
    UWORD8 *pu1_dst_tmp;
    WORD32 shift;
    WORD32 tmp_shift;
    WORD32 tmp_lvl_shift;
    int16x4_t lvl_shift1_t;
    int16x4_t lvl_shift2_t;
    shift = SHIFT_14_MINUS_BIT_DEPTH + 1;
    tmp_shift = 0 - shift;
    tmp_lvl_shift = 1 << (shift - 1);
    tmp_lvl_shift_t = vmovq_n_s32(tmp_lvl_shift);
    tmp_shift_t = vmovq_n_s32(tmp_shift);

    lvl_shift1_t = vmov_n_s16((int16_t)lvl_shift1);
    lvl_shift2_t = vmov_n_s16((int16_t)lvl_shift2);

    /* Used i4_tmp1_t & i4_tmp1_t to process 2 rows at a time.                                  */
    /* height has also been unrolled, hence 2 rows will processed at a time                     */
    /* store also has been taken care for two row process                                       */
    /* vcombine_u16 has been used since after narrowing we get 16x4 value which can't be        */
    /* saturated and narrowed                                                                   */

    for(row = ht; row > 0; row -= 2)
    {
        for(col = 2 * wd; col > 0; col -= 4)
        {
            pi2_src_tmp1 = pi2_src1 + src_strd1;
            pi2_src_tmp2 = pi2_src2 + src_strd2;

            pi2_src1_val1 = vld1_s16((int16_t *)pi2_src1);
            pi2_src1 += 4;
            pu1_dst_tmp = pu1_dst + dst_strd;

            pi2_src2_val1 = vld1_s16((int16_t *)pi2_src2);
            pi2_src2 += 4;
            i4_tmp1_t1 = vaddl_s16(pi2_src1_val1, lvl_shift1_t);

            pi2_src1_val2 = vld1_s16((int16_t *)pi2_src_tmp1);
            i4_tmp1_t2 = vaddl_s16(pi2_src2_val1, lvl_shift2_t);

            pi2_src2_val2 = vld1_s16((int16_t *)pi2_src_tmp2);
            i4_tmp1_t1 = vaddq_s32(i4_tmp1_t1, i4_tmp1_t2);

            i4_tmp2_t1 = vaddl_s16(pi2_src1_val2, lvl_shift1_t);
            i4_tmp1_t1 = vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t);

            i4_tmp2_t2 = vaddl_s16(pi2_src2_val2, lvl_shift2_t);
            sto_res_tmp1 = vshlq_s32(i4_tmp1_t1, tmp_shift_t);

            i4_tmp2_t1 = vaddq_s32(i4_tmp2_t1, i4_tmp2_t2);
            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);

            i4_tmp2_t1 = vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            sto_res_tmp1 = vshlq_s32(i4_tmp2_t1, tmp_shift_t);
            sto_res = vqmovn_u16(sto_res_tmp3);

            sto_res_tmp2 = vqmovun_s32(sto_res_tmp1);
            sto_res_tmp3 = vcombine_u16(sto_res_tmp2, sto_res_tmp2);

            vst1_lane_u32((uint32_t *)pu1_dst, vreinterpret_u32_u8(sto_res), 0);
            pu1_dst += 4;

            sto_res = vqmovn_u16(sto_res_tmp3);
            vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(sto_res), 0);
        }
        pi2_src1 += 2 * src_strd1 - 2 * wd;
        pi2_src2 += 2 * src_strd2 - 2 * wd;
        pu1_dst += 2 * dst_strd - 2 * wd;
    }
}
//WEIGHTED_PRED_CHROMA_BI_DEFAULT
