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
 *  ihevcd_structs.h
 *
 * @brief
 *  Structure definitions used in the decoder
 *
 * @author
 *  Harish
 *
 * @par List of Functions:
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

#ifndef _IHEVCD_FMT_CONV_H_
#define _IHEVCD_FMT_CONV_H_

#define COEFF1          13073
#define COEFF2          -3207
#define COEFF3          -6664
#define COEFF4          16530

typedef void ihevcd_fmt_conv_420sp_to_rgba8888_ft(UWORD8 *pu1_y_src,
                                                  UWORD8 *pu1_uv_src,
                                                  UWORD32 *pu4_rgba_dst,
                                                  WORD32 wd,
                                                  WORD32 ht,
                                                  WORD32 src_y_strd,
                                                  WORD32 src_uv_strd,
                                                  WORD32 dst_strd,
                                                  WORD32 is_u_first);

typedef void ihevcd_fmt_conv_420sp_to_rgb565_ft(UWORD8 *pu1_y_src,
                                                UWORD8 *pu1_uv_src,
                                                UWORD16 *pu2_rgb_dst,
                                                WORD32 wd,
                                                WORD32 ht,
                                                WORD32 src_y_strd,
                                                WORD32 src_uv_strd,
                                                WORD32 dst_strd,
                                                WORD32 is_u_first);


typedef void ihevcd_fmt_conv_420sp_to_420sp_ft(UWORD8 *pu1_y_src,
                                               UWORD8 *pu1_uv_src,
                                               UWORD8 *pu1_y_dst,
                                               UWORD8 *pu1_uv_dst,
                                               WORD32 wd,
                                               WORD32 ht,
                                               WORD32 src_y_strd,
                                               WORD32 src_uv_strd,
                                               WORD32 dst_y_strd,
                                               WORD32 dst_uv_strd);
typedef void ihevcd_fmt_conv_420sp_to_420p_ft(UWORD8 *pu1_y_src,
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
                                              WORD32 disable_luma_copy);

/* C function declarations */
ihevcd_fmt_conv_420sp_to_rgba8888_ft ihevcd_fmt_conv_420sp_to_rgba8888;
ihevcd_fmt_conv_420sp_to_rgb565_ft ihevcd_fmt_conv_420sp_to_rgb565;
ihevcd_fmt_conv_420sp_to_420sp_ft ihevcd_fmt_conv_420sp_to_420sp;
ihevcd_fmt_conv_420sp_to_420p_ft ihevcd_fmt_conv_420sp_to_420p;

/* A9Q function declarations */
ihevcd_fmt_conv_420sp_to_rgba8888_ft ihevcd_fmt_conv_420sp_to_rgba8888_a9q;
ihevcd_fmt_conv_420sp_to_420sp_ft ihevcd_fmt_conv_420sp_to_420sp_a9q;
ihevcd_fmt_conv_420sp_to_420p_ft ihevcd_fmt_conv_420sp_to_420p_a9q;

/* A9A function declarations */
ihevcd_fmt_conv_420sp_to_rgba8888_ft ihevcd_fmt_conv_420sp_to_rgba8888_a9a;
ihevcd_fmt_conv_420sp_to_420sp_ft ihevcd_fmt_conv_420sp_to_420sp_a9a;
ihevcd_fmt_conv_420sp_to_420p_ft ihevcd_fmt_conv_420sp_to_420p_a9a;

/* SSSe31 function declarations */
ihevcd_fmt_conv_420sp_to_420p_ft ihevcd_fmt_conv_420sp_to_420p_ssse3;

/* SSE4 function declarations */
ihevcd_fmt_conv_420sp_to_420p_ft ihevcd_fmt_conv_420sp_to_420p_sse42;

/* armv8 function declarations */
ihevcd_fmt_conv_420sp_to_rgba8888_ft ihevcd_fmt_conv_420sp_to_rgba8888_av8;
ihevcd_fmt_conv_420sp_to_420sp_ft ihevcd_fmt_conv_420sp_to_420sp_av8;
ihevcd_fmt_conv_420sp_to_420p_ft ihevcd_fmt_conv_420sp_to_420p_av8;

#endif /* _IHEVCD_FMT_CONV_H_ */
