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
//* //file
//*  ihevc_inter_pred_chroma_vert_neon_w16inp_w16out_neon.s
//*
//* //brief
//*  contains function definitions for inter prediction  interpolation.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* //author
//*  yogeswaran rs / parthiban
//*
//* //par list of functions:
//*
//*
//* //remarks
//*  none
//*
//*******************************************************************************
//*/
///**
///**
//*******************************************************************************
//*
//* //brief
//*    chroma interprediction filter for 16bit vertical input and output.
//*
//* //par description:
//*    applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
//*    the elements pointed by 'pu1_src' and  writes to the location pointed by
//*    'pu1_dst'  input is 16 bits  the filter output is downshifted by 6 and
//*    8192 is  subtracted to store it as a 16 bit number  the output is used as
//*    a input to weighted prediction   assumptions : the function is optimized
//*    considering the fact width and  height are multiple of 2.
//*
//* //param[in] pi2_src
//*  word16 pointer to the source
//*
//* //param[out] pi2_dst
//*  word16 pointer to the destination
//*
//* //param[in] src_strd
//*  integer source stride
//*
//* //param[in] dst_strd
//*  integer destination stride
//*
//* //param[in] pi1_coeff
//*  word8 pointer to the filter coefficients
//*
//* //param[in] ht
//*  integer height of the array
//*
//* //param[in] wd
//*  integer width of the array
//*
//* //returns
//*
//* //remarks
//*  none
//*
//*******************************************************************************
//*/
//void ihevc_inter_pred_chroma_vert_w16inp_w16out(word16 *pi2_src,
//                                                 word16 *pi2_dst,
//                                                 word32 src_strd,
//                                                 word32 dst_strd,
//                                                 word8 *pi1_coeff,
//                                                 word32 ht,
//                                                 word32 wd)
//**************variables vs registers*****************************************
//x0 => *pu1_src
//x1 => *pi2_dst
//x2 =>  src_strd
//x3 =>  dst_strd
.text
.align 4

.include "ihevc_neon_macros.s"

.globl ihevc_inter_pred_chroma_vert_w16inp_w16out_av8

.type ihevc_inter_pred_chroma_vert_w16inp_w16out_av8, %function

ihevc_inter_pred_chroma_vert_w16inp_w16out_av8:

    // stmfd sp!, {x4-x12, x14}                    //stack stores the values of the arguments

    stp         x19, x20,[sp,#-16]!

    mov         x15,x4 // pi1_coeff
    mov         x16,x5 // ht
    mov         x17,x6 // wd

    mov         x4, x15                     //loads pi1_coeff
    mov         x6, x17                     //wd
    lsl         x2,x2,#1                    //src_strd = 2* src_strd
    mov         x5,x16                      //loads ht
    ld1         {v0.8b},[x4]                //loads pi1_coeff
    sub         x4,x0,x2                    //pu1_src - src_strd
    sxtl        v0.8h, v0.8b                //long the value

    tst         x6,#3                       //checks wd  == 2
    dup         v16.4h, v0.h[0]             //coeff_0
    dup         v17.4h, v0.h[1]             //coeff_1
    dup         v18.4h, v0.h[2]             //coeff_2
    dup         v19.4h, v0.h[3]             //coeff_3

    bgt         core_loop_ht_2              //jumps to loop handles wd 2

    tst         x5,#3                       //checks ht == mul of 4
    beq         core_loop_ht_4              //jumps to loop handles ht mul of 4

core_loop_ht_2:
    lsl         x7,x2,#1                    //2*src_strd
    lsl         x3,x3,#1                    //2*dst_strd
    lsl         x9,x6,#2                    //4*wd
    sub         x6,x3,x6,lsl #1             //2*dst_strd - 2*wd
    sub         x8,x7,x9                    //2*src_strd - 4*wd
    mov         x12,x9                      //4wd

inner_loop_ht_2:
    add         x0,x4,x2                    //increments pi2_src
    ld1         {v0.4h},[x4],#8             //loads pu1_src
    smull       v0.4s, v0.4h, v16.4h        //vmull_s16(src_tmp1, coeff_0)
    subs        x12,x12,#8                  //2wd + 8
    ld1         {v2.4h},[x0],x2             //loads pi2_src
    smull       v7.4s, v2.4h, v16.4h        //vmull_s16(src_tmp2, coeff_0)
    ld1         {v3.4h},[x0],x2             //loads pi2_src
    smlal       v0.4s, v2.4h, v17.4h
    ld1         {v6.4h},[x0],x2
    smlal       v7.4s, v3.4h, v17.4h
    ld1         {v2.4h},[x0]
    add         x7,x1,x3                    //pu1_dst + dst_strd
    smlal       v0.4s, v3.4h, v18.4h
    smlal       v7.4s, v6.4h, v18.4h
    smlal       v0.4s, v6.4h, v19.4h
    smlal       v7.4s, v2.4h, v19.4h
    sqshrn      v0.4h, v0.4s,#6             //right shift
    sqshrn      v30.4h, v7.4s,#6            //right shift
    st1         {v0.2s},[x1],#8             //stores the loaded value
    st1         {v30.2s},[x7]               //stores the loaded value
    bgt         inner_loop_ht_2             //inner loop -again

    //inner loop ends
    subs        x5,x5,#2                    //increments ht
    add         x1,x1,x6,lsl #1             //pu1_dst += 2*dst_strd - 2*wd
    mov         x12,x9                      //4wd
    add         x4,x4,x8                    //pi1_src_tmp1 += 2*src_strd - 4*wd
    bgt         inner_loop_ht_2             //loop again

    b           end_loops                   //jumps to end

core_loop_ht_4:
    lsl         x7,x2,#2                    //2*src_strd
    lsl         x10,x3,#2                   //2*dst_strd
    lsr         x11, x6, #1                 //divide by 2
    sub         x14,x10,x6,lsl #1           //2*dst_strd - 2*wd
    sub         x8,x7,x6,lsl #2             //2*src_strd - 4*wd

    mul         x12, x5 , x11               //multiply height by width
    sub         x12, x12,#4                 //subtract by one for epilog
    lsl         x11, x6, #1                 //2*wd
    lsl         x3,x3,#1                    //2*dst_strd

prolog:
    add         x0,x4,x2                    //increments pi2_src
    ld1         {v0.4h},[x4],#8             //loads pu1_src
    ld1         {v1.4h},[x0],x2             //loads pi2_src
    subs        x11,x11,#4
    ld1         {v2.4h},[x0],x2             //loads pi2_src
    smull       v30.4s, v0.4h, v16.4h       //vmull_s16(src_tmp1, coeff_0)
    ld1         {v3.4h},[x0],x2
    smlal       v30.4s, v1.4h, v17.4h
    smlal       v30.4s, v2.4h, v18.4h
    add         x9,x1,x3                    //pu1_dst + dst_strd
    smlal       v30.4s, v3.4h, v19.4h

    ld1         {v4.4h},[x0],x2
    smull       v28.4s, v1.4h, v16.4h       //vmull_s16(src_tmp2, coeff_0)
    add         x20,x4,x8
    csel        x4, x20, x4,le
    lsl         x20,x6,#1
    csel        x11, x20, x11,le
    smlal       v28.4s, v2.4h, v17.4h
    smlal       v28.4s, v3.4h, v18.4h
    ld1         {v5.4h},[x0],x2
    smlal       v28.4s, v4.4h, v19.4h

    sqshrn      v30.4h, v30.4s,#6           //right shift

    ld1         {v6.4h},[x0],x2
    smull       v26.4s, v2.4h, v16.4h       //vmull_s16(src_tmp2, coeff_0)
    smlal       v26.4s, v3.4h, v17.4h
    smlal       v26.4s, v4.4h, v18.4h
    add         x0,x4,x2
    ld1         {v0.4h},[x4],#8             //loads pu1_src
    smlal       v26.4s, v5.4h, v19.4h

    sqshrn      v28.4h, v28.4s,#6           //right shift

    ld1         {v1.4h},[x0],x2             //loads pi2_src
    smull       v24.4s, v3.4h, v16.4h       //vmull_s16(src_tmp2, coeff_0)
    st1         {v30.2s},[x1],#8            //stores the loaded value
    smlal       v24.4s, v4.4h, v17.4h
    ld1         {v2.4h},[x0],x2             //loads pi2_src
    smlal       v24.4s, v5.4h, v18.4h
    ld1         {v3.4h},[x0],x2
    smlal       v24.4s, v6.4h, v19.4h
    add         x20,x1,x14,lsl #1
    csel        x1, x20, x1,le

    sqshrn      v26.4h, v26.4s,#6           //right shift
    subs        x12,x12,#4

    beq         epilog                      //jumps to epilog

kernel_4:
    smull       v30.4s, v0.4h, v16.4h       //vmull_s16(src_tmp1, coeff_0)
    subs        x11,x11,#4
    smlal       v30.4s, v1.4h, v17.4h
    st1         {v28.2s},[x9],x3            //stores the loaded value
    smlal       v30.4s, v2.4h, v18.4h
    smlal       v30.4s, v3.4h, v19.4h

    sqshrn      v24.4h, v24.4s,#6           //right shift

    ld1         {v4.4h},[x0],x2
    smull       v28.4s, v1.4h, v16.4h       //vmull_s16(src_tmp2, coeff_0)
    smlal       v28.4s, v2.4h, v17.4h
    smlal       v28.4s, v3.4h, v18.4h
    smlal       v28.4s, v4.4h, v19.4h
    st1         {v26.2s},[x9],x3            //stores the loaded value
    add         x20,x4,x8
    csel        x4, x20, x4,le
    lsl         x20,x6,#1
    csel        x11, x20, x11,le

    sqshrn      v30.4h, v30.4s,#6           //right shift

    ld1         {v5.4h},[x0],x2
    smull       v26.4s, v2.4h, v16.4h       //vmull_s16(src_tmp2, coeff_0)
    ld1         {v6.4h},[x0],x2
    smlal       v26.4s, v3.4h, v17.4h
    st1         {v24.2s},[x9]               //stores the loaded value
    add         x0,x4,x2
    smlal       v26.4s, v4.4h, v18.4h
    ld1         {v0.4h},[x4],#8             //loads pu1_src
    smlal       v26.4s, v5.4h, v19.4h

    sqshrn      v28.4h, v28.4s,#6           //right shift

    ld1         {v1.4h},[x0],x2             //loads pi2_src
    smull       v24.4s, v3.4h, v16.4h       //vmull_s16(src_tmp2, coeff_0)
    ld1         {v2.4h},[x0],x2             //loads pi2_src
    smlal       v24.4s, v4.4h, v17.4h
    add         x9,x1,x3                    //pu1_dst + dst_strd
    ld1         {v3.4h},[x0],x2
    smlal       v24.4s, v5.4h, v18.4h

    st1         {v30.2s},[x1],#8            //stores the loaded value
    smlal       v24.4s, v6.4h, v19.4h

    sqshrn      v26.4h, v26.4s,#6           //right shift
    add         x20,x1,x14,lsl #1
    csel        x1, x20, x1,le

    subs        x12,x12,#4

    bgt         kernel_4                    //jumps to kernel_4

epilog:
    smull       v30.4s, v0.4h, v16.4h       //vmull_s16(src_tmp1, coeff_0)
    st1         {v28.2s},[x9],x3            //stores the loaded value
    smlal       v30.4s, v1.4h, v17.4h
    smlal       v30.4s, v2.4h, v18.4h
    smlal       v30.4s, v3.4h, v19.4h

    sqshrn      v24.4h, v24.4s,#6           //right shift

    smull       v28.4s, v1.4h, v16.4h       //vmull_s16(src_tmp2, coeff_0)
    ld1         {v4.4h},[x0],x2
    smlal       v28.4s, v2.4h, v17.4h
    st1         {v26.2s},[x9],x3            //stores the loaded value
    smlal       v28.4s, v3.4h, v18.4h
    smlal       v28.4s, v4.4h, v19.4h

    sqshrn      v30.4h, v30.4s,#6           //right shift

    smull       v26.4s, v2.4h, v16.4h       //vmull_s16(src_tmp2, coeff_0)
    ld1         {v5.4h},[x0],x2
    smlal       v26.4s, v3.4h, v17.4h
    smlal       v26.4s, v4.4h, v18.4h
    smlal       v26.4s, v5.4h, v19.4h

    sqshrn      v28.4h, v28.4s,#6           //right shift

    st1         {v24.2s},[x9]               //stores the loaded value
    smull       v24.4s, v3.4h, v16.4h       //vmull_s16(src_tmp2, coeff_0)
    smlal       v24.4s, v4.4h, v17.4h
    add         x9,x1,x3                    //pu1_dst + dst_strd
    ld1         {v6.4h},[x0],x2
    smlal       v24.4s, v5.4h, v18.4h
    smlal       v24.4s, v6.4h, v19.4h
    st1         {v30.2s},[x1],#8            //stores the loaded value

    sqshrn      v26.4h, v26.4s,#6           //right shift

    st1         {v28.2s},[x9],x3            //stores the loaded value

    sqshrn      v24.4h, v24.4s,#6           //right shift
    st1         {v26.2s},[x9],x3            //stores the loaded value

    st1         {v24.2s},[x9]               //stores the loaded value

end_loops:
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret




