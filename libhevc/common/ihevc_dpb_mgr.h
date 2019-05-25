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
 *  ihevc_dpb_mgr.h
 *
 * @brief
 *  Function declarations used for decoded picture buffer management
 *
 * @author
 *  Srinivas T
 *
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */
#ifndef _DPB_MANAGER_H
#define _DPB_MANAGER_H

/* Temporary definitions. Have to be defined later */

#define MAX_DPB_BUFS                (MAX_DPB_SIZE * 4)

#define MARK_ST_PICNUM_AS_NONREF    1
#define MARK_LT_INDEX_AS_NONREF     2
#define MARK_ST_PICNUM_AS_LT_INDEX  3
#define RESET_REF_PICTURES          5

typedef struct dpb_info_t dpb_info_t;

enum
{
    UNUSED_FOR_REF = 0,
    LONG_TERM_REF,
    SHORT_TERM_REF,
};
struct dpb_info_t
{
    /**
     * Pointer to picture buffer structure
     */
    pic_buf_t *ps_pic_buf;

    /**
     * Link to the DPB buffer with previous pic Num
     */
    dpb_info_t *ps_prev_dpb;

};

typedef struct
{
    /**
     * Pointer to the most recent pic Num
     */
    dpb_info_t *ps_dpb_head;

    /**
     * Physical storage for dpbInfo for ref bufs
     */
    dpb_info_t as_dpb_info[MAX_DPB_BUFS];

    /**
     * Number of reference buffers
     */
    UWORD8 u1_num_ref_bufs;

}dpb_mgr_t;

void ihevc_dpb_mgr_init(dpb_mgr_t *ps_dpb_mgr);

WORD32 ihevc_dpb_mgr_insert_ref(dpb_mgr_t *ps_dpb_mgr,
                                pic_buf_t *ps_pic_buf,
                                WORD32 buf_id);

void ihevc_dpb_mgr_del_ref(dpb_mgr_t *ps_dpb_mgr,
                           buf_mgr_t *ps_buf_mgr,
                           WORD32 u4_abs_poc);

pic_buf_t* ihevc_dpb_mgr_get_ref_by_nearest_poc(dpb_mgr_t *ps_dpb_mgr, WORD32 cur_abs_poc);

pic_buf_t* ihevc_dpb_mgr_get_ref_by_poc(dpb_mgr_t *ps_dpb_mgr, WORD32 abs_poc);

pic_buf_t* ihevc_dpb_mgr_get_ref_by_poc_lsb(dpb_mgr_t *ps_dpb_mgr, WORD32 poc_lsb);

void ihevc_dpb_mgr_reset(dpb_mgr_t *ps_dpb_mgr, buf_mgr_t *ps_buf_mgr);

void ihevc_dpb_mgr_release_pics(buf_mgr_t *ps_buf_mgr, UWORD8 u1_disp_bufs);

#endif /*  _DPB_MANAGER_H */
