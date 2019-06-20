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
*  ihevc_itrans_recon.h
*
* @brief
*  Functions declarations for inverse transform and  reconstruction
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_ITRANS_RECON_H_
#define _IHEVC_ITRANS_RECON_H_

typedef void ihevc_itrans_recon_4x4_ttype1_ft(WORD16 *pi2_src,
                                              WORD16 *pi2_tmp,
                                              UWORD8 *pu1_pred,
                                              UWORD8 *pu1_dst,
                                              WORD32 src_strd,
                                              WORD32 pred_strd,
                                              WORD32 dst_strd,
                                              WORD32 zero_cols,
                                              WORD32 zero_rows);
typedef void ihevc_hbd_itrans_recon_4x4_ttype1_ft(WORD16 *pi2_src,
                                                  WORD16 *pi2_tmp,
                                                  UWORD16 *pu2_pred,
                                                  UWORD16 *pu2_dst,
                                                  WORD32 src_strd,
                                                  WORD32 pred_strd,
                                                  WORD32 dst_strd,
                                                  WORD32 zero_cols,
                                                  WORD32 zero_rows,
                                                  UWORD8 bit_depth);
typedef void ihevc_itrans_recon_4x4_ft(WORD16 *pi2_src,
                                       WORD16 *pi2_tmp,
                                       UWORD8 *pu1_pred,
                                       UWORD8 *pu1_dst,
                                       WORD32 src_strd,
                                       WORD32 pred_strd,
                                       WORD32 dst_strd,
                                       WORD32 zero_cols,
                                       WORD32 zero_rows);
typedef void ihevc_hbd_itrans_recon_4x4_ft(WORD16 *pi2_src,
                                           WORD16 *pi2_tmp,
                                           UWORD16 *pu2_pred,
                                           UWORD16 *pu2_dst,
                                           WORD32 src_strd,
                                           WORD32 pred_strd,
                                           WORD32 dst_strd,
                                           WORD32 zero_cols,
                                           WORD32 zero_rows,
                                           UWORD8 bit_depth);
typedef void ihevc_itrans_recon_8x8_ft(WORD16 *pi2_src,
                                       WORD16 *pi2_tmp,
                                       UWORD8 *pu1_pred,
                                       UWORD8 *pu1_dst,
                                       WORD32 src_strd,
                                       WORD32 pred_strd,
                                       WORD32 dst_strd,
                                       WORD32 zero_cols,
                                       WORD32 zero_rows);
typedef void ihevc_hbd_itrans_recon_8x8_ft(WORD16 *pi2_src,
                                           WORD16 *pi2_tmp,
                                           UWORD16 *pu2_pred,
                                           UWORD16 *pu2_dst,
                                           WORD32 src_strd,
                                           WORD32 pred_strd,
                                           WORD32 dst_strd,
                                           WORD32 zero_cols,
                                           WORD32 zero_rows,
                                           UWORD8 bit_depth);
typedef void ihevc_itrans_recon_16x16_ft(WORD16 *pi2_src,
                                         WORD16 *pi2_tmp,
                                         UWORD8 *pu1_pred,
                                         UWORD8 *pu1_dst,
                                         WORD32 src_strd,
                                         WORD32 pred_strd,
                                         WORD32 dst_strd,
                                         WORD32 zero_cols,
                                         WORD32 zero_rows);
typedef void ihevc_hbd_itrans_recon_16x16_ft(WORD16 *pi2_src,
                                             WORD16 *pi2_tmp,
                                             UWORD16 *pu2_pred,
                                             UWORD16 *pu2_dst,
                                             WORD32 src_strd,
                                             WORD32 pred_strd,
                                             WORD32 dst_strd,
                                             WORD32 zero_cols,
                                             WORD32 zero_rows,
                                             UWORD8 bit_depth);
typedef void ihevc_itrans_recon_32x32_ft(WORD16 *pi2_src,
                                         WORD16 *pi2_tmp,
                                         UWORD8 *pu1_pred,
                                         UWORD8 *pu1_dst,
                                         WORD32 src_strd,
                                         WORD32 pred_strd,
                                         WORD32 dst_strd,
                                         WORD32 zero_cols,
                                         WORD32 zero_rows);
typedef void ihevc_hbd_itrans_recon_32x32_ft(WORD16 *pi2_src,
                                             WORD16 *pi2_tmp,
                                             UWORD16 *pu2_pred,
                                             UWORD16 *pu2_dst,
                                             WORD32 src_strd,
                                             WORD32 pred_strd,
                                             WORD32 dst_strd,
                                             WORD32 zero_cols,
                                             WORD32 zero_rows,
                                             UWORD8 bit_depth);

/* C function declarations */
ihevc_itrans_recon_4x4_ttype1_ft ihevc_itrans_recon_4x4_ttype1;
ihevc_itrans_recon_4x4_ft ihevc_itrans_recon_4x4;
ihevc_itrans_recon_8x8_ft ihevc_itrans_recon_8x8;
ihevc_itrans_recon_16x16_ft ihevc_itrans_recon_16x16;
ihevc_itrans_recon_32x32_ft ihevc_itrans_recon_32x32;

ihevc_hbd_itrans_recon_4x4_ttype1_ft ihevc_hbd_itrans_recon_4x4_ttype1;
ihevc_hbd_itrans_recon_4x4_ft ihevc_hbd_itrans_recon_4x4;
ihevc_hbd_itrans_recon_8x8_ft ihevc_hbd_itrans_recon_8x8;
ihevc_hbd_itrans_recon_16x16_ft ihevc_hbd_itrans_recon_16x16;
ihevc_hbd_itrans_recon_32x32_ft ihevc_hbd_itrans_recon_32x32;

/* A9 Q function declarations */
ihevc_itrans_recon_4x4_ttype1_ft ihevc_itrans_recon_4x4_ttype1_a9q;
ihevc_itrans_recon_4x4_ft ihevc_itrans_recon_4x4_a9q;
ihevc_itrans_recon_8x8_ft ihevc_itrans_recon_8x8_a9q;
ihevc_itrans_recon_16x16_ft ihevc_itrans_recon_16x16_a9q;
ihevc_itrans_recon_32x32_ft ihevc_itrans_recon_32x32_a9q;

/* A9 A function declarations */
ihevc_itrans_recon_4x4_ttype1_ft ihevc_itrans_recon_4x4_ttype1_a9a;
ihevc_itrans_recon_4x4_ft ihevc_itrans_recon_4x4_a9a;
ihevc_itrans_recon_8x8_ft ihevc_itrans_recon_8x8_a9a;
ihevc_itrans_recon_16x16_ft ihevc_itrans_recon_16x16_a9a;
ihevc_itrans_recon_32x32_ft ihevc_itrans_recon_32x32_a9a;

/* NEONINTR function declarations */
ihevc_itrans_recon_4x4_ttype1_ft ihevc_itrans_recon_4x4_ttype1_neonintr;
ihevc_itrans_recon_4x4_ft ihevc_itrans_recon_4x4_neonintr;
ihevc_itrans_recon_8x8_ft ihevc_itrans_recon_8x8_neonintr;
ihevc_itrans_recon_16x16_ft ihevc_itrans_recon_16x16_neonintr;
ihevc_itrans_recon_32x32_ft ihevc_itrans_recon_32x32_neonintr;

/* SSSE31 function declarations */
ihevc_itrans_recon_4x4_ttype1_ft ihevc_itrans_recon_4x4_ttype1_ssse3;
ihevc_itrans_recon_4x4_ft ihevc_itrans_recon_4x4_ssse3;
ihevc_itrans_recon_8x8_ft ihevc_itrans_recon_8x8_ssse3;
ihevc_itrans_recon_16x16_ft ihevc_itrans_recon_16x16_ssse3;
ihevc_itrans_recon_32x32_ft ihevc_itrans_recon_32x32_ssse3;

/* SSE42 function declarations */
ihevc_itrans_recon_4x4_ttype1_ft ihevc_itrans_recon_4x4_ttype1_sse42;
ihevc_itrans_recon_4x4_ft ihevc_itrans_recon_4x4_sse42;
ihevc_itrans_recon_8x8_ft ihevc_itrans_recon_8x8_sse42;
ihevc_itrans_recon_32x32_ft ihevc_itrans_recon_32x32_sse42;

ihevc_hbd_itrans_recon_4x4_ttype1_ft ihevc_hbd_itrans_recon_4x4_ttype1_sse42;
ihevc_hbd_itrans_recon_4x4_ft ihevc_hbd_itrans_recon_4x4_sse42;
ihevc_hbd_itrans_recon_8x8_ft ihevc_hbd_itrans_recon_8x8_sse42;
ihevc_hbd_itrans_recon_16x16_ft ihevc_hbd_itrans_recon_16x16_sse42;
ihevc_hbd_itrans_recon_32x32_ft ihevc_hbd_itrans_recon_32x32_sse42;


/* armv8 function declarations */
ihevc_itrans_recon_4x4_ttype1_ft ihevc_itrans_recon_4x4_ttype1_av8;
ihevc_itrans_recon_4x4_ft ihevc_itrans_recon_4x4_av8;
ihevc_itrans_recon_8x8_ft ihevc_itrans_recon_8x8_av8;
ihevc_itrans_recon_16x16_ft ihevc_itrans_recon_16x16_av8;
ihevc_itrans_recon_32x32_ft ihevc_itrans_recon_32x32_av8;
#endif /*_IHEVC_ITRANS_RECON_H_*/
