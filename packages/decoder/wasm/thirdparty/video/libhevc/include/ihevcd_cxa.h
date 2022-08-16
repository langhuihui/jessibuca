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
*  ihevcd_cxa.h
*
* @brief
*  This file contains all the necessary structure and  enumeration
* definitions needed for the Application  Program Interface(API) of the
* Ittiam HEVC decoder  on Cortex Ax
*
* @author
*  Harish
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef __IHEVCD_CXA_H__
#define __IHEVCD_CXA_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "iv.h"
#include "ivd.h"


/*****************************************************************************/
/* Constant Macros                                                           */
/*****************************************************************************/
#define IVD_ERROR_MASK 0xFF

/*****************************************************************************/
/* Function Macros                                                           */
/*****************************************************************************/
#define IS_IVD_CONCEALMENT_APPLIED(x)       (x & (1 << IVD_APPLIEDCONCEALMENT))
#define IS_IVD_INSUFFICIENTDATA_ERROR(x)    (x & (1 << IVD_INSUFFICIENTDATA))
#define IS_IVD_CORRUPTEDDATA_ERROR(x)       (x & (1 << IVD_CORRUPTEDDATA))
#define IS_IVD_CORRUPTEDHEADER_ERROR(x)     (x & (1 << IVD_CORRUPTEDHEADER))
#define IS_IVD_UNSUPPORTEDINPUT_ERROR(x)    (x & (1 << IVD_UNSUPPORTEDINPUT))
#define IS_IVD_UNSUPPORTEDPARAM_ERROR(x)    (x & (1 << IVD_UNSUPPORTEDPARAM))
#define IS_IVD_FATAL_ERROR(x)               (x & (1 << IVD_FATALERROR))
#define IS_IVD_INVALID_BITSTREAM_ERROR(x)   (x & (1 << IVD_INVALID_BITSTREAM))
#define IS_IVD_INCOMPLETE_BITSTREAM_ERROR(x) (x & (1 << IVD_INCOMPLETE_BITSTREAM))


/*****************************************************************************/
/* API Function Prototype                                                    */
/*****************************************************************************/
IV_API_CALL_STATUS_T ihevcd_cxa_api_function(iv_obj_t *ps_handle,
                                             void *pv_api_ip,
                                             void *pv_api_op);

/*****************************************************************************/
/* Enums                                                                     */
/*****************************************************************************/
/* Codec Error codes for HEVC  Decoder                                       */


typedef enum {
    /**
     *  No error
     */
    IHEVCD_SUCCESS = 0,

    /**
     *  Codec calls done without successful init
     */
    IHEVCD_INIT_NOT_DONE                        = IVD_DUMMY_ELEMENT_FOR_CODEC_EXTENSIONS,


    IHEVCD_CXA_VID_HDR_DEC_NUM_FRM_BUF_NOT_SUFFICIENT,

    /**
     *  Unsupported level passed as an argument
     */
    IHEVCD_LEVEL_UNSUPPORTED,
    /**
     *  Unsupported number of reference pictures passed as an argument
     */
    IHEVCD_NUM_REF_UNSUPPORTED,
    /**
     *  Unsupported number of reorder pictures passed as an argument
     */
    IHEVCD_NUM_REORDER_UNSUPPORTED,
    /**
     *  Unsupported number of extra display pictures passed as an argument
     */
    IHEVCD_NUM_EXTRA_DISP_UNSUPPORTED,
    /**
     *  Invalid display stride requested.
     */
    IHEVCD_INVALID_DISP_STRD,

    /**
     * Reached end of sequence
     */
    IHEVCD_END_OF_SEQUENCE,

    /**
     * Width/height greater than max width and max height
     */
    IHEVCD_UNSUPPORTED_DIMENSIONS,

    /**
     *  Buffer size to hold version string is not sufficient
     *  Allocate more to hold version string
     */
    IHEVCD_CXA_VERS_BUF_INSUFFICIENT,
    /**
     * Stream chroma format other than YUV420
     */
    IHEVCD_UNSUPPORTED_CHROMA_FMT_IDC,

    /**
     * Generic failure
     */
    IHEVCD_FAIL                             = 0x7FFFFFFF


}IHEVCD_CXA_ERROR_CODES_T;

/*****************************************************************************/
/* Extended Structures                                                       */
/*****************************************************************************/


/*****************************************************************************/
/*  Delete Codec                                                             */
/*****************************************************************************/

typedef struct {
    ivd_delete_ip_t               s_ivd_delete_ip_t;
}ihevcd_cxa_delete_ip_t;


typedef struct {
    ivd_delete_op_t               s_ivd_delete_op_t;
}ihevcd_cxa_delete_op_t;

/*****************************************************************************/
/*   Initialize decoder                                                      */
/*****************************************************************************/

typedef struct {
    ivd_create_ip_t                         s_ivd_create_ip_t;
}ihevcd_cxa_create_ip_t;


typedef struct {
    ivd_create_op_t                         s_ivd_create_op_t;
}ihevcd_cxa_create_op_t;

/*****************************************************************************/
/*   Video Decode                                                            */
/*****************************************************************************/

typedef struct {

    /**
     * ivd_video_decode_ip_t
     */
    ivd_video_decode_ip_t                   s_ivd_video_decode_ip_t;
}ihevcd_cxa_video_decode_ip_t;


typedef struct {

    /**
     * ivd_video_decode_op_t
     */
    ivd_video_decode_op_t                   s_ivd_video_decode_op_t;
}ihevcd_cxa_video_decode_op_t;


/*****************************************************************************/
/*   Get Display Frame                                                       */
/*****************************************************************************/

typedef struct
{
    /**
     * ivd_get_display_frame_ip_t
     */
    ivd_get_display_frame_ip_t              s_ivd_get_display_frame_ip_t;
}ihevcd_cxa_get_display_frame_ip_t;


typedef struct
{
    /**
     * ivd_get_display_frame_op_t
     */
    ivd_get_display_frame_op_t              s_ivd_get_display_frame_op_t;
}ihevcd_cxa_get_display_frame_op_t;

/*****************************************************************************/
/*   Set Display Frame                                                       */
/*****************************************************************************/


typedef struct
{
    /**
     * ivd_set_display_frame_ip_t
     */
    ivd_set_display_frame_ip_t              s_ivd_set_display_frame_ip_t;
}ihevcd_cxa_set_display_frame_ip_t;


typedef struct
{
    /**
     * ivd_set_display_frame_op_t
     */
    ivd_set_display_frame_op_t              s_ivd_set_display_frame_op_t;
}ihevcd_cxa_set_display_frame_op_t;

/*****************************************************************************/
/*   Release Display Buffers                                                 */
/*****************************************************************************/


typedef struct
{
    /**
     * ivd_rel_display_frame_ip_t
     */

    ivd_rel_display_frame_ip_t                  s_ivd_rel_display_frame_ip_t;
}ihevcd_cxa_rel_display_frame_ip_t;


typedef struct
{
    /**
     * ivd_rel_display_frame_op_t
     */
    ivd_rel_display_frame_op_t                  s_ivd_rel_display_frame_op_t;
}ihevcd_cxa_rel_display_frame_op_t;


typedef enum
{
    /** Set number of cores/threads to be used */
    IHEVCD_CXA_CMD_CTL_SET_NUM_CORES         = IVD_CMD_CTL_CODEC_SUBCMD_START,

    /** Set processor details */
    IHEVCD_CXA_CMD_CTL_SET_PROCESSOR         = IVD_CMD_CTL_CODEC_SUBCMD_START + 0x001,

    /** Get display buffer dimensions */
    IHEVCD_CXA_CMD_CTL_GET_BUFFER_DIMENSIONS = IVD_CMD_CTL_CODEC_SUBCMD_START + 0x100,

    /** Get VUI parameters */
    IHEVCD_CXA_CMD_CTL_GET_VUI_PARAMS        = IVD_CMD_CTL_CODEC_SUBCMD_START + 0x101,

    /** Get SEI Mastering display color volume parameters */
    IHEVCD_CXA_CMD_CTL_GET_SEI_MASTERING_PARAMS   = IVD_CMD_CTL_CODEC_SUBCMD_START + 0x102,

    /** Enable/disable GPU, supported on select platforms */
    IHEVCD_CXA_CMD_CTL_GPU_ENABLE_DISABLE    = IVD_CMD_CTL_CODEC_SUBCMD_START + 0x200,

    /** Set degrade level */
    IHEVCD_CXA_CMD_CTL_DEGRADE               = IVD_CMD_CTL_CODEC_SUBCMD_START + 0x300
}IHEVCD_CXA_CMD_CTL_SUB_CMDS;
/*****************************************************************************/
/*   Video control  Flush                                                    */
/*****************************************************************************/


typedef struct {

    /**
     * ivd_ctl_flush_ip_t
     */
    ivd_ctl_flush_ip_t                      s_ivd_ctl_flush_ip_t;
}ihevcd_cxa_ctl_flush_ip_t;


typedef struct {

    /**
     * ivd_ctl_flush_op_t
     */
    ivd_ctl_flush_op_t                      s_ivd_ctl_flush_op_t;
}ihevcd_cxa_ctl_flush_op_t;

/*****************************************************************************/
/*   Video control reset                                                     */
/*****************************************************************************/


typedef struct {

    /**
     * ivd_ctl_reset_ip_t
     */
    ivd_ctl_reset_ip_t                      s_ivd_ctl_reset_ip_t;
}ihevcd_cxa_ctl_reset_ip_t;


typedef struct {

    /**
     * ivd_ctl_reset_op_t
     */
    ivd_ctl_reset_op_t                      s_ivd_ctl_reset_op_t;
}ihevcd_cxa_ctl_reset_op_t;


/*****************************************************************************/
/*   Video control  Set Params                                               */
/*****************************************************************************/


typedef struct {

    /**
     *  ivd_ctl_set_config_ip_t
     */
    ivd_ctl_set_config_ip_t             s_ivd_ctl_set_config_ip_t;
}ihevcd_cxa_ctl_set_config_ip_t;


typedef struct {

    /**
     * ivd_ctl_set_config_op_t
     */
    ivd_ctl_set_config_op_t             s_ivd_ctl_set_config_op_t;
}ihevcd_cxa_ctl_set_config_op_t;

/*****************************************************************************/
/*   Video control:Get Buf Info                                              */
/*****************************************************************************/


typedef struct {

    /**
     * ivd_ctl_getbufinfo_ip_t
     */
    ivd_ctl_getbufinfo_ip_t             s_ivd_ctl_getbufinfo_ip_t;
}ihevcd_cxa_ctl_getbufinfo_ip_t;



typedef struct {

    /**
     * ivd_ctl_getbufinfo_op_t
     */
    ivd_ctl_getbufinfo_op_t             s_ivd_ctl_getbufinfo_op_t;
}ihevcd_cxa_ctl_getbufinfo_op_t;


/*****************************************************************************/
/*   Video control:Getstatus Call                                            */
/*****************************************************************************/


typedef struct {

    /**
     * ivd_ctl_getstatus_ip_t
     */
    ivd_ctl_getstatus_ip_t                  s_ivd_ctl_getstatus_ip_t;
}ihevcd_cxa_ctl_getstatus_ip_t;



typedef struct {

    /**
     * ivd_ctl_getstatus_op_t
     */
    ivd_ctl_getstatus_op_t                  s_ivd_ctl_getstatus_op_t;

    /**
     * Height of the coding picture without cropping
     */
    UWORD32                  u4_coded_pic_ht;

    /**
     * Width of the coding picture without cropping
     */
    UWORD32                  u4_coded_pic_wd;
}ihevcd_cxa_ctl_getstatus_op_t;


/*****************************************************************************/
/*   Video control:Get Version Info                                          */
/*****************************************************************************/


typedef struct {

    /**
     *  ivd_ctl_getversioninfo_ip_t
     */
    ivd_ctl_getversioninfo_ip_t         s_ivd_ctl_getversioninfo_ip_t;
}ihevcd_cxa_ctl_getversioninfo_ip_t;



typedef struct {

    /**
     *  ivd_ctl_getversioninfo_op_t
     */
    ivd_ctl_getversioninfo_op_t         s_ivd_ctl_getversioninfo_op_t;
}ihevcd_cxa_ctl_getversioninfo_op_t;


typedef struct {

    /**
     * u4_size
     */
    UWORD32                                     u4_size;

    /**
     * cmd
     */
    IVD_API_COMMAND_TYPE_T                      e_cmd;

    /**
     * sub_cmd
     */
    IVD_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;

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

}ihevcd_cxa_ctl_degrade_ip_t;

typedef struct
{
    /**
     * u4_size
     */
    UWORD32                                     u4_size;

    /**
     * error_code
     */
    UWORD32                                     u4_error_code;
}ihevcd_cxa_ctl_degrade_op_t;

typedef struct
{

    /**
     * size
     */
    UWORD32                                     u4_size;

    /**
     * cmd
     */
    IVD_API_COMMAND_TYPE_T                      e_cmd;

    /**
     * sub_cmd
     */
    IVD_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;

    /**
     * num_cores
     */
    UWORD32                                     u4_num_cores;
}ihevcd_cxa_ctl_set_num_cores_ip_t;

typedef struct
{

    /**
     * size
     */
    UWORD32                                     u4_size;

    /**
     * error_code
     */
    UWORD32                                     u4_error_code;
}ihevcd_cxa_ctl_set_num_cores_op_t;

typedef struct
{
    /**
     * size
     */
    UWORD32                                     u4_size;
    /**
     * cmd
     */
    IVD_API_COMMAND_TYPE_T                      e_cmd;
    /**
     * sub cmd
     */
    IVD_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
    /**
     * Processor type
     */
    UWORD32                                     u4_arch;
    /**
     * SOC type
     */
    UWORD32                                     u4_soc;

    /**
     * num_cores
     */
    UWORD32                                     u4_num_cores;

}ihevcd_cxa_ctl_set_processor_ip_t;

typedef struct
{
    /**
     * size
     */
    UWORD32                                     u4_size;
    /**
     * error_code
     */
    UWORD32                                     u4_error_code;
}ihevcd_cxa_ctl_set_processor_op_t;

typedef struct
{

    /**
     * size
     */
    UWORD32                                     u4_size;

    /**
     * cmd
     */
    IVD_API_COMMAND_TYPE_T                      e_cmd;

    /**
     * sub cmd
     */
    IVD_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
}ihevcd_cxa_ctl_get_frame_dimensions_ip_t;


typedef struct {

    /**
     * size
     */
    UWORD32                                     u4_size;

    /**
     * error_code
     */
    UWORD32                                     u4_error_code;

    /**
     * x_offset[3]
     */
    UWORD32                                     u4_x_offset[3];

    /**
     * y_offset[3]
     */
    UWORD32                                     u4_y_offset[3];

    /**
     * disp_wd[3]
     */
    UWORD32                                     u4_disp_wd[3];

    /**
     * disp_ht[3]
     */
    UWORD32                                     u4_disp_ht[3];

    /**
     * buffer_wd[3]
     */
    UWORD32                                     u4_buffer_wd[3];

    /**
     * buffer_ht[3]
     */
    UWORD32                                     u4_buffer_ht[3];
}ihevcd_cxa_ctl_get_frame_dimensions_op_t;

typedef struct {
    UWORD32                                     u4_size;
    IVD_API_COMMAND_TYPE_T                      e_cmd;
    IVD_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
}ihevcd_cxa_ctl_get_vui_params_ip_t;

typedef struct {
    UWORD32                                     u4_size;
    UWORD32                                     u4_error_code;

    /**
    *  indicates the presence of aspect_ratio
    */
    UWORD8 u1_aspect_ratio_info_present_flag;

    /**
    *  specifies the aspect ratio of the luma samples
    */
    UWORD8 u1_aspect_ratio_idc;

    /**
    *  width of the luma samples. user dependent
    */
    UWORD16 u2_sar_width;

    /**
    *  hieght of the luma samples. user dependent
    */
    UWORD16 u2_sar_height;

    /**
    * if 1, specifies that the overscan_appropriate_flag is present
    * if 0, the preferred display method for the video signal is unspecified
    */
    UWORD8 u1_overscan_info_present_flag;

    /**
    * if 1,indicates that the cropped decoded pictures output
    * are suitable for display using overscan
    */
    UWORD8 u1_overscan_appropriate_flag;

    /**
    * if 1 specifies that video_format, video_full_range_flag and
    * colour_description_present_flag are present
    */
    UWORD8 u1_video_signal_type_present_flag;

    /**
    *
    */
    UWORD8 u1_video_format;

    /**
    * indicates the black level and range of the luma and chroma signals
    */
    UWORD8 u1_video_full_range_flag;

    /**
    * if 1,to 1 specifies that colour_primaries, transfer_characteristics
    * and matrix_coefficients are present
    */
    UWORD8 u1_colour_description_present_flag;

    /**
    * indicates the chromaticity coordinates of the source primaries
    */
    UWORD8 u1_colour_primaries;

    /**
    * indicates the opto-electronic transfer characteristic of the source picture
    */
    UWORD8 u1_transfer_characteristics;

    /**
    * the matrix coefficients used in deriving luma and chroma signals
    * from the green, blue, and red primaries
    */
    UWORD8 u1_matrix_coefficients;

    /**
    * if 1, specifies that chroma_sample_loc_type_top_field and
    * chroma_sample_loc_type_bottom_field are present
    */
    UWORD8 u1_chroma_loc_info_present_flag;

    /**
    * location of chroma samples
    */
    UWORD8 u1_chroma_sample_loc_type_top_field;

    UWORD8 u1_chroma_sample_loc_type_bottom_field;

    /**
    * if 1, indicates that the value of all decoded chroma samples is
    * equal to 1 << ( BitDepthC - 1 )
    */
    UWORD8 u1_neutral_chroma_indication_flag;

    /**
    *  1 indicates that the coded video sequence conveys pictures that represent fields
    *  0 indicates the pictures that represents field
    */
    UWORD8 u1_field_seq_flag;

    /**
    * specifies that picture timing SEI messages are present for every picture
    */
    UWORD8 u1_frame_field_info_present_flag;

    /**
    * 1 indicates that the default display window parameters follow next in the VUI
    */
    UWORD8 u1_default_display_window_flag;

    /**
    * specify the samples of the pictures in the coded video sequence
    * that are within the default display window,
    * in terms of a rectangular region specified in picture coordinates for display
    */
    UWORD32 u4_def_disp_win_left_offset;

    UWORD32 u4_def_disp_win_right_offset;

    UWORD32 u4_def_disp_win_top_offset;

    UWORD32 u4_def_disp_win_bottom_offset;

    /**
    *  to 1 specifies that the syntax structure hrd_parameters is present in the vui_parameters syntax structue
    */
    UWORD8 u1_vui_hrd_parameters_present_flag;

    /**
    *   Indicates the presence of the
    *   num_units_in_ticks, time_scale flag
    */
    UWORD8 u1_vui_timing_info_present_flag;

    /**
    *   Number of units that
    *   correspond to one increment of the
    *   clock. Indicates the  resolution
    */
    UWORD32 u4_vui_num_units_in_tick;

    /**
    *   The number of time units that pass in one second
    */
    UWORD32 u4_vui_time_scale;
    /**
    * if 1, indicates that the POC for each picture in the coded video sequence (cvs) (not the first picture), in decoding order,
    * is proportional to the output time of the picture relative to that of the first picture in the cvs
    */
    UWORD8 u1_poc_proportional_to_timing_flag;

    /**
    * num_ticks_poc_diff_one_minus1 plus 1 specifies the number of clock ticks
    * corresponding to a difference of poc values equal to 1
    */
    UWORD32 u4_num_ticks_poc_diff_one_minus1;

    /**
    * 1, specifies that the following cvs bitstream restriction parameters are present
    */
    UWORD8 u1_bitstream_restriction_flag;

    /**
    *  if 1, indicates that each pps that is active in the cvs has
    *  the same value of the tile syntax elements
    */
    UWORD8 u1_tiles_fixed_structure_flag;

    /**
    * if 0, indicates that no pel outside the pic boundaries and
    * no sub-pels derived using pels outside the pic boundaries is used for inter prediction
    */
    UWORD8 u1_motion_vectors_over_pic_boundaries_flag;

    /**
    * if 1, indicates
    * all P/B slices belonging to the same pic have an identical refpic list0,
    * all B slices that belong to the same picture have an identical refpic list1.
    */
    UWORD8 u1_restricted_ref_pic_lists_flag;

    /**
    *
    */
    UWORD8 u4_min_spatial_segmentation_idc;
    /**
    * Indicates a number of bytes not exceeded by the sum of the sizes of the VCL NAL units
    * associated with any coded picture
    */
    UWORD8 u1_max_bytes_per_pic_denom;

    /**
    *  Indicates an upper bound for the number of bits of coding_unit() data
    */
    UWORD8 u1_max_bits_per_mincu_denom;

    /**
    * Indicate the maximum absolute value of a decoded horizontal MV component
    * in quarter-pel luma units
    */
    UWORD8 u1_log2_max_mv_length_horizontal;

    /**
    * Indicate the maximum absolute value of a decoded vertical MV component
    * in quarter-pel luma units
    */
    UWORD8 u1_log2_max_mv_length_vertical;

    /**
     * HRD parameters
     */


    /**
    *   Indicates the presence of the
    *   num_units_in_ticks, time_scale flag
    */
    UWORD8 u1_timing_info_present_flag;

    /**
    *   Number of units that
    *   correspond to one increment of the
    *   clock. Indicates the  resolution
    */
    UWORD32 u4_num_units_in_tick;

    /**
    *   The number of time units that pass in one second
    */
    UWORD32 u4_time_scale;

    /**
    * Nal- hrd parameters flag
    */
    UWORD8 u1_nal_hrd_parameters_present_flag;

    /**
    * VCL- hrd parameters flag
    */
    UWORD8 u1_vcl_hrd_parameters_present_flag;

    /**
    * Indicates the presence of NAL-HRD params or VCL_HRD params
    * in the bitstream
    */
    UWORD8 u1_cpbdpb_delays_present_flag;

    /**
    * specifies that sub-picture level CPB removal delay parameters are
    * present in picture timing SEI messages
    */
    UWORD8 u1_sub_pic_cpb_params_present_flag;

    /**
    * specify the clock sub-tick
    * (the minimum interval of time that can be represented in the coded data when sub_pic_cpb_params_present_flag is equal to 1)
    */
    UWORD8 u1_tick_divisor_minus2;

    /**
    * specifies the length, in bits for the du cpb delay syntax in pt_sei
    */
    UWORD8 u1_du_cpb_removal_delay_increment_length_minus1;

    /**
    * Indicates presence of sub_pic_cpb_params in pic timing sei
    */
    UWORD8 u1_sub_pic_cpb_params_in_pic_timing_sei_flag;

    /**
     * Indicates dpb output delay for the du
     */
    UWORD8 u1_dpb_output_delay_du_length_minus1;

    /**
    * (together with bit_rate_value_minus1) specifies the
    * maximum input bit rate of the i-th CPB
    */
    UWORD8 u4_bit_rate_scale;

    /**
    * (together with cpb_size_du_value_minus1) specfies
    * CPB size of the i-th CPB when the CPB operates
    * at the access unit level
    */
    UWORD8 u4_cpb_size_scale;

    /**
    * (together with cpb_size_du_value_minus1) specfies
    * CPB size of the i-th CPB when the CPB operates
    * at the sub-picture level
    */
    UWORD8 u4_cpb_size_du_scale;


    /**
    * specifies the length, in bits for initial cpb delay (nal/vcl)sysntax in bp sei
    */
    UWORD8  u1_initial_cpb_removal_delay_length_minus1;

    /**
    * specifies the length, in bits for the au cpb delay syntax in pt_sei
    */
    UWORD8  u1_au_cpb_removal_delay_length_minus1;

    /**
    * specifies the length, in bits, of the pic_dpb_output_delay syntax element in the pt SEI message
    */
    UWORD8  u1_dpb_output_delay_length_minus1;

    /**
    * if 1, , for the highest temporal sub-layers, the temporal distance between the HRD output times
    *  of consecutive pictures in output order is constrained refer to Table E-6
    */
    UWORD8 au1_fixed_pic_rate_general_flag[6];

    UWORD8 au1_fixed_pic_rate_within_cvs_flag[6];

    /**
    * if 1, , for the highest temporal sub-layers, the temporal distance (in clock ticks) between the
    * element units that specify HRD output times of consecutive pictures in output order is constrained
    * refer to Table E-6
    */
    UWORD16 au2_elemental_duration_in_tc_minus1[6];

    /**
    * specifies the HRD operational mode
    */
    UWORD8 au1_low_delay_hrd_flag[6];

    /**
    * 1 specifies the number of alternative CPB specifications in the
    * bitstream of the cvs when HighestTid is equal to i
    */
    UWORD8 au1_cpb_cnt_minus1[6];
}ihevcd_cxa_ctl_get_vui_params_op_t;

typedef struct
{
    UWORD32                                     u4_size;
    IVD_API_COMMAND_TYPE_T                      e_cmd;
    IVD_CONTROL_API_COMMAND_TYPE_T              e_sub_cmd;
}ihevcd_cxa_ctl_get_sei_mastering_params_ip_t;

typedef struct
{
    UWORD32                                     u4_size;
    UWORD32                                     u4_error_code;

    /**
     * Array to store the display_primaries_x values
     */
    UWORD16 au2_display_primaries_x[3];

    /**
     * Array to store the display_primaries_y values
     */
    UWORD16 au2_display_primaries_y[3];

    /**
     * Variable to store the white point x value
     */
    UWORD16 u2_white_point_x;

    /**
     * Variable to store the white point y value
     */
    UWORD16 u2_white_point_y;

    /**
     * Variable to store the max display mastering luminance value
     */
    UWORD32 u4_max_display_mastering_luminance;

    /**
     * Variable to store the min display mastering luminance value
     */
    UWORD32 u4_min_display_mastering_luminance;

}ihevcd_cxa_ctl_get_sei_mastering_params_op_t;

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif
#endif /* __IHEVCD_CXA_H__ */
