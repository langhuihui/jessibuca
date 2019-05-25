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
*  ihevcd_bitps_bitstrm.h
*
* @brief
*  Header for bitps_bitstrm access functions
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

#ifndef _IHEVCD_BITSTREAM_H_
#define _IHEVCD_BITSTREAM_H_
/**
 *  @brief  defines the maximum number of bits in a bitstream word
 */
#define WORD_SIZE         32
/**
 *  @brief  Twice the WORD_SIZE
 */
#define DBL_WORD_SIZE     (2 * (WORD_SIZE))

/**
 *  @brief  WORD_SIZE - 1
 */
#define WORD_SIZE_MINUS1  (WORD_SIZE - 1)

/**
******************************************************************************
* @brief Macro used to copy elements in bistream structure to local variables.
******************************************************************************
*/

#define GET_STREAM(m_ps_bitstrm, m_pu4_buf, m_u4_bit_ofst,  \
                  m_u4_cur_word, m_u4_nxt_word)             \
{                                                           \
    m_pu4_buf            = m_ps_bitstrm->pu4_buf;           \
    m_u4_bit_ofst        = m_ps_bitstrm->u4_bit_ofst;       \
    m_u4_cur_word        = m_ps_bitstrm->u4_cur_word;       \
    m_u4_nxt_word        = m_ps_bitstrm->u4_nxt_word;       \
}

/**
******************************************************************************
* @brief Macro used to copy local variables to elements in bistream structure.
******************************************************************************
*/
#define SET_STREAM(m_ps_bitstrm, m_pu4_buf, m_u4_bit_ofst,  \
                  m_u4_cur_word, m_u4_nxt_word)             \
{                                                           \
    m_ps_bitstrm->pu4_buf       = m_pu4_buf;                \
    m_ps_bitstrm->u4_bit_ofst   = m_u4_bit_ofst;            \
    m_ps_bitstrm->u4_cur_word   = m_u4_cur_word;            \
    m_ps_bitstrm->u4_nxt_word   = m_u4_nxt_word;            \
}



/**
******************************************************************************
* @brief  Snoop next m_cnt bits without updating offsets or buffer increments.
* Data is not consumed in this call
******************************************************************************
*/
#define BITS_NXT(m_u4_bits, m_pu4_buf, m_u4_bit_ofst,       \
                 m_u4_cur_word, m_u4_nxt_word, m_cnt)       \
{                                                           \
    m_u4_bits = (m_u4_cur_word << m_u4_bit_ofst)  >>        \
                              (WORD_SIZE - m_cnt);          \
                                                            \
    if(m_u4_bit_ofst > (WORD_SIZE - m_cnt))                 \
    {                                                       \
        m_u4_bits |= SHR(m_u4_nxt_word,                     \
                   (WORD_SIZE + WORD_SIZE - m_cnt           \
                          - m_u4_bit_ofst));                \
    }                                                       \
}


/**
******************************************************************************
*  @brief Snoop next 32 bits without updating offsets or buffer increments.
* Data is not consumed in this call
******************************************************************************
*/
#define BITS_NXT32(m_u4_bits, m_pu4_buf, m_u4_bit_ofst,             \
                 m_u4_cur_word, m_u4_nxt_word)                      \
{                                                                   \
    m_u4_bits = (m_u4_cur_word << m_u4_bit_ofst);                   \
                                                                    \
    m_u4_bits |= SHR(m_u4_nxt_word, (WORD_SIZE - m_u4_bit_ofst));   \
}


/**
******************************************************************************
*  @brief  Flush m_u4_bits and updated the buffer pointer.
* Data is consumed
******************************************************************************
*/
#define BITS_FLUSH(m_pu4_buf, m_u4_bit_ofst, m_u4_cur_word, \
                    m_u4_nxt_word, m_cnt)                   \
{                                                           \
    UWORD32 temp;                                           \
                                                            \
    m_u4_bit_ofst += m_cnt;                                 \
    if( m_u4_bit_ofst >=   WORD_SIZE )                      \
    {                                                       \
        m_u4_cur_word  = m_u4_nxt_word;                     \
        /* Getting the next word */                         \
        temp = *(m_pu4_buf++);                              \
                                                            \
        m_u4_bit_ofst -= WORD_SIZE;                         \
        /* Swapping little endian to big endian conversion*/\
        m_u4_nxt_word = ITT_BIG_ENDIAN(temp);                   \
    }                                                       \
}
/**
******************************************************************************
*  @brief Get m_cnt number of bits and update bffer pointers and offset.
* Data is consumed
******************************************************************************
*/
#define BITS_GET(m_u4_bits, m_pu4_buf, m_u4_bit_ofst,           \
                          m_u4_cur_word,m_u4_nxt_word, m_cnt)   \
{                                                               \
    m_u4_bits = (m_u4_cur_word << m_u4_bit_ofst)                \
                             >> (WORD_SIZE - m_cnt);            \
    m_u4_bit_ofst += m_cnt;                                     \
    if(m_u4_bit_ofst > WORD_SIZE)                               \
    {                                                           \
        m_u4_bits |= SHR(m_u4_nxt_word,                         \
                     (DBL_WORD_SIZE - m_u4_bit_ofst));          \
    }                                                           \
                                                                \
    if( m_u4_bit_ofst >=   WORD_SIZE )                          \
    {                                                           \
        UWORD32 pu4_word_tmp;                                   \
        m_u4_cur_word  = m_u4_nxt_word;                         \
        /* Getting the next word */                             \
        pu4_word_tmp = *(m_pu4_buf++);                          \
                                                                \
        m_u4_bit_ofst -= WORD_SIZE;                             \
        /* Swapping little endian to big endian conversion*/    \
        m_u4_nxt_word  = ITT_BIG_ENDIAN(pu4_word_tmp);              \
    }                                                           \
}

/**
******************************************************************************
*  @brief Get 1 bit and update buffer pointers and offset.
* Data is consumed
******************************************************************************
*/

#define BIT_GET(m_u4_bits,m_pu4_buf,m_u4_bit_ofst,              \
                          m_u4_cur_word,m_u4_nxt_word)          \
{                                                               \
    m_u4_bits = (m_u4_cur_word << m_u4_bit_ofst)                \
                             >> (WORD_SIZE_MINUS1);             \
    m_u4_bit_ofst++;                                            \
                                                                \
    if(m_u4_bit_ofst ==  WORD_SIZE)                             \
    {                                                           \
        UWORD32 pu4_word_tmp;                                   \
        m_u4_cur_word  = m_u4_nxt_word;                         \
        /* Getting the next word */                             \
        pu4_word_tmp = *m_pu4_buf++;                            \
                                                                \
        m_u4_bit_ofst = 0;                                      \
        /* Swapping little endian to big endian conversion*/    \
        m_u4_nxt_word  = ITT_BIG_ENDIAN(pu4_word_tmp);              \
    }                                                           \
}

void ihevcd_bits_init(bitstrm_t *ps_bitstrm,
                      UWORD8 *pu1_buf,
                      UWORD32 u4_numbytes);
void ihevcd_bits_flush(bitstrm_t *ps_bitstrm, UWORD32 u4_numbits);

void ihevcd_bits_flush_to_byte_boundary(bitstrm_t *ps_bitstrm);

UWORD32 ihevcd_bits_nxt(bitstrm_t *ps_bitstrm, UWORD32 u4_numbits);

UWORD32 ihevcd_bits_nxt32(bitstrm_t *ps_bitstrm, UWORD32 u4_numbits);


UWORD32 ihevcd_bits_get(bitstrm_t *ps_bitstrm, UWORD32 u4_numbits);

UWORD32  ihevcd_bits_num_bits_remaining(bitstrm_t *ps_bitstrm);


UWORD32  ihevcd_bits_num_bits_consumed(bitstrm_t *ps_bitstrm);

UWORD32 ihevcd_uev(bitstrm_t *ps_bitstrm);

WORD32 ihevcd_sev(bitstrm_t *ps_bitstrm);

void ihevcd_bits_seek(bitstrm_t *ps_bitstrm, WORD32 numbits);

#endif /* _IHEVCD_BITSTREAM_H_ */
