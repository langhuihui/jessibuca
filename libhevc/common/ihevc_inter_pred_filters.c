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
*  ihevc_inter_pred_filters.c
*
* @brief
*  Contains function definitions for inter prediction  interpolation filters
*
*
* @author
*  Srinivas T
*
* @par List of Functions:
*  - ihevc_inter_pred_luma_copy()
*  - ihevc_inter_pred_luma_horz()
*  - ihevc_inter_pred_luma_vert()
*  - ihevc_inter_pred_luma_copy_w16out()
*  - ihevc_inter_pred_luma_horz_w16out()
*  - ihevc_inter_pred_luma_vert_w16out()
*  - ihevc_inter_pred_luma_vert_w16inp()
*  - ihevc_inter_pred_luma_vert_w16inp_w16out()
*  - ihevc_inter_pred_chroma_copy()
*  - ihevc_inter_pred_chroma_horz()
*  - ihevc_inter_pred_chroma_vert()
*  - ihevc_inter_pred_chroma_copy_w16out()
*  - ihevc_inter_pred_chroma_horz_w16out()
*  - ihevc_inter_pred_chroma_vert_w16out()
*  - ihevc_inter_pred_chroma_vert_w16inp()
*  - ihevc_inter_pred_chroma_vert_w16inp_w16out()
*
* @remarks
*  None
*
*******************************************************************************
*/

/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/
#include "ihevc_typedefs.h"
#include "ihevc_defs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_func_selector.h"

#include "ihevc_inter_pred.h"
/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/

/**
*******************************************************************************
*
* @brief
*       Interprediction luma function for copy
*
* @par Description:
*    Copies the array of width 'wd' and height 'ht' from the  location pointed
*    by 'src' to the location pointed by 'dst'
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pu1_dst
*  UWORD8 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_luma_copy(UWORD8 *pu1_src,
                                UWORD8 *pu1_dst,
                                WORD32 src_strd,
                                WORD32 dst_strd,
                                WORD8 *pi1_coeff,
                                WORD32 ht,
                                WORD32 wd)
{
    WORD32 row, col;
    UNUSED(pi1_coeff);
    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            pu1_dst[col] = pu1_src[col];
        }

        pu1_src += src_strd;
        pu1_dst += dst_strd;
    }
}

/**
*******************************************************************************
*
* @brief
*     Interprediction luma filter for horizontal input
*
* @par Description:
*    Applies a horizontal filter with coefficients pointed to  by 'pi1_coeff'
*    to the elements pointed by 'pu1_src' and  writes to the location pointed
*    by 'pu1_dst'  The output is downshifted by 6 and clipped to 8 bits
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pu1_dst
*  UWORD8 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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

void ihevc_inter_pred_luma_horz(UWORD8 *pu1_src,
                                UWORD8 *pu1_dst,
                                WORD32 src_strd,
                                WORD32 dst_strd,
                                WORD8 *pi1_coeff,
                                WORD32 ht,
                                WORD32 wd)
{
    WORD32 row, col, i;
    WORD16 i2_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            i2_tmp = 0;
            for(i = 0; i < NTAPS_LUMA; i++)
                i2_tmp += pi1_coeff[i] * pu1_src[col + (i - 3)];

            i2_tmp = (i2_tmp + OFFSET_14_MINUS_BIT_DEPTH) >> SHIFT_14_MINUS_BIT_DEPTH;
            i2_tmp = CLIP_U8(i2_tmp);

            pu1_dst[col] = (UWORD8)i2_tmp;
        }

        pu1_src += src_strd;
        pu1_dst += dst_strd;
    }

}


/**
*******************************************************************************
*
* @brief
*    Interprediction luma filter for vertical input
*
* @par Description:
*   Applies a vertcal filter with coefficients pointed to  by 'pi1_coeff' to
*   the elements pointed by 'pu1_src' and  writes to the location pointed by
*   'pu1_dst'  The output is downshifted by 6 and clipped to 8 bits
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pu1_dst
*  UWORD8 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_luma_vert(UWORD8 *pu1_src,
                                UWORD8 *pu1_dst,
                                WORD32 src_strd,
                                WORD32 dst_strd,
                                WORD8 *pi1_coeff,
                                WORD32 ht,
                                WORD32 wd)
{
    WORD32 row, col, i;
    WORD16 i2_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            i2_tmp = 0;
            for(i = 0; i < NTAPS_LUMA; i++)
                i2_tmp += pi1_coeff[i] * pu1_src[col + (i - 3) * src_strd];

            i2_tmp = (i2_tmp + OFFSET_14_MINUS_BIT_DEPTH) >> SHIFT_14_MINUS_BIT_DEPTH;
            i2_tmp = CLIP_U8(i2_tmp);

            pu1_dst[col] = (UWORD8)i2_tmp;
        }

        pu1_src += src_strd;
        pu1_dst += dst_strd;
    }

}


/**
*******************************************************************************
*
* @brief
*       Interprediction luma filter for copy 16bit output
*
* @par Description:
*    Copies the array of width 'wd' and height 'ht' from the  location pointed
*    by 'src' to the location pointed by 'dst' The output is upshifted by 6
*    bits and is used as input for vertical filtering or weighted prediction
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pi2_dst
*  WORD16 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_luma_copy_w16out(UWORD8 *pu1_src,
                                       WORD16 *pi2_dst,
                                       WORD32 src_strd,
                                       WORD32 dst_strd,
                                       WORD8 *pi1_coeff,
                                       WORD32 ht,
                                       WORD32 wd)
{
    WORD32 row, col;
    UNUSED(pi1_coeff);
    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            pi2_dst[col] = (pu1_src[col] << SHIFT_14_MINUS_BIT_DEPTH);
        }

        pu1_src += src_strd;
        pi2_dst += dst_strd;
    }

}


/**
*******************************************************************************
*
* @brief
*     Interprediction luma filter for horizontal 16bit output
*
* @par Description:
*    Applies a horizontal filter with coefficients pointed to  by 'pi1_coeff'
*    to the elements pointed by 'pu1_src' and  writes to the location pointed
*    by 'pu1_dst'  No downshifting or clipping is done and the output is  used
*    as an input for vertical filtering or weighted  prediction
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pi2_dst
*  WORD16 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_luma_horz_w16out(UWORD8 *pu1_src,
                                       WORD16 *pi2_dst,
                                       WORD32 src_strd,
                                       WORD32 dst_strd,
                                       WORD8 *pi1_coeff,
                                       WORD32 ht,
                                       WORD32 wd)
{
    WORD32 row, col, i;
    WORD16 i2_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            i2_tmp = 0;
            for(i = 0; i < NTAPS_LUMA; i++)
                i2_tmp += pi1_coeff[i] * pu1_src[col + (i - 3)];

            pi2_dst[col] = i2_tmp;
        }

        pu1_src += src_strd;
        pi2_dst += dst_strd;
    }

}


/**
*******************************************************************************
*
* @brief
*      Interprediction luma filter for vertical 16bit output
*
* @par Description:
*    Applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
*    the elements pointed by 'pu1_src' and  writes to the location pointed by
*    'pu1_dst'  No downshifting or clipping is done and the output is  used as
*    an input for weighted prediction
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pi2_dst
*  WORD16 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_luma_vert_w16out(UWORD8 *pu1_src,
                                       WORD16 *pi2_dst,
                                       WORD32 src_strd,
                                       WORD32 dst_strd,
                                       WORD8 *pi1_coeff,
                                       WORD32 ht,
                                       WORD32 wd)
{
    WORD32 row, col, i;
    WORD16 i2_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            i2_tmp = 0;
            for(i = 0; i < NTAPS_LUMA; i++)
                i2_tmp += pi1_coeff[i] * pu1_src[col + (i - 3) * src_strd];

            pi2_dst[col] = i2_tmp;
        }

        pu1_src += src_strd;
        pi2_dst += dst_strd;
    }

}

/**
*******************************************************************************
*
* @brief
*
*        Luma vertical filter for 16bit input.
*
* @par Description:
*   Applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
*   the elements pointed by 'pu1_src' and  writes to the location pointed by
*   'pu1_dst'  Input is 16 bits  The filter output is downshifted by 12 and
*   clipped to lie  between 0 and 255
*
* @param[in] pi2_src
*  WORD16 pointer to the source
*
* @param[out] pu1_dst
*  UWORD8 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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

void ihevc_inter_pred_luma_vert_w16inp(WORD16 *pi2_src,
                                       UWORD8 *pu1_dst,
                                       WORD32 src_strd,
                                       WORD32 dst_strd,
                                       WORD8 *pi1_coeff,
                                       WORD32 ht,
                                       WORD32 wd)
{
    WORD32 row, col, i;
    WORD32 i4_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            i4_tmp = 0;
            for(i = 0; i < NTAPS_LUMA; i++)
                i4_tmp += pi1_coeff[i] * pi2_src[col + (i - 3) * src_strd];

            i4_tmp = ((i4_tmp >> SHIFT_14_MINUS_BIT_DEPTH) + OFFSET_14_MINUS_BIT_DEPTH) >> SHIFT_14_MINUS_BIT_DEPTH;
            i4_tmp = CLIP_U8(i4_tmp);

            pu1_dst[col] = i4_tmp;
        }

        pi2_src += src_strd;
        pu1_dst += dst_strd;
    }

}


/**
*******************************************************************************
*
* @brief
*      Luma prediction filter for vertical 16bit input & output
*
* @par Description:
*    Applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
*    the elements pointed by 'pu1_src' and  writes to the location pointed by
*    'pu1_dst'  Input is 16 bits  The filter output is downshifted by 6 and
*    8192 is  subtracted to store it as a 16 bit number  The output is used as
*    a input to weighted prediction
*
* @param[in] pi2_src
*  WORD16 pointer to the source
*
* @param[out] pi2_dst
*  WORD16 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_luma_vert_w16inp_w16out(WORD16 *pi2_src,
                                              WORD16 *pi2_dst,
                                              WORD32 src_strd,
                                              WORD32 dst_strd,
                                              WORD8 *pi1_coeff,
                                              WORD32 ht,
                                              WORD32 wd)
{
    WORD32 row, col, i;
    WORD32 i4_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < wd; col++)
        {
            i4_tmp = 0;
            for(i = 0; i < NTAPS_LUMA; i++)
                i4_tmp += pi1_coeff[i] * pi2_src[col + (i - 3) * src_strd];

            i4_tmp = (i4_tmp >> SHIFT_14_MINUS_BIT_DEPTH) - OFFSET14;

            pi2_dst[col] = i4_tmp;
        }

        pi2_src += src_strd;
        pi2_dst += dst_strd;
    }

}



/**
*******************************************************************************
*
* @brief
*      Chroma interprediction filter for copy
*
* @par Description:
*    Copies the array of width 'wd' and height 'ht' from the  location pointed
*    by 'src' to the location pointed by 'dst'
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pu1_dst
*  UWORD8 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_chroma_copy(UWORD8 *pu1_src,
                                  UWORD8 *pu1_dst,
                                  WORD32 src_strd,
                                  WORD32 dst_strd,
                                  WORD8 *pi1_coeff,
                                  WORD32 ht,
                                  WORD32 wd)
{
    WORD32 row, col;
    UNUSED(pi1_coeff);
    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < 2 * wd; col++)
        {
            pu1_dst[col] = pu1_src[col];
        }

        pu1_src += src_strd;
        pu1_dst += dst_strd;
    }
}



/**
*******************************************************************************
*
* @brief
*     Chroma interprediction filter for horizontal input
*
* @par Description:
*    Applies a horizontal filter with coefficients pointed to  by 'pi1_coeff'
*    to the elements pointed by 'pu1_src' and  writes to the location pointed
*    by 'pu1_dst'  The output is downshifted by 6 and clipped to 8 bits
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pu1_dst
*  UWORD8 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_chroma_horz(UWORD8 *pu1_src,
                                  UWORD8 *pu1_dst,
                                  WORD32 src_strd,
                                  WORD32 dst_strd,
                                  WORD8 *pi1_coeff,
                                  WORD32 ht,
                                  WORD32 wd)
{
    WORD32 row, col, i;
    WORD16 i2_tmp_u, i2_tmp_v;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < 2 * wd; col += 2)
        {
            i2_tmp_u = 0;
            i2_tmp_v = 0;
            for(i = 0; i < NTAPS_CHROMA; i++)
            {
                i2_tmp_u += pi1_coeff[i] * pu1_src[col + (i - 1) * 2];
                i2_tmp_v += pi1_coeff[i] * pu1_src[col + 1 + (i - 1) * 2];
            }

            i2_tmp_u = (i2_tmp_u + OFFSET_14_MINUS_BIT_DEPTH) >> SHIFT_14_MINUS_BIT_DEPTH;
            i2_tmp_u = CLIP_U8(i2_tmp_u);
            i2_tmp_v = (i2_tmp_v + OFFSET_14_MINUS_BIT_DEPTH) >> SHIFT_14_MINUS_BIT_DEPTH;
            i2_tmp_v = CLIP_U8(i2_tmp_v);


            pu1_dst[col] = (UWORD8)i2_tmp_u;
            pu1_dst[col + 1] = (UWORD8)i2_tmp_v;
        }

        pu1_src += src_strd;
        pu1_dst += dst_strd;
    }
}



/**
*******************************************************************************
*
* @brief
*     Chroma interprediction filter for vertical input
*
* @par Description:
*    Applies a vertcal filter with coefficients pointed to  by 'pi1_coeff' to
*    the elements pointed by 'pu1_src' and  writes to the location pointed by
*    'pu1_dst'  The output is downshifted by 6 and clipped to 8 bits
*
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pu1_dst
*  UWORD8 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_chroma_vert(UWORD8 *pu1_src,
                                  UWORD8 *pu1_dst,
                                  WORD32 src_strd,
                                  WORD32 dst_strd,
                                  WORD8 *pi1_coeff,
                                  WORD32 ht,
                                  WORD32 wd)
{
    WORD32 row, col, i;
    WORD16 i2_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < 2 * wd; col++)
        {
            i2_tmp = 0;
            for(i = 0; i < NTAPS_CHROMA; i++)
            {
                i2_tmp += pi1_coeff[i] * pu1_src[col + (i - 1) * src_strd];
            }

            i2_tmp = (i2_tmp + OFFSET_14_MINUS_BIT_DEPTH) >> SHIFT_14_MINUS_BIT_DEPTH;
            i2_tmp = CLIP_U8(i2_tmp);

            pu1_dst[col] = (UWORD8)i2_tmp;
        }

        pu1_src += src_strd;
        pu1_dst += dst_strd;
    }
}



/**
*******************************************************************************
*
* @brief
*       chroma interprediction filter for copying 16bit output
*
* @par Description:
*    Copies the array of width 'wd' and height 'ht' from the  location pointed
*    by 'src' to the location pointed by 'dst' The output is upshifted by 6
*    bits and is used as input for vertical filtering or weighted prediction
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pi2_dst
*  WORD16 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_chroma_copy_w16out(UWORD8 *pu1_src,
                                         WORD16 *pi2_dst,
                                         WORD32 src_strd,
                                         WORD32 dst_strd,
                                         WORD8 *pi1_coeff,
                                         WORD32 ht,
                                         WORD32 wd)
{
    WORD32 row, col;
    UNUSED(pi1_coeff);
    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < 2 * wd; col++)
        {
            pi2_dst[col] = (pu1_src[col] << SHIFT_14_MINUS_BIT_DEPTH);
        }

        pu1_src += src_strd;
        pi2_dst += dst_strd;
    }
}



/**
*******************************************************************************
*
* @brief
*       chroma interprediction filter to store horizontal 16bit ouput
*
* @par Description:
*    Applies a horizontal filter with coefficients pointed to  by 'pi1_coeff'
*    to the elements pointed by 'pu1_src' and  writes to the location pointed
*    by 'pu1_dst'  No downshifting or clipping is done and the output is  used
*    as an input for vertical filtering or weighted  prediction
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pi2_dst
*  WORD16 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_chroma_horz_w16out(UWORD8 *pu1_src,
                                         WORD16 *pi2_dst,
                                         WORD32 src_strd,
                                         WORD32 dst_strd,
                                         WORD8 *pi1_coeff,
                                         WORD32 ht,
                                         WORD32 wd)
{
    WORD32 row, col, i;
    WORD16 i2_tmp_u, i2_tmp_v;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < 2 * wd; col += 2)
        {
            i2_tmp_u = 0;
            i2_tmp_v = 0;
            for(i = 0; i < NTAPS_CHROMA; i++)
            {
                i2_tmp_u += pi1_coeff[i] * pu1_src[col + (i - 1) * 2];
                i2_tmp_v += pi1_coeff[i] * pu1_src[col + 1 + (i - 1) * 2];
            }

            pi2_dst[col] = i2_tmp_u;
            pi2_dst[col + 1] = i2_tmp_v;
        }

        pu1_src += src_strd;
        pi2_dst += dst_strd;
    }
}



/**
*******************************************************************************
*
* @brief
*     Interprediction chroma filter to store vertical 16bit ouput
*
* @par Description:
*    Applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
*    the elements pointed by 'pu1_src' and  writes to the location pointed by
*    'pu1_dst'  No downshifting or clipping is done and the output is  used as
*    an input for weighted prediction
*
* @param[in] pu1_src
*  UWORD8 pointer to the source
*
* @param[out] pi2_dst
*  WORD16 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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


void ihevc_inter_pred_chroma_vert_w16out(UWORD8 *pu1_src,
                                         WORD16 *pi2_dst,
                                         WORD32 src_strd,
                                         WORD32 dst_strd,
                                         WORD8 *pi1_coeff,
                                         WORD32 ht,
                                         WORD32 wd)
{
    WORD32 row, col, i;
    WORD16 i2_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < 2 * wd; col++)
        {
            i2_tmp = 0;
            for(i = 0; i < NTAPS_CHROMA; i++)
            {
                i2_tmp += pi1_coeff[i] * pu1_src[col + (i - 1) * src_strd];
            }

            pi2_dst[col] = i2_tmp;
        }

        pu1_src += src_strd;
        pi2_dst += dst_strd;
    }
}


/**
*******************************************************************************
*
* @brief
*     chroma interprediction filter for vertical 16bit input
*
* @par Description:
*    Applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
*    the elements pointed by 'pu1_src' and  writes to the location pointed by
*    'pu1_dst'  Input is 16 bits  The filter output is downshifted by 12 and
*    clipped to lie  between 0 and 255
*
* @param[in] pi2_src
*  WORD16 pointer to the source
*
* @param[out] pu1_dst
*  UWORD8 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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

void ihevc_inter_pred_chroma_vert_w16inp(WORD16 *pi2_src,
                                         UWORD8 *pu1_dst,
                                         WORD32 src_strd,
                                         WORD32 dst_strd,
                                         WORD8 *pi1_coeff,
                                         WORD32 ht,
                                         WORD32 wd)
{
    WORD32 row, col, i;
    WORD32 i4_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < 2 * wd; col++)
        {
            i4_tmp = 0;
            for(i = 0; i < NTAPS_CHROMA; i++)
            {
                i4_tmp += pi1_coeff[i] * pi2_src[col + (i - 1) * src_strd];
            }

            i4_tmp = ((i4_tmp >> SHIFT_14_MINUS_BIT_DEPTH) + OFFSET_14_MINUS_BIT_DEPTH) >> SHIFT_14_MINUS_BIT_DEPTH;
            i4_tmp = CLIP_U8(i4_tmp);

            pu1_dst[col] = i4_tmp;
        }

        pi2_src += src_strd;
        pu1_dst += dst_strd;
    }

}


/**
*******************************************************************************
*
* @brief
*
*      Chroma interprediction filter for 16bit vertical input and output.
*
* @par Description:
*       Applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
*       the elements pointed by 'pu1_src' and  writes to the location pointed by
*       'pu1_dst'  Input is 16 bits  The filter output is downshifted by 6 and
*       8192 is  subtracted to store it as a 16 bit number  The output is used as
*       a input to weighted prediction
*
* @param[in] pi2_src
*  WORD16 pointer to the source
*
* @param[out] pi2_dst
*  WORD16 pointer to the destination
*
* @param[in] src_strd
*  integer source stride
*
* @param[in] dst_strd
*  integer destination stride
*
* @param[in] pi1_coeff
*  WORD8 pointer to the filter coefficients
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

void ihevc_inter_pred_chroma_vert_w16inp_w16out(WORD16 *pi2_src,
                                                WORD16 *pi2_dst,
                                                WORD32 src_strd,
                                                WORD32 dst_strd,
                                                WORD8 *pi1_coeff,
                                                WORD32 ht,
                                                WORD32 wd)
{
    WORD32 row, col, i;
    WORD32 i4_tmp;

    for(row = 0; row < ht; row++)
    {
        for(col = 0; col < 2 * wd; col++)
        {
            i4_tmp = 0;
            for(i = 0; i < NTAPS_CHROMA; i++)
            {
                i4_tmp += pi1_coeff[i] * pi2_src[col + (i - 1) * src_strd];
            }

            i4_tmp = (i4_tmp >> SHIFT_14_MINUS_BIT_DEPTH);

            pi2_dst[col] = i4_tmp;
        }

        pi2_src += src_strd;
        pi2_dst += dst_strd;
    }

}


