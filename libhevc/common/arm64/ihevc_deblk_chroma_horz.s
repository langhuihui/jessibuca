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
///*******************************************************************************
//* @file
//*  ihevc_deblk_luma_horz.s
//*
//* @brief
//*  contains function definitions for inter prediction  interpolation.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* @author
//*  anand s
//*
//* @par list of functions:
//*
//*
//* @remarks
//*  none
//*
//void ihevc_deblk_chroma_horz(UWORD8 *pu1_src,
//                             WORD32 src_strd,
//                             WORD32 quant_param_p,
//                             WORD32 quant_param_q,
//                             WORD32 qp_offset_u,
//                             WORD32 qp_offset_v,
//                             WORD32 tc_offset_div2,
//                             WORD32 filter_flag_p,
//                             WORD32 filter_flag_q)
//

.text
.align 4
.include "ihevc_neon_macros.s"



.extern gai4_ihevc_qp_table
.extern gai4_ihevc_tc_table
.globl ihevc_deblk_chroma_horz_av8

.type ihevc_deblk_chroma_horz_av8, %function

ihevc_deblk_chroma_horz_av8:
    sxtw        x4,w4
    sxtw        x5,w5
    sxtw        x6,w6
    ldr         w9, [sp]
    sxtw        x9,w9
    push_v_regs
    stp         x19, x20,[sp,#-16]!
    mov         x10, x4
    mov         x8, x7
    mov         x7, x5
    mov         x4, x6

    sub         x12,x0,x1
    ld1         {v0.8b},[x0]
    sub         x5,x12,x1
    add         x6,x0,x1
    add         x1,x2,x3
    uxtl        v0.8h, v0.8b
    ld1         {v2.8b},[x12]
    add         x2,x1,#1
    ld1         {v4.8b},[x5]
    ld1         {v16.8b},[x6]
    adds        x1,x10,x2,asr #1
    uxtl        v2.8h, v2.8b
    adrp        x3, :got:gai4_ihevc_qp_table
    ldr         x3, [x3, #:got_lo12:gai4_ihevc_qp_table]
    bmi         l1.3312
    cmp         x1,#0x39
    bgt         lbl78
    ldr         w1, [x3,x1,lsl #2]
lbl78:
    sub         x20,x1,#6
    csel        x1, x20, x1,gt
l1.3312:
    adds        x2,x7,x2,asr #1
    uxtl        v4.8h, v4.8b
    bmi         l1.3332
    cmp         x2,#0x39
    bgt         lbl85
    ldr         w2, [x3,x2,lsl #2]
lbl85:
    sub         x20,x2,#6
    csel        x2, x20, x2,gt
l1.3332:
    add         x1,x1,x4,lsl #1
    sub         v6.8h,  v0.8h ,  v2.8h
    add         x3,x1,#2
    cmp         x3,#0x35
    mov         x20,#0x35
    csel        x1, x20, x1,gt
    shl         v6.8h, v6.8h,#2
    uxtl        v16.8h, v16.8b
    bgt         l1.3368
    adds        x3,x1,#2
    add         x20,x1,#2
    csel        x1, x20, x1,pl
    mov         x20,#0
    csel        x1, x20, x1,mi
l1.3368:
    adrp        x3, :got:gai4_ihevc_tc_table
    ldr         x3, [x3, #:got_lo12:gai4_ihevc_tc_table]
    add         v4.8h,  v6.8h ,  v4.8h
    add         x2,x2,x4,lsl #1
    sub         v6.8h,  v4.8h ,  v16.8h
    add         x4,x2,#2
    ldr         w1, [x3,x1,lsl #2]
    cmp         x4,#0x35
    mov         x20,#0x35
    csel        x2, x20, x2,gt
    bgt         l1.3412
    adds        x4,x2,#2
    add         x20,x2,#2
    csel        x2, x20, x2,pl
    mov         x20,#0
    csel        x2, x20, x2,mi
l1.3412:


    ldr         w2, [x3,x2,lsl #2]
    cmp         x8,#0
    dup         v31.8h,w2
    dup         v30.8h,w1
    sub         x20,x1,#0
    neg         x1, x20
    srshr       v6.8h, v6.8h,#3
    dup         v28.8h,w1
    sub         x20,x2,#0
    neg         x1, x20
    zip1        v4.8h, v30.8h, v31.8h
    dup         v29.8h,w1

    zip1        v18.8h, v28.8h, v29.8h

    smin        v16.8h,  v6.8h ,  v4.8h
    smax        v4.8h,  v18.8h ,  v16.8h
    add         v2.8h,  v2.8h ,  v4.8h
    sub         v0.8h,  v0.8h ,  v4.8h
    sqxtun      v2.8b, v2.8h
    sqxtun      v0.8b, v0.8h
    beq         l1.3528
    st1         {v2.8b},[x12]
l1.3528:
    cmp         x9,#0
    beq         l1.3540
    st1         {v0.8b},[x0]
l1.3540:
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret




