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
*  ihevcd_api.c
*
* @brief
*  Contains api functions definitions for HEVC decoder
*
* @author
*  Harish
*
* @par List of Functions:
* - api_check_struct_sanity()
* - ihevcd_get_version()
* - ihevcd_set_default_params()
* - ihevcd_init()
* - ihevcd_get_num_rec()
* - ihevcd_allocate_static_bufs()
* - ihevcd_create()
* - ihevcd_retrieve_memrec()
* - ihevcd_set_display_frame()
* - ihevcd_set_flush_mode()
* - ihevcd_get_status()
* - ihevcd_get_buf_info()
* - ihevcd_set_params()
* - ihevcd_reset()
* - ihevcd_rel_display_frame()
* - ihevcd_disable_deblk()
* - ihevcd_get_frame_dimensions()
* - ihevcd_set_num_cores()
* - ihevcd_ctl()
* - ihevcd_cxa_api_function()
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
#include "ithread.h"

#include "ihevc_defs.h"
#include "ihevc_debug.h"

#include "ihevc_structs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"

#include "ihevc_buf_mgr.h"
#include "ihevc_dpb_mgr.h"
#include "ihevc_disp_mgr.h"
#include "ihevc_common_tables.h"
#include "ihevc_cabac_tables.h"
#include "ihevc_error.h"

#include "ihevcd_defs.h"
#include "ihevcd_trace.h"

#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_utils.h"
#include "ihevcd_decode.h"
#include "ihevcd_job_queue.h"
#include "ihevcd_statistics.h"


#define ALIGNED_FREE(ps_codec, y) \
if(y) {ps_codec->pf_aligned_free(ps_codec->pv_mem_ctxt, ((void *)y)); (y) = NULL;}

/*****************************************************************************/
/* Function Prototypes                                                       */
/*****************************************************************************/
IV_API_CALL_STATUS_T ihevcd_get_version(CHAR *pc_version_string,
                                        UWORD32 u4_version_buffer_size);
WORD32 ihevcd_free_dynamic_bufs(codec_t *ps_codec);


/**
*******************************************************************************
*
* @brief
*  Used to test arguments for corresponding API call
*
* @par Description:
*  For each command the arguments are validated
*
* @param[in] ps_handle
*  Codec handle at API level
*
* @param[in] pv_api_ip
*  Pointer to input structure
*
* @param[out] pv_api_op
*  Pointer to output structure
*
* @returns  Status of error checking
*
* @remarks
*
*
*******************************************************************************
*/

static IV_API_CALL_STATUS_T api_check_struct_sanity(iv_obj_t *ps_handle,
                                                    void *pv_api_ip,
                                                    void *pv_api_op)
{
    IVD_API_COMMAND_TYPE_T e_cmd;
    UWORD32 *pu4_api_ip;
    UWORD32 *pu4_api_op;
    WORD32 i;

    if(NULL == pv_api_op)
        return (IV_FAIL);

    if(NULL == pv_api_ip)
        return (IV_FAIL);

    pu4_api_ip = (UWORD32 *)pv_api_ip;
    pu4_api_op = (UWORD32 *)pv_api_op;
    e_cmd = (IVD_API_COMMAND_TYPE_T)*(pu4_api_ip + 1);

    *(pu4_api_op + 1) = 0;
    /* error checks on handle */
    switch((WORD32)e_cmd)
    {
        case IVD_CMD_CREATE:
            break;

        case IVD_CMD_REL_DISPLAY_FRAME:
        case IVD_CMD_SET_DISPLAY_FRAME:
        case IVD_CMD_GET_DISPLAY_FRAME:
        case IVD_CMD_VIDEO_DECODE:
        case IVD_CMD_DELETE:
        case IVD_CMD_VIDEO_CTL:
            if(ps_handle == NULL)
            {
                *(pu4_api_op + 1) |= 1 << IVD_UNSUPPORTEDPARAM;
                *(pu4_api_op + 1) |= IVD_HANDLE_NULL;
                return IV_FAIL;
            }

            if(ps_handle->u4_size != sizeof(iv_obj_t))
            {
                *(pu4_api_op + 1) |= 1 << IVD_UNSUPPORTEDPARAM;
                *(pu4_api_op + 1) |= IVD_HANDLE_STRUCT_SIZE_INCORRECT;
                return IV_FAIL;
            }


            if(ps_handle->pv_codec_handle == NULL)
            {
                *(pu4_api_op + 1) |= 1 << IVD_UNSUPPORTEDPARAM;
                *(pu4_api_op + 1) |= IVD_INVALID_HANDLE_NULL;
                return IV_FAIL;
            }
            break;
        default:
            *(pu4_api_op + 1) |= 1 << IVD_UNSUPPORTEDPARAM;
            *(pu4_api_op + 1) |= IVD_INVALID_API_CMD;
            return IV_FAIL;
    }

    switch((WORD32)e_cmd)
    {
        case IVD_CMD_CREATE:
        {
            ihevcd_cxa_create_ip_t *ps_ip = (ihevcd_cxa_create_ip_t *)pv_api_ip;
            ihevcd_cxa_create_op_t *ps_op = (ihevcd_cxa_create_op_t *)pv_api_op;


            ps_op->s_ivd_create_op_t.u4_error_code = 0;

            if((ps_ip->s_ivd_create_ip_t.u4_size > sizeof(ihevcd_cxa_create_ip_t))
                            || (ps_ip->s_ivd_create_ip_t.u4_size
                                            < sizeof(ivd_create_ip_t)))
            {
                ps_op->s_ivd_create_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_create_op_t.u4_error_code |=
                                IVD_IP_API_STRUCT_SIZE_INCORRECT;

                return (IV_FAIL);
            }

            if((ps_op->s_ivd_create_op_t.u4_size != sizeof(ihevcd_cxa_create_op_t))
                            && (ps_op->s_ivd_create_op_t.u4_size
                                            != sizeof(ivd_create_op_t)))
            {
                ps_op->s_ivd_create_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_create_op_t.u4_error_code |=
                                IVD_OP_API_STRUCT_SIZE_INCORRECT;

                return (IV_FAIL);
            }


            if((ps_ip->s_ivd_create_ip_t.e_output_format != IV_YUV_420P)
                            && (ps_ip->s_ivd_create_ip_t.e_output_format
                                            != IV_YUV_422ILE)
                            && (ps_ip->s_ivd_create_ip_t.e_output_format
                                            != IV_RGB_565)
                            && (ps_ip->s_ivd_create_ip_t.e_output_format
                                            != IV_YUV_420SP_UV)
                            && (ps_ip->s_ivd_create_ip_t.e_output_format
                                            != IV_YUV_420SP_VU))
            {
                ps_op->s_ivd_create_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_create_op_t.u4_error_code |=
                                IVD_INIT_DEC_COL_FMT_NOT_SUPPORTED;

                return (IV_FAIL);
            }

        }
            break;

        case IVD_CMD_GET_DISPLAY_FRAME:
        {
            ihevcd_cxa_get_display_frame_ip_t *ps_ip =
                            (ihevcd_cxa_get_display_frame_ip_t *)pv_api_ip;
            ihevcd_cxa_get_display_frame_op_t *ps_op =
                            (ihevcd_cxa_get_display_frame_op_t *)pv_api_op;

            ps_op->s_ivd_get_display_frame_op_t.u4_error_code = 0;

            if((ps_ip->s_ivd_get_display_frame_ip_t.u4_size
                            != sizeof(ihevcd_cxa_get_display_frame_ip_t))
                            && (ps_ip->s_ivd_get_display_frame_ip_t.u4_size
                                            != sizeof(ivd_get_display_frame_ip_t)))
            {
                ps_op->s_ivd_get_display_frame_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_get_display_frame_op_t.u4_error_code |=
                                IVD_IP_API_STRUCT_SIZE_INCORRECT;
                return (IV_FAIL);
            }

            if((ps_op->s_ivd_get_display_frame_op_t.u4_size
                            != sizeof(ihevcd_cxa_get_display_frame_op_t))
                            && (ps_op->s_ivd_get_display_frame_op_t.u4_size
                                            != sizeof(ivd_get_display_frame_op_t)))
            {
                ps_op->s_ivd_get_display_frame_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_get_display_frame_op_t.u4_error_code |=
                                IVD_OP_API_STRUCT_SIZE_INCORRECT;
                return (IV_FAIL);
            }

        }
            break;

        case IVD_CMD_REL_DISPLAY_FRAME:
        {
            ihevcd_cxa_rel_display_frame_ip_t *ps_ip =
                            (ihevcd_cxa_rel_display_frame_ip_t *)pv_api_ip;
            ihevcd_cxa_rel_display_frame_op_t *ps_op =
                            (ihevcd_cxa_rel_display_frame_op_t *)pv_api_op;

            ps_op->s_ivd_rel_display_frame_op_t.u4_error_code = 0;

            if((ps_ip->s_ivd_rel_display_frame_ip_t.u4_size
                            != sizeof(ihevcd_cxa_rel_display_frame_ip_t))
                            && (ps_ip->s_ivd_rel_display_frame_ip_t.u4_size
                                            != sizeof(ivd_rel_display_frame_ip_t)))
            {
                ps_op->s_ivd_rel_display_frame_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_rel_display_frame_op_t.u4_error_code |=
                                IVD_IP_API_STRUCT_SIZE_INCORRECT;
                return (IV_FAIL);
            }

            if((ps_op->s_ivd_rel_display_frame_op_t.u4_size
                            != sizeof(ihevcd_cxa_rel_display_frame_op_t))
                            && (ps_op->s_ivd_rel_display_frame_op_t.u4_size
                                            != sizeof(ivd_rel_display_frame_op_t)))
            {
                ps_op->s_ivd_rel_display_frame_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_rel_display_frame_op_t.u4_error_code |=
                                IVD_OP_API_STRUCT_SIZE_INCORRECT;
                return (IV_FAIL);
            }

        }
            break;

        case IVD_CMD_SET_DISPLAY_FRAME:
        {
            ihevcd_cxa_set_display_frame_ip_t *ps_ip =
                            (ihevcd_cxa_set_display_frame_ip_t *)pv_api_ip;
            ihevcd_cxa_set_display_frame_op_t *ps_op =
                            (ihevcd_cxa_set_display_frame_op_t *)pv_api_op;
            UWORD32 j;

            ps_op->s_ivd_set_display_frame_op_t.u4_error_code = 0;

            if((ps_ip->s_ivd_set_display_frame_ip_t.u4_size
                            != sizeof(ihevcd_cxa_set_display_frame_ip_t))
                            && (ps_ip->s_ivd_set_display_frame_ip_t.u4_size
                                            != sizeof(ivd_set_display_frame_ip_t)))
            {
                ps_op->s_ivd_set_display_frame_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_set_display_frame_op_t.u4_error_code |=
                                IVD_IP_API_STRUCT_SIZE_INCORRECT;
                return (IV_FAIL);
            }

            if((ps_op->s_ivd_set_display_frame_op_t.u4_size
                            != sizeof(ihevcd_cxa_set_display_frame_op_t))
                            && (ps_op->s_ivd_set_display_frame_op_t.u4_size
                                            != sizeof(ivd_set_display_frame_op_t)))
            {
                ps_op->s_ivd_set_display_frame_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_set_display_frame_op_t.u4_error_code |=
                                IVD_OP_API_STRUCT_SIZE_INCORRECT;
                return (IV_FAIL);
            }

            if(ps_ip->s_ivd_set_display_frame_ip_t.num_disp_bufs == 0)
            {
                ps_op->s_ivd_set_display_frame_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_set_display_frame_op_t.u4_error_code |=
                                IVD_DISP_FRM_ZERO_OP_BUFS;
                return IV_FAIL;
            }

            for(j = 0; j < ps_ip->s_ivd_set_display_frame_ip_t.num_disp_bufs;
                            j++)
            {
                if(ps_ip->s_ivd_set_display_frame_ip_t.s_disp_buffer[j].u4_num_bufs
                                == 0)
                {
                    ps_op->s_ivd_set_display_frame_op_t.u4_error_code |= 1
                                    << IVD_UNSUPPORTEDPARAM;
                    ps_op->s_ivd_set_display_frame_op_t.u4_error_code |=
                                    IVD_DISP_FRM_ZERO_OP_BUFS;
                    return IV_FAIL;
                }

                for(i = 0;
                                i
                                                < (WORD32)ps_ip->s_ivd_set_display_frame_ip_t.s_disp_buffer[j].u4_num_bufs;
                                i++)
                {
                    if(ps_ip->s_ivd_set_display_frame_ip_t.s_disp_buffer[j].pu1_bufs[i]
                                    == NULL)
                    {
                        ps_op->s_ivd_set_display_frame_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_set_display_frame_op_t.u4_error_code |=
                                        IVD_DISP_FRM_OP_BUF_NULL;
                        return IV_FAIL;
                    }

                    if(ps_ip->s_ivd_set_display_frame_ip_t.s_disp_buffer[j].u4_min_out_buf_size[i]
                                    == 0)
                    {
                        ps_op->s_ivd_set_display_frame_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_set_display_frame_op_t.u4_error_code |=
                                        IVD_DISP_FRM_ZERO_OP_BUF_SIZE;
                        return IV_FAIL;
                    }
                }
            }
        }
            break;

        case IVD_CMD_VIDEO_DECODE:
        {
            ihevcd_cxa_video_decode_ip_t *ps_ip =
                            (ihevcd_cxa_video_decode_ip_t *)pv_api_ip;
            ihevcd_cxa_video_decode_op_t *ps_op =
                            (ihevcd_cxa_video_decode_op_t *)pv_api_op;

            DEBUG("The input bytes is: %d",
                            ps_ip->s_ivd_video_decode_ip_t.u4_num_Bytes);
            ps_op->s_ivd_video_decode_op_t.u4_error_code = 0;

            if(ps_ip->s_ivd_video_decode_ip_t.u4_size
                            != sizeof(ihevcd_cxa_video_decode_ip_t)
                            && ps_ip->s_ivd_video_decode_ip_t.u4_size
                                            != offsetof(ivd_video_decode_ip_t,
                                                        s_out_buffer))
            {
                ps_op->s_ivd_video_decode_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_video_decode_op_t.u4_error_code |=
                                IVD_IP_API_STRUCT_SIZE_INCORRECT;
                return (IV_FAIL);
            }

            if(ps_op->s_ivd_video_decode_op_t.u4_size
                            != sizeof(ihevcd_cxa_video_decode_op_t)
                            && ps_op->s_ivd_video_decode_op_t.u4_size
                                            != offsetof(ivd_video_decode_op_t,
                                                        u4_output_present))
            {
                ps_op->s_ivd_video_decode_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_video_decode_op_t.u4_error_code |=
                                IVD_OP_API_STRUCT_SIZE_INCORRECT;
                return (IV_FAIL);
            }

        }
            break;

        case IVD_CMD_DELETE:
        {
            ihevcd_cxa_delete_ip_t *ps_ip =
                            (ihevcd_cxa_delete_ip_t *)pv_api_ip;
            ihevcd_cxa_delete_op_t *ps_op =
                            (ihevcd_cxa_delete_op_t *)pv_api_op;

            ps_op->s_ivd_delete_op_t.u4_error_code = 0;

            if(ps_ip->s_ivd_delete_ip_t.u4_size
                            != sizeof(ihevcd_cxa_delete_ip_t))
            {
                ps_op->s_ivd_delete_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_delete_op_t.u4_error_code |=
                                IVD_IP_API_STRUCT_SIZE_INCORRECT;
                return (IV_FAIL);
            }

            if(ps_op->s_ivd_delete_op_t.u4_size
                            != sizeof(ihevcd_cxa_delete_op_t))
            {
                ps_op->s_ivd_delete_op_t.u4_error_code |= 1
                                << IVD_UNSUPPORTEDPARAM;
                ps_op->s_ivd_delete_op_t.u4_error_code |=
                                IVD_OP_API_STRUCT_SIZE_INCORRECT;
                return (IV_FAIL);
            }

        }
            break;

        case IVD_CMD_VIDEO_CTL:
        {
            UWORD32 *pu4_ptr_cmd;
            UWORD32 sub_command;

            pu4_ptr_cmd = (UWORD32 *)pv_api_ip;
            pu4_ptr_cmd += 2;
            sub_command = *pu4_ptr_cmd;

            switch(sub_command)
            {
                case IVD_CMD_CTL_SETPARAMS:
                {
                    ihevcd_cxa_ctl_set_config_ip_t *ps_ip;
                    ihevcd_cxa_ctl_set_config_op_t *ps_op;
                    ps_ip = (ihevcd_cxa_ctl_set_config_ip_t *)pv_api_ip;
                    ps_op = (ihevcd_cxa_ctl_set_config_op_t *)pv_api_op;

                    if(ps_ip->s_ivd_ctl_set_config_ip_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_set_config_ip_t))
                    {
                        ps_op->s_ivd_ctl_set_config_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_set_config_op_t.u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                }
                    //no break; is needed here
                case IVD_CMD_CTL_SETDEFAULT:
                {
                    ihevcd_cxa_ctl_set_config_op_t *ps_op;
                    ps_op = (ihevcd_cxa_ctl_set_config_op_t *)pv_api_op;
                    if(ps_op->s_ivd_ctl_set_config_op_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_set_config_op_t))
                    {
                        ps_op->s_ivd_ctl_set_config_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_set_config_op_t.u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                }
                    break;

                case IVD_CMD_CTL_GETPARAMS:
                {
                    ihevcd_cxa_ctl_getstatus_ip_t *ps_ip;
                    ihevcd_cxa_ctl_getstatus_op_t *ps_op;

                    ps_ip = (ihevcd_cxa_ctl_getstatus_ip_t *)pv_api_ip;
                    ps_op = (ihevcd_cxa_ctl_getstatus_op_t *)pv_api_op;
                    if(ps_ip->s_ivd_ctl_getstatus_ip_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_getstatus_ip_t))
                    {
                        ps_op->s_ivd_ctl_getstatus_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_getstatus_op_t.u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                    if((ps_op->s_ivd_ctl_getstatus_op_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_getstatus_op_t)) &&
                       (ps_op->s_ivd_ctl_getstatus_op_t.u4_size
                                    != sizeof(ivd_ctl_getstatus_op_t)))
                    {
                        ps_op->s_ivd_ctl_getstatus_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_getstatus_op_t.u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                }
                    break;

                case IVD_CMD_CTL_GETBUFINFO:
                {
                    ihevcd_cxa_ctl_getbufinfo_ip_t *ps_ip;
                    ihevcd_cxa_ctl_getbufinfo_op_t *ps_op;
                    ps_ip = (ihevcd_cxa_ctl_getbufinfo_ip_t *)pv_api_ip;
                    ps_op = (ihevcd_cxa_ctl_getbufinfo_op_t *)pv_api_op;

                    if(ps_ip->s_ivd_ctl_getbufinfo_ip_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_getbufinfo_ip_t))
                    {
                        ps_op->s_ivd_ctl_getbufinfo_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_getbufinfo_op_t.u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                    if(ps_op->s_ivd_ctl_getbufinfo_op_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_getbufinfo_op_t))
                    {
                        ps_op->s_ivd_ctl_getbufinfo_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_getbufinfo_op_t.u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                }
                    break;

                case IVD_CMD_CTL_GETVERSION:
                {
                    ihevcd_cxa_ctl_getversioninfo_ip_t *ps_ip;
                    ihevcd_cxa_ctl_getversioninfo_op_t *ps_op;
                    ps_ip = (ihevcd_cxa_ctl_getversioninfo_ip_t *)pv_api_ip;
                    ps_op = (ihevcd_cxa_ctl_getversioninfo_op_t *)pv_api_op;
                    if(ps_ip->s_ivd_ctl_getversioninfo_ip_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_getversioninfo_ip_t))
                    {
                        ps_op->s_ivd_ctl_getversioninfo_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_getversioninfo_op_t.u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                    if(ps_op->s_ivd_ctl_getversioninfo_op_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_getversioninfo_op_t))
                    {
                        ps_op->s_ivd_ctl_getversioninfo_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_getversioninfo_op_t.u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                }
                    break;

                case IVD_CMD_CTL_FLUSH:
                {
                    ihevcd_cxa_ctl_flush_ip_t *ps_ip;
                    ihevcd_cxa_ctl_flush_op_t *ps_op;
                    ps_ip = (ihevcd_cxa_ctl_flush_ip_t *)pv_api_ip;
                    ps_op = (ihevcd_cxa_ctl_flush_op_t *)pv_api_op;
                    if(ps_ip->s_ivd_ctl_flush_ip_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_flush_ip_t))
                    {
                        ps_op->s_ivd_ctl_flush_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_flush_op_t.u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                    if(ps_op->s_ivd_ctl_flush_op_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_flush_op_t))
                    {
                        ps_op->s_ivd_ctl_flush_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_flush_op_t.u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                }
                    break;

                case IVD_CMD_CTL_RESET:
                {
                    ihevcd_cxa_ctl_reset_ip_t *ps_ip;
                    ihevcd_cxa_ctl_reset_op_t *ps_op;
                    ps_ip = (ihevcd_cxa_ctl_reset_ip_t *)pv_api_ip;
                    ps_op = (ihevcd_cxa_ctl_reset_op_t *)pv_api_op;
                    if(ps_ip->s_ivd_ctl_reset_ip_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_reset_ip_t))
                    {
                        ps_op->s_ivd_ctl_reset_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_reset_op_t.u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                    if(ps_op->s_ivd_ctl_reset_op_t.u4_size
                                    != sizeof(ihevcd_cxa_ctl_reset_op_t))
                    {
                        ps_op->s_ivd_ctl_reset_op_t.u4_error_code |= 1
                                        << IVD_UNSUPPORTEDPARAM;
                        ps_op->s_ivd_ctl_reset_op_t.u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }
                }
                    break;
                case IHEVCD_CXA_CMD_CTL_DEGRADE:
                {
                    ihevcd_cxa_ctl_degrade_ip_t *ps_ip;
                    ihevcd_cxa_ctl_degrade_op_t *ps_op;

                    ps_ip = (ihevcd_cxa_ctl_degrade_ip_t *)pv_api_ip;
                    ps_op = (ihevcd_cxa_ctl_degrade_op_t *)pv_api_op;

                    if(ps_ip->u4_size
                                    != sizeof(ihevcd_cxa_ctl_degrade_ip_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

                    if(ps_op->u4_size
                                    != sizeof(ihevcd_cxa_ctl_degrade_op_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

                    if((ps_ip->i4_degrade_pics < 0) ||
                       (ps_ip->i4_degrade_pics > 4) ||
                       (ps_ip->i4_nondegrade_interval < 0) ||
                       (ps_ip->i4_degrade_type < 0) ||
                       (ps_ip->i4_degrade_type > 15))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        return IV_FAIL;
                    }

                    break;
                }

                case IHEVCD_CXA_CMD_CTL_GET_BUFFER_DIMENSIONS:
                {
                    ihevcd_cxa_ctl_get_frame_dimensions_ip_t *ps_ip;
                    ihevcd_cxa_ctl_get_frame_dimensions_op_t *ps_op;

                    ps_ip =
                                    (ihevcd_cxa_ctl_get_frame_dimensions_ip_t *)pv_api_ip;
                    ps_op =
                                    (ihevcd_cxa_ctl_get_frame_dimensions_op_t *)pv_api_op;

                    if(ps_ip->u4_size
                                    != sizeof(ihevcd_cxa_ctl_get_frame_dimensions_ip_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

                    if(ps_op->u4_size
                                    != sizeof(ihevcd_cxa_ctl_get_frame_dimensions_op_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

                    break;
                }

                case IHEVCD_CXA_CMD_CTL_GET_VUI_PARAMS:
                {
                    ihevcd_cxa_ctl_get_vui_params_ip_t *ps_ip;
                    ihevcd_cxa_ctl_get_vui_params_op_t *ps_op;

                    ps_ip =
                                    (ihevcd_cxa_ctl_get_vui_params_ip_t *)pv_api_ip;
                    ps_op =
                                    (ihevcd_cxa_ctl_get_vui_params_op_t *)pv_api_op;

                    if(ps_ip->u4_size
                                    != sizeof(ihevcd_cxa_ctl_get_vui_params_ip_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

                    if(ps_op->u4_size
                                    != sizeof(ihevcd_cxa_ctl_get_vui_params_op_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

                    break;
                }
                case IHEVCD_CXA_CMD_CTL_GET_SEI_MASTERING_PARAMS:
                {
                    ihevcd_cxa_ctl_get_sei_mastering_params_ip_t *ps_ip;
                    ihevcd_cxa_ctl_get_sei_mastering_params_op_t *ps_op;

                    ps_ip = (ihevcd_cxa_ctl_get_sei_mastering_params_ip_t *)pv_api_ip;
                    ps_op = (ihevcd_cxa_ctl_get_sei_mastering_params_op_t *)pv_api_op;

                    if(ps_ip->u4_size
                                    != sizeof(ihevcd_cxa_ctl_get_sei_mastering_params_ip_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

                    if(ps_op->u4_size
                                    != sizeof(ihevcd_cxa_ctl_get_sei_mastering_params_op_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

                    break;
                }
                case IHEVCD_CXA_CMD_CTL_SET_NUM_CORES:
                {
                    ihevcd_cxa_ctl_set_num_cores_ip_t *ps_ip;
                    ihevcd_cxa_ctl_set_num_cores_op_t *ps_op;

                    ps_ip = (ihevcd_cxa_ctl_set_num_cores_ip_t *)pv_api_ip;
                    ps_op = (ihevcd_cxa_ctl_set_num_cores_op_t *)pv_api_op;

                    if(ps_ip->u4_size
                                    != sizeof(ihevcd_cxa_ctl_set_num_cores_ip_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

                    if(ps_op->u4_size
                                    != sizeof(ihevcd_cxa_ctl_set_num_cores_op_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

#ifdef MULTICORE
                    if((ps_ip->u4_num_cores < 1) || (ps_ip->u4_num_cores > MAX_NUM_CORES))
#else
                    if(ps_ip->u4_num_cores != 1)
#endif
                        {
                            ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                            return IV_FAIL;
                        }
                    break;
                }
                case IHEVCD_CXA_CMD_CTL_SET_PROCESSOR:
                {
                    ihevcd_cxa_ctl_set_processor_ip_t *ps_ip;
                    ihevcd_cxa_ctl_set_processor_op_t *ps_op;

                    ps_ip = (ihevcd_cxa_ctl_set_processor_ip_t *)pv_api_ip;
                    ps_op = (ihevcd_cxa_ctl_set_processor_op_t *)pv_api_op;

                    if(ps_ip->u4_size
                                    != sizeof(ihevcd_cxa_ctl_set_processor_ip_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_IP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

                    if(ps_op->u4_size
                                    != sizeof(ihevcd_cxa_ctl_set_processor_op_t))
                    {
                        ps_op->u4_error_code |= 1 << IVD_UNSUPPORTEDPARAM;
                        ps_op->u4_error_code |=
                                        IVD_OP_API_STRUCT_SIZE_INCORRECT;
                        return IV_FAIL;
                    }

                    break;
                }
                default:
                    *(pu4_api_op + 1) |= 1 << IVD_UNSUPPORTEDPARAM;
                    *(pu4_api_op + 1) |= IVD_UNSUPPORTED_API_CMD;
                    return IV_FAIL;
            }
        }
            break;
        default:
            *(pu4_api_op + 1) |= 1 << IVD_UNSUPPORTEDPARAM;
            *(pu4_api_op + 1) |= IVD_UNSUPPORTED_API_CMD;
            return IV_FAIL;
    }

    return IV_SUCCESS;
}


/**
*******************************************************************************
*
* @brief
*  Sets default dynamic parameters
*
* @par Description:
*  Sets default dynamic parameters. Will be called in ihevcd_init() to ensure
* that even if set_params is not called, codec  continues to work
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_set_default_params(codec_t *ps_codec)
{

    WORD32 ret = IV_SUCCESS;

    ps_codec->e_pic_skip_mode = IVD_SKIP_NONE;
    ps_codec->i4_strd = 0;
    ps_codec->i4_disp_strd = 0;
    ps_codec->i4_header_mode = 0;
    ps_codec->e_pic_out_order = IVD_DISPLAY_FRAME_OUT;
    return ret;
}

void ihevcd_update_function_ptr(codec_t *ps_codec)
{

    /* Init inter pred function array */
    ps_codec->apf_inter_pred[0] = NULL;
    ps_codec->apf_inter_pred[1] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_luma_copy_fptr;
    ps_codec->apf_inter_pred[2] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_luma_vert_fptr;
    ps_codec->apf_inter_pred[3] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_luma_horz_fptr;
    ps_codec->apf_inter_pred[4] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_luma_horz_w16out_fptr;
    ps_codec->apf_inter_pred[5] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_luma_copy_w16out_fptr;
    ps_codec->apf_inter_pred[6] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_luma_vert_w16out_fptr;
    ps_codec->apf_inter_pred[7] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_luma_horz_w16out_fptr;
    ps_codec->apf_inter_pred[8] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_luma_horz_w16out_fptr;
    ps_codec->apf_inter_pred[9] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_luma_vert_w16inp_fptr;
    ps_codec->apf_inter_pred[10] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_luma_vert_w16inp_w16out_fptr;
    ps_codec->apf_inter_pred[11] = NULL;
    ps_codec->apf_inter_pred[12] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_chroma_copy_fptr;
    ps_codec->apf_inter_pred[13] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_chroma_vert_fptr;
    ps_codec->apf_inter_pred[14] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_chroma_horz_fptr;
    ps_codec->apf_inter_pred[15] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_chroma_horz_w16out_fptr;
    ps_codec->apf_inter_pred[16] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_chroma_copy_w16out_fptr;
    ps_codec->apf_inter_pred[17] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_chroma_vert_w16out_fptr;
    ps_codec->apf_inter_pred[18] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_chroma_horz_w16out_fptr;
    ps_codec->apf_inter_pred[19] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_chroma_horz_w16out_fptr;
    ps_codec->apf_inter_pred[20] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_chroma_vert_w16inp_fptr;
    ps_codec->apf_inter_pred[21] = (pf_inter_pred)ps_codec->s_func_selector.ihevc_inter_pred_chroma_vert_w16inp_w16out_fptr;

    /* Init intra pred function array */
    ps_codec->apf_intra_pred_luma[0] = (pf_intra_pred)NULL;
    ps_codec->apf_intra_pred_luma[1] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_luma_planar_fptr;
    ps_codec->apf_intra_pred_luma[2] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_luma_dc_fptr;
    ps_codec->apf_intra_pred_luma[3] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_luma_mode2_fptr;
    ps_codec->apf_intra_pred_luma[4] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_luma_mode_3_to_9_fptr;
    ps_codec->apf_intra_pred_luma[5] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_luma_horz_fptr;
    ps_codec->apf_intra_pred_luma[6] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_luma_mode_11_to_17_fptr;
    ps_codec->apf_intra_pred_luma[7] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_luma_mode_18_34_fptr;
    ps_codec->apf_intra_pred_luma[8] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_luma_mode_19_to_25_fptr;
    ps_codec->apf_intra_pred_luma[9] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_luma_ver_fptr;
    ps_codec->apf_intra_pred_luma[10] =  (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_luma_mode_27_to_33_fptr;

    ps_codec->apf_intra_pred_chroma[0] = (pf_intra_pred)NULL;
    ps_codec->apf_intra_pred_chroma[1] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_chroma_planar_fptr;
    ps_codec->apf_intra_pred_chroma[2] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_chroma_dc_fptr;
    ps_codec->apf_intra_pred_chroma[3] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_chroma_mode2_fptr;
    ps_codec->apf_intra_pred_chroma[4] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_chroma_mode_3_to_9_fptr;
    ps_codec->apf_intra_pred_chroma[5] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_chroma_horz_fptr;
    ps_codec->apf_intra_pred_chroma[6] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_chroma_mode_11_to_17_fptr;
    ps_codec->apf_intra_pred_chroma[7] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_chroma_mode_18_34_fptr;
    ps_codec->apf_intra_pred_chroma[8] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_chroma_mode_19_to_25_fptr;
    ps_codec->apf_intra_pred_chroma[9] =  (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_chroma_ver_fptr;
    ps_codec->apf_intra_pred_chroma[10] = (pf_intra_pred)ps_codec->s_func_selector.ihevc_intra_pred_chroma_mode_27_to_33_fptr;

    /* Init itrans_recon function array */
    ps_codec->apf_itrans_recon[0] = (pf_itrans_recon)ps_codec->s_func_selector.ihevc_itrans_recon_4x4_ttype1_fptr;
    ps_codec->apf_itrans_recon[1] = (pf_itrans_recon)ps_codec->s_func_selector.ihevc_itrans_recon_4x4_fptr;
    ps_codec->apf_itrans_recon[2] = (pf_itrans_recon)ps_codec->s_func_selector.ihevc_itrans_recon_8x8_fptr;
    ps_codec->apf_itrans_recon[3] = (pf_itrans_recon)ps_codec->s_func_selector.ihevc_itrans_recon_16x16_fptr;
    ps_codec->apf_itrans_recon[4] = (pf_itrans_recon)ps_codec->s_func_selector.ihevc_itrans_recon_32x32_fptr;
    ps_codec->apf_itrans_recon[5] = (pf_itrans_recon)ps_codec->s_func_selector.ihevc_chroma_itrans_recon_4x4_fptr;
    ps_codec->apf_itrans_recon[6] = (pf_itrans_recon)ps_codec->s_func_selector.ihevc_chroma_itrans_recon_8x8_fptr;
    ps_codec->apf_itrans_recon[7] = (pf_itrans_recon)ps_codec->s_func_selector.ihevc_chroma_itrans_recon_16x16_fptr;

    /* Init recon function array */
    ps_codec->apf_recon[0] = (pf_recon)ps_codec->s_func_selector.ihevc_recon_4x4_ttype1_fptr;
    ps_codec->apf_recon[1] = (pf_recon)ps_codec->s_func_selector.ihevc_recon_4x4_fptr;
    ps_codec->apf_recon[2] = (pf_recon)ps_codec->s_func_selector.ihevc_recon_8x8_fptr;
    ps_codec->apf_recon[3] = (pf_recon)ps_codec->s_func_selector.ihevc_recon_16x16_fptr;
    ps_codec->apf_recon[4] = (pf_recon)ps_codec->s_func_selector.ihevc_recon_32x32_fptr;
    ps_codec->apf_recon[5] = (pf_recon)ps_codec->s_func_selector.ihevc_chroma_recon_4x4_fptr;
    ps_codec->apf_recon[6] = (pf_recon)ps_codec->s_func_selector.ihevc_chroma_recon_8x8_fptr;
    ps_codec->apf_recon[7] = (pf_recon)ps_codec->s_func_selector.ihevc_chroma_recon_16x16_fptr;

    /* Init itrans_recon_dc function array */
    ps_codec->apf_itrans_recon_dc[0] = (pf_itrans_recon_dc)ps_codec->s_func_selector.ihevcd_itrans_recon_dc_luma_fptr;
    ps_codec->apf_itrans_recon_dc[1] = (pf_itrans_recon_dc)ps_codec->s_func_selector.ihevcd_itrans_recon_dc_chroma_fptr;

    /* Init sao function array */
    ps_codec->apf_sao_luma[0] = (pf_sao_luma)ps_codec->s_func_selector.ihevc_sao_edge_offset_class0_fptr;
    ps_codec->apf_sao_luma[1] = (pf_sao_luma)ps_codec->s_func_selector.ihevc_sao_edge_offset_class1_fptr;
    ps_codec->apf_sao_luma[2] = (pf_sao_luma)ps_codec->s_func_selector.ihevc_sao_edge_offset_class2_fptr;
    ps_codec->apf_sao_luma[3] = (pf_sao_luma)ps_codec->s_func_selector.ihevc_sao_edge_offset_class3_fptr;

    ps_codec->apf_sao_chroma[0] = (pf_sao_chroma)ps_codec->s_func_selector.ihevc_sao_edge_offset_class0_chroma_fptr;
    ps_codec->apf_sao_chroma[1] = (pf_sao_chroma)ps_codec->s_func_selector.ihevc_sao_edge_offset_class1_chroma_fptr;
    ps_codec->apf_sao_chroma[2] = (pf_sao_chroma)ps_codec->s_func_selector.ihevc_sao_edge_offset_class2_chroma_fptr;
    ps_codec->apf_sao_chroma[3] = (pf_sao_chroma)ps_codec->s_func_selector.ihevc_sao_edge_offset_class3_chroma_fptr;
}
/**
*******************************************************************************
*
* @brief
*  Initialize the context. This will be called by  create and during
* reset
*
* @par Description:
*  Initializes the context
*
* @param[in] ps_codec
*  Codec context pointer
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_init(codec_t *ps_codec)
{
    WORD32 status = IV_SUCCESS;
    WORD32 i;

    /* Free any dynamic buffers that are allocated */
    ihevcd_free_dynamic_bufs(ps_codec);

    ps_codec->u4_allocate_dynamic_done = 0;
    ps_codec->i4_num_disp_bufs = 1;
    ps_codec->i4_flush_mode = 0;

    ps_codec->i4_ht = ps_codec->i4_disp_ht = 0;
    ps_codec->i4_wd = ps_codec->i4_disp_wd = 0;
    ps_codec->i4_strd = 0;
    ps_codec->i4_disp_strd = 0;
    ps_codec->i4_num_cores = 1;

    ps_codec->u4_pic_cnt = 0;
    ps_codec->u4_disp_cnt = 0;

    ps_codec->i4_header_mode = 0;
    ps_codec->i4_header_in_slice_mode = 0;
    ps_codec->i4_sps_done = 0;
    ps_codec->i4_pps_done = 0;
    ps_codec->i4_init_done   = 1;
    ps_codec->i4_first_pic_done = 0;
    ps_codec->s_parse.i4_first_pic_init = 0;
    ps_codec->i4_error_code = 0;
    ps_codec->i4_reset_flag = 0;
    ps_codec->i4_cra_as_first_pic = 1;
    ps_codec->i4_rasl_output_flag = 0;

    ps_codec->i4_prev_poc_msb = 0;
    ps_codec->i4_prev_poc_lsb = -1;
    ps_codec->i4_max_prev_poc_lsb = -1;
    ps_codec->s_parse.i4_abs_pic_order_cnt = -1;

    /* Set ref chroma format by default to 420SP UV interleaved */
    ps_codec->e_ref_chroma_fmt = IV_YUV_420SP_UV;

    /* If the codec is in shared mode and required format is 420 SP VU interleaved then change
     * reference buffers chroma format
     */
    if(IV_YUV_420SP_VU == ps_codec->e_chroma_fmt)
    {
        ps_codec->e_ref_chroma_fmt = IV_YUV_420SP_VU;
    }



    ps_codec->i4_disable_deblk_pic = 0;

    ps_codec->i4_degrade_pic_cnt    = 0;
    ps_codec->i4_degrade_pics       = 0;
    ps_codec->i4_degrade_type       = 0;
    ps_codec->i4_disable_sao_pic    = 0;
    ps_codec->i4_fullpel_inter_pred = 0;
    ps_codec->u4_enable_fmt_conv_ahead = 0;
    ps_codec->i4_share_disp_buf_cnt = 0;

    {
        sps_t *ps_sps = ps_codec->ps_sps_base;
        pps_t *ps_pps = ps_codec->ps_pps_base;

        for(i = 0; i < MAX_SPS_CNT; i++)
        {
            ps_sps->i1_sps_valid = 0;
            ps_sps++;
        }

        for(i = 0; i < MAX_PPS_CNT; i++)
        {
            ps_pps->i1_pps_valid = 0;
            ps_pps++;
        }
    }

    ihevcd_set_default_params(ps_codec);
    /* Initialize MV Bank buffer manager */
    ihevc_buf_mgr_init((buf_mgr_t *)ps_codec->pv_mv_buf_mgr);

    /* Initialize Picture buffer manager */
    ihevc_buf_mgr_init((buf_mgr_t *)ps_codec->pv_pic_buf_mgr);

    ps_codec->ps_pic_buf = (pic_buf_t *)ps_codec->pv_pic_buf_base;

    memset(ps_codec->ps_pic_buf, 0, BUF_MGR_MAX_CNT  * sizeof(pic_buf_t));



    /* Initialize display buffer manager */
    ihevc_disp_mgr_init((disp_mgr_t *)ps_codec->pv_disp_buf_mgr);

    /* Initialize dpb manager */
    ihevc_dpb_mgr_init((dpb_mgr_t *)ps_codec->pv_dpb_mgr);

    ps_codec->e_processor_soc = SOC_GENERIC;
    /* The following can be over-ridden using soc parameter as a hack */
    ps_codec->u4_nctb = 0x7FFFFFFF;
    ihevcd_init_arch(ps_codec);

    ihevcd_init_function_ptr(ps_codec);

    ihevcd_update_function_ptr(ps_codec);

    return status;
}

/**
*******************************************************************************
*
* @brief
*  Allocate static memory for the codec
*
* @par Description:
*  Allocates static memory for the codec
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_allocate_static_bufs(iv_obj_t **pps_codec_obj,
                                   ihevcd_cxa_create_ip_t *ps_create_ip,
                                   ihevcd_cxa_create_op_t *ps_create_op)
{
    WORD32 size;
    void *pv_buf;
    UWORD8 *pu1_buf;
    WORD32 i;
    codec_t *ps_codec;
    IV_API_CALL_STATUS_T status = IV_SUCCESS;
    void *(*pf_aligned_alloc)(void *pv_mem_ctxt, WORD32 alignment, WORD32 size);
    void (*pf_aligned_free)(void *pv_mem_ctxt, void *pv_buf);
    void *pv_mem_ctxt;

    /* Request memory for HEVCD object */
    ps_create_op->s_ivd_create_op_t.pv_handle = NULL;

    pf_aligned_alloc = ps_create_ip->s_ivd_create_ip_t.pf_aligned_alloc;
    pf_aligned_free = ps_create_ip->s_ivd_create_ip_t.pf_aligned_free;
    pv_mem_ctxt  = ps_create_ip->s_ivd_create_ip_t.pv_mem_ctxt;


    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, sizeof(iv_obj_t));
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    *pps_codec_obj = (iv_obj_t *)pv_buf;
    ps_create_op->s_ivd_create_op_t.pv_handle = *pps_codec_obj;


    (*pps_codec_obj)->pv_codec_handle = NULL;
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, sizeof(codec_t));
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    (*pps_codec_obj)->pv_codec_handle = (codec_t *)pv_buf;
    ps_codec = (codec_t *)pv_buf;

    memset(ps_codec, 0, sizeof(codec_t));

#ifndef LOGO_EN
    ps_codec->i4_share_disp_buf = ps_create_ip->s_ivd_create_ip_t.u4_share_disp_buf;
#else
    ps_codec->i4_share_disp_buf = 0;
#endif

    /* Shared display mode is supported only for 420SP and 420P formats */
    if((ps_create_ip->s_ivd_create_ip_t.e_output_format != IV_YUV_420P) &&
       (ps_create_ip->s_ivd_create_ip_t.e_output_format != IV_YUV_420SP_UV) &&
       (ps_create_ip->s_ivd_create_ip_t.e_output_format != IV_YUV_420SP_VU))
    {
        ps_codec->i4_share_disp_buf = 0;
    }

    ps_codec->e_chroma_fmt = ps_create_ip->s_ivd_create_ip_t.e_output_format;

    ps_codec->pf_aligned_alloc = pf_aligned_alloc;
    ps_codec->pf_aligned_free = pf_aligned_free;
    ps_codec->pv_mem_ctxt = pv_mem_ctxt;

    /* Request memory to hold thread handles for each processing thread */
    size = MAX_PROCESS_THREADS * ithread_get_handle_size();
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);

    for(i = 0; i < MAX_PROCESS_THREADS; i++)
    {
        WORD32 handle_size = ithread_get_handle_size();
        ps_codec->apv_process_thread_handle[i] =
                        (UWORD8 *)pv_buf + (i * handle_size);
    }

    /* Request memory for static bitstream buffer which holds bitstream after emulation prevention */
    size = MIN_BITSBUF_SIZE;
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size + 16); //Alloc extra for parse optimization
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    ps_codec->pu1_bitsbuf_static = pv_buf;
    ps_codec->u4_bitsbuf_size_static = size;

    /* size for holding display manager context */
    size = sizeof(buf_mgr_t);
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    ps_codec->pv_disp_buf_mgr = pv_buf;

    /* size for holding dpb manager context */
    size = sizeof(dpb_mgr_t);
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    ps_codec->pv_dpb_mgr = pv_buf;

    /* size for holding buffer manager context */
    size = sizeof(buf_mgr_t);
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    ps_codec->pv_pic_buf_mgr = pv_buf;

    /* size for holding mv buffer manager context */
    size = sizeof(buf_mgr_t);
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    ps_codec->pv_mv_buf_mgr = pv_buf;

    size = MAX_VPS_CNT * sizeof(vps_t);
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->ps_vps_base = pv_buf;
    ps_codec->s_parse.ps_vps_base = ps_codec->ps_vps_base;

    size = MAX_SPS_CNT * sizeof(sps_t);
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->ps_sps_base = pv_buf;
    ps_codec->s_parse.ps_sps_base = ps_codec->ps_sps_base;

    size = MAX_PPS_CNT * sizeof(pps_t);
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->ps_pps_base = pv_buf;
    ps_codec->s_parse.ps_pps_base = ps_codec->ps_pps_base;

    size = MAX_SLICE_HDR_CNT * sizeof(slice_header_t);
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->ps_slice_hdr_base = (slice_header_t *)pv_buf;
    ps_codec->s_parse.ps_slice_hdr_base = ps_codec->ps_slice_hdr_base;


    SCALING_MAT_SIZE(size)
    size = (MAX_SPS_CNT + MAX_PPS_CNT) * size * sizeof(WORD16);
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    ps_codec->pi2_scaling_mat = (WORD16 *)pv_buf;


    /* Size for holding pic_buf_t for each reference picture
     * Since this is only a structure allocation and not actual buffer allocation,
     * it is allocated for BUF_MGR_MAX_CNT entries
     */
    size = BUF_MGR_MAX_CNT * sizeof(pic_buf_t);
    pv_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    ps_codec->pv_pic_buf_base = (UWORD8 *)pv_buf;

    /* TO hold scratch buffers needed for each SAO context */
    size = 4 * MAX_CTB_SIZE * MAX_CTB_SIZE;

    /* 2 temporary buffers*/
    size *= 2;
    size *= MAX_PROCESS_THREADS;

    pu1_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pu1_buf), IV_FAIL);

    for(i = 0; i < MAX_PROCESS_THREADS; i++)
    {
        ps_codec->as_process[i].s_sao_ctxt.pu1_tmp_buf_luma = (UWORD8 *)pu1_buf;
        pu1_buf += 4 * MAX_CTB_SIZE * MAX_CTB_SIZE * sizeof(UWORD8);

        ps_codec->as_process[i].s_sao_ctxt.pu1_tmp_buf_chroma = (UWORD8 *)pu1_buf;
        pu1_buf += 4 * MAX_CTB_SIZE * MAX_CTB_SIZE * sizeof(UWORD8);
    }

    /* Allocate intra pred modes buffer */
    /* 8 bits per 4x4 */
    /* 16 bytes each for top and left 64 pixels and 16 bytes for default mode */
    size =  3 * 16 * sizeof(UWORD8);
    pu1_buf = pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pu1_buf), IV_FAIL);
    memset(pu1_buf, 0, size);
    ps_codec->s_parse.pu1_luma_intra_pred_mode_left = pu1_buf;
    ps_codec->s_parse.pu1_luma_intra_pred_mode_top  = pu1_buf + 16;

    {
        WORD32 inter_pred_tmp_buf_size, ntaps_luma;
        WORD32 pic_pu_idx_map_size;

        /* Max inter pred size */
        ntaps_luma = 8;
        inter_pred_tmp_buf_size = sizeof(WORD16) * (MAX_CTB_SIZE + ntaps_luma) * MAX_CTB_SIZE;

        inter_pred_tmp_buf_size = ALIGN64(inter_pred_tmp_buf_size);

        /* To hold pu_index w.r.t. frame level pu_t array for a CTB */
        pic_pu_idx_map_size = sizeof(WORD32) * (18 * 18);
        pic_pu_idx_map_size = ALIGN64(pic_pu_idx_map_size);

        size =  inter_pred_tmp_buf_size * 2;
        size += pic_pu_idx_map_size;
        size *= MAX_PROCESS_THREADS;

        pu1_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
        RETURN_IF((NULL == pu1_buf), IV_FAIL);
        memset(pu1_buf, 0, size);

        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].pi2_inter_pred_tmp_buf1 = (WORD16 *)pu1_buf;
            pu1_buf += inter_pred_tmp_buf_size;

            ps_codec->as_process[i].pi2_inter_pred_tmp_buf2 = (WORD16 *)pu1_buf;
            pu1_buf += inter_pred_tmp_buf_size;

            /* Inverse transform intermediate and inverse scan output buffers reuse inter pred scratch buffers */
            ps_codec->as_process[i].pi2_itrans_intrmd_buf =
                            ps_codec->as_process[i].pi2_inter_pred_tmp_buf2;
            ps_codec->as_process[i].pi2_invscan_out =
                            ps_codec->as_process[i].pi2_inter_pred_tmp_buf1;

            ps_codec->as_process[i].pu4_pic_pu_idx_map = (UWORD32 *)pu1_buf;
            ps_codec->as_process[i].s_bs_ctxt.pu4_pic_pu_idx_map =
                            (UWORD32 *)pu1_buf;
            pu1_buf += pic_pu_idx_map_size;

            //   ps_codec->as_process[i].pi2_inter_pred_tmp_buf3 = (WORD16 *)pu1_buf;
            //   pu1_buf += inter_pred_tmp_buf_size;

            ps_codec->as_process[i].i4_inter_pred_tmp_buf_strd = MAX_CTB_SIZE;

        }
    }
    /* Initialize pointers in PPS structures */
    {
        sps_t *ps_sps = ps_codec->ps_sps_base;
        pps_t *ps_pps = ps_codec->ps_pps_base;
        WORD16 *pi2_scaling_mat =  ps_codec->pi2_scaling_mat;
        WORD32 scaling_mat_size;

        SCALING_MAT_SIZE(scaling_mat_size);

        for(i = 0; i < MAX_SPS_CNT; i++)
        {
            ps_sps->pi2_scaling_mat  = pi2_scaling_mat;
            pi2_scaling_mat += scaling_mat_size;
            ps_sps++;
        }

        for(i = 0; i < MAX_PPS_CNT; i++)
        {
            ps_pps->pi2_scaling_mat  = pi2_scaling_mat;
            pi2_scaling_mat += scaling_mat_size;
            ps_pps++;
        }
    }

    return (status);
}

/**
*******************************************************************************
*
* @brief
*  Free static memory for the codec
*
* @par Description:
*  Free static memory for the codec
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_free_static_bufs(iv_obj_t *ps_codec_obj)
{
    codec_t *ps_codec;

    void (*pf_aligned_free)(void *pv_mem_ctxt, void *pv_buf);
    void *pv_mem_ctxt;

    ps_codec = (codec_t *)ps_codec_obj->pv_codec_handle;
    pf_aligned_free = ps_codec->pf_aligned_free;
    pv_mem_ctxt = ps_codec->pv_mem_ctxt;


    ALIGNED_FREE(ps_codec, ps_codec->apv_process_thread_handle[0]);
    ALIGNED_FREE(ps_codec, ps_codec->pu1_bitsbuf_static);

    ALIGNED_FREE(ps_codec, ps_codec->pv_disp_buf_mgr);
    ALIGNED_FREE(ps_codec, ps_codec->pv_dpb_mgr);
    ALIGNED_FREE(ps_codec, ps_codec->pv_pic_buf_mgr);
    ALIGNED_FREE(ps_codec, ps_codec->pv_mv_buf_mgr);
    ALIGNED_FREE(ps_codec, ps_codec->ps_vps_base);
    ALIGNED_FREE(ps_codec, ps_codec->ps_sps_base);
    ALIGNED_FREE(ps_codec, ps_codec->ps_pps_base);
    ALIGNED_FREE(ps_codec, ps_codec->ps_slice_hdr_base);
    ALIGNED_FREE(ps_codec, ps_codec->pi2_scaling_mat);
    ALIGNED_FREE(ps_codec, ps_codec->pv_pic_buf_base);
    ALIGNED_FREE(ps_codec, ps_codec->s_parse.pu1_luma_intra_pred_mode_left);
    ALIGNED_FREE(ps_codec, ps_codec->as_process[0].s_sao_ctxt.pu1_tmp_buf_luma);
    ALIGNED_FREE(ps_codec, ps_codec->as_process[0].pi2_inter_pred_tmp_buf1);
    ALIGNED_FREE(ps_codec, ps_codec_obj->pv_codec_handle);

    if(ps_codec_obj)
    {
        pf_aligned_free(pv_mem_ctxt, ps_codec_obj);
    }

    return IV_SUCCESS;

}


/**
*******************************************************************************
*
* @brief
*  Allocate dynamic memory for the codec
*
* @par Description:
*  Allocates dynamic memory for the codec
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_allocate_dynamic_bufs(codec_t *ps_codec)
{
    WORD32 max_tile_cols, max_tile_rows;
    WORD32 max_ctb_rows, max_ctb_cols;
    WORD32 max_num_cu_cols;
    WORD32 max_num_cu_rows;
    WORD32 max_num_4x4_cols;
    WORD32 max_ctb_cnt;
    WORD32 wd;
    WORD32 ht;
    WORD32 i;
    WORD32 max_dpb_size;
    void *pv_mem_ctxt = ps_codec->pv_mem_ctxt;
    void *pv_buf;
    UWORD8 *pu1_buf;
    WORD32 size;

    wd = ALIGN64(ps_codec->i4_wd);
    ht = ALIGN64(ps_codec->i4_ht);

    max_tile_cols = (wd + MIN_TILE_WD - 1) / MIN_TILE_WD;
    max_tile_rows = (ht + MIN_TILE_HT - 1) / MIN_TILE_HT;
    max_ctb_rows  = ht / MIN_CTB_SIZE;
    max_ctb_cols  = wd / MIN_CTB_SIZE;
    max_ctb_cnt   = max_ctb_rows * max_ctb_cols;
    max_num_cu_cols = wd / MIN_CU_SIZE;
    max_num_cu_rows = ht / MIN_CU_SIZE;
    max_num_4x4_cols = wd / 4;

    /* Allocate tile structures */
    size = max_tile_cols * max_tile_rows;
    size *= sizeof(tile_t);
    size *= MAX_PPS_CNT;

    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->ps_tile = (tile_t *)pv_buf;


    /* Allocate memory to hold entry point offsets */
    /* One entry point per tile */
    size = max_tile_cols * max_tile_rows;

    /* One entry point per row of CTBs */
    /*********************************************************************/
    /* Only tiles or entropy sync is enabled at a time in main           */
    /* profile, but since memory required does not increase too much,    */
    /* this allocation is done to handle both cases                      */
    /*********************************************************************/
    size  += max_ctb_rows;
    size *= sizeof(WORD32);

    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->pi4_entry_ofst = (WORD32 *)pv_buf;

    /* Allocate parse skip flag buffer */
    /* 1 bit per 8x8 */
    size = max_num_cu_cols / 8;
    size = ALIGN4(size);
    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->s_parse.pu4_skip_cu_top = (UWORD32 *)pv_buf;

    /* Allocate parse coding tree depth buffer */
    /* 2 bits per 8x8 */
    size =  max_num_cu_cols / 4;
    size = ALIGN4(size);
    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->s_parse.pu4_ct_depth_top = (UWORD32 *)pv_buf;

    /* Allocate intra flag buffer */
    /* 1 bit per 8x8 */
    size =  max_num_cu_cols * max_num_cu_rows / 8;
    size = ALIGN4(size);
    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->pu1_pic_intra_flag = (UWORD8 *)pv_buf;
    ps_codec->s_parse.pu1_pic_intra_flag = ps_codec->pu1_pic_intra_flag;

    /* Allocate transquant bypass flag buffer */
    /* 1 bit per 8x8 */
    /* Extra row and column are allocated for easy processing of top and left blocks while loop filtering */
    size =  ((max_num_cu_cols + 8) * (max_num_cu_rows + 8)) / 8;
    size = ALIGN4(size);
    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 1, size);
    {
        WORD32 loop_filter_strd = (wd + 63) >> 6;
        ps_codec->pu1_pic_no_loop_filter_flag_base = pv_buf;
        /* The offset is added for easy processing of top and left blocks while loop filtering */
        ps_codec->pu1_pic_no_loop_filter_flag = (UWORD8 *)pv_buf + loop_filter_strd + 1;
        ps_codec->s_parse.pu1_pic_no_loop_filter_flag = ps_codec->pu1_pic_no_loop_filter_flag;
        ps_codec->s_parse.s_deblk_ctxt.pu1_pic_no_loop_filter_flag = ps_codec->pu1_pic_no_loop_filter_flag;
        ps_codec->s_parse.s_sao_ctxt.pu1_pic_no_loop_filter_flag = ps_codec->pu1_pic_no_loop_filter_flag;
    }

    /* Initialize pointers in PPS structures */
    {
        pps_t *ps_pps = ps_codec->ps_pps_base;
        tile_t *ps_tile =  ps_codec->ps_tile;

        for(i = 0; i < MAX_PPS_CNT; i++)
        {
            ps_pps->ps_tile = ps_tile;
            ps_tile += (max_tile_cols * max_tile_rows);
            ps_pps++;
        }

    }

    /* Allocate memory for job queue */

    /* One job per row of CTBs */
    size  = max_ctb_rows;

    /* One each tile a row of CTBs, num_jobs has to incremented */
    size  *= max_tile_cols;

    /* One format convert/frame copy job per row of CTBs for non-shared mode*/
    size  += max_ctb_rows;

    size *= sizeof(proc_job_t);

    size += ihevcd_jobq_ctxt_size();
    size = ALIGN4(size);

    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    ps_codec->pv_proc_jobq_buf = pv_buf;
    ps_codec->i4_proc_jobq_buf_size = size;

    size =  max_ctb_cnt;
    size = ALIGN4(size);
    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->pu1_parse_map = (UWORD8 *)pv_buf;

    size =  max_ctb_cnt;
    size = ALIGN4(size);
    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->pu1_proc_map = (UWORD8 *)pv_buf;

    /** Holds top and left neighbor's pu idx into picture level pu array */
    /* Only one top row is enough but left has to be replicated for each process context */
    size =  (max_num_4x4_cols  /* left */ + MAX_PROCESS_THREADS * (MAX_CTB_SIZE / 4)/* top */ + 1/* top right */) * sizeof(WORD32);
    size = ALIGN4(size);
    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);

    for(i = 0; i < MAX_PROCESS_THREADS; i++)
    {
        UWORD32 *pu4_buf = (UWORD32 *)pv_buf;
        ps_codec->as_process[i].pu4_pic_pu_idx_left = pu4_buf + i * (MAX_CTB_SIZE / 4);
        memset(ps_codec->as_process[i].pu4_pic_pu_idx_left, 0, sizeof(UWORD32) * MAX_CTB_SIZE / 4);
        ps_codec->as_process[i].pu4_pic_pu_idx_top = pu4_buf + MAX_PROCESS_THREADS * (MAX_CTB_SIZE / 4);
    }
    memset(ps_codec->as_process[0].pu4_pic_pu_idx_top, 0, sizeof(UWORD32) * (wd / 4 + 1));

    {
        /* To hold SAO left buffer for luma */
        size  = sizeof(UWORD8) * (MAX(ht, wd));

        /* To hold SAO left buffer for chroma */
        size += sizeof(UWORD8) * (MAX(ht, wd));

        /* To hold SAO top buffer for luma */
        size += sizeof(UWORD8) * wd;

        /* To hold SAO top buffer for chroma */
        size += sizeof(UWORD8) * wd;

        /* To hold SAO top left luma pixel value for last output ctb in a row*/
        size += sizeof(UWORD8) * max_ctb_rows;

        /* To hold SAO top left chroma pixel value last output ctb in a row*/
        size += sizeof(UWORD8) * max_ctb_rows * 2;

        /* To hold SAO top left pixel luma for current ctb - column array*/
        size += sizeof(UWORD8) * max_ctb_rows;

        /* To hold SAO top left pixel chroma for current ctb-column array*/
        size += sizeof(UWORD8) * max_ctb_rows * 2;

        /* To hold SAO top right pixel luma pixel value last output ctb in a row*/
        size += sizeof(UWORD8) * max_ctb_cols;

        /* To hold SAO top right pixel chroma pixel value last output ctb in a row*/
        size += sizeof(UWORD8) * max_ctb_cols * 2;

        /*To hold SAO botton bottom left pixels for luma*/
        size += sizeof(UWORD8) * max_ctb_rows;

        /*To hold SAO botton bottom left pixels for luma*/
        size += sizeof(UWORD8) * max_ctb_rows * 2;
        size = ALIGN64(size);

        pu1_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
        RETURN_IF((NULL == pu1_buf), IV_FAIL);
        memset(pu1_buf, 0, size);

        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_left_luma = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_left_luma = (UWORD8 *)pu1_buf;
        pu1_buf += MAX(ht, wd);

        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_left_chroma = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_left_chroma = (UWORD8 *)pu1_buf;
        pu1_buf += MAX(ht, wd);
        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_top_luma = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_top_luma = (UWORD8 *)pu1_buf;
        pu1_buf += wd;

        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_top_chroma = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_top_chroma = (UWORD8 *)pu1_buf;
        pu1_buf += wd;
        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_luma_top_left_ctb = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_luma_top_left_ctb = (UWORD8 *)pu1_buf;
        pu1_buf += ht / MIN_CTB_SIZE;

        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_chroma_top_left_ctb = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_chroma_top_left_ctb = (UWORD8 *)pu1_buf;
        pu1_buf += (ht / MIN_CTB_SIZE) * 2;

        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_top_left_luma_curr_ctb = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_top_left_luma_curr_ctb = (UWORD8 *)pu1_buf;
        pu1_buf += ht / MIN_CTB_SIZE;

        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_top_left_chroma_curr_ctb = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_top_left_chroma_curr_ctb = (UWORD8 *)pu1_buf;

        pu1_buf += (ht / MIN_CTB_SIZE) * 2;
        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_top_left_luma_top_right = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_top_left_luma_top_right = (UWORD8 *)pu1_buf;

        pu1_buf += wd / MIN_CTB_SIZE;
        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_top_left_chroma_top_right = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_top_left_chroma_top_right = (UWORD8 *)pu1_buf;

        pu1_buf += (wd / MIN_CTB_SIZE) * 2;

        /*Per CTB, Store 1 value for luma , 2 values for chroma*/
        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_top_left_luma_bot_left = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_top_left_luma_bot_left = (UWORD8 *)pu1_buf;

        pu1_buf += (ht / MIN_CTB_SIZE);

        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_sao_ctxt.pu1_sao_src_top_left_chroma_bot_left = (UWORD8 *)pu1_buf;
        }
        ps_codec->s_parse.s_sao_ctxt.pu1_sao_src_top_left_chroma_bot_left = (UWORD8 *)pu1_buf;

        pu1_buf += (ht / MIN_CTB_SIZE) * 2;
    }


    {
        UWORD8 *pu1_buf = (UWORD8 *)pv_buf;
        WORD32 vert_bs_size, horz_bs_size;
        WORD32 qp_const_flag_size;
        WORD32 qp_size;
        WORD32 num_8x8;

        /* Max Number of vertical edges */
        vert_bs_size = wd / 8 + 2 * MAX_CTB_SIZE / 8;

        /* Max Number of horizontal edges - extra MAX_CTB_SIZE / 8 to handle the last 4 rows separately(shifted CTB processing) */
        vert_bs_size *= (ht + MAX_CTB_SIZE) / MIN_TU_SIZE;

        /* Number of bytes */
        vert_bs_size /= 8;

        /* Two bits per edge */
        vert_bs_size *= 2;

        /* Max Number of horizontal edges */
        horz_bs_size = ht / 8 + MAX_CTB_SIZE / 8;

        /* Max Number of vertical edges - extra MAX_CTB_SIZE / 8 to handle the last 4 columns separately(shifted CTB processing) */
        horz_bs_size *= (wd + MAX_CTB_SIZE) / MIN_TU_SIZE;

        /* Number of bytes */
        horz_bs_size /= 8;

        /* Two bits per edge */
        horz_bs_size *= 2;

        /* Max CTBs in a row */
        qp_const_flag_size = wd / MIN_CTB_SIZE + 1 /* The last ctb row deblk is done in last ctb + 1 row.*/;

        /* Max CTBs in a column */
        qp_const_flag_size *= ht / MIN_CTB_SIZE;

        /* Number of bytes */
        qp_const_flag_size /= 8;

        /* QP changes at CU level - So store at 8x8 level */
        num_8x8 = (ht * wd) / (MIN_CU_SIZE * MIN_CU_SIZE);
        qp_size = num_8x8;

        /* To hold vertical boundary strength */
        size += vert_bs_size;

        /* To hold horizontal boundary strength */
        size += horz_bs_size;

        /* To hold QP */
        size += qp_size;

        /* To hold QP const in CTB flags */
        size += qp_const_flag_size;

        pu1_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
        RETURN_IF((NULL == pu1_buf), IV_FAIL);

        memset(pu1_buf, 0, size);

        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].s_bs_ctxt.pu4_pic_vert_bs = (UWORD32 *)pu1_buf;
            ps_codec->as_process[i].s_deblk_ctxt.s_bs_ctxt.pu4_pic_vert_bs = (UWORD32 *)pu1_buf;
            ps_codec->s_parse.s_deblk_ctxt.s_bs_ctxt.pu4_pic_vert_bs = (UWORD32 *)pu1_buf;
            pu1_buf += vert_bs_size;

            ps_codec->as_process[i].s_bs_ctxt.pu4_pic_horz_bs = (UWORD32 *)pu1_buf;
            ps_codec->as_process[i].s_deblk_ctxt.s_bs_ctxt.pu4_pic_horz_bs = (UWORD32 *)pu1_buf;
            ps_codec->s_parse.s_deblk_ctxt.s_bs_ctxt.pu4_pic_horz_bs = (UWORD32 *)pu1_buf;
            pu1_buf += horz_bs_size;

            ps_codec->as_process[i].s_bs_ctxt.pu1_pic_qp = (UWORD8 *)pu1_buf;
            ps_codec->as_process[i].s_deblk_ctxt.s_bs_ctxt.pu1_pic_qp = (UWORD8 *)pu1_buf;
            ps_codec->s_parse.s_deblk_ctxt.s_bs_ctxt.pu1_pic_qp = (UWORD8 *)pu1_buf;
            pu1_buf += qp_size;

            ps_codec->as_process[i].s_bs_ctxt.pu1_pic_qp_const_in_ctb = (UWORD8 *)pu1_buf;
            ps_codec->as_process[i].s_deblk_ctxt.s_bs_ctxt.pu1_pic_qp_const_in_ctb = (UWORD8 *)pu1_buf;
            ps_codec->s_parse.s_deblk_ctxt.s_bs_ctxt.pu1_pic_qp_const_in_ctb = (UWORD8 *)pu1_buf;
            pu1_buf += qp_const_flag_size;

            pu1_buf -= (vert_bs_size + horz_bs_size + qp_size + qp_const_flag_size);
        }
        ps_codec->s_parse.s_bs_ctxt.pu4_pic_vert_bs = (UWORD32 *)pu1_buf;
        pu1_buf += vert_bs_size;

        ps_codec->s_parse.s_bs_ctxt.pu4_pic_horz_bs = (UWORD32 *)pu1_buf;
        pu1_buf += horz_bs_size;

        ps_codec->s_parse.s_bs_ctxt.pu1_pic_qp = (UWORD8 *)pu1_buf;
        pu1_buf += qp_size;

        ps_codec->s_parse.s_bs_ctxt.pu1_pic_qp_const_in_ctb = (UWORD8 *)pu1_buf;
        pu1_buf += qp_const_flag_size;

    }

    /* Max CTBs in a row */
    size  = wd / MIN_CTB_SIZE;
    /* Max CTBs in a column */
    size *= (ht / MIN_CTB_SIZE + 2) /* Top row and bottom row extra. This ensures accessing left,top in first row
                                              and right in last row will not result in invalid access*/;

    size *= sizeof(UWORD16);
    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);

    ps_codec->pu1_tile_idx_base = pv_buf;
    for(i = 0; i < MAX_PROCESS_THREADS; i++)
    {
        ps_codec->as_process[i].pu1_tile_idx = (UWORD16 *)pv_buf + wd / MIN_CTB_SIZE /* Offset 1 row */;
    }

    /* 4 bytes per color component per CTB */
    size = 3 * 4;

    /* MAX number of CTBs in a row */
    size *= wd / MIN_CTB_SIZE;

    /* MAX number of CTBs in a column */
    size *= ht / MIN_CTB_SIZE;
    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);

    ps_codec->s_parse.ps_pic_sao = (sao_t *)pv_buf;
    ps_codec->s_parse.s_sao_ctxt.ps_pic_sao = (sao_t *)pv_buf;
    for(i = 0; i < MAX_PROCESS_THREADS; i++)
    {
        ps_codec->as_process[i].s_sao_ctxt.ps_pic_sao = ps_codec->s_parse.ps_pic_sao;
    }

    /* Only if width * height * 3 / 2 is greater than MIN_BITSBUF_SIZE,
    then allocate dynamic bistream buffer */
    ps_codec->pu1_bitsbuf_dynamic = NULL;
    size = wd * ht;
    if(size > MIN_BITSBUF_SIZE)
    {
        pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size + 16); //Alloc extra for parse optimization
        RETURN_IF((NULL == pv_buf), IV_FAIL);
        ps_codec->pu1_bitsbuf_dynamic = pv_buf;
        ps_codec->u4_bitsbuf_size_dynamic = size;
    }

    size = ihevcd_get_tu_data_size(wd * ht);
    pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
    RETURN_IF((NULL == pv_buf), IV_FAIL);
    memset(pv_buf, 0, size);
    ps_codec->pv_tu_data = pv_buf;

    {
        sps_t *ps_sps = (ps_codec->s_parse.ps_sps_base + ps_codec->i4_sps_id);


        /* Allocate for pu_map, pu_t and pic_pu_idx for each MV bank */
        /* Note: Number of luma samples is not max_wd * max_ht here, instead it is
         * set to maximum number of luma samples allowed at the given level.
         * This is done to ensure that any stream with width and height lesser
         * than max_wd and max_ht is supported. Number of buffers required can be greater
         * for lower width and heights at a given level and this increased number of buffers
         * might require more memory than what max_wd and max_ht buffer would have required
         * Also note one extra buffer is allocted to store current pictures MV bank
         * In case of asynchronous parsing and processing, number of buffers should increase here
         * based on when parsing and processing threads are synchronized
         */
        max_dpb_size = ps_sps->ai1_sps_max_dec_pic_buffering[ps_sps->i1_sps_max_sub_layers - 1];
        /* Size for holding mv_buf_t for each MV Bank
         * One extra MV Bank is needed to hold current pics MV bank.
         */
        size = (max_dpb_size + 1) * sizeof(mv_buf_t);

        size += (max_dpb_size + 1) *
                        ihevcd_get_pic_mv_bank_size(wd * ht);

        pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
        RETURN_IF((NULL == pv_buf), IV_FAIL);

        ps_codec->pv_mv_bank_buf_base = pv_buf;
        ps_codec->i4_total_mv_bank_size = size;

    }

    /* In case of non-shared mode allocate for reference picture buffers */
    /* In case of shared and 420p output, allocate for chroma samples */
    if(0 == ps_codec->i4_share_disp_buf)
    {
        /* Number of buffers is doubled in order to return one frame at a time instead of sending
         * multiple outputs during dpb full case.
         * Also note one extra buffer is allocted to store current picture
         * In case of asynchronous parsing and processing, number of buffers should increase here
         * based on when parsing and processing threads are synchronized
         */
        size = ihevcd_get_total_pic_buf_size(ps_codec, wd, ht);
        pv_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
        RETURN_IF((NULL == pv_buf), IV_FAIL);


        ps_codec->i4_total_pic_buf_size = size;
        ps_codec->pu1_ref_pic_buf_base = (UWORD8 *)pv_buf;
    }

    ps_codec->pv_proc_jobq = ihevcd_jobq_init(ps_codec->pv_proc_jobq_buf, ps_codec->i4_proc_jobq_buf_size);
    RETURN_IF((ps_codec->pv_proc_jobq == NULL), IV_FAIL);

    /* Update the jobq context to all the threads */
    ps_codec->s_parse.pv_proc_jobq = ps_codec->pv_proc_jobq;
    for(i = 0; i < MAX_PROCESS_THREADS; i++)
    {
        ps_codec->as_process[i].pv_proc_jobq = ps_codec->pv_proc_jobq;
        ps_codec->as_process[i].i4_id = i;
        ps_codec->as_process[i].ps_codec = ps_codec;

        /* Set the following to zero assuming it is a single core solution
         * When threads are launched these will be set appropriately
         */
        ps_codec->as_process[i].i4_check_parse_status = 0;
        ps_codec->as_process[i].i4_check_proc_status = 0;
    }

    ps_codec->u4_allocate_dynamic_done = 1;

    return IV_SUCCESS;
}

/**
*******************************************************************************
*
* @brief
*  Free dynamic memory for the codec
*
* @par Description:
*  Free dynamic memory for the codec
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_free_dynamic_bufs(codec_t *ps_codec)
{

    if(ps_codec->pv_proc_jobq)
    {
        ihevcd_jobq_deinit(ps_codec->pv_proc_jobq);
        ps_codec->pv_proc_jobq = NULL;
    }

    ALIGNED_FREE(ps_codec, ps_codec->ps_tile);
    ALIGNED_FREE(ps_codec, ps_codec->pi4_entry_ofst);
    ALIGNED_FREE(ps_codec, ps_codec->s_parse.pu4_skip_cu_top);
    ALIGNED_FREE(ps_codec, ps_codec->s_parse.pu4_ct_depth_top);
    ALIGNED_FREE(ps_codec, ps_codec->pu1_pic_intra_flag);
    ALIGNED_FREE(ps_codec, ps_codec->pu1_pic_no_loop_filter_flag_base);
    ALIGNED_FREE(ps_codec, ps_codec->pv_proc_jobq_buf);
    ALIGNED_FREE(ps_codec, ps_codec->pu1_parse_map);
    ALIGNED_FREE(ps_codec, ps_codec->pu1_proc_map);
    ALIGNED_FREE(ps_codec, ps_codec->as_process[0].pu4_pic_pu_idx_left);
    ALIGNED_FREE(ps_codec, ps_codec->as_process[0].s_sao_ctxt.pu1_sao_src_left_luma);
    ALIGNED_FREE(ps_codec, ps_codec->as_process[0].s_bs_ctxt.pu4_pic_vert_bs);
    ALIGNED_FREE(ps_codec, ps_codec->pu1_tile_idx_base);
    ALIGNED_FREE(ps_codec, ps_codec->s_parse.ps_pic_sao);
    ALIGNED_FREE(ps_codec, ps_codec->pu1_bitsbuf_dynamic);
    ALIGNED_FREE(ps_codec, ps_codec->pv_tu_data);
    ALIGNED_FREE(ps_codec, ps_codec->pv_mv_bank_buf_base);
    ALIGNED_FREE(ps_codec, ps_codec->pu1_ref_pic_buf_base);
    ALIGNED_FREE(ps_codec, ps_codec->pu1_cur_chroma_ref_buf);

    ps_codec->u4_allocate_dynamic_done = 0;
    return IV_SUCCESS;
}


/**
*******************************************************************************
*
* @brief
*  Initializes from mem records passed to the codec
*
* @par Description:
*  Initializes pointers based on mem records passed
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_create(iv_obj_t *ps_codec_obj,
                           void *pv_api_ip,
                           void *pv_api_op)
{
    ihevcd_cxa_create_ip_t *ps_create_ip;
    ihevcd_cxa_create_op_t *ps_create_op;

    WORD32 ret;
    codec_t *ps_codec;
    ps_create_ip = (ihevcd_cxa_create_ip_t *)pv_api_ip;
    ps_create_op = (ihevcd_cxa_create_op_t *)pv_api_op;

    ps_create_op->s_ivd_create_op_t.u4_error_code = 0;
    ps_codec_obj = NULL;
    ret = ihevcd_allocate_static_bufs(&ps_codec_obj, pv_api_ip, pv_api_op);

    /* If allocation of some buffer fails, then free buffers allocated till then */
    if(IV_FAIL == ret)
    {
        if(NULL != ps_codec_obj)
        {
            if(ps_codec_obj->pv_codec_handle)
            {
                ihevcd_free_static_bufs(ps_codec_obj);
            }
            else
            {
                void (*pf_aligned_free)(void *pv_mem_ctxt, void *pv_buf);
                void *pv_mem_ctxt;

                pf_aligned_free = ps_create_ip->s_ivd_create_ip_t.pf_aligned_free;
                pv_mem_ctxt  = ps_create_ip->s_ivd_create_ip_t.pv_mem_ctxt;
                pf_aligned_free(pv_mem_ctxt, ps_codec_obj);
            }
        }
        ps_create_op->s_ivd_create_op_t.u4_error_code = IVD_MEM_ALLOC_FAILED;
        ps_create_op->s_ivd_create_op_t.u4_error_code = 1 << IVD_FATALERROR;

        return IV_FAIL;
    }
    ps_codec = (codec_t *)ps_codec_obj->pv_codec_handle;
    ret = ihevcd_init(ps_codec);

    TRACE_INIT(NULL);
    STATS_INIT();

    return ret;
}
/**
*******************************************************************************
*
* @brief
*  Delete codec
*
* @par Description:
*  Delete codec
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_delete(iv_obj_t *ps_codec_obj, void *pv_api_ip, void *pv_api_op)
{
    codec_t *ps_dec;
    ihevcd_cxa_delete_ip_t *ps_ip = (ihevcd_cxa_delete_ip_t *)pv_api_ip;
    ihevcd_cxa_delete_op_t *ps_op = (ihevcd_cxa_delete_op_t *)pv_api_op;

    ps_dec = (codec_t *)(ps_codec_obj->pv_codec_handle);
    UNUSED(ps_ip);
    ps_op->s_ivd_delete_op_t.u4_error_code = 0;
    ihevcd_free_dynamic_bufs(ps_dec);
    ihevcd_free_static_bufs(ps_codec_obj);
    return IV_SUCCESS;
}


/**
*******************************************************************************
*
* @brief
*  Passes display buffer from application to codec
*
* @par Description:
*  Adds display buffer to the codec
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_set_display_frame(iv_obj_t *ps_codec_obj,
                                void *pv_api_ip,
                                void *pv_api_op)
{
    WORD32 ret = IV_SUCCESS;

    ivd_set_display_frame_ip_t *ps_dec_disp_ip;
    ivd_set_display_frame_op_t *ps_dec_disp_op;

    WORD32 i;

    codec_t *ps_codec = (codec_t *)(ps_codec_obj->pv_codec_handle);

    ps_dec_disp_ip = (ivd_set_display_frame_ip_t *)pv_api_ip;
    ps_dec_disp_op = (ivd_set_display_frame_op_t *)pv_api_op;

    ps_codec->i4_num_disp_bufs = 0;
    if(ps_codec->i4_share_disp_buf)
    {
        UWORD32 num_bufs = ps_dec_disp_ip->num_disp_bufs;
        pic_buf_t *ps_pic_buf;
        UWORD8 *pu1_buf;
        WORD32 buf_ret;

        UWORD8 *pu1_chroma_buf = NULL;
        num_bufs = MIN(num_bufs, BUF_MGR_MAX_CNT);
        ps_codec->i4_num_disp_bufs = num_bufs;

        ps_pic_buf = (pic_buf_t *)ps_codec->ps_pic_buf;

        /* If color format is 420P, then allocate chroma buffers to hold semiplanar
         * chroma data */
        if(ps_codec->e_chroma_fmt == IV_YUV_420P)
        {
            WORD32 num_samples = ps_dec_disp_ip->s_disp_buffer[0].u4_min_out_buf_size[1] << 1;
            WORD32 size = num_samples * num_bufs;
            void *pv_mem_ctxt = ps_codec->pv_mem_ctxt;

            pu1_chroma_buf = ps_codec->pf_aligned_alloc(pv_mem_ctxt, 128, size);
            RETURN_IF((NULL == pu1_chroma_buf), IV_FAIL);

            ps_codec->pu1_cur_chroma_ref_buf = pu1_chroma_buf;
        }
        for(i = 0; i < (WORD32)num_bufs; i++)
        {
            /* Stride is not available in some cases here.
               So store base pointers to buffer manager now,
               and update these pointers once header is decoded */
            pu1_buf =  ps_dec_disp_ip->s_disp_buffer[i].pu1_bufs[0];
            ps_pic_buf->pu1_luma = pu1_buf;

            if(ps_codec->e_chroma_fmt == IV_YUV_420P)
            {
                pu1_buf = pu1_chroma_buf;
                pu1_chroma_buf += ps_dec_disp_ip->s_disp_buffer[0].u4_min_out_buf_size[1] << 1;
            }
            else
            {
                /* For YUV 420SP case use display buffer itself as chroma ref buffer */
                pu1_buf =  ps_dec_disp_ip->s_disp_buffer[i].pu1_bufs[1];
            }

            ps_pic_buf->pu1_chroma = pu1_buf;

            buf_ret = ihevc_buf_mgr_add((buf_mgr_t *)ps_codec->pv_pic_buf_mgr, ps_pic_buf, i);

            if(0 != buf_ret)
            {
                ps_codec->i4_error_code = IHEVCD_BUF_MGR_ERROR;
                return IHEVCD_BUF_MGR_ERROR;
            }

            /* Mark pic buf as needed for display */
            /* This ensures that till the buffer is explicitly passed to the codec,
             * application owns the buffer. Decoder is allowed to use a buffer only
             * when application sends it through fill this buffer call in OMX
             */
            ihevc_buf_mgr_set_status((buf_mgr_t *)ps_codec->pv_pic_buf_mgr, i, BUF_MGR_DISP);

            ps_pic_buf++;

            /* Store display buffers in codec context. Needed for 420p output */
            memcpy(&ps_codec->s_disp_buffer[ps_codec->i4_share_disp_buf_cnt],
                   &ps_dec_disp_ip->s_disp_buffer[i],
                   sizeof(ps_dec_disp_ip->s_disp_buffer[i]));

            ps_codec->i4_share_disp_buf_cnt++;

        }
    }

    ps_dec_disp_op->u4_error_code = 0;
    return ret;

}

/**
*******************************************************************************
*
* @brief
*  Sets the decoder in flush mode. Decoder will come out of  flush only
* after returning all the buffers or at reset
*
* @par Description:
*  Sets the decoder in flush mode
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_set_flush_mode(iv_obj_t *ps_codec_obj,
                             void *pv_api_ip,
                             void *pv_api_op)
{

    codec_t *ps_codec;
    ivd_ctl_flush_op_t *ps_ctl_op = (ivd_ctl_flush_op_t *)pv_api_op;
    UNUSED(pv_api_ip);
    ps_codec = (codec_t *)(ps_codec_obj->pv_codec_handle);

    /* Signal flush frame control call */
    ps_codec->i4_flush_mode = 1;

    ps_ctl_op->u4_error_code = 0;

    /* Set pic count to zero, so that decoder starts buffering again */
    /* once it comes out of flush mode */
    ps_codec->u4_pic_cnt = 0;
    ps_codec->u4_disp_cnt = 0;
    return IV_SUCCESS;


}

/**
*******************************************************************************
*
* @brief
*  Gets decoder status and buffer requirements
*
* @par Description:
*  Gets the decoder status
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/

WORD32 ihevcd_get_status(iv_obj_t *ps_codec_obj,
                         void *pv_api_ip,
                         void *pv_api_op)
{

    WORD32 i;
    codec_t *ps_codec;
    WORD32 wd, ht;
    ivd_ctl_getstatus_op_t *ps_ctl_op = (ivd_ctl_getstatus_op_t *)pv_api_op;

    UNUSED(pv_api_ip);

    ps_ctl_op->u4_error_code = 0;

    ps_codec = (codec_t *)(ps_codec_obj->pv_codec_handle);

    ps_ctl_op->u4_min_num_in_bufs = MIN_IN_BUFS;
    if(ps_codec->e_chroma_fmt == IV_YUV_420P)
        ps_ctl_op->u4_min_num_out_bufs = MIN_OUT_BUFS_420;
    else if(ps_codec->e_chroma_fmt == IV_YUV_422ILE)
        ps_ctl_op->u4_min_num_out_bufs = MIN_OUT_BUFS_422ILE;
    else if(ps_codec->e_chroma_fmt == IV_RGB_565)
        ps_ctl_op->u4_min_num_out_bufs = MIN_OUT_BUFS_RGB565;
    else if(ps_codec->e_chroma_fmt == IV_RGBA_8888)
        ps_ctl_op->u4_min_num_out_bufs = MIN_OUT_BUFS_RGBA8888;
    else if((ps_codec->e_chroma_fmt == IV_YUV_420SP_UV)
                    || (ps_codec->e_chroma_fmt == IV_YUV_420SP_VU))
        ps_ctl_op->u4_min_num_out_bufs = MIN_OUT_BUFS_420SP;

    ps_ctl_op->u4_num_disp_bufs = 1;

    for(i = 0; i < (WORD32)ps_ctl_op->u4_min_num_in_bufs; i++)
    {
        wd = ALIGN64(ps_codec->i4_wd);
        ht = ALIGN64(ps_codec->i4_ht);
        ps_ctl_op->u4_min_in_buf_size[i] = MAX((wd * ht), MIN_BITSBUF_SIZE);
    }

    wd = ps_codec->i4_wd;
    ht = ps_codec->i4_ht;

    if(ps_codec->i4_sps_done)
    {
        if(0 == ps_codec->i4_share_disp_buf)
        {
            wd = ps_codec->i4_disp_wd;
            ht = ps_codec->i4_disp_ht;

        }
        else
        {
            wd = ps_codec->i4_disp_strd;
            ht = ps_codec->i4_ht + PAD_HT;
        }
    }

    if(ps_codec->i4_disp_strd > wd)
        wd = ps_codec->i4_disp_strd;

    if(0 == ps_codec->i4_share_disp_buf)
        ps_ctl_op->u4_num_disp_bufs = 1;
    else
    {
        if(ps_codec->i4_sps_done)
        {
            sps_t *ps_sps = (ps_codec->s_parse.ps_sps_base + ps_codec->i4_sps_id);
            WORD32 reorder_pic_cnt, ref_pic_cnt;
            reorder_pic_cnt = 0;
            if(ps_codec->e_frm_out_mode != IVD_DECODE_FRAME_OUT)
                reorder_pic_cnt = ps_sps->ai1_sps_max_num_reorder_pics[ps_sps->i1_sps_max_sub_layers - 1];
            ref_pic_cnt = ps_sps->ai1_sps_max_dec_pic_buffering[ps_sps->i1_sps_max_sub_layers - 1];

            ps_ctl_op->u4_num_disp_bufs = reorder_pic_cnt;

            ps_ctl_op->u4_num_disp_bufs += ref_pic_cnt + 1;
        }
        else
        {
            ps_ctl_op->u4_num_disp_bufs = MAX_REF_CNT;
        }

        ps_ctl_op->u4_num_disp_bufs = MIN(
                        ps_ctl_op->u4_num_disp_bufs, 32);
    }

    /*!*/
    if(ps_codec->e_chroma_fmt == IV_YUV_420P)
    {
        ps_ctl_op->u4_min_out_buf_size[0] = (wd * ht);
        ps_ctl_op->u4_min_out_buf_size[1] = (wd * ht) >> 2;
        ps_ctl_op->u4_min_out_buf_size[2] = (wd * ht) >> 2;
    }
    else if(ps_codec->e_chroma_fmt == IV_YUV_422ILE)
    {
        ps_ctl_op->u4_min_out_buf_size[0] = (wd * ht) * 2;
        ps_ctl_op->u4_min_out_buf_size[1] =
                        ps_ctl_op->u4_min_out_buf_size[2] = 0;
    }
    else if(ps_codec->e_chroma_fmt == IV_RGB_565)
    {
        ps_ctl_op->u4_min_out_buf_size[0] = (wd * ht) * 2;
        ps_ctl_op->u4_min_out_buf_size[1] =
                        ps_ctl_op->u4_min_out_buf_size[2] = 0;
    }
    else if(ps_codec->e_chroma_fmt == IV_RGBA_8888)
    {
        ps_ctl_op->u4_min_out_buf_size[0] = (wd * ht) * 4;
        ps_ctl_op->u4_min_out_buf_size[1] =
                        ps_ctl_op->u4_min_out_buf_size[2] = 0;
    }
    else if((ps_codec->e_chroma_fmt == IV_YUV_420SP_UV)
                    || (ps_codec->e_chroma_fmt == IV_YUV_420SP_VU))
    {
        ps_ctl_op->u4_min_out_buf_size[0] = (wd * ht);
        ps_ctl_op->u4_min_out_buf_size[1] = (wd * ht) >> 1;
        ps_ctl_op->u4_min_out_buf_size[2] = 0;
    }
    ps_ctl_op->u4_pic_ht = ht;
    ps_ctl_op->u4_pic_wd = wd;
    ps_ctl_op->u4_frame_rate = 30000;
    ps_ctl_op->u4_bit_rate = 1000000;
    ps_ctl_op->e_content_type = IV_PROGRESSIVE;
    ps_ctl_op->e_output_chroma_format = ps_codec->e_chroma_fmt;
    ps_codec->i4_num_disp_bufs = ps_ctl_op->u4_num_disp_bufs;

    if(ps_ctl_op->u4_size == sizeof(ihevcd_cxa_ctl_getstatus_op_t))
    {
        ihevcd_cxa_ctl_getstatus_op_t *ps_ext_ctl_op = (ihevcd_cxa_ctl_getstatus_op_t *)ps_ctl_op;
        ps_ext_ctl_op->u4_coded_pic_wd = ps_codec->i4_wd;
        ps_ext_ctl_op->u4_coded_pic_wd = ps_codec->i4_ht;
    }
    return IV_SUCCESS;
}
/**
*******************************************************************************
*
* @brief
*  Gets decoder buffer requirements
*
* @par Description:
*  Gets the decoder buffer requirements. If called before  header decoder,
* buffer requirements are based on max_wd  and max_ht else actual width and
* height will be used
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_get_buf_info(iv_obj_t *ps_codec_obj,
                           void *pv_api_ip,
                           void *pv_api_op)
{

    codec_t *ps_codec;
    UWORD32 i = 0;
    WORD32 wd, ht;
    ivd_ctl_getbufinfo_op_t *ps_ctl_op =
                    (ivd_ctl_getbufinfo_op_t *)pv_api_op;

    UNUSED(pv_api_ip);
    ps_ctl_op->u4_error_code = 0;

    ps_codec = (codec_t *)(ps_codec_obj->pv_codec_handle);

    ps_ctl_op->u4_min_num_in_bufs = MIN_IN_BUFS;
    if(ps_codec->e_chroma_fmt == IV_YUV_420P)
        ps_ctl_op->u4_min_num_out_bufs = MIN_OUT_BUFS_420;
    else if(ps_codec->e_chroma_fmt == IV_YUV_422ILE)
        ps_ctl_op->u4_min_num_out_bufs = MIN_OUT_BUFS_422ILE;
    else if(ps_codec->e_chroma_fmt == IV_RGB_565)
        ps_ctl_op->u4_min_num_out_bufs = MIN_OUT_BUFS_RGB565;
    else if(ps_codec->e_chroma_fmt == IV_RGBA_8888)
        ps_ctl_op->u4_min_num_out_bufs = MIN_OUT_BUFS_RGBA8888;
    else if((ps_codec->e_chroma_fmt == IV_YUV_420SP_UV)
                    || (ps_codec->e_chroma_fmt == IV_YUV_420SP_VU))
        ps_ctl_op->u4_min_num_out_bufs = MIN_OUT_BUFS_420SP;

    ps_ctl_op->u4_num_disp_bufs = 1;

    for(i = 0; i < ps_ctl_op->u4_min_num_in_bufs; i++)
    {
        wd = ALIGN64(ps_codec->i4_wd);
        ht = ALIGN64(ps_codec->i4_ht);

        ps_ctl_op->u4_min_in_buf_size[i] = MAX((wd * ht), MIN_BITSBUF_SIZE);
    }

    wd = 0;
    ht = 0;

    if(ps_codec->i4_sps_done)
    {
        if(0 == ps_codec->i4_share_disp_buf)
        {
            wd = ps_codec->i4_disp_wd;
            ht = ps_codec->i4_disp_ht;

        }
        else
        {
            wd = ps_codec->i4_disp_strd;
            ht = ps_codec->i4_ht + PAD_HT;
        }
    }
    else
    {
        if(1 == ps_codec->i4_share_disp_buf)
        {
            wd = ALIGN32(wd + PAD_WD);
            ht += PAD_HT;
        }
    }

    if(ps_codec->i4_disp_strd > wd)
        wd = ps_codec->i4_disp_strd;

    if(0 == ps_codec->i4_share_disp_buf)
        ps_ctl_op->u4_num_disp_bufs = 1;
    else
    {
        if(ps_codec->i4_sps_done)
        {
            sps_t *ps_sps = (ps_codec->s_parse.ps_sps_base + ps_codec->i4_sps_id);
            WORD32 reorder_pic_cnt, ref_pic_cnt;
            reorder_pic_cnt = 0;
            if(ps_codec->e_frm_out_mode != IVD_DECODE_FRAME_OUT)
                reorder_pic_cnt = ps_sps->ai1_sps_max_num_reorder_pics[ps_sps->i1_sps_max_sub_layers - 1];
            ref_pic_cnt = ps_sps->ai1_sps_max_dec_pic_buffering[ps_sps->i1_sps_max_sub_layers - 1];

            ps_ctl_op->u4_num_disp_bufs = reorder_pic_cnt;

            ps_ctl_op->u4_num_disp_bufs += ref_pic_cnt + 1;
        }
        else
        {
            ps_ctl_op->u4_num_disp_bufs = MAX_REF_CNT;
        }

        ps_ctl_op->u4_num_disp_bufs = MIN(
                        ps_ctl_op->u4_num_disp_bufs, 32);

    }

    /*!*/
    if(ps_codec->e_chroma_fmt == IV_YUV_420P)
    {
        ps_ctl_op->u4_min_out_buf_size[0] = (wd * ht);
        ps_ctl_op->u4_min_out_buf_size[1] = (wd * ht) >> 2;
        ps_ctl_op->u4_min_out_buf_size[2] = (wd * ht) >> 2;
    }
    else if(ps_codec->e_chroma_fmt == IV_YUV_422ILE)
    {
        ps_ctl_op->u4_min_out_buf_size[0] = (wd * ht) * 2;
        ps_ctl_op->u4_min_out_buf_size[1] =
                        ps_ctl_op->u4_min_out_buf_size[2] = 0;
    }
    else if(ps_codec->e_chroma_fmt == IV_RGB_565)
    {
        ps_ctl_op->u4_min_out_buf_size[0] = (wd * ht) * 2;
        ps_ctl_op->u4_min_out_buf_size[1] =
                        ps_ctl_op->u4_min_out_buf_size[2] = 0;
    }
    else if(ps_codec->e_chroma_fmt == IV_RGBA_8888)
    {
        ps_ctl_op->u4_min_out_buf_size[0] = (wd * ht) * 4;
        ps_ctl_op->u4_min_out_buf_size[1] =
                        ps_ctl_op->u4_min_out_buf_size[2] = 0;
    }
    else if((ps_codec->e_chroma_fmt == IV_YUV_420SP_UV)
                    || (ps_codec->e_chroma_fmt == IV_YUV_420SP_VU))
    {
        ps_ctl_op->u4_min_out_buf_size[0] = (wd * ht);
        ps_ctl_op->u4_min_out_buf_size[1] = (wd * ht) >> 1;
        ps_ctl_op->u4_min_out_buf_size[2] = 0;
    }
    ps_codec->i4_num_disp_bufs = ps_ctl_op->u4_num_disp_bufs;

    return IV_SUCCESS;
}


/**
*******************************************************************************
*
* @brief
*  Sets dynamic parameters
*
* @par Description:
*  Sets dynamic parameters. Note Frame skip, decode header  mode are dynamic
*  Dynamic change in stride is not  supported
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_set_params(iv_obj_t *ps_codec_obj,
                         void *pv_api_ip,
                         void *pv_api_op)
{

    codec_t *ps_codec;
    WORD32 ret = IV_SUCCESS;
    WORD32 strd;
    ivd_ctl_set_config_ip_t *s_ctl_dynparams_ip =
                    (ivd_ctl_set_config_ip_t *)pv_api_ip;
    ivd_ctl_set_config_op_t *s_ctl_dynparams_op =
                    (ivd_ctl_set_config_op_t *)pv_api_op;

    ps_codec = (codec_t *)(ps_codec_obj->pv_codec_handle);

    s_ctl_dynparams_op->u4_error_code = 0;

    ps_codec->e_pic_skip_mode = s_ctl_dynparams_ip->e_frm_skip_mode;

    if(s_ctl_dynparams_ip->e_frm_skip_mode != IVD_SKIP_NONE)
    {

        if((s_ctl_dynparams_ip->e_frm_skip_mode != IVD_SKIP_P) &&
           (s_ctl_dynparams_ip->e_frm_skip_mode != IVD_SKIP_B) &&
           (s_ctl_dynparams_ip->e_frm_skip_mode != IVD_SKIP_PB))
        {
            s_ctl_dynparams_op->u4_error_code = (1 << IVD_UNSUPPORTEDPARAM);
            ret = IV_FAIL;
        }
    }

    strd = ps_codec->i4_disp_strd;
    if(1 == ps_codec->i4_share_disp_buf)
    {
        strd = ps_codec->i4_strd;
    }


    {
        if((WORD32)s_ctl_dynparams_ip->u4_disp_wd >= ps_codec->i4_disp_wd)
        {
            strd = s_ctl_dynparams_ip->u4_disp_wd;
        }
        else if(0 == ps_codec->i4_sps_done)
        {
            strd = s_ctl_dynparams_ip->u4_disp_wd;
        }
        else if(s_ctl_dynparams_ip->u4_disp_wd == 0)
        {
            strd = ps_codec->i4_disp_strd;
        }
        else
        {
            strd = 0;
            s_ctl_dynparams_op->u4_error_code |= (1 << IVD_UNSUPPORTEDPARAM);
            s_ctl_dynparams_op->u4_error_code |= IHEVCD_INVALID_DISP_STRD;
            ret = IV_FAIL;
        }
    }

    ps_codec->i4_disp_strd = strd;
    if(1 == ps_codec->i4_share_disp_buf)
    {
        ps_codec->i4_strd = strd;
    }

    if(s_ctl_dynparams_ip->e_vid_dec_mode == IVD_DECODE_FRAME)
        ps_codec->i4_header_mode = 0;
    else if(s_ctl_dynparams_ip->e_vid_dec_mode == IVD_DECODE_HEADER)
        ps_codec->i4_header_mode = 1;
    else
    {

        s_ctl_dynparams_op->u4_error_code = (1 << IVD_UNSUPPORTEDPARAM);
        ps_codec->i4_header_mode = 1;
        ret = IV_FAIL;
    }

    ps_codec->e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;

    if((s_ctl_dynparams_ip->e_frm_out_mode != IVD_DECODE_FRAME_OUT) &&
       (s_ctl_dynparams_ip->e_frm_out_mode != IVD_DISPLAY_FRAME_OUT))
    {
        s_ctl_dynparams_op->u4_error_code = (1 << IVD_UNSUPPORTEDPARAM);
        ret = IV_FAIL;
    }
    ps_codec->e_frm_out_mode = s_ctl_dynparams_ip->e_frm_out_mode;

    return ret;

}
/**
*******************************************************************************
*
* @brief
*  Resets the decoder state
*
* @par Description:
*  Resets the decoder state by calling ihevcd_init()
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_reset(iv_obj_t *ps_codec_obj, void *pv_api_ip, void *pv_api_op)
{
    codec_t *ps_codec;
    ivd_ctl_reset_op_t *s_ctl_reset_op = (ivd_ctl_reset_op_t *)pv_api_op;
    UNUSED(pv_api_ip);
    ps_codec = (codec_t *)(ps_codec_obj->pv_codec_handle);

    if(ps_codec != NULL)
    {
        DEBUG("\nReset called \n");
        ihevcd_init(ps_codec);
    }
    else
    {
        DEBUG("\nReset called without Initializing the decoder\n");
        s_ctl_reset_op->u4_error_code = IHEVCD_INIT_NOT_DONE;
    }

    return IV_SUCCESS;
}

/**
*******************************************************************************
*
* @brief
*  Releases display buffer from application to codec  to signal to the codec
* that it can write to this buffer  if required. Till release is called,
* codec can not write  to this buffer
*
* @par Description:
*  Marks the buffer as display done
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/

WORD32 ihevcd_rel_display_frame(iv_obj_t *ps_codec_obj,
                                void *pv_api_ip,
                                void *pv_api_op)
{

    ivd_rel_display_frame_ip_t *ps_dec_rel_disp_ip;
    ivd_rel_display_frame_op_t *ps_dec_rel_disp_op;

    codec_t *ps_codec = (codec_t *)ps_codec_obj->pv_codec_handle;

    ps_dec_rel_disp_ip = (ivd_rel_display_frame_ip_t *)pv_api_ip;
    ps_dec_rel_disp_op = (ivd_rel_display_frame_op_t *)pv_api_op;

    UNUSED(ps_dec_rel_disp_op);

    if(0 == ps_codec->i4_share_disp_buf)
    {
        return IV_SUCCESS;
    }

    ihevc_buf_mgr_release((buf_mgr_t *)ps_codec->pv_pic_buf_mgr, ps_dec_rel_disp_ip->u4_disp_buf_id, BUF_MGR_DISP);

    return IV_SUCCESS;
}
/**
*******************************************************************************
*
* @brief
*  Sets degrade params
*
* @par Description:
*  Sets degrade params.
*  Refer to ihevcd_cxa_ctl_degrade_ip_t definition for details
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/

WORD32 ihevcd_set_degrade(iv_obj_t *ps_codec_obj,
                          void *pv_api_ip,
                          void *pv_api_op)
{
    ihevcd_cxa_ctl_degrade_ip_t *ps_ip;
    ihevcd_cxa_ctl_degrade_op_t *ps_op;
    codec_t *ps_codec = (codec_t *)ps_codec_obj->pv_codec_handle;

    ps_ip = (ihevcd_cxa_ctl_degrade_ip_t *)pv_api_ip;
    ps_op = (ihevcd_cxa_ctl_degrade_op_t *)pv_api_op;

    ps_codec->i4_degrade_type = ps_ip->i4_degrade_type;
    ps_codec->i4_nondegrade_interval = ps_ip->i4_nondegrade_interval;
    ps_codec->i4_degrade_pics = ps_ip->i4_degrade_pics;

    ps_op->u4_error_code = 0;
    ps_codec->i4_degrade_pic_cnt = 0;

    return IV_SUCCESS;
}


/**
*******************************************************************************
*
* @brief
*  Gets frame dimensions/offsets
*
* @par Description:
*  Gets frame buffer chararacteristics such a x & y offsets  display and
* buffer dimensions
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/

WORD32 ihevcd_get_frame_dimensions(iv_obj_t *ps_codec_obj,
                                   void *pv_api_ip,
                                   void *pv_api_op)
{
    ihevcd_cxa_ctl_get_frame_dimensions_ip_t *ps_ip;
    ihevcd_cxa_ctl_get_frame_dimensions_op_t *ps_op;
    codec_t *ps_codec = (codec_t *)ps_codec_obj->pv_codec_handle;
    WORD32 disp_wd, disp_ht, buffer_wd, buffer_ht, x_offset, y_offset;
    ps_ip = (ihevcd_cxa_ctl_get_frame_dimensions_ip_t *)pv_api_ip;
    ps_op = (ihevcd_cxa_ctl_get_frame_dimensions_op_t *)pv_api_op;
    UNUSED(ps_ip);
    if(ps_codec->i4_sps_done)
    {
        disp_wd = ps_codec->i4_disp_wd;
        disp_ht = ps_codec->i4_disp_ht;

        if(0 == ps_codec->i4_share_disp_buf)
        {
            buffer_wd = disp_wd;
            buffer_ht = disp_ht;
        }
        else
        {
            buffer_wd = ps_codec->i4_strd;
            buffer_ht = ps_codec->i4_ht + PAD_HT;
        }
    }
    else
    {

        disp_wd = 0;
        disp_ht = 0;

        if(0 == ps_codec->i4_share_disp_buf)
        {
            buffer_wd = disp_wd;
            buffer_ht = disp_ht;
        }
        else
        {
            buffer_wd = ALIGN16(disp_wd) + PAD_WD;
            buffer_ht = ALIGN16(disp_ht) + PAD_HT;

        }
    }
    if(ps_codec->i4_strd > buffer_wd)
        buffer_wd = ps_codec->i4_strd;

    if(0 == ps_codec->i4_share_disp_buf)
    {
        x_offset = 0;
        y_offset = 0;
    }
    else
    {
        y_offset = PAD_TOP;
        x_offset = PAD_LEFT;
    }

    ps_op->u4_disp_wd[0] = disp_wd;
    ps_op->u4_disp_ht[0] = disp_ht;
    ps_op->u4_buffer_wd[0] = buffer_wd;
    ps_op->u4_buffer_ht[0] = buffer_ht;
    ps_op->u4_x_offset[0] = x_offset;
    ps_op->u4_y_offset[0] = y_offset;

    ps_op->u4_disp_wd[1] = ps_op->u4_disp_wd[2] = ((ps_op->u4_disp_wd[0] + 1)
                    >> 1);
    ps_op->u4_disp_ht[1] = ps_op->u4_disp_ht[2] = ((ps_op->u4_disp_ht[0] + 1)
                    >> 1);
    ps_op->u4_buffer_wd[1] = ps_op->u4_buffer_wd[2] = (ps_op->u4_buffer_wd[0]
                    >> 1);
    ps_op->u4_buffer_ht[1] = ps_op->u4_buffer_ht[2] = (ps_op->u4_buffer_ht[0]
                    >> 1);
    ps_op->u4_x_offset[1] = ps_op->u4_x_offset[2] = (ps_op->u4_x_offset[0]
                    >> 1);
    ps_op->u4_y_offset[1] = ps_op->u4_y_offset[2] = (ps_op->u4_y_offset[0]
                    >> 1);

    if((ps_codec->e_chroma_fmt == IV_YUV_420SP_UV)
                    || (ps_codec->e_chroma_fmt == IV_YUV_420SP_VU))
    {
        ps_op->u4_disp_wd[2] = 0;
        ps_op->u4_disp_ht[2] = 0;
        ps_op->u4_buffer_wd[2] = 0;
        ps_op->u4_buffer_ht[2] = 0;
        ps_op->u4_x_offset[2] = 0;
        ps_op->u4_y_offset[2] = 0;

        ps_op->u4_disp_wd[1] <<= 1;
        ps_op->u4_buffer_wd[1] <<= 1;
        ps_op->u4_x_offset[1] <<= 1;
    }

    return IV_SUCCESS;

}


/**
*******************************************************************************
*
* @brief
*  Gets vui parameters
*
* @par Description:
*  Gets VUI parameters
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_get_vui_params(iv_obj_t *ps_codec_obj,
                             void *pv_api_ip,
                             void *pv_api_op)
{
    ihevcd_cxa_ctl_get_vui_params_ip_t *ps_ip;
    ihevcd_cxa_ctl_get_vui_params_op_t *ps_op;
    codec_t *ps_codec = (codec_t *)ps_codec_obj->pv_codec_handle;
    sps_t *ps_sps;
    vui_t *ps_vui;
    WORD32 i;

    ps_ip = (ihevcd_cxa_ctl_get_vui_params_ip_t *)pv_api_ip;
    ps_op = (ihevcd_cxa_ctl_get_vui_params_op_t *)pv_api_op;

    if(0 == ps_codec->i4_sps_done)
    {
        ps_op->u4_error_code = IHEVCD_VUI_PARAMS_NOT_FOUND;
        return IV_FAIL;
    }

    ps_sps = ps_codec->s_parse.ps_sps;
    if(0 == ps_sps->i1_sps_valid || 0 == ps_sps->i1_vui_parameters_present_flag)
    {
        WORD32 sps_idx = 0;
        ps_sps = ps_codec->ps_sps_base;

        while((0 == ps_sps->i1_sps_valid) || (0 == ps_sps->i1_vui_parameters_present_flag))
        {
            sps_idx++;
            ps_sps++;

            if(sps_idx == MAX_SPS_CNT - 1)
            {
                ps_op->u4_error_code = IHEVCD_VUI_PARAMS_NOT_FOUND;
                return IV_FAIL;
            }
        }
    }

    ps_vui = &ps_sps->s_vui_parameters;
    UNUSED(ps_ip);

    ps_op->u1_aspect_ratio_info_present_flag         =  ps_vui->u1_aspect_ratio_info_present_flag;
    ps_op->u1_aspect_ratio_idc                       =  ps_vui->u1_aspect_ratio_idc;
    ps_op->u2_sar_width                              =  ps_vui->u2_sar_width;
    ps_op->u2_sar_height                             =  ps_vui->u2_sar_height;
    ps_op->u1_overscan_info_present_flag             =  ps_vui->u1_overscan_info_present_flag;
    ps_op->u1_overscan_appropriate_flag              =  ps_vui->u1_overscan_appropriate_flag;
    ps_op->u1_video_signal_type_present_flag         =  ps_vui->u1_video_signal_type_present_flag;
    ps_op->u1_video_format                           =  ps_vui->u1_video_format;
    ps_op->u1_video_full_range_flag                  =  ps_vui->u1_video_full_range_flag;
    ps_op->u1_colour_description_present_flag        =  ps_vui->u1_colour_description_present_flag;
    ps_op->u1_colour_primaries                       =  ps_vui->u1_colour_primaries;
    ps_op->u1_transfer_characteristics               =  ps_vui->u1_transfer_characteristics;
    ps_op->u1_matrix_coefficients                    =  ps_vui->u1_matrix_coefficients;
    ps_op->u1_chroma_loc_info_present_flag           =  ps_vui->u1_chroma_loc_info_present_flag;
    ps_op->u1_chroma_sample_loc_type_top_field       =  ps_vui->u1_chroma_sample_loc_type_top_field;
    ps_op->u1_chroma_sample_loc_type_bottom_field    =  ps_vui->u1_chroma_sample_loc_type_bottom_field;
    ps_op->u1_neutral_chroma_indication_flag         =  ps_vui->u1_neutral_chroma_indication_flag;
    ps_op->u1_field_seq_flag                         =  ps_vui->u1_field_seq_flag;
    ps_op->u1_frame_field_info_present_flag          =  ps_vui->u1_frame_field_info_present_flag;
    ps_op->u1_default_display_window_flag            =  ps_vui->u1_default_display_window_flag;
    ps_op->u4_def_disp_win_left_offset               =  ps_vui->u4_def_disp_win_left_offset;
    ps_op->u4_def_disp_win_right_offset              =  ps_vui->u4_def_disp_win_right_offset;
    ps_op->u4_def_disp_win_top_offset                =  ps_vui->u4_def_disp_win_top_offset;
    ps_op->u4_def_disp_win_bottom_offset             =  ps_vui->u4_def_disp_win_bottom_offset;
    ps_op->u1_vui_hrd_parameters_present_flag        =  ps_vui->u1_vui_hrd_parameters_present_flag;
    ps_op->u1_vui_timing_info_present_flag           =  ps_vui->u1_vui_timing_info_present_flag;
    ps_op->u4_vui_num_units_in_tick                  =  ps_vui->u4_vui_num_units_in_tick;
    ps_op->u4_vui_time_scale                         =  ps_vui->u4_vui_time_scale;
    ps_op->u1_poc_proportional_to_timing_flag        =  ps_vui->u1_poc_proportional_to_timing_flag;
    ps_op->u1_num_ticks_poc_diff_one_minus1          =  ps_vui->u1_num_ticks_poc_diff_one_minus1;
    ps_op->u1_bitstream_restriction_flag             =  ps_vui->u1_bitstream_restriction_flag;
    ps_op->u1_tiles_fixed_structure_flag             =  ps_vui->u1_tiles_fixed_structure_flag;
    ps_op->u1_motion_vectors_over_pic_boundaries_flag =  ps_vui->u1_motion_vectors_over_pic_boundaries_flag;
    ps_op->u1_restricted_ref_pic_lists_flag          =  ps_vui->u1_restricted_ref_pic_lists_flag;
    ps_op->u4_min_spatial_segmentation_idc           =  ps_vui->u4_min_spatial_segmentation_idc;
    ps_op->u1_max_bytes_per_pic_denom                =  ps_vui->u1_max_bytes_per_pic_denom;
    ps_op->u1_max_bits_per_mincu_denom               =  ps_vui->u1_max_bits_per_mincu_denom;
    ps_op->u1_log2_max_mv_length_horizontal          =  ps_vui->u1_log2_max_mv_length_horizontal;
    ps_op->u1_log2_max_mv_length_vertical            =  ps_vui->u1_log2_max_mv_length_vertical;


    /* HRD parameters */
    ps_op->u1_timing_info_present_flag                         =    ps_vui->s_vui_hrd_parameters.u1_timing_info_present_flag;
    ps_op->u4_num_units_in_tick                                =    ps_vui->s_vui_hrd_parameters.u4_num_units_in_tick;
    ps_op->u4_time_scale                                       =    ps_vui->s_vui_hrd_parameters.u4_time_scale;
    ps_op->u1_nal_hrd_parameters_present_flag                  =    ps_vui->s_vui_hrd_parameters.u1_nal_hrd_parameters_present_flag;
    ps_op->u1_vcl_hrd_parameters_present_flag                  =    ps_vui->s_vui_hrd_parameters.u1_vcl_hrd_parameters_present_flag;
    ps_op->u1_cpbdpb_delays_present_flag                       =    ps_vui->s_vui_hrd_parameters.u1_cpbdpb_delays_present_flag;
    ps_op->u1_sub_pic_cpb_params_present_flag                  =    ps_vui->s_vui_hrd_parameters.u1_sub_pic_cpb_params_present_flag;
    ps_op->u1_tick_divisor_minus2                              =    ps_vui->s_vui_hrd_parameters.u1_tick_divisor_minus2;
    ps_op->u1_du_cpb_removal_delay_increment_length_minus1     =    ps_vui->s_vui_hrd_parameters.u1_du_cpb_removal_delay_increment_length_minus1;
    ps_op->u1_sub_pic_cpb_params_in_pic_timing_sei_flag        =    ps_vui->s_vui_hrd_parameters.u1_sub_pic_cpb_params_in_pic_timing_sei_flag;
    ps_op->u1_dpb_output_delay_du_length_minus1                =    ps_vui->s_vui_hrd_parameters.u1_dpb_output_delay_du_length_minus1;
    ps_op->u4_bit_rate_scale                                   =    ps_vui->s_vui_hrd_parameters.u4_bit_rate_scale;
    ps_op->u4_cpb_size_scale                                   =    ps_vui->s_vui_hrd_parameters.u4_cpb_size_scale;
    ps_op->u4_cpb_size_du_scale                                =    ps_vui->s_vui_hrd_parameters.u4_cpb_size_du_scale;
    ps_op->u1_initial_cpb_removal_delay_length_minus1          =    ps_vui->s_vui_hrd_parameters.u1_initial_cpb_removal_delay_length_minus1;
    ps_op->u1_au_cpb_removal_delay_length_minus1               =    ps_vui->s_vui_hrd_parameters.u1_au_cpb_removal_delay_length_minus1;
    ps_op->u1_dpb_output_delay_length_minus1                   =    ps_vui->s_vui_hrd_parameters.u1_dpb_output_delay_length_minus1;

    for(i = 0; i < 6; i++)
    {
        ps_op->au1_fixed_pic_rate_general_flag[i]                  =    ps_vui->s_vui_hrd_parameters.au1_fixed_pic_rate_general_flag[i];
        ps_op->au1_fixed_pic_rate_within_cvs_flag[i]               =    ps_vui->s_vui_hrd_parameters.au1_fixed_pic_rate_within_cvs_flag[i];
        ps_op->au1_elemental_duration_in_tc_minus1[i]              =    ps_vui->s_vui_hrd_parameters.au1_elemental_duration_in_tc_minus1[i];
        ps_op->au1_low_delay_hrd_flag[i]                           =    ps_vui->s_vui_hrd_parameters.au1_low_delay_hrd_flag[i];
        ps_op->au1_cpb_cnt_minus1[i]                               =    ps_vui->s_vui_hrd_parameters.au1_cpb_cnt_minus1[i];
    }


    return IV_SUCCESS;
}

/**
*******************************************************************************
*
* @brief
*  Gets SEI mastering display color volume parameters
*
* @par Description:
*  Gets SEI mastering display color volume parameters
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_get_sei_mastering_params(iv_obj_t *ps_codec_obj,
                             void *pv_api_ip,
                             void *pv_api_op)
{
    ihevcd_cxa_ctl_get_sei_mastering_params_ip_t *ps_ip;
    ihevcd_cxa_ctl_get_sei_mastering_params_op_t *ps_op;
    codec_t *ps_codec = (codec_t *)ps_codec_obj->pv_codec_handle;
    sei_params_t *ps_sei;
    mastering_dis_col_vol_sei_params_t *ps_mastering_dis_col_vol;
    WORD32 i;

    ps_ip = (ihevcd_cxa_ctl_get_sei_mastering_params_ip_t *)pv_api_ip;
    ps_op = (ihevcd_cxa_ctl_get_sei_mastering_params_op_t *)pv_api_op;
    UNUSED(ps_ip);
    if(NULL == ps_codec->ps_disp_buf)
    {
        ps_op->u4_error_code = IHEVCD_SEI_MASTERING_PARAMS_NOT_FOUND;
        return IV_FAIL;
    }
    ps_sei = &ps_codec->ps_disp_buf->s_sei_params;
    if((0 == ps_sei->i4_sei_mastering_disp_colour_vol_params_present_flags)
                    || (0 == ps_sei->i1_sei_parameters_present_flag))
    {
        ps_op->u4_error_code = IHEVCD_SEI_MASTERING_PARAMS_NOT_FOUND;
        return IV_FAIL;
    }

    ps_mastering_dis_col_vol = &ps_sei->s_mastering_dis_col_vol_sei_params;

    for(i = 0; i < 3; i++)
    {
        ps_op->au2_display_primaries_x[i] =
                    ps_mastering_dis_col_vol->au2_display_primaries_x[i];

        ps_op->au2_display_primaries_y[i] =
                    ps_mastering_dis_col_vol->au2_display_primaries_y[i];
    }

    ps_op->u2_white_point_x = ps_mastering_dis_col_vol->u2_white_point_x;

    ps_op->u2_white_point_y = ps_mastering_dis_col_vol->u2_white_point_y;

    ps_op->u4_max_display_mastering_luminance =
                    ps_mastering_dis_col_vol->u4_max_display_mastering_luminance;

    ps_op->u4_min_display_mastering_luminance =
                    ps_mastering_dis_col_vol->u4_min_display_mastering_luminance;

    return IV_SUCCESS;
}

/**
*******************************************************************************
*
* @brief
*  Sets Processor type
*
* @par Description:
*  Sets Processor type
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/

WORD32 ihevcd_set_processor(iv_obj_t *ps_codec_obj,
                            void *pv_api_ip,
                            void *pv_api_op)
{
    ihevcd_cxa_ctl_set_processor_ip_t *ps_ip;
    ihevcd_cxa_ctl_set_processor_op_t *ps_op;
    codec_t *ps_codec = (codec_t *)ps_codec_obj->pv_codec_handle;

    ps_ip = (ihevcd_cxa_ctl_set_processor_ip_t *)pv_api_ip;
    ps_op = (ihevcd_cxa_ctl_set_processor_op_t *)pv_api_op;

    ps_codec->e_processor_arch = (IVD_ARCH_T)ps_ip->u4_arch;
    ps_codec->e_processor_soc = (IVD_SOC_T)ps_ip->u4_soc;

    ihevcd_init_function_ptr(ps_codec);

    ihevcd_update_function_ptr(ps_codec);

    if(ps_codec->e_processor_soc && (ps_codec->e_processor_soc <= SOC_HISI_37X))
    {
        /* 8th bit indicates if format conversion is to be done ahead */
        if(ps_codec->e_processor_soc & 0x80)
            ps_codec->u4_enable_fmt_conv_ahead = 1;

        /* Lower 7 bit indicate NCTB - if non-zero */
        ps_codec->e_processor_soc &= 0x7F;

        if(ps_codec->e_processor_soc)
            ps_codec->u4_nctb = ps_codec->e_processor_soc;


    }

    if((ps_codec->e_processor_soc == SOC_HISI_37X) && (ps_codec->i4_num_cores == 2))
    {
        ps_codec->u4_nctb = 2;
    }


    ps_op->u4_error_code = 0;
    return IV_SUCCESS;
}

/**
*******************************************************************************
*
* @brief
*  Sets Number of cores that can be used in the codec. Codec uses these many
* threads for decoding
*
* @par Description:
*  Sets number of cores
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/

WORD32 ihevcd_set_num_cores(iv_obj_t *ps_codec_obj,
                            void *pv_api_ip,
                            void *pv_api_op)
{
    ihevcd_cxa_ctl_set_num_cores_ip_t *ps_ip;
    ihevcd_cxa_ctl_set_num_cores_op_t *ps_op;
    codec_t *ps_codec = (codec_t *)ps_codec_obj->pv_codec_handle;

    ps_ip = (ihevcd_cxa_ctl_set_num_cores_ip_t *)pv_api_ip;
    ps_op = (ihevcd_cxa_ctl_set_num_cores_op_t *)pv_api_op;

#ifdef MULTICORE
    ps_codec->i4_num_cores = ps_ip->u4_num_cores;
#else
    ps_codec->i4_num_cores = 1;
#endif
    ps_op->u4_error_code = 0;
    return IV_SUCCESS;
}
/**
*******************************************************************************
*
* @brief
*  Codec control call
*
* @par Description:
*  Codec control call which in turn calls appropriate calls  based on
* subcommand
*
* @param[in] ps_codec_obj
*  Pointer to codec object at API level
*
* @param[in] pv_api_ip
*  Pointer to input argument structure
*
* @param[out] pv_api_op
*  Pointer to output argument structure
*
* @returns  Status
*
* @remarks
*
*
*******************************************************************************
*/

WORD32 ihevcd_ctl(iv_obj_t *ps_codec_obj, void *pv_api_ip, void *pv_api_op)
{
    ivd_ctl_set_config_ip_t *ps_ctl_ip;
    ivd_ctl_set_config_op_t *ps_ctl_op;
    WORD32 ret = 0;
    WORD32 subcommand;
    codec_t *ps_codec = (codec_t *)ps_codec_obj->pv_codec_handle;

    ps_ctl_ip = (ivd_ctl_set_config_ip_t *)pv_api_ip;
    ps_ctl_op = (ivd_ctl_set_config_op_t *)pv_api_op;

    if(ps_codec->i4_init_done != 1)
    {
        ps_ctl_op->u4_error_code |= 1 << IVD_FATALERROR;
        ps_ctl_op->u4_error_code |= IHEVCD_INIT_NOT_DONE;
        return IV_FAIL;
    }
    subcommand = ps_ctl_ip->e_sub_cmd;

    switch(subcommand)
    {
        case IVD_CMD_CTL_GETPARAMS:
            ret = ihevcd_get_status(ps_codec_obj, (void *)pv_api_ip,
                                    (void *)pv_api_op);
            break;
        case IVD_CMD_CTL_SETPARAMS:
            ret = ihevcd_set_params(ps_codec_obj, (void *)pv_api_ip,
                                    (void *)pv_api_op);
            break;
        case IVD_CMD_CTL_RESET:
            ret = ihevcd_reset(ps_codec_obj, (void *)pv_api_ip,
                               (void *)pv_api_op);
            break;
        case IVD_CMD_CTL_SETDEFAULT:
        {
            ivd_ctl_set_config_op_t *s_ctl_dynparams_op =
                            (ivd_ctl_set_config_op_t *)pv_api_op;

            ret = ihevcd_set_default_params(ps_codec);
            if(IV_SUCCESS == ret)
                s_ctl_dynparams_op->u4_error_code = 0;
            break;
        }
        case IVD_CMD_CTL_FLUSH:
            ret = ihevcd_set_flush_mode(ps_codec_obj, (void *)pv_api_ip,
                                        (void *)pv_api_op);
            break;
        case IVD_CMD_CTL_GETBUFINFO:
            ret = ihevcd_get_buf_info(ps_codec_obj, (void *)pv_api_ip,
                                      (void *)pv_api_op);
            break;
        case IVD_CMD_CTL_GETVERSION:
        {
            ivd_ctl_getversioninfo_ip_t *ps_ip;
            ivd_ctl_getversioninfo_op_t *ps_op;
            IV_API_CALL_STATUS_T ret;
            ps_ip = (ivd_ctl_getversioninfo_ip_t *)pv_api_ip;
            ps_op = (ivd_ctl_getversioninfo_op_t *)pv_api_op;

            ps_op->u4_error_code = IV_SUCCESS;

            if((WORD32)ps_ip->u4_version_buffer_size <= 0)
            {
                ps_op->u4_error_code = IHEVCD_CXA_VERS_BUF_INSUFFICIENT;
                ret = IV_FAIL;
            }
            else
            {
                ret = ihevcd_get_version((CHAR *)ps_ip->pv_version_buffer,
                                         ps_ip->u4_version_buffer_size);
                if(ret != IV_SUCCESS)
                {
                    ps_op->u4_error_code = IHEVCD_CXA_VERS_BUF_INSUFFICIENT;
                    ret = IV_FAIL;
                }
            }
        }
            break;
        case IHEVCD_CXA_CMD_CTL_DEGRADE:
            ret = ihevcd_set_degrade(ps_codec_obj, (void *)pv_api_ip,
                            (void *)pv_api_op);
            break;
        case IHEVCD_CXA_CMD_CTL_SET_NUM_CORES:
            ret = ihevcd_set_num_cores(ps_codec_obj, (void *)pv_api_ip,
                                       (void *)pv_api_op);
            break;
        case IHEVCD_CXA_CMD_CTL_GET_BUFFER_DIMENSIONS:
            ret = ihevcd_get_frame_dimensions(ps_codec_obj, (void *)pv_api_ip,
                                              (void *)pv_api_op);
            break;
        case IHEVCD_CXA_CMD_CTL_GET_VUI_PARAMS:
            ret = ihevcd_get_vui_params(ps_codec_obj, (void *)pv_api_ip,
                                        (void *)pv_api_op);
            break;
        case IHEVCD_CXA_CMD_CTL_GET_SEI_MASTERING_PARAMS:
            ret = ihevcd_get_sei_mastering_params(ps_codec_obj, (void *)pv_api_ip,
                                        (void *)pv_api_op);
            break;
        case IHEVCD_CXA_CMD_CTL_SET_PROCESSOR:
            ret = ihevcd_set_processor(ps_codec_obj, (void *)pv_api_ip,
                            (void *)pv_api_op);
            break;
        default:
            DEBUG("\nDo nothing\n");
            break;
    }

    return ret;
}

/**
*******************************************************************************
*
* @brief
*  Codecs entry point function. All the function calls to  the codec are
* done using this function with different  values specified in command
*
* @par Description:
*  Arguments are tested for validity and then based on the  command
* appropriate function is called
*
* @param[in] ps_handle
*  API level handle for codec
*
* @param[in] pv_api_ip
*  Input argument structure
*
* @param[out] pv_api_op
*  Output argument structure
*
* @returns  Status of the function corresponding to command
*
* @remarks
*
*
*******************************************************************************
*/
IV_API_CALL_STATUS_T ihevcd_cxa_api_function(iv_obj_t *ps_handle,
                                             void *pv_api_ip,
                                             void *pv_api_op)
{
    WORD32 command;
    UWORD32 *pu4_ptr_cmd;
    WORD32 ret = 0;
    IV_API_CALL_STATUS_T e_status;
    e_status = api_check_struct_sanity(ps_handle, pv_api_ip, pv_api_op);

    if(e_status != IV_SUCCESS)
    {
        DEBUG("error code = %d\n", *((UWORD32 *)pv_api_op + 1));
        return IV_FAIL;
    }

    pu4_ptr_cmd = (UWORD32 *)pv_api_ip;
    pu4_ptr_cmd++;

    command = *pu4_ptr_cmd;

    switch(command)
    {
        case IVD_CMD_CREATE:
            ret = ihevcd_create(ps_handle, (void *)pv_api_ip, (void *)pv_api_op);
            break;
        case IVD_CMD_DELETE:
            ret = ihevcd_delete(ps_handle, (void *)pv_api_ip, (void *)pv_api_op);
            break;

        case IVD_CMD_VIDEO_DECODE:
            ret = ihevcd_decode(ps_handle, (void *)pv_api_ip, (void *)pv_api_op);
            break;

        case IVD_CMD_GET_DISPLAY_FRAME:
            //ret = ihevcd_get_display_frame(ps_handle,(void *)pv_api_ip,(void *)pv_api_op);
            break;

        case IVD_CMD_SET_DISPLAY_FRAME:
            ret = ihevcd_set_display_frame(ps_handle, (void *)pv_api_ip,
                                           (void *)pv_api_op);

            break;

        case IVD_CMD_REL_DISPLAY_FRAME:
            ret = ihevcd_rel_display_frame(ps_handle, (void *)pv_api_ip,
                                           (void *)pv_api_op);
            break;

        case IVD_CMD_VIDEO_CTL:
            ret = ihevcd_ctl(ps_handle, (void *)pv_api_ip, (void *)pv_api_op);
            break;
        default:
            ret = IV_FAIL;
            break;
    }

    return (IV_API_CALL_STATUS_T)ret;
}

