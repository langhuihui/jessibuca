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
@*  ihevc_inter_pred_chroma_copy_w16out_neon.s
@*
@* @brief
@*  contains function definitions for inter prediction  interpolation.
@* functions are coded using neon  intrinsics and can be compiled using

@* rvct
@*
@* @author
@*  yogeswaran rs
@*
@* @par list of functions:
@*
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
@*   chroma interprediction filter for copy
@*
@* @par description:
@*    copies the array of width 'wd' and height 'ht' from the  location pointed
@*    by 'src' to the location pointed by 'dst'
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
@* @param[in] pi1_coeff
@*  word8 pointer to the filter coefficients
@*
@* @param[in] ht
@*  integer height of the array
@*
@* @param[in] wd
@*  integer width of the array
@*
@* @returns
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/

@void ihevc_inter_pred_chroma_copy_w16out(uword8 *pu1_src,
@                                           word16 *pi2_dst,
@                                           word32 src_strd,
@                                           word32 dst_strd,
@                                           word8 *pi1_coeff,
@                                           word32 ht,
@                                           word32 wd)
@**************variables vs registers*****************************************
@r0 => *pu1_src
@r1 => *pi2_dst
@r2 =>  src_strd
@r3 =>  dst_strd
@r4 => *pi1_coeff
@r5 =>  ht
@r6 =>  wd

.equ    coeff_offset,   104
.equ    ht_offset,      108
.equ    wd_offset,      112


.text
.align 4




.globl ihevc_inter_pred_chroma_copy_w16out_a9q

.type ihevc_inter_pred_chroma_copy_w16out_a9q, %function

ihevc_inter_pred_chroma_copy_w16out_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush        {d8 - d15}

    ldr         r12,[sp,#wd_offset]                @loads wd
    lsl         r12,r12,#1                  @2*wd
    ldr         r7,[sp,#ht_offset]                 @loads ht
    cmp         r7,#0                       @ht condition(ht == 0)
    ble         end_loops                   @loop
    and         r8,r7,#3                    @check ht for mul of 2
    sub         r9,r7,r8                    @check the rounded height value
    and         r11,r7,#6
    cmp         r11,#6
    beq         loop_ht_6
    tst         r12,#7                      @conditional check for wd (multiples)
    beq         core_loop_wd_8

loop_ht_6:
    sub         r11,r12,#4
    lsls        r6,r3,#1
    cmp         r9,#0
    beq         outer_loop_wd_4_ht_2

outer_loop_wd_4:
    subs        r4,r12,#0                   @wd conditional subtract
    ble         end_inner_loop_wd_4

inner_loop_wd_4:
    vld1.8      {d0},[r0]                   @vld1_u8(pu1_src_tmp)
    add         r5,r0,r2                    @pu1_src +src_strd
    vmovl.u8    q0,d0                       @vmovl_u8(vld1_u8(pu1_src_tmp)
    add         r10,r1,r6
    subs        r4,r4,#4                    @wd - 4
    vshl.i64    q0,q0,#6                    @vshlq_n_s64(temp, 6)
    vld1.8      {d22},[r5],r2               @vld1_u8(pu1_src_tmp)
    add         r0,r0,#4                    @pu1_src += 4
    vst1.64     {d0},[r1]                   @vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    add         r1,r1,#8
    vmovl.u8    q11,d22                     @vmovl_u8(vld1_u8(pu1_src_tmp)
    vld1.8      {d24},[r5],r2               @vld1_u8(pu1_src_tmp)
    vshl.i64    q11,q11,#6                  @vshlq_n_s64(temp, 6)
    vmovl.u8    q12,d24                     @vmovl_u8(vld1_u8(pu1_src_tmp)
    vst1.64     {d22},[r10],r6              @vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    vshl.i64    q12,q12,#6                  @vshlq_n_s64(temp, 6)
    vld1.8      {d26},[r5],r2               @vld1_u8(pu1_src_tmp)
    vst1.64     {d24},[r10],r6              @vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    vmovl.u8    q13,d26                     @vmovl_u8(vld1_u8(pu1_src_tmp)
    vshl.i64    q13,q13,#6                  @vshlq_n_s64(temp, 6)
    vst1.64     {d26},[r10],r6              @vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    bgt         inner_loop_wd_4

end_inner_loop_wd_4:
    subs        r9,r9,#4                    @ht - 4
    sub         r0,r5,r11
    sub         r1,r10,r11,lsl #1
    bgt         outer_loop_wd_4
    cmp         r8,#0
    bgt         outer_loop_wd_4_ht_2


end_loops:
    vpop         {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp


outer_loop_wd_4_ht_2:
    subs        r4,r12,#0                   @wd conditional subtract
    ble         end_inner_loop_wd_4

inner_loop_wd_4_ht_2:
    vld1.8      {d0},[r0]                   @vld1_u8(pu1_src_tmp)
    add         r5,r0,r2                    @pu1_src +src_strd
    vmovl.u8    q0,d0                       @vmovl_u8(vld1_u8(pu1_src_tmp)
    add         r10,r1,r6
    subs        r4,r4,#4                    @wd - 4
    vshl.i64    q0,q0,#6                    @vshlq_n_s64(temp, 6)
    vld1.8      {d22},[r5],r2               @vld1_u8(pu1_src_tmp)
    add         r0,r0,#4                    @pu1_src += 4
    vst1.64     {d0},[r1]                   @vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    add         r1,r1,#8
    vmovl.u8    q11,d22                     @vmovl_u8(vld1_u8(pu1_src_tmp)
    vld1.8      {d24},[r5],r2               @vld1_u8(pu1_src_tmp)
    vshl.i64    q11,q11,#6                  @vshlq_n_s64(temp, 6)
    vmovl.u8    q12,d24                     @vmovl_u8(vld1_u8(pu1_src_tmp)
    vst1.64     {d22},[r10],r6              @vst1q_lane_s64(pi2_dst_tmp, temp, 0)
    bgt         inner_loop_wd_4_ht_2
    b           end_loops


core_loop_wd_8:
    @sub            r11,r12,#8
    lsls        r5,r3,#1
    rsb         r11,r12,r3, lsl #2          @ r11 = (dst_strd * 4) - width
    rsb         r8,r12,r2,lsl #2            @r2->src_strd
    mov         r4,r12, lsr #3              @ divide by 8
    mov         r7,r9
    mul         r7, r4
    sub         r4,r12,#0                   @wd conditional check
    sub         r7,r7,#4                    @subtract one for epilog
    cmp         r9,#0
    beq         core_loop_wd_8_ht_2

prolog:
    add         r6,r0,r2                    @pu1_src_tmp += src_strd
    add         r10,r1,r5
    vld1.8      {d8},[r0]!                  @vld1_u8(pu1_src_tmp)
    vld1.8      {d10},[r6],r2               @vld1_u8(pu1_src_tmp)
    vld1.8      {d12},[r6],r2               @vld1_u8(pu1_src_tmp)
    vld1.8      {d14},[r6],r2               @vld1_u8(pu1_src_tmp)
    vmovl.u8    q8,d8                       @vmovl_u8(vld1_u8(pu1_src_tmp))
    vmovl.u8    q9,d10                      @vmovl_u8(vld1_u8(pu1_src_tmp)
    vmovl.u8    q10,d12                     @vmovl_u8(vld1_u8(pu1_src_tmp)
    vmovl.u8    q11,d14                     @vmovl_u8(vld1_u8(pu1_src_tmp)
    subs        r4,r4,#8                    @wd decrements by 8
    vshl.i16    q0,q8,#6                    @vshlq_n_s16(tmp, 6)
    vshl.i16    q1,q9,#6                    @vshlq_n_s16(tmp, 6)
    vshl.i16    q2,q10,#6                   @vshlq_n_s16(tmp, 6)
    vshl.i16    q3,q11,#6                   @vshlq_n_s16(tmp, 6)
    addle       r0,r0,r8
    add         r6,r0,r2                    @pu1_src_tmp += src_strd
    vld1.8      {d8},[r0]!                  @vld1_u8(pu1_src_tmp)
    vld1.8      {d10},[r6],r2               @vld1_u8(pu1_src_tmp)
    vld1.8      {d12},[r6],r2               @vld1_u8(pu1_src_tmp)
    vld1.8      {d14},[r6],r2               @vld1_u8(pu1_src_tmp)

    vst1.16     {d0,d1},[r1]!               @vst1q_s16(pi2_dst_tmp, tmp)
    addle       r1,r1,r11,lsl #1
    suble       r4,r12,#0                   @wd conditional check

    subs        r7,r7,#4                    @ht - 4

    blt         epilog_end                  @jumps to epilog_end
    beq         epilog                      @jumps to epilog



outer_loop_wd_8:

    vst1.16     {d2,d3},[r10],r5            @vst1q_s16(pi2_dst_tmp, tmp)
    vmovl.u8    q8,d8                       @vmovl_u8(vld1_u8(pu1_src_tmp))

    vst1.16     {d4,d5},[r10],r5            @vst1q_s16(pi2_dst_tmp, tmp)
    vmovl.u8    q9,d10                      @vmovl_u8(vld1_u8(pu1_src_tmp)

    vst1.16     {d6,d7},[r10],r5            @vst1q_s16(pi2_dst_tmp, tmp)
    vmovl.u8    q10,d12                     @vmovl_u8(vld1_u8(pu1_src_tmp)

    vmovl.u8    q11,d14                     @vmovl_u8(vld1_u8(pu1_src_tmp)

    subs        r4,r4,#8                    @wd decrements by 8
    addle       r0,r0,r8

    add         r6,r0,r2                    @pu1_src_tmp += src_strd

    vld1.8      {d8},[r0]!                  @vld1_u8(pu1_src_tmp)
    vshl.i16    q0,q8,#6                    @vshlq_n_s16(tmp, 6)

    vld1.8      {d10},[r6],r2               @vld1_u8(pu1_src_tmp)
    vshl.i16    q1,q9,#6                    @vshlq_n_s16(tmp, 6)

    vld1.8      {d12},[r6],r2               @vld1_u8(pu1_src_tmp)
    vshl.i16    q2,q10,#6                   @vshlq_n_s16(tmp, 6)

    vld1.8      {d14},[r6],r2               @vld1_u8(pu1_src_tmp)
    add         r10,r1,r5

    vshl.i16    q3,q11,#6                   @vshlq_n_s16(tmp, 6)

    vst1.16     {d0,d1},[r1]!               @vst1q_s16(pi2_dst_tmp, tmp)

    addle       r1,r1,r11,lsl #1
    suble       r4,r12,#0                   @wd conditional check

    subs        r7,r7,#4                    @ht - 4
    bgt         outer_loop_wd_8

epilog:
    vst1.16     {d2,d3},[r10],r5            @vst1q_s16(pi2_dst_tmp, tmp)
    vmovl.u8    q8,d8                       @vmovl_u8(vld1_u8(pu1_src_tmp))

    vst1.16     {d4,d5},[r10],r5            @vst1q_s16(pi2_dst_tmp, tmp)
    vmovl.u8    q9,d10                      @vmovl_u8(vld1_u8(pu1_src_tmp)

    vst1.16     {d6,d7},[r10],r5            @vst1q_s16(pi2_dst_tmp, tmp)
    vmovl.u8    q10,d12                     @vmovl_u8(vld1_u8(pu1_src_tmp)

    vmovl.u8    q11,d14                     @vmovl_u8(vld1_u8(pu1_src_tmp)
    @add        r6,r0,r2                @pu1_src_tmp += src_strd

    vshl.i16    q0,q8,#6                    @vshlq_n_s16(tmp, 6)
    vshl.i16    q1,q9,#6                    @vshlq_n_s16(tmp, 6)
    vshl.i16    q2,q10,#6                   @vshlq_n_s16(tmp, 6)
    add         r10,r1,r5
    vshl.i16    q3,q11,#6                   @vshlq_n_s16(tmp, 6)

    vst1.16     {d0,d1},[r1]!               @vst1q_s16(pi2_dst_tmp, tmp)
epilog_end:
    vst1.16     {d2,d3},[r10],r5            @vst1q_s16(pi2_dst_tmp, tmp)
    vst1.16     {d4,d5},[r10],r5            @vst1q_s16(pi2_dst_tmp, tmp)
    vst1.16     {d6,d7},[r10],r5            @vst1q_s16(pi2_dst_tmp, tmp)
    b           end_loops

core_loop_wd_8_ht_2:
    add         r6,r0,r2                    @pu1_src_tmp += src_strd
    add         r10,r1,r5
    vld1.8      {d8},[r0]!                  @vld1_u8(pu1_src_tmp)
    vld1.8      {d10},[r6],r2               @vld1_u8(pu1_src_tmp)
    vmovl.u8    q8,d8                       @vmovl_u8(vld1_u8(pu1_src_tmp))
    vmovl.u8    q9,d10                      @vmovl_u8(vld1_u8(pu1_src_tmp)
    subs        r12,r12,#8                  @wd decrements by 8
    vshl.i16    q0,q8,#6                    @vshlq_n_s16(tmp, 6)
    vshl.i16    q1,q9,#6                    @vshlq_n_s16(tmp, 6)
    vst1.16     {d0,d1},[r1]!               @vst1q_s16(pi2_dst_tmp, tmp)
    vst1.16     {d2,d3},[r10],r5            @vst1q_s16(pi2_dst_tmp, tmp)
    bgt         core_loop_wd_8_ht_2

    vpop         {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp






