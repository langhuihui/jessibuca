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
*  ihevc_func_selector.h
*
* @brief
*  For each function decide whether to use C function,  or Neon intrinsics
* or Cortex A8 intrinsics or Neon  assembly or cortex a8 assembly
*
* @author
*  Harish
*
* @remarks
*  None
*
*******************************************************************************
*/

#ifndef __IHEVC_FUNC_SELECTOR_H__
#define __IHEVC_FUNC_SELECTOR_H__

#include "ihevc_func_types.h"

#define    INTER_PRED_LUMA_COPY                     C
#define    INTER_PRED_LUMA_HORZ                     C
#define    INTER_PRED_LUMA_VERT                     C
#define    INTER_PRED_LUMA_COPY_W16OUT              C
#define    INTER_PRED_LUMA_HORZ_W16OUT              C

#define    INTER_PRED_LUMA_VERT_W16OUT              C
#define    INTER_PRED_LUMA_VERT_W16INP              C
#define    INTER_PRED_LUMA_VERT_W16INP_W16OUT       C

#define    INTER_PRED_CHROMA_COPY                   C
#define    INTER_PRED_CHROMA_HORZ                   C
#define    INTER_PRED_CHROMA_VERT                   C
#define    INTER_PRED_CHROMA_COPY_W16OUT            C
#define    INTER_PRED_CHROMA_HORZ_W16OUT            C
#define    INTER_PRED_CHROMA_VERT_W16OUT            C
#define    INTER_PRED_CHROMA_VERT_W16INP            C
#define    INTER_PRED_CHROMA_VERT_W16INP_W16OUT     C

#define    WEIGHTED_PRED_UNI                        C
#define    WEIGHTED_PRED_BI                         C
#define    WEIGHTED_PRED_BI_DEFAULT                 C
#define    WEIGHTED_PRED_CHROMA_UNI                 C
#define    WEIGHTED_PRED_CHROMA_BI                  C
#define    WEIGHTED_PRED_CHROMA_BI_DEFAULT          C

#define    PAD_VERT                                 C
#define    PAD_HORZ                                 C
#define    PAD_LEFT_LUMA                            C
#define    PAD_LEFT_CHROMA                          C
#define    PAD_RIGHT_LUMA                           C
#define    PAD_RIGHT_CHROMA                         C

#define     DEBLOCKING_ASM                          C
#define     DEBLK_LUMA_HORZ                         C
#define     DEBLK_LUMA_VERT                         C
#define     DEBLK_CHROMA_HORZ                       C
#define     DEBLK_CHROMA_VERT                       C

#define     SAO_BAND_OFFSET_LUMA                    C
#define     SAO_BAND_OFFSET_CHROMA                  C
#define     SAO_EDGE_OFFSET_CLASS0_LUMA             C
#define     SAO_EDGE_OFFSET_CLASS1_LUMA             C
#define     SAO_EDGE_OFFSET_CLASS2_LUMA             C
#define     SAO_EDGE_OFFSET_CLASS3_LUMA             C
#define     SAO_EDGE_OFFSET_CLASS0_CHROMA           C
#define     SAO_EDGE_OFFSET_CLASS1_CHROMA           C
#define     SAO_EDGE_OFFSET_CLASS2_CHROMA           C
#define     SAO_EDGE_OFFSET_CLASS3_CHROMA           C

#define     INTRA_PRED_LUMA_REF_SUBSTITUTION        C
#define     INTRA_PRED_REF_FILTERING                 C
#define     INTRA_PRED_LUMA_PLANAR                  C
#define     INTRA_PRED_LUMA_DC                      C
#define     INTRA_PRED_LUMA_HORZ                    C
#define     INTRA_PRED_LUMA_VER                     C
#define     INTRA_PRED_LUMA_MODE_2                  C
#define     INTRA_PRED_LUMA_MODE_18_34              C
#define     INTRA_PRED_LUMA_MODE_3_T0_9             C
#define     INTRA_PRED_LUMA_MODE_11_T0_17           C
#define     INTRA_PRED_LUMA_MODE_19_T0_25           C
#define     INTRA_PRED_LUMA_MODE_27_T0_33           C

#define     INTRA_PRED_CHROMA_PLANAR                C
#define     INTRA_PRED_CHROMA_DC                    C
#define     INTRA_PRED_CHROMA_HOR                   C
#define     INTRA_PRED_CHROMA_VER                   C
#define     INTRA_PRED_CHROMA_MODE_2                C
#define     INTRA_PRED_CHROMA_18_34                 C
#define     INTRA_PRED_CHROMA_3_T0_9                C
#define     INTRA_PRED_CHROMA_11_T0_17              C
#define     INTRA_PRED_CHROMA_19_T0_25              C
#define     INTRA_PRED_CHROMA_27_T0_33              C
#define     INTRA_PRED_CHROMA_REF_SUBSTITUTION      C

/* Forward transform functions */
/* Luma */
#define RESI_TRANS_QUANT_4X4_TTYPE1                 C
#define RESI_TRANS_QUANT_4X4                        C
#define RESI_TRANS_QUANT_8X8                        C
#define RESI_TRANS_QUANT_16X16                      C
#define RESI_TRANS_QUANT_32X32                      C

#define RESI_QUANT_4X4_TTYPE1                       C
#define RESI_QUANT_4X4                              C
#define RESI_QUANT_8X8                              C
#define RESI_QUANT_16X16                            C
#define RESI_QUANT_32X32                            C

#define RESI_TRANS_4X4_TTYPE1                       C
#define RESI_TRANS_4X4                              C
#define RESI_TRANS_8X8                              C
#define RESI_TRANS_16X16                            C
#define RESI_TRANS_32X32                            C

#define RESI_4X4_TTYPE1                             C
#define RESI_4X4                                    C
#define RESI_8X8                                    C
#define RESI_16X16                                  C
#define RESI_32X32                                  C

#define TRANS_4X4_TTYPE1                            C
#define TRANS_4X4                                   C
#define TRANS_8X8                                   C
#define TRANS_16X16                                 C
#define TRANS_32X32                                 C

#define QUANT_4X4_TTYPE1                            C
#define QUANT_4X4                                   C
#define QUANT_8X8                                   C
#define QUANT_16X16                                 C
#define QUANT_32X32                                 C

/* Chroma interleaved*/
#define CHROMA_RESI_TRANS_QUANT_4X4                        C
#define CHROMA_RESI_TRANS_QUANT_8X8                        C
#define CHROMA_RESI_TRANS_QUANT_16X16                      C

#define CHROMA_RESI_QUANT_4X4                              C
#define CHROMA_RESI_QUANT_8X8                              C
#define CHROMA_RESI_QUANT_16X16                            C

#define CHROMA_RESI_TRANS_4X4                              C
#define CHROMA_RESI_TRANS_8X8                              C
#define CHROMA_RESI_TRANS_16X16                            C

#define CHROMA_RESI_4X4                                    C
#define CHROMA_RESI_8X8                                    C
#define CHROMA_RESI_16X16                                  C

/* Inverse transform functions */
/* Luma */
#define IQUANT_ITRANS_RECON_4X4_TTYPE1              C
#define IQUANT_ITRANS_RECON_4X4                     C
#define IQUANT_ITRANS_RECON_8X8                     C
#define IQUANT_ITRANS_RECON_16X16                   C
#define IQUANT_ITRANS_RECON_32X32                   C

#define IQUANT_RECON_4X4_TTYPE1                     C
#define IQUANT_RECON_4X4                            C
#define IQUANT_RECON_8X8                            C
#define IQUANT_RECON_16X16                          C
#define IQUANT_RECON_32X32                          C

#define ITRANS_RECON_4X4_TTYPE1                     C
#define ITRANS_RECON_4X4                            C
#define ITRANS_RECON_8X8                            C
#define ITRANS_RECON_16X16                          C
#define ITRANS_RECON_32X32                          C

#define RECON_4X4_TTYPE1                            C
#define RECON_4X4                                   C
#define RECON_8X8                                   C
#define RECON_16X16                                 C
#define RECON_32X32                                 C

#define ITRANS_4X4_TTYPE1                           C
#define ITRANS_4X4                                  C
#define ITRANS_8X8                                  C
#define ITRANS_16X16                                C
#define ITRANS_32X32                                C

/* Chroma interleaved */
#define CHROMA_IQUANT_ITRANS_RECON_4X4                     C
#define CHROMA_IQUANT_ITRANS_RECON_8X8                     C
#define CHROMA_IQUANT_ITRANS_RECON_16X16                   C

#define CHROMA_IQUANT_RECON_4X4                            C
#define CHROMA_IQUANT_RECON_8X8                            C
#define CHROMA_IQUANT_RECON_16X16                          C

#define CHROMA_ITRANS_RECON_4X4                            C
#define CHROMA_ITRANS_RECON_8X8                            C
#define CHROMA_ITRANS_RECON_16X16                          C

#define CHROMA_RECON_4X4                                   C
#define CHROMA_RECON_8X8                                   C
#define CHROMA_RECON_16X16                                 C

#define IHEVC_MEMCPY                                C
#define IHEVC_MEMSET                                C
#define IHEVC_MEMSET_16BIT                          C
#define IHEVC_MEMCPY_MUL_8                          C
#define IHEVC_MEMSET_MUL_8                          C
#define IHEVC_MEMSET_16BIT_MUL_8                    C

#endif  /* __IHEVC_FUNC_SELECTOR_H__ */
