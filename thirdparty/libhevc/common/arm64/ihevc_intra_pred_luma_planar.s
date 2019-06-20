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
//*  ihevc_intra_pred_filters_planar.s
//*
//* @brief
//*  contains function definitions for inter prediction  interpolation.
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
//*    luma intraprediction filter for planar input
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

//void ihevc_intra_pred_luma_planar(uword8* pu1_ref,
//                                  word32 src_strd,
//                                  uword8* pu1_dst,
//                                  word32 dst_strd,
//                                  word32 nt,
//                                  word32 mode,
//                   word32 pi1_coeff)
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



.globl ihevc_intra_pred_luma_planar_av8
.extern gau1_ihevc_planar_factor
.extern gau1_ihevc_planar_factor_1

.type ihevc_intra_pred_luma_planar_av8, %function

ihevc_intra_pred_luma_planar_av8:

    // stmfd sp!, {x4-x12, x14}            //stack stores the values of the arguments

    stp         x19, x20,[sp,#-16]!

    adrp        x11, :got:gau1_ihevc_planar_factor //loads table of coeffs
    ldr         x11, [x11, #:got_lo12:gau1_ihevc_planar_factor]

    clz         w5,w4
    sub         x20, x5, #32
    neg         x5, x20
    dup         v29.8h,w5
    neg         v29.8h, v29.8h              //shr value (so vneg)
    dup         v2.8b,w4                    //nt
    dup         v16.8h,w4                   //nt

    sub         x6, x4, #1                  //nt-1
    add         x6, x6, x0
    ldr         w7,  [x6]
    sxtw        x7,w7
    dup         v0.8b,w7                    //src[nt-1]

    add         x6, x4, x4,lsl #1           //3nt
    add         x6, x6, #1                  //3nt + 1
    add         x6, x6, x0
    ldr         w7,  [x6]
    sxtw        x7,w7
    dup         v1.8b,w7                    //src[3nt+1]

    add         x6, x4, x4                  //2nt
    add         x14, x6, #1                 //2nt+1
    sub         x6, x6, #1                  //2nt-1
    add         x6, x6, x0                  //&src[2nt-1]
    add         x14, x14, x0                //&src[2nt+1]

    mov         x8, #1                      //row+1 (row is first 0)
    sub         x9, x4, x8                  //nt-1-row (row is first 0)

    dup         v5.8b,w8                    //row + 1
    dup         v6.8b,w9                    //nt - 1 - row
    mov         v7.8b, v5.8b                //mov #1 to d7 to used for inc for row+1 and dec for nt-1-row

    add         x12, x11, #1                //coeffs (to be reloaded after every row)
    mov         x1, x4                      //nt (row counter) (dec after every row)
    mov         x5, x2                      //dst (to be reloaded after every row and inc by dst_strd)
    mov         x10, #8                     //increment for the coeffs
    mov         x0, x14                     //&src[2nt+1] (to be reloaded after every row)

    cmp         x4, #4
    beq         tf_sz_4

//@ ========== ***************** =====================
prolog:
tf_sz_8_16_32:

    mov         x7, x4                      //column counter (set to no of cols)
    lsr         x9, x4, #3                  //divide nt by 8
    mul         x7, x7, x9                  //multiply width * height
    adrp        x5, :got:gau1_ihevc_planar_factor_1 //loads table of coeffs
    ldr         x5, [x5, #:got_lo12:gau1_ihevc_planar_factor_1]
    sub         x6, x6, #7
    mov         x8, x2
    lsl         x9, x3, #3                  //4*stride
    sub         x20, x9, #8                 //8-4*stride
    neg         x9, x20
    mov         x10, x4                     //nt
    sub         x10, x10, #8                //nt - 8

col_loop_8_16_32:

    ld1         {v17.8b},[x12]              //(1-8)load 8 coeffs [col+1]
    dup         v27.8h,w4                   //(1)
    ld1         {v4.8b},[x6]                //(1-8)src[2nt-1-row]
    sub         v19.8b,  v2.8b ,  v17.8b    //(1-8)[nt-1-col]


    umlal       v27.8h, v5.8b, v0.8b        //(1)(row+1)    *    src[nt-1]

    ld1         {v3.8b},[x14]               //(1-8)load 8 src[2nt+1+col]
    umlal       v27.8h, v17.8b, v1.8b       //(1)(col+1)    *    src[3nt+1]

    dup         v20.8b, v4.b[7]             //(1)
    umlal       v27.8h, v6.8b, v3.8b        //(1)(nt-1-row)    *    src[2nt+1+col]

    dup         v21.8b, v4.b[6]             //(2)
    umlal       v27.8h, v19.8b, v20.8b      //(1)(nt-1-col)    *    src[2nt-1-row]

    dup         v30.8h,w4                   //(2)
    add         v5.8b,  v5.8b ,  v7.8b      //(1)

    sub         v6.8b,  v6.8b ,  v7.8b      //(1)

    dup         v22.8b, v4.b[5]             //(3)
    umlal       v30.8h, v5.8b, v0.8b        //(2)

    dup         v28.8h,w4                   //(3)
    umlal       v30.8h, v17.8b, v1.8b       //(2)

    umlal       v30.8h, v6.8b, v3.8b        //(2)
    umlal       v30.8h, v19.8b, v21.8b      //(2)

    sshl        v27.8h, v27.8h, v29.8h      //(1)shr

    add         v5.8b,  v5.8b ,  v7.8b      //(2)
    sub         v6.8b,  v6.8b ,  v7.8b      //(2)

    xtn         v27.8b,  v27.8h             //(1)
    umlal       v28.8h, v5.8b, v0.8b        //(3)

    dup         v23.8b, v4.b[4]             //(4)
    umlal       v28.8h, v17.8b, v1.8b       //(3)

    dup         v25.8h,w4                   //(4)
    umlal       v28.8h, v6.8b, v3.8b        //(3)

    st1         {v27.8b},[x2], x3           //(1)str 8 values
    umlal       v28.8h, v19.8b, v22.8b      //(3)

    sshl        v30.8h, v30.8h, v29.8h      //(2)shr

    add         v5.8b,  v5.8b ,  v7.8b      //(3)
    sub         v6.8b,  v6.8b ,  v7.8b      //(3)

    xtn         v30.8b,  v30.8h             //(2)
    umlal       v25.8h, v5.8b, v0.8b        //(4)

    dup         v20.8b, v4.b[3]             //(5)
    umlal       v25.8h, v17.8b, v1.8b       //(4)

    dup         v16.8h,w4                   //(5)
    umlal       v25.8h, v6.8b, v3.8b        //(4)

    st1         {v30.8b},[x2], x3           //(2)str 8 values
    umlal       v25.8h, v19.8b, v23.8b      //(4)

    sshl        v28.8h, v28.8h, v29.8h      //(3)shr

    add         v5.8b,  v5.8b ,  v7.8b      //(4)
    sub         v6.8b,  v6.8b ,  v7.8b      //(4)

    xtn         v28.8b,  v28.8h             //(3)
    umlal       v16.8h, v5.8b, v0.8b        //(5)

    dup         v21.8b, v4.b[2]             //(6)
    umlal       v16.8h, v17.8b, v1.8b       //(5)

    dup         v18.8h,w4                   //(6)
    umlal       v16.8h, v6.8b, v3.8b        //(5)

    st1         {v28.8b},[x2], x3           //(3)str 8 values
    umlal       v16.8h, v19.8b, v20.8b      //(5)

    sshl        v25.8h, v25.8h, v29.8h      //(4)shr
    add         v5.8b,  v5.8b ,  v7.8b      //(5)
    sub         v6.8b,  v6.8b ,  v7.8b      //(5)

    xtn         v25.8b,  v25.8h             //(4)
    umlal       v18.8h, v5.8b, v0.8b        //(6)

    dup         v22.8b, v4.b[1]             //(7)
    umlal       v18.8h, v17.8b, v1.8b       //(6)

    dup         v26.8h,w4                   //(7)
    umlal       v18.8h, v6.8b, v3.8b        //(6)

    st1         {v25.8b},[x2], x3           //(4)str 8 values
    umlal       v18.8h, v19.8b, v21.8b      //(6)

    sshl        v16.8h, v16.8h, v29.8h      //(5)shr

    add         v5.8b,  v5.8b ,  v7.8b      //(6)
    sub         v6.8b,  v6.8b ,  v7.8b      //(6)

    xtn         v16.8b,  v16.8h             //(5)
    umlal       v26.8h, v5.8b, v0.8b        //(7)

    dup         v23.8b, v4.b[0]             //(8)
    umlal       v26.8h, v17.8b, v1.8b       //(7)

    dup         v24.8h,w4                   //(8)
    umlal       v26.8h, v6.8b, v3.8b        //(7)

    st1         {v16.8b},[x2], x3           //(5)str 8 values
    umlal       v26.8h, v19.8b, v22.8b      //(7)

    sshl        v18.8h, v18.8h, v29.8h      //(6)shr

    add         v5.8b,  v5.8b ,  v7.8b      //(7)
    sub         v6.8b,  v6.8b ,  v7.8b      //(7)

    xtn         v18.8b,  v18.8h             //(6)
    umlal       v24.8h, v5.8b, v0.8b        //(8)


    umlal       v24.8h, v17.8b, v1.8b       //(8)

    umlal       v24.8h, v6.8b, v3.8b        //(8)

    st1         {v18.8b},[x2], x3           //(6)str 8 values
    umlal       v24.8h, v19.8b, v23.8b      //(8)

    sshl        v26.8h, v26.8h, v29.8h      //(7)shr

    subs        x7, x7, #8

    beq         epilog

    subs        x1, x1, #8                  //row counter
    add         x20, x12, #8                //col inc
    csel        x12, x20, x12,gt
    add         x20, x14, #8                //also for col inc
    csel        x14, x20, x14,gt
    csel        x1, x4, x1,le               //nt reloaded (refresh the value)
    add         x20, x11, #1                //x12 reset
    csel        x12, x20, x12,le

    csel        x14, x0, x14,le             //x14 reset
    ld1         {v17.8b},[x12]              //(1n)(1-8)load 8 coeffs [col+1]

    sub         x20, x6, #8                 //for next set of rows
    csel        x6, x20, x6,le
    ld1         {v3.8b},[x14]               //(1n)(1-8)load 8 src[2nt+1+col]

    add         x20, x5, #8
    csel        x5, x20, x5,le
    dup         v27.8h,w4                   //(1n)(1)

    ld1         {v5.8b},[x5]

    ld1         {v4.8b},[x6]                //(1n)(1-8)src[2nt-1-row]
    sub         v19.8b,  v2.8b ,  v17.8b    //(1n)(1-8)[nt-1-col]

    dup         v20.8b, v4.b[7]             //(1n)(1)
    sub         v6.8b,  v2.8b ,  v5.8b

    beq         epilog

kernel_plnr:

    cmp         x1, #0                      // (cond loop)
    sshl        v24.8h, v24.8h, v29.8h      //(8)shr

    xtn         v26.8b,  v26.8h             //(7)
    umlal       v27.8h, v5.8b, v0.8b        //(1)(row+1)    *    src[nt-1]

    xtn         v24.8b,  v24.8h             //(8)
    umlal       v27.8h, v17.8b, v1.8b       //(1)(col+1)    *    src[3nt+1]

    dup         v21.8b, v4.b[6]             //(2)
    umlal       v27.8h, v6.8b, v3.8b        //(1)(nt-1-row)    *    src[2nt+1+col]

    dup         v30.8h,w4                   //(2)
    umlal       v27.8h, v19.8b, v20.8b      //(1)(nt-1-col)    *    src[2nt-1-row]

    st1         {v26.8b},[x2], x3           //(7)str 8 values
    add         v5.8b,  v5.8b ,  v7.8b      //(1)

    st1         {v24.8b},[x2], x3           //(8)str 8 values
    sub         v6.8b,  v6.8b ,  v7.8b      //(1)

    add         x20, x2, x9                 //since more cols to fill, dst + 8 - 6*strd (cond loop)
    csel        x2, x20, x2,gt
    umlal       v30.8h, v5.8b, v0.8b        //(2)

    sub         x20, x2, x10                //else go to next set of rows, dst - (nt-8) (cond loop)
    csel        x2, x20, x2,le
    umlal       v30.8h, v17.8b, v1.8b       //(2)

    dup         v22.8b, v4.b[5]             //(3)
    umlal       v30.8h, v6.8b, v3.8b        //(2)

    dup         v28.8h,w4                   //(3)
    umlal       v30.8h, v19.8b, v21.8b      //(2)

    sshl        v27.8h, v27.8h, v29.8h      //(1)shr

    add         v5.8b,  v5.8b ,  v7.8b      //(2)
    csel        x1, x4, x1,le               //nt reloaded (refresh the value)    (cond loop)

    sub         v6.8b,  v6.8b ,  v7.8b      //(2)
    subs        x1, x1, #8                  //row counter (loop)

    xtn         v27.8b,  v27.8h             //(1)
    umlal       v28.8h, v5.8b, v0.8b        //(3)

    dup         v23.8b, v4.b[4]             //(4)
    umlal       v28.8h, v17.8b, v1.8b       //(3)

    dup         v25.8h,w4                   //(4)
    umlal       v28.8h, v6.8b, v3.8b        //(3)

    st1         {v27.8b},[x2], x3           //(1)str 8 values
    umlal       v28.8h, v19.8b, v22.8b      //(3)

    sshl        v30.8h, v30.8h, v29.8h      //(2)shr

    add         v5.8b,  v5.8b ,  v7.8b      //(3)

    sub         v6.8b,  v6.8b ,  v7.8b      //(3)

    xtn         v30.8b,  v30.8h             //(2)
    umlal       v25.8h, v5.8b, v0.8b        //(4)

    dup         v20.8b, v4.b[3]             //(5)
    umlal       v25.8h, v17.8b, v1.8b       //(4)

    dup         v16.8h,w4                   //(5)
    umlal       v25.8h, v6.8b, v3.8b        //(4)

    st1         {v30.8b},[x2], x3           //(2)str 8 values
    umlal       v25.8h, v19.8b, v23.8b      //(4)

    sshl        v28.8h, v28.8h, v29.8h      //(3)shr

    add         v5.8b,  v5.8b ,  v7.8b      //(4)

    sub         v6.8b,  v6.8b ,  v7.8b      //(4)

    xtn         v28.8b,  v28.8h             //(3)
    umlal       v16.8h, v5.8b, v0.8b        //(5)

    dup         v21.8b, v4.b[2]             //(6)
    umlal       v16.8h, v17.8b, v1.8b       //(5)

    dup         v18.8h,w4                   //(6)
    umlal       v16.8h, v6.8b, v3.8b        //(5)

    st1         {v28.8b},[x2], x3           //(3)str 8 values
    umlal       v16.8h, v19.8b, v20.8b      //(5)

    add         x20, x11, #1                //x12 reset (cond loop)
    csel        x12, x20, x12,le
    sshl        v25.8h, v25.8h, v29.8h      //(4)shr

    add         x20, x12, #8                //col inc (cond loop)
    csel        x12, x20, x12,gt
    add         v5.8b,  v5.8b ,  v7.8b      //(5)

    add         x20, x14, #8                //also for col inc (cond loop)
    csel        x14, x20, x14,gt
    sub         v6.8b,  v6.8b ,  v7.8b      //(5)

    xtn         v25.8b,  v25.8h             //(4)
    umlal       v18.8h, v5.8b, v0.8b        //(6)

    dup         v22.8b, v4.b[1]             //(7)
    umlal       v18.8h, v17.8b, v1.8b       //(6)

    dup         v26.8h,w4                   //(7)
    umlal       v18.8h, v6.8b, v3.8b        //(6)

    st1         {v25.8b},[x2], x3           //(4)str 8 values
    umlal       v18.8h, v19.8b, v21.8b      //(6)

    csel        x14, x0, x14,le             //x14 reset (cond loop)
    sshl        v16.8h, v16.8h, v29.8h      //(5)shr

    sub         x20, x6, #8                 //for next set of rows (cond loop)
    csel        x6, x20, x6,le
    add         v5.8b,  v5.8b ,  v7.8b      //(6)

    add         x20, x5, #8                 // (cond loop)
    csel        x5, x20, x5,le
    sub         v6.8b,  v6.8b ,  v7.8b      //(6)

    xtn         v16.8b,  v16.8h             //(5)
    umlal       v26.8h, v5.8b, v0.8b        //(7)

    dup         v23.8b, v4.b[0]             //(8)
    umlal       v26.8h, v17.8b, v1.8b       //(7)

    dup         v24.8h,w4                   //(8)
    umlal       v26.8h, v6.8b, v3.8b        //(7)

    st1         {v16.8b},[x2], x3           //(5)str 8 values
    umlal       v26.8h, v19.8b, v22.8b      //(7)

    ld1         {v4.8b},[x6]                //(1n)(1-8)src[2nt-1-row]
    sshl        v18.8h, v18.8h, v29.8h      //(6)shr

    add         v5.8b,  v5.8b ,  v7.8b      //(7)

    sub         v6.8b,  v6.8b ,  v7.8b      //(7)

    xtn         v18.8b,  v18.8h             //(6)
    umlal       v24.8h, v5.8b, v0.8b        //(8)

    ld1         {v5.8b},[x5]                //(row+1 value)
    umlal       v24.8h, v17.8b, v1.8b       //(8)

    dup         v20.8b, v4.b[7]             //(1n)(1)
    umlal       v24.8h, v6.8b, v3.8b        //(8)

    st1         {v18.8b},[x2], x3           //(6)str 8 values
    umlal       v24.8h, v19.8b, v23.8b      //(8)

    ld1         {v17.8b},[x12]              //(1n)(1-8)load 8 coeffs [col+1]
    sub         v6.8b,  v2.8b ,  v5.8b      //(nt-1-row) value

    subs        x7, x7, #8                  //col counter

    ld1         {v3.8b},[x14]               //(1n)(1-8)load 8 src[2nt+1+col]
    sshl        v26.8h, v26.8h, v29.8h      //(7)shr

    dup         v27.8h,w4                   //(1n)(1)
    sub         v19.8b,  v2.8b ,  v17.8b    //(1n)(1-8)[nt-1-col]

    bne         kernel_plnr

epilog:

    xtn         v26.8b,  v26.8h             //(7)
    st1         {v26.8b},[x2], x3           //(7)str 8 values

    sshl        v24.8h, v24.8h, v29.8h      //(8)shr
    xtn         v24.8b,  v24.8h             //(8)
    st1         {v24.8b},[x2], x3           //(8)str 8 values

//@ ========== ***************** =====================

    beq         end_loop

tf_sz_4:
    ld1         {v25.8b},[x14]              //load src[2nt+1+col]
    ld1         {v17.8b},[x12], x10         //load 8 coeffs [col+1]
loop_sz_4:
    mov         x10, #4                     //reduce inc to #4 for 4x4
    ldr         w7,  [x6], #-1              //src[2nt-1-row] (dec to take into account row)
    sxtw        x7,w7
    dup         v4.8b,w7                    //src[2nt-1-row]

    sub         v19.8b,  v2.8b ,  v17.8b    //[nt-1-col]

    umull       v27.8h, v5.8b, v0.8b        //(row+1)    *    src[nt-1]
    umlal       v27.8h, v6.8b, v25.8b       //(nt-1-row)    *    src[2nt+1+col]
    umlal       v27.8h, v17.8b, v1.8b       //(col+1)    *    src[3nt+1]
    umlal       v27.8h, v19.8b, v4.8b       //(nt-1-col)    *    src[2nt-1-row]
//    vadd.i16    q6, q6, q8            @add (nt)
//    vshl.s16     q6, q6, q7            @shr
//    vmovn.i16     d12, q6
    rshrn       v27.8b, v27.8h,#3
    st1         {v27.s}[0],[x2], x3

    add         v5.8b,  v5.8b ,  v7.8b      //row++ [(row+1)++]
    sub         v6.8b,  v6.8b ,  v7.8b      //[nt-1-row]--
    subs        x1, x1, #1

    bne         loop_sz_4

end_loop:
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret








