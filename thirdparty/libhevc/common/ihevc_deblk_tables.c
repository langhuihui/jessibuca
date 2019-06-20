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
*  ihevc_deblk_tables.c
*
* @brief
*  Contains tables used for deblock filters
*
* @author
*  Srinivas T
*
* @par List of Tables:
* gai4_ihevc_beta_table
* gai4_ihevc_tc_table
* gai4_ihevc_qp_table
*
* @remarks
*  None
*
*******************************************************************************
*/
#include "ihevc_typedefs.h"
#include "ihevc_deblk_tables.h"

/**
 * Beta table for deblocking
 * Table 8-10 - Derivation of threshold variables beta and tc from input Q
 */
const WORD32 gai4_ihevc_beta_table[52] =
{    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38,
    40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64
};


/**
 * Tc table for deblocking
 * Table 8-10 - Derivation of threshold variables beta and tc from input Q
 */
const WORD32 gai4_ihevc_tc_table[54] =
{
     0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,
     1,  1,  1,  1,  1,  1,  1,  1,  1,
     2,  2,  2,  2,  3,  3,  3,  3,  4,
     4,  4,  5,  5,  6,  6,  7,  8,  9,
    10, 11, 13, 14, 16, 18, 20, 22, 24
};

/**
 * QP table for deblocking
 * Table 8-9  Specification of QPC as a function of qPi
 */
const WORD32 gai4_ihevc_qp_table[58] =
{
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 29, 30, 31, 32,
    33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51
};
