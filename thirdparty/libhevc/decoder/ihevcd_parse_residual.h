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
*  ihevcd_parse_residual.h
*
* @brief
*  Parsing of residual data
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

#ifndef _IHEVCD_PARSE_RESIDUAL_H_
#define _IHEVCD_PARSE_RESIDUAL_H_
WORD32 ihevcd_parse_residual_coding(codec_t *ps_codec,
                                    WORD32 x0, WORD32 y0,
                                    WORD32 log2_trafo_size,
                                    WORD32 c_idx,
                                    WORD32 intra_pred_mode);

#endif /* _IHEVCD_PARSE_RESIDUAL_H_ */
