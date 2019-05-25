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
@*  ihevc_sao_edge_offset_class0_chroma.s
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
@void ihevc_sao_edge_offset_class0_chroma(UWORD8 *pu1_src,
@                              WORD32 src_strd,
@                              UWORD8 *pu1_src_left,
@                              UWORD8 *pu1_src_top,
@                              UWORD8 *pu1_src_top_left,
@                              UWORD8 *pu1_src_top_right,
@                              UWORD8 *pu1_src_bot_left,
@                              UWORD8 *pu1_avail,
@                              WORD8 *pi1_sao_offset_u,
@                              WORD8 *pi1_sao_offset_v,
@                              WORD32 wd,
@
@**************Variables Vs Registers*****************************************
@r0 =>  *pu1_src
@r1 =>  src_strd
@r2 =>  *pu1_src_left
@r3 =>  *pu1_src_top
@r4 =>  *pu1_src_top_left
@r7 =>  *pu1_avail
@r8 =>  *pi1_sao_offset_u
@r5 =>  *pi1_sao_offset_v
@r9 =>  wd
@r10=>  ht

.equ    pu1_src_top_left_offset,    104
.equ    pu1_src_top_right_offset,   108
.equ    pu1_src_bot_left_offset,    112
.equ    pu1_avail_offset,           116
.equ    pi1_sao_u_offset,           120
.equ    pi1_sao_v_offset,           124
.equ    wd_offset,                  128
.equ    ht_offset,                  132

.text
.p2align 2

.extern gi1_table_edge_idx
.globl ihevc_sao_edge_offset_class0_chroma_a9q

gi1_table_edge_idx_addr:
.long gi1_table_edge_idx - ulbl1 - 8

ihevc_sao_edge_offset_class0_chroma_a9q:


    STMFD       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8  -  d15}

    LDR         r9,[sp,#wd_offset]          @Loads wd

    LDR         r4,[sp,#pu1_src_top_left_offset]    @Loads pu1_src_top_left
    ADD         r11,r3,r9                   @pu1_src_top[wd]

    LDR         r10,[sp,#ht_offset]         @Loads ht
    VMOV.I8     Q1,#2                       @const_2 = vdupq_n_s8(2)
    LDRH        r12,[r11,#-2]               @pu1_src_top[wd - 1]

    LDR         r7,[sp,#pu1_avail_offset]   @Loads pu1_avail
    VMOV.I16    Q2,#0                       @const_min_clip = vdupq_n_s16(0)
    STRH        r12,[r4]                    @*pu1_src_top_left = pu1_src_top[wd - 1]

    LDR         r8,[sp,#pi1_sao_u_offset]   @Loads pi1_sao_offset_u
    VMOV.I16    Q3,#255                     @const_max_clip = vdupq_n_u16((1 << bit_depth) - 1)
    SUB         r4,r10,#1                   @(ht - 1)

    LDR         r14, gi1_table_edge_idx_addr @table pointer
ulbl1:
    add         r14,r14,pc
    VMOV.S8     Q4,#0xFF                    @au1_mask = vdupq_n_s8(-1)
    MUL         r4,r4,r1                    @(ht - 1) * src_strd

    LDR         r5,[sp,#pi1_sao_v_offset]   @Loads pi1_sao_offset_v
    VLD1.8      D11,[r8]                    @offset_tbl = vld1_s8(pi1_sao_offset_u)
    ADD         r4,r4,r0                    @pu1_src[(ht - 1) * src_strd]

    MOV         r6,r0                       @pu1_src_org
    VLD1.8      D10,[r14]                   @edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    MOV         r12,r9                      @Move wd to r12 for loop count

SRC_TOP_LOOP:                               @wd is always multiple of 8
    VLD1.8      D0,[r4]!                    @Load pu1_src[(ht - 1) * src_strd + col]
    SUBS        r12,r12,#8                  @Decrement the loop counter by 8
    VST1.8      D0,[r3]!                    @Store to pu1_src_top[col]
    BNE         SRC_TOP_LOOP
    ADD         r6,r6,#14                   @pu1_src_org[14]

    MOV         r3,r2                       @pu1_src_left backup to reload later
    VLD1.8      D0,[r5]                     @offset_tbl = vld1_s8(pi1_sao_offset_v)
    CMP         r9,#16                      @Compare wd with 16

    BLT         WIDTH_RESIDUE               @If not jump to WIDTH_RESIDUE where loop is unrolled for 8 case

    MOV         r8,r9                       @move wd to r8 for loop count

WIDTH_LOOP_16:
    CMP         r8,r9                       @if(col == wd)
    BNE         AU1_MASK_FF                 @jump to else part
    LDRB        r12,[r7]                    @pu1_avail[0]
    VMOV.8      D8[0],r12                   @vsetq_lane_s8(pu1_avail[0], au1_mask, 0)
    VMOV.8      D8[1],r12                   @vsetq_lane_s8(pu1_avail[0], au1_mask, 1)
    B           SKIP_AU1_MASK_FF            @Skip the else part

AU1_MASK_FF:
    MOV         r12,#-1                     @move -1 to r12
    VMOV.16     D8[0],r12                   @au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

SKIP_AU1_MASK_FF:
    CMP         r8,#16                      @If col == 16
    BNE         SKIP_MASKING_IF_NOT16       @If not skip masking
    LDRB        r12,[r7,#1]                 @pu1_avail[1]
    VMOV.8      D9[6],r12                   @au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 14)
    VMOV.8      D9[7],r12                   @au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)

SKIP_MASKING_IF_NOT16:
    MOV         r12,r0                      @pu1_src_cpy = pu1_src
    MOV         r4,r10                      @move ht to r4 for loop count

PU1_SRC_LOOP:
    LDRH        r11,[r2]                    @load pu1_src_left since ht - row =0 when it comes first pu1_src_left is incremented later
    VLD1.8      D12,[r12]!                  @pu1_cur_row = vld1q_u8(pu1_src_cpy)
    VLD1.8      D13,[r12],r1                @pu1_cur_row = vld1q_u8(pu1_src_cpy)
    SUB         r12,#8
    SUB         r5,r9,r8                    @wd - col

    SUB         r14,r10,r4                  @ht - row
    VMOV.16     D15[3],r11                  @vsetq_lane_u16(pu1_src_left[ht - row], pu1_cur_row_tmp, 14,15)
    MUL         r14,r14,r1                  @(ht - row) * src_strd

    VLD1.8      D30,[r12]!                  @II Iteration pu1_cur_row = vld1q_u8(pu1_src_cpy)
    VLD1.8      D31,[r12]                   @II Iteration pu1_cur_row = vld1q_u8(pu1_src_cpy)
    SUB         r12,#8
    VEXT.8      Q7,Q7,Q6,#14                @pu1_cur_row_tmp = vextq_u8(pu1_cur_row_tmp, pu1_cur_row, 14)
    SUB         r12,r12,r1

    LDRH        r11,[r2,#2]                 @II load pu1_src_left since ht - row =0
    VCGT.U8     Q8,Q6,Q7                    @vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
    ADD         r5,r14,r5                   @(ht - row) * src_strd + (wd - col)

    VMOV.16     D29[3],r11                  @II vsetq_lane_u16(pu1_src_left[ht - row], pu1_cur_row_tmp, 14,15)
    VCLT.U8     Q9,Q6,Q7                    @vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)

    LDRH        r14,[r6,r5]                 @pu1_src_org[(ht - row) * src_strd + 14 + (wd - col)]
    VSUB.U8     Q10,Q9,Q8                   @sign_left = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    SUB         r4,r4,#1

    LDRB        r11,[r12,#16]               @pu1_src_cpy[16]
    VEXT.8      Q14,Q14,Q15,#14             @II pu1_cur_row_tmp = vextq_u8(pu1_cur_row_tmp, pu1_cur_row, 14)

    VMOV.8      D14[0],r11                  @pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[16], pu1_cur_row_tmp, 0)
    VCGT.U8     Q13,Q15,Q14                 @II vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)

    LDRB        r11,[r12,#17]               @pu1_src_cpy[17]
    VCLT.U8     Q12,Q15,Q14                 @II vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    STRH        r14,[r2],#2                 @pu1_src_left[(ht - row)] = au1_src_left_tmp[(ht - row)]

    ADD         r12,r12,r1
    VMOV.8      D14[1],r11                  @pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[17], pu1_cur_row_tmp, 1)
    LDRB        r11,[r12,#16]               @II pu1_src_cpy[16]

    VEXT.8      Q7,Q6,Q7,#2                 @pu1_cur_row_tmp = vextq_u8(pu1_cur_row, pu1_cur_row_tmp, 2)
    VMOV.8      D28[0],r11                  @II pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[16], pu1_cur_row_tmp, 0)

    LDRB        r11,[r12,#17]               @II pu1_src_cpy[17]
    VCGT.U8     Q8,Q6,Q7                    @vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
    SUB         r12,r12,r1

    VCLT.U8     Q9,Q6,Q7                    @vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    VMOV.8      D28[1],r11                  @II pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[17], pu1_cur_row_tmp, 1)

    VSUB.U8     Q11,Q9,Q8                   @sign_right = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    VEXT.8      Q14,Q15,Q14,#2              @II pu1_cur_row_tmp = vextq_u8(pu1_cur_row, pu1_cur_row_tmp, 2)

    VADD.U8     Q7,Q1,Q10                   @edge_idx = vaddq_s8(const_2, sign_left)

    VADD.U8     Q7,Q7,Q11                   @edge_idx = vaddq_s8(edge_idx, sign_right)
    VTBL.8      D14,{D10},D14               @vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VSUB.U8     Q10,Q12,Q13                 @II sign_left = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    VCGT.U8     Q13,Q15,Q14                 @II vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
    VTBL.8      D15,{D10},D15               @vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    VCLT.U8     Q12,Q15,Q14                 @II vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)

    VAND        Q7,Q7,Q4                    @edge_idx = vandq_s8(edge_idx, au1_mask)
    VUZP.8      D14,D15

    VSUB.U8     Q11,Q12,Q13                 @II sign_right = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    VTBL.8      D16,{D11},D14               @offset = vtbl1_s8(offset_tbl_u, vget_low_s8(edge_idx))
    VADD.U8     Q12,Q1,Q10                  @II edge_idx = vaddq_s8(const_2, sign_left)

    VMOVL.U8    Q9,D12                      @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VTBL.8      D17,{D0},D15
    VADD.U8     Q12,Q12,Q11                 @II edge_idx = vaddq_s8(edge_idx, sign_right)

    VZIP.S8     D16,D17
    VTBL.8      D24,{D10},D24               @II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VMOVL.U8    Q6,D13                      @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))

    VADDW.S8    Q9,Q9,D16                   @pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    VTBL.8      D25,{D10},D25               @II vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    VMAX.S16    Q9,Q9,Q2                    @pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    VAND        Q12,Q12,Q4                  @II edge_idx = vandq_s8(edge_idx, au1_mask)
    VMIN.U16    Q9,Q9,Q3                    @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))
    VUZP.8      D24,D25                     @II

    VADDW.S8    Q6,Q6,D17                   @pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    VTBL.8      D26,{D11},D24               @II offset = vtbl1_s8(offset_tbl_u, vget_low_s8(edge_idx))
    VMAX.S16    Q6,Q6,Q2                    @pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)

    VMIN.U16    Q6,Q6,Q3                    @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))
    VTBL.8      D27,{D0},D25                @II
    VMOVN.I16   D14,Q9                      @vmovn_s16(pi2_tmp_cur_row.val[0])

    VMOVN.I16   D15,Q6                      @vmovn_s16(pi2_tmp_cur_row.val[1])
    VZIP.S8     D26,D27                     @II

    SUB         r5,r9,r8                    @II wd - col
    VMOVL.U8    Q14,D30                     @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    SUB         r14,r10,r4                  @II ht - row

    MUL         r14,r14,r1                  @II (ht - row) * src_strd
    VADDW.S8    Q14,Q14,D26                 @II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    ADD         r5,r14,r5                   @II (ht - row) * src_strd + (wd - col)

    LDRH        r14,[r6,r5]                 @II pu1_src_org[(ht - row) * src_strd + 14 + (wd - col)]
    VMAX.S16    Q14,Q14,Q2                  @II pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    STRH        r14,[r2],#2                 @II pu1_src_left[(ht - row)] = au1_src_left_tmp[(ht - row)]
    VMIN.U16    Q14,Q14,Q3                  @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VMOVL.U8    Q15,D31                     @II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))

    VADDW.S8    Q15,Q15,D27                 @II pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    VST1.8      {D14,D15},[r12],r1          @vst1q_u8(pu1_src_cpy, pu1_cur_row)

    VMAX.S16    Q15,Q15,Q2                  @II pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    SUBS        r4,r4,#1                    @Decrement row by 1
    VMIN.U16    Q15,Q15,Q3                  @II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    VMOVN.I16   D28,Q14                     @II vmovn_s16(pi2_tmp_cur_row.val[0])
    VMOVN.I16   D29,Q15                     @II vmovn_s16(pi2_tmp_cur_row.val[1])

    VST1.8      {D28,D29},[r12],r1          @II vst1q_u8(pu1_src_cpy, pu1_cur_row)

    BNE         PU1_SRC_LOOP                @If not equal jump to the inner loop

    ADD         r0,r0,#16                   @pu1_src += 16

    SUBS        r8,r8,#16                   @Decrement column by 16
    CMP         r8,#8                       @Check whether residue remains
    MOV         r2,r3                       @Reload pu1_src_left
    BEQ         WIDTH_RESIDUE               @If residue remains jump to residue loop
    BGT         WIDTH_LOOP_16               @If not equal jump to width_loop
    BLT         END_LOOPS                   @Jump to end function

WIDTH_RESIDUE:
    SUB         r6,r6,#14
    AND         r8,r9,#0xF                  @wd_rem = wd & 0xF
    CMP         r8,#0                       @Residue check
    BEQ         END_LOOPS                   @No Residue jump to end function

    CMP         r8,r9                       @if(wd_rem == wd)
    BNE         AU1_MASK_FF_RESIDUE         @jump to else part
    LDRB        r12,[r7]                    @pu1_avail[0]
    VMOV.8      D8[0],r12                   @vsetq_lane_s8(pu1_avail[0], au1_mask, 0)
    VMOV.8      D8[1],r12                   @vsetq_lane_s8(pu1_avail[0], au1_mask, 0)
    B           SKIP_AU1_MASK_FF_RESIDUE    @Skip the else part

AU1_MASK_FF_RESIDUE:
    MOV         r12,#-1                     @move -1 to r12
    VMOV.16     D8[0],r12                   @au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

SKIP_AU1_MASK_FF_RESIDUE:
    LDRB        r12,[r7,#1]                 @pu1_avail[1]
    VMOV.8      D8[6],r12                   @au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)
    VMOV.8      D8[7],r12                   @au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)

    MOV         r12,r0                      @pu1_src_cpy = pu1_src
    MOV         r4,r10                      @move ht to r4 for loop count

PU1_SRC_LOOP_RESIDUE:
    LDRH        r11,[r2]                    @load pu1_src_left
    VLD1.8      D12,[r12]!                  @pu1_cur_row = vld1q_u8(pu1_src_cpy)
    VLD1.8      D13,[r12],r1                @pu1_cur_row = vld1q_u8(pu1_src_cpy)
    SUB         r12,#8
    SUB         r5,r9,#2                    @wd - 2

    SUB         r14,r10,r4                  @(ht - row)
    VMOV.16     D15[3],r11                  @vsetq_lane_u8(pu1_src_left[ht - row], pu1_cur_row_tmp, 15)
    LSL         r14,r14,#1                  @(ht - row) * 2

    VLD1.8      D30,[r12]!                  @II pu1_cur_row = vld1q_u8(pu1_src_cpy)
    VLD1.8      D31,[r12]                   @II pu1_cur_row = vld1q_u8(pu1_src_cpy)
    SUB         r12,#8
    VEXT.8      Q7,Q7,Q6,#14                @pu1_cur_row_tmp = vextq_u8(pu1_cur_row_tmp, pu1_cur_row, 15)
    SUB         r12,r12,r1

    LDRH        r11,[r2,#2]                 @II load pu1_src_left
    VCGT.U8     Q8,Q6,Q7                    @vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
    MUL         r14,r14,r1                  @(ht - row) * 2 * src_strd

    VCLT.U8     Q9,Q6,Q7                    @vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    VMOV.16     D29[3],r11                  @II vsetq_lane_u8(pu1_src_left[ht - row], pu1_cur_row_tmp, 15)

    LDRB        r11,[r12,#16]               @pu1_src_cpy[16]
    VSUB.U8     Q10,Q9,Q8                   @sign_left = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    ADD         r5,r14,r5                   @(ht - row) * 2 * src_strd + (wd - 2)

    VMOV.8      D14[0],r11                  @pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[16], pu1_cur_row_tmp, 0)
    VEXT.8      Q14,Q14,Q15,#14             @II pu1_cur_row_tmp = vextq_u8(pu1_cur_row_tmp, pu1_cur_row, 15)

    LDRB        r11,[r12,#17]               @pu1_src_cpy[17]
    VCGT.U8     Q13,Q15,Q14                 @II vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
    LDRH        r14,[r6, r5]                @pu1_src_org[(ht - row)  * 2* src_strd + (wd - 2)]

    VMOV.8      D14[1],r11                  @pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[17], pu1_cur_row_tmp, 1)
    VCLT.U8     Q12,Q15,Q14                 @II vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    ADD         r12,r12,r1

    STRH        r14,[r2],#2                 @pu1_src_left[(ht - row) * 2] = au1_src_left_tmp[(ht - row) * 2]
    VEXT.8      Q7,Q6,Q7,#2                 @pu1_cur_row_tmp = vextq_u8(pu1_cur_row, pu1_cur_row_tmp, 1)
    LDRB        r11,[r12,#16]               @II pu1_src_cpy[16]

    VCGT.U8     Q8,Q6,Q7                    @vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
    VMOV.8      D28[0],r11                  @II pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[16], pu1_cur_row_tmp, 0)

    LDRB        r11,[r12,#17]               @II pu1_src_cpy[17]
    VCLT.U8     Q9,Q6,Q7                    @vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    SUB         r4,r4,#1                    @II Decrement row by 1

    VSUB.U8     Q11,Q9,Q8                   @sign_right = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    VMOV.8      D28[1],r11                  @II pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[17], pu1_cur_row_tmp, 1)
    SUB         r12,r12,r1

    VADD.U8     Q7,Q1,Q10                   @edge_idx = vaddq_s8(const_2, sign_left)
    VEXT.8      Q14,Q15,Q14,#2              @II pu1_cur_row_tmp = vextq_u8(pu1_cur_row, pu1_cur_row_tmp, 1)

    VADD.U8     Q7,Q7,Q11                   @edge_idx = vaddq_s8(edge_idx, sign_right)

    VSUB.U8     Q10,Q12,Q13                 @II sign_left = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    VTBL.8      D14,{D10},D14               @vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VCGT.U8     Q13,Q15,Q14                 @II vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)

    VCLT.U8     Q12,Q15,Q14                 @II vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    VTBL.8      D15,{D10},D15               @vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    VSUB.U8     Q11,Q12,Q13                 @II sign_right = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    VAND        Q7,Q7,Q4                    @edge_idx = vandq_s8(edge_idx, au1_mask)
    VUZP.8      D14,D15

    VADD.U8     Q14,Q1,Q10                  @II edge_idx = vaddq_s8(const_2, sign_left)
    VTBL.8      D16,{D11},D14               @offset = vtbl1_s8(offset_tbl_u, vget_low_s8(edge_idx))
    VADD.U8     Q14,Q14,Q11                 @II edge_idx = vaddq_s8(edge_idx, sign_right)

    VMOVL.U8    Q9,D12                      @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VTBL.8      D17,{D0},D15
    VMOVL.U8    Q12,D30                     @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    VZIP.S8     D16,D17
    VTBL.8      D28,{D10},D28               @II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VADDW.S8    Q9,Q9,D16                   @pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    VMAX.S16    Q9,Q9,Q2                    @pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VTBL.8      D29,{D10},D29               @II vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    VMIN.U16    Q9,Q9,Q3                    @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VMOVN.I16   D18,Q9                      @vmovn_s16(pi2_tmp_cur_row.val[0])
    VAND        Q14,Q14,Q4                  @II edge_idx = vandq_s8(edge_idx, au1_mask)

    SUB         r5,r9,#2                    @II wd - 2
    VUZP.8      D28,D29                     @II
    SUB         r14,r10,r4                  @II (ht - row)

    LSL         r14,r14,#1                  @II (ht - row) * 2
    VTBL.8      D26,{D11},D28               @II offset = vtbl1_s8(offset_tbl_u, vget_low_s8(edge_idx))
    MUL         r14,r14,r1                  @II (ht - row) * 2 * src_strd

    ADD         r5,r14,r5                   @II (ht - row) * 2 * src_strd + (wd - 2)
    VTBL.8      D27,{D0},D29                @II
    LDRH        r14,[r6, r5]                @II pu1_src_org[(ht - row)  * 2* src_strd + (wd - 2)]

    VZIP.S8     D26,D27                     @II
    VST1.8      {D18},[r12],r1              @vst1q_u8(pu1_src_cpy, pu1_cur_row)

    STRH        r14,[r2],#2                 @II pu1_src_left[(ht - row) * 2] = au1_src_left_tmp[(ht - row) * 2]
    VADDW.S8    Q12,Q12,D26                 @II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    SUBS        r4,r4,#1                    @Decrement row by 1

    VMAX.S16    Q12,Q12,Q2                  @II pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VMIN.U16    Q12,Q12,Q3                  @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VMOVN.I16   D28,Q12                     @II vmovn_s16(pi2_tmp_cur_row.val[0])

    VST1.8      {D28},[r12],r1              @II vst1q_u8(pu1_src_cpy, pu1_cur_row)

    BNE         PU1_SRC_LOOP_RESIDUE        @If not equal jump to the pu1_src loop

END_LOOPS:
    vpop        {d8  -  d15}
    LDMFD       sp!,{r4-r12,r15}            @Reload the registers from SP





