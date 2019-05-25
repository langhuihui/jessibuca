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
//*  ihevc_weighted_pred_uni.s
//*
//* @brief
//*  contains function definitions for weighted prediction used in inter
//* prediction
//*
//* @author
//*  parthiban v
//*
//* @par list of functions:
//*  - ihevc_weighted_pred_uni()
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
//*  does uni-weighted prediction on the array pointed by  pi2_src and stores
//* it at the location pointed by pi2_dst assumptions : the function is
//* optimized considering the fact width and  height are multiple of 2.
//*
//* @par description:
//*  dst = ( (src + lvl_shift) * wgt0 + (1 << (shift - 1)) )  >> shift +
//* offset
//*
//* @param[in] pi2_src
//*  pointer to the source
//*
//* @param[out] pu1_dst
//*  pointer to the destination
//*
//* @param[in] src_strd
//*  source stride
//*
//* @param[in] dst_strd
//*  destination stride
//*
//* @param[in] wgt0
//*  weight to be multiplied to the source
//*
//* @param[in] off0
//*  offset to be added after rounding and
//*
//* @param[in] shifting
//*
//*
//* @param[in] shift
//*  (14 bit depth) + log2_weight_denominator
//*
//* @param[in] lvl_shift
//*  added before shift and offset
//*
//* @param[in] ht
//*  height of the source
//*
//* @param[in] wd
//*  width of the source
//*
//* @returns
//*
//* @remarks
//*  none
//*
//*******************************************************************************
//*/

//void ihevc_weighted_pred_uni(word16 *pi2_src,
//                             uword8 *pu1_dst,
//                             word32 src_strd,
//                             word32 dst_strd,
//                             word32 wgt0,
//                             word32 off0,
//                             word32 shift,
//                             word32 lvl_shift,
//                             word32 ht,
//                             word32 wd)

//**************variables vs registers*****************************************
//    x0 => *pi2_src
//    x1 => *pu1_dst
//    x2 =>  src_strd
//    x3 =>  dst_strd
//    x4 =>  wgt0
//    x5 =>  off0
//    x6 =>  shift
//    x7 =>  lvl_shift
//    x8 =>    ht
//    x9    =>    wd

.text
.align 4

.include "ihevc_neon_macros.s"

.globl ihevc_weighted_pred_uni_av8

.type ihevc_weighted_pred_uni_av8, %function

ihevc_weighted_pred_uni_av8:

    // stmfd sp!, {x4-x12, x14}            //stack stores the values of the arguments

    ldr         w8,[sp,#0]
    ldr         w9,[sp,#8]

    // stmfd sp!, {x4-x12, x14}                //stack stores the values of the arguments

    stp         x19, x20,[sp,#-16]!
    stp         x21, x22,[sp,#-16]!

    mov         x15,x4 // src_strd2 40
    mov         x16,x5 // dst_strd 44
    mov         x17,x6 // lvl_shift1 48
    mov         x19,x7 // lvl_shift2 52
    mov         x20,x8 // ht 56
    mov         x21,x9 // wd 60

    mov         x4,x15                      //load wgt0
    mov         x7,x19                      //load lvl_shift
    mov         x11,#1
    mov         x5,x16                      //load off0
    mul         x10, x7, x4                 //lvl_shift * wgt0
    mov         x6,x17                      //load shift
    mov         x8,x20                      //load ht
    lsl         x22,x5,x6
    add         x10,x10,x22                 //lvl_shift * wgt0 + (off0 << shift)
    mov         x9,x21                      //load wt
    sub         x12,x6,#1
    mov         v0.h[0], w4                 //moved for scalar multiplication
    lsl         x2,x2,#1
    dup         v28.4s,w6                   //vmovq_n_s32(tmp_shift)
    lsl         x22,x11,x12
    add         x10,x10,x22                 //tmp_lvl_shift += (1 << (shift - 1))
    dup         v30.4s,w10                  //vmovq_n_s32(tmp_lvl_shift)
    neg         v28.4s, v28.4s
    lsl         x4,x9,#1

    cmp         x8,#0                       //check ht == 0
    beq         end_loops                   //if equal, then end the function

outer_loop:
    cmp         x9,#0                       //check wd == 0
    beq         end_loops                   //if equal, then end the function

core_loop:
    add         x5,x0,x2                    //pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         x6,x1,x3                    //pu1_dst_tmp = pu1_dst + dst_strd
    ld1         {v1.4h},[x0],#8             //load and increment the pi2_src
    ld1         {v2.4h},[x5],x2             //load and increment the pi2_src_tmp ii iteration
    smull       v4.4s, v1.4h, v0.h[0]       //vmull_n_s16(pi2_src_val1, (int16_t) wgt0)

    add         v4.4s,  v4.4s ,  v30.4s     //vaddq_s32(i4_tmp1_t, tmp_lvl_shift_t)
    ld1         {v3.4h},[x5],x2             //load and increment the pi2_src iii iteration

    smull       v6.4s, v2.4h, v0.h[0]       //vmull_n_s16(pi2_src_val2, (int16_t) wgt0) ii iteration
    ld1         {v5.4h},[x5],x2             //load and increment the pi2_src_tmp iv iteration

    sshl        v4.4s,v4.4s,v28.4s
    //vshl.s32    q2,q2,q14                    //vshlq_s32(i4_tmp1_t, tmp_shift_t)
    add         v6.4s,  v6.4s ,  v30.4s     //vaddq_s32(i4_tmp2_t, tmp_lvl_shift_t) ii iteration

    smull       v7.4s, v3.4h, v0.h[0]       //vmull_n_s16(pi2_src_val1, (int16_t) wgt0) iii iteration
    sqxtun      v4.4h, v4.4s                //vqmovun_s32(sto_res_tmp1)

    add         v7.4s,  v7.4s ,  v30.4s     //vaddq_s32(i4_tmp1_t, tmp_lvl_shift_t) iii iteration
    //mov v5, v4                        //vcombine_u16(sto_res_tmp2, sto_res_tmp2)

    sshl        v6.4s,v6.4s,v28.4s
    //vshl.s32    q3,q3,q14                    //vshlq_s32(i4_tmp2_t, tmp_shift_t) ii iteration

    smull       v16.4s, v5.4h, v0.h[0]      //vmull_n_s16(pi2_src_val2, (int16_t) wgt0) iv iteration
    uqxtn       v4.8b,  v4.8h               //vqmovn_u16(sto_res_tmp3)

    sshl        v7.4s,v7.4s,v28.4s
    //vshl.s32    q5,q5,q14                    //vshlq_s32(i4_tmp1_t, tmp_shift_t) iii iteration
    sqxtun      v6.4h, v6.4s                //vqmovun_s32(sto_res_tmp1) ii iteration

    add         v16.4s,  v16.4s ,  v30.4s   //vaddq_s32(i4_tmp2_t, tmp_lvl_shift_t) iv iteration
    //mov v7, v6                        //vcombine_u16(sto_res_tmp2, sto_res_tmp2) ii iteration

    sqxtun      v7.4h, v7.4s                //vqmovun_s32(sto_res_tmp1) iii iteration

    sshl        v16.4s,v16.4s,v28.4s
    //vshl.s32    q6,q6,q14                    //vshlq_s32(i4_tmp2_t, tmp_shift_t) iv iteration
    st1         {v4.s}[0],[x1],#4           //store pu1_dst i iteration
    //mov v11, v10                        //vcombine_u16(sto_res_tmp2, sto_res_tmp2) iii iteration

    uqxtn       v6.8b,  v6.8h               //vqmovn_u16(sto_res_tmp3) ii iteration
    st1         {v6.s}[0],[x6],x3           //store pu1_dst ii iteration

    uqxtn       v7.8b,  v7.8h               //vqmovn_u16(sto_res_tmp3) iii iteration
    sqxtun      v16.4h, v16.4s              //vqmovun_s32(sto_res_tmp1) iv iteration

    //mov v13, v12                        //vcombine_u16(sto_res_tmp2, sto_res_tmp2) iv iteration
    st1         {v7.s}[0],[x6],x3           //store pu1_dst i iteration iii iteration
    uqxtn       v16.8b,  v16.8h             //vqmovn_u16(sto_res_tmp3) iv iteration

    subs        x9,x9,#4                    //decrement wd by 4 and check for 0
    st1         {v16.s}[0],[x6],x3          //store pu1_dst iv iteration
    bgt         core_loop                   //if greater than 0 repeat the core loop again

end_core_loop:
    sub         x22,x4,x2,lsl #2            //2*src_strd - wd
    neg         x11, x22
    subs        x8,x8,#4                    //decrement the ht by 4
    add         x0,x0,x11                   //pi2_src + 4*src_strd - 2*wd(since pi2_src is 16 bit pointer double the increment with double the wd decrement)
    asr         x9,x4,#1
    sub         x22,x9,x3,lsl #2            //2*dst_strd - wd
    neg         x12, x22
    add         x1,x1,x12                   //pu1_dst + dst_std - wd
    bgt         core_loop                   //if ht is greater than 0 goto outer_loop

end_loops:
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x21, x22,[sp],#16
    ldp         x19, x20,[sp],#16

    ret


