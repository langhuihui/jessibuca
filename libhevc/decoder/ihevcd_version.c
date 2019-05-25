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
*  ihevcd_version.c
*
* @brief
*  Contains version info for HEVC decoder
*
* @author
*  Harish
*
* @par List of Functions:
* - ihevcd_get_version()
*
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
#include "ihevcd_cxa.h"

#include "ihevc_defs.h"
#include "ihevc_debug.h"
#include "ihevc_structs.h"
/**
 * Name of the codec
 */
#define CODEC_NAME              "HEVCDEC"
/**
 * Codec release type, production or evaluation
 */
#define CODEC_RELEASE_TYPE      "production"
/**
 * Version string. First two digits signify major version and last two minor
 * Increment major version for API change or major feature update
 */
#define CODEC_RELEASE_VER       "05.00"
/**
 * Vendor name
 */
#define CODEC_VENDOR            "ITTIAM"

/**
*******************************************************************************
* Concatenates various strings to form a version string
*******************************************************************************
*/
#define MAXVERSION_STRLEN       511
#ifdef __ANDROID__
#define VERSION(version_string, codec_name, codec_release_type, codec_release_ver, codec_vendor)    \
    snprintf(version_string, MAXVERSION_STRLEN,                                                     \
             "@(#)Id:%s_%s Ver:%s Released by %s",                                                  \
             codec_name, codec_release_type, codec_release_ver, codec_vendor)
#else
#define VERSION(version_string, codec_name, codec_release_type, codec_release_ver, codec_vendor)    \
    snprintf(version_string, MAXVERSION_STRLEN,                                                     \
             "@(#)Id:%s_%s Ver:%s Released by %s Build: %s @ %s",                                   \
             codec_name, codec_release_type, codec_release_ver, codec_vendor, __DATE__, __TIME__)
#endif

/**
*******************************************************************************
*
* @brief
*  Fills the version info in the given string
*
* @par Description:
*
*
* @param[in] pc_version_string
*  Pointer to hold version info
*
* @param[in] u4_version_buffer_size
*  Size of the buffer passed
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
IV_API_CALL_STATUS_T ihevcd_get_version(CHAR *pc_version_string,
                                        UWORD32 u4_version_buffer_size)
{
    CHAR ac_version_tmp[MAXVERSION_STRLEN + 1];
    UWORD32 u4_len;
    VERSION(ac_version_tmp, CODEC_NAME, CODEC_RELEASE_TYPE, CODEC_RELEASE_VER, CODEC_VENDOR);
    u4_len = strnlen(ac_version_tmp, MAXVERSION_STRLEN) + 1;
    if(u4_version_buffer_size >= u4_len)
    {
        memcpy(pc_version_string, ac_version_tmp, u4_len);
        return IV_SUCCESS;
    }
    else
    {
        return IV_FAIL;
    }

}


