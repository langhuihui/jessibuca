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
*  ihevc_recon.h
*
* @brief
*  Functions declarations reconstruction
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_RECON_H_
#define _IHEVC_RECON_H_

typedef void ihevc_recon_4x4_ttype1_ft(WORD16 *pi2_src,
                                       UWORD8 *pu1_pred,
                                       UWORD8 *pu1_dst,
                                       WORD32 src_strd,
                                       WORD32 pred_strd,
                                       WORD32 dst_strd,
                                       WORD32 zero_cols);
typedef void ihevc_hbd_recon_4x4_ttype1_ft(WORD16 *pi2_src,
                                           UWORD16 *pu2_pred,
                                           UWORD16 *pu2_dst,
                                           WORD32 src_strd,
                                           WORD32 pred_strd,
                                           WORD32 dst_strd,
                                           WORD32 zero_cols,
                                           UWORD8 bit_depth);
typedef void ihevc_recon_4x4_ft(WORD16 *pi2_src,
                                UWORD8 *pu1_pred,
                                UWORD8 *pu1_dst,
                                WORD32 src_strd,
                                WORD32 pred_strd,
                                WORD32 dst_strd,
                                WORD32 zero_cols);
typedef void ihevc_hbd_recon_4x4_ft(WORD16 *pi2_src,
                                    UWORD16 *pu2_pred,
                                    UWORD16 *pu2_dst,
                                    WORD32 src_strd,
                                    WORD32 pred_strd,
                                    WORD32 dst_strd,
                                    WORD32 zero_cols,
                                    UWORD8 bit_depth);
typedef void ihevc_recon_8x8_ft(WORD16 *pi2_src,
                                UWORD8 *pu1_pred,
                                UWORD8 *pu1_dst,
                                WORD32 src_strd,
                                WORD32 pred_strd,
                                WORD32 dst_strd,
                                WORD32 zero_cols);
typedef void ihevc_hbd_recon_8x8_ft(WORD16 *pi2_src,
                                    UWORD16 *pu2_pred,
                                    UWORD16 *pu2_dst,
                                    WORD32 src_strd,
                                    WORD32 pred_strd,
                                    WORD32 dst_strd,
                                    WORD32 zero_cols,
                                    UWORD8 bit_depth);
typedef void ihevc_recon_16x16_ft(WORD16 *pi2_src,
                                  UWORD8 *pu1_pred,
                                  UWORD8 *pu1_dst,
                                  WORD32 src_strd,
                                  WORD32 pred_strd,
                                  WORD32 dst_strd,
                                  WORD32 zero_cols);
typedef void ihevc_hbd_recon_16x16_ft(WORD16 *pi2_src,
                                      UWORD16 *pu2_pred,
                                      UWORD16 *pu2_dst,
                                      WORD32 src_strd,
                                      WORD32 pred_strd,
                                      WORD32 dst_strd,
                                      WORD32 zero_cols,
                                      UWORD8 bit_depth);
typedef void ihevc_recon_32x32_ft(WORD16 *pi2_src,
                                  UWORD8 *pu1_pred,
                                  UWORD8 *pu1_dst,
                                  WORD32 src_strd,
                                  WORD32 pred_strd,
                                  WORD32 dst_strd,
                                  WORD32 zero_cols);
typedef void ihevc_hbd_recon_32x32_ft(WORD16 *pi2_src,
                                      UWORD16 *pu2_pred,
                                      UWORD16 *pu2_dst,
                                      WORD32 src_strd,
                                      WORD32 pred_strd,
                                      WORD32 dst_strd,
                                      WORD32 zero_cols,
                                      UWORD8 bit_depth);

ihevc_recon_4x4_ttype1_ft ihevc_recon_4x4_ttype1;
ihevc_hbd_recon_4x4_ttype1_ft ihevc_hbd_recon_4x4_ttype1;
ihevc_recon_4x4_ft ihevc_recon_4x4;
ihevc_hbd_recon_4x4_ft ihevc_hbd_recon_4x4;
ihevc_recon_8x8_ft ihevc_recon_8x8;
ihevc_hbd_recon_8x8_ft ihevc_hbd_recon_8x8;
ihevc_recon_16x16_ft ihevc_recon_16x16;
ihevc_hbd_recon_16x16_ft ihevc_hbd_recon_16x16;
ihevc_recon_32x32_ft ihevc_recon_32x32;
ihevc_hbd_recon_32x32_ft ihevc_hbd_recon_32x32;

#endif /*_IHEVC_RECON_H_*/
