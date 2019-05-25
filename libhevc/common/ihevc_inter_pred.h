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
*  ihevc_inter_pred.h
*
* @brief
*  Declarations for the fucntions defined in  ihevc_inter_pred_ft.c
*
* @author
*  Srinivas T
*
* @par List of Functions:
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_INTER_PRED_H_
#define _IHEVC_INTER_PRED_H_

#define NTAPS_LUMA 8
#define NTAPS_CHROMA 4
#define SHIFT_14_MINUS_BIT_DEPTH (14 - BIT_DEPTH)
#define OFFSET_14_MINUS_BIT_DEPTH (1 << (SHIFT_14_MINUS_BIT_DEPTH - 1))
#define OFFSET14 (1 << (14 - 1))
#define FILTER_PREC 6

#define REF_WIDTH 1280
#define REF_HEIGHT 720

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

typedef void ihevc_inter_pred_ft(
                UWORD8 *pu1_src,
                UWORD8 *pu1_dst,
                WORD32 src_strd,
                WORD32 dst_strd,
                WORD8 *pi1_coeff,
                WORD32 ht,
                WORD32 wd);

typedef void ihevc_inter_pred_w16out_ft(
                UWORD8 *pu1_src,
                WORD16 *pi2_dst,
                WORD32 src_strd,
                WORD32 dst_strd,
                WORD8 *pi1_coeff,
                WORD32 ht,
                WORD32 wd);

typedef void ihevc_inter_pred_w16inp_ft(
                WORD16 *pi2_src,
                UWORD8 *pu1_dst,
                WORD32 src_strd,
                WORD32 dst_strd,
                WORD8 *pi1_coeff,
                WORD32 ht,
                WORD32 wd);

typedef void ihevc_inter_pred_w16inp_w16out_ft(
                WORD16 *pi2_src,
                WORD16 *pi2_dst,
                WORD32 src_strd,
                WORD32 dst_strd,
                WORD8 *pi1_coeff,
                WORD32 ht,
                WORD32 wd);


typedef void ihevc_hbd_inter_pred_ft(UWORD16 *pu2_src,
                                     UWORD16 *pu2_dst,
                                     WORD32 src_strd,
                                     WORD32 dst_strd,
                                     WORD8 *pi1_coeff,
                                     WORD32 ht,
                                     WORD32 wd,
                                     UWORD8 bit_depth);


typedef void ihevc_hbd_inter_pred_w16out_ft(
                UWORD16 *pu2_src,
                WORD16 *pi2_dst,
                WORD32 src_strd,
                WORD32 dst_strd,
                WORD8 *pi1_coeff,
                WORD32 ht,
                WORD32 wd,
                UWORD8 bit_depth);


typedef void ihevc_hbd_inter_pred_w16inp_ft(
                WORD16 *pi2_src,
                UWORD16 *pu2_dst,
                WORD32 src_strd,
                WORD32 dst_strd,
                WORD8 *pi1_coeff,
                WORD32 ht,
                WORD32 wd,
                UWORD8 bit_depth);


typedef void ihevc_hbd_inter_pred_w16inp_w16out_ft(
                WORD16 *pi2_src,
                WORD16 *pi2_dst,
                WORD32 src_strd,
                WORD32 dst_strd,
                WORD8 *pi1_coeff,
                WORD32 ht,
                WORD32 wd,
                UWORD8 bit_depth);

typedef void ihevc_hbd_weighted_pred_uni_ft(
                WORD16 *pi2_src,
                UWORD16 *pu2_dst,
                WORD32 src_strd,
                WORD32 dst_strd,
                WORD32 wgt0,
                WORD32 off0,
                WORD32 shift,
                WORD32 lvl_shift,
                WORD32 ht,
                WORD32 wd,
                UWORD8 bit_depth);


typedef void ihevc_hbd_weighted_pred_bi_ft(
                WORD16 *pi2_src1,
                WORD16 *pi2_src2,
                UWORD16 *pu2_dst,
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
                WORD32 wd,
                UWORD8 bit_depth);


typedef void ihevc_hbd_weighted_pred_bi_default_ft(
                WORD16 *pi2_src1,
                WORD16 *pi2_src2,
                UWORD16 *pu2_dst,
                WORD32 src_strd1,
                WORD32 src_strd2,
                WORD32 dst_strd,
                WORD32 lvl_shift1,
                WORD32 lvl_shift2,
                WORD32 ht,
                WORD32 wd,
                UWORD8 bit_depth);
typedef void ihevc_hbd_weighted_pred_chroma_uni_ft(WORD16 *pi2_src,
                                                   UWORD16 *pu2_dst,
                                                   WORD32 src_strd,
                                                   WORD32 dst_strd,
                                                   WORD32 wgt0_cb,
                                                   WORD32 wgt0_cr,
                                                   WORD32 off0_cb,
                                                   WORD32 off0_cr,
                                                   WORD32 shift,
                                                   WORD32 lvl_shift,
                                                   WORD32 ht,
                                                   WORD32 wd,
                                                   UWORD8 bit_depth);

typedef void ihevc_hbd_weighted_pred_chroma_bi_ft(WORD16 *pi2_src1,
                                                  WORD16 *pi2_src2,
                                                  UWORD16 *pu2_dst,
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
                                                  WORD32 wd,
                                                  UWORD8 bit_depth);

typedef void ihevc_hbd_weighted_pred_chroma_bi_default_ft(WORD16 *pi2_src1,
                                                          WORD16 *pi2_src2,
                                                          UWORD16 *pu2_dst,
                                                          WORD32 src_strd1,
                                                          WORD32 src_strd2,
                                                          WORD32 dst_strd,
                                                          WORD32 lvl_shift1,
                                                          WORD32 lvl_shift2,
                                                          WORD32 ht,
                                                          WORD32 wd,
                                                          UWORD8 bit_depth);
/* C function declarations */
ihevc_inter_pred_ft ihevc_inter_pred_luma_copy;
ihevc_inter_pred_ft ihevc_inter_pred_luma_horz;
ihevc_inter_pred_ft ihevc_inter_pred_luma_vert;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_copy_w16out;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_horz_w16out;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_vert_w16out;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_luma_vert_w16inp;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_luma_vert_w16inp_w16out;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_copy;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_horz;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_vert;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_copy_w16out;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_horz_w16out;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_vert_w16out;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_chroma_vert_w16inp;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_chroma_vert_w16inp_w16out;

ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_luma_copy;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_luma_horz;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_luma_vert;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_luma_copy_w16out;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_luma_horz_w16out;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_luma_vert_w16out;
ihevc_hbd_inter_pred_w16inp_ft ihevc_hbd_inter_pred_luma_vert_w16inp;
ihevc_hbd_inter_pred_w16inp_w16out_ft ihevc_hbd_inter_pred_luma_vert_w16inp_w16out;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_chroma_copy;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_chroma_horz;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_chroma_vert;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_chroma_copy_w16out;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_chroma_horz_w16out;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_chroma_vert_w16out;
ihevc_hbd_inter_pred_w16inp_ft ihevc_hbd_inter_pred_chroma_vert_w16inp;
ihevc_hbd_inter_pred_w16inp_w16out_ft ihevc_hbd_inter_pred_chroma_vert_w16inp_w16out;
ihevc_hbd_weighted_pred_uni_ft ihevc_hbd_weighted_pred_uni;
ihevc_hbd_weighted_pred_bi_ft ihevc_hbd_weighted_pred_bi;
ihevc_hbd_weighted_pred_bi_default_ft ihevc_hbd_weighted_pred_bi_default;
ihevc_hbd_weighted_pred_chroma_uni_ft ihevc_hbd_weighted_pred_chroma_uni;
ihevc_hbd_weighted_pred_chroma_bi_ft ihevc_hbd_weighted_pred_chroma_bi;
ihevc_hbd_weighted_pred_chroma_bi_default_ft ihevc_hbd_weighted_pred_chroma_bi_default;

/* A9 Q function declarations */
ihevc_inter_pred_ft ihevc_inter_pred_luma_copy_a9q;
ihevc_inter_pred_ft ihevc_inter_pred_luma_horz_a9q;
ihevc_inter_pred_ft ihevc_inter_pred_luma_vert_a9q;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_copy_w16out_a9q;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_horz_w16out_a9q;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_vert_w16out_a9q;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_luma_vert_w16inp_a9q;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_luma_vert_w16inp_w16out_a9q;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_copy_a9q;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_horz_a9q;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_vert_a9q;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_copy_w16out_a9q;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_horz_w16out_a9q;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_vert_w16out_a9q;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_chroma_vert_w16inp_a9q;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_chroma_vert_w16inp_w16out_a9q;

/* A9 A function declarations */
ihevc_inter_pred_ft ihevc_inter_pred_luma_copy_a9a;
ihevc_inter_pred_ft ihevc_inter_pred_luma_horz_a9a;
ihevc_inter_pred_ft ihevc_inter_pred_luma_vert_a9a;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_copy_w16out_a9a;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_horz_w16out_a9a;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_vert_w16out_a9a;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_luma_vert_w16inp_a9a;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_luma_vert_w16inp_w16out_a9a;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_copy_a9a;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_horz_a9a;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_vert_a9a;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_copy_w16out_a9a;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_horz_w16out_a9a;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_vert_w16out_a9a;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_chroma_vert_w16inp_a9a;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_chroma_vert_w16inp_w16out_a9a;

/* NEONINTR function declarations */
ihevc_inter_pred_ft ihevc_inter_pred_luma_copy_neonintr;
ihevc_inter_pred_ft ihevc_inter_pred_luma_horz_neonintr;
ihevc_inter_pred_ft ihevc_inter_pred_luma_vert_neonintr;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_copy_w16out_neonintr;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_horz_w16out_neonintr;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_vert_w16out_neonintr;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_luma_vert_w16inp_neonintr;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_luma_vert_w16inp_w16out_neonintr;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_copy_neonintr;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_horz_neonintr;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_vert_neonintr;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_copy_w16out_neonintr;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_horz_w16out_neonintr;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_vert_w16out_neonintr;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_chroma_vert_w16inp_neonintr;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_chroma_vert_w16inp_w16out_neonintr;

/* SSSE31 function declarations */
ihevc_inter_pred_ft ihevc_inter_pred_luma_copy_ssse3;
ihevc_inter_pred_ft ihevc_inter_pred_luma_horz_ssse3;
ihevc_inter_pred_ft ihevc_inter_pred_luma_vert_ssse3;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_copy_w16out_ssse3;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_horz_w16out_ssse3;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_vert_w16out_ssse3;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_luma_vert_w16inp_ssse3;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_luma_vert_w16inp_w16out_ssse3;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_copy_ssse3;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_horz_ssse3;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_vert_ssse3;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_copy_w16out_ssse3;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_horz_w16out_ssse3;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_vert_w16out_ssse3;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_chroma_vert_w16inp_ssse3;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_chroma_vert_w16inp_w16out_ssse3;

/* SSE42 function declarations */
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_copy_w16out_sse42;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_copy_sse42;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_copy_w16out_sse42;

ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_luma_copy_sse42;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_luma_horz_sse42;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_luma_vert_sse42;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_luma_copy_w16out_sse42;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_luma_horz_w16out_sse42;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_luma_vert_w16out_sse42;
ihevc_hbd_inter_pred_w16inp_ft ihevc_hbd_inter_pred_luma_vert_w16inp_sse42;
ihevc_hbd_inter_pred_w16inp_w16out_ft ihevc_hbd_inter_pred_luma_vert_w16inp_w16out_sse42;

ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_chroma_copy_sse42;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_chroma_horz_sse42;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_chroma_vert_sse42;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_chroma_copy_w16out_sse42;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_chroma_horz_w16out_sse42;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_chroma_vert_w16out_sse42;
ihevc_hbd_inter_pred_w16inp_ft ihevc_hbd_inter_pred_chroma_vert_w16inp_sse42;
ihevc_hbd_inter_pred_w16inp_w16out_ft ihevc_hbd_inter_pred_chroma_vert_w16inp_w16out_sse42;

ihevc_hbd_weighted_pred_uni_ft ihevc_hbd_weighted_pred_uni_sse42;
ihevc_hbd_weighted_pred_bi_ft ihevc_hbd_weighted_pred_bi_sse42;
ihevc_hbd_weighted_pred_bi_default_ft ihevc_hbd_weighted_pred_bi_default_sse42;
ihevc_hbd_weighted_pred_chroma_uni_ft ihevc_hbd_weighted_pred_chroma_uni_sse42;
ihevc_hbd_weighted_pred_chroma_bi_ft ihevc_hbd_weighted_pred_chroma_bi_sse42;
ihevc_hbd_weighted_pred_chroma_bi_default_ft ihevc_hbd_weighted_pred_chroma_bi_default_sse42;

#ifndef DISABLE_AVX2
/* AVX2 function declarations */
ihevc_inter_pred_ft ihevc_inter_pred_luma_copy_avx2;
ihevc_inter_pred_ft ihevc_inter_pred_luma_horz_avx2;
ihevc_inter_pred_ft ihevc_inter_pred_luma_vert_avx2;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_copy_w16out_avx2;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_horz_w16out_avx2;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_vert_w16out_avx2;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_luma_vert_w16inp_avx2;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_luma_vert_w16inp_w16out_avx2;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_copy_avx2;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_horz_avx2;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_vert_avx2;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_copy_w16out_avx2;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_horz_w16out_avx2;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_vert_w16out_avx2;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_chroma_vert_w16inp_avx2;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_chroma_vert_w16inp_w16out_avx2;

ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_luma_copy_avx2;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_luma_horz_avx2;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_luma_vert_avx2;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_luma_copy_w16out_avx2;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_luma_horz_w16out_avx2;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_luma_vert_w16out_avx2;
ihevc_hbd_inter_pred_w16inp_ft ihevc_hbd_inter_pred_luma_vert_w16inp_avx2;
ihevc_hbd_inter_pred_w16inp_w16out_ft ihevc_hbd_inter_pred_luma_vert_w16inp_w16out_avx2;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_chroma_copy_avx2;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_chroma_horz_avx2;
ihevc_hbd_inter_pred_ft ihevc_hbd_inter_pred_chroma_vert_avx2;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_chroma_copy_w16out_avx2;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_chroma_horz_w16out_avx2;
ihevc_hbd_inter_pred_w16out_ft ihevc_hbd_inter_pred_chroma_vert_w16out_avx2;
ihevc_hbd_inter_pred_w16inp_ft ihevc_hbd_inter_pred_chroma_vert_w16inp_avx2;
ihevc_hbd_inter_pred_w16inp_w16out_ft ihevc_hbd_inter_pred_chroma_vert_w16inp_w16out_avx2;
#endif

/* armv8 function declarations */
ihevc_inter_pred_ft ihevc_inter_pred_luma_copy_av8;
ihevc_inter_pred_ft ihevc_inter_pred_luma_horz_av8;
ihevc_inter_pred_ft ihevc_inter_pred_luma_vert_av8;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_copy_w16out_av8;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_horz_w16out_av8;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_luma_vert_w16out_av8;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_luma_vert_w16inp_av8;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_luma_vert_w16inp_w16out_av8;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_copy_av8;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_horz_av8;
ihevc_inter_pred_ft ihevc_inter_pred_chroma_vert_av8;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_copy_w16out_av8;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_horz_w16out_av8;
ihevc_inter_pred_w16out_ft ihevc_inter_pred_chroma_vert_w16out_av8;
ihevc_inter_pred_w16inp_ft ihevc_inter_pred_chroma_vert_w16inp_av8;
ihevc_inter_pred_w16inp_w16out_ft ihevc_inter_pred_chroma_vert_w16inp_w16out_av8;
#endif /*_IHEVC_INTER_PRED_H_*/
