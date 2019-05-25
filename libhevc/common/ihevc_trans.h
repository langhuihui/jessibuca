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
*  ihevc_trans.h
*
* @brief
*  Functions declarations for forward transform
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_TRANS_H_
#define _IHEVC_TRANS_H_

typedef void ihevc_trans_4x4_ttype1_ft(WORD16 *pi2_src,
                                       WORD16 *pi2_dst,
                                       WORD32 i4_src_strd,
                                       WORD32 i4_dst_strd,
                                       WORD32 i4_shift,
                                       WORD32 i4_zero_rows);
typedef void ihevc_trans_4x4_ft(WORD16 *pi2_src,
                                WORD16 *pi2_dst,
                                WORD32 i4_src_strd,
                                WORD32 i4_dst_strd,
                                WORD32 i4_shift,
                                WORD32 i4_zero_rows);
typedef void ihevc_trans_8x8_ft(WORD16 *pi2_src,
                                WORD16 *pi2_dst,
                                WORD32 i4_src_strd,
                                WORD32 i4_dst_strd,
                                WORD32 i4_shift,
                                WORD32 i4_zero_rows);
typedef void ihevc_trans_16x16_ft(WORD16 *pi2_src,
                                  WORD16 *pi2_dst,
                                  WORD32 i4_src_strd,
                                  WORD32 i4_dst_strd,
                                  WORD32 i4_shift,
                                  WORD32 i4_zero_rows);
typedef void ihevc_trans_32x32_ft(WORD16 *pi2_src,
                                  WORD16 *pi2_dst,
                                  WORD32 i4_src_strd,
                                  WORD32 i4_dst_strd,
                                  WORD32 i4_shift,
                                  WORD32 i4_zero_rows);

ihevc_trans_4x4_ttype1_ft ihevc_trans_4x4_ttype1;
ihevc_trans_4x4_ft ihevc_trans_4x4;
ihevc_trans_8x8_ft ihevc_trans_8x8;
ihevc_trans_16x16_ft ihevc_trans_16x16;
ihevc_trans_32x32_ft ihevc_trans_32x32;


#endif /*_IHEVC_TRANS_H_*/
