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
*  ihevc_quant_tables.h
*
* @brief
*  Tables for forward and inverse quantization
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_QUANT_TABLES_H_
#define _IHEVC_QUANT_TABLES_H_

extern const WORD16 gi2_flat_scale_mat_32x32[];

extern const WORD16 gi2_intra_default_scale_mat_8x8[];

extern const WORD16 gi2_inter_default_scale_mat_8x8[];

extern const WORD16 gi2_intra_default_scale_mat_16x16[];

extern const WORD16 gi2_inter_default_scale_mat_16x16[];

extern const WORD16 gi2_intra_default_scale_mat_32x32[];

extern const WORD16 gi2_inter_default_scale_mat_32x32[];


extern const WORD16 gi2_flat_rescale_mat_32x32[];

extern const WORD16 gi2_intra_default_rescale_mat_8x8[];

extern const WORD16 gi2_inter_default_rescale_mat_8x8[];

extern const WORD16 gi2_intra_default_rescale_mat_16x16[];

extern const WORD16 gi2_inter_default_rescale_mat_16x16[];

extern const WORD16 gi2_intra_default_rescale_mat_32x32[];

extern const WORD16 gi2_inter_default_rescale_mat_32x32[];

#endif
