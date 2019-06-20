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
@/*******************************************************************************
@* @file
@*  ihevc_deblk_luma_horz.s
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
.globl ihevc_deblk_chroma_horz_a9q

gai4_ihevc_qp_table_addr:
.long gai4_ihevc_qp_table - ulbl1 - 8

gai4_ihevc_tc_table_addr:
.long gai4_ihevc_tc_table - ulbl2 - 8

.type ihevc_deblk_chroma_horz_a9q, %function

ihevc_deblk_chroma_horz_a9q:
    push        {r4-r12,lr}
    sub         r12,r0,r1
    vld1.8      {d0},[r0]
    sub         r5,r12,r1
    add         r6,r0,r1
    add         r1,r2,r3
    vmovl.u8    q0,d0
    ldr         r10,[sp,#qp_offset_u_offset]
    vld1.8      {d2},[r12]
    add         r2,r1,#1
    ldr         r4,[sp,#tc_offset_div2_offset]
    vld1.8      {d4},[r5]
    ldr         r8,[sp,#filter_p_offset]
    vld1.8      {d16},[r6]
    ldr         r9,[sp,#filter_q_offset]
    adds        r1,r10,r2,asr #1
    vmovl.u8    q1,d2
    ldr         r7,[sp,#qp_offset_v_offset]
    ldr         r3,gai4_ihevc_qp_table_addr
ulbl1:
    add         r3, r3, pc
    bmi         l1.3312
    cmp         r1,#0x39
    ldrle       r1,[r3,r1,lsl #2]
    subgt       r1,r1,#6
l1.3312:
    adds        r2,r7,r2,asr #1
    vmovl.u8    q2,d4
    bmi         l1.3332
    cmp         r2,#0x39
    ldrle       r2,[r3,r2,lsl #2]
    subgt       r2,r2,#6
l1.3332:
    add         r1,r1,r4,lsl #1
    vsub.i16    q3,q0,q1
    add         r3,r1,#2
    cmp         r3,#0x35
    movgt       r1,#0x35
    vshl.i16    q3,q3,#2
    vmovl.u8    q8,d16
    bgt         l1.3368
    adds        r3,r1,#2
    addpl       r1,r1,#2
    movmi       r1,#0
l1.3368:
    ldr         r3,gai4_ihevc_tc_table_addr
ulbl2:
    add         r3, r3, pc
    vadd.i16    q2,q3,q2
    add         r2,r2,r4,lsl #1
    vsub.i16    q3,q2,q8
    add         r4,r2,#2
    ldr         r1,[r3,r1,lsl #2]
    cmp         r4,#0x35
    movgt       r2,#0x35
    bgt         l1.3412
    adds        r4,r2,#2
    addpl       r2,r2,#2
    movmi       r2,#0
l1.3412:


    ldr         r2,[r3,r2,lsl #2]
    cmp         r8,#0
    vdup.16     q8,r2
    vdup.16     q2,r1
    rsb         r1,r1,#0
    vrshr.s16   q3,q3,#3
    vdup.16     q9,r1
    rsb         r1,r2,#0
    vzip.16     q2,q8
    vdup.16     q10,r1

    vzip.16     q9,q10

    vmin.s16    q8,q3,q2
    vmax.s16    q2,q9,q8
    vadd.i16    q1,q1,q2
    vsub.i16    q0,q0,q2
    vqmovun.s16 d2,q1
    vqmovun.s16 d0,q0
    beq         l1.3528
    vst1.8      {d2},[r12]
l1.3528:
    cmp         r9,#0
    beq         l1.3540
    vst1.8      {d0},[r0]
l1.3540:
    pop         {r4-r12,pc}


