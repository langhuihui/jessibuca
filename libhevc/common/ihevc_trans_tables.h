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
*  ihevc_trans_tables.h
*
* @brief
*  Tables for forward and inverse transform
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_TRANS_TABLES_H_
#define _IHEVC_TRANS_TABLES_H_


#include "ihevc_defs.h"

extern const WORD32 g_ihevc_iquant_scales[6];

extern const WORD16 g_ihevc_iquant_intr_scales[6][8];

extern const WORD32 g_ihevc_iquant_scales_flat_scale[6];

extern const WORD32 g_ihevc_quant_scales[6];

extern const WORD16 g_ai2_ihevc_trans_dst_4[4][4];

extern const WORD16 g_ai2_ihevc_trans_4[4][4];

extern const WORD16 g_ai2_ihevc_trans_4_transpose[4][4];

extern const WORD16 g_ai2_ihevc_trans_8[8][8];

extern const WORD16 g_ai2_ihevc_trans_16[16][16];
extern const WORD32 g_ai4_ihevc_trans_16[16][8];
extern const WORD16 g_ai2_ihevc_trans_16_transpose[1][16];
extern const WORD16 g_ai2_ihevc_trans_32_transpose[1][32];
extern const WORD16 g_ai2_ihevc_trans_32[32][32];
extern const WORD32 g_ai4_ihevc_trans_32[32][16];

extern const WORD32 g_ai4_ihevc_trans_dst_intr_4[3][4];

extern const WORD32 g_ai4_ihevc_trans_4_intr[3][4];
extern const WORD16 g_ai2_ihevc_trans_4_intr[8];

extern const WORD32 g_ai4_ihevc_trans_8_intr[7][4];
extern const WORD16 g_ai2_ihevc_trans_8_intr[8][8];


extern const WORD32 g_ai4_ihevc_trans_4_ttype1[3][4];

extern const WORD32 g_ai4_ihevc_trans_4_ttype0[3][4];

extern const WORD32 g_ai4_ihevc_trans_intr_even_8[3][4];

extern const WORD32 g_ai4_ihevc_trans_intr_odd_8[4][4];

extern const WORD32 g_ai4_ihevc_trans_16_even[7][4];

extern const WORD32 g_ai4_ihevc_trans_16_odd[8][4];

extern const WORD32 g_ai2_ihevc_trans_32_intr_8[8][4];
extern const WORD32 g_ai2_ihevc_trans_32_intr_16[15][4];

extern const WORD16 g_ai2_ihevc_trans_16_intr_even[12][8];

extern const WORD16 g_ai2_ihevc_trans_16_intr_odd[32][8];


extern const WORD16 g_ai2_ihevc_trans_32_intr_odd[32][16];

extern const WORD16 g_ai2_ihevc_trans_32_intr_even[22][8];

#ifndef DISABLE_AVX2
extern const WORD16 g_ai2_ihevc_trans_8_intr_avx2[8][16];
extern const WORD32 g_ai4_ihevc_trans_8_intr_avx2[7][8];
extern const WORD16 g_ai2_ihevc_trans_16_intr_odd_avx2[32][16];
extern const WORD16 g_ai2_ihevc_trans_16_intr_even_avx2[12][16];
extern const WORD32 g_ai2_ihevc_trans_32_intr_8_avx2[8][8];
extern const WORD32 g_ai2_ihevc_trans_32_intr_16_avx2[15][8];
#endif

extern MEM_ALIGN16 const WORD16 g_ai2_ihevc_trans_16_even_packed[12][8];
extern MEM_ALIGN16 const WORD16 g_ai2_ihevc_trans_32_intr_packed[32][8];
extern MEM_ALIGN16 const WORD16 g_ai2_ihevc_trans_32_intr_odd_packed[128][8];

extern MEM_ALIGN16 const WORD16 g_ai2_ihevc_trans_16_even[12][8];
extern MEM_ALIGN16 const WORD16 g_ai2_ihevc_trans_16_odd[32][8];

extern MEM_ALIGN16 const WORD16 g_ai2_ihevc_trans_intr_even_8[4][8];
extern MEM_ALIGN16 const WORD16 g_ai2_ihevc_trans_intr_odd_8[8][8];

extern const WORD16 g_ai2_ihevc_trans_intr_4[4][8];

extern const UWORD8 IHEVCE_CHROMA_SHUFFLEMASK_HBD[8];

#endif /*_IHEVC_TRANS_TABLES_H_*/
