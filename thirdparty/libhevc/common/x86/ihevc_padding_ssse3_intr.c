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
*  ihevc_padding_atom_intr.c
*
* @brief
*  Contains function definitions for Padding
*
* @author
*  Srinivas T
*
* @par List of Functions:
*   - ihevc_pad_left_luma_ssse3()
*   - ihevc_pad_left_chroma_ssse3()
*   - ihevc_pad_right_luma_ssse3()
*   - ihevc_pad_right_chroma_ssse3()
*
* @remarks
*  None
*
*******************************************************************************
*/

#include <string.h>
#include <assert.h>
#include "ihevc_typedefs.h"
#include "ihevc_func_selector.h"
#include "ihevc_platform_macros.h"
#include "ihevc_mem_fns.h"
#include "ihevc_debug.h"

#include <immintrin.h>


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

void ihevc_pad_left_luma_ssse3(UWORD8 *pu1_src,
                               WORD32 src_strd,
                               WORD32 ht,
                               WORD32 pad_size)
{
    WORD32 row;
    WORD32 i;
    UWORD8 *pu1_dst;
    __m128i const0_16x8b;

    const0_16x8b = _mm_setzero_si128();

    ASSERT(pad_size % 8 == 0);

    for(row = 0; row < ht; row++)
    {
        __m128i src_temp0_16x8b;

        src_temp0_16x8b =  _mm_loadu_si128((__m128i *)pu1_src);
        pu1_dst = pu1_src - pad_size;
        src_temp0_16x8b = _mm_shuffle_epi8(src_temp0_16x8b, const0_16x8b);
        for(i = 0; i < pad_size; i += 8)
        {
            _mm_storel_epi64((__m128i *)(pu1_dst + i), src_temp0_16x8b);
        }
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

void ihevc_pad_left_chroma_ssse3(UWORD8 *pu1_src,
                                 WORD32 src_strd,
                                 WORD32 ht,
                                 WORD32 pad_size)
{
    WORD32 row;
    WORD32 col;
    UWORD8 *pu1_dst;
    __m128i const0_16x8b, const1_16x8b;
    const0_16x8b = _mm_setzero_si128();
    const1_16x8b = _mm_set1_epi8(1);
    const0_16x8b = _mm_unpacklo_epi8(const0_16x8b, const1_16x8b);

    ASSERT(pad_size % 8 == 0);
    for(row = 0; row < ht; row++)
    {
        __m128i src_temp0_16x8b;

        src_temp0_16x8b =  _mm_loadu_si128((__m128i *)pu1_src);
        pu1_dst = pu1_src - pad_size;
        src_temp0_16x8b = _mm_shuffle_epi8(src_temp0_16x8b, const0_16x8b);

        for(col = 0; col < pad_size; col += 8)
        {
            _mm_storel_epi64((__m128i *)(pu1_dst + col), src_temp0_16x8b);
        }
        pu1_src += src_strd;
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

void ihevc_pad_right_luma_ssse3(UWORD8 *pu1_src,
                                WORD32 src_strd,
                                WORD32 ht,
                                WORD32 pad_size)
{
    WORD32 row;
    WORD32 col;
    UWORD8 *pu1_dst;
    __m128i const0_16x8b;

    ASSERT(pad_size % 8 == 0);

    for(row = 0; row < ht; row++)
    {
        __m128i src_temp0_16x8b;

        src_temp0_16x8b =  _mm_loadu_si128((__m128i *)(pu1_src - 1));
        const0_16x8b = _mm_setzero_si128();
        pu1_dst = pu1_src;
        src_temp0_16x8b = _mm_shuffle_epi8(src_temp0_16x8b, const0_16x8b);
        for(col = 0; col < pad_size; col += 8)
        {
            _mm_storel_epi64((__m128i *)(pu1_dst + col), src_temp0_16x8b);
        }
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

void ihevc_pad_right_chroma_ssse3(UWORD8 *pu1_src,
                                  WORD32 src_strd,
                                  WORD32 ht,
                                  WORD32 pad_size)
{
    WORD32 row;
    WORD32 col;
    UWORD8 *pu1_dst;
    __m128i const0_16x8b, const1_16x8b;
    const0_16x8b = _mm_setzero_si128();
    const1_16x8b = _mm_set1_epi8(1);
    const0_16x8b = _mm_unpacklo_epi8(const0_16x8b, const1_16x8b);

    ASSERT(pad_size % 8 == 0);

    for(row = 0; row < ht; row++)
    {
        __m128i src_temp0_16x8b;

        src_temp0_16x8b =  _mm_loadu_si128((__m128i *)(pu1_src - 2));
        pu1_dst = pu1_src;
        src_temp0_16x8b = _mm_shuffle_epi8(src_temp0_16x8b, const0_16x8b);
        for(col = 0; col < pad_size; col += 8)
        {
            _mm_storel_epi64((__m128i *)(pu1_dst + col), src_temp0_16x8b);
        }

        pu1_src += src_strd;
    }
}

