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
//*  ihevc_weighted_pred_bi_default.s
//*
//* @brief
//*  contains function definitions for weighted prediction used in inter
//* prediction
//*
//* @author
//*  parthiban v
//*
//* @par list of functions:
//*  - ihevc_weighted_pred_bi_default()
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
//*  does default bi-weighted prediction on the arrays pointed by pi2_src1 and
//* pi2_src2 and stores it at location  pointed by pi2_dst assumptions : the
//* function is optimized considering the fact width and  height are multiple
//* of 2.
//*
//* @par description:
//*  dst = ( (src1 + lvl_shift1) +  (src2 + lvl_shift2) +  1 << (shift - 1) )
//* >> shift  where shift = 15 - bitdepth
//*
//* @param[in] pi2_src1
//*  pointer to source 1
//*
//* @param[in] pi2_src2
//*  pointer to source 2
//*
//* @param[out] pu1_dst
//*  pointer to destination
//*
//* @param[in] src_strd1
//*  source stride 1
//*
//* @param[in] src_strd2
//*  source stride 2
//*
//* @param[in] dst_strd
//*  destination stride
//*
//* @param[in] lvl_shift1
//*  added before shift and offset
//*
//* @param[in] lvl_shift2
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
//void ihevc_weighted_pred_bi_default(word16 *pi2_src1,
//                                    word16 *pi2_src2,
//                                    uword8 *pu1_dst,
//                                    word32 src_strd1,
//                                    word32 src_strd2,
//                                    word32 dst_strd,
//                                    word32 lvl_shift1,
//                                    word32 lvl_shift2,
//                                    word32 ht,
//                                    word32 wd)

//**************variables vs registers*****************************************
//    x0 => *pi2_src1
//    x1 => *pi2_src2
//    x2 => *pu1_dst
//    x3 =>  src_strd1
//    x4 =>  src_strd2
//    x5 =>  dst_strd
//    x6 =>  lvl_shift1
//    x7 =>  lvl_shift2
//    x8 =>  ht
//    x9 =>  wd
.text
.align 4

.include "ihevc_neon_macros.s"

.globl ihevc_weighted_pred_bi_default_av8

.type ihevc_weighted_pred_bi_default_av8, %function

ihevc_weighted_pred_bi_default_av8:

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

    mov         x4,x15                      //load src_strd2
    lsl         x3,x3,#1
    mov         x5,x16                      //load dst_strd
    mov         x6,x17                      //load lvl_shift1
    lsl         x4,x4,#1
    mov         x7,x19                      //load lvl_shift2
    mov         x8,x20                      //load ht
    mov         x9,x21                      //load wd
    dup         v4.8h,w6                    //lvl_shift1_t = vmov_n_s16((int16_t)lvl_shift1)
    dup         v6.8h,w7                    //lvl_shift2_t = vmov_n_s16((int16_t)lvl_shift2)
    movi        v0.8h, #0x40                //tmp_lvl_shift = 1 << (shift - 1)
    add         v4.8h,  v4.8h,v6.8h
    add         v0.8h,  v0.8h ,  v4.8h
//   vmvn.i32    v2.8h,#0x6                         @vmovq_n_s32(tmp_shift)
    lsl         x6,x9,#1
    sub         x20,x6,x3,lsl #2            //4*src_strd1 - wd
    neg         x7, x20
    sub         x20,x6,x4,lsl #2            //4*src_strd2 - wd
    neg         x10, x20
    //asr            x6,#1
    //rsb            x6,x6,x5,lsl #2             @4*dst_strd - wd

    cmp         x8,#0                       //check ht == 0
    beq         end_loops                   //if equal, then end the function

chroma_decision:
    orr         x14,x8,x9
    cmp         x14,#10
    beq         outer_loop_chroma_8x2

    cmp         x14,#6
    beq         outer_loop_chroma_4x2


luma_decision:
    cmp         x9,#24
    beq         outer_loop_8

    cmp         x9,#16
    bge         outer_loop_16

    cmp         x9,#12
    beq         outer_loop_4

    cmp         x9,#8
    bge         outer_loop_8






outer_loop_4:
    cmp         x9,#0                       //check wd == 0
    beq         end_loops                   //if equal, then end the function

core_loop_4:
    add         x11,x0,x3                   //pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         x12,x1,x4                   //pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    ld1         {v6.4h},[x0],#8             //load and increment the pi2_src1
    add         x14,x2,x5                   //pu1_dst_tmp = pu1_dst + dst_strd
    ld1         {v7.4h},[x1],#8             //load and increment the pi2_src2
    ld1         {v1.4h},[x11],x3            //load and increment the pi2_src1 ii iteration
    sqadd       v18.4h,v6.4h,v7.4h
    sqadd       v18.4h,v18.4h,v0.4h         //vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t)
    ld1         {v3.4h},[x12],x4            //load and increment the pi2_src2 ii iteration
    sqadd       v20.4h,v1.4h,v3.4h          //vaddq_s32(i4_tmp2_t1, i4_tmp2_t2)
    sqadd       v19.4h,v20.4h,v0.4h         //vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t)
    mov         v18.d[1],v19.d[0]
    sqshrun     v20.8b, v18.8h,#7
    ld1         {v22.4h},[x11],x3           //load and increment the pi2_src1 iii iteration
    ld1         {v23.4h},[x12],x4           //load and increment the pi2_src2 iii iteration
    sqadd       v30.4h,v22.4h,v23.4h
    sqadd       v30.4h,v30.4h,v0.4h         //vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t) iii iteration
    ld1         {v24.4h},[x11],x3           //load and increment the pi2_src1 iv iteration
    ld1         {v25.4h},[x12],x4           //load and increment the pi2_src2 iv iteration
    sqadd       v18.4h,v24.4h,v25.4h        //vaddq_s32(i4_tmp2_t1, i4_tmp2_t2) iv iteration
    sqadd       v31.4h,v18.4h,v0.4h
    mov         v30.d[1],v31.d[0]
    st1         {v20.s}[0],[x2],#4          //store pu1_dst i iteration
    st1         {v20.s}[1],[x14],x5         //store pu1_dst ii iteration
    sqshrun     v30.8b, v30.8h,#7
    st1         {v30.s}[0],[x14],x5         //store pu1_dst iii iteration                                                //vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t) iv iteratio
    subs        x9,x9,#4                    //decrement wd by 4 and check for 0
    st1         {v30.s}[1],[x14],x5         //store pu1_dst iv iteration
    bgt         core_loop_4                 //if greater than 0 repeat the core loop again

end_core_loop_4:

    subs        x8,x8,#4                    //decrement the ht by 4

    add         x0,x0,x7                    //pi2_src1 + 4*src_strd1 - 2*wd(since pi2_src1 is 16 bit pointer double the increment with double the wd decrement)
    asr         x9,x6,#1
    add         x1,x1,x10                   //pi2_src2 + 4*src_strd2 - 2*wd
    sub         x20,x9,x5,lsl #2            //4*dst_strd - wd
    neg         x14, x20
    add         x2,x2,x14
                                            //pu1_dst + dst_std - wd
    bgt         core_loop_4                 //if ht is greater than 0 goto outer_loop

    b           end_loops


// this is only for chroma module with input 2x2
outer_loop_chroma_4x2:
    cmp         x9,#0                       //check wd == 0
    beq         end_loops                   //if equal, then end the function
    sub         x20,x6,x3,lsl #1            //2*src_strd1 - wd
    neg         x7, x20
    sub         x20,x6,x4,lsl #1            //2*src_strd2 - wd
    neg         x10, x20
core_loop_chroma_4x2:
    add         x11,x0,x3                   //pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         x12,x1,x4                   //pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    ld1         {v6.4h},[x0],#8             //load and increment the pi2_src1
    add         x14,x2,x5                   //pu1_dst_tmp = pu1_dst + dst_strd
    ld1         {v7.4h},[x1],#8             //load and increment the pi2_src2
    ld1         {v1.4h},[x11],x3            //load and increment the pi2_src1 ii iteration
    sqadd       v18.4h,v6.4h,v7.4h
    sqadd       v18.4h,v18.4h,v0.4h         //vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t)
    ld1         {v3.4h},[x12],x4            //load and increment the pi2_src2 ii iteration
    sqadd       v20.4h,v1.4h,v3.4h          //vaddq_s32(i4_tmp2_t1, i4_tmp2_t2)
    sqadd       v19.4h,v20.4h,v0.4h         //vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t)
    mov         v18.d[1],v19.d[0]
    sqshrun     v20.8b, v18.8h,#7
    st1         {v20.s}[0],[x2],#4          //store pu1_dst i iteration
    st1         {v20.s}[1],[x14],x5         //store pu1_dst ii iteration

    subs        x9,x9,#4                    //decrement wd by 4 and check for 0

    bgt         core_loop_chroma_4x2        //if greater than 0 repeat the core loop again

end_core_loop_chorma_4x2:

    subs        x8,x8,#2                    //decrement the ht by 4

    add         x0,x0,x7                    //pi2_src1 + 2*src_strd1 - 2*wd(since pi2_src1 is 16 bit pointer double the increment with double the wd decrement)
    asr         x9,x6,#1
    add         x1,x1,x10                   //pi2_src2 + 2*src_strd2 - 2*wd
    sub         x20,x9,x5,lsl #1            //2*dst_strd - wd
    neg         x14, x20
    add         x2,x2,x14
                                            //pu1_dst + dst_std - wd
    bgt         core_loop_chroma_4x2        //if ht is greater than 0 goto outer_loop

    b           end_loops



outer_loop_8:
    cmp         x9,#0                       //check wd == 0
    beq         end_loops                   //if equal, then end the function
    add         x11,x0,x3                   //pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         x12,x1,x4                   //pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
core_loop_8:

    ld1         { v24.8h},[x0],#16          //load and increment the pi2_src1
    add         x14,x2,x5                   //pu1_dst_tmp = pu1_dst + dst_strd
    ld1         { v26.8h},[x1],#16          //load and increment the pi2_src2
    sqadd       v24.8h,v24.8h,v26.8h
    ld1         { v28.8h},[x11],x3          //load and increment the pi2_src1 ii iteration
    sqadd       v24.8h,v24.8h,v0.8h         //vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t)
    ld1         { v30.8h},[x12],x4          //load and increment the pi2_src2 ii iteration
    ld1         { v16.8h},[x11],x3          //load and increment the pi2_src1 iii iteration
    sqadd       v22.8h,v28.8h,v30.8h        //vaddq_s32(i4_tmp2_t1, i4_tmp2_t2)
    ld1         { v18.8h},[x12],x4          //load and increment the pi2_src2 iii iteration
    sqadd       v22.8h,v22.8h,v0.8h         //vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t)
    sqshrun     v20.8b, v24.8h,#7
    ld1         { v17.8h},[x11],x3          //load and increment the pi2_src1 iv iteration
    sqadd       v30.8h,v16.8h,v18.8h
    sqshrun     v21.8b, v22.8h,#7
    ld1         { v29.8h},[x12],x4          //load and increment the pi2_src2 iv iteration
    sqadd       v30.8h,v30.8h,v0.8h         //vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t) iii iteration
    st1         {v20.2s},[x2],#8            //store pu1_dst i iteration
    sqadd       v1.8h,v17.8h,v29.8h         //vaddq_s32(i4_tmp2_t1, i4_tmp2_t2) iv iteration
    st1         {v21.2s},[x14],x5           //store pu1_dst ii iteration
    sqadd       v1.8h,v1.8h,v0.8h
    sqshrun     v30.8b, v30.8h,#7
    sqshrun     v31.8b, v1.8h,#7
    add         x11,x0,x3                   //pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         x12,x1,x4                   //pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    st1         {v30.2s},[x14],x5           //store pu1_dst iii iteration                                                //vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t) iv iteratio
    subs        x9,x9,#8                    //decrement wd by 4 and check for 0
    st1         {v31.2s},[x14],x5           //store pu1_dst iv iteration
    bgt         core_loop_8                 //if greater than 0 repeat the core loop again

end_core_loop_8:

    subs        x8,x8,#4                    //decrement the ht by 4

    add         x0,x0,x7                    //pi2_src1 + 4*src_strd1 - 2*wd(since pi2_src1 is 16 bit pointer double the increment with double the wd decrement)
    asr         x9,x6,#1
    add         x1,x1,x10                   //pi2_src2 + 4*src_strd2 - 2*wd
    sub         x20,x9,x5,lsl #2            //4*dst_strd - wd
    neg         x14, x20
    add         x2,x2,x14
    add         x11,x0,x3                   //pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         x12,x1,x4                   //pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)                                  //pu1_dst + dst_std - wd

    bgt         core_loop_8
    b           end_loops



// this is only for chroma module with inpput 4x2
outer_loop_chroma_8x2:
    cmp         x9,#0                       //check wd == 0
    beq         end_loops                   //if equal, then end the function
    add         x11,x0,x3                   //pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         x12,x1,x4                   //pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    sub         x20,x6,x3,lsl #1            //2*src_strd1 - wd
    neg         x7, x20
    sub         x20,x6,x4,lsl #1            //2*src_strd2 - wd
    neg         x10, x20
core_loop_chroma_8x2:

    ld1         { v24.8h},[x0],#16          //load and increment the pi2_src1
    add         x14,x2,x5                   //pu1_dst_tmp = pu1_dst + dst_strd
    ld1         { v26.8h},[x1],#16          //load and increment the pi2_src2
    sqadd       v24.8h,v24.8h,v26.8h
    ld1         { v28.8h},[x11],x3          //load and increment the pi2_src1 ii iteration
    sqadd       v24.8h,v24.8h,v0.8h         //vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t)
    ld1         { v30.8h},[x12],x4          //load and increment the pi2_src2 ii iteration
    ld1         { v16.8h},[x11],x3          //load and increment the pi2_src1 iii iteration
    sqadd       v22.8h,v28.8h,v30.8h        //vaddq_s32(i4_tmp2_t1, i4_tmp2_t2)
    sqadd       v22.8h,v22.8h,v0.8h         //vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t)
    sqshrun     v20.8b, v24.8h,#7
    sqshrun     v21.8b, v22.8h,#7
    st1         {v20.2s},[x2],#8            //store pu1_dst i iteration
    st1         {v21.2s},[x14],x5           //store pu1_dst ii iteration

    add         x11,x0,x3                   //pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         x12,x1,x4                   //pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
                                            //vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t) iv iteratio
    subs        x9,x9,#8                    //decrement wd by 4 and check for 0

    bgt         core_loop_chroma_8x2        //if greater than 0 repeat the core loop again

end_core_loop_chroma_8x2:

    subs        x8,x8,#2                    //decrement the ht by 4

    add         x0,x0,x7                    //pi2_src1 + 4*src_strd1 - 2*wd(since pi2_src1 is 16 bit pointer double the increment with double the wd decrement)
    asr         x9,x6,#1
    add         x1,x1,x10                   //pi2_src2 + 4*src_strd2 - 2*wd
    sub         x20,x9,x5,lsl #1            //4*dst_strd - wd
    neg         x14, x20
    add         x2,x2,x14
    add         x11,x0,x3                   //pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         x12,x1,x4                   //pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)                                  //pu1_dst + dst_std - wd

    bgt         core_loop_chroma_8x2

    b           end_loops




outer_loop_16:
    cmp         x9,#0                       //check wd == 0
    beq         end_loops                   //if equal, then end the function
    add         x11,x0,x3                   //pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         x12,x1,x4                   //pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    sub         x20,x6,x3,lsl #1            //2*src_strd1 - wd
    neg         x7, x20
    mov         x14,#16
    sub         x10,x14,x5
    sub         x11,x3,x14
    sub         x12,x14,x3

    sub         x20,x9,x5,lsl #1            //2*dst_strd - wd
    neg         x14, x20



prolog_16:


    ld1         { v2.8h},[x0],#16           //load and increment the pi2_src1
    ld1         { v4.8h},[x1],#16           //load and increment the pi2_src2
    ld1         { v5.8h},[x0],x11           //load and increment the pi2_src1
    ld1         { v17.8h},[x1],x11          //load and increment the pi2_src2
    ld1         { v6.8h},[x0],#16           //load and increment the pi2_src1 ii iteration
    subs        x9,x9,#16
    ld1         { v1.8h},[x1],#16           //load and increment the pi2_src2 ii iteration
    sub         x20,x8,#2
    csel        x8, x20, x8,eq
    sqadd       v22.8h,v2.8h,v4.8h
    ld1         { v29.8h},[x0],x12          //load and increment the pi2_src1 ii iteration
    sqadd       v28.8h,v5.8h,v17.8h
    ld1         { v16.8h},[x1],x12          //load and increment the pi2_src2 ii iteration
    add         x20,x0,x7
    csel        x0, x20, x0,eq
    add         x20,x1,x7
    csel        x1, x20, x1,eq
    sqadd       v24.8h,v6.8h,v1.8h
    ld1         { v2.8h},[x0],#16
    sqadd       v26.8h,v29.8h,v16.8h
// if the input is chroma with 8x2 block size
    cmp         x8,#0
    beq         epilog_16

    ld1         { v4.8h},[x1],#16           //load and increment the pi2_src2
    sqadd       v22.8h,v22.8h,v0.8h
    ld1         { v5.8h},[x0],x11           //load and increment the pi2_src1
    sqadd       v28.8h,v28.8h,v0.8h
    ld1         { v17.8h},[x1],x11          //load and increment the pi2_src2
    sqadd       v24.8h,v24.8h,v0.8h
    ld1         { v6.8h},[x0],#16           //load and increment the pi2_src1 ii iteration
    sqadd       v30.8h,v26.8h,v0.8h
    sqshrun     v20.8b, v22.8h,#7
    ld1         { v1.8h},[x1],#16           //load and increment the pi2_src2 ii iteration
    sqshrun     v21.8b, v28.8h,#7
    ld1         { v29.8h},[x0],x12          //load and increment the pi2_src1 ii iteration
    sqshrun     v26.8b, v24.8h,#7
    ld1         { v16.8h},[x1],x12          //load and increment the pi2_src2 ii iteration
    sqshrun     v27.8b, v30.8h,#7



core_loop_16:

    cmp         x9,#0
    sqadd       v22.8h,v2.8h,v4.8h
    asr         x20,x6,#1
    csel        x9,x20,x9,eq
    //asreq           x9,x6,#1
    mov         v20.d[1],v21.d[0]
    mov         v26.d[1],v27.d[0]
    st1         { v20.4s},[x2],x5
    sqadd       v28.8h,v5.8h,v17.8h
    st1         { v26.4s},[x2],x10
    add         x20,x2,x14
    csel        x2, x20, x2,eq
    sqadd       v24.8h,v6.8h,v1.8h
    subs        x9,x9,#16
    add         x20,x0,x7
    csel        x0, x20, x0,eq
    sqadd       v26.8h,v29.8h,v16.8h

    add         x20,x1,x7
    csel        x1, x20, x1,eq
    sub         x20,x8,#2
    csel        x8,x20,x8,eq
    cmp         x8,#0
    //subeqs           x8,x8,#2                      //decrement the ht by 2
    beq         epilog_16


    sqadd       v22.8h,v22.8h,v0.8h
    ld1         { v2.8h},[x0],#16           //load and increment the pi2_src1
    sqadd       v28.8h,v28.8h,v0.8h
    ld1         { v4.8h},[x1],#16           //load and increment the pi2_src2
    sqadd       v24.8h,v24.8h,v0.8h
    ld1         { v5.8h},[x0],x11           //load and increment the pi2_src1
    sqadd       v30.8h,v26.8h,v0.8h
    ld1         { v17.8h},[x1],x11          //load and increment the pi2_src2
    sqshrun     v20.8b, v22.8h,#7
    ld1         { v6.8h},[x0],#16           //load and increment the pi2_src1 ii iteration
    sqshrun     v21.8b, v28.8h,#7
    ld1         { v1.8h},[x1],#16           //load and increment the pi2_src2 ii iteration
    sqshrun     v26.8b, v24.8h,#7
    ld1         { v29.8h},[x0],x12          //load and increment the pi2_src1 ii iteration
    sqshrun     v27.8b, v30.8h,#7
    ld1         { v16.8h},[x1],x12          //load and increment the pi2_src2 ii iteration


    b           core_loop_16


epilog_16:

    sqadd       v22.8h,v22.8h,v0.8h
    sqadd       v28.8h,v28.8h,v0.8h
    sqadd       v24.8h,v24.8h,v0.8h
    sqadd       v30.8h,v26.8h,v0.8h
    sqshrun     v20.8b, v22.8h,#7
    sqshrun     v21.8b, v28.8h,#7
    sqshrun     v26.8b, v24.8h,#7
    sqshrun     v27.8b, v30.8h,#7
    mov         v20.d[1],v21.d[0]
    mov         v26.d[1],v27.d[0]
    st1         { v20.4s},[x2],x5
    st1         { v26.4s},[x2]



end_core_loop_16:








end_loops:
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x21, x22,[sp],#16
    ldp         x19, x20,[sp],#16

    ret




