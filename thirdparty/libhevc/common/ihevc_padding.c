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
*  ihevc_padding.c
*
* @brief
*  Contains function definitions for Padding
*
* @author
*  Srinivas T
*
* @par List of Functions:
*   - ihevc_pad_horz_luma()
*   - ihevc_pad_horz_chroma()
*   - ihevc_pad_vert()
*   - ihevc_pad_left_luma()
*   - ihevc_pad_left_chroma()
*   - ihevc_pad_right_luma()
*   - ihevc_pad_right_chroma()
*   - ihevc_pad_top()
*   - ihevc_pad_bottom()
*
* @remarks
*  None
*
*******************************************************************************
*/

#include <string.h>
#include "ihevc_typedefs.h"
#include "ihevc_func_selector.h"
#include "ihevc_platform_macros.h"
#include "ihevc_mem_fns.h"
/**
*******************************************************************************
*
* @brief
*       Padding function for horizontal input variable
*
* @par Description:
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @param[in] pad_size
*  integer -padding size of the array
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_pad_vert(UWORD8 *pu1_src,
                    WORD32 src_strd,
                    WORD32 ht,
                    WORD32 wd,
                    WORD32 pad_size)
{
    WORD32 row;

    for(row = 1; row <= pad_size; row++)
    {
        memcpy(pu1_src - row * src_strd, pu1_src, wd);
        memcpy(pu1_src + (ht + row - 1) * src_strd,
               pu1_src + (ht - 1) * src_strd, wd);
    }
}

/**
*******************************************************************************
*
* @brief
*   Padding function for vertical input variable
*
* @par Description:
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @param[in] pad_size
*  integer -padding size of the array
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_pad_horz_chroma(UWORD8 *pu1_src,
                           WORD32 src_strd,
                           WORD32 ht,
                           WORD32 wd,
                           WORD32 pad_size)
{
    WORD32 row;
    //WORD32 col;
    UWORD16 *pu2_src = (UWORD16 *)pu1_src;

    src_strd >>= 1;
    wd >>= 1;
    pad_size >>= 1;

    for(row = 0; row < ht; row++)
    {
        UWORD16 u2_uv_val;

        u2_uv_val = pu2_src[0];
        ihevc_memset_16bit(&pu2_src[-pad_size], u2_uv_val, pad_size);

        u2_uv_val = pu2_src[wd - 1];
        ihevc_memset_16bit(&pu2_src[wd], u2_uv_val, pad_size);

        pu2_src += src_strd;
    }
}


/**
*******************************************************************************
*
* @brief
*   Padding function for vertical input variable
*
* @par Description:
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @param[in] pad_size
*  integer -padding size of the array
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_pad_horz_luma(UWORD8 *pu1_src,
                         WORD32 src_strd,
                         WORD32 ht,
                         WORD32 wd,
                         WORD32 pad_size)
{
    WORD32 row;

    for(row = 0; row < ht; row++)
    {
        memset(pu1_src - pad_size, *pu1_src, pad_size);
        memset(pu1_src + wd, *(pu1_src + wd - 1), pad_size);

        pu1_src += src_strd;
    }
}



/**
*******************************************************************************
*
* @brief
*       Padding at the top of a 2d array
*
* @par Description:
*       The top row of a 2d array is replicated for pad_size times at the top
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @param[in] pad_size
*  integer -padding size of the array
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_pad_top(UWORD8 *pu1_src,
                   WORD32 src_strd,
                   WORD32 wd,
                   WORD32 pad_size)
{
    WORD32 row;

    for(row = 1; row <= pad_size; row++)
    {
        memcpy(pu1_src - row * src_strd, pu1_src, wd);
    }
}



/**
*******************************************************************************
*
* @brief
*   Padding at the bottom of a 2d array
*
* @par Description:
*   The bottom row of a 2d array is replicated for pad_size times at the bottom
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @param[in] pad_size
*  integer -padding size of the array
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_pad_bottom(UWORD8 *pu1_src,
                      WORD32 src_strd,
                      WORD32 wd,
                      WORD32 pad_size)
{
    WORD32 row;

    for(row = 1; row <= pad_size; row++)
    {
        memcpy(pu1_src + (row - 1) * src_strd,
               pu1_src - 1 * src_strd, wd);
    }
}



/**
*******************************************************************************
*
* @brief
*   Padding (luma block) at the left of a 2d array
*
* @par Description:
*   The left column of a 2d array is replicated for pad_size times at the left
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @param[in] pad_size
*  integer -padding size of the array
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_pad_left_luma(UWORD8 *pu1_src,
                         WORD32 src_strd,
                         WORD32 ht,
                         WORD32 pad_size)
{
    WORD32 row;

    for(row = 0; row < ht; row++)
    {
        memset(pu1_src - pad_size, *pu1_src, pad_size);

        pu1_src += src_strd;
    }
}



/**
*******************************************************************************
*
* @brief
*   Padding (chroma block) at the left of a 2d array
*
* @par Description:
*   The left column of a 2d array is replicated for pad_size times at the left
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array (each colour component)
*
* @param[in] pad_size
*  integer -padding size of the array
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_pad_left_chroma(UWORD8 *pu1_src,
                           WORD32 src_strd,
                           WORD32 ht,
                           WORD32 pad_size)
{
    WORD32 row;
    WORD32 col;
    UWORD16 *pu2_src = (UWORD16 *)pu1_src;

    src_strd >>= 1;
    pad_size >>= 1;

    for(row = 0; row < ht; row++)
    {
        UWORD16 u2_uv_val;

        u2_uv_val = pu2_src[0];
        for(col = -pad_size; col < 0; col++)
            pu2_src[col] = u2_uv_val;

        pu2_src += src_strd;
    }
}



/**
*******************************************************************************
*
* @brief
* Padding (luma block) at the right of a 2d array
*
* @par Description:
* The right column of a 2d array is replicated for pad_size times at the right
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @param[in] pad_size
*  integer -padding size of the array
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_pad_right_luma(UWORD8 *pu1_src,
                          WORD32 src_strd,
                          WORD32 ht,
                          WORD32 pad_size)
{
    WORD32 row;

    for(row = 0; row < ht; row++)
    {
        memset(pu1_src, *(pu1_src - 1), pad_size);

        pu1_src += src_strd;
    }
}



/**
*******************************************************************************
*
* @brief
* Padding (chroma block) at the right of a 2d array
*
* @par Description:
* The right column of a 2d array is replicated for pad_size times at the right
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array (each colour component)
*
* @param[in] pad_size
*  integer -padding size of the array
*
* @param[in] ht
*  integer height of the array
*
* @param[in] wd
*  integer width of the array
*
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/

void ihevc_pad_right_chroma(UWORD8 *pu1_src,
                            WORD32 src_strd,
                            WORD32 ht,
                            WORD32 pad_size)
{
    WORD32 row;
    WORD32 col;
    UWORD16 *pu2_src = (UWORD16 *)pu1_src;

    src_strd >>= 1;
    pad_size >>= 1;

    for(row = 0; row < ht; row++)
    {
        UWORD16 u2_uv_val;

        u2_uv_val = pu2_src[-1];
        for(col = 0; col < pad_size; col++)
            pu2_src[col] = u2_uv_val;

        pu2_src += src_strd;
    }
}

