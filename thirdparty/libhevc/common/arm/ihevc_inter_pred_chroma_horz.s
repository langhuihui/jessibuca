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
@*  ihevc_inter_pred_chroma_horz_neon.s
@*
@* @brief
@*  contains function definitions for inter prediction  interpolation.
@* functions are coded using neon  intrinsics and can be compiled using

@* rvct
@*
@* @author
@*  yogeswaran rs / akshaya mukund
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
@*    chroma interprediction filter for horizontal input
@*
@* @par description:
@*    applies a horizontal filter with coefficients pointed to  by 'pi1_coeff'
@*    to the elements pointed by 'pu1_src' and  writes to the location pointed
@*    by 'pu1_dst'  the output is downshifted by 6 and clipped to 8 bits
@*    assumptions : the function is optimized considering the fact width is
@*    multiple of 2,4 or 8. if width is 2, then height  should be multiple of 2.
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

@void ihevc_inter_pred_chroma_horz(uword8 *pu1_src,
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




.globl ihevc_inter_pred_chroma_horz_a9q

.type ihevc_inter_pred_chroma_horz_a9q, %function

ihevc_inter_pred_chroma_horz_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush        {d8 - d15}

    ldr         r4,[sp,#coeff_offset]                 @loads pi1_coeff
    ldr         r7,[sp,#ht_offset]                 @loads ht
    ldr         r10,[sp,#wd_offset]                @loads wd

    vld1.8      {d0},[r4]                   @coeff = vld1_s8(pi1_coeff)
    subs        r14,r7,#0                   @checks for ht == 0
    vabs.s8     d2,d0                       @vabs_s8(coeff)
    mov         r11,#2
    ble         end_loops

    vdup.8      d24,d2[0]                   @coeffabs_0 = vdup_lane_u8(coeffabs, 0)
    sub         r12,r0,#2                   @pu1_src - 2
    vdup.8      d25,d2[1]                   @coeffabs_1 = vdup_lane_u8(coeffabs, 1)
    add         r4,r12,r2                   @pu1_src_tmp2_8 = pu1_src + src_strd
    vdup.8      d26,d2[2]                   @coeffabs_2 = vdup_lane_u8(coeffabs, 2)

    tst         r10,#3                      @checks wd for multiples
    mov         r5,r10,lsl #1

    vdup.8      d27,d2[3]                   @coeffabs_3 = vdup_lane_u8(coeffabs, 3)

    bne         outer_loop_4
    cmp         r10,#12
    beq         skip_16

    cmp         r10,#8
    bge         outer_loop_16
skip_16:
    tst         r7,#3

    sub         r9,r0,#2
    beq         outer_loop_ht_4             @jumps to else condition

    b           outer_loop_8


outer_loop_16:
    mov         r10,r5                      @2wd
    mul         r14,r14,r10

    rsb         r6,r3,#16

    add         r4,r12,r2
    mov         r9,#10
    and         r0, r12, #31
    rsb         r8,r5,r3,lsl #1
    pld         [r12, r2, lsl #1]




    vld1.u32    {q0},[r12],r11              @vector load pu1_src
    pld         [r4, r2, lsl #1]
    vld1.u32    {q1},[r12],r11              @vector load pu1_src

    vld1.u32    {q2},[r12],r11              @vector load pu1_src

    vld1.u32    {q3},[r12],r9               @vector load pu1_src


    vmull.u8    q15,d2,d25                  @mul_res = vmull_u8(src[0_3], coeffabs_3)@
    vld1.u32    {q4},[r4],r11               @vector load pu1_src
    vmlsl.u8    q15,d0,d24                  @mul_res = vmlsl_u8(src[0_2], coeffabs_2)@
    vld1.u32    {q5},[r4],r11               @vector load pu1_src
    vmlal.u8    q15,d4,d26                  @mul_res = vmlsl_u8(src[0_0], coeffabs_0)@
    vld1.u32    {q6},[r4],r11               @vector load pu1_src
    vmlsl.u8    q15,d6,d27                  @mul_res = vmlal_u8(src[0_1], coeffabs_1)@
    vld1.u32    {q7},[r4],r9                @vector load pu1_src
    vmull.u8    q14,d3,d25

    vmlsl.u8    q14,d1,d24


    vmlal.u8    q14,d5,d26

    vmlsl.u8    q14,d7,d27


    cmp         r14,#32
    beq         epilog_end
    sub         r14,#64

inner_loop_16:




@    bgt            l_2

@   pld         [r12, r2, lsl #1]
@   pld         [r4, r2, lsl #1]

    pld         [r12, r2, lsl #2]
    pld         [r4, r2, lsl #2]

    subs        r10,r10,#16

    vmull.u8    q11,d10,d25                 @mul_res = vmull_u8(src[0_3], coeffabs_3)@


    addeq       r12,r12,r8
    addeq       r4,r12,r2
    vmlsl.u8    q11,d8,d24                  @mul_res = vmlsl_u8(src[0_2], coeffabs_2)@



    vqrshrun.s16 d30,q15,#6

    vld1.u32    {q0},[r12],r11              @vector load pu1_src
    vqrshrun.s16 d31,q14,#6


    vld1.u32    {q1},[r12],r11              @vector load pu1_src
    vmlal.u8    q11,d12,d26                 @mul_res = vmlsl_u8(src[0_0], coeffabs_0)@




    vld1.u32    {q2},[r12],r11              @vector load pu1_src
    vmlsl.u8    q11,d14,d27                 @mul_res = vmlal_u8(src[0_1], coeffabs_1)@


    vld1.u32    {q3},[r12],r9               @vector load pu1_src
    vmull.u8    q10,d11,d25                 @mul_res = vmull_u8(src[0_3], coeffabs_3)@

    vmlsl.u8    q10,d9,d24                  @mul_res = vmlsl_u8(src[0_2], coeffabs_2)@

    vst1.16     {q15}, [r1],r3
    vmlal.u8    q10,d13,d26                 @mul_res = vmlsl_u8(src[0_0], coeffabs_0)@

    vld1.u32    {q4},[r4],r11               @vector load pu1_src
    vmlsl.u8    q10,d15,d27                 @mul_res = vmlal_u8(src[0_1], coeffabs_1)@


    vld1.u32    {q5},[r4],r11               @vector load pu1_src
    vmull.u8    q15,d2,d25                  @mul_res = vmull_u8(src[0_3], coeffabs_3)@

    vld1.u32    {q6},[r4],r11               @vector load pu1_src
    vmlsl.u8    q15,d0,d24                  @mul_res = vmlsl_u8(src[0_2], coeffabs_2)@

    vld1.u32    {q7},[r4],r9                @vector load pu1_src
    vmlal.u8    q15,d4,d26                  @mul_res = vmlsl_u8(src[0_0], coeffabs_0)@

    cmp         r10,#0
    vqrshrun.s16 d22,q11,#6
    vqrshrun.s16 d23,q10,#6



    vmlsl.u8    q15,d6,d27                  @mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    moveq       r10,r5                      @2wd
    vmull.u8    q14,d3,d25


    vst1.16     {q11},[r1],r6               @store the result pu1_dst
    vmlsl.u8    q14,d1,d24


    addeq       r1,r1,r8
    vmlal.u8    q14,d5,d26

    subs        r14,r14,#32                 @decrement the ht loop
    vmlsl.u8    q14,d7,d27

@     mov           r0, r7

    bgt         inner_loop_16



    add         r14,r14,#64
    cmp         r14,#32
    beq         epilog_end

epilog:
    vqrshrun.s16 d30,q15,#6
    vqrshrun.s16 d31,q14,#6



    vst1.16     {q15}, [r1],r3
    vmull.u8    q11,d10,d25                 @mul_res = vmull_u8(src[0_3], coeffabs_3)@




    vmlsl.u8    q11,d8,d24                  @mul_res = vmlsl_u8(src[0_2], coeffabs_2)@
    subs        r10,r10,#16                 @decrement the wd loop
    vmlal.u8    q11,d12,d26                 @mul_res = vmlsl_u8(src[0_0], coeffabs_0)@
    addeq       r12,r12,r8
    vmlsl.u8    q11,d14,d27                 @mul_res = vmlal_u8(src[0_1], coeffabs_1)@
    moveq       r10,r5                      @2wd


    addeq       r4,r12,r2
    vmull.u8    q10,d11,d25                 @mul_res = vmull_u8(src[0_3], coeffabs_3)@
    vld1.u32    {q0},[r12],r11              @vector load pu1_src
    vmlsl.u8    q10,d9,d24                  @mul_res = vmlsl_u8(src[0_2], coeffabs_2)@
    vld1.u32    {q1},[r12],r11              @vector load pu1_src
    vmlal.u8    q10,d13,d26                 @mul_res = vmlsl_u8(src[0_0], coeffabs_0)@
    vld1.u32    {q2},[r12],r11              @vector load pu1_src
    vmlsl.u8    q10,d15,d27                 @mul_res = vmlal_u8(src[0_1], coeffabs_1)@
    vld1.u32    {q3},[r12],r9               @vector load pu1_src
    vmull.u8    q15,d2,d25                  @mul_res = vmull_u8(src[0_3], coeffabs_3)@


    vld1.u32    {q4},[r4],r11               @vector load pu1_src
    vmlsl.u8    q15,d0,d24                  @mul_res = vmlsl_u8(src[0_2], coeffabs_2)@
    vld1.u32    {q5},[r4],r11               @vector load pu1_src
    vmlal.u8    q15,d4,d26                  @mul_res = vmlsl_u8(src[0_0], coeffabs_0)@

    vmlsl.u8    q15,d6,d27                  @mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    vld1.u32    {q6},[r4],r11               @vector load pu1_src
    vmull.u8    q14,d3,d25
    vld1.u32    {q7},[r4],r9                @vector load pu1_src
    vmlsl.u8    q14,d1,d24
    vqrshrun.s16 d22,q11,#6
    vqrshrun.s16 d23,q10,#6

    vst1.16     {q11},[r1],r6               @store the result pu1_dst
    vmlal.u8    q14,d5,d26

    vmlsl.u8    q14,d7,d27
    addeq       r1,r1,r8



epilog_end:
    vqrshrun.s16 d30,q15,#6
    vqrshrun.s16 d31,q14,#6


    vmull.u8    q11,d10,d25                 @mul_res = vmull_u8(src[0_3], coeffabs_3)@
    vmlsl.u8    q11,d8,d24                  @mul_res = vmlsl_u8(src[0_2], coeffabs_2)@
    vmlal.u8    q11,d12,d26                 @mul_res = vmlsl_u8(src[0_0], coeffabs_0)@
    vmlsl.u8    q11,d14,d27                 @mul_res = vmlal_u8(src[0_1], coeffabs_1)@


    vmull.u8    q10,d11,d25                 @mul_res = vmull_u8(src[0_3], coeffabs_3)@
    vmlsl.u8    q10,d9,d24                  @mul_res = vmlsl_u8(src[0_2], coeffabs_2)@
    vmlal.u8    q10,d13,d26                 @mul_res = vmlsl_u8(src[0_0], coeffabs_0)@
    vmlsl.u8    q10,d15,d27                 @mul_res = vmlal_u8(src[0_1], coeffabs_1)@
    vqrshrun.s16 d22,q11,#6
    vqrshrun.s16 d23,q10,#6


    vst1.16     {q15}, [r1],r3

    vst1.16     {q11},[r1]                  @store the result pu1_dst



    b           end_loops



















outer_loop_8:


    add         r6,r1,r3                    @pu1_dst + dst_strd
    mov         r7,r5
    add         r4,r12,r2                   @pu1_src + src_strd


inner_loop_8:
    @vld1.u32  {d0,d1},[r12],r11               @vector load pu1_src
    vld1.u32    {d0},[r12],r11              @vector load pu1_src
    vld1.u32    {d1},[r12],r11              @vector load pu1_src
    vld1.u32    {d2},[r12],r11              @vector load pu1_src
    vld1.u32    {d3},[r12],r11              @vector load pu1_src

    @vext.u8   d2,d0,d1,#2                     @vector extract of src[0_2]
    vmull.u8    q4,d1,d25                   @mul_res = vmull_u8(src[0_3], coeffabs_3)@
    vmlsl.u8    q4,d0,d24                   @mul_res = vmlsl_u8(src[0_2], coeffabs_2)@
    @vext.u8   d4,d0,d1,#4                     @vector extract of src[0_4]
    @vext.u8   d6,d0,d1,#6                     @vector extract of src[0_6]
    vmlal.u8    q4,d2,d26                   @mul_res = vmlsl_u8(src[0_0], coeffabs_0)@
    vmlsl.u8    q4,d3,d27                   @mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    vld1.u32    {d4},[r4],r11               @vector load pu1_src
    vld1.u32    {d5},[r4],r11               @vector load pu1_src
    vld1.u32    {d6},[r4],r11               @vector load pu1_src
    vld1.u32    {d7},[r4],r11               @vector load pu1_src
    @vld1.u32  {d12,d13},[r4],r11              @vector load pu1_src + src_strd
    @vext.u8   d14,d12,d13,#2                  @vector extract of src[0_2]
    vmull.u8    q5,d5,d25                   @mul_res = vmull_u8(src[0_3], coeffabs_3)@
    vmlsl.u8    q5,d4,d24                   @mul_res = vmlsl_u8(src[0_2], coeffabs_2)@
    @vext.u8   d16,d12,d13,#4                  @vector extract of src[0_4]
    @vext.u8   d18,d12,d13,#6                  @vector extract of src[0_6]
    vqrshrun.s16 d8,q4,#6                   @right shift and saturating narrow result 1
    vmlal.u8    q5,d6,d26                   @mul_res = vmlsl_u8(src[0_0], coeffabs_0)@
    vmlsl.u8    q5,d7,d27                   @mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    vst1.8      {d8},[r1]!                  @store the result pu1_dst

    vqrshrun.s16 d10,q5,#6                  @right shift and saturating narrow result 2
    subs        r7,r7,#8                    @decrement the wd loop
    vst1.8      {d10},[r6]!                 @store the result pu1_dst
    bgt         inner_loop_8

    sub         r12,r12,r5
    subs        r14,r14,#2                  @decrement the ht loop
    sub         r1,r1,r5
    add         r12,r12,r2,lsl #1
    add         r1,r1,r3,lsl #1
    bgt         outer_loop_8
    b           end_loops

@height if 4 comes
outer_loop_ht_4:

    mov         r7,r5

prologue_ht_4:

inner_loop_ht_4:

    mov         r12,r9
    mov         r4,r1

    sub         r8, r2, #6

    vld1.u32    {d0},[r12],r11              @(1)vector load pu1_src
    vld1.u32    {d1},[r12],r11              @(1)vector load pu1_src
    vld1.u32    {d2},[r12],r11              @(1)vector load pu1_src
    @vld1.u32  {d3},[r12],r2               @(1)vector load pu1_src
    vld1.u32    {d3},[r12],r8               @(1)vector load pu1_src

    @sub       r12, r12, #6                @(1)

    vld1.u32    {d4},[r12],r11              @(2)vector load pu1_src
    vld1.u32    {d5},[r12],r11              @(2)vector load pu1_src
    vld1.u32    {d6},[r12],r11              @(2)vector load pu1_src
    @vld1.u32  {d7},[r12],r2               @(2)vector load pu1_src
    vld1.u32    {d7},[r12],r8               @(2)vector load pu1_src

    @sub       r12, r12, #6                @(2)

    vld1.u32    {d14},[r12],r11             @(3)vector load pu1_src
    vmull.u8    q4,d1,d25                   @(1)mul_res = vmull_u8(src[0_3], coeffabs_3)@

    vld1.u32    {d15},[r12],r11             @(3)vector load pu1_src
    vmlsl.u8    q4,d0,d24                   @(1)mul_res = vmlsl_u8(src[0_2], coeffabs_2)@

    vld1.u32    {d16},[r12],r11             @(3)vector load pu1_src
    vmlal.u8    q4,d2,d26                   @(1)mul_res = vmlsl_u8(src[0_0], coeffabs_0)@

    @vld1.u32  {d17},[r12],r2              @(3)vector load pu1_src
    vld1.u32    {d17},[r12],r8              @(3)vector load pu1_src
    vmlsl.u8    q4,d3,d27                   @(1)mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    @sub       r12, r12, #6                @(3)
    vmull.u8    q5,d5,d25                   @(2)mul_res = vmull_u8(src[0_3], coeffabs_3)@

    vld1.u32    {d18},[r12],r11             @(4)vector load pu1_src
    vmlsl.u8    q5,d4,d24                   @(2)mul_res = vmlsl_u8(src[0_2], coeffabs_2)@

    vld1.u32    {d19},[r12],r11             @(4)vector load pu1_src
    vmlal.u8    q5,d6,d26                   @(2)mul_res = vmlsl_u8(src[0_0], coeffabs_0)@

    vld1.u32    {d20},[r12],r11             @(4)vector load pu1_src
    vmlsl.u8    q5,d7,d27                   @(2)mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    vld1.u32    {d21},[r12],r2              @(4)vector load pu1_src
    vqrshrun.s16 d8,q4,#6                   @(1)right shift and saturating narrow result 1

    add         r9,r9,#8                    @(core loop)

    subs        r7,r7,#8                    @(prologue)decrement the wd loop
    beq         epilogue

core_loop:
    mov         r12,r9

    vld1.u32    {d0},[r12],r11              @(1_1)vector load pu1_src
    vmull.u8    q6,d15,d25                  @(3)mul_res = vmull_u8(src[0_3], coeffabs_3)@

    vld1.u32    {d1},[r12],r11              @(1_1)vector load pu1_src
    vmlsl.u8    q6,d14,d24                  @(3)mul_res = vmlsl_u8(src[0_2], coeffabs_2)@

    vld1.u32    {d2},[r12],r11              @(1_1)vector load pu1_src
    vmlal.u8    q6,d16,d26                  @(3)mul_res = vmlsl_u8(src[0_0], coeffabs_0)@

    @vld1.u32  {d3},[r12],r2               @(1_1)vector load pu1_src
    vld1.u32    {d3},[r12],r8               @(1_1)vector load pu1_src
    vmlsl.u8    q6,d17,d27                  @(3)mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    @sub       r12, r12, #6                @(1_1)

    vst1.8      {d8},[r4],r3                @(1)store the result pu1_dst
    vqrshrun.s16 d10,q5,#6                  @(2)right shift and saturating narrow result 2

    vld1.u32    {d4},[r12],r11              @(2_1)vector load pu1_src
    vmull.u8    q11,d19,d25                 @(4)mul_res = vmull_u8(src[0_3], coeffabs_3)@

    vld1.u32    {d5},[r12],r11              @(2_1)vector load pu1_src
    vmlsl.u8    q11,d18,d24                 @(4)mul_res = vmlsl_u8(src[0_2], coeffabs_2)@

    vld1.u32    {d6},[r12],r11              @(2_1)vector load pu1_src
    vmlal.u8    q11,d20,d26                 @(4)mul_res = vmlsl_u8(src[0_0], coeffabs_0)@

    @vld1.u32  {d7},[r12],r2               @(2_1)vector load pu1_src
    vld1.u32    {d7},[r12],r8               @(2_1)vector load pu1_src
    vmlsl.u8    q11,d21,d27                 @(4)mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    @sub       r12, r12, #6                @(2_1)

    vst1.8      {d10},[r4],r3               @(2)store the result pu1_dst
    vqrshrun.s16 d12,q6,#6                  @(3)right shift and saturating narrow result 1

    vld1.u32    {d14},[r12],r11             @(3_1)vector load pu1_src
    vmull.u8    q4,d1,d25                   @(1_1)mul_res = vmull_u8(src[0_3], coeffabs_3)@

    vld1.u32    {d15},[r12],r11             @(3_1)vector load pu1_src
    vmlsl.u8    q4,d0,d24                   @(1_1)mul_res = vmlsl_u8(src[0_2], coeffabs_2)@

    vld1.u32    {d16},[r12],r11             @(3_1)vector load pu1_src
    vmlal.u8    q4,d2,d26                   @(1_1)mul_res = vmlsl_u8(src[0_0], coeffabs_0)@

    @vld1.u32  {d17},[r12],r2              @(3_1)vector load pu1_src
    vld1.u32    {d17},[r12],r8              @(3_1)vector load pu1_src
    vmlsl.u8    q4,d3,d27                   @(1_1)mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    @sub       r12, r12, #6                @(3_1)

    vst1.8      {d12},[r4],r3               @(3)store the result pu1_dst
    vqrshrun.s16 d22,q11,#6                 @(4)right shift and saturating narrow result 2

    add         r9,r9,#8                    @(core loop)

    vmull.u8    q5,d5,d25                   @(2_1)mul_res = vmull_u8(src[0_3], coeffabs_3)@
    vld1.u32    {d18},[r12],r11             @(4_1)vector load pu1_src

    vld1.u32    {d19},[r12],r11             @(4_1)vector load pu1_src
    vmlsl.u8    q5,d4,d24                   @(2_1)mul_res = vmlsl_u8(src[0_2], coeffabs_2)@

    vld1.u32    {d20},[r12],r11             @(4_1)vector load pu1_src
    vmlal.u8    q5,d6,d26                   @(2_1)mul_res = vmlsl_u8(src[0_0], coeffabs_0)@

    vld1.u32    {d21},[r12],r2              @(4_1)vector load pu1_src
    vmlsl.u8    q5,d7,d27                   @(2_1)mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    add         r1,r1,#8                    @(core loop)

    subs        r7,r7,#8                    @(core loop)

    vst1.8      {d22}, [r4], r3             @(4)store the result pu1_dst
    vqrshrun.s16 d8,q4,#6                   @(1_1)right shift and saturating narrow result 1

    mov         r4, r1                      @(core loop)

    bgt         core_loop                   @loopback

epilogue:
    vmull.u8    q6,d15,d25                  @(3)mul_res = vmull_u8(src[0_3], coeffabs_3)@

    vmlsl.u8    q6,d14,d24                  @(3)mul_res = vmlsl_u8(src[0_2], coeffabs_2)@

    vmlal.u8    q6,d16,d26                  @(3)mul_res = vmlsl_u8(src[0_0], coeffabs_0)@

    vmlsl.u8    q6,d17,d27                  @(3)mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    vst1.8      {d8},[r4],r3                @(1)store the result pu1_dst
    vqrshrun.s16 d10,q5,#6                  @(2)right shift and saturating narrow result 2

    vmull.u8    q11,d19,d25                 @(4)mul_res = vmull_u8(src[0_3], coeffabs_3)@
    vmlsl.u8    q11,d18,d24                 @(4)mul_res = vmlsl_u8(src[0_2], coeffabs_2)@

    vmlal.u8    q11,d20,d26                 @(4)mul_res = vmlsl_u8(src[0_0], coeffabs_0)@

    vmlsl.u8    q11,d21,d27                 @(4)mul_res = vmlal_u8(src[0_1], coeffabs_1)@

    vst1.8      {d10},[r4],r3               @(2)store the result pu1_dst
    vqrshrun.s16 d12,q6,#6                  @(3)right shift and saturating narrow result 1

    vst1.8      {d12},[r4],r3               @(3)store the result pu1_dst

    add         r1,r1,#8                    @(core loop)

    vqrshrun.s16 d22,q11,#6                 @(4)right shift and saturating narrow result 2


    vst1.8      {d22}, [r4], r3             @(4)store the result pu1_dst

    sub         r9,r9,r5
    subs        r14,r14,#4                  @decrement the ht loop
    sub         r1,r1,r5
    add         r9,r9,r2,lsl #2
    add         r1,r1,r3,lsl #2
    bgt         outer_loop_ht_4
    b           end_loops

outer_loop_4:
    add         r6,r1,r3                    @pu1_dst + dst_strd
    mov         r7,r5
    add         r4,r12,r2                   @pu1_src + src_strd

inner_loop_4:
    @vld1.u32  {d0,d1},[r12]                   @vector load pu1_src

    vld1.u32    {d0},[r12],r11              @vector load pu1_src
    vld1.u32    {d1},[r12],r11              @vector load pu1_src
    vld1.u32    {d2},[r12],r11              @vector load pu1_src
    vld1.u32    {d3},[r12]                  @vector load pu1_src

    sub         r12,r12,#2                  @increment the input pointer
    vld1.u32    {d4},[r4],r11               @vector load pu1_src
    vld1.u32    {d5},[r4],r11               @vector load pu1_src
    vld1.u32    {d6},[r4],r11               @vector load pu1_src
    vld1.u32    {d7},[r4]                   @vector load pu1_src
    @vext.u8   d2,d0,d1,#2                     @vector extract of src[0_2]
    @vext.u8   d4,d0,d1,#4                     @vector extract of src[0_4]
    @vld1.u32  {d12,d13},[r4]                  @vector load pu1_src + src_strd
    @vext.u8   d6,d0,d1,#6                     @vector extract of src[0_6]

    sub         r4,r4,#2                    @increment the input pointer
    @vext.u8   d14,d12,d13,#2                  @vector extract of src[0_2]
    @vext.u8   d16,d12,d13,#4                  @vector extract of src[0_4]
    @vext.u8   d18,d12,d13,#6                  @vector extract of src[0_6]

    vzip.32     d0,d4                       @vector zip the i iteration and ii interation in single register
    vzip.32     d1,d5
    vzip.32     d2,d6
    vzip.32     d3,d7

    vmull.u8    q4,d1,d25                   @arithmetic operations for ii iteration in the same time
    vmlsl.u8    q4,d0,d24
    vmlal.u8    q4,d2,d26
    vmlsl.u8    q4,d3,d27

    vqrshrun.s16 d8,q4,#6                   @narrow right shift and saturating the result
    vst1.32     {d8[0]},[r1]!               @store the i iteration result which is in upper part of the register
    subs        r7,r7,#4                    @decrement the wd by 4

    vst1.32     {d8[1]},[r6]!               @store the ii iteration result which is in lower part of the register

    bgt         inner_loop_4

    sub         r12,r12,r5
    subs        r14,r14,#2                  @decrement the ht by 2
    sub         r1,r1,r5
    add         r12,r12,r2,lsl #1
    add         r1,r1,r3,lsl #1
    bgt         outer_loop_4

end_loops:

    vpop         {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp








