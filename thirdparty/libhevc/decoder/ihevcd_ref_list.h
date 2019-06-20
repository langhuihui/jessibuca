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
*  ihevcd_ref_list.h
*
* @brief
*  Contains functions definitions for reference list generation
*
* @author
*  Srinivas T
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVCD_REF_LIST_H_
#define _IHEVCD_REF_LIST_H_

WORD32 ihevcd_ref_list(codec_t *ps_codec, pps_t *ps_pps, sps_t *ps_sps, slice_header_t *ps_slice_hdr);

#endif
