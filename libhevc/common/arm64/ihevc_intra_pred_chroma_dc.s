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
//*  ihevc_intra_pred_chroma_dc_neon.s
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

//void ihevc_intra_pred_chroma_dc(uword8 *pu1_ref,
//                                word32 src_strd,
//                                uword8 *pu1_dst,
//                                word32 dst_strd,
//                                word32 nt,
//                                word32 mode)
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



.globl ihevc_intra_pred_chroma_dc_av8

.type ihevc_intra_pred_chroma_dc_av8, %function

ihevc_intra_pred_chroma_dc_av8:

    // stmfd sp!, {x4-x12, x14}    //stack stores the values of the arguments
    push_v_regs
    stp         x19, x20,[sp,#-16]!

    mov         x9, #0
    mov         v17.s[0], w9
    mov         v17.s[1], w9

    clz         w5,w4                       //counts leading zeros

    add         x6, x0, x4,lsl #1           //&src[2nt]
    mov         v18.s[0], w9
    mov         v18.s[1], w9
    sub         x20, x5, #32                //log2nt
    neg         x5, x20
    add         x7, x0, x4, lsl #2          //&src[4nt]
    mov         x12,x5
    add         x8, x7, #2                  //&src[4nt+2]

    cmp         x4, #4
    beq         dc_4                        //nt=4 loop


add_loop:
    ld2         {v30.8b, v31.8b}, [x6], #16 //load from src[nt]
    lsl         x10,x4,#1                   //2nt

    uaddlp      v2.4h,  v30.8b
    subs        x10, x10,#0x10

    ld2         {v26.8b, v27.8b}, [x8],#16  //load from src[2nt+1]

    uaddlp      v3.4h,  v31.8b
    uaddlp      v2.2s,  v2.4h
    uaddlp      v3.2s,  v3.4h

    uadalp      v17.1d,  v2.2s

    uadalp      v18.1d,  v3.2s

    uaddlp      v2.4h,  v26.8b
    uaddlp      v3.4h,  v27.8b

    uaddlp      v2.2s,  v2.4h
    uaddlp      v3.2s,  v3.4h

    uadalp      v17.1d,  v2.2s
    uadalp      v18.1d,  v3.2s

    beq         epil_add_loop

core_loop_add:
    ld2         {v30.8b, v31.8b}, [x6],#16  //load from src[nt]
    uaddlp      v28.4h,  v30.8b
    uaddlp      v3.4h,  v31.8b

    ld2         {v26.8b, v27.8b}, [x8],#16  //load from src[2nt+1]

    uaddlp      v3.2s,  v3.4h
    uaddlp      v29.2s,  v28.4h

    uadalp      v18.1d,  v3.2s
    uadalp      v17.1d,  v29.2s

    uaddlp      v3.4h,  v27.8b
    uaddlp      v28.4h,  v26.8b

    uaddlp      v3.2s,  v3.4h
    uaddlp      v29.2s,  v28.4h

    uadalp      v18.1d,  v3.2s
    uadalp      v17.1d,  v29.2s


epil_add_loop:

    smov        x1, v18.s[0]
    smov        x11, v17.s[0]

    add         x1,x1,x4
    add         x11,x11,x4

    lsr         x1,x1,x12
    lsr         x11,x11,x12

    dup         v17.8b,w1
    dup         v16.8b,w11

prologue_cpy_32:

    add         x5, x2, x3
    subs        x9, x4, #8
    lsl         x6, x3, #2
    csel        x11, x6, x11,eq
    add         x8, x5, x3
    add         x10, x8, x3

    beq         epilogue_copy

    st2         {v16.8b, v17.8b}, [x2],#16
    sub         x6, x6, #16

    st2         {v16.8b, v17.8b}, [x5],#16
    st2         {v16.8b, v17.8b}, [x8],#16
    mov         x20,#16
    csel        x11, x20, x11,ne
    st2         {v16.8b, v17.8b}, [x10],#16


    st2         {v16.8b, v17.8b}, [x2], x6
    st2         {v16.8b, v17.8b}, [x5], x6
    st2         {v16.8b, v17.8b}, [x8], x6
    st2         {v16.8b, v17.8b}, [x10], x6

kernel_copy:
    st2         {v16.8b, v17.8b}, [x2],#16
    st2         {v16.8b, v17.8b}, [x5],#16
    st2         {v16.8b, v17.8b}, [x8],#16
    st2         {v16.8b, v17.8b}, [x10],#16

    st2         {v16.8b, v17.8b}, [x2], x6
    st2         {v16.8b, v17.8b}, [x5], x6
    st2         {v16.8b, v17.8b}, [x8], x6
    st2         {v16.8b, v17.8b}, [x10], x6

    st2         {v16.8b, v17.8b}, [x2],#16
    st2         {v16.8b, v17.8b}, [x5],#16
    st2         {v16.8b, v17.8b}, [x8],#16
    st2         {v16.8b, v17.8b}, [x10],#16

    st2         {v16.8b, v17.8b}, [x2], x6
    st2         {v16.8b, v17.8b}, [x5], x6
    st2         {v16.8b, v17.8b}, [x8], x6
    st2         {v16.8b, v17.8b}, [x10], x6

epilogue_copy:
    st2         {v16.8b, v17.8b}, [x2],x11
    st2         {v16.8b, v17.8b}, [x5],x11
    st2         {v16.8b, v17.8b}, [x8],x11
    st2         {v16.8b, v17.8b}, [x10],x11

    st2         {v16.8b, v17.8b}, [x2]
    st2         {v16.8b, v17.8b}, [x5]
    st2         {v16.8b, v17.8b}, [x8]
    st2         {v16.8b, v17.8b}, [x10]
    b           end_func

dc_4:
    ld2         {v30.8b, v31.8b},[x6]       //load from src[nt]
    shl         d3, d30,#32

    ld2         {v26.8b, v27.8b},[x8]       //load from src[2nt+1]
    shl         d2, d31,#32

    uaddlp      v3.4h,  v3.8b
    uaddlp      v2.4h,  v2.8b
    uaddlp      v3.2s,  v3.4h
    uaddlp      v2.2s,  v2.4h
    uadalp      v17.1d,  v3.2s
    uadalp      v18.1d,  v2.2s

    shl         d3, d26,#32
    shl         d2, d27,#32
    uaddlp      v3.4h,  v3.8b
    uaddlp      v2.4h,  v2.8b
    uaddlp      v3.2s,  v3.4h
    uaddlp      v2.2s,  v2.4h
    uadalp      v17.1d,  v3.2s
    uadalp      v18.1d,  v2.2s

    smov        x10, v17.s[0]
    smov        x11, v18.s[0]

    add         x10,x10,x4
    add         x11,x11,x4
    lsr         x10,x10,x12
    lsr         x11,x11,x12
    orr         x10,x10,x11,lsl #8
    dup         v0.4h,w10

    st1         {v0.8b},[x2],x3
    st1         {v0.8b},[x2],x3
    st1         {v0.8b},[x2],x3
    st1         {v0.8b},[x2]

end_func:
    // ldmfd sp!,{x4-x12,x15}     //reload the registers from sp
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret




