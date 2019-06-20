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
*  ihevc_weighted_pred.h
*
* @brief
*  Function declarations used for buffer management
*
* @author
*  Srinivas T
*
* @remarks
*  None
*
*******************************************************************************
*/

#ifndef IHEVC_WEIGHTED_PRED_H_
#define IHEVC_WEIGHTED_PRED_H_

typedef void ihevc_weighted_pred_uni_ft(WORD16 *pi2_src,
                                        UWORD8 *pu1_dst,
                                        WORD32 src_strd,
                                        WORD32 dst_strd,
                                        WORD32 wgt0,
                                        WORD32 off0,
                                        WORD32 shift,
                                        WORD32 lvl_shift,
                                        WORD32 ht,
                                        WORD32 wd);

typedef void ihevc_weighted_pred_chroma_uni_ft(WORD16 *pi2_src,
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
                                               WORD32 wd);

typedef void ihevc_weighted_pred_bi_ft(WORD16 *pi2_src1,
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
                                       WORD32 wd);

typedef void ihevc_weighted_pred_chroma_bi_ft(WORD16 *pi2_src1,
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
                                              WORD32 wd);

typedef void ihevc_weighted_pred_bi_default_ft(WORD16 *pi2_src1,
                                               WORD16 *pi2_src2,
                                               UWORD8 *pu1_dst,
                                               WORD32 src_strd1,
                                               WORD32 src_strd2,
                                               WORD32 dst_strd,
                                               WORD32 lvl_shift1,
                                               WORD32 lvl_shift2,
                                               WORD32 ht,
                                               WORD32 wd);

typedef void ihevc_weighted_pred_chroma_bi_default_ft(WORD16 *pi2_src1,
                                                      WORD16 *pi2_src2,
                                                      UWORD8 *pu1_dst,
                                                      WORD32 src_strd1,
                                                      WORD32 src_strd2,
                                                      WORD32 dst_strd,
                                                      WORD32 lvl_shift1,
                                                      WORD32 lvl_shift2,
                                                      WORD32 ht,
                                                      WORD32 wd);
/* C function declarations */
ihevc_weighted_pred_uni_ft ihevc_weighted_pred_uni;
ihevc_weighted_pred_chroma_uni_ft ihevc_weighted_pred_chroma_uni;
ihevc_weighted_pred_bi_ft ihevc_weighted_pred_bi;
ihevc_weighted_pred_chroma_bi_ft ihevc_weighted_pred_chroma_bi;
ihevc_weighted_pred_bi_default_ft ihevc_weighted_pred_bi_default;
ihevc_weighted_pred_chroma_bi_default_ft ihevc_weighted_pred_chroma_bi_default;

/* A9 Q function declarations */
ihevc_weighted_pred_uni_ft ihevc_weighted_pred_uni_a9q;
ihevc_weighted_pred_chroma_uni_ft ihevc_weighted_pred_chroma_uni_a9q;
ihevc_weighted_pred_bi_ft ihevc_weighted_pred_bi_a9q;
ihevc_weighted_pred_chroma_bi_ft ihevc_weighted_pred_chroma_bi_a9q;
ihevc_weighted_pred_bi_default_ft ihevc_weighted_pred_bi_default_a9q;
ihevc_weighted_pred_chroma_bi_default_ft ihevc_weighted_pred_chroma_bi_default_a9q;

/* A9 A function declarations */
ihevc_weighted_pred_uni_ft ihevc_weighted_pred_uni_a9a;
ihevc_weighted_pred_chroma_uni_ft ihevc_weighted_pred_chroma_uni_a9a;
ihevc_weighted_pred_bi_ft ihevc_weighted_pred_bi_a9a;
ihevc_weighted_pred_chroma_bi_ft ihevc_weighted_pred_chroma_bi_a9a;
ihevc_weighted_pred_bi_default_ft ihevc_weighted_pred_bi_default_a9a;
ihevc_weighted_pred_chroma_bi_default_ft ihevc_weighted_pred_chroma_bi_default_a9a;

/* NEONINTR function declarations */
ihevc_weighted_pred_uni_ft ihevc_weighted_pred_uni_neonintr;
ihevc_weighted_pred_chroma_uni_ft ihevc_weighted_pred_chroma_uni_neonintr;
ihevc_weighted_pred_bi_ft ihevc_weighted_pred_bi_neonintr;
ihevc_weighted_pred_chroma_bi_ft ihevc_weighted_pred_chroma_bi_neonintr;
ihevc_weighted_pred_bi_default_ft ihevc_weighted_pred_bi_default_neonintr;
ihevc_weighted_pred_chroma_bi_default_ft ihevc_weighted_pred_chroma_bi_default_neonintr;
/* SSSE3 function declarations */
ihevc_weighted_pred_uni_ft ihevc_weighted_pred_uni_ssse3;
ihevc_weighted_pred_chroma_uni_ft ihevc_weighted_pred_chroma_uni_ssse3;
ihevc_weighted_pred_bi_ft ihevc_weighted_pred_bi_ssse3;
ihevc_weighted_pred_chroma_bi_ft ihevc_weighted_pred_chroma_bi_ssse3;
ihevc_weighted_pred_bi_default_ft ihevc_weighted_pred_bi_default_ssse3;
ihevc_weighted_pred_chroma_bi_default_ft ihevc_weighted_pred_chroma_bi_default_ssse3;

/* SSE42 function declarations */
ihevc_weighted_pred_uni_ft ihevc_weighted_pred_uni_sse42;
ihevc_weighted_pred_chroma_uni_ft ihevc_weighted_pred_chroma_uni_sse42;
ihevc_weighted_pred_bi_ft ihevc_weighted_pred_bi_sse42;
ihevc_weighted_pred_chroma_bi_ft ihevc_weighted_pred_chroma_bi_sse42;
ihevc_weighted_pred_bi_default_ft ihevc_weighted_pred_bi_default_sse42;
ihevc_weighted_pred_chroma_bi_default_ft ihevc_weighted_pred_chroma_bi_default_sse42;

/* AVX2 function declarations */
ihevc_weighted_pred_bi_default_ft ihevc_weighted_pred_bi_default_avx2;
ihevc_weighted_pred_chroma_bi_default_ft ihevc_weighted_pred_chroma_bi_default_avx2;

/* armv8 function declarations */
ihevc_weighted_pred_uni_ft ihevc_weighted_pred_uni_av8;
ihevc_weighted_pred_chroma_uni_ft ihevc_weighted_pred_chroma_uni_av8;
ihevc_weighted_pred_bi_ft ihevc_weighted_pred_bi_av8;
ihevc_weighted_pred_chroma_bi_ft ihevc_weighted_pred_chroma_bi_av8;
ihevc_weighted_pred_bi_default_ft ihevc_weighted_pred_bi_default_av8;
ihevc_weighted_pred_chroma_bi_default_ft ihevc_weighted_pred_chroma_bi_default_av8;

#endif /* IHEVC_WEIGHTED_PRED_H_ */
