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
 *  ihevcd_function_selector.h
 *
 * @brief
 *  Structure definitions used in the decoder
 *
 * @author
 *  Harish
 *
 * @par List of Functions:
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

#ifndef _IHEVCD_FUNCTION_SELECTOR_H_
#define _IHEVCD_FUNCTION_SELECTOR_H_

#include "ihevc_deblk.h"
#include "ihevc_itrans.h"
#include "ihevc_itrans_recon.h"
#include "ihevc_chroma_itrans_recon.h"
#include "ihevc_chroma_intra_pred.h"
#include "ihevc_recon.h"
#include "ihevc_chroma_recon.h"
#include "ihevc_intra_pred.h"
#include "ihevc_inter_pred.h"
#include "ihevc_mem_fns.h"
#include "ihevc_padding.h"
#include "ihevc_weighted_pred.h"
#include "ihevc_sao.h"
#include "ihevcd_fmt_conv.h"
#include "ihevcd_itrans_recon_dc.h"

#define D_ARCH_NA                   1
#define D_ARCH_ARM_NONEON           2
#define D_ARCH_ARM_A9Q              3
#define D_ARCH_ARM_A9A              4
#define D_ARCH_ARM_A9               5
#define D_ARCH_ARM_A7               6
#define D_ARCH_ARM_A5               7
#define D_ARCH_ARM_A15              8
#define D_ARCH_ARM_NEONINTR         9
#define D_ARCH_ARMV8_GENERIC        10
#define D_ARCH_X86_GENERIC          11
#define D_ARCH_X86_SSSE3            12
#define D_ARCH_X86_SSE42            13
#define D_ARCH_X86_AVX2             14
#define D_ARCH_MIPS_GENERIC         15
#define D_ARCH_MIPS_32              16

void ihevcd_init_arch(void *pv_codec);

void ihevcd_init_function_ptr(void *pv_codec);

void ihevcd_init_function_ptr_generic(void *pv_codec);
void ihevcd_init_function_ptr_ssse3(void *pv_codec);
void ihevcd_init_function_ptr_sse42(void *pv_codec);

#ifndef DISABLE_AVX2
void ihevcd_init_function_ptr_avx2(void *pv_codec);
#endif

typedef struct
{
    ihevc_deblk_chroma_horz_ft *ihevc_deblk_chroma_horz_fptr;
    ihevc_deblk_chroma_vert_ft *ihevc_deblk_chroma_vert_fptr;
    ihevc_deblk_luma_vert_ft *ihevc_deblk_luma_vert_fptr;
    ihevc_deblk_luma_horz_ft *ihevc_deblk_luma_horz_fptr;

    ihevc_inter_pred_ft *ihevc_inter_pred_chroma_copy_fptr;
    ihevc_inter_pred_w16out_ft *ihevc_inter_pred_chroma_copy_w16out_fptr;
    ihevc_inter_pred_ft *ihevc_inter_pred_chroma_horz_fptr;
    ihevc_inter_pred_w16out_ft *ihevc_inter_pred_chroma_horz_w16out_fptr;
    ihevc_inter_pred_ft *ihevc_inter_pred_chroma_vert_fptr;
    ihevc_inter_pred_w16inp_ft *ihevc_inter_pred_chroma_vert_w16inp_fptr;
    ihevc_inter_pred_w16inp_w16out_ft *ihevc_inter_pred_chroma_vert_w16inp_w16out_fptr;
    ihevc_inter_pred_w16out_ft *ihevc_inter_pred_chroma_vert_w16out_fptr;
    ihevc_inter_pred_ft *ihevc_inter_pred_luma_horz_fptr;
    ihevc_inter_pred_ft *ihevc_inter_pred_luma_vert_fptr;
    ihevc_inter_pred_w16out_ft *ihevc_inter_pred_luma_vert_w16out_fptr;
    ihevc_inter_pred_w16inp_ft *ihevc_inter_pred_luma_vert_w16inp_fptr;
    ihevc_inter_pred_ft *ihevc_inter_pred_luma_copy_fptr;
    ihevc_inter_pred_w16out_ft *ihevc_inter_pred_luma_copy_w16out_fptr;
    ihevc_inter_pred_w16out_ft *ihevc_inter_pred_luma_horz_w16out_fptr;
    ihevc_inter_pred_w16inp_w16out_ft *ihevc_inter_pred_luma_vert_w16inp_w16out_fptr;

    ihevc_intra_pred_chroma_ref_substitution_ft *ihevc_intra_pred_chroma_ref_substitution_fptr;
    ihevc_intra_pred_luma_ref_substitution_ft *ihevc_intra_pred_luma_ref_substitution_fptr;
    ihevc_intra_pred_luma_ref_subst_all_avlble_ft *ihevc_intra_pred_luma_ref_subst_all_avlble_fptr;
    ihevc_intra_pred_ref_filtering_ft *ihevc_intra_pred_ref_filtering_fptr;
    ihevc_intra_pred_chroma_dc_ft *ihevc_intra_pred_chroma_dc_fptr;
    ihevc_intra_pred_chroma_horz_ft *ihevc_intra_pred_chroma_horz_fptr;
    ihevc_intra_pred_chroma_mode2_ft *ihevc_intra_pred_chroma_mode2_fptr;
    ihevc_intra_pred_chroma_mode_18_34_ft *ihevc_intra_pred_chroma_mode_18_34_fptr;
    ihevc_intra_pred_chroma_mode_27_to_33_ft *ihevc_intra_pred_chroma_mode_27_to_33_fptr;
    ihevc_intra_pred_chroma_mode_3_to_9_ft *ihevc_intra_pred_chroma_mode_3_to_9_fptr;
    ihevc_intra_pred_chroma_planar_ft *ihevc_intra_pred_chroma_planar_fptr;
    ihevc_intra_pred_chroma_ver_ft *ihevc_intra_pred_chroma_ver_fptr;
    ihevc_intra_pred_chroma_mode_11_to_17_ft *ihevc_intra_pred_chroma_mode_11_to_17_fptr;
    ihevc_intra_pred_chroma_mode_19_to_25_ft *ihevc_intra_pred_chroma_mode_19_to_25_fptr;
    ihevc_intra_pred_luma_mode_11_to_17_ft *ihevc_intra_pred_luma_mode_11_to_17_fptr;
    ihevc_intra_pred_luma_mode_19_to_25_ft *ihevc_intra_pred_luma_mode_19_to_25_fptr;
    ihevc_intra_pred_luma_dc_ft *ihevc_intra_pred_luma_dc_fptr;
    ihevc_intra_pred_luma_horz_ft *ihevc_intra_pred_luma_horz_fptr;
    ihevc_intra_pred_luma_mode2_ft *ihevc_intra_pred_luma_mode2_fptr;
    ihevc_intra_pred_luma_mode_18_34_ft *ihevc_intra_pred_luma_mode_18_34_fptr;
    ihevc_intra_pred_luma_mode_27_to_33_ft *ihevc_intra_pred_luma_mode_27_to_33_fptr;
    ihevc_intra_pred_luma_mode_3_to_9_ft *ihevc_intra_pred_luma_mode_3_to_9_fptr;
    ihevc_intra_pred_luma_planar_ft *ihevc_intra_pred_luma_planar_fptr;
    ihevc_intra_pred_luma_ver_ft *ihevc_intra_pred_luma_ver_fptr;
    ihevc_itrans_4x4_ttype1_ft *ihevc_itrans_4x4_ttype1_fptr;
    ihevc_itrans_4x4_ft *ihevc_itrans_4x4_fptr;
    ihevc_itrans_8x8_ft *ihevc_itrans_8x8_fptr;
    ihevc_itrans_16x16_ft *ihevc_itrans_16x16_fptr;
    ihevc_itrans_32x32_ft *ihevc_itrans_32x32_fptr;
    ihevc_itrans_recon_4x4_ttype1_ft *ihevc_itrans_recon_4x4_ttype1_fptr;
    ihevc_itrans_recon_4x4_ft *ihevc_itrans_recon_4x4_fptr;
    ihevc_itrans_recon_8x8_ft *ihevc_itrans_recon_8x8_fptr;
    ihevc_itrans_recon_16x16_ft *ihevc_itrans_recon_16x16_fptr;
    ihevc_itrans_recon_32x32_ft *ihevc_itrans_recon_32x32_fptr;
    ihevc_chroma_itrans_recon_4x4_ft *ihevc_chroma_itrans_recon_4x4_fptr;
    ihevc_chroma_itrans_recon_8x8_ft *ihevc_chroma_itrans_recon_8x8_fptr;
    ihevc_chroma_itrans_recon_16x16_ft *ihevc_chroma_itrans_recon_16x16_fptr;
    ihevc_recon_4x4_ttype1_ft *ihevc_recon_4x4_ttype1_fptr;
    ihevc_recon_4x4_ft *ihevc_recon_4x4_fptr;
    ihevc_recon_8x8_ft *ihevc_recon_8x8_fptr;
    ihevc_recon_16x16_ft *ihevc_recon_16x16_fptr;
    ihevc_recon_32x32_ft *ihevc_recon_32x32_fptr;
    ihevc_chroma_recon_4x4_ft *ihevc_chroma_recon_4x4_fptr;
    ihevc_chroma_recon_8x8_ft *ihevc_chroma_recon_8x8_fptr;
    ihevc_chroma_recon_16x16_ft *ihevc_chroma_recon_16x16_fptr;
    ihevc_memcpy_mul_8_ft *ihevc_memcpy_mul_8_fptr;
    ihevc_memcpy_ft *ihevc_memcpy_fptr;
    ihevc_memset_mul_8_ft *ihevc_memset_mul_8_fptr;
    ihevc_memset_ft *ihevc_memset_fptr;
    ihevc_memset_16bit_mul_8_ft *ihevc_memset_16bit_mul_8_fptr;
    ihevc_memset_16bit_ft *ihevc_memset_16bit_fptr;
    ihevc_pad_left_luma_ft *ihevc_pad_left_luma_fptr;
    ihevc_pad_left_chroma_ft *ihevc_pad_left_chroma_fptr;
    ihevc_pad_right_luma_ft *ihevc_pad_right_luma_fptr;
    ihevc_pad_right_chroma_ft *ihevc_pad_right_chroma_fptr;
    ihevc_weighted_pred_bi_ft *ihevc_weighted_pred_bi_fptr;
    ihevc_weighted_pred_bi_default_ft *ihevc_weighted_pred_bi_default_fptr;
    ihevc_weighted_pred_uni_ft *ihevc_weighted_pred_uni_fptr;
    ihevc_weighted_pred_chroma_bi_ft *ihevc_weighted_pred_chroma_bi_fptr;
    ihevc_weighted_pred_chroma_bi_default_ft *ihevc_weighted_pred_chroma_bi_default_fptr;
    ihevc_weighted_pred_chroma_uni_ft *ihevc_weighted_pred_chroma_uni_fptr;
    ihevc_sao_band_offset_luma_ft *ihevc_sao_band_offset_luma_fptr;
    ihevc_sao_band_offset_chroma_ft *ihevc_sao_band_offset_chroma_fptr;
    ihevc_sao_edge_offset_class0_ft *ihevc_sao_edge_offset_class0_fptr;
    ihevc_sao_edge_offset_class0_chroma_ft *ihevc_sao_edge_offset_class0_chroma_fptr;
    ihevc_sao_edge_offset_class1_ft *ihevc_sao_edge_offset_class1_fptr;
    ihevc_sao_edge_offset_class1_chroma_ft *ihevc_sao_edge_offset_class1_chroma_fptr;
    ihevc_sao_edge_offset_class2_ft *ihevc_sao_edge_offset_class2_fptr;
    ihevc_sao_edge_offset_class2_chroma_ft *ihevc_sao_edge_offset_class2_chroma_fptr;
    ihevc_sao_edge_offset_class3_ft *ihevc_sao_edge_offset_class3_fptr;
    ihevc_sao_edge_offset_class3_chroma_ft *ihevc_sao_edge_offset_class3_chroma_fptr;
    ihevcd_fmt_conv_420sp_to_rgba8888_ft *ihevcd_fmt_conv_420sp_to_rgba8888_fptr;
    ihevcd_fmt_conv_420sp_to_rgb565_ft *ihevcd_fmt_conv_420sp_to_rgb565_fptr;
    ihevcd_fmt_conv_420sp_to_420sp_ft *ihevcd_fmt_conv_420sp_to_420sp_fptr;
    ihevcd_fmt_conv_420sp_to_420p_ft *ihevcd_fmt_conv_420sp_to_420p_fptr;
    ihevcd_itrans_recon_dc_luma_ft *ihevcd_itrans_recon_dc_luma_fptr;
    ihevcd_itrans_recon_dc_chroma_ft *ihevcd_itrans_recon_dc_chroma_fptr;
}func_selector_t;

#endif /* _IHEVCD_FUNCTION_SELECTOR_H_ */
