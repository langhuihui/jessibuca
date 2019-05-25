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
*  ihevcd_profile.h
*
* @brief
*  Contains macros for profiling individual modules of decoder
*
* @author
*  Naveen SR
*
* @par List of Functions:
*
* @remarks
*  None
*
*******************************************************************************
*/

#ifndef _IHEVCD_PROFILE_H_
#define _IHEVCD_PROFILE_H_

#include "ihevc_defs.h"
/* Define return; to disable individual module */
#ifdef PROFILE_DIS_SAO_LEAF_LEVEL
#define PROFILE_DISABLE_SAO_LEAF_LEVEL() return;
#else
#define PROFILE_DISABLE_SAO_LEAF_LEVEL() ;
#endif

#ifdef PROFILE_DIS_SAO
#define PROFILE_DISABLE_SAO() return;
#else
#define PROFILE_DISABLE_SAO() ;
#endif

#ifdef PROFILE_DIS_DEBLK
#define PROFILE_DISABLE_DEBLK() return;
#else
#define PROFILE_DISABLE_DEBLK() ;
#endif

#ifdef PROFILE_DIS_IQ_IT_RECON_INTRA_PRED
#define PROFILE_DISABLE_IQ_IT_RECON_INTRA_PRED() return;
#else
#define PROFILE_DISABLE_IQ_IT_RECON_INTRA_PRED() ;
#endif

#ifdef PROFILE_DIS_INTER_PRED
#define PROFILE_DISABLE_INTER_PRED() return;
#else
#define PROFILE_DISABLE_INTER_PRED() ;
#endif

#ifdef PROFILE_DIS_PROCESS_CTB
#define PROFILE_DISABLE_PROCESS_CTB() return;
/* When processing is disabled, no point in format converion as well */
#define PROFILE_DISABLE_FMT_CONV() return 0;
#else
#define PROFILE_DISABLE_PROCESS_CTB() ;
#define PROFILE_DISABLE_FMT_CONV();
#endif

#ifdef PROFILE_DIS_BOUNDARY_STRENGTH
#define PROFILE_DISABLE_BOUNDARY_STRENGTH() return;
#else
#define PROFILE_DISABLE_BOUNDARY_STRENGTH() ;
#endif

#ifdef PROFILE_DIS_MV_PREDICTION
#define PROFILE_DISABLE_MV_PREDICTION() return;
#else
#define PROFILE_DISABLE_MV_PREDICTION() ;
#endif

//#define PROFILE_DISABLE_INTER_PRED_LUMA(clr_indx) {if(clr_indx == 0) continue;}
//#define PROFILE_DISABLE_INTER_PRED_CHROMA(clr_indx) {if(clr_indx == 1) continue;}
//#define PROFILE_DISABLE_INTER_PRED_LUMA_AVERAGING(clr_indx) {if(clr_indx == 0) continue;}
//#define PROFILE_DISABLE_INTER_PRED_CHROMA_AVERAGING(clr_indx) {if(clr_indx == 1) continue;}

#define PROFILE_DISABLE_INTER_PRED_LUMA(clr_indx) ;
#define PROFILE_DISABLE_INTER_PRED_CHROMA(clr_indx) ;
#define PROFILE_DISABLE_INTER_PRED_LUMA_AVERAGING(clr_indx) ;
#define PROFILE_DISABLE_INTER_PRED_CHROMA_AVERAGING(clr_indx) ;

#endif /* _IHEVCD_PROFILE_H_ */
