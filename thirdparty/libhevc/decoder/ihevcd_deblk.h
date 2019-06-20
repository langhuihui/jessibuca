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
*  ihevcd_deblk.h
*
* @brief
*
*
* @author
*  Srinivas T
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVCD_DEBLK_H_
#define _IHEVCD_DEBLK_H_

void ihevcd_deblk_ctb(deblk_ctxt_t *ps_deblk,
                      WORD32 i4_is_last_ctb_x,
                      WORD32 i4_is_last_ctb_y);


#endif /*_IHEVC_DEBLK_H_*/
