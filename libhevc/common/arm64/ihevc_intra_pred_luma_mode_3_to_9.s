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
//*  ihevc_intra_pred_luma_mode_3_to_9.s
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

//void ihevc_intra_pred_luma_mode_3_to_9(uword8* pu1_ref,
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



.globl ihevc_intra_pred_luma_mode_3_to_9_av8
.extern gai4_ihevc_ang_table
.extern gai4_ihevc_inv_ang_table
.extern col_for_intra_luma
.extern idx_neg_idx_3_9


.type ihevc_intra_pred_luma_mode_3_to_9_av8, %function

ihevc_intra_pred_luma_mode_3_to_9_av8:

    // stmfd sp!, {x4-x12, x14}        //stack stores the values of the arguments

    stp         d12,d13,[sp,#-16]!
    stp         d14,d15,[sp,#-16]!
    stp         x19, x20,[sp,#-16]!

    adrp        x7,  :got:gai4_ihevc_ang_table
    ldr         x7,  [x7, #:got_lo12:gai4_ihevc_ang_table]

    adrp        x8,  :got:gai4_ihevc_inv_ang_table
    ldr         x8,  [x8, #:got_lo12:gai4_ihevc_inv_ang_table]

    add         x7, x7, x5, lsl #2          //gai4_ihevc_ang_table[mode]
    ldr         w7,  [x7]                   //intra_pred_ang
    sxtw        x7,w7
    dup         v30.8b,w7                   //intra_pred_ang

    adrp        x14,  :got:col_for_intra_luma
    ldr         x14,  [x14, #:got_lo12:col_for_intra_luma]

    cmp         x4, #4

    beq         sz_4_proc
    b           prologue_8_16_32

prologue_8_16_32:
    lsr         x10, x4, #3
    ld1         {v31.8b},[x14],#8
    mul         x10, x4, x10                //block counter (dec by #8)

    mov         x11, x4                     //col counter to be inc/dec by #8
    smull       v22.8h, v30.8b, v31.8b      //(col+1)*intra_pred_angle [0:7](col)

    sub         x7, x5, #3
    movi        v2.8b, #1                   //contains #1 for adding to get ref_main_idx + 1
    adrp        x12, :got:idx_neg_idx_3_9   //load least idx table
    ldr         x12, [x12, #:got_lo12:idx_neg_idx_3_9]
    movi        v3.8b, #2

    add         x12, x12, x7, lsl #4
    mov         x8, x12

    mov         x7, #8
    sub         x7, x7, x3, lsl #3          //x7 = 8-8x3

    ldr         w9,  [x8]
    sxtw        x9,w9
    add         x1, x0, x4, lsl #1          //pu1_ref + nt

    xtn         v6.8b,  v22.8h
    dup         v26.8b,w9                   //least idx added to final idx values
    sub         x1, x1, #9                  //ref_main_idx + 2nt - (8 + 1)(two_nt - idx - row ) for 8 & 8 - 1row

    sub         x6, x1, x9

    ld1         {v0.16b}, [x6]              //stores the 32 values reqd based on indices values (from least idx)
    sshr        v22.8h, v22.8h,#5

    movi        v29.8b, #31                 //contains #31 for vand operation

    movi        v28.8b, #32

    sqxtn       v1.8b,  v22.8h

    and         v6.8b,  v6.8b ,  v29.8b     //fract values in d1/ idx values in d0

    mov         x0, #1

    movi        v27.8b, #7                  //row 0 to 7

    sub         v1.8b,  v1.8b ,  v2.8b      //ref_main_idx (sub row)
    sub         v1.8b,  v26.8b ,  v1.8b     //ref_main_idx (row 0)
    add         v1.8b,  v1.8b ,  v27.8b     //t0 compensate the pu1_src idx incremented by 8
    sub         v19.8b,  v1.8b ,  v2.8b     //ref_main_idx + 1 (row 0)
    tbl         v12.8b, {v0.16b},v1.8b      //load from ref_main_idx (row 0)
    sub         v7.8b,  v28.8b ,  v6.8b     //32-fract

    tbl         v13.8b, {v0.16b},v19.8b     //load from ref_main_idx + 1 (row 0)
    sub         v4.8b,  v1.8b ,  v2.8b      //ref_main_idx (row 1)
    sub         v5.8b,  v19.8b ,  v2.8b     //ref_main_idx + 1 (row 1)

    tbl         v16.8b, {v0.16b},v4.8b      //load from ref_main_idx (row 1)
    umull       v24.8h, v12.8b, v7.8b       //mul (row 0)
    umlal       v24.8h, v13.8b, v6.8b       //mul (row 0)

    tbl         v17.8b, {v0.16b},v5.8b      //load from ref_main_idx + 1 (row 1)
    sub         v1.8b,  v1.8b ,  v3.8b      //ref_main_idx (row 2)
    sub         v19.8b,  v19.8b ,  v3.8b    //ref_main_idx + 1 (row 2)

    rshrn       v24.8b, v24.8h,#5           //round shft (row 0)

    tbl         v14.8b, {v0.16b},v1.8b      //load from ref_main_idx (row 2)
    umull       v22.8h, v16.8b, v7.8b       //mul (row 1)
    umlal       v22.8h, v17.8b, v6.8b       //mul (row 1)

    tbl         v15.8b, {v0.16b},v19.8b     //load from ref_main_idx + 1 (row 2)
    sub         v4.8b,  v4.8b ,  v3.8b      //ref_main_idx (row 3)
    sub         v5.8b,  v5.8b ,  v3.8b      //ref_main_idx + 1 (row 3)

    st1         {v24.8b},[x2], x3           //st (row 0)
    rshrn       v22.8b, v22.8h,#5           //round shft (row 1)

    tbl         v23.8b, {v0.16b},v4.8b      //load from ref_main_idx (row 3)
    umull       v20.8h, v14.8b, v7.8b       //mul (row 2)
    umlal       v20.8h, v15.8b, v6.8b       //mul (row 2)

    tbl         v25.8b, {v0.16b},v5.8b      //load from ref_main_idx + 1 (row 3)
    sub         v1.8b,  v1.8b ,  v3.8b      //ref_main_idx (row 4)
    sub         v19.8b,  v19.8b ,  v3.8b    //ref_main_idx + 1 (row 4)

    st1         {v22.8b},[x2], x3           //st (row 1)
    rshrn       v20.8b, v20.8h,#5           //round shft (row 2)

    tbl         v12.8b, {v0.16b},v1.8b      //load from ref_main_idx (row 4)
    umull       v18.8h, v23.8b, v7.8b       //mul (row 3)
    umlal       v18.8h, v25.8b, v6.8b       //mul (row 3)

    tbl         v13.8b, {v0.16b},v19.8b     //load from ref_main_idx + 1 (row 4)
    sub         v4.8b,  v4.8b ,  v3.8b      //ref_main_idx (row 5)
    sub         v5.8b,  v5.8b ,  v3.8b      //ref_main_idx + 1 (row 5)

    st1         {v20.8b},[x2], x3           //st (row 2)
    rshrn       v18.8b, v18.8h,#5           //round shft (row 3)

    tbl         v16.8b, {v0.16b},v4.8b      //load from ref_main_idx (row 5)
    umull       v24.8h, v12.8b, v7.8b       //mul (row 4)
    umlal       v24.8h, v13.8b, v6.8b       //mul (row 4)

    tbl         v17.8b, {v0.16b},v5.8b      //load from ref_main_idx + 1 (row 5)
    sub         v1.8b,  v1.8b ,  v3.8b      //ref_main_idx (row 6)
    sub         v19.8b,  v19.8b ,  v3.8b    //ref_main_idx + 1 (row 6)

    st1         {v18.8b},[x2], x3           //st (row 3)
    rshrn       v24.8b, v24.8h,#5           //round shft (row 4)

    tbl         v14.8b, {v0.16b},v1.8b      //load from ref_main_idx (row 6)
    umull       v22.8h, v16.8b, v7.8b       //mul (row 5)
    umlal       v22.8h, v17.8b, v6.8b       //mul (row 5)

    tbl         v15.8b, {v0.16b},v19.8b     //load from ref_main_idx + 1 (row 6)
    sub         v4.8b,  v4.8b ,  v3.8b      //ref_main_idx (row 7)
    sub         v5.8b,  v5.8b ,  v3.8b      //ref_main_idx + 1 (row 7)

    st1         {v24.8b},[x2], x3           //st (row 4)
    rshrn       v22.8b, v22.8h,#5           //round shft (row 5)

    tbl         v23.8b, {v0.16b},v4.8b      //load from ref_main_idx (row 7)
    umull       v20.8h, v14.8b, v7.8b       //mul (row 6)
    umlal       v20.8h, v15.8b, v6.8b       //mul (row 6)

    tbl         v25.8b, {v0.16b},v5.8b      //load from ref_main_idx + 1 (row 7)
    umull       v18.8h, v23.8b, v7.8b       //mul (row 7)
    umlal       v18.8h, v25.8b, v6.8b       //mul (row 7)

    st1         {v22.8b},[x2], x3           //st (row 5)
    rshrn       v20.8b, v20.8h,#5           //round shft (row 6)
    rshrn       v18.8b, v18.8h,#5           //round shft (row 7)

    st1         {v20.8b},[x2], x3           //st (row 6)

    subs        x10, x10, #8                //subtract 8 and go to end if 8x8

    st1         {v18.8b},[x2], x3           //st (row 7)

    beq         end_func

    subs        x11, x11, #8
    add         x20, x8, #4
    csel        x8, x20, x8,gt
    add         x20, x2, x7
    csel        x2, x20, x2,gt
    csel        x8, x12, x8,le
    sub         x20, x2, x4
    csel        x2, x20, x2,le
    add         x20, x2, #8
    csel        x2, x20, x2,le
    csel        x11, x4, x11,le
    bgt         lbl284
    adrp        x14,  :got:col_for_intra_luma
    ldr         x14,  [x14, #:got_lo12:col_for_intra_luma]
lbl284:
    add         x20, x0, #8
    csel        x0, x20, x0,le

    mov         x5,x2
    ld1         {v31.8b},[x14],#8
    smull       v12.8h, v30.8b, v31.8b      //(col+1)*intra_pred_angle [0:7](col)
    xtn         v23.8b,  v12.8h
    sshr        v12.8h, v12.8h,#5
    sqxtn       v25.8b,  v12.8h
    ldr         w9,  [x8]
    sxtw        x9,w9
    add         x9, x0, x9
    sub         x9, x9, #1
    dup         v26.8b,w9
    movi        v16.8b, #8

    sub         x4,x4,#8

kernel_8_16_32:

    sub         v1.8b,  v26.8b ,  v25.8b    //ref_main_idx
    mov         v26.8b, v23.8b

    subs        x11, x11, #8
    sub         x6, x1, x9
    tbl         v23.8b, {v0.16b},v4.8b      //load from ref_main_idx (row 7)
    add         v1.8b,  v1.8b ,  v16.8b     //to compensate the pu1_src idx incremented by 8

    umull       v20.8h, v14.8b, v7.8b       //mul (row 6)
    tbl         v25.8b, {v0.16b},v5.8b      //load from ref_main_idx - 1 (row 7)
    umlal       v20.8h, v15.8b, v6.8b       //mul (row 6)

    sub         v19.8b,  v1.8b ,  v2.8b     //ref_main_idx - 1
    add         x20, x0, #8
    csel        x0, x20, x0,le
    add         x20, x8, #4
    csel        x8, x20, x8,gt
    ld1         {v0.16b}, [x6]              //stores the 32 values reqd based on indices values (from least idx)

    st1         {v24.8b},[x5], x3           //st (row 4)
    rshrn       v22.8b, v22.8h,#5           //round shft (row 5)

    bgt         lbl323
    adrp        x14,  :got:col_for_intra_luma
    ldr         x14,  [x14, #:got_lo12:col_for_intra_luma]
lbl323:
    csel        x8, x12, x8,le
    dup         v27.8b,w0                   //row value inc or reset accordingly

    sub         v4.8b,  v1.8b ,  v2.8b      //ref_main_idx (row 1)
    tbl         v12.8b, {v0.16b},v1.8b      //load from ref_main_idx (row 0)
    sub         v5.8b,  v19.8b ,  v2.8b     //ref_main_idx - 1 (row 1)


    umull       v18.8h, v23.8b, v7.8b       //mul (row 7)
    tbl         v13.8b, {v0.16b},v19.8b     //load from ref_main_idx + 1 (row 0)
    umlal       v18.8h, v25.8b, v6.8b       //mul (row 7)

    ld1         {v31.8b},[x14],#8
    and         v6.8b,  v29.8b ,  v26.8b    //fract values in d1/ idx values in d0

    st1         {v22.8b},[x5], x3           //(from previous loop)st (row 5)
    rshrn       v20.8b, v20.8h,#5           //(from previous loop)round shft (row 6)

    sub         v1.8b,  v1.8b ,  v3.8b      //ref_main_idx (row 2)
    tbl         v23.8b, {v0.16b},v4.8b      //load from ref_main_idx (row 1)
    sub         v19.8b,  v19.8b ,  v3.8b    //ref_main_idx - 1 (row 2)

    add         x20, x4, #8
    csel        x11, x20, x11,le
    ldr         w9,  [x8]
    sxtw        x9,w9
    sub         v7.8b,  v28.8b ,  v6.8b     //32-fract

    umull       v24.8h, v12.8b, v7.8b       //mul (row 0)
    tbl         v17.8b, {v0.16b},v5.8b      //load from ref_main_idx + 1 (row 1)
    umlal       v24.8h, v13.8b, v6.8b       //mul (row 0)

    st1         {v20.8b},[x5], x3           //(from previous loop)st (row 6)
    rshrn       v18.8b, v18.8h,#5           //(from previous loop)round shft (row 7)

    sub         v4.8b,  v4.8b ,  v3.8b      //ref_main_idx (row 3)
    tbl         v14.8b, {v0.16b},v1.8b      //load from ref_main_idx (row 2)
    sub         v5.8b,  v5.8b ,  v3.8b      //ref_main_idx - 1 (row 3)

    umull       v22.8h, v23.8b, v7.8b       //mul (row 1)
    tbl         v15.8b, {v0.16b},v19.8b     //load from ref_main_idx + 1 (row 2)
    umlal       v22.8h, v17.8b, v6.8b       //mul (row 1)

    rshrn       v24.8b, v24.8h,#5           //round shft (row 0)
    st1         {v18.8b},[x5], x3           //(from previous loop)st (row 7)

    sub         v1.8b,  v1.8b ,  v3.8b      //ref_main_idx (row 4)
    tbl         v23.8b, {v0.16b},v4.8b      //load from ref_main_idx (row 3)
    sub         v19.8b,  v19.8b ,  v3.8b    //ref_main_idx - 1 (row 4)

    umull       v20.8h, v14.8b, v7.8b       //mul (row 2)
    tbl         v25.8b, {v0.16b},v5.8b      //load from ref_main_idx + 1 (row 3)
    umlal       v20.8h, v15.8b, v6.8b       //mul (row 2)

    smull       v14.8h, v30.8b, v31.8b      //(col+1)*intra_pred_angle [0:7](col)
    add         x5,x2,x3,lsl#2
    add         x9, x0, x9

    st1         {v24.8b},[x2], x3           //st (row 0)
    rshrn       v22.8b, v22.8h,#5           //round shft (row 1)

    sub         v4.8b,  v4.8b ,  v3.8b      //ref_main_idx (row 5)
    tbl         v12.8b, {v0.16b},v1.8b      //load from ref_main_idx (row 4)
    sub         v5.8b,  v5.8b ,  v3.8b      //ref_main_idx - 1 (row 5)

    umull       v18.8h, v23.8b, v7.8b       //mul (row 3)
    tbl         v13.8b, {v0.16b},v19.8b     //load from ref_main_idx + 1 (row 4)
    umlal       v18.8h, v25.8b, v6.8b       //mul (row 3)

    st1         {v22.8b},[x2], x3           //st (row 1)
    rshrn       v20.8b, v20.8h,#5           //round shft (row 2)

    xtn         v23.8b,  v14.8h
    sshr        v14.8h, v14.8h,#5

    sub         v1.8b,  v1.8b ,  v3.8b      //ref_main_idx (row 6)
    tbl         v21.8b, {v0.16b},v4.8b      //load from ref_main_idx (row 5)
    sub         v19.8b,  v19.8b ,  v3.8b    //ref_main_idx - 1 (row 6)

    umull       v24.8h, v12.8b, v7.8b       //mul (row 4)
    tbl         v17.8b, {v0.16b},v5.8b      //load from ref_main_idx + 1 (row 5)
    umlal       v24.8h, v13.8b, v6.8b       //mul (row 4)

    st1         {v20.8b},[x2], x3           //st (row 2)
    rshrn       v18.8b, v18.8h,#5           //round shft (row 3)

    sub         x9, x9, #1
    sqxtn       v25.8b,  v14.8h

    sub         v4.8b,  v4.8b ,  v3.8b      //ref_main_idx (row 7)
    tbl         v14.8b, {v0.16b},v1.8b      //load from ref_main_idx (row 6)
    sub         v5.8b,  v5.8b ,  v3.8b      //ref_main_idx - 1 (row 7)

    umull       v22.8h, v21.8b, v7.8b       //mul (row 5)
    tbl         v15.8b, {v0.16b},v19.8b     //load from ref_main_idx + 1 (row 6)
    umlal       v22.8h, v17.8b, v6.8b       //mul (row 5)

    add         v25.8b,  v27.8b ,  v25.8b   //ref_main_idx (add row)
    dup         v26.8b,w9

    st1         {v18.8b},[x2], x3           //st (row 3)
    rshrn       v24.8b, v24.8h,#5           //round shft (row 4)

    add         x2, x2, x3, lsl #2
    sub         v25.8b,  v25.8b ,  v2.8b    //ref_main_idx -1 (sub 1)
    add         x20, x7, x2
    csel        x2, x20, x2,gt

    sub         x20, x2, x4
    csel        x2, x20, x2,le

    subs        x10, x10, #8                //subtract 8 and go to end if 8x8

    bne         kernel_8_16_32

epil_8_16_32:
    tbl         v23.8b, {v0.16b},v4.8b      //load from ref_main_idx (row 7)

    umull       v20.8h, v14.8b, v7.8b       //mul (row 6)
    tbl         v25.8b, {v0.16b},v5.8b      //load from ref_main_idx + 1 (row 7)
    umlal       v20.8h, v15.8b, v6.8b       //mul (row 6)

    st1         {v24.8b},[x5], x3           //st (row 4)
    rshrn       v24.8b, v22.8h,#5           //round shft (row 5)

    umull       v18.8h, v23.8b, v7.8b       //mul (row 7)
    umlal       v18.8h, v25.8b, v6.8b       //mul (row 7)

    st1         {v24.8b},[x5], x3           //(from previous loop)st (row 5)
    rshrn       v20.8b, v20.8h,#5           //(from previous loop)round shft (row 6)

    st1         {v20.8b},[x5], x3           //(from previous loop)st (row 6)
    rshrn       v18.8b, v18.8h,#5           //(from previous loop)round shft (row 7)

    st1         {v18.8b},[x5], x3           //st (row 7)

    b           end_func

sz_4_proc:
    ld1         {v31.8b},[x14]
    movi        v2.8b, #1                   //contains #1 for adding to get ref_main_idx - 1

    movi        v3.8b, #2
    adrp        x12, :got:idx_neg_idx_3_9   //load least idx table
    ldr         x12, [x12, #:got_lo12:idx_neg_idx_3_9]

    smull       v22.8h, v30.8b, v31.8b      //(col+1)*intra_pred_angle [0:7](col)
    sub         x7, x5, #3

    add         x12, x12, x7, lsl #4
    mov         x8, x12

    ldr         w9,  [x8]
    sxtw        x9,w9

    dup         v26.8b,w9                   //least idx added to final idx values
    add         x6, x0, x4, lsl #1          //pu1_ref + 2nt

    xtn         v6.8b,  v22.8h
    sub         x6, x6, #9                  //ref_main_idx + 2nt - (8 + 1)(two_nt - idx - row ) for 8 & 8 - 1row
    sub         x6, x6, x9

    ld1         {v0.16b}, [x6]              //stores the 32 values reqd based on indices values (from least idx)

    movi        v29.8b, #31                 //contains #31 for vand operation

    movi        v28.8b, #32

    sshr        v22.8h, v22.8h,#5
    sqxtn       v1.8b,  v22.8h

    and         v6.8b,  v6.8b ,  v29.8b     //fract values in d1/ idx values in d0
    sub         v7.8b,  v28.8b ,  v6.8b     //32-fract

    movi        v27.8b, #7                  //row 0 to 7(row-1)
    sub         v1.8b,  v1.8b ,  v2.8b      //ref_main_idx (add 1)
    sub         v1.8b,  v26.8b ,  v1.8b     //ref_main_idx
    add         v1.8b,  v1.8b ,  v27.8b     //t0 compensate the pu1_src idx incremented by 8
    sub         v19.8b,  v1.8b ,  v2.8b     //ref_main_idx - 1

    sub         v4.8b,  v1.8b ,  v2.8b      //row 1 ref_main_idx
    sub         v5.8b,  v19.8b ,  v2.8b

    tbl         v12.8b, {v0.16b},v1.8b      //load from ref_main_idx (row 0)
    tbl         v13.8b, {v0.16b},v19.8b     //load from ref_main_idx + 1 (row 0)


    umull       v24.8h, v12.8b, v7.8b       //mul (row 0)
    tbl         v16.8b, {v0.16b},v4.8b      //load from ref_main_idx    (row 1)
    umlal       v24.8h, v13.8b, v6.8b       //mul (row 0)

    sub         v1.8b,  v1.8b ,  v3.8b      //idx (row 2)
    tbl         v17.8b, {v0.16b},v5.8b      //load from ref_main_idx + 1 (row 1)
    sub         v19.8b,  v19.8b ,  v3.8b    //idx+1 (row 2)

    umull       v22.8h, v16.8b, v7.8b       //mul (row 1)
    tbl         v12.8b, {v0.16b},v1.8b      //load from ref_main_idx    (row 2)
    umlal       v22.8h, v17.8b, v6.8b       //mul (row 1)

    rshrn       v24.8b, v24.8h,#5           //round shift (row 0)

    sub         v4.8b,  v4.8b ,  v3.8b      //idx (row 3)
    tbl         v13.8b, {v0.16b},v19.8b     //load from ref_main_idx + 1 (row 2)
    sub         v5.8b,  v5.8b ,  v3.8b      //idx+1 (row 3)

    umull       v20.8h, v12.8b, v7.8b       //mul (row 2)
    tbl         v16.8b, {v0.16b},v4.8b      //load from ref_main_idx (row 3)
    umlal       v20.8h, v13.8b, v6.8b       //mul (row 2)

    st1         {v24.s}[0],[x2], x3         //st row 0
    rshrn       v22.8b, v22.8h,#5           //round shift (row 1)

    tbl         v17.8b, {v0.16b},v5.8b      //load from ref_main_idx + 1 (row 3)

    umull       v18.8h, v16.8b, v7.8b       //mul (row 3)
    umlal       v18.8h, v17.8b, v6.8b       //mul (row 3)

    st1         {v22.s}[0],[x2], x3         //st row 1
    rshrn       v20.8b, v20.8h,#5           //round shift (row 2)

    st1         {v20.s}[0],[x2], x3         //st row 2

    rshrn       v18.8b, v18.8h,#5           //round shift (row 3)

    st1         {v18.s}[0],[x2], x3         //st (row 3)

end_func:
    // ldmfd sp!,{x4-x12,x15}          //reload the registers from sp
    ldp         x19, x20,[sp],#16
    ldp         d14,d15,[sp],#16
    ldp         d12,d13,[sp],#16
    ret




