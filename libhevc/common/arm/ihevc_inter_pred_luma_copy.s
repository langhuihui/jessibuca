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
@/**
@*******************************************************************************
@*
@* @brief
@*     interprediction luma function for copy
@*
@* @par description:
@*   copies the array of width 'wd' and height 'ht' from the  location pointed
@*   by 'src' to the location pointed by 'dst'
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
@void ihevc_inter_pred_luma_copy (
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
@   r3 =>  dst_strd
@   r7 =>  ht
@   r12 => wd

.equ    coeff_offset,   104
.equ    ht_offset,      108
.equ    wd_offset,      112

.text
.align 4




.globl ihevc_inter_pred_luma_copy_a9q

.type ihevc_inter_pred_luma_copy_a9q, %function

ihevc_inter_pred_luma_copy_a9q:
    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush        {d8 - d15}
    ldr         r12,[sp,#wd_offset]                @loads wd
    ldr         r7,[sp,#ht_offset]                 @loads ht
    cmp         r7,#0                       @checks ht == 0
    ble         end_loops
    tst         r12,#15                     @checks wd for multiples for 4 & 8
    beq         core_loop_wd_16
    tst         r12,#7                      @checks wd for multiples for 4 & 8
    beq         core_loop_wd_8
    sub         r11,r12,#4

outer_loop_wd_4:
    subs        r4,r12,#0                   @checks wd == 0
    ble         end_inner_loop_wd_4

inner_loop_wd_4:
    vld1.32     {d0[0]},[r0]                @vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    add         r5,r0,r2                    @pu1_src_tmp += src_strd
    add         r6,r1,r3                    @pu1_dst_tmp += dst_strd
    vst1.32     {d0[0]},[r1]                @vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    vld1.32     {d0[0]},[r5],r2             @vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    add         r0,r0,#4                    @pu1_src += 4
    vst1.32     {d0[0]},[r6],r3             @vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    vld1.32     {d0[0]},[r5],r2             @vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    subs        r4,r4,#4                    @(wd -4)
    vst1.32     {d0[0]},[r6],r3             @vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)
    vld1.32     {d0[0]},[r5],r2             @vld1_lane_u32((uint32_t *)pu1_src_tmp, src_tmp, 0)
    add         r1,r1,#4                    @pu1_dst += 4
    vst1.32     {d0[0]},[r6],r3             @vst1_lane_u32((uint32_t *)pu1_dst_tmp, src_tmp, 0)

    bgt         inner_loop_wd_4

end_inner_loop_wd_4:
    subs        r7,r7,#4                    @ht - 4
    sub         r0,r5,r11                   @pu1_src = pu1_src_tmp
    sub         r1,r6,r11                   @pu1_dst = pu1_dst_tmp
    bgt         outer_loop_wd_4

end_loops:
    vpop         {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp


core_loop_wd_8:
    sub         r11,r12,#8

outer_loop_wd_8:
    subs        r4,r12,#0                   @checks wd
    ble         end_inner_loop_wd_8

inner_loop_wd_8:
    add         r5,r0,r2                    @pu1_src_tmp += src_strd
    vld1.8      {d0},[r0]!                  @vld1_u8(pu1_src_tmp)
    add         r6,r1,r3                    @pu1_dst_tmp += dst_strd
    vst1.8      {d0},[r1]!                  @vst1_u8(pu1_dst_tmp, tmp_src)
    vld1.8      {d1},[r5],r2                @vld1_u8(pu1_src_tmp)
    vst1.8      {d1},[r6],r3                @vst1_u8(pu1_dst_tmp, tmp_src)
    subs        r4,r4,#8                    @wd - 8(loop condition)
    vld1.8      {d2},[r5],r2                @vld1_u8(pu1_src_tmp)
    vst1.8      {d2},[r6],r3                @vst1_u8(pu1_dst_tmp, tmp_src)
    vld1.8      {d3},[r5],r2                @vld1_u8(pu1_src_tmp)
    vst1.8      {d3},[r6],r3                @vst1_u8(pu1_dst_tmp, tmp_src)
    bgt         inner_loop_wd_8

end_inner_loop_wd_8:
    subs        r7,r7,#4                    @ht -= 4
    sub         r0,r5,r11                   @pu1_src = pu1_src_tmp
    sub         r1,r6,r11                   @pu1_dst = pu1_dst_tmp
    bgt         outer_loop_wd_8

    vpop         {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp

core_loop_wd_16:
    sub         r11,r12,#16

outer_loop_wd_16:
    subs        r4,r12,#0                   @checks wd
    ble         end_inner_loop_wd_16

inner_loop_wd_16:
    add         r5,r0,r2                    @pu1_src_tmp += src_strd
    vld1.8      {q0},[r0]!                  @vld1_u8(pu1_src_tmp)
    add         r6,r1,r3                    @pu1_dst_tmp += dst_strd
    vst1.8      {q0},[r1]!                  @vst1_u8(pu1_dst_tmp, tmp_src)
    vld1.8      {q1},[r5],r2                @vld1_u8(pu1_src_tmp)
    vst1.8      {q1},[r6],r3                @vst1_u8(pu1_dst_tmp, tmp_src)
    subs        r4,r4,#16                   @wd - 8(loop condition)
    vld1.8      {q2},[r5],r2                @vld1_u8(pu1_src_tmp)
    vst1.8      {q2},[r6],r3                @vst1_u8(pu1_dst_tmp, tmp_src)
    vld1.8      {q3},[r5],r2                @vld1_u8(pu1_src_tmp)
    vst1.8      {q3},[r6],r3                @vst1_u8(pu1_dst_tmp, tmp_src)
    bgt         inner_loop_wd_16

end_inner_loop_wd_16:
    subs        r7,r7,#4                    @ht -= 4
    sub         r0,r5,r11                   @pu1_src = pu1_src_tmp
    sub         r1,r6,r11                   @pu1_dst = pu1_dst_tmp
    bgt         outer_loop_wd_16

    vpop         {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp





