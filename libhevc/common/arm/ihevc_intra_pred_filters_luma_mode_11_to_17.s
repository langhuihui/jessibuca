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
@*  ihevc_intra_pred_luma_mode_11_to_17.s
@*
@* @brief
@*  contains function definitions for intra prediction dc filtering.
@* functions are coded using neon  intrinsics and can be compiled using

@* rvct
@*
@* @author
@*  akshaya mukund
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
@*    luma intraprediction filter for dc input
@*
@* @par description:
@*
@* @param[in] pu1_ref
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
@* @param[in] nt
@*  size of tranform block
@*
@* @param[in] mode
@*  type of filtering
@*
@* @returns
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/

@void ihevc_intra_pred_luma_mode_11_to_17(uword8* pu1_ref,
@                               word32 src_strd,
@                               uword8* pu1_dst,
@                               word32 dst_strd,
@                               word32 nt,
@                               word32 mode)
@
@**************variables vs registers*****************************************
@r0 => *pu1_ref
@r1 => src_strd
@r2 => *pu1_dst
@r3 => dst_strd

@stack contents from #236
@   nt
@   mode

.equ    nt_offset,      236
.equ    mode_offset,    240

.text
.align 4




.globl ihevc_intra_pred_luma_mode_11_to_17_a9q
.extern gai4_ihevc_ang_table
.extern gai4_ihevc_inv_ang_table
.extern col_for_intra_luma
.extern idx_neg_idx_11_17

gai4_ihevc_ang_table_addr:
.long gai4_ihevc_ang_table - ulbl1 - 8

gai4_ihevc_inv_ang_table_addr:
.long gai4_ihevc_inv_ang_table - ulbl2 - 8

idx_neg_idx_11_17_addr_1:
.long idx_neg_idx_11_17 - ulbl3 - 8

idx_neg_idx_11_17_addr_2:
.long idx_neg_idx_11_17 - ulbl4 - 8

col_for_intra_luma_addr_1:
.long col_for_intra_luma - ulbl_1 - 8

col_for_intra_luma_addr_2:
.long col_for_intra_luma - ulbl_2 - 8

col_for_intra_luma_addr_3:
.long col_for_intra_luma - ulbl_3 - 8

col_for_intra_luma_addr_4:
.long col_for_intra_luma - ulbl_4 - 8

.type ihevc_intra_pred_luma_mode_11_to_17_a9q, %function

ihevc_intra_pred_luma_mode_11_to_17_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8 - d15}
    sub         sp, sp, #132                @ref_temp[2 * max_cu_size + 1]
    ldr         r4,[sp,#nt_offset]          @loads nt
    ldr         r7, gai4_ihevc_ang_table_addr
ulbl1:
    add         r7,r7,pc

    ldr         r5,[sp,#mode_offset]        @mode (11 to 17)
    ldr         r8, gai4_ihevc_inv_ang_table_addr
ulbl2:
    add         r8,r8,pc

    add         r7, r7, r5, lsl #2          @gai4_ihevc_ang_table[mode]
    add         r8, r8, r5, lsl #2          @gai4_ihevc_inv_ang_table[mode - 11]
    sub         r8, r8, #44

    ldr         r7, [r7]                    @intra_pred_ang

    ldr         r8, [r8]                    @inv_ang
    add         r6, sp, r4                  @ref_temp + nt

    mul         r9, r4, r7                  @nt*intra_pred_ang

    sub         r6, r6, #1                  @ref_temp + nt - 1

    add         r1, r0, r4, lsl #1          @r1 = &src[2nt]
    vdup.8      d30, r7                     @intra_pred_ang

    mov         r7, r4

    ldrb        r11, [r1], #-1

    asr         r9, r9, #5

    ldrb        r12, [r1], #-1
    ldrb        r10, [r1], #-1
    ldrb        r14, [r1], #-1

    strb        r11, [r6], #1
    strb        r12, [r6], #1
    strb        r10, [r6], #1
    strb        r14, [r6], #1

    subs        r7, r7, #4
    beq         end_loop_copy

    sub         r6,#4
    sub         r1,#3

    subs        r7,r7,#4
    beq         loop_copy_8
    subs        r7,r7,#8
    beq         loop_copy_16

loop_copy_32:
    vld1.8      d0,[r1]
    sub         r1,#8
    vld1.8      d1,[r1]
    sub         r1,#8
    vld1.8      d2,[r1]
    sub         r1,#8
    vld1.8      d3,[r1]

    vrev64.8    d0,d0
    vrev64.8    d1,d1
    vst1.8      d0,[r6]!
    vrev64.8    d2,d2
    vst1.8      d1,[r6]!
    vrev64.8    d3,d3
    vst1.8      d2,[r6]!
    vst1.8      d3,[r6]!
    sub         r1,#1
    b           end_loop_copy

loop_copy_16:
    vld1.8      d0,[r1]
    sub         r1,#8
    vld1.8      d1,[r1]

    vrev64.8    d0,d0
    vrev64.8    d1,d1

    vst1.8      d0,[r6]!
    vst1.8      d1,[r6]!
    sub         r1,#1
    b           end_loop_copy

loop_copy_8:
    vld1.8      d0,[r1]
    vrev64.8    d0,d0
    vst1.8      d0,[r6]!
    sub         r1,#1
end_loop_copy:

    ldrb        r11, [r1], #-1
    strb        r11, [r6], #1

    cmp         r9, #-1
    bge         prologue_8_16_32

    add         r6, sp, r4                  @ref_temp + nt
    sub         r6, r6, #2                  @ref_temp + nt - 2

    mov         r12, #0xffffffff

    rsb         r9, r9, r12                 @count to take care off ref_idx

    add         r1, r0, r4, lsl #1          @r1 = &src[2nt]

    mov         r7, #128                    @inv_ang_sum

loop_copy_ref_idx:

    add         r7, r7, r8                  @inv_ang_sum += inv_ang

    ldrb        r11, [r1, r7, lsr #8]
    strb        r11, [r6], #-1

    subs        r9, r9, #1

    bne         loop_copy_ref_idx

prologue_8_16_32:
    cmp         r4, #4
    beq         sz_4_proc
    ldr         r14, col_for_intra_luma_addr_1
ulbl_1:
    add         r14,r14,pc

    lsr         r10, r4, #3
    vld1.8      d31, [r14]!
    mul         r10, r4, r10                @block counter (dec by #8)

    mov         r11, r4                     @col counter to be inc/dec by #8
    vmull.s8    q11, d30, d31               @(col+1)*intra_pred_angle [0:7](col)
    mov         r0, #1

    sub         r7, r5, #11
    vdup.8      d2, r0                      @contains #1 for adding to get ref_main_idx + 1
    ldr         r12, idx_neg_idx_11_17_addr_1 @load least idx table
ulbl3:
    add         r12,r12,pc

    mov         r0, #2
    vdup.8      d3, r0

    add         r12, r12, r7, lsl #4
    mov         r8, r12

    mov         r7, #8
    sub         r7, r7, r3, lsl #3          @r7 = 8-8r3

    ldr         r9, [r8]
    add         r1, sp, r4                  @ref_temp + nt

    vmovn.s16   d6, q11
    vdup.8      d26, r9                     @least idx added to final idx values
    sub         r1, r1, #1                  @ref_temp + nt - 1

    add         r6, r1, r9

    vld1.8      {d0,d1}, [r6]               @stores the 32 values reqd based on indices values (from least idx)
    vshr.s16    q11, q11, #5

    mov         r0, #31
    vdup.8      d29, r0                     @contains #31 for vand operation

    mov         r0, #32
    vdup.8      d28, r0

    vqmovn.s16  d8, q11

    vand        d6, d6, d29                 @fract values in d1/ idx values in d0

    mov         r0, #1
    vdup.8      d27, r0                     @row value inc or reset accordingly

    vadd.s8     d8, d8, d27                 @ref_main_idx (add row)
    vsub.s8     d8, d8, d26                 @ref_main_idx (row 0)
    vadd.s8     d9, d8, d2                  @ref_main_idx + 1 (row 0)
    vtbl.8      d12, {d0,d1}, d8            @load from ref_main_idx (row 0)
    vsub.s8     d7, d28, d6                 @32-fract

    vtbl.8      d13, {d0,d1}, d9            @load from ref_main_idx + 1 (row 0)
    vadd.s8     d4, d8, d2                  @ref_main_idx (row 1)
    vadd.s8     d5, d9, d2                  @ref_main_idx + 1 (row 1)

    vtbl.8      d16, {d0,d1}, d4            @load from ref_main_idx (row 1)
    vmull.u8    q12, d12, d7                @mul (row 0)
    vmlal.u8    q12, d13, d6                @mul (row 0)

    vtbl.8      d17, {d0,d1}, d5            @load from ref_main_idx + 1 (row 1)
    vadd.s8     d8, d8, d3                  @ref_main_idx (row 2)
    vadd.s8     d9, d9, d3                  @ref_main_idx + 1 (row 2)

    vrshrn.i16  d24, q12, #5                @round shft (row 0)

    vtbl.8      d14, {d0,d1}, d8            @load from ref_main_idx (row 2)
    vmull.u8    q11, d16, d7                @mul (row 1)
    vmlal.u8    q11, d17, d6                @mul (row 1)

    vtbl.8      d15, {d0,d1}, d9            @load from ref_main_idx + 1 (row 2)
    vadd.s8     d4, d4, d3                  @ref_main_idx (row 3)
    vadd.s8     d5, d5, d3                  @ref_main_idx + 1 (row 3)

    vst1.8      d24, [r2], r3               @st (row 0)
    vrshrn.i16  d22, q11, #5                @round shft (row 1)

    vtbl.8      d10, {d0,d1}, d4            @load from ref_main_idx (row 3)
    vmull.u8    q10, d14, d7                @mul (row 2)
    vmlal.u8    q10, d15, d6                @mul (row 2)

    vtbl.8      d11, {d0,d1}, d5            @load from ref_main_idx + 1 (row 3)
    vadd.s8     d8, d8, d3                  @ref_main_idx (row 4)
    vadd.s8     d9, d9, d3                  @ref_main_idx + 1 (row 4)

    vst1.8      d22, [r2], r3               @st (row 1)
    vrshrn.i16  d20, q10, #5                @round shft (row 2)

    vtbl.8      d12, {d0,d1}, d8            @load from ref_main_idx (row 4)
    vmull.u8    q9, d10, d7                 @mul (row 3)
    vmlal.u8    q9, d11, d6                 @mul (row 3)

    vtbl.8      d13, {d0,d1}, d9            @load from ref_main_idx + 1 (row 4)
    vadd.s8     d4, d4, d3                  @ref_main_idx (row 5)
    vadd.s8     d5, d5, d3                  @ref_main_idx + 1 (row 5)

    vst1.8      d20, [r2], r3               @st (row 2)
    vrshrn.i16  d18, q9, #5                 @round shft (row 3)

    vtbl.8      d16, {d0,d1}, d4            @load from ref_main_idx (row 5)
    vmull.u8    q12, d12, d7                @mul (row 4)
    vmlal.u8    q12, d13, d6                @mul (row 4)

    vtbl.8      d17, {d0,d1}, d5            @load from ref_main_idx + 1 (row 5)
    vadd.s8     d8, d8, d3                  @ref_main_idx (row 6)
    vadd.s8     d9, d9, d3                  @ref_main_idx + 1 (row 6)

    vst1.8      d18, [r2], r3               @st (row 3)
    vrshrn.i16  d24, q12, #5                @round shft (row 4)

    vtbl.8      d14, {d0,d1}, d8            @load from ref_main_idx (row 6)
    vmull.u8    q11, d16, d7                @mul (row 5)
    vmlal.u8    q11, d17, d6                @mul (row 5)

    vtbl.8      d15, {d0,d1}, d9            @load from ref_main_idx + 1 (row 6)
    vadd.s8     d4, d4, d3                  @ref_main_idx (row 7)
    vadd.s8     d5, d5, d3                  @ref_main_idx + 1 (row 7)

    vst1.8      d24, [r2], r3               @st (row 4)
    vrshrn.i16  d22, q11, #5                @round shft (row 5)

    vtbl.8      d10, {d0,d1}, d4            @load from ref_main_idx (row 7)
    vmull.u8    q10, d14, d7                @mul (row 6)
    vmlal.u8    q10, d15, d6                @mul (row 6)

    vtbl.8      d11, {d0,d1}, d5            @load from ref_main_idx + 1 (row 7)
    vmull.u8    q9, d10, d7                 @mul (row 7)
    vmlal.u8    q9, d11, d6                 @mul (row 7)

    vst1.8      d22, [r2], r3               @st (row 5)
    vrshrn.i16  d20, q10, #5                @round shft (row 6)
    vrshrn.i16  d18, q9, #5                 @round shft (row 7)

    vst1.8      d20, [r2], r3               @st (row 6)

    subs        r10, r10, #8                @subtract 8 and go to end if 8x8

    vst1.8      d18, [r2], r3               @st (row 7)

    beq         end_func

    subs        r11, r11, #8
    addgt       r8, r8, #4
    addgt       r2, r2, r7
    movle       r8, r12
    suble       r2, r2, r4
    addle       r2, r2, #8
    movle       r11, r4
    ldrle       r14, col_for_intra_luma_addr_2
ulbl_2:
    addle       r14,r14,pc
    addle       r0, r0, #8

    mov         r5,r2
    vld1.8      d31, [r14]!
    vmull.s8    q6, d30, d31                @(col+1)*intra_pred_angle [0:7](col)
    vmovn.s16   d10, q6
    vshr.s16    q6, q6, #5
    vqmovn.s16  d11, q6
    vdup.8      d27, r0                     @row value inc or reset accordingly
    ldr         r9, [r8]
    add         r9, r0, r9
    sub         r9, r9, #1
    vdup.8      d26, r9
    vadd.s8     d8, d27, d11                @ref_main_idx (add row)

    sub         r4,r4,#8

kernel_8_16_32:

    vsub.s8     d8, d8, d26                 @ref_main_idx
    vmov        d26,d10

    subs        r11, r11, #8
    add         r6, r1, r9
    vtbl.8      d10, {d0,d1}, d4            @load from ref_main_idx (row 7)
    vadd.s8     d9, d2, d8                  @ref_main_idx + 1

    vmull.u8    q10, d14, d7                @mul (row 6)
    vtbl.8      d11, {d0,d1}, d5            @load from ref_main_idx + 1 (row 7)
    vmlal.u8    q10, d15, d6                @mul (row 6)

    addle       r0, r0, #8
    addgt       r8, r8, #4
    vld1.8      {d0,d1}, [r6]               @stores the 32 values reqd based on indices values (from least idx)

    vst1.8      d24, [r5], r3               @st (row 4)
    vrshrn.i16  d24, q11, #5                @round shft (row 5)

    ldrle       r14, col_for_intra_luma_addr_3
ulbl_3:
    addle       r14,r14,pc
    movle       r8, r12
    vdup.8      d27, r0                     @row value inc or reset accordingly

    vadd.s8     d4, d2, d8                  @ref_main_idx (row 1)
    vtbl.8      d12, {d0,d1}, d8            @load from ref_main_idx (row 0)
    vadd.s8     d5, d2, d9                  @ref_main_idx + 1 (row 1)


    vmull.u8    q9, d10, d7                 @mul (row 7)
    vtbl.8      d13, {d0,d1}, d9            @load from ref_main_idx + 1 (row 0)
    vmlal.u8    q9, d11, d6                 @mul (row 7)

    vld1.8      d31, [r14]!
    vand        d6, d29, d26                @fract values in d1/ idx values in d0

    vst1.8      d24, [r5], r3               @(from previous loop)st (row 5)
    vrshrn.i16  d20, q10, #5                @(from previous loop)round shft (row 6)

    vadd.s8     d8, d3, d8                  @ref_main_idx (row 2)
    vtbl.8      d16, {d0,d1}, d4            @load from ref_main_idx (row 1)
    vadd.s8     d9, d3, d9                  @ref_main_idx + 1 (row 2)

    addle       r11, r4, #8
    ldr         r9, [r8]
    vsub.s8     d7, d28, d6                 @32-fract

    vmull.u8    q12, d12, d7                @mul (row 0)
    vtbl.8      d17, {d0,d1}, d5            @load from ref_main_idx + 1 (row 1)
    vmlal.u8    q12, d13, d6                @mul (row 0)

    vst1.8      d20, [r5], r3               @(from previous loop)st (row 6)
    vrshrn.i16  d18, q9, #5                 @(from previous loop)round shft (row 7)

    vadd.s8     d4, d4, d3                  @ref_main_idx (row 3)
    vtbl.8      d14, {d0,d1}, d8            @load from ref_main_idx (row 2)
    vadd.s8     d5, d5, d3                  @ref_main_idx + 1 (row 3)

    vmull.u8    q11, d16, d7                @mul (row 1)
    vtbl.8      d15, {d0,d1}, d9            @load from ref_main_idx + 1 (row 2)
    vmlal.u8    q11, d17, d6                @mul (row 1)

    vrshrn.i16  d24, q12, #5                @round shft (row 0)
    vst1.8      d18, [r5], r3               @(from previous loop)st (row 7)

    vadd.s8     d8, d8, d3                  @ref_main_idx (row 4)
    vtbl.8      d10, {d0,d1}, d4            @load from ref_main_idx (row 3)
    vadd.s8     d9, d9, d3                  @ref_main_idx + 1 (row 4)

    vmull.u8    q10, d14, d7                @mul (row 2)
    vtbl.8      d11, {d0,d1}, d5            @load from ref_main_idx + 1 (row 3)
    vmlal.u8    q10, d15, d6                @mul (row 2)

    vmull.s8    q7, d30, d31                @(col+1)*intra_pred_angle [0:7](col)
    add         r5,r2,r3,lsl#2
    add         r9, r0, r9


    vst1.8      d24, [r2], r3               @st (row 0)
    vrshrn.i16  d22, q11, #5                @round shft (row 1)

    vadd.s8     d4, d4, d3                  @ref_main_idx (row 5)
    vtbl.8      d12, {d0,d1}, d8            @load from ref_main_idx (row 4)
    vadd.s8     d5, d5, d3                  @ref_main_idx + 1 (row 5)

    vmull.u8    q9, d10, d7                 @mul (row 3)
    vtbl.8      d13, {d0,d1}, d9            @load from ref_main_idx + 1 (row 4)
    vmlal.u8    q9, d11, d6                 @mul (row 3)

    vst1.8      d22, [r2], r3               @st (row 1)
    vrshrn.i16  d20, q10, #5                @round shft (row 2)

    vmovn.s16   d10, q7
    vshr.s16    q7, q7, #5

    vadd.s8     d8, d8, d3                  @ref_main_idx (row 6)
    vtbl.8      d16, {d0,d1}, d4            @load from ref_main_idx (row 5)
    vadd.s8     d9, d9, d3                  @ref_main_idx + 1 (row 6)

    vmull.u8    q12, d12, d7                @mul (row 4)
    vtbl.8      d17, {d0,d1}, d5            @load from ref_main_idx + 1 (row 5)
    vmlal.u8    q12, d13, d6                @mul (row 4)

    vst1.8      d20, [r2], r3               @st (row 2)
    vrshrn.i16  d18, q9, #5                 @round shft (row 3)

    sub         r9, r9, #1
    vqmovn.s16  d11, q7

    vadd.s8     d4, d4, d3                  @ref_main_idx (row 7)
    vtbl.8      d14, {d0,d1}, d8            @load from ref_main_idx (row 6)
    vadd.s8     d5, d5, d3                  @ref_main_idx + 1 (row 7)

    vmull.u8    q11, d16, d7                @mul (row 5)
    vtbl.8      d15, {d0,d1}, d9            @load from ref_main_idx + 1 (row 6)
    vmlal.u8    q11, d17, d6                @mul (row 5)

    vadd.s8     d8, d27, d11                @ref_main_idx (add row)
    vdup.8      d26, r9

    vst1.8      d18, [r2], r3               @st (row 3)
    vrshrn.i16  d24, q12, #5                @round shft (row 4)


    add         r2,r3, lsl #2
    addgt       r2, r7, r2
    suble       r2, r2, r4

    subs        r10, r10, #8                @subtract 8 and go to end if 8x8

    bne         kernel_8_16_32
epil_8_16_32:

    vtbl.8      d10, {d0,d1}, d4            @load from ref_main_idx (row 7)

    vmull.u8    q10, d14, d7                @mul (row 6)
    vtbl.8      d11, {d0,d1}, d5            @load from ref_main_idx + 1 (row 7)
    vmlal.u8    q10, d15, d6                @mul (row 6)

    vst1.8      d24, [r5], r3               @st (row 4)
    vrshrn.i16  d24, q11, #5                @round shft (row 5)

    vmull.u8    q9, d10, d7                 @mul (row 7)
    vmlal.u8    q9, d11, d6                 @mul (row 7)

    vst1.8      d24, [r5], r3               @(from previous loop)st (row 5)
    vrshrn.i16  d20, q10, #5                @(from previous loop)round shft (row 6)

    vst1.8      d20, [r5], r3               @(from previous loop)st (row 6)
    vrshrn.i16  d18, q9, #5                 @(from previous loop)round shft (row 7)

    vst1.8      d18, [r5], r3               @st (row 7)


    b           end_func

sz_4_proc:
    ldr         r14, col_for_intra_luma_addr_4
ulbl_4:
    add         r14,r14,pc

    vld1.8      d31, [r14]
    mov         r12, #1

    vdup.8      d2, r12                     @contains #1 for adding to get ref_main_idx + 1
    mov         r0, #2

    vdup.8      d3, r0
    ldr         r12, idx_neg_idx_11_17_addr_2 @load least idx table
ulbl4:
    add         r12,r12,pc

    vmull.s8    q11, d30, d31               @(col+1)*intra_pred_angle [0:7](col)
    sub         r7, r5, #11

    add         r12, r12, r7, lsl #4
    mov         r8, r12

    ldr         r9, [r8]

    vdup.8      d26, r9                     @least idx added to final idx values
    add         r6, sp, r4                  @ref_temp + nt

    sub         r6, r6, #1                  @ref_temp + nt - 1
    vmovn.s16   d6, q11
    add         r6, r6, r9

    vld1.8      {d0,d1}, [r6]               @stores the 32 values reqd based on indices values (from least idx)
    mov         r0, #31

    vdup.8      d29, r0                     @contains #31 for vand operation
    mov         r1, #32

    vdup.8      d28, r1

    vshr.s16    q11, q11, #5
    vqmovn.s16  d8, q11

    vand        d6, d6, d29                 @fract values in d1/ idx values in d0
    vsub.s8     d7, d28, d6                 @32-fract

    vadd.s8     d8, d8, d2                  @ref_main_idx (add 1)
    vsub.s8     d8, d8, d26                 @ref_main_idx
    vadd.s8     d9, d8, d2                  @ref_main_idx + 1

    vadd.s8     d4, d8, d2                  @row 1 ref_main_idx
    vadd.s8     d5, d9, d2

    vtbl.8      d12, {d0,d1}, d8            @load from ref_main_idx (row 0)
    vtbl.8      d13, {d0,d1}, d9            @load from ref_main_idx + 1 (row 0)


    vmull.u8    q12, d12, d7                @mul (row 0)
    vtbl.8      d16, {d0,d1}, d4            @load from ref_main_idx (row 1)
    vmlal.u8    q12, d13, d6                @mul (row 0)

    vadd.s8     d8, d8, d3                  @idx (row 2)
    vtbl.8      d17, {d0,d1}, d5            @load from ref_main_idx + 1 (row 1)
    vadd.s8     d9, d9, d3                  @idx+1 (row 2)

    vmull.u8    q11, d16, d7                @mul (row 1)
    vtbl.8      d12, {d0,d1}, d8            @load from ref_main_idx (row 2)
    vmlal.u8    q11, d17, d6                @mul (row 1)

    vrshrn.i16  d24, q12, #5                @round shift (row 0)

    vadd.s8     d4, d4, d3                  @idx (row 3)
    vtbl.8      d13, {d0,d1}, d9            @load from ref_main_idx + 1 (row 2)
    vadd.s8     d5, d5, d3                  @idx+1 (row 3)

    vmull.u8    q10, d12, d7                @mul (row 2)
    vtbl.8      d16, {d0,d1}, d4            @load from ref_main_idx (row 3)
    vmlal.u8    q10, d13, d6                @mul (row 2)

    vst1.32     d24[0], [r2], r3            @st row 0
    vrshrn.i16  d22, q11, #5                @round shift (row 1)

    vtbl.8      d17, {d0,d1}, d5            @load from ref_main_idx + 1 (row 3)

    vmull.u8    q9, d16, d7                 @mul (row 3)
    vmlal.u8    q9, d17, d6                 @mul (row 3)

    vst1.32     d22[0], [r2], r3            @st row 1
    vrshrn.i16  d20, q10, #5                @round shift (row 2)

    vst1.32     d20[0], [r2], r3            @st row 2

    vrshrn.i16  d18, q9, #5                 @round shift (row 3)

    vst1.32     d18[0], [r2], r3            @st (row 3)

end_func:
    add         sp, sp, #132
    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp






