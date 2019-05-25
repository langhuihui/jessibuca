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
 *  ihevcd_structs.h
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

#ifndef _IHEVCD_STRUCTS_H_
#define _IHEVCD_STRUCTS_H_
typedef enum
{
    INIT_DONE, HEADER_DONE, FIRST_FRAME_DONE,
}CODEC_STATE_T;



typedef struct _codec_t codec_t;

/** Structure to hold format conversion context */
typedef struct
{
    /** Current row for which format conversion should be done */
    WORD32 i4_cur_row;

    /** Number of rows for which format conversion should be done */
    WORD32 i4_num_rows;
}fmt_conv_t;

/**
 * Bitstream structure
 */
typedef struct
{
    /**
     * Bitstream buffer base pointer
     */
    UWORD8 *pu1_buf_base;

    /**
     * Bitstream bit offset in current word. Value between 0 and 31
     */
    UWORD32 u4_bit_ofst;

    /**
     * Current bitstream buffer pointer
     */
    UWORD32 *pu4_buf;

    /**
     * Current word
     */
    UWORD32 u4_cur_word;

    /**
     * Next word
     */
    UWORD32 u4_nxt_word;

    /**
     * Max address for bitstream
     */
    UWORD8 *pu1_buf_max;
}bitstrm_t;

/**
******************************************************************************
 *  @brief      Cabac context for decoder
******************************************************************************
 */
typedef struct cab_ctxt
{
    /*********************************************************************/
    /*  CABAC ENGINE related fields                                      */
    /*********************************************************************/
    /** cabac interval range  R */
    UWORD32  u4_range;

    /** cabac interval offset O  */
    UWORD32  u4_ofst;

    /*********************************************************************/
    /*  CABAC context models                                             */
    /*********************************************************************/
    /** All Context models stored in pscked form pState[bits6-1] | MPS[bit0] */
    UWORD8  au1_ctxt_models[IHEVC_CAB_CTXT_END];

    /** Context models memorized after decoding 2nd CTB in a row to be used
     * during entropy sync cases
     */
    UWORD8 au1_ctxt_models_sync[IHEVC_CAB_CTXT_END];

}cab_ctxt_t;

typedef enum
{
    CMD_PROCESS,
    CMD_FMTCONV,
}JOBQ_CMD_T;

/**
 * Structure to represent a processing job entry
 */
typedef struct
{
    /**
     * Command
     * Currently: PROCESS, FMTCONV are the only two jobs
     */
    WORD32 i4_cmd;
    /**
     * CTB x of the starting CTB
     */
    WORD16 i2_ctb_x;

    /**
     * CTB y of the starting CTB
     */

    WORD16 i2_ctb_y;

    /**
     * Number of CTBs that need to be processed in this job
     */
    WORD16 i2_ctb_cnt;

    /**
     *  Slice index for the current CTB
     */
    WORD16 i2_slice_idx;

    /**
     * TU coefficient data offset for the current job
     */
    WORD32 i4_tu_coeff_data_ofst;
}proc_job_t;
/**
 * Structure to represent a MV Bank buffer
 */
typedef struct
{
    /**
     *  Pointer to hold PU index for each CTB in a picture
     */
    UWORD32 *pu4_pic_pu_idx;

    /**
     * Pointer to hold pu_t for each PU in a picture
     */
    pu_t *ps_pic_pu;

    /**
     * Pointer to hold PU map for each CTB in a picture
     */
    UWORD8 *pu1_pic_pu_map;

    /**
     * Pointer to hold the Slice map
     */
    UWORD16 *pu1_pic_slice_map;

    /**
     * Absolute POC for the current MV Bank
     */
    WORD32 i4_abs_poc;

    /**
     * Absolute POCs of reference List 0 for all slices in the frame from which this frame is reconstructed
     */
    WORD32 ai4_l0_collocated_poc[MAX_SLICE_HDR_CNT][MAX_DPB_SIZE];

    /**
     * Flag to indicate Long Term reference for POCs of reference List 0 for all slices in the frame from which this frame is reconstructed
     */
    WORD8 ai1_l0_collocated_poc_lt[MAX_SLICE_HDR_CNT][MAX_DPB_SIZE];

    /**
     * Absolute POCs of reference List 1 for all slices in the frame from which this frame is reconstructed
     */
    WORD32 ai4_l1_collocated_poc[MAX_SLICE_HDR_CNT][MAX_DPB_SIZE];
    /**
     * Flag to indicate Long Term reference for POCs of reference List 1 for all slices in the frame from which this frame is reconstructed
     */
    WORD8 ai1_l1_collocated_poc_lt[MAX_SLICE_HDR_CNT][MAX_DPB_SIZE];

}mv_buf_t;

typedef struct
{
    /**
     * Pointer to current PPS
     */
    pps_t *ps_pps;

    /**
     * Pointer to current SPS
     */
    sps_t *ps_sps;

    /**
     * Pointer to current slice header structure
     */
    slice_header_t *ps_slice_hdr;

    /**
     * CTB's x position within a picture in raster scan in CTB units
     */
    WORD32 i4_ctb_x;

    /**
     * CTB's y position within a picture in raster scan in CTB units
     */

    WORD32 i4_ctb_y;

    /**
     * Current PU structure - set to CTB pu_t pointer at the start of CTB processing and incremented
     * for every TU
     */
    pu_t *ps_pu;

    /**
     * Pointer to frame level pu_t for the current frame being parsed
     * where MVs and Intra pred modes will be updated
     */
    pu_t *ps_pic_pu;

    /**
     * Store the current tile's information. This is needed for the computation of mvs.
     */
    tile_t *ps_tile;

    /**
     * Points to an array of PU indices which is used to identify
     * start index of pu_t in ps_pic_pu and also to identify number of
     * PUs in the current CTB by subtracting current idx from next CTB's
     * PU idx
     */
    UWORD32 *pu4_pic_pu_idx;

    /** PU Index map per CTB. The indices in this map are w.r.t picture pu array and not
     * w.r.t CTB pu array.
     * This will be used during mv prediction and since neighbours will have different CTB pu map
     * it will be easier if they all have indices w.r.t picture level PU array rather than CTB level
     * PU array.
     * pu1_pic_pu_map is map w.r.t CTB's pu_t array
     */
    UWORD32 *pu4_pic_pu_idx_map;

    /**
      * Pointer to pu_map for the current frame being parsed
      * where MVs and Intra pred modes will be updated
      */
    UWORD8 *pu1_pic_pu_map;

    /**
     *  PU count in current CTB
     */
    WORD32 i4_ctb_pu_cnt;

    /**
     *  PU count in current CTB
     */
    WORD32 i4_ctb_start_pu_idx;

    /**
     *  Top availability for current CTB level
     */
    UWORD8 u1_top_ctb_avail;

    /**
     *  Top right availability for current CTB level
     */
    UWORD8 u1_top_rt_ctb_avail;
    /**
     *  Top left availability for current CTB level
     */
    UWORD8 u1_top_lt_ctb_avail;
    /**
     *  left availability for current CTB level
     */
    UWORD8 u1_left_ctb_avail;

}mv_ctxt_t;

typedef struct
{
    /**
     * Pointer to current PPS
     */
    pps_t *ps_pps;

    /**
     * Pointer to current SPS
     */
    sps_t *ps_sps;

    /*
     * Pointer to codec context
     */
    codec_t *ps_codec;

    /**
     * Index of the current Tile being parsed
     */
    tile_t *ps_tile;

    /**
     * Pointer to the current slice header
     */
    slice_header_t *ps_slice_hdr;

    /**
     *  TU count in current CTB
     */
    WORD32 i4_ctb_tu_cnt;

    /**
     * CTB's x position within a picture in raster scan in CTB units
     */
    WORD32 i4_ctb_x;

    /**
     * CTB's y position within a picture in raster scan in CTB units
     */

    WORD32 i4_ctb_y;

    /**
     * CTB's x position within a Tile in raster scan in CTB units
     */
    WORD32 i4_ctb_tile_x;

    /**
     * CTB's y position within a Tile in raster scan in CTB units
     */

    WORD32 i4_ctb_tile_y;

    /**
     * CTB's x position within a Slice in raster scan in CTB units
     */
    WORD32 i4_ctb_slice_x;

    /**
     * CTB's y position within a Slice in raster scan in CTB units
     */

    WORD32 i4_ctb_slice_y;

    /* Two bits per edge.
    Stored in format. BS[15] | BS[14] | .. |BS[0]*/
    UWORD32 *pu4_pic_vert_bs;

    /**
     * Horizontal Boundary strength
     */

    /* Two bits per edge.
    Stored in format. BS[15] | BS[14] | .. |BS[0]*/
    UWORD32 *pu4_pic_horz_bs;

    /**
     * Flags to indicate if QP is constant through out a CTB - 1 bit for each CTB
     * The bits are packed from LSB to MSB
     * To get the flag corresponding to CTB with (ctb_x, ctb_y), use
     *      pu4_qp_const_in_ctb[(ctb_x + pic_wd_in_ctb * ctb_y) >> 3] & (1 << ((ctb_x + pic_wd_in_ctb * ctb_y) & 7))
     */
    UWORD8 *pu1_pic_qp_const_in_ctb;

    /**
     *  Qp array stored for each 8x8 pixels
     */
    UWORD8  *pu1_pic_qp;

    /**
     * Current TU structure - set to CTB tu_t pointer at the start of CTB processing and incremented
     * for every TU
     */
    tu_t *ps_tu;

    /**
     * Points to an array of TU indices which is used to identify
     * start index of tu_t in ps_pic_tu and also to identify number of
     * TUs in the current CTB by subtracting current idx from next CTB's
     * TU idx
     */
    UWORD32 *pu4_pic_tu_idx;

    /**
     * Points to an array of PU indices which is used to identify
     * start index of pu_t in ps_pic_pu and also to identify number of
     * PUs in the current CTB by subtracting current idx from next CTB's
     * PU idx
     */
    UWORD32 *pu4_pic_pu_idx;

    /**
     * Current PU structure - set to CTB pu_t pointer at the start of CTB processing and incremented
     * for every TU
     */
    pu_t *ps_pu;

    /**
     * Pointer to frame level pu_t for the current frame being parsed
     * where MVs and Intra pred modes will be updated
     */
    pu_t *ps_pic_pu;

    /** PU Index map per CTB. The indices in this map are w.r.t picture pu array and not
     * w.r.t CTB pu array.
     * This will be used during mv prediction and since neighbours will have different CTB pu map
     * it will be easier if they all have indices w.r.t picture level PU array rather than CTB level
     * PU array.
     * pu1_pic_pu_map is map w.r.t CTB's pu_t array
     */
    UWORD32 *pu4_pic_pu_idx_map;

    /**
     * Variable to store the next ctb count to compute pu idx
     */
    WORD32 i4_next_pu_ctb_cnt;

    /**
     * Variable to store the next ctb count to compute tu idx
     */
    WORD32 i4_next_tu_ctb_cnt;
    /**
     * Points to the array of slice indices which is used to identify the slice
     *  to which each CTB in a frame belongs.
     */
    UWORD16 *pu1_slice_idx;
}bs_ctxt_t;

typedef struct
{
    /**
     * Pointer to current PPS
     */
    pps_t *ps_pps;

    /**
     * Pointer to current SPS
     */
    sps_t *ps_sps;

    /*
     * Pointer to codec context
     */
    codec_t *ps_codec;

    /**
     * Pointer to current slice header structure
     */
    slice_header_t *ps_slice_hdr;

    /**
     * Pointer to the structure that contains BS and QP frame level arrays
     */
    bs_ctxt_t s_bs_ctxt;

    /**
     * CTB's x position within a picture in raster scan in CTB units
     */
    WORD32 i4_ctb_x;

    /**
     * CTB's y position within a picture in raster scan in CTB units
     */

    WORD32 i4_ctb_y;

    /**
     * Current pictures loop filter flag map at 8x8 level
     */
    UWORD8 *pu1_pic_no_loop_filter_flag;

    /**
     * Current CTB's no_loop_filter_flags
     * each element corresponds to one row - including the left CTB's last 8x8
     */
    UWORD16 au2_ctb_no_loop_filter_flag[9];

    /*
     * Pointer to 0th luma pixel in current pic
     */
    UWORD8 *pu1_cur_pic_luma;

    /*
     * Pointer to 0th chroma pixel in current pic
     */
    UWORD8 *pu1_cur_pic_chroma;

    /* Points to the array of slice indices which is used to identify the slice
    *  to which each CTB in a frame belongs.
    */
    UWORD16 *pu1_slice_idx;

    /* Specifies if the chroma format is yuv420sp_vu */
    WORD32 is_chroma_yuv420sp_vu;

}deblk_ctxt_t;

typedef struct
{
    /**
     * Pointer to current PPS
     */
    pps_t *ps_pps;

    /**
     * Pointer to current SPS
     */
    sps_t *ps_sps;

    /* Pointer to codec context
     *
     */
    codec_t *ps_codec;

    /**
     * Pointer to base slice header structure
     */
    slice_header_t *ps_slice_hdr_base;

    /**
     * Pointer to current slice header structure
     */
    slice_header_t *ps_slice_hdr;

    /**
     * Pointer to current tile structure
     */
    tile_t *ps_tile;
    /**
     * CTB's x position within a picture in raster scan in CTB units
     */
    WORD32 i4_ctb_x;

    /**
     * CTB's y position within a picture in raster scan in CTB units
     */

    WORD32 i4_ctb_y;

    /**
     * Current pictures loop filter flag map at 8x8 level
     */
    UWORD8 *pu1_pic_no_loop_filter_flag;

    /*
     * Pointer to 0th luma pixel in current pic
     */
    UWORD8 *pu1_cur_pic_luma;

    /*
     * Pointer to 0th chroma pixel in current pic
     */
    UWORD8 *pu1_cur_pic_chroma;

    /**
     * Pointer to frame level sao_t for the current frame being parsed
     */
    sao_t *ps_pic_sao;

    /**
     * Temporary buffer needed during SAO processing
     */
    UWORD8 *pu1_tmp_buf_luma;

    /**
     * Temporary buffer needed during SAO processing
     */
    UWORD8 *pu1_tmp_buf_chroma;

    /**
     * Left column of luma pixels - used by SAO
     */
    UWORD8 *pu1_sao_src_left_luma;

    /**
     * Top row of luma pixels - used by SAO
     */
    UWORD8 *pu1_sao_src_top_luma;

    /**
     * Left column of chroma pixels(interleaved) - used by SAO
     */
    UWORD8 *pu1_sao_src_left_chroma;

    /**
     * Top row of chroma pixels(interleaved) - used by SAO
     */
    UWORD8 *pu1_sao_src_top_chroma;

    /**
     * Top-left luma pixel - used by SAO (for the top CTB row)
     */
    UWORD8 *pu1_sao_src_luma_top_left_ctb;

    /**
     * Top-left chroma pixel(interleaved) - used by SAO (for the top CTB row)
     */
    UWORD8 *pu1_sao_src_chroma_top_left_ctb;

    /**
     * Top-left luma pixel - used by SAO (for the current  CTB row)
     */
    UWORD8 *pu1_sao_src_top_left_luma_curr_ctb;

    /**
     * Top-left chroma pixel(interleaved) - used by SAO (for the current CTB row)
     */
    UWORD8 *pu1_sao_src_top_left_chroma_curr_ctb;

    /**
     * Top-right luma pixel - used by SAO (for the top CTB row)
     */
    UWORD8 *pu1_sao_src_top_left_luma_top_right;

    /**
     * Top-right chroma pixel(interleaved) - used by SAO (for the top CTB row)
     */
    UWORD8 *pu1_sao_src_top_left_chroma_top_right;

    /**
     * Bottom-left luma pixel - used by SAO
     */
    UWORD8 u1_sao_src_top_left_luma_bot_left;
    /**
     *  Pointer to array that stores bottom left luma pixel per row(interleaved) - used by SAO
     */
    UWORD8 *pu1_sao_src_top_left_luma_bot_left;

    /**
     * Bottom left chroma pixel(interleaved) - used by SAO
     */
    UWORD8 au1_sao_src_top_left_chroma_bot_left[2];
    /**
     *  Pointer to array that stores bottom left chroma pixel per row(interleaved) - used by SAO
     */
    UWORD8 *pu1_sao_src_top_left_chroma_bot_left;

    /*
     * Slice counter in a picture.
     */
    UWORD32 i4_cur_slice_idx;
    /**
     * Points to the array of slice indices which is used to identify the slice
     *  to which each CTB in a frame belongs.
     */
    UWORD16 *pu1_slice_idx;
    /**
     * Points to the array of tile indices which is used to identify the slice
     *  to which each CTB in a frame belongs.
     */
    UWORD16 *pu1_tile_idx;

    /* Specifies if the chroma format is yuv420sp_vu */
    WORD32 is_chroma_yuv420sp_vu;

}sao_ctxt_t;

typedef struct
{
    /** Log2 CU's size */
    WORD32 i4_log2_cb_size;

    /** CU's x position */
    WORD32 i4_pos_x;

    /** CU's y position */
    WORD32 i4_pos_y;
    /**
     * Transquant Bypass enable flag at CU level - To be replicated at TU level
     */
    WORD32 i4_cu_transquant_bypass;
    /**
     * Prediction mode
     */
    WORD32 i4_pred_mode;

    /**
     * Partition mode
     */
    WORD32 i4_part_mode;

    /**
     * Intra luma pred mode for current CU. In case of PART2Nx2N
     * the first value is replicated to avoid checks later
     */
    WORD32 ai4_intra_luma_pred_mode[4];

    /**
     * Previous intra luma pred flag used for intra pred mode computation
     */
    WORD32 ai4_prev_intra_luma_pred_flag[4];

    /**
     * mpm index used in intra prediction mode computation
     */
    WORD32 ai4_mpm_idx[4];
    /**
     * Remaining intra pred mode
     */
    WORD32 ai4_rem_intra_luma_pred_mode[4];
    /**
     * Chroma pred mode index to be used to compute intra pred mode for chroma
     */
    WORD32 i4_intra_chroma_pred_mode_idx;
    /**
     * Maximum transform depth
     */
    WORD32 i4_max_trafo_depth;

    /**
     *  Luma CBF for current TU
     */
    UWORD8 i1_cbf_luma;

    /**
     * Cb CBF
     */
    UWORD8 ai1_cbf_cb[MAX_TRAFO_DEPTH];

    /**
     * Cr CBF
     */
    UWORD8 ai1_cbf_cr[MAX_TRAFO_DEPTH];

    /**
     * Intra split flag
     */
    WORD32 i4_intra_split_flag;

    /**
     * Current QP
     */
    WORD32 i4_qp;

    /**
     * Number of TUs in CU parsed before a change in QP is signaled
     */
    WORD32 i4_tu_cnt;

    /**
     * Cu QP delta
     */
    WORD32 i4_cu_qp_delta;

}parse_cu_t;
/**
 * Structure contains few common state variables such as CTB positions, current SPS, PPS ids etc which are to be
 * used in the parsing thread. By keeping it a different structure it is being explicitly signalled that these
 * variables are specific to Parsing threads context and other threads should not update these elements
 */
typedef struct
{
    /**
     * CTB's x position within a picture in raster scan in CTB units
     */
    WORD32 i4_ctb_x;

    /**
     * CTB's y position within a picture in raster scan in CTB units
     */

    WORD32 i4_ctb_y;

    /**
     * CTB's x position within a Tile in raster scan in CTB units
     */
    WORD32 i4_ctb_tile_x;

    /**
     * CTB's y position within a Tile in raster scan in CTB units
     */

    WORD32 i4_ctb_tile_y;

    /**
     * CTB's x position within a Slice in raster scan in CTB units
     */
    WORD32 i4_ctb_slice_x;

    /**
     * CTB's y position within a Slice in raster scan in CTB units
     */

    WORD32 i4_ctb_slice_y;

    /**
     * Index of the current Tile being parsed
     */
    tile_t *ps_tile;

    /**
     * Current slice idx - Used in multi-core cases to ensure slice header is
     * preserved till the last CB of the slice is decoded
     */
    WORD32 i4_cur_slice_idx;
    /**
     * Current slice idx - Used in multi-core cases to ensure slice header is
     * preserved till the last CB of the slice is decoded
     */
    WORD32 i4_cur_independent_slice_idx;

    /**
     * Current slice idx - Used in multi-core cases to ensure slice header is
     * preserved till the last CB of the slice is decoded
     */
    WORD32 i4_cur_tile_idx;

    /**
     * Pointer to current PPS
     */
    pps_t *ps_pps;

    /**
     * Pointer to current SPS
     */
    sps_t *ps_sps;

    /**
     * Signal that pic_init is called first time
     */
    WORD32 i4_first_pic_init;

    /**
     * Flag to indicate if CU QP delta is coded.
     * By default it is set to 0 at the beginning of coding quad tree
     */
    WORD32 i4_is_cu_qp_delta_coded;

    /**
     * CU Qp delta
     * By default it is set to 0 at the beginning of coding quad tree
     */
    WORD32 i4_cu_qp_delta;

    /**
     * Bitstream structure
     */
    bitstrm_t s_bitstrm;

    /**
     * Pointer frame level TU subblock coeff data
     */
    void *pv_pic_tu_coeff_data;

    /**
     * Pointer to TU subblock coeff data and number of coded subblocks and scan idx
     * Incremented each time a coded subblock is parsed
     *
     */
    void *pv_tu_coeff_data;

    /**
     * Current TU structure - set to CTB tu_t pointer at the start of CTB parsing and incremented
     * for every TU
     */
    tu_t *ps_tu;

    /**
     * Current ctb's TU map
     */
    UWORD8 *pu1_tu_map;

    /**
     * Current PU structure - set to CTB pu_t pointer at the start of CTB parsing and incremented
     * for every TU
     */
    pu_t *ps_pu;

    /**
     * Points to the array of slice indices which is used to identify the independent slice
     *  to which each CTB in a frame belongs.
     */
    UWORD16 *pu1_slice_idx;

    /**
     * Current PU index in a frame
     */
    WORD32 i4_pic_pu_idx;

    /**
     * Current TU index in a frame
     */
    WORD32 i4_pic_tu_idx;

    /**
     * Current PU structure - set to CTB pu_map pointer at the start of CTB parsing
     */
    UWORD8 *pu1_pu_map;

    /**
     * Current QP
     */
    WORD32 u4_qp;

    /**
     * Current Group's QP
     */
    WORD32 u4_qpg;

    /**
     * Number of PCM blocks in current CTB - Needed only during parsing
     * If needed during recon then move it to ctb_t
     */
    WORD32 i4_ctb_num_pcm_blks;

    /**
     * PCM flag for the current CU
     */
    WORD32 i4_cu_pcm_flag;

    /**
     * CU related information to be used to populate tu_t and pu_t during
     * pred unit and transform tree parsing.
     */
    parse_cu_t s_cu;

    /**
     * Pointer to pu_map for the current frame being parsed
     */
    UWORD8 *pu1_pic_pu_map;

    /**
     * Pointer to frame level pu_t for the current frame being parsed
     * where MVs and Intra pred modes will be updated
     */
    pu_t *ps_pic_pu;

    /**
     * Pointer to tu_map for the current frame being parsed
     */
    UWORD8 *pu1_pic_tu_map;

    /**
     * Pointer to frame level tu_t for the current frame being parsed
     * where transform unit related info will be updated
     */
    tu_t *ps_pic_tu;

    /**
     * Points to an array of TU indices which is used to identify
     * start index of tu_t in ps_pic_tu and also to identify number of
     * TUs in the current CTB by subtracting current idx from next CTB's
     * TU idx
     */
    UWORD32 *pu4_pic_tu_idx;

    /**
     * Points to an array of PU indices which is used to identify
     * start index of pu_t in ps_pic_pu and also to identify number of
     * PUs in the current CTB by subtracting current idx from next CTB's
     * PU idx
     */
    UWORD32 *pu4_pic_pu_idx;


    /**
     * Current pictures intra mode map at 8x8 level
     */
    UWORD8 *pu1_pic_intra_flag;

    /**
     * Current pictures loop filter flag map at 8x8 level
     */
    UWORD8 *pu1_pic_no_loop_filter_flag;

    /**
     * Array to hold one row (top) of skip_flag flag stored at (8x8) level
     * 1 bit per (8x8)
     * read and written as a UWORD32
     * LSB gives skip_flag for 0th 8x8 and MSB gives skip_flag for 31st 8x8 and so on
     * This is independent of CTB size or minCU size
     * Packed format requires extra calculations in extracting required bits but makes it easier
     * to store skip data for larger sizes such as 32 x 32 where 4 bits need to be set instead of
     * 4 bytes or for 64 x 64 where 8 bits need to be set instead of 8 bytes.
     */
    UWORD32 *pu4_skip_cu_top;

    /**
     * Array to hold one 64 pixel column (left) of skip_flag flag stored at (8x8) level
     * 1 bit per (8x8)
     * read and written as a UWORD32
     * LSB gives skip_flag for 0th 8x8 and MSB gives skip for 31st 8x8 and so on
     * This is independent of CTB size and allocated to store data for 64 pixels, of
     * this only first ctb_size number of bits (starting from MSB) will have valid data
     * This is also independent of min CU size and data is stored at 8x8 level.
     * Since only 8 bits are needed to represent left 64 pixels at 8x8 level, this is not an array
     */
    UWORD32 u4_skip_cu_left;

    /**
     * Array to hold one row (top) of coding_tree_depth stored at (8x8) level
     * 2 bits per (8x8) pixels
     * read and written as a WORD32
     * 2 LSBits give coding_tree_depth for 0th 8x8 and 2 MSBits give coding_tree_depth for 15th 8x8 and so on
     * This is independent of CTB size or minCU size
     */
    UWORD32 *pu4_ct_depth_top;

    /**
     * Array to hold one 64 pixel column (left) of coding_tree_depth stored at (8x8) level
     * 2 bits per (8x8) pixels
     * read and written as a WORD32
     * 2 LSBits give coding_tree_depth for 0th 8x8 and 2 MSBits give coding_tree_depth for 15th 8x8 and so on
     * This is independent of CTB size and allocated to store data for 64 pixels, of
     * this only first ctb_size * 2 number of bits (starting from MSB) will have valid data
     * This is also independent of min CU size and data is stored at 8x8 level.
     * Since only 16 bits are needed to represent left 64 pixels at 8x8 level, this is not an array
     */
    UWORD32 u4_ct_depth_left;

    /**
     * Array to hold top (one row) luma_intra_pred_mode stored at (4x4) level for a CTB
     * 8 bits per (4x4) pixels
     * read and written as a UWORD8
     * This is independent of CTB size or minCU size
     * This is independent of CTB size and allocated to store data for 64 pixels i.e. 64 bits is the size
     * Note this data is used only within a CTB, There is no inter CTB dependencies for this
     */
    UWORD8 *pu1_luma_intra_pred_mode_top;

    /**
     * Array to hold  left (one column) luma_intra_pred_mode stored at (4x4) level for a CTB
     * 8 bits per (4x4) pixels
     * read and written as a UWORD8
     * This is independent of CTB size and allocated to store data for 64 pixels i.e. 64 bits is the size
     * This is also independent of min CU size and data is stored at 8x8 level.
     * This is used for prediction of next CTB within a row in a slice or tile
     */
    UWORD8 *pu1_luma_intra_pred_mode_left;


    /**
     * Pointer to base of Video parameter set structure array
     */
    vps_t *ps_vps_base;

    /**
     * Pointer to base of Sequence parameter set structure array
     */
    sps_t *ps_sps_base;

    /**
     * Pointer to base of Picture parameter set structure array
     */
    pps_t *ps_pps_base;

    /**
     * Pointer to base of slice header structure array
     */
    slice_header_t *ps_slice_hdr_base;

    /**
     * Pointer to current slice header structure
     */
    slice_header_t *ps_slice_hdr;


    /**
     * Error code during parse stage
     */
    WORD32 i4_error_code;

    /**
     * Void pointer to process job context
     */
    void *pv_proc_jobq;

    /* Cabac context */
    cab_ctxt_t s_cabac;

    /* Current Coding tree depth */
    WORD32 i4_ct_depth;

    /** Flag to signal end of frame */
    WORD32 i4_end_of_frame;

    /**
     * Index of the next CTB parsed
     */
    WORD32 i4_next_ctb_indx;

    /**
     * Pointer to the structure that contains BS and QP frame level arrays
     */
    bs_ctxt_t s_bs_ctxt;

    /**
     * Pointer to the structure that contains deblock context
     */
    deblk_ctxt_t s_deblk_ctxt;

    /**
     * Pointer to the structure that contains sao context
     */
    sao_ctxt_t s_sao_ctxt;

    /**
     * QP Array for the current CTB
     * Used in QP prediction
     */
    WORD8 ai1_8x8_cu_qp[MAX_CU_IN_CTB];


    /**
     * Pointer to frame level sao_t for the current frame being parsed
     */
    sao_t *ps_pic_sao;

    /**
     * Abs POC count of the frame
     */
    WORD32 i4_abs_pic_order_cnt;

    /**
     * Pointer points to mv_buffer of current frame
     */
    mv_buf_t *ps_cur_mv_buf;

    /**
     * Variable to store the next ctb count to compute pu idx
     */
    WORD32 i4_next_pu_ctb_cnt;

    /**
     * Variable to store the next ctb count to compute tu idx
     */
    WORD32 i4_next_tu_ctb_cnt;

    /**
     * SEI parameters
     */
    sei_params_t s_sei_params;
}parse_ctxt_t;

/**
 * Pixel processing thread context
 */

typedef struct
{
    /* Pointer to codec context
     *
     */
    codec_t *ps_codec;

    /**
     * CTB's x position within a picture in raster scan in CTB units
     */
    WORD32 i4_ctb_x;

    /**
     * CTB's y position within a picture in raster scan in CTB units
     */

    WORD32 i4_ctb_y;

    /**
     * CTB's x position within a Tile in raster scan in CTB units
     */
    WORD32 i4_ctb_tile_x;

    /**
     * CTB's y position within a Tile in raster scan in CTB units
     */

    WORD32 i4_ctb_tile_y;

    /**
     * CTB's x position within a Slice in raster scan in CTB units
     */
    WORD32 i4_ctb_slice_x;

    /**
     * CTB's y position within a Slice in raster scan in CTB units
     */

    WORD32 i4_ctb_slice_y;

    /**
     * Current tile being processed
     */
    tile_t *ps_tile;

    /**
     * Current slice idx - Used in multi-core cases to store slice index for
     * each ctb for sao filtering.
     */
    WORD32 i4_cur_slice_idx;

    /**
     * Current tile idx - Used in multi-core cases to store tile index for
     * each ctb for sao filtering.
     */
    WORD32 i4_cur_tile_idx;
    /**
     * Pointer to current PPS
     */
    pps_t *ps_pps;

    /**
     * Pointer to current SPS
     */
    sps_t *ps_sps;

    /**
     * Pointer to current slice header structure
     */
    slice_header_t *ps_slice_hdr;

    /**
     * Error code during parse stage
     */
    WORD32 i4_error_code;

    /**
     * Signal that pic_init is called first time
     */
    WORD32 i4_first_pic_init;

    /**
     * Pointer frame level TU subblock coeff data
     */
    void *pv_pic_tu_coeff_data;

    /**
     * Pointer to TU subblock coeff data and number of subblocks and scan idx
     * Incremented each time a coded subblock is processed
     *
     */
    void *pv_tu_coeff_data;

    /**
     * Current TU structure - set to CTB tu_t pointer at the start of CTB processing and incremented
     * for every TU
     */
    tu_t *ps_tu;

    /**
     * Current ctb's TU map
     */
    UWORD8 *pu1_tu_map;

    /**
     * Current PU structure - set to CTB pu_t pointer at the start of CTB processing and incremented
     * for every TU
     */
    pu_t *ps_pu;

    /**
     * Points to an array of TU indices which is used to identify
     * start index of tu_t in ps_pic_tu and also to identify number of
     * TUs in the current CTB by subtracting current idx from next CTB's
     * TU idx
     */
    UWORD32 *pu4_pic_tu_idx;

    /**
     * Points to an array of PU indices which is used to identify
     * start index of pu_t in ps_pic_pu and also to identify number of
     * PUs in the current CTB by subtracting current idx from next CTB's
     * PU idx
     */
    UWORD32 *pu4_pic_pu_idx;

    /**
     * Pointer to tu_map for the current frame being parsed
     */
    UWORD8 *pu1_pic_tu_map;

    /**
      * Pointer to pu_map for the current frame being parsed
      * where MVs and Intra pred modes will be updated
      */
    UWORD8 *pu1_pic_pu_map;

    /**
     * Pointer to frame level pu_t for the current frame being parsed
     * where MVs and Intra pred modes will be updated
     */
    pu_t *ps_pic_pu;

    /** PU Index map per CTB. The indices in this map are w.r.t picture pu array and not
     * w.r.t CTB pu array.
     * This will be used during mv prediction and since neighbours will have different CTB pu map
     * it will be easier if they all have indices w.r.t picture level PU array rather than CTB level
     * PU array.
     * pu1_pic_pu_map is map w.r.t CTB's pu_t array
     */
    UWORD32 *pu4_pic_pu_idx_map;

    /**
     * PU Index of top 4x4 neighbors stored for an entire row
     */
    UWORD32 *pu4_pic_pu_idx_top;

    /**
     * PU Index of left 4x4 neighbors stored for 64 pixels
     */
    UWORD32 *pu4_pic_pu_idx_left;

    /**
     * Holds top left PU index at CTB level - top left gets overwritten
     * by left CTB while updating top array. Before updating top at CTB
     * level required top-left index is backed up in the following
     */
    UWORD32 u4_ctb_top_left_pu_idx;

    /**
     * Pointer to frame level tu_t for the current frame being parsed
     * where transform unit related info will be updated
     */
    tu_t *ps_pic_tu;


    /**
    * Current PU structure - set to CTB pu_map pointer at the start of CTB parsing
    */
    UWORD8 *pu1_pu_map;

    /** Current MV Bank's buffer ID */
    WORD32 i4_cur_mv_bank_buf_id;

    /**
     * Current pictures intra mode map at 8x8 level
     */
    UWORD8 *pu1_pic_intra_flag;

    /**
     * Current pictures loop filter flag map at 8x8 level
     */
    UWORD8 *pu1_pic_no_loop_filter_flag;

    /**
     * Void pointer to process job context
     */

    void *pv_proc_jobq;

    /**
     * Number of CTBs to be processed in the current Job
     */
    WORD32 i4_ctb_cnt;
    /**
     * ID for the current context - Used for debugging
     */
    WORD32 i4_id;

    /**
     * Flag to indicate if parsing status has to be checked
     * Needed when parsing and processing are done in different threads
     */
    WORD32 i4_check_parse_status;

    /**
     * Flag to indicate if processing status of top row CTBs has to be checked
     * Needed when processing of different rows is done in different threads
     */
    WORD32 i4_check_proc_status;

    /**
     * Holds Intra dequantization matrices
     */
    WORD16 *api2_dequant_intra_matrix[4];

    /**
     * Holds Inter dequantization matrices
     */
    WORD16 *api2_dequant_inter_matrix[4];


    /**
     * Temporary buffer 1 - Used as a scratch in inter_pred_ctb()
     */
    WORD16 *pi2_inter_pred_tmp_buf1;

    /**
     * Temporary buffer 2 - Used as a scratch in inter_pred_ctb()
     */
    WORD16 *pi2_inter_pred_tmp_buf2;

    /**
     * Temporary buffer 3 - Used as a scratch in inter_pred_ctb()
     */
    WORD16 *pi2_inter_pred_tmp_buf3;

    /**
     * The above temporary buffers' stride
     */
    WORD32 i4_inter_pred_tmp_buf_strd;
    /**
     * Picture stride
     * Used as prediction stride, destination stride while computing inverse transform
     */
    WORD32 i4_pic_strd;

    /**
     * Picture qp offset for U
     */
    WORD8 i1_pic_cb_qp_offset;

    /**
     * Slice qp offset for U
     */
    WORD32 i1_slice_cb_qp_offset;

    /**
     * Picture qp offset for V
     */
    WORD8 i1_pic_cr_qp_offset;

    /**
     * Slice qp offset for V
     */
    WORD32 i1_slice_cr_qp_offset;

    /** Pointer to current picture buffer structure */
    pic_buf_t *ps_cur_pic;

    /** Current pic_buf's picture buffer id */
    WORD32 i4_cur_pic_buf_id;

    /** Pointer to 0th luma pixel in current pic */
    UWORD8 *pu1_cur_pic_luma;

    /** Pointer to 0th chroma pixel in current pic */
    UWORD8 *pu1_cur_pic_chroma;

    /** Intermediate buffer to be used during inverse transform */
    WORD16 *pi2_itrans_intrmd_buf;

    /** Buffer to hold output of inverse scan */
    WORD16 *pi2_invscan_out;

    /**
     *  Top availability for current CTB level
     */
    UWORD8 u1_top_ctb_avail;

    /**
     *  Top right availability for current CTB level
     */
    UWORD8 u1_top_rt_ctb_avail;
    /**
     *  Top left availability for current CTB level
     */
    UWORD8 u1_top_lt_ctb_avail;
    /**
     *  left availability for current CTB level
     */
    UWORD8 u1_left_ctb_avail;
    /**
     *  TU count in current CTB
     */
    WORD32 i4_ctb_tu_cnt;

    /**
     *  Recon pointer to current CTB luma
     */
    UWORD8 *pu1_cur_ctb_luma;
    /**
     *  Recon pointer to current CTB chroma
     */
    UWORD8 *pu1_cur_ctb_chroma;

    /**
     *  PU count in current CTB
     */
    WORD32 i4_ctb_pu_cnt;

    /**
     *  PU count in current CTB
     */
    WORD32 i4_ctb_start_pu_idx;

    /* Pointer to a structure describing output display buffer */
    ivd_out_bufdesc_t *ps_out_buffer;

    /** Flag to indicate if ps_proc was intialized at least once in a frame.
     * This is needed to handle cases where a core starts to handle format conversion jobs directly
     */
    WORD32 i4_init_done;

    /**
     * Pointer to the structure that contains BS and QP frame level arrays
     */
    bs_ctxt_t s_bs_ctxt;

    /**
     * Pointer to the structure that contains deblock context
     */
    deblk_ctxt_t s_deblk_ctxt;

    /**
     * Pointer to the structure that contains sao context
     */
    sao_ctxt_t s_sao_ctxt;

    /**
     * Points to the array of slice indices which is used to identify the independent
     * slice to which each CTB in a frame belongs.
     */
    UWORD16 *pu1_slice_idx;

    /**
     * Points to the array of slice indices which is used to identify the slice
     *  to which each CTB in a frame belongs.
     */
    UWORD16 *pu1_tile_idx;
    /**
     * Variable to store the next ctb count to compute pu idx
     */
    WORD32 i4_next_pu_ctb_cnt;

    /**
     * Variable to store the next ctb count to compute tu idx
     */
    WORD32 i4_next_tu_ctb_cnt;
    /**
     * Number of ctb's to process in one loop
     */
    WORD32 i4_nctb;
}process_ctxt_t;

typedef void (*pf_inter_pred)(void *,
                              void *,
                              WORD32,
                              WORD32,
                              WORD8 *,
                              WORD32,
                              WORD32);


typedef void (*pf_intra_pred)(UWORD8 *pu1_ref,
                              WORD32 src_strd,
                              UWORD8 *pu1_dst,
                              WORD32 dst_strd,
                              WORD32 nt,
                              WORD32 mode);

typedef void (*pf_itrans_recon)(WORD16 *pi2_src,
                                WORD16 *pi2_tmp,
                                UWORD8 *pu1_pred,
                                UWORD8 *pu1_dst,
                                WORD32 src_strd,
                                WORD32 pred_strd,
                                WORD32 dst_strd,
                                WORD32 zero_cols,
                                WORD32 zero_rows);

typedef void (*pf_recon)(WORD16 *pi2_src,
                         UWORD8 *pu1_pred,
                         UWORD8 *pu1_dst,
                         WORD32 src_strd,
                         WORD32 pred_strd,
                         WORD32 dst_strd,
                         WORD32 zero_cols);

typedef void (*pf_itrans_recon_dc)(UWORD8 *pu1_pred,
                                   UWORD8 *pu1_dst,
                                   WORD32 pred_strd,
                                   WORD32 dst_strd,
                                   WORD32 log2_trans_size,
                                   WORD16 i2_coeff_value);


typedef void (*pf_sao_luma)(UWORD8 *,
                            WORD32,
                            UWORD8 *,
                            UWORD8 *,
                            UWORD8 *,
                            UWORD8 *,
                            UWORD8 *,
                            UWORD8 *,
                            WORD8 *,
                            WORD32,
                            WORD32);

typedef void (*pf_sao_chroma)(UWORD8 *,
                              WORD32,
                              UWORD8 *,
                              UWORD8 *,
                              UWORD8 *,
                              UWORD8 *,
                              UWORD8 *,
                              UWORD8 *,
                              WORD8 *,
                              WORD8 *,
                              WORD32,
                              WORD32);

/**
 * Codec context
 */

struct _codec_t
{
    /**
     * Width : pic_width_in_luma_samples
     */
    WORD32 i4_wd;

    /**
     * Height : pic_height_in_luma_samples
     */
    WORD32 i4_ht;

    /**
     * Display width after cropping
     */
    WORD32 i4_disp_wd;

    /**
     * Display height after cropping
     */
    WORD32 i4_disp_ht;

    /**
     * Display stride
     */
    WORD32 i4_disp_strd;

    /**
     * Stride of reference buffers.
     * For shared mode even display buffer will use the same stride
     */
    WORD32 i4_strd;

    /**
     * Number of cores to be used
     */
    WORD32 i4_num_cores;

    /**
     * RASL output flag
     */
    WORD32 i4_rasl_output_flag;

    /**
     * This flag is set if the next picture received is a CRA and has to be treated as a first pic in the video sequence
     * For example, it is set, if an EOS (end of stream) NAL is received
     */
    WORD32 i4_cra_as_first_pic;

    /**
     * Pictures that are are degraded
     * 0 : No degrade
     * 1 : Only on non-reference frames
     * 2 : Use interval specified by u4_nondegrade_interval
     * 3 : All non-key frames
     * 4 : All frames
     */
    WORD32                                     i4_degrade_pics;

    /**
     * Interval for pictures which are completely decoded without any degradation
     */
    WORD32                                     i4_nondegrade_interval;

    /**
     * bit position (lsb is zero): Type of degradation
     * 0 : Disable SAO
     * 1 : Disable deblocking
     * 2 : Faster inter prediction filters
     * 3 : Fastest inter prediction filters
     */
    WORD32                                     i4_degrade_type;

    /** Degrade pic count, Used to maintain the interval between non-degraded pics
     *
     */
    WORD32  i4_degrade_pic_cnt;

    /**
     * Total number of display buffers to be used
     * In case of shared mode, this will be number of reference frames
     */
    WORD32 i4_num_disp_bufs;

    /**
     * Flag to enable shared display buffer mode
     */
    WORD32 i4_share_disp_buf;

    /**
     * Chroma format of display buffers.
     In shared mode only 420SP_UV and 420SP_VU are supported
     */
    IV_COLOR_FORMAT_T e_chroma_fmt;

    /**
     * Chroma format of reference buffers.
     * In non-shared mode it will be 420SP_UV
     * In shared mode only 420SP_UV and 420SP_VU are supported
     */
    IV_COLOR_FORMAT_T e_ref_chroma_fmt;

    /**
     * Frame skip mode
     */
    IVD_FRAME_SKIP_MODE_T e_pic_skip_mode;

    /**
     * Display or decode order dump of output
     */
    IVD_DISPLAY_FRAME_OUT_MODE_T e_pic_out_order;

    /**
     * Coding type of the picture that is decoded
     */
    IV_PICTURE_CODING_TYPE_T e_dec_pic_type;

    /**
     * Flag to signal if a frame was decoded in this call
     */
    WORD32 i4_pic_decoded;

    /**
     * Flag to signal if picture data is present in the current input bitstream
     */
    WORD32 i4_pic_present;

    /**
     * Flag to disable deblocking of a frame
     */
    WORD32 i4_disable_deblk_pic;

    /**
     * Flag to disable sao of a frame
     */
    WORD32 i4_disable_sao_pic;

    /**
     * Flag to use full pel MC
     */
    WORD32 i4_fullpel_inter_pred;
    /**
     * Flush mode
     */
    WORD32 i4_flush_mode;

    /**
     * Decode header mode
     */
    WORD32 i4_header_mode;

    /**
     * Header in slice mode
     */
    WORD32 i4_header_in_slice_mode;

    /**
     * Flag to signal sps done
     */
    WORD32 i4_sps_done;

    /**
     * Flag to signal pps done
     */
    WORD32 i4_pps_done;

    /**
     * To signal successful completion of init
     */
    WORD32 i4_init_done;

    /**
     * To signal that at least one picture was decoded
     */
    WORD32 i4_first_pic_done;

    /**
     * To signal error in slice
     */
    WORD32 i4_slice_error;

    /**
     * Reset flag - Codec is reset if this flag is set
     */
    WORD32 i4_reset_flag;

    /**
     * Number of pictures decoded till now
     */
    UWORD32 u4_pic_cnt;

    /**
     * Number of pictures displayed till now
     */
    UWORD32 u4_disp_cnt;

    /**
     * Current error code
     */
    WORD32 i4_error_code;

    /**
     * Pointer to input bitstream. This is incremented everytime a NAL is processed
     */
    UWORD8 *pu1_inp_bitsbuf;

    /**
     * Offset to first byte after the start code in current NAL
     */
    WORD32 i4_nal_ofst;

    /**
     * Length of the NAL unit including the emulation bytes
     */
    WORD32 i4_nal_len;

    /**
     * Number of emulation prevention bytes present in the current NAL
     */
    WORD32 i4_num_emln_bytes;

    /**
     * Number of bytes remaining in the input bitstream
     */
    /**
     * Decremented everytime a NAL is processed
     */
    WORD32 i4_bytes_remaining;

    /**
     * Pointer to bitstream after emulation prevention
     */
    UWORD8 *pu1_bitsbuf;

    /**
     * Pointer to static bitstream after emulation prevention
     * This is a fixed size buffer used initially till SPS is decoded
     */
    UWORD8 *pu1_bitsbuf_static;

    /**
     * Pointer to dynamic bitstream after emulation prevention
     * This is allocated after SPS is done, based on width and height
     */
    UWORD8 *pu1_bitsbuf_dynamic;

    /**
     * Size of intermediate bitstream buffer
     */
    UWORD32 u4_bitsbuf_size;

    /**
     * Size of intermediate static bitstream buffer
     */
    UWORD32 u4_bitsbuf_size_static;

    /**
     * Size of intermediate dynamic bitstream buffer
     */
    UWORD32 u4_bitsbuf_size_dynamic;

    /**
     * Pointer to hold TU data for a set of CTBs or a picture
     */
    void *pv_tu_data;

    /**
     * Process Job queue buffer base
     */
    void *pv_proc_jobq_buf;

    /**
     * Process Job Queue mem tab size
     */
    WORD32 i4_proc_jobq_buf_size;

    /** Parse status: one byte per CTB */
    UWORD8 *pu1_parse_map;

    /** Process status: one byte per CTB */
    UWORD8 *pu1_proc_map;
    /**
     * Current pictures intra mode map at 8x8 level
     */
    UWORD8 *pu1_pic_intra_flag;

    /**
     * No LPF buffer base
     */
    UWORD8 *pu1_pic_no_loop_filter_flag_base;

    /**
     * Current pictures loop filter flag map at 8x8 level
     */
    UWORD8 *pu1_pic_no_loop_filter_flag;
    /**
     * MV Bank buffer manager
     */
    void *pv_mv_buf_mgr;

    /**
     * Pointer to MV Buf structure array
     */
    void *ps_mv_buf;

    /** Holds the number of mv_buf_t structures allocated */
    WORD32 i4_max_dpb_size;

    /**
     * Base address for Motion Vector bank buffer
     */
    void *pv_mv_bank_buf_base;

    /**
     * MV Bank size allocated
     */
    WORD32 i4_total_mv_bank_size;

    /**
     * Picture buffer manager
     */
    void *pv_pic_buf_mgr;

    /**
     * Pointer to Pic Buf structure array
     */
    void *ps_pic_buf;

    /**
     * Base address for Picture buffer
     */
    void *pv_pic_buf_base;

    /**
     * Total pic buffer size allocated
     */
    WORD32 i4_total_pic_buf_size;

    /**
     * Current chroma buffer base - used for shared mode with 420p output
     */
    UWORD8 *pu1_cur_chroma_ref_buf;

    /**
     * Picture buffer manager
     */
    void *pv_disp_buf_mgr;

    /**
     * Current display buffer's buffer ID
     */
    WORD32 i4_disp_buf_id;

    /**
     * Current display buffer
     */
    pic_buf_t *ps_disp_buf;

    /**
     * Pointer to dpb manager structure
     */
    void *pv_dpb_mgr;

    /**
     * Scaling matrices for each PPS
     */
    WORD16 *pi2_scaling_mat;

    /**
     * Array containing Tile information for each PPS
     */
    tile_t *ps_tile;

    /**
     * Timestamp associated with the current display output
     */
    UWORD32 u4_ts;

    /**
     * Pointer to base of Video parameter set structure array
     */
    vps_t *ps_vps_base;

    /**
     * Pointer to base of Sequence parameter set structure array
     */
    sps_t *ps_sps_base;

    /**
     * Pointer to base of Picture parameter set structure array
     */
    pps_t *ps_pps_base;

    /**
     * Pointer to base of slice header structure array
     */
    slice_header_t *ps_slice_hdr_base;
    /**
     * Pointer to base of entry point offsets in a frame
     */
    WORD32 *pi4_entry_ofst;

    /**
     * Current offset in pi4_entry_ofst
     */
    WORD32 i4_cur_entry_ofst;

    /**
     *  Parsing context
     */
    parse_ctxt_t s_parse;

    /**
     * Processing context - One for each processing thread
     */
    process_ctxt_t as_process[MAX_PROCESS_THREADS];

    /**
     * Thread handle for each of the processing threads
     */
    void *apv_process_thread_handle[MAX_PROCESS_THREADS];

    /**
     * Thread created flag for each of the processing threads
     */
    WORD32 ai4_process_thread_created[MAX_PROCESS_THREADS];

    /**
     * Void pointer to process job context
     */
    void *pv_proc_jobq;

    /* Number of CTBs processed together for better instruction cache handling */
    WORD32 i4_proc_nctb;

    /**
     * Previous POC lsb
     */
    WORD32 i4_prev_poc_lsb;

    /**
     * Previous POC msb
     */
    WORD32 i4_prev_poc_msb;

    /**
     * Max POC lsb that has arrived till now
     */
    WORD32 i4_max_prev_poc_lsb;

    /** Context for format conversion */
    fmt_conv_t s_fmt_conv;

    /** Pointer to a structure describing output display buffer */
    ivd_out_bufdesc_t *ps_out_buffer;
    /**
     * Variable to store the next ctb count to compute pu idx
     */
    WORD32 i4_next_pu_ctb_cnt;

    /**
     * Variable to store the next ctb count to compute tu idx
     */
    WORD32 i4_next_tu_ctb_cnt;

    /**  Active SPS id - mainly to be used during codec initializations in shared mode */
    WORD32 i4_sps_id;

    /**  Number of ctbs to be decoded in one process call */
    UWORD32 u4_nctb;

    /** Flag to enable scheduling of format conversion jobs ahead of processing jobs */
    UWORD32 u4_enable_fmt_conv_ahead;

    /** Mask used to change MVs to full pel when configured to run in reduced complexity mode */
    WORD32 i4_mv_frac_mask;

    /** Memory holding tile indices */
    UWORD8 *pu1_tile_idx_base;

    /** Callback for aligned allocation */
    void *(*pf_aligned_alloc)(void *pv_mem_ctxt, WORD32 alignment, WORD32 size);

    /** Callback for aligned free */
    void (*pf_aligned_free)(void *pv_mem_ctxt, void *pv_buf);

    /** Memory context passed from application */
    void *pv_mem_ctxt;

    /** Base address of reference buffrers allocated */
    UWORD8 *pu1_ref_pic_buf_base;

    /** Flag to indicate if dynamic buffers are allocated */
    UWORD32 u4_allocate_dynamic_done;

    /** Flag to signal display order */
    IVD_DISPLAY_FRAME_OUT_MODE_T  e_frm_out_mode;

    /**  Funtion pointers for inter_pred leaf level functions */
    pf_inter_pred apf_inter_pred[22];

    /**  Funtion pointers for inter_pred_luma leaf level functions */
    pf_intra_pred apf_intra_pred_luma[11];

    /**  Funtion pointers for inter_pred_chroma leaf level functions */
    pf_intra_pred apf_intra_pred_chroma[11];

    /**  Funtion pointers for itrans_recon leaf level functions */
    pf_itrans_recon apf_itrans_recon[8];

    /**  Funtion pointers for recon leaf level functions */
    pf_recon apf_recon[8];

    /**  Funtion pointers for itrans_recon_dc leaf level functions */
    pf_itrans_recon_dc apf_itrans_recon_dc[2];

    /**  Funtion pointers for sao_luma leaf level functions */
    pf_sao_luma apf_sao_luma[4];

    /**  Funtion pointers for sao_chroma leaf level functions */
    pf_sao_chroma apf_sao_chroma[4];

    /**  Funtion pointers for all the leaf level functions */
    func_selector_t s_func_selector;
    /**  Processor architecture */
    IVD_ARCH_T e_processor_arch;
    /**  Processor soc */
    IVD_SOC_T e_processor_soc;

    /** Display buffer array - for shared mode */
    ivd_out_bufdesc_t s_disp_buffer[IVD_VIDDEC_MAX_IO_BUFFERS];

    /** Number of active display buffers - for shared mode */
    WORD32  i4_share_disp_buf_cnt;
};

#endif /* _IHEVCD_STRUCTS_H_ */
