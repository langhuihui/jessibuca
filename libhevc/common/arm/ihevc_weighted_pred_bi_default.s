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
@*  ihevc_weighted_pred_bi_default.s
@*
@* @brief
@*  contains function definitions for weighted prediction used in inter
@* prediction
@*
@* @author
@*  parthiban v
@*
@* @par list of functions:
@*  - ihevc_weighted_pred_bi_default()
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/
@/**
@*******************************************************************************
@*
@* @brief
@*  does default bi-weighted prediction on the arrays pointed by pi2_src1 and
@* pi2_src2 and stores it at location  pointed by pi2_dst assumptions : the
@* function is optimized considering the fact width and  height are multiple
@* of 2.
@*
@* @par description:
@*  dst = ( (src1 + lvl_shift1) +  (src2 + lvl_shift2) +  1 << (shift - 1) )
@* >> shift  where shift = 15 - bitdepth
@*
@* @param[in] pi2_src1
@*  pointer to source 1
@*
@* @param[in] pi2_src2
@*  pointer to source 2
@*
@* @param[out] pu1_dst
@*  pointer to destination
@*
@* @param[in] src_strd1
@*  source stride 1
@*
@* @param[in] src_strd2
@*  source stride 2
@*
@* @param[in] dst_strd
@*  destination stride
@*
@* @param[in] lvl_shift1
@*  added before shift and offset
@*
@* @param[in] lvl_shift2
@*  added before shift and offset
@*
@* @param[in] ht
@*  height of the source
@*
@* @param[in] wd
@*  width of the source
@*
@* @returns
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/
@void ihevc_weighted_pred_bi_default(word16 *pi2_src1,
@                                    word16 *pi2_src2,
@                                    uword8 *pu1_dst,
@                                    word32 src_strd1,
@                                    word32 src_strd2,
@                                    word32 dst_strd,
@                                    word32 lvl_shift1,
@                                    word32 lvl_shift2,
@                                    word32 ht,
@                                    word32 wd)

@**************variables vs registers*****************************************
@   r0 => *pi2_src1
@   r1 => *pi2_src2
@   r2 => *pu1_dst
@   r3 =>  src_strd1
@   r4 =>  src_strd2
@   r5 =>  dst_strd
@   r6 =>  lvl_shift1
@   r7 =>  lvl_shift2
@   r8 =>  ht
@   r9 =>  wd

.equ    src_strd2_offset,       104
.equ    dst_strd_offset,        108
.equ    lvl_shift1_offset,      112
.equ    lvl_shift2_offset,      116
.equ    ht_offset,              120
.equ    wd_offset,              124

.text
.syntax unified
.align 4




.globl ihevc_weighted_pred_bi_default_a9q

.type ihevc_weighted_pred_bi_default_a9q, %function

ihevc_weighted_pred_bi_default_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8  -  d15}
    ldr         r4,[sp,#src_strd2_offset]   @load src_strd2
    lsl         r3,r3,#1
    ldr         r5,[sp,#dst_strd_offset]    @load dst_strd
    ldr         r6,[sp,#lvl_shift1_offset]  @load lvl_shift1
    lsl         r4,r4,#1
    ldr         r7,[sp,#lvl_shift2_offset]  @load lvl_shift2
    ldr         r8,[sp,#ht_offset]          @load ht
    ldr         r9,[sp,#wd_offset]          @load wd
    vdup.16     q2,r6                       @lvl_shift1_t = vmov_n_s16((int16_t)lvl_shift1)
    vdup.16     q3,r7                       @lvl_shift2_t = vmov_n_s16((int16_t)lvl_shift2)
    vmov.i16    q0,#0x40                    @tmp_lvl_shift = 1 << (shift - 1)
    vadd.i16    q2,q3
    vadd.s16    q0,q0,q2
@   vmvn.i32    q1,#0x6                         @vmovq_n_s32(tmp_shift)
    lsl         r6,r9,#1
    rsb         r7,r6,r3,lsl #2             @4*src_strd1 - wd
    rsb         r10,r6,r4,lsl #2            @4*src_strd2 - wd
    @asr            r6,#1
    @rsb            r6,r6,r5,lsl #2             @4*dst_strd - wd

    cmp         r8,#0                       @check ht == 0
    beq         end_loops                   @if equal, then end the function

chroma_decision:
    orr         r14,r8,r9
    cmp         r14,#10
    beq         outer_loop_chroma_8x2

    cmp         r14,#6
    beq         outer_loop_chroma_4x2


luma_decision:
    cmp         r9,#24
    beq         outer_loop_8

    cmp         r9,#16
    bge         outer_loop_16

    cmp         r9,#12
    beq         outer_loop_4

    cmp         r9,#8
    bge         outer_loop_8






outer_loop_4:
    cmp         r9,#0                       @check wd == 0
    beq         end_loops                   @if equal, then end the function

core_loop_4:
    add         r11,r0,r3                   @pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         r12,r1,r4                   @pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    vld1.s16    {d6},[r0]!                  @load and increment the pi2_src1
    add         r14,r2,r5                   @pu1_dst_tmp = pu1_dst + dst_strd
    vld1.s16    {d7},[r1]!                  @load and increment the pi2_src2
    vld1.s16    {d8},[r11],r3               @load and increment the pi2_src1 ii iteration
    vqadd.s16   d18,d6,d7
    vqadd.s16   d18,d18,d0                  @vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t)
    vld1.s16    {d9},[r12],r4               @load and increment the pi2_src2 ii iteration
    vqadd.s16   d20,d8,d9                   @vaddq_s32(i4_tmp2_t1, i4_tmp2_t2)
    vqadd.s16   d19,d20,d0                  @vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t)
    vqshrun.s16 d20,q9,#7
    vld1.s16    {d22},[r11],r3              @load and increment the pi2_src1 iii iteration
    vld1.s16    {d23},[r12],r4              @load and increment the pi2_src2 iii iteration
    vqadd.s16   d30,d22,d23
    vqadd.s16   d30,d30,d0                  @vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t) iii iteration
    vld1.s16    {d24},[r11],r3              @load and increment the pi2_src1 iv iteration
    vld1.s16    {d25},[r12],r4              @load and increment the pi2_src2 iv iteration
    vqadd.s16   d18,d24,d25                 @vaddq_s32(i4_tmp2_t1, i4_tmp2_t2) iv iteration
    vqadd.s16   d31,d18,d0
    vst1.32     {d20[0]},[r2]!              @store pu1_dst i iteration
    vst1.32     {d20[1]},[r14],r5           @store pu1_dst ii iteration
    vqshrun.s16 d30,q15,#7
    vst1.32     {d30[0]},[r14],r5           @store pu1_dst iii iteration                                                @vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t) iv iteratio
    subs        r9,r9,#4                    @decrement wd by 4 and check for 0
    vst1.32     {d30[1]},[r14],r5           @store pu1_dst iv iteration
    bgt         core_loop_4                 @if greater than 0 repeat the core loop again

end_core_loop_4:

    subs        r8,r8,#4                    @decrement the ht by 4

    add         r0,r0,r7                    @pi2_src1 + 4*src_strd1 - 2*wd(since pi2_src1 is 16 bit pointer double the increment with double the wd decrement)
    asr         r9,r6,#1
    add         r1,r1,r10                   @pi2_src2 + 4*src_strd2 - 2*wd
    rsb         r14,r9,r5,lsl #2            @4*dst_strd - wd
    add         r2,r2,r14
                                            @pu1_dst + dst_std - wd
    bgt         core_loop_4                 @if ht is greater than 0 goto outer_loop

    b           end_loops


@ this is only for chroma module with input 2x2
outer_loop_chroma_4x2:
    cmp         r9,#0                       @check wd == 0
    beq         end_loops                   @if equal, then end the function
    rsb         r7,r6,r3,lsl #1             @2*src_strd1 - wd
    rsb         r10,r6,r4,lsl #1            @2*src_strd2 - wd
core_loop_chroma_4x2:
    add         r11,r0,r3                   @pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         r12,r1,r4                   @pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    vld1.s16    {d6},[r0]!                  @load and increment the pi2_src1
    add         r14,r2,r5                   @pu1_dst_tmp = pu1_dst + dst_strd
    vld1.s16    {d7},[r1]!                  @load and increment the pi2_src2
    vld1.s16    {d8},[r11],r3               @load and increment the pi2_src1 ii iteration
    vqadd.s16   d18,d6,d7
    vqadd.s16   d18,d18,d0                  @vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t)
    vld1.s16    {d9},[r12],r4               @load and increment the pi2_src2 ii iteration
    vqadd.s16   d20,d8,d9                   @vaddq_s32(i4_tmp2_t1, i4_tmp2_t2)
    vqadd.s16   d19,d20,d0                  @vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t)
    vqshrun.s16 d20,q9,#7
    vst1.32     {d20[0]},[r2]!              @store pu1_dst i iteration
    vst1.32     {d20[1]},[r14],r5           @store pu1_dst ii iteration

    subs        r9,r9,#4                    @decrement wd by 4 and check for 0

    bgt         core_loop_chroma_4x2        @if greater than 0 repeat the core loop again

end_core_loop_chorma_4x2:

    subs        r8,r8,#2                    @decrement the ht by 4

    add         r0,r0,r7                    @pi2_src1 + 2*src_strd1 - 2*wd(since pi2_src1 is 16 bit pointer double the increment with double the wd decrement)
    asr         r9,r6,#1
    add         r1,r1,r10                   @pi2_src2 + 2*src_strd2 - 2*wd
    rsb         r14,r9,r5,lsl #1            @2*dst_strd - wd
    add         r2,r2,r14
                                            @pu1_dst + dst_std - wd
    bgt         core_loop_chroma_4x2        @if ht is greater than 0 goto outer_loop

    b           end_loops



outer_loop_8:
    cmp         r9,#0                       @check wd == 0
    beq         end_loops                   @if equal, then end the function
    add         r11,r0,r3                   @pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         r12,r1,r4                   @pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
core_loop_8:

    vld1.s16    {q12},[r0]!                 @load and increment the pi2_src1
    add         r14,r2,r5                   @pu1_dst_tmp = pu1_dst + dst_strd
    vld1.s16    {q13},[r1]!                 @load and increment the pi2_src2
    vqadd.s16   q12,q12,q13
    vld1.s16    {q14},[r11],r3              @load and increment the pi2_src1 ii iteration
    vqadd.s16   q12,q12,q0                  @vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t)
    vld1.s16    {q15},[r12],r4              @load and increment the pi2_src2 ii iteration
    vld1.s16    {q8},[r11],r3               @load and increment the pi2_src1 iii iteration
    vqadd.s16   q11,q14,q15                 @vaddq_s32(i4_tmp2_t1, i4_tmp2_t2)
    vld1.s16    {q9},[r12],r4               @load and increment the pi2_src2 iii iteration
    vqadd.s16   q11,q11,q0                  @vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t)
    vqshrun.s16 d20,q12,#7
    vld1.s16    {q6},[r11],r3               @load and increment the pi2_src1 iv iteration
    vqadd.s16   q15,q8,q9
    vqshrun.s16 d21,q11,#7
    vld1.s16    {q7},[r12],r4               @load and increment the pi2_src2 iv iteration
    vqadd.s16   q15,q15,q0                  @vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t) iii iteration
    vst1.32     {d20},[r2]!                 @store pu1_dst i iteration
    vqadd.s16   q4,q6,q7                    @vaddq_s32(i4_tmp2_t1, i4_tmp2_t2) iv iteration
    vst1.32     {d21},[r14],r5              @store pu1_dst ii iteration
    vqadd.s16   q4,q4,q0
    vqshrun.s16 d30,q15,#7
    vqshrun.s16 d31,q4,#7
    add         r11,r0,r3                   @pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         r12,r1,r4                   @pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    vst1.32     {d30},[r14],r5              @store pu1_dst iii iteration                                                @vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t) iv iteratio
    subs        r9,r9,#8                    @decrement wd by 4 and check for 0
    vst1.32     {d31},[r14],r5              @store pu1_dst iv iteration
    bgt         core_loop_8                 @if greater than 0 repeat the core loop again

end_core_loop_8:

    subs        r8,r8,#4                    @decrement the ht by 4

    add         r0,r0,r7                    @pi2_src1 + 4*src_strd1 - 2*wd(since pi2_src1 is 16 bit pointer double the increment with double the wd decrement)
    asr         r9,r6,#1
    add         r1,r1,r10                   @pi2_src2 + 4*src_strd2 - 2*wd
    rsb         r14,r9,r5,lsl #2            @4*dst_strd - wd
    add         r2,r2,r14
    add         r11,r0,r3                   @pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         r12,r1,r4                   @pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)                                  @pu1_dst + dst_std - wd

    bgt         core_loop_8
    b           end_loops



@ this is only for chroma module with inpput 4x2
outer_loop_chroma_8x2:
    cmp         r9,#0                       @check wd == 0
    beq         end_loops                   @if equal, then end the function
    add         r11,r0,r3                   @pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         r12,r1,r4                   @pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    rsb         r7,r6,r3,lsl #1             @2*src_strd1 - wd
    rsb         r10,r6,r4,lsl #1            @2*src_strd2 - wd
core_loop_chroma_8x2:

    vld1.s16    {q12},[r0]!                 @load and increment the pi2_src1
    add         r14,r2,r5                   @pu1_dst_tmp = pu1_dst + dst_strd
    vld1.s16    {q13},[r1]!                 @load and increment the pi2_src2
    vqadd.s16   q12,q12,q13
    vld1.s16    {q14},[r11],r3              @load and increment the pi2_src1 ii iteration
    vqadd.s16   q12,q12,q0                  @vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t)
    vld1.s16    {q15},[r12],r4              @load and increment the pi2_src2 ii iteration
    vld1.s16    {q8},[r11],r3               @load and increment the pi2_src1 iii iteration
    vqadd.s16   q11,q14,q15                 @vaddq_s32(i4_tmp2_t1, i4_tmp2_t2)
    vqadd.s16   q11,q11,q0                  @vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t)
    vqshrun.s16 d20,q12,#7
    vqshrun.s16 d21,q11,#7
    vst1.32     {d20},[r2]!                 @store pu1_dst i iteration
    vst1.32     {d21},[r14],r5              @store pu1_dst ii iteration

    add         r11,r0,r3                   @pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         r12,r1,r4                   @pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
                                            @vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t) iv iteratio
    subs        r9,r9,#8                    @decrement wd by 4 and check for 0

    bgt         core_loop_chroma_8x2        @if greater than 0 repeat the core loop again

end_core_loop_chroma_8x2:

    subs        r8,r8,#2                    @decrement the ht by 4

    add         r0,r0,r7                    @pi2_src1 + 4*src_strd1 - 2*wd(since pi2_src1 is 16 bit pointer double the increment with double the wd decrement)
    asr         r9,r6,#1
    add         r1,r1,r10                   @pi2_src2 + 4*src_strd2 - 2*wd
    rsb         r14,r9,r5,lsl #1            @4*dst_strd - wd
    add         r2,r2,r14
    add         r11,r0,r3                   @pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         r12,r1,r4                   @pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)                                  @pu1_dst + dst_std - wd

    bgt         core_loop_chroma_8x2

    b           end_loops




outer_loop_16:
    cmp         r9,#0                       @check wd == 0
    beq         end_loops                   @if equal, then end the function
    add         r11,r0,r3                   @pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         r12,r1,r4                   @pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    rsb         r7,r6,r3,lsl #1             @2*src_strd1 - wd
    mov         r14,#16
    sub         r10,r14,r5
    sub         r11,r3,r14
    sub         r12,r14,r3

    rsb         r14,r9,r5,lsl #1            @2*dst_strd - wd



prolog_16:


    vld1.s16    {q1},[r0]!                  @load and increment the pi2_src1
    vld1.s16    {q2},[r1]!                  @load and increment the pi2_src2
    vld1.s16    {q5},[r0],r11               @load and increment the pi2_src1
    vld1.s16    {q6},[r1],r11               @load and increment the pi2_src2
    vld1.s16    {q3},[r0]!                  @load and increment the pi2_src1 ii iteration
    subs        r9,r9,#16
    vld1.s16    {q4},[r1]!                  @load and increment the pi2_src2 ii iteration
    subeq       r8,r8,#2
    vqadd.s16   q11,q1,q2
    vld1.s16    {q7},[r0],r12               @load and increment the pi2_src1 ii iteration
    vqadd.s16   q14,q5,q6
    vld1.s16    {q8},[r1],r12               @load and increment the pi2_src2 ii iteration
    addeq       r0,r0,r7
    addeq       r1,r1,r7
    vqadd.s16   q12,q3,q4
    vld1.s16    {q1},[r0]!
    vqadd.s16   q13,q7,q8
@ if the input is chroma with 8x2 block size
    cmp         r8,#0
    beq         epilog_16

    vld1.s16    {q2},[r1]!                  @load and increment the pi2_src2
    vqadd.s16   q11,q11,q0
    vld1.s16    {q5},[r0],r11               @load and increment the pi2_src1
    vqadd.s16   q14,q14,q0
    vld1.s16    {q6},[r1],r11               @load and increment the pi2_src2
    vqadd.s16   q12,q12,q0
    vld1.s16    {q3},[r0]!                  @load and increment the pi2_src1 ii iteration
    vqadd.s16   q15,q13,q0
    vqshrun.s16 d20,q11,#7
    vld1.s16    {q4},[r1]!                  @load and increment the pi2_src2 ii iteration
    vqshrun.s16 d21,q14,#7
    vld1.s16    {q7},[r0],r12               @load and increment the pi2_src1 ii iteration
    vqshrun.s16 d26,q12,#7
    vld1.s16    {q8},[r1],r12               @load and increment the pi2_src2 ii iteration
    vqshrun.s16 d27,q15,#7



core_loop_16:

    cmp         r9,#0
    vqadd.s16   q11,q1,q2
    asreq       r9,r6,#1
    vst1.32     {q10},[r2],r5
    vqadd.s16   q14,q5,q6
    vst1.32     {q13},[r2],r10
    addeq       r2,r2,r14
    vqadd.s16   q12,q3,q4
    subs        r9,r9,#16
    addeq       r0,r0,r7
    vqadd.s16   q13,q7,q8

    addeq       r1,r1,r7
    subseq      r8,r8,#2                    @decrement the ht by 2
    beq         epilog_16


    vqadd.s16   q11,q11,q0
    vld1.s16    {q1},[r0]!                  @load and increment the pi2_src1
    vqadd.s16   q14,q14,q0
    vld1.s16    {q2},[r1]!                  @load and increment the pi2_src2
    vqadd.s16   q12,q12,q0
    vld1.s16    {q5},[r0],r11               @load and increment the pi2_src1
    vqadd.s16   q15,q13,q0
    vld1.s16    {q6},[r1],r11               @load and increment the pi2_src2
    vqshrun.s16 d20,q11,#7
    vld1.s16    {q3},[r0]!                  @load and increment the pi2_src1 ii iteration
    vqshrun.s16 d21,q14,#7
    vld1.s16    {q4},[r1]!                  @load and increment the pi2_src2 ii iteration
    vqshrun.s16 d26,q12,#7
    vld1.s16    {q7},[r0],r12               @load and increment the pi2_src1 ii iteration
    vqshrun.s16 d27,q15,#7
    vld1.s16    {q8},[r1],r12               @load and increment the pi2_src2 ii iteration


    b           core_loop_16


epilog_16:

    vqadd.s16   q11,q11,q0
    vqadd.s16   q14,q14,q0
    vqadd.s16   q12,q12,q0
    vqadd.s16   q15,q13,q0
    vqshrun.s16 d20,q11,#7
    vqshrun.s16 d21,q14,#7
    vqshrun.s16 d26,q12,#7
    vqshrun.s16 d27,q15,#7
    vst1.32     {q10},[r2],r5
    vst1.32     {q13},[r2]



end_core_loop_16:








end_loops:
    vpop        {d8  -  d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp




