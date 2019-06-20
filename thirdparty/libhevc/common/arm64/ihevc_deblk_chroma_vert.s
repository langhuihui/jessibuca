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
///*******************************************************************************
//* @file
//*  ihevc_deblk_luma_vert.s
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
//*******************************************************************************/
//void ihevc_deblk_chroma_vert(UWORD8 *pu1_src,
//                             WORD32 src_strd,
//                             WORD32 quant_param_p,
//                             WORD32 quant_param_q,
//                             WORD32 qp_offset_u,
//                             WORD32 qp_offset_v,
//                             WORD32 tc_offset_div2,
//                             WORD32 filter_flag_p,
//                             WORD32 filter_flag_q)

.text
.align 4
.include "ihevc_neon_macros.s"



.extern gai4_ihevc_qp_table
.extern gai4_ihevc_tc_table
.globl ihevc_deblk_chroma_vert_av8

.type ihevc_deblk_chroma_vert_av8, %function

ihevc_deblk_chroma_vert_av8:
    sxtw        x4,w4
    sxtw        x5,w5
    sxtw        x6,w6
    mov         x15,x5
    mov         x5,x6
    mov         x6,x15
    mov         x12, x7
    mov         x7, x4
    ldr         w4, [sp]

    push_v_regs
    stp         x19, x20,[sp,#-16]!

    sub         x8,x0,#4
    add         x2,x2,x3
    ld1         {v5.8b},[x8],x1
    add         x2,x2,#1
    ld1         {v17.8b},[x8],x1
    ld1         {v16.8b},[x8],x1
    ld1         {v4.8b},[x8]

    trn1        v29.8b, v5.8b, v17.8b
    trn2        v17.8b, v5.8b, v17.8b
    mov         v5.d[0], v29.d[0]
    adds        x3,x7,x2,asr #1
    trn1        v29.8b, v16.8b, v4.8b
    trn2        v4.8b, v16.8b, v4.8b
    mov         v16.d[0], v29.d[0]
    adrp        x7, :got:gai4_ihevc_qp_table
    ldr         x7, [x7, #:got_lo12:gai4_ihevc_qp_table]


    bmi         l1.2944
    cmp         x3,#0x39
    bgt         lbl78
    ldr         w3, [x7,x3,lsl #2]
    sxtw        x3,w3
lbl78:
    sub         x20,x3,#6
    csel        x3, x20, x3,gt
l1.2944:
    trn1        v29.4h, v5.4h, v16.4h
    trn2        v16.4h, v5.4h, v16.4h
    mov         v5.d[0], v29.d[0]
    adds        x2,x6,x2,asr #1
    trn1        v29.4h, v17.4h, v4.4h
    trn2        v4.4h, v17.4h, v4.4h
    mov         v17.d[0], v29.d[0]
    bmi         l1.2964
    cmp         x2,#0x39
    bgt         lbl86
    ldr         w2, [x7,x2,lsl #2]
    sxtw        x2,w2
lbl86:
    sub         x20,x2,#6
    csel        x2, x20, x2,gt
l1.2964:
    trn1        v29.2s, v5.2s, v17.2s
    trn2        v17.2s, v5.2s, v17.2s
    mov         v5.d[0], v29.d[0]
    add         x3,x3,x5,lsl #1
    trn1        v29.2s, v16.2s, v4.2s
    trn2        v4.2s, v16.2s, v4.2s
    mov         v16.d[0], v29.d[0]
    add         x6,x3,#2
    uxtl        v18.8h, v17.8b
    cmp         x6,#0x35
    mov         x20,#0x35
    csel        x3, x20, x3,gt
    bgt         l1.2996
    adds        x6,x3,#2
    add         x20,x3,#2
    csel        x3, x20, x3,pl
    mov         x20,#0
    csel        x3, x20, x3,mi
l1.2996:
    usubl       v0.8h, v17.8b, v16.8b
    adrp        x6, :got:gai4_ihevc_tc_table
    ldr         x6, [x6, #:got_lo12:gai4_ihevc_tc_table]
    shl         v0.8h, v0.8h,#2
    add         x2,x2,x5,lsl #1
    add         x5,x2,#2
    uaddw       v0.8h,  v0.8h ,  v5.8b
    cmp         x5,#0x35
    ldr         w3, [x6,x3,lsl #2]
    sxtw        x3,w3
    usubw       v4.8h,  v0.8h ,  v4.8b
    mov         x20,#0x35
    csel        x2, x20, x2,gt
    bgt         l1.3036
    adds        x5,x2,#2
    add         x20,x2,#2
    csel        x2, x20, x2,pl
    mov         x20,#0
    csel        x2, x20, x2,mi
l1.3036:


    srshr       v6.8h, v4.8h,#3
    dup         v2.4h,w3
    ldr         w2, [x6,x2,lsl #2]
    sxtw        x2,w2
    sub         x20,x3,#0
    neg         x3, x20
    cmp         x12,#0
    dup         v3.4h,w2
    sub         x20,x2,#0
    neg         x2, x20
    dup         v30.4h,w3
    dup         v31.4h,w2

    mov         v30.d[1],v31.d[0]
    mov         v2.d[1],v3.d[0]

    smin        v4.8h,  v6.8h ,  v2.8h
    smax        v2.8h,  v30.8h ,  v4.8h

    uxtl        v6.8h, v16.8b

    add         v0.8h,  v6.8h ,  v2.8h
    sub         v2.8h,  v18.8h ,  v2.8h
    sqxtun      v0.8b, v0.8h
    sub         x2,x0,#2
    sqxtun      v1.8b, v2.8h
    trn1        v29.2s, v0.2s, v1.2s
    trn2        v1.2s, v0.2s, v1.2s
    mov         v0.d[0], v29.d[0]
    trn1        v29.8b, v0.8b, v1.8b
    trn2        v1.8b, v0.8b, v1.8b
    mov         v0.d[0], v29.d[0]
    beq         l1.3204

    st1         {v0.h}[0],[x2],x1
    st1         {v1.h}[0],[x2],x1
    st1         {v0.h}[1],[x2],x1
    st1         {v1.h}[1],[x2]
l1.3204:
    cmp         x4,#0
    beq         l1.3228
    st1         {v0.h}[2],[x0],x1
    st1         {v1.h}[2],[x0],x1
    st1         {v0.h}[3],[x0],x1
    st1         {v1.h}[3],[x0]
l1.3228:
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret



