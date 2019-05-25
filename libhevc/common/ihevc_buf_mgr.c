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
*  ihevc_buf_mgr.c
*
* @brief
*  Contains function definitions for buffer management
*
* @author
*  Srinivas T
*
* @par List of Functions:
*   - ihevc_buf_mgr_init()
*   - ihevc_buf_mgr_add()
*   - ihevc_buf_mgr_get_next_free()
*   - ihevc_buf_mgr_check_free()
*   - ihevc_buf_mgr_release()
*   - ihevc_buf_mgr_set_status()
*   - ihevc_buf_mgr_get_status()
*   - ihevc_buf_mgr_get_buf()
*   - ihevc_buf_mgr_get_num_active_buf()
*
* @remarks
*  None
*
*******************************************************************************
*/
#include <stdlib.h>
#include <assert.h>
#include "ihevc_typedefs.h"
#include "ihevc_macros.h"
#include "ihevc_func_selector.h"
#include "ihevc_buf_mgr.h"
#include "ihevc_debug.h"


/**
*******************************************************************************
*
* @brief
*      Buffer manager initialization function.
*
* @par Description:
*    Initializes the buffer manager structure
*
* @param[in] ps_buf_mgr
*  Pointer to the buffer manager
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_buf_mgr_init(
                buf_mgr_t *ps_buf_mgr)
{
    WORD32 id;

    ps_buf_mgr->u4_max_buf_cnt = BUF_MGR_MAX_CNT;
    ps_buf_mgr->u4_active_buf_cnt = 0;

    for(id = 0; id < BUF_MGR_MAX_CNT; id++)
    {
        ps_buf_mgr->au4_status[id] = 0;
        ps_buf_mgr->apv_ptr[id] = NULL;
    }
}


/**
*******************************************************************************
*
* @brief
*       Adds and increments the buffer and buffer count.
*
* @par Description:
*     Adds a buffer to the buffer manager if it is not already  present and
*   increments the  active buffer count
*
* @param[in] ps_buf_mgr
*  Pointer to the buffer manager
*
* @param[in] pv_ptr
*  Pointer to the buffer to be added
*
* @returns  Returns 0 on success, -1 otherwise
*
* @remarks
*  None
*
*******************************************************************************
*/
WORD32 ihevc_buf_mgr_add(
                buf_mgr_t *ps_buf_mgr,
                void *pv_ptr,
                WORD32 buf_id)
{

    /* Check if buffer ID is within allowed range */
    if(buf_id >= (WORD32)ps_buf_mgr->u4_max_buf_cnt)
    {
        return (-1);
    }

    /* Check if the current ID is being used to hold some other buffer */
    if((ps_buf_mgr->apv_ptr[buf_id] != NULL) &&
       (ps_buf_mgr->apv_ptr[buf_id] != pv_ptr))
    {
        return (-1);
    }
    ps_buf_mgr->apv_ptr[buf_id] = pv_ptr;

    return 0;
}


/**
*******************************************************************************
*
* @brief
*   Gets the next free buffer.
*
* @par Description:
*     Returns the next free buffer available and sets the  corresponding status
*   to DEC
*
* @param[in] ps_buf_mgr
*  Pointer to the buffer manager
*
* @param[in] pi4_buf_id
*  Pointer to the id of the free buffer
*
* @returns  Pointer to the free buffer
*
* @remarks
*  None
*
*******************************************************************************
*/
void* ihevc_buf_mgr_get_next_free(
                buf_mgr_t *ps_buf_mgr,
                WORD32 *pi4_buf_id)
{
    WORD32 id;
    void *pv_ret_ptr;

    pv_ret_ptr = NULL;
    for(id = 0; id < (WORD32)ps_buf_mgr->u4_max_buf_cnt; id++)
    {
        ASSERT(ps_buf_mgr->au4_status[id] != 2);

        /* Check if the buffer is non-null and status is zero */
        if((ps_buf_mgr->au4_status[id] == 0) && (ps_buf_mgr->apv_ptr[id]))
        {
            *pi4_buf_id = id;
            /* DEC is set to 1 */
            ps_buf_mgr->au4_status[id] = 1;
            pv_ret_ptr = ps_buf_mgr->apv_ptr[id];
            break;
        }
    }

    return pv_ret_ptr;
}


/**
*******************************************************************************
*
* @brief
*      Checks the buffer manager for free buffers available.
*
* @par Description:
*  Checks if there are any free buffers available
*
* @param[in] ps_buf_mgr
*  Pointer to the buffer manager
*
* @returns  Returns 0 if available, -1 otherwise
*
* @remarks
*  None
*
*******************************************************************************
*/
WORD32 ihevc_buf_mgr_check_free(
                buf_mgr_t *ps_buf_mgr)
{
    UWORD32 id;

    for(id = 0; id < ps_buf_mgr->u4_max_buf_cnt; id++)
    {
        ASSERT(ps_buf_mgr->au4_status[id] != 2);

        if((ps_buf_mgr->au4_status[id] == 0) &&
           (ps_buf_mgr->apv_ptr[id]))
        {
            return 1;
        }
    }

    return 0;

}


/**
*******************************************************************************
*
* @brief
*       Resets the status bits.
*
* @par Description:
*     resets the status bits that the mask contains (status  corresponding to
*    the id)
*
* @param[in] ps_buf_mgr
*  Pointer to the buffer manager
*
* @param[in] buf_id
*  ID of the buffer status to be released
*
* @param[in] mask
*  Contains the bits that are to be reset
*
* @returns  0 if success, -1 otherwise
*
* @remarks
*  None
*
*******************************************************************************
*/
WORD32 ihevc_buf_mgr_release(
                buf_mgr_t *ps_buf_mgr,
                WORD32 buf_id,
                UWORD32 mask)
{
    /* If the given id is pointing to an id which is not yet added */
    if(buf_id >= (WORD32)ps_buf_mgr->u4_max_buf_cnt)
    {
        return (-1);
    }

    ps_buf_mgr->au4_status[buf_id] &= ~mask;
    ASSERT(ps_buf_mgr->au4_status[buf_id] != 2);

    /* If both the REF and DISP are zero, DEC is set to zero */
    if(ps_buf_mgr->au4_status[buf_id] == 1)
    {
        ps_buf_mgr->au4_status[buf_id] = 0;
    }

    return 0;
}


/**
*******************************************************************************
*
* @brief
*      Sets the status bit.
*
* @par Description:
*     sets the status bits that the mask contains (status  corresponding to the
*    id)
*
*
* @param[in] ps_buf_mgr
*  Pointer to the buffer manager
*
* @param[in] buf_id
*  ID of the buffer whose status needs to be modified
*
*
* @param[in] mask
*  Contains the bits that are to be set
*
* @returns  0 if success, -1 otherwise
*
* @remarks
*  None
*
*******************************************************************************
*/
WORD32 ihevc_buf_mgr_set_status(
                buf_mgr_t *ps_buf_mgr,
                WORD32 buf_id,
                UWORD32 mask)
{
    if(buf_id >= (WORD32)ps_buf_mgr->u4_max_buf_cnt)
    {
        return (-1);
    }


    if((ps_buf_mgr->au4_status[buf_id] & mask) != 0)
    {
        return (-1);
    }

    ps_buf_mgr->au4_status[buf_id] |= mask;
    ASSERT(ps_buf_mgr->au4_status[buf_id] != 2);
    return 0;
}


/**
*******************************************************************************
*
* @brief
*   Returns the status of the buffer.
*
* @par Description:
*  Returns the status of the buffer corresponding to the id
*
* @param[in] ps_buf_mgr
*  Pointer to the buffer manager
*
* @param[in] buf_id
*  ID of the buffer status required
*
* @returns  Status of the buffer corresponding to the id
*
* @remarks
*  None
*
*******************************************************************************
*/
UWORD32 ihevc_buf_mgr_get_status(
                buf_mgr_t *ps_buf_mgr,
                WORD32 buf_id)
{
    return ps_buf_mgr->au4_status[buf_id];
}


/**
*******************************************************************************
*
* @brief
*      Gets the buffer from the buffer manager
*
* @par Description:
*        Returns the pointer to the buffer corresponding to the id
*
* @param[in] ps_buf_mgr
*  Pointer to the buffer manager
*
* @param[in] buf_id
*  ID of the buffer required
*
* @returns  Pointer to the buffer required
*
* @remarks
*  None
*
*******************************************************************************
*/
void* ihevc_buf_mgr_get_buf(
                buf_mgr_t *ps_buf_mgr,
                WORD32 buf_id)
{
    return ps_buf_mgr->apv_ptr[buf_id];
}


/**
*******************************************************************************
*
* @brief
*        Gets the no.of active buffer
*
* @par Description:
*      Return the number of active buffers in the buffer manager
*
* @param[in] ps_buf_mgr
*  Pointer to the buffer manager
*
* @returns  number of active buffers
*
* @remarks
*  None
*
*******************************************************************************
*/
UWORD32 ihevc_buf_mgr_get_num_active_buf(
                buf_mgr_t *ps_buf_mgr)
{
    return ps_buf_mgr->u4_max_buf_cnt;
}
