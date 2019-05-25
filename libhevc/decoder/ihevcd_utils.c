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
*  ihevcd_utils.c
*
* @brief
*  Contains miscellaneous utility functions such as init() etc
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
/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ihevc_typedefs.h"
#include "iv.h"
#include "ivd.h"
#include "ihevcd_cxa.h"
#include "ithread.h"

#include "ihevc_defs.h"
#include "ihevc_debug.h"
#include "ihevc_defs.h"
#include "ihevc_error.h"
#include "ihevc_structs.h"
#include "ihevc_buf_mgr.h"
#include "ihevc_dpb_mgr.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"

#include "ihevc_common_tables.h"
#include "ihevc_buf_mgr.h"
#include "ihevc_disp_mgr.h"
#include "ihevc_cabac_tables.h"

#include "ihevcd_defs.h"

#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_nal.h"
#include "ihevcd_bitstream.h"
#include "ihevcd_utils.h"
#include "ihevcd_trace.h"
#include "ihevcd_process_slice.h"
#include "ihevcd_job_queue.h"
#define MAX_DPB_PIC_BUF 6

/* Function declarations */
mv_buf_t* ihevcd_mv_mgr_get_poc(buf_mgr_t *ps_mv_buf_mgr, UWORD32 abs_poc);

/**
*******************************************************************************
*
* @brief
*  Used to get level index for a given level
*
* @par Description:
*  Converts from level_idc (which is multiplied by 30) to an index that can be
*  used as a lookup. Also used to ignore invalid levels like 2.2 , 3.2 etc
*
* @param[in] level
*  Level of the stream
*
* @returns  Level index for a given level
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_get_lvl_idx(WORD32 level)
{
    WORD32 lvl_idx = 0;

    if(level < IHEVC_LEVEL_20)
    {
        lvl_idx = 0;
    }
    else if(level >= IHEVC_LEVEL_20 && level < IHEVC_LEVEL_21)
    {
        lvl_idx = 1;
    }
    else if(level >= IHEVC_LEVEL_21 && level < IHEVC_LEVEL_30)
    {
        lvl_idx = 2;
    }
    else if(level >= IHEVC_LEVEL_30 && level < IHEVC_LEVEL_31)
    {
        lvl_idx = 3;
    }
    else if(level >= IHEVC_LEVEL_31 && level < IHEVC_LEVEL_40)
    {
        lvl_idx = 4;
    }
    else if(level >= IHEVC_LEVEL_40 && level < IHEVC_LEVEL_41)
    {
        lvl_idx = 5;
    }
    else if(level >= IHEVC_LEVEL_41 && level < IHEVC_LEVEL_50)
    {
        lvl_idx = 6;
    }
    else if(level >= IHEVC_LEVEL_50 && level < IHEVC_LEVEL_51)
    {
        lvl_idx = 7;
    }
    else if(level >= IHEVC_LEVEL_51 && level < IHEVC_LEVEL_52)
    {
        lvl_idx = 8;
    }
    else if(level >= IHEVC_LEVEL_52 && level < IHEVC_LEVEL_60)
    {
        lvl_idx = 9;
    }
    else if(level >= IHEVC_LEVEL_60 && level < IHEVC_LEVEL_61)
    {
        lvl_idx = 10;
    }
    else if(level >= IHEVC_LEVEL_61 && level < IHEVC_LEVEL_62)
    {
        lvl_idx = 11;
    }
    else if(level >= IHEVC_LEVEL_62)
    {
        lvl_idx = 12;
    }

    return (lvl_idx);
}

/**
*******************************************************************************
*
* @brief
*  Used to get reference picture buffer size for a given level and
*  and padding used
*
* @par Description:
*  Used to get reference picture buffer size for a given level and padding used
*  Each picture is padded on all four sides
*
* @param[in] pic_size
*  Mumber of luma samples (Width * Height)
*
* @param[in] level
*  Level
*
* @param[in] horz_pad
*  Total padding used in horizontal direction
*
* @param[in] vert_pad
*  Total padding used in vertical direction
*
* @returns  Total picture buffer size
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_get_total_pic_buf_size(codec_t *ps_codec,
                                     WORD32 wd,
                                     WORD32 ht)
{
    WORD32 size;
    WORD32 num_luma_samples;
    WORD32 max_dpb_size;
    WORD32 num_samples;


    sps_t *ps_sps = (ps_codec->s_parse.ps_sps_base + ps_codec->i4_sps_id);

    /* Get maximum number of buffers for the current picture size */
    max_dpb_size = ps_sps->ai1_sps_max_dec_pic_buffering[ps_sps->i1_sps_max_sub_layers - 1];

    if(ps_codec->e_frm_out_mode != IVD_DECODE_FRAME_OUT)
        max_dpb_size += ps_sps->ai1_sps_max_num_reorder_pics[ps_sps->i1_sps_max_sub_layers - 1];

    max_dpb_size++;
    /* Allocation is required for
     * (Wd + horz_pad) * (Ht + vert_pad) * (2 * max_dpb_size + 1)
     */

    /* Account for padding area */
    num_luma_samples = (wd + PAD_WD) * (ht + PAD_HT);

    /* Account for chroma */
    num_samples = num_luma_samples * 3 / 2;

    /* Number of bytes in reference pictures */
    size = num_samples * max_dpb_size;


    return size;
}
/**
*******************************************************************************
*
* @brief
*  Used to get MV bank size for a given number of luma samples
*
* @par Description:
*  For given number of luma samples  one MV bank size is computed
*  Each MV bank includes pu_map and pu_t for all the min PUs(4x4) in a picture
*
* @param[in] num_luma_samples
*  Max number of luma pixels in the frame
*
* @returns  Total MV Bank size
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_get_pic_mv_bank_size(WORD32 num_luma_samples)
{
    WORD32 size;

    WORD32 pic_size;

    WORD32 mv_bank_size;
    WORD32 num_pu;
    WORD32 num_ctb;
    pic_size = num_luma_samples;


    num_pu = pic_size / (MIN_PU_SIZE * MIN_PU_SIZE);
    num_ctb = pic_size / (MIN_CTB_SIZE * MIN_CTB_SIZE);

    mv_bank_size = 0;

    /* Size for storing pu_t start index each CTB */
    /* One extra entry is needed to compute number of PUs in the last CTB */
    mv_bank_size += (num_ctb + 1) * sizeof(WORD32);

    /* Size for pu_map */
    mv_bank_size += num_pu;

    /* Size for storing pu_t for each PU */
    mv_bank_size += num_pu * sizeof(pu_t);

    /* Size for storing slice_idx for each CTB */
    mv_bank_size += ALIGN4(num_ctb * sizeof(UWORD16));

    size =  mv_bank_size;
    return size;
}
/**
*******************************************************************************
*
* @brief
*  Used to get TU data size for a given number luma samples
*
* @par Description:
*  For a given number of luma samples TU data size is computed
*  Each TU data includes tu_map and tu_t and coeff data for all
*  the min TUs(4x4) in given CTB
*
* @param[in] num_luma_samples
*  Number of 64 x 64 CTBs for which TU data has to be allocated.
*
* @returns  Total TU data size
*
* @remarks Assumption is num_luma_samples will be at least
* 64 x 64 to handle CTB of size 64 x 64. Can be frame size as well
*
*******************************************************************************
*/
WORD32 ihevcd_get_tu_data_size(WORD32 num_luma_samples)
{


    WORD32 tu_data_size;
    WORD32 num_ctb;
    WORD32 num_luma_tu, num_chroma_tu, num_tu;
    num_ctb = num_luma_samples / (MIN_CTB_SIZE * MIN_CTB_SIZE);

    num_luma_tu = num_luma_samples / (MIN_TU_SIZE * MIN_TU_SIZE);
    num_chroma_tu = num_luma_tu >> 1;

    num_tu = num_luma_tu + num_chroma_tu;
    tu_data_size = 0;

    /* Size for storing tu_t start index each CTB */
    /* One extra entry is needed to compute number of TUs in the last CTB */
    tu_data_size += (num_ctb + 1) * sizeof(WORD32);

    /* Size for storing tu map */
    tu_data_size += num_luma_tu * sizeof(UWORD8);

    /* Size for storing tu_t for each TU */
    tu_data_size += num_tu * sizeof(tu_t);

    /* Size for storing number of coded subblocks and scan_idx for each TU */
    tu_data_size += num_tu * (sizeof(WORD8) + sizeof(WORD8));

    /* Size for storing coeff data for each TU */
    tu_data_size += num_tu * sizeof(tu_sblk_coeff_data_t);


    return tu_data_size;
}


WORD32 ihevcd_nctb_cnt(codec_t *ps_codec, sps_t *ps_sps)
{
    WORD32 nctb = 1;
    UNUSED(ps_codec);
    //TODO: Currently set to 1
    /* If CTB size is less than 32 x 32 then set nCTB as 4 */
    if(ps_sps->i1_log2_ctb_size < 5)
        nctb = 1;

    return nctb;
}

IHEVCD_ERROR_T ihevcd_get_tile_pos(pps_t *ps_pps,
                                   sps_t *ps_sps,
                                   WORD32 ctb_x,
                                   WORD32 ctb_y,
                                   WORD32 *pi4_ctb_tile_x,
                                   WORD32 *pi4_ctb_tile_y,
                                   WORD32 *pi4_tile_idx)
{

    tile_t *ps_tile_tmp;
    WORD32 i;
    WORD32 tile_row, tile_col;

    if(ctb_x < 0 || ctb_y < 0)
    {
        *pi4_ctb_tile_x = 0;
        *pi4_ctb_tile_y = 0;
        *pi4_tile_idx = 0;

        return (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    }

    tile_row = 0;
    tile_col = 0;
    ps_tile_tmp = ps_pps->ps_tile;
    if(0 == ps_pps->i1_tiles_enabled_flag)
    {
        *pi4_ctb_tile_x = ctb_x;
        *pi4_ctb_tile_y = ctb_y;
        *pi4_tile_idx = 0;
    }
    else
    {
        for(i = 0; i < ps_pps->i1_num_tile_columns; i++)
        {
            WORD16 next_tile_ctb_x;
            ps_tile_tmp = ps_pps->ps_tile + i; //* ps_pps->i1_num_tile_rows;
            if((ps_pps->i1_num_tile_columns - 1) == i)
            {
                next_tile_ctb_x = ps_sps->i2_pic_wd_in_ctb;
            }
            else
            {
                tile_t *ps_tile_next_tmp;
                ps_tile_next_tmp = ps_pps->ps_tile + i + 1;
                next_tile_ctb_x = ps_tile_next_tmp->u1_pos_x;
            }
            if((ctb_x >= ps_tile_tmp->u1_pos_x) && (ctb_x < next_tile_ctb_x))
            {
                tile_col = i;
                break;
            }
        }
        *pi4_ctb_tile_x = ctb_x - ps_tile_tmp->u1_pos_x;

        for(i = 0; i < ps_pps->i1_num_tile_rows; i++)
        {
            WORD16 next_tile_ctb_y;
            ps_tile_tmp = ps_pps->ps_tile + i * ps_pps->i1_num_tile_columns;
            if((ps_pps->i1_num_tile_rows - 1) == i)
            {
                next_tile_ctb_y = ps_sps->i2_pic_ht_in_ctb;
            }
            else
            {
                tile_t *ps_tile_next_tmp;
                ps_tile_next_tmp = ps_pps->ps_tile + ((i + 1) * ps_pps->i1_num_tile_columns);
                next_tile_ctb_y = ps_tile_next_tmp->u1_pos_y;
            }
            if((ctb_y >= ps_tile_tmp->u1_pos_y) && (ctb_y < next_tile_ctb_y))
            {
                tile_row = i;
                break;
            }

        }
        *pi4_ctb_tile_y = ctb_y - ps_tile_tmp->u1_pos_y;
        *pi4_tile_idx = tile_row * ps_pps->i1_num_tile_columns
                        + tile_col;
    }
    return (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
}
/**
*******************************************************************************
*
* @brief
*  Function to initialize ps_pic_buf structs add pic buffers to
*  buffer manager in case of non-shared mode
*
* @par Description:
*  Function to initialize ps_pic_buf structs add pic buffers to
*  buffer manager in case of non-shared mode
*  To be called once per stream or for every reset
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns  Error from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_pic_buf_mgr_add_bufs(codec_t *ps_codec)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 i;
    WORD32 max_dpb_size;
    sps_t *ps_sps;
    UWORD8 *pu1_buf;
    pic_buf_t *ps_pic_buf;
    WORD32 pic_buf_size_allocated;




    /* Initialize Pic buffer manager */
    ps_sps = ps_codec->s_parse.ps_sps;

    /* Compute the number of Pic buffers needed */
    max_dpb_size = ps_sps->ai1_sps_max_dec_pic_buffering[ps_sps->i1_sps_max_sub_layers - 1];

    if(ps_codec->e_frm_out_mode != IVD_DECODE_FRAME_OUT)
        max_dpb_size += ps_sps->ai1_sps_max_num_reorder_pics[ps_sps->i1_sps_max_sub_layers - 1];

    /* Allocate one extra picture to handle current frame
     * In case of asynchronous parsing and processing, number of buffers should increase here
     * based on when parsing and processing threads are synchronized
     */
    max_dpb_size++;


    pu1_buf = (UWORD8 *)ps_codec->pu1_ref_pic_buf_base;

    ps_pic_buf = (pic_buf_t *)ps_codec->ps_pic_buf;

    /* In case of non-shared mode, add picture buffers to buffer manager
     * In case of shared mode buffers are added in the run-time
     */
    if(0 == ps_codec->i4_share_disp_buf)
    {
        WORD32 buf_ret;
        WORD32 luma_samples;
        WORD32 chroma_samples;
        pic_buf_size_allocated = ps_codec->i4_total_pic_buf_size;

        luma_samples = (ps_codec->i4_strd) *
                        (ps_sps->i2_pic_height_in_luma_samples + PAD_HT);

        chroma_samples = luma_samples / 2;

        /* Try to add as many buffers as possible since memory is already allocated */
        /* If the number of buffers that can be added is less than max_num_bufs
         * return with an error.
         */
        for(i = 0; i < max_dpb_size; i++)
        {
            pic_buf_size_allocated -= (luma_samples + chroma_samples);

            if(pic_buf_size_allocated < 0)
            {
                ps_codec->s_parse.i4_error_code = IHEVCD_INSUFFICIENT_MEM_PICBUF;
                return IHEVCD_INSUFFICIENT_MEM_PICBUF;
            }

            ps_pic_buf->pu1_luma = pu1_buf + ps_codec->i4_strd * PAD_TOP + PAD_LEFT;
            pu1_buf += luma_samples;

            ps_pic_buf->pu1_chroma = pu1_buf + ps_codec->i4_strd * (PAD_TOP / 2) + PAD_LEFT;
            pu1_buf += chroma_samples;

            /* Pad boundary pixels (one pixel on all sides) */
            /* This ensures SAO does not read uninitialized pixels */
            /* Note these are not used in actual processing */
            {
                UWORD8 *pu1_buf;
                WORD32 strd, wd, ht;
                WORD32 i;
                strd = ps_codec->i4_strd;
                wd = ps_codec->i4_wd;
                ht = ps_codec->i4_ht;

                pu1_buf = ps_pic_buf->pu1_luma;
                for(i = 0; i < ht; i++)
                {
                    pu1_buf[-1] = 0;
                    pu1_buf[wd] = 0;
                    pu1_buf += strd;
                }
                pu1_buf = ps_pic_buf->pu1_luma;
                memset(pu1_buf - strd - 1, 0, wd + 2);

                pu1_buf += strd * ht;
                memset(pu1_buf - 1, 0, wd + 2);

                pu1_buf = ps_pic_buf->pu1_chroma;
                ht >>= 1;
                for(i = 0; i < ht; i++)
                {
                    pu1_buf[-1] = 0;
                    pu1_buf[-2] = 0;
                    pu1_buf[wd] = 0;
                    pu1_buf[wd + 1] = 0;
                    pu1_buf += strd;
                }
                pu1_buf = ps_pic_buf->pu1_chroma;
                memset(pu1_buf - strd - 2, 0, wd + 4);

                pu1_buf += strd * ht;
                memset(pu1_buf - 2, 0, wd + 4);
            }

            buf_ret = ihevc_buf_mgr_add((buf_mgr_t *)ps_codec->pv_pic_buf_mgr, ps_pic_buf, i);


            if(0 != buf_ret)
            {
                ps_codec->s_parse.i4_error_code = IHEVCD_BUF_MGR_ERROR;
                return IHEVCD_BUF_MGR_ERROR;
            }
            ps_pic_buf++;
        }
    }
    else
    {
        /* In case of shared mode, buffers are added without adjusting for padding.
           Update luma and chroma pointers here to account for padding as per stride.
           In some cases stride might not be available when set_display_frame is called.
           Hence updated luma and chroma pointers here */

        for(i = 0; i < BUF_MGR_MAX_CNT; i++)
        {
            ps_pic_buf = ihevc_buf_mgr_get_buf((buf_mgr_t *)ps_codec->pv_pic_buf_mgr, i);
            if((NULL == ps_pic_buf) ||
               (NULL == ps_pic_buf->pu1_luma) ||
               (NULL == ps_pic_buf->pu1_chroma))
            {
                break;
            }
            ps_pic_buf->pu1_luma += ps_codec->i4_strd * PAD_TOP + PAD_LEFT;
            ps_pic_buf->pu1_chroma += ps_codec->i4_strd * (PAD_TOP / 2) + PAD_LEFT;
        }
    }

    return ret;
}
/**
*******************************************************************************
*
* @brief
*  Function to add buffers to MV Bank buffer manager
*
* @par Description:
*  Function to add buffers to MV Bank buffer manager
*  To be called once per stream or for every reset
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns  Error from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_mv_buf_mgr_add_bufs(codec_t *ps_codec)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    WORD32 i;
    WORD32 max_dpb_size;
    WORD32 mv_bank_size_allocated;
    WORD32 pic_mv_bank_size;

    sps_t *ps_sps;
    UWORD8 *pu1_buf;
    mv_buf_t *ps_mv_buf;


    /* Initialize MV Bank buffer manager */
    ps_sps = ps_codec->s_parse.ps_sps;


    /* Compute the number of MV Bank buffers needed */
    max_dpb_size = ps_sps->ai1_sps_max_dec_pic_buffering[ps_sps->i1_sps_max_sub_layers - 1];

    /* Allocate one extra MV Bank to handle current frame
     * In case of asynchronous parsing and processing, number of buffers should increase here
     * based on when parsing and processing threads are synchronized
     */
    max_dpb_size++;

    ps_codec->i4_max_dpb_size = max_dpb_size;

    pu1_buf = (UWORD8 *)ps_codec->pv_mv_bank_buf_base;

    ps_mv_buf = (mv_buf_t *)pu1_buf;
    pu1_buf += max_dpb_size * sizeof(mv_buf_t);
    ps_codec->ps_mv_buf = ps_mv_buf;
    mv_bank_size_allocated = ps_codec->i4_total_mv_bank_size - max_dpb_size  * sizeof(mv_buf_t);

    /* Compute MV bank size per picture */
    pic_mv_bank_size = ihevcd_get_pic_mv_bank_size(ALIGN64(ps_sps->i2_pic_width_in_luma_samples) *
                                                   ALIGN64(ps_sps->i2_pic_height_in_luma_samples));

    for(i = 0; i < max_dpb_size; i++)
    {
        WORD32 buf_ret;
        WORD32 num_pu;
        WORD32 num_ctb;
        WORD32 pic_size;
        pic_size = ALIGN64(ps_sps->i2_pic_width_in_luma_samples) *
                        ALIGN64(ps_sps->i2_pic_height_in_luma_samples);


        num_pu = pic_size / (MIN_PU_SIZE * MIN_PU_SIZE);
        num_ctb = pic_size / (MIN_CTB_SIZE * MIN_CTB_SIZE);


        mv_bank_size_allocated -= pic_mv_bank_size;

        if(mv_bank_size_allocated < 0)
        {
            ps_codec->s_parse.i4_error_code = IHEVCD_INSUFFICIENT_MEM_MVBANK;
            return IHEVCD_INSUFFICIENT_MEM_MVBANK;
        }

        ps_mv_buf->pu4_pic_pu_idx = (UWORD32 *)pu1_buf;
        pu1_buf += (num_ctb + 1) * sizeof(WORD32);

        ps_mv_buf->pu1_pic_pu_map = pu1_buf;
        pu1_buf += num_pu;

        ps_mv_buf->pu1_pic_slice_map = (UWORD16 *)pu1_buf;
        pu1_buf += ALIGN4(num_ctb * sizeof(UWORD16));

        ps_mv_buf->ps_pic_pu = (pu_t *)pu1_buf;
        pu1_buf += num_pu * sizeof(pu_t);

        buf_ret = ihevc_buf_mgr_add((buf_mgr_t *)ps_codec->pv_mv_buf_mgr, ps_mv_buf, i);

        if(0 != buf_ret)
        {
            ps_codec->s_parse.i4_error_code = IHEVCD_BUF_MGR_ERROR;
            return IHEVCD_BUF_MGR_ERROR;
        }

        ps_mv_buf++;

    }
    return ret;
}
/**
*******************************************************************************
*
* @brief
*  Output buffer check
*
* @par Description:
*  Check for the number of buffers and buffer sizes of output buffer
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns  Error from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_check_out_buf_size(codec_t *ps_codec)
{
    ivd_out_bufdesc_t *ps_out_buffer = ps_codec->ps_out_buffer;
    UWORD32 au4_min_out_buf_size[IVD_VIDDEC_MAX_IO_BUFFERS];
    UWORD32 u4_min_num_out_bufs = 0, i;
    UWORD32 wd, ht;

    if(0 == ps_codec->i4_share_disp_buf)
    {
        wd = ps_codec->i4_disp_wd;
        ht = ps_codec->i4_disp_ht;
    }
    else
    {
        /* In case of shared mode, do not check validity of ps_codec->ps_out_buffer */
        return (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    }

    if(ps_codec->i4_disp_strd > (WORD32)wd)
        wd = ps_codec->i4_disp_strd;

    if(ps_codec->e_chroma_fmt == IV_YUV_420P)
        u4_min_num_out_bufs = MIN_OUT_BUFS_420;
    else if(ps_codec->e_chroma_fmt == IV_YUV_422ILE)
        u4_min_num_out_bufs = MIN_OUT_BUFS_422ILE;
    else if(ps_codec->e_chroma_fmt == IV_RGB_565)
        u4_min_num_out_bufs = MIN_OUT_BUFS_RGB565;
    else if(ps_codec->e_chroma_fmt == IV_RGBA_8888)
        u4_min_num_out_bufs = MIN_OUT_BUFS_RGBA8888;
    else if((ps_codec->e_chroma_fmt == IV_YUV_420SP_UV)
                    || (ps_codec->e_chroma_fmt == IV_YUV_420SP_VU))
        u4_min_num_out_bufs = MIN_OUT_BUFS_420SP;

    if(ps_codec->e_chroma_fmt == IV_YUV_420P)
    {
        au4_min_out_buf_size[0] = (wd * ht);
        au4_min_out_buf_size[1] = (wd * ht) >> 2;
        au4_min_out_buf_size[2] = (wd * ht) >> 2;
    }
    else if(ps_codec->e_chroma_fmt == IV_YUV_422ILE)
    {
        au4_min_out_buf_size[0] = (wd * ht) * 2;
        au4_min_out_buf_size[1] =
                        au4_min_out_buf_size[2] = 0;
    }
    else if(ps_codec->e_chroma_fmt == IV_RGB_565)
    {
        au4_min_out_buf_size[0] = (wd * ht) * 2;
        au4_min_out_buf_size[1] =
                        au4_min_out_buf_size[2] = 0;
    }
    else if(ps_codec->e_chroma_fmt == IV_RGBA_8888)
    {
        au4_min_out_buf_size[0] = (wd * ht) * 4;
        au4_min_out_buf_size[1] =
                        au4_min_out_buf_size[2] = 0;
    }
    else if((ps_codec->e_chroma_fmt == IV_YUV_420SP_UV)
                    || (ps_codec->e_chroma_fmt == IV_YUV_420SP_VU))
    {
        au4_min_out_buf_size[0] = (wd * ht);
        au4_min_out_buf_size[1] = (wd * ht) >> 1;
        au4_min_out_buf_size[2] = 0;
    }

    if(ps_out_buffer->u4_num_bufs < u4_min_num_out_bufs)
    {
        return (IHEVCD_ERROR_T)IV_FAIL;
    }

    for (i = 0 ; i < u4_min_num_out_bufs; i++)
    {
        if(ps_out_buffer->u4_min_out_buf_size[i] < au4_min_out_buf_size[i])
        {
            return (IHEVCD_ERROR_T)IV_FAIL;
        }
    }

    return (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
}

/**
*******************************************************************************
*
* @brief
*  Picture level initializations required during parsing
*
* @par Description:
*  Initialize picture level context variables during parsing Initialize mv
* bank buffer manager in the first init call
*
* @param[in] ps_codec
*  Pointer to codec context
*
* @returns  Error from IHEVCD_ERROR_T
*
* @remarks
*
*
*******************************************************************************
*/
IHEVCD_ERROR_T ihevcd_parse_pic_init(codec_t *ps_codec)
{
    IHEVCD_ERROR_T ret = (IHEVCD_ERROR_T)IHEVCD_SUCCESS;
    mv_buf_t *ps_mv_buf;
    sps_t *ps_sps;
    WORD32 num_min_cu;
    WORD32 cur_pic_buf_id;
    WORD32 cur_mv_bank_buf_id;
    pic_buf_t *ps_cur_pic;
    slice_header_t *ps_slice_hdr;
    UWORD8 *pu1_cur_pic_luma, *pu1_cur_pic_chroma;
    WORD32 i;

    ps_codec->s_parse.i4_error_code = IHEVCD_SUCCESS;
    ps_sps = ps_codec->s_parse.ps_sps;
    ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr;

    /* Memset picture level intra map and transquant bypass map to zero */
    num_min_cu = ((ps_sps->i2_pic_height_in_luma_samples + 7) / 8) * ((ps_sps->i2_pic_width_in_luma_samples + 63) / 64);
    memset(ps_codec->s_parse.pu1_pic_intra_flag, 0, num_min_cu);
    memset(ps_codec->s_parse.pu1_pic_no_loop_filter_flag, 0, num_min_cu);



    if(0 == ps_codec->s_parse.i4_first_pic_init)
    {
        ret = ihevcd_mv_buf_mgr_add_bufs(ps_codec);
        RETURN_IF((ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS), ret);

        ret = ihevcd_pic_buf_mgr_add_bufs(ps_codec);
        RETURN_IF((ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS), ret);

        ps_codec->s_parse.i4_first_pic_init = 1;
    }

    /* Output buffer check */
    ret = ihevcd_check_out_buf_size(ps_codec);
    RETURN_IF((ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS), ret);

    /* Initialize all the slice headers' slice addresses to zero */
    {
        WORD32 slice_idx;
        WORD32 slice_start_idx;

        slice_start_idx = ps_codec->i4_slice_error ? 2 : 1;

        for(slice_idx = slice_start_idx; slice_idx < MAX_SLICE_HDR_CNT; slice_idx++)
        {
            slice_header_t *ps_slice_hdr_tmp = ps_codec->ps_slice_hdr_base + slice_idx;
            ps_slice_hdr_tmp->i2_ctb_x = -1;
            ps_slice_hdr_tmp->i2_ctb_y = -1;

        }
    }

    /* Get free MV Bank to hold current picture's motion vector data */
    {
        ps_mv_buf = (mv_buf_t *)ihevc_buf_mgr_get_next_free((buf_mgr_t *)ps_codec->pv_mv_buf_mgr, &cur_mv_bank_buf_id);

        /* If there are no free buffers then return with an error code.
         * If the buffer is to be freed by another thread , change the
         * following to call thread yield and wait for buffer to be freed
         */
        if(NULL == ps_mv_buf)
        {
            ps_codec->s_parse.i4_error_code = IHEVCD_NO_FREE_MVBANK;
            ps_codec->i4_error_code = IHEVCD_NO_FREE_MVBANK;
            return IHEVCD_NO_FREE_MVBANK;
        }

        ps_codec->s_parse.ps_cur_mv_buf = ps_mv_buf;
        /* Set current ABS poc to ps_mv_buf, so that while freeing a reference buffer
         * corresponding mv buffer can be found by looping through ps_codec->ps_mv_buf array
         * and getting a buffer id to free
         */
        ps_mv_buf->i4_abs_poc = ps_slice_hdr->i4_abs_pic_order_cnt;
    }

    /* Get free picture buffer to hold current picture recon data */
    /* TODO: For asynchronous api the following initializations related to picture
     * buffer should be moved to processing side
     */
    {

        UWORD8 *pu1_buf;
        ps_cur_pic = (pic_buf_t *)ihevc_buf_mgr_get_next_free((buf_mgr_t *)ps_codec->pv_pic_buf_mgr, &cur_pic_buf_id);

        /* If there are no free buffers then return with an error code.
         * TODO: If the buffer is to be freed by another thread , change the
         * following to call thread yield and wait for buffer to be freed
         */
        if(NULL == ps_cur_pic)
        {
            ps_codec->s_parse.i4_error_code = IHEVCD_NO_FREE_PICBUF;
            ps_codec->i4_error_code = IHEVCD_NO_FREE_PICBUF;
            return IHEVCD_NO_FREE_PICBUF;
        }

        /* Store input timestamp sent with input buffer */
        ps_cur_pic->u4_ts = ps_codec->u4_ts;
        ps_cur_pic->i4_abs_poc = ps_slice_hdr->i4_abs_pic_order_cnt;
        ps_cur_pic->i4_poc_lsb = ps_slice_hdr->i4_pic_order_cnt_lsb;
        pu1_buf = ps_cur_pic->pu1_luma;
        pu1_cur_pic_luma = pu1_buf;

        pu1_buf = ps_cur_pic->pu1_chroma;

        pu1_cur_pic_chroma = pu1_buf;

        ps_cur_pic->s_sei_params.i1_sei_parameters_present_flag = 0;
        if(ps_codec->s_parse.s_sei_params.i1_sei_parameters_present_flag)
        {
            sei_params_t *ps_sei = &ps_codec->s_parse.s_sei_params;
            ps_cur_pic->s_sei_params = ps_codec->s_parse.s_sei_params;

            /* Once sei_params is copied to pic_buf,
             * mark sei_params in s_parse as not present,
             * this ensures that future frames do not use this data again.
             */
            ps_sei->i1_sei_parameters_present_flag = 0;
            ps_sei->i1_user_data_registered_present_flag = 0;
            ps_sei->i1_aud_present_flag = 0;
            ps_sei->i1_time_code_present_flag = 0;
            ps_sei->i1_buf_period_params_present_flag = 0;
            ps_sei->i1_pic_timing_params_present_flag = 0;
            ps_sei->i1_recovery_point_params_present_flag = 0;
            ps_sei->i1_active_parameter_set = 0;
            ps_sei->i4_sei_mastering_disp_colour_vol_params_present_flags = 0;
        }
    }

    if(0 == ps_codec->u4_pic_cnt)
    {
        memset(ps_cur_pic->pu1_luma, 128, (ps_sps->i2_pic_width_in_luma_samples + PAD_WD) * ps_sps->i2_pic_height_in_luma_samples);
        memset(ps_cur_pic->pu1_chroma, 128, (ps_sps->i2_pic_width_in_luma_samples + PAD_WD) * ps_sps->i2_pic_height_in_luma_samples / 2);
    }

    /* Fill the remaining entries of the reference lists with the nearest POC
     * This is done to handle cases where there is a corruption in the reference index */
    {
        pic_buf_t *ps_pic_buf_ref;
        mv_buf_t *ps_mv_buf_ref;
        WORD32 r_idx;
        dpb_mgr_t *ps_dpb_mgr = (dpb_mgr_t *)ps_codec->pv_dpb_mgr;
        buf_mgr_t *ps_mv_buf_mgr = (buf_mgr_t *)ps_codec->pv_mv_buf_mgr;

        ps_pic_buf_ref = ihevc_dpb_mgr_get_ref_by_nearest_poc(ps_dpb_mgr, ps_slice_hdr->i4_abs_pic_order_cnt);
        if(NULL == ps_pic_buf_ref)
        {
            WORD32 size;

            WORD32 num_pu;
            WORD32 num_ctb;
            WORD32 pic_size;
            /* In case current mv buffer itself is being used as reference mv buffer for colocated
             * calculations, then memset all the buffers to zero.
             */
            pic_size = ALIGN64(ps_sps->i2_pic_width_in_luma_samples) *
                            ALIGN64(ps_sps->i2_pic_height_in_luma_samples);

            num_pu = pic_size / (MIN_PU_SIZE * MIN_PU_SIZE);
            num_ctb = pic_size / (MIN_CTB_SIZE * MIN_CTB_SIZE);

            memset(ps_mv_buf->ai4_l0_collocated_poc, 0, sizeof(ps_mv_buf->ai4_l0_collocated_poc));
            memset(ps_mv_buf->ai1_l0_collocated_poc_lt, 0, sizeof(ps_mv_buf->ai1_l0_collocated_poc_lt));
            memset(ps_mv_buf->ai4_l1_collocated_poc, 0, sizeof(ps_mv_buf->ai4_l1_collocated_poc));
            memset(ps_mv_buf->ai1_l1_collocated_poc_lt, 0, sizeof(ps_mv_buf->ai1_l1_collocated_poc_lt));

            size = (num_ctb + 1) * sizeof(WORD32);
            memset(ps_mv_buf->pu4_pic_pu_idx, 0, size);

            size = num_pu;
            memset(ps_mv_buf->pu1_pic_pu_map, 0, size);
            size = ALIGN4(num_ctb * sizeof(UWORD16));
            memset(ps_mv_buf->pu1_pic_slice_map, 0, size);
            size = num_pu * sizeof(pu_t);
            memset(ps_mv_buf->ps_pic_pu, 0, size);

            ps_pic_buf_ref = ps_cur_pic;
            ps_mv_buf_ref = ps_mv_buf;
        }
        else
        {
            ps_mv_buf_ref = ihevcd_mv_mgr_get_poc(ps_mv_buf_mgr, ps_pic_buf_ref->i4_abs_poc);
        }

        for(r_idx = 0; r_idx < ps_slice_hdr->i1_num_ref_idx_l0_active; r_idx++)
        {
            if(NULL == ps_slice_hdr->as_ref_pic_list0[r_idx].pv_pic_buf)
            {
                ps_slice_hdr->as_ref_pic_list0[r_idx].pv_pic_buf = (void *)ps_pic_buf_ref;
                ps_slice_hdr->as_ref_pic_list0[r_idx].pv_mv_buf = (void *)ps_mv_buf_ref;
            }
        }

        for(r_idx = ps_slice_hdr->i1_num_ref_idx_l0_active; r_idx < MAX_DPB_SIZE; r_idx++)
        {
            ps_slice_hdr->as_ref_pic_list0[r_idx].pv_pic_buf = (void *)ps_pic_buf_ref;
            ps_slice_hdr->as_ref_pic_list0[r_idx].pv_mv_buf = (void *)ps_mv_buf_ref;
        }

        for(r_idx = 0; r_idx < ps_slice_hdr->i1_num_ref_idx_l1_active; r_idx++)
        {
            if(NULL == ps_slice_hdr->as_ref_pic_list1[r_idx].pv_pic_buf)
            {
                ps_slice_hdr->as_ref_pic_list1[r_idx].pv_pic_buf = (void *)ps_pic_buf_ref;
                ps_slice_hdr->as_ref_pic_list1[r_idx].pv_mv_buf = (void *)ps_mv_buf_ref;
            }
        }

        for(r_idx = ps_slice_hdr->i1_num_ref_idx_l1_active; r_idx < MAX_DPB_SIZE; r_idx++)
        {
            ps_slice_hdr->as_ref_pic_list1[r_idx].pv_pic_buf = (void *)ps_pic_buf_ref;
            ps_slice_hdr->as_ref_pic_list1[r_idx].pv_mv_buf = (void *)ps_mv_buf_ref;
        }
    }


    /* Reset the jobq to start of the jobq buffer */
    ihevcd_jobq_reset((jobq_t *)ps_codec->pv_proc_jobq);

    ps_codec->s_parse.i4_pic_pu_idx = 0;
    ps_codec->s_parse.i4_pic_tu_idx = 0;

    ps_codec->s_parse.pu1_pic_pu_map = ps_mv_buf->pu1_pic_pu_map;
    ps_codec->s_parse.ps_pic_pu      = ps_mv_buf->ps_pic_pu;
    ps_codec->s_parse.pu4_pic_pu_idx = ps_mv_buf->pu4_pic_pu_idx;
    ps_codec->s_parse.pu1_slice_idx = (UWORD16 *)ps_mv_buf->pu1_pic_slice_map;
    for(i = 0; i < MAX_PROCESS_THREADS; i++)
    {
        ps_codec->as_process[i].pu1_slice_idx = (UWORD16 *)ps_mv_buf->pu1_pic_slice_map;
    }
    ps_codec->s_parse.pu1_pu_map = ps_codec->s_parse.pu1_pic_pu_map;
    ps_codec->s_parse.ps_pu = ps_codec->s_parse.ps_pic_pu;

    {
        UWORD8 *pu1_buf;
        WORD32 ctb_luma_min_tu_cnt, ctb_chroma_min_tu_cnt, ctb_min_tu_cnt;
        WORD32 pic_size;
        WORD32 num_ctb;

        pic_size = ALIGN64(ps_sps->i2_pic_width_in_luma_samples) *
                        ALIGN64(ps_sps->i2_pic_height_in_luma_samples);

        ctb_luma_min_tu_cnt = pic_size / (MIN_TU_SIZE * MIN_TU_SIZE);

        ctb_chroma_min_tu_cnt = ctb_luma_min_tu_cnt >> 1;

        ctb_min_tu_cnt = ctb_luma_min_tu_cnt + ctb_chroma_min_tu_cnt;

        num_ctb = pic_size / (MIN_CTB_SIZE * MIN_CTB_SIZE);
        pu1_buf  = (UWORD8 *)ps_codec->pv_tu_data;
        ps_codec->s_parse.pu4_pic_tu_idx = (UWORD32 *)pu1_buf;
        pu1_buf += (num_ctb + 1) * sizeof(WORD32);

        ps_codec->s_parse.pu1_pic_tu_map = pu1_buf;
        pu1_buf += ctb_min_tu_cnt;

        ps_codec->s_parse.ps_pic_tu = (tu_t *)pu1_buf;
        pu1_buf += ctb_min_tu_cnt * sizeof(tu_t);

        ps_codec->s_parse.pv_pic_tu_coeff_data = pu1_buf;

        ps_codec->s_parse.pu1_tu_map = ps_codec->s_parse.pu1_pic_tu_map;
        ps_codec->s_parse.ps_tu = ps_codec->s_parse.ps_pic_tu;
        ps_codec->s_parse.pv_tu_coeff_data = ps_codec->s_parse.pv_pic_tu_coeff_data;
    }

    ps_codec->s_parse.s_bs_ctxt.ps_pic_pu = ps_codec->s_parse.ps_pic_pu;
    ps_codec->s_parse.s_bs_ctxt.pu4_pic_pu_idx = ps_codec->s_parse.pu4_pic_pu_idx;
    ps_codec->s_parse.s_bs_ctxt.pu4_pic_tu_idx = ps_codec->s_parse.pu4_pic_tu_idx;


    /* Set number of CTBs to be processed simultaneously */
    ps_codec->i4_proc_nctb = ihevcd_nctb_cnt(ps_codec, ps_sps);

    /* Memset Parse Map and process map at the start of frame */
    //TODO: In case of asynchronous API proc_map can not be set to zero here
    {
        WORD32 num_ctb;

        num_ctb = ps_sps->i4_pic_size_in_ctb;

        memset(ps_codec->pu1_parse_map, 0, num_ctb);

        memset(ps_codec->pu1_proc_map, 0, num_ctb);
    }



    /* Initialize disp buf id to -1, this will be updated at the end of frame if there is
     * buffer to be displayed
     */
    ps_codec->i4_disp_buf_id = -1;
    ps_codec->ps_disp_buf = NULL;

    ps_codec->i4_disable_deblk_pic  = 0;
    ps_codec->i4_disable_sao_pic    = 0;
    ps_codec->i4_fullpel_inter_pred = 0;
    ps_codec->i4_mv_frac_mask       = 0x7FFFFFFF;

    /* If degrade is enabled, set the degrade flags appropriately */
    if(ps_codec->i4_degrade_type && ps_codec->i4_degrade_pics)
    {
        WORD32 degrade_pic;
        ps_codec->i4_degrade_pic_cnt++;
        degrade_pic = 0;

        /* If degrade is to be done in all frames, then do not check further */
        switch(ps_codec->i4_degrade_pics)
        {
            case 4:
            {
                degrade_pic = 1;
                break;
            }
            case 3:
            {
                if(ps_slice_hdr->i1_slice_type != ISLICE)
                    degrade_pic = 1;

                break;
            }
            case 2:
            {

                /* If pic count hits non-degrade interval or it is an islice, then do not degrade */
                if((ps_slice_hdr->i1_slice_type != ISLICE) &&
                   (ps_codec->i4_degrade_pic_cnt != ps_codec->i4_nondegrade_interval))
                    degrade_pic = 1;

                break;
            }
            case 1:
            {
                /* Check if the current picture is non-ref */
                if((ps_slice_hdr->i1_nal_unit_type < NAL_BLA_W_LP) &&
                   (ps_slice_hdr->i1_nal_unit_type % 2 == 0))
                {
                    degrade_pic = 1;
                }
                break;
            }


        }
        if(degrade_pic)
        {
            if(ps_codec->i4_degrade_type & 0x1)
                ps_codec->i4_disable_sao_pic = 1;

            if(ps_codec->i4_degrade_type & 0x2)
                ps_codec->i4_disable_deblk_pic = 1;

            /* MC degrading is done only for non-ref pictures */
            if((ps_slice_hdr->i1_nal_unit_type < NAL_BLA_W_LP) &&
               (ps_slice_hdr->i1_nal_unit_type % 2 == 0))
            {
                if(ps_codec->i4_degrade_type & 0x4)
                    ps_codec->i4_mv_frac_mask = 0;

                if(ps_codec->i4_degrade_type & 0x8)
                    ps_codec->i4_mv_frac_mask = 0;
            }
        }
        else
            ps_codec->i4_degrade_pic_cnt = 0;
    }


    {
        WORD32 i;
        for(i = 0; i < MAX_PROCESS_THREADS; i++)
        {
            ps_codec->as_process[i].pu4_pic_pu_idx = ps_codec->s_parse.pu4_pic_pu_idx;
            ps_codec->as_process[i].ps_pic_pu = ps_codec->s_parse.ps_pic_pu;
            ps_codec->as_process[i].pu1_pic_pu_map = ps_codec->s_parse.pu1_pic_pu_map;
            ps_codec->as_process[i].pu4_pic_tu_idx = ps_codec->s_parse.pu4_pic_tu_idx;
            ps_codec->as_process[i].ps_pic_tu = ps_codec->s_parse.ps_pic_tu;
            ps_codec->as_process[i].pu1_pic_tu_map = ps_codec->s_parse.pu1_pic_tu_map;
            ps_codec->as_process[i].pv_pic_tu_coeff_data = ps_codec->s_parse.pv_pic_tu_coeff_data;
            ps_codec->as_process[i].i4_cur_mv_bank_buf_id = cur_mv_bank_buf_id;
            ps_codec->as_process[i].s_sao_ctxt.pu1_slice_idx = ps_codec->as_process[i].pu1_slice_idx;
            ps_codec->as_process[i].s_sao_ctxt.pu1_tile_idx = ps_codec->as_process[i].pu1_tile_idx;

            /* TODO: For asynchronous api the following initializations related to picture
             * buffer should be moved to processing side
             */
            ps_codec->as_process[i].pu1_cur_pic_luma = pu1_cur_pic_luma;
            ps_codec->as_process[i].pu1_cur_pic_chroma = pu1_cur_pic_chroma;
            ps_codec->as_process[i].ps_cur_pic = ps_cur_pic;
            ps_codec->as_process[i].i4_cur_pic_buf_id = cur_pic_buf_id;

            ps_codec->as_process[i].ps_out_buffer = ps_codec->ps_out_buffer;
            if(1 < ps_codec->i4_num_cores)
            {
                ps_codec->as_process[i].i4_check_parse_status = 1;
                ps_codec->as_process[i].i4_check_proc_status = 1;
            }
            else
            {
                ps_codec->as_process[i].i4_check_parse_status = 0;
                ps_codec->as_process[i].i4_check_proc_status = 0;
            }
            ps_codec->as_process[i].pu1_pic_intra_flag = ps_codec->s_parse.pu1_pic_intra_flag;
            ps_codec->as_process[i].pu1_pic_no_loop_filter_flag = ps_codec->s_parse.pu1_pic_no_loop_filter_flag;
            ps_codec->as_process[i].i4_init_done = 0;

            ps_codec->as_process[i].s_bs_ctxt.pu4_pic_tu_idx = ps_codec->as_process[i].pu4_pic_tu_idx;
            ps_codec->as_process[i].s_bs_ctxt.pu4_pic_pu_idx = ps_codec->as_process[i].pu4_pic_pu_idx;
            ps_codec->as_process[i].s_bs_ctxt.ps_pic_pu = ps_codec->as_process[i].ps_pic_pu;
            ps_codec->as_process[i].s_deblk_ctxt.pu1_pic_no_loop_filter_flag = ps_codec->s_parse.pu1_pic_no_loop_filter_flag;
            ps_codec->as_process[i].s_deblk_ctxt.pu1_cur_pic_luma = pu1_cur_pic_luma;
            ps_codec->as_process[i].s_deblk_ctxt.pu1_cur_pic_chroma = pu1_cur_pic_chroma;
            ps_codec->as_process[i].s_sao_ctxt.pu1_pic_no_loop_filter_flag = ps_codec->s_parse.pu1_pic_no_loop_filter_flag;
            ps_codec->as_process[i].s_sao_ctxt.pu1_cur_pic_luma = pu1_cur_pic_luma;
            ps_codec->as_process[i].s_sao_ctxt.pu1_cur_pic_chroma = pu1_cur_pic_chroma;
            if(i < (ps_codec->i4_num_cores - 1))
            {
                ithread_create(ps_codec->apv_process_thread_handle[i], NULL,
                               (void *)ihevcd_process_thread,
                               (void *)&ps_codec->as_process[i]);
                ps_codec->ai4_process_thread_created[i] = 1;
            }
            else
            {
                ps_codec->ai4_process_thread_created[i] = 0;
            }

        }
        ps_codec->s_parse.s_deblk_ctxt.pu1_cur_pic_luma = pu1_cur_pic_luma;
        ps_codec->s_parse.s_deblk_ctxt.pu1_cur_pic_chroma = pu1_cur_pic_chroma;

        ps_codec->s_parse.s_sao_ctxt.pu1_cur_pic_luma = pu1_cur_pic_luma;
        ps_codec->s_parse.s_sao_ctxt.pu1_cur_pic_chroma = pu1_cur_pic_chroma;
    }
    /* Since any input bitstream buffer that contains slice data will be sent to output(even in
     * case of error, this buffer is added to display queue and next buffer in the display queue
     * will be returned as the display buffer.
     * Note: If format conversion (or frame copy) is used and is scheduled
     * in a different thread then it has to check if the processing for the current row is complete before
     * it copies/converts a given row. In case of low delay or in case of B pictures, current frame being decoded has to be
     * returned, which requires a status check to ensure that the current row is reconstructed before copying.
     */
    /* Add current picture to display manager */
    {
        WORD32 abs_poc;
        slice_header_t *ps_slice_hdr;
        ps_slice_hdr = ps_codec->s_parse.ps_slice_hdr;
        abs_poc = ps_slice_hdr->i4_abs_pic_order_cnt;
        ihevc_disp_mgr_add((disp_mgr_t *)ps_codec->pv_disp_buf_mgr,
                           ps_codec->as_process[0].i4_cur_pic_buf_id,
                           abs_poc,
                           ps_codec->as_process[0].ps_cur_pic);
    }
    ps_codec->ps_disp_buf = NULL;
    /* Get picture to be displayed if number of pictures decoded is more than max allowed reorder */
    /* Since the current will be decoded, check is fore >= instead of > */
    if(((WORD32)(ps_codec->u4_pic_cnt - ps_codec->u4_disp_cnt) >= ps_sps->ai1_sps_max_num_reorder_pics[ps_sps->i1_sps_max_sub_layers - 1]) ||
       (ps_codec->e_frm_out_mode == IVD_DECODE_FRAME_OUT))

    {
        ps_codec->ps_disp_buf = (pic_buf_t *)ihevc_disp_mgr_get((disp_mgr_t *)ps_codec->pv_disp_buf_mgr, &ps_codec->i4_disp_buf_id);
        ps_codec->u4_disp_cnt++;
    }

    ps_codec->s_fmt_conv.i4_cur_row = 0;
    /* Set number of rows to be processed at a time */
    ps_codec->s_fmt_conv.i4_num_rows = 4;

    if(ps_codec->u4_enable_fmt_conv_ahead && (ps_codec->i4_num_cores > 1))
    {
        process_ctxt_t *ps_proc;

        /* i4_num_cores - 1 contexts are currently being used by other threads */
        ps_proc = &ps_codec->as_process[ps_codec->i4_num_cores - 1];

        /* If the frame being decoded and displayed are different, schedule format conversion jobs
         * this will keep the proc threads busy and lets parse thread decode few CTBs ahead
         * If the frame being decoded and displayed are same, then format conversion is scheduled later.
         */
        if((ps_codec->ps_disp_buf) && (ps_codec->i4_disp_buf_id != ps_proc->i4_cur_pic_buf_id) &&
           ((0 == ps_codec->i4_share_disp_buf) || (IV_YUV_420P == ps_codec->e_chroma_fmt)))
        {

            for(i = 0; i < ps_sps->i2_pic_ht_in_ctb; i++)
            {
                proc_job_t s_job;
                IHEVCD_ERROR_T ret;
                s_job.i4_cmd = CMD_FMTCONV;
                s_job.i2_ctb_cnt = 0;
                s_job.i2_ctb_x = 0;
                s_job.i2_ctb_y = i;
                s_job.i2_slice_idx = 0;
                s_job.i4_tu_coeff_data_ofst = 0;
                ret = ihevcd_jobq_queue((jobq_t *)ps_codec->s_parse.pv_proc_jobq,
                                        &s_job, sizeof(proc_job_t), 1);
                if(ret != (IHEVCD_ERROR_T)IHEVCD_SUCCESS)
                    return ret;
            }
        }
    }

    /* If parse_pic_init is called, then slice data is present in the input bitstrea stream */
    ps_codec->i4_pic_present = 1;

    return ret;
}


