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
 *  ihevcd_common_tables.c
 *
 * @brief
 *  Contains common global tables for decoder
 *
 * @author
 *  Naveen S R
 *
 * @par List of Functions:
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

#include "ihevc_typedefs.h"
#include "ihevcd_common_tables.h"
#include "ihevc_defs.h"

const WORD16 gai2_ihevcd_chroma_qp[] =
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 29,
    30, 31, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

const UWORD8 gau1_intra_pred_chroma_modes[] =
    { INTRA_PLANAR, INTRA_ANGULAR(26), INTRA_ANGULAR(10), INTRA_DC };

