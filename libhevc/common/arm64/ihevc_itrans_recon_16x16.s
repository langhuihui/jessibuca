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
// * anand s
// *
// * @par list of functions:
// *  - ihevc_itrans_recon_16x16()
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
// *  input 16x16 coefficients
// *
// * @param[in] pi2_tmp
// *  temporary 16x16 buffer for storing inverse
// *
// *  transform
// *  1st stage output
// *
// * @param[in] pu1_pred
// *  prediction 16x16 block
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
// * @param[in] x12
// *  zero columns in pi2_src
// *
// * @returns  void
// *
// * @remarks
// *  none
// *
// *******************************************************************************
// */

//void ihevc_itrans_recon_16x16(word16 *pi2_src,
//                            word16 *pi2_tmp,
//                            uword8 *pu1_pred,
//                            uword8 *pu1_dst,
//                            word32 src_strd,
//                            word32 pred_strd,
//                            word32 dst_strd,
//                            word32 x12
//                             word32    x11                )

//**************variables vs registers*************************
//    x0 => *pi2_src
//    x1 => *pi2_tmp
//    x2 => *pu1_pred
//    x3 => *pu1_dst
//    src_strd
//    pred_strd
//    dst_strd
//    x12
//    x11

.text
.align 4

.include "ihevc_neon_macros.s"




.set shift_stage1_idct ,   7
.set shift_stage2_idct ,   12
//#define zero_cols         x12
//#define zero_rows         x11
.globl ihevc_itrans_recon_16x16_av8

.extern g_ai2_ihevc_trans_16_transpose

.type ihevc_itrans_recon_16x16_av8, %function

ihevc_itrans_recon_16x16_av8:

    ldr         w11, [sp]
    // stmfd sp!,{x4-x12,x14}
    push_v_regs
    stp         x19, x20,[sp,#-16]!
    stp         x5, x6,[sp,#-16]!
//    add             sp,sp,#40



//    ldr            x8,[sp,#4]     @ prediction stride
//    ldr            x7,[sp,#8]     @ destination stride
    mov         x6, x4 // src stride
    mov         x12, x7



    adrp        x14, :got:g_ai2_ihevc_trans_16_transpose
    ldr         x14, [x14, #:got_lo12:g_ai2_ihevc_trans_16_transpose]
    ld1         {v0.4h, v1.4h, v2.4h, v3.4h},[x14] ////d0,d1 are used for storing the constant data
    mov         x7,#0xffff
    and         x12,x12,x7
    and         x11,x11,x7
    lsl         x6, x6, #1                  // x sizeof(word16)
    add         x9,x0,x6, lsl #1            // 2 rows

    add         x10,x6,x6, lsl #1           // 3 rows
    add         x5,x6,x6,lsl #2
    mov         x7,#0xfff0

    cmp         x12,x7
    bge         zero_12cols_decision

    mov         x19,#0xff00
    cmp         x12,x19
    bge         zero_8cols_decision




    mov         x14,#4
    cmp         x11,x7
    sub         x20,x6,#0
    neg         x20, x20
    csel        x10,x20,x10,ge

    mov         x19,#0xff00
    cmp         x11,x19
    csel        x8, x5, x8,ge
    sub         x20,x8,#0
    neg         x20, x20
    csel        x8,x20,x8,ge
    csel        x8, x10, x8,lt
    add         x5,x5,x6,lsl #3
    sub         x20,x5,#0
    neg         x5, x20

    b           first_stage_top_four_bottom_four

zero_12cols_decision:
    mov         x14,#1
    mov         x19,#0xff00
    cmp         x11,x19
    csel        x8, x5, x8,ge
    csel        x8, x10, x8,lt
    add         x5,x5,x6,lsl #3
    sub         x20,x5,#0
    neg         x5, x20

    b           first_stage_top_four_bottom_four

zero_8cols_decision:
    mov         x14,#2
    mov         x8,x5
    sub         x20,x8,#0
    neg         x8, x20
    mov         x19,#0xff00
    cmp         x11,x19
    csel        x8, x10, x8,lt
    add         x5,x5,x6,lsl #3
    sub         x20,x5,#0
    neg         x5, x20
    cmp         x11,x7
    sub         x20,x6,#0
    neg         x20, x20
    csel        x10,x20,x10,ge


    b           first_stage_top_four_bottom_four


//d0[0]=    64        d2[0]=64
//d0[1]= 90        d2[1]=57
//d0[2]= 89        d2[2]=50
//d0[3]= 87        d2[3]=43
//d1[0]= 83         d3[0]=36
//d1[1]= 80        d3[1]=25
//d1[2]= 75        d3[2]=18
//d1[3]= 70        d3[3]=9



first_stage:
    add         x0,x0,#8
    add         x9,x9,#8

first_stage_top_four_bottom_four:

    ld1         {v10.4h},[x0],x6
    ld1         {v11.4h},[x9],x6
    ld1         {v6.4h},[x0],x10
    ld1         {v7.4h},[x9],x10
    cmp         x11,x7
    bge         skip_load4rows

    ld1         {v4.4h},[x0],x6
    ld1         {v5.4h},[x9],x6
    ld1         {v8.4h},[x0],x8
    ld1         {v9.4h},[x9],x8

// registers used: q0,q1,q3,q5,q2,q4

// d10 =x0
//d6= x1
//d11=x2
//d7=x3

skip_load4rows:
    smull       v24.4s, v6.4h, v0.h[1]      //// y1 * cos1(part of b0)
    smull       v26.4s, v6.4h, v0.h[3]      //// y1 * cos3(part of b1)
    smull       v28.4s, v6.4h, v1.h[1]      //// y1 * sin3(part of b2)
    smull       v30.4s, v6.4h, v1.h[3]      //// y1 * sin1(part of b3)

    smlal       v24.4s, v7.4h, v0.h[3]      //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v7.4h, v2.h[1]      //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v7.4h, v3.h[3]      //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v7.4h, v2.h[3]      //// y1 * sin1 - y3 * sin3(part of b3)






    smull       v12.4s, v10.4h, v0.h[0]
    smlal       v12.4s, v11.4h, v0.h[2]
    smull       v14.4s, v10.4h, v0.h[0]
    smlal       v14.4s, v11.4h, v1.h[2]
    smull       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v2.h[2]
    smull       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v3.h[2]

    bge         skip_last12rows_kernel1


    smlal       v24.4s, v8.4h, v1.h[1]
    smlal       v26.4s, v8.4h, v3.h[3]
    smlsl       v28.4s, v8.4h, v1.h[3]
    smlsl       v30.4s, v8.4h, v0.h[3]


    smlal       v24.4s, v9.4h, v1.h[3]
    smlsl       v26.4s, v9.4h, v2.h[3]
    smlsl       v28.4s, v9.4h, v0.h[3]
    smlal       v30.4s, v9.4h, v3.h[3]





    smlal       v12.4s, v4.4h, v1.h[0]
    smlal       v12.4s, v5.4h, v1.h[2]
    smlal       v14.4s, v4.4h, v3.h[0]
    smlsl       v14.4s, v5.4h, v3.h[2]
    smlsl       v16.4s, v4.4h, v3.h[0]
    smlsl       v16.4s, v5.4h, v0.h[2]
    smlsl       v18.4s, v4.4h, v1.h[0]
    smlsl       v18.4s, v5.4h, v2.h[2]

//d0[0]=    64        d2[0]=64
//d0[1]= 90        d2[1]=57
//d0[2]= 89        d2[2]=50
//d0[3]= 87        d2[3]=43
//d1[0]= 83         d3[0]=36
//d1[1]= 80        d3[1]=25
//d1[2]= 75        d3[2]=18
//d1[3]= 70        d3[3]=9
    mov         x19,#0xff00
    cmp         x11,x19
    bge         skip_last12rows_kernel1


    ld1         {v10.4h},[x0],x6
    ld1         {v11.4h},[x9],x6
    ld1         {v6.4h},[x0],x10
    ld1         {v7.4h},[x9],x10
    ld1         {v4.4h},[x0],x6
    ld1         {v5.4h},[x9],x6
    ld1         {v8.4h},[x0],x5
    ld1         {v9.4h},[x9],x5




    smlal       v24.4s, v6.4h, v2.h[1]      //// y1 * cos1(part of b0)
    smlsl       v26.4s, v6.4h, v1.h[1]      //// y1 * cos3(part of b1)
    smlsl       v28.4s, v6.4h, v3.h[1]      //// y1 * sin3(part of b2)
    smlal       v30.4s, v6.4h, v0.h[1]      //// y1 * sin1(part of b3)

    smlal       v24.4s, v7.4h, v2.h[3]      //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v7.4h, v0.h[1]      //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v7.4h, v2.h[1]      //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v7.4h, v3.h[1]      //// y1 * sin1 - y3 * sin3(part of b3)



    smlal       v24.4s, v8.4h, v3.h[1]
    smlsl       v26.4s, v8.4h, v1.h[3]
    smlal       v28.4s, v8.4h, v0.h[1]
    smlsl       v30.4s, v8.4h, v1.h[1]


    smlal       v24.4s, v9.4h, v3.h[3]
    smlsl       v26.4s, v9.4h, v3.h[1]
    smlal       v28.4s, v9.4h, v2.h[3]
    smlsl       v30.4s, v9.4h, v2.h[1]





    smlal       v12.4s, v10.4h, v0.h[0]
    smlal       v12.4s, v11.4h, v2.h[2]
    smlal       v12.4s, v4.4h, v3.h[0]
    smlal       v12.4s, v5.4h, v3.h[2]




    smlsl       v14.4s, v10.4h, v0.h[0]
    smlsl       v14.4s, v11.4h, v0.h[2]
    smlsl       v14.4s, v4.4h, v1.h[0]
    smlsl       v14.4s, v5.4h, v2.h[2]


    smlsl       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v3.h[2]
    smlal       v16.4s, v4.4h, v1.h[0]
    smlal       v16.4s, v5.4h, v1.h[2]


    smlal       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v1.h[2]
    smlsl       v18.4s, v4.4h, v3.h[0]
    smlsl       v18.4s, v5.4h, v0.h[2]

skip_last12rows_kernel1:
    add         v20.4s,  v12.4s ,  v24.4s
    sub         v22.4s,  v12.4s ,  v24.4s

    add         v12.4s,  v14.4s ,  v26.4s
    sub         v24.4s,  v14.4s ,  v26.4s

    add         v14.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s







    sqrshrn     v30.4h, v20.4s,#shift_stage1_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v19.4h, v22.4s,#shift_stage1_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v31.4h, v14.4s,#shift_stage1_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v18.4h, v26.4s,#shift_stage1_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v12.4h, v12.4s,#shift_stage1_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v15.4h, v24.4s,#shift_stage1_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v13.4h, v16.4s,#shift_stage1_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v14.4h, v28.4s,#shift_stage1_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    st1         {v30.4h, v31.4h},[x1],#16
    st1         {v18.4h, v19.4h},[x1],#16
    sub         x1,x1,#32

    bge         skip_stage1_kernel_load

first_stage_middle_eight:



    ld1         {v10.4h},[x0],x6
    ld1         {v11.4h},[x9],x6
    ld1         {v6.4h},[x0],x10
    ld1         {v7.4h},[x9],x10
    ld1         {v4.4h},[x0],x6
    ld1         {v5.4h},[x9],x6
    ld1         {v8.4h},[x0],x8
    ld1         {v9.4h},[x9],x8


skip_stage1_kernel_load:
    smull       v24.4s, v6.4h, v2.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v6.4h, v2.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v6.4h, v3.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v6.4h, v3.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v7.4h, v1.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v7.4h, v0.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v7.4h, v1.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v7.4h, v3.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)






    smull       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v3.h[2]
    smull       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v2.h[2]
    smull       v16.4s, v10.4h, v0.h[0]
    smlsl       v16.4s, v11.4h, v1.h[2]
    smull       v18.4s, v10.4h, v0.h[0]
    smlsl       v18.4s, v11.4h, v0.h[2]


    cmp         x11,x7
    bge         skip_last12rows_kernel2

    smlsl       v24.4s, v8.4h, v3.h[1]
    smlal       v26.4s, v8.4h, v2.h[1]
    smlal       v28.4s, v8.4h, v0.h[1]
    smlal       v30.4s, v8.4h, v2.h[3]


    smlal       v24.4s, v9.4h, v0.h[1]
    smlal       v26.4s, v9.4h, v3.h[1]
    smlsl       v28.4s, v9.4h, v1.h[1]
    smlsl       v30.4s, v9.4h, v2.h[1]



    smlsl       v22.4s, v4.4h, v1.h[0]
    smlal       v22.4s, v5.4h, v2.h[2]
    smlsl       v20.4s, v4.4h, v3.h[0]
    smlal       v20.4s, v5.4h, v0.h[2]
    smlal       v16.4s, v4.4h, v3.h[0]
    smlal       v16.4s, v5.4h, v3.h[2]
    smlal       v18.4s, v4.4h, v1.h[0]
    smlsl       v18.4s, v5.4h, v1.h[2]

//d0[0]=    64        d2[0]=64
//d0[1]= 90        d2[1]=57
//d0[2]= 89        d2[2]=50
//d0[3]= 87        d2[3]=43
//d1[0]= 83         d3[0]=36
//d1[1]= 80        d3[1]=25
//d1[2]= 75        d3[2]=18
//d1[3]= 70        d3[3]=9
    mov         x19,#0xff00
    cmp         x11,x19
    bge         skip_last12rows_kernel2

    ld1         {v10.4h},[x0],x6
    ld1         {v11.4h},[x9],x6
    ld1         {v6.4h},[x0],x10
    ld1         {v7.4h},[x9],x10
    ld1         {v4.4h},[x0],x6
    ld1         {v5.4h},[x9],x6
    ld1         {v8.4h},[x0],x5
    ld1         {v9.4h},[x9],x5


    smlsl       v24.4s, v6.4h, v3.h[3]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v6.4h, v0.h[3]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v6.4h, v2.h[3]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v6.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v7.4h, v0.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v7.4h, v1.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v7.4h, v3.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v7.4h, v1.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)


    smlal       v24.4s, v8.4h, v2.h[3]
    smlal       v26.4s, v8.4h, v3.h[3]
    smlsl       v28.4s, v8.4h, v2.h[1]
    smlal       v30.4s, v8.4h, v0.h[3]


    smlal       v24.4s, v9.4h, v1.h[3]
    smlsl       v26.4s, v9.4h, v1.h[1]
    smlal       v28.4s, v9.4h, v0.h[3]
    smlsl       v30.4s, v9.4h, v0.h[1]




    smlal       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v1.h[2]
    smlsl       v22.4s, v4.4h, v3.h[0]
    smlal       v22.4s, v5.4h, v0.h[2]



    smlsl       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v3.h[2]
    smlal       v20.4s, v4.4h, v1.h[0]
    smlsl       v20.4s, v5.4h, v1.h[2]


    smlsl       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v0.h[2]
    smlsl       v16.4s, v4.4h, v1.h[0]
    smlal       v16.4s, v5.4h, v2.h[2]



    smlal       v18.4s, v10.4h, v0.h[0]
    smlsl       v18.4s, v11.4h, v2.h[2]
    smlal       v18.4s, v4.4h, v3.h[0]
    smlsl       v18.4s, v5.4h, v3.h[2]

skip_last12rows_kernel2:

    add         v4.4s,  v22.4s ,  v24.4s
    sub         v22.4s,  v22.4s ,  v24.4s

    add         v6.4s,  v20.4s ,  v26.4s
    sub         v24.4s,  v20.4s ,  v26.4s

    add         v10.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s


    sqrshrn     v18.4h, v4.4s,#shift_stage1_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v31.4h, v22.4s,#shift_stage1_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v19.4h, v10.4s,#shift_stage1_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v30.4h, v26.4s,#shift_stage1_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v20.4h, v6.4s,#shift_stage1_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v23.4h, v24.4s,#shift_stage1_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v21.4h, v16.4s,#shift_stage1_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v22.4h, v28.4s,#shift_stage1_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)


    // registers used:    {q2,q4,q6,q7}, {q9,q15,q10,q11}






    ld1         {v4.4h, v5.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],#16
    sub         x1,x1,#32

//d4=x0
//d12=x1
//d5=x2
//d13=x3

//d18=x4
//d20=x5
//d19=x6
//d21=x7

//d22=x8
//d30=x9
//d23=x10
//d31=x11

//d14=x12
//d8=x13
//d15=x14
//d9=x15

    umov        x15,v26.d[0]
    umov        x16,v27.d[0]
    umov        x19,v28.d[0]
    umov        x20,v29.d[0]

    trn1        v26.4h, v4.4h, v12.4h
    trn2        v27.4h, v4.4h, v12.4h
    trn1        v28.4h, v5.4h, v13.4h
    trn2        v29.4h, v5.4h, v13.4h

    trn1        v4.2s, v26.2s, v28.2s
    trn2        v5.2s, v26.2s, v28.2s
    trn1        v12.2s, v27.2s, v29.2s
    trn2        v13.2s, v27.2s, v29.2s

    trn1        v26.4h, v18.4h, v20.4h
    trn2        v27.4h, v18.4h, v20.4h
    trn1        v28.4h, v19.4h, v21.4h
    trn2        v29.4h, v19.4h, v21.4h

    trn1        v18.2s, v26.2s, v28.2s
    trn2        v19.2s, v26.2s, v28.2s
    trn1        v20.2s, v27.2s, v29.2s
    trn2        v21.2s, v27.2s, v29.2s

    trn1        v26.4h, v22.4h, v30.4h
    trn2        v27.4h, v22.4h, v30.4h
    trn1        v28.4h, v23.4h, v31.4h
    trn2        v29.4h, v23.4h, v31.4h

    trn1        v22.2s, v26.2s, v28.2s
    trn2        v23.2s, v26.2s, v28.2s
    trn1        v30.2s, v27.2s, v29.2s
    trn2        v31.2s, v27.2s, v29.2s

    trn1        v26.4h, v14.4h, v8.4h
    trn2        v27.4h, v14.4h, v8.4h
    trn1        v28.4h, v15.4h, v9.4h
    trn2        v29.4h, v15.4h, v9.4h

    trn1        v14.2s, v26.2s, v28.2s
    trn2        v15.2s, v26.2s, v28.2s
    trn1        v8.2s, v27.2s, v29.2s
    trn2        v9.2s, v27.2s, v29.2s

    mov         v26.d[0],x15
    mov         v27.d[0],x16
    mov         v28.d[0],x19
    mov         v29.d[0],x20

// d4 =x0 1- 4 values
// d5 =x2 1- 4 values
// d12=x1 1- 4 values
// d13=x3 1- 4 values

// d18 =x0 5- 8 values
// d19 =x2 5- 8 values
// d20=x1 5- 8 values
// d21=x3 5- 8 values

// d22 =x0 9- 12 values
// d23 =x2 9- 12 values
// d30=x1 9- 12 values
// d31=x3 9- 12 values

// d14 =x0 13-16 values
// d15 =x2 13- 16 values
// d8=x1 13- 16 values
// d9=x3 13- 16 values


    st1         { v4.4h, v5.4h},[x1],#16
    st1         { v12.4h, v13.4h},[x1],#16

    st1         { v18.4h, v19.4h},[x1],#16
    st1         { v20.4h, v21.4h},[x1],#16
    st1         { v22.4h, v23.4h},[x1],#16
    st1         { v30.4h, v31.4h},[x1],#16
    st1         { v14.4h, v15.4h},[x1],#16
    st1         { v8.4h, v9.4h},[x1],#16


    subs        x14,x14,#1
    bne         first_stage










    mov         x6,x7

    ldp         x8, x7,[sp],#16

    mov         x10,#16

    cmp         x12,x6
    sub         x20,x1,#128
    csel        x1, x20, x1,ge
    bge         label1

    mov         x19,#0xff00
    cmp         x12,x19
    sub         x20,x1,#256
    csel        x1, x20, x1,ge
    bge         label_2

    sub         x1,x1,#512
    sub         x20,x10,#0
    neg         x10, x20

label_2:
    add         x9,x1,#128
    add         x11,x9,#128
    add         x0,x11,#128



label1:
//    mov   x6,x1


    mov         x14,#4
    add         x4,x2,x8, lsl #1            // x4 = x2 + pred_strd * 2    => x4 points to 3rd row of pred data
    add         x5,x8,x8, lsl #1            //
//    add x0,x3,x7, lsl #1    @ x0 points to 3rd row of dest data
//    add x10,x7,x7, lsl #1    @




second_stage:
    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v6.4h, v7.4h},[x1],x10
    cmp         x12,x6
    bge         second_stage_process
    ld1         {v4.4h, v5.4h},[x9],#16
    ld1         {v8.4h, v9.4h},[x9],x10

second_stage_process:


    smull       v24.4s, v6.4h, v0.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v6.4h, v0.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v6.4h, v1.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v6.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v7.4h, v0.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v7.4h, v2.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v7.4h, v3.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v7.4h, v2.h[3]     //// y1 * sin1 - y3 * sin3(part of b3)


    smull       v12.4s, v10.4h, v0.h[0]
    smlal       v12.4s, v11.4h, v0.h[2]
    smull       v14.4s, v10.4h, v0.h[0]
    smlal       v14.4s, v11.4h, v1.h[2]
    smull       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v2.h[2]
    smull       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v3.h[2]

    bge         skip_last8rows_stage2_kernel1

    smlal       v24.4s, v8.4h, v1.h[1]
    smlal       v26.4s, v8.4h, v3.h[3]
    smlsl       v28.4s, v8.4h, v1.h[3]
    smlsl       v30.4s, v8.4h, v0.h[3]


    smlal       v24.4s, v9.4h, v1.h[3]
    smlsl       v26.4s, v9.4h, v2.h[3]
    smlsl       v28.4s, v9.4h, v0.h[3]
    smlal       v30.4s, v9.4h, v3.h[3]


    smlal       v12.4s, v4.4h, v1.h[0]
    smlal       v12.4s, v5.4h, v1.h[2]
    smlal       v14.4s, v4.4h, v3.h[0]
    smlsl       v14.4s, v5.4h, v3.h[2]
    smlsl       v16.4s, v4.4h, v3.h[0]
    smlsl       v16.4s, v5.4h, v0.h[2]
    smlsl       v18.4s, v4.4h, v1.h[0]
    smlsl       v18.4s, v5.4h, v2.h[2]

    mov         x19,#0xff00
    cmp         x12,x19
    bge         skip_last8rows_stage2_kernel1


    ld1         {v10.4h, v11.4h},[x11],#16
    ld1         {v6.4h, v7.4h},[x11],x10
    ld1         {v4.4h, v5.4h},[x0],#16
    ld1         {v8.4h, v9.4h},[x0],x10





    smlal       v24.4s, v6.4h, v2.h[1]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v6.4h, v1.h[1]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v6.4h, v3.h[1]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v6.4h, v0.h[1]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v7.4h, v2.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v7.4h, v0.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v7.4h, v2.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v7.4h, v3.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)



    smlal       v24.4s, v8.4h, v3.h[1]
    smlsl       v26.4s, v8.4h, v1.h[3]
    smlal       v28.4s, v8.4h, v0.h[1]
    smlsl       v30.4s, v8.4h, v1.h[1]


    smlal       v24.4s, v9.4h, v3.h[3]
    smlsl       v26.4s, v9.4h, v3.h[1]
    smlal       v28.4s, v9.4h, v2.h[3]
    smlsl       v30.4s, v9.4h, v2.h[1]





    smlal       v12.4s, v10.4h, v0.h[0]
    smlal       v12.4s, v11.4h, v2.h[2]
    smlal       v12.4s, v4.4h, v3.h[0]
    smlal       v12.4s, v5.4h, v3.h[2]




    smlsl       v14.4s, v10.4h, v0.h[0]
    smlsl       v14.4s, v11.4h, v0.h[2]
    smlsl       v14.4s, v4.4h, v1.h[0]
    smlsl       v14.4s, v5.4h, v2.h[2]


    smlsl       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v3.h[2]
    smlal       v16.4s, v4.4h, v1.h[0]
    smlal       v16.4s, v5.4h, v1.h[2]


    smlal       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v1.h[2]
    smlsl       v18.4s, v4.4h, v3.h[0]
    smlsl       v18.4s, v5.4h, v0.h[2]






skip_last8rows_stage2_kernel1:



    add         v20.4s,  v12.4s ,  v24.4s
    sub         v22.4s,  v12.4s ,  v24.4s

    add         v12.4s,  v14.4s ,  v26.4s
    sub         v24.4s,  v14.4s ,  v26.4s

    add         v14.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s







    sqrshrn     v30.4h, v20.4s,#shift_stage2_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v19.4h, v22.4s,#shift_stage2_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v31.4h, v14.4s,#shift_stage2_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v18.4h, v26.4s,#shift_stage2_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v12.4h, v12.4s,#shift_stage2_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v15.4h, v24.4s,#shift_stage2_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v13.4h, v16.4s,#shift_stage2_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v14.4h, v28.4s,#shift_stage2_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    bge         skip_stage2_kernel_load

    //q2,q4,q6,q7 is used
    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v6.4h, v7.4h},[x1],#16
    ld1         {v4.4h, v5.4h},[x9],#16
    ld1         {v8.4h, v9.4h},[x9],#16
skip_stage2_kernel_load:
    sub         x1,x1,#32
    st1         {v30.4h, v31.4h},[x1],#16
    st1         {v18.4h, v19.4h},[x1],#16
    sub         x1,x1,#32

    smull       v24.4s, v6.4h, v2.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v6.4h, v2.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v6.4h, v3.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v6.4h, v3.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v7.4h, v1.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v7.4h, v0.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v7.4h, v1.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v7.4h, v3.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)


    smull       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v3.h[2]
    smull       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v2.h[2]
    smull       v16.4s, v10.4h, v0.h[0]
    smlsl       v16.4s, v11.4h, v1.h[2]
    smull       v18.4s, v10.4h, v0.h[0]
    smlsl       v18.4s, v11.4h, v0.h[2]



    cmp         x12,x6
    bge         skip_last8rows_stage2_kernel2


    smlsl       v24.4s, v8.4h, v3.h[1]
    smlal       v26.4s, v8.4h, v2.h[1]
    smlal       v28.4s, v8.4h, v0.h[1]
    smlal       v30.4s, v8.4h, v2.h[3]


    smlal       v24.4s, v9.4h, v0.h[1]
    smlal       v26.4s, v9.4h, v3.h[1]
    smlsl       v28.4s, v9.4h, v1.h[1]
    smlsl       v30.4s, v9.4h, v2.h[1]



    smlsl       v22.4s, v4.4h, v1.h[0]
    smlal       v22.4s, v5.4h, v2.h[2]
    smlsl       v20.4s, v4.4h, v3.h[0]
    smlal       v20.4s, v5.4h, v0.h[2]
    smlal       v16.4s, v4.4h, v3.h[0]
    smlal       v16.4s, v5.4h, v3.h[2]
    smlal       v18.4s, v4.4h, v1.h[0]
    smlsl       v18.4s, v5.4h, v1.h[2]
    mov         x19,#0xff00
    cmp         x12,x19
    bge         skip_last8rows_stage2_kernel2

    ld1         {v10.4h, v11.4h},[x11],#16
    ld1         {v6.4h, v7.4h},[x11],#16
    ld1         {v4.4h, v5.4h},[x0],#16
    ld1         {v8.4h, v9.4h},[x0],#16

    smlsl       v24.4s, v6.4h, v3.h[3]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v6.4h, v0.h[3]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v6.4h, v2.h[3]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v6.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v7.4h, v0.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v7.4h, v1.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v7.4h, v3.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v7.4h, v1.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)


    smlal       v24.4s, v8.4h, v2.h[3]
    smlal       v26.4s, v8.4h, v3.h[3]
    smlsl       v28.4s, v8.4h, v2.h[1]
    smlal       v30.4s, v8.4h, v0.h[3]


    smlal       v24.4s, v9.4h, v1.h[3]
    smlsl       v26.4s, v9.4h, v1.h[1]
    smlal       v28.4s, v9.4h, v0.h[3]
    smlsl       v30.4s, v9.4h, v0.h[1]




    smlal       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v1.h[2]
    smlsl       v22.4s, v4.4h, v3.h[0]
    smlal       v22.4s, v5.4h, v0.h[2]



    smlsl       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v3.h[2]
    smlal       v20.4s, v4.4h, v1.h[0]
    smlsl       v20.4s, v5.4h, v1.h[2]


    smlsl       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v0.h[2]
    smlsl       v16.4s, v4.4h, v1.h[0]
    smlal       v16.4s, v5.4h, v2.h[2]



    smlal       v18.4s, v10.4h, v0.h[0]
    smlsl       v18.4s, v11.4h, v2.h[2]
    smlal       v18.4s, v4.4h, v3.h[0]
    smlsl       v18.4s, v5.4h, v3.h[2]


skip_last8rows_stage2_kernel2:



    add         v4.4s,  v22.4s ,  v24.4s
    sub         v22.4s,  v22.4s ,  v24.4s

    add         v6.4s,  v20.4s ,  v26.4s
    sub         v24.4s,  v20.4s ,  v26.4s

    add         v10.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s


    sqrshrn     v18.4h, v4.4s,#shift_stage2_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v31.4h, v22.4s,#shift_stage2_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v19.4h, v10.4s,#shift_stage2_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v30.4h, v26.4s,#shift_stage2_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v20.4h, v6.4s,#shift_stage2_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v23.4h, v24.4s,#shift_stage2_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v21.4h, v16.4s,#shift_stage2_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v22.4h, v28.4s,#shift_stage2_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    ld1         {v4.4h, v5.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],#16



    // registers used:    {q2,q4,q6,q7}, {q9,q15,q10,q11}

//d4=x0
//d12=x1
//d5=x2
//d13=x3

//d18=x4
//d20=x5
//d19=x6
//d21=x7

//d22=x8
//d30=x9
//d23=x10
//d31=x11

//d14=x12
//d8=x13
//d15=x14
//d9=x15

    umov        x15,v26.d[0]
    umov        x16,v27.d[0]
    umov        x19,v28.d[0]
    umov        x20,v29.d[0]

    trn1        v26.4h, v4.4h, v12.4h
    trn2        v27.4h, v4.4h, v12.4h
    trn1        v28.4h, v5.4h, v13.4h
    trn2        v29.4h, v5.4h, v13.4h

    trn1        v4.2s, v26.2s, v28.2s
    trn2        v5.2s, v26.2s, v28.2s
    trn1        v12.2s, v27.2s, v29.2s
    trn2        v13.2s, v27.2s, v29.2s

    trn1        v26.4h, v18.4h, v20.4h
    trn2        v27.4h, v18.4h, v20.4h
    trn1        v28.4h, v19.4h, v21.4h
    trn2        v29.4h, v19.4h, v21.4h

    trn1        v18.2s, v26.2s, v28.2s
    trn2        v19.2s, v26.2s, v28.2s
    trn1        v20.2s, v27.2s, v29.2s
    trn2        v21.2s, v27.2s, v29.2s

    trn1        v26.4h, v22.4h, v30.4h
    trn2        v27.4h, v22.4h, v30.4h
    trn1        v28.4h, v23.4h, v31.4h
    trn2        v29.4h, v23.4h, v31.4h

    trn1        v22.2s, v26.2s, v28.2s
    trn2        v23.2s, v26.2s, v28.2s
    trn1        v30.2s, v27.2s, v29.2s
    trn2        v31.2s, v27.2s, v29.2s

    trn1        v26.4h, v14.4h, v8.4h
    trn2        v27.4h, v14.4h, v8.4h
    trn1        v28.4h, v15.4h, v9.4h
    trn2        v29.4h, v15.4h, v9.4h

    trn1        v14.2s, v26.2s, v28.2s
    trn2        v15.2s, v26.2s, v28.2s
    trn1        v8.2s, v27.2s, v29.2s
    trn2        v9.2s, v27.2s, v29.2s

    mov         v26.d[0],x15
    mov         v27.d[0],x16
    mov         v28.d[0],x19
    mov         v29.d[0],x20

// d4 =x0 1- 4 values
// d5 =x2 1- 4 values
// d12=x1 1- 4 values
// d13=x3 1- 4 values

// d18 =x0 5- 8 values
// d19 =x2 5- 8 values
// d20=x1 5- 8 values
// d21=x3 5- 8 values

// d22 =x0 9- 12 values
// d23 =x2 9- 12 values
// d30=x1 9- 12 values
// d31=x3 9- 12 values

// d14 =x0 13-16 values
// d15 =x2 13- 16 values
// d8=x1 13- 16 values
// d9=x3 13- 16 values

    // swapping v5 and v15
    mov         v5.d[1],v5.d[0]
    mov         v5.d[0],v18.d[0]
    mov         v18.d[0],v5.d[1]
    // swapping v23 and v14
    mov         v23.d[1],v23.d[0]
    mov         v23.d[0],v14.d[0]
    mov         v14.d[0],v23.d[1]
    // swapping v13 and v20
    mov         v13.d[1],v13.d[0]
    mov         v13.d[0],v20.d[0]
    mov         v20.d[0],v13.d[1]
    // swapping v31 and v8
    mov         v31.d[1],v31.d[0]
    mov         v31.d[0],v8.d[0]
    mov         v8.d[0],v31.d[1]

// q2: x0 1-8 values
// q11: x0 9-16 values
// q9 : x2 1-8 values
// q7 : x2 9-16 values
// q6 : x1 1- 8 values
// q10: x3 1-8 values
// q15: x1 9-16 values
// q4:  x3 9-16 values


//    registers free: q8,q14,q12,q13


    ld1         {v16.8b, v17.8b},[x2],x8
    ld1         {v28.8b, v29.8b},[x2],x5
    ld1         {v24.8b, v25.8b},[x4],x8
    ld1         {v26.8b, v27.8b},[x4],x5

    mov         v4.d[1] ,v5.d[0]
    mov         v22.d[1] ,v23.d[0]
    mov         v12.d[1] ,v13.d[0]
    mov         v30.d[1] ,v31.d[0]
    mov         v18.d[1] ,v19.d[0]
    mov         v14.d[1] ,v15.d[0]
    mov         v20.d[1] ,v21.d[0]
    mov         v8.d[1] ,v9.d[0]

    uaddw       v4.8h,  v4.8h ,  v16.8b
    uaddw       v22.8h,  v22.8h ,  v17.8b
    uaddw       v12.8h,  v12.8h ,  v28.8b
    uaddw       v30.8h,  v30.8h ,  v29.8b
    uaddw       v18.8h,  v18.8h ,  v24.8b
    uaddw       v14.8h,  v14.8h ,  v25.8b
    uaddw       v20.8h,  v20.8h ,  v26.8b
    uaddw       v8.8h,  v8.8h ,  v27.8b


    sqxtun      v16.8b, v4.8h
    sqxtun      v17.8b, v22.8h
    sqxtun      v28.8b, v12.8h
    sqxtun      v29.8b, v30.8h
    sqxtun      v24.8b, v18.8h
    sqxtun      v25.8b, v14.8h
    sqxtun      v26.8b, v20.8h
    sqxtun      v27.8b, v8.8h



    st1         {v16.8b, v17.8b},[x3],x7
    st1         {v28.8b, v29.8b},[x3],x7
    st1         {v24.8b, v25.8b},[x3],x7
    st1         {v26.8b, v27.8b},[x3],x7

    subs        x14,x14,#1



    bne         second_stage


//    sub         sp,sp,#40
    // ldmfd sp!,{x4-x12,pc}
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret











