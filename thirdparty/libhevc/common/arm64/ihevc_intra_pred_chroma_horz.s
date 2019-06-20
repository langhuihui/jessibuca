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
//*  ihevc_intra_pred_chroma_horz_neon.s
//*
//* @brief
//*  contains function definition for intra prediction  interpolation filters
//*
//*
//* @author
//*  parthiban v
//*
//* @par list of functions:
//*  - ihevc_intra_pred_luma_horz()
//*
//* @remarks
//*  none
//*
//*******************************************************************************
//*/
//
///**
//*******************************************************************************
//*
//* @brief
//*     intra prediction interpolation filter for horizontal luma variable.
//*
//* @par description:
//*      horizontal intraprediction(mode 10) with.extern  samples location
//*      pointed by 'pu1_ref' to the tu block  location pointed by 'pu1_dst'  refer
//*      to section 8.4.4.2.6 in the standard (special case)
//*
//* @param[in] pu1_src
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
//*  integer transform block size
//*
//* @param[in] mode
//*  integer intraprediction mode
//*
//* @returns
//*
//* @remarks
//*  none
//*
//*******************************************************************************
//*/
//void ihevc_intra_pred_chroma_horz(uword8 *pu1_ref,
//                                  word32 src_strd,
//                                  uword8 *pu1_dst,
//                                  word32 dst_strd,
//                                  word32 nt,
//                                  word32 mode)
//**************variables vs registers*****************************************
//x0 => *pu1_ref
//x1 =>  src_strd
//x2 => *pu1_dst
//x3 =>  dst_strd

.text
.align 4
.include "ihevc_neon_macros.s"


.globl ihevc_intra_pred_chroma_horz_av8

.type ihevc_intra_pred_chroma_horz_av8, %function

ihevc_intra_pred_chroma_horz_av8:

    // stmfd sp!, {x4-x12, x14}                //stack stores the values of the arguments

    stp         x19, x20,[sp,#-16]!

    lsl         x6,x4,#2                    //four_nt

    add         x12,x0,x6                   //*pu1_ref[four_nt]
    cmp         x4,#4                       //if nt == 4
    beq         core_loop_4

    cmp         x4,#8                       //if nt == 8
    beq         core_loop_8

    //cmp            x4,#16                            @if nt == 16
    //beq            core_loop_16

    sub         x12,x12,#16                 //move to 16th value pointer
    add         x9,x2,#16

core_loop_16:
    ld1         { v0.8h},[x12]              //load 16 values. d1[7] will have the 1st value.
    sub         x12,x12,#16
    ld1         { v18.8h},[x12]             //load 16 values. d1[7] will have the 1st value.

    dup         v2.8h, v0.h[7]              //duplicate the i value.

    dup         v4.8h, v0.h[6]              //duplicate the ii value.
    dup         v6.8h, v0.h[5]              //duplicate the iii value.
    st1         { v2.8h},[x2],x3            //store in 1st row 0-16 columns
    st1         { v2.8h},[x9],x3            //store in 1st row 16-32 columns

    dup         v1.8h, v0.h[4]
    st1         { v4.8h},[x2],x3
    st1         { v4.8h},[x9],x3

    dup         v2.8h, v0.h[3]
    st1         { v6.8h},[x2],x3
    st1         { v6.8h},[x9],x3

    dup         v4.8h, v0.h[2]
    st1         { v1.8h},[x2],x3
    st1         { v1.8h},[x9],x3

    dup         v6.8h, v0.h[1]
    st1         { v2.8h},[x2],x3
    st1         { v2.8h},[x9],x3

    dup         v1.8h, v0.h[0]
    st1         { v4.8h},[x2],x3
    st1         { v4.8h},[x9],x3

    dup         v2.8h, v18.h[7]
    st1         { v6.8h},[x2],x3
    st1         { v6.8h},[x9],x3

    dup         v4.8h, v18.h[6]
    st1         { v1.8h},[x2],x3
    st1         { v1.8h},[x9],x3

    dup         v6.8h, v18.h[5]
    st1         { v2.8h},[x2],x3
    st1         { v2.8h},[x9],x3

    dup         v1.8h, v18.h[4]
    st1         { v4.8h},[x2],x3
    st1         { v4.8h},[x9],x3

    dup         v2.8h, v18.h[3]
    st1         { v6.8h},[x2],x3
    st1         { v6.8h},[x9],x3

    dup         v4.8h, v18.h[2]
    st1         { v1.8h},[x2],x3
    st1         { v1.8h},[x9],x3

    dup         v6.8h, v18.h[1]
    st1         { v2.8h},[x2],x3
    st1         { v2.8h},[x9],x3
    sub         x12,x12,#16                 //move to 16th value pointer

    dup         v1.8h, v18.h[0]
    st1         { v4.8h},[x2],x3
    st1         { v4.8h},[x9],x3

    subs        x4,x4,#16                   //decrement the loop count by 16
    st1         { v6.8h},[x2],x3
    st1         { v6.8h},[x9],x3

    st1         { v1.8h},[x2],x3
    st1         { v1.8h},[x9],x3
    bgt         core_loop_16
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret
    b           endloop

core_loop_8:
    ldrb        w14,[x12],#1                //pu1_ref[two_nt]
    sxtw        x14,w14
    //vld1.8        {q15},[x12]                        @pu1_ref[two_nt + 1 + col]

    dup         v28.8b,w14
    sub         x12,x12,#17
    ld1         { v0.16b},[x12]

    sub         x12,x12,#16
//    ld1 { v30.16b},[x12]
    dup         v18.8h, v0.h[7]
    //vmovl.u8    q13,d26

    dup         v2.8h, v0.h[6]
    //vsubl.u8    q12,d30,d28

    dup         v4.8h, v0.h[5]
    //vshr.s16    q12,q12,#1

    dup         v6.8h, v0.h[4]
    //vqadd.s16    q11,q13,q12

    dup         v1.8h, v0.h[3]
    //vqmovun.s16 d22,q11

    st1         { v18.8h},[x2],x3

    dup         v18.8h, v0.h[2]
    //vsubl.u8    q12,d31,d28

    dup         v19.8h, v0.h[1]
    //vshr.s16    q12,q12,#1

    dup         v20.8h, v0.h[0]
    //vqadd.s16    q11,q13,q12

    dup         v16.8h, v0.h[3]
    //vqmovun.s16 d22,q11

    st1         { v2.8h},[x2],x3
    //sub            x2,x2,#8

    st1         { v4.8h},[x2],x3

    st1         { v6.8h},[x2],x3
    st1         { v1.8h},[x2],x3
    st1         { v18.8h},[x2],x3

    //vdup.8        q1,d0[2]
    st1         { v19.8h},[x2],x3

    //vdup.8        q2,d0[1]
    st1         { v20.8h},[x2],x3

    //vdup.8        q3,d0[0]
    //vst1.8        {q7},[x2],x3

    //vdup.8        q4,d0[3]
    //vst1.8        {q8},[x2],x3

    //vdup.8        q5,d0[2]
    //vst1.8        {q1},[x2],x3

    //vdup.8        q6,d0[1]
    //vst1.8        {q2},[x2],x3

    //vdup.8        q7,d0[0]
    //vst1.8        {q3},[x2],x3

    //vst1.8        {q4},[x2],x3
    //vst1.8        {q5},[x2],x3
    //vst1.8        {q6},[x2],x3
    //vst1.8        {q7},[x2],x3

    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret
    b           endloop


core_loop_4:
    ldrb        w14,[x12]                   //pu1_ref[two_nt]
    sxtw        x14,w14
    add         x12,x12,#1                  //pu1_ref[two_nt + 1]
    //vld1.8        {d30},[x12]                        @pu1_ref[two_nt + 1 + col]

    sub         x12,x12,#9
    ld1         {v0.8b},[x12]
    sub         x12,x12,#8
    ld1         {v30.8b},[x12]
    dup         v26.4h, v0.h[3]
    dup         v28.8b,w14

    dup         v3.4h, v0.h[2]
    uxtl        v26.8h, v26.8b

    dup         v4.4h, v0.h[1]
    usubl       v24.8h, v30.8b, v28.8b

    dup         v5.4h, v0.h[0]
    sshr        v24.8h, v24.8h,#1

    dup         v6.4h, v0.h[3]
    sqadd       v22.8h,  v26.8h ,  v24.8h

    dup         v7.4h, v0.h[2]
    sqxtun      v22.8b, v22.8h

    st1         {v6.8b},[x2],x3
    st1         {v3.8b},[x2],x3

    dup         v1.4h, v0.h[1]
    st1         {v4.8b},[x2],x3
    st1         {v5.8b},[x2],x3

    dup         v17.4h, v0.h[0]
    //vst1.8        {d6},[x2],x3
    //vst1.8        {d7},[x2],x3

    //vst1.8        {d8},[x2],x3
    //vst1.8        {d9},[x2],x3
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret
    b           endloop


//core_loop_4
    ldrb        w14,[x12]                   //pu1_ref[two_nt]
    sxtw        x14,w14
    add         x12,x12,#1                  //pu1_ref[two_nt + 1]
    ld1         {v30.8b},[x12]              //pu1_ref[two_nt + 1 + col]

    sub         x12,x12,#5
    ld1         {v0.8b},[x12]
    dup         v28.8b,w14
    dup         v26.8b, v0.b[3]
    uxtl        v26.8h, v26.8b

    dup         v3.8b, v0.b[2]
    usubl       v24.8h, v30.8b, v28.8b

    dup         v4.8b, v0.b[1]
    sshr        v24.8h, v24.8h,#1

    dup         v5.8b, v0.b[0]
    sqadd       v22.8h,  v26.8h ,  v24.8h

    sqxtun      v22.8b, v22.8h

    st1         {v22.s}[0],[x2],x3
    st1         {v3.s}[0],[x2],x3
    st1         {v4.s}[0],[x2],x3
    st1         {v5.s}[0],[x2],x3

    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret

endloop:



