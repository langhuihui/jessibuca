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
*  ihevcd_itrans_recon.h
*
* @brief
*  Header for itrans recon dc functions
*
* @author
*  Naveen
*
* @par List of Functions:
*
* @remarks
*  None
*
*******************************************************************************
*/

#ifndef _IHEVCD_ITRANS_RECON_DC_H_
#define _IHEVCD_ITRANS_RECON_DC_H_

typedef void ihevcd_itrans_recon_dc_luma_ft(UWORD8 *pu1_pred,
                                            UWORD8 *pu1_dst,
                                            WORD32 pred_strd,
                                            WORD32 dst_strd,
                                            WORD32 log2_trans_size,
                                            WORD16 i2_coeff_value);
typedef void ihevcd_itrans_recon_dc_chroma_ft(UWORD8 *pu1_pred,
                                              UWORD8 *pu1_dst,
                                              WORD32 pred_strd,
                                              WORD32 dst_strd,
                                              WORD32 log2_trans_size,
                                              WORD16 i2_coeff_value);

/* C function declarations */
ihevcd_itrans_recon_dc_luma_ft ihevcd_itrans_recon_dc_luma;
ihevcd_itrans_recon_dc_chroma_ft ihevcd_itrans_recon_dc_chroma;

/* A9Q function declarations */
ihevcd_itrans_recon_dc_luma_ft ihevcd_itrans_recon_dc_luma_a9q;
ihevcd_itrans_recon_dc_chroma_ft ihevcd_itrans_recon_dc_chroma_a9q;

/* A9A function declarations */
ihevcd_itrans_recon_dc_luma_ft ihevcd_itrans_recon_dc_luma_a9a;
ihevcd_itrans_recon_dc_chroma_ft ihevcd_itrans_recon_dc_chroma_a9a;

/* SSSE3 function declarations */
ihevcd_itrans_recon_dc_luma_ft ihevcd_itrans_recon_dc_luma_ssse3;
ihevcd_itrans_recon_dc_chroma_ft ihevcd_itrans_recon_dc_chroma_ssse3;

/* SSS4.2 function declarations */
ihevcd_itrans_recon_dc_luma_ft ihevcd_itrans_recon_dc_luma_sse42;
ihevcd_itrans_recon_dc_chroma_ft ihevcd_itrans_recon_dc_chroma_sse42;

/* armv8 function declarations */
ihevcd_itrans_recon_dc_luma_ft ihevcd_itrans_recon_dc_luma_av8;
ihevcd_itrans_recon_dc_chroma_ft ihevcd_itrans_recon_dc_chroma_av8;

#endif /* _IHEVCD_ITRANS_RECON_DC_H_ */
