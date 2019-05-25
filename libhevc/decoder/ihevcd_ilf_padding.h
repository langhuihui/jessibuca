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
*  ihevc_ilf_padding_frame.h
*
* @brief
*  Does frame level loop filtering (deblocking and SAO) and padding
*
* @author
*  Srinivas T
*
* @par List of Functions:
*   - ihevc_ilf_pad_frame()
*
* @remarks
*  None
*
*******************************************************************************
*/

#ifndef IHEVCD_ILF_PADDING_H_
#define IHEVCD_ILF_PADDING_H_

void ihevcd_ilf_pad_frame(deblk_ctxt_t *ps_deblk_ctxt, sao_ctxt_t *ps_sao_ctxt);


#endif /* IHEVCD_ILF_PADDING_H_ */

