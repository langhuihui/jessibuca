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
*  ihevc_itrans.h
*
* @brief
*  Functions declarations for inverse transform
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_ITRANS_H_
#define _IHEVC_ITRANS_H_

typedef void ihevc_itrans_4x4_ttype1_ft(WORD16 *pi2_src,
                                        WORD16 *pi2_dst,
                                        WORD32 i4_src_strd,
                                        WORD32 i4_dst_strd,
                                        WORD32 i4_shift,
                                        WORD32 i4_zero_cols);
typedef void ihevc_itrans_4x4_ft(WORD16 *pi2_src,
                                 WORD16 *pi2_dst,
                                 WORD32 i4_src_strd,
                                 WORD32 i4_dst_strd,
                                 WORD32 i4_shift,
                                 WORD32 i4_zero_cols);
typedef void ihevc_itrans_8x8_ft(WORD16 *pi2_src,
                                 WORD16 *pi2_dst,
                                 WORD32 i4_src_strd,
                                 WORD32 i4_dst_strd,
                                 WORD32 i4_shift,
                                 WORD32 i4_zero_cols);
typedef void ihevc_itrans_16x16_ft(WORD16 *pi2_src,
                                   WORD16 *pi2_dst,
                                   WORD32 i4_src_strd,
                                   WORD32 i4_dst_strd,
                                   WORD32 i4_shift,
                                   WORD32 i4_zero_cols);
typedef void ihevc_itrans_32x32_ft(WORD16 *pi2_src,
                                   WORD16 *pi2_dst,
                                   WORD32 i4_src_strd,
                                   WORD32 i4_dst_strd,
                                   WORD32 i4_shift,
                                   WORD32 i4_zero_cols);

/* C function declarations */
ihevc_itrans_4x4_ttype1_ft ihevc_itrans_4x4_ttype1;
ihevc_itrans_4x4_ft ihevc_itrans_4x4;
ihevc_itrans_8x8_ft ihevc_itrans_8x8;
ihevc_itrans_16x16_ft ihevc_itrans_16x16;
ihevc_itrans_32x32_ft ihevc_itrans_32x32;

/* A9 Q function declarations */
ihevc_itrans_4x4_ttype1_ft ihevc_itrans_4x4_ttype1_a9q;
ihevc_itrans_4x4_ft ihevc_itrans_4x4_a9q;
ihevc_itrans_8x8_ft ihevc_itrans_8x8_a9q;
ihevc_itrans_16x16_ft ihevc_itrans_16x16_a9q;
ihevc_itrans_32x32_ft ihevc_itrans_32x32_a9q;

/* A9 Q function declarations */
ihevc_itrans_4x4_ttype1_ft ihevc_itrans_4x4_ttype1_neonintr;
ihevc_itrans_4x4_ft ihevc_itrans_4x4_neonintr;
ihevc_itrans_8x8_ft ihevc_itrans_8x8_neonintr;
ihevc_itrans_16x16_ft ihevc_itrans_16x16_neonintr;
ihevc_itrans_32x32_ft ihevc_itrans_32x32_neonintr;

/* SSSE3 function declarations */
ihevc_itrans_4x4_ttype1_ft ihevc_itrans_4x4_ttype1_ssse3;
ihevc_itrans_4x4_ft ihevc_itrans_4x4_ssse3;
ihevc_itrans_8x8_ft ihevc_itrans_8x8_ssse3;
ihevc_itrans_16x16_ft ihevc_itrans_16x16_ssse3;
ihevc_itrans_32x32_ft ihevc_itrans_32x32_ssse3;

/* SSE4.2 function declarations */
ihevc_itrans_4x4_ttype1_ft ihevc_itrans_4x4_ttype1_sse42;
ihevc_itrans_4x4_ft ihevc_itrans_4x4_sse42;
ihevc_itrans_8x8_ft ihevc_itrans_8x8_sse42;
ihevc_itrans_16x16_ft ihevc_itrans_16x16_sse42;
ihevc_itrans_32x32_ft ihevc_itrans_32x32_sse42;

/* armv8 function declarations */
ihevc_itrans_4x4_ttype1_ft ihevc_itrans_4x4_ttype1_av8;
ihevc_itrans_4x4_ft ihevc_itrans_4x4_av8;
ihevc_itrans_8x8_ft ihevc_itrans_8x8_av8;
ihevc_itrans_16x16_ft ihevc_itrans_16x16_av8;
ihevc_itrans_32x32_ft ihevc_itrans_32x32_av8;
#endif /*_IHEVC_ITRANS_H_*/
