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
*  ihevcd_frm_cvt_x86_intr.c
*
* @brief
*  Platform specific intrinsic implementation of certain functions
*
* @author
*  Ittiam
* @par List of Functions:
*  - ihevcd_itrans_recon_dc
*  - ihevcd_fmt_conv_420sp_to_420p
*
* @remarks
*  None
*
*******************************************************************************
*/
#include "string.h"
#include "ihevc_typedefs.h"
#include "ihevc_defs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevcd_function_selector.h"
#include <string.h>
#include <immintrin.h>


void ihevcd_fmt_conv_420sp_to_420p_ssse3(UWORD8 *pu1_y_src,
                                         UWORD8 *pu1_uv_src,
                                         UWORD8 *pu1_y_dst,
                                         UWORD8 *pu1_u_dst,
                                         UWORD8 *pu1_v_dst,
                                         WORD32 wd,
                                         WORD32 ht,
                                         WORD32 src_y_strd,
                                         WORD32 src_uv_strd,
                                         WORD32 dst_y_strd,
                                         WORD32 dst_uv_strd,
                                         WORD32 is_u_first,
                                         WORD32 disable_luma_copy)
{
    UWORD8 *pu1_src, *pu1_dst;
    UWORD8 *pu1_u_src, *pu1_v_src;
    WORD32 num_rows, num_cols, src_strd, dst_strd, cols, rows;
    WORD32 i, j;

    cols = 0;
    pu1_u_src = (UWORD8 *)pu1_uv_src;
    pu1_v_src = (UWORD8 *)pu1_uv_src + 1;
    if(0 == disable_luma_copy)
    {
        /* copy luma */
        pu1_src = (UWORD8 *)pu1_y_src;
        pu1_dst = (UWORD8 *)pu1_y_dst;

        num_rows = ht;
        num_cols = wd;

        src_strd = src_y_strd;
        dst_strd = dst_y_strd;
        for(i = 0; i < num_rows; i++)
        {
            memcpy(pu1_dst, pu1_src, num_cols);
            pu1_dst += dst_strd;
            pu1_src += src_strd;
        }
    }

    /* de-interleave U and V and copy to destination */
    if(!is_u_first)
    {
        UWORD8 *temp = pu1_u_dst;
        pu1_u_dst = pu1_v_dst;
        pu1_v_dst = temp;

        pu1_u_src = (UWORD8 *)pu1_uv_src + 1;
        pu1_v_src = (UWORD8 *)pu1_uv_src;
    }

    {
        __m128i src_uv0_8x16b, src_uv1_8x16b, src_u_8x16b, src_v_8x16b;
        __m128i temp0_8x16b, temp1_8x16b, alt_first_mask;

        UWORD8 FIRST_ALT_SHUFFLE[16] = {
            0x00, 0x02, 0x04, 0x06,
            0x08, 0x0A, 0x0C, 0x0E,
            0x01, 0x03, 0x05, 0x07,
            0x09, 0x0B, 0x0D, 0x0F };

        PREFETCH((char const *)(pu1_uv_src + (0 * src_uv_strd)), _MM_HINT_T0)
        PREFETCH((char const *)(pu1_uv_src + (1 * src_uv_strd)), _MM_HINT_T0)
        PREFETCH((char const *)(pu1_uv_src + (2 * src_uv_strd)), _MM_HINT_T0)
        PREFETCH((char const *)(pu1_uv_src + (3 * src_uv_strd)), _MM_HINT_T0)
        PREFETCH((char const *)(pu1_uv_src + (4 * src_uv_strd)), _MM_HINT_T0)
        PREFETCH((char const *)(pu1_uv_src + (5 * src_uv_strd)), _MM_HINT_T0)
        PREFETCH((char const *)(pu1_uv_src + (6 * src_uv_strd)), _MM_HINT_T0)
        PREFETCH((char const *)(pu1_uv_src + (7 * src_uv_strd)), _MM_HINT_T0)

        num_rows = ht >> 1;
        num_cols = wd >> 1;

        src_strd = src_uv_strd;
        dst_strd = dst_uv_strd;

        alt_first_mask = _mm_loadu_si128((__m128i *)&FIRST_ALT_SHUFFLE[0]);

        if(num_cols > 15)
        {
            cols = num_cols >> 4;

            for(i = 0; i < (num_rows >> 2); i++)
            {
                UWORD8 *pu1_uv_src_temp, *pu1_u_dst_temp, *pu1_v_dst_temp;

                PREFETCH((char const *)(pu1_uv_src + (8 * src_strd)), _MM_HINT_T0)
                PREFETCH((char const *)(pu1_uv_src + (9 * src_strd)), _MM_HINT_T0)
                PREFETCH((char const *)(pu1_uv_src + (10 * src_strd)), _MM_HINT_T0)
                PREFETCH((char const *)(pu1_uv_src + (11 * src_strd)), _MM_HINT_T0)

                pu1_uv_src_temp = pu1_uv_src;
                pu1_u_dst_temp =  pu1_u_dst;
                pu1_v_dst_temp =  pu1_v_dst;

                for(j = 0; j < cols; j++)
                {

                    /**** Row 0 ***/
                    src_uv0_8x16b = _mm_loadu_si128((__m128i *)pu1_uv_src_temp);
                    src_uv1_8x16b = _mm_loadu_si128((__m128i *)(pu1_uv_src_temp + 16));

                    temp0_8x16b = _mm_shuffle_epi8(src_uv0_8x16b, alt_first_mask);
                    temp1_8x16b = _mm_shuffle_epi8(src_uv1_8x16b, alt_first_mask);

                    src_u_8x16b = _mm_unpacklo_epi64(temp0_8x16b, temp1_8x16b);
                    src_v_8x16b = _mm_unpackhi_epi64(temp0_8x16b, temp1_8x16b);

                    _mm_storeu_si128((__m128i *)(pu1_u_dst_temp), src_u_8x16b);
                    _mm_storeu_si128((__m128i *)(pu1_v_dst_temp), src_v_8x16b);

                    /**** Row 1 ***/
                    src_uv0_8x16b = _mm_loadu_si128((__m128i *)(pu1_uv_src_temp + (1 * src_strd)));
                    src_uv1_8x16b = _mm_loadu_si128((__m128i *)(pu1_uv_src_temp + (1 * src_strd) + 16));

                    temp0_8x16b = _mm_shuffle_epi8(src_uv0_8x16b, alt_first_mask);
                    temp1_8x16b = _mm_shuffle_epi8(src_uv1_8x16b, alt_first_mask);

                    src_u_8x16b = _mm_unpacklo_epi64(temp0_8x16b, temp1_8x16b);
                    src_v_8x16b = _mm_unpackhi_epi64(temp0_8x16b, temp1_8x16b);

                    _mm_storeu_si128((__m128i *)(pu1_u_dst_temp + (1 * dst_strd)), src_u_8x16b);
                    _mm_storeu_si128((__m128i *)(pu1_v_dst_temp + (1 * dst_strd)), src_v_8x16b);

                    /**** Row 2 ***/
                    src_uv0_8x16b = _mm_loadu_si128((__m128i *)(pu1_uv_src_temp + (2 * src_strd)));
                    src_uv1_8x16b = _mm_loadu_si128((__m128i *)(pu1_uv_src_temp + (2 * src_strd) + 16));

                    temp0_8x16b = _mm_shuffle_epi8(src_uv0_8x16b, alt_first_mask);
                    temp1_8x16b = _mm_shuffle_epi8(src_uv1_8x16b, alt_first_mask);

                    src_u_8x16b = _mm_unpacklo_epi64(temp0_8x16b, temp1_8x16b);
                    src_v_8x16b = _mm_unpackhi_epi64(temp0_8x16b, temp1_8x16b);

                    _mm_storeu_si128((__m128i *)(pu1_u_dst_temp + (2 * dst_strd)), src_u_8x16b);
                    _mm_storeu_si128((__m128i *)(pu1_v_dst_temp + (2 * dst_strd)), src_v_8x16b);

                    /**** Row 3 ***/
                    src_uv0_8x16b = _mm_loadu_si128((__m128i *)(pu1_uv_src_temp + (3 * src_strd)));
                    src_uv1_8x16b = _mm_loadu_si128((__m128i *)(pu1_uv_src_temp + (3 * src_strd) + 16));

                    temp0_8x16b = _mm_shuffle_epi8(src_uv0_8x16b, alt_first_mask);
                    temp1_8x16b = _mm_shuffle_epi8(src_uv1_8x16b, alt_first_mask);

                    src_u_8x16b = _mm_unpacklo_epi64(temp0_8x16b, temp1_8x16b);
                    src_v_8x16b = _mm_unpackhi_epi64(temp0_8x16b, temp1_8x16b);

                    _mm_storeu_si128((__m128i *)(pu1_u_dst_temp + (3 * dst_strd)), src_u_8x16b);
                    _mm_storeu_si128((__m128i *)(pu1_v_dst_temp + (3 * dst_strd)), src_v_8x16b);

                    pu1_u_dst_temp += 16;
                    pu1_v_dst_temp += 16;
                    pu1_uv_src_temp += 32;
                }

                pu1_u_dst += 4 * dst_strd;
                pu1_v_dst += 4 * dst_strd;
                pu1_uv_src += 4 * src_strd;
                //pu1_v_src += src_strd;
            }
            rows = num_rows & 0x3;
            if(rows)
            {
                for(i = 0; i < rows; i++)
                {
                    UWORD8 *pu1_uv_src_temp, *pu1_u_dst_temp, *pu1_v_dst_temp;

                    pu1_uv_src_temp = pu1_uv_src;
                    pu1_u_dst_temp =  pu1_u_dst;
                    pu1_v_dst_temp =  pu1_v_dst;

                    for(j = 0; j < cols; j++)
                    {

                        src_uv0_8x16b = _mm_loadu_si128((__m128i *)pu1_uv_src_temp);
                        src_uv1_8x16b = _mm_loadu_si128((__m128i *)(pu1_uv_src_temp + 16));

                        temp0_8x16b = _mm_shuffle_epi8(src_uv0_8x16b, alt_first_mask);
                        temp1_8x16b = _mm_shuffle_epi8(src_uv1_8x16b, alt_first_mask);

                        src_u_8x16b = _mm_unpacklo_epi64(temp0_8x16b, temp1_8x16b);
                        src_v_8x16b = _mm_unpackhi_epi64(temp0_8x16b, temp1_8x16b);

                        _mm_storeu_si128((__m128i *)(pu1_u_dst_temp), src_u_8x16b);
                        _mm_storeu_si128((__m128i *)(pu1_v_dst_temp), src_v_8x16b);

                        pu1_u_dst_temp += 16;
                        pu1_v_dst_temp += 16;
                        pu1_uv_src_temp += 32;
                    }

                    pu1_u_dst += dst_strd;
                    pu1_v_dst += dst_strd;
                    pu1_uv_src += src_strd;
                }
            }
            pu1_u_dst -= (num_rows * dst_strd);
            pu1_v_dst -= (num_rows * dst_strd);
            num_cols &= 0x0F;
        }
        if(num_cols)
        {
            pu1_u_dst += (cols << 4);
            pu1_v_dst += (cols << 4);
            pu1_u_src += 2 * (cols << 4);
            pu1_v_src += 2 * (cols << 4);
            for(i = 0; i < num_rows; i++)
            {
                for(j = 0; j < num_cols; j++)
                {
                    pu1_u_dst[j] = pu1_u_src[j * 2];
                    pu1_v_dst[j] = pu1_v_src[j * 2];
                }

                pu1_u_dst += dst_strd;
                pu1_v_dst += dst_strd;
                pu1_u_src += src_strd;
                pu1_v_src += src_strd;
            }
        }
    }
    return;
}
