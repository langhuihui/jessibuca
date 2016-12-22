/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DE265_VUI_H
#define DE265_VUI_H

#include "libde265/de265.h"
#include "libde265/bitstream.h"

#include <vector>

class error_queue;
class seq_parameter_set;


enum VideoFormat {
  VideoFormat_Component = 0,
  VideoFormat_PAL   = 1,
  VideoFormat_NTSC  = 2,
  VideoFormat_SECAM = 3,
  VideoFormat_MAC   = 4,
  VideoFormat_Unspecified = 5
};

const char* get_video_format_name(enum VideoFormat);


class video_usability_information
{
 public:
  video_usability_information();

  de265_error read(error_queue*, bitreader*, const seq_parameter_set*);
  void dump(int fd) const;


  // --- sample aspect ratio (SAR) ---

  bool     aspect_ratio_info_present_flag;
  uint16_t sar_width;  // sar_width and sar_height are zero if unspecified
  uint16_t sar_height;


  // --- overscan ---

  bool     overscan_info_present_flag;
  bool     overscan_appropriate_flag;


  // --- video signal type ---

  bool     video_signal_type_present_flag;
  enum VideoFormat  video_format;
  bool     video_full_range_flag;
  bool     colour_description_present_flag;
  uint8_t  colour_primaries;
  uint8_t  transfer_characteristics;
  uint8_t  matrix_coeffs;

  // --- chroma / interlaced ---

  bool     chroma_loc_info_present_flag;
  uint8_t  chroma_sample_loc_type_top_field;
  uint8_t  chroma_sample_loc_type_bottom_field;

  bool     neutral_chroma_indication_flag;
  bool     field_seq_flag;
  bool     frame_field_info_present_flag;

  // --- default display window ---

  bool     default_display_window_flag;
  uint32_t def_disp_win_left_offset;
  uint32_t def_disp_win_right_offset;
  uint32_t def_disp_win_top_offset;
  uint32_t def_disp_win_bottom_offset;


  // --- timing ---

  bool     vui_timing_info_present_flag;
  uint32_t vui_num_units_in_tick;
  uint32_t vui_time_scale;

  bool     vui_poc_proportional_to_timing_flag;
  uint32_t vui_num_ticks_poc_diff_one;


  // --- hrd parameters ---

  bool     vui_hrd_parameters_present_flag;
  //hrd_parameters vui_hrd_parameters;


  // --- bitstream restriction ---

  bool bitstream_restriction_flag;
  bool tiles_fixed_structure_flag;
  bool motion_vectors_over_pic_boundaries_flag;
  bool restricted_ref_pic_lists_flag;
  uint16_t min_spatial_segmentation_idc;
  uint8_t  max_bytes_per_pic_denom;
  uint8_t  max_bits_per_min_cu_denom;
  uint8_t  log2_max_mv_length_horizontal;
  uint8_t  log2_max_mv_length_vertical;
};


#endif
