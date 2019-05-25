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
@*  ihevc_weighted_pred_uni.s
@*
@* @brief
@*  contains function definitions for weighted prediction used in inter
@* prediction
@*
@* @author
@*  parthiban v
@*
@* @par list of functions:
@*  - ihevc_weighted_pred_uni()
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
@*  does uni-weighted prediction on the array pointed by  pi2_src and stores
@* it at the location pointed by pi2_dst assumptions : the function is
@* optimized considering the fact width and  height are multiple of 2.
@*
@* @par description:
@*  dst = ( (src + lvl_shift) * wgt0 + (1 << (shift - 1)) )  >> shift +
@* offset
@*
@* @param[in] pi2_src
@*  pointer to the source
@*
@* @param[out] pu1_dst
@*  pointer to the destination
@*
@* @param[in] src_strd
@*  source stride
@*
@* @param[in] dst_strd
@*  destination stride
@*
@* @param[in] wgt0
@*  weight to be multiplied to the source
@*
@* @param[in] off0
@*  offset to be added after rounding and
@*
@* @param[in] shifting
@*
@*
@* @param[in] shift
@*  (14 bit depth) + log2_weight_denominator
@*
@* @param[in] lvl_shift
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

@void ihevc_weighted_pred_uni(word16 *pi2_src,
@                             uword8 *pu1_dst,
@                             word32 src_strd,
@                             word32 dst_strd,
@                             word32 wgt0,
@                             word32 off0,
@                             word32 shift,
@                             word32 lvl_shift,
@                             word32 ht,
@                             word32 wd)

@**************variables vs registers*****************************************
@   r0 => *pi2_src
@   r1 => *pu1_dst
@   r2 =>  src_strd
@   r3 =>  dst_strd
@   r4 =>  wgt0
@   r5 =>  off0
@   r6 =>  shift
@   r7 =>  lvl_shift
@   r8 =>   ht
@   r9  =>  wd

.equ    wgt0_offset,        104
.equ    off0_offset,        108
.equ    shift_offset,       112
.equ    lvl_shift_offset,   116
.equ    ht_offset,          120
.equ    wd_offset,          124

.text
.align 4




.globl ihevc_weighted_pred_uni_a9q

.type ihevc_weighted_pred_uni_a9q, %function

ihevc_weighted_pred_uni_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8  -  d15}

    ldr         r4,[sp,#wgt0_offset]        @load wgt0
    ldr         r7,[sp,#lvl_shift_offset]   @load lvl_shift
    mov         r11,#1
    ldr         r5,[sp,#off0_offset]        @load off0
    mul         r10,r7,r4                   @lvl_shift * wgt0
    ldr         r6,[sp,#shift_offset]       @load shift
    ldr         r8,[sp,#ht_offset]          @load ht
    add         r10,r10,r5,lsl r6           @lvl_shift * wgt0 + (off0 << shift)
    ldr         r9,[sp,#wd_offset]          @load wt
    sub         r12,r6,#1
    vmov.s16    d0[0],r4                    @moved for scalar multiplication
    lsl         r2,r2,#1
    vdup.u32    q14,r6                      @vmovq_n_s32(tmp_shift)
    add         r10,r10,r11,lsl r12         @tmp_lvl_shift += (1 << (shift - 1))
    vdup.s32    q15,r10                     @vmovq_n_s32(tmp_lvl_shift)
    vneg.s32    q14,q14
    lsl         r4,r9,#1

    cmp         r8,#0                       @check ht == 0
    beq         end_loops                   @if equal, then end the function

outer_loop:
    cmp         r9,#0                       @check wd == 0
    beq         end_loops                   @if equal, then end the function

core_loop:
    add         r5,r0,r2                    @pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         r6,r1,r3                    @pu1_dst_tmp = pu1_dst + dst_strd
    vld1.s16    {d1},[r0]!                  @load and increment the pi2_src
    vld1.s16    {d2},[r5],r2                @load and increment the pi2_src_tmp ii iteration
    vmull.s16   q2,d1,d0[0]                 @vmull_n_s16(pi2_src_val1, (int16_t) wgt0)

    vadd.i32    q2,q2,q15                   @vaddq_s32(i4_tmp1_t, tmp_lvl_shift_t)
    vld1.s16    {d8},[r5],r2                @load and increment the pi2_src iii iteration

    vmull.s16   q3,d2,d0[0]                 @vmull_n_s16(pi2_src_val2, (int16_t) wgt0) ii iteration
    vld1.s16    {d9},[r5],r2                @load and increment the pi2_src_tmp iv iteration

    vshl.s32    q2,q2,q14                   @vshlq_s32(i4_tmp1_t, tmp_shift_t)
    vadd.i32    q3,q3,q15                   @vaddq_s32(i4_tmp2_t, tmp_lvl_shift_t) ii iteration

    vmull.s16   q5,d8,d0[0]                 @vmull_n_s16(pi2_src_val1, (int16_t) wgt0) iii iteration
    vqmovun.s32 d4,q2                       @vqmovun_s32(sto_res_tmp1)

    vadd.i32    q5,q5,q15                   @vaddq_s32(i4_tmp1_t, tmp_lvl_shift_t) iii iteration
    vmov.s32    d5,d4                       @vcombine_u16(sto_res_tmp2, sto_res_tmp2)

    vshl.s32    q3,q3,q14                   @vshlq_s32(i4_tmp2_t, tmp_shift_t) ii iteration

    vmull.s16   q6,d9,d0[0]                 @vmull_n_s16(pi2_src_val2, (int16_t) wgt0) iv iteration
    vqmovn.u16  d4,q2                       @vqmovn_u16(sto_res_tmp3)

    vshl.s32    q5,q5,q14                   @vshlq_s32(i4_tmp1_t, tmp_shift_t) iii iteration
    vqmovun.s32 d6,q3                       @vqmovun_s32(sto_res_tmp1) ii iteration

    vadd.i32    q6,q6,q15                   @vaddq_s32(i4_tmp2_t, tmp_lvl_shift_t) iv iteration
    vmov.s32    d7,d6                       @vcombine_u16(sto_res_tmp2, sto_res_tmp2) ii iteration

    vqmovun.s32 d10,q5                      @vqmovun_s32(sto_res_tmp1) iii iteration

    vshl.s32    q6,q6,q14                   @vshlq_s32(i4_tmp2_t, tmp_shift_t) iv iteration
    vst1.32     {d4[0]},[r1]!               @store pu1_dst i iteration
    vmov.s32    d11,d10                     @vcombine_u16(sto_res_tmp2, sto_res_tmp2) iii iteration

    vqmovn.u16  d6,q3                       @vqmovn_u16(sto_res_tmp3) ii iteration
    vst1.32     {d6[0]},[r6],r3             @store pu1_dst ii iteration

    vqmovn.u16  d10,q5                      @vqmovn_u16(sto_res_tmp3) iii iteration
    vqmovun.s32 d12,q6                      @vqmovun_s32(sto_res_tmp1) iv iteration

    vmov.s32    d13,d12                     @vcombine_u16(sto_res_tmp2, sto_res_tmp2) iv iteration
    vst1.32     {d10[0]},[r6],r3            @store pu1_dst i iteration iii iteration
    vqmovn.u16  d12,q6                      @vqmovn_u16(sto_res_tmp3) iv iteration

    subs        r9,r9,#4                    @decrement wd by 4 and check for 0
    vst1.32     {d12[0]},[r6],r3            @store pu1_dst iv iteration
    bgt         core_loop                   @if greater than 0 repeat the core loop again

end_core_loop:
    rsb         r11,r4,r2,lsl #2            @2*src_strd - wd
    subs        r8,r8,#4                    @decrement the ht by 4
    add         r0,r0,r11                   @pi2_src + 4*src_strd - 2*wd(since pi2_src is 16 bit pointer double the increment with double the wd decrement)
    asr         r9,r4,#1
    rsb         r12,r9,r3,lsl #2            @2*dst_strd - wd
    add         r1,r1,r12                   @pu1_dst + dst_std - wd
    bgt         core_loop                   @if ht is greater than 0 goto outer_loop

end_loops:
    vpop        {d8  -  d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp


