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
*  ihevc_intra_pred.h
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

#ifndef IHEVC_CHROMA_INTRA_PRED_H_
#define IHEVC_CHROMA_INTRA_PRED_H_

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/
typedef void ihevc_intra_pred_chroma_planar_ft(
                UWORD8 *pu1_ref,
                WORD32 src_strd,
                UWORD8 *pu1_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_intra_pred_chroma_dc_ft(
                UWORD8 *pu1_ref,
                WORD32 src_strd,
                UWORD8 *pu1_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_intra_pred_chroma_horz_ft(
                UWORD8 *pu1_ref,
                WORD32 src_strd,
                UWORD8 *pu1_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_intra_pred_chroma_ver_ft(
                UWORD8 *pu1_ref,
                WORD32 src_strd,
                UWORD8 *pu1_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_intra_pred_chroma_mode2_ft(
                UWORD8 *pu1_ref,
                WORD32 src_strd,
                UWORD8 *pu1_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_intra_pred_chroma_mode_18_34_ft(
                UWORD8 *pu1_ref,
                WORD32 src_strd,
                UWORD8 *pu1_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_intra_pred_chroma_mode_3_to_9_ft(
                UWORD8 *pu1_ref,
                WORD32 src_strd,
                UWORD8 *pu1_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_intra_pred_chroma_mode_11_to_17_ft(
                UWORD8 *pu1_ref,
                WORD32 src_strd,
                UWORD8 *pu1_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_intra_pred_chroma_mode_19_to_25_ft(
                UWORD8 *pu1_ref,
                WORD32 src_strd,
                UWORD8 *pu1_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_intra_pred_chroma_mode_27_to_33_ft(
                UWORD8 *pu1_ref,
                WORD32 src_strd,
                UWORD8 *pu1_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_intra_pred_chroma_ref_substitution_ft(UWORD8 *pu1_top_left,
                                                         UWORD8 *pu1_top,
                                                         UWORD8 *pu1_left,
                                                         WORD32 src_strd,
                                                         WORD32 nt,
                                                         WORD32 nbr_flags,
                                                         UWORD8 *pu1_dst,
                                                         WORD32 dst_strd);

typedef void ihevc_hbd_intra_pred_chroma_planar_ft(
                UWORD16 *pu2_ref,
                WORD32 src_strd,
                UWORD16 *pu2_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);


typedef void ihevc_hbd_intra_pred_chroma_dc_ft(
                UWORD16 *pu2_ref,
                WORD32 src_strd,
                UWORD16 *pu2_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);


typedef void ihevc_hbd_intra_pred_chroma_horz_ft(
                UWORD16 *pu2_ref,
                WORD32 src_strd,
                UWORD16 *pu2_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_hbd_intra_pred_chroma_ver_ft(
                UWORD16 *pu2_ref,
                WORD32 src_strd,
                UWORD16 *pu2_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);


typedef void ihevc_hbd_intra_pred_chroma_mode2_ft(
                UWORD16 *pu2_ref,
                WORD32 src_strd,
                UWORD16 *pu2_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);



typedef void ihevc_hbd_intra_pred_chroma_mode_18_34_ft(
                UWORD16 *pu2_ref,
                WORD32 src_strd,
                UWORD16 *pu2_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);

typedef void ihevc_hbd_intra_pred_chroma_mode_3_to_9_ft(
                UWORD16 *pu2_ref,
                WORD32 src_strd,
                UWORD16 *pu2_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);


typedef void ihevc_hbd_intra_pred_chroma_mode_11_to_17_ft(
                UWORD16 *pu2_ref,
                WORD32 src_strd,
                UWORD16 *pu2_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);



typedef void ihevc_hbd_intra_pred_chroma_mode_19_to_25_ft(
                UWORD16 *pu2_ref,
                WORD32 src_strd,
                UWORD16 *pu2_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);


typedef void ihevc_hbd_intra_pred_chroma_mode_27_to_33_ft(
                UWORD16 *pu2_ref,
                WORD32 src_strd,
                UWORD16 *pu2_dst,
                WORD32 dst_strd,
                WORD32 nt,
                WORD32 mode);


typedef void ihevc_hbd_intra_pred_chroma_ref_substitution_ft(UWORD16 *pu2_top_left,
                                                             UWORD16 *pu2_top,
                                                             UWORD16 *pu2_left,
                                                             WORD32 src_strd,
                                                             WORD32 nt,
                                                             WORD32 nbr_flags,
                                                             UWORD16 *pu2_dst,
                                                             WORD32 dst_strd,
                                                             UWORD8 bit_depth);

/* C function declarations */
ihevc_intra_pred_chroma_planar_ft ihevc_intra_pred_chroma_planar;
ihevc_intra_pred_chroma_dc_ft ihevc_intra_pred_chroma_dc;
ihevc_intra_pred_chroma_horz_ft ihevc_intra_pred_chroma_horz;
ihevc_intra_pred_chroma_ver_ft ihevc_intra_pred_chroma_ver;
ihevc_intra_pred_chroma_mode2_ft ihevc_intra_pred_chroma_mode2;
ihevc_intra_pred_chroma_mode_18_34_ft ihevc_intra_pred_chroma_mode_18_34;
ihevc_intra_pred_chroma_mode_3_to_9_ft ihevc_intra_pred_chroma_mode_3_to_9;
ihevc_intra_pred_chroma_mode_11_to_17_ft ihevc_intra_pred_chroma_mode_11_to_17;
ihevc_intra_pred_chroma_mode_19_to_25_ft ihevc_intra_pred_chroma_mode_19_to_25;
ihevc_intra_pred_chroma_mode_27_to_33_ft ihevc_intra_pred_chroma_mode_27_to_33;
ihevc_intra_pred_chroma_ref_substitution_ft ihevc_intra_pred_chroma_ref_substitution;

ihevc_hbd_intra_pred_chroma_planar_ft ihevc_hbd_intra_pred_chroma_planar;
ihevc_hbd_intra_pred_chroma_dc_ft ihevc_hbd_intra_pred_chroma_dc;
ihevc_hbd_intra_pred_chroma_horz_ft ihevc_hbd_intra_pred_chroma_horz;
ihevc_hbd_intra_pred_chroma_ver_ft ihevc_hbd_intra_pred_chroma_ver;
ihevc_hbd_intra_pred_chroma_mode2_ft ihevc_hbd_intra_pred_chroma_mode2;
ihevc_hbd_intra_pred_chroma_mode_18_34_ft ihevc_hbd_intra_pred_chroma_mode_18_34;
ihevc_hbd_intra_pred_chroma_mode_3_to_9_ft ihevc_hbd_intra_pred_chroma_mode_3_to_9;
ihevc_hbd_intra_pred_chroma_mode_11_to_17_ft ihevc_hbd_intra_pred_chroma_mode_11_to_17;
ihevc_hbd_intra_pred_chroma_mode_19_to_25_ft ihevc_hbd_intra_pred_chroma_mode_19_to_25;
ihevc_hbd_intra_pred_chroma_mode_27_to_33_ft ihevc_hbd_intra_pred_chroma_mode_27_to_33;
ihevc_hbd_intra_pred_chroma_ref_substitution_ft ihevc_hbd_intra_pred_chroma_ref_substitution;

/* C function declarations */
ihevc_intra_pred_chroma_planar_ft ihevc_intra_pred_chroma_planar;
ihevc_intra_pred_chroma_dc_ft ihevc_intra_pred_chroma_dc;
ihevc_intra_pred_chroma_horz_ft ihevc_intra_pred_chroma_horz;
ihevc_intra_pred_chroma_ver_ft ihevc_intra_pred_chroma_ver;
ihevc_intra_pred_chroma_mode2_ft ihevc_intra_pred_chroma_mode2;
ihevc_intra_pred_chroma_mode_18_34_ft ihevc_intra_pred_chroma_mode_18_34;
ihevc_intra_pred_chroma_mode_3_to_9_ft ihevc_intra_pred_chroma_mode_3_to_9;
ihevc_intra_pred_chroma_mode_11_to_17_ft ihevc_intra_pred_chroma_mode_11_to_17;
ihevc_intra_pred_chroma_mode_19_to_25_ft ihevc_intra_pred_chroma_mode_19_to_25;
ihevc_intra_pred_chroma_mode_27_to_33_ft ihevc_intra_pred_chroma_mode_27_to_33;
ihevc_intra_pred_chroma_ref_substitution_ft ihevc_intra_pred_chroma_ref_substitution;

/* A9Q function declarations */
ihevc_intra_pred_chroma_planar_ft ihevc_intra_pred_chroma_planar_a9q;
ihevc_intra_pred_chroma_dc_ft ihevc_intra_pred_chroma_dc_a9q;
ihevc_intra_pred_chroma_horz_ft ihevc_intra_pred_chroma_horz_a9q;
ihevc_intra_pred_chroma_ver_ft ihevc_intra_pred_chroma_ver_a9q;
ihevc_intra_pred_chroma_mode2_ft ihevc_intra_pred_chroma_mode2_a9q;
ihevc_intra_pred_chroma_mode_18_34_ft ihevc_intra_pred_chroma_mode_18_34_a9q;
ihevc_intra_pred_chroma_mode_3_to_9_ft ihevc_intra_pred_chroma_mode_3_to_9_a9q;
ihevc_intra_pred_chroma_mode_11_to_17_ft ihevc_intra_pred_chroma_mode_11_to_17_a9q;
ihevc_intra_pred_chroma_mode_19_to_25_ft ihevc_intra_pred_chroma_mode_19_to_25_a9q;
ihevc_intra_pred_chroma_mode_27_to_33_ft ihevc_intra_pred_chroma_mode_27_to_33_a9q;
ihevc_intra_pred_chroma_ref_substitution_ft ihevc_intra_pred_chroma_ref_substitution_a9q;

/* SSE4.2 function declarations */
ihevc_intra_pred_chroma_planar_ft ihevc_intra_pred_chroma_planar_neonintr;
ihevc_intra_pred_chroma_dc_ft ihevc_intra_pred_chroma_dc_neonintr;
ihevc_intra_pred_chroma_horz_ft ihevc_intra_pred_chroma_horz_neonintr;
ihevc_intra_pred_chroma_ver_ft ihevc_intra_pred_chroma_ver_neonintr;
ihevc_intra_pred_chroma_mode2_ft ihevc_intra_pred_chroma_mode2_neonintr;
ihevc_intra_pred_chroma_mode_18_34_ft ihevc_intra_pred_chroma_mode_18_34_neonintr;
ihevc_intra_pred_chroma_mode_3_to_9_ft ihevc_intra_pred_chroma_mode_3_to_9_neonintr;
ihevc_intra_pred_chroma_mode_11_to_17_ft ihevc_intra_pred_chroma_mode_11_to_17_neonintr;
ihevc_intra_pred_chroma_mode_19_to_25_ft ihevc_intra_pred_chroma_mode_19_to_25_neonintr;
ihevc_intra_pred_chroma_mode_27_to_33_ft ihevc_intra_pred_chroma_mode_27_to_33_neonintr;
ihevc_intra_pred_chroma_ref_substitution_ft ihevc_intra_pred_chroma_ref_substitution_neonintr;

/* SSSE3 function declarations */
ihevc_intra_pred_chroma_planar_ft ihevc_intra_pred_chroma_planar_ssse3;
ihevc_intra_pred_chroma_dc_ft ihevc_intra_pred_chroma_dc_ssse3;
ihevc_intra_pred_chroma_horz_ft ihevc_intra_pred_chroma_horz_ssse3;
ihevc_intra_pred_chroma_ver_ft ihevc_intra_pred_chroma_ver_ssse3;
ihevc_intra_pred_chroma_mode2_ft ihevc_intra_pred_chroma_mode2_ssse3;
ihevc_intra_pred_chroma_mode_18_34_ft ihevc_intra_pred_chroma_mode_18_34_ssse3;
ihevc_intra_pred_chroma_mode_3_to_9_ft ihevc_intra_pred_chroma_mode_3_to_9_ssse3;
ihevc_intra_pred_chroma_mode_11_to_17_ft ihevc_intra_pred_chroma_mode_11_to_17_ssse3;
ihevc_intra_pred_chroma_mode_19_to_25_ft ihevc_intra_pred_chroma_mode_19_to_25_ssse3;
ihevc_intra_pred_chroma_mode_27_to_33_ft ihevc_intra_pred_chroma_mode_27_to_33_ssse3;
ihevc_intra_pred_chroma_ref_substitution_ft ihevc_intra_pred_chroma_ref_substitution_ssse3;

/* SSE4.2 function declarations */
ihevc_intra_pred_chroma_planar_ft ihevc_intra_pred_chroma_planar_sse42;
ihevc_intra_pred_chroma_dc_ft ihevc_intra_pred_chroma_dc_sse42;
ihevc_intra_pred_chroma_ref_substitution_ft ihevc_intra_pred_chroma_ref_substitution_sse42;

ihevc_hbd_intra_pred_chroma_planar_ft ihevc_hbd_intra_pred_chroma_planar_sse42;
ihevc_hbd_intra_pred_chroma_dc_ft ihevc_hbd_intra_pred_chroma_dc_sse42;
ihevc_hbd_intra_pred_chroma_horz_ft ihevc_hbd_intra_pred_chroma_horz_sse42;
ihevc_hbd_intra_pred_chroma_ver_ft ihevc_hbd_intra_pred_chroma_ver_sse42;
ihevc_hbd_intra_pred_chroma_mode2_ft ihevc_hbd_intra_pred_chroma_mode2_sse42;
ihevc_hbd_intra_pred_chroma_mode_18_34_ft ihevc_hbd_intra_pred_chroma_mode_18_34_sse42;
ihevc_hbd_intra_pred_chroma_mode_3_to_9_ft ihevc_hbd_intra_pred_chroma_mode_3_to_9_sse42;
ihevc_hbd_intra_pred_chroma_mode_11_to_17_ft ihevc_hbd_intra_pred_chroma_mode_11_to_17_sse42;
ihevc_hbd_intra_pred_chroma_mode_19_to_25_ft ihevc_hbd_intra_pred_chroma_mode_19_to_25_sse42;
ihevc_hbd_intra_pred_chroma_mode_27_to_33_ft ihevc_hbd_intra_pred_chroma_mode_27_to_33_sse42;
ihevc_hbd_intra_pred_chroma_ref_substitution_ft ihevc_hbd_intra_pred_chroma_ref_substitution_sse42;

ihevc_intra_pred_chroma_planar_ft ihevc_intra_pred_chroma_planar_a9a;
ihevc_intra_pred_chroma_dc_ft ihevc_intra_pred_chroma_dc_a9a;
ihevc_intra_pred_chroma_horz_ft ihevc_intra_pred_chroma_horz_a9a;
ihevc_intra_pred_chroma_ver_ft ihevc_intra_pred_chroma_ver_a9a;
ihevc_intra_pred_chroma_mode2_ft ihevc_intra_pred_chroma_mode2_a9a;
ihevc_intra_pred_chroma_mode_18_34_ft ihevc_intra_pred_chroma_mode_18_34_a9a;
ihevc_intra_pred_chroma_mode_3_to_9_ft ihevc_intra_pred_chroma_mode_3_to_9_a9a;
ihevc_intra_pred_chroma_mode_11_to_17_ft ihevc_intra_pred_chroma_mode_11_to_17_a9a;
ihevc_intra_pred_chroma_mode_19_to_25_ft ihevc_intra_pred_chroma_mode_19_to_25_a9a;
ihevc_intra_pred_chroma_mode_27_to_33_ft ihevc_intra_pred_chroma_mode_27_to_33_a9a;
ihevc_intra_pred_chroma_ref_substitution_ft ihevc_intra_pred_chroma_ref_substitution_a9a;

/* AVX function declaration*/
ihevc_intra_pred_chroma_dc_ft ihevc_intra_pred_chroma_dc_avx;
ihevc_intra_pred_chroma_mode_18_34_ft ihevc_intra_pred_chroma_mode_18_34_avx;
ihevc_intra_pred_chroma_ver_ft ihevc_intra_pred_chroma_ver_avx;

ihevc_hbd_intra_pred_chroma_dc_ft ihevc_hbd_intra_pred_chroma_dc_avx;
ihevc_hbd_intra_pred_chroma_mode_18_34_ft ihevc_hbd_intra_pred_chroma_mode_18_34_avx;
ihevc_hbd_intra_pred_chroma_ver_ft ihevc_hbd_intra_pred_chroma_ver_avx;

/* armv8 function declarations */
ihevc_intra_pred_chroma_planar_ft ihevc_intra_pred_chroma_planar_av8;
ihevc_intra_pred_chroma_dc_ft ihevc_intra_pred_chroma_dc_av8;
ihevc_intra_pred_chroma_horz_ft ihevc_intra_pred_chroma_horz_av8;
ihevc_intra_pred_chroma_ver_ft ihevc_intra_pred_chroma_ver_av8;
ihevc_intra_pred_chroma_mode2_ft ihevc_intra_pred_chroma_mode2_av8;
ihevc_intra_pred_chroma_mode_18_34_ft ihevc_intra_pred_chroma_mode_18_34_av8;
ihevc_intra_pred_chroma_mode_3_to_9_ft ihevc_intra_pred_chroma_mode_3_to_9_av8;
ihevc_intra_pred_chroma_mode_11_to_17_ft ihevc_intra_pred_chroma_mode_11_to_17_av8;
ihevc_intra_pred_chroma_mode_19_to_25_ft ihevc_intra_pred_chroma_mode_19_to_25_av8;
ihevc_intra_pred_chroma_mode_27_to_33_ft ihevc_intra_pred_chroma_mode_27_to_33_av8;
ihevc_intra_pred_chroma_ref_substitution_ft ihevc_intra_pred_chroma_ref_substitution_av8;
#endif /* IHEVC_CHROMA_INTRA_PRED_H_ */
