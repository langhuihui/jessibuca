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



.globl ihevc_intra_pred_chroma_mode2_av8

.type ihevc_intra_pred_chroma_mode2_av8, %function

ihevc_intra_pred_chroma_mode2_av8:

    // stmfd sp!, {x4-x12, x14}    //stack stores the values of the arguments
    push_v_regs
    stp         x19, x20,[sp,#-16]!

    mov         x8,#-4

    cmp         x4,#4
    beq         mode2_4

    add         x0,x0,x4,lsl #2

    sub         x0,x0,#0x12                 //src[1]
    sub         x10,x0,#2

prologue_cpy_32:

    ld2         {v0.8b, v1.8b},[x0],x8

    mov         x11,x4
    rev64       v16.8b,  v0.8b
    rev64       v17.8b,  v1.8b

    ld2         {v2.8b, v3.8b},[x10],x8
    mov         x6, x2

    ld2         {v4.8b, v5.8b},[x0],x8
    ld2         {v6.8b, v7.8b},[x10],x8
    lsr         x1, x4, #3

    ld2         {v8.8b, v9.8b},[x0],x8
    ld2         {v10.8b, v11.8b},[x10],x8
    ld2         {v12.8b, v13.8b},[x0],x8
    mul         x1, x4, x1

    ld2         {v14.8b, v15.8b},[x10],x8
    add         x7,x6,x3

    rev64       v18.8b,  v2.8b
    rev64       v19.8b,  v3.8b
    lsl         x5, x3, #2

    rev64       v20.8b,  v4.8b
    rev64       v21.8b,  v5.8b
    add         x9,x7,x3

    rev64       v22.8b,  v6.8b
    rev64       v23.8b,  v7.8b

    rev64       v24.8b,  v8.8b
    rev64       v25.8b,  v9.8b

    rev64       v26.8b,  v10.8b
    subs        x1,x1,#8

    rev64       v27.8b,  v11.8b

    rev64       v28.8b,  v12.8b
    rev64       v29.8b,  v13.8b

    rev64       v30.8b,  v14.8b
    add         x14,x9,x3
    rev64       v31.8b,  v15.8b

    beq         epilogue_mode2

    sub         x12,x4,#8

kernel_mode2:

    st2         {v16.8b, v17.8b},[x6],x5
    st2         {v18.8b, v19.8b},[x7],x5
    subs        x11,x11,#8
    st2         {v20.8b, v21.8b},[x9],x5
    st2         {v22.8b, v23.8b},[x14],x5
    st2         {v24.8b, v25.8b},[x6],x5
    add         x20,x2,#16
    csel        x2, x20, x2,gt
    st2         {v26.8b, v27.8b},[x7],x5
    st2         {v28.8b, v29.8b},[x9],x5
    st2         {v30.8b, v31.8b},[x14],x5

    ld2         {v0.8b, v1.8b},[x0],x8
    csel        x11, x4, x11,le

    ld2         {v2.8b, v3.8b},[x10],x8
    ld2         {v4.8b, v5.8b},[x0],x8
    add         x20, x2, x3, lsl #2
    csel        x2, x20, x2,le
    ld2         {v6.8b, v7.8b},[x10],x8
    rev64       v16.8b,  v0.8b

    ld2         {v8.8b, v9.8b},[x0],x8
    ld2         {v10.8b, v11.8b},[x10],x8
    sub         x20, x6,#16
    csel        x2, x20, x2,le
    ld2         {v12.8b, v13.8b},[x0],x8
    rev64       v17.8b,  v1.8b
    ld2         {v14.8b, v15.8b},[x10],x8

    subs        x12,x12,#8
    mov         x6, x2
    add         x20, x0, x4,lsl #1
    csel        x0, x20, x0,le
    add         x7, x6, x3

    rev64       v18.8b,  v2.8b
    sub         x20, x0, #16
    csel        x0, x20, x0,le
    rev64       v19.8b,  v3.8b

    rev64       v20.8b,  v4.8b
    csel        x12, x4, x12,le
    rev64       v21.8b,  v5.8b

    rev64       v22.8b,  v6.8b
    add         x9, x7, x3
    rev64       v23.8b,  v7.8b

    rev64       v24.8b,  v8.8b
    sub         x10,x0,#2
    rev64       v25.8b,  v9.8b

    rev64       v26.8b,  v10.8b
    subs        x1, x1, #8
    rev64       v27.8b,  v11.8b

    rev64       v28.8b,  v12.8b
    rev64       v29.8b,  v13.8b

    rev64       v30.8b,  v14.8b
    add         x14, x9, x3
    rev64       v31.8b,  v15.8b

    bne         kernel_mode2

epilogue_mode2:

    st2         {v16.8b, v17.8b},[x6],x5
    st2         {v18.8b, v19.8b},[x7],x5
    st2         {v20.8b, v21.8b},[x9],x5
    st2         {v22.8b, v23.8b},[x14],x5
    st2         {v24.8b, v25.8b},[x6],x5
    st2         {v26.8b, v27.8b},[x7],x5
    st2         {v28.8b, v29.8b},[x9],x5
    st2         {v30.8b, v31.8b},[x14],x5

    b           end_func

mode2_4:

    lsl         x12,x4,#1
    add         x0,x0,x12
    sub         x0,x0,#2

    ld2         {v12.8b, v13.8b},[x0],x8
    shl         d0, d12,#32
    add         x10,x0,#2
    shl         d1, d13,#32

    rev64       v0.8b,  v0.8b
    ld2         {v14.8b, v15.8b},[x10],x8
    shl         d2, d14,#32

    rev64       v1.8b,  v1.8b
    shl         d3, d15,#32
    zip1        v0.8b, v0.8b, v1.8b
    zip2        v1.8b, v0.8b, v1.8b
    st1         {v0.8b},[x2],x3

    rev64       v2.8b,  v2.8b
    ld2         {v16.8b, v17.8b},[x0],x8
    shl         d4, d16,#32
    rev64       v3.8b,  v3.8b
    shl         d5, d17,#32
    zip1        v2.8b, v2.8b, v3.8b
    zip2        v3.8b, v2.8b, v3.8b
    rev64       v4.8b,  v4.8b
    rev64       v5.8b,  v5.8b
    st1         {v2.8b},[x2],x3


    ld2         {v18.8b, v19.8b},[x10],x8
    shl         d6, d18,#32

    zip1        v4.8b, v4.8b, v5.8b
    zip2        v5.8b, v4.8b, v5.8b
    shl         d7, d19,#32
    rev64       v6.8b,  v6.8b
    st1         {v4.8b},[x2],x3

    rev64       v7.8b,  v7.8b
    zip1        v6.8b, v6.8b, v7.8b
    zip2        v7.8b, v6.8b, v7.8b
    st1         {v6.8b},[x2],x3

end_func:
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret






