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
@*******************************************************************************
@* @file
@*  ihevc_intra_pred_chroma_horz_neon.s
@*
@* @brief
@*  contains function definition for intra prediction  interpolation filters
@*
@*
@* @author
@*  parthiban v
@*
@* @par list of functions:
@*  - ihevc_intra_pred_luma_horz()
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/
@
@/**
@*******************************************************************************
@*
@* @brief
@*     intra prediction interpolation filter for horizontal luma variable.
@*
@* @par description:
@*      horizontal intraprediction(mode 10) with.extern  samples location
@*      pointed by 'pu1_ref' to the tu block  location pointed by 'pu1_dst'  refer
@*      to section 8.4.4.2.6 in the standard (special case)
@*
@* @param[in] pu1_src
@*  uword8 pointer to the source
@*
@* @param[out] pu1_dst
@*  uword8 pointer to the destination
@*
@* @param[in] src_strd
@*  integer source stride
@*
@* @param[in] dst_strd
@*  integer destination stride
@*
@* @param[in] nt
@*  integer transform block size
@*
@* @param[in] mode
@*  integer intraprediction mode
@*
@* @returns
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/
@void ihevc_intra_pred_chroma_horz(uword8 *pu1_ref,
@                                  word32 src_strd,
@                                  uword8 *pu1_dst,
@                                  word32 dst_strd,
@                                  word32 nt,
@                                  word32 mode)
@**************variables vs registers*****************************************
@r0 => *pu1_ref
@r1 =>  src_strd
@r2 => *pu1_dst
@r3 =>  dst_strd

.equ    nt_offset,      104

.text
.align 4




.globl ihevc_intra_pred_chroma_horz_a9q

.type ihevc_intra_pred_chroma_horz_a9q, %function

ihevc_intra_pred_chroma_horz_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8 - d15}

    ldr         r4,[sp,#nt_offset]          @loads nt

    lsl         r6,r4,#2                    @four_nt

    add         r12,r0,r6                   @*pu1_ref[four_nt]
    cmp         r4,#4                       @if nt == 4
    beq         core_loop_4

    cmp         r4,#8                       @if nt == 8
    beq         core_loop_8

    @cmp            r4,#16                          @if nt == 16
    @beq            core_loop_16

    sub         r12,r12,#16                 @move to 16th value pointer
    add         r9,r2,#16

core_loop_16:
    vld1.16     {q0},[r12]                  @load 16 values. d1[7] will have the 1st value.
    sub         r12,r12,#16
    vld1.16     {q5},[r12]                  @load 16 values. d1[7] will have the 1st value.

    vdup.16     q1,d1[3]                    @duplicate the i value.

    vdup.16     q2,d1[2]                    @duplicate the ii value.
    vdup.16     q3,d1[1]                    @duplicate the iii value.
    vst1.16     {q1},[r2],r3                @store in 1st row 0-16 columns
    vst1.16     {q1},[r9],r3                @store in 1st row 16-32 columns

    vdup.16     q4,d1[0]
    vst1.16     {q2},[r2],r3
    vst1.16     {q2},[r9],r3

    vdup.16     q1,d0[3]
    vst1.16     {q3},[r2],r3
    vst1.16     {q3},[r9],r3

    vdup.16     q2,d0[2]
    vst1.16     {q4},[r2],r3
    vst1.16     {q4},[r9],r3

    vdup.16     q3,d0[1]
    vst1.16     {q1},[r2],r3
    vst1.16     {q1},[r9],r3

    vdup.16     q4,d0[0]
    vst1.16     {q2},[r2],r3
    vst1.16     {q2},[r9],r3

    vdup.16     q1,d11[3]
    vst1.16     {q3},[r2],r3
    vst1.16     {q3},[r9],r3

    vdup.16     q2,d11[2]
    vst1.16     {q4},[r2],r3
    vst1.16     {q4},[r9],r3

    vdup.16     q3,d11[1]
    vst1.16     {q1},[r2],r3
    vst1.16     {q1},[r9],r3

    vdup.16     q4,d11[0]
    vst1.16     {q2},[r2],r3
    vst1.16     {q2},[r9],r3

    vdup.16     q1,d10[3]
    vst1.16     {q3},[r2],r3
    vst1.16     {q3},[r9],r3

    vdup.16     q2,d10[2]
    vst1.16     {q4},[r2],r3
    vst1.16     {q4},[r9],r3

    vdup.16     q3,d10[1]
    vst1.16     {q1},[r2],r3
    vst1.16     {q1},[r9],r3
    sub         r12,r12,#16                 @move to 16th value pointer

    vdup.16     q4,d10[0]
    vst1.16     {q2},[r2],r3
    vst1.16     {q2},[r9],r3

    subs        r4,r4,#16                   @decrement the loop count by 16
    vst1.16     {q3},[r2],r3
    vst1.16     {q3},[r9],r3

    vst1.16     {q4},[r2],r3
    vst1.16     {q4},[r9],r3
    bgt         core_loop_16
    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp
    b           endloop

core_loop_8:
    ldrb        lr,[r12],#1                 @pu1_ref[two_nt]
    @vld1.8     {q15},[r12]                     @pu1_ref[two_nt + 1 + col]

    vdup.8      d28,lr
    sub         r12,r12,#17
    vld1.8      {q0},[r12]

    sub         r12,r12,#16
    vld1.8      {q15},[r12]
    vdup.16     q5,d1[3]
    @vmovl.u8   q13,d26

    vdup.16     q1,d1[2]
    @vsubl.u8   q12,d30,d28

    vdup.16     q2,d1[1]
    @vshr.s16   q12,q12,#1

    vdup.16     q3,d1[0]
    @vqadd.s16  q11,q13,q12

    vdup.16     q4,d0[3]
    @vqmovun.s16 d22,q11

    vst1.16     {q5},[r2],r3

    vdup.16     q5,d0[2]
    @vsubl.u8   q12,d31,d28

    vdup.16     q6,d0[1]
    @vshr.s16   q12,q12,#1

    vdup.16     q7,d0[0]
    @vqadd.s16  q11,q13,q12

    vdup.16     q8,d0[3]
    @vqmovun.s16 d22,q11

    vst1.16     {q1},[r2],r3
    @sub            r2,r2,#8

    vst1.16     {q2},[r2],r3

    vst1.16     {q3},[r2],r3
    vst1.16     {q4},[r2],r3
    vst1.16     {q5},[r2],r3

    @vdup.8     q1,d0[2]
    vst1.16     {q6},[r2],r3

    @vdup.8     q2,d0[1]
    vst1.16     {q7},[r2],r3

    @vdup.8     q3,d0[0]
    @vst1.8     {q7},[r2],r3

    @vdup.8     q4,d0[3]
    @vst1.8     {q8},[r2],r3

    @vdup.8     q5,d0[2]
    @vst1.8     {q1},[r2],r3

    @vdup.8     q6,d0[1]
    @vst1.8     {q2},[r2],r3

    @vdup.8     q7,d0[0]
    @vst1.8     {q3},[r2],r3

    @vst1.8     {q4},[r2],r3
    @vst1.8     {q5},[r2],r3
    @vst1.8     {q6},[r2],r3
    @vst1.8     {q7},[r2],r3
    vpop        {d8 - d15}

    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp
    b           endloop


core_loop_4:
    ldrb        lr,[r12]                    @pu1_ref[two_nt]
    add         r12,r12,#1                  @pu1_ref[two_nt + 1]
    @vld1.8     {d30},[r12]                     @pu1_ref[two_nt + 1 + col]

    sub         r12,r12,#9
    vld1.8      {d0},[r12]
    sub         r12,r12,#8
    vld1.8      {d30},[r12]
    vdup.16     d26,d0[3]
    vdup.8      d28,lr

    vdup.16     d3,d0[2]
    vmovl.u8    q13,d26

    vdup.16     d4,d0[1]
    vsubl.u8    q12,d30,d28

    vdup.16     d5,d0[0]
    vshr.s16    q12,q12,#1

    vdup.16     d6,d0[3]
    vqadd.s16   q11,q13,q12

    vdup.16     d7,d0[2]
    vqmovun.s16 d22,q11

    vst1.8      {d6},[r2],r3
    vst1.8      {d3},[r2],r3

    vdup.16     d8,d0[1]
    vst1.8      {d4},[r2],r3
    vst1.8      {d5},[r2],r3

    vdup.16     d9,d0[0]
    @vst1.8     {d6},[r2],r3
    @vst1.8     {d7},[r2],r3

    @vst1.8     {d8},[r2],r3
    @vst1.8     {d9},[r2],r3
    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp
    b           endloop


@core_loop_4
    ldrb        lr,[r12]                    @pu1_ref[two_nt]
    add         r12,r12,#1                  @pu1_ref[two_nt + 1]
    vld1.8      {d30},[r12]                 @pu1_ref[two_nt + 1 + col]

    sub         r12,r12,#5
    vld1.8      {d0},[r12]
    vdup.8      d28,lr
    vdup.8      d26,d0[3]
    vmovl.u8    q13,d26

    vdup.8      d3,d0[2]
    vsubl.u8    q12,d30,d28

    vdup.8      d4,d0[1]
    vshr.s16    q12,q12,#1

    vdup.8      d5,d0[0]
    vqadd.s16   q11,q13,q12

    vqmovun.s16 d22,q11

    vst1.32     {d22[0]},[r2],r3
    vst1.32     {d3[0]},[r2],r3
    vst1.32     {d4[0]},[r2],r3
    vst1.32     {d5[0]},[r2],r3

    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp

endloop:


