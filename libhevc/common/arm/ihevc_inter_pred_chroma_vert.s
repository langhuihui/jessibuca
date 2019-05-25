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
@*  ihevc_inter_pred_chroma_vert_neon.s
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
@/**
@*******************************************************************************
@*
@* @brief
@*   chroma interprediction filter for vertical input
@*
@* @par description:
@*    applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
@*    the elements pointed by 'pu1_src' and  writes to the location pointed by
@*    'pu1_dst'  the output is down shifted by 6 and clipped to 8 bits
@*    assumptions : the function is optimized considering the fact width is
@*    multiple of 2,4 or 8. and also considering height  should be multiple of 2
@*    width 4,8 is optimized further
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
@void ihevc_inter_pred_chroma_vert(uword8 *pu1_src,
@                                   uword8 *pu1_dst,
@                                   word32 src_strd,
@                                   word32 dst_strd,
@                                   word8 *pi1_coeff,
@                                   word32 ht,
@                                   word32 wd)
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




.globl ihevc_inter_pred_chroma_vert_a9q

.type ihevc_inter_pred_chroma_vert_a9q, %function

ihevc_inter_pred_chroma_vert_a9q:

    stmfd       sp!,{r4-r12,r14}            @stack stores the values of the arguments
    vpush        {d8 - d15}

    ldr         r4,[sp,#ht_offset]                 @loads ht
    ldr         r12,[sp,#coeff_offset]                @loads pi1_coeff
    cmp         r4,#0                       @checks ht == 0
    ldr         r6,[sp,#wd_offset]                 @loads wd
    sub         r0,r0,r2                    @pu1_src - src_strd
    vld1.8      {d0},[r12]                  @loads pi1_coeff

    ble         end_loops                   @jumps to end

    tst         r6,#3                       @checks (wd & 3)
    vabs.s8     d3,d0                       @vabs_s8(coeff)
    lsl         r10,r6,#1                   @2*wd
    vdup.8      d0,d3[0]                    @coeffabs_0
    vdup.8      d1,d3[1]                    @coeffabs_1
    vdup.8      d2,d3[2]                    @coeffabs_2
    vdup.8      d3,d3[3]                    @coeffabs_3

    bgt         outer_loop_wd_2             @jumps to loop handling wd ==2

    tst         r4,#7                       @checks ht for mul of 8
    beq         core_loop_ht_8              @when height is multiple of 8

    lsl         r7,r3,#1                    @2*dst_strd
    sub         r9,r7,r10                   @2*dst_strd - 2wd
    lsl         r12,r2,#1                   @2*src_strd
    sub         r8,r12,r10                  @2*src_strd - 2wd
    mov         r5,r10                      @2wd

inner_loop_ht_2:                            @called when wd is multiple of 4 and ht is 4,2

    add         r6,r0,r2                    @pu1_src +src_strd
    vld1.8      {d9},[r6],r2                @loads pu1_src
    subs        r5,r5,#8                    @2wd - 8
    vld1.8      {d5},[r0]!                  @loads src
    vmull.u8    q3,d9,d1                    @vmull_u8(vreinterpret_u8_u32(src_tmp2), coeffabs_1)
    vld1.8      {d4},[r6],r2                @loads incremented src
    vmlsl.u8    q3,d5,d0                    @vmlsl_u8(mul_res1, vreinterpret_u8_u32(src_tmp1), coeffabs_0)
    vld1.8      {d8},[r6],r2                @loads incremented src
    vmlal.u8    q3,d4,d2                    @vmlal_u8(mul_res1, vreinterpret_u8_u32(src_tmp3), coeffabs_2)
    vmull.u8    q2,d4,d1
    vmlsl.u8    q3,d8,d3
    vmlsl.u8    q2,d9,d0
    vld1.8      {d10},[r6]                  @loads the incremented src
    vmlal.u8    q2,d8,d2
    vqrshrun.s16 d6,q3,#6                   @shifts right
    vmlsl.u8    q2,d10,d3
    add         r6,r1,r3                    @pu1_dst + dst_strd
    vqrshrun.s16 d4,q2,#6                   @shifts right
    vst1.8      {d6},[r1]!                  @stores the loaded value

    vst1.8      {d4},[r6]                   @stores the loaded value

    bgt         inner_loop_ht_2             @inner loop again

    subs        r4,r4,#2                    @ht - 2
    add         r1,r1,r9                    @pu1_dst += (2*dst_strd - 2wd)
    mov         r5,r10                      @2wd
    add         r0,r0,r8                    @pu1_src += (2*src_strd - 2wd)

    bgt         inner_loop_ht_2             @loop again

    b           end_loops                   @jumps to end

outer_loop_wd_2:                            @called when width is multiple of 2
    lsl         r5,r3,#1                    @2*dst_strd
    mov         r12,r10                     @2wd
    sub         r9,r5,r10                   @2*dst_strd - 2wd
    lsl         r7,r2,#1                    @2*src_strd
    sub         r8,r7,r10                   @2*src_strd - 2wd

inner_loop_wd_2:

    add         r6,r0,r2                    @pu1_src + src_strd
    vld1.32     {d6[0]},[r0]                @vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp1, 0
    subs        r12,r12,#4                  @2wd - 4
    add         r0,r0,#4                    @pu1_src + 4
    vld1.32     {d6[1]},[r6],r2             @loads pu1_src_tmp
    vdup.32     d7,d6[1]
    vld1.32     {d7[1]},[r6],r2             @loads pu1_src_tmp
    vmull.u8    q2,d7,d1                    @vmull_u8(vreinterpret_u8_u32(src_tmp2), coeffabs_1)
    vdup.32     d7,d7[1]
    vld1.32     {d7[1]},[r6],r2
    vmlsl.u8    q2,d6,d0
    vmlal.u8    q2,d7,d2
    vdup.32     d7,d7[1]
    vld1.32     {d7[1]},[r6]
    add         r6,r1,r3                    @pu1_dst + dst_strd
    vmlsl.u8    q2,d7,d3
    vqrshrun.s16 d4,q2,#6                   @vrshrq_n_s16(vreinterpretq_s16_u16(mul_res1),6)
    vst1.32     {d4[0]},[r1]                @stores the loaded value
    add         r1,r1,#4                    @pu1_dst += 4
    vst1.32     {d4[1]},[r6]                @stores the loaded value

    bgt         inner_loop_wd_2             @inner loop again

    @inner loop ends
    subs        r4,r4,#2                    @ht - 2
    add         r1,r1,r9                    @pu1_dst += 2*dst_strd - 2*wd
    mov         r12,r10                     @2wd
    add         r0,r0,r8                    @pu1_src += 2*src_strd - 2*wd

    bgt         inner_loop_wd_2             @loop again

    b           end_loops                   @jumps to end

core_loop_ht_8:                             @when wd & ht is multiple of 8

    lsl         r12,r3,#2                   @4*dst_strd
    sub         r8,r12,r10                  @4*dst_strd - 2wd
    lsl         r12,r2,#2                   @4*src_strd
    sub         r9,r12,r10                  @4*src_strd - 2wd

    bic         r5,r10,#7                   @r5 ->wd
    mov         r14,r10,lsr #3              @divide by 8
    mul         r12,r4,r14                  @multiply height by width
    sub         r12,#4                      @subtract by one for epilog

prolog:
    add         r6,r0,r2                    @pu1_src + src_strd
    vld1.8      {d5},[r6],r2                @loads pu1_src
    subs        r5,r5,#8                    @2wd - 8
    vld1.8      {d4},[r0]!                  @loads the source
    vld1.8      {d6},[r6],r2                @load and increment
    vmull.u8    q15,d5,d1                   @mul with coeff 1
    vld1.8      {d7},[r6],r2                @load and increment
    vmlsl.u8    q15,d4,d0
    add         r7,r1,r3                    @pu1_dst
    vmlal.u8    q15,d6,d2
    vmlsl.u8    q15,d7,d3
    vld1.8      {d8},[r6],r2                @load and increment

    vmull.u8    q14,d6,d1                   @mul_res 2
    addle       r0,r0,r9                    @pu1_dst += 4*dst_strd - 2*wd
    vmlsl.u8    q14,d5,d0
    bicle       r5,r10,#7                   @r5 ->wd
    vmlal.u8    q14,d7,d2
    vld1.8      {d9},[r6],r2
    vmlsl.u8    q14,d8,d3
    vqrshrun.s16 d30,q15,#6

    vld1.8      {d10},[r6],r2
    vmull.u8    q13,d7,d1
    add         r6,r0,r2                    @pu1_src + src_strd
    vmlsl.u8    q13,d6,d0
    vst1.8      {d30},[r1]!                 @stores the loaded value
    vmlal.u8    q13,d8,d2
    vld1.8      {d4},[r0]!                  @loads the source
    vmlsl.u8    q13,d9,d3
    vqrshrun.s16 d28,q14,#6

    addle       r1,r1,r8                    @pu1_src += 4*src_strd - 2*wd
    vmull.u8    q12,d8,d1
    vld1.8      {d5},[r6],r2                @loads pu1_src
    vmlsl.u8    q12,d7,d0
    subs        r12,r12,#4
    vld1.8      {d6},[r6],r2                @load and increment
    vmlal.u8    q12,d9,d2
    vld1.8      {d7},[r6],r2                @load and increment
    vmlsl.u8    q12,d10,d3

    lsl         r11,r2,#2
    vst1.8      {d28},[r7],r3               @stores the loaded value
    vqrshrun.s16 d26,q13,#6
    rsb         r11,r2,r2,lsl #3
    add         r14,r2,r2,lsl #1
    add         r14,r14,r11
    ble         epilog                      @jumps to epilog

kernel_8:

    vmull.u8    q15,d5,d1                   @mul with coeff 1
    subs        r5,r5,#8                    @2wd - 8
    vmlsl.u8    q15,d4,d0
    addle       r0,r0,r9                    @pu1_dst += 4*dst_strd - 2*wd
    vmlal.u8    q15,d6,d2
    rsble       r11,r2,r2,lsl #3
    vmlsl.u8    q15,d7,d3
    vst1.8      {d26},[r7],r3               @stores the loaded value
    vqrshrun.s16 d24,q12,#6

    vld1.8      {d8},[r6],r2                @load and increment

    vmull.u8    q14,d6,d1                   @mul_res 2
    bicle       r5,r10,#7                   @r5 ->wd
    vmlsl.u8    q14,d5,d0
    vst1.8      {d24},[r7],r3               @stores the loaded value

    vmlal.u8    q14,d7,d2

    vld1.8      {d9},[r6],r2
    vqrshrun.s16 d30,q15,#6

    vmlsl.u8    q14,d8,d3
    vld1.8      {d10},[r6],r2
    add         r7,r1,r3                    @pu1_dst
    vmull.u8    q13,d7,d1
    add         r6,r0,r2                    @pu1_src + src_strd

    pld         [r0,r11]


    vmlsl.u8    q13,d6,d0
    vld1.8      {d4},[r0]!                  @loads the source

    vmlal.u8    q13,d8,d2
    vst1.8      {d30},[r1]!                 @stores the loaded value

    vmlsl.u8    q13,d9,d3
    vld1.8      {d5},[r6],r2                @loads pu1_src

    add         r11,r11,r2
    vqrshrun.s16 d28,q14,#6

    vmull.u8    q12,d8,d1
    vld1.8      {d6},[r6],r2                @load and increment
    addle       r1,r1,r8                    @pu1_src += 4*src_strd - 2*wd

    cmp         r11,r14
    rsbgt       r11,r2,r2,lsl #3

    vmlsl.u8    q12,d7,d0
    subs        r12,r12,#4

    vmlal.u8    q12,d9,d2
    vld1.8      {d7},[r6],r2                @load and increment

    vmlsl.u8    q12,d10,d3
    vst1.8      {d28},[r7],r3               @stores the loaded value
    vqrshrun.s16 d26,q13,#6

    bgt         kernel_8                    @jumps to kernel_8

epilog:

    vmull.u8    q15,d5,d1                   @mul with coeff 1
    vmlsl.u8    q15,d4,d0
    vmlal.u8    q15,d6,d2
    vmlsl.u8    q15,d7,d3
    vst1.8      {d26},[r7],r3               @stores the loaded value
    vqrshrun.s16 d24,q12,#6

    vld1.8      {d8},[r6],r2                @load and increment
    vmull.u8    q14,d6,d1                   @mul_res 2
    vmlsl.u8    q14,d5,d0
    vmlal.u8    q14,d7,d2
    vmlsl.u8    q14,d8,d3
    vst1.8      {d24},[r7],r3               @stores the loaded value
    vqrshrun.s16 d30,q15,#6

    vld1.8      {d9},[r6],r2
    vmull.u8    q13,d7,d1
    add         r7,r1,r3                    @pu1_dst
    vmlsl.u8    q13,d6,d0
    vst1.8      {d30},[r1]!                 @stores the loaded value

    vqrshrun.s16 d28,q14,#6
    vmlal.u8    q13,d8,d2
    vld1.8      {d10},[r6],r2
    vmlsl.u8    q13,d9,d3

    vmull.u8    q12,d8,d1
    vqrshrun.s16 d26,q13,#6
    vst1.8      {d28},[r7],r3               @stores the loaded value
    vmlsl.u8    q12,d7,d0
    vmlal.u8    q12,d9,d2
    vst1.8      {d26},[r7],r3               @stores the loaded value
    vmlsl.u8    q12,d10,d3

    vqrshrun.s16 d24,q12,#6
    vst1.8      {d24},[r7],r3               @stores the loaded value
end_loops:
    vpop         {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp



