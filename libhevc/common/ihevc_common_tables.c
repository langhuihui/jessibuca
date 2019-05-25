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
*  ihevc_common_tables.c
*
* @brief
*  Contains common global tables
*
* @author
*  Harish M
*
* @par List of Tables:
* gai4_ihevc_max_luma_pic_size
* gai4_ihevc_max_wd_ht
* gai4_ihevc_min_wd_ht
* gai4_ihevc_ang_table
* col_for_intra_luma
* col_for_intra_chroma
* idx_neg_vals_3_9
* idx_neg_idx_3_9
* idx_neg_idx_chroma_3_9
* idx_neg_idx_11_17
* idx_neg_idx_chroma_11_17
* gai4_ihevc_inv_ang_table
* gau1_ihevc_invscan8x8
* gau1_ihevc_invscan4x4
* gau1_ihevc_invscan2x2
* gau1_ihevc_scan8x8
* gau1_ihevc_scan4x4
* gau1_ihevc_scan2x2
* *gapv_ihevc_scan
* *gapv_ihevc_invscan
* gau1_ihevc_chroma_qp_scale
* gai1_ihevc_chroma_qp_scale
* gau1_ihevc_planar_factor
* gau1_ihevc_planar_factor_1
* gai4_ihevc_ang_table_chroma
* gai4_ihevc_inv_ang_table_chroma
* gau1_ihevc_planar_factor_chroma
* gau1_intra_pred_ref_filter
* gi1_table_edge_idx
* gu1_table_band_idx
* gu2_table_band_idx
*
* @remarks
*  None
*
*******************************************************************************
*/

#include "ihevc_typedefs.h"
#include "ihevc_platform_macros.h"
#include "ihevc_common_tables.h"

/*****************************************************************************/
/* Level specific tables                                                     */
/*****************************************************************************/

/**
 * Array giving size of max luma samples in a picture for a given level
 */
const WORD32 gai4_ihevc_max_luma_pic_size[] =
{
    /* Level 1 */
    36864,
    /* Level 2 */
    122880,
    /* Level 2.1 */
    245760,
    /* Level 3 */
    552960,
    /* Level 3.1 */
    983040,
    /* Level 4 */
    2228224,
    /* Level 4.1 */
    2228224,
    /* Level 5 */
    8912896,
    /* Level 5.1 */
    8912896,
    /* Level 5.2 */
    8912896,
    /* Level 6 */
    33423360,
    /* Level 6.1 */
    33423360,
    /* Level 6.2 */
    33423360
};
/** Max width and height allowed for a given level */
/** This is derived as SQRT(8 * gai4_ihevc_max_luma_pic_size[]) */
const WORD32 gai4_ihevc_max_wd_ht[] =
{
    /* Level 1 */
    543,
    /* Level 2 */
    991,
    /* Level 2.1 */
    1402,
    /* Level 3 */
    2103,
    /* Level 3.1 */
    2804,
    /* Level 4 */
    4222,
    /* Level 4.1 */
    4222,
    /* Level 5 */
    8444,
    /* Level 5.1 */
    8444,
    /* Level 5.2 */
    8444,
    /* Level 6 */
    16888,
    /* Level 6.1 */
    16888,
    /* Level 6.2 */
    16888
};

/** Min width and height allowed for a given level */
/** This is derived as gai4_ihevc_max_luma_pic_size[]/gai4_ihevc_max_wd_ht[] */
const WORD32 gai4_ihevc_min_wd_ht[] =
{
    /* Level 1 */
    67,
    /* Level 2 */
    123,
    /* Level 2.1 */
    175,
    /* Level 3 */
    262,
    /* Level 3.1 */
    350,
    /* Level 4 */
    527,
    /* Level 4.1 */
    527,
    /* Level 5 */
    1055,
    /* Level 5.1 */
    1055,
    /* Level 5.2 */
    1055,
    /* Level 6 */
    2111,
    /* Level 6.1 */
    2111,
    /* Level 6.2 */
    2111
};
/*****************************************************************************/
/* Intra prediction tables                                                   */
/*****************************************************************************/
/**
 * Intra pred angles
 */
/* g_ang_table = tan(actual angle) in Q5 format for all 33 modes */
const WORD32 gai4_ihevc_ang_table[35] =
    { 0, 0, 32, 26, 21, 17, 13, 9, 5, 2, 0, -2, -5, -9, -13, -17, -21, -26, -32,
                    -26, -21, -17, -13, -9, -5, -2, 0, 2, 5, 9, 13, 17, 21, 26, 32 };

const WORD8 col_for_intra_luma[32] =
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
                    29, 30, 31, 32 };

const WORD8 col_for_intra_chroma[32] =
    { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16 };

const WORD8 idx_neg_vals_3_9[7] =
    { 26, 21, 17, 13, 9, 5, 2 };

const WORD32 idx_neg_idx_3_9[28] =
    { 6, 13, 19, 26, 5, 10, 15, 21, 4, 8, 12, 17, 3, 6, 9, 13, 2, 4, 6, 9,
                    1, 2, 3, 5, 0, 0, 1, 2 };


const WORD32 idx_neg_idx_chroma_3_9[28] =
  { 3, 6, 9, 13,
    2, 5, 7, 10,
    2, 4, 6, 8,
    1, 3, 4, 6,
    1, 2, 3, 4,
    0, 1, 1, 2,
    0, 0, 0, 1 };
const WORD32 idx_neg_idx_11_17[28] =
    { -1, -1, -2, -2, -2, -3, -4, -5, -3, -5, -7, -9, -4, -7, -10, -13, -5, -9, -13, -17, -6, -11,
                    -16, -21, -7, -13, -20, -26 };

const WORD32 idx_neg_idx_chroma_11_17[28] =
  { -1, -1, -1, -1,
    -1, -2, -2, -3,
    -2, -3, -4, -5,
    -2, -4, -5, -7,
    -3, -5, -7, -9,
    -3, -6, -8, -11,
    -4, -7, -10, -13 };

/**
 * Intra pred inverse angles
 */
/* g_invAngTable = Inverse angle in Q5 format, required for negative angles */
const WORD32 gai4_ihevc_inv_ang_table[14] =
    { 4096, 1638, 910, 630, 482, 390, 315, 315, 390, 482, 630, 910, 1638, 4096 };

/*****************************************************************************/
/* Scan matrices                                                             */
/*****************************************************************************/
/**
 * Inverse Scan matrix for 8x8 Section 6.5.3
 */
const UWORD8  gau1_ihevc_invscan8x8[][64] =
{
    /* Upright diagonal */
    {
        0,  8,  1,  16, 9,  2,  24, 17,
        10, 3,  32, 25, 18, 11, 4,  40,
        33, 26, 19, 12, 5,  48, 41, 34,
        27, 20, 13, 6,  56, 49, 42, 35,
        28, 21, 14, 7,  57, 50, 43, 36,
        29, 22, 15, 58, 51, 44, 37, 30,
        23, 59, 52, 45, 38, 31, 60, 53,
        46, 39, 61, 54, 47, 62, 55, 63
    },
    /* Horizontal */
    {
        0,  1,  2,  3,  4,  5,  6,  7,
        8,  9,  10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 62, 63
    },
    /* Vertical */
    {
        0,  8,  16, 24, 32, 40, 48, 56,
        1,  9,  17, 25, 33, 41, 49, 57,
        2,  10, 18, 26, 34, 42, 50, 58,
        3,  11, 19, 27, 35, 43, 51, 59,
        4,  12, 20, 28, 36, 44, 52, 60,
        5,  13, 21, 29, 37, 45, 53, 61,
        6,  14, 22, 30, 38, 46, 54, 62,
        7,  15, 23, 31, 39, 47, 55, 63
    }
};

/**
 * Inverse Scan matrix for 4x4 Section 6.5.3
 */
const UWORD8  gau1_ihevc_invscan4x4[][16] =
{
    /* Upright diagonal */
    {
        0, 4,  1,  8,
        5, 2,  12, 9,
        6, 3,  13, 10,
        7, 14, 11, 15
    },
    /* Horizontal */
    {
        0,  1,  2,  3,
        4,  5,  6,  7,
        8,  9,  10, 11,
        12, 13, 14, 15
    },
    /* Vertical */
    {
        0,  4,  8,  12,
        1,  5,  9,  13,
        2,  6,  10, 14,
        3,  7,  11, 15
    }
};

/**
 * Inverse Scan matrix for 4x4 Section 6.5.3
 */
const UWORD8  gau1_ihevc_invscan2x2[][4] =
{
    /* Upright diagonal */
    {
        0,  2,
        1,  3
    },
    /* Horizontal */
    {
        0,  1,
        2,  3
    },
    /* Vertical */
    {
        0,  2,
        1,  3,
    }
};

/**
 * Scan matrix for 8x8 Section 6.5.3
 */

const UWORD8  gau1_ihevc_scan8x8[][64] =
{
    /* Upright diagonal */
    {
        0,  2,  5,  9,  14, 20, 27, 35,
        1,  4,  8,  13, 19, 26, 34, 42,
        3,  7,  12, 18, 25, 33, 41, 48,
        6,  11, 17, 24, 32, 40, 47, 53,
        10, 16, 23, 31, 39, 46, 52, 57,
        15, 22, 30, 38, 45, 51, 56, 60,
        21, 29, 37, 44, 50, 55, 59, 62,
        28, 36, 43, 49, 54, 58, 61, 63
    },
    /* Horizontal */
    {
        0,  1,  2,  3,  4,  5,  6,  7,
        8,  9,  10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39,
        40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55,
        56, 57, 58, 59, 60, 61, 62, 63
    },
    /* Vertical */
    {
        0,  8,  16, 24, 32, 40, 48, 56,
        1,  9,  17, 25, 33, 41, 49, 57,
        2,  10, 18, 26, 34, 42, 50, 58,
        3,  11, 19, 27, 35, 43, 51, 59,
        4,  12, 20, 28, 36, 44, 52, 60,
        5,  13, 21, 29, 37, 45, 53, 61,
        6,  14, 22, 30, 38, 46, 54, 62,
        7,  15, 23, 31, 39, 47, 55, 63
    }
};

/**
 * Scan matrix for 4x4 Section 6.5.3
 */
const UWORD8  gau1_ihevc_scan4x4[][16] =
{
    /* Upright diagonal */
    {
        0,  2,  5,  9,
        1,  4,  8,  12,
        3,  7,  11, 14,
        6,  10, 13, 15
    },
    /* Horizontal */
    {
        0,  1,  2,  3,
        4,  5,  6,  7,
        8,  9,  10, 11,
        12, 13, 14, 15
    },
    /* Vertical */
    {
        0,  4,  8,  12,
        1,  5,  9,  13,
        2,  6,  10, 14,
        3,  7,  11, 15
    }
};

/**
 * Scan matrix for 4x4 Section 6.5.3
 */
const UWORD8  gau1_ihevc_scan2x2[][4] =
{
    /* Upright diagonal */
    {
        0,  2,
        1,  3
    },
    /* Horizontal */
    {
        0,  1,
        2,  3
    },
    /* Vertical */
    {
        0,  2,
        1,  3,
    }
};

/**
 * Table containing all the scan matrices
 */
const void *gapv_ihevc_scan[] =
{
    gau1_ihevc_scan2x2[0],
    gau1_ihevc_scan4x4[0],
    gau1_ihevc_scan8x8[0],

    gau1_ihevc_scan2x2[1],
    gau1_ihevc_scan4x4[1],
    gau1_ihevc_scan8x8[1],

    gau1_ihevc_scan2x2[2],
    gau1_ihevc_scan4x4[2],
    gau1_ihevc_scan8x8[2],

};

const void *gapv_ihevc_invscan[] =
{
    gau1_ihevc_invscan2x2[0],
    gau1_ihevc_invscan4x4[0],
    gau1_ihevc_invscan8x8[0],

    gau1_ihevc_invscan2x2[1],
    gau1_ihevc_invscan4x4[1],
    gau1_ihevc_invscan8x8[1],

    gau1_ihevc_invscan2x2[2],
    gau1_ihevc_invscan4x4[2],
    gau1_ihevc_invscan8x8[2],
};
/**
 * Table for luma to chroma qp conversion
 */

// FOR MAIN branch encoder ( 8 bit)
const UWORD8 gau1_ihevc_chroma_qp_scale[58] =
{

     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 29, 30, 31, 32,
    33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51
};

// FOR HBD branch encoder ( 8, 10 and 12 bit)
const WORD8 gai1_ihevc_chroma_qp_scale[82]=  //EXTENDED for 12 bit
{
  -24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,
  -12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,
  17,18,19,20,21,22,23,24,25,26,27,28,29,29,30,31,32,
  33,33,34,34,35,35,36,36,37,37,38,39,40,41,42,43,44,
  45,46,47,48,49,50,51
};


/** constant planar factor values table */
const UWORD8 gau1_ihevc_planar_factor[65] = {    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 62, 63, 64 };
//AX CHANGES
const UWORD8 gau1_ihevc_planar_factor_1[32] = {    1, 1, 1, 1, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9,
    17, 17, 17, 17, 17, 17, 17, 17, 25, 25, 25, 25, 25, 25,
    25, 25 };
//AX CHANGES

/** g_ang_table = tan(actual angle) in Q5 format for all 33 modes */
const WORD32 gai4_ihevc_ang_table_chroma[35] = { 0, 0, 32, 26, 21, 17, 13, 9, 5, 2, 0, -2,  -5, -9,
    -13, -17, -21, -26, -32, -26, -21, -17, -13, -9, -5,
    -2, 0, 2, 5, 9, 13, 17, 21, 26, 32 };
/** g_invAngTable = Inverse angle in Q5 format, required for negative angles */
const WORD32 gai4_ihevc_inv_ang_table_chroma[14] = { 4096, 1638, 910, 630, 482, 390, 315,
    315, 390, 482, 630, 910, 1638, 4096 };


/** constant planar factor values table */
const UWORD8 gau1_ihevc_planar_factor_chroma[33] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32  };



/** Filter flag values for intra pred referenece filtering - intra pred mode is the index
*   flag for nt = 4 is the Bit 0, nt = 8 is Bit 1, nt = 16 is Bit 2, nt = 32 is Bit 3
*/
const UWORD8 gau1_intra_pred_ref_filter[] =
{
    14,  0, 14, 12, 12, 12, 12,
    12, 12,  8,  0,  8, 12, 12,
    12, 12, 12, 12, 14, 12, 12,
    12, 12, 12, 12,  8,  0,  8,
    12, 12, 12, 12, 12, 12, 14
};


const WORD8 gi1_table_edge_idx[8] = { 1, 2, 0, 3, 4, 0, 0, 0 }; /* First 5 values are valid. Last 3 dummy values are added to help SIMD load*/

const UWORD8 gu1_table_band_idx[32] = {  0,  1,  2,  3,  4,  5,  6,  7,
                                         8,  9, 10, 11, 12, 13, 14, 15,
                                        16, 17, 18, 19, 20, 21, 22, 23,
                                        24, 25, 26, 27, 28, 29, 30, 31
};

const UWORD16 gu2_table_band_idx[32] = {  0,  1,  2,  3,  4,  5,  6,  7,
                                         8,  9, 10, 11, 12, 13, 14, 15,
                                        16, 17, 18, 19, 20, 21, 22, 23,
                                        24, 25, 26, 27, 28, 29, 30, 31
};

#ifdef ENABLE_SSE4_1_INTR
/*Used as a lookup table to have popcnt instruction working for SSE4.1 platform.
Each unit indicates number of 1s the index at which it is persent
*/
const WORD8   gi1_popcnt_byte_table[] =
{
    0,  1,  1,  2,  1,  2,  2,  3,  1,  2,  2,  3,  2,  3,  3,  4,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    1,  2,  2,  3,  2,  3,  3,  4,  2,  3,  3,  4,  3,  4,  4,  5,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    2,  3,  3,  4,  3,  4,  4,  5,  3,  4,  4,  5,  4,  5,  5,  6,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    3,  4,  4,  5,  4,  5,  5,  6,  4,  5,  5,  6,  5,  6,  6,  7,
    4,  5,  5,  6,  5,  6,  6,  7,  5,  6,  6,  7,  6,  7,  7,  8
};
#endif
