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
@*  ihevc_sao_edge_offset_class3.s
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
@void ihevc_sao_edge_offset_class3(UWORD8 *pu1_src,
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
@r8=>   ht

.equ    pu1_src_top_left_offset,    264
.equ    pu1_src_top_right_offset,   268
.equ    pu1_src_bot_left_offset,    272
.equ    pu1_avail_offset,           276
.equ    pi1_sao_offset,             280
.equ    wd_offset,                  284
.equ    ht_offset,                  288

.text
.syntax unified
.p2align 2

.extern gi1_table_edge_idx
.globl ihevc_sao_edge_offset_class3_a9q

gi1_table_edge_idx_addr_1:
.long gi1_table_edge_idx - ulbl1 - 8

gi1_table_edge_idx_addr_2:
.long gi1_table_edge_idx - ulbl2 - 8

gi1_table_edge_idx_addr_3:
.long gi1_table_edge_idx - ulbl3 - 8

ihevc_sao_edge_offset_class3_a9q:


    STMFD       sp!,{r4-r12,r14}            @stack stores the values of the arguments
    vpush       {d8  -  d15}
    SUB         sp,sp,#160                  @Decrement the stack pointer to store some temp arr values
    LDR         r7,[sp,#wd_offset]          @Loads wd

    LDR         r8,[sp,#ht_offset]          @Loads ht
    SUB         r9,r7,#1                    @wd - 1

    LDR         r4,[sp,#pu1_src_top_left_offset]               @Loads pu1_src_top_left
    LDRB        r10,[r3,r9]                 @pu1_src_top[wd - 1]

    MOV         r9,r7                       @Move width to r9 for loop count

    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    LDR         r6,[sp,#pi1_sao_offset]     @Loads pi1_sao_offset
    STR         r3,[sp,#156]                @Store pu1_src_top in sp


    STRB        r10,[sp]                    @u1_src_top_left_tmp = pu1_src_top[wd - 1]
    SUB         r10,r8,#1                   @ht-1
    MLA         r11,r10,r1,r0               @pu1_src[(ht - 1) * src_strd + col]
    ADD         r12,sp,#2                   @temp array

AU1_SRC_TOP_LOOP:
    VLD1.8      D0,[r11]!                   @pu1_src[(ht - 1) * src_strd + col]
    SUBS        r9,r9,#8                    @Decrement the loop count by 8
    VST1.8      D0,[r12]!                   @au1_src_top_tmp[col] = pu1_src[(ht - 1) * src_strd + col]
    BNE         AU1_SRC_TOP_LOOP

PU1_AVAIL_5_LOOP:
    LDRB        r9,[r5,#5]                  @pu1_avail[5]
    CMP         r9,#0
    SUB         r10,r7,#1                   @[wd - 1]
    LDRB        r9,[r0,r10]                 @u1_pos_0_0_tmp = pu1_src[wd - 1]
    BEQ         PU1_AVAIL_6_LOOP

    LDR         r11,[sp,#pu1_src_top_right_offset]  @Load pu1_src_top_right from sp
    SUB         r10,r10,#1                  @[wd - 1 - 1]

    LDRB        r11,[r11]                   @pu1_src_top_right[0]
    SUB         r12,r9,r11                  @pu1_src[wd - 1] - pu1_src_top_right[0]

    ADD         r11,r0,r1                   @pu1_src + src_strd

    LDRB        r14,[r11,r10]               @pu1_src[wd - 1 - 1 + src_strd]
    CMP         r12,#0
    MVNLT       r12,#0
    SUB         r11,r9,r14                  @pu1_src[wd - 1] - pu1_src[wd - 1 - 1 + src_strd]

    MOVGT       r12,#1                      @SIGN(pu1_src[wd - 1] - pu1_src_top_right[0])
    CMP         r11,#0
    MVNLT       r11,#0
    MOVGT       r11,#1                      @SIGN(pu1_src[wd - 1] - pu1_src[wd - 1 - 1 + src_strd])
    LDR         r14, gi1_table_edge_idx_addr_1 @table pointer
ulbl1:
    add         r14,r14,pc
    ADD         r11,r12,r11                 @SIGN(pu1_src[wd - 1] - pu1_src_top_right[0]) +  SIGN(pu1_src[wd - 1] - pu1_src[wd - 1 - 1 + src_strd])
    ADD         r11,r11,#2                  @edge_idx

    LDRSB       r12,[r14,r11]               @edge_idx = gi1_table_edge_idx[edge_idx]
    CMP         r12,#0                      @0 != edge_idx
    BEQ         PU1_AVAIL_6_LOOP
    LDRSB       r10,[r6,r12]                @pi1_sao_offset[edge_idx]
    ADD         r9,r9,r10                   @pu1_src[0] + pi1_sao_offset[edge_idx]
    USAT        r9,#8,r9                    @u1_pos_0_0_tmp = CLIP3(pu1_src[0] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1)

PU1_AVAIL_6_LOOP:
    LDRB        r10,[r5,#6]                 @pu1_avail[6]
    SUB         r11,r8,#1                   @ht - 1

    CMP         r10,#0
    STR         r0,[sp,#148]                @Store pu1_src in sp
    MLA         r12,r11,r1,r0               @pu1_src[(ht - 1) * src_strd]

    LDRB        r10,[r12]                   @u1_pos_wd_ht_tmp = pu1_src[(ht - 1) * src_strd]
    BEQ         PU1_AVAIL_3_LOOP

    LDR         r14,[sp,#pu1_src_bot_left_offset]   @Load pu1_src_bot_left from sp
    SUB         r11,r12,r1                  @pu1_src[(ht - 1) * src_strd) - src_strd]

    LDRB        r14,[r14]                   @Load pu1_src_bot_left[0]
    ADD         r11,r11,#1                  @pu1_src[(ht - 1) * src_strd + 1 - src_strd]

    LDRB        r11,[r11]                   @Load pu1_src[(ht - 1) * src_strd + 1 - src_strd]
    SUB         r14,r10,r14                 @pu1_src[(ht - 1) * src_strd] - pu1_src_bot_left[0]

    SUB         r11,r10,r11                 @pu1_src[(ht - 1) * src_strd] - pu1_src[(ht - 1) * src_strd + 1 - src_strd]
    CMP         r11,#0
    MVNLT       r11,#0
    MOVGT       r11,#1                      @SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src[(ht - 1) * src_strd + 1 - src_strd])

    CMP         r14,#0
    MVNLT       r14,#0
    MOVGT       r14,#1                      @SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src_bot_left[0])

    ADD         r11,r11,r14                 @Add 2 sign value

    LDR         r14, gi1_table_edge_idx_addr_2 @table pointer
ulbl2:
    add         r14,r14,pc
    ADD         r11,r11,#2                  @edge_idx

    LDRSB       r12,[r14,r11]               @edge_idx = gi1_table_edge_idx[edge_idx]
    CMP         r12,#0
    BEQ         PU1_AVAIL_3_LOOP
    LDRSB       r11,[r6,r12]                @pi1_sao_offset[edge_idx]
    ADD         r10,r10,r11                 @pu1_src[(ht - 1) * src_strd] + pi1_sao_offset[edge_idx]
    USAT        r10,#8,r10                  @u1_pos_wd_ht_tmp = CLIP3(pu1_src[(ht - 1) * src_strd] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1)

PU1_AVAIL_3_LOOP:
    STR         r2,[sp,#152]                @Store pu1_src_left in sp
    MOV         r12,r8                      @Move ht

    MOV         r14,r2                      @Move pu1_src_left to pu1_src_left_cpy
    VMOV.I8     Q0,#2                       @const_2 = vdupq_n_s8(2)
    LDRB        r11,[r5,#3]                 @pu1_avail[3]

    CMP         r11,#0
    VMOV.I16    Q1,#0                       @const_min_clip = vdupq_n_s16(0)
    SUBEQ       r12,r12,#1                  @ht_tmp--

    LDRB        r5,[r5,#2]                  @pu1_avail[2]
    VMOV.I16    Q2,#255                     @const_max_clip = vdupq_n_u16((1 << bit_depth) - 1)
    CMP         r5,#0

    ADDEQ       r0,r0,r1                    @pu1_src += src_strd
    VLD1.8      D7,[r6]                     @offset_tbl = vld1_s8(pi1_sao_offset)
    SUBEQ       r12,r12,#1                  @ht_tmp--

    LDR         r6, gi1_table_edge_idx_addr_3 @table pointer
ulbl3:
    add         r6,r6,pc
    VMOV.S8     Q4,#0xFF                    @au1_mask = vdupq_n_s8(-1)
    ADDEQ       r14,r14,#1                  @pu1_src_left_cpy += 1

    STR         r0,[sp,#144]                @Store pu1_src in sp
    VLD1.8      D6,[r6]                     @edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    MOV         r6,r7                       @move wd to r6 loop_count

    CMP         r7,#16                      @Compare wd with 16
    BLT         WIDTH_RESIDUE               @If not jump to WIDTH_RESIDUE where loop is unrolled for 8 case
    CMP         r8,#4                       @Compare ht with 4
    BLE         WD_16_HT_4_LOOP             @If jump to WD_16_HT_4_LOOP

WIDTH_LOOP_16:
    LDR         r7,[sp,#wd_offset]          @Loads wd

    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    CMP         r6,r7                       @col == wd
    LDRBEQ      r8,[r5]                     @pu1_avail[0]
    MOVNE       r8,#-1
    VMOV.8      d8[0],r8                    @au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

    CMP         r6,#16                      @if(col == 16)
    BNE         SKIP_AU1_MASK_VAL
    LDRB        r8,[r5,#1]                  @pu1_avail[1]
    VMOV.8      d9[7],r8                    @au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)

SKIP_AU1_MASK_VAL:
    LDRB        r8,[r5,#2]                  @pu1_avail[2]
    CMP         r8,#0

    LDR         r4,[sp,#ht_offset]          @Loads ht
    SUBEQ       r8,r0,r1                    @pu1_src - src_strd

    MOVNE       r8,r3
    ADD         r5,sp,#66                   @*au1_src_left_tmp

    LDR         r7,[sp,#wd_offset]          @Loads wd
    ADD         r8,r8,#1                    @pu1_src - src_strd + 1

    SUB         r7,r7,r6                    @(wd - col)
    VLD1.8      D10,[r8]!                   @pu1_top_row = vld1q_u8(pu1_src - src_strd + 1)
    VLD1.8      D11,[r8]                    @pu1_top_row = vld1q_u8(pu1_src - src_strd + 1)
    SUB         r8,#8
    ADD         r3,r3,#16

    LDR         r8,[sp,#148]                @Loads *pu1_src
    VLD1.8      D12,[r0]!                   @pu1_cur_row = vld1q_u8(pu1_src)
    VLD1.8      D13,[r0]                    @pu1_cur_row = vld1q_u8(pu1_src)
    SUB         r0,#8
    ADD         r7,r7,#15                   @15 + (wd - col)

    ADD         r7,r8,r7                    @pu1_src[0 * src_strd + 15 + (wd - col)]
    VCGT.U8     Q7,Q6,Q5                    @vcgtq_u8(pu1_cur_row, pu1_top_row)
    SUB         r5,r5,#1

AU1_SRC_LEFT_LOOP:
    LDRB        r8,[r7],r1                  @load the value and increment by src_strd
    SUBS        r4,r4,#1                    @decrement the loop count
    STRB        r8,[r5,#1]!                 @store it in the stack pointer
    BNE         AU1_SRC_LEFT_LOOP

    VMOV.I8     Q9,#0
    VCLT.U8     Q8,Q6,Q5                    @vcltq_u8(pu1_cur_row, pu1_top_row)

    ADD         r8,r0,r1                    @I *pu1_src + src_strd
    VSUB.U8     Q7,Q8,Q7                    @sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    MOV         r7,r12                      @row count, move ht_tmp to r7

    SUB         r5,r12,r7                   @I ht_tmp - row
    VLD1.8      D16,[r8]!                   @I pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D17,[r8]                    @I pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r8,#8
    ADD         r8,r14,r5                   @I pu1_src_left_cpy[ht_tmp - row]

    ADD         r8,r8,#1                    @I pu1_src_left_cpy[ht_tmp - row + 1]
    LDRB        r8,[r8]

    LDR         r5,[sp,#pu1_avail_offset]   @I Loads pu1_avail
    VMOV.8      D19[7],r8                   @I vsetq_lane_u8
    LDRB        r5,[r5,#2]                  @I pu1_avail[2]

    VEXT.8      Q9,Q9,Q8,#15                @I pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 15)
    CMP         r5,#0                       @I
    BNE         SIGN_UP_CHANGE_DONE         @I

SIGN_UP_CHANGE:
    LDRB        r8,[r0,#15]                 @I pu1_src_cpy[15]
    SUB         r5,r0,r1                    @I pu1_src_cpy[16 - src_strd]

    LDRB        r5,[r5,#16]                 @I load the value
    SUB         r8,r8,r5                    @I pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]
    CMP         r8,#0                       @I
    MVNLT       r8,#0                       @I
    MOVGT       r8,#1                       @I SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd])
    VMOV.8      D15[7],r8                   @I sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]), sign_up, 15)

SIGN_UP_CHANGE_DONE:
    VCGT.U8     Q5,Q6,Q9                    @I vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)
    VCLT.U8     Q9,Q6,Q9                    @I vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    VSUB.U8     Q5,Q9,Q5                    @I sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    VADD.I8     Q9,Q0,Q7                    @I edge_idx = vaddq_s8(const_2, sign_up)
    VADD.I8     Q9,Q9,Q5                    @I edge_idx = vaddq_s8(edge_idx, sign_down)
    VTBL.8      D18,{D6},D18                @I vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VNEG.S8     Q7,Q5                       @I sign_up = vnegq_s8(sign_down)

    VEXT.8      Q7,Q7,Q7,#1                 @I sign_up = vextq_s8(sign_up, sign_up, 1)
    VTBL.8      D19,{D6},D19                @I vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))

    VMOVL.U8    Q10,D12                     @I pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VAND        Q9,Q9,Q4                    @I edge_idx = vandq_s8(edge_idx, au1_mask)

    VTBL.8      D10,{D7},D18                @I offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))

    VMOVL.U8    Q11,D13                     @I pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    VADDW.S8    Q10,Q10,D10                 @I pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    VMAX.S16    Q10,Q10,Q1                  @I pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VTBL.8      D11,{D7},D19                @I offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    VMIN.U16    Q10,Q10,Q2                  @I pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VMOV        Q6,Q8
    VADDW.S8    Q11,Q11,D11                 @I pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)

    VMAX.S16    Q11,Q11,Q1                  @I pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    VMIN.U16    Q11,Q11,Q2                  @I pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    SUB         r7,r7,#1                    @I Decrement the ht_tmp loop count by 1

PU1_SRC_LOOP:
    ADD         r8,r0,r1,LSL #1             @II *pu1_src + src_strd
    VMOVN.I16   D20,Q10                     @I vmovn_s16(pi2_tmp_cur_row.val[0])
    SUB         r5,r12,r7                   @II ht_tmp - row

    ADD         r4,r0,r1                    @II pu1_src_cpy[16 - src_strd]
    VMOVN.I16   D21,Q11                     @I vmovn_s16(pi2_tmp_cur_row.val[1])
    ADD         r2,r8,r1                    @III *pu1_src + src_strd

    LDRB        r11,[r4,#15]                @II pu1_src_cpy[15]
    VLD1.8      D16,[r8]!                   @II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D17,[r8]                    @II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r8,#8
    SUB         r7,r7,#1                    @II Decrement the ht_tmp loop count by 1

    ADD         r8,r14,r5                   @II pu1_src_left_cpy[ht_tmp - row]
    VLD1.8      D30,[r2]!                   @III pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D31,[r2]                    @III pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r2,#8
    LDRB        r8,[r8,#1]

    LDRB        r4,[r0,#16]                 @II load the value
    VMOV.8      D19[7],r8                   @II vsetq_lane_u8
    SUB         r11,r11,r4                  @II pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]

    CMP         r11,#0                      @II
    VST1.8      {Q10},[r0],r1               @I vst1q_u8(pu1_src_cpy, pu1_cur_row)
    SUB         r5,r12,r7                   @III ht_tmp - row

    MVNLT       r11,#0                      @II
    VEXT.8      Q9,Q9,Q8,#15                @II pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 15)
    MOVGT       r11,#1                      @II SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd])

    ADD         r8,r14,r5                   @III pu1_src_left_cpy[ht_tmp - row]
    VMOV.8      D15[7],r11                  @II sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]), sign_up, 15)
    CMP         r7,#1                       @III

    BNE         NEXT_ROW_ELSE_2             @III
    LDR         r5,[sp,#pu1_avail_offset]   @III Loads pu1_avail
    LDRB        r5,[r5,#3]                  @III pu1_avail[3]
    CMP         r5,#0                       @III
    SUBNE       r8,r2,#2                    @III pu1_src_cpy[src_strd - 1]

NEXT_ROW_ELSE_2:
    LDRB        r8,[r8,#1]                  @III
    VCGT.U8     Q12,Q6,Q9                   @II vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)
    ADD         r5,r0,r1

    LDRB        r2,[r5,#15]                 @III pu1_src_cpy[15]
    VCLT.U8     Q13,Q6,Q9                   @II vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    LDRB        r5,[r0,#16]                 @III load the value

    SUB         r2,r2,r5                    @III pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]
    VSUB.U8     Q12,Q13,Q12                 @II sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    CMP         r2,#0                       @III

    MVNLT       r2,#0                       @III
    VMOV.8      D19[7],r8                   @III vsetq_lane_u8
    MOVGT       r2,#1                       @III SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd])

    SUB         r7,r7,#1                    @III Decrement the ht_tmp loop count by 1
    VADD.I8     Q13,Q0,Q7                   @II edge_idx = vaddq_s8(const_2, sign_up)

    VNEG.S8     Q7,Q12                      @II sign_up = vnegq_s8(sign_down)
    VEXT.8      Q9,Q9,Q15,#15               @III pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 15)

    VADD.I8     Q13,Q13,Q12                 @II edge_idx = vaddq_s8(edge_idx, sign_down)

    VEXT.8      Q7,Q7,Q7,#1                 @II sign_up = vextq_s8(sign_up, sign_up, 1)
    VTBL.8      D26,{D6},D26                @II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VCGT.U8     Q5,Q8,Q9                    @III vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)

    VMOV.8      D15[7],r2                   @III sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]), sign_up, 15)
    VTBL.8      D27,{D6},D27                @II vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    VCLT.U8     Q9,Q8,Q9                    @III vcltq_u8(pu1_cur_row, pu1_next_row_tmp)

    VMOVL.U8    Q14,D12                     @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VAND        Q13,Q13,Q4                  @II edge_idx = vandq_s8(edge_idx, au1_mask)

    VSUB.U8     Q5,Q9,Q5                    @III sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    VTBL.8      D24,{D7},D26                @II offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VADD.I8     Q9,Q0,Q7                    @III edge_idx = vaddq_s8(const_2, sign_up)

    VADD.I8     Q9,Q9,Q5                    @III edge_idx = vaddq_s8(edge_idx, sign_down)
    VTBL.8      D25,{D7},D27                @II offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    VNEG.S8     Q7,Q5                       @III sign_up = vnegq_s8(sign_down)

    VADDW.S8    Q14,Q14,D24                 @II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    VTBL.8      D18,{D6},D18                @III vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VMAX.S16    Q14,Q14,Q1                  @II pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    VEXT.8      Q7,Q7,Q7,#1                 @III sign_up = vextq_s8(sign_up, sign_up, 1)
    VTBL.8      D19,{D6},D19                @III vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    VMIN.U16    Q14,Q14,Q2                  @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VMOVL.U8    Q13,D13                     @II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    VAND        Q9,Q9,Q4                    @III edge_idx = vandq_s8(edge_idx, au1_mask)

    VADDW.S8    Q13,Q13,D25                 @II pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    VTBL.8      D10,{D7},D18                @III offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VMAX.S16    Q13,Q13,Q1                  @II pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)

    VMOVL.U8    Q10,D16                     @III pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VMIN.U16    Q13,Q13,Q2                  @II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    VADDW.S8    Q10,Q10,D10                 @III pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    VTBL.8      D11,{D7},D19                @III offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    VMAX.S16    Q10,Q10,Q1                  @III pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    VMOVL.U8    Q11,D17                     @III pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    VMIN.U16    Q10,Q10,Q2                  @III pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VMOVN.I16   D28,Q14                     @II vmovn_s16(pi2_tmp_cur_row.val[0])
    VADDW.S8    Q11,Q11,D11                 @III pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)

    VMOVN.I16   D29,Q13                     @II vmovn_s16(pi2_tmp_cur_row.val[1])
    VMAX.S16    Q11,Q11,Q1                  @III pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)

    VMOV        Q6,Q15                      @II pu1_cur_row = pu1_next_row
    VMIN.U16    Q11,Q11,Q2                  @III pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    CMP         r7,#1                       @III
    VST1.8      {Q14},[r0],r1               @II vst1q_u8(pu1_src_cpy, pu1_cur_row)
    BGT         PU1_SRC_LOOP                @If not equal jump to PU1_SRC_LOOP
    BLT         INNER_LOOP_DONE

    ADD         r8,r0,r1,LSL #1             @*pu1_src + src_strd
    VMOVN.I16   D20,Q10                     @III vmovn_s16(pi2_tmp_cur_row.val[0])
    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail

    LDRB        r5,[r5,#3]                  @pu1_avail[3]
    VMOVN.I16   D21,Q11                     @III vmovn_s16(pi2_tmp_cur_row.val[1])
    CMP         r5,#0

    ADD         r4,r0,r1                    @pu1_src_cpy[16 - src_strd]
    VLD1.8      D16,[r8]!                   @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D17,[r8]                    @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r8,#8
    LDRB        r5,[r0,#16]                 @load the value

    BEQ         NEXT_ROW_ELSE_3
    LDRB        r8,[r8,#-1]                 @pu1_src_cpy[src_strd - 1]
    B           NEXT_ROW_POINTER_ASSIGNED_3
NEXT_ROW_ELSE_3:
    SUB         r11,r12,r7                  @ht_tmp - row
    ADD         r8,r14,r11                  @pu1_src_left_cpy[ht_tmp - row]
    ADD         r8,r8,#1                    @pu1_src_left_cpy[ht_tmp - row + 1]
    LDRB        r8,[r8]

NEXT_ROW_POINTER_ASSIGNED_3:
    LDRB        r11,[r4,#15]                @pu1_src_cpy[15]
    VMOV.8      D19[7],r8                   @vsetq_lane_u8
    SUB         r8,r11,r5                   @pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]

    CMP         r8,#0
    VEXT.8      Q9,Q9,Q8,#15                @pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 15)
    MVNLT       r8,#0

    VST1.8      {Q10},[r0],r1               @III vst1q_u8(pu1_src_cpy, pu1_cur_row)
    VCGT.U8     Q12,Q6,Q9                   @vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)

    MOVGT       r8,#1                       @SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd])
    VCLT.U8     Q13,Q6,Q9                   @vcltq_u8(pu1_cur_row, pu1_next_row_tmp)

    VMOV.8      D15[7],r8                   @sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]), sign_up, 15)
    VSUB.U8     Q12,Q13,Q12                 @sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    VMOVL.U8    Q10,D12                     @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VADD.I8     Q13,Q0,Q7                   @edge_idx = vaddq_s8(const_2, sign_up)

    VMOVL.U8    Q11,D13                     @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    VADD.I8     Q13,Q13,Q12                 @edge_idx = vaddq_s8(edge_idx, sign_down)

    VTBL.8      D26,{D6},D26                @vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VTBL.8      D27,{D6},D27                @vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))

    VAND        Q13,Q13,Q4                  @edge_idx = vandq_s8(edge_idx, au1_mask)

    VTBL.8      D24,{D7},D26                @offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))

    VADDW.S8    Q10,Q10,D24                 @pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    VTBL.8      D25,{D7},D27                @offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    VMAX.S16    Q10,Q10,Q1                  @pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    VMIN.U16    Q10,Q10,Q2                  @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VADDW.S8    Q11,Q11,D25                 @pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    VMAX.S16    Q11,Q11,Q1                  @pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    VMIN.U16    Q11,Q11,Q2                  @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

INNER_LOOP_DONE:
    VMOVN.I16   D20,Q10                     @vmovn_s16(pi2_tmp_cur_row.val[0])
    LDR         r8,[sp,#ht_offset]          @Loads ht

    VMOVN.I16   D21,Q11                     @vmovn_s16(pi2_tmp_cur_row.val[1])
    ADD         r5,sp,#66                   @*au1_src_left_tmp

    VST1.8      {Q10},[r0],r1               @vst1q_u8(pu1_src_cpy, pu1_cur_row)
    LDR         r2,[sp,#152]                @Loads *pu1_src_left
SRC_LEFT_LOOP:
    LDR         r7,[r5],#4                  @au1_src_left_tmp[row]
    SUBS        r8,r8,#4
    STR         r7,[r2],#4                  @pu1_src_left[row] = au1_src_left_tmp[row]
    BNE         SRC_LEFT_LOOP

    SUBS        r6,r6,#16                   @Decrement the wd loop count by 16
    CMP         r6,#8                       @Check whether residue remains
    BLT         RE_ASSINING_LOOP            @Jump to re-assigning loop
    LDR         r7,[sp,#wd_offset]          @Loads wd
    LDR         r0,[sp,#144]                @Loads *pu1_src
    SUB         r7,r7,r6
    ADD         r0,r0,r7
    BGT         WIDTH_LOOP_16               @If not equal jump to width_loop
    BEQ         WIDTH_RESIDUE               @If residue remains jump to residue loop



WD_16_HT_4_LOOP:
    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    LDR         r7,[sp,#wd_offset]          @Loads wd
    CMP         r6,r7                       @col == wd
    LDRBEQ      r8,[r5]                     @pu1_avail[0]
    MOVNE       r8,#-1
    VMOV.8      d8[0],r8                    @au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

    CMP         r6,#16                      @if(col == 16)
    BNE         SKIP_AU1_MASK_VAL_WD_16_HT_4
    LDRB        r8,[r5,#1]                  @pu1_avail[1]
    VMOV.8      d9[7],r8                    @au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)

SKIP_AU1_MASK_VAL_WD_16_HT_4:
    LDRB        r8,[r5,#2]                  @pu1_avail[2]
    CMP         r8,#0

    SUBEQ       r8,r0,r1                    @pu1_src - src_strd
    MOVNE       r8,r3
    ADD         r8,r8,#1                    @pu1_src - src_strd + 1
    VLD1.8      D10,[r8]!                   @pu1_top_row = vld1q_u8(pu1_src - src_strd + 1)
    VLD1.8      D11,[r8]                    @pu1_top_row = vld1q_u8(pu1_src - src_strd + 1)
    SUB         r8,#8

    ADD         r3,r3,#16
    ADD         r5,sp,#66                   @*au1_src_left_tmp
    LDR         r4,[sp,#ht_offset]          @Loads ht
    LDR         r7,[sp,#wd_offset]          @Loads wd
    SUB         r7,r7,r6                    @(wd - col)
    ADD         r7,r7,#15                   @15 + (wd - col)
    LDR         r8,[sp,#148]                @Loads *pu1_src
    ADD         r7,r8,r7                    @pu1_src[0 * src_strd + 15 + (wd - col)]
    SUB         r5,r5,#1

AU1_SRC_LEFT_LOOP_WD_16_HT_4:
    LDRB        r8,[r7],r1                  @load the value and increment by src_strd
    STRB        r8,[r5,#1]!                 @store it in the stack pointer
    SUBS        r4,r4,#1                    @decrement the loop count
    BNE         AU1_SRC_LEFT_LOOP_WD_16_HT_4

    VLD1.8      D12,[r0]!                   @pu1_cur_row = vld1q_u8(pu1_src)
    VLD1.8      D13,[r0]                    @pu1_cur_row = vld1q_u8(pu1_src)
    SUB         r0,#8

    VCGT.U8     Q7,Q6,Q5                    @vcgtq_u8(pu1_cur_row, pu1_top_row)
    VCLT.U8     Q8,Q6,Q5                    @vcltq_u8(pu1_cur_row, pu1_top_row)
    VSUB.U8     Q7,Q8,Q7                    @sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    VMOV.I8     Q9,#0
    MOV         r7,r12                      @row count, move ht_tmp to r7

PU1_SRC_LOOP_WD_16_HT_4:
    ADD         r8,r0,r1                    @*pu1_src + src_strd
    VLD1.8      D16,[r8]!                   @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D17,[r8]                    @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r8,#8
    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    LDRB        r5,[r5,#3]                  @pu1_avail[3]
    CMP         r5,#0
    BEQ         NEXT_ROW_ELSE_WD_16_HT_4
    CMP         r7,#1
    LDRBEQ      r8,[r8,#-1]                 @pu1_src_cpy[src_strd - 1]
    BEQ         NEXT_ROW_POINTER_ASSIGNED_WD_16_HT_4
NEXT_ROW_ELSE_WD_16_HT_4:
    SUB         r5,r12,r7                   @ht_tmp - row
    ADD         r8,r14,r5                   @pu1_src_left_cpy[ht_tmp - row]
    ADD         r8,r8,#1                    @pu1_src_left_cpy[ht_tmp - row + 1]
    LDRB        r8,[r8]

NEXT_ROW_POINTER_ASSIGNED_WD_16_HT_4:
    VMOV.8      D19[7],r8                   @vsetq_lane_u8
    VEXT.8      Q9,Q9,Q8,#15                @pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 15)

    CMP         r7,r12
    BNE         SIGN_UP_CHANGE_WD_16_HT_4
    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    LDRB        r5,[r5,#2]                  @pu1_avail[2]
    CMP         r5,#0
    BNE         SIGN_UP_CHANGE_DONE_WD_16_HT_4

SIGN_UP_CHANGE_WD_16_HT_4:
    LDRB        r8,[r0,#15]                 @pu1_src_cpy[15]
    ADD         r5,r0,#16                   @pu1_src_cpy[16]
    SUB         r5,r5,r1                    @pu1_src_cpy[16 - src_strd]
    LDRB        r5,[r5]                     @load the value
    SUB         r8,r8,r5                    @pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]
    CMP         r8,#0
    MVNLT       r8,#0
    MOVGT       r8,#1                       @SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd])
    VMOV.8      D15[7],r8                   @sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]), sign_up, 15)

SIGN_UP_CHANGE_DONE_WD_16_HT_4:
    VCGT.U8     Q10,Q6,Q9                   @vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)
    VCLT.U8     Q11,Q6,Q9                   @vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    VSUB.U8     Q12,Q11,Q10                 @sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    VADD.I8     Q13,Q0,Q7                   @edge_idx = vaddq_s8(const_2, sign_up)
    VADD.I8     Q13,Q13,Q12                 @edge_idx = vaddq_s8(edge_idx, sign_down)
    VTBL.8      D26,{D6},D26                @vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VTBL.8      D27,{D6},D27                @vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))

    VAND        Q13,Q13,Q4                  @edge_idx = vandq_s8(edge_idx, au1_mask)

    VNEG.S8     Q7,Q12                      @sign_up = vnegq_s8(sign_down)
    VEXT.8      Q7,Q7,Q7,#1                 @sign_up = vextq_s8(sign_up, sign_up, 1)

    VTBL.8      D24,{D7},D26                @offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VMOVL.U8    Q14,D12                     @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VADDW.S8    Q14,Q14,D24                 @pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    VMAX.S16    Q14,Q14,Q1                  @pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VMIN.U16    Q14,Q14,Q2                  @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VTBL.8      D25,{D7},D27                @offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    VMOVL.U8    Q15,D13                     @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    VADDW.S8    Q15,Q15,D25                 @pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    VMAX.S16    Q15,Q15,Q1                  @pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    VMIN.U16    Q15,Q15,Q2                  @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    VMOVN.I16   D28,Q14                     @vmovn_s16(pi2_tmp_cur_row.val[0])
    VMOVN.I16   D29,Q15                     @vmovn_s16(pi2_tmp_cur_row.val[1])

    VST1.8      {Q14},[r0],r1               @vst1q_u8(pu1_src_cpy, pu1_cur_row)

    VMOV        Q6,Q8                       @pu1_cur_row = pu1_next_row
    SUBS        r7,r7,#1                    @Decrement the ht_tmp loop count by 1
    BNE         PU1_SRC_LOOP_WD_16_HT_4     @If not equal jump to PU1_SRC_LOOP_WD_16_HT_4

    LDR         r8,[sp,#ht_offset]          @Loads ht
    ADD         r5,sp,#66                   @*au1_src_left_tmp
    LDR         r2,[sp,#152]                @Loads *pu1_src_left
SRC_LEFT_LOOP_WD_16_HT_4:
    LDR         r7,[r5],#4                  @au1_src_left_tmp[row]
    STR         r7,[r2],#4                  @pu1_src_left[row] = au1_src_left_tmp[row]
    SUBS        r8,r8,#4
    BNE         SRC_LEFT_LOOP_WD_16_HT_4

    SUBS        r6,r6,#16                   @Decrement the wd loop count by 16
    BLE         RE_ASSINING_LOOP            @Jump to re-assigning loop
    LDR         r7,[sp,#wd_offset]          @Loads wd
    LDR         r0,[sp,#144]                @Loads *pu1_src
    SUB         r7,r7,r6
    ADD         r0,r0,r7
    BGT         WD_16_HT_4_LOOP             @If not equal jump to width_loop


WIDTH_RESIDUE:
    LDR         r7,[sp,#wd_offset]          @Loads wd
    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    CMP         r6,r7                       @wd_residue == wd
    LDRBEQ      r8,[r5]                     @pu1_avail[0]

    MOVNE       r8,#-1
    VMOV.8      d8[0],r8                    @au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

    LDRB        r8,[r5,#1]                  @pu1_avail[1]
    VMOV.8      d8[7],r8                    @au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)

PU1_AVAIL_2_RESIDUE:
    LDRB        r8,[r5,#2]                  @pu1_avail[2]
    CMP         r8,#0

    SUBEQ       r8,r0,r1                    @pu1_src - src_strd
    MOVNE       r8,r3
    ADD         r8,r8,#1                    @pu1_src - src_strd + 1
    VLD1.8      D10,[r8]!                   @pu1_top_row = vld1q_u8(pu1_src - src_strd + 1)
    VLD1.8      D11,[r8]                    @pu1_top_row = vld1q_u8(pu1_src - src_strd + 1)
    SUB         r8,#8


    ADD         r5,sp,#66                   @*au1_src_left_tmp
    LDR         r4,[sp,#ht_offset]          @Loads ht
    LDR         r7,[sp,#wd_offset]          @Loads wd
    LDR         r8,[sp,#148]                @Loads *pu1_src
    SUB         r7,r7,#1                    @(wd - 1)
    ADD         r7,r8,r7                    @pu1_src[0 * src_strd + (wd - 1)]
    SUB         r5,r5,#1

AU1_SRC_LEFT_LOOP_RESIDUE:
    LDRB        r8,[r7],r1                  @load the value and increment by src_strd
    STRB        r8,[r5,#1]!                 @store it in the stack pointer
    SUBS        r4,r4,#1                    @decrement the loop count
    BNE         AU1_SRC_LEFT_LOOP_RESIDUE

    VLD1.8      D12,[r0]!                   @pu1_cur_row = vld1q_u8(pu1_src)
    VLD1.8      D13,[r0]                    @pu1_cur_row = vld1q_u8(pu1_src)
    SUB         r0,#8

    VCGT.U8     Q7,Q6,Q5                    @vcgtq_u8(pu1_cur_row, pu1_top_row)
    VCLT.U8     Q8,Q6,Q5                    @vcltq_u8(pu1_cur_row, pu1_top_row)
    VSUB.U8     Q7,Q8,Q7                    @sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    MOV         r7,r12                      @row count, move ht_tmp to r7

PU1_SRC_LOOP_RESIDUE:
    VMOV.I8     Q9,#0
    ADD         r8,r0,r1                    @*pu1_src + src_strd
    VLD1.8      D16,[r8]!                   @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D17,[r8]                    @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r8,#8
    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    LDRB        r5,[r5,#3]                  @pu1_avail[3]
    CMP         r5,#0
    BEQ         NEXT_ROW_ELSE_RESIDUE
    CMP         r7,#1
    LDRBEQ      r8,[r8,#-1]                 @pu1_src_cpy[src_strd - 1]
    BEQ         NEXT_ROW_POINTER_ASSIGNED_RESIDUE
NEXT_ROW_ELSE_RESIDUE:
    SUB         r5,r12,r7                   @ht_tmp - row
    ADD         r8,r14,r5                   @pu1_src_left_cpy[ht_tmp - row]
    ADD         r8,r8,#1                    @pu1_src_left_cpy[ht_tmp - row + 1]
    LDRB        r8,[r8]

NEXT_ROW_POINTER_ASSIGNED_RESIDUE:
    VMOV.8      D19[7],r8                   @vsetq_lane_u8
    VEXT.8      Q9,Q9,Q8,#15                @pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 15)

    CMP         r7,r12
    BNE         SIGN_UP_CHANGE_RESIDUE
    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    LDRB        r5,[r5,#2]                  @pu1_avail[2]
    CMP         r5,#0
    BNE         SIGN_UP_CHANGE_DONE_RESIDUE

SIGN_UP_CHANGE_RESIDUE:
    LDRB        r8,[r0,#15]                 @pu1_src_cpy[15]
    ADD         r5,r0,#16                   @pu1_src_cpy[16]
    SUB         r5,r5,r1                    @pu1_src_cpy[16 - src_strd]
    LDRB        r5,[r5]                     @load the value
    SUB         r8,r8,r5                    @pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]
    CMP         r8,#0
    MVNLT       r8,#0
    MOVGT       r8,#1                       @SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd])
    VMOV.8      D15[7],r8                   @sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] - pu1_src_cpy[16 - src_strd]), sign_up, 15)

SIGN_UP_CHANGE_DONE_RESIDUE:
    VCGT.U8     Q10,Q6,Q9                   @vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)
    VCLT.U8     Q11,Q6,Q9                   @vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    VSUB.U8     Q12,Q11,Q10                 @sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    VADD.I8     Q13,Q0,Q7                   @edge_idx = vaddq_s8(const_2, sign_up)
    VADD.I8     Q13,Q13,Q12                 @edge_idx = vaddq_s8(edge_idx, sign_down)
    VTBL.8      D26,{D6},D26                @vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VTBL.8      D27,{D6},D27                @vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))

    VAND        Q13,Q13,Q4                  @edge_idx = vandq_s8(edge_idx, au1_mask)

    VNEG.S8     Q7,Q12                      @sign_up = vnegq_s8(sign_down)
    VEXT.8      Q7,Q7,Q7,#1                 @sign_up = vextq_s8(sign_up, sign_up, 1)

    VTBL.8      D24,{D7},D26                @offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VMOVL.U8    Q14,D12                     @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VADDW.S8    Q14,Q14,D24                 @pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    VMAX.S16    Q14,Q14,Q1                  @pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VMIN.U16    Q14,Q14,Q2                  @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VMOVN.I16   D30,Q14                     @vmovn_s16(pi2_tmp_cur_row.val[0])

    VST1.8      {D30},[r0],r1               @vst1q_u8(pu1_src_cpy, pu1_cur_row)
    VMOV        Q6,Q8                       @pu1_cur_row = pu1_next_row
    SUBS        r7,r7,#1
    BNE         PU1_SRC_LOOP_RESIDUE

    LDR         r8,[sp,#ht_offset]          @Loads ht
    LDR         r2,[sp,#152]                @Loads *pu1_src_left
    ADD         r5,sp,#66                   @*au1_src_left_tmp

SRC_LEFT_LOOP_RESIDUE:
    LDR         r7,[r5],#4                  @au1_src_left_tmp[row]
    SUBS        r8,r8,#4
    STR         r7,[r2],#4                  @pu1_src_left[row] = au1_src_left_tmp[row]
    BNE         SRC_LEFT_LOOP_RESIDUE


RE_ASSINING_LOOP:
    LDR         r7,[sp,#wd_offset]          @Loads wd
    LDR         r0,[sp,#148]                @Loads *pu1_src

    LDR         r11,[sp,#ht_offset]         @Loads ht
    ADD         r8,r0,r7                    @pu1_src[wd]

    LDR         r4,[sp,#pu1_src_top_left_offset]    @Loads pu1_src_top_left
    SUB         r11,r11,#1                  @ht - 1

    STRB        r9,[r8,#-1]                 @pu1_src_org[wd - 1] = u1_pos_wd_0_tmp
    MLA         r6,r11,r1,r0                @pu1_src_org[(ht - 1) * src_strd]

    LDRB        r8,[sp]                     @load u1_src_top_left_tmp from stack pointer
    ADD         r12,sp,#2

    STRB        r10,[r6]                    @pu1_src_org[wd - 1 + (ht - 1) * src_strd] = u1_pos_wd_ht_tmp
    STRB        r8,[r4]                     @*pu1_src_top_left = u1_src_top_left_tmp
    LDR         r3,[sp,#156]                @Loads pu1_src_top

SRC_TOP_LOOP:
    VLD1.8      D0,[r12]!                   @pu1_src_top[col] = au1_src_top_tmp[col]
    SUBS        r7,r7,#8                    @Decrement the width
    VST1.8      D0,[r3]!                    @pu1_src_top[col] = au1_src_top_tmp[col]
    BNE         SRC_TOP_LOOP

END_LOOPS:
    ADD         sp,sp,#160
    vpop        {d8  -  d15}
    LDMFD       sp!,{r4-r12,r15}            @Reload the registers from SP



