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
*  ihevc_deblk.h
*
* @brief
*  Declarations for the fucntions defined in  ihevc_deblk.c
*
* @author
*  Srinivas T
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_DEBLK_H_
#define _IHEVC_DEBLK_H_

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

typedef void ihevc_deblk_luma_vert_ft(
                UWORD8 *pu1_src,
                WORD32 src_strd,
                WORD32 bs,
                WORD32 quant_param_p,
                WORD32 quant_param_q,
                WORD32 beta_offset_div2,
                WORD32 tc_offset_div2,
                WORD32 filter_flag_p,
                WORD32 filter_flag_q);

typedef void ihevc_deblk_luma_horz_ft(
                UWORD8 *pu1_src,
                WORD32 src_strd,
                WORD32 bs,
                WORD32 quant_param_p,
                WORD32 quant_param_q,
                WORD32 beta_offset_div2,
                WORD32 tc_offset_div2,
                WORD32 filter_flag_p,
                WORD32 filter_flag_q);

typedef void ihevc_deblk_chroma_vert_ft(
                UWORD8 *pu1_src,
                WORD32 src_strd,
                WORD32 quant_param_p,
                WORD32 quant_param_q,
                WORD32 qp_offset_u,
                WORD32 qp_offset_v,
                WORD32 tc_offset_div2,
                WORD32 filter_flag_p,
                WORD32 filter_flag_q);

typedef void ihevc_deblk_chroma_horz_ft(
                UWORD8 *pu1_src,
                WORD32 src_strd,
                WORD32 quant_param_p,
                WORD32 quant_param_q,
                WORD32 qp_offset_u,
                WORD32 qp_offset_v,
                WORD32 tc_offset_div2,
                WORD32 filter_flag_p,
                WORD32 filter_flag_q);

typedef void ihevc_hbd_deblk_luma_vert_ft(
                UWORD16 *pu2_src,
                WORD32 src_strd,
                WORD32 bs,
                WORD32 quant_param_p,
                WORD32 quant_param_q,
                WORD32 beta_offset_div2,
                WORD32 tc_offset_div2,
                WORD32 filter_flag_p,
                WORD32 filter_flag_q,
                UWORD8 bit_depth);

typedef void ihevc_hbd_deblk_luma_horz_ft(
                UWORD16 *pu2_src,
                WORD32 src_strd,
                WORD32 bs,
                WORD32 quant_param_p,
                WORD32 quant_param_q,
                WORD32 beta_offset_div2,
                WORD32 tc_offset_div2,
                WORD32 filter_flag_p,
                WORD32 filter_flag_q,
                UWORD8 bit_depth);

typedef void ihevc_hbd_deblk_chroma_vert_ft(
                UWORD16 *pu2_src,
                WORD32 src_strd,
                WORD32 quant_param_p,
                WORD32 quant_param_q,
                WORD32 qp_offset_u,
                WORD32 qp_offset_v,
                WORD32 tc_offset_div2,
                WORD32 filter_flag_p,
                WORD32 filter_flag_q,
                UWORD8 bit_depth);

typedef void ihevc_hbd_deblk_chroma_horz_ft(
                UWORD16 *pu2_src,
                WORD32 src_strd,
                WORD32 quant_param_p,
                WORD32 quant_param_q,
                WORD32 qp_offset_u,
                WORD32 qp_offset_v,
                WORD32 tc_offset_div2,
                WORD32 filter_flag_p,
                WORD32 filter_flag_q,
                UWORD8 bit_depth);

ihevc_deblk_luma_vert_ft ihevc_deblk_luma_vert;
ihevc_deblk_luma_horz_ft ihevc_deblk_luma_horz;
ihevc_deblk_chroma_vert_ft ihevc_deblk_chroma_vert;
ihevc_deblk_chroma_horz_ft ihevc_deblk_chroma_horz;
ihevc_deblk_chroma_vert_ft ihevc_deblk_422chroma_vert;
ihevc_deblk_chroma_horz_ft ihevc_deblk_422chroma_horz;

ihevc_deblk_luma_vert_ft ihevc_deblk_luma_vert_a9q;
ihevc_deblk_luma_horz_ft ihevc_deblk_luma_horz_a9q;
ihevc_deblk_chroma_vert_ft ihevc_deblk_chroma_vert_a9q;
ihevc_deblk_chroma_horz_ft ihevc_deblk_chroma_horz_a9q;

ihevc_deblk_luma_vert_ft ihevc_deblk_luma_vert_a9a;
ihevc_deblk_luma_horz_ft ihevc_deblk_luma_horz_a9a;
ihevc_deblk_chroma_vert_ft ihevc_deblk_chroma_vert_a9a;
ihevc_deblk_chroma_horz_ft ihevc_deblk_chroma_horz_a9a;

ihevc_deblk_luma_vert_ft ihevc_deblk_luma_vert_neonintr;
ihevc_deblk_luma_horz_ft ihevc_deblk_luma_horz_neonintr;
ihevc_deblk_chroma_vert_ft ihevc_deblk_chroma_vert_neonintr;
ihevc_deblk_chroma_horz_ft ihevc_deblk_chroma_horz_neonintr;

ihevc_deblk_luma_vert_ft ihevc_deblk_luma_vert_ssse3;
ihevc_deblk_luma_horz_ft ihevc_deblk_luma_horz_ssse3;
ihevc_deblk_chroma_vert_ft ihevc_deblk_chroma_vert_ssse3;
ihevc_deblk_chroma_horz_ft ihevc_deblk_chroma_horz_ssse3;
ihevc_deblk_chroma_vert_ft ihevc_deblk_422chroma_vert_ssse3;
ihevc_deblk_chroma_horz_ft ihevc_deblk_422chroma_horz_ssse3;

ihevc_hbd_deblk_luma_vert_ft ihevc_hbd_deblk_luma_vert;
ihevc_hbd_deblk_luma_horz_ft ihevc_hbd_deblk_luma_horz;
ihevc_hbd_deblk_chroma_vert_ft ihevc_hbd_deblk_chroma_vert;
ihevc_hbd_deblk_chroma_horz_ft ihevc_hbd_deblk_chroma_horz;
ihevc_hbd_deblk_chroma_vert_ft ihevc_hbd_deblk_422chroma_vert;
ihevc_hbd_deblk_chroma_horz_ft ihevc_hbd_deblk_422chroma_horz;

ihevc_hbd_deblk_luma_vert_ft ihevc_hbd_deblk_luma_vert_sse42;
ihevc_hbd_deblk_luma_horz_ft ihevc_hbd_deblk_luma_horz_sse42;
ihevc_hbd_deblk_chroma_vert_ft ihevc_hbd_deblk_chroma_vert_sse42;
ihevc_hbd_deblk_chroma_horz_ft ihevc_hbd_deblk_chroma_horz_sse42;
ihevc_hbd_deblk_chroma_vert_ft ihevc_hbd_deblk_422chroma_vert_sse42;
ihevc_hbd_deblk_chroma_horz_ft ihevc_hbd_deblk_422chroma_horz_sse42;

ihevc_deblk_luma_vert_ft ihevc_deblk_luma_vert_av8;
ihevc_deblk_luma_horz_ft ihevc_deblk_luma_horz_av8;
ihevc_deblk_chroma_vert_ft ihevc_deblk_chroma_vert_av8;
ihevc_deblk_chroma_horz_ft ihevc_deblk_chroma_horz_av8;

#endif /*_IHEVC_DEBLK_H_*/
