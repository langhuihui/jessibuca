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
//*  ihevc_intra_pred_filters_vert.s
//*
//* @brief
//*  contains function definitions for intra prediction dc filtering.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* @author
//*  akshaya mukund
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

//void ihevc_intra_pred_luma_ver(uword8* pu1_ref,
//                               word32 src_strd,
//                               uword8* pu1_dst,
//                               word32 dst_strd,
//                               word32 nt,
//                               word32 mode)
//
//**************variables vs registers*****************************************
//x0 => *pu1_ref
//x1 => src_strd
//x2 => *pu1_dst
//x3 => dst_strd

//stack contents from #40
//    nt
//    mode

.text
.align 4
.include "ihevc_neon_macros.s"



.globl ihevc_intra_pred_luma_ver_av8

.type ihevc_intra_pred_luma_ver_av8, %function

ihevc_intra_pred_luma_ver_av8:

    // stmfd sp!, {x4-x12, x14}            //stack stores the values of the arguments

    stp         x19, x20,[sp,#-16]!

    lsl         x5, x4, #1                  //2nt

    cmp         x4, #16
    beq         blk_16
    blt         blk_4_8

    add         x5, x5, #1                  //2nt+1
    add         x6, x0, x5                  //&src[2nt+1]

copy_32:
    add         x5, x2, x3
    ld1         {v20.8b, v21.8b}, [x6],#16  //16 loads (col 0:15)
    add         x8, x5, x3

    add         x10, x8, x3
    ld1         {v22.8b, v23.8b}, [x6]      //16 loads (col 16:31)
    lsl         x11, x3, #2

    sub         x11, x11, #16
    st1         {v20.8b, v21.8b}, [x2],#16
    st1         {v20.8b, v21.8b}, [x5],#16
    st1         {v20.8b, v21.8b}, [x8],#16
    st1         {v20.8b, v21.8b}, [x10],#16

    st1         {v22.8b, v23.8b}, [x2], x11
    st1         {v22.8b, v23.8b}, [x5], x11
    st1         {v22.8b, v23.8b}, [x8], x11
    st1         {v22.8b, v23.8b}, [x10], x11

    subs        x4, x4, #8

kernel_copy_32:
    st1         {v20.8b, v21.8b}, [x2],#16
    st1         {v20.8b, v21.8b}, [x5],#16
    st1         {v20.8b, v21.8b}, [x8],#16
    st1         {v20.8b, v21.8b}, [x10],#16

    st1         {v22.8b, v23.8b}, [x2], x11
    st1         {v22.8b, v23.8b}, [x5], x11
    st1         {v22.8b, v23.8b}, [x8], x11
    st1         {v22.8b, v23.8b}, [x10], x11

    subs        x4, x4, #8

    st1         {v20.8b, v21.8b}, [x2],#16
    st1         {v20.8b, v21.8b}, [x5],#16
    st1         {v20.8b, v21.8b}, [x8],#16
    st1         {v20.8b, v21.8b}, [x10],#16

    st1         {v22.8b, v23.8b}, [x2], x11
    st1         {v22.8b, v23.8b}, [x5], x11
    st1         {v22.8b, v23.8b}, [x8], x11
    st1         {v22.8b, v23.8b}, [x10], x11

    bne         kernel_copy_32

    st1         {v20.8b, v21.8b}, [x2],#16
    st1         {v20.8b, v21.8b}, [x5],#16
    st1         {v20.8b, v21.8b}, [x8],#16
    st1         {v20.8b, v21.8b}, [x10],#16

    st1         {v22.8b, v23.8b}, [x2], x11
    st1         {v22.8b, v23.8b}, [x5], x11
    st1         {v22.8b, v23.8b}, [x8], x11
    st1         {v22.8b, v23.8b}, [x10], x11

    b           end_func

blk_16:
    add         x6, x0, x5                  //&src[2nt]

    ldrb        w11, [x6], #1               //src[2nt]
    sxtw        x11,w11

    dup         v22.16b,w11                 //src[2nt]
    ldrb        w12, [x6]                   //src[2nt+1]
    sxtw        x12,w12

    ld1         {v16.8b, v17.8b}, [x6]      //ld for repl to cols src[2nt+1+col(0:15)] (0 ignored for stores)
    sub         x6, x6, #17                 //subtract -9 to take it to src[2nt-1-row(15)]

    dup         v24.16b,w12                 //src[2nt+1]
    dup         v30.8h,w12
    lsl         x5, x3, #3                  //8*stride

    ld1         {v26.16b}, [x6],#16         //load src[2nt-1-row](rows 0:15)
    add         x5, x2, x5                  //x5 ->

    movi        d18, #0x00000000000000ff
    uhsub       v26.16b,  v26.16b ,  v22.16b //(src[2nt-1-row] - src[2nt])>>1
    //vsubl.u8    q0, d26, d22
    //vsubl.u8    q14, d27, d22

    //vshr.s16    q0, q0, #1
    //vshr.s16    q14, q14, #1

    mov         v19.d[0],v17.d[0]
    //vaddl.s8    q0, d24, d26
    sxtl        v0.8h, v26.8b
    sxtl2       v28.8h, v26.16b
    sqadd       v0.8h,  v0.8h ,  v30.8h
    sqadd       v28.8h,  v28.8h ,  v30.8h

    movi        d3, #0x00000000000000ff
    //vaddl.s8    q1, d25, d27

    sqxtun      v24.8b, v28.8h
    sqxtun2     v24.16b, v0.8h
    //vmovn.u16    d25, q0
    //vmovn.u16    d24, q1

    rev64       v24.16b,  v24.16b
    mov         v25.d[0], v24.d[1]

    mov         v4.d[0],v17.d[0]

    bsl         v18.8b,  v24.8b ,  v16.8b   //only select row values from q12(predpixel)
    bsl         v3.8b,  v25.8b ,  v16.8b

    movi        d1, #0x00000000000000ff
    mov         v2.d[0],v17.d[0]

    movi        d6, #0x00000000000000ff
    mov         v7.d[0],v17.d[0]

    st1         {v18.8b, v19.8b}, [x2], x3
    sshr        d24, d24,#8

    st1         {v3.8b, v4.8b}, [x5], x3
    sshr        d25, d25,#8


    bsl         v1.8b,  v24.8b ,  v16.8b
    bsl         v6.8b,  v25.8b ,  v16.8b

    st1         {v1.8b, v2.8b}, [x2], x3
    sshr        d24, d24,#8

    st1         {v6.8b, v7.8b}, [x5], x3
    sshr        d25, d25,#8

    subs        x4, x4,#8

    movi        d18, #0x00000000000000ff
    //vmov.i64    d19, d17

    movi        d3, #0x00000000000000ff
    //vmov.i64    d11, d17


loop_16:


    movi        d1, #0x00000000000000ff

    movi        d6, #0x00000000000000ff

    bsl         v18.8b,  v24.8b ,  v16.8b   //only select row values from q12(predpixel)
    bsl         v3.8b,  v25.8b ,  v16.8b

    st1         {v18.8b, v19.8b}, [x2], x3
    sshr        d24, d24,#8

    st1         {v3.8b, v4.8b}, [x5], x3
    sshr        d25, d25,#8

    movi        d18, #0x00000000000000ff

    movi        d3, #0x00000000000000ff

    bsl         v1.8b,  v24.8b ,  v16.8b
    bsl         v6.8b,  v25.8b ,  v16.8b

    st1         {v1.8b, v2.8b}, [x2], x3
    sshr        d24, d24,#8

    st1         {v6.8b, v7.8b}, [x5], x3
    sshr        d25, d25,#8

    subs        x4, x4, #4

    bne         loop_16

    movi        d1, #0x00000000000000ff

    movi        d6, #0x00000000000000ff

    bsl         v18.8b,  v24.8b ,  v16.8b   //only select row values from q12(predpixel)
    bsl         v3.8b,  v25.8b ,  v16.8b

    st1         {v18.8b, v19.8b}, [x2], x3
    sshr        d24, d24,#8

    st1         {v3.8b, v4.8b}, [x5], x3
    sshr        d25, d25,#8

    bsl         v1.8b,  v24.8b ,  v16.8b
    bsl         v6.8b,  v25.8b ,  v16.8b

    st1         {v1.8b, v2.8b}, [x2], x3

    st1         {v6.8b, v7.8b}, [x5], x3

    b           end_func


blk_4_8:
    movi        d4, #0x00000000000000ff
    add         x6, x0, x5                  //&src[2nt]

    movi        d3, #0x00000000000000ff
    ldrb        w11, [x6], #1               //src[2nt]
    sxtw        x11,w11

    dup         v22.8b,w11                  //src[2nt]
    ldrb        w12, [x6]                   //src[2nt+1]
    sxtw        x12,w12

    ld1         {v16.8b},[x6]               //ld for repl to cols src[2nt+1+col(0:3 or 0:7)](0 ignored for st)
    sub         x6, x6, #9                  //subtract -9 to take it to src[2nt-1-row(15)]

    dup         v24.8b,w12                  //src[2nt+1]
    dup         v30.8h,w12

    ld1         {v26.8b},[x6],#8            //load src[2nt-1-row](rows 0:15)

    movi        d18, #0x00000000000000ff
    uhsub       v26.8b,  v26.8b ,  v22.8b   //(src[2nt-1-row] - src[2nt])>>1
    //vsubl.u8    q13, d26, d22

    //vshr.s16    q13, q13, #1

    movi        d19, #0x00000000000000ff
    sxtl        v26.8h, v26.8b
    //vaddl.s8    q0, d24, d26
    sqadd       v0.8h,  v26.8h ,  v30.8h

    sqxtun      v24.8b, v0.8h
    //vmovn.s16    d24, q0

    rev64       v24.8b,  v24.8b

    cmp         x4, #4
    beq         blk_4

    bsl         v18.8b,  v24.8b ,  v16.8b   //only select row values from q12(predpixel)

    st1         {v18.8b},[x2], x3
    sshr        d24, d24,#8

    movi        d18, #0x00000000000000ff

    bsl         v19.8b,  v24.8b ,  v16.8b

    st1         {v19.8b},[x2], x3
    sshr        d24, d24,#8

    movi        d19, #0x00000000000000ff

    bsl         v3.8b,  v24.8b ,  v16.8b

    st1         {v3.8b},[x2], x3
    sshr        d24, d24,#8

    movi        d3, #0x00000000000000ff

    bsl         v4.8b,  v24.8b ,  v16.8b

    st1         {v4.8b},[x2], x3
    sshr        d24, d24,#8

    movi        d4, #0x00000000000000ff

    bsl         v18.8b,  v24.8b ,  v16.8b   //only select row values from q12(predpixel)

    st1         {v18.8b},[x2], x3
    sshr        d24, d24,#8

    bsl         v19.8b,  v24.8b ,  v16.8b

    st1         {v19.8b},[x2], x3
    sshr        d24, d24,#8

    bsl         v3.8b,  v24.8b ,  v16.8b

    st1         {v3.8b},[x2], x3
    sshr        d24, d24,#8

    bsl         v4.8b,  v24.8b ,  v16.8b

    st1         {v4.8b},[x2], x3
    sshr        d24, d24,#8

    b           end_func


blk_4:
    bsl         v18.8b,  v24.8b ,  v16.8b   //only select row values from q12(predpixel)

    st1         {v18.s}[0],[x2], x3
    sshr        d24, d24,#8

    bsl         v19.8b,  v24.8b ,  v16.8b

    st1         {v19.s}[0],[x2], x3
    sshr        d24, d24,#8

    bsl         v3.8b,  v24.8b ,  v16.8b

    st1         {v3.s}[0],[x2], x3
    sshr        d24, d24,#8

    bsl         v4.8b,  v24.8b ,  v16.8b
    st1         {v4.s}[0],[x2], x3


end_func:
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret





