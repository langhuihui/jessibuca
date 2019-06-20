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
 *  ihevc_structs.h
 *
 * @brief
 *  Structure definitions used in the code
 *
 * @author
 *  Ittiam
 *
 * @par List of Functions:
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

#ifndef _IHEVC_STRUCTS_H_
#define _IHEVC_STRUCTS_H_

/**
 * Buffering Period SEI parameters Info
 */
typedef struct
{
    /**
     * specifies SPS Id active for the coded picture assosiated
     * with the bp message.
     */
    UWORD8 u1_bp_seq_parameter_set_id;

    /**
     * Derived from Hrd parameters
     */
    UWORD8 u1_sub_pic_cpb_params_present_flag;

    /**
     * specifies the presence of the initial_alt_cpb_removal_delay[ i ]
     * and initial_alt_cpb_removal_offset[ i ] syntax elements
     */
    UWORD8 u1_rap_cpb_params_present_flag;

    /**
     * cbp removal delay used in buffering period SEI
     */
    UWORD32 u4_cpb_delay_offset;

    /**
     * dbp removal delay used in buffering period SEI
     */
    UWORD32 u4_dpb_delay_offset;

    /**
     * concatanation flag
     */
    UWORD8 u1_concatenation_flag;

    /**
     * delata cbp removal delay
     */
    UWORD32 u4_au_cpb_removal_delay_delta_minus1;

    /**
     * specify the default initial CPB removal delays, respectively,
     * for the CPB when the NAL HRD parameters are in use
     */
    UWORD32 au4_nal_initial_cpb_removal_delay[MAX_CPB_CNT];

    /**
     * specify the alternate initial CPB removal delays, respectively,
     * for the CPB when the NAL HRD parameters are in use
     */
    UWORD32 au4_nal_initial_alt_cpb_removal_delay[MAX_CPB_CNT];

    /**
     * specify the initial CPB removal delay offset, respectively,
     * for the CPB when the NAL HRD parameters are in use
     */
    UWORD32 au4_nal_initial_cpb_removal_delay_offset[MAX_CPB_CNT];

    /**
     * specify the alternate initial CPB removal delays offsets, respectively,
     * for the CPB when the NAL HRD parameters are in use
     */
    UWORD32 au4_nal_initial_alt_cpb_removal_delay_offset[MAX_CPB_CNT];

    /**
     * specify the default initial CPB removal delays, respectively,
     * for the CPB when the VCL HRD parameters are in use
     */
    UWORD32 au4_vcl_initial_cpb_removal_delay[MAX_CPB_CNT];

    /**
     * specify the initial alt CPB removal delays , respectively,
     * for the CPB when the VCL HRD parameters are in use
     */
    UWORD32 au4_vcl_initial_alt_cpb_removal_delay[MAX_CPB_CNT];

    /**
     * specify the initial CPB removal delay offset, respectively,
     * for the CPB when the VCL HRD parameters are in use
     */
    UWORD32 au4_vcl_initial_cpb_removal_delay_offset[MAX_CPB_CNT];

    /**
     * specify the alternate initial CPB removal delays offsets, respectively,
     * for the CPB when the VCL HRD parameters are in use
     */
    UWORD32 au4_vcl_initial_alt_cpb_removal_delay_offset[MAX_CPB_CNT];

    /**
     * Inital CPB removal delay
     */
    UWORD32 u4_initial_cpb_removal_delay_length;

    /**
     * CPB cnt for corr. sublayer
     */
    UWORD32 u4_cpb_cnt;

    /**
     * VBV buffer size used in buffering period SEI
     */
    UWORD32 u4_buffer_size_sei;

    /**
     * Encoder buffer fullness  used in buffering period SEI
     */
    UWORD32 u4_dbf_sei;

    /**
     * target bitrate used in buffering period SEI
     */
    UWORD32 u4_target_bit_rate_sei;

} buf_period_sei_params_t;

/**
 * Picture Timing SEI parameters Info
 */
typedef struct
{
    /**
     * derived from vui parameters
     */
    UWORD8 u1_frame_field_info_present_flag;

    /**
     * indicates whether a picture should be displayed as a
     * frame or as one or more fields
     */
    UWORD32 u4_pic_struct;

    UWORD32 u4_source_scan_type;

    /**
     * if 1, indicates if the current pic is a duplicte pic in output order
     */
    UWORD8 u1_duplicate_flag;

    /**
     * specifies the number clock ticks between the nominal CPB removal time
     * au associated with the pt SEI message and
     * the preceding au in decoding order that contained a bp SEI message
     */
    UWORD32 u4_au_cpb_removal_delay_minus1;

    /**
     * compute the DPB output time of the picture
     */
    UWORD32 u4_pic_dpb_output_delay;

    UWORD32 u4_pic_dpb_output_du_delay;

    /**
     * specifies the number of decoding units in the access unit
     * the picture timing SEI message is associated with
     */
    UWORD32 u4_num_decoding_units_minus1;

    /**
     * if 1 specifies that the du_common_cpb_removal_delay_increment_minus1 is present
     */
    UWORD8 u1_du_common_cpb_removal_delay_flag;

    /**
     * specifies the duration, in units of clock sub-ticks,
     * between the nominal CPB removal times of any two consecutive decoding units
     * in decoding order in the access unit associated with the pt_SEI message
     */
    UWORD32 u4_du_common_cpb_removal_delay_increment_minus1; //same as u4_du_cpb_removal_delay_increment_minus1

    /**
     * specifies the number of NAL units in the decoding unit of the access unit
     * the picture timing SEI message is associated with.
     * range from 0 to (pic size in ctby - 1)
     */
    UWORD32 au4_num_nalus_in_du_minus1[4320 / MIN_CTB_SIZE];

    /**
     * specifies the duration, in units of clock sub-ticks,
     * between the nominal CPB removal times of the ( i + 1 )-th decoding unit and the i-th decoding unit,
     * in decoding order, in the access unit associated with the pt_SEI message
     */
    UWORD32 au4_du_cpb_removal_delay_increment_minus1[4320 / MIN_CTB_SIZE];

} pic_timing_sei_params_t;

/**
 * Structure to hold Recovery point SEI parameters Info
 */
typedef struct
{
    /**
     * specifies the recovery point of output pictures in output order
     */
    WORD32 i4_recovery_poc_cnt;

    UWORD8 u1_exact_match_flag;

    /**
     * indicates the presence or absence of a broken link in the NAL unit
     * stream at the location of the recovery point SEI message
     */

    UWORD8 u1_broken_link_flag;

} recovery_point_sei_params_t;

/**
 * Structure to hold Mastering Display Colour Volume SEI
 */
typedef struct
{
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

} mastering_dis_col_vol_sei_params_t;

/**
 * Structure to hold active parameter parameter set SEI parameters Info
 */
typedef struct
{
    /*
    * active vps id
    */

    UWORD8 u1_active_video_parameter_set_id;

    /*
     * default set to zero.
     */
    UWORD8 u1_self_contained_cvs_flag;

    UWORD8 u1_no_parameter_set_update_flag;

    UWORD8 u1_num_sps_ids_minus1;

    /*
     * active sps id
     */
    UWORD8 au1_active_seq_parameter_set_id[15];

    UWORD32 au4_layer_sps_idx[64];

} active_parameter_set_sei_param_t;

/**
 * Structure to hold SEI Hash values
 */
typedef struct
{
    /*
     * SEI Hash values for each color component
     */
    UWORD8 au1_sei_hash[3][16];

} hash_sei_param_t;

/**
 * Structure to hold user data registered SEI param Info
 */
typedef struct
{
    /**
     * Contains country code by Annex A of Recommendation ITU-T T.35
     */
    UWORD8 u1_itu_t_t35_country_code;

    /**
     * Contains country code by Annex B of Recommendation ITU-T T.35
     */
    UWORD8 u1_itu_t_t35_country_code_extension_byte;

    /**
     * Contains data registered as specified in Recommendation ITU-T T.35
     */
    UWORD8 u1_itu_t_t35_payload_byte[MAX_USERDATA_PAYLOAD];

    /**
     * Valid payload size present in this buffer
     */
    WORD32 i4_valid_payload_size;

    /**
     * Total payload size incase payloadSize > IHEVCD_MAX_USERDATA_PAYLOAD
     */
    WORD32 i4_payload_size;
} user_data_registered_itu_t_t35_t;

/**
 * Structure to hold time code SEI param info
 */
typedef struct
{
    /**
     * Number of sets of clock timestamp syntax elements present for the current picture
     */
    UWORD8 u1_num_clock_ts;

    /**
     * Indicates presenc of associated set of clock timestamps
     */
    UWORD8 au1_clock_timestamp_flag[MAX_NUM_CLOCK_TS];

    /**
     * Used in calculating clockTimestamp[i]
     */
    UWORD8 au1_units_field_based_flag[MAX_NUM_CLOCK_TS];

    /**
     * Specifies the method of dropping values of the n_frames[i] syntax element
     */
    UWORD8 au1_counting_type[MAX_NUM_CLOCK_TS];

    /**
     * Specifies that the n_frames[i] syntax element is followed by seconds_value[i],
     * minutes_value[i] and hours_value[i]
     */
    UWORD8 au1_full_timestamp_flag[MAX_NUM_CLOCK_TS];

    /**
     * Indicates the discontinuity in clockTimestamp
     */
    UWORD8 au1_discontinuity_flag[MAX_NUM_CLOCK_TS];

    /**
     * Specifies the skipping of one or more values of n_frames[i]
     */
    UWORD8 au1_cnt_dropped_flag[MAX_NUM_CLOCK_TS];

    /**
     * Specifies the value of nFrames used to compute clockTimestamp[i]
     */
    UWORD16 au2_n_frames[MAX_NUM_CLOCK_TS];

    /**
     * Specifies the presence of seconds_value[i] and minutes_flag[i]
     */
    UWORD8 au1_seconds_flag[MAX_NUM_CLOCK_TS];

    /**
     * Specifies the presence of minutes_value[i] and hours_flag[i]
     */
    UWORD8 au1_minutes_flag[MAX_NUM_CLOCK_TS];

    /**
     * Specifies the presence of hours_value[i]
     */
    UWORD8 au1_hours_flag[MAX_NUM_CLOCK_TS];

    /**
     * Specifies the value of sS used to compute clockTimestamp[i]
     */
    UWORD8 au1_seconds_value[MAX_NUM_CLOCK_TS];

    /**
     * Specifies the value of mM used to compute clockTimestamp[i]
     */
    UWORD8 au1_minutes_value[MAX_NUM_CLOCK_TS];

    /**
     * Specifies the value of hH used to compute clockTimestamp[i]
     */
    UWORD8 au1_hours_value[MAX_NUM_CLOCK_TS];

    /**
     * Specifies the length in bits of the time_offset_value[i]
     */
    UWORD8 au1_time_offset_length[MAX_NUM_CLOCK_TS];

    /**
     * pecifies the value of tOffset used to compute clockTimestamp[i]
     */
    UWORD8 au1_time_offset_value[MAX_NUM_CLOCK_TS];

} time_code_t;

/**
 * Structure to hold SEI parameters Info
 */
typedef struct
{

    WORD8 i1_sei_parameters_present_flag;

    WORD8 i1_aud_present_flag;

    WORD8 i1_buf_period_params_present_flag;

    WORD8 i1_pic_timing_params_present_flag;

    WORD8 i1_recovery_point_params_present_flag;

    WORD8 i1_active_parameter_set;

    WORD8 i4_sei_mastering_disp_colour_vol_params_present_flags;

    /* Enable/Disable SEI Hash on the Decoded picture & Hash type */
    /* < 3 : Checksum, 2 : CRC, 1 : MD5, 0 : disable >            */
    /* Other values are not supported                             */
    WORD8 i1_decoded_pic_hash_sei_flag;

    /* number of user data e.g. CC data, BAR data, AFD data etc */
    WORD32 i4_sei_user_data_cnt;

    WORD8 i1_user_data_registered_present_flag;

    WORD8 i1_time_code_present_flag;

    buf_period_sei_params_t s_buf_period_sei_params;

    pic_timing_sei_params_t s_pic_timing_sei_params;

    recovery_point_sei_params_t s_recovery_point_params;

    active_parameter_set_sei_param_t s_active_parameter_set_sei_params;

    hash_sei_param_t s_hash_sei_params;

    mastering_dis_col_vol_sei_params_t s_mastering_dis_col_vol_sei_params;

    user_data_registered_itu_t_t35_t as_user_data_registered_itu_t_t35[USER_DATA_MAX];

    time_code_t s_time_code;
} sei_params_t;

/**
 * Sub-layer HRD parameters Info
 */
typedef struct
{
    /**
     * (together with bit_rate_scale) specifies the
     * maximum input bit rate for the i-th CPB
     */
    UWORD32 au4_bit_rate_value_minus1[MAX_CPB_CNT];
    /**
     * together with cpb_size_scale to specify the
     * CPB size when the CPB operates at the access unit level.
     */
    UWORD32 au4_cpb_size_value_minus1[MAX_CPB_CNT];

    /**
     * together with cpb_size_du_scale to specify the CPB size
     * when the CPB operates at sub-picture level
     */
    UWORD32 au4_cpb_size_du_value_minus1[MAX_CPB_CNT];

    /**
     * specifies the maximum input bit rate for the i-th CPB when the CPB
     * operates at the sub-picture level. bit_rate_du_value_minus1[ i ]
     * shall be in the range of 0 to 2^32 - 2
     */
    UWORD32 au4_bit_rate_du_value_minus1[MAX_CPB_CNT];

    /**
     * if 1, specifies that the HSS operates in a constant bit rate (CBR) mode
     * if 0, specifies that the HSS operates in a intermittent bit rate (CBR) mode
     */
    UWORD8 au1_cbr_flag[32];

} sub_lyr_hrd_params_t;

/**
 * HRD parameters Info
 */
typedef struct
{
    /**
     * Indicates the presence of the
     * num_units_in_ticks, time_scale flag
     */
    UWORD8 u1_timing_info_present_flag;

    /**
     * Number of units that
     * correspond to one increment of the
     * clock. Indicates the  resolution
     */
    UWORD32 u4_num_units_in_tick;

    /**
     * The number of time units that pass in one second
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
     * specifies the length, in bits, of the pic_dpb_output_du_delay syntax
     * element in the picture timing SEI message and the
     * pic_spt_dpb_output_du_delay syntax element in the decoding unit
     * information SEI message
     */
    UWORD8 u1_dpb_output_delay_du_length_minus1;

    /**
     * (together with bit_rate_value_minus1) specifies the
     * maximum input bit rate of the i-th CPB
     */
    UWORD32 u4_bit_rate_scale;

    /**
     * (together with cpb_size_du_value_minus1) specfies
     * CPB size of the i-th CPB when the CPB operates
     * at the access unit level
     */
    UWORD32 u4_cpb_size_scale;

    /**
     * (together with cpb_size_du_value_minus1) specfies
     * CPB size of the i-th CPB when the CPB operates
     * at the sub-picture level
     */
    UWORD32 u4_cpb_size_du_scale;

    /**
     * specifies the length, in bits for initial cpb delay (nal/vcl)sysntax in bp sei
     */
    UWORD8 u1_initial_cpb_removal_delay_length_minus1;

    /**
     * specifies the length, in bits for the au cpb delay syntax in pt_sei
     */
    UWORD8 u1_au_cpb_removal_delay_length_minus1;

    /**
     * specifies the length, in bits, of the pic_dpb_output_delay syntax element in the pt SEI message
     */
    UWORD8 u1_dpb_output_delay_length_minus1;

    /**
     * if 1, , for the highest temporal sub-layers, the temporal distance between the HRD output times
     * of consecutive pictures in output order is constrained refer to Table E-6
     */
    UWORD8 au1_fixed_pic_rate_general_flag[VPS_MAX_SUB_LAYERS];

    UWORD8 au1_fixed_pic_rate_within_cvs_flag[VPS_MAX_SUB_LAYERS];

    /**
     * if 1, , for the highest temporal sub-layers, the temporal distance (in clock ticks) between the
     * element units that specify HRD output times of consecutive pictures in output order is constrained
     * refer to Table E-6
     */
    UWORD8 au1_elemental_duration_in_tc_minus1[VPS_MAX_SUB_LAYERS];

    /**
     * specifies the HRD operational mode
     */
    UWORD8 au1_low_delay_hrd_flag[VPS_MAX_SUB_LAYERS];

    /**
     * 1 specifies the number of alternative CPB specifications in the
     * bitstream of the cvs when HighestTid is equal to i
     */
    UWORD8 au1_cpb_cnt_minus1[VPS_MAX_SUB_LAYERS];

    /**
     * VUI level Sub-layer HRD parameters
     */
    sub_lyr_hrd_params_t as_sub_layer_hrd_params[VPS_MAX_SUB_LAYERS];

} hrd_params_t;

/**
 * Structure to hold VUI parameters Info
 */
typedef struct
{
    /**
     * indicates the presence of aspect_ratio
     */
    UWORD8 u1_aspect_ratio_info_present_flag;

    /**
     * specifies the aspect ratio of the luma samples
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
     * 1 indicates that the coded video sequence conveys pictures that represent fields
     * 0 indicates the pictures that represents field
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
     * to 1 specifies that the syntax structure hrd_parameters is present in the vui_parameters syntax structue
     */
    UWORD8 u1_vui_hrd_parameters_present_flag;

    /**
     * VUI level HRD parameters
     */
    hrd_params_t s_vui_hrd_parameters;

    /**
     * Indicates the presence of the
     * num_units_in_ticks, time_scale flag
     */
    UWORD8 u1_vui_timing_info_present_flag;

    /**
     * Number of units that
     * correspond to one increment of the
     * clock. Indicates the  resolution
     */
    UWORD32 u4_vui_num_units_in_tick;

    /**
     * The number of time units that pass in one second
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
    UWORD8 u1_num_ticks_poc_diff_one_minus1;

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
     * min_spatial_segmentation_idc, when not equal to 0, establishes a bound on the maximum possible size of distinct
     * coded spatial segmentation regions in the pictures of the CVS. When min_spatial_segmentation_idc is not present, it is
     * inferred to be equal to 0. The value of min_spatial_segmentation_idc shall be in the range of 0 to 4095, inclusive.
     *
     * can be used by a decoder to calculate the maximum number of luma samples to be processed by one processing thread
     *
     * If tiles=0 and entropy_sync=0 then
     *     no slice shall exceed ( 4 * PicSizeInSamplesY ) / minSpatialSegmentationTimes4 luma samples
     *
     * If tiles=1 and entropy_sync=0 then
     *     no tile shall exceed ( 4 * PicSizeInSamplesY ) / minSpatialSegmentationTimes4 luma samples
     *
     * If tiles=0 and entropy_sync=1 then
     *     ( 2 * pic_height_in_luma_samples + pic_width_in_luma_samples ) * CtbSizeY
     *             <= ( 4 * PicSizeInSamplesY ) / minSpatialSegmentationTimes4
     */
    UWORD32 u4_min_spatial_segmentation_idc;
    /**
     * Indicates a number of bytes not exceeded by the sum of the sizes of the VCL NAL units
     * associated with any coded picture
     */
    UWORD8 u1_max_bytes_per_pic_denom;

    /**
     * Indicates an upper bound for the number of bits of coding_unit() data
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
} vui_t;

/**
 * Picture buffer
 */
typedef struct
{
    UWORD8 *pu1_luma;
    UWORD8 *pu1_chroma;

    WORD32 i4_abs_poc;
    WORD32 i4_poc_lsb;
    /** Used to store display Timestamp for current buffer */
    WORD32 u4_ts;
    UWORD8 u1_used_as_ref;

    UWORD8 u1_free_delay_cnt;

    /**
     * buffer ID from buffer manager
     */
    UWORD8 u1_buf_id;

    // See IV_FLD_TYPE_T for all field types
    UWORD32 e4_fld_type;

    sei_params_t s_sei_params;

    WORD32 i4_vui_present;

    vui_t s_vui;

} pic_buf_t;

/**
 * Reference List
 */
typedef struct
{
    void *pv_pic_buf;

    void *pv_mv_buf;

    UWORD8 u1_used_as_ref;

} ref_list_t;

/**
 * SAO
 */
typedef struct
{
    /**
     * sao_type_idx_luma
     */
    UWORD32 b3_y_type_idx : 3;

    /**
     * luma SaoOffsetVal[1]
     */
    WORD32 b4_y_offset_1 : 4;

    /**
     * luma SaoOffsetVal[2]
     */
    WORD32 b4_y_offset_2 : 4;

    /**
     * luma SaoOffsetVal[3]
     */
    WORD32 b4_y_offset_3 : 4;

    /**
     * luma SaoOffsetVal[4]
     */
    WORD32 b4_y_offset_4 : 4;

    /**
     * luma sao_band_position
     */
    UWORD32 b5_y_band_pos : 5;

    WORD32 : 0;

    /**
     * sao_type_idx_chroma
     */
    UWORD32 b3_cb_type_idx : 3;

    /**
     * chroma SaoOffsetVal[1]
     */
    WORD32 b4_cb_offset_1 : 4;

    /**
     * chroma SaoOffsetVal[2]
     */
    WORD32 b4_cb_offset_2 : 4;

    /**
     * chroma SaoOffsetVal[3]
     */
    WORD32 b4_cb_offset_3 : 4;

    /**
     * chroma SaoOffsetVal[4]
     */
    WORD32 b4_cb_offset_4 : 4;

    /**
     * cb sao_band_position
     */
    UWORD32 b5_cb_band_pos : 5;

    WORD32 : 0;

    /**
     * sao_type_idx_chroma
     */
    UWORD32 b3_cr_type_idx : 3;

    /**
     * chroma SaoOffsetVal[1]
     */
    WORD32 b4_cr_offset_1 : 4;

    /**
     * chroma SaoOffsetVal[2]
     */
    WORD32 b4_cr_offset_2 : 4;

    /**
     * chroma SaoOffsetVal[3]
     */
    WORD32 b4_cr_offset_3 : 4;

    /**
     * chroma SaoOffsetVal[4]
     */
    WORD32 b4_cr_offset_4 : 4;

    /**
     * cr sao_band_position
     */
    UWORD32 b5_cr_band_pos : 5;

    WORD32 : 0;

} sao_t;

/**
 * SAO
 */
typedef struct
{
    /**
     * sao_type_idx_luma
     */
    UWORD32 b3_y_type_idx : 3;

    /**
     * luma SaoOffsetVal[1]
     */
    WORD32 b8_y_offset_1 : 8;

    /**
     * luma SaoOffsetVal[2]
     */
    WORD32 b8_y_offset_2 : 8;

    /**
     * luma SaoOffsetVal[3]
     */
    WORD32 b8_y_offset_3 : 8;

    /**
     * luma SaoOffsetVal[4]
     */
    WORD32 b8_y_offset_4 : 8;

    /**
     * luma sao_band_position
     */
    UWORD32 b5_y_band_pos : 5;

    WORD32 : 0;

    /**
     * sao_type_idx_chroma
     */
    UWORD32 b3_cb_type_idx : 3;

    /**
     * chroma SaoOffsetVal[1]
     */
    WORD32 b8_cb_offset_1 : 8;

    /**
     * chroma SaoOffsetVal[2]
     */
    WORD32 b8_cb_offset_2 : 8;

    /**
     * chroma SaoOffsetVal[3]
     */
    WORD32 b8_cb_offset_3 : 8;

    /**
     * chroma SaoOffsetVal[4]
     */
    WORD32 b8_cb_offset_4 : 8;

    /**
     * cb sao_band_position
     */
    UWORD32 b5_cb_band_pos : 5;

    WORD32 : 0;

    /**
     * sao_type_idx_chroma
     */
    UWORD32 b3_cr_type_idx : 3;

    /**
     * chroma SaoOffsetVal[1]
     */
    WORD32 b8_cr_offset_1 : 8;

    /**
     * chroma SaoOffsetVal[2]
     */
    WORD32 b8_cr_offset_2 : 8;

    /**
     * chroma SaoOffsetVal[3]
     */
    WORD32 b8_cr_offset_3 : 8;

    /**
     * chroma SaoOffsetVal[4]
     */
    WORD32 b8_cr_offset_4 : 8;

    /**
     * cr sao_band_position
     */
    UWORD32 b5_cr_band_pos : 5;

    WORD32 : 0;

} sao_10bd_t;

/**
 * Motion vector
 */
typedef struct
{
    /**
     * Horizontal Motion Vector
     */
    WORD16 i2_mvx;

    /**
     * Vertical Motion Vector
     */
    WORD16 i2_mvy;
} mv_t;

/*****************************************************************************/
/* Following results in packed 48 bit structure. If mv_t included            */
/*  ref_pic_buf_id, then 8 bits will be wasted for each mv for aligning.     */
/*  Also using mv_t as elements directly instead of a pointer to l0 and l1   */
/*  mvs. Since pointer takes 4 bytes and MV itself is 4 bytes. It does not   */
/*  really help using pointers.                                              */
/*****************************************************************************/

/**
 * PU Motion Vector info
 */
typedef struct
{
    /**
     *  L0 Motion Vector
     */
    mv_t s_l0_mv;

    /**
     *  L1 Motion Vector
     */
    mv_t s_l1_mv;

    /**
     *  L0 Ref index
     */
    WORD8 i1_l0_ref_idx;

    /**
     *  L1 Ref index
     */
    WORD8 i1_l1_ref_idx;

    /**
     *  L0 Ref Pic Buf ID
     */
    WORD8 i1_l0_ref_pic_buf_id;

    /**
     *  L1 Ref Pic Buf ID
     */
    WORD8 i1_l1_ref_pic_buf_id;

} pu_mv_t;

/**
 * PU information
 */
typedef struct
{

    /**
     *  PU motion vectors
     */
    pu_mv_t mv;

    /**
     *  PU X position in terms of min PU (4x4) units
     */
    UWORD32 b4_pos_x : 4;

    /**
     *  PU Y position in terms of min PU (4x4) units
     */
    UWORD32 b4_pos_y : 4;

    /**
     *  PU width in pixels = (b4_wd + 1) << 2
     */
    UWORD32 b4_wd : 4;

    /**
     *  PU height in pixels = (b4_ht + 1) << 2
     */
    UWORD32 b4_ht : 4;

    /**
     *  Intra or Inter flag for each partition - 0 or 1
     */
    UWORD32 b1_intra_flag : 1;

    /**
     *  PRED_L0, PRED_L1, PRED_BI - Initialized in parsing only for MVP case
     */
    UWORD32 b2_pred_mode : 2;

    /**
     *  Merge flag for each partition - 0 or 1
     */
    UWORD32 b1_merge_flag : 1;

    /**
     *  Merge index for each partition - 0 to 4
     */
    UWORD32 b3_merge_idx : 3;

    /*************************************************************************/
    /* Following two flags can be overloaded with b3_merge_idx if there      */
    /* is need for additional bits                                           */
    /*************************************************************************/

    /**
     *  If merge is zero, following gives presence of mvd for L0 MV
     */
    UWORD32 b1_l0_mvp_idx : 1;

    /**
     *  If merge is zero, following gives presence of mvd for L1 MV
     */
    UWORD32 b1_l1_mvp_idx : 1;

    /**
     * Partition mode - Needed during MV merge stage
     * Note: Part mode can be derived using pu_wd, pu_ht and minCB size
     * If there is a need for bits, the following can be removed at the cost
     * of more control code in MV Merge
     */
    UWORD32 b3_part_mode : 3;

    /**
     * Partition index - Needed during MV merge stage
     */
    UWORD32 b2_part_idx : 2;

} pu_t;

/**
 * TU information
 */
typedef struct
{
    /**
     *  TU X position in terms of min TU (4x4) units
     */
    UWORD32 b4_pos_x : 4;

    /**
     *  TU Y position in terms of min TU (4x4) units
     */
    UWORD32 b4_pos_y : 4;

    /*************************************************************************/
    /* Luma TU size (width or height) = 1 << (b3_size + 2)                   */
    /*   i.e. 0 : 4, 1 : 8, 2: 16, 3: 32, 4: 64                              */
    /* Note: Though 64 x 64 TU is not possible, this size is supported to    */
    /* signal SKIP CUs or PCM CUs etc where transform is not called          */
    /* Chroma width will be half of luma except for 4x4 luma                 */
    /*************************************************************************/
    /**
     * Luma TU size (width or height)
     */
    UWORD32 b3_size : 3; //To be changed.

    /*************************************************************************/
    /* Chroma present : For 4x4 Luma TUs only the fourth one contains Cb     */
    /* Cr info. For the first three TUs in 8x8 (for 4x4 luma) this will      */
    /* be zero. For all the other cases this will be 1                       */
    /*************************************************************************/
    /**
     * 4x4 Luma TUs only the fourth one contains cb,cr
     * TODO: Check if this is really needed, cb_cbf and cr_cbf should be enough
     */

    /**
     *  Y CBF
     */
    UWORD32 b1_y_cbf : 1;

    /**
     *  Cb CBF
     */
    UWORD32 b1_cb_cbf : 1;

    /**
     *  Cr CBF
     */
    UWORD32 b1_cr_cbf : 1;

    /**
     *  Flag to indicate if it is the first TU in a CU
     */
    UWORD32 b1_first_tu_in_cu : 1;

    /**
     *  Transform quant bypass flag
     */
    UWORD32 b1_transquant_bypass : 1;

    /**
     *  Y Qp
     */
    //UWORD32     b6_qp               : 6; // BUG_FIX related to nighbour QP's in case of negative QP for HBD.
    WORD32 b7_qp : 7;

    /**
     *  Luma Intra Mode 0 - 34
     */
    UWORD32 b6_luma_intra_mode : 6;

    /*************************************************************************/
    /* Chroma Intra Mode Index 0 - 4: Actual mode (0, 1, 10, 26, 34, X) to be*/
    /* derived using luma_intra_mode and the following                       */
    /*************************************************************************/
    /**
     * Chroma Intra Mode Index 0 - 4
     */
    UWORD32 b3_chroma_intra_mode_idx : 3;

} tu_t;

/**
 * CU information
 */
typedef struct
{

    /**
     *  CU X position in terms of min CU (8x8) units
     */
    UWORD32 b3_cu_pos_x : 3;

    /**
     *  CU Y position in terms of min CU (8x8) units
     */
    UWORD32 b3_cu_pos_y : 3;

    /**
     *  CU size in terms of min CU (8x8) units
     */
    UWORD32 b4_cu_size : 4;

    /**
     *  transquant bypass flag ; 0 for this encoder
     */
    UWORD32 b1_tq_bypass_flag : 1;

    /**
     *  CU skip flag
     */
    UWORD32 b1_skip_flag : 1;

    /**
     *  intra / inter CU flag
     */
    UWORD32 b1_pred_mode_flag : 1;

    /**
     *  indicates partition information for CU
     *  For intra 0 : for 2Nx2N / 1 for NxN iff CU=minCBsize
     *  For inter 0 : @sa PART_SIZE_E
     */
    UWORD32 b3_part_mode : 3;

    /**
     *  0 for this encoder
     */
    UWORD32 b1_pcm_flag : 1;

    /**
     *  only applicable for intra cu
     */
    UWORD32 b3_chroma_intra_pred_mode : 3;

    /**
     * only applicable for intra cu
     */
    UWORD32 b1_prev_intra_luma_pred_flag0 : 1;

    /**
     * only applicable for intra cu and pred_mode=NxN
     */
    UWORD32 b1_prev_intra_luma_pred_flag1 : 1;

    /**
     * only applicable for intra cu and pred_mode=NxN
     */
    UWORD32 b1_prev_intra_luma_pred_flag2 : 1;

    /**
     * only applicable for intra cu and pred_mode=NxN
     */
    UWORD32 b1_prev_intra_luma_pred_flag3 : 1;

    /**
     *  only applicable for luma intra cu
     */
    UWORD32 b2_mpm_idx0 : 2;

    /**
     *  only applicable for intra cu and pred_mode=NxN
     */
    UWORD32 b2_mpm_idx1 : 2;

    /**
     *  only applicable for intra cu and pred_mode=NxN
     */
    UWORD32 b2_mpm_idx2 : 2;

    /**
     *  only applicable for intra cu and pred_mode=NxN
     */
    UWORD32 b2_mpm_idx3 : 2;

    /**
     *  only applicable for intra cu
     */
    UWORD32 b5_rem_intra_pred_mode0 : 5;

    /**
     *  only applicable for intra cu and pred_mode=NxN
     */
    UWORD32 b5_rem_intra_pred_mode1 : 5;

    /**
     *  only applicable for intra cu and pred_mode=NxN
     */
    UWORD32 b5_rem_intra_pred_mode2 : 5;

    /**
     *  only applicable for intra cu and pred_mode=NxN
     */
    UWORD32 b5_rem_intra_pred_mode3 : 5;

    /**
     *  no residue flag for cu
     */
    UWORD32 b1_no_residual_syntax_flag : 1;

} cu_t;

/*****************************************************************************/
/* Since the following data will be accessed linearly (no random access      */
/*  is needed for this) there is no need to store a frame level offset for   */
/*  each CTB's TU data. Only a pointer to this is stored in CTB's structure  */
/*****************************************************************************/

typedef struct
{
    /*************************************************************************/
    /* Number of TUs filled in as_tu                                         */
    /* Having the first entry as 32 bit data, helps in keeping each of       */
    /* the structures aligned to 32 bits at CTB level                        */
    /*************************************************************************/
    /**
     * Number of TUs filled in as_tu
     */
    WORD32 i4_tu_cnt;

    /**
     *  Array to map each min TU unit to a corresponding entry in as_tu
     */
    UWORD8 au1_tu_map[MAX_TU_IN_CTB];

    /*************************************************************************/
    /* TU level information                                                  */
    /* Though the allocation for as_pu as done to handle worst case data,    */
    /* only valid number of TUs will be filled in the following array.       */
    /* Next CTB starts after the valid as_tu entries                         */
    /*************************************************************************/
    /**
     *  TU level information
     */
    tu_t as_tu[MAX_TU_IN_CTB];

} ctb_tu_list_t;

/*****************************************************************************/
/* Info from last TU row of CTB is stored in a row level neighbour buffer    */
/* , which will be used for Boundary Strength computation                    */
/*****************************************************************************/
/**
 *  CTB neighbor info
 */
typedef struct
{
    /**
     *  Slice index of the ctb
     */
    UWORD16 u2_slice_idx;

    /*************************************************************************/
    /* CBF of bottom TU row (replicated in 4 pixel boundary)                 */
    /* MSB contains CBF of first TU in the last row and LSB contains CBF     */
    /* of last TU in the last row                                            */
    /*************************************************************************/
    /**
     * CBF of bottom TU row
     */
    UWORD16 u2_packed_cbf;

    /*************************************************************************/
    /* QP of bottom TU row (replicated at 8 pixel boundary (Since QP can     */
    /* not change at less than min CU granularity)                           */
    /*************************************************************************/
    /**
     * QP of bottom TU row
     */
    UWORD8 au1_qp[MAX_CU_IN_CTB_ROW];

} ctb_top_ny_info_t;

/**
 *  CTB level info
 */
typedef struct _ctb_t
{
    /*************************************************************************/
    /* Tile boundary can be detected by looking at tile start x and tile     */
    /* start y.  And based on the tile, slice and frame boundary the         */
    /* following will be initialized.                                        */
    /*************************************************************************/
    /**
     *  Pointer to left CTB
     */
    /*  If not available, this will be set to NULL   */
    struct _ctb_t *ps_ctb_left;

    /**
     *  Pointer to top-left CTB
     */
    /* If not available, this will be set to NULL   */
    ctb_top_ny_info_t *ps_ctb_ny_topleft;

    /**
     *  Pointer to top CTB
     */
    /* If not available, this will be set to NULL  */
    ctb_top_ny_info_t *ps_ctb_ny_top;

    /**
     *  Pointer to top-right CTB
     */
    /* If not available, this will be set to NULL */
    ctb_top_ny_info_t *ps_ctb_ny_topright;

    /*************************************************************************/
    /* Pointer to PU data.                                                   */
    /* This points to a MV Bank stored at frame level. Though this           */
    /* pointer can be derived by reading offset at frame level, it is        */
    /* stored here for faster access. Can be removed if storage of CTB       */
    /* structure is critical                                                 */
    /*************************************************************************/
    /**
     * Pointer to PU data
     */
    pu_t *ps_pu;

    /*************************************************************************/
    /* Pointer to a PU map stored at frame level,                            */
    /* Though this pointer can be derived by multiplying CTB adress with     */
    /* number of minTUs in a CTB, it is stored here for faster access.       */
    /* Can be removed if storage of CTB structure is critical                */
    /*************************************************************************/
    /**
     * Pointer to a PU map stored at frame level
     */
    UWORD8 *pu1_pu_map;

    /**
     *  Number of TUs filled in as_tu
     */
    /*************************************************************************/
    /* Having the first entry as 32 bit data, helps in keeping each of       */
    /* the structures aligned to 32 bits at CTB level                        */
    /*************************************************************************/
    WORD32 i4_tu_cnt;

    /**
     *  Array to map each min TU unit to a corresponding entry in as_tu
     */
    UWORD8 *pu1_tu_map;

    /**
     *  TU level information
     */
    /*************************************************************************/
    /* Though the allocation for as_pu as done to handle worst case data,    */
    /* only valid number of TUs will be filled in the following array.       */
    /* Next CTB starts after the valid as_tu entries                         */
    /*************************************************************************/
    tu_t *ps_tu;

    /**
     *  Pointer to transform coeff data
     */
    /*************************************************************************/
    /* Following format is repeated for every coded TU                       */
    /* Luma Block                                                            */
    /* num_coeffs      : 16 bits                                             */
    /* zero_cols       : 8 bits ( 1 bit per 4 columns)                       */
    /* sig_coeff_map   : ((TU Size * TU Size) + 31) >> 5 number of WORD32s   */
    /* coeff_data      : Non zero coefficients                               */
    /* Cb Block (only for last TU in 4x4 case else for every luma TU)        */
    /* num_coeffs      : 16 bits                                             */
    /* zero_cols       : 8 bits ( 1 bit per 4 columns)                       */
    /* sig_coeff_map   : ((TU Size * TU Size) + 31) >> 5 number of WORD32s   */
    /* coeff_data      : Non zero coefficients                               */
    /* Cr Block (only for last TU in 4x4 case else for every luma TU)        */
    /* num_coeffs      : 16 bits                                             */
    /* zero_cols       : 8 bits ( 1 bit per 4 columns)                       */
    /* sig_coeff_map   : ((TU Size * TU Size) + 31) >> 5 number of WORD32s   */
    /* coeff_data      : Non zero coefficients                               */
    /*************************************************************************/
    void *pv_coeff_data;

    /**
     *  Slice to which the CTB belongs to
     */
    WORD32 i4_slice_idx;

    /**
     *  CTB column position
     */
    WORD32 i4_pos_x;

    /**
     *  CTB row position
     */
    WORD32 i4_pos_y;

    /**
     *  Number of PUs filled in ps_pu
     */
    WORD32 i4_pu_cnt;

    /**
     *  Index of current PU being processed in ps_pu
     */
    /*  Scratch variable set to 0 at the start of any PU processing function */
    WORD32 i4_pu_idx;

    /**
     * Vertical Boundary strength
     */
    /* Two bits per edge.
    Stored in format. BS[15] | BS[14] | .. |BS[0]*/
    UWORD32 *pu4_vert_bs;

    /**
     * Horizontal Boundary strength
     */

    /* Two bits per edge.
    Stored in format. BS[15] | BS[14] | .. |BS[0]*/
    UWORD32 *pu4_horz_bs;

    /**
     *  Qp array stored for each 8x8 pixels
     */
    UWORD8 *pu1_qp;

    /**
     *  Pointer to current frame's pu_t array
     */
    pu_t *ps_frm_pu;

    /**
     * Pointer to current frame's pu_t index array, which stores starting index
     * of pu_t for every CTB
     */
    UWORD32 *pu4_frm_pu_idx;

    /**
     *  Pointer to current frame's pu map array
     */
    UWORD8 *pu1_frm_pu_map;

    /*************************************************************************/
    /* Need to add encoder specific elements for identifying the order of    */
    /* coding for CU, TU and PU if any                                       */
    /*************************************************************************/
} ctb_t;

/*****************************************************************************/
/* The following can be used to typecast coefficient data that is stored     */
/*  per subblock. Note that though i2_level is shown as an array that        */
/*  holds 16 coefficients, only the first few entries will be valid. Next    */
/*  subblocks data starts after the valid number of coefficients. Number     */
/*  of non-zero coefficients will be derived using number of non-zero bits   */
/*  in sig coeff map                                                         */
/*****************************************************************************/
/**
 * Structure to hold coefficient info for a 4x4 subblock
 */
typedef struct
{
    /**
     * sub block position
     */
    UWORD16 u2_subblk_pos;

    /**
     * significant coefficient map
     */
    UWORD16 u2_sig_coeff_map;

    /**
     * holds 16 coefficients
     */
    WORD16 ai2_level[SUBBLK_COEFF_CNT];
} tu_sblk_coeff_data_t;

/*************************************************************************/
/* The following describes how each of the CU cases are handled          */
/*************************************************************************/

/*************************************************************************/
/* For SKIP CU                                                           */
/* One Inter PU with appropriate MV                                      */
/* One TU which says Y, Cb and Cr CBF is zero with size equal to CB size */
/*************************************************************************/

/*************************************************************************/
/* For Inter CU                                                          */
/* M Inter PU with appropriate MVs (M between 1 to 4)                    */
/* N TU (N is number of TU in CU)                                        */
/*************************************************************************/

/*************************************************************************/
/* For Intra CU                                                          */
/* N TU (N is number of TU in CU)                                        */
/* N Intra PU with appropriate pred modes for luma and chroma            */
/*************************************************************************/

/*************************************************************************/
/* For Intra PCM CU                                                      */
/* One TU which says transquant bypass is 1  with size equal to CB size  */
/* 1 Intra PU with pcm flag set to 1(which ensures no intra pred is done)*/
/*************************************************************************/

/*************************************************************************/
/* For a CU where cu_transquant_bypass_flag is 1                         */
/* One TU which says transquant bypass is 1  with size equal to CB size  */
/* N Intra/Inter PUs                                                     */
/*************************************************************************/

/*************************************************************************/
/* For a CU where no_residual_syntax_flag is 1                           */
/* One TU which says Y, Cb, Cr CBF is 0  with size equal to CB size      */
/* N Inter PUs                                                           */
/*************************************************************************/

/**
 * Structure giving information about the tile
 */
typedef struct
{
    /* X position of the tile in the current frame in CTB units */
    UWORD8 u1_pos_x;

    /* Y position of the tile in the current frame in CTB units */
    UWORD8 u1_pos_y;

    /* Tile width in CTB units */
    UWORD16 u2_wd;

    /* Tile height in CTB units */
    UWORD16 u2_ht;

} tile_t;

/**
 * Structure to hold Profile tier level info for a given layer
 */

typedef struct
{
    /**
     *  NAL unit type
     */
    WORD8 i1_nal_unit_type;

    /**
     *  NAL temporal id
     */
    WORD8 i1_nuh_temporal_id;
} nal_header_t;

/**
 * Structure to hold Profile tier level info for a given layer
 */

typedef struct
{
    /**
     *  profile_space
     */
    WORD8 i1_profile_space;

    /**
     *  tier_flag
     */
    WORD8 i1_tier_flag;

    /**
     *  profile_idc
     */
    WORD8 i1_profile_idc;

    /**
     *  profile_compatibility_flag[]
     */
    WORD8 ai1_profile_compatibility_flag[MAX_PROFILE_COMPATBLTY];

    /**
     * progressive_source_flag
     */
    WORD8 i1_general_progressive_source_flag;

    /**
     * interlaced_source_flag
     */
    WORD8 i1_general_interlaced_source_flag;

    /**
     * non_packed_constraint_flag
     */
    WORD8 i1_general_non_packed_constraint_flag;

    /**
     * frame_only_constraint_flag
     */
    WORD8 i1_frame_only_constraint_flag;

    /**
     * general_max_12bit_constraint_flag
     */
    WORD8 i1_general_max_12bit_constraint_flag;

    /**
     * general_max_10bit_constraint_flag
     */
    WORD8 i1_general_max_10bit_constraint_flag;

    /**
     * general_max_8bit_constraint_flag
     */
    WORD8 i1_general_max_8bit_constraint_flag;

    /**
     * general_max_422chroma_constraint_flag
     */
    WORD8 i1_general_max_422chroma_constraint_flag;

    /**
     * general_max_420chroma_constraint_flag
     */
    WORD8 i1_general_max_420chroma_constraint_flag;

    /**
     * general_max_monochrome_constraint_flag
     */
    WORD8 i1_general_max_monochrome_constraint_flag;

    /**
     * general_intra_constraint_flag
     */
    WORD8 i1_general_intra_constraint_flag;

    /**
     * general_one_picture_only_constraint_flag
     */
    WORD8 i1_general_one_picture_only_constraint_flag;

    /**
     * general_lower_bit_rate_constraint_flag
     */
    WORD8 i1_general_lower_bit_rate_constraint_flag;

    /**
     *  level_idc
     */
    UWORD8 u1_level_idc;
} profile_tier_lvl_t;

/**
 * Structure to hold Profile tier level info for all layers
 */
typedef struct
{
    /**
     *  Profile and tier information for general
     */
    profile_tier_lvl_t s_ptl_gen;

    /**
     *  sub_layer_profile_present_flag[]
     */
    WORD8 ai1_sub_layer_profile_present_flag[VPS_MAX_SUB_LAYERS - 1];

    /**
     *  sub_layer_level_present_flag[]
     */
    WORD8 ai1_sub_layer_level_present_flag[VPS_MAX_SUB_LAYERS - 1];

    /**
     *  Profile and tier information for sub layers
     */
    profile_tier_lvl_t as_ptl_sub[VPS_MAX_SUB_LAYERS - 1];

} profile_tier_lvl_info_t;

/**
 * Structure to hold short term reference picture set info
 */
typedef struct
{
    /**
     *  delta_poc_s0_minus1[ i ] and delta_poc_s1_minus1[ i ]
     */
    WORD16 ai2_delta_poc[MAX_DPB_SIZE];

    /**
     *  inter_ref_pic_set_prediction_flag
     */
    WORD8 i1_inter_ref_pic_set_prediction_flag;

    /**
     *  num_negative_pics
     */
    WORD8 i1_num_neg_pics;

    /**
     *  num_positive_pics
     */
    WORD8 i1_num_pos_pics;

    /**
     *  used_by_curr_pic_s0_flag[ i ] and used_by_curr_pic_s1_flag[i]
     */
    WORD8 ai1_used[MAX_DPB_SIZE];

    /**
     *  Ref Idc
     */
    WORD8 ai1_ref_idc[MAX_DPB_SIZE];

    /**
     *  Sum of positive and negative pics for each refence
     */
    WORD8 i1_num_delta_pocs;

    /**
     *  Number of ref_idc
     */
    WORD8 i1_num_ref_idc;
} stref_picset_t;

/**
 * Structure to hold weighted prediction info such as weights and offsets
 */
typedef struct
{
    /** luma_log2_weight_denom */
    WORD8 i1_luma_log2_weight_denom;

    /** delta_chroma_log2_weight_denom */
    WORD8 i1_chroma_log2_weight_denom;

    /** luma_weight_l0_flag[ i ] */
    WORD8 i1_luma_weight_l0_flag[MAX_DPB_SIZE];

    /** chroma_weight_l0_flag[ i ] */
    WORD8 i1_chroma_weight_l0_flag[MAX_DPB_SIZE];

    /** delta_luma_weight_l0[ i ] */
    WORD16 i2_luma_weight_l0[MAX_DPB_SIZE];

    /** luma_offset_l0[ i ] */
    WORD16 i2_luma_offset_l0[MAX_DPB_SIZE];

    /** delta_chroma_weight_l0[ i ][ j ] */
    WORD16 i2_chroma_weight_l0_cb[MAX_DPB_SIZE];

    /** delta_chroma_offset_l0[ i ][ j ] */
    WORD16 i2_chroma_offset_l0_cb[MAX_DPB_SIZE];

    /** delta_chroma_weight_l0[ i ][ j ] */
    WORD16 i2_chroma_weight_l0_cr[MAX_DPB_SIZE];

    /** delta_chroma_offset_l0[ i ][ j ] */
    WORD16 i2_chroma_offset_l0_cr[MAX_DPB_SIZE];

    /** luma_weight_l1_flag[ i ] */
    WORD8 i1_luma_weight_l1_flag[MAX_DPB_SIZE];

    /** chroma_weight_l1_flag[ i ] */
    WORD8 i1_chroma_weight_l1_flag[MAX_DPB_SIZE];

    /** delta_luma_weight_l1[ i ] */
    WORD16 i2_luma_weight_l1[MAX_DPB_SIZE];

    /** luma_offset_l1[ i ] */
    WORD16 i2_luma_offset_l1[MAX_DPB_SIZE];

    /** delta_chroma_weight_l1[ i ][ j ] */
    WORD16 i2_chroma_weight_l1_cb[MAX_DPB_SIZE];

    /** delta_chroma_offset_l1[ i ][ j ] */
    WORD16 i2_chroma_offset_l1_cb[MAX_DPB_SIZE];

    /** delta_chroma_weight_l1[ i ][ j ] */
    WORD16 i2_chroma_weight_l1_cr[MAX_DPB_SIZE];

    /** delta_chroma_offset_l1[ i ][ j ] */
    WORD16 i2_chroma_offset_l1_cr[MAX_DPB_SIZE];

} pred_wt_ofst_t;

/**
 * Structure to hold Reference picture list modification info
 */
typedef struct
{
    /* ref_pic_list_modification_flag_l0 */
    WORD8 i1_ref_pic_list_modification_flag_l0;

    /* list_entry_l0[ i ] */
    WORD8 i1_list_entry_l0[16];

    /* ref_pic_list_modification_flag_l1 */
    WORD8 i1_ref_pic_list_modification_flag_l1;

    /* list_entry_l1[ i ] */
    WORD8 i1_list_entry_l1[16];

    /* Reference POC values for L0,L1 */
    WORD32 i4_ref_poc_l0[16];
    WORD32 i4_ref_poc_l1[16];
} rplm_t;

/**
 * Structure to hold VPS info
 */
typedef struct
{
    /**
     *  video_parameter_set_id
     */
    WORD8 i1_vps_id;

    /**
     *  vps_temporal_id_nesting_flag
     */
    WORD8 i1_vps_temporal_id_nesting_flag;
    /**
     * sub_layer_ordering_info_present_flag
     */
    WORD8 i1_sub_layer_ordering_info_present_flag;
    /**
     *  vps_max_sub_layers_minus1
     */
    WORD8 i1_vps_max_sub_layers;

    /**
     *  vps_max_dec_pic_buffering
     */
    WORD8 ai1_vps_max_dec_pic_buffering[VPS_MAX_SUB_LAYERS];

    /**
     *  vps_max_num_reorder_pics
     */
    WORD8 ai1_vps_max_num_reorder_pics[VPS_MAX_SUB_LAYERS];

    /**
     *  vps_max_latency_increase
     */
    WORD8 ai1_vps_max_latency_increase[VPS_MAX_SUB_LAYERS];

    /**
     *  vps_num_hrd_parameters
     */
    WORD8 i1_vps_num_hrd_parameters;

    /**
     * vps_max_nuh_reserved_zero_layer_id
     */
    WORD8 i1_vps_max_nuh_reserved_zero_layer_id;

    /**
     * vps_num_op_sets
     */
    WORD8 i1_vps_num_op_sets;

    /**
     * layer_id_included_flag
     */
    //WORD8 ai1_layer_id_included_flag[2][MAX_NUH_LAYERS];
    /**
     *  Profile, Tier and Level info
     */
    profile_tier_lvl_info_t s_ptl;

    /**
     * bit_rate_info_present_flag[i]
     */
    WORD8 ai1_bit_rate_info_present_flag[VPS_MAX_SUB_LAYERS];

    /**
     * pic_rate_info_present_flag[i]
     */
    WORD8 ai1_pic_rate_info_present_flag[VPS_MAX_SUB_LAYERS];

    /**
     * avg_bit_rate[i]
     */
    UWORD16 au2_avg_bit_rate[VPS_MAX_SUB_LAYERS];

    /**
     * max_bit_rate[i]
     */
    UWORD16 au2_max_bit_rate[VPS_MAX_SUB_LAYERS];

    /**
     * constant_pic_rate_idc[i]
     */
    WORD8 ai1_constant_pic_rate_idc[VPS_MAX_SUB_LAYERS];

    /**
     * avg_pic_rate[i]
     */
    UWORD16 au2_avg_pic_rate[VPS_MAX_SUB_LAYERS];
} vps_t;

/**
 * Structure to hold SPS info
 */
typedef struct
{
    /**
     * pic_width_in_luma_samples
     */
    WORD16 i2_pic_width_in_luma_samples;

    /**
     *  pic_height_in_luma_samples
     */
    WORD16 i2_pic_height_in_luma_samples;

    /**
     *  pic_crop_left_offset
     */
    WORD16 i2_pic_crop_left_offset;

    /**
     *  pic_crop_right_offset
     */
    WORD16 i2_pic_crop_right_offset;

    /**
     *  pic_crop_top_offset
     */
    WORD16 i2_pic_crop_top_offset;

    /**
     *  pic_crop_bottom_offset
     */
    WORD16 i2_pic_crop_bottom_offset;

    /**
     *  seq_parameter_set_id
     */
    WORD8 i1_sps_id;

    /**
     *  video_parameter_set_id
     */
    WORD8 i1_vps_id;

    /**
     *  sps_max_sub_layers_minus1
     */
    WORD8 i1_sps_max_sub_layers;

    /**
     *  chroma_format_idc
     */
    WORD8 i1_chroma_format_idc;

    /**
     * Bit depth of luma samples
     */
    WORD8 i1_bit_depth_luma_minus8;

    /**
     * Bit depth of chrma samples
     */
    WORD8 i1_bit_depth_chroma_minus8;

    /* separate_colour_plane_flag */
    WORD8 i1_separate_colour_plane_flag;

    /**
     *  pic_cropping_flag
     */
    WORD8 i1_pic_cropping_flag;

    /**
     *  pcm_enabled_flag
     */
    WORD8 i1_pcm_enabled_flag;

    /**
     *  pcm_sample_bit_depth_luma
     */
    WORD8 i1_pcm_sample_bit_depth_luma;

    /**
     *  pcm_sample_bit_depth_chroma
     */
    WORD8 i1_pcm_sample_bit_depth_chroma;

    /**
     *  log2_max_pic_order_cnt_lsb_minus4
     */
    WORD8 i1_log2_max_pic_order_cnt_lsb;
    /**
     * sps_sub_layer_ordering_info_present_flag
     */
    WORD8 i1_sps_sub_layer_ordering_info_present_flag;
    /**
     *  sps_max_dec_pic_buffering
     */
    WORD8 ai1_sps_max_dec_pic_buffering[SPS_MAX_SUB_LAYERS];

    /**
     *  sps_max_num_reorder_pics
     */
    WORD8 ai1_sps_max_num_reorder_pics[SPS_MAX_SUB_LAYERS];

    /**
     *  sps_max_latency_increase
     */
    WORD8 ai1_sps_max_latency_increase[SPS_MAX_SUB_LAYERS];

    /**
     *  log2_min_coding_block_size_minus3
     */
    WORD8 i1_log2_min_coding_block_size;

    /**
     *  log2_diff_max_min_coding_block_size
     */
    WORD8 i1_log2_diff_max_min_coding_block_size;

    /**
     *  log2_min_transform_block_size_minus2
     */
    WORD8 i1_log2_min_transform_block_size;

    /**
     *  log2_diff_max_min_transform_block_size
     */
    WORD8 i1_log2_diff_max_min_transform_block_size;

    /**
     *  log2_min_pcm_coding_block_size_minus3
     */
    WORD8 i1_log2_min_pcm_coding_block_size;

    /**
     *  log2_diff_max_min_pcm_coding_block_size
     */
    WORD8 i1_log2_diff_max_min_pcm_coding_block_size;

    /**
     *  max_transform_hierarchy_depth_inter
     */
    WORD8 i1_max_transform_hierarchy_depth_inter;

    /**
     *  max_transform_hierarchy_depth_intra
     */
    WORD8 i1_max_transform_hierarchy_depth_intra;

    /**
     *  scaling_list_enable_flag
     */
    WORD8 i1_scaling_list_enable_flag;

    /**
     *  sps_scaling_list_data_present_flag
     */
    WORD8 i1_sps_scaling_list_data_present_flag;

    /**
     *  amp_enabled_flag
     */
    WORD8 i1_amp_enabled_flag;

    /**
     *  sample_adaptive_offset_enabled_flag
     */
    WORD8 i1_sample_adaptive_offset_enabled_flag;

    /**
     *  pcm_loop_filter_disable_flag
     */
    WORD8 i1_pcm_loop_filter_disable_flag;

    /**
     *  sps_temporal_id_nesting_flag
     */
    WORD8 i1_sps_temporal_id_nesting_flag;

    /**
     *  num_short_term_ref_pic_sets
     */
    WORD8 i1_num_short_term_ref_pic_sets;

    /**
     *  long_term_ref_pics_present_flag
     */
    WORD8 i1_long_term_ref_pics_present_flag;

    /**
     *  num_long_term_ref_pics_sps
     */
    WORD8 i1_num_long_term_ref_pics_sps;

    /**
     *  lt_ref_pic_poc_lsb_sps[]
     */
    UWORD16 au2_lt_ref_pic_poc_lsb_sps[MAX_LTREF_PICS_SPS];

    /**
     *  used_by_curr_pic_lt_sps_flag[]
     */
    WORD8 ai1_used_by_curr_pic_lt_sps_flag[MAX_LTREF_PICS_SPS];

    /**
     *  sps_temporal_mvp_enable_flag
     */
    WORD8 i1_sps_temporal_mvp_enable_flag;

    /**
     * strong_intra_smoothing_enable_flag
     */
    WORD8 i1_strong_intra_smoothing_enable_flag;

    /**
     *  vui_parameters_present_flag
     */
    WORD8 i1_vui_parameters_present_flag;

    /**
     * vui parameters Structure info
     */
    vui_t s_vui_parameters;

    /**
     *  Log2(CTB Size) in luma units
     */

    WORD8 i1_log2_ctb_size;

    /**
     * Maximum transform block size
     */
    WORD8 i1_log2_max_transform_block_size;

    /**
     *  Picture width in CTB units
     */

    WORD16 i2_pic_wd_in_ctb;

    /**
     *  Picture height in CTB units
     */

    WORD16 i2_pic_ht_in_ctb;

    /**
     * Picture width in min CB units
     */

    WORD16 i2_pic_wd_in_min_cb;

    /**
     *  Picture height in min CB units
     */

    WORD16 i2_pic_ht_in_min_cb;

    /**
     *  Picture size in CTB units
     */
    WORD32 i4_pic_size_in_ctb;

    /**
     *  Profile, Tier and Level info
     */

    profile_tier_lvl_info_t s_ptl;

    /**
     *  Short term reference pic set
     */
    stref_picset_t as_stref_picset[MAX_STREF_PICS_SPS];

    /**
     *  Pointer to scaling matrix
     */
    /*************************************************************************/
    /* Contanis the matrice in the following order in a 1D buffer            */
    /* Intra 4 x 4 Y, 4 x 4 U, 4 x 4 V                                       */
    /* Inter 4 x 4 Y, 4 x 4 U, 4 x 4 V                                       */
    /* Intra 8 x 8 Y, 8 x 8 U, 8 x 8 V                                       */
    /* Inter 8 x 8 Y, 8 x 8 U, 8 x 8 V                                       */
    /* Intra 16x16 Y, 16x16 U, 16x16 V                                       */
    /* Inter 16x16 Y, 16x16 U, 16x16 V                                       */
    /* Intra 32x32 Y                                                         */
    /* Inter 32x32 Y                                                         */
    /*************************************************************************/
    WORD16 *pi2_scaling_mat;

    /*
     * Flag indicating if the SPS is parsed
     */
    WORD8 i1_sps_valid;

} sps_t;

/**
 * Structure to hold PPS info
 */
typedef struct
{
    /**
     *  Pointer to scaling matrix
     */
    /*************************************************************************/
    /* Contanis the matrice in the following order in a 1D buffer            */
    /* Intra 4 x 4 Y, 4 x 4 U, 4 x 4 V                                       */
    /* Inter 4 x 4 Y, 4 x 4 U, 4 x 4 V                                       */
    /* Intra 8 x 8 Y, 8 x 8 U, 8 x 8 V                                       */
    /* Inter 8 x 8 Y, 8 x 8 U, 8 x 8 V                                       */
    /* Intra 16x16 Y, 16x16 U, 16x16 V                                       */
    /* Inter 16x16 Y, 16x16 U, 16x16 V                                       */
    /* Intra 32x32 Y                                                         */
    /* Inter 32x32 Y                                                         */
    /*************************************************************************/
    WORD16 *pi2_scaling_mat;

    /**
     *  Pointer to an array containing tile info such as position, width, height
     *  of each tile
     */

    /* column_width_minus1[ i ] and row_height_minus1[ i ] */
    tile_t *ps_tile;

    /**
     *  pic_parameter_set_id
     */
    WORD8 i1_pps_id;

    /**
     *  seq_parameter_set_id
     */
    WORD8 i1_sps_id;

    /**
     *  sign_data_hiding_flag
     */
    WORD8 i1_sign_data_hiding_flag;

    /**
     *  cabac_init_present_flag
     */
    WORD8 i1_cabac_init_present_flag;

    /**
     *  num_ref_idx_l0_default_active_minus1
     */
    WORD8 i1_num_ref_idx_l0_default_active;

    /**
     * num_ref_idx_l1_default_active_minus1
     */
    WORD8 i1_num_ref_idx_l1_default_active;

    /**
     *  pic_init_qp_minus26
     */
    WORD8 i1_pic_init_qp;

    /**
     *  constrained_intra_pred_flag
     */
    WORD8 i1_constrained_intra_pred_flag;

    /**
     *  transform_skip_enabled_flag
     */
    WORD8 i1_transform_skip_enabled_flag;

    /**
     *  cu_qp_delta_enabled_flag
     */
    WORD8 i1_cu_qp_delta_enabled_flag;

    /**
     * diff_cu_qp_delta_depth
     */
    WORD8 i1_diff_cu_qp_delta_depth;

    /**
     *  pic_cb_qp_offset
     */
    WORD8 i1_pic_cb_qp_offset;

    /**
     *  pic_cr_qp_offset
     */
    WORD8 i1_pic_cr_qp_offset;

    /**
     *  pic_slice_level_chroma_qp_offsets_present_flag
     */
    WORD8 i1_pic_slice_level_chroma_qp_offsets_present_flag;

    /**
     *  weighted_pred_flag
     */
    WORD8 i1_weighted_pred_flag;

    /**
     *  weighted_bipred_flag
     */
    WORD8 i1_weighted_bipred_flag;

    /**
     *  output_flag_present_flag
     */
    WORD8 i1_output_flag_present_flag;

    /**
     *  transquant_bypass_enable_flag
     */
    WORD8 i1_transquant_bypass_enable_flag;

    /**
     *  dependent_slice_enabled_flag
     */
    WORD8 i1_dependent_slice_enabled_flag;

    /**
     *  tiles_enabled_flag
     */
    WORD8 i1_tiles_enabled_flag;

    /**
     *  entropy_coding_sync_enabled_flag
     */
    WORD8 i1_entropy_coding_sync_enabled_flag;

    /**
     * entropy_slice_enabled_flag
     */
    WORD8 i1_entropy_slice_enabled_flag;

    /**
     *  num_tile_columns_minus1
     */
    WORD8 i1_num_tile_columns;

    /**
     *  num_tile_rows_minus1
     */
    WORD8 i1_num_tile_rows;

    /**
     *  uniform_spacing_flag
     */
    WORD8 i1_uniform_spacing_flag;

    /**
     *  loop_filter_across_tiles_enabled_flag
     */
    WORD8 i1_loop_filter_across_tiles_enabled_flag;

    /**
     *  loop_filter_across_slices_enabled_flag
     */
    WORD8 i1_loop_filter_across_slices_enabled_flag;

    /**
     *  deblocking_filter_control_present_flag
     */
    WORD8 i1_deblocking_filter_control_present_flag;

    /**
     *  deblocking_filter_override_enabled_flag
     */
    WORD8 i1_deblocking_filter_override_enabled_flag;

    /**
     *  pic_disable_deblocking_filter_flag
     */
    WORD8 i1_pic_disable_deblocking_filter_flag;

    /**
     *  beta_offset_div2
     */
    WORD8 i1_beta_offset_div2;

    /**
     *  tc_offset_div2
     */
    WORD8 i1_tc_offset_div2;

    /**
     *  pps_scaling_list_data_present_flag
     */
    WORD8 i1_pps_scaling_list_data_present_flag;

    /**
     * lists_modification_present_flag
     */
    WORD8 i1_lists_modification_present_flag;

    /**
     * num_extra_slice_header_bits
     */
    WORD8 i1_num_extra_slice_header_bits;

    /**
     *  log2_parallel_merge_level_minus2
     */
    WORD8 i1_log2_parallel_merge_level;

    /**
     *  slice_header_extension_present_flag
     */
    WORD8 i1_slice_header_extension_present_flag;

    /**
     *  slice_extension_present_flag
     */
    WORD8 i1_slice_extension_present_flag;

    /**
     *  scaling_list_dc_coef_minus8
     */
    /*************************************************************************/
    /* DC value of the scaling list                                          */
    /* Only 16 x 16 and 32 x 32 scaling lists have valid entries.            */
    /* Entries stored for all sizes for uniformity.                          */
    /* Remaining will be initialized to default values if used               */
    /*************************************************************************/
    UWORD8 au1_scaling_list_dc_coef[TOTAL_SCALE_MAT_COUNT];

    /**
     * Log2MinCuQpDeltaSize
     */
    WORD8 i1_log2_min_cu_qp_delta_size;

    /*
     * Flag indicating if the PPS is parsed
     */
    WORD8 i1_pps_valid;

} pps_t;

/**
 * Structure to hold slice header info
 */
typedef struct
{
    /**
     *  entry_point_offset[ i ]
     */
    WORD32 *pi4_entry_point_offset;

    /**
     *  poc_lsb_lt[ i ]
     */
    WORD32 ai4_poc_lsb_lt[MAX_DPB_SIZE];

    /**
     *  slice_header_extension_length
     */
    WORD16 i2_slice_header_extension_length;

    /**
     *  slice_address
     */
    WORD16 i2_slice_address;

    /**
     *  first_slice_in_pic_flag
     */
    WORD8 i1_first_slice_in_pic_flag;

    /* PPS id */
    WORD8 i1_pps_id;
    /**
     *  no_output_of_prior_pics_flag
     */
    WORD8 i1_no_output_of_prior_pics_flag;

    /**
     *  dependent_slice_flag
     */
    WORD8 i1_dependent_slice_flag;

    /**
     *  slice_type
     */
    WORD8 i1_slice_type;

    /**
     *  pic_output_flag
     */
    WORD8 i1_pic_output_flag;

    /**
     *  colour_plane_id
     */
    WORD8 i1_colour_plane_id;

    /**
     *  pic_order_cnt_lsb
     */
    WORD32 i4_pic_order_cnt_lsb;

    /**
     *  absolute pic_order_cnt
     */
    WORD32 i4_abs_pic_order_cnt;

    /**
     *  short_term_ref_pic_set_sps_flag
     */
    WORD8 i1_short_term_ref_pic_set_sps_flag;

    /**
     *  short_term_ref_pic_set_idx
     */
    WORD8 i1_short_term_ref_pic_set_idx;

    /**
     *  num_long_term_sps
     */
    WORD8 i1_num_long_term_sps;

    /**
     *  num_long_term_pics
     */
    WORD8 i1_num_long_term_pics;

    /**
     *  lt_idx_sps[ i ]
     */
    WORD8 ai1_lt_idx_sps[MAX_DPB_SIZE];

    /**
     *  used_by_curr_pic_lt_flag[ i ]
     */
    WORD8 ai1_used_by_curr_pic_lt_flag[MAX_DPB_SIZE];

    /**
     *  delta_poc_msb_present_flag[ i ]
     */
    WORD8 ai1_delta_poc_msb_present_flag[MAX_DPB_SIZE];

    /**
     *  delta_poc_msb_cycle_lt[ i ]
     */
    WORD8 ai1_delta_poc_msb_cycle_lt[MAX_DPB_SIZE];

    /**
     *  slice_sao_luma_flag
     */
    WORD8 i1_slice_sao_luma_flag;

    /**
     *  slice_sao_chroma_flag
     */
    WORD8 i1_slice_sao_chroma_flag;

    /**
     *  slice_temporal_mvp_enable_flag
     */
    WORD8 i1_slice_temporal_mvp_enable_flag;

    /**
     *  num_ref_idx_active_override_flag
     */
    WORD8 i1_num_ref_idx_active_override_flag;

    /**
     *  num_ref_idx_l0_active_minus1
     */
    WORD8 i1_num_ref_idx_l0_active;

    /**
     *  num_ref_idx_l1_active_minus1
     */
    WORD8 i1_num_ref_idx_l1_active;

    /**
     *  mvd_l1_zero_flag
     */
    WORD8 i1_mvd_l1_zero_flag;

    /**
     *  cabac_init_flag
     */
    WORD8 i1_cabac_init_flag;

    /**
     *  collocated_from_l0_flag
     */
    WORD8 i1_collocated_from_l0_flag;

    /**
     *  collocated_ref_idx
     */
    WORD8 i1_collocated_ref_idx;

    /**
     * five_minus_max_num_merge_cand
     */
    WORD8 i1_max_num_merge_cand;

    /**
     *  slice_qp_delta
     */
    WORD8 i1_slice_qp_delta;

    /**
     *  slice_cb_qp_offset
     */
    WORD8 i1_slice_cb_qp_offset;

    /**
     *  slice_cr_qp_offset
     */
    WORD8 i1_slice_cr_qp_offset;

    /**
     *  deblocking_filter_override_flag
     */
    WORD8 i1_deblocking_filter_override_flag;

    /**
     *  slice_disable_deblocking_filter_flag
     */
    WORD8 i1_slice_disable_deblocking_filter_flag;

    /**
     *  beta_offset_div2
     */
    WORD8 i1_beta_offset_div2;

    /**
     *  tc_offset_div2
     */
    WORD8 i1_tc_offset_div2;

    /**
     *  slice_loop_filter_across_slices_enabled_flag
     */
    WORD8 i1_slice_loop_filter_across_slices_enabled_flag;

    /**
     *  NUmber of entry point offsets
     */
    WORD32 i4_num_entry_point_offsets;

    /**
     *  offset_len_minus1
     */
    WORD8 i1_offset_len;

    /**
     *  Entry point offsets
     */
    WORD32 *pu4_entry_point_offset;

    /**
     * Short term reference picture set
     */
    stref_picset_t s_stref_picset;

    /**
     *  Weight and offset info for Weighted prediction
     */
    pred_wt_ofst_t s_wt_ofst;

    /**
     *  Reference prediction list modification
     */
    rplm_t s_rplm;

    /**
     *  First CTB' X pos : slice_address % i2_pic_wd_in_ctb
     */
    WORD16 i2_ctb_x;

    /**
     *  First CTB' Y pos : slice_address / i2_pic_wd_in_ctb
     */
    WORD16 i2_ctb_y;

    /**
     * L0 Reference pic lists
     */
    ref_list_t as_ref_pic_list0[MAX_DPB_SIZE];

    /**
     * L1 Reference pic lists
     */
    ref_list_t as_ref_pic_list1[MAX_DPB_SIZE];

    /**
     * NAL unit type of the slice
     */
    WORD8 i1_nal_unit_type;

    /**
     * Low delay check flag
     */
    WORD8 i1_low_delay_flag;

    /**
     * The last independent slice's start ctb_x
     * If the current slice is independent, it is the same as the current CTBs ctb_x
     */
    WORD16 i2_independent_ctb_x;

    /**
     * The last independent slice's start ctb_y
     * If the current slice is independent, it is the same as the current CTBs ctb_y
     */
    WORD16 i2_independent_ctb_y;

    UWORD8 u1_parse_data_init_done;

    /**
     * Temporal ID in NAL header
     */
    WORD32 u4_nuh_temporal_id;
} slice_header_t;

#endif /* _IHEVC_STRUCTS_H_ */
