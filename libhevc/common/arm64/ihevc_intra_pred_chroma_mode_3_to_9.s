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
//*  ihevc_intra_pred_chroma_mode_3_to_9.s
//*
//* @brief
//*  contains function definitions for intra prediction dc filtering.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* @author
//*  parthiban v
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
//void ihevc_intra_pred_chroma_mode_3_to_9(uword8 *pu1_ref,
//                                       word32 src_strd,
//                                       uword8 *pu1_dst,
//                                       word32 dst_strd,
//                                       word32 nt,
//                                       word32 mode)
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



.globl ihevc_intra_pred_chroma_mode_3_to_9_av8
.extern gai4_ihevc_ang_table
.extern gai4_ihevc_inv_ang_table
.extern col_for_intra_chroma
.extern idx_neg_idx_chroma_3_9

.type ihevc_intra_pred_chroma_mode_3_to_9_av8, %function

ihevc_intra_pred_chroma_mode_3_to_9_av8:

    // stmfd sp!, {x4-x12, x14}        //stack stores the values of the arguments

    stp         d13,d14,[sp,#-16]!
    stp         d8,d15,[sp,#-16]!           // Storing d15 using { sub sp,sp,#8; str d15,[sp] } is giving bus error.
                                            // d8 is used as dummy register and stored along with d15 using stp. d8 is not used in the function.
    stp         x19, x20,[sp,#-16]!

    adrp        x7,  :got:gai4_ihevc_ang_table
    ldr         x7,  [x7, #:got_lo12:gai4_ihevc_ang_table]

    adrp        x8,  :got:gai4_ihevc_inv_ang_table
    ldr         x8,  [x8, #:got_lo12:gai4_ihevc_inv_ang_table]

    add         x7, x7, x5, lsl #2          //gai4_ihevc_ang_table[mode]
    ldr         w7,  [x7]                   //intra_pred_ang
    sxtw        x7,w7
    dup         v30.8b,w7                   //intra_pred_ang

    adrp        x14,  :got:col_for_intra_chroma
    ldr         x14,  [x14, #:got_lo12:col_for_intra_chroma]

prologue_8_16_32:
    lsr         x10, x4, #3
    ld1         {v31.8b},[x14],#8
    mul         x10, x4, x10                //block counter (dec by #8)

    lsl         x11, x4, #1                 //col counter to be inc/dec by #8
    smull       v22.8h, v30.8b, v31.8b      //(col+1)*intra_pred_angle [0:7](col)

    sub         x7, x5, #3
    adrp        x12,  :got:idx_neg_idx_chroma_3_9 //load most idx table
    ldr         x12, [x12,  #:got_lo12:idx_neg_idx_chroma_3_9]

    add         x12, x12, x7, lsl #4
    mov         x8, x12

    mov         x7, #8
    sub         x7, x7, x3, lsl #3          //x7 = 8-8x3

    ldr         w9,  [x8]
    sxtw        x9,w9
    lsl         x9, x9, #1
    add         x1, x0, x4, lsl #2          //pu1_ref + 4*nt

    xtn         v6.8b,  v22.8h
    dup         v26.8b,w9                   //most idx added to final idx values
    sub         x1, x1, #26                 //ref_main_idx + 2nt - (8 + 1)(two_nt - idx - row ) for 8 & 8 - 1row

    sub         x6, x1, x9

    ld1         {v0.16b, v1.16b}, [x6]      //stores the 32 values reqd based on indices values (from most idx)
    sshr        v22.8h, v22.8h,#5

    movi        v29.8b, #31                 //contains #31 for vand operation

    movi        v28.8b, #32

    sqxtn       v2.8b,  v22.8h
    shl         v2.8b, v2.8b,#1             // 2 * idx

    and         v6.8b,  v6.8b ,  v29.8b     //fract values in d1/ idx values in d0
    movi        v29.8b, #2                  //contains #2 for adding to get ref_main_idx + 1

    mov         x0,#0x302                   // idx value for v is +1 of u
    dup         v27.4h,w0
    mov         x0,#0

    movi        v3.8b, #22                  //row 0 to 7

    sub         v2.8b,  v2.8b ,  v27.8b     //ref_main_idx (sub row)
    sub         v2.8b,  v26.8b ,  v2.8b     //ref_main_idx (row 0)
    add         v2.8b,  v2.8b ,  v3.8b      //to compensate the pu1_src idx incremented by 8
    sub         v3.8b,  v2.8b ,  v29.8b     //ref_main_idx + 1 (row 0)
    tbl         v25.8b, {  v0.16b, v1.16b}, v2.8b //load from ref_main_idx (row 0)
    sub         v7.8b,  v28.8b ,  v6.8b     //32-fract

    tbl         v13.8b, {  v0.16b, v1.16b}, v3.8b //load from ref_main_idx + 1 (row 0)
    sub         v4.8b,  v2.8b ,  v29.8b     //ref_main_idx (row 1)
    sub         v5.8b,  v3.8b ,  v29.8b     //ref_main_idx + 1 (row 1)

    movi        v29.8b, #4

    tbl         v16.8b, {  v0.16b, v1.16b}, v4.8b //load from ref_main_idx (row 1)
    umull       v24.8h, v25.8b, v7.8b       //mul (row 0)
    umlal       v24.8h, v13.8b, v6.8b       //mul (row 0)

    tbl         v17.8b, {  v0.16b, v1.16b}, v5.8b //load from ref_main_idx + 1 (row 1)
    sub         v2.8b,  v2.8b ,  v29.8b     //ref_main_idx (row 2)
    sub         v3.8b,  v3.8b ,  v29.8b     //ref_main_idx + 1 (row 2)

    rshrn       v24.8b, v24.8h,#5           //round shft (row 0)

    tbl         v14.8b, {  v0.16b, v1.16b}, v2.8b //load from ref_main_idx (row 2)
    umull       v22.8h, v16.8b, v7.8b       //mul (row 1)
    umlal       v22.8h, v17.8b, v6.8b       //mul (row 1)

    tbl         v15.8b, {  v0.16b, v1.16b}, v3.8b //load from ref_main_idx + 1 (row 2)
    sub         v4.8b,  v4.8b ,  v29.8b     //ref_main_idx (row 3)
    sub         v5.8b,  v5.8b ,  v29.8b     //ref_main_idx + 1 (row 3)

    st1         {v24.8b},[x2], x3           //st (row 0)
    rshrn       v22.8b, v22.8h,#5           //round shft (row 1)

    tbl         v19.8b, {  v0.16b, v1.16b}, v4.8b //load from ref_main_idx (row 3)
    umull       v20.8h, v14.8b, v7.8b       //mul (row 2)
    umlal       v20.8h, v15.8b, v6.8b       //mul (row 2)

    tbl         v23.8b, {  v0.16b, v1.16b}, v5.8b //load from ref_main_idx + 1 (row 3)
    sub         v2.8b,  v2.8b ,  v29.8b     //ref_main_idx (row 4)
    sub         v3.8b,  v3.8b ,  v29.8b     //ref_main_idx + 1 (row 4)

    st1         {v22.8b},[x2], x3           //st (row 1)
    rshrn       v20.8b, v20.8h,#5           //round shft (row 2)

    tbl         v25.8b, {  v0.16b, v1.16b}, v2.8b //load from ref_main_idx (row 4)
    umull       v18.8h, v19.8b, v7.8b       //mul (row 3)
    umlal       v18.8h, v23.8b, v6.8b       //mul (row 3)

    tbl         v13.8b, {  v0.16b, v1.16b}, v3.8b //load from ref_main_idx + 1 (row 4)
    sub         v4.8b,  v4.8b ,  v29.8b     //ref_main_idx (row 5)
    sub         v5.8b,  v5.8b ,  v29.8b     //ref_main_idx + 1 (row 5)

    st1         {v20.8b},[x2], x3           //st (row 2)
    rshrn       v18.8b, v18.8h,#5           //round shft (row 3)

    tbl         v16.8b, {  v0.16b, v1.16b}, v4.8b //load from ref_main_idx (row 5)
    umull       v24.8h, v25.8b, v7.8b       //mul (row 4)
    umlal       v24.8h, v13.8b, v6.8b       //mul (row 4)

    tbl         v17.8b, {  v0.16b, v1.16b}, v5.8b //load from ref_main_idx + 1 (row 5)
    sub         v2.8b,  v2.8b ,  v29.8b     //ref_main_idx (row 6)
    sub         v3.8b,  v3.8b ,  v29.8b     //ref_main_idx + 1 (row 6)

    st1         {v18.8b},[x2], x3           //st (row 3)
    cmp         x4,#4
    beq         end_func
    rshrn       v24.8b, v24.8h,#5           //round shft (row 4)

    tbl         v14.8b, {  v0.16b, v1.16b}, v2.8b //load from ref_main_idx (row 6)
    umull       v22.8h, v16.8b, v7.8b       //mul (row 5)
    umlal       v22.8h, v17.8b, v6.8b       //mul (row 5)

    tbl         v15.8b, {  v0.16b, v1.16b}, v3.8b //load from ref_main_idx + 1 (row 6)
    sub         v4.8b,  v4.8b ,  v29.8b     //ref_main_idx (row 7)
    sub         v5.8b,  v5.8b ,  v29.8b     //ref_main_idx + 1 (row 7)

    st1         {v24.8b},[x2], x3           //st (row 4)
    rshrn       v22.8b, v22.8h,#5           //round shft (row 5)

    tbl         v19.8b, {  v0.16b, v1.16b}, v4.8b //load from ref_main_idx (row 7)
    umull       v20.8h, v14.8b, v7.8b       //mul (row 6)
    umlal       v20.8h, v15.8b, v6.8b       //mul (row 6)

    tbl         v23.8b, {  v0.16b, v1.16b}, v5.8b //load from ref_main_idx + 1 (row 7)
    umull       v18.8h, v19.8b, v7.8b       //mul (row 7)
    umlal       v18.8h, v23.8b, v6.8b       //mul (row 7)

    st1         {v22.8b},[x2], x3           //st (row 5)
    rshrn       v20.8b, v20.8h,#5           //round shft (row 6)
    rshrn       v18.8b, v18.8h,#5           //round shft (row 7)

    st1         {v20.8b},[x2], x3           //st (row 6)

    subs        x10, x10, #4                //subtract 8 and go to end if 8x8

    st1         {v18.8b},[x2], x3           //st (row 7)

    beq         end_func

    subs        x11, x11, #8                //decrement the processed col
    add         x20, x8, #4
    csel        x8, x20, x8,gt
    add         x20, x2, x7
    csel        x2, x20, x2,gt
    csel        x8, x12, x8,le
    sub         x20, x2, x4
    csel        x2, x20, x2,le
    add         x20, x2, #8
    csel        x2, x20, x2,le
    lsl         x20, x4,  #1
    csel        x11,x20,x11,le
    bgt         lbl284
    adrp        x14,  :got:col_for_intra_chroma
    ldr         x14,  [x14, #:got_lo12:col_for_intra_chroma]
lbl284:
    add         x20, x0, #8
    csel        x0, x20, x0,le

    ld1         {v31.8b},[x14],#8
    smull       v25.8h, v30.8b, v31.8b      //(col+1)*intra_pred_angle [0:7](col)
    xtn         v19.8b,  v25.8h
    sshr        v25.8h, v25.8h,#5
    sqxtn       v23.8b,  v25.8h
    shl         v23.8b, v23.8b,#1
    mov         x5, #0x302                  //idx value for v is +1 of u
    dup         v27.4h,w5                   //row value inc or reset accordingly
    ldr         w9,  [x8]                   //loads index value
    sxtw        x9,w9
    lsl         x9, x9, #1
    mov         x5, #22
    sub         x5, x5, x0, lsl #1
    dup         v16.8b,w5
    dup         v26.8b,w9

    mov         x5,x2
    sub         v23.8b,  v23.8b ,  v27.8b   //ref_main_idx (sub row)

kernel_8_16_32:
    movi        v29.8b, #2                  //contains #2 for adding to get ref_main_idx + 1
    sub         v2.8b,  v26.8b ,  v23.8b    //ref_main_idx
    mov         v26.8b, v19.8b

    subs        x11, x11, #8
    sub         x6, x1, x9
    tbl         v19.8b, {  v0.16b, v1.16b}, v4.8b //load from ref_main_idx (row 7)
    add         v2.8b,  v2.8b ,  v16.8b     //to compensate the pu1_src idx incremented by 8

    umull       v20.8h, v14.8b, v7.8b       //mul (row 6)
    tbl         v23.8b, {  v0.16b, v1.16b}, v5.8b //load from ref_main_idx - 1 (row 7)
    umlal       v20.8h, v15.8b, v6.8b       //mul (row 6)

    add         x20, x0, #8
    csel        x0, x20, x0,le
    sub         v3.8b,  v2.8b ,  v29.8b     //ref_main_idx - 2
    add         x20, x8, #4
    csel        x8, x20, x8,gt

    ld1         {v0.16b, v1.16b}, [x6]      //stores the 32 values reqd based on indices values (from most idx)
    rshrn       v22.8b, v22.8h,#5           //round shft (row 5)

    bgt         lbl326
    adrp        x14,  :got:col_for_intra_chroma
    ldr         x14,  [x14, #:got_lo12:col_for_intra_chroma]
lbl326:
    st1         {v24.8b},[x5], x3           //st (row 4)
    csel        x8, x12, x8,le

    mov         x9,#0x302
    dup         v27.4h,w9                   //row value inc or reset accordingly
    sub         v4.8b,  v2.8b ,  v29.8b     //ref_main_idx (row 1)

    sub         v5.8b,  v3.8b ,  v29.8b     //ref_main_idx - 1 (row 1)
    tbl         v25.8b, {  v0.16b, v1.16b}, v2.8b //load from ref_main_idx (row 0)
    movi        v29.8b, #31                 //contains #2 for adding to get ref_main_idx + 1

    umull       v18.8h, v19.8b, v7.8b       //mul (row 7)
    tbl         v13.8b, {  v0.16b, v1.16b}, v3.8b //load from ref_main_idx + 1 (row 0)
    umlal       v18.8h, v23.8b, v6.8b       //mul (row 7)

    ld1         {v31.8b},[x14],#8
    and         v6.8b,  v29.8b ,  v26.8b    //fract values in d1/ idx values in d0

    lsl         x20, x4,  #1
    csel        x11,x20,x11,le
    movi        v29.8b, #4                  //contains #2 for adding to get ref_main_idx + 1
    ldr         w9,  [x8]
    sxtw        x9,w9

    st1         {v22.8b},[x5], x3           //(from previous loop)st (row 5)
    rshrn       v20.8b, v20.8h,#5           //(from previous loop)round shft (row 6)

    sub         v2.8b,  v2.8b ,  v29.8b     //ref_main_idx (row 2)
    tbl         v19.8b, {  v0.16b, v1.16b}, v4.8b //load from ref_main_idx (row 1)
    sub         v3.8b,  v3.8b ,  v29.8b     //ref_main_idx - 1 (row 2)

    lsl         x9, x9, #1
    sub         v7.8b,  v28.8b ,  v6.8b     //32-fract

    umull       v24.8h, v25.8b, v7.8b       //mul (row 0)
    tbl         v17.8b, {  v0.16b, v1.16b}, v5.8b //load from ref_main_idx + 1 (row 1)
    umlal       v24.8h, v13.8b, v6.8b       //mul (row 0)

    st1         {v20.8b},[x5], x3           //(from previous loop)st (row 6)
    rshrn       v18.8b, v18.8h,#5           //(from previous loop)round shft (row 7)

    sub         v4.8b,  v4.8b ,  v29.8b     //ref_main_idx (row 3)
    tbl         v14.8b, {  v0.16b, v1.16b}, v2.8b //load from ref_main_idx (row 2)
    sub         v5.8b,  v5.8b ,  v29.8b     //ref_main_idx - 1 (row 3)

    umull       v22.8h, v19.8b, v7.8b       //mul (row 1)
    tbl         v15.8b, {  v0.16b, v1.16b}, v3.8b //load from ref_main_idx + 1 (row 2)
    umlal       v22.8h, v17.8b, v6.8b       //mul (row 1)

    rshrn       v24.8b, v24.8h,#5           //round shft (row 0)
    st1         {v18.8b},[x5], x3           //(from previous loop)st (row 7)

    sub         v2.8b,  v2.8b ,  v29.8b     //ref_main_idx (row 4)
    tbl         v19.8b, {  v0.16b, v1.16b}, v4.8b //load from ref_main_idx (row 3)
    sub         v3.8b,  v3.8b ,  v29.8b     //ref_main_idx - 1 (row 4)

    umull       v20.8h, v14.8b, v7.8b       //mul (row 2)
    tbl         v23.8b, {  v0.16b, v1.16b}, v5.8b //load from ref_main_idx + 1 (row 3)
    umlal       v20.8h, v15.8b, v6.8b       //mul (row 2)

    add         x5,x2,x3,lsl#2
    smull       v14.8h, v30.8b, v31.8b      //(col+1)*intra_pred_angle [0:7](col)
    add         x9, x9, x0, lsl #1

    st1         {v24.8b},[x2], x3           //st (row 0)
    rshrn       v22.8b, v22.8h,#5           //round shft (row 1)

    sub         v4.8b,  v4.8b ,  v29.8b     //ref_main_idx (row 5)
    tbl         v25.8b, {  v0.16b, v1.16b}, v2.8b //load from ref_main_idx (row 4)
    sub         v5.8b,  v5.8b ,  v29.8b     //ref_main_idx - 1 (row 5)

    umull       v18.8h, v19.8b, v7.8b       //mul (row 3)
    tbl         v13.8b, {  v0.16b, v1.16b}, v3.8b //load from ref_main_idx + 1 (row 4)
    umlal       v18.8h, v23.8b, v6.8b       //mul (row 3)

    st1         {v22.8b},[x2], x3           //st (row 1)
    rshrn       v20.8b, v20.8h,#5           //round shft (row 2)

    xtn         v19.8b,  v14.8h
    sshr        v14.8h, v14.8h,#5

    sub         v2.8b,  v2.8b ,  v29.8b     //ref_main_idx (row 6)
    tbl         v21.8b, {  v0.16b, v1.16b}, v4.8b //load from ref_main_idx (row 5)
    sub         v3.8b,  v3.8b ,  v29.8b     //ref_main_idx - 1 (row 6)

    umull       v24.8h, v25.8b, v7.8b       //mul (row 4)
    tbl         v17.8b, {  v0.16b, v1.16b}, v5.8b //load from ref_main_idx + 1 (row 5)
    sqxtn       v23.8b,  v14.8h

    st1         {v20.8b},[x2], x3           //st (row 2)
    umlal       v24.8h, v13.8b, v6.8b       //mul (row 4)

    rshrn       v18.8b, v18.8h,#5           //round shft (row 3)
    dup         v26.8b,w9

    sub         v4.8b,  v4.8b ,  v29.8b     //ref_main_idx (row 7)
    tbl         v14.8b, {  v0.16b, v1.16b}, v2.8b //load from ref_main_idx (row 6)
    sub         v5.8b,  v5.8b ,  v29.8b     //ref_main_idx - 1 (row 7)

    mov         x6, #22                     //to compensate the 2*row value
    shl         v23.8b, v23.8b,#1
    sub         x6, x6, x0, lsl #1

    umull       v22.8h, v21.8b, v7.8b       //mul (row 5)
    tbl         v15.8b, {  v0.16b, v1.16b}, v3.8b //load from ref_main_idx + 1 (row 6)
    umlal       v22.8h, v17.8b, v6.8b       //mul (row 5)

    st1         {v18.8b},[x2], x3           //st (row 3)
    rshrn       v24.8b, v24.8h,#5           //round shft (row 4)

    add         x2,x2,x3, lsl #2
    dup         v16.8b,w6
    add         x20, x7, x2
    csel        x2, x20, x2,gt

    sub         x20, x2, x4
    csel        x2, x20, x2,le
    sub         v23.8b,  v23.8b ,  v27.8b   //ref_main_idx (add row)
    sub         x20,x2,#8
    csel        x2, x20, x2,le

    subs        x10, x10, #4                //subtract 8 and go to end if 8x8

    bne         kernel_8_16_32

epil_8_16_32:
    tbl         v19.8b, {  v0.16b, v1.16b}, v4.8b //load from ref_main_idx (row 7)

    umull       v20.8h, v14.8b, v7.8b       //mul (row 6)
    tbl         v23.8b, {  v0.16b, v1.16b}, v5.8b //load from ref_main_idx + 1 (row 7)
    umlal       v20.8h, v15.8b, v6.8b       //mul (row 6)

    st1         {v24.8b},[x5], x3           //st (row 4)
    rshrn       v24.8b, v22.8h,#5           //round shft (row 5)

    umull       v18.8h, v19.8b, v7.8b       //mul (row 7)
    umlal       v18.8h, v23.8b, v6.8b       //mul (row 7)

    st1         {v24.8b},[x5], x3           //(from previous loop)st (row 5)
    rshrn       v20.8b, v20.8h,#5           //(from previous loop)round shft (row 6)

    st1         {v20.8b},[x5], x3           //(from previous loop)st (row 6)
    rshrn       v18.8b, v18.8h,#5           //(from previous loop)round shft (row 7)

    st1         {v18.8b},[x5], x3           //st (row 7)

end_func:
    // ldmfd sp!,{x4-x12,x15}               //reload the registers from sp
    ldp         x19, x20,[sp],#16
    ldp         d8,d15,[sp],#16             // Loading d15 using { ldr d15,[sp]; add sp,sp,#8 } is giving bus error.
                                            // d8 is used as dummy register and loaded along with d15 using ldp. d8 is not used in the function.
    ldp         d13,d14,[sp],#16
    ret








