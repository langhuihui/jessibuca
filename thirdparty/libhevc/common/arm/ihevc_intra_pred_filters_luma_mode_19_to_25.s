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
@*  ihevc_intra_pred_luma_mode_19_to_25.s
@*
@* @brief
@*  contains function definitions for intra prediction dc filtering.
@* functions are coded using neon  intrinsics and can be compiled using

@* rvct
@*
@* @author
@*  naveen sr
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

@void ihevc_intra_pred_luma_mode_19_to_25(uword8* pu1_ref,
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




.globl ihevc_intra_pred_luma_mode_19_to_25_a9q
.extern gai4_ihevc_ang_table
.extern gai4_ihevc_inv_ang_table
.extern gau1_ihevc_planar_factor

gai4_ihevc_inv_ang_table_addr:
.long gai4_ihevc_inv_ang_table - ulbl1 - 8

gau1_ihevc_planar_factor_addr:
.long gau1_ihevc_planar_factor - ulbl2 - 8

gai4_ihevc_ang_table_addr_1:
.long gai4_ihevc_ang_table - ulbl_1 - 8

gai4_ihevc_ang_table_addr_2:
.long gai4_ihevc_ang_table - ulbl_2 - 8

.type ihevc_intra_pred_luma_mode_19_to_25_a9q, %function

ihevc_intra_pred_luma_mode_19_to_25_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8 - d15}
    sub         sp, sp, #132                @ref_temp[2 * max_cu_size + 1]

    ldr         r4,[sp,#nt_offset]          @loads nt
    ldr         r7, gai4_ihevc_ang_table_addr_1
ulbl_1:
    add         r7,r7,pc

    ldr         r5,[sp,#mode_offset]        @mode (19 to 25)
    ldr         r8, gai4_ihevc_inv_ang_table_addr
ulbl1:
    add         r8,r8,pc

    add         r7, r7, r5, lsl #2          @gai4_ihevc_ang_table[mode]
    add         r8, r8, r5, lsl #2          @gai4_ihevc_inv_ang_table
    sub         r8, r8, #48                 @gai4_ihevc_inv_ang_table[mode - 12]

    ldr         r7, [r7]                    @intra_pred_ang

    ldr         r8, [r8]                    @inv_ang
    add         r6, sp, r4                  @ref_temp + nt

    mul         r9, r4, r7                  @nt*intra_pred_ang

    sub         r6, r6, #1                  @ref_temp + nt - 1

    add         r1, r0, r4, lsl #1          @r1 = &src[2nt]
    vdup.8      d30, r7                     @intra_pred_ang

    mov         r7, r4

    asr         r9, r9, #5

    vld1.32     d0[0],[r1]!                 @ pu1_ref[two_nt + k]

    vst1.32     d0[0],[r6]!                 @ref_temp[k + nt - 1] = pu1_ref[two_nt + k]@

    subs        r7, r7, #4
    beq         end_loop_copy
    sub         r1,#4
    sub         r6,#4
    subs        r7,r7,#4
    beq         loop_copy_8
    subs        r7,r7,#8
    beq         loop_copy_16

loop_copy_32:
    vld1.8      d0,[r1]!
    vld1.8      d1,[r1]!
    vld1.8      d2,[r1]!
    vld1.8      d3,[r1]!

    vst1.8      d0,[r6]!
    vst1.8      d1,[r6]!
    vst1.8      d2,[r6]!
    vst1.8      d3,[r6]!
    b           end_loop_copy

loop_copy_16:
    vld1.8      d0,[r1]!
    vld1.8      d1,[r1]!

    vst1.8      d0,[r6]!
    vst1.8      d1,[r6]!
    b           end_loop_copy

loop_copy_8:
    vld1.8      d0,[r1]!
    vst1.8      d0,[r6]!

end_loop_copy:

    ldrb        r11, [r1]
    strb        r11, [r6]

    cmp         r9, #-1
    bge         linear_filtering

    add         r6, sp, r4                  @ref_temp + nt
    sub         r6, r6, #2                  @ref_temp + nt - 2

    mov         r12, #0xffffffff

    rsb         r9, r9, r12                 @count to take care off ref_idx

    add         r1, r0, r4, lsl #1          @r1 = &src[2nt]

    mov         r7, #128                    @inv_ang_sum

loop_copy_ref_idx:

    add         r7, r7, r8                  @inv_ang_sum += inv_ang
    mov         r14,r7,lsr #8
    ldrb        r11, [r1, -r14]
@   ldrb        r11, [r1, -r7, lsr #8]
    strb        r11, [r6], #-1

    subs        r9, r9, #1

    bne         loop_copy_ref_idx


linear_filtering:
@   after copy
@   below code is taken from mode 27 to 33 and modified

    ldr         r6,gai4_ihevc_ang_table_addr_2 @loads word32 gai4_ihevc_ang_table[35]
ulbl_2:
    add         r6,r6,pc

    add         r8,r6,r5,lsl #2             @*gai4_ihevc_ang_table[mode]
    ldr         r9,[r8]                     @intra_pred_ang = gai4_ihevc_ang_table[mode]
    ldr         r1,gau1_ihevc_planar_factor_addr @used for ((row + 1) * intra_pred_ang) row values
ulbl2:
    add         r1,r1,pc
    add         r6,r1,#1

    add         r8, sp, r4                  @ref_temp + nt
    sub         r8,#1                       @ref_temp + nt -1

    tst         r4,#7
    mov         lr,#0                       @row
    mov         r12,r4
    bne         core_loop_4

core_loop_8:
    add         r8,r8,#1                    @pu1_ref_main_idx += (two_nt + 1)
    vdup.8      d0,r9                       @intra_pred_ang
    mov         r12,r4,lsr #3               @divide by 8

    vmov.i8     d1,#32
    mul         r7,r4,r12

    vmov.i16    q3,#31
    @lsl            r12,r3,#3

    mov         r1,r8
    @sub            r12,r12,r4
    mov         r5,r4
    mov         r11,#1

prologue:
    vld1.8      {d3},[r6]                   @loads the row value
    vmull.s8    q1,d3,d0                    @pos = ((row + 1) * intra_pred_ang)
    vand        q2,q1,q3                    @dup_const_fract(fract = pos & (31))
    vmovn.i16   d4,q2
    vshrn.s16   d5,q1,#5                    @idx = pos >> 5

    vdup.8      d31,d4[0]
    add         r0,r2,r3

    vmov.u32    lr,d5[0]                    @(i row)extract idx to the r register

    vdup.8      d29,d4[1]                   @(ii)
    sbfx        r9,lr,#0,#8

    add         r10,r8,r9                   @(i row)*pu1_ref[ref_main_idx]

    vld1.8      {d8},[r10],r11              @(i row)ref_main_idx
    sbfx        r9,lr,#8,#8

    vld1.8      {d9},[r10]                  @(i row)ref_main_idx_1
    add         r12,r8,r9                   @(ii)*pu1_ref[ref_main_idx]

    sbfx        r9,lr,#16,#8
    vsub.u8     d30,d1,d31                  @32-fract(dup_const_32_fract)
    add         r10,r8,r9                   @(iii)*pu1_ref[ref_main_idx]

    vld1.8      {d12},[r12],r11             @(ii)ref_main_idx
    vmull.u8    q5,d8,d30                   @(i row)vmull_u8(ref_main_idx, dup_const_32_fract)

    vld1.8      {d13},[r12]                 @(ii)ref_main_idx_1
    vmlal.u8    q5,d9,d31                   @(i row)vmull_u8(ref_main_idx_1, dup_const_fract)

    vdup.8      d27,d4[2]                   @(iii)
    vsub.u8     d28,d1,d29                  @(ii)32-fract(dup_const_32_fract)
    sbfx        r9,lr,#24,#8

    vdup.8      d25,d4[3]                   @(iv)
    vmull.u8    q7,d12,d28                  @(ii)vmull_u8(ref_main_idx, dup_const_32_fract)
    add         r12,r8,r9                   @(iv)*pu1_ref[ref_main_idx]

    vld1.8      {d16},[r10],r11             @(iii)ref_main_idx
    vmlal.u8    q7,d13,d29                  @(ii)vmull_u8(ref_main_idx_1, dup_const_fract)

    vld1.8      {d17},[r10]                 @(iii)ref_main_idx_1
    vrshrn.i16  d10,q5,#5                   @(i row)shift_res = vrshrn_n_u16(add_res, 5)

    vld1.8      {d20},[r12],r11             @(iv)ref_main_idx
    vsub.u8     d26,d1,d27                  @(iii)32-fract(dup_const_32_fract)

    vld1.8      {d21},[r12]                 @(iv)ref_main_idx_1

    vdup.8      d31,d4[4]                   @(v)
    vmull.u8    q9,d16,d26                  @(iii)vmull_u8(ref_main_idx, dup_const_32_fract)

    vmov.u32    lr,d5[1]                    @extract idx to the r register
    vmlal.u8    q9,d17,d27                  @(iii)vmull_u8(ref_main_idx_1, dup_const_fract)

    vst1.8      {d10},[r2]!                 @(i row)
    vrshrn.i16  d14,q7,#5                   @(ii)shift_res = vrshrn_n_u16(add_res, 5)

    sbfx        r9,lr,#0,#8
    vdup.8      d29,d4[5]                   @(vi)
    add         r10,r8,r9                   @(v)*pu1_ref[ref_main_idx]

    vld1.8      {d8},[r10],r11              @(v)ref_main_idx
    vsub.u8     d24,d1,d25                  @(iv)32-fract(dup_const_32_fract)

    vmull.u8    q11,d20,d24                 @(iv)vmull_u8(ref_main_idx, dup_const_32_fract)
    sbfx        r9,lr,#8,#8

    vld1.8      {d9},[r10]                  @(v)ref_main_idx_1
    vmlal.u8    q11,d21,d25                 @(iv)vmull_u8(ref_main_idx_1, dup_const_fract)

    vst1.8      {d14},[r0],r3               @(ii)
    vrshrn.i16  d18,q9,#5                   @(iii)shift_res = vrshrn_n_u16(add_res, 5)

    add         r12,r8,r9                   @(vi)*pu1_ref[ref_main_idx]
    vdup.8      d27,d4[6]                   @(vii)

    sbfx        r9,lr,#16,#8
    vsub.u8     d30,d1,d31                  @(v)32-fract(dup_const_32_fract)
    add         r10,r8,r9                   @(vii)*pu1_ref[ref_main_idx]

    vld1.8      {d12},[r12],r11             @(vi)ref_main_idx
    vmull.u8    q5,d8,d30                   @(v)vmull_u8(ref_main_idx, dup_const_32_fract)

    vld1.8      {d13},[r12]                 @(vi)ref_main_idx_1
    vmlal.u8    q5,d9,d31                   @(v)vmull_u8(ref_main_idx_1, dup_const_fract)

    vst1.8      {d18},[r0],r3               @(iii)
    vrshrn.i16  d22,q11,#5                  @(iv)shift_res = vrshrn_n_u16(add_res, 5)

    vdup.8      d25,d4[7]                   @(viii)
    sbfx        r9,lr,#24,#8

    vld1.8      {d16},[r10],r11             @(vii)ref_main_idx
    vsub.u8     d28,d1,d29                  @(vi)32-fract(dup_const_32_fract)

    vld1.8      {d17},[r10]                 @(vii)ref_main_idx_1
    vmull.u8    q7,d12,d28                  @(vi)vmull_u8(ref_main_idx, dup_const_32_fract)

    add         r12,r8,r9                   @(viii)*pu1_ref[ref_main_idx]
    vmlal.u8    q7,d13,d29                  @(vi)vmull_u8(ref_main_idx_1, dup_const_fract)
    subs        r4,r4,#8

    vst1.8      {d22},[r0],r3               @(iv)
    vrshrn.i16  d10,q5,#5                   @(v)shift_res = vrshrn_n_u16(add_res, 5)

    vld1.8      {d20},[r12],r11             @(viii)ref_main_idx
    vsub.u8     d26,d1,d27                  @(vii)32-fract(dup_const_32_fract)

    vld1.8      {d21},[r12]                 @(viii)ref_main_idx_1
    vmull.u8    q9,d16,d26                  @(vii)vmull_u8(ref_main_idx, dup_const_32_fract)

    addgt       r8,r8,#8
    vmlal.u8    q9,d17,d27                  @(vii)vmull_u8(ref_main_idx_1, dup_const_fract)
    subgt       r7,r7,#8

    vst1.8      {d10},[r0],r3               @(v)
    vrshrn.i16  d14,q7,#5                   @(vi)shift_res = vrshrn_n_u16(add_res, 5)

    beq         epilogue

    vld1.8      {d5},[r6]                   @loads the row value
    vmull.s8    q1,d5,d0                    @pos = ((row + 1) * intra_pred_ang)
    vand        q2,q1,q3                    @dup_const_fract(fract = pos & (31))
    vmovn.i16   d4,q2
    vshrn.s16   d3,q1,#5                    @idx = pos >> 5
    vmov.u32    lr,d3[0]                    @(i)extract idx to the r register
    sbfx        r9,lr,#0,#8
    add         r10,r8,r9                   @(i)*pu1_ref[ref_main_idx]

kernel_8_rows:
    vdup.8      d31,d4[0]
    subs        r4,r4,#8
    sbfx        r9,lr,#8,#8

    vld1.8      {d8},[r10],r11              @(i)ref_main_idx
    vsub.u8     d24,d1,d25                  @(viii)32-fract(dup_const_32_fract)

    addle       r6,r6,#8                    @increment the row value
    add         r12,r8,r9                   @(ii)*pu1_ref[ref_main_idx]

    vld1.8      {d9},[r10]                  @(i)ref_main_idx_1
    vmull.u8    q11,d20,d24                 @(viii)vmull_u8(ref_main_idx, dup_const_32_fract)

    vld1.8      {d5},[r6]                   @loads the row value
    vmlal.u8    q11,d21,d25                 @(viii)vmull_u8(ref_main_idx_1, dup_const_fract)

    vdup.8      d29,d4[1]                   @(ii)
    vrshrn.i16  d18,q9,#5                   @(vii)shift_res = vrshrn_n_u16(add_res, 5)

    sbfx        r9,lr,#16,#8

    vst1.8      {d14},[r0],r3               @(vi)
    vsub.u8     d30,d1,d31                  @(i)32-fract(dup_const_32_fract)

    add         r10,r8,r9                   @(iii)*pu1_ref[ref_main_idx]

    vld1.8      {d12},[r12],r11             @(ii)ref_main_idx
    vmull.u8    q5,d8,d30                   @(i)vmull_u8(ref_main_idx, dup_const_32_fract)

    vld1.8      {d13},[r12]                 @(ii)ref_main_idx_1
    vmlal.u8    q5,d9,d31                   @(i)vmull_u8(ref_main_idx_1, dup_const_fract)

    sbfx        r9,lr,#24,#8
    movle       r4,r5                       @reload nt

    vmov.u32    lr,d3[1]                    @extract idx to the r register
    vrshrn.i16  d22,q11,#5                  @(viii)shift_res = vrshrn_n_u16(add_res, 5)

    vdup.8      d27,d4[2]                   @(iii)
    vsub.u8     d28,d1,d29                  @(ii)32-fract(dup_const_32_fract)
    add         r12,r8,r9                   @(iv)*pu1_ref[ref_main_idx]

    vld1.8      {d16},[r10],r11             @(iii)ref_main_idx
    vmull.u8    q7,d12,d28                  @(ii)vmull_u8(ref_main_idx, dup_const_32_fract)

    vst1.8      {d18},[r0],r3               @(vii)
    vmlal.u8    q7,d13,d29                  @(ii)vmull_u8(ref_main_idx_1, dup_const_fract)

    vld1.8      {d17},[r10]                 @(iii)ref_main_idx_1
    vrshrn.i16  d10,q5,#5                   @(i)shift_res = vrshrn_n_u16(add_res, 5)

    vdup.8      d25,d4[3]                   @(iv)
    vmull.s8    q1,d5,d0                    @pos = ((row + 1) * intra_pred_ang)

    vst1.8      {d22},[r0]                  @(viii)
    vsub.u8     d26,d1,d27                  @(iii)32-fract(dup_const_32_fract)

    vld1.8      {d20},[r12],r11             @(iv)ref_main_idx
    vmull.u8    q9,d16,d26                  @(iii)vmull_u8(ref_main_idx, dup_const_32_fract)

    vld1.8      {d21},[r12]                 @(iv)ref_main_idx_1
    vmlal.u8    q9,d17,d27                  @(iii)vmull_u8(ref_main_idx_1, dup_const_fract)

    sbfx        r9,lr,#0,#8
    add         r0,r2,r3

    vdup.8      d31,d4[4]                   @(v)
    vrshrn.i16  d14,q7,#5                   @(ii)shift_res = vrshrn_n_u16(add_res, 5)

    add         r10,r8,r9                   @(v)*pu1_ref[ref_main_idx]
    sbfx        r9,lr,#8,#8

    vst1.8      {d10},[r2]!                 @(i)
    vsub.u8     d24,d1,d25                  @(iv)32-fract(dup_const_32_fract)

    vdup.8      d29,d4[5]                   @(vi)
    vmull.u8    q11,d20,d24                 @(iv)vmull_u8(ref_main_idx, dup_const_32_fract)

    vdup.8      d27,d4[6]                   @(vii)
    vmlal.u8    q11,d21,d25                 @(iv)vmull_u8(ref_main_idx_1, dup_const_fract)

    add         r12,r8,r9                   @(vi)*pu1_ref[ref_main_idx]
    sbfx        r9,lr,#16,#8

    vdup.8      d25,d4[7]                   @(viii)
    vrshrn.i16  d18,q9,#5                   @(iii)shift_res = vrshrn_n_u16(add_res, 5)

    vld1.8      {d8},[r10],r11              @(v)ref_main_idx
    vand        q2,q1,q3                    @dup_const_fract(fract = pos & (31))

    vld1.8      {d9},[r10]                  @(v)ref_main_idx_1
    vshrn.s16   d3,q1,#5                    @idx = pos >> 5

    vst1.8      {d14},[r0],r3               @(ii)
    vrshrn.i16  d22,q11,#5                  @(iv)shift_res = vrshrn_n_u16(add_res, 5)

    add         r10,r8,r9                   @(vii)*pu1_ref[ref_main_idx]
    sbfx        r9,lr,#24,#8

    vld1.8      {d12},[r12],r11             @(vi)ref_main_idx
    vsub.u8     d30,d1,d31                  @(v)32-fract(dup_const_32_fract)

    vld1.8      {d13},[r12]                 @(vi)ref_main_idx_1
    vmull.u8    q5,d8,d30                   @(v)vmull_u8(ref_main_idx, dup_const_32_fract)

    vmov.u32    lr,d3[0]                    @(i)extract idx to the r register
    vmlal.u8    q5,d9,d31                   @(v)vmull_u8(ref_main_idx_1, dup_const_fract)

    add         r12,r8,r9                   @(viii)*pu1_ref[ref_main_idx]
    movle       r8,r1                       @reload the source to pu1_src+2nt

    vld1.8      {d16},[r10],r11             @(vii)ref_main_idx
    vsub.u8     d28,d1,d29                  @(vi)32-fract(dup_const_32_fract)

    vst1.8      {d18},[r0],r3               @(iii)
    vmull.u8    q7,d12,d28                  @(vi)vmull_u8(ref_main_idx, dup_const_32_fract)

    vld1.8      {d17},[r10]                 @(vii)ref_main_idx_1
    vmlal.u8    q7,d13,d29                  @(vi)vmull_u8(ref_main_idx_1, dup_const_fract)

    vld1.8      {d20},[r12],r11             @(viii)ref_main_idx
    vrshrn.i16  d10,q5,#5                   @(v)shift_res = vrshrn_n_u16(add_res, 5)

    vld1.8      {d21},[r12]                 @(viii)ref_main_idx_1
    vsub.u8     d26,d1,d27                  @(vii)32-fract(dup_const_32_fract)

    addgt       r8,r8,#8                    @increment the source next set 8 columns in same row
    lslle       r12,r3,#3
    suble       r12,r12,r5

    vst1.8      {d22},[r0],r3               @(iv)
    vmull.u8    q9,d16,d26                  @(vii)vmull_u8(ref_main_idx, dup_const_32_fract)

    vst1.8      {d10},[r0],r3               @(v)
    vmlal.u8    q9,d17,d27                  @(vii)vmull_u8(ref_main_idx_1, dup_const_fract)

    addle       r2,r2,r12                   @increment the dst pointer to 8*dst_strd - nt
    sbfx        r9,lr,#0,#8

    vmovn.i16   d4,q2
    vrshrn.i16  d14,q7,#5                   @(vi)shift_res = vrshrn_n_u16(add_res, 5)

    subs        r7,r7,#8
    add         r10,r8,r9                   @(i)*pu1_ref[ref_main_idx]

    bne         kernel_8_rows

epilogue:
    vst1.8      {d14},[r0],r3               @(vi)
    vrshrn.i16  d18,q9,#5                   @(vii)shift_res = vrshrn_n_u16(add_res, 5)

    vsub.u8     d24,d1,d25                  @(viii)32-fract(dup_const_32_fract)
    vmull.u8    q11,d20,d24                 @(viii)vmull_u8(ref_main_idx, dup_const_32_fract)
    vmlal.u8    q11,d21,d25                 @(viii)vmull_u8(ref_main_idx_1, dup_const_fract)

    vst1.8      {d18},[r0],r3               @(vii)
    vrshrn.i16  d22,q11,#5                  @(viii)shift_res = vrshrn_n_u16(add_res, 5)

    vst1.8      {d22},[r0],r3               @(viii)
    b           end_loops

core_loop_4:
    add         r6,r8,#1                    @pu1_ref_main_idx +=  1
    mov         r8,#0

    add         r5,r8,#1                    @row + 1
    mul         r5,r5,r9                    @pos = ((row + 1) * intra_pred_ang)
    mov         lr,r5,asr #5                @if(fract_prev > fract)
    and         r5,r5,#31                   @fract = pos & (31)
    add         r10,r6,lr                   @pu1_ref_main_idx += 1
    add         r11,r10,#1                  @pu1_ref_main_idx_1 += 1
    vdup.8      d0,r5                       @dup_const_fract
    rsb         r4,r5,#32
    vdup.8      d1,r4                       @dup_const_32_fract

@inner_loop_4
    vld1.32     {d2[0]},[r10]               @ref_main_idx
    add         r8,r8,#1
@   mov         lr,r5                           @fract_prev = fract

    vld1.32     {d3[0]},[r11]               @ref_main_idx_1
    add         r5,r8,#1                    @row + 1
    mul         r5,r5,r9                    @pos = ((row + 1) * intra_pred_ang)
    mov         lr,r5,asr #5                @ pos >> 5
    and         r5,r5,#31                   @fract = pos & (31)
    add         r10,r6,lr                   @pu1_ref_main_idx += 1
    add         r11,r10,#1                  @pu1_ref_main_idx_1 += 1

    vdup.8      d6,r5                       @dup_const_fract
    vmull.u8    q2,d2,d1                    @vmull_u8(ref_main_idx, dup_const_32_fract)

    rsb         r4,r5,#32
    vdup.8      d7,r4                       @dup_const_32_fract
    vmlal.u8    q2,d3,d0                    @vmull_u8(ref_main_idx_1, dup_const_fract)

    vld1.32     {d8[0]},[r10]               @ref_main_idx
    add         r8,r8,#1

    vld1.32     {d9[0]},[r11]               @ref_main_idx_1
    vrshrn.i16  d4,q2,#5                    @shift_res = vrshrn_n_u16(add_res, 5)

@   mov         lr,r5                           @fract_prev = fract
    add         r5,r8,#1                    @row + 1
    mul         r5,r5,r9                    @pos = ((row + 1) * intra_pred_ang)
    mov         lr,r5,asr #5                @if(fract_prev > fract)
    and         r5,r5,#31                   @fract = pos & (31)
    add         r10,r6,lr                   @ref_main + idx
    add         r11,r10,#1                  @pu1_ref_main_idx_1 += 1

    vdup.8      d12,r5                      @dup_const_fract
    vmull.u8    q5,d8,d7                    @vmull_u8(ref_main_idx, dup_const_32_fract)

    rsb         r4,r5,#32
    vdup.8      d13,r4                      @dup_const_32_fract
    vmlal.u8    q5,d9,d6                    @vmull_u8(ref_main_idx_1, dup_const_fract)

    vld1.32     {d14[0]},[r10]              @ref_main_idx
    add         r8,r8,#1

    vst1.32     {d4[0]},[r2],r3
    vrshrn.i16  d10,q5,#5                   @shift_res = vrshrn_n_u16(add_res, 5)

    vld1.32     {d15[0]},[r11]              @ref_main_idx_1
@   mov         lr,r5                           @fract_prev = fract
    add         r5,r8,#1                    @row + 1
    mul         r5,r5,r9                    @pos = ((row + 1) * intra_pred_ang)
    mov         lr,r5,asr #5                @if(fract_prev > fract)
    and         r5,r5,#31                   @fract = pos & (31)
    add         r10,r6,lr                   @pu1_ref_main_idx += 1
    add         r11,r10,#1                  @pu1_ref_main_idx_1 += 1

    vdup.8      d18,r5                      @dup_const_fract
    vmull.u8    q8,d14,d13                  @vmull_u8(ref_main_idx, dup_const_32_fract)

    rsb         r4,r5,#32
    vdup.8      d19,r4                      @dup_const_32_fract
    vmlal.u8    q8,d15,d12                  @vmull_u8(ref_main_idx_1, dup_const_fract)

    vld1.32     {d20[0]},[r10]              @ref_main_idx

    vst1.32     {d10[0]},[r2],r3
    vrshrn.i16  d16,q8,#5                   @shift_res = vrshrn_n_u16(add_res, 5)
    vld1.32     {d21[0]},[r11]              @ref_main_idx_1

    vmull.u8    q11,d20,d19                 @vmull_u8(ref_main_idx, dup_const_32_fract)
    vmlal.u8    q11,d21,d18                 @vmull_u8(ref_main_idx_1, dup_const_fract)

    vst1.32     {d16[0]},[r2],r3
    vrshrn.i16  d22,q11,#5                  @shift_res = vrshrn_n_u16(add_res, 5)

    vst1.32     {d22[0]},[r2],r3

end_loops:
    add         sp, sp, #132
    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp






