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
//*  ihevc_inter_pred_chroma_copy_w16out_neon.s
//*
//* //brief
//*  contains function definitions for inter prediction  interpolation.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* //author
//*  yogeswaran rs
//*
//* //par list of functions:
//*
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
//*   chroma interprediction filter for copy
//*
//* //par description:
//*    copies the array of width 'wd' and height 'ht' from the  location pointed
//*    by 'src' to the location pointed by 'dst'
//*
//* //param[in] pu1_src
//*  uword8 pointer to the source
//*
//* //param[out] pu1_dst
//*  uword8 pointer to the destination
//*
//* //param[in] src_strd
//*  integer source stride
//*
//* //param[in] dst_strd
//*  integer destination stride
//*
//* //param[in] pi1_coeff
//*  word8 pointer to the filter coefficients
//*
//* //param[in] ht
//*  integer height of the array
//*
//* //param[in] wd
//*  integer width of the array
//*
//* //returns
//*
//* //remarks
//*  none
//*
//*******************************************************************************
//*/

//void ihevc_inter_pred_chroma_copy_w16out(uword8 *pu1_src,
//                                            word16 *pi2_dst,
//                                            word32 src_strd,
//                                            word32 dst_strd,
//                                            word8 *pi1_coeff,
//                                            word32 ht,
//                                            word32 wd)
//**************variables vs registers*****************************************
//x0 => *pu1_src
//x1 => *pi2_dst
//x2 =>  src_strd
//x3 =>  dst_strd
//x4 => *pi1_coeff
//x5 =>  ht
//x6 =>  wd

.text
.align 4

.include "ihevc_neon_macros.s"

.globl ihevc_inter_pred_chroma_copy_w16out_av8

.type ihevc_inter_pred_chroma_copy_w16out_av8, %function

ihevc_inter_pred_chroma_copy_w16out_av8:

    // stmfd sp!, {x4-x12, x14}        //stack stores the values of the arguments

    stp         x19, x20,[sp,#-16]!

    mov         x15,x4 // pi1_coeff
    mov         x16,x5 // ht
    mov         x17,x6 // wd


    mov         x12,x17                     //loads wd
    lsl         x12,x12,#1                  //2*wd
    mov         x7,x16                      //loads ht
    cmp         x7,#0                       //ht condition(ht == 0)
    ble         end_loops                   //loop
    and         x8,x7,#3                    //check ht for mul of 2
    sub         x9,x7,x8                    //check the rounded height value
    and         x11,x7,#6
    cmp         x11,#6
    beq         loop_ht_6
    tst         x12,#7                      //conditional check for wd (multiples)
    beq         core_loop_wd_8

loop_ht_6:
    sub         x11,x12,#4
    lsl         x6, x3,#1
    adds        x6, x6,#0
    cmp         x9,#0
    beq         outer_loop_wd_4_ht_2

outer_loop_wd_4:
    subs        x4,x12,#0                   //wd conditional subtract
    ble         end_inner_loop_wd_4

inner_loop_wd_4:
    ld1         {v0.8b},[x0]                //vld1_u8(pu1_src_tmp)
    add         x5,x0,x2                    //pu1_src +src_strd
    uxtl        v0.8h, v0.8b                //vmovl_u8(vld1_u8(pu1_src_tmp)
    add         x10,x1,x6
    subs        x4,x4,#4                    //wd - 4
    shl         v0.2d, v0.2d,#6             //vshlq_n_s64(temp, 6)
    ld1         {v22.8b},[x5],x2            //vld1_u8(pu1_src_tmp)
    add         x0,x0,#4                    //pu1_src += 4
    st1         {v0.1d},[x1]                //vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    add         x1,x1,#8
    uxtl        v22.8h, v22.8b              //vmovl_u8(vld1_u8(pu1_src_tmp)
    ld1         {v24.8b},[x5],x2            //vld1_u8(pu1_src_tmp)
    shl         v22.2d, v22.2d,#6           //vshlq_n_s64(temp, 6)
    uxtl        v24.8h, v24.8b              //vmovl_u8(vld1_u8(pu1_src_tmp)
    st1         {v22.1d},[x10],x6           //vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    shl         v24.2d, v24.2d,#6           //vshlq_n_s64(temp, 6)
    ld1         {v26.8b},[x5],x2            //vld1_u8(pu1_src_tmp)
    st1         {v24.1d},[x10],x6           //vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    uxtl        v26.8h, v26.8b              //vmovl_u8(vld1_u8(pu1_src_tmp)
    shl         v26.2d, v26.2d,#6           //vshlq_n_s64(temp, 6)
    st1         {v26.1d},[x10],x6           //vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    bgt         inner_loop_wd_4

end_inner_loop_wd_4:
    subs        x9,x9,#4                    //ht - 4
    sub         x0,x5,x11
    sub         x1,x10,x11,lsl #1
    bgt         outer_loop_wd_4
    cmp         x8,#0
    bgt         outer_loop_wd_4_ht_2


end_loops:
    // ldmfd sp!,{x4-x12,x15}        //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret


outer_loop_wd_4_ht_2:
    subs        x4,x12,#0                   //wd conditional subtract
    ble         end_inner_loop_wd_4

inner_loop_wd_4_ht_2:
    ld1         {v0.8b},[x0]                //vld1_u8(pu1_src_tmp)
    add         x5,x0,x2                    //pu1_src +src_strd
    uxtl        v0.8h, v0.8b                //vmovl_u8(vld1_u8(pu1_src_tmp)
    add         x10,x1,x6
    subs        x4,x4,#4                    //wd - 4
    shl         v0.2d, v0.2d,#6             //vshlq_n_s64(temp, 6)
    ld1         {v22.8b},[x5],x2            //vld1_u8(pu1_src_tmp)
    add         x0,x0,#4                    //pu1_src += 4
    st1         {v0.1d},[x1]                //vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    add         x1,x1,#8
    uxtl        v22.8h, v22.8b              //vmovl_u8(vld1_u8(pu1_src_tmp)
    ld1         {v24.8b},[x5],x2            //vld1_u8(pu1_src_tmp)
    shl         v22.2d, v22.2d,#6           //vshlq_n_s64(temp, 6)
    uxtl        v24.8h, v24.8b              //vmovl_u8(vld1_u8(pu1_src_tmp)
    st1         {v22.1d},[x10],x6           //vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    bgt         inner_loop_wd_4_ht_2
    b           end_loops


core_loop_wd_8:
    //sub            x11,x12,#8
    lsl         x5, x3,#1
    adds        x5, x5,#0
    sub         x20,x12,x3, lsl #2          // x11 = (dst_strd * 4) - width
    neg         x11, x20
    sub         x20,x12,x2,lsl #2           //x2->src_strd
    neg         x8, x20
    lsr         x4, x12, #3                 // divide by 8
    mov         x7,x9
    mul         x7, x7, x4
    sub         x4,x12,#0                   //wd conditional check
    sub         x7,x7,#4                    //subtract one for epilog
    cmp         x9,#0
    beq         core_loop_wd_8_ht_2

prolog:
    add         x6,x0,x2                    //pu1_src_tmp += src_strd
    add         x10,x1,x5
    ld1         {v1.8b},[x0],#8             //vld1_u8(pu1_src_tmp)
    ld1         {v3.8b},[x6],x2             //vld1_u8(pu1_src_tmp)
    ld1         {v5.8b},[x6],x2             //vld1_u8(pu1_src_tmp)
    ld1         {v7.8b},[x6],x2             //vld1_u8(pu1_src_tmp)
    uxtl        v16.8h, v1.8b               //vmovl_u8(vld1_u8(pu1_src_tmp))
    uxtl        v18.8h, v3.8b               //vmovl_u8(vld1_u8(pu1_src_tmp)
    uxtl        v20.8h, v5.8b               //vmovl_u8(vld1_u8(pu1_src_tmp)
    uxtl        v22.8h, v7.8b               //vmovl_u8(vld1_u8(pu1_src_tmp)
    subs        x4,x4,#8                    //wd decrements by 8
    shl         v0.8h, v16.8h,#6            //vshlq_n_s16(tmp, 6)
    shl         v2.8h, v18.8h,#6            //vshlq_n_s16(tmp, 6)
    shl         v4.8h, v20.8h,#6            //vshlq_n_s16(tmp, 6)
    shl         v6.8h, v22.8h,#6            //vshlq_n_s16(tmp, 6)
    add         x20,x0,x8
    csel        x0, x20, x0,le
    add         x6,x0,x2                    //pu1_src_tmp += src_strd
    ld1         {v1.8b},[x0],#8             //vld1_u8(pu1_src_tmp)
    ld1         {v3.8b},[x6],x2             //vld1_u8(pu1_src_tmp)
    ld1         {v5.8b},[x6],x2             //vld1_u8(pu1_src_tmp)
    ld1         {v7.8b},[x6],x2             //vld1_u8(pu1_src_tmp)

    st1         {v0.8h},[x1],#16            //vst1q_s16(pi2_dst_tmp, tmp)
    add         x20,x1,x11,lsl #1
    csel        x1, x20, x1,le
    sub         x20,x12,#0                  //wd conditional check
    csel        x4, x20, x4,le

    subs        x7,x7,#4                    //ht - 4

    blt         epilog_end                  //jumps to epilog_end
    beq         epilog                      //jumps to epilog



outer_loop_wd_8:

    st1         {v2.8h},[x10],x5            //vst1q_s16(pi2_dst_tmp, tmp)
    uxtl        v16.8h, v1.8b               //vmovl_u8(vld1_u8(pu1_src_tmp))

    st1         {v4.8h},[x10],x5            //vst1q_s16(pi2_dst_tmp, tmp)
    uxtl        v18.8h, v3.8b               //vmovl_u8(vld1_u8(pu1_src_tmp)

    st1         {v6.8h},[x10],x5            //vst1q_s16(pi2_dst_tmp, tmp)
    uxtl        v20.8h, v5.8b               //vmovl_u8(vld1_u8(pu1_src_tmp)

    uxtl        v22.8h, v7.8b               //vmovl_u8(vld1_u8(pu1_src_tmp)

    subs        x4,x4,#8                    //wd decrements by 8
    add         x20,x0,x8
    csel        x0, x20, x0,le

    add         x6,x0,x2                    //pu1_src_tmp += src_strd

    ld1         {v1.8b},[x0],#8             //vld1_u8(pu1_src_tmp)
    shl         v0.8h, v16.8h,#6            //vshlq_n_s16(tmp, 6)

    ld1         {v3.8b},[x6],x2             //vld1_u8(pu1_src_tmp)
    shl         v2.8h, v18.8h,#6            //vshlq_n_s16(tmp, 6)

    ld1         {v5.8b},[x6],x2             //vld1_u8(pu1_src_tmp)
    shl         v4.8h, v20.8h,#6            //vshlq_n_s16(tmp, 6)

    ld1         {v7.8b},[x6],x2             //vld1_u8(pu1_src_tmp)
    add         x10,x1,x5

    shl         v6.8h, v22.8h,#6            //vshlq_n_s16(tmp, 6)

    st1         {v0.8h},[x1],#16            //vst1q_s16(pi2_dst_tmp, tmp)

    add         x20,x1,x11,lsl #1
    csel        x1, x20, x1,le
    sub         x20,x12,#0                  //wd conditional check
    csel        x4, x20, x4,le

    subs        x7,x7,#4                    //ht - 4
    bgt         outer_loop_wd_8

epilog:
    st1         {v2.8h},[x10],x5            //vst1q_s16(pi2_dst_tmp, tmp)
    uxtl        v16.8h, v1.8b               //vmovl_u8(vld1_u8(pu1_src_tmp))

    st1         {v4.8h},[x10],x5            //vst1q_s16(pi2_dst_tmp, tmp)
    uxtl        v18.8h, v3.8b               //vmovl_u8(vld1_u8(pu1_src_tmp)

    st1         {v6.8h},[x10],x5            //vst1q_s16(pi2_dst_tmp, tmp)
    uxtl        v20.8h, v5.8b               //vmovl_u8(vld1_u8(pu1_src_tmp)

    uxtl        v22.8h, v7.8b               //vmovl_u8(vld1_u8(pu1_src_tmp)
    //add          x6,x0,x2                //pu1_src_tmp += src_strd

    shl         v0.8h, v16.8h,#6            //vshlq_n_s16(tmp, 6)
    shl         v2.8h, v18.8h,#6            //vshlq_n_s16(tmp, 6)
    shl         v4.8h, v20.8h,#6            //vshlq_n_s16(tmp, 6)
    add         x10,x1,x5
    shl         v6.8h, v22.8h,#6            //vshlq_n_s16(tmp, 6)

    st1         {v0.8h},[x1],#16            //vst1q_s16(pi2_dst_tmp, tmp)
epilog_end:
    st1         {v2.8h},[x10],x5            //vst1q_s16(pi2_dst_tmp, tmp)
    st1         {v4.8h},[x10],x5            //vst1q_s16(pi2_dst_tmp, tmp)
    st1         {v6.8h},[x10],x5            //vst1q_s16(pi2_dst_tmp, tmp)
    b           end_loops

core_loop_wd_8_ht_2:
    add         x6,x0,x2                    //pu1_src_tmp += src_strd
    add         x10,x1,x5
    ld1         {v1.8b},[x0],#8             //vld1_u8(pu1_src_tmp)
    ld1         {v3.8b},[x6],x2             //vld1_u8(pu1_src_tmp)
    uxtl        v16.8h, v1.8b               //vmovl_u8(vld1_u8(pu1_src_tmp))
    uxtl        v18.8h, v3.8b               //vmovl_u8(vld1_u8(pu1_src_tmp)
    subs        x12,x12,#8                  //wd decrements by 8
    shl         v0.8h, v16.8h,#6            //vshlq_n_s16(tmp, 6)
    shl         v2.8h, v18.8h,#6            //vshlq_n_s16(tmp, 6)
    st1         {v0.8h},[x1],#16            //vst1q_s16(pi2_dst_tmp, tmp)
    st1         {v2.8h},[x10],x5            //vst1q_s16(pi2_dst_tmp, tmp)
    bgt         core_loop_wd_8_ht_2

    // ldmfd sp!,{x4-x12,x15}         //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret






