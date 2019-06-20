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
 *  ihevc_dpb_mgr.c
 *
 * @brief
 *  Function definitions used for decoded picture buffer management
 *
 * @author
 *  Srinivas T
 *
 * @par List of Functions:
 *   - ihevc_dpb_mgr_init()
 *   - ihevc_dpb_mgr_del_lt()
 *   - ihevc_dpb_mgr_insert_lt()
 *   - ihevc_dpb_mgr_del_st_or_make_lt()
 *   - ihevc_dpb_mgr_insert_st()
 *   - ihevc_dpb_mgr_reset()
 *   - ihevc_dpb_mgr_release_pics()
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>

#include "ihevc_typedefs.h"
#include "ihevc_defs.h"
#include "ihevc_macros.h"
#include "ihevc_func_selector.h"
#include "ihevc_structs.h"
#include "ihevc_buf_mgr.h"
#include "ihevc_dpb_mgr.h"

/**
 *******************************************************************************
 *
 * @brief
 *  DPB manager initializer
 *
 * @par Description:
 *  Initialises the DPB manager structure
 *
 * @param[in] ps_dpb_mgr
 *  Pointer to the DPB manager structure
 *
 * @returns
 *
 * @remarks
 *
 *
 *******************************************************************************
 */

void ihevc_dpb_mgr_init(dpb_mgr_t *ps_dpb_mgr)
{
    UWORD32 i;
    dpb_info_t *ps_dpb_info = ps_dpb_mgr->as_dpb_info;
    for(i = 0; i < MAX_DPB_BUFS; i++)
    {
        ps_dpb_info[i].ps_prev_dpb = NULL;
        ps_dpb_info[i].ps_pic_buf = NULL;

    }

    ps_dpb_mgr->u1_num_ref_bufs = 0;
    ps_dpb_mgr->ps_dpb_head = NULL;

}


/**
 *******************************************************************************
 *
 * @brief
 *  Adds a reference picture into the linked  list
 *
 * @par Description:
 *  Adds the reference buffer with the given buffer id into the DPB manager
 *
 *
 * @param[in] ps_dpb_mgr
 *  Pointer to the DPB manager structure
 *
 * @param[in] ps_picBuf
 *  Pointer to the picture buffer
 *
 * @param[in] buf_id
 *  buffer id of the picture buffer
 *
 * @returns  0 if successful, -1 otherwise
 *
 * @remarks
 *
 *
 *******************************************************************************
 */

WORD32 ihevc_dpb_mgr_insert_ref(dpb_mgr_t *ps_dpb_mgr,
                                pic_buf_t *ps_pic_buf,
                                WORD32 buf_id)
{
    int i;
    dpb_info_t *ps_dpb_info;

    ps_dpb_info = ps_dpb_mgr->as_dpb_info;

    /* Return error if buffer is already present in the DPB */
    for(i = 0; i < MAX_DPB_BUFS; i++)
    {
        if((ps_dpb_info[i].ps_pic_buf == ps_pic_buf)
                        && (ps_dpb_info[i].ps_pic_buf->u1_used_as_ref))
        {
            return (-1);
        }


    }

    /* Find an unused DPB location */
    for(i = 0; i < MAX_DPB_BUFS; i++)
    {
        if(NULL == ps_dpb_info[i].ps_pic_buf)
        {
            break;
        }
    }
    if(i == MAX_DPB_BUFS)
    {
        return (-1);
    }

    /* Create DPB info */
    ps_dpb_info[i].ps_pic_buf = ps_pic_buf;
    ps_dpb_info[i].ps_prev_dpb = ps_dpb_mgr->ps_dpb_head;
    ps_dpb_info[i].ps_pic_buf->u1_buf_id = buf_id;
    ps_dpb_info[i].ps_pic_buf->u1_used_as_ref = SHORT_TERM_REF;

    /* update the head node of linked list to point to the current picture */
    ps_dpb_mgr->ps_dpb_head = ps_dpb_info + i;

    /* Increment Short term buffer count */
    ps_dpb_mgr->u1_num_ref_bufs++;

    return 0;
}

/**
 *******************************************************************************
 *
 * @brief
 *  Deletes a reference buffer from the dpb manager
 *
 * @par Description:
 *  Delete short term reference with a given POC from the linked
 *  list
 *
 * @param[in] ps_dpb_mgr
 *  Pointer to DPB Manager structure
 *
 * @param[in] ps_buf_mgr
 *  Pointer to buffer manager structure
 *
 * @param[in] u4_abs_poc
 *  Node's absolute poc
 *
 *
 * @returns  0 if successful, -1 otherwise
 *
 * @remarks
 *
 *
 *******************************************************************************
 */

void ihevc_dpb_mgr_del_ref(dpb_mgr_t *ps_dpb_mgr,
                           buf_mgr_t *ps_buf_mgr,
                           WORD32 i4_abs_poc)
{
    int i;
    dpb_info_t *ps_next_dpb;

    dpb_info_t *ps_unmark_node;
    UWORD8 u1_del_node;
    UNUSED(u1_del_node);
    u1_del_node = 0;

    /* Find the node with matching absolute POC */
    ps_next_dpb = ps_dpb_mgr->ps_dpb_head;
    if(ps_next_dpb->ps_pic_buf->i4_abs_poc == i4_abs_poc)
    {
        ps_unmark_node = ps_next_dpb;
    }
    else
    {
        for(i = 1; i < ps_dpb_mgr->u1_num_ref_bufs; i++)
        {
            if(ps_next_dpb->ps_prev_dpb->ps_pic_buf->i4_abs_poc == i4_abs_poc)
                break;
            ps_next_dpb = ps_next_dpb->ps_prev_dpb;
        }

        if(i == ps_dpb_mgr->u1_num_ref_bufs)
        {
            return;
        }
        else
            ps_unmark_node = ps_next_dpb->ps_prev_dpb;
    }

    if(ps_unmark_node == ps_dpb_mgr->ps_dpb_head)
    {
        ps_dpb_mgr->ps_dpb_head = ps_unmark_node->ps_prev_dpb;
    }
    else
    {
        ps_next_dpb->ps_prev_dpb = ps_unmark_node->ps_prev_dpb; //update link
        ps_unmark_node->ps_prev_dpb = NULL;
    }
    ps_dpb_mgr->u1_num_ref_bufs--; //decrement buffer count

    /* Release the physical buffer */
    ihevc_buf_mgr_release((buf_mgr_t *)ps_buf_mgr, ps_unmark_node->ps_pic_buf->u1_buf_id,
                          BUF_MGR_REF);
    ps_unmark_node->ps_prev_dpb = NULL;
    ps_unmark_node->ps_pic_buf = NULL;
}


/**
 *******************************************************************************
 *
 * @brief
 *  Gets a buffer with abs_poc closest to the current poc
 *
 * @par Description:
 *  Returns the pointer to the picture buffer whose poc is equal to abs_poc
 *
 * @param[in] ps_dpb_mgr
 *  Pointer to DPB Manager structure
 *
 * @param[out] ps_pic_buf
 *  Pointer to picture buffer

 * @param[in] abs_poc
 *  poc of the buffer to be returned
 *
 * @returns
 *  0 if successful, pic_buf otherwise
 * @remarks
 *
 *
 *******************************************************************************
 */
pic_buf_t* ihevc_dpb_mgr_get_ref_by_nearest_poc(dpb_mgr_t *ps_dpb_mgr, WORD32 cur_abs_poc)
{
    WORD32 i;
    WORD32 min_diff = 0x7FFFFFFF;
    pic_buf_t *ps_pic_buf = NULL;

    for(i = 0; i < MAX_DPB_BUFS; i++)
    {
        if((ps_dpb_mgr->as_dpb_info[i].ps_pic_buf) &&
                        (ps_dpb_mgr->as_dpb_info[i].ps_pic_buf->u1_used_as_ref != UNUSED_FOR_REF))
        {
            WORD32 poc_diff = cur_abs_poc - ps_dpb_mgr->as_dpb_info[i].ps_pic_buf->i4_abs_poc;
            if((poc_diff > 0) && (poc_diff < min_diff))
            {
                min_diff = poc_diff;
                ps_pic_buf = ps_dpb_mgr->as_dpb_info[i].ps_pic_buf;
            }
        }
    }

    if(NULL == ps_pic_buf)
    {
        min_diff = 0x7FFFFFFF;
        for(i = 0; i < MAX_DPB_BUFS; i++)
        {
            if((ps_dpb_mgr->as_dpb_info[i].ps_pic_buf) &&
                            (ps_dpb_mgr->as_dpb_info[i].ps_pic_buf->u1_used_as_ref != UNUSED_FOR_REF))
            {
                WORD32 poc_diff = cur_abs_poc - ps_dpb_mgr->as_dpb_info[i].ps_pic_buf->i4_abs_poc;
                if(ABS(poc_diff) < min_diff)
                {
                    min_diff = ABS(poc_diff);
                    ps_pic_buf = ps_dpb_mgr->as_dpb_info[i].ps_pic_buf;
                }
            }
        }
    }

    return ps_pic_buf;
}


/**
 *******************************************************************************
 *
 * @brief
 *  Gets a buffer with abs_poc
 *
 * @par Description:
 *  Returns the pointer to the picture buffer whose poc is equal to abs_poc
 *
 * @param[in] ps_dpb_mgr
 *  Pointer to DPB Manager structure
 *
 * @param[out] ps_pic_buf
 *  Pointer to picture buffer

 * @param[in] abs_poc
 *  poc of the buffer to be returned
 *
 * @returns
 *  0 if successful, pic_buf otherwise
 * @remarks
 *
 *
 *******************************************************************************
 */
pic_buf_t* ihevc_dpb_mgr_get_ref_by_poc(dpb_mgr_t *ps_dpb_mgr, WORD32 abs_poc)
{
    UWORD32 i;
    dpb_info_t *ps_next_ref;
    pic_buf_t *ps_pic_buf = NULL;


    ps_next_ref = ps_dpb_mgr->ps_dpb_head;
    for(i = 0; i < ps_dpb_mgr->u1_num_ref_bufs; i++)
    {
        if(ps_next_ref->ps_pic_buf->i4_abs_poc == abs_poc)
        {
            ps_pic_buf = ps_next_ref->ps_pic_buf;
            break;
        }

        ps_next_ref = ps_next_ref->ps_prev_dpb;
    }

    if(i == ps_dpb_mgr->u1_num_ref_bufs)
    {
        ps_pic_buf = NULL;
    }

    return ps_pic_buf;
}

/**
 *******************************************************************************
 *
 * @brief
 *  Gets a buffer with poc_lsb
 *
 * @par Description:
 *  Returns the pointer to the picture buffer whose poc is equal to poc_lsb
 *
 * @param[in] ps_dpb_mgr
 *  Pointer to DPB Manager structure
 *
 * @param[out] ps_pic_buf
 *  Pointer to picture buffer

 * @param[in] poc_lsb
 *  poc_lsb of the buffer to be returned
 *
 * @returns
 *  0 if successful, pic_buf otherwise
 * @remarks
 *
 *
 *******************************************************************************
 */

pic_buf_t* ihevc_dpb_mgr_get_ref_by_poc_lsb(dpb_mgr_t *ps_dpb_mgr, WORD32 poc_lsb)
{
    pic_buf_t *ps_pic_buf = NULL;
    UWORD32 i;
    dpb_info_t *ps_next_ref;

    ps_next_ref = ps_dpb_mgr->ps_dpb_head;
    for(i = 0; i < ps_dpb_mgr->u1_num_ref_bufs; i++)
    {
        if(ps_next_ref->ps_pic_buf->i4_poc_lsb == poc_lsb)
        {
            ps_pic_buf = ps_next_ref->ps_pic_buf;
            break;
        }

        ps_next_ref = ps_next_ref->ps_prev_dpb;
    }

    if(i == ps_dpb_mgr->u1_num_ref_bufs)
    {
        ps_pic_buf = NULL;
    }

    return ps_pic_buf;
}


/**
 *******************************************************************************
 *
 * @brief
 *  Resets the DPB manager
 *
 * @par Description:
 *  Re-initialises the DPB manager structure
 *
 * @param[in] ps_dpb_mgr
 *  Pointer to DPB Manager structure
 *
 * @param[in] ps_buf_mgr
 *  Pointer to buffer manager structure
 *
 * @returns
 *
 * @remarks
 *
 *
 *******************************************************************************
 */

void ihevc_dpb_mgr_reset(dpb_mgr_t *ps_dpb_mgr, buf_mgr_t *ps_buf_mgr)
{
    int i;
    dpb_info_t *ps_dpb_info;

    ps_dpb_info = ps_dpb_mgr->as_dpb_info;

    for(i = 0; i < MAX_DPB_BUFS; i++)
    {
        if(ps_dpb_info[i].ps_pic_buf->u1_used_as_ref)
        {
            ps_dpb_info[i].ps_pic_buf->u1_used_as_ref = UNUSED_FOR_REF;
            ps_dpb_info[i].ps_prev_dpb = NULL;
            //Release physical buffer
            ihevc_buf_mgr_release(ps_buf_mgr, ps_dpb_info[i].ps_pic_buf->u1_buf_id,
                                  BUF_MGR_REF);

            ps_dpb_info[i].ps_pic_buf = NULL;
        }
    }
    ps_dpb_mgr->u1_num_ref_bufs = 0;
    ps_dpb_mgr->ps_dpb_head = NULL;

}

/**
 *******************************************************************************
 *
 * @brief
 *  deletes all pictures from DPB
 *
 * @par Description:
 *  Deletes all pictures present in the DPB manager
 *
 * @param[in] ps_buf_mgr
 *  Pointer to buffer manager structure
 *
 * @param[in] u1_disp_bufs
 *  Number of buffers to be deleted
 *
 * @returns
 *
 * @remarks
 *
 *
 *******************************************************************************
 */

void ihevc_dpb_mgr_release_pics(buf_mgr_t *ps_buf_mgr, UWORD8 u1_disp_bufs)
{
    WORD8 i;
    UWORD32 buf_status;

    for(i = 0; i < u1_disp_bufs; i++)
    {
        buf_status = ihevc_buf_mgr_get_status(ps_buf_mgr, i);
        if(0 != buf_status)
        {
            ihevc_buf_mgr_release((buf_mgr_t *)ps_buf_mgr, i, BUF_MGR_REF);
        }
    }
}
