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
//* //file
//*  ihevcd_itrans_recon_dc_luma.s
//*
//* //brief
//*  contains function definitions itrans and recon for dc only case
//*
//* //author
//*  ittiam
//*
//* //par list of functions:
//*
//*
//* //remarks
//*  none
//*
//*******************************************************************************/

.text
.include "ihevc_neon_macros.s"



.globl ihevcd_itrans_recon_dc_luma_av8

.type ihevcd_itrans_recon_dc_luma_av8, %function

ihevcd_itrans_recon_dc_luma_av8:

//void ihevcd_itrans_recon_dc_luma(uword8 *pu1_pred,
//                            uword8 *pu1_dst,
//                            word32 pred_strd,
//                            word32 dst_strd,
//                            word32 log2_trans_size,
//                            word16 i2_coeff_value)

//x0:pu1_pred
//x1:pu1_dest
//x2:pred_strd
//x3:dst_strd




    stp         x19, x20,[sp,#-16]!
    sxth        x5,w5

    mov         x10,#1
    lsl         x4,x10,x4                   //    trans_size = (1 << log2_trans_size)//
    mov         x6,#64                      // 1 << (shift1 - 1)//
    mov         x7,#2048                    // 1<<(shift2-1)

    add         x8,x6,x5,lsl #6
    asr         x20, x8, #7
    mov         x19, #32767
    cmp         x20,x19
    blt         lbl37
    mov         x8,#32767
    b           lbl37_1
lbl37:
    mov         x19,#-32768
    cmp         x20,x19
    csel        x8, x19, x20, lt
lbl37_1:

    add         x5,x7,x8,lsl #6
    asr         x20, x5, #12
    mov         x19,#32767
    cmp         x20,x19
    blt         lbl39
    mov         x6,#32767
    b           lbl39_1
lbl39:
    mov         x19,#-32768
    cmp         x20,x19
    csel        x6, x19, x20, lt
lbl39_1:

    mov         x9,x4
    mov         x8,x4

    // x6 has the dc_value
    // x4 has the trans_size value
    // x8 has the row value
    // x9 has the col value
    dup         v0.8h,w6
    cmp         x4,#4
    beq         row_loop_4


row_loop:
    mov         x9,x4


col_loop:

    mov         x7,x0
    ld1         {v2.8b},[x7],x2
    ld1         {v3.8b},[x7],x2
    ld1         {v4.8b},[x7],x2
    ld1         {v5.8b},[x7],x2

    ld1         {v6.8b},[x7],x2
    ld1         {v7.8b},[x7],x2
    ld1         {v1.8b},[x7],x2
    ld1         {v17.8b},[x7]

    add         x0,x0,#8


    uaddw       v30.8h,  v0.8h ,  v2.8b
    uaddw       v28.8h,  v0.8h ,  v3.8b
    uaddw       v26.8h,  v0.8h ,  v4.8b
    uaddw       v24.8h,  v0.8h ,  v5.8b
    uaddw       v22.8h,  v0.8h ,  v6.8b
    uaddw       v20.8h,  v0.8h ,  v7.8b
    uaddw       v18.8h,  v0.8h ,  v1.8b
    uaddw       v16.8h,  v0.8h ,  v17.8b

    mov         x11,x1
    sqxtun      v2.8b, v30.8h
    sqxtun      v3.8b, v28.8h
    sqxtun      v4.8b, v26.8h
    sqxtun      v5.8b, v24.8h
    sqxtun      v6.8b, v22.8h
    sqxtun      v7.8b, v20.8h
    sqxtun      v1.8b, v18.8h
    sqxtun      v17.8b, v16.8h


    st1         {v2.2s},[x11],x3
    st1         {v3.2s},[x11],x3
    st1         {v4.2s},[x11],x3
    st1         {v5.2s},[x11],x3
    st1         {v6.2s},[x11],x3
    st1         {v7.2s},[x11],x3
    st1         {v1.2s},[x11],x3
    st1         {v17.2s},[x11]

    add         x1,x1,#8

    subs        x9,x9,#8
    bgt         col_loop

    subs        x8,x8,#8

    add         x0,x0,x2,lsl #3
    add         x1,x1,x3,lsl #3
    sub         x0,x0,x4
    sub         x1,x1,x4
    bgt         row_loop
    b           end_loops


row_loop_4:
    mov         x9,x10


col_loop_4:


    ld1         {v2.8b},[x0],x2
    ld1         {v3.8b},[x0],x2
    ld1         {v4.8b},[x0],x2
    ld1         {v5.8b},[x0]




    uaddw       v30.8h,  v0.8h ,  v2.8b
    uaddw       v28.8h,  v0.8h ,  v3.8b
    uaddw       v26.8h,  v0.8h ,  v4.8b
    uaddw       v24.8h,  v0.8h ,  v5.8b



    sqxtun      v2.8b, v30.8h
    sqxtun      v3.8b, v28.8h
    sqxtun      v4.8b, v26.8h
    sqxtun      v5.8b, v24.8h



    st1         {v2.s}[0],[x1],x3
    st1         {v3.s}[0],[x1],x3
    st1         {v4.s}[0],[x1],x3
    st1         {v5.s}[0],[x1]

end_loops:
    ldp         x19, x20,[sp],#16

    ret








