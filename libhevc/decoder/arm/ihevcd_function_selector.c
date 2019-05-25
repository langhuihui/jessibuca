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
*  ihevcd_function_selector.c
*
* @brief
*  Contains functions to initialize function pointers used in hevc
*
* @author
*  Naveen
*
* @par List of Functions:
* @remarks
*  None
*
*******************************************************************************
*/
/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ihevc_typedefs.h"
#include "iv.h"
#include "ivd.h"
#include "ihevc_defs.h"
#include "ihevc_debug.h"
#include "ihevc_structs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_cabac_tables.h"
#include "ihevc_disp_mgr.h"
#include "ihevc_buf_mgr.h"
#include "ihevc_dpb_mgr.h"
#include "ihevc_error.h"

#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"

void ihevcd_init_function_ptr_neonintr(codec_t *ps_codec);
void ihevcd_init_function_ptr_noneon(codec_t *ps_codec);
void ihevcd_init_function_ptr_a9q(codec_t *ps_codec);
void ihevcd_init_function_ptr_av8(codec_t *ps_codec);
void ihevcd_init_function_ptr(void *pv_codec)
{
    codec_t *ps_codec = (codec_t *)pv_codec;

#ifndef ARMV8
    switch(ps_codec->e_processor_arch)
    {
#ifndef DISABLE_NEONINTR
        case ARCH_ARM_NEONINTR:
            ihevcd_init_function_ptr_neonintr(ps_codec);
            break;
#endif
        case ARCH_ARM_NONEON:
            ihevcd_init_function_ptr_noneon(ps_codec);
            break;
        default:
        case ARCH_ARM_A5:
        case ARCH_ARM_A7:
        case ARCH_ARM_A9:
        case ARCH_ARM_A15:
        case ARCH_ARM_A9Q:
#ifndef DISABLE_NEON
            ihevcd_init_function_ptr_a9q(ps_codec);
#else
            ihevcd_init_function_ptr_noneon(ps_codec);
#endif
            break;
    }
    switch(ps_codec->e_processor_soc)
    {

        case SOC_HISI_37X:
#ifndef DISABLE_NEON
            ps_codec->s_func_selector.ihevcd_fmt_conv_420sp_to_420sp_fptr               =  &ihevcd_fmt_conv_420sp_to_420sp_a9q;
#endif
            break;
        case SOC_GENERIC:
        default:
            break;
    }
#else
    switch(ps_codec->e_processor_arch)
    {
        case ARCH_ARM_NONEON:
            ihevcd_init_function_ptr_noneon(ps_codec);
            break;
        case ARCH_ARMV8_GENERIC:
        default:
            ihevcd_init_function_ptr_av8(ps_codec);
            break;
    }
#endif
}

void ihevcd_init_arch(void *pv_codec)
{
    codec_t *ps_codec = (codec_t *)pv_codec;
#ifdef DEFAULT_ARCH
#if DEFAULT_ARCH == D_ARCH_ARM_NONEON
    ps_codec->e_processor_arch = ARCH_ARM_NONEON;
#elif DEFAULT_ARCH == D_ARCH_ARMV8_GENERIC
    ps_codec->e_processor_arch = ARCH_ARMV8_GENERIC;
#elif DEFAULT_ARCH == D_ARCH_ARM_NEONINTR
    ps_codec->e_processor_arch = ARCH_ARM_NEONINTR;
#else
    ps_codec->e_processor_arch = ARCH_ARM_A9Q;
#endif
#else
    ps_codec->e_processor_arch = ARCH_ARM_A9Q;
#endif
}
