///*****************************************************************************
//*
//* Copyright (C) 2012 Ittiam Systems Pvt Ltd, Bangalore
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at:
//*
//* http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.
//*
//*****************************************************************************/
///**
// *******************************************************************************
// * ,:file
// *  ihevc_mem_fns_neon.s
// *
// * ,:brief
// *  Contains function definitions for memory manipulation
// *
// * ,:author
// *     Naveen SR
// *
// * ,:par List of Functions:
// *  - ihevc_memcpy()
// *  - ihevc_memset_mul_8()
// *  - ihevc_memset_16bit_mul_8()
// *
// * ,:remarks
// *  None
// *
// *******************************************************************************
//*/

///**
//*******************************************************************************
//*
//* ,:brief
//*   memcpy of a 1d array
//*
//* ,:par Description:
//*   Does memcpy of 8bit data from source to destination for 8,16 or 32 number of bytes
//*
//* ,:param[in] pu1_dst
//*  UWORD8 pointer to the destination
//*
//* ,:param[in] pu1_src
//*  UWORD8 pointer to the source
//*
//* ,:param[in] num_bytes
//*  number of bytes to copy
//* ,:returns
//*
//* ,:remarks
//*  None
//*
//*******************************************************************************
//*/
//void ihevc_memcpy_mul_8(UWORD8 *pu1_dst,
//                      UWORD8 *pu1_src,
//                      UWORD8 num_bytes)
//**************Variables Vs Registers*************************
//    x0 => *pu1_dst
//    x1 => *pu1_src
//    x2 => num_bytes

.text
.p2align 2


    .global ihevc_memcpy_mul_8_av8
.type ihevc_memcpy_mul_8_av8, %function

ihevc_memcpy_mul_8_av8:

LOOP_NEON_MEMCPY_MUL_8:
    // Memcpy 8 bytes
    LD1         {v0.8b},[x1],#8
    ST1         {v0.8b},[x0],#8

    SUBS        x2,x2,#8
    BNE         LOOP_NEON_MEMCPY_MUL_8
    ret



//*******************************************************************************
//*/
//void ihevc_memcpy(UWORD8 *pu1_dst,
//                  UWORD8 *pu1_src,
//                  UWORD8 num_bytes)
//**************Variables Vs Registers*************************
//    x0 => *pu1_dst
//    x1 => *pu1_src
//    x2 => num_bytes



    .global ihevc_memcpy_av8
.type ihevc_memcpy_av8, %function

ihevc_memcpy_av8:
    SUBS        x2,x2,#8
    BLT         ARM_MEMCPY
LOOP_NEON_MEMCPY:
    // Memcpy 8 bytes
    LD1         {v0.8b},[x1],#8
    ST1         {v0.8b},[x0],#8

    SUBS        x2,x2,#8
    BGE         LOOP_NEON_MEMCPY
    CMN         x2,#8
    BEQ         MEMCPY_RETURN

ARM_MEMCPY:
    ADD         x2,x2,#8

LOOP_ARM_MEMCPY:
    LDRB        w3,[x1],#1
    STRB        w3,[x0],#1
    SUBS        x2,x2,#1
    BNE         LOOP_ARM_MEMCPY
MEMCPY_RETURN:
    ret




//void ihevc_memset_mul_8(UWORD8 *pu1_dst,
//                       UWORD8 value,
//                       UWORD8 num_bytes)
//**************Variables Vs Registers*************************
//    x0 => *pu1_dst
//    x1 => value
//    x2 => num_bytes

.text
.p2align 2



    .global ihevc_memset_mul_8_av8
.type ihevc_memset_mul_8_av8, %function

ihevc_memset_mul_8_av8:

// Assumptions: numbytes is either 8, 16 or 32
    dup         v0.8b,w1
LOOP_MEMSET_MUL_8:
    // Memset 8 bytes
    ST1         {v0.8b},[x0],#8

    SUBS        x2,x2,#8
    BNE         LOOP_MEMSET_MUL_8

    ret




//void ihevc_memset(UWORD8 *pu1_dst,
//                       UWORD8 value,
//                       UWORD8 num_bytes)
//**************Variables Vs Registers*************************
//    x0 => *pu1_dst
//    x1 => value
//    x2 => num_bytes



    .global ihevc_memset_av8
.type ihevc_memset_av8, %function

ihevc_memset_av8:
    SUBS        x2,x2,#8
    BLT         ARM_MEMSET
    dup         v0.8b,w1
LOOP_NEON_MEMSET:
    // Memcpy 8 bytes
    ST1         {v0.8b},[x0],#8

    SUBS        x2,x2,#8
    BGE         LOOP_NEON_MEMSET
    CMN         x2,#8
    BEQ         MEMSET_RETURN

ARM_MEMSET:
    ADD         x2,x2,#8

LOOP_ARM_MEMSET:
    STRB        w1,[x0],#1
    SUBS        x2,x2,#1
    BNE         LOOP_ARM_MEMSET

MEMSET_RETURN:
    ret




//void ihevc_memset_16bit_mul_8(UWORD16 *pu2_dst,
//                                      UWORD16 value,
//                                      UWORD8 num_words)
//**************Variables Vs Registers*************************
//    x0 => *pu2_dst
//    x1 => value
//    x2 => num_words

.text
.p2align 2



    .global ihevc_memset_16bit_mul_8_av8
.type ihevc_memset_16bit_mul_8_av8, %function

ihevc_memset_16bit_mul_8_av8:

// Assumptions: num_words is either 8, 16 or 32

    // Memset 8 words
    dup         v0.8h,w1
LOOP_MEMSET_16BIT_MUL_8:
    ST1         {v0.8h},[x0],#16

    SUBS        x2,x2,#8
    BNE         LOOP_MEMSET_16BIT_MUL_8

    ret




//void ihevc_memset_16bit(UWORD16 *pu2_dst,
//                       UWORD16 value,
//                       UWORD8 num_words)
//**************Variables Vs Registers*************************
//    x0 => *pu2_dst
//    x1 => value
//    x2 => num_words



    .global ihevc_memset_16bit_av8
.type ihevc_memset_16bit_av8, %function

ihevc_memset_16bit_av8:
    SUBS        x2,x2,#8
    BLT         ARM_MEMSET_16BIT
    dup         v0.8h,w1
LOOP_NEON_MEMSET_16BIT:
    // Memset 8 words
    ST1         {v0.8h},[x0],#16

    SUBS        x2,x2,#8
    BGE         LOOP_NEON_MEMSET_16BIT
    CMN         x2,#8
    BEQ         MEMSET_16BIT_RETURN

ARM_MEMSET_16BIT:
    ADD         x2,x2,#8

LOOP_ARM_MEMSET_16BIT:
    STRH        w1,[x0],#2
    SUBS        x2,x2,#1
    BNE         LOOP_ARM_MEMSET_16BIT

MEMSET_16BIT_RETURN:
    ret




    .section .note.GNU-stack,"",%progbits

