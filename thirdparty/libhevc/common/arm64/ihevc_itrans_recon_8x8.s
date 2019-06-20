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
// *******************************************************************************
// * @file
// *  ihevc_itrans_recon_8x8_neon.s
// *
// * @brief
// *  contains function definitions for single stage  inverse transform
// *
// * @author
// *  anand s
// *
// * @par list of functions:
// *  - ihevc_itrans_recon_8x8()
// *
// * @remarks
// *  none
// *
// *******************************************************************************
//*/

///**
// *******************************************************************************
// *
// * @brief
// *  this function performs inverse transform  and reconstruction for 8x8
// * input block
// *
// * @par description:
// *  performs inverse transform and adds the prediction  data and clips output
// * to 8 bit
// *
// * @param[in] pi2_src
// *  input 8x8 coefficients
// *
// * @param[in] pi2_tmp
// *  temporary 8x8 buffer for storing inverse
// *
// *  transform
// *  1st stage output
// *
// * @param[in] pu1_pred
// *  prediction 8x8 block
// *
// * @param[out] pu1_dst
// *  output 8x8 block
// *
// * @param[in] src_strd
// *  input stride
// *
// * @param[in] pred_strd
// *  prediction stride
// *
// * @param[in] dst_strd
// *  output stride
// *
// * @param[in] shift
// *  output shift
// *
// * @param[in] zero_cols
// *  zero columns in pi2_src
// *
// * @returns  void
// *
// * @remarks
// *  none
// *
// *******************************************************************************
// */

//void ihevc_itrans_recon_8x8(word16 *pi2_src,
//                            word16 *pi2_tmp,
//                            uword8 *pu1_pred,
//                            uword8 *pu1_dst,
//                            word32 src_strd,
//                            word32 pred_strd,
//                            word32 dst_strd,
//                            word32 zero_cols
//                             word32    zero_rows                )

//**************variables vs registers*************************
//    x0 => *pi2_src
//    x1 => *pi2_tmp
//    x2 => *pu1_pred
//    x3 => *pu1_dst
//    src_strd
//    pred_strd
//    dst_strd
//    zero_cols



.text
.align 4
.include "ihevc_neon_macros.s"



.set width_x_size_x5 ,   40
.set width_x_size_x2 ,   32
.set shift_stage1_idct ,   7
.set shift_stage2_idct ,   12

.globl ihevc_itrans_recon_8x8_av8

.extern g_ai2_ihevc_trans_8_transpose

.type ihevc_itrans_recon_8x8_av8, %function

ihevc_itrans_recon_8x8_av8:
////register usage.extern        - loading and until idct of columns
////    cosine constants     -     d0
////    sine constants         -     d1
////    row 0 first half     -     d2        -    y0
////    row 1 first half     -     d6        -    y1
////    row 2 first half     -     d3        -    y2
////    row 3 first half     -     d7        -    y3
////    row 4 first half     -     d10        -    y4
////    row 5 first half     -     d14        -    y5
////    row 6 first half     -     d11        -    y6
////    row 7 first half     -     d15        -    y7

////    row 0 second half    -     d4        -    y0
////    row 1 second half    -     d8      -    y1
////    row 2 second half    -     d5      -    y2
////    row 3 second half    -     d9      -    y3
////    row 4 second half    -     d12     -    y4
////    row 5 second half    -     d16     -    y5
////    row 6 second half    -     d13     -    y6
////    row 7 second half    -     d17     -    y7

    //// copy the input pointer to another register
    //// step 1 : load all constants
    // stmfd sp!,{x4-x12,x14}

    ldr         w11, [sp]                   // zero rows

    push_v_regs
    stp         x19, x20,[sp,#-16]!

    mov         x12, x7 // zero columns
    mov         x8, x5 // prediction stride
    mov         x7, x6 // destination stride
    mov         x6, x4 // src stride
    lsl         x6, x6, #1                  // x sizeof(word16)
    add         x9,x0,x6, lsl #1            // 2 rows

    add         x10,x6,x6, lsl #1           // 3 rows

    sub         x10,x10, #8                 // - 4 cols * sizeof(word16)
    sub         x5,x6, #8                   // src_strd - 4 cols * sizeof(word16)

    adrp        x14, :got:g_ai2_ihevc_trans_8_transpose
    ldr         x14, [x14, #:got_lo12:g_ai2_ihevc_trans_8_transpose]

    ld1         {v0.4h, v1.4h},[x14]        ////d0,d1 are used for storing the constant data

    ////step 2 load all the input data
    ////step 3 operate first 4 colums at a time

    and         x11,x11,#0xff
    and         x12,x12,#0xff

    cmp         x11,#0xf0
    bge         skip_last4_rows


    ld1         {v2.4h},[x0],#8
    ld1         {v3.4h},[x9],#8
    ld1         {v4.4h},[x0],x5
    smull       v20.4s, v2.4h, v0.h[0]      //// y0 * cos4(part of c0 and c1)
    ld1         {v5.4h},[x9],x5
    smull       v18.4s, v3.4h, v1.h[2]      //// y2 * sin2 (q3 is freed by this time)(part of d1)
    ld1         {v6.4h},[x0],#8
    ld1         {v7.4h},[x9],#8
    smull       v24.4s, v6.4h, v0.h[1]      //// y1 * cos1(part of b0)
    ld1         {v8.4h},[x0],x10
    smull       v26.4s, v6.4h, v0.h[3]      //// y1 * cos3(part of b1)
    ld1         {v9.4h},[x9],x10
    smull       v28.4s, v6.4h, v1.h[1]      //// y1 * sin3(part of b2)
    ld1         {v10.4h},[x0],#8
    smull       v30.4s, v6.4h, v1.h[3]      //// y1 * sin1(part of b3)
    ld1         {v11.4h},[x9],#8
    smlal       v24.4s, v7.4h, v0.h[3]      //// y1 * cos1 + y3 * cos3(part of b0)
    ld1         {v12.4h},[x0],x5
    smlsl       v26.4s, v7.4h, v1.h[3]      //// y1 * cos3 - y3 * sin1(part of b1)
    ld1         {v13.4h},[x9],x5
    smlsl       v28.4s, v7.4h, v0.h[1]      //// y1 * sin3 - y3 * cos1(part of b2)
    ld1         {v14.4h},[x0],#8
    smlsl       v30.4s, v7.4h, v1.h[1]      //// y1 * sin1 - y3 * sin3(part of b3)
    ld1         {v15.4h},[x9],#8
    smull       v22.4s, v10.4h, v0.h[0]     //// y4 * cos4(part of c0 and c1)
    ld1         {v16.4h},[x0],x10
    smull       v6.4s, v3.4h, v0.h[2]       //// y2 * cos2(part of d0)
    ld1         {v17.4h},[x9],x10

    ///* this following was activated when alignment is not there */
////    vld1.16        d2,[x0]!
////    vld1.16        d3,[x2]!
////    vld1.16        d4,[x0]!
////    vld1.16        d5,[x2]!
////    vld1.16        d6,[x0]!
////    vld1.16        d7,[x2]!
////    vld1.16        d8,[x0],x3
////    vld1.16        d9,[x2],x3
////    vld1.16        d10,[x0]!
////    vld1.16        d11,[x2]!
////    vld1.16        d12,[x0]!
////    vld1.16        d13,[x2]!
////    vld1.16        d14,[x0]!
////    vld1.16        d15,[x2]!
////    vld1.16        d16,[x0],x3
////    vld1.16        d17,[x2],x3




    smlal       v24.4s, v14.4h, v1.h[1]     //// y1 * cos1 + y3 * cos3 + y5 * sin3(part of b0)
    smlsl       v26.4s, v14.4h, v0.h[1]     //// y1 * cos3 - y3 * sin1 - y5 * cos1(part of b1)
    smlal       v28.4s, v14.4h, v1.h[3]     //// y1 * sin3 - y3 * cos1 + y5 * sin1(part of b2)
    smlal       v30.4s, v14.4h, v0.h[3]     //// y1 * sin1 - y3 * sin3 + y5 * cos3(part of b3)

    smlsl       v18.4s, v11.4h, v0.h[2]     //// d1 = y2 * sin2 - y6 * cos2(part of a0 and a1)
    smlal       v6.4s, v11.4h, v1.h[2]      //// d0 = y2 * cos2 + y6 * sin2(part of a0 and a1)

    add         v10.4s,  v20.4s ,  v22.4s   //// c0 = y0 * cos4 + y4 * cos4(part of a0 and a1)
    sub         v20.4s,  v20.4s ,  v22.4s   //// c1 = y0 * cos4 - y4 * cos4(part of a0 and a1)

    smlal       v24.4s, v15.4h, v1.h[3]     //// b0 = y1 * cos1 + y3 * cos3 + y5 * sin3 + y7 * sin1(part of x0,x7)
    smlsl       v26.4s, v15.4h, v1.h[1]     //// b1 = y1 * cos3 - y3 * sin1 - y5 * cos1 - y7 * sin3(part of x1,x6)
    smlal       v28.4s, v15.4h, v0.h[3]     //// b2 = y1 * sin3 - y3 * cos1 + y5 * sin1 + y7 * cos3(part of x2,x5)
    smlsl       v30.4s, v15.4h, v0.h[1]     //// b3 = y1 * sin1 - y3 * sin3 + y5 * cos3 - y7 * cos1(part of x3,x4)

    add         v14.4s,  v10.4s ,  v6.4s    ////    a0 = c0 + d0(part of x0,x7)
    sub         v10.4s,  v10.4s ,  v6.4s    //// a3 = c0 - d0(part of x3,x4)
    sub         v22.4s,  v20.4s ,  v18.4s   //// a2 = c1 - d1(part of x2,x5)
    add         v18.4s,  v20.4s ,  v18.4s   //// a1 = c1 + d1(part of x1,x6)

    add         v20.4s,  v14.4s ,  v24.4s   //// a0 + b0(part of x0)
    sub         v6.4s,  v14.4s ,  v24.4s    //// a0 - b0(part of x7)

    add         v24.4s,  v22.4s ,  v28.4s   //// a2 + b2(part of x2)
    sub         v22.4s,  v22.4s ,  v28.4s   //// a2 - b2(part of x5)

    add         v28.4s,  v18.4s ,  v26.4s   //// a1 + b1(part of x1)
    sub         v18.4s,  v18.4s ,  v26.4s   //// a1 - b1(part of x6)

    add         v26.4s,  v10.4s ,  v30.4s   //// a3 + b3(part of x3)
    sub         v30.4s,  v10.4s ,  v30.4s   //// a3 - b3(part of x4)

    sqrshrn     v2.4h, v20.4s,#shift_stage1_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v15.4h, v6.4s,#shift_stage1_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v3.4h, v24.4s,#shift_stage1_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v14.4h, v22.4s,#shift_stage1_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v6.4h, v28.4s,#shift_stage1_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v11.4h, v18.4s,#shift_stage1_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v7.4h, v26.4s,#shift_stage1_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v10.4h, v30.4s,#shift_stage1_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)


    b           last4_cols



skip_last4_rows:



    ld1         {v2.4h},[x0],#8
    ld1         {v3.4h},[x9],#8
    ld1         {v4.4h},[x0],x5
    ld1         {v5.4h},[x9],x5
    ld1         {v6.4h},[x0],#8
    ld1         {v7.4h},[x9],#8
    ld1         {v8.4h},[x0],x10
    ld1         {v9.4h},[x9],x10



    movi        v12.4h, #0
    movi        v13.4h, #0
    movi        v16.4h, #0
    movi        v17.4h, #0




    smull       v24.4s, v6.4h, v0.h[1]      //// y1 * cos1(part of b0)
    smull       v26.4s, v6.4h, v0.h[3]      //// y1 * cos3(part of b1)
    smull       v28.4s, v6.4h, v1.h[1]      //// y1 * sin3(part of b2)
    smull       v30.4s, v6.4h, v1.h[3]      //// y1 * sin1(part of b3)

    smlal       v24.4s, v7.4h, v0.h[3]      //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v7.4h, v1.h[3]      //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v7.4h, v0.h[1]      //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v7.4h, v1.h[1]      //// y1 * sin1 - y3 * sin3(part of b3)

    smull       v18.4s, v3.4h, v1.h[2]      //// y2 * sin2 (q3 is freed by this time)(part of d1)
    smull       v6.4s, v3.4h, v0.h[2]       //// y2 * cos2(part of d0)

    smull       v20.4s, v2.4h, v0.h[0]      //// y0 * cos4(part of c0 and c1)


    add         v14.4s,  v20.4s ,  v6.4s    ////    a0 = c0 + d0(part of x0,x7)
    sub         v10.4s,  v20.4s ,  v6.4s    //// a3 = c0 - d0(part of x3,x4)
    sub         v22.4s,  v20.4s ,  v18.4s   //// a2 = c1 - d1(part of x2,x5)
    add         v18.4s,  v20.4s ,  v18.4s   //// a1 = c1 + d1(part of x1,x6)

    add         v20.4s,  v14.4s ,  v24.4s   //// a0 + b0(part of x0)
    sub         v6.4s,  v14.4s ,  v24.4s    //// a0 - b0(part of x7)

    add         v24.4s,  v22.4s ,  v28.4s   //// a2 + b2(part of x2)
    sub         v22.4s,  v22.4s ,  v28.4s   //// a2 - b2(part of x5)

    add         v28.4s,  v18.4s ,  v26.4s   //// a1 + b1(part of x1)
    sub         v18.4s,  v18.4s ,  v26.4s   //// a1 - b1(part of x6)

    add         v26.4s,  v10.4s ,  v30.4s   //// a3 + b3(part of x3)
    sub         v30.4s,  v10.4s ,  v30.4s   //// a3 - b3(part of x4)

    sqrshrn     v2.4h, v20.4s,#shift_stage1_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v15.4h, v6.4s,#shift_stage1_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v3.4h, v24.4s,#shift_stage1_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v14.4h, v22.4s,#shift_stage1_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v6.4h, v28.4s,#shift_stage1_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v11.4h, v18.4s,#shift_stage1_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v7.4h, v26.4s,#shift_stage1_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v10.4h, v30.4s,#shift_stage1_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)


last4_cols:


    cmp         x12,#0xf0
    bge         skip_last4cols

    smull       v24.4s, v8.4h, v0.h[1]      //// y1 * cos1(part of b0)
    smull       v26.4s, v8.4h, v0.h[3]      //// y1 * cos3(part of b1)
    smull       v28.4s, v8.4h, v1.h[1]      //// y1 * sin3(part of b2)
    smull       v30.4s, v8.4h, v1.h[3]      //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v0.h[3]      //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v1.h[3]      //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v0.h[1]      //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v1.h[1]      //// y1 * sin1 - y3 * sin3(part of b3)

    smull       v18.4s, v5.4h, v1.h[2]      //// y2 * sin2 (q4 is freed by this time)(part of d1)
    smull       v8.4s, v5.4h, v0.h[2]       //// y2 * cos2(part of d0)

    smull       v20.4s, v4.4h, v0.h[0]      //// y0 * cos4(part of c0 and c1)
    smull       v22.4s, v12.4h, v0.h[0]     //// y4 * cos4(part of c0 and c1)

    smlal       v24.4s, v16.4h, v1.h[1]     //// y1 * cos1 + y3 * cos3 + y5 * sin3(part of b0)
    smlsl       v26.4s, v16.4h, v0.h[1]     //// y1 * cos3 - y3 * sin1 - y5 * cos1(part of b1)
    smlal       v28.4s, v16.4h, v1.h[3]     //// y1 * sin3 - y3 * cos1 + y5 * sin1(part of b2)
    smlal       v30.4s, v16.4h, v0.h[3]     //// y1 * sin1 - y3 * sin3 + y5 * cos3(part of b3)

    smlsl       v18.4s, v13.4h, v0.h[2]     //// d1 = y2 * sin2 - y6 * cos2(part of a0 and a1)
    smlal       v8.4s, v13.4h, v1.h[2]      //// d0 = y2 * cos2 + y6 * sin2(part of a0 and a1)

    add         v12.4s,  v20.4s ,  v22.4s   //// c0 = y0 * cos4 + y4 * cos4(part of a0 and a1)
    sub         v20.4s,  v20.4s ,  v22.4s   //// c1 = y0 * cos4 - y4 * cos4(part of a0 and a1)

    smlal       v24.4s, v17.4h, v1.h[3]     //// b0 = y1 * cos1 + y3 * cos3 + y5 * sin3 + y7 * sin1(part of e0,e7)
    smlsl       v26.4s, v17.4h, v1.h[1]     //// b1 = y1 * cos3 - y3 * sin1 - y5 * cos1 - y7 * sin3(part of e1,e6)
    smlal       v28.4s, v17.4h, v0.h[3]     //// b2 = y1 * sin3 - y3 * cos1 + y5 * sin1 + y7 * cos3(part of e2,e5)
    smlsl       v30.4s, v17.4h, v0.h[1]     //// b3 = y1 * sin1 - y3 * sin3 + y5 * cos3 - y7 * cos1(part of e3,e4)

    add         v16.4s,  v12.4s ,  v8.4s    ////    a0 = c0 + d0(part of e0,e7)
    sub         v12.4s,  v12.4s ,  v8.4s    //// a3 = c0 - d0(part of e3,e4)
    sub         v22.4s,  v20.4s ,  v18.4s   //// a2 = c1 - d1(part of e2,e5)
    add         v18.4s,  v20.4s ,  v18.4s   //// a1 = c1 + d1(part of e1,e6)

    add         v20.4s,  v16.4s ,  v24.4s   //// a0 + b0(part of e0)
    sub         v8.4s,  v16.4s ,  v24.4s    //// a0 - b0(part of e7)

    add         v24.4s,  v22.4s ,  v28.4s   //// a2 + b2(part of e2)
    sub         v22.4s,  v22.4s ,  v28.4s   //// a2 - b2(part of e5)

    add         v28.4s,  v18.4s ,  v26.4s   //// a1 + b1(part of e1)
    sub         v18.4s,  v18.4s ,  v26.4s   //// a1 - b1(part of e6)

    add         v26.4s,  v12.4s ,  v30.4s   //// a3 + b3(part of e3)
    sub         v30.4s,  v12.4s ,  v30.4s   //// a3 - b3(part of x4)

    sqrshrn     v4.4h, v20.4s,#shift_stage1_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v17.4h, v8.4s,#shift_stage1_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v5.4h, v24.4s,#shift_stage1_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v16.4h, v22.4s,#shift_stage1_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v8.4h, v28.4s,#shift_stage1_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v13.4h, v18.4s,#shift_stage1_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v9.4h, v26.4s,#shift_stage1_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v12.4h, v30.4s,#shift_stage1_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)
    b           end_skip_last4cols



skip_last4cols:

    umov        x15,v25.d[0]

    trn1        v25.4h, v2.4h, v6.4h
    trn2        v29.4h, v2.4h, v6.4h        ////[x3,x1],[x2,x0] first qudrant transposing

    trn1        v27.4h, v3.4h, v7.4h
    trn2        v31.4h, v3.4h, v7.4h        ////[x3,x1],[x2,x0] first qudrant transposing

    trn1        v6.2s, v29.2s, v31.2s
    trn2        v7.2s, v29.2s, v31.2s       ////x0,x1,x2,x3 first qudrant transposing continued.....
    trn1        v2.2s, v25.2s, v27.2s
    trn2        v3.2s, v25.2s, v27.2s       ////x0,x1,x2,x3 first qudrant transposing continued.....


    trn1        v25.4h, v10.4h, v14.4h
    trn2        v29.4h, v10.4h, v14.4h      ////[x7,x5],[x6,x4] third qudrant transposing

    trn1        v27.4h, v11.4h, v15.4h
    trn2        v31.4h, v11.4h, v15.4h      ////[x7,x5],[x6,x4] third qudrant transposing

    trn1        v10.2s, v25.2s, v27.2s
    trn2        v11.2s, v25.2s, v27.2s      ////x4,x5,x6,x7 third qudrant transposing continued.....
    trn1        v14.2s, v29.2s, v31.2s
    trn2        v15.2s, v29.2s, v31.2s      ////x4,x5,x6,x7 third qudrant transposing continued.....

    mov         v25.d[0],x15

    smull       v24.4s, v6.4h, v0.h[1]      //// y1 * cos1(part of b0)
    smull       v26.4s, v6.4h, v0.h[3]      //// y1 * cos3(part of b1)
    smull       v28.4s, v6.4h, v1.h[1]      //// y1 * sin3(part of b2)
    smull       v30.4s, v6.4h, v1.h[3]      //// y1 * sin1(part of b3)

    smlal       v24.4s, v7.4h, v0.h[3]      //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v7.4h, v1.h[3]      //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v7.4h, v0.h[1]      //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v7.4h, v1.h[1]      //// y1 * sin1 - y3 * sin3(part of b3)

    smull       v20.4s, v2.4h, v0.h[0]      //// y0 * cos4(part of c0 and c1)
//    vmull.s16    q11,d4,d0[0]                    @// y4 * cos4(part of c0 and c1)

    smull       v18.4s, v3.4h, v1.h[2]      //// y2 * sin2 (q3 is freed by this time)(part of d1)
    smull       v6.4s, v3.4h, v0.h[2]       //// y2 * cos2(part of d0)




    sub         v22.4s,  v20.4s ,  v6.4s    //// a3 = c0 - d0(part of x3,x4)
    add         v4.4s,  v20.4s ,  v6.4s     ////    a0 = c0 + d0(part of x0,x7)


    add         v2.4s,  v4.4s ,  v24.4s

    sub         v6.4s,  v4.4s ,  v24.4s

    add         v8.4s,  v22.4s ,  v30.4s

    sub         v24.4s,  v22.4s ,  v30.4s

    sqrshrn     v5.4h, v8.4s,#shift_stage2_idct
    sqrshrn     v2.4h, v2.4s,#shift_stage2_idct
    sqrshrn     v9.4h, v6.4s,#shift_stage2_idct
    sqrshrn     v6.4h, v24.4s,#shift_stage2_idct

    sub         v22.4s,  v20.4s ,  v18.4s   //// a2 = c1 - d1(part of x2,x5)
    add         v18.4s,  v20.4s ,  v18.4s   //// a1 = c1 + d1(part of x1,x6)


    add         v30.4s,  v22.4s ,  v28.4s

    sub         v24.4s,  v22.4s ,  v28.4s

    add         v28.4s,  v18.4s ,  v26.4s

    sub         v22.4s,  v18.4s ,  v26.4s
    sqrshrn     v4.4h, v30.4s,#shift_stage2_idct
    sqrshrn     v7.4h, v24.4s,#shift_stage2_idct
    sqrshrn     v3.4h, v28.4s,#shift_stage2_idct
    sqrshrn     v8.4h, v22.4s,#shift_stage2_idct



    umov        x19,v25.d[0]
    umov        x20,v25.d[1]

    trn1        v27.4h, v2.4h, v3.4h
    trn2        v29.4h, v2.4h, v3.4h
    trn1        v25.4h, v4.4h, v5.4h
    trn2        v31.4h, v4.4h, v5.4h

    trn1        v2.2s, v27.2s, v25.2s
    trn2        v4.2s, v27.2s, v25.2s
    trn1        v3.2s, v29.2s, v31.2s
    trn2        v5.2s, v29.2s, v31.2s

    trn1        v27.4h, v6.4h, v7.4h
    trn2        v29.4h, v6.4h, v7.4h
    trn1        v25.4h, v8.4h, v9.4h
    trn2        v31.4h, v8.4h, v9.4h

    trn1        v6.2s, v27.2s, v25.2s
    trn2        v8.2s, v27.2s, v25.2s
    trn1        v7.2s, v29.2s, v31.2s
    trn2        v9.2s, v29.2s, v31.2s

    mov         v25.d[0],x19
    mov         v25.d[1],x20

    smull       v24.4s, v14.4h, v0.h[1]     //// y1 * cos1(part of b0)

    smull       v26.4s, v14.4h, v0.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v14.4h, v1.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v14.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v15.4h, v0.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v15.4h, v1.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v15.4h, v0.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v15.4h, v1.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)
    smull       v20.4s, v10.4h, v0.h[0]     //// y0 * cos4(part of c0 and c1)
    smull       v18.4s, v11.4h, v1.h[2]     //// y2 * sin2 (q7 is freed by this time)(part of d1)
    smull       v14.4s, v11.4h, v0.h[2]     //// y2 * cos2(part of d0)


    add         x4,x2,x8, lsl #1            // x4 = x2 + pred_strd * 2    => x4 points to 3rd row of pred data


    add         x5,x8,x8, lsl #1            //


    add         x0,x3,x7, lsl #1            // x0 points to 3rd row of dest data


    add         x10,x7,x7, lsl #1           //

    // swapping v3 and v6
    mov         v31.d[0], v3.d[0]
    mov         v3.d[0], v6.d[0]
    mov         v6.d[0], v31.d[0]

    // swapping v5 and v8
    mov         v31.d[0], v5.d[0]
    mov         v5.d[0], v8.d[0]
    mov         v8.d[0], v31.d[0]


    sub         v22.4s,  v20.4s ,  v14.4s   //// a3 = c0 - d0(part of x3,x4)
    add         v12.4s,  v20.4s ,  v14.4s   ////    a0 = c0 + d0(part of x0,x7)


    add         v0.4s,  v12.4s ,  v24.4s


    sub         v24.4s,  v12.4s ,  v24.4s


    add         v12.4s,  v22.4s ,  v30.4s


    sub         v14.4s,  v22.4s ,  v30.4s

    sqrshrn     v10.4h, v0.4s,#shift_stage2_idct
    sqrshrn     v17.4h, v24.4s,#shift_stage2_idct
    sqrshrn     v13.4h, v12.4s,#shift_stage2_idct
    sqrshrn     v14.4h, v14.4s,#shift_stage2_idct

    sub         v22.4s,  v20.4s ,  v18.4s   //// a2 = c1 - d1(part of x2,x5)
    add         v18.4s,  v20.4s ,  v18.4s   //// a1 = c1 + d1(part of x1,x6)


    add         v0.4s,  v22.4s ,  v28.4s


    sub         v24.4s,  v22.4s ,  v28.4s


    add         v28.4s,  v18.4s ,  v26.4s


    sub         v26.4s,  v18.4s ,  v26.4s
    ld1         {v18.8b},[x2],x8

    sqrshrn     v12.4h, v0.4s,#shift_stage2_idct
    ld1         {v20.8b},[x2],x5


    sqrshrn     v15.4h, v24.4s,#shift_stage2_idct
    ld1         {v19.8b},[x2],x8




    sqrshrn     v11.4h, v28.4s,#shift_stage2_idct
    ld1         {v22.8b},[x4],x8




    sqrshrn     v16.4h, v26.4s,#shift_stage2_idct
    ld1         {v21.8b},[x2],x5


    b           pred_buff_addition
end_skip_last4cols:


    umov        x19,v25.d[0]
    umov        x20,v25.d[1]

///* now the idct of columns is done, transpose so that row idct done efficiently(step5) */
    trn1        v27.4h, v2.4h, v6.4h
    trn2        v29.4h, v2.4h, v6.4h        ////[x3,x1],[x2,x0] first qudrant transposing
    trn1        v25.4h, v3.4h, v7.4h
    trn2        v31.4h, v3.4h, v7.4h        ////[x3,x1],[x2,x0] first qudrant transposing

    trn1        v2.2s, v27.2s, v25.2s
    trn2        v3.2s, v27.2s, v25.2s       ////x0,x1,x2,x3 first qudrant transposing continued.....
    trn1        v6.2s, v29.2s, v31.2s
    trn2        v7.2s, v29.2s, v31.2s       ////x0,x1,x2,x3 first qudrant transposing continued.....

    trn1        v27.4h, v4.4h, v8.4h
    trn2        v29.4h, v4.4h, v8.4h        ////[x3,x1],[x2,x0] second qudrant transposing
    trn1        v25.4h, v5.4h, v9.4h
    trn2        v31.4h, v5.4h, v9.4h        ////[x3,x1],[x2,x0] second qudrant transposing

    trn1        v4.2s, v27.2s, v25.2s
    trn2        v5.2s, v27.2s, v25.2s       ////x0,x1,x2,x3 second qudrant transposing continued.....
    trn1        v8.2s, v29.2s, v31.2s
    trn2        v9.2s, v29.2s, v31.2s       ////x0,x1,x2,x3 second qudrant transposing continued.....

    trn1        v27.4h, v10.4h, v14.4h
    trn2        v29.4h, v10.4h, v14.4h      ////[x7,x5],[x6,x4] third qudrant transposing
    trn1        v25.4h, v11.4h, v15.4h
    trn2        v31.4h, v11.4h, v15.4h      ////[x7,x5],[x6,x4] third qudrant transposing

    trn1        v10.2s, v27.2s, v25.2s
    trn2        v11.2s, v27.2s, v25.2s      ////x4,x5,x6,x7 third qudrant transposing continued.....
    trn1        v14.2s, v29.2s, v31.2s
    trn2        v15.2s, v29.2s, v31.2s      ////x4,x5,x6,x7 third qudrant transposing continued.....

    trn1        v27.4h, v12.4h, v16.4h
    trn2        v29.4h, v12.4h, v16.4h      ////[x7,x5],[x6,x4] fourth qudrant transposing
    trn1        v25.4h, v13.4h, v17.4h
    trn2        v31.4h, v13.4h, v17.4h      ////[x7,x5],[x6,x4] fourth qudrant transposing

    trn1        v12.2s, v27.2s, v25.2s
    trn2        v13.2s, v27.2s, v25.2s      ////x4,x5,x6,x7 fourth qudrant transposing continued.....
    trn1        v16.2s, v29.2s, v31.2s
    trn2        v17.2s, v29.2s, v31.2s      ////x4,x5,x6,x7 fourth qudrant transposing continued.....

    mov         v25.d[0],x19
    mov         v25.d[1],x20

    ////step6 operate on first four rows and find their idct
    ////register usage.extern        - storing and idct of rows
////    cosine constants     -     d0
////    sine constants         -     d1
////    element 0 first four     -     d2        -    y0
////    element 1 first four     -     d6        -    y1
////    element 2 first four     -     d3        -    y2
////    element 3 first four     -     d7        -    y3
////    element 4 first four     -     d4        -    y4
////    element 5 first four     -     d8        -    y5
////    element 6 first four     -     d5        -    y6
////    element 7 first four     -     d9        -    y7
////    element 0 second four    -     d10        -    y0
////    element 1 second four    -     d14     -    y1
////    element 2 second four    -     d11     -    y2
////    element 3 second four    -     d15     -    y3
////    element 4 second four    -     d12     -    y4
////    element 5 second four    -     d16     -    y5
////    element 6 second four    -     d13     -    y6
////    element 7 second four    -     d17     -    y7

    //// map between first kernel code seq and current
////        d2    ->    d2
////        d6    ->    d6
////        d3    ->    d3
////        d7    ->    d7
////        d10    ->    d4
////        d14    ->    d8
////        d11    ->    d5
////        d15    ->    d9
////        q3    ->    q3
////        q5    ->    q2
////        q7    ->    q4

    smull       v24.4s, v6.4h, v0.h[1]      //// y1 * cos1(part of b0)
    smull       v26.4s, v6.4h, v0.h[3]      //// y1 * cos3(part of b1)
    smull       v28.4s, v6.4h, v1.h[1]      //// y1 * sin3(part of b2)
    smull       v30.4s, v6.4h, v1.h[3]      //// y1 * sin1(part of b3)

    smlal       v24.4s, v7.4h, v0.h[3]      //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v7.4h, v1.h[3]      //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v7.4h, v0.h[1]      //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v7.4h, v1.h[1]      //// y1 * sin1 - y3 * sin3(part of b3)

    smull       v20.4s, v2.4h, v0.h[0]      //// y0 * cos4(part of c0 and c1)
    smull       v22.4s, v4.4h, v0.h[0]      //// y4 * cos4(part of c0 and c1)

    smull       v18.4s, v3.4h, v1.h[2]      //// y2 * sin2 (q3 is freed by this time)(part of d1)
    smull       v6.4s, v3.4h, v0.h[2]       //// y2 * cos2(part of d0)


    smlal       v24.4s, v8.4h, v1.h[1]      //// y1 * cos1 + y3 * cos3 + y5 * sin3(part of b0)
    smlsl       v26.4s, v8.4h, v0.h[1]      //// y1 * cos3 - y3 * sin1 - y5 * cos1(part of b1)
    smlal       v28.4s, v8.4h, v1.h[3]      //// y1 * sin3 - y3 * cos1 + y5 * sin1(part of b2)
    smlal       v30.4s, v8.4h, v0.h[3]      //// y1 * sin1 - y3 * sin3 + y5 * cos3(part of b3)

    smlsl       v18.4s, v5.4h, v0.h[2]      //// d1 = y2 * sin2 - y6 * cos2(part of a0 and a1)
    smlal       v6.4s, v5.4h, v1.h[2]       //// d0 = y2 * cos2 + y6 * sin2(part of a0 and a1)

    add         v2.4s,  v20.4s ,  v22.4s    //// c0 = y0 * cos4 + y4 * cos4(part of a0 and a1)
    sub         v20.4s,  v20.4s ,  v22.4s   //// c1 = y0 * cos4 - y4 * cos4(part of a0 and a1)

    smlal       v24.4s, v9.4h, v1.h[3]      //// b0 = y1 * cos1 + y3 * cos3 + y5 * sin3 + y7 * sin1(part of x0,x7)
    smlsl       v26.4s, v9.4h, v1.h[1]      //// b1 = y1 * cos3 - y3 * sin1 - y5 * cos1 - y7 * sin3(part of x1,x6)
    smlal       v28.4s, v9.4h, v0.h[3]      //// b2 = y1 * sin3 - y3 * cos1 + y5 * sin1 + y7 * cos3(part of x2,x5)
    smlsl       v30.4s, v9.4h, v0.h[1]      //// b3 = y1 * sin1 - y3 * sin3 + y5 * cos3 - y7 * cos1(part of x3,x4)

    sub         v22.4s,  v2.4s ,  v6.4s     //// a3 = c0 - d0(part of x3,x4)
    add         v4.4s,  v2.4s ,  v6.4s      ////    a0 = c0 + d0(part of x0,x7)


    add         v2.4s,  v4.4s ,  v24.4s

    sub         v6.4s,  v4.4s ,  v24.4s

    add         v8.4s,  v22.4s ,  v30.4s

    sub         v24.4s,  v22.4s ,  v30.4s

    sqrshrn     v5.4h, v8.4s,#shift_stage2_idct
    sqrshrn     v2.4h, v2.4s,#shift_stage2_idct
    sqrshrn     v9.4h, v6.4s,#shift_stage2_idct
    sqrshrn     v6.4h, v24.4s,#shift_stage2_idct

    sub         v22.4s,  v20.4s ,  v18.4s   //// a2 = c1 - d1(part of x2,x5)
    add         v18.4s,  v20.4s ,  v18.4s   //// a1 = c1 + d1(part of x1,x6)


    add         v30.4s,  v22.4s ,  v28.4s

    sub         v24.4s,  v22.4s ,  v28.4s

    add         v28.4s,  v18.4s ,  v26.4s

    sub         v22.4s,  v18.4s ,  v26.4s
    sqrshrn     v4.4h, v30.4s,#shift_stage2_idct
    sqrshrn     v7.4h, v24.4s,#shift_stage2_idct
    sqrshrn     v3.4h, v28.4s,#shift_stage2_idct
    sqrshrn     v8.4h, v22.4s,#shift_stage2_idct



    umov        x19,v25.d[0]
    umov        x20,v25.d[1]

    trn1        v27.4h, v2.4h, v3.4h
    trn2        v29.4h, v2.4h, v3.4h
    trn1        v25.4h, v4.4h, v5.4h
    trn2        v31.4h, v4.4h, v5.4h

    trn1        v2.2s, v27.2s, v25.2s
    trn2        v4.2s, v27.2s, v25.2s
    trn1        v3.2s, v29.2s, v31.2s
    trn2        v5.2s, v29.2s, v31.2s

    trn1        v27.4h, v6.4h, v7.4h
    trn2        v29.4h, v6.4h, v7.4h
    trn1        v25.4h, v8.4h, v9.4h
    trn2        v31.4h, v8.4h, v9.4h

    trn1        v6.2s, v27.2s, v25.2s
    trn2        v8.2s, v27.2s, v25.2s
    trn1        v7.2s, v29.2s, v31.2s
    trn2        v9.2s, v29.2s, v31.2s

    mov         v25.d[0],x19
    mov         v25.d[1],x20



    smull       v24.4s, v14.4h, v0.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v14.4h, v0.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v14.4h, v1.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v14.4h, v1.h[3]     //// y1 * sin1(part of b3)
    smlal       v24.4s, v15.4h, v0.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v15.4h, v1.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v15.4h, v0.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v15.4h, v1.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)
    smull       v20.4s, v10.4h, v0.h[0]     //// y0 * cos4(part of c0 and c1)
    smull       v22.4s, v12.4h, v0.h[0]     //// y4 * cos4(part of c0 and c1)
    smull       v18.4s, v11.4h, v1.h[2]     //// y2 * sin2 (q7 is freed by this time)(part of d1)
    smull       v14.4s, v11.4h, v0.h[2]     //// y2 * cos2(part of d0)
    smlal       v24.4s, v16.4h, v1.h[1]     //// y1 * cos1 + y3 * cos3 + y5 * sin3(part of b0)

    add         x4,x2,x8, lsl #1            // x4 = x2 + pred_strd * 2    => x4 points to 3rd row of pred data
    smlsl       v26.4s, v16.4h, v0.h[1]     //// y1 * cos3 - y3 * sin1 - y5 * cos1(part of b1)

    add         x5,x8,x8, lsl #1            //
    smlal       v28.4s, v16.4h, v1.h[3]     //// y1 * sin3 - y3 * cos1 + y5 * sin1(part of b2)

    add         x0,x3,x7, lsl #1            // x0 points to 3rd row of dest data
    smlal       v30.4s, v16.4h, v0.h[3]     //// y1 * sin1 - y3 * sin3 + y5 * cos3(part of b3)

    add         x10,x7,x7, lsl #1           //
    smlsl       v18.4s, v13.4h, v0.h[2]     //// d1 = y2 * sin2 - y6 * cos2(part of a0 and a1)


    smlal       v14.4s, v13.4h, v1.h[2]     //// d0 = y2 * cos2 + y6 * sin2(part of a0 and a1)

    add         v12.4s,  v20.4s ,  v22.4s   //// c0 = y0 * cos4 + y4 * cos4(part of a0 and a1)
    sub         v20.4s,  v20.4s ,  v22.4s   //// c1 = y0 * cos4 - y4 * cos4(part of a0 and a1)

    smlal       v24.4s, v17.4h, v1.h[3]     //// b0 = y1 * cos1 + y3 * cos3 + y5 * sin3 + y7 * sin1(part of x0,x7)

    // swapping v3 and v6
    mov         v31.d[0], v3.d[0]
    mov         v3.d[0], v6.d[0]
    mov         v6.d[0], v31.d[0]

    smlsl       v26.4s, v17.4h, v1.h[1]     //// b1 = y1 * cos3 - y3 * sin1 - y5 * cos1 - y7 * sin3(part of x1,x6)
    // swapping v5 and v8
    mov         v31.d[0], v5.d[0]
    mov         v5.d[0], v8.d[0]
    mov         v8.d[0], v31.d[0]

    smlal       v28.4s, v17.4h, v0.h[3]     //// b2 = y1 * sin3 - y3 * cos1 + y5 * sin1 + y7 * cos3(part of x2,x5)
    smlsl       v30.4s, v17.4h, v0.h[1]     //// b3 = y1 * sin1 - y3 * sin3 + y5 * cos3 - y7 * cos1(part of x3,x4)

    sub         v22.4s,  v12.4s ,  v14.4s   //// a3 = c0 - d0(part of x3,x4)
    add         v12.4s,  v12.4s ,  v14.4s   ////    a0 = c0 + d0(part of x0,x7)


    add         v0.4s,  v12.4s ,  v24.4s


    sub         v24.4s,  v12.4s ,  v24.4s


    add         v12.4s,  v22.4s ,  v30.4s


    sub         v14.4s,  v22.4s ,  v30.4s

    sqrshrn     v10.4h, v0.4s,#shift_stage2_idct
    sqrshrn     v17.4h, v24.4s,#shift_stage2_idct
    sqrshrn     v13.4h, v12.4s,#shift_stage2_idct
    sqrshrn     v14.4h, v14.4s,#shift_stage2_idct

    sub         v22.4s,  v20.4s ,  v18.4s   //// a2 = c1 - d1(part of x2,x5)
    add         v18.4s,  v20.4s ,  v18.4s   //// a1 = c1 + d1(part of x1,x6)


    add         v0.4s,  v22.4s ,  v28.4s


    sub         v24.4s,  v22.4s ,  v28.4s


    add         v28.4s,  v18.4s ,  v26.4s


    sub         v26.4s,  v18.4s ,  v26.4s
    ld1         {v18.8b},[x2],x8

    sqrshrn     v12.4h, v0.4s,#shift_stage2_idct
    ld1         {v20.8b},[x2],x5


    sqrshrn     v15.4h, v24.4s,#shift_stage2_idct
    ld1         {v19.8b},[x2],x8




    sqrshrn     v11.4h, v28.4s,#shift_stage2_idct
    ld1         {v22.8b},[x4],x8




    sqrshrn     v16.4h, v26.4s,#shift_stage2_idct
    ld1         {v21.8b},[x2],x5




pred_buff_addition:

    umov        x19,v25.d[0]
    umov        x20,v25.d[1]

    trn1        v27.4h, v10.4h, v11.4h
    trn2        v29.4h, v10.4h, v11.4h
    trn1        v25.4h, v12.4h, v13.4h
    trn2        v31.4h, v12.4h, v13.4h

    trn1        v10.2s, v27.2s, v25.2s
    trn2        v12.2s, v27.2s, v25.2s
    trn1        v11.2s, v29.2s, v31.2s
    trn2        v13.2s, v29.2s, v31.2s

    trn1        v27.4h, v14.4h, v15.4h
    trn2        v29.4h, v14.4h, v15.4h
    trn1        v25.4h, v16.4h, v17.4h
    trn2        v31.4h, v16.4h, v17.4h

    trn1        v14.2s, v27.2s, v25.2s
    trn2        v16.2s, v27.2s, v25.2s
    trn1        v15.2s, v29.2s, v31.2s
    trn2        v17.2s, v29.2s, v31.2s


    mov         v25.d[0],x19
    mov         v25.d[1],x20


    ld1         {v24.8b},[x4],x5
    ld1         {v23.8b},[x4],x8
    ld1         {v25.8b},[x4],x5
    mov         v2.d[1], v3.d[0]
    mov         v4.d[1], v5.d[0]
    mov         v6.d[1], v7.d[0]
    mov         v8.d[1], v9.d[0]
    uaddw       v2.8h,  v2.8h ,  v18.8b
    uaddw       v4.8h,  v4.8h ,  v22.8b
    uaddw       v6.8h,  v6.8h ,  v20.8b
    uaddw       v8.8h,  v8.8h ,  v24.8b

    // swapping v11 and v14
    mov         v31.d[0], v11.d[0]
    mov         v11.d[0], v14.d[0]
    mov         v14.d[0], v31.d[0]

    // swapping v13 and v16
    mov         v31.d[0], v13.d[0]
    mov         v13.d[0], v16.d[0]
    mov         v16.d[0], v31.d[0]
// row values stored in the q register.

//q1 :x0
//q3: x1
//q2: x2
//q4: x3
//q5: x4
//q7: x5
//q6: x6
//q8: x7



///// adding the prediction buffer









    // load prediction data





    //adding recon with prediction




    mov         v10.d[1], v11.d[0]
    mov         v12.d[1], v13.d[0]
    mov         v14.d[1], v15.d[0]
    mov         v16.d[1], v17.d[0]
    uaddw       v10.8h,  v10.8h ,  v19.8b
    sqxtun      v2.8b, v2.8h
    uaddw       v14.8h,  v14.8h ,  v21.8b
    sqxtun      v4.8b, v4.8h
    uaddw       v12.8h,  v12.8h ,  v23.8b
    sqxtun      v6.8b, v6.8h
    uaddw       v16.8h,  v16.8h ,  v25.8b
    sqxtun      v8.8b, v8.8h







    st1         {v2.8b},[x3],x7
    sqxtun      v10.8b, v10.8h
    st1         {v6.8b},[x3],x10
    sqxtun      v14.8b, v14.8h
    st1         {v4.8b},[x0],x7
    sqxtun      v12.8b, v12.8h
    st1         {v8.8b},[x0],x10
    sqxtun      v16.8b, v16.8h







    st1         {v10.8b},[x3],x7
    st1         {v14.8b},[x3],x10
    st1         {v12.8b},[x0],x7
    st1         {v16.8b},[x0],x10




    // ldmfd sp!,{x4-x12,pc}
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret





