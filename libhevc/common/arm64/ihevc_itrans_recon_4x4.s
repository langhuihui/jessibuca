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
// *  ihevc_itrans_recon_4x4_neon.s
// *
// * @brief
// *  contains function definitions for single stage  inverse transform
// *
// * @author
// *     naveen sr
// *
// * @par list of functions:
// *  - ihevc_itrans_recon_4x4()
// *
// * @remarks
// *  none
// *
// *******************************************************************************
//*/
// /**
// *******************************************************************************
// *
// * @brief
// *  this function performs inverse transform  and reconstruction for 4x4
// * input block
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
//void ihevc_itrans_recon_4x4(word16 *pi2_src,
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



.globl ihevc_itrans_recon_4x4_av8

.extern g_ai2_ihevc_trans_4_transpose

.type ihevc_itrans_recon_4x4_av8, %function

ihevc_itrans_recon_4x4_av8:

    // stmfd sp!, {x4-x12, x14}                //stack stores the values of the arguments

    stp         x19, x20,[sp,#-16]!

    adrp        x8, :got:g_ai2_ihevc_trans_4_transpose
    ldr         x8, [x8, #:got_lo12:g_ai2_ihevc_trans_4_transpose]

    add         x4,x4,x4                    // src_strd in terms of word16
    add         x9,x0,x4                    // pi2_src[0] + src_strd

    ld1         {v4.4h},[x8]                //loading first row of g_ai2_ihevc_trans_4_transpose
    // d4 = {36,64,83,64}
    //index = 3  2  1  0
    add         x10,x9,x4, lsl #1           // 3*src_strd
    add         x4,x4,x4
    ld1         {v1.4h},[x9]                //loading pi2_src 2nd row
    ld1         {v3.4h},[x10]               //loading pi2_src 4th row
    ld1         {v0.4h},[x0],x4             //loading pi2_src 1st row
    ld1         {v2.4h},[x0],x4             //loading pi2_src 3rd row


    // first stage computation starts
    smull       v6.4s, v1.4h, v4.h[1]       //83 * pi2_src[1]
    smlal       v6.4s, v3.4h, v4.h[3]       //o[0] = 83 * pi2_src[1] + 36 * pi2_src[3]
    smull       v5.4s, v1.4h, v4.h[3]       //36 * pi2_src[1]
    ld1         {v22.s}[0],[x2],x5
    smlsl       v5.4s, v3.4h, v4.h[1]       //o[1] = 36 * pi2_src[1] - 83 * pi2_src[3]

    saddl       v7.4s, v0.4h, v2.4h         //pi2_src[0] + pi2_src[2]
    ssubl       v17.4s, v0.4h, v2.4h        //pi2_src[0] - pi2_src[2]
    shl         v7.4s, v7.4s,#6             //e[0] = 64*(pi2_src[0] + pi2_src[2])
    shl         v17.4s, v17.4s,#6           //e[1] = 64*(pi2_src[0] - pi2_src[2])

    add         v19.4s,  v7.4s ,  v6.4s     //((e[0] + o[0] )
    add         v16.4s,  v17.4s ,  v5.4s    //((e[1] + o[1])
    sub         v18.4s,  v17.4s ,  v5.4s    //((e[1] - o[1])
    sub         v20.4s,  v7.4s ,  v6.4s     //((e[0] - o[0])

    sqrshrn     v28.4h, v19.4s,#shift_stage1_idct //pi2_out[0] = clip_s16((e[0] + o[0] + add)>>shift) )
    sqrshrn     v29.4h, v16.4s,#shift_stage1_idct //pi2_out[1] = clip_s16((e[1] + o[1] + add)>>shift) )
    sqrshrn     v30.4h, v18.4s,#shift_stage1_idct //pi2_out[2] = clip_s16((e[0] - o[0] + add)>>shift) )
    sqrshrn     v31.4h, v20.4s,#shift_stage1_idct //pi2_out[3] = clip_s16((e[0] - o[0] + add)>>shift) )

    trn1        v24.4h, v28.4h, v29.4h
    trn2        v25.4h, v28.4h, v29.4h
    trn1        v26.4h, v30.4h, v31.4h
    trn2        v27.4h, v30.4h, v31.4h
    trn1        v0.2s, v24.2s, v26.2s
    trn2        v2.2s, v24.2s, v26.2s
    trn1        v1.2s, v25.2s, v27.2s
    trn2        v3.2s, v25.2s, v27.2s

    // first stage ends
    // output in d0,d1,d2,d3
    // second stage starts
    smull       v6.4s, v1.4h, v4.h[1]       //83 * pi2_src[1]
    ld1         {v22.s}[1],[x2],x5
    smlal       v6.4s, v3.4h, v4.h[3]       //o[0] = 83 * pi2_src[1] + 36 * pi2_src[3]
    smull       v5.4s, v1.4h, v4.h[3]       //36 * pi2_src[1]
    smlsl       v5.4s, v3.4h, v4.h[1]       //o[1] = 36 * pi2_src[1] - 83 * pi2_src[3]
    ld1         {v23.s}[0],[x2],x5

    saddl       v7.4s, v0.4h, v2.4h         //pi2_src[0] + pi2_src[2]
    ssubl       v17.4s, v0.4h, v2.4h        //pi2_src[0] - pi2_src[2]
    shl         v7.4s, v7.4s,#6             //e[0] = 64*(pi2_src[0] + pi2_src[2])
    shl         v17.4s, v17.4s,#6           //e[1] = 64*(pi2_src[0] - pi2_src[2])


    add         v19.4s,  v7.4s ,  v6.4s     //((e[0] + o[0] )
    add         v16.4s,  v17.4s ,  v5.4s    //((e[1] + o[1])
    sub         v18.4s,  v17.4s ,  v5.4s    //((e[1] - o[1])
    sub         v20.4s,  v7.4s ,  v6.4s     //((e[0] - o[0])

    sqrshrn     v28.4h, v19.4s,#shift_stage2_idct //pi2_out[0] = clip_s16((e[0] + o[0] + add)>>shift) )
    sqrshrn     v29.4h, v16.4s,#shift_stage2_idct //pi2_out[1] = clip_s16((e[1] + o[1] + add)>>shift) )
    sqrshrn     v30.4h, v18.4s,#shift_stage2_idct //pi2_out[2] = clip_s16((e[0] - o[0] + add)>>shift) )
    sqrshrn     v31.4h, v20.4s,#shift_stage2_idct //pi2_out[3] = clip_s16((e[0] - o[0] + add)>>shift) )
    ld1         {v23.s}[1],[x2],x5

    trn1        v24.4h, v28.4h, v29.4h
    trn2        v25.4h, v28.4h, v29.4h
    trn1        v26.4h, v30.4h, v31.4h
    trn2        v27.4h, v30.4h, v31.4h
    trn1        v0.2s, v24.2s, v26.2s
    trn2        v2.2s, v24.2s, v26.2s
    trn1        v1.2s, v25.2s, v27.2s
    trn2        v3.2s, v25.2s, v27.2s
    // second stage ends
    // output in d0,d1,d2,d3
    // second stage computation ends

    // loading pred

    mov         v0.d[1],v1.d[0]
    mov         v2.d[1],v3.d[0]

    uaddw       v0.8h,  v0.8h ,  v22.8b     // pi2_out(16bit) + pu1_pred(8bit)
    uaddw       v2.8h,  v2.8h ,  v23.8b     // pi2_out(16bit) + pu1_pred(8bit)
    sqxtun      v0.8b, v0.8h                // clip_u8(pi2_out(16bit) + pu1_pred(8bit))
    sqxtun      v1.8b, v2.8h                // clip_u8(pi2_out(16bit) + pu1_pred(8bit))

    // storing destination
    st1         {v0.s}[0],[x3],x6
    st1         {v0.s}[1],[x3],x6
    st1         {v1.s}[0],[x3],x6
    st1         {v1.s}[1],[x3],x6


    // ldmfd sp!,{x4-x12,x15}                //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret





