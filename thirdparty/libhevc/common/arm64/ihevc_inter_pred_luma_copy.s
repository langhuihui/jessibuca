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
///**
//*******************************************************************************
//*
//* //brief
//*     interprediction luma function for copy
//*
//* //par description:
//*   copies the array of width 'wd' and height 'ht' from the  location pointed
//*   by 'src' to the location pointed by 'dst'
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
//void ihevc_inter_pred_luma_copy (
//                            uword8 *pu1_src,
//                            uword8 *pu1_dst,
//                            word32 src_strd,
//                            word32 dst_strd,
//                            word8 *pi1_coeff,
//                            word32 ht,
//                            word32 wd   )

//**************variables vs registers*****************************************
//    x0 => *pu1_src
//    x1 => *pu1_dst
//    x2 =>  src_strd
//    x3 =>  dst_strd
//    x11 =>  ht
//    x16 => wd

.text
.align 4

.include "ihevc_neon_macros.s"

.globl ihevc_inter_pred_luma_copy_av8

.type ihevc_inter_pred_luma_copy_av8, %function

ihevc_inter_pred_luma_copy_av8:
    // stmfd sp!, {x8-x16, lr}                //stack stores the values of the arguments
    stp         x19,x20,[sp, #-16]!
    mov         x16,x6                      //loads wd
    mov         x11,x5                      //loads ht
    cmp         x11,#0                      //checks ht == 0
    ble         end_loops
    tst         x16,#15                     //checks wd for multiples for 4 & 8
    beq         core_loop_wd_16
    tst         x16,#7                      //checks wd for multiples for 4 & 8
    beq         core_loop_wd_8
    sub         x15,x16,#4

outer_loop_wd_4:
    subs        x8,x16,#0                   //checks wd == 0
    ble         end_inner_loop_wd_4

inner_loop_wd_4:
    ld1         {v0.s}[0],[x0]              //vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    add         x9,x0,x2                    //pu1_src_tmp += src_strd
    add         x10,x1,x3                   //pu1_dst_tmp += dst_strd
    st1         {v0.s}[0],[x1]              //vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    ld1         {v0.s}[0],[x9],x2           //vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    add         x0,x0,#4                    //pu1_src += 4
    st1         {v0.s}[0],[x10],x3          //vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    ld1         {v0.s}[0],[x9],x2           //vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    subs        x8,x8,#4                    //(wd -4)
    st1         {v0.s}[0],[x10],x3          //vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    ld1         {v0.s}[0],[x9],x2           //vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    add         x1,x1,#4                    //pu1_dst += 4
    st1         {v0.s}[0],[x10],x3          //vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)

    bgt         inner_loop_wd_4

end_inner_loop_wd_4:
    subs        x11,x11,#4                  //ht - 4
    sub         x0,x9,x15                   //pu1_src = pu1_src_tmp
    sub         x1,x10,x15                  //pu1_dst = pu1_dst_tmp
    bgt         outer_loop_wd_4

end_loops:
    // ldmfd sp!,{x8-x16,pc}                  //reload the registers from sp
//  MRS x20,PMCCFILTR_EL0
    sub         x0,x20,x19
    ldp         x19,x20,[sp],#16
    ret


core_loop_wd_8:
    sub         x15,x16,#8

outer_loop_wd_8:
    subs        x8,x16,#0                   //checks wd
    ble         end_inner_loop_wd_8

inner_loop_wd_8:
    add         x9,x0,x2                    //pu1_src_tmp += src_strd
    ld1         {v0.8b},[x0],#8             //vld1_u8(pu1_src_tmp)
    add         x10,x1,x3                   //pu1_dst_tmp += dst_strd
    st1         {v0.8b},[x1],#8             //vst1_u8(pu1_dst_tmp, tmp_src)
    ld1         {v1.8b},[x9],x2             //vld1_u8(pu1_src_tmp)
    st1         {v1.8b},[x10],x3            //vst1_u8(pu1_dst_tmp, tmp_src)
    subs        x8,x8,#8                    //wd - 8(loop condition)
    ld1         {v2.8b},[x9],x2             //vld1_u8(pu1_src_tmp)
    st1         {v2.8b},[x10],x3            //vst1_u8(pu1_dst_tmp, tmp_src)
    ld1         {v3.8b},[x9],x2             //vld1_u8(pu1_src_tmp)
    st1         {v3.8b},[x10],x3            //vst1_u8(pu1_dst_tmp, tmp_src)
    bgt         inner_loop_wd_8

end_inner_loop_wd_8:
    subs        x11,x11,#4                  //ht -= 4
    sub         x0,x9,x15                   //pu1_src = pu1_src_tmp
    sub         x1,x10,x15                  //pu1_dst = pu1_dst_tmp
    bgt         outer_loop_wd_8

    // ldmfd sp!,{x8-x16,pc}                  //reload the registers from sp
//  MRS x20,PMCCFILTR_EL0
    sub         x0,x20,x19
    ldp         x19,x20,[sp],#16
    ret

core_loop_wd_16:
    sub         x15,x16,#16

outer_loop_wd_16:
    subs        x8,x16,#0                   //checks wd
    ble         end_inner_loop_wd_16

inner_loop_wd_16:
    add         x9,x0,x2                    //pu1_src_tmp += src_strd
    ld1         {v0.16b},[x0],#16           //vld1_u8(pu1_src_tmp)
    add         x10,x1,x3                   //pu1_dst_tmp += dst_strd
    st1         {v0.16b},[x1],#16           //vst1_u8(pu1_dst_tmp, tmp_src)
    ld1         {v1.16b},[x9],x2            //vld1_u8(pu1_src_tmp)
    st1         {v1.16b},[x10],x3           //vst1_u8(pu1_dst_tmp, tmp_src)
    subs        x8,x8,#16                   //wd - 8(loop condition)
    ld1         {v2.16b},[x9],x2            //vld1_u8(pu1_src_tmp)
    st1         {v2.16b},[x10],x3           //vst1_u8(pu1_dst_tmp, tmp_src)
    ld1         {v3.16b},[x9],x2            //vld1_u8(pu1_src_tmp)
    st1         {v3.16b},[x10],x3           //vst1_u8(pu1_dst_tmp, tmp_src)
    bgt         inner_loop_wd_16

end_inner_loop_wd_16:
    subs        x11,x11,#4                  //ht -= 4
    sub         x0,x9,x15                   //pu1_src = pu1_src_tmp
    sub         x1,x10,x15                  //pu1_dst = pu1_dst_tmp
    bgt         outer_loop_wd_16

    // ldmfd sp!,{x8-x16,pc}                  //reload the registers from sp
//  MRS x20,PMCCFILTR_EL0
    sub         x0,x20,x19
    ldp         x19,x20,[sp],#16
    ret




