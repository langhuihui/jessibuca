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
*  ihevcd_trace.h
*
* @brief
*  Header for codec trace messages
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVCD_TRACE_H_
#define _IHEVCD_TRACE_H_

#define FULLRANGE 1


#define RANGE_NUMBITS 31
#define RANGE_SHIFT  (RANGE_NUMBITS - 9)

#ifdef TRACE
/**
 * Context for trace
 */
typedef struct
{
    /**
     * fp
     */
    FILE    *fp;

    /**
     * u8_cnt
     */
    ULWORD64  u8_cnt;
}trace_t;

/**
 * Global context for trace info
 */
extern trace_t g_trace;

/**
 * Call ihevcd_bits_get() to read from bitstream and dumps the data to trace file
 */
#define BITS_PARSE(m_str, m_value, m_ps_bitstrm, m_numbits)                 \
{                                                                           \
    m_value = ihevcd_bits_get(m_ps_bitstrm, m_numbits);                     \
    fprintf( g_trace.fp, "%-40s u(%d) : %d\n", m_str, m_numbits, m_value ); \
    fflush ( g_trace.fp);                                                   \
}

/**
 * Call ihevcd_uev() to read from bitstream and dumps the data to trace file
 */

#define UEV_PARSE(m_str, m_value, m_ps_bitstrm)                             \
{                                                                           \
    m_value = ihevcd_uev(m_ps_bitstrm);                                     \
    fprintf( g_trace.fp, "%-40s ue(v) : %d\n", m_str, m_value );            \
    fflush ( g_trace.fp);                                                   \
}
/**
 * Call ihevcd_sev() to read from bitstream and dumps the data to trace file
 */
#define SEV_PARSE(m_str, m_value, m_ps_bitstrm)                             \
{                                                                           \
    m_value = ihevcd_sev(m_ps_bitstrm);                                     \
    fprintf( g_trace.fp, "%-40s se(v) : %d\n", m_str, m_value );            \
    fflush ( g_trace.fp);                                                   \
}


#if FULLRANGE
#define TRACE_CABAC_CTXT(m_string, m_range, m_ctxt_idx)                  \
{                                                                        \
    UWORD32 m_clz, m_range_shift, m_state_mps;                           \
    m_state_mps = ps_cabac->au1_ctxt_models[m_ctxt_idx];                  \
    m_clz = CLZ(m_range);                                                \
    m_clz -= (32 - RANGE_NUMBITS);                                       \
    m_range_shift = m_range << m_clz;                                    \
    m_range_shift = m_range_shift >> RANGE_SHIFT;                        \
    fprintf( g_trace.fp, "%-40s: Range:%3d State:%3d MPS:%1d\n",         \
        m_string, m_range_shift, m_state_mps >> 1, m_state_mps & 1);     \
    fflush ( g_trace.fp);                                                \
}
#define AEV_TRACE(m_str, m_value, m_range)                                  \
{                                                                           \
    UWORD32 m_clz, m_range_shift;                                           \
    m_clz = CLZ(m_range);                                                   \
    m_clz -= (32 - RANGE_NUMBITS);                                       \
    m_range_shift = m_range << m_clz;                                       \
    m_range_shift = m_range_shift >> RANGE_SHIFT;                           \
    fprintf( g_trace.fp, "%-40s:%8d R:%d\n", m_str, m_value, m_range_shift);\
    fflush ( g_trace.fp);                                                   \
}
#else
#define TRACE_CABAC_CTXT(m_string, m_range, m_ctxt_idx)                  \
{                                                                        \
    UWORD32 m_state_mps;                                                 \
    m_state_mps = ps_cabac->au1_ctxt_models[m_ctxt_idx];                 \
    fprintf( g_trace.fp, "%-40s: Range:%3d State:%3d MPS:%1d\n",         \
        m_string, m_range, m_state_mps >> 1, m_state_mps & 1);           \
    fflush ( g_trace.fp);                                                \
}

#define AEV_TRACE(m_str, m_value, m_range)                              \
{                                                                       \
    fprintf( g_trace.fp, "%-40s:%8d R:%d\n", m_str, m_value, m_range);  \
    fflush ( g_trace.fp);                                               \
}
#endif

#define TUV_PARSE(m_str, m_value, m_ps_bitstrm)                      \
    m_value = ihevcd_bits_get(m_ps_bitstrm, 1);

#define TRACE_INIT(a)   ihevcd_trace_init(a)
#define TRACE_DEINIT(a) ihevcd_trace_deinit(a)

#else /* TRACE */
/**
 * Call ihevcd_bits_get() to read from bitstream
 */

#define BITS_PARSE(m_str, m_value, m_ps_bitstrm, m_numbits)           \
    m_value = ihevcd_bits_get(m_ps_bitstrm, m_numbits);

/**
 * Call ihevcd_uev() to read from bitstream
 */

#define UEV_PARSE(m_str, m_value, m_ps_bitstrm)                       \
    m_value = ihevcd_uev(m_ps_bitstrm);

/**
 * Call ihevcd_sev() to read from bitstream
 */

#define SEV_PARSE(m_str, m_value, m_ps_bitstrm)                       \
    m_value = ihevcd_sev(m_ps_bitstrm);

#define TUV_PARSE(m_str, m_value, m_ps_bitstrm)                      \
    m_value = ihevcd_bits_get(m_ps_bitstrm, 1);

#define TRACE_CABAC_CTXT(m_string, m_range, m_state_mps)

#define AEV_TRACE(m_str, m_value, m_range)


#define TRACE_INIT(a)
#define TRACE_DEINIT(a)
#endif /* TRACE */
#endif /* _IHEVCD_TRACE_H_ */
