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
*  ihevc_buf_mgr.h
*
* @brief
*  Function declarations used for buffer management
*
* @author
*  Srinivas T
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _BUF_MGR_H_
#define _BUF_MGR_H_

#define BUF_MGR_MAX_CNT 64

#define BUF_MGR_DEC         1
#define BUF_MGR_REF         (1 << 1)
#define BUF_MGR_DISP        (1 << 2)

typedef struct
{
    /**
     * max_buf_cnt
     */
    UWORD32 u4_max_buf_cnt;

    /**
     * active_buf_cnt
     */
    UWORD32 u4_active_buf_cnt;
    /**
     *  au4_status[BUF_MGR_MAX_CNT]
     */
    UWORD32 au4_status[BUF_MGR_MAX_CNT];
    /* The last three bit of status are:    */
    /* Bit 0 - DEC                          */
    /* Bit 1 - REF                          */
    /* Bit 2 - DISP                         */

    void    *apv_ptr[BUF_MGR_MAX_CNT];
}buf_mgr_t;

// intializes the buffer API structure
void ihevc_buf_mgr_init(
                buf_mgr_t *ps_buf_mgr);

// Add buffer to buffer manager. 0: success, -1: fail (u4_active_buf_cnt has reached u4_max_buf_cnt)
WORD32 ihevc_buf_mgr_add(
                buf_mgr_t *ps_buf_mgr,
                void *pv_ptr,
                WORD32 buf_id);

// this function will set the buffer status to DEC
void* ihevc_buf_mgr_get_next_free(
                buf_mgr_t *ps_buf_mgr,
                WORD32 *pi4_id);

// this function will check if there are any free buffers
WORD32 ihevc_buf_mgr_check_free(
                buf_mgr_t *ps_buf_mgr);

// mask will have who released it: DISP:REF:DEC
WORD32 ihevc_buf_mgr_release(
                buf_mgr_t *ps_buf_mgr,
                WORD32 id,
                UWORD32 mask);

// sets the status to one or all of DISP:REF:DEC
WORD32 ihevc_buf_mgr_set_status(
                buf_mgr_t *ps_buf_mgr,
                WORD32 id,
                UWORD32 mask);

// Gets status of the buffer
UWORD32 ihevc_buf_mgr_get_status(
                buf_mgr_t *ps_buf_mgr,
                WORD32 id);

// pass the ID - buffer will be returned
void* ihevc_buf_mgr_get_buf(
                buf_mgr_t *ps_buf_mgr,
                WORD32 id);

// will return number of active buffers
UWORD32 ihevc_buf_mgr_get_num_active_buf(
                buf_mgr_t *ps_buf_mgr);



#endif  //_BUF_MGR_H_
