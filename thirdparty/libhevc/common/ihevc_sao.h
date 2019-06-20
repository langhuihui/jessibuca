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
******************************************************************************
* @file ihevc_sao.h
*
* @brief
*  This file contains enumerations, macros and extern declarations of HEVC
*  SAO
*
* @author
*  Ittiam
******************************************************************************
*/

#ifndef _IHEVC_SAO_H_
#define _IHEVC_SAO_H_

enum
{
    SAO_NONE,

    SAO_BAND,

    SAO_EDGE_0_DEG,

    SAO_EDGE_90_DEG,

    SAO_EDGE_135_DEG,

    SAO_EDGE_45_DEG
};

static const WORD32 gi4_ihevc_hbd_table_edge_idx[5] = { 1, 2, 0, 3, 4 };

typedef void ihevc_sao_band_offset_luma_ft(UWORD8 *pu1_src,
                                           WORD32 src_strd,
                                           UWORD8 *pu1_src_left,
                                           UWORD8 *pu1_src_top,
                                           UWORD8 *pu1_src_top_left,
                                           WORD32 sao_band_pos,
                                           WORD8 *pi4_sao_offset,
                                           WORD32 wd,
                                           WORD32 ht);

typedef void ihevc_hbd_sao_band_offset_luma_ft(UWORD16 *pu2_src,
                                               WORD32 src_strd,
                                               UWORD16 *pu2_src_left,
                                               UWORD16 *pu2_src_top,
                                               UWORD16 *pu2_src_top_left,
                                               WORD32 sao_band_pos,
                                               WORD8 *pi1_sao_offset,
                                               WORD32 wd,
                                               WORD32 ht,
                                               UWORD32 bitdepth);

typedef void ihevc_sao_band_offset_chroma_ft(UWORD8 *pu1_src,
                                             WORD32 src_strd,
                                             UWORD8 *pu1_src_left,
                                             UWORD8 *pu1_src_top,
                                             UWORD8 *pu1_src_top_left,
                                             WORD32 sao_band_pos_u,
                                             WORD32 sao_band_pos_v,
                                             WORD8 *pi4_sao_offset_u,
                                             WORD8 *pi4_sao_offset_v,
                                             WORD32 wd,
                                             WORD32 ht);

typedef void ihevc_hbd_sao_band_offset_chroma_ft(UWORD16 *pu2_src,
                                                 WORD32 src_strd,
                                                 UWORD16 *pu2_src_left,
                                                 UWORD16 *pu2_src_top,
                                                 UWORD16 *pu2_src_top_left,
                                                 WORD32 sao_band_pos_u,
                                                 WORD32 sao_band_pos_v,
                                                 WORD8 *pi1_sao_offset_u,
                                                 WORD8 *pi1_sao_offset_v,
                                                 WORD32 wd,
                                                 WORD32 ht,
                                                 UWORD32 bit_depth);

typedef void ihevc_sao_edge_offset_class0_ft(UWORD8 *pu1_src,
                                             WORD32 src_strd,
                                             UWORD8 *pu1_src_left,
                                             UWORD8 *pu1_src_top,
                                             UWORD8 *pu1_src_top_left,
                                             UWORD8 *pu1_src_top_right,
                                             UWORD8 *pu1_src_bot_left,
                                             UWORD8 *pu1_avail,
                                             WORD8 *pi4_sao_offset,
                                             WORD32 wd,
                                             WORD32 ht);

typedef void ihevc_hbd_sao_edge_offset_class0_ft(UWORD16 *pu2_src,
                                                 WORD32 src_strd,
                                                 UWORD16 *pu2_src_left,
                                                 UWORD16 *pu2_src_top,
                                                 UWORD16 *pu2_src_top_left,
                                                 UWORD16 *pu2_src_top_right,
                                                 UWORD16 *pu2_src_bot_left,
                                                 UWORD8 *pu1_avail,
                                                 WORD8 *pi1_sao_offset,
                                                 WORD32 wd,
                                                 WORD32 ht,
                                                 UWORD32 bit_depth);

typedef void ihevc_sao_edge_offset_class0_chroma_ft(UWORD8 *pu1_src,
                                                    WORD32 src_strd,
                                                    UWORD8 *pu1_src_left,
                                                    UWORD8 *pu1_src_top,
                                                    UWORD8 *pu1_src_top_left,
                                                    UWORD8 *pu1_src_top_right,
                                                    UWORD8 *pu1_src_bot_left,
                                                    UWORD8 *pu1_avail,
                                                    WORD8 *pi4_sao_offset_u,
                                                    WORD8 *pi4_sao_offset_v,
                                                    WORD32 wd,
                                                    WORD32 ht);

typedef void ihevc_hbd_sao_edge_offset_class0_chroma_ft(UWORD16 *pu2_src,
                                                        WORD32 src_strd,
                                                        UWORD16 *pu2_src_left,
                                                        UWORD16 *pu2_src_top,
                                                        UWORD16 *pu2_src_top_left,
                                                        UWORD16 *pu2_src_top_right,
                                                        UWORD16 *pu2_src_bot_left,
                                                        UWORD8 *pu1_avail,
                                                        WORD8 *pi1_sao_offset_u,
                                                        WORD8 *pi1_sao_offset_v,
                                                        WORD32 wd,
                                                        WORD32 ht,
                                                        UWORD32 bit_depth);

typedef void ihevc_sao_edge_offset_class1_ft(UWORD8 *pu1_src,
                                             WORD32 src_strd,
                                             UWORD8 *pu1_src_left,
                                             UWORD8 *pu1_src_top,
                                             UWORD8 *pu1_src_top_left,
                                             UWORD8 *pu1_src_top_right,
                                             UWORD8 *pu1_src_bot_left,
                                             UWORD8 *pu1_avail,
                                             WORD8 *pi4_sao_offset,
                                             WORD32 wd,
                                             WORD32 ht);

typedef void ihevc_hbd_sao_edge_offset_class1_ft(UWORD16 *pu2_src,
                                                 WORD32 src_strd,
                                                 UWORD16 *pu2_src_left,
                                                 UWORD16 *pu2_src_top,
                                                 UWORD16 *pu2_src_top_left,
                                                 UWORD16 *pu2_src_top_right,
                                                 UWORD16 *pu2_src_bot_left,
                                                 UWORD8 *pu1_avail,
                                                 WORD8 *pi1_sao_offset,
                                                 WORD32 wd,
                                                 WORD32 ht,
                                                 UWORD32 bit_depth);

typedef void ihevc_sao_edge_offset_class1_chroma_ft(UWORD8 *pu1_src,
                                                    WORD32 src_strd,
                                                    UWORD8 *pu1_src_left,
                                                    UWORD8 *pu1_src_top,
                                                    UWORD8 *pu1_src_top_left,
                                                    UWORD8 *pu1_src_top_right,
                                                    UWORD8 *pu1_src_bot_left,
                                                    UWORD8 *pu1_avail,
                                                    WORD8 *pi4_sao_offset_u,
                                                    WORD8 *pi4_sao_offset_v,
                                                    WORD32 wd,
                                                    WORD32 ht);

typedef void ihevc_hbd_sao_edge_offset_class1_chroma_ft(UWORD16 *pu2_src,
                                                        WORD32 src_strd,
                                                        UWORD16 *pu2_src_left,
                                                        UWORD16 *pu2_src_top,
                                                        UWORD16 *pu2_src_top_left,
                                                        UWORD16 *pu2_src_top_right,
                                                        UWORD16 *pu2_src_bot_left,
                                                        UWORD8 *pu1_avail,
                                                        WORD8 *pi1_sao_offset_u,
                                                        WORD8 *pi1_sao_offset_v,
                                                        WORD32 wd,
                                                        WORD32 ht,
                                                        UWORD32 bit_depth);

typedef void ihevc_sao_edge_offset_class2_ft(UWORD8 *pu1_src,
                                             WORD32 src_strd,
                                             UWORD8 *pu1_src_left,
                                             UWORD8 *pu1_src_top,
                                             UWORD8 *pu1_src_top_left,
                                             UWORD8 *pu1_src_top_right,
                                             UWORD8 *pu1_src_bot_left,
                                             UWORD8 *pu1_avail,
                                             WORD8 *pi4_sao_offset,
                                             WORD32 wd,
                                             WORD32 ht);

typedef void ihevc_hbd_sao_edge_offset_class2_ft(UWORD16 *pu2_src,
                                                 WORD32 src_strd,
                                                 UWORD16 *pu2_src_left,
                                                 UWORD16 *pu2_src_top,
                                                 UWORD16 *pu2_src_top_left,
                                                 UWORD16 *pu2_src_top_right,
                                                 UWORD16 *pu2_src_bot_left,
                                                 UWORD8 *pu1_avail,
                                                 WORD8 *pi1_sao_offset,
                                                 WORD32 wd,
                                                 WORD32 ht,
                                                 UWORD32 bit_depth);

typedef void ihevc_sao_edge_offset_class2_chroma_ft(UWORD8 *pu1_src,
                                                    WORD32 src_strd,
                                                    UWORD8 *pu1_src_left,
                                                    UWORD8 *pu1_src_top,
                                                    UWORD8 *pu1_src_top_left,
                                                    UWORD8 *pu1_src_top_right,
                                                    UWORD8 *pu1_src_bot_left,
                                                    UWORD8 *pu1_avail,
                                                    WORD8 *pi4_sao_offset_u,
                                                    WORD8 *pi4_sao_offset_v,
                                                    WORD32 wd,
                                                    WORD32 ht);

typedef void ihevc_hbd_sao_edge_offset_class2_chroma_ft(UWORD16 *pu2_src,
                                                        WORD32 src_strd,
                                                        UWORD16 *pu2_src_left,
                                                        UWORD16 *pu2_src_top,
                                                        UWORD16 *pu2_src_top_left,
                                                        UWORD16 *pu2_src_top_right,
                                                        UWORD16 *pu2_src_bot_left,
                                                        UWORD8 *pu1_avail,
                                                        WORD8 *pi1_sao_offset_u,
                                                        WORD8 *pi1_sao_offset_v,
                                                        WORD32 wd,
                                                        WORD32 ht,
                                                        UWORD32 bit_depth);

typedef void ihevc_sao_edge_offset_class3_ft(UWORD8 *pu1_src,
                                             WORD32 src_strd,
                                             UWORD8 *pu1_src_left,
                                             UWORD8 *pu1_src_top,
                                             UWORD8 *pu1_src_top_left,
                                             UWORD8 *pu1_src_top_right,
                                             UWORD8 *pu1_src_bot_left,
                                             UWORD8 *pu1_avail,
                                             WORD8 *pi4_sao_offset,
                                             WORD32 wd,
                                             WORD32 ht);

typedef void ihevc_hbd_sao_edge_offset_class3_ft(UWORD16 *pu2_src,
                                                 WORD32 src_strd,
                                                 UWORD16 *pu2_src_left,
                                                 UWORD16 *pu2_src_top,
                                                 UWORD16 *pu2_src_top_left,
                                                 UWORD16 *pu2_src_top_right,
                                                 UWORD16 *pu2_src_bot_left,
                                                 UWORD8 *pu1_avail,
                                                 WORD8 *pi1_sao_offset,
                                                 WORD32 wd,
                                                 WORD32 ht,
                                                 UWORD32 bit_depth);
typedef void ihevc_sao_edge_offset_class3_chroma_ft(UWORD8 *pu1_src,
                                                    WORD32 src_strd,
                                                    UWORD8 *pu1_src_left,
                                                    UWORD8 *pu1_src_top,
                                                    UWORD8 *pu1_src_top_left,
                                                    UWORD8 *pu1_src_top_right,
                                                    UWORD8 *pu1_src_bot_left,
                                                    UWORD8 *pu1_avail,
                                                    WORD8 *pi4_sao_offset_u,
                                                    WORD8 *pi4_sao_offset_v,
                                                    WORD32 wd,
                                                    WORD32 ht);

typedef void ihevc_hbd_sao_edge_offset_class3_chroma_ft(UWORD16 *pu2_src,
                                                        WORD32 src_strd,
                                                        UWORD16 *pu2_src_left,
                                                        UWORD16 *pu2_src_top,
                                                        UWORD16 *pu2_src_top_left,
                                                        UWORD16 *pu2_src_top_right,
                                                        UWORD16 *pu2_src_bot_left,
                                                        UWORD8 *pu1_avail,
                                                        WORD8 *pi1_sao_offset_u,
                                                        WORD8 *pi1_sao_offset_v,
                                                        WORD32 wd,
                                                        WORD32 ht,
                                                        UWORD32 bit_depth);
/* C function declarations */
ihevc_sao_band_offset_luma_ft ihevc_sao_band_offset_luma;
ihevc_sao_band_offset_chroma_ft ihevc_sao_band_offset_chroma;
ihevc_sao_edge_offset_class0_ft ihevc_sao_edge_offset_class0;
ihevc_sao_edge_offset_class0_chroma_ft ihevc_sao_edge_offset_class0_chroma;
ihevc_sao_edge_offset_class1_ft ihevc_sao_edge_offset_class1;
ihevc_sao_edge_offset_class1_chroma_ft ihevc_sao_edge_offset_class1_chroma;
ihevc_sao_edge_offset_class2_ft ihevc_sao_edge_offset_class2;
ihevc_sao_edge_offset_class2_chroma_ft ihevc_sao_edge_offset_class2_chroma;
ihevc_sao_edge_offset_class3_ft ihevc_sao_edge_offset_class3;
ihevc_sao_edge_offset_class3_chroma_ft ihevc_sao_edge_offset_class3_chroma;

/* NEONINTR function declarations */
ihevc_sao_band_offset_luma_ft ihevc_sao_band_offset_luma_neonintr;
ihevc_sao_band_offset_chroma_ft ihevc_sao_band_offset_chroma_neonintr;
ihevc_sao_edge_offset_class0_ft ihevc_sao_edge_offset_class0_neonintr;
ihevc_sao_edge_offset_class0_chroma_ft ihevc_sao_edge_offset_class0_chroma_neonintr;
ihevc_sao_edge_offset_class1_ft ihevc_sao_edge_offset_class1_neonintr;
ihevc_sao_edge_offset_class1_chroma_ft ihevc_sao_edge_offset_class1_chroma_neonintr;
ihevc_sao_edge_offset_class2_ft ihevc_sao_edge_offset_class2_neonintr;
ihevc_sao_edge_offset_class2_chroma_ft ihevc_sao_edge_offset_class2_chroma_neonintr;
ihevc_sao_edge_offset_class3_ft ihevc_sao_edge_offset_class3_neonintr;
ihevc_sao_edge_offset_class3_chroma_ft ihevc_sao_edge_offset_class3_chroma_neonintr;

/* A9Q function declarations */
ihevc_sao_band_offset_luma_ft ihevc_sao_band_offset_luma_a9q;
ihevc_sao_band_offset_chroma_ft ihevc_sao_band_offset_chroma_a9q;
ihevc_sao_edge_offset_class0_ft ihevc_sao_edge_offset_class0_a9q;
ihevc_sao_edge_offset_class0_chroma_ft ihevc_sao_edge_offset_class0_chroma_a9q;
ihevc_sao_edge_offset_class1_ft ihevc_sao_edge_offset_class1_a9q;
ihevc_sao_edge_offset_class1_chroma_ft ihevc_sao_edge_offset_class1_chroma_a9q;
ihevc_sao_edge_offset_class2_ft ihevc_sao_edge_offset_class2_a9q;
ihevc_sao_edge_offset_class2_chroma_ft ihevc_sao_edge_offset_class2_chroma_a9q;
ihevc_sao_edge_offset_class3_ft ihevc_sao_edge_offset_class3_a9q;
ihevc_sao_edge_offset_class3_chroma_ft ihevc_sao_edge_offset_class3_chroma_a9q;

/* A9A (Apple) function declarations */
ihevc_sao_band_offset_luma_ft ihevc_sao_band_offset_luma_a9a;
ihevc_sao_band_offset_chroma_ft ihevc_sao_band_offset_chroma_a9a;
ihevc_sao_edge_offset_class0_ft ihevc_sao_edge_offset_class0_a9a;
ihevc_sao_edge_offset_class0_chroma_ft ihevc_sao_edge_offset_class0_chroma_a9a;
ihevc_sao_edge_offset_class1_ft ihevc_sao_edge_offset_class1_a9a;
ihevc_sao_edge_offset_class1_chroma_ft ihevc_sao_edge_offset_class1_chroma_a9a;
ihevc_sao_edge_offset_class2_ft ihevc_sao_edge_offset_class2_a9a;
ihevc_sao_edge_offset_class2_chroma_ft ihevc_sao_edge_offset_class2_chroma_a9a;
ihevc_sao_edge_offset_class3_ft ihevc_sao_edge_offset_class3_a9a;
ihevc_sao_edge_offset_class3_chroma_ft ihevc_sao_edge_offset_class3_chroma_a9a;

/* SSSE31 function declarations */
ihevc_sao_band_offset_luma_ft ihevc_sao_band_offset_luma_ssse3;
ihevc_sao_band_offset_chroma_ft ihevc_sao_band_offset_chroma_ssse3;
ihevc_sao_edge_offset_class0_ft ihevc_sao_edge_offset_class0_ssse3;
ihevc_sao_edge_offset_class0_chroma_ft ihevc_sao_edge_offset_class0_chroma_ssse3;
ihevc_sao_edge_offset_class1_ft ihevc_sao_edge_offset_class1_ssse3;
ihevc_sao_edge_offset_class1_chroma_ft ihevc_sao_edge_offset_class1_chroma_ssse3;
ihevc_sao_edge_offset_class2_ft ihevc_sao_edge_offset_class2_ssse3;
ihevc_sao_edge_offset_class2_chroma_ft ihevc_sao_edge_offset_class2_chroma_ssse3;
ihevc_sao_edge_offset_class3_ft ihevc_sao_edge_offset_class3_ssse3;
ihevc_sao_edge_offset_class3_chroma_ft ihevc_sao_edge_offset_class3_chroma_ssse3;

/* SSE4 function declarations */

/* C high bit depth function declarations */
ihevc_hbd_sao_band_offset_luma_ft ihevc_hbd_sao_band_offset_luma;
ihevc_hbd_sao_band_offset_chroma_ft ihevc_hbd_sao_band_offset_chroma;
ihevc_hbd_sao_edge_offset_class0_ft ihevc_hbd_sao_edge_offset_class0;
ihevc_hbd_sao_edge_offset_class0_chroma_ft ihevc_hbd_sao_edge_offset_class0_chroma;
ihevc_hbd_sao_edge_offset_class1_ft ihevc_hbd_sao_edge_offset_class1;
ihevc_hbd_sao_edge_offset_class1_chroma_ft ihevc_hbd_sao_edge_offset_class1_chroma;
ihevc_hbd_sao_edge_offset_class2_ft ihevc_hbd_sao_edge_offset_class2;
ihevc_hbd_sao_edge_offset_class2_chroma_ft ihevc_hbd_sao_edge_offset_class2_chroma;
ihevc_hbd_sao_edge_offset_class3_ft ihevc_hbd_sao_edge_offset_class3;
ihevc_hbd_sao_edge_offset_class3_chroma_ft ihevc_hbd_sao_edge_offset_class3_chroma;

/* SSE4.2 HBD function Declarations*/
ihevc_hbd_sao_band_offset_luma_ft ihevc_hbd_sao_band_offset_luma_sse42;
ihevc_hbd_sao_band_offset_chroma_ft ihevc_hbd_sao_band_offset_chroma_sse42;
ihevc_hbd_sao_edge_offset_class0_ft ihevc_hbd_sao_edge_offset_class0_sse42;
ihevc_hbd_sao_edge_offset_class0_chroma_ft ihevc_hbd_sao_edge_offset_class0_chroma_sse42;
ihevc_hbd_sao_edge_offset_class1_ft ihevc_hbd_sao_edge_offset_class1_sse42;
ihevc_hbd_sao_edge_offset_class1_chroma_ft ihevc_hbd_sao_edge_offset_class1_chroma_sse42;
ihevc_hbd_sao_edge_offset_class2_ft ihevc_hbd_sao_edge_offset_class2_sse42;
ihevc_hbd_sao_edge_offset_class2_chroma_ft ihevc_hbd_sao_edge_offset_class2_chroma_sse42;
ihevc_hbd_sao_edge_offset_class3_ft ihevc_hbd_sao_edge_offset_class3_sse42;
ihevc_hbd_sao_edge_offset_class3_chroma_ft ihevc_hbd_sao_edge_offset_class3_chroma_sse42;

/* armv8 function declarations */
ihevc_sao_band_offset_luma_ft ihevc_sao_band_offset_luma_av8;
ihevc_sao_band_offset_chroma_ft ihevc_sao_band_offset_chroma_av8;
ihevc_sao_edge_offset_class0_ft ihevc_sao_edge_offset_class0_av8;
ihevc_sao_edge_offset_class0_chroma_ft ihevc_sao_edge_offset_class0_chroma_av8;
ihevc_sao_edge_offset_class1_ft ihevc_sao_edge_offset_class1_av8;
ihevc_sao_edge_offset_class1_chroma_ft ihevc_sao_edge_offset_class1_chroma_av8;
ihevc_sao_edge_offset_class2_ft ihevc_sao_edge_offset_class2_av8;
ihevc_sao_edge_offset_class2_chroma_ft ihevc_sao_edge_offset_class2_chroma_av8;
ihevc_sao_edge_offset_class3_ft ihevc_sao_edge_offset_class3_av8;
ihevc_sao_edge_offset_class3_chroma_ft ihevc_sao_edge_offset_class3_chroma_av8;

#endif /* _IHEVC_SAO_H_ */
