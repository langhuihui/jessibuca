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
//*  ihevc_intra_pred_luma_mode_18_34_neon.s
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

//void ihevc_intra_pred_luma_mode_18_34(uword8 *pu1_ref,
//                                      word32 src_strd,
//                                      uword8 *pu1_dst,
//                                      word32 dst_strd,
//                                      word32 nt,
//                                      word32 mode)
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



.globl ihevc_intra_pred_luma_mode_18_34_av8

.type ihevc_intra_pred_luma_mode_18_34_av8, %function

ihevc_intra_pred_luma_mode_18_34_av8:

    // stmfd sp!, {x4-x12, x14}    //stack stores the values of the arguments
    push_v_regs
    stp         x19, x20,[sp,#-16]!

    cmp         x4,#4
    beq         mode2_4

    mov         x11,x4
    mov         x12,x4
    sub         x14,x4,#8

    add         x0,x0,x4,lsl #1

    cmp         x5,#0x22
    mov         x10,x2

    add         x0,x0,#2
    sub         x20,x0,#2
    csel        x0, x20, x0,ne
    mov         x20,#1
    csel        x6, x20, x6,eq
    mov         x20,#-1
    csel        x6, x20, x6,ne
    mov         x8,x0

prologue_cpy_32:

    ld1         {v0.8b},[x8],x6
    lsr         x1, x4, #3
    ld1         {v1.8b},[x8],x6
    mul         x1, x4, x1
    ld1         {v2.8b},[x8],x6
    ld1         {v3.8b},[x8],x6
    subs        x1,x1,#8
    ld1         {v4.8b},[x8],x6
    ld1         {v5.8b},[x8],x6
    ld1         {v6.8b},[x8],x6

    ld1         {v7.8b},[x8],x6


    beq         epilogue_mode2
    sub         x11,x11,#8

    cmp         x5,#0x22
    add         x20,x0,#8
    csel        x0, x20, x0,ne
    csel        x8, x0, x8,ne
    bne         kernel_mode18
    //add        x8,x0,#8

kernel_mode2:
    st1         {v0.8b},[x10],x3
    st1         {v1.8b},[x10],x3
    subs        x12,x12,#8
    st1         {v2.8b},[x10],x3
    add         x20,x2,#8
    csel        x2, x20, x2,ne
    st1         {v3.8b},[x10],x3

    ld1         {v0.8b},[x8],x6
    st1         {v4.8b},[x10],x3

    st1         {v5.8b},[x10],x3
    ld1         {v1.8b},[x8],x6
    st1         {v6.8b},[x10],x3
    ld1         {v2.8b},[x8],x6
    st1         {v7.8b},[x10],x3

    ld1         {v3.8b},[x8],x6
    sub         x20,x10,x14
    csel        x2, x20, x2,eq
    ld1         {v4.8b},[x8],x6
    mov         x10,x2
    ld1         {v5.8b},[x8],x6
    csel        x12, x4, x12,eq
    ld1         {v6.8b},[x8],x6
    subs        x11,x11,#8

    ld1         {v7.8b},[x8],x6

    add         x20,x0,#8
    csel        x0, x20, x0,eq
    csel        x11, x4, x11,eq
    csel        x8, x0, x8,eq

    subs        x1, x1, #8

    bne         kernel_mode2

    b           epilogue_mode2

kernel_mode18:
    st1         {v0.8b},[x10],x3
    st1         {v1.8b},[x10],x3
    subs        x12,x12,#8
    st1         {v2.8b},[x10],x3
    add         x20,x2,#8
    csel        x2, x20, x2,ne
    st1         {v3.8b},[x10],x3

    ld1         {v0.8b},[x8],x6
    st1         {v4.8b},[x10],x3

    st1         {v5.8b},[x10],x3
    ld1         {v1.8b},[x8],x6

    st1         {v6.8b},[x10],x3
    ld1         {v2.8b},[x8],x6
    st1         {v7.8b},[x10],x3

    ld1         {v3.8b},[x8],x6
    sub         x20,x10,x14
    csel        x2, x20, x2,eq
    ld1         {v4.8b},[x8],x6
    mov         x10,x2
    ld1         {v5.8b},[x8],x6
    csel        x12, x4, x12,eq
    ld1         {v6.8b},[x8],x6
    subs        x11,x11,#8
    ld1         {v7.8b},[x8],x6

    add         x20,x0,#8
    csel        x0, x20, x0,ne
    csel        x11, x4, x11,eq
    sub         x20,x8,x14
    csel        x0, x20, x0,eq
    subs        x1, x1, #8
    mov         x8,x0

    bne         kernel_mode18


epilogue_mode2:

    st1         {v0.8b},[x10],x3
    st1         {v1.8b},[x10],x3
    st1         {v2.8b},[x10],x3
    st1         {v3.8b},[x10],x3
    st1         {v4.8b},[x10],x3
    st1         {v5.8b},[x10],x3
    st1         {v6.8b},[x10],x3
    st1         {v7.8b},[x10],x3

    b           end_func

mode2_4:

    add         x0,x0,#10
    cmp         x5,#0x22
    sub         x20,x0,#2
    csel        x0, x20, x0,ne

    mov         x20,#1
    csel        x8, x20, x8,eq
    mov         x20,#-1
    csel        x8, x20, x8,ne

    ld1         {v0.8b},[x0],x8
    st1         {v0.s}[0],[x2],x3

    ld1         {v0.8b},[x0],x8
    st1         {v0.s}[0],[x2],x3

    ld1         {v0.8b},[x0],x8
    st1         {v0.s}[0],[x2],x3

    ld1         {v0.8b},[x0],x8
    st1         {v0.s}[0],[x2],x3

end_func:
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret







