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
@*  ihevc_inter_pred_chroma_vert_neon_w16inp_neon.s
@*
@* @brief
@*  contains function definitions for inter prediction  interpolation.
@* functions are coded using neon  intrinsics and can be compiled using

@* rvct
@*
@* @author
@*  yogeswaran rs / parthiban
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
@/**
@*******************************************************************************
@*
@* @brief
@*       chroma interprediction filter for 16bit vertical input.
@*
@* @par description:
@*    applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
@*    the elements pointed by 'pu1_src' and  writes to the location pointed by
@*    'pu1_dst'  input is 16 bits  the filter output is downshifted by 12 and
@*    clipped to lie  between 0 and 255   assumptions : the function is
@*    optimized considering the fact width and  height are multiple of 2.
@*
@* @param[in] pi2_src
@*  word16 pointer to the source
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
@void ihevc_inter_pred_chroma_vert_w16inp(word16 *pi2_src,
@                                          uword8 *pu1_dst,
@                                          word32 src_strd,
@                                          word32 dst_strd,
@                                          word8 *pi1_coeff,
@                                          word32 ht,
@                                          word32 wd)
@**************variables vs registers*****************************************
@r0 => *pu1_src
@r1 => *pi2_dst
@r2 =>  src_strd
@r3 =>  dst_strd

.equ    coeff_offset,   104
.equ    ht_offset,      108
.equ    wd_offset,      112


.text
.align 4




.globl ihevc_inter_pred_chroma_vert_w16inp_a9q

.type ihevc_inter_pred_chroma_vert_w16inp_a9q, %function

ihevc_inter_pred_chroma_vert_w16inp_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush        {d8 - d15}

    ldr         r4, [sp,#coeff_offset]                @loads pi1_coeff
    ldr         r6, [sp,#wd_offset]                @wd
    lsl         r2,r2,#1                    @src_strd = 2* src_strd
    ldr         r5,[sp,#ht_offset]                 @loads ht
    vld1.8      {d0},[r4]                   @loads pi1_coeff
    sub         r4,r0,r2                    @pu1_src - src_strd
    vmovl.s8    q0,d0                       @long the value

    tst         r6,#3                       @checks wd  == 2
    vdup.16     d12,d0[0]                   @coeff_0
    vdup.16     d13,d0[1]                   @coeff_1
    vdup.16     d14,d0[2]                   @coeff_2
    vdup.16     d15,d0[3]                   @coeff_3

    bgt         core_loop_ht_2              @jumps to loop handles wd 2

    tst         r5,#3                       @checks ht == mul of 4
    beq         core_loop_ht_4              @jumps to loop handles ht mul of 4

core_loop_ht_2:
    lsl         r7,r2,#1                    @2*src_strd
    lsl         r12,r3,#1                   @2*dst_strd
    lsl         r9,r6,#2                    @4*wd
    sub         r6,r12,r6,lsl #1            @2*dst_strd - 2*wd
    sub         r8,r7,r9                    @2*src_strd - 4*wd
    mov         r12,r9                      @4wd

inner_loop_ht_2:
    add         r0,r4,r2                    @increments pi2_src
    vld1.16     {d0},[r4]!                  @loads pu1_src
    vmull.s16   q0,d0,d12                   @vmull_s16(src_tmp1, coeff_0)
    subs        r12,r12,#8                  @2wd + 8
    vld1.16     {d2},[r0],r2                @loads pi2_src
    vmull.s16   q4,d2,d12                   @vmull_s16(src_tmp2, coeff_0)
    vld1.16     {d3},[r0],r2                @loads pi2_src
    vmlal.s16   q0,d2,d13
    vld1.16     {d6},[r0],r2
    vmlal.s16   q4,d3,d13
    vld1.16     {d2},[r0]
    add         r7,r1,r3                    @pu1_dst + dst_strd
    vmlal.s16   q0,d3,d14
    vmlal.s16   q4,d6,d14
    vmlal.s16   q0,d6,d15
    vmlal.s16   q4,d2,d15
    vqshrn.s32  d0,q0,#6                    @right shift
    vqshrn.s32  d30,q4,#6                   @right shift
    vqrshrun.s16 d0,q0,#6                   @rounding shift
    vqrshrun.s16 d30,q15,#6                 @rounding shift
    vst1.32     {d0[0]},[r1]!               @stores the loaded value
    vst1.32     {d30[0]},[r7]               @stores the loaded value
    bgt         inner_loop_ht_2             @inner loop -again

    @inner loop ends
    subs        r5,r5,#2                    @increments ht
    add         r1,r1,r6                    @pu1_dst += 2*dst_strd - 2*wd
    mov         r12,r9                      @4wd
    add         r4,r4,r8                    @pi1_src_tmp1 += 2*src_strd - 4*wd
    bgt         inner_loop_ht_2             @loop again

    b           end_loops                   @jumps to end

core_loop_ht_4:
    lsl         r7,r2,#2                    @2*src_strd
    lsl         r12,r3,#2                   @2*dst_strd
    mov         r11,r6,lsr #1               @divide by 2
    sub         lr,r12,r6,lsl #1            @2*dst_strd - 2*wd
    sub         r8,r7,r6,lsl #2             @2*src_strd - 4*wd

    mul         r12,r5,r11                  @multiply height by width
    sub         r12,#4                      @subtract by one for epilog
    mov         r11,r6,lsl #1               @2*wd

prolog:
    add         r0,r4,r2                    @increments pi2_src
    vld1.16     {d0},[r4]!                  @loads pu1_src
    vld1.16     {d1},[r0],r2                @loads pi2_src
    subs        r11,r11,#4
    vld1.16     {d2},[r0],r2                @loads pi2_src
    vmull.s16   q15,d0,d12                  @vmull_s16(src_tmp1, coeff_0)
    vld1.16     {d3},[r0],r2
    vmlal.s16   q15,d1,d13
    vmlal.s16   q15,d2,d14
    add         r9,r1,r3                    @pu1_dst + dst_strd
    vmlal.s16   q15,d3,d15

    vld1.16     {d4},[r0],r2
    vmull.s16   q14,d1,d12                  @vmull_s16(src_tmp2, coeff_0)
    addle       r4,r4,r8
    vmlal.s16   q14,d2,d13
    vld1.s16    {d5},[r0],r2
    vmlal.s16   q14,d3,d14
    vld1.s16    {d6},[r0],r2
    vmlal.s16   q14,d4,d15
    movle       r11,r6,lsl #1

    vqshrn.s32  d30,q15,#6                  @right shift

    vmull.s16   q13,d2,d12                  @vmull_s16(src_tmp2, coeff_0)
    add         r0,r4,r2
    vmlal.s16   q13,d3,d13
    vmlal.s16   q13,d4,d14
    vld1.16     {d0},[r4]!                  @loads pu1_src
    vmlal.s16   q13,d5,d15

    vqrshrun.s16 d30,q15,#6                 @rounding shift
    vqshrn.s32  d28,q14,#6                  @right shift

    vld1.16     {d1},[r0],r2                @loads pi2_src
    vmull.s16   q12,d3,d12                  @vmull_s16(src_tmp2, coeff_0)
    vst1.32     {d30[0]},[r1]!              @stores the loaded value
    vmlal.s16   q12,d4,d13
    vld1.16     {d2},[r0],r2                @loads pi2_src
    vmlal.s16   q12,d5,d14
    vld1.16     {d3},[r0],r2
    vmlal.s16   q12,d6,d15
    addle       r1,r1,lr

    vqshrn.s32  d26,q13,#6                  @right shift
    subs        r12,r12,#4
    vqrshrun.s16 d28,q14,#6                 @rounding shift

    beq         epilog                      @jumps to epilog

kernel_4:
    vmull.s16   q15,d0,d12                  @vmull_s16(src_tmp1, coeff_0)
    subs        r11,r11,#4
    vmlal.s16   q15,d1,d13
    vst1.32     {d28[0]},[r9],r3            @stores the loaded value
    vmlal.s16   q15,d2,d14
    vmlal.s16   q15,d3,d15

    vqshrn.s32  d24,q12,#6                  @right shift
    vqrshrun.s16 d26,q13,#6                 @rounding shift

    vld1.16     {d4},[r0],r2
    vmull.s16   q14,d1,d12                  @vmull_s16(src_tmp2, coeff_0)
    vmlal.s16   q14,d2,d13
    vmlal.s16   q14,d3,d14
    vmlal.s16   q14,d4,d15
    vst1.32     {d26[0]},[r9],r3            @stores the loaded value
    addle       r4,r4,r8
    movle       r11,r6,lsl #1

    vqshrn.s32  d30,q15,#6                  @right shift
    vqrshrun.s16 d24,q12,#6                 @rounding shift

    vld1.s16    {d5},[r0],r2
    vmull.s16   q13,d2,d12                  @vmull_s16(src_tmp2, coeff_0)
    vld1.s16    {d6},[r0],r2
    vmlal.s16   q13,d3,d13
    vst1.32     {d24[0]},[r9]               @stores the loaded value
    add         r0,r4,r2
    vmlal.s16   q13,d4,d14
    vld1.16     {d0},[r4]!                  @loads pu1_src
    vmlal.s16   q13,d5,d15

    vqshrn.s32  d28,q14,#6                  @right shift
    vqrshrun.s16 d30,q15,#6                 @rounding shift

    vld1.16     {d1},[r0],r2                @loads pi2_src
    vmull.s16   q12,d3,d12                  @vmull_s16(src_tmp2, coeff_0)
    add         r9,r1,r3                    @pu1_dst + dst_strd
    vld1.16     {d2},[r0],r2                @loads pi2_src
    vmlal.s16   q12,d4,d13
    vld1.16     {d3},[r0],r2
    vmlal.s16   q12,d5,d14

    vst1.32     {d30[0]},[r1]!              @stores the loaded value
    vmlal.s16   q12,d6,d15

    vqshrn.s32  d26,q13,#6                  @right shift
    vqrshrun.s16 d28,q14,#6                 @rounding shift
    addle       r1,r1,lr

    subs        r12,r12,#4

    bgt         kernel_4                    @jumps to kernel_4

epilog:
    vmull.s16   q15,d0,d12                  @vmull_s16(src_tmp1, coeff_0)
    vst1.32     {d28[0]},[r9],r3            @stores the loaded value
    vmlal.s16   q15,d1,d13
    vmlal.s16   q15,d2,d14
    vmlal.s16   q15,d3,d15

    vqshrn.s32  d24,q12,#6                  @right shift
    vqrshrun.s16 d26,q13,#6                 @rounding shift

    vmull.s16   q14,d1,d12                  @vmull_s16(src_tmp2, coeff_0)
    vld1.16     {d4},[r0],r2
    vmlal.s16   q14,d2,d13
    vst1.32     {d26[0]},[r9],r3            @stores the loaded value
    vmlal.s16   q14,d3,d14
    vmlal.s16   q14,d4,d15

    vqshrn.s32  d30,q15,#6                  @right shift
    vqrshrun.s16 d24,q12,#6                 @rounding shift

    vmull.s16   q13,d2,d12                  @vmull_s16(src_tmp2, coeff_0)
    vld1.s16    {d5},[r0],r2
    vmlal.s16   q13,d3,d13
    vmlal.s16   q13,d4,d14
    vmlal.s16   q13,d5,d15

    vqshrn.s32  d28,q14,#6                  @right shift
    vqrshrun.s16 d30,q15,#6                 @rounding shift

    vst1.32     {d24[0]},[r9]               @stores the loaded value
    vmull.s16   q12,d3,d12                  @vmull_s16(src_tmp2, coeff_0)
    vmlal.s16   q12,d4,d13
    add         r9,r1,r3                    @pu1_dst + dst_strd
    vld1.s16    {d6},[r0],r2
    vmlal.s16   q12,d5,d14
    vmlal.s16   q12,d6,d15
    vst1.32     {d30[0]},[r1]!              @stores the loaded value

    vqrshrun.s16 d28,q14,#6                 @rounding shift
    vqshrn.s32  d26,q13,#6                  @right shift

    vst1.32     {d28[0]},[r9],r3            @stores the loaded value
    vqrshrun.s16 d26,q13,#6                 @rounding shift

    vqshrn.s32  d24,q12,#6                  @right shift
    vst1.32     {d26[0]},[r9],r3            @stores the loaded value
    vqrshrun.s16 d24,q12,#6                 @rounding shift

    vst1.32     {d24[0]},[r9]               @stores the loaded value

end_loops:
    vpop         {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp




