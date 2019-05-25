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
*  ihevc_platform_macros.h
*
* @brief
*  Platform specific Macro definitions used in the codec
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/


#ifndef _IHEVC_PLATFORM_MACROS_H_
#define _IHEVC_PLATFORM_MACROS_H_

//#include <immintrin.h>


#define CLIP_U8(x) CLIP3((x), 0,     255)
#define CLIP_S8(x) CLIP3((x), -128,  127)

#define CLIP_U10(x) CLIP3((x), 0,     1023);
#define CLIP_S10(x) CLIP3((x), -512,  511);

#define CLIP_U12(x) CLIP3((x), 0,     4095);
#define CLIP_S12(x) CLIP3((x), -2048,  2047);

#define CLIP_U16(x) CLIP3((x), 0,        65535)
#define CLIP_S16(x) CLIP3((x), -32768,   32767)



#define SHL(x,y) (((y) < 32) ? ((x) << (y)) : 0)
#define SHR(x,y) (((y) < 32) ? ((x) >> (y)) : 0)

#define SHR_NEG(val,shift)  ((shift>0)?(val>>shift):(val<<(-shift)))
#define SHL_NEG(val,shift)  ((shift<0)?(val>>(-shift)):(val<<shift))


#define ITT_BIG_ENDIAN(x)       ((x << 24))                |   \
                            ((x & 0x0000ff00) << 8)    |   \
                            ((x & 0x00ff0000) >> 8)    |   \
                            ((UWORD32)x >> 24);


#define NOP(nop_cnt)    {UWORD32 nop_i; for (nop_i = (nop_cnt) ; nop_i > 0 ; nop_i--) asm("nop");}

#define POPCNT_U32(x)       __builtin_popcount(x)

#define PLD(a)
#define INLINE inline

static INLINE UWORD32 CLZ(UWORD32 u4_word)
{
    if(u4_word)
        return (__builtin_clz(u4_word));
    else
        return 32;
}
static INLINE UWORD32 CLZNZ(UWORD32 u4_word)
{
   return (__builtin_clz(u4_word));
}
static INLINE UWORD32 CTZ(UWORD32 u4_word)
{
    if(0 == u4_word)
        return 31;
    else
    {
        unsigned int index;
        index = __builtin_ctz(u4_word);
        return (UWORD32)index;
    }
}

#define DATA_SYNC()  __sync_synchronize()

/**
******************************************************************************
 *  @brief  returns postion of msb bit for 32bit input
******************************************************************************
 */
#define GET_POS_MSB_32(r,word)                         \
{                                                       \
    if(word)                                           \
    {                                                   \
        r = 31 - __builtin_clz(word);                  \
    }                                                   \
    else                                                \
    {                                                   \
        r = -1;                                         \
    }                                                   \
}

/**
******************************************************************************
 *  @brief  returns postion of msb bit for 64bit input
******************************************************************************
 */
#define GET_POS_MSB_64(r,word)                          \
{                                                       \
    if(word)                                            \
    {                                                   \
        r = 63 - __builtin_clzll(word);                 \
    }                                                   \
    else                                                \
    {                                                   \
        r = -1;                                         \
    }                                                   \
}


/**
******************************************************************************
 *  @brief  returns max number of bits required to represent input word (max 32bits)
******************************************************************************
 */
#define GETRANGE(r,word)                                \
{                                                       \
    if(word)                                            \
    {                                                   \
        r = 32 - __builtin_clz(word);                   \
    }                                                   \
    else                                                \
    {                                                   \
        r = 1;                                          \
    }                                                   \
}
#define GCC_ENABLE 0

#if GCC_ENABLE
#define _mm256_loadu2_m128i(X,Y) _mm256_insertf128_si256(_mm256_castsi128_si256(_mm_loadu_si128((Y))), _mm_loadu_si128((X)),1);

#define _mm256_storeu2_m128i(X,Y,Z) {_mm_storeu_si128 ((Y), _mm256_castsi256_si128((Z)));_mm_storeu_si128 ((X), _mm256_extracti128_si256((Z),1));}

#define _mm256_set_m128i(X,Y) _mm256_insertf128_si256(_mm256_castsi128_si256((Y)),(X),1);

#endif


#define PREFETCH_ENABLE 1

#if PREFETCH_ENABLE
#define PREFETCH(ptr, type) _mm_prefetch(ptr, type);
#else
#define PREFETCH(ptr, type)
#endif

#define MEM_ALIGN8 __attribute__ ((aligned (8)))
#define MEM_ALIGN16 __attribute__ ((aligned (16)))
#define MEM_ALIGN32 __attribute__ ((aligned (32)))

#endif /* _IHEVC_PLATFORM_MACROS_H_ */
