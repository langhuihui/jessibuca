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
*  ihevcd_bitps_bitstrm.h
*
* @brief
*  Header for bitps_bitstrm access functions
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

#ifndef IHEVCD_BOUNDARY_STRENGTH_H_
#define IHEVCD_BOUNDARY_STRENGTH_H_

WORD32 ihevcd_ctb_boundary_strength_islice(bs_ctxt_t *ps_bs_ctxt);

WORD32 ihevcd_ctb_boundary_strength_pbslice(bs_ctxt_t *ps_bs_ctxt);

WORD32 ihevcd_pu_boundary_strength(pu_t *ps_pu,
                                   pu_t *ps_ngbr_pu);



#endif /* IHEVCD_BOUNDARY_STRENGTH_H_ */
