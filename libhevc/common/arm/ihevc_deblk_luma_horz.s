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

.equ    qp_q_offset,                108
.equ    beta_offset_div2_offset,    112
.equ    tc_offset_div2_offset,      116
.equ    filter_p_offset,            120
.equ    filter_q_offset,            124

.text
.align 4





.extern gai4_ihevc_tc_table
.extern gai4_ihevc_beta_table
.globl ihevc_deblk_luma_horz_a9q

gai4_ihevc_tc_table_addr:
.long gai4_ihevc_tc_table  - ulbl1 - 8

gai4_ihevc_beta_table_addr:
.long gai4_ihevc_beta_table  - ulbl2 - 8

.type ihevc_deblk_luma_horz_a9q, %function

ihevc_deblk_luma_horz_a9q:
    stmfd       sp!, {r3-r12,lr}
    vpush       {d8  -  d15}

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
    bgt         l1.1532
    cmp         r7,#0x0
    movlt       r7,#0x0                     @ r7 has the beta_index value
l1.1532:
    @     bic      r2,r2,#1
    asr         r2,r2,#1

    add         r3,r3,r2,lsl #1
    cmp         r3,#0x35
    movgt       r3,#0x35
    bgt         l1.1564
    cmp         r3,#0x0
    movlt       r3,#0x0                     @ r3 has the tc_index value

    @    qp_luma = (quant_param_p + quant_param_q + 1) >> 1@
    @    beta_indx = clip3(qp_luma + (beta_offset_div2 << 1), 0, 51)@
    @    tc_indx = clip3(qp_luma + (2 * (bs >> 1)) + (tc_offset_div2 << 1), 0, 53)@

l1.1564:
    ldr         r2,gai4_ihevc_beta_table_addr
ulbl2:
    add         r2,r2,pc
    ldr         r4,gai4_ihevc_tc_table_addr
ulbl1:
    add         r4,r4,pc

    ldr         r5,[r2,r7,lsl #2]           @ beta
    ldr         r6,[r4,r3,lsl #2]           @ tc



    cmp         r6,#0
    beq         l1.2404
    vmov.i16    d0,#0x2
    lsl         r7,r6,#1
    add         r14,r1,r1,lsl #1
    ldr         r8,[r0,-r14]                @ -3 value
    vdup.8      d1,r7
    ldr         r10,[r0,-r1,lsl #1]         @-2 value
    vdup.32     d23,r8                      @ -3 value
    ldr         r11,[r0,-r1]                @-1 value
    vdup.32     d24,r10                     @ -2 value
    and         r8,#0xff
    ldr         r12,[r0,#0]                 @ 0 value
    vdup.32     d25, r11                    @-1 value
    and         r10,#0xff
    ldr         r9,[r0,r1]                  @ 1 value
    vdup.32     d26,r12                     @ 0 value
    and         r11,#0xff
    ldr         r2,[r0,r1,lsl #1]           @ 2 value
    vdup.32     d27,r9                      @ 1value
    and         r12,#0xff
    vdup.32     d28,r2                      @ 2 value
    and         r9,#0xff
    and         r2,#0xff

    add         r12,r12,r2
    subs        r9,r12,r9,lsl #1            @ dq0 value is stored in r9
    rsbmi       r9,r9,#0
    @dq0 = abs( pu1_src[2] - 2 * pu1_src[1] + pu1_src[0] )@

    add         r8,r8,r11
    subs        r8,r8,r10,lsl #1
    rsbmi       r8,r8,#0                    @ dp0 value is stored in r8
    @  dp0 = abs( pu1_src[-3] - 2 * pu1_src[-2] + pu1_src[-1] )@



    add         r3,r1,r1,lsl #1
    add         r14,r0,#3


    ldrb        r2,[r14,-r3]                @ -2 value
    ldrb        r10,[r14,-r1,lsl #1]        @ -2 value
    ldrb        r11,[r14,-r1]               @ -1 value
    ldrb        r12,[r14,#0]                @ 0 value
    ldrb        r3,[r14,r1]                 @ 1 value
    ldrb        r4,[r14,r1,lsl #1]          @ 2 value


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
    bge         l1.2404

    @    if(d < beta)


    @ registers which cannont be altered : r3,r4 r5,r6,r12,r13,r0,r1,r11

    @ registers for use: r2,r7,r8,r9,r10,

    asr         r10,r5,#2
    vqadd.u8    d30,d26,d1
    cmp         r10,r3,lsl #1
    vqsub.u8    d31,d26,d1
    ble         l1.1840
    add         r10,r1,r1,lsl #1
    vaddl.u8    q3,d25,d26
    ldr         r2,[r0,-r1,lsl #2]          @ has the -4 value
    ldrb        r7,[r0,-r1]                 @ has the -1 value
    vdup.32     d22,r2                      @ -4 value
    vaddw.u8    q4,q3,d27
    ldrb        r3,[r0,#0]                  @ r4 has the 0 value
    vqadd.u8    d16,d27,d1
    and         r2,#0xff
    vmul.i16    q6,q4,d0[0]
    ldr         r8,[r0,r10]                 @ has the 3 value
    vaddl.u8    q5,d24,d28
    subs        r2,r2,r7
    vqsub.u8    d17,d27,d1
    vdup.32     d29,r8                      @ 3 value
    and         r8,#0xff
    vadd.i16    q6,q6,q5
    rsbmi       r2,r2,#0
    vrshrn.i16  d20,q6,#3
    subs        r8,r8,r3
    rsbmi       r8,r8,#0
    vmin.u8     d18,d20,d30
    add         r8,r8,r2

    cmp         r8,r5,asr #3
    bge         l1.1840
    vaddw.u8    q7,q4,d28
    subs        r7,r3,r7
    vmax.u8     d4,d18,d31
    rsbmi       r7,r7,#0
    vqadd.u8    d30,d28,d1
    mov         r10,#5
    vrshrn.i16  d21,q7,#2
    mul         r10,r10,r6
    vqsub.u8    d31,d28,d1
    add         r10,#1
    cmp         r7,r10,asr #1
    vmin.u8     d18,d21,d16
    bge         l1.1840


    @        if( (2 * d3 < (beta >> 2)&& ( abs(pu1_src[3] - pu1_src[0]) + abs(pu1_src[-1] - pu1_src[-4])  < (beta >> 3) )
    @            && abs(pu1_src[0] - pu1_src[-1]) < ( (5 * tc + 1) >> 1 ) )

    vmax.u8     d5,d18,d17
    asr         r10,r5,#2
    vaddl.u8    q8,d29,d28
    cmp         r10,r4,lsl #1
    ble         l1.1840

    add         r10,r1,r1,lsl #1
    vmul.i16    q8,q8,d0[0]
    add         r4,r0,#3


    ldrb        r2,[r4,-r1,lsl #2]
    vadd.i16    q8,q8,q7
    ldrb        r7,[r4,-r1]
    vrshrn.i16  d19,q8,#3
    ldrb        r3,[r4,#0]
    ldrb        r8,[r4,r10]
    @   ubfx   r7,r2,#24,#8           @ has the -1 value
    @  and    r2,#0xff               @ has the -4 value
    @  ubfx   r8,r3,#24,#8           @ has the 3 value
    @  and    r3,#0xff               @ r4 has the 0 value



    subs        r8,r8,r3
    vmin.u8     d18,d19,d30
    rsbmi       r8,r8,#0
    vaddl.u8    q3,d25,d24
    subs        r2,r2,r7
    vmax.u8     d3,d18,d31
    rsbmi       r2,r2,#0
    vaddw.u8    q4,q3,d26
    add         r8,r8,r2
    vqadd.u8    d30,d25,d1
    cmp         r8,r5,asr #3
    vqsub.u8    d31,d25,d1
    bge         l1.1840
    vmul.i16    q6,q4,d0[0]
    subs        r7,r3,r7
    vqadd.u8    d16,d24,d1
    rsbmi       r7,r7,#0
    vaddl.u8    q5,d23,d27
    mov         r10,#5
    vqsub.u8    d17,d24,d1
    mul         r10,r10,r6
    vadd.i16    q6,q6,q5
    add         r10,#1
    vrshrn.i16  d20,q6,#3
    cmp         r7,r10,asr #1
    vaddw.u8    q7,q4,d23
    bge         l1.1840
    vmin.u8     d18,d20,d30
    mov         r2,#2
    vqadd.u8    d30,d23,d1
    ldr         r4,[sp,#filter_p_offset]         @ loading the filter_flag_p
    vmax.u8     d2,d18,d31
    ldr         r5,[sp,#filter_q_offset]         @ loading the filter_flag_q
    vrshrn.i16  d21,q7,#2
    b           end_dep_deq_decision_horz
    @ r2 has the value of de
    @ r6 has teh value of tc
    @ r5 has the value of beta
    @ r14 has the value of dp
    @ r12 has the value of dq
    @ r0 has the value of source address
    @ r1 has the src stride

l1.1840:
    mov         r2,#1

    mov         r11,r5
    ldr         r4,[sp,#filter_p_offset]         @ loading the filter_flag_p
    ldr         r5,[sp,#filter_q_offset]         @ loading the filter_flag_q

    cmp         r6,#1
    moveq       r9,#0
    moveq       r10,#0
    beq         end_dep_deq_decision_horz

    and         r7,r4,r5
    cmp         r7,#1
    beq         both_flags_set_horz
    cmp         r4,#0
    beq         set_flag_dep_zero_horz


    add         r8,r11,r11,asr #1
    mov         r10,#0
    asr         r8,#3
    cmp         r8,r14
    movgt       r9,#1
    movle       r9,#0
    b           end_dep_deq_decision_horz
set_flag_dep_zero_horz:

    add         r8,r11,r11,asr #1
    mov         r9,#0
    asr         r8,#3
    cmp         r8,r12
    movgt       r10,#1
    movle       r10,#0
    b           end_dep_deq_decision_horz

both_flags_set_horz:
    add         r8,r11,r11,asr #1
    asr         r8,#3
    cmp         r8,r14
    movgt       r9,#1
    movle       r9,#0
    cmp         r8,r12
    movgt       r10,#1
    movle       r10,#0
end_dep_deq_decision_horz:

    @r0=source address
    @r1=stride
    @ r2 =de
    @ r4=flag p
    @r5= flag q
    @r6 =tc
    @ r9 =dep
    @ r10=deq



    @   add     r14,r1,r1,lsl #1
    @   lsl     r7,r6,#1
    @   vdup.8  d1,r7
    @   vmov.i16  d0,#0x2
    vmin.u8     d18,d21,d16
    cmp         r2,#1
    vqsub.u8    d31,d23,d1
    beq         l1.2408
    vaddl.u8    q4,d23,d22
    cmp         r5,#1

    bne         strong_filtering_p

strong_filtering_q:
    mov         r12,r0
    vst1.32     d4[0],[r12],r1
    vst1.32     d5[0],[r12],r1
    vst1.32     d3[0],[r12]
    cmp         r4,#1
    bne         l1.2404
strong_filtering_p:
    vmax.u8     d5,d18,d17
    mov         r12,r0
    vmul.i16    q4,q4,d0[0]
    rsb         r11,r1,#0
    vadd.i16    q8,q4,q7
    add         r12,r12,r11
    vrshrn.i16  d19,q8,#3
    vst1.32     d2[0],[r12],r11
    vmin.u8     d18,d19,d30
    vst1.32     d5[0],[r12],r11
    vmax.u8     d3,d18,d31
    vst1.32     d3[0],[r12]

l1.2404:
    vpop        {d8  -  d15}
    ldmfd       sp!, {r3-r12,pc}

    @ r4=flag p
    @r5= flag q
    @r6 =tc
    @ r9 =dep
    @ r10=deq


    @       d22          -4 value

    @d23        @ -3 value

    @   vdup.32 d24,r11         @ -2 value

    @   vdup.32 d25, r11        @-1 value

    @   vdup.32 d26,r11         @ 0 value

    @   vdup.32 d27,r11         @ 1value

    @   vdup.32 d28,r11         @ 2 value

    @   vdup.32 d29,r11         @ 3 value

l1.2408:

    vmov.i16    d0,#0x9

    vsubl.u8    q5,d26,d25

    vmul.i16    q5,q5,d0[0]

    vmov.i16    d0,#0x3

    vsubl.u8    q6,d27,d24
    vmul.i16    q6,q6,d0[0]


    vdup.8      d30,r6                      @ duplicating the +tc value

    rsb         r12,r6,#0
    vdup.8      d31,r12                     @ duplicating the -tc value



    vsub.i16    q5,q5,q6



    vrshr.s16   q5,q5,#4
    @   delta = ( 9 * (pu1_src[0] - pu1_src[-1]) - 3 * (pu1_src[1] - pu1_src[-2]) + 8 ) >> 4@

    vabs.s16    q4,q5
    vmovn.i16   d9,q4
    @ storing the absolute values of delta in d9

    vqmovn.s16  d10,q5
    @ storing the clipped values of delta in d16


    vmin.s8     d11,d10,d30
    vmax.s8     d8,d31,d11                  @ d8 has the value  delta = clip3(delta, -tc, tc)@


    vmovl.u8    q3,d25

    vaddw.s8    q2,q3,d8

    vqmovun.s16 d12,q2
    vmovl.u8    q3,d26
    vsubw.s8    q2,q3,d8
    vqmovun.s16 d13,q2


    mov         r11,#0xa
    mul         r12,r11,r6
    vdup.8      d2,r12                      @ d2 has the 10*tc value
    vmov        d18,d24
    vdup.8      d0,r6
    vshr.s8     d0,#1
    vneg.s8     d1,d0

    cmp         r4,#1
    bne         l1.2724
    cmp         r9,#1
    bne         l1.2700

    @ d12 and d13 have the value temp_p0 and temp_q0
    vaddl.u8    q7,d23,d25
    vrshrn.u16  d14,q7,#1
    vsubl.u8    q7,d14,d24
    vaddw.s8    q7,q7,d8
    vqshrn.s16  d14,q7,#1
    vmin.s8     d15,d14,d0
    vmax.s8     d14,d1,d15

    @ d14 has the delta p value
    vmovl.u8    q8,d24
    vaddw.s8    q8,q8,d14
    vqmovun.s16 d14,q8

    @  d14 =tmp_p1 = clip_u8(pu1_src[-2 * src_strd] + delta_p)@
    vcge.u8     d18,d9,d2
    vbsl        d18,d24,d14

l1.2700:
    mov         r12,r0
    rsb         r11,r1,#0
    add         r12,r11
    vcge.u8     d19,d9,d2
    vbsl        d19,d25,d12
    vst1.32     {d19[0]},[r12],r11
    vst1.32     {d18[0]},[r12]
l1.2724:
    cmp         r5,#1
    bne         l1.2404
    cmp         r10,#1
    vmov        d18, d27
    bne         l1.2852

    vaddl.u8    q7,d26,d28
    vrshrn.u16  d14,q7,#1
    vsubl.u8    q7,d14,d27
    vsubw.s8    q7,q7,d8
    vqshrn.s16  d14,q7,#1
    vmin.s8     d15,d14,d0
    vmax.s8     d14,d1,d15
@ d14 has the delta p value
    vmovl.u8    q8,d27
    vaddw.s8    q8,q8,d14
    vqmovun.s16 d14,q8
    vcge.u8     d18,d9,d2
    vbsl        d18,d27,d14
l1.2852:
    mov         r12,r0
    vcge.u8     d19,d9,d2
    vbsl        d19,d26,d13
    vst1.32     {d19[0]},[r12],r1
    vst1.32     {d18[0]},[r12]

    vpop        {d8  -  d15}
    ldmfd       sp!, {r3-r12,r15}



