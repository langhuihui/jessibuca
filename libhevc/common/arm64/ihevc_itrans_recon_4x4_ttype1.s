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
// *  ihevc_itrans_recon_4x4_ttype1.s
// *
// * @brief
// *  contains function definitions for inverse transform  and reconstruction
// *
// *
// * @author
// *  naveen sr
// *
// * @par list of functions:
// *  - ihevc_itrans_recon_4x4_ttype1()
// *
// * @remarks
// *  none
// *
// *******************************************************************************
// */

///* all the functions here are replicated from ihevc_itrans.c and modified to */
///* include reconstruction */
//
///**
// *******************************************************************************
// *
// * @brief
// *  this function performs inverse transform type 1 (dst)  and reconstruction
// * for 4x4 input block
// *
// * @par description:
// *  performs inverse transform and adds the prediction  data and clips output
// * to 8 bit
// *
// * @param[in] pi2_src
// *  input 4x4 coefficients
// *
// * @param[in] pi2_tmp
// *  temporary 4x4 buffer for storing inverse
// *
// *  transform
// *  1st stage output
// *
// * @param[in] pu1_pred
// *  prediction 4x4 block
// *
// * @param[out] pu1_dst
// *  output 4x4 block
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
//void ihevc_itrans_recon_4x4_ttype1(word16 *pi2_src,
//        word16 *pi2_tmp,
//        uword8 *pu1_pred,
//        uword8 *pu1_dst,
//        word32 src_strd,
//        word32 pred_strd,
//        word32 dst_strd,
//        word32 zero_cols)

//**************variables vs registers*************************
//    x0 => *pi2_src
//    x1 => *pi2_tmp
//    x2 => *pu1_pred
//    x3 => *pu1_dst
//    x4 => src_strd
//    x5 => pred_strd
//    x6 => dst_strd
//    x7 => zero_cols

.text
.align 4

.include "ihevc_neon_macros.s"

.set shift_stage1_idct ,   7
.set shift_stage2_idct ,   12

.globl ihevc_itrans_recon_4x4_ttype1_av8

.type ihevc_itrans_recon_4x4_ttype1_av8, %function

ihevc_itrans_recon_4x4_ttype1_av8:

    // stmfd sp!, {x4-x12, x14}    //stack stores the values of the arguments

    stp         x19, x20,[sp,#-16]!

    add         x4,x4,x4                    // src_strd in terms of word16

    mov         x8,#29
    mov         x9,#55
    mov         x10,#74
    mov         x11,#84
    mov         v4.h[0], w8
    ld1         {v0.4h},[x0],x4             //loading pi2_src 1st row
    mov         v4.h[1], w9
    ld1         {v1.4h},[x0],x4             //loading pi2_src 2nd row
    mov         v4.h[2], w10
    ld1         {v2.4h},[x0],x4             //loading pi2_src 3rd row
    mov         v4.h[3], w11
    ld1         {v3.4h},[x0],x4             //loading pi2_src 4th row

    // first stage computation starts
    smull       v6.4s, v1.4h, v4.h[2]       //74 * pi2_src[1]
    smlal       v6.4s, v0.4h, v4.h[0]       //74 * pi2_src[1] + 29 * pi2_src[0]
    smlal       v6.4s, v3.4h, v4.h[1]       //74 * pi2_src[1] + 29 * pi2_src[0] + 55 * pi2_src[3]
    smlal       v6.4s, v2.4h, v4.h[3]       //pi2_out[0] = 29* pi2_src[0] + 74 * pi2_src[1] + 84* pi2_src[2] + 55 * pi2_src[3]

    smull       v5.4s, v1.4h, v4.h[2]       //74 * pi2_src[1]
    smlal       v5.4s, v0.4h, v4.h[1]       //74 * pi2_src[1] + 55 * pi2_src[0]
    smlsl       v5.4s, v2.4h, v4.h[0]       //74 * pi2_src[1] + 55 * pi2_src[0] -  29 * pi2_src[2]
    smlsl       v5.4s, v3.4h, v4.h[3]       //pi2_out[1] = 74 * pi2_src[1] + 55 * pi2_src[0] -  29 * pi2_src[2] - 84 * pi2_src[3])

    smull       v7.4s, v0.4h, v4.h[2]       // 74 * pi2_src[0]
    smlsl       v7.4s, v2.4h, v4.h[2]       // 74 * pi2_src[0] - 74 * pi2_src[2]
    smlal       v7.4s, v3.4h, v4.h[2]       //pi2_out[2] = 74 * pi2_src[0] - 74 * pi2_src[2] + 74 * pi2_src[3]

    smull       v20.4s, v2.4h, v4.h[1]      // 55 * pi2_src[2]
    smlsl       v20.4s, v1.4h, v4.h[2]      // 55 * pi2_src[2] - 74 * pi2_src[1]
    smlsl       v20.4s, v3.4h, v4.h[0]      // - 74 * pi2_src[1] +   55 * pi2_src[2]    - 29 * pi2_src[3]
    smlal       v20.4s, v0.4h, v4.h[3]      //pi2_out[3] = 84 * pi2_src[0] - 74 * pi2_src[1] + 55 * pi2_src[2] - 29 * pi2_src[3]

    sqrshrn     v28.4h, v6.4s,#shift_stage1_idct // (pi2_out[0] + rounding ) >> shift_stage1_idct
    sqrshrn     v29.4h, v5.4s,#shift_stage1_idct // (pi2_out[1] + rounding ) >> shift_stage1_idct
    sqrshrn     v30.4h, v7.4s,#shift_stage1_idct // (pi2_out[2] + rounding ) >> shift_stage1_idct
    sqrshrn     v31.4h, v20.4s,#shift_stage1_idct // (pi2_out[3] + rounding ) >> shift_stage1_idct
    ld1         {v18.s}[0],[x2],x5

    trn1        v24.4h, v28.4h, v29.4h
    trn2        v25.4h, v28.4h, v29.4h
    trn1        v26.4h, v30.4h, v31.4h
    trn2        v27.4h, v30.4h, v31.4h
    trn1        v21.2s, v24.2s, v26.2s
    trn2        v16.2s, v24.2s, v26.2s
    trn1        v22.2s, v25.2s, v27.2s
    trn2        v17.2s, v25.2s, v27.2s
    // output in d14,d15,d16,d17
    // first stage computation ends

    // second stage computation starts  :  copy pasting 1st stage
    // register changes
    // d14 - d0
    // d15 - d1
    // d16 - d2
    // d17 - d3
    ld1         {v18.s}[1],[x2],x5
    smull       v6.4s, v22.4h, v4.h[2]      //74 * pi2_src[1]
    smlal       v6.4s, v21.4h, v4.h[0]      //74 * pi2_src[1] + 29 * pi2_src[0]
    smlal       v6.4s, v17.4h, v4.h[1]      //74 * pi2_src[1] + 29 * pi2_src[0] + 55 * pi2_src[3]
    smlal       v6.4s, v16.4h, v4.h[3]      //pi2_out[0] = 29* pi2_src[0] + 74 * pi2_src[1] + 84* pi2_src[2] + 55 * pi2_src[3]

    smull       v5.4s, v22.4h, v4.h[2]      //74 * pi2_src[1]
    smlal       v5.4s, v21.4h, v4.h[1]      //74 * pi2_src[1] + 55 * pi2_src[0]
    smlsl       v5.4s, v16.4h, v4.h[0]      //74 * pi2_src[1] + 55 * pi2_src[0] -  29 * pi2_src[2]
    smlsl       v5.4s, v17.4h, v4.h[3]      //pi2_out[1] = 74 * pi2_src[1] + 55 * pi2_src[0] -  29 * pi2_src[2] - 84 * pi2_src[3])

    smull       v7.4s, v21.4h, v4.h[2]      // 74 * pi2_src[0]
    smlsl       v7.4s, v16.4h, v4.h[2]      // 74 * pi2_src[0] - 74 * pi2_src[2]
    smlal       v7.4s, v17.4h, v4.h[2]      //pi2_out[2] = 74 * pi2_src[0] - 74 * pi2_src[2] + 74 * pi2_src[3]
    ld1         {v19.s}[0],[x2],x5

    smull       v20.4s, v16.4h, v4.h[1]     // 55 * pi2_src[2]
    smlsl       v20.4s, v22.4h, v4.h[2]     //  - 74 * pi2_src[1] +   55 * pi2_src[2]
    smlsl       v20.4s, v17.4h, v4.h[0]     // - 74 * pi2_src[1] +   55 * pi2_src[2]    - 29 * pi2_src[3]
    smlal       v20.4s, v21.4h, v4.h[3]     //pi2_out[3] = 84 * pi2_src[0] - 74 * pi2_src[1] + 55 * pi2_src[2] - 29 * pi2_src[3]

    sqrshrn     v28.4h, v6.4s,#shift_stage2_idct // (pi2_out[0] + rounding ) >> shift_stage1_idct
    sqrshrn     v29.4h, v5.4s,#shift_stage2_idct // (pi2_out[1] + rounding ) >> shift_stage1_idct
    sqrshrn     v30.4h, v7.4s,#shift_stage2_idct // (pi2_out[2] + rounding ) >> shift_stage1_idct
    sqrshrn     v31.4h, v20.4s,#shift_stage2_idct // (pi2_out[3] + rounding ) >> shift_stage1_idct
    ld1         {v19.s}[1],[x2],x5
    trn1        v24.4h, v28.4h, v29.4h
    trn2        v25.4h, v28.4h, v29.4h
    trn1        v26.4h, v30.4h, v31.4h
    trn2        v27.4h, v30.4h, v31.4h
    trn1        v0.2s, v24.2s, v26.2s
    trn2        v2.2s, v24.2s, v26.2s
    trn1        v1.2s, v25.2s, v27.2s
    trn2        v3.2s, v25.2s, v27.2s
    // output in d0,d1,d2,d3
    // second stage computation ends

    // loading pred
    mov         v0.d[1],v1.d[0]
    mov         v2.d[1],v3.d[0]

    uaddw       v0.8h,  v0.8h ,  v18.8b     // pi2_out(16bit) + pu1_pred(8bit)
    sqxtun      v0.8b, v0.8h                // clip_u8(pi2_out(16bit) + pu1_pred(8bit))
    uaddw       v2.8h,  v2.8h ,  v19.8b     // pi2_out(16bit) + pu1_pred(8bit)
    sqxtun      v1.8b, v2.8h                // clip_u8(pi2_out(16bit) + pu1_pred(8bit))

    // storing destination
    st1         {v0.s}[0],[x3],x6
    st1         {v0.s}[1],[x3],x6
    st1         {v1.s}[0],[x3],x6
    st1         {v1.s}[1],[x3],x6

    // ldmfd sp!,{x4-x12,x15}            //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret









