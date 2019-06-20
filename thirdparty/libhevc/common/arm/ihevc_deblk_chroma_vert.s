@/*****************************************************************************
@*
@* Copyright (C) 2012 Ittiam Systems Pvt Ltd, Bangalore
@*
@* Licensed under the Apache License, Version 2.0 (the "License");
@* you may not use this file except in compliance with the License.
@* You may obtain a copy of the License at:
@*
@* http://www.apache.org/licenses/LICENSE-2.0
@*
@* Unless required by applicable law or agreed to in writing, software
@* distributed under the License is distributed on an "AS IS" BASIS,
@* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@* See the License for the specific language governing permissions and
@* limitations under the License.
@*
@*****************************************************************************/
@/**
@/*******************************************************************************
@* @file
@*  ihevc_deblk_luma_vert.s
@*
@* @brief
@*  contains function definitions for inter prediction  interpolation.
@* functions are coded using neon  intrinsics and can be compiled using

@* rvct
@*
@* @author
@*  anand s
@*
@* @par list of functions:
@*
@*
@* @remarks
@*  none
@*
@*******************************************************************************/

.equ    qp_offset_u_offset,     40
.equ    qp_offset_v_offset,     44
.equ    tc_offset_div2_offset,  48
.equ    filter_p_offset,        52
.equ    filter_q_offset,        56

.text
.align 4





.extern gai4_ihevc_qp_table
.extern gai4_ihevc_tc_table
.globl ihevc_deblk_chroma_vert_a9q

gai4_ihevc_qp_table_addr:
.long gai4_ihevc_qp_table - ulbl1 - 8

gai4_ihevc_tc_table_addr:
.long gai4_ihevc_tc_table  - ulbl2 - 8

.type ihevc_deblk_chroma_vert_a9q, %function

ihevc_deblk_chroma_vert_a9q:
    push        {r4-r12,lr}
    sub         r8,r0,#4
    add         r2,r2,r3
    vld1.8      {d5},[r8],r1
    add         r2,r2,#1
    vld1.8      {d17},[r8],r1
    ldr         r7,[sp,#qp_offset_u_offset]
    vld1.8      {d16},[r8],r1
    ldr         r4,[sp,#filter_q_offset]
    vld1.8      {d4},[r8]
    ldr         r5,[sp,#tc_offset_div2_offset]
    vtrn.8      d5,d17
    adds        r3,r7,r2,asr #1
    vtrn.8      d16,d4
    ldr         r7,gai4_ihevc_qp_table_addr
ulbl1:
    add         r7,r7,pc
    ldr         r12,[sp,#filter_p_offset]
    ldr         r6,[sp,#qp_offset_v_offset]
    bmi         l1.2944
    cmp         r3,#0x39
    ldrle       r3,[r7,r3,lsl #2]
    subgt       r3,r3,#6
l1.2944:
    vtrn.16     d5,d16
    adds        r2,r6,r2,asr #1
    vtrn.16     d17,d4
    bmi         l1.2964
    cmp         r2,#0x39
    ldrle       r2,[r7,r2,lsl #2]
    subgt       r2,r2,#6
l1.2964:
    vtrn.32     d5,d17
    add         r3,r3,r5,lsl #1
    vtrn.32     d16,d4
    add         r6,r3,#2
    vmovl.u8    q9,d17
    cmp         r6,#0x35
    movgt       r3,#0x35
    bgt         l1.2996
    adds        r6,r3,#2
    addpl       r3,r3,#2
    movmi       r3,#0
l1.2996:
    vsubl.u8    q0,d17,d16
    ldr         r6,gai4_ihevc_tc_table_addr
ulbl2:
    add         r6,r6,pc
    vshl.i16    q0,q0,#2
    add         r2,r2,r5,lsl #1
    add         r5,r2,#2
    vaddw.u8    q0,q0,d5
    cmp         r5,#0x35
    ldr         r3,[r6,r3,lsl #2]
    vsubw.u8    q2,q0,d4
    movgt       r2,#0x35
    bgt         l1.3036
    adds        r5,r2,#2
    addpl       r2,r2,#2
    movmi       r2,#0
l1.3036:


    vrshr.s16   q3,q2,#3
    vdup.16     d2,r3
    ldr         r2,[r6,r2,lsl #2]
    rsb         r3,r3,#0
    cmp         r12,#0
    vdup.16     d3,r2
    rsb         r2,r2,#0
    vdup.16     d30,r3
    vdup.16     d31,r2


    vmin.s16    q2,q3,q1
    vmax.s16    q1,q15,q2

    vmovl.u8    q3,d16

    vadd.i16    q0,q3,q1
    vsub.i16    q1,q9,q1
    vqmovun.s16 d0,q0
    sub         r2,r0,#2
    vqmovun.s16 d1,q1
    vtrn.32     d0,d1
    vtrn.8      d0,d1
    beq         l1.3204

    vst1.16     {d0[0]},[r2],r1
    vst1.16     {d1[0]},[r2],r1
    vst1.16     {d0[1]},[r2],r1
    vst1.16     {d1[1]},[r2]
l1.3204:
    cmp         r4,#0
    beq         l1.3228
    vst1.16     {d0[2]},[r0],r1
    vst1.16     {d1[2]},[r0],r1
    vst1.16     {d0[3]},[r0],r1
    vst1.16     {d1[3]},[r0]
l1.3228:
    pop         {r4-r12,pc}



