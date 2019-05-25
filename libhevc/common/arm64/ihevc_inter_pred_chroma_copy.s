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
//*  ihevc_inter_pred_chroma_copy.s
//*
//* @brief
//*  Contains function definitions for inter prediction  interpolation.
//* Functions are coded using NEON  intrinsics and can be compiled using ARM
//* RVCT
//*
//* @author
//*  Yogeswaran RS
//*
//* @par List of Functions:
//*
//*
//* @remarks
//*  None
//*
//*******************************************************************************
//*/
///**
//*******************************************************************************
//*
//* @brief
//*   Chroma interprediction filter for copy
//*
//* @par Description:
//*    Copies the array of width 'wd' and height 'ht' from the  location pointed
//*    by 'src' to the location pointed by 'dst'
//*
//* @param[in] pu1_src
//*  UWORD8 pointer to the source
//*
//* @param[out] pu1_dst
//*  UWORD8 pointer to the destination
//*
//* @param[in] src_strd
//*  integer source stride
//*
//* @param[in] dst_strd
//*  integer destination stride
//*
//* @param[in] pi1_coeff
//*  WORD8 pointer to the filter coefficients
//*
//* @param[in] ht
//*  integer height of the array
//*
//* @param[in] wd
//*  integer width of the array
//*
//* @returns
//*
//* @remarks
//*  None
//*
//*******************************************************************************
//*/

//void ihevc_inter_pred_chroma_copy( UWORD8 *pu1_src,
//                                   UWORD8 *pu1_dst,
//                                   WORD32 src_strd,
//                                   WORD32 dst_strd,
//                                   WORD8 *pi1_coeff,
//                                   WORD32 ht,
//                                   WORD32 wd)
//**************Variables Vs Registers*****************************************
//x0 => *pu1_src
//x1 => *pu1_dst
//x2 =>  src_strd
//x3 =>  dst_strd
//x4 => *pi1_coeff
//x5 =>  ht
//x6 =>  wd

.text
.align 4

.globl ihevc_inter_pred_chroma_copy_av8

.type ihevc_inter_pred_chroma_copy_av8, %function

ihevc_inter_pred_chroma_copy_av8:

    LSL         x12,x6,#1                   //wd << 1
    CMP         x5,#0                       //checks ht == 0
    BLE         END_LOOPS
    AND         x8,x5,#3                    //check ht for mul of 2
    SUB         x5,x5,x8                    //check the rounded height value
    TST         x12,#15                     //checks wd for multiples for 16
    BEQ         CORE_LOOP_WD_16
    TST         x12,#7                      //checks wd for multiples for 4 & 8
    BEQ         CORE_LOOP_WD_8
    SUB         x11,x12,#4
    CMP         x5,#0
    BEQ         OUTER_LOOP_WD_4_HT_2

OUTER_LOOP_WD_4:
    SUBS        x4,x12,#0                   //checks wd == 0
    BLE         END_INNER_LOOP_WD_4

INNER_LOOP_WD_4:
    LD1         {v0.s}[0],[x0]              //vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    ADD         x7,x0,x2                    //pu1_src_tmp += src_strd
    ADD         x6,x1,x3                    //pu1_dst_tmp += dst_strd
    ST1         {v0.s}[0],[x1]              //vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    LD1         {v0.s}[0],[x7],x2           //vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    ADD         x0,x0,#4                    //pu1_src += 4
    ST1         {v0.s}[0],[x6],x3           //vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    LD1         {v0.s}[0],[x7],x2           //vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    SUBS        x4,x4,#4                    //(wd -4)
    ST1         {v0.s}[0],[x6],x3           //vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    LD1         {v0.s}[0],[x7],x2           //vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    ADD         x1,x1,#4                    //pu1_dst += 4
    ST1         {v0.s}[0],[x6],x3           //vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    BGT         INNER_LOOP_WD_4

END_INNER_LOOP_WD_4:
    SUBS        x5,x5,#4                    //ht - 4
    SUB         x0,x7,x11                   //pu1_src = pu1_src_tmp
    SUB         x1,x6,x11                   //pu1_dst = pu1_dst_tmp
    BGT         OUTER_LOOP_WD_4
    CMP         x8,#0
    BGT         OUTER_LOOP_WD_4_HT_2

END_LOOPS:
    RET

OUTER_LOOP_WD_4_HT_2:
    SUBS        x4,x12,#0                   //checks wd == 0
    BLE         END_LOOPS

INNER_LOOP_WD_4_HT_2:
    LD1         {v0.s}[0],[x0]              //vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    ADD         x7,x0,x2                    //pu1_src_tmp += src_strd
    ADD         x6,x1,x3                    //pu1_dst_tmp += dst_strd
    ST1         {v0.s}[0],[x1]              //vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    LD1         {v0.s}[0],[x7],x2           //vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    ADD         x0,x0,#4                    //pu1_src += 4
    ST1         {v0.s}[0],[x6],x3           //vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    SUBS        x4,x4,#4                    //(wd -4)
    ADD         x1,x1,#4                    //pu1_dst += 4
    BGT         INNER_LOOP_WD_4_HT_2
    B           END_LOOPS

CORE_LOOP_WD_8:
    SUB         x11,x12,#8
    CMP         x5,#0
    BEQ         OUTER_LOOP_WD_8_HT_2

OUTER_LOOP_WD_8:
    SUBS        x4,x12,#0                   //checks wd
    BLE         END_INNER_LOOP_WD_8


INNER_LOOP_WD_8:
    ADD         x7,x0,x2                    //pu1_src_tmp += src_strd
    LD1         {v0.8b},[x0],#8             //vld1_u8(pu1_src_tmp)
    ADD         x6,x1,x3                    //pu1_dst_tmp += dst_strd
    ST1         {v0.8b},[x1],#8             //vst1_u8(pu1_dst_tmp, tmp_src)
    LD1         {v1.8b},[x7],x2             //vld1_u8(pu1_src_tmp)
    ST1         {v1.8b},[x6],x3             //vst1_u8(pu1_dst_tmp, tmp_src)
    SUBS        x4,x4,#8                    //wd - 8(Loop condition)
    LD1         {v2.8b},[x7],x2             //vld1_u8(pu1_src_tmp)
    ST1         {v2.8b},[x6],x3             //vst1_u8(pu1_dst_tmp, tmp_src)
    LD1         {v3.8b},[x7],x2             //vld1_u8(pu1_src_tmp)
    ST1         {v3.8b},[x6],x3             //vst1_u8(pu1_dst_tmp, tmp_src)
    BGT         INNER_LOOP_WD_8

END_INNER_LOOP_WD_8:
    SUBS        x5,x5,#4                    //ht -= 4
    SUB         x0,x7,x11                   //pu1_src = pu1_src_tmp
    SUB         x1,x6,x11                   //pu1_dst = pu1_dst_tmp
    BGT         OUTER_LOOP_WD_8
    CMP         x8,#0
    BGT         OUTER_LOOP_WD_8_HT_2
    B           END_LOOPS

OUTER_LOOP_WD_8_HT_2:
    SUBS        x4,x12,#0                   //checks wd
    BLE         END_LOOPS

INNER_LOOP_WD_8_HT_2:
    ADD         x7,x0,x2                    //pu1_src_tmp += src_strd
    LD1         {v0.8b},[x0],#8             //vld1_u8(pu1_src_tmp)
    ADD         x6,x1,x3                    //pu1_dst_tmp += dst_strd
    ST1         {v0.8b},[x1],#8             //vst1_u8(pu1_dst_tmp, tmp_src)
    LD1         {v1.8b},[x7],x2             //vld1_u8(pu1_src_tmp)
    ST1         {v1.8b},[x6],x3             //vst1_u8(pu1_dst_tmp, tmp_src)
    B           END_LOOPS

CORE_LOOP_WD_16:
    SUB         x11,x12,#16
    CMP         x5,#0
    BEQ         OUTER_LOOP_WD_16_HT_2

OUTER_LOOP_WD_16:
    SUBS        x4,x12,#0                   //checks wd
    BLE         END_INNER_LOOP_WD_16

INNER_LOOP_WD_16:
    ADD         x7,x0,x2                    //pu1_src_tmp += src_strd
    LD1         {v0.16b},[x0],#16           //vld1_u8(pu1_src_tmp)
    ADD         x6,x1,x3                    //pu1_dst_tmp += dst_strd
    ST1         {v0.16b},[x1],#16           //vst1_u8(pu1_dst_tmp, tmp_src)
    LD1         {v1.16b},[x7],x2            //vld1_u8(pu1_src_tmp)
    ST1         {v1.16b},[x6],x3            //vst1_u8(pu1_dst_tmp, tmp_src)
    SUBS        x4,x4,#16                   //wd - 16(Loop condition)
    LD1         {v2.16b},[x7],x2            //vld1_u8(pu1_src_tmp)
    ST1         {v2.16b},[x6],x3            //vst1_u8(pu1_dst_tmp, tmp_src)
    LD1         {v3.16b},[x7],x2            //vld1_u8(pu1_src_tmp)
    ST1         {v3.16b},[x6],x3            //vst1_u8(pu1_dst_tmp, tmp_src)
    BGT         INNER_LOOP_WD_16

END_INNER_LOOP_WD_16:
    SUBS        x5,x5,#4                    //ht -= 4
    SUB         x0,x7,x11                   //pu1_src = pu1_src_tmp
    SUB         x1,x6,x11                   //pu1_dst = pu1_dst_tmp
    BGT         OUTER_LOOP_WD_16
    CMP         x8,#0
    BGT         OUTER_LOOP_WD_16_HT_2
    B           END_LOOPS

OUTER_LOOP_WD_16_HT_2:
    SUBS        x4,x12,#0                   //checks wd
    BLE         END_LOOPS

INNER_LOOP_WD_16_HT_2:
    ADD         x7,x0,x2                    //pu1_src_tmp += src_strd
    LD1         {v0.16b},[x0],#16           //vld1_u8(pu1_src_tmp)
    ADD         x6,x1,x3                    //pu1_dst_tmp += dst_strd
    ST1         {v0.16b},[x1],#16           //vst1_u8(pu1_dst_tmp, tmp_src)
    LD1         {v1.16b},[x7],x2            //vld1_u8(pu1_src_tmp)
    ST1         {v1.16b},[x6],x3            //vst1_u8(pu1_dst_tmp, tmp_src)

    RET


