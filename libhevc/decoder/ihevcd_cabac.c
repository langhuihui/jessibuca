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
 ******************************************************************************
 * @file ihevcd_cabac.c
 *
 * @brief
 *    This file contains function definitions related to CABAC parsing
 *
 * @author
 *    Ittiam
 *
 *
 * List of Functions
 *
 *   ihevcd_cabac_init()
 *   ihevcd_cabac_decode_bin()
 *   ihevcd_cabac_decode_bypass_bin()
 *   ihevcd_cabac_decode_bypass_bins_tunary()
 *   ihevcd_cabac_decode_terminate()
 *   ihevcd_cabac_decode_bin_tunary()
 *   ihevcd_cabac_decode_bypass_bins()
 *   ihevcd_cabac_decode_bypass_bins_egk()
 *   ihevcd_cabac_decode_trunc_rice()
 *   ihevcd_cabac_flush()
 *
 ******************************************************************************
 */

/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ihevc_typedefs.h"
#include "iv.h"
#include "ivd.h"
#include "ihevcd_cxa.h"


#include "ihevc_debug.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_cabac_tables.h"
#include "ihevc_defs.h"
#include "ihevc_structs.h"
#include "ihevc_cabac_tables.h"


#include "ihevcd_defs.h"
#include "ihevcd_function_selector.h"
#include "ihevcd_structs.h"
#include "ihevcd_error.h"
#include "ihevcd_bitstream.h"
#include "ihevcd_cabac.h"
#include "ihevcd_trace.h"

#ifdef TRACE
extern trace_t g_trace;
#endif
#if DEBUG_CABAC_RANGE_OFST
#if FULLRANGE
#define DEBUG_RANGE_OFST(str, m_range, m_ofst )  \
{\
    UWORD32 m_clz, m_range_shift, m_ofst_shift;                           \
    m_clz = CLZ(m_range);                                                \
    m_clz -= (32 - RANGE_NUMBITS);                                      \
    m_range_shift = m_range << m_clz;                                    \
    m_range_shift = m_range_shift >> RANGE_SHIFT;                                 \
    m_ofst_shift = m_ofst << m_clz;                                    \
    m_ofst_shift = m_ofst_shift >> RANGE_SHIFT;                                 \
    fprintf( g_trace.fp, "%-40s R: %3d O: %3d\n", str, m_range_shift, m_ofst_shift); \
}

#else
#define DEBUG_RANGE_OFST(str,  m_range, m_ofst) \
    fprintf( g_trace.fp, "%-40s R: %3d O: %3d\n", str, m_range, m_ofst);
#endif
#else
#define DEBUG_RANGE_OFST(str, m_range, m_ofst )
#endif
/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/

/**
 ******************************************************************************
 *
 *  @brief Initializes the decoder cabac engine
 *
 *  @par   Description
 *  This routine needs to be called at start of slice/frame decode
 *
 *  @param[in,out]   ps_cabac_ctxt
 *  pointer to cabac context (handle)
 *
 *  @param[in]   ps_bitstrm
 *  pointer to bitstream context (handle)
 *
 *  @param[in]   qp
 *  current slice Qp
 *
 *  @param[in]   cabac_init_idc
 *  current slice init idc (range  [0 - 2])
 *
 *  @param[in]   pu1_init_ctxt
 *  Init cabac context to be used (range  [0 - 2])
 *
 *  @return      success or failure error code
 *
 ******************************************************************************
 */
IHEVCD_ERROR_T ihevcd_cabac_init(cab_ctxt_t *ps_cabac,
                                 bitstrm_t *ps_bitstrm,
                                 WORD32 qp,
                                 WORD32 cabac_init_idc,
                                 const UWORD8 *pu1_init_ctxt)
{
    /* Sanity checks */
    ASSERT(ps_cabac != NULL);
    ASSERT(ps_bitstrm != NULL);
    ASSERT((qp >= 0) && (qp < 52));
    ASSERT((cabac_init_idc >= 0) && (cabac_init_idc < 3));
    UNUSED(qp);
    UNUSED(cabac_init_idc);
    /* CABAC engine uses 32 bit range instead of 9 bits as specified by
     * the spec. This is done to reduce number of renormalizations
     */
    /* cabac engine initialization */
#if FULLRANGE
    ps_cabac->u4_range = (UWORD32)510 << RANGE_SHIFT;
    BITS_GET(ps_cabac->u4_ofst, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,
                    ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word, (9 + RANGE_SHIFT));

#else
    ps_cabac->u4_range = (UWORD32)510;
    BITS_GET(ps_cabac->u4_ofst, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,
                    ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word, 9);

#endif

    /* cabac context initialization based on init idc and slice qp */
    memcpy(ps_cabac->au1_ctxt_models,
           pu1_init_ctxt,
           IHEVC_CAB_CTXT_END);
    DEBUG_RANGE_OFST("init", ps_cabac->u4_range, ps_cabac->u4_ofst);

    /*
     * If the offset is greater than or equal to range, return fail.
     */
    if(ps_cabac->u4_ofst >= ps_cabac->u4_range)
    {
        return ((IHEVCD_ERROR_T)IHEVCD_FAIL);
    }

    return ((IHEVCD_ERROR_T)IHEVCD_SUCCESS);
}

IHEVCD_ERROR_T ihevcd_cabac_reset(cab_ctxt_t *ps_cabac,
                                  bitstrm_t *ps_bitstrm)
{
    /* Sanity checks */
    ASSERT(ps_cabac != NULL);
    ASSERT(ps_bitstrm != NULL);

    /* CABAC engine uses 32 bit range instead of 9 bits as specified by
     * the spec. This is done to reduce number of renormalizations
     */
    /* cabac engine initialization */
#if FULLRANGE
    ps_cabac->u4_range = (UWORD32)510 << RANGE_SHIFT;
    BITS_GET(ps_cabac->u4_ofst, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,
                    ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word, (9 + RANGE_SHIFT));

#else
    ps_cabac->u4_range = (UWORD32)510;
    BITS_GET(ps_cabac->u4_ofst, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,
                    ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word, 9);

#endif

    return ((IHEVCD_ERROR_T)IHEVCD_SUCCESS);
}

/**
 ******************************************************************************
 *
 *  @brief Decodes a bin based on probablilty and mps packed context model
 *
 *  @par   Description
 *  Decodes a bin as per Section : 9.3.3.2.1 and calls renormalization if required
 *  as per section 9.3.3.2.2
 *  1. Apart from decoding bin, context model is updated as per state transition
 *  2. Range and Low renormalization is done based on bin and original state
 *  3. After renorm bistream is updated (if required)
 *
 *  @param[in,out]   ps_cabac
 *  pointer to cabac context (handle)
 *
 *  @param[in]   ctxt_index
 *  index of cabac context model containing pState[bits6-1] | MPS[bit0]
 *
 *  @param[in]   ps_bitstrm
 *  Bitstream context
 *
 *  @return      bin(boolean) to be decoded
 *
 ******************************************************************************
 */
UWORD32 ihevcd_cabac_decode_bin(cab_ctxt_t *ps_cabac,
                                bitstrm_t *ps_bitstrm,
                                WORD32 ctxt_index

                               )
{
    UWORD32 u4_range = ps_cabac->u4_range;
    UWORD32 u4_ofst = ps_cabac->u4_ofst;
    UWORD32 u4_rlps;
    UWORD32 u4_bin;
    UWORD8 *pu1_ctxt_model = &ps_cabac->au1_ctxt_models[ctxt_index];
    WORD32 state_mps = *pu1_ctxt_model;
#if FULLRANGE
    WORD32 clz;
#endif
    UWORD32 u4_qnt_range;

    /* Sanity checks */
    ASSERT(u4_range >= 256);
    ASSERT((ctxt_index >= 0) && (ctxt_index < IHEVC_CAB_CTXT_END));
    ASSERT(state_mps < 128);
#if FULLRANGE
    clz = CLZ(u4_range);
    clz -= (32 - RANGE_NUMBITS);
    u4_qnt_range = u4_range << clz;
    u4_qnt_range = (u4_qnt_range >> (RANGE_SHIFT + 6)) & 0x3;
#else
    u4_qnt_range = (u4_range >> 6) & 0x3;
#endif
    /* Get the lps range from LUT based on quantized range and state */
    u4_rlps = gau1_ihevc_cabac_rlps[state_mps >> 1][u4_qnt_range];
#if FULLRANGE
    u4_rlps = u4_rlps << (RANGE_SHIFT - clz);
#endif
    u4_range -= u4_rlps;

    u4_bin = state_mps & 1;

    if(u4_ofst >= u4_range)
    {
        u4_bin = 1 - u4_bin;
        u4_ofst -= u4_range;
        u4_range = u4_rlps;
    }

    *pu1_ctxt_model = gau1_ihevc_next_state[(state_mps << 1) | u4_bin];

    /*****************************************************************/
    /* Re-normalization; calculate bits generated based on range(R)  */
    /*****************************************************************/
    if(u4_range < (1 << 8))
    {
        UWORD32 u4_bits;
        WORD32 numbits;
        numbits = CLZ(u4_range);
        numbits -= (32 - RANGE_NUMBITS);
#if !FULLRANGE
        numbits -= RANGE_SHIFT;
#endif
        BITS_GET(u4_bits, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,
                 ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word, numbits);

        u4_ofst <<= numbits;
        u4_ofst |= u4_bits;
        u4_range <<= numbits;

    }
    /* Update the cabac context */
    ps_cabac->u4_range = u4_range;
    ps_cabac->u4_ofst = u4_ofst;
    DEBUG_RANGE_OFST("bin", ps_cabac->u4_range, ps_cabac->u4_ofst);

    return (u4_bin);


}

/**
 ******************************************************************************
 *
 *  @brief Decodes a bypass bin (equi-probable 0 / 1)
 *
 *  @par   Description
 *  Decodes a bypss bin as per Section : 9.3.3.2.3
 *
 *  @param[in,out]  ps_cabac
 *  pointer to cabac context (handle)
 *
 *  @param[in]   ps_bitstrm
 *  Bitstream context
 *
 *  @return      Decoded bypass bin
 *
 ******************************************************************************
 */
UWORD32 ihevcd_cabac_decode_bypass_bin(cab_ctxt_t *ps_cabac,
                                       bitstrm_t *ps_bitstrm)
{

    UWORD32 u4_bin;
    UWORD32 u4_range = ps_cabac->u4_range;
    UWORD32 u4_ofst = ps_cabac->u4_ofst;
    UWORD32 u4_bits;

    /* Sanity checks */
    ASSERT(u4_range >= 256);

    BIT_GET(u4_bits, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,
            ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word);

    u4_ofst <<= 1;
    u4_ofst |= u4_bits;

    u4_bin = 0;
    if(u4_ofst >= u4_range)
    {
        u4_bin = 1;
        u4_ofst -= u4_range;
    }

    /* Update the cabac context */
    ps_cabac->u4_ofst = u4_ofst;
    DEBUG_RANGE_OFST("bypass end", ps_cabac->u4_range, ps_cabac->u4_ofst);
    return (u4_bin);
}

/**
 ******************************************************************************
 *
 *  @brief Decodes a terminate bin (1:terminate 0:do not terminate)
 *
 *  @par   Description
 *  Decodes a terminate bin to be called for end_of_slice_flag and pcm_flag
 *  as per Section : 9.3.3.2.4
 *
 *  @param[in,out]  ps_cabac
 *  pointer to cabac context (handle)
 *
 *  @param[in]   ps_bitstrm
 *  Bitstream context
 *
 *  @return    Decoded Bin to indicate whether to terminate or not
 *
 ******************************************************************************
 */
UWORD32 ihevcd_cabac_decode_terminate(cab_ctxt_t *ps_cabac,
                                      bitstrm_t *ps_bitstrm)
{
    UWORD32 u4_range = ps_cabac->u4_range;
    UWORD32 u4_ofst = ps_cabac->u4_ofst;
    UWORD32 u4_bin;
#if FULLRANGE
    WORD32 clz;
#endif
    /* Sanity checks */
    ASSERT(u4_range >= 256);
#if FULLRANGE
    clz = CLZ(u4_range);
    clz -= (32 - RANGE_NUMBITS);
    u4_range -= 2 << (RANGE_SHIFT - clz);
#else
    u4_range -= 2;
#endif

    if(u4_ofst >= u4_range)
    {
        u4_bin = 1;

#if FULLRANGE
        /* In case of FULLRANGE extra bits read earlier need to pushed back to the bitstream */
        {
            WORD32 clz;
            WORD32 numbits;
            clz = CLZ(ps_cabac->u4_range);

            numbits = (32 - clz);
            numbits -= 9;

            ihevcd_bits_seek(ps_bitstrm, -numbits);
        }
#endif

    }
    else
    {
        u4_bin = 0;
    }
    if(0 == u4_bin)
    {
        UWORD32 u4_bits;
        WORD32 numbits;
        numbits = CLZ(u4_range);
        numbits -= (32 - RANGE_NUMBITS);
#if !FULLRANGE
        numbits -= RANGE_SHIFT;
#endif
        /* Renormalize if required */
        if(numbits)
        {
            BITS_GET(u4_bits, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,
                     ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word, numbits);

            u4_ofst <<= numbits;
            u4_ofst |= u4_bits;
            u4_range <<= numbits;
        }
    }
    /* bits to be inserted in the bitstream */
    ps_cabac->u4_range = u4_range;
    ps_cabac->u4_ofst = u4_ofst;
    DEBUG_RANGE_OFST("term", ps_cabac->u4_range, ps_cabac->u4_ofst);

    return (u4_bin);
}

/**
 ******************************************************************************
 *
 *  @brief Decodes a bypass bin (equi-probable 0 / 1)
 *
 *  @par   Description
 *  Decodes a bypss bin as per Section : 9.3.3.2.3
 *
 *  @param[in,out]  ps_cabac
 *  pointer to cabac context (handle)
 *
 *  @param[in]   ps_bitstrm
 *  Bitstream context
 *
 *  @param[in]   numbins
 *  Number of bins to decoded
 *
 *  @return      Decoded bypass bin
 *
 *  @remarks     Tested only for numbins less than 17
 *
 ******************************************************************************
 */

UWORD32 ihevcd_cabac_decode_bypass_bins(cab_ctxt_t *ps_cabac,
                                        bitstrm_t *ps_bitstrm,
                                        WORD32 numbins)
{
    UWORD32 u4_bins;


    UWORD32 u4_range = ps_cabac->u4_range;
    UWORD32 u4_ofst = ps_cabac->u4_ofst;
    UWORD32 u4_bits;
    ASSERT(u4_range >= 256);
    ASSERT(numbins > 0);

    /* Sanity checks */
    ASSERT(numbins < 17);

    u4_bins = 0;

    BITS_GET(u4_bits, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,
                    ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word, numbins);

    do
    {
        UWORD32 u4_bit;
        numbins--;
        u4_bit = (u4_bits >> numbins) & 1;
        u4_ofst <<= 1;
        u4_ofst |= u4_bit;

        u4_bins <<= 1;
        if(u4_ofst >= u4_range)
        {
            u4_bins += 1;
            u4_ofst -= u4_range;
        }
    }while(numbins);

    /* Update the cabac context */
    ps_cabac->u4_ofst = u4_ofst;
    DEBUG_RANGE_OFST("bypass", ps_cabac->u4_range, ps_cabac->u4_ofst);
    return (u4_bins);
}

/**
 ******************************************************************************
 *
 *  @brief Decodes a truncated unary symbol associated with context model(s)
 *
 *  @par   Description
 *  Decodes symbols coded with TUnary binarization as per sec 9.3.2.2
 *  This is used for computing symbols like qp_delta,
 *  last_sig_coeff_prefix_x, last_sig_coeff_prefix_y.
 *
 *  The context models associated with each bin is computed as :
 *   current bin context = "base context idx" + (bin_idx >> shift)
 *  where
 *   1. "base context idx" is the base index for the syntax element
 *   2. "bin_idx" is the current bin index of the unary code
 *   3. "shift" is the shift factor associated with this syntax element
 *
 *  @param[in,out] ps_cabac
 *   pointer to cabac context (handle)
 *
 *  @param[in]   ps_bitstrm
 *  Bitstream context
 *
 *  @param[in]   c_max
 *   maximum value of sym (required for tunary binarization)
 *
 *  @param[in]   ctxt_index
 *   base context model index for this syntax element
 *
 *  @param[in]   ctxt_shift
 *   shift factor for context increments associated with this syntax element
 *
 *  @param[in]   ctxt_inc_max
 *   max value of context increment beyond which all bins will use same ctxt
 *
 *  @return     syntax element decoded
 *
 ******************************************************************************
 */
UWORD32 ihevcd_cabac_decode_bins_tunary(cab_ctxt_t *ps_cabac,
                                        bitstrm_t *ps_bitstrm,
                                        WORD32 c_max,
                                        WORD32 ctxt_index,
                                        WORD32 ctxt_shift,
                                        WORD32 ctxt_inc_max)
{
    UWORD32 u4_sym;
    WORD32 bin;

    /* Sanity checks */
    ASSERT(c_max > 0);
    ASSERT((ctxt_index >= 0) && (ctxt_index < IHEVC_CAB_CTXT_END));
    ASSERT((ctxt_index + (c_max >> ctxt_shift)) < IHEVC_CAB_CTXT_END);

    u4_sym = 0;
    do
    {
        WORD32 bin_index;
        bin_index = ctxt_index + MIN((u4_sym >> ctxt_shift), (UWORD32)ctxt_inc_max);
        IHEVCD_CABAC_DECODE_BIN(bin, ps_cabac, ps_bitstrm,  bin_index);
        u4_sym++;
    }while(((WORD32)u4_sym < c_max) && bin);

    u4_sym = u4_sym - 1 + bin;

    return (u4_sym);
}

/**
 ******************************************************************************
 *
 *  @brief Decodes a syntax element as truncated unary bypass bins
 *
 *  @par   Description
 *  Decodes symbols coded with TUnary binarization as per sec 9.3.2.2
 *  These symbols are coded as bypass bins
 *   This is used for computing symbols like merge_idx,
 *  mpm_idx etc
 *
 *  @param[in,out]ps_cabac
 *   pointer to cabac context (handle)
 *
 *  @param[in]   ps_bitstrm
 *  Bitstream context
 *
 *  @param[in]   c_max
 *   maximum value of sym (required for tunary binarization)
 *
 *  @return      syntax element decoded
 *
 ******************************************************************************
 */
UWORD32 ihevcd_cabac_decode_bypass_bins_tunary(cab_ctxt_t *ps_cabac,
                                               bitstrm_t *ps_bitstrm,
                                               WORD32 c_max)
{

    UWORD32 u4_sym;
    WORD32 bin;
    UWORD32 u4_ofst = ps_cabac->u4_ofst;
    UWORD32 u4_range = ps_cabac->u4_range;
    UWORD32 u4_bits;
    /* Sanity checks */
    ASSERT(c_max > 0);
    ASSERT(u4_range >= 256);
    u4_sym = 0;
    BITS_NXT(u4_bits, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,
                    ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word, (UWORD32)c_max);
    u4_bits <<= (32 - c_max);
    do
    {
        u4_ofst <<= 1;
        u4_ofst |= (u4_bits >> 31);
        u4_bits <<= 1;

        bin = 0;
        if(u4_ofst >= u4_range)
        {
            bin = 1;
            u4_ofst -= u4_range;
        }
        u4_sym++;
    }while(((WORD32)u4_sym < c_max) && bin);
    BITS_FLUSH(ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,
                    ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word, u4_sym);

    u4_sym = u4_sym - 1 + bin;
    /* Update the cabac context */
    ps_cabac->u4_ofst = u4_ofst;

    return (u4_sym);
}

/**
 ******************************************************************************
 *
 *  @brief Decodes a syntax element as kth order Exp-Golomb code (EGK)
 *
 *  @par   Description
 *  Decodes a syntax element binarized as kth order Exp-Golomb code (EGK)
 *  Elements are coded as bypass bins
 *
 *  @param[in,out] ps_cabac
 *   pointer to cabac context (handle)
 *
 *  @param[in]   u4_sym
 *   syntax element to be coded as EGK
 *
 *  @param[in]   k
 *   order of EGk
 *
 *  @return      success or failure error code
 *
 ******************************************************************************
 */
UWORD32 ihevcd_cabac_decode_bypass_bins_egk(cab_ctxt_t *ps_cabac,
                                            bitstrm_t *ps_bitstrm,
                                            WORD32 k)
{

    UWORD32 u4_sym;
    WORD32 numones;
    WORD32 bin;

    /* Sanity checks */
    ASSERT((k >= 0));

    numones = k;
    bin = 1;
    u4_sym = 0;
    while(bin && (numones <= 16))
    {
        IHEVCD_CABAC_DECODE_BYPASS_BIN(bin, ps_cabac, ps_bitstrm);
        u4_sym += bin << numones++;
    }

    numones -= 1;

    if(numones)
    {
        UWORD32 u4_suffix;

        IHEVCD_CABAC_DECODE_BYPASS_BINS(u4_suffix, ps_cabac, ps_bitstrm, numones);
        u4_sym += u4_suffix;
    }
    return (u4_sym);
}

/**
 ******************************************************************************
 *
 *  @brief Decodes a syntax element as truncated rice code (TR)
 *
 *  @par   Description
 *  Decodes a syntax element as truncated rice code (TR)
 *  Elements are coded as bypass bins
 *  This function ise used for coeff_abs_level_remaining coding when
 *  level is less than c_rice_max
 *
 *  @param[in,out] ps_cabac
 *   pointer to cabac context (handle)
 *
 *  @param[in]   u4_sym
 *   syntax element to be coded as truncated rice code
 *
 *  @param[in]   c_rice_param
 *    shift factor for truncated unary prefix coding of (u4_sym >> c_rice_param)
 *
 *  @param[in]   c_rice_max
 *    max symbol val below which a suffix is coded as (u4_sym%(1<<c_rice_param))
 *    This is currently (4 << c_rice_param) for coeff_abs_level_remaining
 *
 *  @return      success or failure error code
 *
 ******************************************************************************
 */
UWORD32 ihevcd_cabac_decode_bypass_bins_trunc_rice(cab_ctxt_t *ps_cabac,
                                                   bitstrm_t *ps_bitstrm,
                                                   WORD32 c_rice_param,
                                                   WORD32 c_rice_max)
{
    UWORD32 u4_sym;
    WORD32 bin;
    WORD32 c_max;
    UWORD32 u4_suffix;
    /* Sanity checks */
    ASSERT((c_rice_param >= 0));


    /* Decode prefix coded as TUnary */
    c_max = c_rice_max >> c_rice_param;
    u4_sym = 0;
    do
    {
        IHEVCD_CABAC_DECODE_BYPASS_BIN(bin, ps_cabac, ps_bitstrm);
        u4_sym++;

    }while(((WORD32)u4_sym < c_max) && bin);
    u4_sym = u4_sym - 1 + bin;

    /* If suffix is present, then decode c_rice_param number of bins */
    if(c_rice_param)
    {
        IHEVCD_CABAC_DECODE_BYPASS_BINS(u4_suffix, ps_cabac, ps_bitstrm, c_rice_param);

        u4_sym = (u4_sym << c_rice_param) | u4_suffix;
    }
    return (u4_sym);
}
