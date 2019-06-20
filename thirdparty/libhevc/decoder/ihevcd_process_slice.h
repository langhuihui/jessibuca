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
*  ihevcd_parse_slice.h
*
* @brief
*  Processing of slice level data
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

#ifndef _IHEVCD_PROCESS_SLICE_H_
#define _IHEVCD_PROCESS_SLICE_H_

IHEVCD_ERROR_T ihevcd_process(process_ctxt_t *ps_proc);
void ihevcd_init_proc_ctxt(process_ctxt_t *ps_proc, WORD32 tu_coeff_data_ofst);
void ihevcd_process_thread(process_ctxt_t *ps_proc);

#endif /* _IHEVCD_PROCESS_SLICE_H_ */
