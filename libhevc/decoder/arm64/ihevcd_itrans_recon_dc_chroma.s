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
//*  ihevcd_itrans_recon_dc_chroma.s
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


.globl ihevcd_itrans_recon_dc_chroma_av8

.type ihevcd_itrans_recon_dc_chroma_av8, %function

ihevcd_itrans_recon_dc_chroma_av8:

//void ihevcd_itrans_recon_dc_chroma(uword8 *pu1_pred,
//                            uword8 *pu1_dst,
//                            word32 pred_strd,
//                            word32 dst_strd,
//                            word32 log2_trans_size,
//                            word16 i2_coeff_value)

//x0:pu1_pred
//x1:pu1_dest
//x2:pred_strd
//x3:dst_strd



    push_v_regs
    stp         x19, x20,[sp,#-16]!

    sxth        x5, w5 // since the argument is of word16, sign extend to x register

    mov         x10,#1
    lsl         x4,x10,x4                   //    trans_size = (1 << log2_trans_size)//
    mov         x6,#64                      // 1 << (shift1 - 1)//
    mov         x7,#2048                    // 1<<(shift2-1)

    add         x8,x6,x5,lsl #6
    asr         x20, x8, #7
    mov         x19,#32767
    cmp         x20,x19
    blt         lbl36
    mov         x8,#32767
    b           lbl36_1
lbl36:
    mov         x19,#-32768
    cmp         x20,x19
    csel        x8, x19, x20, lt
lbl36_1:

    add         x5,x7,x8,lsl #6
    asr         x20, x5, #12
    mov         x19,#32767
    cmp         x20,x19
    blt         lbl38
    mov         x6,#32767
    b           lbl38_1
lbl38:
    mov         x19,#-32768
    cmp         x20,x19
    csel        x6, x19, x20, lt
lbl38_1:

    mov         x9,x4
    mov         x8,x4

    // x6 has the dc_value
    // x4 has the trans_size value
    // x8 has the row value
    // x9 has the col value
    dup         v0.8h,w6
    cmp         x4,#4
    beq         row_loop_4chroma


row_loop_chroma:
    mov         x9,x4


col_loop_chroma:

    mov         x7,x0
    ld2         {v2.8b, v3.8b},[x7],x2
    ld2         {v4.8b, v5.8b},[x7],x2
    ld2         {v6.8b, v7.8b},[x7],x2
    ld2         {v8.8b, v9.8b},[x7],x2

    ld2         {v10.8b, v11.8b},[x7],x2
    ld2         {v12.8b, v13.8b},[x7],x2
    ld2         {v14.8b, v15.8b},[x7],x2
    ld2         {v16.8b, v17.8b},[x7]

    add         x0,x0,#16


    uaddw       v30.8h,  v0.8h ,  v2.8b
    uaddw       v28.8h,  v0.8h ,  v4.8b
    uaddw       v26.8h,  v0.8h ,  v6.8b
    uaddw       v24.8h,  v0.8h ,  v8.8b
    uaddw       v22.8h,  v0.8h ,  v10.8b
    uaddw       v20.8h,  v0.8h ,  v12.8b
    uaddw       v18.8h,  v0.8h ,  v14.8b


    mov         x11,x1
    sqxtun      v2.8b, v30.8h
    sqxtun      v4.8b, v28.8h
    sqxtun      v6.8b, v26.8h
    sqxtun      v8.8b, v24.8h

    uaddw       v30.8h,  v0.8h ,  v16.8b

    sqxtun      v10.8b, v22.8h
    sqxtun      v12.8b, v20.8h
    sqxtun      v14.8b, v18.8h
    sqxtun      v16.8b, v30.8h

    st2         {v2.8b, v3.8b},[x11],x3
    st2         {v4.8b, v5.8b},[x11],x3
    st2         {v6.8b, v7.8b},[x11],x3
    st2         {v8.8b, v9.8b},[x11],x3

    st2         {v10.8b, v11.8b},[x11],x3
    st2         {v12.8b, v13.8b},[x11],x3
    st2         {v14.8b, v15.8b},[x11],x3
    st2         {v16.8b, v17.8b},[x11]

    add         x1,x1,#16

    subs        x9,x9,#8
    bgt         col_loop_chroma

    subs        x8,x8,#8

    add         x0,x0,x2,lsl #3
    add         x1,x1,x3,lsl #3
    sub         x0,x0,x4,lsl #1
    sub         x1,x1,x4,lsl #1
    bgt         row_loop_chroma
    b           end_loops_chroma


row_loop_4chroma:
    mov         x9,x10


col_loop_4chroma:


    ld2         {v2.8b, v3.8b},[x0],x2
    ld2         {v4.8b, v5.8b},[x0],x2
    ld2         {v6.8b, v7.8b},[x0],x2
    ld2         {v8.8b, v9.8b},[x0]




    uaddw       v30.8h,  v0.8h ,  v2.8b
    uaddw       v28.8h,  v0.8h ,  v4.8b
    uaddw       v26.8h,  v0.8h ,  v6.8b
    uaddw       v24.8h,  v0.8h ,  v8.8b



    sqxtun      v31.8b, v30.8h
    sqxtun      v29.8b, v28.8h
    sqxtun      v27.8b, v26.8h
    sqxtun      v25.8b, v24.8h


    zip1        v2.8b, v31.8b, v3.8b
    zip1        v4.8b, v29.8b, v5.8b
    zip1        v6.8b, v27.8b, v7.8b
    zip1        v8.8b, v25.8b, v9.8b

    st1         {v2.2s},[x1],x3
    st1         {v4.2s},[x1],x3
    st1         {v6.2s},[x1],x3
    st1         {v8.2s},[x1]

end_loops_chroma:
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret


