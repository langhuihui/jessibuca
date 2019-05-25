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
*  ihevc_deblk_edge_filter.c
*
* @brief
*  Contains function definitions for deblocking filters
*
* @author
*  Srinivas T
*
* @par List of Functions:
*   - ihevc_deblk_luma_vert()
*   - ihevc_deblk_luma_horz()
*   - ihevc_deblk_chroma_vert()
*   - ihevc_deblk_chroma_horz()
* @remarks
*  None
*
*******************************************************************************
*/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "ihevc_typedefs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_func_selector.h"
#include "ihevc_deblk.h"
#include "ihevc_deblk_tables.h"
#include "ihevc_debug.h"


/**
*******************************************************************************
*
* @brief
*       Decision process and filtering for the luma block vertical edge.
*
* @par Description:
*     The decision process for the luma block vertical edge is  carried out and
*     an appropriate filter is applied. The  boundary filter strength, bs should
*     be greater than 0.  The pcm flags and the transquant bypass flags should
*     be  taken care of by the calling function.
*
* @param[in] pu1_src
*  Pointer to the src sample q(0,0)
*
* @param[in] src_strd
*  Source stride
*
* @param[in] bs
*  Boundary filter strength of q(0,0)
*
* @param[in] quant_param_p
*  quantization parameter of p block
*
* @param[in] quant_param_q
*  quantization parameter of p block
*
* @param[in] beta_offset_div2
*
*
* @param[in] tc_offset_div2
*
*
* @param[in] filter_flag_p
*  flag whether to filter the p block
*
* @param[in] filter_flag_q
*  flag whether to filter the q block
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_deblk_luma_vert(UWORD8 *pu1_src,
                           WORD32 src_strd,
                           WORD32 bs,
                           WORD32 quant_param_p,
                           WORD32 quant_param_q,
                           WORD32 beta_offset_div2,
                           WORD32 tc_offset_div2,
                           WORD32 filter_flag_p,
                           WORD32 filter_flag_q)
{
    WORD32 qp_luma, beta_indx, tc_indx;
    WORD32 beta, tc;
    WORD32 dp0, dp3, dq0, dq3, d0, d3, dp, dq, d;
    WORD32 d_sam0, d_sam3;
    WORD32 de, dep, deq;
    WORD32 row;
    WORD32 tmp_p0, tmp_p1, tmp_p2, tmp_q0, tmp_q1, tmp_q2;
    WORD32 delta, delta_p, delta_q;

    ASSERT((bs > 0) && (bs <= 3));
    ASSERT(filter_flag_p || filter_flag_q);

    qp_luma = (quant_param_p + quant_param_q + 1) >> 1;
    beta_indx = CLIP3(qp_luma + (beta_offset_div2 << 1), 0, 51);

    /* BS based on implementation can take value 3 if it is intra/inter egde          */
    /* based on BS, tc index is calcuated by adding 2 * ( bs - 1) to QP and tc_offset */
    /* for BS = 1 adding factor is (0*2), BS = 2 or 3 adding factor is (1*2)          */
    /* the above desired functionallity is achieved by doing (2*(bs>>1))              */

    tc_indx = CLIP3(qp_luma + (2 * (bs >> 1)) + (tc_offset_div2 << 1), 0, 53);

    beta = gai4_ihevc_beta_table[beta_indx];
    tc = gai4_ihevc_tc_table[tc_indx];
    if(0 == tc)
    {
        return;
    }

    dq0 = ABS(pu1_src[2] - 2 * pu1_src[1] + pu1_src[0]);
    dq3 = ABS(pu1_src[3 * src_strd + 2] - 2 * pu1_src[3 * src_strd + 1]
                    + pu1_src[3 * src_strd + 0]);
    dp0 = ABS(pu1_src[-3] - 2 * pu1_src[-2] + pu1_src[-1]);
    dp3 = ABS(pu1_src[3 * src_strd - 3] - 2 * pu1_src[3 * src_strd - 2]
                    + pu1_src[3 * src_strd - 1]);

    d0 = dp0 + dq0;
    d3 = dp3 + dq3;

    dp = dp0 + dp3;
    dq = dq0 + dq3;

    d = d0 + d3;

    de = 0;
    dep = 0;
    deq = 0;

    if(d < beta)
    {
        d_sam0 = 0;
        if((2 * d0 < (beta >> 2))
                        && (ABS(pu1_src[3] - pu1_src[0]) + ABS(pu1_src[-1] - pu1_src[-4])
                                        < (beta >> 3))
                        && ABS(pu1_src[0] - pu1_src[-1]) < ((5 * tc + 1) >> 1))
        {
            d_sam0 = 1;
        }

        pu1_src += 3 * src_strd;
        d_sam3 = 0;
        if((2 * d3 < (beta >> 2))
                        && (ABS(pu1_src[3] - pu1_src[0]) + ABS(pu1_src[-1] - pu1_src[-4])
                                        < (beta >> 3))
                        && ABS(pu1_src[0] - pu1_src[-1]) < ((5 * tc + 1) >> 1))
        {
            d_sam3 = 1;
        }
        pu1_src -= 3 * src_strd;

        de = (d_sam0 == 1 && d_sam3 == 1) ? 2 : 1;
        dep = (dp < (beta + (beta >> 1)) >> 3) ? 1 : 0;
        deq = (dq < (beta + (beta >> 1)) >> 3) ? 1 : 0;
        if(tc <= 1)
        {
            dep = 0;
            deq = 0;
        }
    }

    if(de != 0)
    {
        for(row = 0; row < 4; row++)
        {
            tmp_p0 = pu1_src[-1];
            tmp_p1 = pu1_src[-2];
            tmp_p2 = pu1_src[-3];

            tmp_q0 = pu1_src[0];
            tmp_q1 = pu1_src[1];
            tmp_q2 = pu1_src[2];

            if(de == 2)
            {
                tmp_q0 = CLIP3((pu1_src[2] + 2 * pu1_src[1] +
                                2 * pu1_src[0] + 2 * pu1_src[-1] +
                                pu1_src[-2] + 4) >> 3,
                                pu1_src[0] - 2 * tc,
                                pu1_src[0] + 2 * tc);

                tmp_q1 = CLIP3((pu1_src[2] + pu1_src[1] + pu1_src[0] +
                                pu1_src[-1] + 2) >> 2,
                                pu1_src[1] - 2 * tc,
                                pu1_src[1] + 2 * tc);

                tmp_q2 = CLIP3((2 * pu1_src[3] + 3 * pu1_src[2] +
                                pu1_src[1] + pu1_src[0] +
                                pu1_src[-1] + 4) >> 3,
                                pu1_src[2] - 2 * tc,
                                pu1_src[2] + 2 * tc);

                tmp_p0 = CLIP3((pu1_src[1] + 2 * pu1_src[0] +
                                2 * pu1_src[-1] + 2 * pu1_src[-2] +
                                pu1_src[-3] + 4) >> 3,
                                pu1_src[-1] - 2 * tc,
                                pu1_src[-1] + 2 * tc);

                tmp_p1 = CLIP3((pu1_src[0] + pu1_src[-1] +
                                pu1_src[-2] + pu1_src[-3] + 2) >> 2,
                                pu1_src[-2] - 2 * tc,
                                pu1_src[-2] + 2 * tc);

                tmp_p2 = CLIP3((pu1_src[0] + pu1_src[-1] +
                                pu1_src[-2] + 3 * pu1_src[-3] +
                                2 * pu1_src[-4] + 4) >> 3,
                                pu1_src[-3] - 2 * tc,
                                pu1_src[-3] + 2 * tc);
            }
            else
            {
                delta = (9 * (pu1_src[0] - pu1_src[-1]) -
                                3 * (pu1_src[1] - pu1_src[-2]) + 8) >> 4;
                if(ABS(delta) < 10 * tc)
                {
                    delta = CLIP3(delta, -tc, tc);

                    tmp_p0 = CLIP_U8(pu1_src[-1] + delta);
                    tmp_q0 = CLIP_U8(pu1_src[0] - delta);

                    if(dep == 1)
                    {
                        delta_p = CLIP3((((pu1_src[-3] + pu1_src[-1] + 1) >> 1)
                                        - pu1_src[-2] + delta) >> 1,
                                        -(tc >> 1),
                                        (tc >> 1));
                        tmp_p1 = CLIP_U8(pu1_src[-2] + delta_p);
                    }

                    if(deq == 1)
                    {
                        delta_q = CLIP3((((pu1_src[2] + pu1_src[0] + 1) >> 1)
                                        - pu1_src[1] - delta) >> 1,
                                        -(tc >> 1),
                                        (tc >> 1));
                        tmp_q1 = CLIP_U8(pu1_src[1] + delta_q);
                    }
                }
            }

            if(filter_flag_p != 0)
            {
                pu1_src[-3] = tmp_p2;
                pu1_src[-2] = tmp_p1;
                pu1_src[-1] = tmp_p0;
            }

            if(filter_flag_q != 0)
            {
                pu1_src[0] = tmp_q0;
                pu1_src[1] = tmp_q1;
                pu1_src[2] = tmp_q2;
            }

            pu1_src += src_strd;
        }
    }

}

/**
*******************************************************************************
*
* @brief
*
*     Decision process and filtering for the luma block horizontal edge
*
* @par Description:
*     The decision process for the luma block horizontal edge  is carried out
*    and an appropriate filter is applied. The  boundary filter strength, bs
*    should be greater than 0.  The pcm flags and the transquant bypass flags
*    should be  taken care of by the calling function.
*
* @param[in] pu1_src
*  Pointer to the src sample q(0,0)
*
* @param[in] src_strd
*  Source stride
*
* @param[in] bs
*  Boundary filter strength of q(0,0)
*
* @param[in] quant_param_p
*  quantization parameter of p block
*
* @param[in] quant_param_q
*  quantization parameter of p block
*
* @param[in] beta_offset_div2
*
*
* @param[in] tc_offset_div2
*
*
* @param[in] filter_flag_p
*  flag whether to filter the p block
*
* @param[in] filter_flag_q
*  flag whether to filter the q block
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_deblk_luma_horz(UWORD8 *pu1_src,
                           WORD32 src_strd,
                           WORD32 bs,
                           WORD32 quant_param_p,
                           WORD32 quant_param_q,
                           WORD32 beta_offset_div2,
                           WORD32 tc_offset_div2,
                           WORD32 filter_flag_p,
                           WORD32 filter_flag_q)
{
    WORD32 qp_luma, beta_indx, tc_indx;
    WORD32 beta, tc;
    WORD32 dp0, dp3, dq0, dq3, d0, d3, dp, dq, d;
    WORD32 d_sam0, d_sam3;
    WORD32 de, dep, deq;
    WORD32 col;
    WORD32 tmp_p0, tmp_p1, tmp_p2, tmp_q0, tmp_q1, tmp_q2;
    WORD32 delta, delta_p, delta_q;

    ASSERT((bs > 0));
    ASSERT(filter_flag_p || filter_flag_q);

    qp_luma = (quant_param_p + quant_param_q + 1) >> 1;
    beta_indx = CLIP3(qp_luma + (beta_offset_div2 << 1), 0, 51);

    /* BS based on implementation can take value 3 if it is intra/inter egde          */
    /* based on BS, tc index is calcuated by adding 2 * ( bs - 1) to QP and tc_offset */
    /* for BS = 1 adding factor is (0*2), BS = 2 or 3 adding factor is (1*2)          */
    /* the above desired functionallity is achieved by doing (2*(bs>>1))              */

    tc_indx = CLIP3(qp_luma + 2 * (bs >> 1) + (tc_offset_div2 << 1), 0, 53);

    beta = gai4_ihevc_beta_table[beta_indx];
    tc = gai4_ihevc_tc_table[tc_indx];
    if(0 == tc)
    {
        return;
    }

    dq0 = ABS(pu1_src[2 * src_strd] - 2 * pu1_src[1 * src_strd] +
                    pu1_src[0 * src_strd]);

    dq3 = ABS(pu1_src[3 + 2 * src_strd] - 2 * pu1_src[3 + 1 * src_strd] +
                    pu1_src[3 + 0 * src_strd]);

    dp0 = ABS(pu1_src[-3 * src_strd] - 2 * pu1_src[-2 * src_strd] +
                    pu1_src[-1 * src_strd]);

    dp3 = ABS(pu1_src[3 - 3 * src_strd] - 2 * pu1_src[3 - 2 * src_strd] +
                    pu1_src[3 - 1 * src_strd]);

    d0 = dp0 + dq0;
    d3 = dp3 + dq3;

    dp = dp0 + dp3;
    dq = dq0 + dq3;

    d = d0 + d3;

    de = 0;
    dep = 0;
    deq = 0;

    if(d < beta)
    {
        d_sam0 = 0;
        if((2 * d0 < (beta >> 2))
                        && (ABS(pu1_src[3 * src_strd] - pu1_src[0 * src_strd]) +
                                        ABS(pu1_src[-1 * src_strd] - pu1_src[-4 * src_strd])
                                        < (beta >> 3))
                        && ABS(pu1_src[0 * src_strd] - pu1_src[-1 * src_strd])
                        < ((5 * tc + 1) >> 1))
        {
            d_sam0 = 1;
        }

        pu1_src += 3;
        d_sam3 = 0;
        if((2 * d3 < (beta >> 2))
                        && (ABS(pu1_src[3 * src_strd] - pu1_src[0 * src_strd]) +
                                        ABS(pu1_src[-1 * src_strd] - pu1_src[-4 * src_strd])
                                        < (beta >> 3))
                        && ABS(pu1_src[0 * src_strd] - pu1_src[-1 * src_strd])
                        < ((5 * tc + 1) >> 1))
        {
            d_sam3 = 1;
        }
        pu1_src -= 3;

        de = (d_sam0 == 1 && d_sam3 == 1) ? 2 : 1;
        dep = (dp < ((beta + (beta >> 1)) >> 3)) ? 1 : 0;
        deq = (dq < ((beta + (beta >> 1)) >> 3)) ? 1 : 0;
        if(tc <= 1)
        {
            dep = 0;
            deq = 0;
        }
    }

    if(de != 0)
    {
        for(col = 0; col < 4; col++)
        {
            tmp_p0 = pu1_src[-1 * src_strd];
            tmp_p1 = pu1_src[-2 * src_strd];
            tmp_p2 = pu1_src[-3 * src_strd];

            tmp_q0 = pu1_src[0 * src_strd];
            tmp_q1 = pu1_src[1 * src_strd];
            tmp_q2 = pu1_src[2 * src_strd];
            if(de == 2)
            {
                tmp_q0 = CLIP3((pu1_src[2 * src_strd] +
                                2 * pu1_src[1 * src_strd] +
                                2 * pu1_src[0 * src_strd] +
                                2 * pu1_src[-1 * src_strd] +
                                pu1_src[-2 * src_strd] + 4) >> 3,
                                pu1_src[0 * src_strd] - 2 * tc,
                                pu1_src[0 * src_strd] + 2 * tc);

                tmp_q1 = CLIP3((pu1_src[2 * src_strd] +
                                pu1_src[1 * src_strd] +
                                pu1_src[0 * src_strd] +
                                pu1_src[-1 * src_strd] + 2) >> 2,
                                pu1_src[1 * src_strd] - 2 * tc,
                                pu1_src[1 * src_strd] + 2 * tc);

                tmp_q2 = CLIP3((2 * pu1_src[3 * src_strd] +
                                3 * pu1_src[2 * src_strd] +
                                pu1_src[1 * src_strd] +
                                pu1_src[0 * src_strd] +
                                pu1_src[-1 * src_strd] + 4) >> 3,
                                pu1_src[2 * src_strd] - 2 * tc,
                                pu1_src[2 * src_strd] + 2 * tc);

                tmp_p0 = CLIP3((pu1_src[1 * src_strd] +
                                2 * pu1_src[0 * src_strd] +
                                2 * pu1_src[-1 * src_strd] +
                                2 * pu1_src[-2 * src_strd] +
                                pu1_src[-3 * src_strd] + 4) >> 3,
                                pu1_src[-1 * src_strd] - 2 * tc,
                                pu1_src[-1 * src_strd] + 2 * tc);

                tmp_p1 = CLIP3((pu1_src[0 * src_strd] +
                                pu1_src[-1 * src_strd] +
                                pu1_src[-2 * src_strd] +
                                pu1_src[-3 * src_strd] + 2) >> 2,
                                pu1_src[-2 * src_strd] - 2 * tc,
                                pu1_src[-2 * src_strd] + 2 * tc);

                tmp_p2 = CLIP3((pu1_src[0 * src_strd] +
                                pu1_src[-1 * src_strd] +
                                pu1_src[-2 * src_strd] +
                                3 * pu1_src[-3 * src_strd] +
                                2 * pu1_src[-4 * src_strd] + 4) >> 3,
                                pu1_src[-3 * src_strd] - 2 * tc,
                                pu1_src[-3 * src_strd] + 2 * tc);
            }
            else
            {
                delta = (9 * (pu1_src[0 * src_strd] - pu1_src[-1 * src_strd]) -
                                3 * (pu1_src[1 * src_strd] - pu1_src[-2 * src_strd]) +
                                8) >> 4;
                if(ABS(delta) < 10 * tc)
                {
                    delta = CLIP3(delta, -tc, tc);

                    tmp_p0 = CLIP_U8(pu1_src[-1 * src_strd] + delta);
                    tmp_q0 = CLIP_U8(pu1_src[0 * src_strd] - delta);

                    if(dep == 1)
                    {
                        delta_p = CLIP3((((pu1_src[-3 * src_strd] +
                                        pu1_src[-1 * src_strd] + 1) >> 1) -
                                        pu1_src[-2 * src_strd] + delta) >> 1,
                                        -(tc >> 1),
                                        (tc >> 1));
                        tmp_p1 = CLIP_U8(pu1_src[-2 * src_strd] + delta_p);
                    }

                    if(deq == 1)
                    {
                        delta_q = CLIP3((((pu1_src[2 * src_strd] +
                                        pu1_src[0 * src_strd] + 1) >> 1) -
                                        pu1_src[1 * src_strd] - delta) >> 1,
                                        -(tc >> 1),
                                        (tc >> 1));
                        tmp_q1 = CLIP_U8(pu1_src[1 * src_strd] + delta_q);
                    }
                }
            }

            if(filter_flag_p != 0)
            {
                pu1_src[-3 * src_strd] = tmp_p2;
                pu1_src[-2 * src_strd] = tmp_p1;
                pu1_src[-1 * src_strd] = tmp_p0;
            }

            if(filter_flag_q != 0)
            {
                pu1_src[0 * src_strd] = tmp_q0;
                pu1_src[1 * src_strd] = tmp_q1;
                pu1_src[2 * src_strd] = tmp_q2;
            }

            pu1_src += 1;
        }
    }

}

/**
*******************************************************************************
*
* @brief
*     Filtering for the chroma block vertical edge.
*
* @par Description:
*     Filter for chroma vertical edge. The  boundary filter strength, bs
*    should be greater than 1.  The pcm flags and the transquant bypass flags
*    should be  taken care of by the calling function.
*
* @param[in] pu1_src
*  Pointer to the src sample q(0,0)
*
* @param[in] src_strd
*  Source stride
*
* @param[in] bs
*  Boundary filter strength of q(0,0)
*
* @param[in] quant_param_p
*  quantization parameter of p block
*
* @param[in] quant_param_q
*  quantization parameter of p block
*
* @param[in] beta_offset_div2
*
*
* @param[in] tc_offset_div2
*
*
* @param[in] filter_flag_p
*  flag whether to filter the p block
*
* @param[in] filter_flag_q
*  flag whether to filter the q block
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_deblk_chroma_vert(UWORD8 *pu1_src,
                             WORD32 src_strd,
                             WORD32 quant_param_p,
                             WORD32 quant_param_q,
                             WORD32 qp_offset_u,
                             WORD32 qp_offset_v,
                             WORD32 tc_offset_div2,
                             WORD32 filter_flag_p,
                             WORD32 filter_flag_q)
{
    WORD32 qp_indx_u, qp_chroma_u;
    WORD32 qp_indx_v, qp_chroma_v;
    WORD32 tc_indx_u, tc_u;
    WORD32 tc_indx_v, tc_v;
    WORD32 delta_u, tmp_p0_u, tmp_q0_u;
    WORD32 delta_v, tmp_p0_v, tmp_q0_v;
    WORD32 row;

    ASSERT(filter_flag_p || filter_flag_q);

    /* chroma processing is done only if BS is 2             */
    /* this function is assumed to be called only if BS is 2 */
    qp_indx_u = qp_offset_u + ((quant_param_p + quant_param_q + 1) >> 1);
    qp_chroma_u = qp_indx_u < 0 ? qp_indx_u : (qp_indx_u > 57 ? qp_indx_u - 6 : gai4_ihevc_qp_table[qp_indx_u]);

    qp_indx_v = qp_offset_v + ((quant_param_p + quant_param_q + 1) >> 1);
    qp_chroma_v = qp_indx_v < 0 ? qp_indx_v : (qp_indx_v > 57 ? qp_indx_v - 6 : gai4_ihevc_qp_table[qp_indx_v]);

    tc_indx_u = CLIP3(qp_chroma_u + 2 + (tc_offset_div2 << 1), 0, 53);
    tc_u = gai4_ihevc_tc_table[tc_indx_u];

    tc_indx_v = CLIP3(qp_chroma_v + 2 + (tc_offset_div2 << 1), 0, 53);
    tc_v = gai4_ihevc_tc_table[tc_indx_v];

    if(0 == tc_u && 0 == tc_v)
    {
        return;
    }

    for(row = 0; row < 4; row++)
    {
        delta_u = CLIP3((((pu1_src[0] - pu1_src[-2]) << 2) +
                        pu1_src[-4] - pu1_src[2] + 4) >> 3,
                        -tc_u, tc_u);

        tmp_p0_u = CLIP_U8(pu1_src[-2] + delta_u);
        tmp_q0_u = CLIP_U8(pu1_src[0] - delta_u);

        delta_v = CLIP3((((pu1_src[1] - pu1_src[-1]) << 2) +
                        pu1_src[-3] - pu1_src[3] + 4) >> 3,
                        -tc_v, tc_v);

        tmp_p0_v = CLIP_U8(pu1_src[-1] + delta_v);
        tmp_q0_v = CLIP_U8(pu1_src[1] - delta_v);

        if(filter_flag_p != 0)
        {
            pu1_src[-2] = tmp_p0_u;
            pu1_src[-1] = tmp_p0_v;
        }

        if(filter_flag_q != 0)
        {
            pu1_src[0] = tmp_q0_u;
            pu1_src[1] = tmp_q0_v;
        }

        pu1_src += src_strd;
    }

}



/**
*******************************************************************************
*
* @brief
*   Filtering for the chroma block horizontal edge.
*
* @par Description:
*     Filter for chroma horizontal edge. The  boundary filter strength, bs
*    should be greater than 1.  The pcm flags and the transquant bypass flags
*    should be  taken care of by the calling function.
*
* @param[in] pu1_src
*  Pointer to the src sample q(0,0)
*
* @param[in] src_strd
*  Source stride
*
* @param[in] bs
*  Boundary filter strength of q(0,0)
*
* @param[in] quant_param_p
*  quantization parameter of p block
*
* @param[in] quant_param_q
*  quantization parameter of p block
*
* @param[in] beta_offset_div2
*
*
* @param[in] tc_offset_div2
*
*
* @param[in] filter_flag_p
*  flag whether to filter the p block
*
* @param[in] filter_flag_q
*  flag whether to filter the q block
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_deblk_chroma_horz(UWORD8 *pu1_src,
                             WORD32 src_strd,
                             WORD32 quant_param_p,
                             WORD32 quant_param_q,
                             WORD32 qp_offset_u,
                             WORD32 qp_offset_v,
                             WORD32 tc_offset_div2,
                             WORD32 filter_flag_p,
                             WORD32 filter_flag_q)
{
    WORD32 qp_indx_u, qp_chroma_u;
    WORD32 qp_indx_v, qp_chroma_v;
    WORD32 tc_indx_u, tc_u;
    WORD32 tc_indx_v, tc_v;
    WORD32 tc;

    WORD32 delta, tmp_p0, tmp_q0;
    WORD32 col;

    ASSERT(filter_flag_p || filter_flag_q);

    /* chroma processing is done only if BS is 2             */
    /* this function is assumed to be called only if BS is 2 */
    qp_indx_u = qp_offset_u + ((quant_param_p + quant_param_q + 1) >> 1);
    qp_chroma_u = qp_indx_u < 0 ? qp_indx_u : (qp_indx_u > 57 ? qp_indx_u - 6 : gai4_ihevc_qp_table[qp_indx_u]);

    qp_indx_v = qp_offset_v + ((quant_param_p + quant_param_q + 1) >> 1);
    qp_chroma_v = qp_indx_v < 0 ? qp_indx_v : (qp_indx_v > 57 ? qp_indx_v - 6 : gai4_ihevc_qp_table[qp_indx_v]);

    tc_indx_u = CLIP3(qp_chroma_u + 2 + (tc_offset_div2 << 1), 0, 53);
    tc_u = gai4_ihevc_tc_table[tc_indx_u];

    tc_indx_v = CLIP3(qp_chroma_v + 2 + (tc_offset_div2 << 1), 0, 53);
    tc_v = gai4_ihevc_tc_table[tc_indx_v];

    if(0 == tc_u && 0 == tc_v)
    {
        return;
    }

    for(col = 0; col < 8; col++)
    {
        tc = (col & 1) ? tc_v : tc_u;
        delta = CLIP3((((pu1_src[0 * src_strd] -
                      pu1_src[-1 * src_strd]) << 2) +
                      pu1_src[-2 * src_strd] -
                      pu1_src[1 * src_strd] + 4) >> 3,
                      -tc, tc);

        tmp_p0 = CLIP_U8(pu1_src[-1 * src_strd] + delta);
        tmp_q0 = CLIP_U8(pu1_src[0 * src_strd] - delta);

        if(filter_flag_p != 0)
        {
            pu1_src[-1 * src_strd] = tmp_p0;
        }

        if(filter_flag_q != 0)
        {
            pu1_src[0 * src_strd] = tmp_q0;
        }

        pu1_src += 1;
    }

}

