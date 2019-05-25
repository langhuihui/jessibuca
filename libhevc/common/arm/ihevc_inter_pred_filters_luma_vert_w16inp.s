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
@*  ihevc_inter_pred_filters_luma_vert_w16inp.s
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
@*  - ihevc_inter_pred_filters_luma_vert_w16inp()
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/

@/* all the functions here are replicated from ihevc_inter_pred_filters.c and modified to */
@/* include reconstruction */
@

@/**
@*******************************************************************************
@*
@* @brief
@*    luma vertical filter for 16bit input.
@*
@* @par description:
@*     applies a vertical filter with coefficients pointed to  by 'pi1_coeff' to
@*     the elements pointed by 'pu1_src' and  writes to the location pointed by
@*    'pu1_dst'  input is 16 bits  the filter output is downshifted by 12 and
@*     clipped to lie  between 0 and 255   assumptions : the function is
@*     optimized considering the fact width is  multiple of 4. and height as
@*     multiple of 2.
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

@void ihevc_inter_pred_luma_vert_w16inp(word16 *pi2_src,
@                                    uword8 *pu1_dst,
@                                    word32 src_strd,
@                                    word32 dst_strd,
@                                    word8 *pi1_coeff,
@                                    word32 ht,
@                                    word32 wd   )

.equ    coeff_offset,   104
.equ    ht_offset,      108
.equ    wd_offset,      112

.text
.align 4




.globl ihevc_inter_pred_luma_vert_w16inp_a9q

.type ihevc_inter_pred_luma_vert_w16inp_a9q, %function

ihevc_inter_pred_luma_vert_w16inp_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush        {d8 - d15}

    ldr         r12,[sp,#coeff_offset]                @load pi1_coeff
    mov         r6,r3
    ldr         r5,[sp,#wd_offset]                 @load wd
    vld1.8      {d0},[r12]                  @coeff = vld1_s8(pi1_coeff)
    mov         r2, r2, lsl #1
    sub         r12,r2,r2,lsl #2            @src_ctrd & pi1_coeff
    @vabs.s8    d0,d0               @vabs_s8(coeff)
    add         r0,r0,r12                   @r0->pu1_src    r12->pi1_coeff
    ldr         r3,[sp,#ht_offset]                 @load ht
    subs        r7,r3,#0                    @r3->ht
    @ble        end_loops           @end loop jump
    vmovl.s8    q0,d0
    vdup.16     d22,d0[0]                   @coeffabs_0 = vdup_lane_u8(coeffabs, 0)@
    vdup.16     d23,d0[1]                   @coeffabs_1 = vdup_lane_u8(coeffabs, 1)@
    vdup.16     d24,d0[2]                   @coeffabs_2 = vdup_lane_u8(coeffabs, 2)@
    vdup.16     d25,d0[3]                   @coeffabs_3 = vdup_lane_u8(coeffabs, 3)@
    vdup.16     d26,d1[0]                   @coeffabs_4 = vdup_lane_u8(coeffabs, 4)@
    vdup.16     d27,d1[1]                   @coeffabs_5 = vdup_lane_u8(coeffabs, 5)@
    vdup.16     d28,d1[2]                   @coeffabs_6 = vdup_lane_u8(coeffabs, 6)@
    vdup.16     d29,d1[3]                   @coeffabs_7 = vdup_lane_u8(coeffabs, 7)@

    rsb         r9,r5,r6,lsl #2             @r6->dst_strd   r5  ->wd
    rsb         r8,r5,r2,lsl #2             @r2->src_strd
    sub         r8,r8,r5
    mov         r3, r5, lsr #2              @divide by 4
    mul         r7, r3                      @multiply height by width
    sub         r7, #4                      @subtract by one for epilog
    mov         r4,r5                       @r5 ->wd
    @mov            r2, r2, lsl #1

prolog:

    add         r3,r0,r2                    @pu1_src_tmp += src_strd@
    vld1.16     {d1},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vld1.16     {d0},[r0]!                  @src_tmp1 = vld1_u8(pu1_src_tmp)@
    subs        r4,r4,#4
    vld1.16     {d2},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmull.s16   q4,d1,d23                   @mul_res1 = vmull_u8(src_tmp2, coeffabs_1)@
    vld1.16     {d3},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q4,d0,d22                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_0)@
    vld1.16     {d4},[r3],r2                @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q4,d2,d24                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_2)@
    vld1.16     {d5},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q4,d3,d25                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_3)@
    vld1.16     {d6},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q4,d4,d26                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_4)@
    vld1.16     {d7},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q4,d5,d27                   @mul_res1 = vmlal_u8(mul_res1, src_tmp2, coeffabs_5)@
    vmlal.s16   q4,d6,d28                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_6)@
    vmlal.s16   q4,d7,d29                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_7)@

    vld1.16     {d16},[r3],r2               @src_tmp1 = vld1_u8(pu1_src_tmp)@

    vmull.s16   q5,d2,d23                   @mul_res2 = vmull_u8(src_tmp3, coeffabs_1)@
    addle       r0,r0,r8,lsl #0
    vmlal.s16   q5,d1,d22                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_0)@
    movle       r4,r5                       @r5 ->wd
    vmlal.s16   q5,d3,d24                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_2)@
    vld1.16     {d17},[r3],r2               @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q5,d4,d25                   @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_3)@
    vld1.16     {d18},[r3],r2               @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q5,d5,d26                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_4)@
    add         r3,r0,r2                    @pu1_src_tmp += src_strd@
    vmlal.s16   q5,d6,d27                   @mul_res2 = vmlal_u8(mul_res2, src_tmp3, coeffabs_5)@
    vmlal.s16   q5,d7,d28                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_6)@
    vmlal.s16   q5,d16,d29                  @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_7)@
    vqshrn.s32  d8, q4, #6

    vld1.16     {d1},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmull.s16   q6,d3,d23
    vld1.16     {d0},[r0]!                  @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q6,d2,d22
    vld1.16     {d2},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q6,d4,d24
    vmlal.s16   q6,d5,d25
    vmlal.s16   q6,d6,d26
    vmlal.s16   q6,d7,d27
    vmlal.s16   q6,d16,d28
    vmlal.s16   q6,d17,d29
    add         r14,r1,r6
    vqshrn.s32  d10, q5, #6
    vqrshrun.s16 d8,q4,#6                   @sto_res = vqmovun_s16(sto_res_tmp)@

    vmull.s16   q7,d4,d23
    vmlal.s16   q7,d3,d22
    vmlal.s16   q7,d5,d24
    vmlal.s16   q7,d6,d25
    vld1.16     {d3},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q7,d7,d26
    vld1.16     {d4},[r3],r2                @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q7,d16,d27
    vld1.16     {d5},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q7,d17,d28
    vld1.16     {d6},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q7,d18,d29
    vld1.16     {d7},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@

    vst1.32     {d8[0]},[r1]!               @vst1_u8(pu1_dst,sto_res)@
    vqshrn.s32  d12, q6, #6
    vqrshrun.s16 d10,q5,#6                  @sto_res = vqmovun_s16(sto_res_tmp)@
    addle       r1,r1,r9

    subs        r7,r7,#4

    blt         epilog_end                  @jumps to epilog_end
    beq         epilog                      @jumps to epilog

kernel_8:

    vmull.s16   q4,d1,d23                   @mul_res1 = vmull_u8(src_tmp2, coeffabs_1)@
    subs        r4,r4,#4
    vmlal.s16   q4,d0,d22                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_0)@
    addle       r0,r0,r8,lsl #0
    vmlal.s16   q4,d2,d24                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_2)@
    vmlal.s16   q4,d3,d25                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_3)@
    vmlal.s16   q4,d4,d26                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_4)@
    vmlal.s16   q4,d5,d27                   @mul_res1 = vmlal_u8(mul_res1, src_tmp2, coeffabs_5)@
    vmlal.s16   q4,d6,d28                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_6)@
    vmlal.s16   q4,d7,d29                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_7)@
    vst1.32     {d10[0]},[r14],r6           @vst1_u8(pu1_dst_tmp,sto_res)@

    vqshrn.s32  d14, q7, #6
    vqrshrun.s16 d12,q6,#6
    vld1.16     {d16},[r3],r2               @src_tmp1 = vld1_u8(pu1_src_tmp)@

    vmull.s16   q5,d2,d23                   @mul_res2 = vmull_u8(src_tmp3, coeffabs_1)@
    vmlal.s16   q5,d1,d22                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_0)@
    vmlal.s16   q5,d3,d24                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_2)@
    vmlal.s16   q5,d4,d25                   @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_3)@
    vmlal.s16   q5,d5,d26                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_4)@
    vmlal.s16   q5,d6,d27                   @mul_res2 = vmlal_u8(mul_res2, src_tmp3, coeffabs_5)@
    vst1.32     {d12[0]},[r14],r6

    vmlal.s16   q5,d7,d28                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_6)@
    vld1.16     {d17},[r3],r2               @src_tmp2 = vld1_u8(pu1_src_tmp)@

    vmlal.s16   q5,d16,d29                  @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_7)@

    vqshrn.s32  d8, q4, #6
    vqrshrun.s16 d14,q7,#6

    vmull.s16   q6,d3,d23
    movle       r4,r5                       @r5 ->wd

    vmlal.s16   q6,d2,d22
    vld1.16     {d18},[r3],r2               @src_tmp3 = vld1_u8(pu1_src_tmp)@

    vmlal.s16   q6,d4,d24
    add         r3,r0,r2                    @pu1_src_tmp += src_strd@

    vmlal.s16   q6,d5,d25

    vmlal.s16   q6,d6,d26
    vst1.32     {d14[0]},[r14],r6

    vmlal.s16   q6,d7,d27
    vld1.16     {d1},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@

    vmlal.s16   q6,d16,d28
    add         r14,r1,r6

    vmlal.s16   q6,d17,d29
    vld1.16     {d0},[r0]!                  @src_tmp1 = vld1_u8(pu1_src_tmp)@

    vqshrn.s32  d10, q5, #6
    vqrshrun.s16 d8,q4,#6                   @sto_res = vqmovun_s16(sto_res_tmp)@
    vld1.16     {d2},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@

    vmull.s16   q7,d4,d23
    vmlal.s16   q7,d3,d22
    vmlal.s16   q7,d5,d24
    vld1.16     {d3},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@

    vmlal.s16   q7,d6,d25
    vld1.16     {d4},[r3],r2                @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q7,d7,d26
    vld1.16     {d5},[r3],r2                @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q7,d16,d27
    vld1.16     {d6},[r3],r2                @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q7,d17,d28
    vld1.16     {d7},[r3],r2                @src_tmp4 = vld1_u8(pu1_src_tmp)@
    vmlal.s16   q7,d18,d29
    vst1.32     {d8[0]},[r1]!               @vst1_u8(pu1_dst,sto_res)@

    vqshrn.s32  d12, q6, #6
    addle       r1,r1,r9

    vqrshrun.s16 d10,q5,#6                  @sto_res = vqmovun_s16(sto_res_tmp)@
    subs        r7,r7,#4

    bgt         kernel_8                    @jumps to kernel_8

epilog:

    vmull.s16   q4,d1,d23                   @mul_res1 = vmull_u8(src_tmp2, coeffabs_1)@
    vmlal.s16   q4,d0,d22                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_0)@
    vmlal.s16   q4,d2,d24                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_2)@
    vmlal.s16   q4,d3,d25                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_3)@
    vmlal.s16   q4,d4,d26                   @mul_res1 = vmlal_u8(mul_res1, src_tmp1, coeffabs_4)@
    vmlal.s16   q4,d5,d27                   @mul_res1 = vmlal_u8(mul_res1, src_tmp2, coeffabs_5)@
    vmlal.s16   q4,d6,d28                   @mul_res1 = vmlal_u8(mul_res1, src_tmp3, coeffabs_6)@
    vmlal.s16   q4,d7,d29                   @mul_res1 = vmlal_u8(mul_res1, src_tmp4, coeffabs_7)@
    vst1.32     {d10[0]},[r14],r6

    vqshrn.s32  d14, q7, #6
    vqrshrun.s16 d12,q6,#6

    vld1.16     {d16},[r3],r2               @src_tmp1 = vld1_u8(pu1_src_tmp)@
    vmull.s16   q5,d2,d23                   @mul_res2 = vmull_u8(src_tmp3, coeffabs_1)@
    vmlal.s16   q5,d1,d22                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_0)@
    vmlal.s16   q5,d3,d24                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_2)@
    vmlal.s16   q5,d4,d25                   @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_3)@
    vmlal.s16   q5,d5,d26                   @mul_res2 = vmlal_u8(mul_res2, src_tmp2, coeffabs_4)@
    vmlal.s16   q5,d6,d27                   @mul_res2 = vmlal_u8(mul_res2, src_tmp3, coeffabs_5)@
    vmlal.s16   q5,d7,d28                   @mul_res2 = vmlal_u8(mul_res2, src_tmp4, coeffabs_6)@
    vmlal.s16   q5,d16,d29                  @mul_res2 = vmlal_u8(mul_res2, src_tmp1, coeffabs_7)@
    vst1.32     {d12[0]},[r14],r6

    vqshrn.s32  d8, q4, #6
    vqrshrun.s16 d14,q7,#6

    vld1.16     {d17},[r3],r2               @src_tmp2 = vld1_u8(pu1_src_tmp)@
    vmull.s16   q6,d3,d23
    vmlal.s16   q6,d2,d22
    vmlal.s16   q6,d4,d24
    vmlal.s16   q6,d5,d25
    vmlal.s16   q6,d6,d26
    vmlal.s16   q6,d7,d27
    vmlal.s16   q6,d16,d28
    vmlal.s16   q6,d17,d29
    vst1.32     {d14[0]},[r14],r6
    vqshrn.s32  d10, q5, #6
    vqrshrun.s16 d8,q4,#6                   @sto_res = vqmovun_s16(sto_res_tmp)@

    vld1.16     {d18},[r3],r2               @src_tmp3 = vld1_u8(pu1_src_tmp)@
    vmull.s16   q7,d4,d23
    vmlal.s16   q7,d3,d22
    vmlal.s16   q7,d5,d24
    vmlal.s16   q7,d6,d25
    vmlal.s16   q7,d7,d26
    vmlal.s16   q7,d16,d27
    vmlal.s16   q7,d17,d28
    vmlal.s16   q7,d18,d29
    vqshrn.s32  d12, q6, #6
    vqrshrun.s16 d10,q5,#6                  @sto_res = vqmovun_s16(sto_res_tmp)@

    add         r14,r1,r6
    vst1.32     {d8[0]},[r1]!               @vst1_u8(pu1_dst,sto_res)@

epilog_end:
    vst1.32     {d10[0]},[r14],r6           @vst1_u8(pu1_dst_tmp,sto_res)@
    vqrshrun.s16 d12,q6,#6

    vst1.32     {d12[0]},[r14],r6
    vqshrn.s32  d14, q7, #6
    vqrshrun.s16 d14,q7,#6

    vst1.32     {d14[0]},[r14],r6


end_loops:

    vpop         {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp




















