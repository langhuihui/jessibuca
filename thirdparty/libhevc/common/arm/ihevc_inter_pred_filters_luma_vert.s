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
@******************************************************************************
@* @file
@*  ihevc_inter_pred_filters_luma_vert.s
@*
@* @brief
@*  contains function definitions for inter prediction  interpolation.
@* functions are coded using neon  intrinsics and can be compiled using

@* rvct
@*
@* @author
@*  parthiban v
@*
@* @par list of functions:
@*
@*  - ihevc_inter_pred_luma_vert()
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/

@/* all the functions here are replicated from ihevc_inter_pred_filters.c and modified to */
@/* include reconstruction */



@/**
@*******************************************************************************
@*
@* @brief
@*     interprediction luma filter for vertical input
@*
@* @par description:
@*    applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
@*    the elements pointed by 'pu1_src' and  writes to the location pointed by
@*    'pu1_dst'  the output is downshifted by 6 and clipped to 8 bits
@*    assumptions : the function is optimized considering the fact width is
@*    multiple of 4 or 8. and height as multiple of 2.
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

@void ihevc_inter_pred_luma_vert (
@                            uword8 *pu1_src,
@                            uword8 *pu1_dst,
@                            word32 src_strd,
@                            word32 dst_strd,
@                            word8 *pi1_coeff,
@                            word32 ht,
@                            word32 wd   )

@**************variables vs registers*****************************************
@   r0 => *pu1_src
@   r1 => *pu1_dst
@   r2 =>  src_strd
@   r6 =>  dst_strd
@   r12 => *pi1_coeff
@   r5 =>  ht
@   r3 =>  wd

.equ    coeff_offset,   104
.equ    ht_offset,      108
.equ    wd_offset,      112

.text
.align 4
.syntax unified



.globl ihevc_inter_pred_luma_vert_a9q

.type ihevc_inter_pred_luma_vert_a9q, %function

ihevc_inter_pred_luma_vert_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush        {d8 - d15}

    ldr         r12,[sp,#coeff_offset]                @load pi1_coeff
    mov         r6,r3
    ldr         r5,[sp,#wd_offset]                 @load wd
    vld1.u8     {d0},[r12]                  @coeff = vld1_s8(pi1_coeff)
    sub         r12,r2,r2,lsl #2            @src_ctrd & pi1_coeff
    vabs.s8     d0,d0                       @vabs_s8(coeff)
    add         r0,r0,r12                   @r0->pu1_src    r12->pi1_coeff
    ldr         r3,[sp,#ht_offset]                 @load ht
    subs        r7,r3,#0                    @r3->ht
    @ble        end_loops           @end loop jump
    vdup.u8     d22,d0[0]                   @coeffabs_0 = vdup_lane_u8(coeffabs, 0)@
    cmp         r5,#8
    vdup.u8     d23,d0[1]                   @coeffabs_1 = vdup_lane_u8(coeffabs, 1)@
    vdup.u8     d24,d0[2]                   @coeffabs_2 = vdup_lane_u8(coeffabs, 2)@
    vdup.u8     d25,d0[3]                   @coeffabs_3 = vdup_lane_u8(coeffabs, 3)@
    vdup.u8     d26,d0[4]                   @coeffabs_4 = vdup_lane_u8(coeffabs, 4)@
    vdup.u8     d27,d0[5]                   @coeffabs_5 = vdup_lane_u8(coeffabs, 5)@
    vdup.u8     d28,d0[6]                   @coeffabs_6 = vdup_lane_u8(coeffabs, 6)@
    vdup.u8     d29,d0[7]                   @coeffabs_7 = vdup_lane_u8(coeffabs, 7)@
    blt         core_loop_wd_4              @core loop wd 4 jump
    str         r0, [sp, #-4]!
    str         r1, [sp, #-4]!

    bic         r4,r5,#7                    @r5 ->wd
    rsb         r9,r4,r6,lsl #2             @r6->dst_strd   r5  ->wd
    rsb         r8,r4,r2,lsl #2             @r2->src_strd
    mov         r3, r5, lsr #3              @divide by 8
    mul         r7, r3                      @multiply height by width
    sub         r7, #4                      @subtract by one for epilog

prolog:

    and         r10, r0, #31
    add         r3,r0,r2                    @pu1_src_tmp += src_strd@
    vld1.u8     {d1},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vld1.u8     {d0},[r0]!                  @src_tmp1 = vld1_u8(pu1_src_tmp)@
    subs        r4,r4,#8
    vld1.u8     {d2},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmull.u8    q4,d1,d23                   @mul_res1 = vmull_u8(src_tmp2, coeffabs_1)@
    vld1.u8     {d3},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q4,d0,d22                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp1, coeffabs_0)@
    vld1.u8     {d4},[r3],r2                @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q4,d2,d24                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp3, coeffabs_2)@
    vld1.u8     {d5},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q4,d3,d25                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_3)@
    vld1.u8     {d6},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q4,d4,d26                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_4)@
    vld1.u8     {d7},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q4,d5,d27                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp2, coeffabs_5)@
    vld1.u8     {d16},[r3],r2               @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q4,d6,d28                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_6)@
    vld1.u8     {d17},[r3],r2               @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q4,d7,d29                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp4, coeffabs_7)@


    vld1.u8     {d18},[r3],r2               @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmull.u8    q5,d2,d23                   @mul_res2 = vmull_u8(src_tmp3, coeffabs_1)@

    addle       r0,r0,r8
    vmlsl.u8    q5,d1,d22                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp2, coeffabs_0)@

    bicle       r4,r5,#7                    @r5 ->wd
    vmlsl.u8    q5,d3,d24                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp4, coeffabs_2)@

    pld         [r3]
    vmlal.u8    q5,d4,d25                   @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_3)@
    pld         [r3, r2]
    vmlal.u8    q5,d5,d26                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_4)@
    pld         [r3, r2, lsl #1]
    vmlsl.u8    q5,d6,d27                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp3, coeffabs_5)@

    add         r3, r3, r2
    vmlal.u8    q5,d7,d28                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_6)@

    pld         [r3, r2, lsl #1]
    vmlsl.u8    q5,d16,d29                  @mul_res2 = vmlsl_u8(mul_res2, src_tmp1, coeffabs_7)@

    add         r3,r0,r2                    @pu1_src_tmp += src_strd@
    vqrshrun.s16 d8,q4,#6                   @sto_res = vqmovun_s16(sto_res_tmp)@

    vld1.u8     {d1},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmull.u8    q6,d3,d23
    vld1.u8     {d0},[r0]!                  @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q6,d2,d22
    vld1.u8     {d2},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q6,d4,d24
    vmlal.u8    q6,d5,d25
    vmlal.u8    q6,d6,d26
    vmlsl.u8    q6,d7,d27
    vmlal.u8    q6,d16,d28
    vmlsl.u8    q6,d17,d29
    add         r14,r1,r6
    vst1.8      {d8},[r1]!                  @vst1_u8(pu1_dst,sto_res)@
    vqrshrun.s16 d10,q5,#6                  @sto_res = vqmovun_s16(sto_res_tmp)@
    addle       r1,r1,r9

    vmull.u8    q7,d4,d23
    subs        r7,r7,#4
    vmlsl.u8    q7,d3,d22
    vmlsl.u8    q7,d5,d24
    vmlal.u8    q7,d6,d25
    vld1.u8     {d3},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q7,d7,d26
    vld1.u8     {d4},[r3],r2                @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q7,d16,d27
    vld1.u8     {d5},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q7,d17,d28
    vld1.u8     {d6},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q7,d18,d29
    vld1.u8     {d7},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@

    vst1.8      {d10},[r14],r6              @vst1_u8(pu1_dst_tmp,sto_res)@
    vqrshrun.s16 d12,q6,#6


    blt         epilog_end                  @jumps to epilog_end
    beq         epilog                      @jumps to epilog

kernel_8:

    subs        r4,r4,#8
    vmull.u8    q4,d1,d23                   @mul_res1 = vmull_u8(src_tmp2, coeffabs_1)@

    addle       r0,r0,r8
    vmlsl.u8    q4,d0,d22                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp1, coeffabs_0)@

    bicle       r4,r5,#7                    @r5 ->wd
    vmlsl.u8    q4,d2,d24                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp3, coeffabs_2)@

    vld1.u8     {d16},[r3],r2               @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q4,d3,d25                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_3)@

    vld1.u8     {d17},[r3],r2               @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q4,d4,d26                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_4)@

    vld1.u8     {d18},[r3],r2               @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q4,d5,d27                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp2, coeffabs_5)@

    vmlal.u8    q4,d6,d28                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_6)@

    vmlsl.u8    q4,d7,d29                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp4, coeffabs_7)@
    vst1.8      {d12},[r14],r6

@   and         r11, r0, #31
    vqrshrun.s16 d14,q7,#6

    add         r3,r0,r2                    @pu1_src_tmp += src_strd@
    vmull.u8    q5,d2,d23                   @mul_res2 = vmull_u8(src_tmp3, coeffabs_1)@

    vld1.u8     {d0},[r0]!                  @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q5,d1,d22                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp2, coeffabs_0)@

    vmlsl.u8    q5,d3,d24                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp4, coeffabs_2)@

    vld1.u8     {d1},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q5,d4,d25                   @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_3)@

    vst1.8      {d14},[r14],r6
    vmlal.u8    q5,d5,d26                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_4)@

    add         r14,r1,#0
    vmlsl.u8    q5,d6,d27                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp3, coeffabs_5)@

    add         r1, r1, #8
    vmlal.u8    q5,d7,d28                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_6)@

    vmlsl.u8    q5,d16,d29                  @mul_res2 = vmlsl_u8(mul_res2, src_tmp1, coeffabs_7)@

    addle       r1,r1,r9
    vqrshrun.s16 d8,q4,#6                   @sto_res = vqmovun_s16(sto_res_tmp)@

@   cmp         r11, r10
    vmull.u8    q6,d3,d23

    add         r10, r3, r2, lsl #3         @ 10*strd - 8+2
    vmlsl.u8    q6,d2,d22

    add         r10, r10, r2                @ 11*strd
    vmlsl.u8    q6,d4,d24

    vld1.u8     {d2},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q6,d5,d25

    vmlal.u8    q6,d6,d26
    vst1.8      {d8},[r14],r6               @vst1_u8(pu1_dst,sto_res)@

    pld         [r10]                       @11+ 0
    vmlsl.u8    q6,d7,d27

    pld         [r10, r2]                   @11+ 1*strd
    vmlal.u8    q6,d16,d28

    pld         [r10, r2, lsl #1]           @11+ 2*strd
    vmlsl.u8    q6,d17,d29

    add         r10, r10, r2                @12*strd
    vqrshrun.s16 d10,q5,#6                  @sto_res = vqmovun_s16(sto_res_tmp)@

    pld         [r10, r2, lsl #1]           @11+ 3*strd
    vmull.u8    q7,d4,d23

@   mov         r10, r11
    vmlsl.u8    q7,d3,d22

    subs        r7,r7,#4
    vmlsl.u8    q7,d5,d24

    vmlal.u8    q7,d6,d25
    vld1.u8     {d3},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q7,d7,d26
    vld1.u8     {d4},[r3],r2                @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q7,d16,d27
    vld1.u8     {d5},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q7,d17,d28
    vld1.u8     {d6},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q7,d18,d29
    vld1.u8     {d7},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@

    vqrshrun.s16 d12,q6,#6
    vst1.8      {d10},[r14],r6              @vst1_u8(pu1_dst_tmp,sto_res)@



    bgt         kernel_8                    @jumps to kernel_8

epilog:

    vmull.u8    q4,d1,d23                   @mul_res1 = vmull_u8(src_tmp2, coeffabs_1)@
    vmlsl.u8    q4,d0,d22                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp1, coeffabs_0)@
    vmlsl.u8    q4,d2,d24                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp3, coeffabs_2)@
    vmlal.u8    q4,d3,d25                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_3)@
    vmlal.u8    q4,d4,d26                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_4)@
    vmlsl.u8    q4,d5,d27                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp2, coeffabs_5)@
    vmlal.u8    q4,d6,d28                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_6)@
    vmlsl.u8    q4,d7,d29                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp4, coeffabs_7)@
    vst1.8      {d12},[r14],r6

    vqrshrun.s16 d14,q7,#6

    vld1.u8     {d16},[r3],r2               @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmull.u8    q5,d2,d23                   @mul_res2 = vmull_u8(src_tmp3, coeffabs_1)@
    vmlsl.u8    q5,d1,d22                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp2, coeffabs_0)@
    vmlsl.u8    q5,d3,d24                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp4, coeffabs_2)@
    vmlal.u8    q5,d4,d25                   @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_3)@
    vmlal.u8    q5,d5,d26                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_4)@
    vmlsl.u8    q5,d6,d27                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp3, coeffabs_5)@
    vmlal.u8    q5,d7,d28                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_6)@
    vmlsl.u8    q5,d16,d29                  @mul_res2 = vmlsl_u8(mul_res2, src_tmp1, coeffabs_7)@
    vst1.8      {d14},[r14],r6

    vqrshrun.s16 d8,q4,#6                   @sto_res = vqmovun_s16(sto_res_tmp)@

    vld1.u8     {d17},[r3],r2               @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmull.u8    q6,d3,d23
    vmlsl.u8    q6,d2,d22
    vmlsl.u8    q6,d4,d24
    vmlal.u8    q6,d5,d25
    vmlal.u8    q6,d6,d26
    vmlsl.u8    q6,d7,d27
    vmlal.u8    q6,d16,d28
    vmlsl.u8    q6,d17,d29
    add         r14,r1,r6
    vst1.8      {d8},[r1]!                  @vst1_u8(pu1_dst,sto_res)@
    vqrshrun.s16 d10,q5,#6                  @sto_res = vqmovun_s16(sto_res_tmp)@

    vld1.u8     {d18},[r3],r2               @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmull.u8    q7,d4,d23
    vmlsl.u8    q7,d3,d22
    vmlsl.u8    q7,d5,d24
    vmlal.u8    q7,d6,d25
    vmlal.u8    q7,d7,d26
    vmlsl.u8    q7,d16,d27
    vmlal.u8    q7,d17,d28
    vmlsl.u8    q7,d18,d29

    vst1.8      {d10},[r14],r6              @vst1_u8(pu1_dst_tmp,sto_res)@
    vqrshrun.s16 d12,q6,#6

epilog_end:
    vst1.8      {d12},[r14],r6
    vqrshrun.s16 d14,q7,#6

    vst1.8      {d14},[r14],r6


end_loops:
    tst         r5,#7
    ldr         r1, [sp], #4
    ldr         r0, [sp], #4

    beq         end1

    mov         r5, #4
    add         r0, r0, #8
    add         r1, r1, #8
    mov         r7, #16
    @

core_loop_wd_4:
    rsb         r9,r5,r6,lsl #2             @r6->dst_strd   r5  ->wd
    rsb         r8,r5,r2,lsl #2             @r2->src_strd
    vmov.i8     d4,#0

outer_loop_wd_4:
    subs        r12,r5,#0
    ble         end_inner_loop_wd_4         @outer loop jump

inner_loop_wd_4:
    add         r3,r0,r2
    vld1.u32    {d4[1]},[r3],r2             @src_tmp1 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp1, 1)@
    subs        r12,r12,#4
    vdup.u32    d5,d4[1]                    @src_tmp2 = vdup_lane_u32(src_tmp1, 1)@
    vld1.u32    {d5[1]},[r3],r2             @src_tmp2 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp2, 1)@
    vld1.u32    {d4[0]},[r0]                @src_tmp1 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp1, 0)@
    vmull.u8    q0,d5,d23                   @mul_res1 = vmull_u8(vreinterpret_u8_u32(src_tmp2), coeffabs_1)@

    vdup.u32    d6,d5[1]                    @src_tmp3 = vdup_lane_u32(src_tmp2, 1)@
    add         r0,r0,#4
    vld1.u32    {d6[1]},[r3],r2             @src_tmp3 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp3, 1)@
    vmlsl.u8    q0,d4,d22                   @mul_res1 = vmlsl_u8(mul_res1, vreinterpret_u8_u32(src_tmp1), coeffabs_0)@

    vdup.u32    d7,d6[1]                    @src_tmp4 = vdup_lane_u32(src_tmp3, 1)@
    vld1.u32    {d7[1]},[r3],r2             @src_tmp4 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp4, 1)@
    vmlsl.u8    q0,d6,d24                   @mul_res1 = vmlsl_u8(mul_res1, vreinterpret_u8_u32(src_tmp3), coeffabs_2)@

    vmull.u8    q4,d7,d23
    vdup.u32    d4,d7[1]                    @src_tmp1 = vdup_lane_u32(src_tmp4, 1)@
    vmull.u8    q1,d7,d25                   @mul_res2 = vmull_u8(vreinterpret_u8_u32(src_tmp4), coeffabs_3)@
    vld1.u32    {d4[1]},[r3],r2             @src_tmp1 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp1, 1)@
    vmlsl.u8    q4,d6,d22
    vmlal.u8    q0,d4,d26                   @mul_res1 = vmlal_u8(mul_res1, vreinterpret_u8_u32(src_tmp1), coeffabs_4)@

    vdup.u32    d5,d4[1]                    @src_tmp2 = vdup_lane_u32(src_tmp1, 1)@
    vmlsl.u8    q4,d4,d24
    vld1.u32    {d5[1]},[r3],r2             @src_tmp2 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp2, 1)@
    vmlsl.u8    q1,d5,d27                   @mul_res2 = vmlsl_u8(mul_res2, vreinterpret_u8_u32(src_tmp2), coeffabs_5)@

    vdup.u32    d6,d5[1]                    @src_tmp3 = vdup_lane_u32(src_tmp2, 1)@
    vmlal.u8    q4,d5,d25
    vld1.u32    {d6[1]},[r3],r2             @src_tmp3 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp3, 1)@
    vmlal.u8    q0,d6,d28                   @mul_res1 = vmlal_u8(mul_res1, vreinterpret_u8_u32(src_tmp3), coeffabs_6)@

    vdup.u32    d7,d6[1]                    @src_tmp4 = vdup_lane_u32(src_tmp3, 1)@
    vmlal.u8    q4,d6,d26
    vld1.u32    {d7[1]},[r3],r2             @src_tmp4 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp4, 1)@
    vmlsl.u8    q1,d7,d29                   @mul_res2 = vmlsl_u8(mul_res2, vreinterpret_u8_u32(src_tmp4), coeffabs_7)@

    vdup.u32    d4,d7[1]
    vadd.i16    q0,q0,q1                    @mul_res1 = vaddq_u16(mul_res1, mul_res2)@

    vmlsl.u8    q4,d7,d27
    vld1.u32    {d4[1]},[r3],r2
    vmlal.u8    q4,d4,d28
    vdup.u32    d5,d4[1]
    vqrshrun.s16 d0,q0,#6                   @sto_res = vqmovun_s16(sto_res_tmp)@

    vld1.u32    {d5[1]},[r3]
    add         r3,r1,r6
    vst1.32     {d0[0]},[r1]                @vst1_lane_u32((uint32_t *)pu1_dst, vreinterpret_u32_u8(sto_res), 0)@

    vmlsl.u8    q4,d5,d29
    vst1.32     {d0[1]},[r3],r6             @vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(sto_res), 1)@
    vqrshrun.s16 d8,q4,#6

    vst1.32     {d8[0]},[r3],r6
    add         r1,r1,#4
    vst1.32     {d8[1]},[r3]
    bgt         inner_loop_wd_4

end_inner_loop_wd_4:
    subs        r7,r7,#4
    add         r1,r1,r9
    add         r0,r0,r8
    bgt         outer_loop_wd_4

end1:
    vpop         {d8 - d15}
    ldmfd       sp!, {r4-r12, r15}          @reload the registers from sp



@/**
@*******************************************************************************
@*
@* @brief
@*     interprediction luma filter for vertical 16bit output
@*
@* @par description:
@*    applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
@*    the elements pointed by 'pu1_src' and  writes to the location pointed by
@*    'pu1_dst'  no downshifting or clipping is done and the output is  used as
@*    an input for weighted prediction   assumptions : the function is optimized
@*    considering the fact width is  multiple of 4 or 8. and height as multiple
@*    of 2.
@*
@* @param[in] pu1_src
@*  uword8 pointer to the source
@*
@* @param[out] pi2_dst
@*  word16 pointer to the destination
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

@void ihevc_inter_pred_luma_vert_w16out(uword8 *pu1_src,
@                                    word16 *pi2_dst,
@                                    word32 src_strd,
@                                    word32 dst_strd,
@                                    word8 *pi1_coeff,
@                                    word32 ht,
@                                    word32 wd   )

@**************variables vs registers*****************************************
@   r0 => *pu1_src
@   r1 => *pu1_dst
@   r2 =>  src_strd
@   r6 =>  dst_strd
@   r12 => *pi1_coeff
@   r5 =>  ht
@   r3 =>  wd



.globl ihevc_inter_pred_luma_vert_w16out_a9q

.type ihevc_inter_pred_luma_vert_w16out_a9q, %function

ihevc_inter_pred_luma_vert_w16out_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush        {d8 - d15}

    ldr         r12,[sp,#coeff_offset]                @load pi1_coeff
    mov         r6,r3
    ldr         r5,[sp,#wd_offset]                 @load wd
    vld1.u8     {d0},[r12]                  @coeff = vld1_s8(pi1_coeff)
    sub         r12,r2,r2,lsl #2            @src_ctrd & pi1_coeff
    vabs.s8     d0,d0                       @vabs_s8(coeff)
    add         r0,r0,r12                   @r0->pu1_src    r12->pi1_coeff
    ldr         r3,[sp,#ht_offset]                 @load ht
    subs        r7,r3,#0                    @r3->ht
    @ble        end_loops_16out         @end loop jump
    vdup.u8     d22,d0[0]                   @coeffabs_0 = vdup_lane_u8(coeffabs, 0)@
    cmp         r5,#8
    vdup.u8     d23,d0[1]                   @coeffabs_1 = vdup_lane_u8(coeffabs, 1)@
    vdup.u8     d24,d0[2]                   @coeffabs_2 = vdup_lane_u8(coeffabs, 2)@
    vdup.u8     d25,d0[3]                   @coeffabs_3 = vdup_lane_u8(coeffabs, 3)@
    vdup.u8     d26,d0[4]                   @coeffabs_4 = vdup_lane_u8(coeffabs, 4)@
    vdup.u8     d27,d0[5]                   @coeffabs_5 = vdup_lane_u8(coeffabs, 5)@
    vdup.u8     d28,d0[6]                   @coeffabs_6 = vdup_lane_u8(coeffabs, 6)@
    vdup.u8     d29,d0[7]                   @coeffabs_7 = vdup_lane_u8(coeffabs, 7)@
    blt         core_loop_wd_4_16out        @core loop wd 4 jump
    str         r0, [sp, #-4]!
    str         r1, [sp, #-4]!

    bic         r4,r5,#7                    @r5 ->wd
    rsb         r9,r4,r6,lsl #2             @r6->dst_strd   r5  ->wd
    rsb         r8,r4,r2,lsl #2             @r2->src_strd
    mov         r6, r6, lsl #1
    mov         r3, r5, lsr #3              @divide by 8
    mul         r7, r3                      @multiply height by width
    sub         r7, #4                      @subtract by one for epilog

prolog_16out:

    and         r10, r0, #31
    add         r3,r0,r2                    @pu1_src_tmp += src_strd@

    vld1.u8     {d1},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vld1.u8     {d0},[r0]!                  @src_tmp1 = vld1_u8(pu1_src_tmp)@
    subs        r4,r4,#8
    vld1.u8     {d2},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmull.u8    q4,d1,d23                   @mul_res1 = vmull_u8(src_tmp2, coeffabs_1)@
    vld1.u8     {d3},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q4,d0,d22                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp1, coeffabs_0)@
    vld1.u8     {d4},[r3],r2                @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q4,d2,d24                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp3, coeffabs_2)@
    vld1.u8     {d5},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q4,d3,d25                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_3)@
    vld1.u8     {d6},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q4,d4,d26                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_4)@
    vld1.u8     {d7},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q4,d5,d27                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp2, coeffabs_5)@
    vld1.u8     {d16},[r3],r2               @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q4,d6,d28                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_6)@
    vld1.u8     {d17},[r3],r2               @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q4,d7,d29                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp4, coeffabs_7)@


    addle       r0,r0,r8
    vmull.u8    q5,d2,d23                   @mul_res2 = vmull_u8(src_tmp3, coeffabs_1)@

    bicle       r4,r5,#7                    @r5 ->wd
    vmlsl.u8    q5,d1,d22                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp2, coeffabs_0)@

    vld1.u8     {d18},[r3],r2               @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q5,d3,d24                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp4, coeffabs_2)@

    pld         [r3]
    vmlal.u8    q5,d4,d25                   @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_3)@
    pld         [r3, r2]
    vmlal.u8    q5,d5,d26                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_4)@
    pld         [r3, r2, lsl #1]
    vmlsl.u8    q5,d6,d27                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp3, coeffabs_5)@
    add         r3, r3, r2
    vmlal.u8    q5,d7,d28                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_6)@
    pld         [r3, r2, lsl #1]
    vmlsl.u8    q5,d16,d29                  @mul_res2 = vmlsl_u8(mul_res2, src_tmp1, coeffabs_7)@

    add         r3,r0,r2                    @pu1_src_tmp += src_strd@
    vmull.u8    q6,d3,d23
    vld1.u8     {d1},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q6,d2,d22
    vld1.u8     {d0},[r0]!                  @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q6,d4,d24
    vld1.u8     {d2},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q6,d5,d25
    vmlal.u8    q6,d6,d26
    vmlsl.u8    q6,d7,d27
    vmlal.u8    q6,d16,d28
    vmlsl.u8    q6,d17,d29
    add         r14,r1,r6
    vst1.8      {d8, d9},[r1]!              @vst1_u8(pu1_dst,sto_res)@
    @vqrshrun.s16 d10,q5,#6         @sto_res = vqmovun_s16(sto_res_tmp)@
    addle       r1,r1,r9,lsl #1

    vmull.u8    q7,d4,d23
    subs        r7,r7,#4
    vmlsl.u8    q7,d3,d22
    vmlsl.u8    q7,d5,d24
    vmlal.u8    q7,d6,d25
    vld1.u8     {d3},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q7,d7,d26
    vld1.u8     {d4},[r3],r2                @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q7,d16,d27
    vld1.u8     {d5},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q7,d17,d28
    vld1.u8     {d6},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q7,d18,d29
    vld1.u8     {d7},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@

    vst1.8      {d10, d11},[r14],r6         @vst1_u8(pu1_dst_tmp,sto_res)@
    @vqrshrun.s16 d12,q6,#6


    blt         epilog_end_16out
    beq         epilog_16out                @jumps to epilog

kernel_8_16out:

    subs        r4,r4,#8
    vmull.u8    q4,d1,d23                   @mul_res1 = vmull_u8(src_tmp2, coeffabs_1)@

    addle       r0,r0,r8
    vmlsl.u8    q4,d0,d22                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp1, coeffabs_0)@

    vld1.u8     {d16},[r3],r2               @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q4,d2,d24                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp3, coeffabs_2)@

    vld1.u8     {d17},[r3],r2               @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q4,d3,d25                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_3)@

    bicle       r4,r5,#7                    @r5 ->wd
    vmlal.u8    q4,d4,d26                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_4)@

    vld1.u8     {d18},[r3],r2               @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q4,d5,d27                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp2, coeffabs_5)@

    vst1.8      {d12,d13},[r14],r6
    vmlal.u8    q4,d6,d28                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_6)@

    add         r3,r0,r2                    @pu1_src_tmp += src_strd@
    vmlsl.u8    q4,d7,d29                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp4, coeffabs_7)@


@   and         r11, r0, #31
    vmull.u8    q5,d2,d23                   @mul_res2 = vmull_u8(src_tmp3, coeffabs_1)@

    vst1.8      {d14,d15},[r14],r6
    vmlsl.u8    q5,d1,d22                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp2, coeffabs_0)@

    add         r14,r1,r6
    vmlsl.u8    q5,d3,d24                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp4, coeffabs_2)@

    vld1.u8     {d0},[r0]!                  @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q5,d4,d25                   @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_3)@

    vld1.u8     {d1},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q5,d5,d26                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_4)@

    vst1.8      {d8,d9},[r1]!               @vst1_u8(pu1_dst,sto_res)@
    vmlsl.u8    q5,d6,d27                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp3, coeffabs_5)@

    addle       r1,r1,r9,lsl #1
    vmlal.u8    q5,d7,d28                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_6)@

@   cmp         r11, r10
    vmlsl.u8    q5,d16,d29                  @mul_res2 = vmlsl_u8(mul_res2, src_tmp1, coeffabs_7)@

    add         r10, r3, r2, lsl #3         @ 10*strd - 8+2
    vmull.u8    q6,d3,d23

    add         r10, r10, r2                @ 11*strd
    vmlsl.u8    q6,d2,d22

    pld         [r10]                       @11+ 0
    vmlsl.u8    q6,d4,d24

    pld         [r10, r2]                   @11+ 1*strd
    vmlal.u8    q6,d5,d25

    pld         [r10, r2, lsl #1]           @11+ 2*strd
    vmlal.u8    q6,d6,d26

    add         r10, r10, r2                @12*strd
    vmlsl.u8    q6,d7,d27

    pld         [r10, r2, lsl #1]           @11+ 3*strd
    vmlal.u8    q6,d16,d28

@   mov         r10, r11
    vmlsl.u8    q6,d17,d29

    vld1.u8     {d2},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmull.u8    q7,d4,d23

    subs        r7,r7,#4
    vmlsl.u8    q7,d3,d22

    vst1.8      {d10, d11},[r14],r6         @vst1_u8(pu1_dst_tmp,sto_res)@
    vmlsl.u8    q7,d5,d24

    vld1.u8     {d3},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q7,d6,d25

    vld1.u8     {d4},[r3],r2                @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q7,d7,d26

    vld1.u8     {d5},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q7,d16,d27

    vld1.u8     {d6},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlal.u8    q7,d17,d28

    vld1.u8     {d7},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlsl.u8    q7,d18,d29


    bgt         kernel_8_16out              @jumps to kernel_8

epilog_16out:

    vmull.u8    q4,d1,d23                   @mul_res1 = vmull_u8(src_tmp2, coeffabs_1)@
    vmlsl.u8    q4,d0,d22                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp1, coeffabs_0)@
    vmlsl.u8    q4,d2,d24                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp3, coeffabs_2)@
    vmlal.u8    q4,d3,d25                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_3)@
    vmlal.u8    q4,d4,d26                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_4)@
    vmlsl.u8    q4,d5,d27                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp2, coeffabs_5)@
    vmlal.u8    q4,d6,d28                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_6)@
    vmlsl.u8    q4,d7,d29                   @mul_res1 = vmlsl_u8(mul_res1, src_tmp4, coeffabs_7)@
    vst1.8      {d12,d13},[r14],r6

    @vqrshrun.s16 d14,q7,#6

    vld1.u8     {d16},[r3],r2               @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmull.u8    q5,d2,d23                   @mul_res2 = vmull_u8(src_tmp3, coeffabs_1)@
    vmlsl.u8    q5,d1,d22                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp2, coeffabs_0)@
    vmlsl.u8    q5,d3,d24                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp4, coeffabs_2)@
    vmlal.u8    q5,d4,d25                   @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_3)@
    vmlal.u8    q5,d5,d26                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_4)@
    vmlsl.u8    q5,d6,d27                   @mul_res2 = vmlsl_u8(mul_res2, src_tmp3, coeffabs_5)@
    vmlal.u8    q5,d7,d28                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_6)@
    vmlsl.u8    q5,d16,d29                  @mul_res2 = vmlsl_u8(mul_res2, src_tmp1, coeffabs_7)@
    vst1.8      {d14,d15},[r14],r6

    @vqrshrun.s16 d8,q4,#6          @sto_res = vqmovun_s16(sto_res_tmp)@

    vld1.u8     {d17},[r3],r2               @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmull.u8    q6,d3,d23
    vmlsl.u8    q6,d2,d22
    vmlsl.u8    q6,d4,d24
    vmlal.u8    q6,d5,d25
    vmlal.u8    q6,d6,d26
    vmlsl.u8    q6,d7,d27
    vmlal.u8    q6,d16,d28
    vmlsl.u8    q6,d17,d29
    add         r14,r1,r6
    vst1.8      {d8,d9},[r1]!               @vst1_u8(pu1_dst,sto_res)@
    @vqrshrun.s16 d10,q5,#6         @sto_res = vqmovun_s16(sto_res_tmp)@

    vld1.u8     {d18},[r3],r2               @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmull.u8    q7,d4,d23
    vmlsl.u8    q7,d3,d22
    vmlsl.u8    q7,d5,d24
    vmlal.u8    q7,d6,d25
    vmlal.u8    q7,d7,d26
    vmlsl.u8    q7,d16,d27
    vmlal.u8    q7,d17,d28
    vmlsl.u8    q7,d18,d29

    vst1.8      {d10,d11},[r14],r6          @vst1_u8(pu1_dst_tmp,sto_res)@
    @vqrshrun.s16 d12,q6,#6

epilog_end_16out:
    vst1.8      {d12,d13},[r14],r6
    @vqrshrun.s16 d14,q7,#6

    vst1.8      {d14,d15},[r14],r6


end_loops_16out:
    tst         r5,#7
    ldr         r1, [sp], #4
    ldr         r0, [sp], #4

    beq         end2

    mov         r5, #4
    add         r0, r0, #8
    add         r1, r1, #16
    mov         r7, #16
    mov         r6, r6, lsr #1

    @

core_loop_wd_4_16out:
    rsb         r9,r5,r6,lsl #2             @r6->dst_strd   r5  ->wd
    rsb         r8,r5,r2,lsl #2             @r2->src_strd
    vmov.i8     d4,#0
    mov         r6, r6, lsl #1

outer_loop_wd_4_16out:
    subs        r12,r5,#0
    ble         end_inner_loop_wd_4_16out   @outer loop jump

inner_loop_wd_4_16out:
    add         r3,r0,r2
    vld1.u32    {d4[1]},[r3],r2             @src_tmp1 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp1, 1)@
    subs        r12,r12,#4
    vdup.u32    d5,d4[1]                    @src_tmp2 = vdup_lane_u32(src_tmp1, 1)@
    vld1.u32    {d5[1]},[r3],r2             @src_tmp2 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp2, 1)@
    vld1.u32    {d4[0]},[r0]                @src_tmp1 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp1, 0)@
    vmull.u8    q0,d5,d23                   @mul_res1 = vmull_u8(vreinterpret_u8_u32(src_tmp2), coeffabs_1)@

    vdup.u32    d6,d5[1]                    @src_tmp3 = vdup_lane_u32(src_tmp2, 1)@
    add         r0,r0,#4
    vld1.u32    {d6[1]},[r3],r2             @src_tmp3 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp3, 1)@
    vmlsl.u8    q0,d4,d22                   @mul_res1 = vmlsl_u8(mul_res1, vreinterpret_u8_u32(src_tmp1), coeffabs_0)@

    vdup.u32    d7,d6[1]                    @src_tmp4 = vdup_lane_u32(src_tmp3, 1)@
    vld1.u32    {d7[1]},[r3],r2             @src_tmp4 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp4, 1)@
    vmlsl.u8    q0,d6,d24                   @mul_res1 = vmlsl_u8(mul_res1, vreinterpret_u8_u32(src_tmp3), coeffabs_2)@

    vmull.u8    q4,d7,d23
    vdup.u32    d4,d7[1]                    @src_tmp1 = vdup_lane_u32(src_tmp4, 1)@
    vmull.u8    q1,d7,d25                   @mul_res2 = vmull_u8(vreinterpret_u8_u32(src_tmp4), coeffabs_3)@
    vld1.u32    {d4[1]},[r3],r2             @src_tmp1 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp1, 1)@
    vmlsl.u8    q4,d6,d22
    vmlal.u8    q0,d4,d26                   @mul_res1 = vmlal_u8(mul_res1, vreinterpret_u8_u32(src_tmp1), coeffabs_4)@

    vdup.u32    d5,d4[1]                    @src_tmp2 = vdup_lane_u32(src_tmp1, 1)@
    vmlsl.u8    q4,d4,d24
    vld1.u32    {d5[1]},[r3],r2             @src_tmp2 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp2, 1)@
    vmlsl.u8    q1,d5,d27                   @mul_res2 = vmlsl_u8(mul_res2, vreinterpret_u8_u32(src_tmp2), coeffabs_5)@

    vdup.u32    d6,d5[1]                    @src_tmp3 = vdup_lane_u32(src_tmp2, 1)@
    vmlal.u8    q4,d5,d25
    vld1.u32    {d6[1]},[r3],r2             @src_tmp3 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp3, 1)@
    vmlal.u8    q0,d6,d28                   @mul_res1 = vmlal_u8(mul_res1, vreinterpret_u8_u32(src_tmp3), coeffabs_6)@

    vdup.u32    d7,d6[1]                    @src_tmp4 = vdup_lane_u32(src_tmp3, 1)@
    vmlal.u8    q4,d6,d26
    vld1.u32    {d7[1]},[r3],r2             @src_tmp4 = vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp4, 1)@
    vmlsl.u8    q1,d7,d29                   @mul_res2 = vmlsl_u8(mul_res2, vreinterpret_u8_u32(src_tmp4), coeffabs_7)@

    vdup.u32    d4,d7[1]
    vadd.i16    q0,q0,q1                    @mul_res1 = vaddq_u16(mul_res1, mul_res2)@

    vmlsl.u8    q4,d7,d27
    vld1.u32    {d4[1]},[r3],r2
    vmlal.u8    q4,d4,d28
    vdup.u32    d5,d4[1]
    @vqrshrun.s16 d0,q0,#6          @sto_res = vqmovun_s16(sto_res_tmp)@

    vld1.u32    {d5[1]},[r3]
    add         r3,r1,r6
    vst1.32     {d0},[r1]!                  @vst1_lane_u32((uint32_t *)pu1_dst, vreinterpret_u32_u8(sto_res), 0)@

    vmlsl.u8    q4,d5,d29
    vst1.32     {d1},[r3],r6                @vst1_lane_u32((uint32_t *)pu1_dst_tmp, vreinterpret_u32_u8(sto_res), 1)@
    @vqrshrun.s16 d8,q4,#6

    vst1.32     {d8},[r3],r6
    @add        r1,r1,#4
    vst1.32     {d9},[r3]
    bgt         inner_loop_wd_4_16out

end_inner_loop_wd_4_16out:
    subs        r7,r7,#4
    add         r1,r1,r9,lsl #1
    add         r0,r0,r8
    bgt         outer_loop_wd_4_16out
end2:
    vpop         {d8 - d15}
    ldmfd       sp!, {r4-r12, r15}          @reload the registers from sp









