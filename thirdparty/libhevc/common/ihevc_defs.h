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
*  ihevc_defs.h
*
* @brief
*  Definitions used in the codec
*
* @author
*  Ittiam
*
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_DEFS_H_
#define _IHEVC_DEFS_H_

/*****************************************************************************/
/* Profile and Levels                                                        */
/*****************************************************************************/
enum
{
    IHEVC_PROFILE_MAIN = 0,
};

enum
{
    IHEVC_TIER_MAIN,
    IHEVC_TIER_HIGH,
};


/* Slice type enums - Do not change the values */

enum
{
    BSLICE = 0,
    PSLICE = 1,
    ISLICE = 2,
};

/** Enum for Level : Multiplication by 30 as per spec is implemented as multiplied by 10 and then by 3
 * User will give level multiplied by 10 in the API
 */
enum
{
    IHEVC_LEVEL_10 = 10 * 3,
    IHEVC_LEVEL_20 = 20 * 3,
    IHEVC_LEVEL_21 = 21 * 3,
    IHEVC_LEVEL_30 = 30 * 3,
    IHEVC_LEVEL_31 = 31 * 3,
    IHEVC_LEVEL_40 = 40 * 3,
    IHEVC_LEVEL_41 = 41 * 3,
    IHEVC_LEVEL_50 = 50 * 3,
    IHEVC_LEVEL_51 = 51 * 3,
    IHEVC_LEVEL_52 = 52 * 3,
    IHEVC_LEVEL_60 = 60 * 3,
    IHEVC_LEVEL_61 = 61 * 3,
    IHEVC_LEVEL_62 = 62 * 3,
};


enum
{
    NAL_TRAIL_N     = 0,
    NAL_TRAIL_R,
    NAL_TSA_N,
    NAL_TSA_R,
    NAL_STSA_N,
    NAL_STSA_R,
    NAL_RADL_N,
    NAL_RADL_R,
    NAL_RASL_N,
    NAL_RASL_R,
    NAL_RSV_VCL_N10 = 10,
    NAL_RSV_VCL_N12 = 12,
    NAL_RSV_VCL_N14 = 14,
    NAL_RSV_VCL_R11 = 11,
    NAL_RSV_VCL_R13 = 13,
    NAL_RSV_VCL_R15 = 15,

    NAL_BLA_W_LP    = 16,
    NAL_BLA_W_DLP,
    NAL_BLA_N_LP,
    NAL_IDR_W_LP,
    NAL_IDR_N_LP,
    NAL_CRA,
    NAL_RSV_RAP_VCL22 = 22,
    NAL_RSV_RAP_VCL23 = 23,
    NAL_RSV_VCL24 = 24,
    NAL_RSV_VCL31 = 31,
    NAL_VPS       = 32,
    NAL_SPS,
    NAL_PPS,
    NAL_AUD,
    NAL_EOS,
    NAL_EOB,
    NAL_FD,
    NAL_PREFIX_SEI = 39,
    NAL_SUFFIX_SEI = 40,
    NAL_RSV_NVCL41 = 41,
    NAL_RSV_NVCL47 = 47 ,
    NAL_UNSPEC48   = 48 ,
    NAL_UNSPEC63   = 49,
};

enum
{
    CHROMA_FMT_IDC_MONOCHROME   = 0,
    CHROMA_FMT_IDC_YUV420       = 1,
    CHROMA_FMT_IDC_YUV422       = 2,
    CHROMA_FMT_IDC_YUV444       = 3,
    CHROMA_FMT_IDC_YUV444_PLANES = 4,
};

/* Pred Modes */
/* Do not change enum values */
enum
{
    PRED_MODE_INTER = 0,
    PRED_MODE_INTRA = 1,
    PRED_MODE_SKIP  = 2
};

/* Partition Modes */
/* Do not change enum values */
enum
{
    PART_2Nx2N  = 0,
    PART_2NxN   = 1,
    PART_Nx2N   = 2,
    PART_NxN    = 3,
    PART_2NxnU  = 4,
    PART_2NxnD  = 5,
    PART_nLx2N  = 6,
    PART_nRx2N  = 7
};

/* Prediction list */
/* Do not change enum values */
enum
{
    PRED_L0 = 0,
    PRED_L1 = 1,
    PRED_BI = 2
};

/**
 * Scan types
 */
enum
{
    SCAN_DIAG_UPRIGHT,
    SCAN_HORZ,
    SCAN_VERT
};

/**
 * VUI aspect ratio indicator
 */
enum
{
    SAR_UNUSED = 0,
    SAR_1_1 = 1,
    SAR_12_11,
    SAR_10_11,
    SAR_16_11,
    SAR_40_33,
    SAR_24_11,
    SAR_20_11,
    SAR_32_11,
    SAR_80_33,
    SAR_18_11,
    SAR_15_11,
    SAR_64_33,
    SAR_160_99,
    SAR_4_3,
    SAR_3_2,
    SAR_2_1,
    EXTENDED_SAR = 255
};

enum
{
    VID_FMT_COMPONENT = 0,
    VID_FMT_PAL,
    VID_FMT_NTSC,
    VID_FMT_SECAM,
    VID_FMT_MAC,
    VID_FMT_UNSPECIFIED
};

typedef enum {
    USER_DATA_BAR_DATA       = 0,
    USER_DATA_CC_DATA        = 1,
    USER_DATA_AFD_DATA       = 2,

    // do not add anything below
    USER_DATA_MAX
} USER_DATA_SEI_TYPE_T;

/**
 ****************************************************************************
 * SEI payload type
 ****************************************************************************
 */
enum
{
    SEI_BUFFERING_PERIOD                     = 0,
    SEI_PICTURE_TIMING                       = 1,
    SEI_PAN_SCAN_RECT                        = 2,
    SEI_FILLER_PAYLOAD                       = 3,
    SEI_USER_DATA_REGISTERED_ITU_T_T35       = 4,
    SEI_USER_DATA_UNREGISTERED               = 5,
    SEI_RECOVERY_POINT                       = 6,
    SEI_SCENE_INFO                           = 9,
    SEI_FULL_FRAME_SNAPSHOT                  = 15,
    SEI_PROGRESSIVE_REFINEMENT_SEGMENT_START = 16,
    SEI_PROGRESSIVE_REFINEMENT_SEGMENT_END   = 17,
    SEI_FILM_GRAIN_CHARACTERISTICS           = 19,
    SEI_POST_FILTER_HINT                     = 22,
    SEI_TONE_MAPPING_INFO                    = 23,
    SEI_FRAME_PACKING                        = 45,
    SEI_DISPLAY_ORIENTATION                  = 47,
    SEI_SOP_DESCRIPTION                      = 128,
    SEI_ACTIVE_PARAMETER_SETS                = 129,
    SEI_DECODING_UNIT_INFO                   = 130,
    SEI_TEMPORAL_LEVEL0_INDEX                = 131,
    SEI_DECODED_PICTURE_HASH                 = 132,
    SEI_SCALABLE_NESTING                     = 133,
    SEI_REGION_REFRESH_INFO                  = 134,
    SEI_NO_DISPLAY                           = 135,
    SEI_TIME_CODE                            = 136,
    SEI_MASTERING_DISPLAY_COLOUR_VOLUME      = 137,
    SEI_SEGM_RECT_FRAME_PACKING              = 138,
    SEI_TEMP_MOTION_CONSTRAINED_TILE_SETS    = 139,
    SEI_CHROMA_SAMPLING_FILTER_HINT          = 140,
    SEI_KNEE_FUNCTION_INFO                   = 141
};


#define BIT_DEPTH           8
#define BIT_DEPTH_LUMA      BIT_DEPTH
#define BIT_DEPTH_CHROMA    BIT_DEPTH
/*****************************************************************************/
/* Profile tier level defs                                                   */
/*****************************************************************************/
#define MAX_PROFILE_COMPATBLTY 32

/*****************************************************************************/
/* Reference frame defs                                                      */
/*****************************************************************************/
/* Maximum DPB size */
#define MAX_DPB_SIZE 16


/*****************************************************************************/
/* VPS restrictions                                                          */
/*****************************************************************************/

/* Number of VPS allowed  in Main Profile */
#define MAX_VPS_CNT         16

/* Max sub layers in VPS */
#define VPS_MAX_SUB_LAYERS  7

/* Max number of HRD parameters */
#define VPS_MAX_HRD_PARAMS  2

/* Maximum number of operation point layers */
#define VPS_MAX_OP_LAYERS 2


/*****************************************************************************/
/* Tile restrictions                                                         */
/*****************************************************************************/
/* Minimum tile width in Main Profile */
#define MIN_TILE_WD  MAX_CTB_SIZE

/* Minimum tile height in Main Profile */
#define MIN_TILE_HT  MAX_CTB_SIZE

/*****************************************************************************/
/* SPS restrictions                                                          */
/*****************************************************************************/

/* Number of SPS allowed in Main Profile*/
/* An extra buffer is allocated to write the parsed data
 * It is copied to the appropriate location later */
#define MAX_SPS_CNT         (16 + 1)

/* Max sub layers in PPS */
#define SPS_MAX_SUB_LAYERS  7

/* Maximum long term reference pics */
#define MAX_LTREF_PICS_SPS 32

#define MAX_STREF_PICS_SPS 64

/*****************************************************************************/
/* PPS restrictions                                                          */
/*****************************************************************************/

/* Number of PPS allowed in Main Profile */
/* An extra buffer is allocated to write the parsed data
 * It is copied to the appropriate location later */
#define MAX_PPS_CNT         (64 + 1)

/*****************************************************************************/
/* Macro definitions for sizes of CTB, PU, TU, CU                            */
/*****************************************************************************/

/* CTB Size Range */
#define MAX_CTB_SIZE        64
#define MIN_CTB_SIZE        16

/* TU Size Range */
#define MAX_TU_SIZE         32
#define MIN_TU_SIZE         4

/* Max Transform Size */
#define MAX_TRANS_SIZE      (MAX_TU_SIZE*MAX_TU_SIZE)

/* PU Size Range */
#define MAX_PU_SIZE         64
#define MIN_PU_SIZE         4

/* CU Size Range */
#define MAX_CU_SIZE         64
#define MIN_CU_SIZE         8


/* Number of max TU in a CTB row */
#define MAX_TU_IN_CTB_ROW   ((MAX_CTB_SIZE / MIN_TU_SIZE))

/* Number of max TU in a CTB row */
#define MAX_CU_IN_CTB_ROW   ((MAX_CTB_SIZE / MIN_CU_SIZE))

/* Number of max PU in a CTb row */
#define MAX_PU_IN_CTB_ROW   ((MAX_CTB_SIZE / MIN_PU_SIZE))

/* Number of max CU in a CTB */
#define MAX_CU_IN_CTB       ((MAX_CTB_SIZE / MIN_CU_SIZE) * \
                             (MAX_CTB_SIZE / MIN_CU_SIZE))

/* Number of max PU in a CTB */
/*****************************************************************************/
/* Note though for 64 x 64 CTB, Max PU in CTB is 128, in order to store      */
/*  intra pred info, 256 entries are needed                                  */
/*****************************************************************************/
#define MAX_PU_IN_CTB       ((MAX_CTB_SIZE / MIN_PU_SIZE) * \
                             (MAX_CTB_SIZE / MIN_PU_SIZE))

/* Number of max TU in a CTB */
#define MAX_TU_IN_CTB       ((MAX_CTB_SIZE / MIN_TU_SIZE) * \
                             (MAX_CTB_SIZE / MIN_TU_SIZE))



/**
 * Maximum transform depths
 */
#define MAX_TRAFO_DEPTH 5




/* Max number of deblocking edges */
#define MAX_VERT_DEBLK_EDGES ((MAX_CTB_SIZE/8) * (MAX_CTB_SIZE/4))
#define MAX_HORZ_DEBLK_EDGES ((MAX_CTB_SIZE/4) * (MAX_CTB_SIZE/8))

/* Qp can not change below 8x8 level */
#define MAX_DEBLK_QP_CNT     ((MAX_CTB_SIZE/8) * (MAX_CTB_SIZE/8))

/*****************************************************************************/
/* Parsing related macros                                                    */
/*****************************************************************************/
#define SUBBLK_COEFF_CNT    16

/* Quant and Trans defs */

/*****************************************************************************/
/* Sizes for Transform functions                                             */
/*****************************************************************************/
#define TRANS_SIZE_4   4
#define TRANS_SIZE_8   8
#define TRANS_SIZE_16 16
#define TRANS_SIZE_32 32


#define IT_SHIFT_STAGE_1 7
#define IT_SHIFT_STAGE_2 12

/**
 * @brief  Maximum transform dynamic range (excluding sign bit)
 */
#define MAX_TR_DYNAMIC_RANGE  15

/**
 * @brief  Q(QP%6) * IQ(QP%6) = 2^20
 */
#define QUANT_IQUANT_SHIFT    20

/**
 * @brief Q factor for Qp%6 multiplication
 */
#define QUANT_SHIFT           14

/**
 * @brief Q shift factor for flat rescale matrix weights
 */
#define FLAT_RESCALE_MAT_Q_SHIFT    11

/**
 * @brief  Scaling matrix is represented in Q15 format
 */
#define SCALING_Q_SHIFT       15

/**
 * @brief  rounding factor for quantization represented in Q9 format
 */
#define QUANT_ROUND_FACTOR_Q   9

/**
 * @brief  Minimum qp supported in HEVC spec
 */
#define MIN_HEVC_QP 0

/**
 * @brief  Maximum qp supported in HEVC spec
 */
#define MAX_HEVC_QP 51  //FOR MAIN Branch Encoder

#define MAX_HEVC_QP_10bit 63  //FOR HBD Branch Encoder

#define MAX_HEVC_QP_12bit 75  //FOR HBD Branch Encoder


/**
 * @brief  Total number of transform sizes
 * used for sizeID while getting scale matrix
 */
#define NUM_UNIQUE_TRANS_SIZE 4

/*****************************************************************************/
/* Number of scaling matrices for each transform size                        */
/*****************************************************************************/
#define SCALE_MAT_CNT_TRANS_SIZE_4    6
#define SCALE_MAT_CNT_TRANS_SIZE_8    6
#define SCALE_MAT_CNT_TRANS_SIZE_16   6
#define SCALE_MAT_CNT_TRANS_SIZE_32   2

/* Maximum number of scale matrices for a given transform size */
#define SCALE_MAT_CNT_MAX_PER_TRANS_SIZE 6

/* Total number of scale matrices */
#define TOTAL_SCALE_MAT_COUNT   (SCALE_MAT_CNT_TRANS_SIZE_4     + \
                                 SCALE_MAT_CNT_TRANS_SIZE_8     + \
                                 SCALE_MAT_CNT_TRANS_SIZE_16    + \
                                 SCALE_MAT_CNT_TRANS_SIZE_32)


/*****************************************************************************/
/* Intra pred Macros                                                         */
/*****************************************************************************/
/** Planar Intra prediction mode */
#define INTRA_PLANAR             0

/** DC Intra prediction mode */
#define INTRA_DC                 1

/** Gives angular mode for intra prediction */
#define INTRA_ANGULAR(x) (x)

/** Following is used to signal no intra prediction in case of pcm blocks
 */
#define INTRA_PRED_NONE  63


/** Following is used to signal no intra prediction is needed for first three
 * 4x4 luma blocks in case of 4x4 TU sizes
 * Also used in pcm cases
 */
#define INTRA_PRED_CHROMA_IDX_NONE  7

#define MAX_NUM_CLOCK_TS    3
#define MAX_USERDATA_PAYLOAD 256

#define MAX_CPB_CNT 32

#endif /*__IHEVC_DEFS_H_*/
