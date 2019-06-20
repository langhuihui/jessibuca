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
*  ihevc_common_tables.h
*
* @brief
*  Common tables
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

#ifndef _IHEVC_COMMON_TABLES_H_
#define _IHEVC_COMMON_TABLES_H_

extern MEM_ALIGN16 const WORD32 gai4_ihevc_max_luma_pic_size[];
extern MEM_ALIGN16 const WORD32 gai4_ihevc_max_wd_ht[];
extern MEM_ALIGN16 const WORD32 gai4_ihevc_min_wd_ht[];


extern MEM_ALIGN16 const WORD32 gai4_ihevc_ang_table[35];
extern MEM_ALIGN16 const WORD32 gai4_ihevc_inv_ang_table[14];

extern MEM_ALIGN16 const UWORD8  gau1_ihevc_scan8x8[][64];
extern MEM_ALIGN16 const UWORD8  gau1_ihevc_scan4x4[][16];
extern MEM_ALIGN16 const UWORD8  gau1_ihevc_scan2x2[][4];

extern MEM_ALIGN16 const UWORD8  gau1_ihevc_invscan8x8[][64];
extern MEM_ALIGN16 const UWORD8  gau1_ihevc_invscan4x4[][16];
extern MEM_ALIGN16 const UWORD8  gau1_ihevc_invscan2x2[][4];

extern MEM_ALIGN16 const void   *gapv_ihevc_scan[];
extern MEM_ALIGN16 const void   *gapv_ihevc_invscan[];
extern MEM_ALIGN16 const UWORD8 gau1_ihevc_chroma_qp_scale[];
extern MEM_ALIGN16 const WORD8 gai1_ihevc_chroma_qp_scale[];

extern MEM_ALIGN16 const WORD32 gai4_ihevc_ang_table_chroma[35];
extern MEM_ALIGN16 const WORD32 gai4_ihevc_inv_ang_table_chroma[14];
extern MEM_ALIGN16 const UWORD8 gau1_ihevc_planar_factor_chroma[33];

extern MEM_ALIGN16 const UWORD8 gau1_ihevc_planar_factor[65];

extern MEM_ALIGN16 const UWORD8 gau1_intra_pred_ref_filter[];

extern MEM_ALIGN16 const WORD8 gi1_table_edge_idx[8];

extern MEM_ALIGN16 const UWORD8 gu1_table_band_idx[32];

extern MEM_ALIGN16 const UWORD16 gu2_table_band_idx[32];

#endif /*_IHEVC_COMMON_TABLES_H_*/
