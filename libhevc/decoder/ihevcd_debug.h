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
*  ihevcd_debug.h
*
* @brief
*  Debug defs
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

#ifndef _IHEVCD_DEBUG_H_
#define _IHEVCD_DEBUG_H_

#define DEBUG_REF_LIST 0
#define DEBUG_PADDED_REGION 0
#define DEBUG_DUMP_PRE_ILF 0
#define DEBUG_PRINT_IQ_IT_RECON 0
#define DEBUG_PRINT_MV 0
#define DEBUG_DEBLK_LEAF_LEVEL 0
#define DEBUG_NAL_TYPE 0
#define DEBUG_SAO_TMP_BUF 0
#define DEBUG_BREAK_AFTER_SLICE_NAL 0
#define DEBUG_DUMP_FRAME_BUFFERS_INFO 0
#define DEBUG_DUMP_FRAME_PU_INFO 0
#define DEBUG_MV_MAP 0

#if (DEBUG_REF_LIST || DEBUG_PADDED_REGION || DEBUG_DUMP_PRE_ILF || DEBUG_PRINT_IQ_IT_RECON || DEBUG_PRINT_MV || DEBUG_DEBLK_LEAF_LEVEL || DEBUG_NAL_TYPE || DEBUG_SAO_TMP_BUF || DEBUG_BREAK_AFTER_SLICE_NAL || DEBUG_DUMP_FRAME_BUFFERS_INFO || DEBUG_DUMP_FRAME_PU_INFO)
#define DEBUG_CODE 1
#else
#define DEBUG_CODE 0
#endif

#if DEBUG_DUMP_FRAME_PU_INFO
#define DEBUG_DUMP_PIC_PU(ps_codec) ihevcd_debug_dump_pic_pu(ps_codec);
#else
#define DEBUG_DUMP_PIC_PU(ps_codec)
#endif

#if DEBUG_DUMP_FRAME_BUFFERS_INFO
UWORD8 au1_pic_avail_ctb_flags[MAX_WD * MAX_HT / MIN_CTB_SIZE / MIN_CTB_SIZE];
UWORD32 au4_pic_ctb_slice_xy[MAX_WD * MAX_HT / MIN_CTB_SIZE / MIN_CTB_SIZE];

#define DEBUG_DUMP_PIC_BUFFERS(ps_codec) ihevcd_debug_dump_pic_buffers(ps_codec);
#else
#define DEBUG_DUMP_PIC_BUFFERS(ps_codec)
#endif

#if DEBUG_BREAK_AFTER_SLICE_NAL
#define BREAK_AFTER_SLICE_NAL()   \
    if (ps_codec->i4_header_done) \
        break;
#else
#define BREAK_AFTER_SLICE_NAL() ;
#endif

#if DEBUG_SAO_TMP_BUF
#define DEBUG_INIT_TMP_BUF(pu1_buf_luma, pu1_buf_chroma) ihevcd_debug_init_tmp_buf(pu1_buf_luma, pu1_buf_chroma)
#define DEBUG_PROCESS_TMP_BUF(pu1_buf_luma, pu1_buf_chroma) ihevcd_debug_process_tmp_buf(pu1_buf_luma, pu1_buf_chroma)
#else
#define DEBUG_INIT_TMP_BUF(pu1_buf_luma, pu1_buf_chroma)
#define DEBUG_PROCESS_TMP_BUF(pu1_buf_luma, pu1_buf_chroma)
#endif

#if DEBUG_NAL_TYPE

#define DEBUG_PRINT_NAL_INFO(ps_codec, nal_type)     \
    ihevcd_debug_print_nal_info(ps_codec, nal_type); \
    break;
#define RETURN_IF_NAL_INFO return;

#else

#define DEBUG_PRINT_NAL_INFO(ps_codec, nal_type)
#define RETURN_IF_NAL_INFO

#endif

#if DEBUG_REF_LIST

#define DEBUG_PRINT_REF_LIST_POCS(i4_pic_order_cnt_val, ps_slice_hdr, ps_dpb_mgr, u4_num_st_curr_before, u4_num_st_curr_after, u4_num_st_foll, u4_num_lt_curr, u4_num_lt_foll, ai4_poc_st_curr_before, ai4_poc_st_curr_after, ai4_poc_st_foll, ai4_poc_lt_curr, ai4_poc_lt_foll) \
    ihevcd_debug_print_ref_list_pocs(i4_pic_order_cnt_val, ps_slice_hdr, ps_dpb_mgr, u4_num_st_curr_before, u4_num_st_curr_after, u4_num_st_foll, u4_num_lt_curr, u4_num_lt_foll, ai4_poc_st_curr_before, ai4_poc_st_curr_after, ai4_poc_st_foll, ai4_poc_lt_curr, ai4_poc_lt_foll);

#else

#define DEBUG_PRINT_REF_LIST_POCS(i4_pic_order_cnt_val, ps_slice_hdr, ps_dpb_mgr, u4_num_st_curr_before, u4_num_st_curr_after, u4_num_st_foll, u4_num_lt_curr, u4_num_lt_foll, ai4_poc_st_curr_before, ai4_poc_st_curr_after, ai4_poc_st_foll, ai4_poc_lt_curr, ai4_poc_lt_foll)

#endif

#if DEBUG_PADDED_REGION

#define DEBUG_VALIDATE_PADDED_REGION(ps_proc) ihevcd_debug_validate_padded_region(ps_proc);

#else

#define DEBUG_VALIDATE_PADDED_REGION(ps_proc)

#endif

#if DEBUG_DUMP_PRE_ILF

#define DUMP_PRE_ILF(pu1_cur_pic_luma, pu1_cur_pic_chroma, pic_wd, pic_ht, pic_strd) ihevcd_debug_dump_pic(pu1_cur_pic_luma, pu1_cur_pic_chroma, pic_wd, pic_ht, pic_strd)
#define DUMP_BS(pu4_pic_vert_bs, pu4_pic_horz_bs, vert_size_in_bytes, horz_size_in_bytes) ihevcd_debug_dump_bs(pu4_pic_vert_bs, pu4_pic_horz_bs, vert_size_in_bytes, horz_size_in_bytes)
#define DUMP_QP(pu1_qp, size_in_bytes) ihevcd_debug_dump_qp(pu1_qp, size_in_bytes)
#define DUMP_QP_CONST_IN_CTB(pu1_qp_const_in_ctb, size_in_bytes) ihevcs_dump_qp_const_in_ctb(pu1_qp_const_in_ctb, size_in_bytes)
#define DUMP_NO_LOOP_FILTER(pu1_pic_no_loop_filter, size_in_bytes) ihevcd_debug_dump_no_loop_filter(pu1_pic_no_loop_filter, size_in_bytes)
#define DUMP_OFFSETS(beta_offset_div_2, tc_offset_div_2, qp_offset_u, qp_offset_v) ihevcd_debug_dump_offsets(beta_offset_div_2, tc_offset_div_2, qp_offset_u, qp_offset_v)

#else

#define DUMP_PRE_ILF(pu1_cur_pic_luma, pu1_cur_pic_chroma, pic_wd, pic_ht, pic_strd)
#define DUMP_BS(pu4_pic_vert_bs, pu4_pic_horz_bs, vert_size_in_bytes, horz_size_in_bytes)
#define DUMP_QP(pu1_qp, size_in_bytes)
#define DUMP_QP_CONST_IN_CTB(pu1_qp_const_in_ctb, size_in_bytes)
#define DUMP_NO_LOOP_FILTER(pu1_pic_no_loop_filter, size_in_bytes)
#define DUMP_OFFSETS(beta_offset_div_2, tc_offset_div_2, qp_offset_u, qp_offset_v)

#endif

#if DEBUG_DEBLK_LEAF_LEVEL

#define DUMP_DEBLK_LUMA_VERT(pu1_src, src_strd, u4_bs, qp_p, qp_q, beta_offset_div2, tc_offset_div2, filter_p, filter_q) ihevcd_debug_deblk_luma_vert(pu1_src, src_strd, u4_bs, qp_p, qp_q, beta_offset_div2, tc_offset_div2, filter_p, filter_q);
#define DUMP_DEBLK_LUMA_HORZ(pu1_src, src_strd, u4_bs, qp_p, qp_q, beta_offset_div2, tc_offset_div2, filter_p, filter_q) ihevcd_debug_deblk_luma_horz(pu1_src, src_strd, u4_bs, qp_p, qp_q, beta_offset_div2, tc_offset_div2, filter_p, filter_q);
#define DUMP_DEBLK_CHROMA_VERT(pu1_src, src_strd, u4_bs, qp_p, qp_q, qp_offset_u, qp_offset_v, tc_offset_div2, filter_p, filter_q) ihevcd_debug_deblk_chroma_vert(pu1_src, src_strd, u4_bs, qp_p, qp_q, qp_offset_u, qp_offset_v, tc_offset_div2, filter_p, filter_q)
#define DUMP_DEBLK_CHROMA_HORZ(pu1_src, src_strd, u4_bs, qp_p, qp_q, qp_offset_u, qp_offset_v, tc_offset_div2, filter_p, filter_q) ihevcd_debug_deblk_chroma_horz(pu1_src, src_strd, u4_bs, qp_p, qp_q, qp_offset_u, qp_offset_v, tc_offset_div2, filter_p, filter_q)

#else

#define DUMP_DEBLK_LUMA_VERT(pu1_src, src_strd, u4_bs3, qp_p, qp_q, beta_offset_div2, tc_offset_div2, filter_p, filter_q)
#define DUMP_DEBLK_LUMA_HORZ(pu1_src, src_strd, u4_bs3, qp_p, qp_q, beta_offset_div2, tc_offset_div2, filter_p, filter_q)
#define DUMP_DEBLK_CHROMA_VERT(pu1_src, src_strd, u4_bs, qp_p, qp_q, qp_offset_u, qp_offset_v, tc_offset_div2, filter_p, filter_q)
#define DUMP_DEBLK_CHROMA_HORZ(pu1_src, src_strd, u4_bs, qp_p, qp_q, qp_offset_u, qp_offset_v, tc_offset_div2, filter_p, filter_q)

#endif

#if DEBUG_MV_MAP
#define DEBUG_DUMP_MV_MAP(ps_codec) ihevcd_debug_dump_mv_map(ps_codec);
#else
#define DEBUG_DUMP_MV_MAP(ps_codec)
#endif
void print_coeff(WORD16 *pi2_tu_coeff, WORD32 trans_size);

void print_dst(UWORD8 *pu1_dst,
               WORD32 dst_strd,
               WORD32 trans_size,
               WORD32 is_luma);

#endif /* _IHEVCD_DEBUG_H_ */
