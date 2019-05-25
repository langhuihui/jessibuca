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
//* //file
//*  ihevc_weighted_pred_bi.s
//*
//* //brief
//*  contains function definitions for weighted prediction used in inter
//* prediction
//*
//* //author
//*  parthiban v
//*
//* //par list of functions:
//*  - ihevc_weighted_pred_bi()
//*
//* //remarks
//*  none
//*
//*******************************************************************************
//*/
///**
//*******************************************************************************
//*
//* //brief
//*  does bi-weighted prediction on the arrays pointed by  pi2_src1 and
//* pi2_src2 and stores it at location pointed  by pi2_dst   assumptions : the
//* function is optimized considering the fact width and  height are multiple
//* of 2.
//*
//* //par description:
//*  dst = ( (src1 + lvl_shift1)*wgt0 +  (src2 + lvl_shift2)*wgt1 +  (off0 +
//* off1 + 1) << (shift - 1) ) >> shift
//*
//* //param[in] pi2_src1
//*  pointer to source 1
//*
//* //param[in] pi2_src2
//*  pointer to source 2
//*
//* //param[out] pu1_dst
//*  pointer to destination
//*
//* //param[in] src_strd1
//*  source stride 1
//*
//* //param[in] src_strd2
//*  source stride 2
//*
//* //param[in] dst_strd
//*  destination stride
//*
//* //param[in] wgt0
//*  weight to be multiplied to source 1
//*
//* //param[in] off0
//*  offset 0
//*
//* //param[in] wgt1
//*  weight to be multiplied to source 2
//*
//* //param[in] off1
//*  offset 1
//*
//* //param[in] shift
//*  (14 bit depth) + log2_weight_denominator
//*
//* //param[in] lvl_shift1
//*  added before shift and offset
//*
//* //param[in] lvl_shift2
//*  added before shift and offset
//*
//* //param[in] ht
//*  height of the source
//*
//* //param[in] wd
//*  width of the source
//*
//* //returns
//*
//* //remarks
//*  none
//*
//*******************************************************************************
//*/

//void ihevc_weighted_pred_bi(word16 *pi2_src1,
//                            word16 *pi2_src2,
//                            uword8 *pu1_dst,
//                            word32 src_strd1,
//                            word32 src_strd2,
//                            word32 dst_strd,
//                            word32 wgt0,
//                            word32 off0,
//                            word32 wgt1,
//                            word32 off1,
//                            word32 shift,
//                            word32 lvl_shift1,
//                            word32 lvl_shift2,
//                            word32 ht,
//                            word32 wd)

//**************variables vs registers*****************************************
//    x0 => *pi2_src1
//    x1 => *pi2_src2
//    x2 => *pu1_dst
//    x3 =>  src_strd1
//    x4 =>  src_strd2
//    x5 =>  dst_strd
//    x6 =>  wgt0
//    x7 =>  off0
//    x8 =>  wgt1
//    x9 =>  off1
//    x10 =>  shift
//    x11 =>  lvl_shift1
//    x12 =>    lvl_shift2
//    x14 =>    ht
//    x7    =>    wd

.text
.align 4

.include "ihevc_neon_macros.s"

.globl ihevc_weighted_pred_bi_av8

.type ihevc_weighted_pred_bi_av8, %function

ihevc_weighted_pred_bi_av8:

    // stmfd sp!, {x4-x12, x14}            //stack stores the values of the arguments

    ldr         w8,[sp,#0]
    ldr         w9,[sp,#8]
    ldr         w10,[sp,#16]
    ldr         w11,[sp,#24]
    ldr         w12,[sp,#32]
    ldr         w13,[sp,#40]
    ldr         w14,[sp,#48]

    sxtw        x8,w8
    sxtw        x9,w9
    sxtw        x10,w10
    sxtw        x11,w11
    sxtw        x12,w12


    stp         x19, x20,[sp,#-16]!
    stp         x21, x22,[sp,#-16]!
    stp         x23, x24,[sp,#-16]!
    stp         x25, x26,[sp,#-16]!

    mov         x15,x4 // src_strd2 40
    mov         x16,x5 // dst_strd 44
    mov         x17,x6 // wgt0 48
    mov         x19,x7 // off0 52
    mov         x20,x8 // wgt1 56
    mov         x21,x9 // off1 60
    mov         x22,x10 // shift 64
    mov         x23,x11 // lvl_shift1 68
    mov         x24,x12 // lvl_shift2 72
    mov         x25,x13 // ht 76
    mov         x26,x14 // wd 80

    mov         x6,x17                      //load wgt0
    mov         x11,x23                     //load lvl_shift1
    mov         x12,x24                     //load lvl_shift2
    mov         v7.h[0],w6                  //moved for scalar multiplication
    mul         x4, x11 , x6                //lvl_shift1 * wgt0
    mov         x8,x20                      //load wgt1
    mov         x7,x19                      //load off0
    mov         v7.h[1],w8                  //moved for scalar multiplication
    madd        x4,x12,x8,x4                //(lvl_shift1 * wgt0) + (lvl_shift2 * wgt1)
    mov         x9,x21                      //load off1
    add         x5,x7,x9                    //off0 + off1
    mov         x10,x22                     //load shift
    add         x5,x5,#1                    //off0 + off1 + 1
    sub         x14,x10,#1                  //shift - 1
    mov         x7,x26                      //load wd
    lsl         x5,x5,x14                   //((off0 + off1 + 1) << (shift - 1))
    dup         v28.4s,w10                  //vmovq_n_s32(0-shift)
    add         x4,x4,x5                    //tmp_lvl_shift += ((off0 + off1 + 1) << (shift - 1))
    dup         v30.4s,w4                   //vmovq_n_s32(tmp_lvl_shift)
    neg         v28.4s, v28.4s
    mov         x4,x15                      //load src_strd2
    lsl         x9,x7,#1
    mov         x5,x16                      //load dst_strd
    lsl         x3,x3,#1
    mov         x14,x25                     //load ht
    lsl         x4,x4,#1

    cmp         x14,#0                      //check ht == 0
    beq         end_loops                   //if equal, then end the function

outer_loop:
    cmp         x7,#0                       //check wd == 0
    beq         end_loops                   //if equal, then end the function

core_loop:
    add         x6,x0,x3                    //pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         x8,x1,x4                    //pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    ld1         {v0.4h},[x0],#8             //load and increment the pi2_src1
    add         x10,x2,x5                   //pu1_dst_tmp = pu1_dst + dst_strd
    ld1         {v1.4h},[x1],#8             //load and increment the pi2_src2
    smull       v4.4s, v0.4h, v7.h[0]       //vmull_n_s16(pi2_src1_val1, (int16_t) wgt0)
    ld1         {v2.4h},[x6],x3             //load and increment the pi2_src_tmp1 ii iteration
    smull       v5.4s, v1.4h, v7.h[1]       //vmull_n_s16(pi2_src2_val1, (int16_t) wgt1)
    ld1         {v3.4h},[x8],x4             //load and increment the pi2_src_tmp1 ii iteration
    add         v4.4s,  v4.4s ,  v5.4s      //vaddq_s32(i4_tmp1_t1, i4_tmp1_t2)

    ld1         {v0.4h},[x6],x3             //load and increment the pi2_src1 iii iteration
    smull       v6.4s, v2.4h, v7.h[0]       //vmull_n_s16(pi2_src1_val2, (int16_t) wgt0) ii iteration

    ld1         {v1.4h},[x8],x4             //load and increment the pi2_src2 iii iteration
    add         v4.4s,  v4.4s ,  v30.4s     //vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t)
    smull       v19.4s, v0.4h, v7.h[0]      //vmull_n_s16(pi2_src1_val1, (int16_t) wgt0) iii iteration

    ld1         {v2.4h},[x6],x3             //load and increment the pi2_src_tmp1 iv iteration
    smull       v17.4s, v3.4h, v7.h[1]      //vmull_n_s16(pi2_src2_val2, (int16_t) wgt1) ii iteration
    sshl        v4.4s,v4.4s,v28.4s          //vshlq_s32(i4_tmp1_t1, tmp_shift_t)

    ld1         {v3.4h},[x8],x4             //load and increment the pi2_src_tmp1 iv iteration
    add         v6.4s,  v6.4s ,  v17.4s     //vaddq_s32(i4_tmp2_t1, i4_tmp2_t2) ii iteration

    sqxtun      v4.4h, v4.4s                //vqmovun_s32(sto_res_tmp1)
    smull       v16.4s, v1.4h, v7.h[1]      //vmull_n_s16(pi2_src2_val1, (int16_t) wgt1) iii iteration

    add         v6.4s,  v6.4s ,  v30.4s     //vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t) ii iteration
    //mov v5, v4                        //vcombine_u16(sto_res_tmp2, sto_res_tmp2)
    add         v19.4s,  v19.4s ,  v16.4s   //vaddq_s32(i4_tmp1_t1, i4_tmp1_t2) iii iteration

    sshl        v6.4s,v6.4s,v28.4s
    //vshl.s32    q5,q5,q14                    //vshlq_s32(i4_tmp2_t1, tmp_shift_t) ii iteration
    smull       v18.4s, v2.4h, v7.h[0]      //vmull_n_s16(pi2_src1_val2, (int16_t) wgt0) iv iteration
    uqxtn       v4.8b,v4.8h
    //vqmovn.u16    d4,q2                        //vqmovn_u16(sto_res_tmp3)
    add         v19.4s,  v19.4s ,  v30.4s   //vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t) iii iteration

    sqxtun      v6.4h, v6.4s                //vqmovun_s32(sto_res_tmp1) ii iteration
    smull       v20.4s, v3.4h, v7.h[1]      //vmull_n_s16(pi2_src2_val2, (int16_t) wgt1) iv iteration

    sshl        v19.4s,v19.4s,v28.4s
    //vshl.s32    q7,q7,q14                    //vshlq_s32(i4_tmp1_t1, tmp_shift_t) iii iteration
    //mov v11, v10                        //vcombine_u16(sto_res_tmp2, sto_res_tmp2) ii iteration

    add         v18.4s,  v18.4s ,  v20.4s   //vaddq_s32(i4_tmp2_t1, i4_tmp2_t2) iv iteration
    sqxtun      v19.4h, v19.4s              //vqmovun_s32(sto_res_tmp1) iii iteration

    add         v18.4s,  v18.4s ,  v30.4s   //vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t) iv iteration
    st1         {v4.s}[0],[x2],#4           //store pu1_dst i iteration

    uqxtn       v6.8b,v6.8h
    //vqmovn.u16    d10,q5                        //vqmovn_u16(sto_res_tmp3) ii iteration
    sshl        v18.4s,v18.4s,v28.4s
    //vshl.s32    q9,q9,q14                    //vshlq_s32(i4_tmp2_t1, tmp_shift_t) iv iteration
    st1         {v6.s}[0],[x10],x5          //store pu1_dst ii iteration


    //mov v15, v14                        //vcombine_u16(sto_res_tmp2, sto_res_tmp2) iii iteration
    uqxtn       v19.8b,v19.8h
    //vqmovn.u16    d14,q7                        //vqmovn_u16(sto_res_tmp3) iii iteration
    sqxtun      v18.4h, v18.4s              //vqmovun_s32(sto_res_tmp1) iv iteration
    //mov v19, v18                        //vcombine_u16(sto_res_tmp2, sto_res_tmp2)
    st1         {v19.s}[0],[x10],x5         //store pu1_dst iii iteration
    uqxtn       v18.8b,v18.8h
    //vqmovn.u16    d18,q9                        //vqmovn_u16(sto_res_tmp3) iv iteration
    subs        x7,x7,#4                    //decrement wd by 4 and check for 0
    st1         {v18.s}[0],[x10],x5         //store pu1_dst iv iteration

    bgt         core_loop                   //if greater than 0 repeat the core loop again

end_core_loop:
    sub         x20,x9,x3,lsl #2            //2*src_strd1 - wd
    neg         x11, x20
    subs        x14,x14,#4                  //decrement the ht by 4
    sub         x20,x9,x4,lsl #2            //2*src_strd2 - wd
    neg         x12, x20
    add         x0,x0,x11                   //pi2_src1 + 4*src_strd1 - 2*wd(since pi2_src1 is 16 bit pointer double the increment with double the wd decrement)
    asr         x7,x9,#1
    add         x1,x1,x12                   //pi2_src2 + 4*src_strd2 - 2*wd
    sub         x20,x7,x5,lsl #2            //2*dst_strd - wd
    neg         x10, x20
    add         x2,x2,x10                   //pu1_dst + dst_std - wd
    bgt         core_loop                   //if ht is greater than 0 goto outer_loop

end_loops:
    // ldmfd sp!,{x4-x12,x15}              //reload the registers from sp
    ldp         x25, x26,[sp],#16
    ldp         x23, x24,[sp],#16
    ldp         x21, x22,[sp],#16
    ldp         x19, x20,[sp],#16

    ret






