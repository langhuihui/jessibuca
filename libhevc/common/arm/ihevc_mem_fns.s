@/*****************************************************************************
@*
@* Copyright (C) 2012 Ittiam Systems Pvt Ltd, Bangalore
@*
@* Licensed under the Apache License, Version 2.0 (the "License");
@* you may not use this file except in compliance with the License.
@* You may obtain a copy of the License at:
@*
@* http://www.apache.org/licenses/LICENSE-2.0
@*
@* Unless required by applicable law or agreed to in writing, software
@* distributed under the License is distributed on an "AS IS" BASIS,
@* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@* See the License for the specific language governing permissions and
@* limitations under the License.
@*
@*****************************************************************************/
@/**
@ *******************************************************************************
@ * ,:file
@ *  ihevc_mem_fns_neon.s
@ *
@ * ,:brief
@ *  Contains function definitions for memory manipulation
@ *
@ * ,:author
@ *  Naveen SR
@ *
@ * ,:par List of Functions:
@ *  - ihevc_memcpy()
@ *  - ihevc_memset_mul_8()
@ *  - ihevc_memset_16bit_mul_8()
@ *
@ * ,:remarks
@ *  None
@ *
@ *******************************************************************************
@*/

@/**
@*******************************************************************************
@*
@* ,:brief
@*   memcpy of a 1d array
@*
@* ,:par Description:
@*   Does memcpy of 8bit data from source to destination for 8,16 or 32 number of bytes
@*
@* ,:param[in] pu1_dst
@*  UWORD8 pointer to the destination
@*
@* ,:param[in] pu1_src
@*  UWORD8 pointer to the source
@*
@* ,:param[in] num_bytes
@*  number of bytes to copy
@* ,:returns
@*
@* ,:remarks
@*  None
@*
@*******************************************************************************
@*/
@void ihevc_memcpy_mul_8(UWORD8 *pu1_dst,
@                    UWORD8 *pu1_src,
@                   UWORD8 num_bytes)
@**************Variables Vs Registers*************************
@   r0 => *pu1_dst
@   r1 => *pu1_src
@   r2 => num_bytes

.text
.p2align 2




    .global ihevc_memcpy_mul_8_a9q
.type ihevc_memcpy_mul_8_a9q, %function

ihevc_memcpy_mul_8_a9q:

LOOP_NEON_MEMCPY_MUL_8:
    @ Memcpy 8 bytes
    VLD1.8      d0,[r1]!
    VST1.8      d0,[r0]!

    SUBS        r2,r2,#8
    BNE         LOOP_NEON_MEMCPY_MUL_8
    MOV         PC,LR



@*******************************************************************************
@*/
@void ihevc_memcpy(UWORD8 *pu1_dst,
@                  UWORD8 *pu1_src,
@                  UWORD8 num_bytes)
@**************Variables Vs Registers*************************
@   r0 => *pu1_dst
@   r1 => *pu1_src
@   r2 => num_bytes



    .global ihevc_memcpy_a9q
.type ihevc_memcpy_a9q, %function

ihevc_memcpy_a9q:
    SUBS        r2,#8
    BLT         ARM_MEMCPY
LOOP_NEON_MEMCPY:
    @ Memcpy 8 bytes
    VLD1.8      d0,[r1]!
    VST1.8      d0,[r0]!

    SUBS        r2,#8
    BGE         LOOP_NEON_MEMCPY
    CMP         r2,#-8
    BXEQ        LR

ARM_MEMCPY:
    ADD         r2,#8

LOOP_ARM_MEMCPY:
    LDRB        r3,[r1],#1
    STRB        r3,[r0],#1
    SUBS        r2,#1
    BNE         LOOP_ARM_MEMCPY
    BX          LR




@void ihevc_memset_mul_8(UWORD8 *pu1_dst,
@                       UWORD8 value,
@                       UWORD8 num_bytes)
@**************Variables Vs Registers*************************
@   r0 => *pu1_dst
@   r1 => value
@   r2 => num_bytes

.text
.p2align 2



    .global ihevc_memset_mul_8_a9q
.type ihevc_memset_mul_8_a9q, %function

ihevc_memset_mul_8_a9q:

@ Assumptions: numbytes is either 8, 16 or 32
    VDUP.8      d0,r1
LOOP_MEMSET_MUL_8:
    @ Memset 8 bytes
    VST1.8      d0,[r0]!

    SUBS        r2,r2,#8
    BNE         LOOP_MEMSET_MUL_8

    BX          LR




@void ihevc_memset(UWORD8 *pu1_dst,
@                       UWORD8 value,
@                       UWORD8 num_bytes)
@**************Variables Vs Registers*************************
@   r0 => *pu1_dst
@   r1 => value
@   r2 => num_bytes



    .global ihevc_memset_a9q
.type ihevc_memset_a9q, %function

ihevc_memset_a9q:
    SUBS        r2,#8
    BLT         ARM_MEMSET
    VDUP.8      d0,r1
LOOP_NEON_MEMSET:
    @ Memcpy 8 bytes
    VST1.8      d0,[r0]!

    SUBS        r2,#8
    BGE         LOOP_NEON_MEMSET
    CMP         r2,#-8
    BXEQ        LR

ARM_MEMSET:
    ADD         r2,#8

LOOP_ARM_MEMSET:
    STRB        r1,[r0],#1
    SUBS        r2,#1
    BNE         LOOP_ARM_MEMSET
    BX          LR




@void ihevc_memset_16bit_mul_8(UWORD16 *pu2_dst,
@                                   UWORD16 value,
@                                   UWORD8 num_words)
@**************Variables Vs Registers*************************
@   r0 => *pu2_dst
@   r1 => value
@   r2 => num_words

.text
.p2align 2



    .global ihevc_memset_16bit_mul_8_a9q
.type ihevc_memset_16bit_mul_8_a9q, %function

ihevc_memset_16bit_mul_8_a9q:

@ Assumptions: num_words is either 8, 16 or 32

    @ Memset 8 words
    VDUP.16     d0,r1
LOOP_MEMSET_16BIT_MUL_8:
    VST1.16     d0,[r0]!
    VST1.16     d0,[r0]!

    SUBS        r2,r2,#8
    BNE         LOOP_MEMSET_16BIT_MUL_8

    BX          LR




@void ihevc_memset_16bit(UWORD16 *pu2_dst,
@                       UWORD16 value,
@                       UWORD8 num_words)
@**************Variables Vs Registers*************************
@   r0 => *pu2_dst
@   r1 => value
@   r2 => num_words



    .global ihevc_memset_16bit_a9q
.type ihevc_memset_16bit_a9q, %function

ihevc_memset_16bit_a9q:
    SUBS        r2,#8
    BLT         ARM_MEMSET_16BIT
    VDUP.16     d0,r1
LOOP_NEON_MEMSET_16BIT:
    @ Memset 8 words
    VST1.16     d0,[r0]!
    VST1.16     d0,[r0]!

    SUBS        r2,#8
    BGE         LOOP_NEON_MEMSET_16BIT
    CMP         r2,#-8
    BXEQ        LR

ARM_MEMSET_16BIT:
    ADD         r2,#8

LOOP_ARM_MEMSET_16BIT:
    STRH        r1,[r0],#2
    SUBS        r2,#1
    BNE         LOOP_ARM_MEMSET_16BIT
    BX          LR




    .section .note.GNU-stack,"",%progbits

