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
* @file ihevcd_cabac.h
*
* @brief
*  This file contains decoder cabac engine related structures and
*  interface prototypes
*
* @author
*  Ittiam
******************************************************************************
*/

#ifndef _IHEVCD_CABAC_H_
#define _IHEVCD_CABAC_H_

#include "ihevc_typedefs.h"
/*****************************************************************************/
/* Constant Macros                                                           */
/*****************************************************************************/
/**
*******************************************************************************
@brief
*******************************************************************************
 */
#define CABAC_BITS  9

/**
 * Following definitions control whether cabac functions are inlined as macros or
 * are called as functions. Set these to 0 to debug cabac leaf level functions
 * Note these macros assume FULLRANGE is 1.
 */
#define CABAC_DECODE_BIN            1
#define CABAC_DECODE_BYPASS_BIN     1
#define CABAC_DECODE_BYPASS_BINS    1

/*****************************************************************************/
/* Function Macros                                                           */
/*****************************************************************************/
#if CABAC_DECODE_BIN
#define IHEVCD_CABAC_DECODE_BIN(u4_bin, ps_cabac, ps_bitstrm, ctxt_index)       \
{                                                                               \
    UWORD32 u4_range = ps_cabac->u4_range;                                      \
    UWORD32 u4_ofst = ps_cabac->u4_ofst;                                        \
    UWORD32 u4_rlps;                                                            \
    UWORD8 *pu1_ctxt_model = &ps_cabac->au1_ctxt_models[ctxt_index];            \
    WORD32 state_mps = *pu1_ctxt_model;                                         \
    WORD32 clz;                                                                 \
    UWORD32 u4_qnt_range;                                                       \
                                                                                \
    /* Sanity checks */                                                         \
    ASSERT(FULLRANGE == 1);                                                     \
    ASSERT(u4_range >= 256);                                                    \
    ASSERT((ctxt_index >= 0) && (ctxt_index < IHEVC_CAB_CTXT_END));             \
    ASSERT(state_mps < 128);                                                    \
    clz = CLZ(u4_range);                                                        \
    clz -= (32 - RANGE_NUMBITS);                                                \
    u4_qnt_range = u4_range << clz;                                             \
    u4_qnt_range = (u4_qnt_range >> (RANGE_SHIFT + 6)) & 0x3;                   \
    /* Get the lps range from LUT based on quantized range and state */         \
    u4_rlps = gau1_ihevc_cabac_rlps[state_mps >> 1][u4_qnt_range];              \
    u4_rlps = u4_rlps << (RANGE_SHIFT - clz);                                   \
    u4_range -= u4_rlps;                                                        \
                                                                                \
    u4_bin = state_mps & 1;                                                     \
                                                                                \
    if(u4_ofst >= u4_range)                                                     \
    {                                                                           \
        u4_bin = 1 - u4_bin;                                                    \
        u4_ofst -= u4_range;                                                    \
        u4_range = u4_rlps;                                                     \
    }                                                                           \
                                                                                \
    *pu1_ctxt_model = gau1_ihevc_next_state[(state_mps << 1) | u4_bin];         \
                                                                                \
    /*****************************************************************/         \
    /* Re-normalization; calculate bits generated based on range(R)  */         \
    /*****************************************************************/         \
    if(u4_range < (1 << 8))                                                     \
    {                                                                           \
        UWORD32 u4_bits;                                                        \
        WORD32 numbits;                                                         \
        numbits = CLZ(u4_range);                                                \
        numbits -= (32 - RANGE_NUMBITS);                                        \
        BITS_GET(u4_bits, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,         \
                 ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word, numbits);    \
                                                                                \
        u4_ofst <<= numbits;                                                    \
        u4_ofst |= u4_bits;                                                     \
        u4_range <<= numbits;                                                   \
                                                                                \
    }                                                                           \
    /* Update the cabac context */                                              \
    ps_cabac->u4_range = u4_range;                                              \
    ps_cabac->u4_ofst = u4_ofst;                                                \
                                                                                \
}
#else
#define IHEVCD_CABAC_DECODE_BIN(u4_bin, ps_cabac, ps_bitstrm, ctxt_index)       \
        u4_bin = ihevcd_cabac_decode_bin(ps_cabac, ps_bitstrm, ctxt_index);
#endif

#if CABAC_DECODE_BYPASS_BIN
#define IHEVCD_CABAC_DECODE_BYPASS_BIN(u4_bin, ps_cabac, ps_bitstrm)            \
{                                                                               \
                                                                                \
    UWORD32 u4_range = ps_cabac->u4_range;                                      \
    UWORD32 u4_ofst = ps_cabac->u4_ofst;                                        \
    UWORD32 u4_bits;                                                            \
                                                                                \
    /* Sanity checks */                                                         \
    ASSERT(FULLRANGE == 1);                                                     \
    ASSERT(u4_range >= 256);                                                    \
                                                                                \
    BIT_GET(u4_bits, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,              \
            ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word);                  \
                                                                                \
    u4_ofst <<= 1;                                                              \
    u4_ofst |= u4_bits;                                                         \
                                                                                \
    u4_bin = 0;                                                                 \
    if(u4_ofst >= u4_range)                                                     \
    {                                                                           \
        u4_bin = 1;                                                             \
        u4_ofst -= u4_range;                                                    \
    }                                                                           \
                                                                                \
    /* Update the cabac context */                                              \
    ps_cabac->u4_ofst = u4_ofst;                                                \
}
#else

#define IHEVCD_CABAC_DECODE_BYPASS_BIN(u4_bin, ps_cabac, ps_bitstrm)            \
                u4_bin = ihevcd_cabac_decode_bypass_bin(ps_cabac, ps_bitstrm);
#endif

#if CABAC_DECODE_BYPASS_BINS
#define IHEVCD_CABAC_DECODE_BYPASS_BINS(u4_bins, ps_cabac, ps_bitstrm, numbins) \
{                                                                               \
    UWORD32 u4_range = ps_cabac->u4_range;                                      \
    UWORD32 u4_ofst = ps_cabac->u4_ofst;                                        \
    UWORD32 u4_bits;                                                            \
    ASSERT(FULLRANGE == 1);                                                     \
    ASSERT(u4_range >= 256);                                                    \
    ASSERT(numbins > 0);                                                        \
    {                                                                           \
        WORD32 numbins_tmp = numbins;                                           \
        /* Sanity checks */                                                     \
        ASSERT(numbins < 17);                                                   \
                                                                                \
        u4_bins = 0;                                                            \
                                                                                \
        BITS_GET(u4_bits, ps_bitstrm->pu4_buf, ps_bitstrm->u4_bit_ofst,         \
                    ps_bitstrm->u4_cur_word, ps_bitstrm->u4_nxt_word, numbins); \
        do                                                                      \
        {                                                                       \
            UWORD32 u4_bit;                                                     \
            numbins_tmp--;                                                      \
            u4_bit = (u4_bits >> numbins_tmp) & 1;                              \
            u4_ofst <<= 1;                                                      \
            u4_ofst |= u4_bit;                                                  \
                                                                                \
            u4_bins <<= 1;                                                      \
            if(u4_ofst >= u4_range)                                             \
            {                                                                   \
                u4_bins += 1;                                                   \
                u4_ofst -= u4_range;                                            \
            }                                                                   \
        }while(numbins_tmp);                                                    \
                                                                                \
        /* Update the cabac context */                                          \
        ps_cabac->u4_ofst = u4_ofst;                                            \
    }                                                                           \
}


#else

#define IHEVCD_CABAC_DECODE_BYPASS_BINS(u4_bins, ps_cabac, ps_bitstrm, numbins) \
      u4_bins = ihevcd_cabac_decode_bypass_bins(ps_cabac, ps_bitstrm, numbins);

#endif
/*****************************************************************************/
/* Structures                                                                */
/*****************************************************************************/



/*****************************************************************************/
/* Extern Function Declarations                                              */
/*****************************************************************************/
IHEVCD_ERROR_T    ihevcd_cabac_init
(
                cab_ctxt_t  *ps_cabac,
                bitstrm_t   *ps_bitstrm,
                WORD32      slice_qp,
                WORD32      cabac_init_idc,
                const UWORD8      *pu1_init_ctxt
);



UWORD32    ihevcd_cabac_decode_bin
(
                cab_ctxt_t  *ps_cabac,
                bitstrm_t *ps_bitstrm,
                WORD32      ctxt_index
);

UWORD32    ihevcd_cabac_decode_bypass_bin
(
                cab_ctxt_t  *ps_cabac,
                bitstrm_t *ps_bitstrm
);

UWORD32    ihevcd_cabac_decode_terminate
(
                cab_ctxt_t  *ps_cabac,
                bitstrm_t *ps_bitstrm
);

UWORD32    ihevcd_cabac_decode_bypass_bins
(
                cab_ctxt_t  *ps_cabac,
                bitstrm_t *ps_bitstrm,
                WORD32       num_bins
);

UWORD32    ihevcd_cabac_decode_bins_tunary
(
                cab_ctxt_t  *ps_cabac,
                bitstrm_t *ps_bitstrm,
                WORD32       c_max,
                WORD32       ctxt_index,
                WORD32       ctxt_shift,
                WORD32       ctxt_inc_max

);

UWORD32    ihevcd_cabac_decode_bypass_bins_tunary
(
                cab_ctxt_t  *ps_cabac,
                bitstrm_t *ps_bitstrm,
                WORD32       c_max

);

UWORD32    ihevcd_cabac_decode_bypass_bins_egk
(
                cab_ctxt_t  *ps_cabac,
                bitstrm_t *ps_bitstrm,
                WORD32       k
);

UWORD32    ihevcd_cabac_decode_bypass_bins_trunc_rice
(
                cab_ctxt_t  *ps_cabac,
                bitstrm_t *ps_bitstrm,
                WORD32       c_rice_param,
                WORD32       c_rice_max
);

IHEVCD_ERROR_T  ihevcd_cabac_flush(cab_ctxt_t  *ps_cabac);

IHEVCD_ERROR_T ihevcd_cabac_reset(cab_ctxt_t *ps_cabac,
                                  bitstrm_t *ps_bitstrm);

#endif /* _IHEVCD_CABAC_H_ */
