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
*  ihevc_macros.h
*
* @brief
*  Macro definitions used in the codec
*
* @author
*  Ittiam
*
* @remarks
*  None
*
*******************************************************************************
*/
#ifndef _IHEVC_MACROS_H_
#define _IHEVC_MACROS_H_

#define RETURN_IF(cond, retval) if(cond) {return (retval);}
#define UNUSED(x) ((void)(x))

#define CLIP3(x, min, max) (((x) > (max)) ? (max) :(((x) < (min))? (min):(x)))

#define MAX(x,y)    (((x) > (y)) ? (x) :(y))
#define MIN(x,y)    (((x) < (y)) ? (x) :(y))
#define SIGN(x)     ((x) >= 0 ? ((x)>0 ? 1: 0) : -1)
#define ABS(x)      ((((WORD32)(x)) > 0)           ? (x) : -(x))

#define ALIGN128(x) ((((x) + 127) >> 7) << 7)
#define ALIGN64(x)  ((((x) + 63) >> 6) << 6)
#define ALIGN32(x)  ((((x) + 31) >> 5) << 5)
#define ALIGN16(x)  ((((x) + 15) >> 4) << 4)
#define ALIGN8(x)   ((((x) + 7) >> 3) << 3)
#define ALIGN4(x)   ((((x) + 3) >> 2) << 2)

#define ALIGN_POW2(ptr,align) ((((WORD32)ptr)+align-1)&(~(align-1)))

/** Sets x bits to '1' starting from MSB */
#define MSB_ONES(x) ((UWORD32)0xFFFFFFFF << (32 - (x)))

/** Generates a pattern of x number of '01' in binary starting from MSB */
#define DUP_MSB_01(x) ((UWORD32)0x55555555 << (32 - ((x) * 2)))

/** Generates a pattern of x number of '10' in binary starting from MSB */
#define DUP_MSB_10(x) ((UWORD32)0xAAAAAAAA << (32 - ((x) * 2)))

/** Generates a pattern of x number of '11' in binary starting from MSB */
#define DUP_MSB_11(x) ((UWORD32)0xFFFFFFFF << (32 - ((x) * 2)))

/** Sets x bits to '1' starting from LSB */
#define LSB_ONES(x) ((UWORD32)0xFFFFFFFF >> (32 - (x)))

/** Generates a pattern of x number of '01' in binary starting from LSB */
#define DUP_LSB_01(x) ((UWORD32)0x55555555 >> (32 - ((x) * 2)))

/** Generates a pattern of x number of '10' in binary starting from LSB */
#define DUP_LSB_10(x) ((UWORD32)0xAAAAAAAA >> (32 - ((x) * 2)))

/** Generates a pattern of x number of '11' in binary starting from LSB */
#define DUP_LSB_11(x) ((UWORD32)0xFFFFFFFF >> (32 - ((x) * 2)))

/** Sets the bit in given position to 1 */
#define BITSET(x, pos) ((x) | (1 << (pos)))

/** Swap two variables */
#define SWAP(X,Y)                   \
{                                   \
    (X) = (X) ^ (Y);                \
    (Y) = (X) ^ (Y);                \
    (X) = (X) ^ (Y);                \
}
#endif /*_IHEVCD_MACROS_H_*/
