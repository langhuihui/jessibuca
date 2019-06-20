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

.equ    qp_q_offset,                44
.equ    beta_offset_div2_offset,    48
.equ    tc_offset_div2_offset,      52
.equ    filter_p_offset,            56
.equ    filter_q_offset,            60

.text
.align 4





.extern gai4_ihevc_tc_table
.extern gai4_ihevc_beta_table

.globl ihevc_deblk_luma_vert_a9q

gai4_ihevc_tc_table_addr:
.long gai4_ihevc_tc_table   - ulbl1 - 8

gai4_ihevc_beta_table_addr:
.long gai4_ihevc_beta_table   - ulbl2 - 8

.type ihevc_deblk_luma_vert_a9q, %function

ihevc_deblk_luma_vert_a9q:

    push        {r3-r12,lr}
    ldr         r4,[sp,#qp_q_offset]
    ldr         r5,[sp,#beta_offset_div2_offset]

    add         r3,r3,r4
    add         r3,r3,#1
    ldr         r6, [sp,#tc_offset_div2_offset]
    asr         r3,r3,#1
    add         r7,r3,r5,lsl #1
    add         r3,r3,r6,lsl #1
    cmp         r7,#0x33
    movgt       r7,#0x33
    bgt         l1.56
    cmp         r7,#0x0
    movlt       r7,#0x0                     @ r7 has the beta_index value
l1.56:

@     bic      r2,r2,#1
    asr         r2,r2,#1

    add         r3,r3,r2,lsl #1
    cmp         r3,#0x35
    movgt       r3,#0x35
    bgt         l1.88
    cmp         r3,#0x0
    movlt       r3,#0x0                     @ r3 has the tc_index value

@    qp_luma = (quant_param_p + quant_param_q + 1) >> 1@
@    beta_indx = clip3(qp_luma + (beta_offset_div2 << 1), 0, 51)@
@    tc_indx = clip3(qp_luma + (2 * (bs >> 1)) + (tc_offset_div2 << 1), 0, 53)@

l1.88:
    ldr         r2,gai4_ihevc_beta_table_addr
ulbl2:
    add         r2,r2,pc
    vmov.i8     d18,#0x2
    ldr         r4,gai4_ihevc_tc_table_addr
ulbl1:
    add         r4,r4,pc

    ldr         r5,[r2,r7,lsl #2]           @ beta
    vmov.i16    q8,#0x2
    ldr         r6,[r4,r3,lsl #2]           @ tc
    lsl         r8,r6,#1
    cmp         r6,#0
    vdup.8      d19,r8
    sub         r7,r0,#4
    vmov.i8     d23,#0x3
    beq         l1.964


    vld1.8      {d24},[r7],r1
    ldrb        r8,[r0,#-3]                 @ -3 value
    vld1.8      {d1},[r7],r1
    ldrb        r10,[r0,#-2]                @-2 value
    vld1.8      {d2},[r7],r1
    ldrb        r11,[r0,#-1]                @-1 value
    vld1.8      {d0},[r7]
    ldrb        r12,[r0,#0]                 @ 0 value
    ldrb        r9,[r0,#1]                  @ 1 value
    vtrn.8      d24,d1
    ldrb        r2,[r0,#2]                  @ 2 value
    vtrn.8      d2,d0
    add         r12,r12,r2
    subs        r9,r12,r9,lsl #1            @ dq0 value is stored in r9
    rsbmi       r9,r9,#0
@dq0 = abs( pu1_src[2] - 2 * pu1_src[1] + pu1_src[0] )@
    vtrn.16     d24,d2
    add         r8,r8,r11
    vtrn.16     d1,d0
    subs        r8,r8,r10,lsl #1
    rsbmi       r8,r8,#0                    @ dp0 value is stored in r8
@  dp0 = abs( pu1_src[-3] - 2 * pu1_src[-2] + pu1_src[-1] )@



    add         r14,r1,r1,lsl #1
    add         r14,r0,r14

    vdup.32     d4,d24[1]
    ldrb        r2,[r14,#-3]                @ -2 value
    vdup.32     d7,d2[1]
    ldrb        r10,[r14,#-2]               @ -2 value
    vdup.32     d3,d2[0]
    ldrb        r11,[r14,#-1]               @ -1 value
    vdup.32     d5,d1[1]
    ldrb        r12,[r14,#0]                @ 0 value
    vdup.32     d6,d1[0]
    ldrb        r3,[r14,#1]                 @ 1 value
    vdup.32     d2,d0[0]
    ldrb        r4,[r14,#2]                 @ 2 value


    add         r12,r12,r4
    subs        r12,r12,r3,lsl #1           @ dq3value is stored in r12
    rsbmi       r12,r12,#0
@    dq3 = abs( pu1_src[3 * src_strd + 2] - 2 * pu1_src[3 * src_strd + 1]+ pu1_src[3 * src_strd + 0] )@


    add         r2,r2,r11
    subs        r11,r2,r10,lsl #1
    rsbmi       r11,r11,#0                  @ dp3 value is stored in r8
@    dp3 = abs( pu1_src[3 * src_strd - 3] - 2 * pu1_src[3 * src_strd - 2]   + pu1_src[3 * src_strd - 1] )@



    add         r3,r8,r9                    @ r3 has the d0 value
    add         r4,r11,r12                  @ r4 has the d3 value


@    d0 = dp0 + dq0@
@    d3 = dp3 + dq3@

    add         r14,r8,r11                  @ r13 has the value dp
    add         r12,r12,r9                  @ r12 has the value  dq
@    dp = dp0 + dp3@
@   dq = dq0 + dq3@

    add         r11, r3, r4                 @ r3 has the value d

@   d = d0 + d3@


    cmp         r11,r5
    vdup.32     d22,d0[1]
    bge         l1.964

@    if(d < beta)


    @ registers which cannont be altered : r3,r4 r5,r6,r12,r13,r0,r1,r11

    @ registers for use: r2,r7,r8,r9,r10,
    vqsub.u8    d30,d7,d19
    asr         r10,r5,#2
    vqadd.u8    d31,d7,d19
    cmp         r10,r3,lsl #1
    vaddl.u8    q0,d5,d4
    ble         l1.336

    ldrb        r2,[r0,#-4]
    vaddw.u8    q0,q0,d2
    ldrb        r7,[r0,#-1]
    vmull.u8    q10,d7,d23
    ldrb        r3,[r0,#0]
    vmlal.u8    q10,d22,d18
    ldrb        r8,[r0,#3]
@   ubfx   r7,r2,#24,#8           @ has the -1 value
@  and    r2,#0xff               @ has the -4 value
@  ubfx   r8,r3,#24,#8           @ has the 3 value
@  and    r3,#0xff               @ r4 has the 0 value

    vadd.i16    q10,q10,q0
    subs        r8,r8,r3
    vrshrn.i16  d22,q10,#3
    rsbmi       r8,r8,#0
    subs        r2,r2,r7
    vmin.u8     d21,d22,d31
    rsbmi       r2,r2,#0
    vmax.u8     d22,d21,d30
    add         r8,r8,r2
    vaddl.u8    q10,d7,d3
    cmp         r8,r5,asr #3
    vmla.i16    q10,q0,q8
    bge         l1.336
    vaddw.u8    q0,q0,d7
    subs        r7,r3,r7
    vrshrn.i16  d20,q10,#3
    rsbmi       r7,r7,#0
    vrshrn.i16  d0,q0,#2
    mov         r10,#5
    vqadd.u8    d30,d5,d19
    mul         r10,r10,r6
    vqsub.u8    d31,d5,d19
    add         r10,#1
    cmp         r7,r10,asr #1
    bge         l1.336


@        if( (2 * d3 < (beta >> 2)&& ( abs(pu1_src[3] - pu1_src[0]) + abs(pu1_src[-1] - pu1_src[-4])  < (beta >> 3) )
@            && abs(pu1_src[0] - pu1_src[-1]) < ( (5 * tc + 1) >> 1 ) )


    asr         r10,r5,#2
    vqsub.u8    d25,d4,d19
    cmp         r10,r4,lsl #1
    vqadd.u8    d21,d4,d19
    ble         l1.336
    vmin.u8     d26,d20,d21
    add         r4,r1,r1,lsl #1
    add         r4,r4,r0
    vmax.u8     d20,d26,d25
    ldrb        r2,[r4,#-4]
    vmin.u8     d19,d0,d30
    ldrb        r7,[r4,#-1]
    vmax.u8     d21,d19,d31
    ldrb        r3,[r4,#0]
    lsl         r10,r6,#1
    ldrb        r8,[r4,#3]
@   ubfx   r7,r2,#24,#8           @ has the -1 value
@  and    r2,#0xff               @ has the -4 value
@  ubfx   r8,r3,#24,#8           @ has the 3 value
@  and    r3,#0xff               @ r4 has the 0 value
    vaddl.u8    q0,d2,d3
    vdup.8      d19,r10
    subs        r8,r8,r3
    vaddw.u8    q0,q0,d4
    rsbmi       r8,r8,#0
    vqadd.u8    d30,d2,d19
    subs        r2,r2,r7
    vqsub.u8    d31,d2,d19
    rsbmi       r2,r2,#0
    vaddl.u8    q13,d5,d6
    add         r8,r8,r2
    vmla.i16    q13,q0,q8
    cmp         r8,r5,asr #3
    bge         l1.336
    vrshrn.i16  d26,q13,#3
    subs        r7,r3,r7
    vqadd.u8    d27,d3,d19
    rsbmi       r7,r7,#0
    vqsub.u8    d28,d3,d19
    mov         r10,#5
    vmin.u8     d16,d26,d30
    mul         r10,r10,r6
    add         r10,#1
    cmp         r7,r10,asr #1
    vmax.u8     d26,d16,d31
    bge         l1.336
    vqadd.u8    d30,d6,d19

    mov         r2,#2
    ldr         r4,[sp,#filter_p_offset]        @ loading the filter_flag_p
    vqsub.u8    d31,d6,d19
    ldr         r5,[sp,#filter_q_offset]        @ loading the filter_flag_q
    b           end_dep_deq_decision
@ r2 has the value of de
@ r6 has teh value of tc
@ r5 has the value of beta
@ r14 has the value of dp
@ r12 has the value of dq
@ r0 has the value of source address
@ r1 has the src stride

l1.336:
    mov         r2,#1
l1.424:
    mov         r11,r5
    ldr         r4,[sp,#filter_p_offset]        @ loading the filter_flag_p
    ldr         r5,[sp,#filter_q_offset]        @ loading the filter_flag_q

    cmp         r6,#1
    moveq       r9,#0
    moveq       r10,#0
    beq         end_dep_deq_decision

    and         r7,r4,r5

    cmp         r7,#1
    beq         both_flags_set
    cmp         r4,#0
    beq         set_flag_dep_zero


    add         r8,r11,r11,asr #1
    mov         r10,#0
    asr         r8,#3
    cmp         r8,r14
    movgt       r9,#1
    movle       r9,#0
    b           end_dep_deq_decision
set_flag_dep_zero:

    add         r8,r11,r11,asr #1
    mov         r9,#0
    asr         r8,#3
    cmp         r8,r12
    movgt       r10,#1
    movle       r10,#0
    b           end_dep_deq_decision

both_flags_set:
    add         r8,r11,r11,asr #1
    asr         r8,#3
    cmp         r8,r14
    movgt       r9,#1
    movle       r9,#0
    cmp         r8,r12
    movgt       r10,#1
    movle       r10,#0
end_dep_deq_decision:

@r0=source address
@r1=stride
@ r2 =de
@ r4=flag p
@r5= flag q
@r6 =tc
@ r9 =dep
@ r10=deq
@   b   l1.964


    cmp         r2,#2
@ r4 has the value of de
    bne         l1.968

    cmp         r5,#0
    beq         l1.780
@ r5 has the flag of q

    add         r3,r0,#2
    vst1.8      {d22[0]},[r3],r1

    vst1.8      {d22[1]},[r3],r1

    vst1.8      {d22[2]},[r3],r1

    vst1.8      {d22[3]},[r3]
    add         r3,r0,r1
    vtrn.8      d20,d21

    vst1.16     {d20[0]},[r0]
    vst1.16     {d21[0]},[r3],r1
    vst1.16     {d20[1]},[r3],r1
    vst1.16     {d21[1]},[r3]


l1.780:
    cmp         r4,#0
    beq         l1.964
    @ r5 has the flag p


    vdup.32     d7,d24[0]
    sub         r3,r0,#1
    vaddw.u8    q8,q0,d6
    add         r7,r3,r1
    vrshrn.i16  d2,q8,#2
    vst1.8      {d26[0]},[r3]
    sub         r0,r0,#3
    vmin.u8     d16,d2,d27
    vst1.8      {d26[1]},[r7],r1
    vmull.u8    q1,d6,d23
    vmlal.u8    q1,d7,d18
    vst1.8      {d26[2]},[r7],r1
    vmax.u8     d5,d16,d28
    vst1.8      {d26[3]},[r7]
    vadd.i16    q0,q1,q0
    vrshrn.i16  d0,q0,#3


    vmin.u8     d1,d0,d30
    vmax.u8     d0,d1,d31

    vtrn.8      d0,d5
    vst1.16     {d0[0]},[r0],r1
    vst1.16     {d5[0]},[r0],r1
    vst1.16     {d0[1]},[r0],r1
    vst1.16     {d5[1]},[r0]
l1.964:
    pop         {r3-r12,pc}
l1.968:


    vmov.i16    q0,#0x9
    rsb         r11,r6,#0
    cmp         r4,#0
    @ checks for the flag p
    vmov.i16    q8,#0x3
    vmov.i8     d24,#0x1


    vdup.8      d30,r11
    and         r11,r6,#0xff
    vdup.8      d31,r11

    vsubl.u8    q9,d4,d2
    vmul.i16    q9,q9,q0
    vsubl.u8    q0,d5,d3



    vmul.i16    q8,q0,q8
    vsub.i16    q8,q9,q8
    vrshr.s16   q8,q8,#4
@   delta = ( 9 * (pu1_src[0] - pu1_src[-1]) - 3 * (pu1_src[1] - pu1_src[-2]) + 8 ) >> 4@

    vabs.s16    q0,q8
    vmovn.i16   d0,q0
    @ storing the absolute values of delta in d0

    vqmovn.s16  d16,q8
    @ storing the clipped values of delta in d16

    vmov.i8     d1,#0xa
    vdup.8      d21,r11
    vmul.i8     d1,d1,d21
    @ d1 stores the value (10 * tc)

@if(abs(delta) < 10 * tc)

    vmin.s8     d18,d16,d31
    vmax.s8     d20,d18,d30

@ delta = clip3(delta, -tc, tc)@
    vmovl.s8    q8,d20
    vmovl.u8    q9,d2
    vadd.i16    q9,q9,q8

    vqmovun.s16 d22,q9
    vmovl.u8    q9,d4
    vsub.i16    q8,q9,q8
    vqmovun.s16 d23,q8
@ tmp_p0 = clip_u8(pu1_src[-1] + delta)@
@  tmp_q0 = clip_u8(pu1_src[0] - delta)@
    beq         l1.1272



    cmp         r9,#1
    bne         l1.1212
@ checks for the flag dep

    asr         r3,r6,#1


    vaddl.u8    q8,d6,d2
    vaddw.u8    q8,q8,d24
    vdup.8      d18,r3
    rsb         r3,r3,#0
    vdup.8      d19,r3
    vshr.u16    q8,q8,#1
    vmovn.i16   d16,q8

    vsubl.u8    q8,d16,d3
    vaddw.s8    q8,q8,d20
    vshr.s16    q8,q8,#1
    vqmovn.s16  d16,q8

    vmin.s8     d17,d16,d18
    vmax.s8     d16,d19,d17




    vmovl.u8    q9,d3
    vmovl.s8    q8,d16
    vadd.i16    q8,q9,q8

    vqmovun.s16 d16,q8
    vmov        d30,d3
    vcge.u8     d3,d0,d1


    vbsl        d3,d30,d16
l1.1212:
    vdup.8      d16,r11
    sub         r12,r0,#3
    sub         r3,r0,#1
@     vmul.i8  d16,d16,d1
    vtrn.8      d6,d3
    vst1.16     {d6[0]},[r12],r1
    vcge.u8     d16,d0,d1
    vst1.16     {d3[0]},[r12],r1
    vbsl        d16,d2,d22
    vst1.8      {d16[0]},[r3],r1
    vst1.8      {d16[1]},[r3],r1
    vst1.16     {d6[1]},[r12],r1
    vst1.8      {d16[2]},[r3],r1
    vst1.16     {d3[1]},[r12]
    vst1.8      {d16[3]},[r3]
l1.1272:
    cmp         r5,#0
    beq         l1.964
    @ checks for the flag q
    cmp         r10,#1
    bne         l1.1412
    @ checks for the flag deq
    vmov        d2,d7
    asr         r3,r6,#1

    vdup.8      d6,r3
    rsb         r3,r3,#0
    vdup.8      d16,r3
    vaddl.u8    q1,d2,d4
    vaddw.u8    q1,q1,d24
    vshr.u16    q1,q1,#1
    vmovn.i16   d2,q1

    vsubl.u8    q1,d2,d5
    vsubw.s8    q1,q1,d20
    vshr.s16    q1,q1,#1
    vqmovn.s16  d3,q1

    vmin.s8     d2,d3,d6
    vmax.s8     d3,d16,d2
    @  vdup.8   d6,r2
    @   vmul.i8  d6,d6,d1



    vmovl.u8    q8,d5
    vmovl.s8    q1,d3
    vadd.i16    q1,q8,q1
    vqmovun.s16 d3,q1
    vmov        d30,d5
    vcge.u8     d5,d0,d1


    vbsl        d5,d30,d3
l1.1412:
    @  vdup.8   d2,r2
    add         r3,r0,#2
    add         r11,r3,r1
    @   vmul.i8  d1,d2,d1
    vst1.8      {d7[0]},[r3]
    vst1.8      {d7[1]},[r11],r1
    vst1.8      {d7[2]},[r11],r1
    vcge.u8     d0,d0,d1
    vst1.8      {d7[3]},[r11]
    vbsl        d0,d4,d23
    vtrn.8      d0,d5
    vst1.16     {d0[0]},[r0],r1
    vst1.16     {d5[0]},[r0],r1
    vst1.16     {d0[1]},[r0],r1
    vst1.16     {d5[1]},[r0]
    pop         {r3-r12,pc}



