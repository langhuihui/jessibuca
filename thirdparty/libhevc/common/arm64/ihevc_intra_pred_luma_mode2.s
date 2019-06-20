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
//*******************************************************************************
//* @file
//*  ihevc_intra_pred_luma_mode2_neon.s
//*
//* @brief
//*  contains function definitions for intra prediction dc filtering.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* @author
//*  yogeswaran rs
//*
//* @par list of functions:
//*
//*
//* @remarks
//*  none
//*
//*******************************************************************************
//*/
///**
//*******************************************************************************
//*
//* @brief
//*    luma intraprediction filter for dc input
//*
//* @par description:
//*
//* @param[in] pu1_ref
//*  uword8 pointer to the source
//*
//* @param[out] pu1_dst
//*  uword8 pointer to the destination
//*
//* @param[in] src_strd
//*  integer source stride
//*
//* @param[in] dst_strd
//*  integer destination stride
//*
//* @param[in] pi1_coeff
//*  word8 pointer to the planar coefficients
//*
//* @param[in] nt
//*  size of tranform block
//*
//* @param[in] mode
//*  type of filtering
//*
//* @returns
//*
//* @remarks
//*  none
//*
//*******************************************************************************
//*/

//void ihevc_intra_pred_luma_mode2(uword8 *pu1_ref,
//                                 word32 src_strd,
//                                 uword8 *pu1_dst,
//                                 word32 dst_strd,
//                                 word32 nt,
//                                 word32 mode)
//
//**************variables vs registers*****************************************
//x0 => *pu1_ref
//x1 => src_strd
//x2 => *pu1_dst
//x3 => dst_strd

//stack contents from #40
//    nt
//    mode
//    pi1_coeff

.text
.align 4
.include "ihevc_neon_macros.s"



.globl ihevc_intra_pred_luma_mode2_av8

.type ihevc_intra_pred_luma_mode2_av8, %function

ihevc_intra_pred_luma_mode2_av8:

    // stmfd sp!, {x4-x12, x14}    //stack stores the values of the arguments

    stp         x19, x20,[sp,#-16]!

    mov         x8,#-2

    cmp         x4,#4
    beq         mode2_4

    add         x0,x0,x4,lsl #1

    sub         x0,x0,#9                    //src[1]
    sub         x10,x0,#1

prologue_cpy_32:

    ld1         {v0.8b},[x0],x8
    mov         x11,x4

    ld1         {v1.8b},[x10],x8
    mov         x6, x2

    ld1         {v2.8b},[x0],x8
    ld1         {v3.8b},[x10],x8
    lsr         x1, x4, #3

    ld1         {v4.8b},[x0],x8
    ld1         {v5.8b},[x10],x8
    ld1         {v6.8b},[x0],x8
    mul         x1, x4, x1

    ld1         {v7.8b},[x10],x8
    add         x7,x6,x3

    rev64       v16.8b,  v0.8b
    rev64       v17.8b,  v1.8b
    lsl         x5, x3, #2

    rev64       v18.8b,  v2.8b
    rev64       v19.8b,  v3.8b
    add         x9,x7,x3

    rev64       v20.8b,  v4.8b
    subs        x1,x1,#8

    rev64       v21.8b,  v5.8b
    rev64       v22.8b,  v6.8b
    rev64       v23.8b,  v7.8b
    add         x14,x9,x3

    beq         epilogue_mode2

    sub         x12,x4,#8

kernel_mode2:

    st1         {v16.8b},[x6],x5
    st1         {v17.8b},[x7],x5
    subs        x11,x11,#8

    st1         {v18.8b},[x9],x5
    add         x20,x2,#8
    csel        x2, x20, x2,gt

    st1         {v19.8b},[x14],x5
    st1         {v20.8b},[x6],x5
    csel        x11, x4, x11,le

    st1         {v21.8b},[x7],x5
    st1         {v22.8b},[x9],x5
    add         x20, x2, x3, lsl #2
    csel        x2, x20, x2,le

    st1         {v23.8b},[x14],x5
    ld1         {v0.8b},[x0],x8
    sub         x14,x4,#8

    ld1         {v1.8b},[x10],x8
    ld1         {v2.8b},[x0],x8
    add         x20, x2, #8
    csel        x2, x20, x2,le

    ld1         {v3.8b},[x10],x8
    ld1         {v4.8b},[x0],x8
    sub         x20, x6, x14
    csel        x2, x20, x2,le

    ld1         {v5.8b},[x10],x8
    subs        x12,x12,#8

    ld1         {v6.8b},[x0],x8
    mov         x6, x2

    ld1         {v7.8b},[x10],x8
    add         x20, x0, x4
    csel        x0, x20, x0,le

    rev64       v16.8b,  v0.8b
    add         x7, x6, x3

    rev64       v17.8b,  v1.8b
    sub         x20, x0, #8
    csel        x0, x20, x0,le

    rev64       v18.8b,  v2.8b
    csel        x12, x4, x12,le

    rev64       v19.8b,  v3.8b
    add         x9, x7, x3

    rev64       v20.8b,  v4.8b
    sub         x10,x0,#1

    rev64       v21.8b,  v5.8b
    subs        x1, x1, #8

    rev64       v22.8b,  v6.8b
    add         x14, x9, x3

    rev64       v23.8b,  v7.8b

    bne         kernel_mode2

epilogue_mode2:

    st1         {v16.8b},[x6],x5
    st1         {v17.8b},[x7],x5
    st1         {v18.8b},[x9],x5
    st1         {v19.8b},[x14],x5
    st1         {v20.8b},[x6],x5
    st1         {v21.8b},[x7],x5
    st1         {v22.8b},[x9],x5
    st1         {v23.8b},[x14],x5

    b           end_func

mode2_4:

    mov         x8,#-2
    sub         x0,x0,#1
    sub         x10,x0,#1

    ld1         {v0.8b},[x0],x8
    add         x5,x2,x3
    ld1         {v2.8b},[x10],x8
    add         x6,x5,x3
    ld1         {v4.8b},[x0]
    add         x7,x6,x3
    ld1         {v6.8b},[x10]

    rev64       v1.8b,  v0.8b
    rev64       v3.8b,  v2.8b



    st1         {v1.s}[0],[x2]
    rev64       v5.8b,  v4.8b
    st1         {v3.s}[0],[x5]
    rev64       v7.8b,  v6.8b
    st1         {v5.s}[0],[x6]
    st1         {v7.s}[0],[x7]

end_func:
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret







