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
*  ihevc_padding.h
*
* @brief
*  Declarations for the fucntions defined in  ihevc_padding.c
*
* @author
*  Srinivas T
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_PADDING_H_
#define _IHEVC_PADDING_H_

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

typedef void ihevc_pad_horz_luma_ft(
                UWORD8 *pu1_src,
                WORD32 src_strd,
                WORD32 ht,
                WORD32 wd,
                WORD32 pad_size);

typedef void ihevc_hbd_pad_horz_luma_ft(
                UWORD16 *pu2_src,
                WORD32 src_strd,
                WORD32 ht,
                WORD32 wd,
                WORD32 pad_size);

typedef void ihevc_pad_horz_chroma_ft(
                UWORD8 *pu1_src,
                WORD32 src_strd,
                WORD32 ht,
                WORD32 wd,
                WORD32 pad_size);

typedef void ihevc_hbd_pad_horz_chroma_ft(
                UWORD16 *pu2_src,
                WORD32 src_strd,
                WORD32 ht,
                WORD32 wd,
                WORD32 pad_size);

typedef void ihevc_pad_vert_ft(
                UWORD8 *pu1_src,
                WORD32 src_strd,
                WORD32 ht,
                WORD32 wd,
                WORD32 pad_size);

typedef void ihevc_hbd_pad_vert_ft(
                UWORD16 *pu2_src,
                WORD32 src_strd,
                WORD32 ht,
                WORD32 wd,
                WORD32 pad_size);

typedef void ihevc_pad_top_ft(UWORD8 *pu1_src,
                              WORD32 src_strd,
                              WORD32 wd,
                              WORD32 pad_size);

typedef void ihevc_hbd_pad_top_ft(UWORD16 *pu2_src,
                                  WORD32 src_strd,
                                  WORD32 wd,
                                  WORD32 pad_size);

typedef void ihevc_pad_bottom_ft(UWORD8 *pu1_src,
                                 WORD32 src_strd,
                                 WORD32 wd,
                                 WORD32 pad_size);

typedef void ihevc_hbd_pad_bottom_ft(UWORD16 *pu2_src,
                                     WORD32 src_strd,
                                     WORD32 wd,
                                     WORD32 pad_size);

typedef void ihevc_pad_left_luma_ft(UWORD8 *pu1_src,
                                    WORD32 src_strd,
                                    WORD32 ht,
                                    WORD32 pad_size);

typedef void ihevc_hbd_pad_left_luma_ft(UWORD16 *pu2_src,
                                        WORD32 src_strd,
                                        WORD32 ht,
                                        WORD32 pad_size);

typedef void ihevc_pad_left_chroma_ft(UWORD8 *pu1_src,
                                      WORD32 src_strd,
                                      WORD32 ht,
                                      WORD32 pad_size);

typedef void ihevc_hbd_pad_left_chroma_ft(UWORD16 *pu2_src,
                                          WORD32 src_strd,
                                          WORD32 ht,
                                          WORD32 pad_size);

typedef void ihevc_pad_right_luma_ft(UWORD8 *pu1_src,
                                     WORD32 src_strd,
                                     WORD32 ht,
                                     WORD32 pad_size);

typedef void ihevc_hbd_pad_right_luma_ft(UWORD16 *pu2_src,
                                         WORD32 src_strd,
                                         WORD32 ht,
                                         WORD32 pad_size);

typedef void ihevc_pad_right_chroma_ft(UWORD8 *pu1_src,
                                       WORD32 src_strd,
                                       WORD32 ht,
                                       WORD32 pad_size);

typedef void ihevc_hbd_pad_right_chroma_ft(UWORD16 *pu2_src,
                                           WORD32 src_strd,
                                           WORD32 ht,
                                           WORD32 pad_size);

/* C function declarations */
ihevc_pad_horz_luma_ft ihevc_pad_horz_luma;
ihevc_pad_horz_chroma_ft ihevc_pad_horz_chroma;
ihevc_pad_vert_ft ihevc_pad_vert;
ihevc_pad_top_ft ihevc_pad_top;
ihevc_pad_bottom_ft ihevc_pad_bottom;
ihevc_pad_left_luma_ft ihevc_pad_left_luma;
ihevc_pad_left_chroma_ft ihevc_pad_left_chroma;
ihevc_pad_right_luma_ft ihevc_pad_right_luma;
ihevc_pad_right_chroma_ft ihevc_pad_right_chroma;

ihevc_hbd_pad_horz_luma_ft ihevc_hbd_pad_horz_luma;
ihevc_hbd_pad_horz_chroma_ft ihevc_hbd_pad_horz_chroma;
ihevc_hbd_pad_vert_ft ihevc_hbd_pad_vert;
ihevc_hbd_pad_top_ft ihevc_hbd_pad_top;
ihevc_hbd_pad_bottom_ft ihevc_hbd_pad_bottom;
ihevc_hbd_pad_left_luma_ft ihevc_hbd_pad_left_luma;
ihevc_hbd_pad_left_chroma_ft ihevc_hbd_pad_left_chroma;
ihevc_hbd_pad_right_luma_ft ihevc_hbd_pad_right_luma;
ihevc_hbd_pad_right_chroma_ft ihevc_hbd_pad_right_chroma;

/* A9 Q function declarations */
ihevc_pad_horz_luma_ft ihevc_pad_horz_luma_a9q;
ihevc_pad_horz_chroma_ft ihevc_pad_horz_chroma_a9q;
ihevc_pad_vert_ft ihevc_pad_vert_a9q;
ihevc_pad_top_ft ihevc_pad_top_a9q;
ihevc_pad_bottom_ft ihevc_pad_bottom_a9q;
ihevc_pad_left_luma_ft ihevc_pad_left_luma_a9q;
ihevc_pad_left_chroma_ft ihevc_pad_left_chroma_a9q;
ihevc_pad_right_luma_ft ihevc_pad_right_luma_a9q;
ihevc_pad_right_chroma_ft ihevc_pad_right_chroma_a9q;

/* A9 a function declarations */
ihevc_pad_horz_luma_ft ihevc_pad_horz_luma_a9a;
ihevc_pad_horz_chroma_ft ihevc_pad_horz_chroma_a9a;
ihevc_pad_vert_ft ihevc_pad_vert_a9a;
ihevc_pad_top_ft ihevc_pad_top_a9a;
ihevc_pad_bottom_ft ihevc_pad_bottom_a9a;
ihevc_pad_left_luma_ft ihevc_pad_left_luma_a9a;
ihevc_pad_left_chroma_ft ihevc_pad_left_chroma_a9a;
ihevc_pad_right_luma_ft ihevc_pad_right_luma_a9a;
ihevc_pad_right_chroma_ft ihevc_pad_right_chroma_a9a;

/* NEONINTR function declarations */
ihevc_pad_horz_luma_ft ihevc_pad_horz_luma_neonintr;
ihevc_pad_horz_chroma_ft ihevc_pad_horz_chroma_neonintr;
ihevc_pad_vert_ft ihevc_pad_vert_neonintr;
ihevc_pad_top_ft ihevc_pad_top_neonintr;
ihevc_pad_bottom_ft ihevc_pad_bottom_neonintr;
/*SSSE3 functions declarations */
ihevc_pad_left_luma_ft ihevc_pad_left_luma_ssse3;
ihevc_pad_left_chroma_ft ihevc_pad_left_chroma_ssse3;
ihevc_pad_right_luma_ft ihevc_pad_right_luma_ssse3;
ihevc_pad_right_chroma_ft ihevc_pad_right_chroma_ssse3;

/* armv8 function declarations */
ihevc_pad_horz_luma_ft ihevc_pad_horz_luma_av8;
ihevc_pad_horz_chroma_ft ihevc_pad_horz_chroma_av8;
ihevc_pad_vert_ft ihevc_pad_vert_av8;
ihevc_pad_top_ft ihevc_pad_top_av8;
ihevc_pad_bottom_ft ihevc_pad_bottom_av8;
ihevc_pad_left_luma_ft ihevc_pad_left_luma_av8;
ihevc_pad_left_chroma_ft ihevc_pad_left_chroma_av8;
ihevc_pad_right_luma_ft ihevc_pad_right_luma_av8;
ihevc_pad_right_chroma_ft ihevc_pad_right_chroma_av8;

#endif /*_IHEVC_PADDING_H_*/
