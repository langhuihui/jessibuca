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
// *  - ihevc_itrans_recon_32x32()
// *
// * @remarks
// *  the input buffer is being corrupted
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

//void ihevc_itrans_recon_32x32(word16 *pi2_src,
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


//d0[0]=    64        d2[0]=83
//d0[1]= 90        d2[1]=82
//d0[2]= 90        d2[2]=80
//d0[3]= 90        d2[3]=78
//d1[0]= 89         d3[0]=75
//d1[1]= 88        d3[1]=73
//d1[2]= 87        d3[2]=70
//d1[3]= 85        d3[3]=67

//d4[0]=    64        d6[0]=36
//d4[1]= 61        d6[1]=31
//d4[2]= 57        d6[2]=25
//d4[3]= 54        d6[3]=22
//d5[0]= 50         d7[0]=18
//d5[1]= 46        d7[1]=13
//d5[2]= 43        d7[2]=9
//d5[3]= 38        d7[3]=4

.text
.align 4
.include "ihevc_neon_macros.s"




.set shift_stage1_idct ,   7
.set shift_stage2_idct ,   12

//#define zero_cols      x12
//#define zero_rows     x11

.globl ihevc_itrans_recon_32x32_av8

.extern g_ai2_ihevc_trans_32_transpose

.type ihevc_itrans_recon_32x32_av8, %function

ihevc_itrans_recon_32x32_av8:

    ldr         w11, [sp]

// stmfd sp!,{x0-x12,x14}
    push_v_regs
    stp         x19, x20,[sp,#-16]!
    stp         x0, x1,[sp,#-16]!
    stp         x5, x6,[sp,#-16]!

//ldr            x8,[sp,#56]     @ prediction stride
//ldr            x7,[sp,#64]     @ destination stride
    mov         x6, x4 // src stride
    mov         x12, x7
    lsl         x6, x6, #1                  // x sizeof(word16)
    add         x10,x6,x6, lsl #1           // 3 rows


    mov         x8,x0

    adrp        x14, :got:g_ai2_ihevc_trans_32_transpose
    ldr         x14, [x14, #:got_lo12:g_ai2_ihevc_trans_32_transpose]

    ld1         {v0.4h, v1.4h, v2.4h, v3.4h},[x14],#32
    ld1         {v4.4h, v5.4h, v6.4h, v7.4h},[x14],#32

//registers which are free
//  x10,x9,x11,x12
    mov         x9,#0xffffff00
    mov         x10,#0xfffffff0
    mov         w5,#0xfffff000
    mov         w7,#0xffff0000
    cmp         x12,x10
    mov         x20,#1
    csel        x14, x20, x14,hs
    bhs         stage1


    cmp         x12,x9
    mov         x20,#2
    csel        x14, x20, x14,hs
    bhs         stage1

    cmp         x12,x5
    mov         x20,#3
    csel        x14, x20, x14,hs
    bhs         stage1

    cmp         x12,x7
    mov         x20,#4
    csel        x14, x20, x14,hs

    mov         x14,#8
    b           stage1
//.ltorg


dct_stage1:
    add         x8,x8,#8
    mov         x0,x8

stage1:
    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6

    smull       v24.4s, v8.4h, v0.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v8.4h, v0.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v8.4h, v1.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v8.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v0.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v9.4h, v2.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v3.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v5.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smull       v20.4s, v10.4h, v0.h[0]
    smlal       v20.4s, v11.4h, v0.h[2]


    smull       v22.4s, v10.4h, v0.h[0]
    smlal       v22.4s, v11.4h, v1.h[2]

    smull       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v2.h[2]

    smull       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v3.h[2]
    cmp         x11,x10
    bhs         shift1

    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6







    smlal       v24.4s, v14.4h, v1.h[1]
    smlal       v26.4s, v14.4h, v3.h[3]
    smlal       v28.4s, v14.4h, v6.h[1]
    smlsl       v30.4s, v14.4h, v7.h[1]


    smlal       v24.4s, v15.4h, v1.h[3]
    smlal       v26.4s, v15.4h, v5.h[1]
    smlsl       v28.4s, v15.4h, v7.h[1]
    smlsl       v30.4s, v15.4h, v3.h[3]


    smlal       v20.4s, v12.4h, v1.h[0]
    smlal       v20.4s, v13.4h, v1.h[2]
    smlal       v22.4s, v12.4h, v3.h[0]
    smlal       v22.4s, v13.4h, v4.h[2]
    smlal       v16.4s, v12.4h, v5.h[0]
    smlal       v16.4s, v13.4h, v7.h[2]
    smlal       v18.4s, v12.4h, v7.h[0]
    smlsl       v18.4s, v13.4h, v5.h[2]

    cmp         x11,x9
    bhs         shift1

    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6


    smlal       v24.4s, v8.4h, v2.h[1]     //// y1 * cos1(part of b0)
    smlal       v26.4s, v8.4h, v6.h[3]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v4.h[3]     //// y1 * sin3(part of b2)
    smlsl       v30.4s, v8.4h, v0.h[1]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v2.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v7.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v2.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v3.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v2.h[0]
    smlal       v20.4s, v11.4h, v2.h[2]


    smlal       v22.4s, v10.4h, v6.h[0]
    smlal       v22.4s, v11.4h, v7.h[2]

    smlsl       v16.4s, v10.4h, v6.h[0]
    smlsl       v16.4s, v11.4h, v3.h[2]

    smlsl       v18.4s, v10.4h, v2.h[0]
    smlsl       v18.4s, v11.4h, v1.h[2]

    cmp         x11,x5
    bhs         shift1


    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6









    smlal       v24.4s, v14.4h, v3.h[1]
    smlsl       v26.4s, v14.4h, v6.h[1]
    smlsl       v28.4s, v14.4h, v0.h[1]
    smlsl       v30.4s, v14.4h, v6.h[3]


    smlal       v24.4s, v15.4h, v3.h[3]
    smlsl       v26.4s, v15.4h, v4.h[3]
    smlsl       v28.4s, v15.4h, v2.h[3]
    smlal       v30.4s, v15.4h, v5.h[3]


    smlal       v20.4s, v12.4h, v3.h[0]
    smlal       v20.4s, v13.4h, v3.h[2]
    smlsl       v22.4s, v12.4h, v7.h[0]
    smlsl       v22.4s, v13.4h, v5.h[2]
    smlsl       v16.4s, v12.4h, v1.h[0]
    smlsl       v16.4s, v13.4h, v1.h[2]
    smlsl       v18.4s, v12.4h, v5.h[0]
    smlal       v18.4s, v13.4h, v7.h[2]

    cmp         x11,x7
    bhs         shift1


    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6



    smlal       v24.4s, v8.4h, v4.h[1]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v3.h[1]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v5.h[1]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v2.h[1]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v4.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v1.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v7.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v1.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v0.h[0]
    smlal       v20.4s, v11.4h, v4.h[2]


    smlsl       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v2.h[2]

    smlsl       v16.4s, v10.4h, v0.h[0]
    smlsl       v16.4s, v11.4h, v6.h[2]

    smlal       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v0.h[2]



    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6




    smlal       v24.4s, v14.4h, v5.h[1]
    smlsl       v26.4s, v14.4h, v0.h[2]
    smlal       v28.4s, v14.4h, v5.h[3]
    smlal       v30.4s, v14.4h, v4.h[3]


    smlal       v24.4s, v15.4h, v5.h[3]
    smlsl       v26.4s, v15.4h, v1.h[1]
    smlal       v28.4s, v15.4h, v3.h[1]
    smlsl       v30.4s, v15.4h, v7.h[3]


    smlal       v20.4s, v12.4h, v5.h[0]
    smlal       v20.4s, v13.4h, v5.h[2]
    smlsl       v22.4s, v12.4h, v1.h[0]
    smlsl       v22.4s, v13.4h, v0.h[2]
    smlal       v16.4s, v12.4h, v7.h[0]
    smlal       v16.4s, v13.4h, v4.h[2]
    smlal       v18.4s, v12.4h, v3.h[0]
    smlal       v18.4s, v13.4h, v6.h[2]


    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6







    smlal       v24.4s, v8.4h, v6.h[1]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v2.h[3]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v0.h[1]     //// y1 * sin3(part of b2)
    smlsl       v30.4s, v8.4h, v4.h[1]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v6.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v4.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v1.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v0.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v6.h[0]
    smlal       v20.4s, v11.4h, v6.h[2]


    smlsl       v22.4s, v10.4h, v2.h[0]
    smlsl       v22.4s, v11.4h, v3.h[2]

    smlal       v16.4s, v10.4h, v2.h[0]
    smlal       v16.4s, v11.4h, v0.h[2]

    smlsl       v18.4s, v10.4h, v6.h[0]
    smlsl       v18.4s, v11.4h, v2.h[2]

    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6


    smlal       v24.4s, v14.4h, v7.h[1]
    smlsl       v26.4s, v14.4h, v5.h[3]
    smlal       v28.4s, v14.4h, v4.h[1]
    smlsl       v30.4s, v14.4h, v2.h[3]


    smlal       v24.4s, v15.4h, v7.h[3]
    smlsl       v26.4s, v15.4h, v7.h[1]
    smlal       v28.4s, v15.4h, v6.h[3]
    smlsl       v30.4s, v15.4h, v6.h[1]


    smlal       v20.4s, v12.4h, v7.h[0]
    smlal       v20.4s, v13.4h, v7.h[2]
    smlsl       v22.4s, v12.4h, v5.h[0]
    smlsl       v22.4s, v13.4h, v6.h[2]
    smlal       v16.4s, v12.4h, v3.h[0]
    smlal       v16.4s, v13.4h, v5.h[2]
    smlsl       v18.4s, v12.4h, v1.h[0]
    smlsl       v18.4s, v13.4h, v4.h[2]



shift1:
    add         v8.4s,  v20.4s ,  v24.4s
    sub         v10.4s,  v20.4s ,  v24.4s

    add         v12.4s,  v22.4s ,  v26.4s
    sub         v24.4s,  v22.4s ,  v26.4s

    add         v14.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s


    sqrshrn     v30.4h, v8.4s,#shift_stage1_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v19.4h, v10.4s,#shift_stage1_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v31.4h, v14.4s,#shift_stage1_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v18.4h, v26.4s,#shift_stage1_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v12.4h, v12.4s,#shift_stage1_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v15.4h, v24.4s,#shift_stage1_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v13.4h, v16.4s,#shift_stage1_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v14.4h, v28.4s,#shift_stage1_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)


    // registers used q15,q14,q6,q7

    umov        x15,v24.d[0]
    umov        x16,v25.d[0]
    umov        x19,v26.d[0]
    umov        x20,v27.d[0]

    trn1        v24.4h, v30.4h, v12.4h
    trn2        v25.4h, v30.4h, v12.4h
    trn1        v26.4h, v31.4h, v13.4h
    trn2        v27.4h, v31.4h, v13.4h

    trn1        v30.2s, v24.2s, v26.2s
    trn2        v31.2s, v24.2s, v26.2s
    trn1        v12.2s, v25.2s, v27.2s
    trn2        v13.2s, v25.2s, v27.2s

    trn1        v24.4h, v14.4h, v18.4h
    trn2        v25.4h, v14.4h, v18.4h
    trn1        v26.4h, v15.4h, v19.4h
    trn2        v27.4h, v15.4h, v19.4h

    trn1        v14.2s, v24.2s, v26.2s
    trn2        v15.2s, v24.2s, v26.2s
    trn1        v18.2s, v25.2s, v27.2s
    trn2        v19.2s, v25.2s, v27.2s

    mov         v24.d[0],x15
    mov         v25.d[0],x16
    mov         v26.d[0],x19
    mov         v27.d[0],x20

// d30 =x0 1- 4 values
// d31 =x2 1- 4 values
// d12=x1 1- 4 values
// d13=x3 1- 4 values
// d14 =x0 28-31 values
// d15 =x2 28- 31 values
// d18=x1 28- 31 values
// d19=x3 28- 31 values



    st1         { v30.4h, v31.4h},[x1],#16
    st1         { v12.4h, v13.4h},[x1],#16
    add         x1,x1,#192
    st1         { v14.4h, v15.4h},[x1],#16
    st1         { v18.4h, v19.4h},[x1],#16
    sub         x1,x1,#224

    mov         x0,x8





    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6




    smull       v24.4s, v8.4h, v2.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v8.4h, v2.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v8.4h, v3.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v8.4h, v3.h[3]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v6.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v7.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v6.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v4.h[3]     //// y1 * sin1 - y3 * sin3(part of b3)





    smull       v20.4s, v10.4h, v0.h[0]
    smlal       v20.4s, v11.4h, v4.h[2]


    smull       v22.4s, v10.4h, v0.h[0]
    smlal       v22.4s, v11.4h, v5.h[2]

    smull       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v6.h[2]

    smull       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v7.h[2]
    cmp         x11,x10
    bhs         shift2

    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6


    smlsl       v24.4s, v14.4h, v4.h[3]
    smlsl       v26.4s, v14.4h, v2.h[1]
    smlsl       v28.4s, v14.4h, v0.h[1]
    smlsl       v30.4s, v14.4h, v2.h[3]


    smlsl       v24.4s, v15.4h, v0.h[3]
    smlsl       v26.4s, v15.4h, v3.h[1]
    smlsl       v28.4s, v15.4h, v6.h[3]
    smlal       v30.4s, v15.4h, v5.h[3]


    smlsl       v20.4s, v12.4h, v7.h[0]
    smlsl       v20.4s, v13.4h, v2.h[2]
    smlsl       v22.4s, v12.4h, v5.h[0]
    smlsl       v22.4s, v13.4h, v0.h[2]
    smlsl       v16.4s, v12.4h, v3.h[0]
    smlsl       v16.4s, v13.4h, v3.h[2]
    smlsl       v18.4s, v12.4h, v1.h[0]
    smlsl       v18.4s, v13.4h, v6.h[2]

    cmp         x11,x9
    bhs         shift2


    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6







    smlsl       v24.4s, v8.4h, v4.h[1]     //// y1 * cos1(part of b0)
    smlal       v26.4s, v8.4h, v7.h[1]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v2.h[3]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v7.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v9.4h, v1.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v3.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v6.h[3]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlsl       v20.4s, v10.4h, v2.h[0]
    smlsl       v20.4s, v11.4h, v6.h[2]


    smlsl       v22.4s, v10.4h, v6.h[0]
    smlal       v22.4s, v11.4h, v4.h[2]

    smlal       v16.4s, v10.4h, v6.h[0]
    smlal       v16.4s, v11.4h, v0.h[2]

    smlal       v18.4s, v10.4h, v2.h[0]
    smlal       v18.4s, v11.4h, v5.h[2]

    cmp         x11,x5
    bhs         shift2


    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6





    smlal       v24.4s, v14.4h, v2.h[3]
    smlal       v26.4s, v14.4h, v3.h[3]
    smlsl       v28.4s, v14.4h, v5.h[3]
    smlsl       v30.4s, v14.4h, v0.h[3]


    smlal       v24.4s, v15.4h, v1.h[3]
    smlsl       v26.4s, v15.4h, v6.h[3]
    smlsl       v28.4s, v15.4h, v0.h[3]
    smlal       v30.4s, v15.4h, v7.h[3]


    smlal       v20.4s, v12.4h, v5.h[0]
    smlal       v20.4s, v13.4h, v0.h[2]
    smlal       v22.4s, v12.4h, v1.h[0]
    smlal       v22.4s, v13.4h, v6.h[2]
    smlal       v16.4s, v12.4h, v7.h[0]
    smlsl       v16.4s, v13.4h, v2.h[2]
    smlsl       v18.4s, v12.4h, v3.h[0]
    smlsl       v18.4s, v13.4h, v4.h[2]


    cmp         x11,x7
    bhs         shift2


    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6







    smlal       v24.4s, v8.4h, v6.h[1]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v1.h[1]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v7.h[1]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v0.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v5.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v4.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v2.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v7.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v7.h[2]


    smlsl       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v1.h[2]

    smlsl       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v5.h[2]

    smlal       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v3.h[2]



    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6


    smlsl       v24.4s, v14.4h, v0.h[1]
    smlal       v26.4s, v14.4h, v6.h[1]
    smlal       v28.4s, v14.4h, v4.h[1]
    smlsl       v30.4s, v14.4h, v1.h[1]


    smlsl       v24.4s, v15.4h, v3.h[3]
    smlal       v26.4s, v15.4h, v0.h[1]
    smlsl       v28.4s, v15.4h, v5.h[1]
    smlsl       v30.4s, v15.4h, v6.h[1]


    smlsl       v20.4s, v12.4h, v3.h[0]
    smlsl       v20.4s, v13.4h, v1.h[2]
    smlsl       v22.4s, v12.4h, v7.h[0]
    smlal       v22.4s, v13.4h, v3.h[2]
    smlal       v16.4s, v12.4h, v1.h[0]
    smlal       v16.4s, v13.4h, v7.h[2]
    smlsl       v18.4s, v12.4h, v5.h[0]
    smlsl       v18.4s, v13.4h, v2.h[2]

    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6




    smlal       v24.4s, v8.4h, v7.h[3]     //// y1 * cos1(part of b0)
    smlal       v26.4s, v8.4h, v4.h[3]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v1.h[1]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v2.h[1]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v3.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v5.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v7.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v5.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlsl       v20.4s, v10.4h, v6.h[0]
    smlal       v20.4s, v11.4h, v5.h[2]


    smlal       v22.4s, v10.4h, v2.h[0]
    smlal       v22.4s, v11.4h, v7.h[2]

    smlsl       v16.4s, v10.4h, v2.h[0]
    smlsl       v16.4s, v11.4h, v4.h[2]

    smlal       v18.4s, v10.4h, v6.h[0]
    smlal       v18.4s, v11.4h, v1.h[2]


    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6





    smlal       v24.4s, v14.4h, v1.h[1]
    smlsl       v26.4s, v14.4h, v0.h[3]
    smlal       v28.4s, v14.4h, v1.h[3]
    smlsl       v30.4s, v14.4h, v3.h[1]


    smlal       v24.4s, v15.4h, v5.h[3]
    smlsl       v26.4s, v15.4h, v5.h[1]
    smlal       v28.4s, v15.4h, v4.h[3]
    smlsl       v30.4s, v15.4h, v4.h[1]


    smlal       v20.4s, v12.4h, v1.h[0]
    smlal       v20.4s, v13.4h, v3.h[2]
    smlsl       v22.4s, v12.4h, v3.h[0]
    smlsl       v22.4s, v13.4h, v2.h[2]
    smlal       v16.4s, v12.4h, v5.h[0]
    smlal       v16.4s, v13.4h, v1.h[2]
    smlsl       v18.4s, v12.4h, v7.h[0]
    smlsl       v18.4s, v13.4h, v0.h[2]

shift2:
    add         v8.4s,  v20.4s ,  v24.4s
    sub         v10.4s,  v20.4s ,  v24.4s

    add         v12.4s,  v22.4s ,  v26.4s
    sub         v24.4s,  v22.4s ,  v26.4s

    add         v14.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s


    sqrshrn     v30.4h, v8.4s,#shift_stage1_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v19.4h, v10.4s,#shift_stage1_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v31.4h, v14.4s,#shift_stage1_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v18.4h, v26.4s,#shift_stage1_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v12.4h, v12.4s,#shift_stage1_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v15.4h, v24.4s,#shift_stage1_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v13.4h, v16.4s,#shift_stage1_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v14.4h, v28.4s,#shift_stage1_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    umov        x15,v24.d[0]
    umov        x16,v25.d[0]
    umov        x19,v26.d[0]
    umov        x20,v27.d[0]

    trn1        v24.4h, v30.4h, v12.4h
    trn2        v25.4h, v30.4h, v12.4h
    trn1        v26.4h, v31.4h, v13.4h
    trn2        v27.4h, v31.4h, v13.4h

    trn1        v30.2s, v24.2s, v26.2s
    trn2        v31.2s, v24.2s, v26.2s
    trn1        v12.2s, v25.2s, v27.2s
    trn2        v13.2s, v25.2s, v27.2s

    trn1        v24.4h, v14.4h, v18.4h
    trn2        v25.4h, v14.4h, v18.4h
    trn1        v26.4h, v15.4h, v19.4h
    trn2        v27.4h, v15.4h, v19.4h

    trn1        v14.2s, v24.2s, v26.2s
    trn2        v15.2s, v24.2s, v26.2s
    trn1        v18.2s, v25.2s, v27.2s
    trn2        v19.2s, v25.2s, v27.2s

    mov         v24.d[0],x15
    mov         v25.d[0],x16
    mov         v26.d[0],x19
    mov         v27.d[0],x20

    st1         { v30.4h, v31.4h},[x1],#16
    st1         { v12.4h, v13.4h},[x1],#16
    add         x1,x1,#128
    st1         { v14.4h, v15.4h},[x1],#16
    st1         { v18.4h, v19.4h},[x1],#16
    sub         x1,x1,#160
    mov         x0,x8



    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6


    smull       v24.4s, v8.4h, v4.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v8.4h, v4.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v8.4h, v5.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v8.4h, v5.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v3.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v1.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v0.h[2]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v1.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smull       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v7.h[2]


    smull       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v6.h[2]

    smull       v16.4s, v10.4h, v0.h[0]
    smlsl       v16.4s, v11.4h, v5.h[2]

    smull       v18.4s, v10.4h, v0.h[0]
    smlsl       v18.4s, v11.4h, v4.h[2]

    cmp         x11,x10
    bhs         shift3

    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6




    smlsl       v24.4s, v14.4h, v5.h[1]
    smlsl       v26.4s, v14.4h, v7.h[3]
    smlal       v28.4s, v14.4h, v5.h[3]
    smlal       v30.4s, v14.4h, v3.h[1]


    smlal       v24.4s, v15.4h, v2.h[1]
    smlal       v26.4s, v15.4h, v1.h[1]
    smlal       v28.4s, v15.4h, v4.h[3]
    smlsl       v30.4s, v15.4h, v7.h[3]


    smlsl       v20.4s, v12.4h, v1.h[0]
    smlal       v20.4s, v13.4h, v6.h[2]
    smlsl       v22.4s, v12.4h, v3.h[0]
    smlal       v22.4s, v13.4h, v3.h[2]
    smlsl       v16.4s, v12.4h, v5.h[0]
    smlal       v16.4s, v13.4h, v0.h[2]
    smlsl       v18.4s, v12.4h, v7.h[0]
    smlal       v18.4s, v13.4h, v2.h[2]

    cmp         x11,x9
    bhs         shift3

    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6

    smlal       v24.4s, v8.4h, v6.h[1]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v5.h[1]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v0.h[3]     //// y1 * sin3(part of b2)
    smlsl       v30.4s, v8.4h, v3.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v1.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v4.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v6.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v0.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v2.h[0]
    smlsl       v20.4s, v11.4h, v5.h[2]


    smlal       v22.4s, v10.4h, v6.h[0]
    smlsl       v22.4s, v11.4h, v0.h[2]

    smlsl       v16.4s, v10.4h, v6.h[0]
    smlsl       v16.4s, v11.4h, v4.h[2]

    smlsl       v18.4s, v10.4h, v2.h[0]
    smlal       v18.4s, v11.4h, v6.h[2]

    cmp         x11,x5
    bhs         shift3


    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6






    smlsl       v24.4s, v14.4h, v7.h[1]
    smlal       v26.4s, v14.4h, v2.h[1]
    smlal       v28.4s, v14.4h, v4.h[1]
    smlsl       v30.4s, v14.4h, v5.h[1]


    smlal       v24.4s, v15.4h, v0.h[3]
    smlal       v26.4s, v15.4h, v7.h[1]
    smlsl       v28.4s, v15.4h, v1.h[1]
    smlsl       v30.4s, v15.4h, v6.h[1]


    smlsl       v20.4s, v12.4h, v3.h[0]
    smlal       v20.4s, v13.4h, v4.h[2]
    smlal       v22.4s, v12.4h, v7.h[0]
    smlal       v22.4s, v13.4h, v2.h[2]
    smlal       v16.4s, v12.4h, v1.h[0]
    smlsl       v16.4s, v13.4h, v6.h[2]
    smlal       v18.4s, v12.4h, v5.h[0]
    smlsl       v18.4s, v13.4h, v0.h[2]


    cmp         x11,x7
    bhs         shift3


    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6


    smlsl       v24.4s, v8.4h, v7.h[3]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v0.h[1]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v6.h[3]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v0.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v9.4h, v5.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v3.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v2.h[3]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v3.h[2]


    smlsl       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v5.h[2]

    smlsl       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v1.h[2]

    smlal       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v7.h[2]


    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6



    smlal       v24.4s, v14.4h, v6.h[3]
    smlal       v26.4s, v14.4h, v3.h[3]
    smlsl       v28.4s, v14.4h, v1.h[3]
    smlal       v30.4s, v14.4h, v7.h[1]


    smlal       v24.4s, v15.4h, v1.h[3]
    smlsl       v26.4s, v15.4h, v2.h[3]
    smlal       v28.4s, v15.4h, v7.h[1]
    smlal       v30.4s, v15.4h, v4.h[1]


    smlsl       v20.4s, v12.4h, v5.h[0]
    smlal       v20.4s, v13.4h, v2.h[2]
    smlal       v22.4s, v12.4h, v1.h[0]
    smlsl       v22.4s, v13.4h, v7.h[2]
    smlsl       v16.4s, v12.4h, v7.h[0]
    smlsl       v16.4s, v13.4h, v3.h[2]
    smlsl       v18.4s, v12.4h, v3.h[0]
    smlal       v18.4s, v13.4h, v1.h[2]



    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6




    smlsl       v24.4s, v8.4h, v5.h[3]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v6.h[3]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v3.h[1]     //// y1 * sin3(part of b2)
    smlsl       v30.4s, v8.4h, v0.h[1]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v2.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v9.4h, v0.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v2.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v4.h[3]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v6.h[0]
    smlsl       v20.4s, v11.4h, v1.h[2]


    smlsl       v22.4s, v10.4h, v2.h[0]
    smlal       v22.4s, v11.4h, v4.h[2]

    smlal       v16.4s, v10.4h, v2.h[0]
    smlsl       v16.4s, v11.4h, v7.h[2]

    smlsl       v18.4s, v10.4h, v6.h[0]
    smlsl       v18.4s, v11.4h, v5.h[2]


    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6

    smlal       v24.4s, v14.4h, v4.h[3]
    smlsl       v26.4s, v14.4h, v6.h[1]
    smlal       v28.4s, v14.4h, v7.h[3]
    smlal       v30.4s, v14.4h, v6.h[3]


    smlal       v24.4s, v15.4h, v3.h[3]
    smlsl       v26.4s, v15.4h, v3.h[1]
    smlal       v28.4s, v15.4h, v2.h[3]
    smlsl       v30.4s, v15.4h, v2.h[1]


    smlsl       v20.4s, v12.4h, v7.h[0]
    smlal       v20.4s, v13.4h, v0.h[2]
    smlal       v22.4s, v12.4h, v5.h[0]
    smlsl       v22.4s, v13.4h, v1.h[2]
    smlsl       v16.4s, v12.4h, v3.h[0]
    smlal       v16.4s, v13.4h, v2.h[2]
    smlal       v18.4s, v12.4h, v1.h[0]
    smlsl       v18.4s, v13.4h, v3.h[2]

shift3:
    add         v8.4s,  v20.4s ,  v24.4s
    sub         v10.4s,  v20.4s ,  v24.4s

    add         v12.4s,  v22.4s ,  v26.4s
    sub         v24.4s,  v22.4s ,  v26.4s

    add         v14.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s


    sqrshrn     v30.4h, v8.4s,#shift_stage1_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v19.4h, v10.4s,#shift_stage1_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v31.4h, v14.4s,#shift_stage1_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v18.4h, v26.4s,#shift_stage1_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v12.4h, v12.4s,#shift_stage1_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v15.4h, v24.4s,#shift_stage1_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v13.4h, v16.4s,#shift_stage1_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v14.4h, v28.4s,#shift_stage1_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    umov        x15,v24.d[0]
    umov        x16,v25.d[0]
    umov        x19,v26.d[0]
    umov        x20,v27.d[0]

    trn1        v24.4h, v30.4h, v12.4h
    trn2        v25.4h, v30.4h, v12.4h
    trn1        v26.4h, v31.4h, v13.4h
    trn2        v27.4h, v31.4h, v13.4h

    trn1        v30.2s, v24.2s, v26.2s
    trn2        v31.2s, v24.2s, v26.2s
    trn1        v12.2s, v25.2s, v27.2s
    trn2        v13.2s, v25.2s, v27.2s

    trn1        v24.4h, v14.4h, v18.4h
    trn2        v25.4h, v14.4h, v18.4h
    trn1        v26.4h, v15.4h, v19.4h
    trn2        v27.4h, v15.4h, v19.4h

    trn1        v14.2s, v24.2s, v26.2s
    trn2        v15.2s, v24.2s, v26.2s
    trn1        v18.2s, v25.2s, v27.2s
    trn2        v19.2s, v25.2s, v27.2s

    mov         v24.d[0],x15
    mov         v25.d[0],x16
    mov         v26.d[0],x19
    mov         v27.d[0],x20
    st1         { v30.4h, v31.4h},[x1],#16
    st1         { v12.4h, v13.4h},[x1],#16
    add         x1,x1,#64
    st1         { v14.4h, v15.4h},[x1],#16
    st1         { v18.4h, v19.4h},[x1],#16
    sub         x1,x1,#96

    mov         x0,x8



    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6


    smull       v24.4s, v8.4h, v6.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v8.4h, v6.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v8.4h, v7.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v8.4h, v7.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v2.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v4.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v5.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v7.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smull       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v3.h[2]


    smull       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v2.h[2]

    smull       v16.4s, v10.4h, v0.h[0]
    smlsl       v16.4s, v11.4h, v1.h[2]

    smull       v18.4s, v10.4h, v0.h[0]
    smlsl       v18.4s, v11.4h, v0.h[2]

    cmp         x11,x10
    bhs         shift4

    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6






    smlal       v24.4s, v14.4h, v0.h[1]
    smlal       v26.4s, v14.4h, v1.h[3]
    smlal       v28.4s, v14.4h, v4.h[1]
    smlal       v30.4s, v14.4h, v6.h[3]


    smlsl       v24.4s, v15.4h, v4.h[1]
    smlsl       v26.4s, v15.4h, v0.h[3]
    smlsl       v28.4s, v15.4h, v2.h[3]
    smlsl       v30.4s, v15.4h, v6.h[1]


    smlal       v20.4s, v12.4h, v7.h[0]
    smlal       v20.4s, v13.4h, v5.h[2]
    smlal       v22.4s, v12.4h, v5.h[0]
    smlsl       v22.4s, v13.4h, v7.h[2]
    smlal       v16.4s, v12.4h, v3.h[0]
    smlsl       v16.4s, v13.4h, v4.h[2]
    smlal       v18.4s, v12.4h, v1.h[0]
    smlsl       v18.4s, v13.4h, v1.h[2]

    cmp         x11,x9
    bhs         shift4

    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6



    smlal       v24.4s, v8.4h, v7.h[3]     //// y1 * cos1(part of b0)
    smlal       v26.4s, v8.4h, v3.h[1]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v1.h[1]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v5.h[3]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v4.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v5.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v0.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v5.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlsl       v20.4s, v10.4h, v2.h[0]
    smlal       v20.4s, v11.4h, v1.h[2]


    smlsl       v22.4s, v10.4h, v6.h[0]
    smlal       v22.4s, v11.4h, v3.h[2]

    smlal       v16.4s, v10.4h, v6.h[0]
    smlsl       v16.4s, v11.4h, v7.h[2]

    smlal       v18.4s, v10.4h, v2.h[0]
    smlsl       v18.4s, v11.4h, v2.h[2]

    cmp         x11,x5
    bhs         shift4


    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6






    smlsl       v24.4s, v14.4h, v1.h[1]
    smlsl       v26.4s, v14.4h, v7.h[3]
    smlal       v28.4s, v14.4h, v1.h[3]
    smlal       v30.4s, v14.4h, v4.h[3]


    smlal       v24.4s, v15.4h, v2.h[1]
    smlal       v26.4s, v15.4h, v5.h[1]
    smlsl       v28.4s, v15.4h, v3.h[1]
    smlsl       v30.4s, v15.4h, v4.h[1]


    smlsl       v20.4s, v12.4h, v5.h[0]
    smlsl       v20.4s, v13.4h, v7.h[2]
    smlsl       v22.4s, v12.4h, v1.h[0]
    smlal       v22.4s, v13.4h, v1.h[2]
    smlsl       v16.4s, v12.4h, v7.h[0]
    smlal       v16.4s, v13.4h, v5.h[2]
    smlal       v18.4s, v12.4h, v3.h[0]
    smlsl       v18.4s, v13.4h, v3.h[2]

    cmp         x11,x7
    bhs         shift4


    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6


    smlsl       v24.4s, v8.4h, v5.h[3]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v2.h[3]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v4.h[3]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v3.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v6.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v9.4h, v0.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v6.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v3.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v0.h[2]


    smlsl       v22.4s, v10.4h, v0.h[0]
    smlal       v22.4s, v11.4h, v6.h[2]

    smlsl       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v2.h[2]

    smlal       v18.4s, v10.4h, v0.h[0]
    smlsl       v18.4s, v11.4h, v4.h[2]




    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6






    smlal       v24.4s, v14.4h, v3.h[1]
    smlsl       v26.4s, v14.4h, v2.h[1]
    smlal       v28.4s, v14.4h, v7.h[3]
    smlal       v30.4s, v14.4h, v2.h[3]


    smlsl       v24.4s, v15.4h, v0.h[3]
    smlal       v26.4s, v15.4h, v4.h[3]
    smlal       v28.4s, v15.4h, v6.h[3]
    smlsl       v30.4s, v15.4h, v2.h[1]


    smlal       v20.4s, v12.4h, v3.h[0]
    smlsl       v20.4s, v13.4h, v6.h[2]
    smlal       v22.4s, v12.4h, v7.h[0]
    smlsl       v22.4s, v13.4h, v4.h[2]
    smlsl       v16.4s, v12.4h, v1.h[0]
    smlal       v16.4s, v13.4h, v0.h[2]
    smlal       v18.4s, v12.4h, v5.h[0]
    smlsl       v18.4s, v13.4h, v5.h[2]


    ld1         {v10.4h},[x0],x6
    ld1         {v8.4h},[x0],x6
    ld1         {v11.4h},[x0],x6
    ld1         {v9.4h},[x0],x6





    smlal       v24.4s, v8.4h, v3.h[3]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v7.h[1]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v5.h[1]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v7.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v6.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v3.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v1.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlsl       v20.4s, v10.4h, v6.h[0]
    smlal       v20.4s, v11.4h, v2.h[2]


    smlal       v22.4s, v10.4h, v2.h[0]
    smlsl       v22.4s, v11.4h, v0.h[2]

    smlsl       v16.4s, v10.4h, v2.h[0]
    smlal       v16.4s, v11.4h, v3.h[2]

    smlal       v18.4s, v10.4h, v6.h[0]
    smlsl       v18.4s, v11.4h, v6.h[2]


    ld1         {v12.4h},[x0],x6
    ld1         {v14.4h},[x0],x6
    ld1         {v13.4h},[x0],x6
    ld1         {v15.4h},[x0],x6




    smlsl       v24.4s, v14.4h, v5.h[1]
    smlal       v26.4s, v14.4h, v3.h[3]
    smlsl       v28.4s, v14.4h, v2.h[1]
    smlal       v30.4s, v14.4h, v0.h[3]


    smlal       v24.4s, v15.4h, v1.h[3]
    smlsl       v26.4s, v15.4h, v1.h[1]
    smlal       v28.4s, v15.4h, v0.h[3]
    smlsl       v30.4s, v15.4h, v0.h[1]


    smlsl       v20.4s, v12.4h, v1.h[0]
    smlal       v20.4s, v13.4h, v4.h[2]
    smlal       v22.4s, v12.4h, v3.h[0]
    smlsl       v22.4s, v13.4h, v5.h[2]
    smlsl       v16.4s, v12.4h, v5.h[0]
    smlal       v16.4s, v13.4h, v6.h[2]
    smlal       v18.4s, v12.4h, v7.h[0]
    smlsl       v18.4s, v13.4h, v7.h[2]

shift4:
    add         v8.4s,  v20.4s ,  v24.4s
    sub         v10.4s,  v20.4s ,  v24.4s

    add         v12.4s,  v22.4s ,  v26.4s
    sub         v24.4s,  v22.4s ,  v26.4s

    add         v14.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s


    sqrshrn     v30.4h, v8.4s,#shift_stage1_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v19.4h, v10.4s,#shift_stage1_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v31.4h, v14.4s,#shift_stage1_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v18.4h, v26.4s,#shift_stage1_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v12.4h, v12.4s,#shift_stage1_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v15.4h, v24.4s,#shift_stage1_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v13.4h, v16.4s,#shift_stage1_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    sqrshrn     v14.4h, v28.4s,#shift_stage1_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    umov        x15,v24.d[0]
    umov        x16,v25.d[0]
    umov        x19,v26.d[0]
    umov        x20,v27.d[0]

    trn1        v24.4h, v30.4h, v12.4h
    trn2        v25.4h, v30.4h, v12.4h
    trn1        v26.4h, v31.4h, v13.4h
    trn2        v27.4h, v31.4h, v13.4h

    trn1        v30.2s, v24.2s, v26.2s
    trn2        v31.2s, v24.2s, v26.2s
    trn1        v12.2s, v25.2s, v27.2s
    trn2        v13.2s, v25.2s, v27.2s

    trn1        v24.4h, v14.4h, v18.4h
    trn2        v25.4h, v14.4h, v18.4h
    trn1        v26.4h, v15.4h, v19.4h
    trn2        v27.4h, v15.4h, v19.4h

    trn1        v14.2s, v24.2s, v26.2s
    trn2        v15.2s, v24.2s, v26.2s
    trn1        v18.2s, v25.2s, v27.2s
    trn2        v19.2s, v25.2s, v27.2s

    mov         v24.d[0],x15
    mov         v25.d[0],x16
    mov         v26.d[0],x19
    mov         v27.d[0],x20

    st1         { v30.4h, v31.4h},[x1],#16
    st1         { v12.4h, v13.4h},[x1],#16
    st1         { v14.4h, v15.4h},[x1],#16
    st1         { v18.4h, v19.4h},[x1],#16

    add         x1,x1,#96

    subs        x14,x14,#1
    bne         dct_stage1
second_stage_dct:
//    mov        x0,x1
    ldp         x8, x7,[sp],#16
    ldp         x0, x1,[sp],#16

//    add x4,x2,x8, lsl #1    @ x4 = x2 + pred_strd * 2    => x4 points to 3rd row of pred data
//    add x5,x8,x8, lsl #1    @
//    sub   x0,x0,#512
    mov         x11,#0xfffffff0
    mov         x5, #0xffffff00
    mov         w6,#0xfffff000
    mov         w9,#0xffff0000
//    sub         x1,x1,#2048
    mov         x4,x1
    mov         x10,#240
    mov         x14,#8
    b           stage2

// registers free :

// arm registers used
// x8 : predicition stride
// x7 : destination stride
// x1: temp buffer
// x2 : pred buffer
// x3 : destination buffer
// x14 : loop counter
//x0 : scratch buffer
//x10 : used as stride
// x4 : used to store the initial address
//x12 : zero cols
// x11 : 0xfffffff0
// x5 : 0xffffff00
dct_stage2:
    add         x4,x4,#32
    mov         x1,x4
stage2:
    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10

    smull       v24.4s, v8.4h, v0.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v8.4h, v0.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v8.4h, v1.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v8.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v0.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v9.4h, v2.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v3.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v5.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)



    smull       v20.4s, v10.4h, v0.h[0]
    smlal       v20.4s, v11.4h, v0.h[2]


    smull       v22.4s, v10.4h, v0.h[0]
    smlal       v22.4s, v11.4h, v1.h[2]

    smull       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v2.h[2]

    smull       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v3.h[2]
    cmp         x12,x11
    bhs         stage2_shift1

    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10






    smlal       v24.4s, v14.4h, v1.h[1]
    smlal       v26.4s, v14.4h, v3.h[3]
    smlal       v28.4s, v14.4h, v6.h[1]
    smlsl       v30.4s, v14.4h, v7.h[1]


    smlal       v24.4s, v15.4h, v1.h[3]
    smlal       v26.4s, v15.4h, v5.h[1]
    smlsl       v28.4s, v15.4h, v7.h[1]
    smlsl       v30.4s, v15.4h, v3.h[3]


    smlal       v20.4s, v12.4h, v1.h[0]
    smlal       v20.4s, v13.4h, v1.h[2]
    smlal       v22.4s, v12.4h, v3.h[0]
    smlal       v22.4s, v13.4h, v4.h[2]
    smlal       v16.4s, v12.4h, v5.h[0]
    smlal       v16.4s, v13.4h, v7.h[2]
    smlal       v18.4s, v12.4h, v7.h[0]
    smlsl       v18.4s, v13.4h, v5.h[2]
    cmp         x12,x5
    bhs         stage2_shift1

    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10

    smlal       v24.4s, v8.4h, v2.h[1]     //// y1 * cos1(part of b0)
    smlal       v26.4s, v8.4h, v6.h[3]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v4.h[3]     //// y1 * sin3(part of b2)
    smlsl       v30.4s, v8.4h, v0.h[1]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v2.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v7.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v2.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v3.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v2.h[0]
    smlal       v20.4s, v11.4h, v2.h[2]


    smlal       v22.4s, v10.4h, v6.h[0]
    smlal       v22.4s, v11.4h, v7.h[2]

    smlsl       v16.4s, v10.4h, v6.h[0]
    smlsl       v16.4s, v11.4h, v3.h[2]

    smlsl       v18.4s, v10.4h, v2.h[0]
    smlsl       v18.4s, v11.4h, v1.h[2]

    cmp         x12,x6
    bhs         stage2_shift1


    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10





    smlal       v24.4s, v14.4h, v3.h[1]
    smlsl       v26.4s, v14.4h, v6.h[1]
    smlsl       v28.4s, v14.4h, v0.h[1]
    smlsl       v30.4s, v14.4h, v6.h[3]


    smlal       v24.4s, v15.4h, v3.h[3]
    smlsl       v26.4s, v15.4h, v4.h[3]
    smlsl       v28.4s, v15.4h, v2.h[3]
    smlal       v30.4s, v15.4h, v5.h[3]


    smlal       v20.4s, v12.4h, v3.h[0]
    smlal       v20.4s, v13.4h, v3.h[2]
    smlsl       v22.4s, v12.4h, v7.h[0]
    smlsl       v22.4s, v13.4h, v5.h[2]
    smlsl       v16.4s, v12.4h, v1.h[0]
    smlsl       v16.4s, v13.4h, v1.h[2]
    smlsl       v18.4s, v12.4h, v5.h[0]
    smlal       v18.4s, v13.4h, v7.h[2]

    cmp         x12,x9
    bhs         stage2_shift1


    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10


    smlal       v24.4s, v8.4h, v4.h[1]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v3.h[1]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v5.h[1]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v2.h[1]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v4.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v1.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v7.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v1.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v0.h[0]
    smlal       v20.4s, v11.4h, v4.h[2]


    smlsl       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v2.h[2]

    smlsl       v16.4s, v10.4h, v0.h[0]
    smlsl       v16.4s, v11.4h, v6.h[2]

    smlal       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v0.h[2]

    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10





    smlal       v24.4s, v14.4h, v5.h[1]
    smlsl       v26.4s, v14.4h, v0.h[2]
    smlal       v28.4s, v14.4h, v5.h[3]
    smlal       v30.4s, v14.4h, v4.h[3]


    smlal       v24.4s, v15.4h, v5.h[3]
    smlsl       v26.4s, v15.4h, v1.h[1]
    smlal       v28.4s, v15.4h, v3.h[1]
    smlsl       v30.4s, v15.4h, v7.h[3]


    smlal       v20.4s, v12.4h, v5.h[0]
    smlal       v20.4s, v13.4h, v5.h[2]
    smlsl       v22.4s, v12.4h, v1.h[0]
    smlsl       v22.4s, v13.4h, v0.h[2]
    smlal       v16.4s, v12.4h, v7.h[0]
    smlal       v16.4s, v13.4h, v4.h[2]
    smlal       v18.4s, v12.4h, v3.h[0]
    smlal       v18.4s, v13.4h, v6.h[2]


    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10




    smlal       v24.4s, v8.4h, v6.h[1]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v2.h[3]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v0.h[1]     //// y1 * sin3(part of b2)
    smlsl       v30.4s, v8.4h, v4.h[1]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v6.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v4.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v1.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v0.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v6.h[0]
    smlal       v20.4s, v11.4h, v6.h[2]


    smlsl       v22.4s, v10.4h, v2.h[0]
    smlsl       v22.4s, v11.4h, v3.h[2]

    smlal       v16.4s, v10.4h, v2.h[0]
    smlal       v16.4s, v11.4h, v0.h[2]

    smlsl       v18.4s, v10.4h, v6.h[0]
    smlsl       v18.4s, v11.4h, v2.h[2]

    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10

    smlal       v24.4s, v14.4h, v7.h[1]
    smlsl       v26.4s, v14.4h, v5.h[3]
    smlal       v28.4s, v14.4h, v4.h[1]
    smlsl       v30.4s, v14.4h, v2.h[3]


    smlal       v24.4s, v15.4h, v7.h[3]
    smlsl       v26.4s, v15.4h, v7.h[1]
    smlal       v28.4s, v15.4h, v6.h[3]
    smlsl       v30.4s, v15.4h, v6.h[1]


    smlal       v20.4s, v12.4h, v7.h[0]
    smlal       v20.4s, v13.4h, v7.h[2]
    smlsl       v22.4s, v12.4h, v5.h[0]
    smlsl       v22.4s, v13.4h, v6.h[2]
    smlal       v16.4s, v12.4h, v3.h[0]
    smlal       v16.4s, v13.4h, v5.h[2]
    smlsl       v18.4s, v12.4h, v1.h[0]
    smlsl       v18.4s, v13.4h, v4.h[2]

stage2_shift1:
    add         v8.4s,  v20.4s ,  v24.4s
    sub         v10.4s,  v20.4s ,  v24.4s

    add         v12.4s,  v22.4s ,  v26.4s
    sub         v24.4s,  v22.4s ,  v26.4s

    add         v14.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s


    sqrshrn     v30.4h, v8.4s,#shift_stage2_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v19.4h, v10.4s,#shift_stage2_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v31.4h, v14.4s,#shift_stage2_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v18.4h, v26.4s,#shift_stage2_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v12.4h, v12.4s,#shift_stage2_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v15.4h, v24.4s,#shift_stage2_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v13.4h, v16.4s,#shift_stage2_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v14.4h, v28.4s,#shift_stage2_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage2_idct)


    umov        x15,v24.d[0]
    umov        x16,v25.d[0]
    umov        x19,v26.d[0]
    umov        x20,v27.d[0]

    trn1        v24.4h, v30.4h, v12.4h
    trn2        v25.4h, v30.4h, v12.4h
    trn1        v26.4h, v31.4h, v13.4h
    trn2        v27.4h, v31.4h, v13.4h

    trn1        v30.2s, v24.2s, v26.2s
    trn2        v31.2s, v24.2s, v26.2s
    trn1        v12.2s, v25.2s, v27.2s
    trn2        v13.2s, v25.2s, v27.2s

    trn1        v24.4h, v14.4h, v18.4h
    trn2        v25.4h, v14.4h, v18.4h
    trn1        v26.4h, v15.4h, v19.4h
    trn2        v27.4h, v15.4h, v19.4h

    trn1        v14.2s, v24.2s, v26.2s
    trn2        v15.2s, v24.2s, v26.2s
    trn1        v18.2s, v25.2s, v27.2s
    trn2        v19.2s, v25.2s, v27.2s

    mov         v24.d[0],x15
    mov         v25.d[0],x16
    mov         v26.d[0],x19
    mov         v27.d[0],x20

    st1         { v30.4h, v31.4h},[x0],#16
    st1         { v12.4h, v13.4h},[x0],#16
    st1         { v14.4h, v15.4h},[x0],#16
    st1         { v18.4h, v19.4h},[x0],#16

    mov         x1,x4






    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10


    smull       v24.4s, v8.4h, v2.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v8.4h, v2.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v8.4h, v3.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v8.4h, v3.h[3]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v6.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v7.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v6.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v4.h[3]     //// y1 * sin1 - y3 * sin3(part of b3)





    smull       v20.4s, v10.4h, v0.h[0]
    smlal       v20.4s, v11.4h, v4.h[2]


    smull       v22.4s, v10.4h, v0.h[0]
    smlal       v22.4s, v11.4h, v5.h[2]

    smull       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v6.h[2]

    smull       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v7.h[2]

    cmp         x12,x11
    bhs         stage2_shift2

    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10


    smlsl       v24.4s, v14.4h, v4.h[3]
    smlsl       v26.4s, v14.4h, v2.h[1]
    smlsl       v28.4s, v14.4h, v0.h[1]
    smlsl       v30.4s, v14.4h, v2.h[3]


    smlsl       v24.4s, v15.4h, v0.h[3]
    smlsl       v26.4s, v15.4h, v3.h[1]
    smlsl       v28.4s, v15.4h, v6.h[3]
    smlal       v30.4s, v15.4h, v5.h[3]


    smlsl       v20.4s, v12.4h, v7.h[0]
    smlsl       v20.4s, v13.4h, v2.h[2]
    smlsl       v22.4s, v12.4h, v5.h[0]
    smlsl       v22.4s, v13.4h, v0.h[2]
    smlsl       v16.4s, v12.4h, v3.h[0]
    smlsl       v16.4s, v13.4h, v3.h[2]
    smlsl       v18.4s, v12.4h, v1.h[0]
    smlsl       v18.4s, v13.4h, v6.h[2]

    cmp         x12,x5
    bhs         stage2_shift2

    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10





    smlsl       v24.4s, v8.4h, v4.h[1]     //// y1 * cos1(part of b0)
    smlal       v26.4s, v8.4h, v7.h[1]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v2.h[3]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v7.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v9.4h, v1.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v3.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v6.h[3]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlsl       v20.4s, v10.4h, v2.h[0]
    smlsl       v20.4s, v11.4h, v6.h[2]


    smlsl       v22.4s, v10.4h, v6.h[0]
    smlal       v22.4s, v11.4h, v4.h[2]

    smlal       v16.4s, v10.4h, v6.h[0]
    smlal       v16.4s, v11.4h, v0.h[2]

    smlal       v18.4s, v10.4h, v2.h[0]
    smlal       v18.4s, v11.4h, v5.h[2]

    cmp         x12,x6
    bhs         stage2_shift2


    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10






    smlal       v24.4s, v14.4h, v2.h[3]
    smlal       v26.4s, v14.4h, v3.h[3]
    smlsl       v28.4s, v14.4h, v5.h[3]
    smlsl       v30.4s, v14.4h, v0.h[3]


    smlal       v24.4s, v15.4h, v1.h[3]
    smlsl       v26.4s, v15.4h, v6.h[3]
    smlsl       v28.4s, v15.4h, v0.h[3]
    smlal       v30.4s, v15.4h, v7.h[3]


    smlal       v20.4s, v12.4h, v5.h[0]
    smlal       v20.4s, v13.4h, v0.h[2]
    smlal       v22.4s, v12.4h, v1.h[0]
    smlal       v22.4s, v13.4h, v6.h[2]
    smlal       v16.4s, v12.4h, v7.h[0]
    smlsl       v16.4s, v13.4h, v2.h[2]
    smlsl       v18.4s, v12.4h, v3.h[0]
    smlsl       v18.4s, v13.4h, v4.h[2]

    cmp         x12,x9
    bhs         stage2_shift2


    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10



    smlal       v24.4s, v8.4h, v6.h[1]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v1.h[1]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v7.h[1]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v0.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v5.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v4.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v2.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v7.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v7.h[2]


    smlsl       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v1.h[2]

    smlsl       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v5.h[2]

    smlal       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v3.h[2]

    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10




    smlsl       v24.4s, v14.4h, v0.h[1]
    smlal       v26.4s, v14.4h, v6.h[1]
    smlal       v28.4s, v14.4h, v4.h[1]
    smlsl       v30.4s, v14.4h, v1.h[1]


    smlsl       v24.4s, v15.4h, v3.h[3]
    smlal       v26.4s, v15.4h, v0.h[1]
    smlsl       v28.4s, v15.4h, v5.h[1]
    smlsl       v30.4s, v15.4h, v6.h[1]


    smlsl       v20.4s, v12.4h, v3.h[0]
    smlsl       v20.4s, v13.4h, v1.h[2]
    smlsl       v22.4s, v12.4h, v7.h[0]
    smlal       v22.4s, v13.4h, v3.h[2]
    smlal       v16.4s, v12.4h, v1.h[0]
    smlal       v16.4s, v13.4h, v7.h[2]
    smlsl       v18.4s, v12.4h, v5.h[0]
    smlsl       v18.4s, v13.4h, v2.h[2]


    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10


    smlal       v24.4s, v8.4h, v7.h[3]     //// y1 * cos1(part of b0)
    smlal       v26.4s, v8.4h, v4.h[3]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v1.h[1]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v2.h[1]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v3.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v5.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v7.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v5.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlsl       v20.4s, v10.4h, v6.h[0]
    smlal       v20.4s, v11.4h, v5.h[2]


    smlal       v22.4s, v10.4h, v2.h[0]
    smlal       v22.4s, v11.4h, v7.h[2]

    smlsl       v16.4s, v10.4h, v2.h[0]
    smlsl       v16.4s, v11.4h, v4.h[2]

    smlal       v18.4s, v10.4h, v6.h[0]
    smlal       v18.4s, v11.4h, v1.h[2]


    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10



    smlal       v24.4s, v14.4h, v1.h[1]
    smlsl       v26.4s, v14.4h, v0.h[3]
    smlal       v28.4s, v14.4h, v1.h[3]
    smlsl       v30.4s, v14.4h, v3.h[1]


    smlal       v24.4s, v15.4h, v5.h[3]
    smlsl       v26.4s, v15.4h, v5.h[1]
    smlal       v28.4s, v15.4h, v4.h[3]
    smlsl       v30.4s, v15.4h, v4.h[1]


    smlal       v20.4s, v12.4h, v1.h[0]
    smlal       v20.4s, v13.4h, v3.h[2]
    smlsl       v22.4s, v12.4h, v3.h[0]
    smlsl       v22.4s, v13.4h, v2.h[2]
    smlal       v16.4s, v12.4h, v5.h[0]
    smlal       v16.4s, v13.4h, v1.h[2]
    smlsl       v18.4s, v12.4h, v7.h[0]
    smlsl       v18.4s, v13.4h, v0.h[2]

stage2_shift2:
    add         v8.4s,  v20.4s ,  v24.4s
    sub         v10.4s,  v20.4s ,  v24.4s

    add         v12.4s,  v22.4s ,  v26.4s
    sub         v24.4s,  v22.4s ,  v26.4s

    add         v14.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s


    sqrshrn     v30.4h, v8.4s,#shift_stage2_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v19.4h, v10.4s,#shift_stage2_idct //// x7 = (a0 - b0 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v31.4h, v14.4s,#shift_stage2_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v18.4h, v26.4s,#shift_stage2_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v12.4h, v12.4s,#shift_stage2_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v15.4h, v24.4s,#shift_stage2_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v13.4h, v16.4s,#shift_stage2_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v14.4h, v28.4s,#shift_stage2_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage2_idct)

    umov        x15,v24.d[0]
    umov        x16,v25.d[0]
    umov        x19,v26.d[0]
    umov        x20,v27.d[0]

    trn1        v24.4h, v30.4h, v12.4h
    trn2        v25.4h, v30.4h, v12.4h
    trn1        v26.4h, v31.4h, v13.4h
    trn2        v27.4h, v31.4h, v13.4h

    trn1        v30.2s, v24.2s, v26.2s
    trn2        v31.2s, v24.2s, v26.2s
    trn1        v12.2s, v25.2s, v27.2s
    trn2        v13.2s, v25.2s, v27.2s

    trn1        v24.4h, v14.4h, v18.4h
    trn2        v25.4h, v14.4h, v18.4h
    trn1        v26.4h, v15.4h, v19.4h
    trn2        v27.4h, v15.4h, v19.4h

    trn1        v14.2s, v24.2s, v26.2s
    trn2        v15.2s, v24.2s, v26.2s
    trn1        v18.2s, v25.2s, v27.2s
    trn2        v19.2s, v25.2s, v27.2s

    mov         v24.d[0],x15
    mov         v25.d[0],x16
    mov         v26.d[0],x19
    mov         v27.d[0],x20

    st1         { v30.4h, v31.4h},[x0],#16
    st1         { v12.4h, v13.4h},[x0],#16
    st1         { v14.4h, v15.4h},[x0],#16
    st1         { v18.4h, v19.4h},[x0],#16


    mov         x1,x4




    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10

    smull       v24.4s, v8.4h, v4.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v8.4h, v4.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v8.4h, v5.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v8.4h, v5.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v3.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v1.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v0.h[2]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v1.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smull       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v7.h[2]


    smull       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v6.h[2]

    smull       v16.4s, v10.4h, v0.h[0]
    smlsl       v16.4s, v11.4h, v5.h[2]

    smull       v18.4s, v10.4h, v0.h[0]
    smlsl       v18.4s, v11.4h, v4.h[2]

    cmp         x12,x11
    bhs         stage2_shift3

    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10

    smlsl       v24.4s, v14.4h, v5.h[1]
    smlsl       v26.4s, v14.4h, v7.h[3]
    smlal       v28.4s, v14.4h, v5.h[3]
    smlal       v30.4s, v14.4h, v3.h[1]


    smlal       v24.4s, v15.4h, v2.h[1]
    smlal       v26.4s, v15.4h, v1.h[1]
    smlal       v28.4s, v15.4h, v4.h[3]
    smlsl       v30.4s, v15.4h, v7.h[3]


    smlsl       v20.4s, v12.4h, v1.h[0]
    smlal       v20.4s, v13.4h, v6.h[2]
    smlsl       v22.4s, v12.4h, v3.h[0]
    smlal       v22.4s, v13.4h, v3.h[2]
    smlsl       v16.4s, v12.4h, v5.h[0]
    smlal       v16.4s, v13.4h, v0.h[2]
    smlsl       v18.4s, v12.4h, v7.h[0]
    smlal       v18.4s, v13.4h, v2.h[2]

    cmp         x12,x5
    bhs         stage2_shift3

    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10



    smlal       v24.4s, v8.4h, v6.h[1]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v5.h[1]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v0.h[3]     //// y1 * sin3(part of b2)
    smlsl       v30.4s, v8.4h, v3.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v1.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v4.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v6.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v0.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v2.h[0]
    smlsl       v20.4s, v11.4h, v5.h[2]


    smlal       v22.4s, v10.4h, v6.h[0]
    smlsl       v22.4s, v11.4h, v0.h[2]

    smlsl       v16.4s, v10.4h, v6.h[0]
    smlsl       v16.4s, v11.4h, v4.h[2]

    smlsl       v18.4s, v10.4h, v2.h[0]
    smlal       v18.4s, v11.4h, v6.h[2]

    cmp         x12,x6
    bhs         stage2_shift3

    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10





    smlsl       v24.4s, v14.4h, v7.h[1]
    smlal       v26.4s, v14.4h, v2.h[1]
    smlal       v28.4s, v14.4h, v4.h[1]
    smlsl       v30.4s, v14.4h, v5.h[1]


    smlal       v24.4s, v15.4h, v0.h[3]
    smlal       v26.4s, v15.4h, v7.h[1]
    smlsl       v28.4s, v15.4h, v1.h[1]
    smlsl       v30.4s, v15.4h, v6.h[1]


    smlsl       v20.4s, v12.4h, v3.h[0]
    smlal       v20.4s, v13.4h, v4.h[2]
    smlal       v22.4s, v12.4h, v7.h[0]
    smlal       v22.4s, v13.4h, v2.h[2]
    smlal       v16.4s, v12.4h, v1.h[0]
    smlsl       v16.4s, v13.4h, v6.h[2]
    smlal       v18.4s, v12.4h, v5.h[0]
    smlsl       v18.4s, v13.4h, v0.h[2]

    cmp         x12,x9
    bhs         stage2_shift3


    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10


    smlsl       v24.4s, v8.4h, v7.h[3]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v0.h[1]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v6.h[3]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v0.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v9.4h, v5.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v3.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v2.h[3]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v3.h[2]


    smlsl       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v5.h[2]

    smlsl       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v1.h[2]

    smlal       v18.4s, v10.4h, v0.h[0]
    smlal       v18.4s, v11.4h, v7.h[2]

    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10




    smlal       v24.4s, v14.4h, v6.h[3]
    smlal       v26.4s, v14.4h, v3.h[3]
    smlsl       v28.4s, v14.4h, v1.h[3]
    smlal       v30.4s, v14.4h, v7.h[1]


    smlal       v24.4s, v15.4h, v1.h[3]
    smlsl       v26.4s, v15.4h, v2.h[3]
    smlal       v28.4s, v15.4h, v7.h[1]
    smlal       v30.4s, v15.4h, v4.h[1]


    smlsl       v20.4s, v12.4h, v5.h[0]
    smlal       v20.4s, v13.4h, v2.h[2]
    smlal       v22.4s, v12.4h, v1.h[0]
    smlsl       v22.4s, v13.4h, v7.h[2]
    smlsl       v16.4s, v12.4h, v7.h[0]
    smlsl       v16.4s, v13.4h, v3.h[2]
    smlsl       v18.4s, v12.4h, v3.h[0]
    smlal       v18.4s, v13.4h, v1.h[2]


    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10


    smlsl       v24.4s, v8.4h, v5.h[3]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v6.h[3]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v3.h[1]     //// y1 * sin3(part of b2)
    smlsl       v30.4s, v8.4h, v0.h[1]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v2.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v9.4h, v0.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v2.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlal       v30.4s, v9.4h, v4.h[3]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v6.h[0]
    smlsl       v20.4s, v11.4h, v1.h[2]


    smlsl       v22.4s, v10.4h, v2.h[0]
    smlal       v22.4s, v11.4h, v4.h[2]

    smlal       v16.4s, v10.4h, v2.h[0]
    smlsl       v16.4s, v11.4h, v7.h[2]

    smlsl       v18.4s, v10.4h, v6.h[0]
    smlsl       v18.4s, v11.4h, v5.h[2]

    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10



    smlal       v24.4s, v14.4h, v4.h[3]
    smlsl       v26.4s, v14.4h, v6.h[1]
    smlal       v28.4s, v14.4h, v7.h[3]
    smlal       v30.4s, v14.4h, v6.h[3]


    smlal       v24.4s, v15.4h, v3.h[3]
    smlsl       v26.4s, v15.4h, v3.h[1]
    smlal       v28.4s, v15.4h, v2.h[3]
    smlsl       v30.4s, v15.4h, v2.h[1]


    smlsl       v20.4s, v12.4h, v7.h[0]
    smlal       v20.4s, v13.4h, v0.h[2]
    smlal       v22.4s, v12.4h, v5.h[0]
    smlsl       v22.4s, v13.4h, v1.h[2]
    smlsl       v16.4s, v12.4h, v3.h[0]
    smlal       v16.4s, v13.4h, v2.h[2]
    smlal       v18.4s, v12.4h, v1.h[0]
    smlsl       v18.4s, v13.4h, v3.h[2]

stage2_shift3:
    add         v8.4s,  v20.4s ,  v24.4s
    sub         v10.4s,  v20.4s ,  v24.4s

    add         v12.4s,  v22.4s ,  v26.4s
    sub         v24.4s,  v22.4s ,  v26.4s

    add         v14.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s


    sqrshrn     v30.4h, v8.4s,#shift_stage2_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v19.4h, v10.4s,#shift_stage2_idct //// x11 = (a0 - b0 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v31.4h, v14.4s,#shift_stage2_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v18.4h, v26.4s,#shift_stage2_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v12.4h, v12.4s,#shift_stage2_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v15.4h, v24.4s,#shift_stage2_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v13.4h, v16.4s,#shift_stage2_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v14.4h, v28.4s,#shift_stage2_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage2_idct)

    umov        x15,v24.d[0]
    umov        x16,v25.d[0]
    umov        x19,v26.d[0]
    umov        x20,v27.d[0]

    trn1        v24.4h, v30.4h, v12.4h
    trn2        v25.4h, v30.4h, v12.4h
    trn1        v26.4h, v31.4h, v13.4h
    trn2        v27.4h, v31.4h, v13.4h

    trn1        v30.2s, v24.2s, v26.2s
    trn2        v31.2s, v24.2s, v26.2s
    trn1        v12.2s, v25.2s, v27.2s
    trn2        v13.2s, v25.2s, v27.2s

    trn1        v24.4h, v14.4h, v18.4h
    trn2        v25.4h, v14.4h, v18.4h
    trn1        v26.4h, v15.4h, v19.4h
    trn2        v27.4h, v15.4h, v19.4h

    trn1        v14.2s, v24.2s, v26.2s
    trn2        v15.2s, v24.2s, v26.2s
    trn1        v18.2s, v25.2s, v27.2s
    trn2        v19.2s, v25.2s, v27.2s

    mov         v24.d[0],x15
    mov         v25.d[0],x16
    mov         v26.d[0],x19
    mov         v27.d[0],x20

    st1         { v30.4h, v31.4h},[x0],#16
    st1         { v12.4h, v13.4h},[x0],#16
    st1         { v14.4h, v15.4h},[x0],#16
    st1         { v18.4h, v19.4h},[x0],#16



    mov         x1,x4




    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10


    smull       v24.4s, v8.4h, v6.h[1]     //// y1 * cos1(part of b0)
    smull       v26.4s, v8.4h, v6.h[3]     //// y1 * cos3(part of b1)
    smull       v28.4s, v8.4h, v7.h[1]     //// y1 * sin3(part of b2)
    smull       v30.4s, v8.4h, v7.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v2.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v4.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v5.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v7.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smull       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v3.h[2]


    smull       v22.4s, v10.4h, v0.h[0]
    smlsl       v22.4s, v11.4h, v2.h[2]

    smull       v16.4s, v10.4h, v0.h[0]
    smlsl       v16.4s, v11.4h, v1.h[2]

    smull       v18.4s, v10.4h, v0.h[0]
    smlsl       v18.4s, v11.4h, v0.h[2]

    cmp         x12,x11
    bhs         stage2_shift4
    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10






    smlal       v24.4s, v14.4h, v0.h[1]
    smlal       v26.4s, v14.4h, v1.h[3]
    smlal       v28.4s, v14.4h, v4.h[1]
    smlal       v30.4s, v14.4h, v6.h[3]


    smlsl       v24.4s, v15.4h, v4.h[1]
    smlsl       v26.4s, v15.4h, v0.h[3]
    smlsl       v28.4s, v15.4h, v2.h[3]
    smlsl       v30.4s, v15.4h, v6.h[1]


    smlal       v20.4s, v12.4h, v7.h[0]
    smlal       v20.4s, v13.4h, v5.h[2]
    smlal       v22.4s, v12.4h, v5.h[0]
    smlsl       v22.4s, v13.4h, v7.h[2]
    smlal       v16.4s, v12.4h, v3.h[0]
    smlsl       v16.4s, v13.4h, v4.h[2]
    smlal       v18.4s, v12.4h, v1.h[0]
    smlsl       v18.4s, v13.4h, v1.h[2]

    cmp         x12,x5
    bhs         stage2_shift4

    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10



    smlal       v24.4s, v8.4h, v7.h[3]     //// y1 * cos1(part of b0)
    smlal       v26.4s, v8.4h, v3.h[1]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v1.h[1]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v5.h[3]     //// y1 * sin1(part of b3)

    smlal       v24.4s, v9.4h, v4.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v5.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v0.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v5.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlsl       v20.4s, v10.4h, v2.h[0]
    smlal       v20.4s, v11.4h, v1.h[2]


    smlsl       v22.4s, v10.4h, v6.h[0]
    smlal       v22.4s, v11.4h, v3.h[2]

    smlal       v16.4s, v10.4h, v6.h[0]
    smlsl       v16.4s, v11.4h, v7.h[2]

    smlal       v18.4s, v10.4h, v2.h[0]
    smlsl       v18.4s, v11.4h, v2.h[2]

    cmp         x12,x6
    bhs         stage2_shift4


    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10






    smlsl       v24.4s, v14.4h, v1.h[1]
    smlsl       v26.4s, v14.4h, v7.h[3]
    smlal       v28.4s, v14.4h, v1.h[3]
    smlal       v30.4s, v14.4h, v4.h[3]


    smlal       v24.4s, v15.4h, v2.h[1]
    smlal       v26.4s, v15.4h, v5.h[1]
    smlsl       v28.4s, v15.4h, v3.h[1]
    smlsl       v30.4s, v15.4h, v4.h[1]


    smlsl       v20.4s, v12.4h, v5.h[0]
    smlsl       v20.4s, v13.4h, v7.h[2]
    smlsl       v22.4s, v12.4h, v1.h[0]
    smlal       v22.4s, v13.4h, v1.h[2]
    smlsl       v16.4s, v12.4h, v7.h[0]
    smlal       v16.4s, v13.4h, v5.h[2]
    smlal       v18.4s, v12.4h, v3.h[0]
    smlsl       v18.4s, v13.4h, v3.h[2]

    cmp         x12,x9
    bhs         stage2_shift4


    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10


    smlsl       v24.4s, v8.4h, v5.h[3]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v2.h[3]     //// y1 * cos3(part of b1)
    smlal       v28.4s, v8.4h, v4.h[3]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v3.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v6.h[3]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlal       v26.4s, v9.4h, v0.h[3]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlsl       v28.4s, v9.4h, v6.h[1]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v3.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlal       v20.4s, v10.4h, v0.h[0]
    smlsl       v20.4s, v11.4h, v0.h[2]


    smlsl       v22.4s, v10.4h, v0.h[0]
    smlal       v22.4s, v11.4h, v6.h[2]

    smlsl       v16.4s, v10.4h, v0.h[0]
    smlal       v16.4s, v11.4h, v2.h[2]

    smlal       v18.4s, v10.4h, v0.h[0]
    smlsl       v18.4s, v11.4h, v4.h[2]

    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10




    smlal       v24.4s, v14.4h, v3.h[1]
    smlsl       v26.4s, v14.4h, v2.h[1]
    smlal       v28.4s, v14.4h, v7.h[3]
    smlal       v30.4s, v14.4h, v2.h[3]


    smlsl       v24.4s, v15.4h, v0.h[3]
    smlal       v26.4s, v15.4h, v4.h[3]
    smlal       v28.4s, v15.4h, v6.h[3]
    smlsl       v30.4s, v15.4h, v2.h[1]


    smlal       v20.4s, v12.4h, v3.h[0]
    smlsl       v20.4s, v13.4h, v6.h[2]
    smlal       v22.4s, v12.4h, v7.h[0]
    smlsl       v22.4s, v13.4h, v4.h[2]
    smlsl       v16.4s, v12.4h, v1.h[0]
    smlal       v16.4s, v13.4h, v0.h[2]
    smlal       v18.4s, v12.4h, v5.h[0]
    smlsl       v18.4s, v13.4h, v5.h[2]


    ld1         {v10.4h, v11.4h},[x1],#16
    ld1         {v8.4h, v9.4h},[x1],x10




    smlal       v24.4s, v8.4h, v3.h[3]     //// y1 * cos1(part of b0)
    smlsl       v26.4s, v8.4h, v7.h[1]     //// y1 * cos3(part of b1)
    smlsl       v28.4s, v8.4h, v5.h[1]     //// y1 * sin3(part of b2)
    smlal       v30.4s, v8.4h, v1.h[3]     //// y1 * sin1(part of b3)

    smlsl       v24.4s, v9.4h, v7.h[1]     //// y1 * cos1 + y3 * cos3(part of b0)
    smlsl       v26.4s, v9.4h, v6.h[1]     //// y1 * cos3 - y3 * sin1(part of b1)
    smlal       v28.4s, v9.4h, v3.h[3]     //// y1 * sin3 - y3 * cos1(part of b2)
    smlsl       v30.4s, v9.4h, v1.h[1]     //// y1 * sin1 - y3 * sin3(part of b3)





    smlsl       v20.4s, v10.4h, v6.h[0]
    smlal       v20.4s, v11.4h, v2.h[2]


    smlal       v22.4s, v10.4h, v2.h[0]
    smlsl       v22.4s, v11.4h, v0.h[2]

    smlsl       v16.4s, v10.4h, v2.h[0]
    smlal       v16.4s, v11.4h, v3.h[2]

    smlal       v18.4s, v10.4h, v6.h[0]
    smlsl       v18.4s, v11.4h, v6.h[2]


    ld1         {v12.4h, v13.4h},[x1],#16
    ld1         {v14.4h, v15.4h},[x1],x10



    smlsl       v24.4s, v14.4h, v5.h[1]
    smlal       v26.4s, v14.4h, v3.h[3]
    smlsl       v28.4s, v14.4h, v2.h[1]
    smlal       v30.4s, v14.4h, v0.h[3]


    smlal       v24.4s, v15.4h, v1.h[3]
    smlsl       v26.4s, v15.4h, v1.h[1]
    smlal       v28.4s, v15.4h, v0.h[3]
    smlsl       v30.4s, v15.4h, v0.h[1]


    smlsl       v20.4s, v12.4h, v1.h[0]
    smlal       v20.4s, v13.4h, v4.h[2]
    smlal       v22.4s, v12.4h, v3.h[0]
    smlsl       v22.4s, v13.4h, v5.h[2]
    smlsl       v16.4s, v12.4h, v5.h[0]
    smlal       v16.4s, v13.4h, v6.h[2]
    smlal       v18.4s, v12.4h, v7.h[0]
    smlsl       v18.4s, v13.4h, v7.h[2]

stage2_shift4:
    add         v8.4s,  v20.4s ,  v24.4s
    sub         v10.4s,  v20.4s ,  v24.4s

    add         v12.4s,  v22.4s ,  v26.4s
    sub         v24.4s,  v22.4s ,  v26.4s

    add         v14.4s,  v16.4s ,  v28.4s
    sub         v26.4s,  v16.4s ,  v28.4s


    add         v16.4s,  v18.4s ,  v30.4s
    sub         v28.4s,  v18.4s ,  v30.4s


    sqrshrn     v30.4h, v8.4s,#shift_stage2_idct //// x0 = (a0 + b0 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v19.4h, v10.4s,#shift_stage2_idct //// x11 = (a0 - b0 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v31.4h, v14.4s,#shift_stage2_idct //// x2 = (a2 + b2 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v18.4h, v26.4s,#shift_stage2_idct //// x5 = (a2 - b2 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v12.4h, v12.4s,#shift_stage2_idct //// x1 = (a1 + b1 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v15.4h, v24.4s,#shift_stage2_idct //// x6 = (a1 - b1 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v13.4h, v16.4s,#shift_stage2_idct //// x3 = (a3 + b3 + rnd) >> 7(shift_stage2_idct)
    sqrshrn     v14.4h, v28.4s,#shift_stage2_idct //// x4 = (a3 - b3 + rnd) >> 7(shift_stage2_idct)



    umov        x15,v24.d[0]
    umov        x16,v25.d[0]
    umov        x19,v26.d[0]
    umov        x20,v27.d[0]

    trn1        v24.4h, v30.4h, v12.4h
    trn2        v25.4h, v30.4h, v12.4h
    trn1        v26.4h, v31.4h, v13.4h
    trn2        v27.4h, v31.4h, v13.4h

    trn1        v30.2s, v24.2s, v26.2s
    trn2        v31.2s, v24.2s, v26.2s
    trn1        v12.2s, v25.2s, v27.2s
    trn2        v13.2s, v25.2s, v27.2s

    trn1        v24.4h, v14.4h, v18.4h
    trn2        v25.4h, v14.4h, v18.4h
    trn1        v26.4h, v15.4h, v19.4h
    trn2        v27.4h, v15.4h, v19.4h

    trn1        v14.2s, v24.2s, v26.2s
    trn2        v15.2s, v24.2s, v26.2s
    trn1        v18.2s, v25.2s, v27.2s
    trn2        v19.2s, v25.2s, v27.2s

    mov         v24.d[0],x15
    mov         v25.d[0],x16
    mov         v26.d[0],x19
    mov         v27.d[0],x20

    st1         { v30.4h, v31.4h},[x0],#16
    st1         { v12.4h, v13.4h},[x0],#16
    st1         { v14.4h, v15.4h},[x0],#16
    st1         { v18.4h, v19.4h},[x0],#16




    sub         x0,x0,#256
prediction_buffer:


    ld1         {v12.8h},[x0],#16
    ld1         {v14.8h},[x0],#16

    add         x0,x0,#32

    ld1         {v16.8h},[x0],#16
    ld1         {v18.8h},[x0],#16
    add         x0,x0,#32

    ld1         {v20.8h},[x0],#16
    ld1         {v22.8h},[x0],#16


    add         x0,x0,#32

    ld1         {v24.8h},[x0],#16
    ld1         {v26.8h},[x0],#16





// d12 =x0 1- 4 values
// d13 =x2 1- 4 values
// d14=x1 1- 4 values
// d15=x3 1- 4 values

// d16 =x0 5- 8 values
// d17 =x2 5- 8 values
// d18=x1 5- 8 values
// d19=x3 5- 8 values

// d20 =x0 9- 12 values
// d21 =x2 9- 12 values
// d22=x1 9- 12 values
// d23=x3 9- 12 values

// d24 =x0 13-16 values
// d25 =x2 13- 16 values
// d26=x1 13- 16 values
// d27=x3 13- 16 values

    // swapping v12 upper and v16 lower 64bits
    mov         v13.d[0], v12.d[1]
    mov         v12.d[1], v16.d[0]
    mov         v16.d[0], v13.d[0]
    // swapping v20 upper and v24 lower 64bits
    mov         v21.d[0], v20.d[1]
    mov         v20.d[1], v24.d[0]
    mov         v24.d[0], v21.d[0]
    // swapping v14 uppper and v18 lower 64bits
    mov         v15.d[0], v14.d[1]
    mov         v14.d[1], v18.d[0]
    mov         v18.d[0], v15.d[0]
    // swapping v22 upper and v26 lower 64bits
    mov         v23.d[0], v22.d[1]
    mov         v22.d[1], v26.d[0]
    mov         v26.d[0], v23.d[0]


    ld1         {v8.8b, v9.8b},[x2],x8
    ld1         {v10.8b, v11.8b},[x2],x8
    ld1         {v28.8b, v29.8b},[x2],x8
    ld1         {v30.8b, v31.8b},[x2],x8


    uaddw       v12.8h,  v12.8h ,  v8.8b
    uaddw       v20.8h,  v20.8h ,  v9.8b
    uaddw       v14.8h,  v14.8h ,  v10.8b
    uaddw       v22.8h,  v22.8h ,  v11.8b
    uaddw       v16.8h,  v16.8h ,  v28.8b
    uaddw       v24.8h,  v24.8h ,  v29.8b
    uaddw       v18.8h,  v18.8h ,  v30.8b
    uaddw       v26.8h,  v26.8h ,  v31.8b
    sub         x2,x2,x8,lsl #2
    add         x2,x2,#16
    sqxtun      v12.8b, v12.8h
    sqxtun      v13.8b, v20.8h
    sqxtun      v20.8b, v14.8h
    sqxtun      v21.8b, v22.8h
    sqxtun      v14.8b, v16.8h
    sqxtun      v15.8b, v24.8h
    sqxtun      v22.8b, v18.8h
    sqxtun      v23.8b, v26.8h


    st1         {v12.8b, v13.8b},[x3],x7
    st1         {v20.8b, v21.8b},[x3],x7
    st1         {v14.8b, v15.8b},[x3],x7
    st1         {v22.8b, v23.8b},[x3],x7


    sub         x3,x3,x7,lsl #2
    add         x3,x3,#16

    ld1         {v12.8h},[x0],#16
    ld1         {v14.8h},[x0],#16

    sub         x0,x0,#96

    ld1         {v16.8h},[x0],#16
    ld1         {v18.8h},[x0],#16
    sub         x0,x0,#96

    ld1         {v20.8h},[x0],#16
    ld1         {v22.8h},[x0],#16


    sub         x0,x0,#96

    ld1         {v24.8h},[x0],#16
    ld1         {v26.8h},[x0],#16


    sub         x0,x0,#64


    // swapping v12 upper and v16 lower 64bits
    mov         v13.d[0], v12.d[1]
    mov         v12.d[1], v16.d[0]
    mov         v16.d[0], v13.d[0]
    // swapping v20 upper and v24 lower 64bits
    mov         v21.d[0], v20.d[1]
    mov         v20.d[1], v24.d[0]
    mov         v24.d[0], v21.d[0]
    // swapping v14 uppper and v18 lower 64bits
    mov         v15.d[0], v14.d[1]
    mov         v14.d[1], v18.d[0]
    mov         v18.d[0], v15.d[0]
    // swapping v22 upper and v26 lower 64bits
    mov         v23.d[0], v22.d[1]
    mov         v22.d[1], v26.d[0]
    mov         v26.d[0], v23.d[0]


    ld1         {v8.8b, v9.8b},[x2],x8
    ld1         {v10.8b, v11.8b},[x2],x8
    ld1         {v28.8b, v29.8b},[x2],x8
    ld1         {v30.8b, v31.8b},[x2],x8


    uaddw       v12.8h,  v12.8h ,  v8.8b
    uaddw       v20.8h,  v20.8h ,  v9.8b
    uaddw       v14.8h,  v14.8h ,  v10.8b
    uaddw       v22.8h,  v22.8h ,  v11.8b
    uaddw       v16.8h,  v16.8h ,  v28.8b
    uaddw       v24.8h,  v24.8h ,  v29.8b
    uaddw       v18.8h,  v18.8h ,  v30.8b
    uaddw       v26.8h,  v26.8h ,  v31.8b
    sub         x2,x2,#16

    sqxtun      v12.8b, v12.8h
    sqxtun      v13.8b, v20.8h
    sqxtun      v20.8b, v14.8h
    sqxtun      v21.8b, v22.8h
    sqxtun      v14.8b, v16.8h
    sqxtun      v15.8b, v24.8h
    sqxtun      v22.8b, v18.8h
    sqxtun      v23.8b, v26.8h


    st1         {v12.8b, v13.8b},[x3],x7
    st1         {v20.8b, v21.8b},[x3],x7
    st1         {v14.8b, v15.8b},[x3],x7
    st1         {v22.8b, v23.8b},[x3],x7

    sub         x3,x3,#16

    subs        x14,x14,#1
    bne         dct_stage2
    // ldmfd sp!,{x0-x12,pc}
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret





