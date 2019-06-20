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
*  ihevcd_statistics.h
*
* @brief
*  Contains macros for generating stats about hevc decoder
*
* @author
*  Naveen SR
*
* @par List of Functions:
*
* @remarks
*  None
*
*******************************************************************************
*/

#ifndef _IHEVCD_STATISTICS_H_
#define _IHEVCD_STATISTICS_H_

#include <stdio.h>
#include "ihevc_typedefs.h"
#include "iv.h"
#include "ivd.h"
#include "ihevcd_cxa.h"
#include "ithread.h"

#include "ihevc_defs.h"
#include "ihevc_structs.h"

#include "ihevc_cabac_tables.h"
#include "ihevcd_defs.h"

#include "ihevcd_structs.h"
#include "ihevcd_iquant_itrans_recon_ctb.h"
#include "ihevcd_statistics.h"

#define STATISTICS_ENABLE 0

#if STATISTICS_ENABLE

typedef struct
{
    UWORD32 num_4x4_dst;
    UWORD32 num_4x4;
    UWORD32 num_8x8;
    UWORD32 num_16x16;
    UWORD32 num_32x32;
    UWORD32 num_64x64;
}stat_trans_t;

typedef struct
{
    /* 4x4 Subblock count */
    UWORD32 trans_4x4_dst[1];
    UWORD32 trans_4x4[1];
    UWORD32 trans_8x8[4];
    UWORD32 trans_16x16[16];
    UWORD32 trans_32x32[64];
}stat_sblk_histogram_t;

typedef struct
{
    /* 4x4 Subblock count */
    UWORD32 trans_4x4_dst[16];
    UWORD32 trans_4x4[16];
    UWORD32 trans_8x8[64];
    UWORD32 trans_16x16[256];
    UWORD32 trans_32x32[1024];
}stat_coeff_histogram_t;

typedef struct
{
    stat_trans_t stat_num_all_trans_block[2]; /* Y and UV */
    stat_trans_t stat_num_coded_trans_block[2]; /* Y and UV */
    stat_trans_t stat_num_coded_dc_block[2]; /* Y and UV */
    stat_trans_t stat_num_coded_one_coeff_block[2]; /* Y and UV */
    stat_sblk_histogram_t stat_last_sblk_pos_histogram; /* Y + UV */
    stat_sblk_histogram_t stat_num_coded_sblk_histogram; /* Y + UV */
    stat_coeff_histogram_t stat_num_coded_coeff_histogram; /* Y + UV */
    UWORD32   stat_pu_all_size_hist[16][16]; /* PU Sizes [Width from 4 to 64 in steps of 4] [Height from 4 to 64 in steps of 4]*/
    UWORD32   stat_pu_skip_size_hist[16][16]; /* PU sizes for skip [Width from 4 to 64 in steps of 4] [Height from 4 to 64 in steps of 4]*/
    UWORD32   stat_pu_inter_size_hist[16][16]; /* PU sizes for inter [Width from 4 to 64 in steps of 4] [Height from 4 to 64 in steps of 4]*/
    UWORD32   stat_pu_intra_size_hist[16][16]; /* PU sizes for intra [Width from 4 to 64 in steps of 4] [Height from 4 to 64 in steps of 4]*/
    UWORD32   stat_pu_bipred_size_hist[16][16]; /* PU sizes for bipred [Width from 4 to 64 in steps of 4] [Height from 4 to 64 in steps of 4]*/
    UWORD32   stat_pu_merge_size_hist[16][16]; /* PU sizes for merge [Width from 4 to 64 in steps of 4] [Height from 4 to 64 in steps of 4]*/
    UWORD32   stat_pu_zeromv_size_hist[16][16]; /* PU sizes for Zero MV [Width from 4 to 64 in steps of 4] [Height from 4 to 64 in steps of 4]*/
    UWORD32   stat_pu_zeromvfpel_size_hist[16][16]; /* PU sizes for Zero MV (includes subpel less than +/- 1 full pel units [Width from 4 to 64 in steps of 4] [Height from 4 to 64 in steps of 4]*/
    UWORD32 last_sblk_pos_x; /* Last sblk pos of transform block in processing */
    UWORD32 last_sblk_pos_y;
    UWORD32 num_coded_sblk;
    UWORD32 num_coded_coeffs;
}statistics_t;

void ihevcd_update_num_all_trans_blocks(TRANSFORM_TYPE e_trans_type, WORD32 c_idx);
void ihevcd_update_num_trans_blocks(TRANSFORM_TYPE e_trans_type, WORD32 c_idx, WORD32 update_type);
void ihevcd_update_sblk_and_coeff_histogram(TRANSFORM_TYPE e_trans_type, WORD32 t_skip_or_tq_bypass);
void ihevcd_sblk_pos_init();
void ihevcd_sblk_pos_update(TRANSFORM_TYPE e_trans_type, WORD32 t_skip_or_tq_bypass, UWORD32 sblk_x, UWORD32 sblk_y);
void ihevcd_print_transform_statistics();
void ihevcd_update_coeff_count();
void ihevcd_update_pu_size(pu_t *ps_pu);
void ihevcd_update_pu_skip_size(pu_t *ps_pu);
#endif //STATISTICS_ENABLE

#if STATISTICS_ENABLE
#define STATS_INIT()  ihevcd_init_statistics();
#define STATS_UPDATE_ALL_TRANS(e_trans_type, c_idx) ihevcd_update_num_all_trans_blocks(e_trans_type, c_idx);
#define STATS_UPDATE_CODED_TRANS(e_trans_type, c_idx, update_type) ihevcd_update_num_trans_blocks(e_trans_type, c_idx, update_type);
#define STATS_PRINT() ihevcd_print_statistics();
#define STATS_INIT_SBLK_AND_COEFF_POS() ihevcd_sblk_pos_init();
#define STATS_LAST_SBLK_POS_UPDATE(e_trans_type, t_skip_or_tq_bypass, sblk_x, sblk_y) ihevcd_sblk_pos_update(e_trans_type, t_skip_or_tq_bypass, sblk_x, sblk_y);
#define STATS_UPDATE_SBLK_AND_COEFF_HISTOGRAM(e_trans_type, t_skip_or_tq_bypass) ihevcd_update_sblk_and_coeff_histogram(e_trans_type, t_skip_or_tq_bypass);
#define STATS_UPDATE_COEFF_COUNT() ihevcd_update_coeff_count();
#define STATS_UPDATE_PU_SIZE(ps_pu) ihevcd_update_pu_size(ps_pu);
#define STATS_UPDATE_PU_SKIP_SIZE(ps_pu) ihevcd_update_pu_skip_size(ps_pu);
#else
#define STATS_INIT()  ;
#define STATS_UPDATE_ALL_TRANS(e_trans_type, c_idx) ;
#define STATS_UPDATE_CODED_TRANS(e_trans_type, c_idx, update_type) ;
#define STATS_PRINT() ;
#define STATS_INIT_SBLK_AND_COEFF_POS() ;
#define STATS_LAST_SBLK_POS_UPDATE(e_trans_type, t_skip_or_tq_bypass, sblk_x, sblk_y) ;
#define STATS_UPDATE_SBLK_AND_COEFF_HISTOGRAM(e_trans_type, t_skip_or_tq_bypass) ;
#define STATS_UPDATE_COEFF_COUNT() ;
#define STATS_UPDATE_PU_SIZE(ps_pu) ;
#define STATS_UPDATE_PU_SKIP_SIZE(ps_pu) ;
#endif

#endif /* _IHEVCD_STATISTICS_H_ */
