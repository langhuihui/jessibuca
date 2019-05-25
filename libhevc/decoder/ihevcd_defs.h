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
*  ihevcd_defs.h
*
* @brief
*  Definitions used in the decoder
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

#ifndef _IHEVCD_DEFS_H_
#define _IHEVCD_DEFS_H_


/*****************************************************************************/
/* Width and height restrictions                                             */
/*****************************************************************************/
/**
 * Minimum width supported by codec
 */
#define MIN_WD   64

/**
 * Maximum width supported by codec
 */

#define MAX_WD   8448

/**
 * Minimum height supported by codec
 */
#define MIN_HT   64

/**
 * Maximum height supported by codec
 */

#define MAX_HT   4320

/*****************************************************************************/
/* Padding sizes                                                             */
/*****************************************************************************/
/**
 * Padding used for top of the frame
 */
#define PAD_TOP     80

/**
 * Padding used for bottom of the frame
 */
#define PAD_BOT     80

/**
 * Padding used at left of the frame
 */
#define PAD_LEFT    80

/**
 * Padding used at right of the frame
 */
#define PAD_RIGHT   80
/**
 * Padding for width
 */
#define PAD_WD      (PAD_LEFT + PAD_RIGHT)
/**
 * Padding for height
 */
#define PAD_HT      (PAD_TOP  + PAD_BOT)

/*****************************************************************************/
/* Number of frame restrictions                                              */
/*****************************************************************************/
/**
 *  Maximum number of reference buffers in DPB manager
 */
#define MAX_REF_CNT  32

/*****************************************************************************/
/* Num cores releated defs                                                   */
/*****************************************************************************/
/**
 *  Maximum number of cores
 */
#define MAX_NUM_CORES       8

/**
 *  Maximum number of threads for pixel processing
 */
#define MAX_PROCESS_THREADS MAX_NUM_CORES

/** If num_cores is greater than MV_PRED_NUM_CORES_THRESHOLD, then mv pred and
    boundary strength computation is done in process side instead of parse side.
    This ensures thread that does parsing does minimal work */
#define MV_PRED_NUM_CORES_THRESHOLD 2

/*****************************************************************************/
/* Profile and level restrictions                                            */
/*****************************************************************************/
/**
 * Max level supported by the codec
 */
#define MAX_LEVEL  IHEVC_LEVEL_62
/**
 * Min level supported by the codec
 */

#define MIN_LEVEL  IHEVC_LEVEL_10


/**
 * Maximum number of slice headers that are held in memory simultaneously
 * For single core implementation only 1 slice header is enough.
 * But for multi-core parsing thread needs to ensure that slice headers are
 * stored till the last CB in a slice is decoded.
 * Parsing thread has to wait till last CB of a slice is consumed before reusing
 * overwriting the slice header
 * MAX_SLICE_HDR_CNT is assumed to be a power of 2
 */

#define LOG2_MAX_SLICE_HDR_CNT 8
#define MAX_SLICE_HDR_CNT (1 << LOG2_MAX_SLICE_HDR_CNT)

/* Number of NOP instructions to wait before yielding in process thread */
#define PROC_NOP_CNT (8 * 128)


/** Max QP delta that can be signalled */
#define TU_MAX_QP_DELTA_ABS     5

/** Max QP delta context increment that can be used for CABAC context */
#define CTXT_MAX_QP_DELTA_ABS   1

/*
 * Flag whether to perform ilf at frame level or CTB level
 */
#define FRAME_ILF_PAD 0

#define MAX_NUM_CTBS_IN_FRAME (MAX_WD * MAX_HT / MIN_CTB_SIZE / MIN_CTB_SIZE)

/* Maximum slice segments allowed per frame in Level 6.2 */
#define MAX_SLICE_SEGMENTS_IN_FRAME 600

/**
 * Buffer allocated for ps_tu is re-used after RESET_TU_BUF_NCTB
 * Set this to MAX_NUM_CTBS_IN_FRAME to disabke reuse
 */
#define RESET_TU_BUF_NCTB MAX_NUM_CTBS_IN_FRAME
/**
 * Flag whether to shift the CTB for SAO
 */
#define SAO_PROCESS_SHIFT_CTB 1

/**
 * Minimum bistream buffer size
 */
#define MIN_BITSBUF_SIZE (1024 * 1024)
/**
 *****************************************************************************
 * Macro to compute total size required to hold on set of scaling matrices
 *****************************************************************************
 */
#define SCALING_MAT_SIZE(m_scaling_mat_size)                                 \
{                                                                            \
    m_scaling_mat_size = 6 * TRANS_SIZE_4 * TRANS_SIZE_4;                    \
    m_scaling_mat_size += 6 * TRANS_SIZE_8 * TRANS_SIZE_8;                   \
    m_scaling_mat_size += 6 * TRANS_SIZE_16 * TRANS_SIZE_16;                 \
    m_scaling_mat_size += 2 * TRANS_SIZE_32 * TRANS_SIZE_32;                 \
}

/**
 ***************************************************************************
 * Enum to hold various mem records being request
 ****************************************************************************
 */
enum
{
    /**
     * Codec Object at API level
     */
    MEM_REC_IV_OBJ,

    /**
     * Codec context
     */
    MEM_REC_CODEC,

    /**
     * Bitstream buffer which holds emulation prevention removed bytes
     */
    MEM_REC_BITSBUF,

    /**
     * Buffer to hold TU structures and coeff data
     */
    MEM_REC_TU_DATA,

    /**
     * Motion vector bank
     */
    MEM_REC_MVBANK,

    /**
     * Holds mem records passed to the codec.
     */
    MEM_REC_BACKUP,

    /**
     * Holds VPS
     */
    MEM_REC_VPS,

    /**
     * Holds SPS
     */
    MEM_REC_SPS,

    /**
     * Holds PPS
     */
    MEM_REC_PPS,

    /**
     * Holds Slice Headers
     */
    MEM_REC_SLICE_HDR,

    /**
     * Holds tile information such as start position, widths and heights
     */
    MEM_REC_TILE,

    /**
     * Holds entry point offsets for tiles and entropy sync points
     */
    MEM_REC_ENTRY_OFST,

    /**
     * Holds scaling matrices
     */
    MEM_REC_SCALING_MAT,

    /**
     * Holds one row skip_flag at 8x8 level used during parsing
     */
    MEM_REC_PARSE_SKIP_FLAG,

    /**
     * Holds one row ctb_tree_depth at 8x8 level used during parsing
     */
    MEM_REC_PARSE_CT_DEPTH,

    /**
     * Holds one row luma intra pred mode at 8x8 level used during parsing
     */
    MEM_REC_PARSE_INTRA_PRED_MODE,

    /**
     * Holds intra flag at 8x8 level for entire frame
     * This is kept at frame level so that processing thread also can use this
     * data during intra prediction and compute BS
     */
    MEM_REC_INTRA_FLAG,

    /**
     * Holds transquant bypass flag at 8x8 level for entire frame
     * This is kept at frame level so that processing thread also can use this
     */
    MEM_REC_TRANSQUANT_BYPASS_FLAG,

    /**
     * Holds thread handles
     */
    MEM_REC_THREAD_HANDLE,

    /**
     * Holds memory for Process JOB Queue
     */
    MEM_REC_PROC_JOBQ,

    /**
     * Contains status map indicating parse status per CTB basis
     */
    MEM_REC_PARSE_MAP,

    /**
     * Contains status map indicating processing status per CTB basis
     */
    MEM_REC_PROC_MAP,

    /**
     * Holds display buffer manager context
     */
    MEM_REC_DISP_MGR,

    /**
     * Holds dpb manager context
     */
    MEM_REC_DPB_MGR,

    /**
     * Holds top and left neighbors' pu_idx array w.r.t picture level pu array
     */
    MEM_REC_PIC_PU_IDX_NEIGHBOR,

    /**
     * Holds intermediate buffers needed during processing stage
     * Memory for process contexts is allocated in this memtab
     */
    MEM_REC_PROC_SCRATCH,

    /**
     * Holds intermediate buffers needed during SAO processing
     */
    MEM_REC_SAO_SCRATCH,

    /**
     * Holds buffers for vert_bs, horz_bs and QP (all frame level)
     */
    MEM_REC_BS_QP,

    /**
     * Contains slice map indicatating the slice index for each CTB
     */
    MEM_REC_TILE_IDX,

    /**
     * Holds buffers for array of SAO structures
     */
    MEM_REC_SAO,

    /**
     * Holds picture buffer manager context and array of pic_buf_ts
     * Also holds reference picture buffers in non-shared mode
     */
    MEM_REC_REF_PIC,



    /**
     * Place holder to compute number of memory records.
     */
    MEM_REC_CNT
    /* Do not add anything below */
};



#define DISABLE_DEBLOCK_INTERVAL 8
#define DISABLE_SAO_INTERVAL 8

/**
 ****************************************************************************
 * Disable deblock levels
 * Level 0 enables deblocking completely and level 4 disables completely
 * Other levels are intermediate values to control deblocking level
 ****************************************************************************
 */
enum
{
    /**
     * Enable deblocking completely
     */
    DISABLE_DEBLK_LEVEL_0,
    /**
     * Disable only within CTB edges - Not supported currently
     */
    DISABLE_DEBLK_LEVEL_1,

    /**
     * Enable deblocking once in DEBLOCK_INTERVAL number of pictures
     * and for I slices
     */
    DISABLE_DEBLK_LEVEL_2,

    /**
     * Enable deblocking only for I slices
     */
    DISABLE_DEBLK_LEVEL_3,

    /**
     * Disable deblocking completely
     */
    DISABLE_DEBLK_LEVEL_4
};

enum
{
    /**
     * Enable deblocking completely
     */
    DISABLE_SAO_LEVEL_0,
    /**
     * Disable only within CTB edges - Not supported currently
     */
    DISABLE_SAO_LEVEL_1,

    /**
     * Enable deblocking once in DEBLOCK_INTERVAL number of pictures
     * and for I slices
     */
    DISABLE_SAO_LEVEL_2,

    /**
     * Enable deblocking only for I slices
     */
    DISABLE_SAO_LEVEL_3,

    /**
     * Disable deblocking completely
     */
    DISABLE_SAO_LEVEL_4
};

/**
 ****************************************************************************
 * Number of buffers for I/O based on format
 ****************************************************************************
 */
#define MIN_IN_BUFS             1
#define MIN_OUT_BUFS_420        3
#define MIN_OUT_BUFS_422ILE     1
#define MIN_OUT_BUFS_RGB565     1
#define MIN_OUT_BUFS_RGBA8888   1
#define MIN_OUT_BUFS_420SP      2

/**
 ****************************************************************************
 * Definitions related to MV pred mv merge
 ****************************************************************************
 */
#define MAX_NUM_MERGE_CAND 5

#define MAX_NUM_MV_NBR 5

#define MAX_MVP_LIST_CAND 2
#define MAX_MVP_LIST_CAND_MEM  (MAX_MVP_LIST_CAND + 1)



#endif /*_IHEVCD_DEFS_H_*/
