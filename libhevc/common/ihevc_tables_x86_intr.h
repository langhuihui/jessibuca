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
*  ihevc_tables_x86_intr.h
*
* @brief
*  Declarations for the fucntions defined in  ihevc_intra_pred_filters
*
* @author
*  Mamatha
*
*
* @remarks
*  None
*
*******************************************************************************
*/

#ifndef IHEVC_TABLES_X86_INTR_H_
#define IHEVC_TABLES_X86_INTR_H_


//Luma intra pred
extern MEM_ALIGN16 const UWORD8 IHEVCE_SHUFFLEMASKY1[16];
extern MEM_ALIGN16 const UWORD8 IHEVCE_SHUFFLEMASKY2[16];
extern MEM_ALIGN16 const UWORD8 IHEVCE_SHUFFLEMASKY3[16];
extern MEM_ALIGN16 const UWORD8 IHEVCE_SHUFFLEMASK4[16];
extern MEM_ALIGN16 const UWORD8 IHEVCE_SHUFFLEMASK5[16];
//Chroma intra pred
extern MEM_ALIGN16 const UWORD8 IHEVCE_SHUFFLEMASKY7[16];

extern MEM_ALIGN16 const UWORD8 IHEVCE_SHUFFLEMASKY8[16];

extern MEM_ALIGN16 const UWORD8 IHEVCE_SHUFFLEMASKY9[16];

extern MEM_ALIGN16 const UWORD8 IHEVCE_SHUFFLEMASKY11[16];

extern MEM_ALIGN16 const UWORD8 inv_angle_shuffle[7][32];
// DEBLOCK TABLES
extern MEM_ALIGN16 const WORD8 coef_d[16];
extern MEM_ALIGN16 const WORD8 coef_de1[16];
extern MEM_ALIGN16 const WORD8 coef_dep1[16];
extern MEM_ALIGN16 const WORD32 shuffle_d[4];
extern const WORD32 shuffle0[2];
extern MEM_ALIGN16 const WORD32 shuffle1[4];
extern MEM_ALIGN16 const WORD32 shuffle2[4];
extern MEM_ALIGN16 const WORD32 shuffle3[4];

extern MEM_ALIGN16 const WORD8 delta0[16];
extern MEM_ALIGN16 const WORD8 delta1[16];
extern MEM_ALIGN16 const WORD32 shuffle_uv[4];

#endif /*IHEVC_TABLES_X86_INTR_H_*/
