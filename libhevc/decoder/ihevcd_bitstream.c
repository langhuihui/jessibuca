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
*  ihevcd_bitstream.c
*
* @brief
*  Contains functions for bitstream access
*
* @author
*  Harish
*
* @par List of Functions:
* - ihevcd_bits_init()
* - ihevcd_bits_flush()
* - ihevcd_bits_flush_to_byte_boundary()
* - ihevcd_bits_nxt()
* - ihevcd_bits_nxt32()
* - ihevcd_bits_get()
* - ihevcd_bits_num_bits_remaining()
* - ihevcd_bits_num_bits_consumed()
* - ihevcd_sev()
* - ihevcd_uev()
*
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

#include "ihevc_defs.h"
#include "ihevc_debug.h"
#include "ihevc_structs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_cabac_tables.h"

#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_bitstream.h"

/*****************************************************************************/
/* Function Prototypes                                                       */
/*****************************************************************************/

/**
*******************************************************************************
*
* @brief
*  Function used for bitstream structure initialization
*
* @par Description:
*  Initialize bitstream structure elements
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @param[in] pu1_buf
*  Pointer to bitstream data
*
* @param[in] u4_numbytes
*  Number of bytes in bitstream
*
* @returns  none
*
* @remarks
*  Assumes pu1_buf is aligned to 4 bytes. If not aligned  then all bitstream
* accesses will be unaligned and hence  costlier. Since this is codec memory
* that holds emulation prevented data, assumption of aligned to 4 bytes is
* valid
*
*******************************************************************************
*/
void ihevcd_bits_init(bitstrm_t *ps_bitstrm,
                      UWORD8 *pu1_buf,
                      UWORD32 u4_numbytes)
{
    UWORD32 u4_cur_word;
    UWORD32 u4_nxt_word;
    UWORD32 u4_temp;
    UWORD32 *pu4_buf;

    pu4_buf     = (UWORD32 *)pu1_buf;
    u4_temp = *pu4_buf++;
    u4_cur_word = ITT_BIG_ENDIAN(u4_temp);
    u4_temp = *pu4_buf++;
    u4_nxt_word = ITT_BIG_ENDIAN(u4_temp);

    ps_bitstrm->u4_bit_ofst     = 0;
    ps_bitstrm->pu1_buf_base    = pu1_buf;
    ps_bitstrm->pu4_buf         = pu4_buf;
    ps_bitstrm->u4_cur_word     = u4_cur_word;
    ps_bitstrm->u4_nxt_word     = u4_nxt_word;

    ps_bitstrm->pu1_buf_max     = pu1_buf + u4_numbytes + 8;

    return;
}

/**
*******************************************************************************
*
* @brief
*  Flushes given number of bits. Bits consumed increases by  this number
*
* @par Description:
*  Increment bit offset by numbits. If bit offset increases  beyond 32, then
* move nxt_word to cur_word, read next  word32 to nxt_word after endian
* conversion
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @param[in] u4_numbits
*  Number of bits to be flushed
*
* @returns  None
*
* @remarks
*
*
*******************************************************************************
*/
void ihevcd_bits_flush(bitstrm_t *ps_bitstrm, UWORD32 u4_numbits)
{

    BITS_FLUSH(ps_bitstrm->pu4_buf,
               ps_bitstrm->u4_bit_ofst,
               ps_bitstrm->u4_cur_word,
               ps_bitstrm->u4_nxt_word,
               u4_numbits);

    return;
}

/**
*******************************************************************************
*
* @brief
*  Flushes to next byte boundary.Bits consumed increases by  this number
*
* @par Description:
*  Compute number of bits remaining in the current byte  then call
* ihevcd_bits_flush() bits with this number
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @returns  None
*
* @remarks
*
*
*******************************************************************************
*/
void ihevcd_bits_flush_to_byte_boundary(bitstrm_t *ps_bitstrm)
{
    UWORD32 u4_numbits;
    u4_numbits = (ps_bitstrm->u4_bit_ofst) & 7;

    u4_numbits = 8 - u4_numbits;

    BITS_FLUSH(ps_bitstrm->pu4_buf,
               ps_bitstrm->u4_bit_ofst,
               ps_bitstrm->u4_cur_word,
               ps_bitstrm->u4_nxt_word,
               u4_numbits);

    return;
}

/**
*******************************************************************************
*
* @brief
*  Seeks by given number of bits in the bitstream from current position
*
* @par Description:
*  Add given number of bits to bitstream offset and update pu4_buf, cur_word and
*  nxt_word accordingly
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @param[in] numbits
*  Number of bits to seek
*
* @returns  None
*
* @remarks
* Assumes emulation prevention has been done before and the buffer does not
* contain any emulation prevention bytes
*
*******************************************************************************
*/
void ihevcd_bits_seek(bitstrm_t *ps_bitstrm, WORD32 numbits)
{
    WORD32 val;
    ASSERT(numbits >= -32);
    ASSERT(numbits <= 32);
    /* Check if Seeking backwards*/
    if(numbits < 0)
    {
        UWORD32 abs_numbits = -numbits;
        if(ps_bitstrm->u4_bit_ofst >= abs_numbits)
        {
            /* If the current offset is greater than number of bits to seek back,
             * then subtract abs_numbits from offset and return.
             */
            ps_bitstrm->u4_bit_ofst -= abs_numbits;
            return;
        }
        else
        {
            /* If the current offset is lesser than number of bits to seek back,
             * then subtract abs_numbits from offset and add 32 and move cur_word to nxt_word
             * and load cur_word appropriately and decrement pu4_buf
             */
            ps_bitstrm->u4_bit_ofst -= abs_numbits;
            ps_bitstrm->u4_bit_ofst += 32;
            ps_bitstrm->pu4_buf--;

            val = *(ps_bitstrm->pu4_buf - 2);
            ps_bitstrm->u4_nxt_word = ps_bitstrm->u4_cur_word;
            ps_bitstrm->u4_cur_word = ITT_BIG_ENDIAN(val);
            return;
        }
    }
    else
    {
        /* Not supported/tested currently */
        ASSERT(1);
        BITS_FLUSH(ps_bitstrm->pu4_buf,
                   ps_bitstrm->u4_bit_ofst,
                   ps_bitstrm->u4_cur_word,
                   ps_bitstrm->u4_nxt_word,
                   numbits);


    }
    return;
}
/**
*******************************************************************************
*
* @brief
*  Snoops for next numbits number of bits from the bitstream this does not
* update the bitstream offset and does not  consume the bits
*
* @par Description:
*  Extract required number of bits from cur_word & nxt_word  return these
* bits
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @param[in] u4_numbits
*  Number of bits
*
* @returns  Next u4_numbits number of bits
*
* @remarks
*
*
*******************************************************************************
*/
UWORD32 ihevcd_bits_nxt(bitstrm_t *ps_bitstrm, UWORD32 u4_numbits)
{
    UWORD32 u4_bits_read;

    BITS_NXT(u4_bits_read,
             ps_bitstrm->pu4_buf,
             ps_bitstrm->u4_bit_ofst,
             ps_bitstrm->u4_cur_word,
             ps_bitstrm->u4_nxt_word,
             u4_numbits);
    return u4_bits_read;
}
/**
*******************************************************************************
*
* @brief
*  Snoops for next 32 bits from the bitstream  this does not update the
* bitstream offset and does not  consume the bits
*
* @par Description:
*  Extract required number of bits from cur_word & nxt_word  return these
* bits
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @param[in] u4_numbits
*  Number of bits
*
* @returns  Next 32 bits
*
* @remarks
*
*
*******************************************************************************
*/
UWORD32 ihevcd_bits_nxt32(bitstrm_t *ps_bitstrm, UWORD32 u4_numbits)
{
    UWORD32 u4_bits_read;
    UNUSED(u4_numbits);
    BITS_NXT32(u4_bits_read,
               ps_bitstrm->pu4_buf,
               ps_bitstrm->u4_bit_ofst,
               ps_bitstrm->u4_cur_word,
               ps_bitstrm->u4_nxt_word);
    return u4_bits_read;
}

/**
*******************************************************************************
*
* @brief
*  Reads next numbits number of bits from the bitstream  this updates the
* bitstream offset and consumes the bits
*
* @par Description:
*  Extract required number of bits from cur_word & nxt_word  return these
* bits
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @param[in] u4_numbits
*  Number of bits
*
* @returns  Bits read
*
* @remarks
*
*
*******************************************************************************
*/
UWORD32 ihevcd_bits_get(bitstrm_t *ps_bitstrm, UWORD32 u4_numbits)
{
    UWORD32 u4_bits_read;

    BITS_GET(u4_bits_read,
             ps_bitstrm->pu4_buf,
             ps_bitstrm->u4_bit_ofst,
             ps_bitstrm->u4_cur_word,
             ps_bitstrm->u4_nxt_word,
             u4_numbits);
    return u4_bits_read;

}

/**
*******************************************************************************
*
* @brief
*  Returns the number of bits remaining in the bitstream
*
* @par Description:
*  Compute number of bits remaining based on current pointer and buffer base
* and current offset. Since 8 bytes are  read at the start into cur_word and
* nxt_word and are not  consumed, 8 has to be subtracted
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @returns  Total number of bits remaining
*
* @remarks
*
*
*******************************************************************************
*/
UWORD32  ihevcd_bits_num_bits_remaining(bitstrm_t *ps_bitstrm)
{
    UWORD32 u4_bits_consumed;
    UWORD32 u4_size_in_bits;

    /* 8 bytes are read in cur_word and nxt_word at the start. Hence */
    /* subtract 8 bytes */
    u4_bits_consumed = (UWORD32)(((UWORD8 *)ps_bitstrm->pu4_buf -
                                  (UWORD8 *)ps_bitstrm->pu1_buf_base - 8) <<
                                 3) + ps_bitstrm->u4_bit_ofst;

    u4_size_in_bits = (UWORD32)(ps_bitstrm->pu1_buf_max -
                    ps_bitstrm->pu1_buf_base) - 8;
    u4_size_in_bits <<= 3;
    if(u4_size_in_bits > u4_bits_consumed)
    {
        return (u4_size_in_bits - u4_bits_consumed);
    }
    else
    {
        return 0;
    }
}

/**
*******************************************************************************
*
* @brief
*  Returns the number of bits consumed in the bitstream
*
* @par Description:
*  Compute number of bits consumed based on current pointer  and buffer base
* and current offset. Since 8 bytes are  read at the start into cur_word and
* nxt_word and are not  consumed, 8 has to be subtracted
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @returns  Total number of bits bits consumed
*
* @remarks
*
*
*******************************************************************************
*/
UWORD32  ihevcd_bits_num_bits_consumed(bitstrm_t *ps_bitstrm)
{
    UWORD32 u4_bits_consumed;
    /* 8 bytes are read in cur_word and nxt_word at the start. Hence */
    /* subtract 8 bytes */

    u4_bits_consumed = (UWORD32)(((UWORD8 *)ps_bitstrm->pu4_buf -
                                  (UWORD8 *)ps_bitstrm->pu1_buf_base - 8) <<
                                 3) + ps_bitstrm->u4_bit_ofst;
    return u4_bits_consumed;
}

/**
*******************************************************************************
*
* @brief
*  Reads unsigned integer 0-th order exp-golomb-coded syntax element from
* the bitstream  Section: 9.2
*
* @par Description:
*  Extract required number of bits from cur_word & nxt_word  return these
* bits
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @returns  UEV decoded syntax element
*
* @remarks
*
*
*******************************************************************************
*/
UWORD32 ihevcd_uev(bitstrm_t *ps_bitstrm)
{
    UWORD32 u4_bits_read;
    UWORD32 u4_clz;


    /***************************************************************/
    /* Find leading zeros in next 32 bits                          */
    /***************************************************************/
    BITS_NXT32(u4_bits_read,
               ps_bitstrm->pu4_buf,
               ps_bitstrm->u4_bit_ofst,
               ps_bitstrm->u4_cur_word,
               ps_bitstrm->u4_nxt_word);


    u4_clz = CLZ(u4_bits_read);

    BITS_FLUSH(ps_bitstrm->pu4_buf,
               ps_bitstrm->u4_bit_ofst,
               ps_bitstrm->u4_cur_word,
               ps_bitstrm->u4_nxt_word,
               (u4_clz + 1));

    u4_bits_read = 0;
    if(u4_clz)
    {
        BITS_GET(u4_bits_read,
                 ps_bitstrm->pu4_buf,
                 ps_bitstrm->u4_bit_ofst,
                 ps_bitstrm->u4_cur_word,
                 ps_bitstrm->u4_nxt_word,
                 u4_clz);
    }
    return ((1 << u4_clz) + u4_bits_read - 1);

}

/**
*******************************************************************************
*
* @brief
*  Reads signed integer 0-th order exp-golomb-coded syntax  element from the
* bitstream. Function similar to get_uev  Section: 9.2.1
*
* @par Description:
*  Extract required number of bits from cur_word & nxt_word  return these
* bits
*
* @param[in] ps_bitstrm
*  Pointer to bitstream structure
*
* @returns  UEV decoded syntax element
*
* @remarks
*
*
*******************************************************************************
*/
WORD32 ihevcd_sev(bitstrm_t *ps_bitstrm)
{
    UWORD32 u4_bits_read;
    UWORD32 u4_clz;
    UWORD32 u4_abs_val;


    /***************************************************************/
    /* Find leading zeros in next 32 bits                          */
    /***************************************************************/
    BITS_NXT32(u4_bits_read,
               ps_bitstrm->pu4_buf,
               ps_bitstrm->u4_bit_ofst,
               ps_bitstrm->u4_cur_word,
               ps_bitstrm->u4_nxt_word);


    u4_clz = CLZ(u4_bits_read);

    BITS_FLUSH(ps_bitstrm->pu4_buf,
               ps_bitstrm->u4_bit_ofst,
               ps_bitstrm->u4_cur_word,
               ps_bitstrm->u4_nxt_word,
               (u4_clz + 1));

    u4_bits_read = 0;
    if(u4_clz)
    {
        BITS_GET(u4_bits_read,
                 ps_bitstrm->pu4_buf,
                 ps_bitstrm->u4_bit_ofst,
                 ps_bitstrm->u4_cur_word,
                 ps_bitstrm->u4_nxt_word,
                 u4_clz);
    }
    u4_abs_val = ((1 << u4_clz) + u4_bits_read) >> 1;
    if(u4_bits_read & 0x1)
        return (-(WORD32)u4_abs_val);
    else
        return (u4_abs_val);
}






