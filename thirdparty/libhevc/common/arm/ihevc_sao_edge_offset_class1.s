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
@* ,:file
@*  ihevc_sao_edge_offset_class1.s
@*
@* ,:brief
@*  Contains function definitions for inter prediction  interpolation.
@* Functions are coded using NEON  intrinsics and can be compiled using@ ARM
@* RVCT
@*
@* ,:author
@*  Parthiban V
@*
@* ,:par List of Functions:
@*
@*
@* ,:remarks
@*  None
@*
@*******************************************************************************
@*/
@void ihevc_sao_edge_offset_class1(UWORD8 *pu1_src,
@                              WORD32 src_strd,
@                              UWORD8 *pu1_src_left,
@                              UWORD8 *pu1_src_top,
@                              UWORD8 *pu1_src_top_left,
@                              UWORD8 *pu1_src_top_right,
@                              UWORD8 *pu1_src_bot_left,
@                              UWORD8 *pu1_avail,
@                              WORD8 *pi1_sao_offset,
@                              WORD32 wd,
@                              WORD32 ht)
@**************Variables Vs Registers*****************************************
@r0 =>  *pu1_src
@r1 =>  src_strd
@r2 =>  *pu1_src_left
@r3 =>  *pu1_src_top
@r4 =>  *pu1_src_top_left
@r5 =>  *pu1_avail
@r6 =>  *pi1_sao_offset
@r7 =>  wd
@r8 =>  ht

.equ    pu1_src_top_left_offset,    104
.equ    pu1_src_top_right_offset,   108
.equ    pu1_src_bot_left_offset,    112
.equ    pu1_avail_offset,           116
.equ    pi1_sao_offset,             120
.equ    wd_offset,                  124
.equ    ht_offset,                  128

.text
.p2align 2

.extern gi1_table_edge_idx
.globl ihevc_sao_edge_offset_class1_a9q

gi1_table_edge_idx_addr:
.long gi1_table_edge_idx - ulbl1 - 8

ihevc_sao_edge_offset_class1_a9q:


    STMFD       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8  -  d15}

    LDR         r7,[sp,#wd_offset]               @Loads wd
    LDR         r4,[sp,#pu1_src_top_left_offset] @Loads pu1_src_top_left
    LDR         r5,[sp,#pu1_avail_offset]        @Loads pu1_avail
    LDR         r6,[sp,#pi1_sao_offset]          @Loads pi1_sao_offset
    LDR         r8,[sp,#ht_offset]               @Loads ht

    SUB         r9,r7,#1                    @wd - 1
    LDRB        r10,[r3,r9]                 @pu1_src_top[wd - 1]
    STRB        r10,[r4]                    @*pu1_src_top_left = pu1_src_top[wd - 1]
    ADD         r10,r0,r9                   @pu1_src[row * src_strd + wd - 1]
    MOV         r11,r2                      @Move pu1_src_left pointer to r11
    MOV         r12,r8                      @Move ht to r12 for loop count
SRC_LEFT_LOOP:
    LDRB        r14,[r10],r1                @Load pu1_src[row * src_strd + wd - 1]
    STRB        r14,[r11],#1                @pu1_src_left[row]
    SUBS        r12,#1                      @Decrement the loop count
    BNE         SRC_LEFT_LOOP               @If not equal to 0 jump to the src_left_loop

    SUB         r12,r8,#1                   @ht - 1
    MUL         r12,r12,r1                  @(ht - 1) * src_strd
    ADD         r12,r12,r0                  @pu1_src[(ht - 1) * src_strd]

    LDRB        r4,[r5,#2]                  @pu1_avail[2]
    CMP         r4,#0                       @0 == pu1_avail[2]
    ADDEQ       r0,r0,r1                    @pu1_src += src_strd
    SUBEQ       r8,r8,#1                    @ht--

    LDRB        r4,[r5,#3]                  @pu1_avail[3]
    CMP         r4,#0                       @0 == pu1_avail[3]
    SUBEQ       r8,r8,#1                    @ht--

    VMOV.I8     Q0,#2                       @const_2 = vdupq_n_s8(2)
    VMOV.I16    Q1,#0                       @const_min_clip = vdupq_n_s16(0)
    VMOV.I16    Q2,#255                     @const_max_clip = vdupq_n_u16((1 << bit_depth) - 1)
    LDR         r14, gi1_table_edge_idx_addr @table pointer
ulbl1:
    add         r14,r14,pc
    VLD1.8      D6,[r14]                    @edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    VLD1.8      D7,[r6]                     @offset_tbl = vld1_s8(pi1_sao_offset)

    CMP         r7,#16                      @Compare wd with 16
    BLT         WIDTH_RESIDUE               @If not jump to WIDTH_RESIDUE where loop is unrolled for 8 case

WIDTH_LOOP_16:
    LDRB        r4,[r5,#2]                  @pu1_avail[2]
    CMP         r4,#0                       @0 == pu1_avail[2]
    SUBEQ       r9,r0,r1                    @pu1_src -= src_strd
    MOVNE       r9,r3                       @*pu1_src_top

    MOV         r10,r0                      @*pu1_src

    VLD1.8      D8,[r9]!                    @pu1_top_row = vld1q_u8(pu1_src_top_cpy || pu1_src - src_strd)
    VLD1.8      D9,[r9]!                    @pu1_top_row = vld1q_u8(pu1_src_top_cpy || pu1_src - src_strd)
    VLD1.8      D10,[r0]!                   @pu1_cur_row = vld1q_u8(pu1_src)
    VLD1.8      D11,[r0]!                   @pu1_cur_row = vld1q_u8(pu1_src)

    VLD1.8      D30,[r12]!                  @vld1q_u8(pu1_src[(ht - 1) * src_strd])
    VLD1.8      D31,[r12]!                  @vld1q_u8(pu1_src[(ht - 1) * src_strd])
    VCGT.U8     Q6,Q5,Q4                    @vcgtq_u8(pu1_cur_row, pu1_top_row)

    VST1.8      {Q15},[r3]!                 @vst1q_u8(pu1_src_top[col])
    VCLT.U8     Q7,Q5,Q4                    @vcltq_u8(pu1_cur_row, pu1_top_row)

    VSUB.U8     Q8,Q7,Q6                    @sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    MOV         r11,r8                      @move ht to r11 for loop count

PU1_SRC_LOOP:
    ADD         r10,r10,r1                  @*pu1_src + src_strd
    VLD1.8      D18,[r10]!                  @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D19,[r10]                   @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r10,#8
    ADD         r6,r10,r1                   @II Iteration *pu1_src + src_strd

    VCGT.U8     Q6,Q5,Q9                    @vcgtq_u8(pu1_cur_row, pu1_top_row)
    VLD1.8      D30,[r6]!                   @II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D31,[r6]                    @II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r6,#8

    VCLT.U8     Q7,Q5,Q9                    @vcltq_u8(pu1_cur_row, pu1_top_row)
    SUB         r10,r10,r1

    VSUB.U8     Q10,Q7,Q6                   @sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    VMOVL.U8    Q13,D18                     @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    VADD.I8     Q6,Q0,Q8                    @edge_idx = vaddq_s8(const_2, sign_up)
    VMOVL.U8    Q14,D19                     @II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))

    VADD.I8     Q6,Q6,Q10                   @edge_idx = vaddq_s8(edge_idx, sign_down)
    VCGT.U8     Q11,Q9,Q15                  @II vcgtq_u8(pu1_cur_row, pu1_top_row)

    VNEG.S8     Q8,Q10                      @sign_up = vnegq_s8(sign_down)
    VTBL.8      D12,{D6},D12                @vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VCLT.U8     Q12,Q9,Q15                  @II vcltq_u8(pu1_cur_row, pu1_top_row)

    VSUB.U8     Q4,Q12,Q11                  @II sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    VTBL.8      D13,{D6},D13                @vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    VADD.I8     Q11,Q0,Q8                   @II edge_idx = vaddq_s8(const_2, sign_up)


    VNEG.S8     Q8,Q4                       @II sign_up = vnegq_s8(sign_down)
    VTBL.8      D12,{D7},D12                @offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VADD.I8     Q11,Q11,Q4                  @II edge_idx = vaddq_s8(edge_idx, sign_down)


    VMOVL.U8    Q10,D10                     @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VTBL.8      D22,{D6},D22                @II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VADDW.S8    Q10,Q10,D12                 @pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    VMAX.S16    Q10,Q10,Q1                  @pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VTBL.8      D23,{D6},D23                @II vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    VMIN.U16    Q10,Q10,Q2                  @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))


    VMOVL.U8    Q4,D11                      @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    VTBL.8      D13,{D7},D13                @offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    VMOV        Q5,Q15                      @II pu1_cur_row = pu1_next_row

    VADDW.S8    Q4,Q4,D13                   @pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    VTBL.8      D24,{D7},D22                @II offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VMAX.S16    Q4,Q4,Q1                    @pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)

    VMIN.U16    Q4,Q4,Q2                    @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))
    VTBL.8      D25,{D7},D23                @II offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))

    VMOVN.I16   D20,Q10                     @vmovn_s16(pi2_tmp_cur_row.val[0])
    VADDW.S8    Q13,Q13,D24                 @II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    VMOVN.I16   D21,Q4                      @vmovn_s16(pi2_tmp_cur_row.val[1])
    VADDW.S8    Q14,Q14,D25                 @II pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)


    VMAX.S16    Q13,Q13,Q1                  @II pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VMIN.U16    Q13,Q13,Q2                  @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VMAX.S16    Q14,Q14,Q1                  @II pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    VMIN.U16    Q14,Q14,Q2                  @II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))
    VST1.8      {Q10},[r10],r1              @vst1q_u8(pu1_src_cpy, pu1_cur_row)

    VMOVN.I16   D30,Q13                     @II vmovn_s16(pi2_tmp_cur_row.val[0])
    SUBS        r11,r11,#2                  @II Decrement the ht loop count by 1
    VMOVN.I16   D31,Q14                     @II vmovn_s16(pi2_tmp_cur_row.val[1])

    VST1.8      {Q15},[r10],r1              @II vst1q_u8(pu1_src_cpy, pu1_cur_row)

    BEQ         PU1_SRC_LOOP_END            @if 0 == pu1_avail[3] || 0 == pu1_avail[2] ht = ht--
    CMP         r11,#1                      @checking any residue remains
    BGT         PU1_SRC_LOOP                @If not equal jump to PU1_SRC_LOOP

    ADD         r10,r10,r1                  @*pu1_src + src_strd
    VLD1.8      D18,[r10]!                  @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D19,[r10]                   @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r10,#8
    VCGT.U8     Q6,Q5,Q9                    @vcgtq_u8(pu1_cur_row, pu1_top_row)
    VCLT.U8     Q7,Q5,Q9                    @vcltq_u8(pu1_cur_row, pu1_top_row)
    VSUB.U8     Q10,Q7,Q6                   @sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    SUB         r10,r10,r1

    VADD.I8     Q11,Q0,Q8                   @edge_idx = vaddq_s8(const_2, sign_up)
    VADD.I8     Q11,Q11,Q10                 @edge_idx = vaddq_s8(edge_idx, sign_down)
    VTBL.8      D22,{D6},D22                @vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VTBL.8      D23,{D6},D23                @vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))

    VTBL.8      D24,{D7},D22                @offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VMOVL.U8    Q13,D10                     @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VADDW.S8    Q13,Q13,D24                 @pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    VMAX.S16    Q13,Q13,Q1                  @pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VMIN.U16    Q13,Q13,Q2                  @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VTBL.8      D25,{D7},D23                @offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    VMOVL.U8    Q14,D11                     @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    VADDW.S8    Q14,Q14,D25                 @pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    VMAX.S16    Q14,Q14,Q1                  @pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    VMIN.U16    Q14,Q14,Q2                  @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    VMOVN.I16   D30,Q13                     @vmovn_s16(pi2_tmp_cur_row.val[0])
    VMOVN.I16   D31,Q14                     @vmovn_s16(pi2_tmp_cur_row.val[1])

    VST1.8      {Q15},[r10],r1              @vst1q_u8(pu1_src_cpy, pu1_cur_row)

PU1_SRC_LOOP_END:
    VMOV        Q5,Q9                       @pu1_cur_row = pu1_next_row
    SUBS        r7,r7,#16                   @Decrement the wd loop count by 16
    CMP         r7,#8                       @Check whether residue remains
    BEQ         WIDTH_RESIDUE               @If residue remains jump to residue loop
    BGT         WIDTH_LOOP_16               @If not equal jump to width_loop
    BLT         END_LOOPS                   @Jump to end function


WIDTH_RESIDUE:
    LDRB        r4,[r5,#2]                  @pu1_avail[2]
    CMP         r4,#0                       @0 == pu1_avail[2]
    SUBEQ       r9,r0,r1                    @pu1_src -= src_strd
    MOVNE       r9,r3                       @*pu1_src_top
    MOV         r10,r0

    VLD1.8      D8,[r9]!                    @pu1_top_row = vld1q_u8(pu1_src_top_cpy || pu1_src - src_strd)
    VLD1.8      D9,[r9]!                    @pu1_top_row = vld1q_u8(pu1_src_top_cpy || pu1_src - src_strd)
    VLD1.8      D10,[r0]!                   @pu1_cur_row = vld1q_u8(pu1_src)
    VLD1.8      D11,[r0]!                   @pu1_cur_row = vld1q_u8(pu1_src)

    VLD1.8      D30,[r12]                   @vld1_u8(pu1_src[(ht - 1) * src_strd])
    VST1.8      {D30},[r3]                  @vst1_u8(pu1_src_top[col])

    VCGT.U8     Q6,Q5,Q4                    @vcgtq_u8(pu1_cur_row, pu1_top_row)
    VCLT.U8     Q7,Q5,Q4                    @vcltq_u8(pu1_cur_row, pu1_top_row)
    VSUB.U8     Q8,Q7,Q6                    @sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    MOV         r11,r8                      @move ht to r11 for loop count

PU1_SRC_LOOP_RESIDUE:
    ADD         r10,r10,r1                  @*pu1_src + src_strd
    VLD1.8      D18,[r10]!                  @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D19,[r10]                   @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r10,#8
    ADD         r6,r10,r1                   @II Iteration *pu1_src + src_strd

    VCGT.U8     Q6,Q5,Q9                    @vcgtq_u8(pu1_cur_row, pu1_next_row)
    VLD1.8      D30,[r6]!                   @II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D31,[r6]                    @II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r6,#8

    VCLT.U8     Q7,Q5,Q9                    @vcltq_u8(pu1_cur_row, pu1_next_row)
    SUB         r10,r10,r1

    VSUB.U8     Q10,Q7,Q6                   @sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    VMOVL.U8    Q13,D18                     @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    VADD.I8     Q6,Q0,Q8                    @edge_idx = vaddq_s8(const_2, sign_up)
    VCGT.U8     Q11,Q9,Q15                  @II vcgtq_u8(pu1_cur_row, pu1_next_row)

    VADD.I8     Q6,Q6,Q10                   @edge_idx = vaddq_s8(edge_idx, sign_down)
    VCLT.U8     Q12,Q9,Q15                  @II vcltq_u8(pu1_cur_row, pu1_next_row)

    VNEG.S8     Q8,Q10                      @sign_up = vnegq_s8(sign_down)
    VTBL.8      D12,{D6},D12                @vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VSUB.U8     Q10,Q12,Q11                 @II sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    VADD.I8     Q11,Q0,Q8                   @II edge_idx = vaddq_s8(const_2, sign_up)
    VTBL.8      D12,{D7},D12                @offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VNEG.S8     Q8,Q10                      @II sign_up = vnegq_s8(sign_down)

    VADD.I8     Q11,Q11,Q10                 @II edge_idx = vaddq_s8(edge_idx, sign_down)
    VMOVL.U8    Q10,D10                     @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    VADDW.S8    Q10,Q10,D12                 @pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    VTBL.8      D22,{D6},D22                @II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VMAX.S16    Q10,Q10,Q1                  @pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    VMIN.U16    Q10,Q10,Q2                  @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))
    VTBL.8      D24,{D7},D22                @II offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VMOVN.I16   D20,Q10                     @vmovn_s16(pi2_tmp_cur_row.val[0])

    VADDW.S8    Q13,Q13,D24                 @II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    VMAX.S16    Q13,Q13,Q1                  @II pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VMIN.U16    Q13,Q13,Q2                  @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VMOV        Q5,Q15                      @II pu1_cur_row = pu1_next_row
    VST1.8      {D20},[r10],r1              @vst1q_u8(pu1_src_cpy, pu1_cur_row)
    VMOVN.I16   D30,Q13                     @II vmovn_s16(pi2_tmp_cur_row.val[0])

    SUBS        r11,r11,#2                  @Decrement the ht loop count by 1
    VST1.8      {D30},[r10],r1              @II vst1q_u8(pu1_src_cpy, pu1_cur_row)

    BEQ         END_LOOPS
    CMP         r11,#1
    BGT         PU1_SRC_LOOP_RESIDUE        @If not equal jump to PU1_SRC_LOOP


    ADD         r10,r10,r1                  @*pu1_src + src_strd
    VLD1.8      D18,[r10]!                  @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D19,[r10]                   @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r10,#8
    VCGT.U8     Q6,Q5,Q9                    @vcgtq_u8(pu1_cur_row, pu1_next_row)
    VCGT.U8     Q7,Q9,Q5                    @vcltq_u8(pu1_cur_row, pu1_next_row)
    VSUB.U8     Q10,Q7,Q6                   @sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    SUB         r10,r10,r1

    VADD.I8     Q11,Q0,Q8                   @edge_idx = vaddq_s8(const_2, sign_up)
    VADD.I8     Q11,Q11,Q10                 @edge_idx = vaddq_s8(edge_idx, sign_down)
    VTBL.8      D22,{D6},D22                @vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))

    VTBL.8      D24,{D7},D22                @offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VMOVL.U8    Q13,D10                     @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VADDW.S8    Q13,Q13,D24                 @pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    VMAX.S16    Q13,Q13,Q1                  @pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VMIN.U16    Q13,Q13,Q2                  @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VMOVN.I16   D30,Q13                     @vmovn_s16(pi2_tmp_cur_row.val[0])

    VST1.8      {D30},[r10],r1              @vst1q_u8(pu1_src_cpy, pu1_cur_row)

END_LOOPS:
    vpop        {d8  -  d15}
    LDMFD       sp!,{r4-r12,r15}            @Reload the registers from SP






