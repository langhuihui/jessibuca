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
@*  ihevc_weighted_pred_bi.s
@*
@* @brief
@*  contains function definitions for weighted prediction used in inter
@* prediction
@*
@* @author
@*  parthiban v
@*
@* @par list of functions:
@*  - ihevc_weighted_pred_bi()
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
@*  does bi-weighted prediction on the arrays pointed by  pi2_src1 and
@* pi2_src2 and stores it at location pointed  by pi2_dst   assumptions : the
@* function is optimized considering the fact width and  height are multiple
@* of 2.
@*
@* @par description:
@*  dst = ( (src1 + lvl_shift1)*wgt0 +  (src2 + lvl_shift2)*wgt1 +  (off0 +
@* off1 + 1) << (shift - 1) ) >> shift
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
@* @param[in] wgt0
@*  weight to be multiplied to source 1
@*
@* @param[in] off0
@*  offset 0
@*
@* @param[in] wgt1
@*  weight to be multiplied to source 2
@*
@* @param[in] off1
@*  offset 1
@*
@* @param[in] shift
@*  (14 bit depth) + log2_weight_denominator
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

@void ihevc_weighted_pred_bi(word16 *pi2_src1,
@                            word16 *pi2_src2,
@                            uword8 *pu1_dst,
@                            word32 src_strd1,
@                            word32 src_strd2,
@                            word32 dst_strd,
@                            word32 wgt0,
@                            word32 off0,
@                            word32 wgt1,
@                            word32 off1,
@                            word32 shift,
@                            word32 lvl_shift1,
@                            word32 lvl_shift2,
@                            word32 ht,
@                            word32 wd)

@**************variables vs registers*****************************************
@   r0 => *pi2_src1
@   r1 => *pi2_src2
@   r2 => *pu1_dst
@   r3 =>  src_strd1
@   r4 =>  src_strd2
@   r5 =>  dst_strd
@   r6 =>  wgt0
@   r7 =>  off0
@   r8 =>  wgt1
@   r9 =>  off1
@   r10 =>  shift
@   r11 =>  lvl_shift1
@   r12 =>  lvl_shift2
@   r14 =>  ht
@   r7  =>  wd

.equ    src_strd2_offset,       104
.equ    dst_strd_offset,        108
.equ    wgt0_offset,            112
.equ    off0_offset,            116
.equ    wgt1_offset,            120
.equ    off1_offset,            124
.equ    shift_offset,           128
.equ    lvl_shift1_offset,      132
.equ    lvl_shift2_offset,      136
.equ    ht_offset,              140
.equ    wd_offset,              144

.text
.align 4




.globl ihevc_weighted_pred_bi_a9q

.type ihevc_weighted_pred_bi_a9q, %function

ihevc_weighted_pred_bi_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8  -  d15}

    ldr         r6,[sp,#wgt0_offset]        @load wgt0
    ldr         r11,[sp,#lvl_shift1_offset] @load lvl_shift1
    ldr         r12,[sp,#lvl_shift2_offset] @load lvl_shift2
    vmov.s16    d7[0],r6                    @moved for scalar multiplication
    mul         r4,r11,r6                   @lvl_shift1 * wgt0
    ldr         r8,[sp,#wgt1_offset]        @load wgt1
    ldr         r7,[sp,#off0_offset]        @load off0
    vmov.s16    d7[1],r8                    @moved for scalar multiplication
    mla         r4,r12,r8,r4                @(lvl_shift1 * wgt0) + (lvl_shift2 * wgt1)
    ldr         r9,[sp,#off1_offset]        @load off1
    add         r5,r7,r9                    @off0 + off1
    ldr         r10,[sp,#shift_offset]      @load shift
    add         r5,r5,#1                    @off0 + off1 + 1
    sub         r14,r10,#1                  @shift - 1
    ldr         r7,[sp,#wd_offset]          @load wd
    lsl         r5,r5,r14                   @((off0 + off1 + 1) << (shift - 1))
    vdup.u32    q14,r10                     @vmovq_n_s32(0-shift)
    add         r4,r4,r5                    @tmp_lvl_shift += ((off0 + off1 + 1) << (shift - 1))
    vdup.u32    q15,r4                      @vmovq_n_s32(tmp_lvl_shift)
    vneg.s32    q14,q14
    ldr         r4,[sp,#src_strd2_offset]   @load src_strd2
    lsl         r9,r7,#1
    ldr         r5,[sp,#dst_strd_offset]    @load dst_strd
    lsl         r3,r3,#1
    ldr         r14,[sp,#ht_offset]         @load ht
    lsl         r4,r4,#1

    cmp         r14,#0                      @check ht == 0
    beq         end_loops                   @if equal, then end the function

outer_loop:
    cmp         r7,#0                       @check wd == 0
    beq         end_loops                   @if equal, then end the function

core_loop:
    add         r6,r0,r3                    @pi2_src_tmp1 = pi2_src1 + 2*src_strd1(2* because pi1_src is a 16 bit pointer)
    add         r8,r1,r4                    @pi2_src_tmp2 = pi2_src2 + 2*src_strd2(2* because pi2_src is a 16 bit pointer)
    vld1.s16    {d0},[r0]!                  @load and increment the pi2_src1
    add         r10,r2,r5                   @pu1_dst_tmp = pu1_dst + dst_strd
    vld1.s16    {d1},[r1]!                  @load and increment the pi2_src2
    vmull.s16   q2,d0,d7[0]                 @vmull_n_s16(pi2_src1_val1, (int16_t) wgt0)
    vld1.s16    {d2},[r6],r3                @load and increment the pi2_src_tmp1 ii iteration
    vmull.s16   q4,d1,d7[1]                 @vmull_n_s16(pi2_src2_val1, (int16_t) wgt1)
    vld1.s16    {d3},[r8],r4                @load and increment the pi2_src_tmp1 ii iteration
    vadd.s32    q2,q2,q4                    @vaddq_s32(i4_tmp1_t1, i4_tmp1_t2)

    vld1.s16    {d0},[r6],r3                @load and increment the pi2_src1 iii iteration
    vmull.s16   q5,d2,d7[0]                 @vmull_n_s16(pi2_src1_val2, (int16_t) wgt0) ii iteration

    vld1.s16    {d1},[r8],r4                @load and increment the pi2_src2 iii iteration
    vadd.s32    q2,q2,q15                   @vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t)
    vmull.s16   q7,d0,d7[0]                 @vmull_n_s16(pi2_src1_val1, (int16_t) wgt0) iii iteration

    vld1.s16    {d2},[r6],r3                @load and increment the pi2_src_tmp1 iv iteration
    vmull.s16   q6,d3,d7[1]                 @vmull_n_s16(pi2_src2_val2, (int16_t) wgt1) ii iteration
    vshl.s32    q2,q2,q14                   @vshlq_s32(i4_tmp1_t1, tmp_shift_t)

    vld1.s16    {d3},[r8],r4                @load and increment the pi2_src_tmp1 iv iteration
    vadd.s32    q5,q5,q6                    @vaddq_s32(i4_tmp2_t1, i4_tmp2_t2) ii iteration

    vqmovun.s32 d4,q2                       @vqmovun_s32(sto_res_tmp1)
    vmull.s16   q8,d1,d7[1]                 @vmull_n_s16(pi2_src2_val1, (int16_t) wgt1) iii iteration

    vadd.s32    q5,q5,q15                   @vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t) ii iteration
    vmov.s32    d5,d4                       @vcombine_u16(sto_res_tmp2, sto_res_tmp2)
    vadd.s32    q7,q7,q8                    @vaddq_s32(i4_tmp1_t1, i4_tmp1_t2) iii iteration

    vshl.s32    q5,q5,q14                   @vshlq_s32(i4_tmp2_t1, tmp_shift_t) ii iteration
    vmull.s16   q9,d2,d7[0]                 @vmull_n_s16(pi2_src1_val2, (int16_t) wgt0) iv iteration
    vqmovn.u16  d4,q2                       @vqmovn_u16(sto_res_tmp3)
    vadd.s32    q7,q7,q15                   @vaddq_s32(i4_tmp1_t1, tmp_lvl_shift_t) iii iteration

    vqmovun.s32 d10,q5                      @vqmovun_s32(sto_res_tmp1) ii iteration
    vmull.s16   q10,d3,d7[1]                @vmull_n_s16(pi2_src2_val2, (int16_t) wgt1) iv iteration

    vshl.s32    q7,q7,q14                   @vshlq_s32(i4_tmp1_t1, tmp_shift_t) iii iteration
    vmov.s32    d11,d10                     @vcombine_u16(sto_res_tmp2, sto_res_tmp2) ii iteration

    vadd.s32    q9,q9,q10                   @vaddq_s32(i4_tmp2_t1, i4_tmp2_t2) iv iteration
    vqmovun.s32 d14,q7                      @vqmovun_s32(sto_res_tmp1) iii iteration

    vadd.s32    q9,q9,q15                   @vaddq_s32(i4_tmp2_t1, tmp_lvl_shift_t) iv iteration
    vst1.s32    {d4[0]},[r2]!               @store pu1_dst i iteration

    vqmovn.u16  d10,q5                      @vqmovn_u16(sto_res_tmp3) ii iteration
    vshl.s32    q9,q9,q14                   @vshlq_s32(i4_tmp2_t1, tmp_shift_t) iv iteration
    vst1.s32    {d10[0]},[r10],r5           @store pu1_dst ii iteration


    vmov.s32    d15,d14                     @vcombine_u16(sto_res_tmp2, sto_res_tmp2) iii iteration
    vqmovn.u16  d14,q7                      @vqmovn_u16(sto_res_tmp3) iii iteration
    vqmovun.s32 d18,q9                      @vqmovun_s32(sto_res_tmp1) iv iteration
    vmov.s32    d19,d18                     @vcombine_u16(sto_res_tmp2, sto_res_tmp2)
    vst1.s32    {d14[0]},[r10],r5           @store pu1_dst iii iteration
    vqmovn.u16  d18,q9                      @vqmovn_u16(sto_res_tmp3) iv iteration
    subs        r7,r7,#4                    @decrement wd by 4 and check for 0
    vst1.s32    {d18[0]},[r10],r5           @store pu1_dst iv iteration

    bgt         core_loop                   @if greater than 0 repeat the core loop again

end_core_loop:
    rsb         r11,r9,r3,lsl #2            @2*src_strd1 - wd
    subs        r14,r14,#4                  @decrement the ht by 4
    rsb         r12,r9,r4,lsl #2            @2*src_strd2 - wd
    add         r0,r0,r11                   @pi2_src1 + 4*src_strd1 - 2*wd(since pi2_src1 is 16 bit pointer double the increment with double the wd decrement)
    asr         r7,r9,#1
    add         r1,r1,r12                   @pi2_src2 + 4*src_strd2 - 2*wd
    rsb         r10,r7,r5,lsl #2            @2*dst_strd - wd
    add         r2,r2,r10                   @pu1_dst + dst_std - wd
    bgt         core_loop                   @if ht is greater than 0 goto outer_loop

end_loops:
    vpop        {d8  -  d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp






