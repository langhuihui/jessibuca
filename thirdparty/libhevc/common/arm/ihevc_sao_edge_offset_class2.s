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
@*  ihevc_sao_edge_offset_class2.s
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
@void ihevc_sao_edge_offset_class2(UWORD8 *pu1_src,
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
.globl ihevc_sao_edge_offset_class2_a9q

gi1_table_edge_idx_addr_1:
.long gi1_table_edge_idx - ulbl1 - 8

gi1_table_edge_idx_addr_2:
.long gi1_table_edge_idx - ulbl2 - 8

gi1_table_edge_idx_addr_3:
.long gi1_table_edge_idx - ulbl3 - 8

ihevc_sao_edge_offset_class2_a9q:


    STMFD       sp!,{r4-r12,r14}            @stack stores the values of the arguments
    vpush       {d8  -  d15}
    SUB         sp,sp,#160                  @Decrement the stack pointer to store some temp arr values

    LDR         r7,[sp,#wd_offset]          @Loads wd
    LDR         r8,[sp,#ht_offset]          @Loads ht
    SUB         r9,r7,#1                    @wd - 1

    LDR         r4,[sp,#pu1_src_top_left_offset]    @Loads pu1_src_top_left
    LDRB        r10,[r3,r9]                 @pu1_src_top[wd - 1]

    STR         r0,[sp,#152]                @Store pu1_src in sp
    MOV         r9,r7                       @Move width to r9 for loop count

    STR         r2,[sp,#156]                @Store pu1_src_left in sp
    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    LDR         r6,[sp,#pi1_sao_offset]     @Loads pi1_sao_offset
    STR         r3,[sp,#148]                @Store pu1_src_top in sp


    STRB        r10,[sp]                    @u1_src_top_left_tmp = pu1_src_top[wd - 1]
    SUB         r10,r8,#1                   @ht-1
    MLA         r11,r10,r1,r0               @pu1_src[(ht - 1) * src_strd + col]
    ADD         r12,sp,#2                   @temp array

AU1_SRC_TOP_LOOP:
    VLD1.8      D0,[r11]!                   @pu1_src[(ht - 1) * src_strd + col]
    SUBS        r9,r9,#8                    @Decrement the loop count by 8
    VST1.8      D0,[r12]!                   @au1_src_top_tmp[col] = pu1_src[(ht - 1) * src_strd + col]
    BNE         AU1_SRC_TOP_LOOP

PU1_AVAIL_4_LOOP:
    LDRB        r10,[r5,#4]                 @pu1_avail[4]
    CMP         r10,#0
    LDRB        r9,[r0]                     @u1_pos_0_0_tmp = pu1_src[0]
    BEQ         PU1_AVAIL_7_LOOP

    LDRB        r11,[r4]                    @pu1_src_top_left[0]
    ADD         r14,r0,r1                   @pu1_src + src_strd

    SUBS        r12,r9,r11                  @pu1_src[0] - pu1_src_top_left[0]
    LDRB        r4,[r14,#1]                 @pu1_src[1 + src_strd]

    MVNLT       r12,#0
    MOVGT       r12,#1                      @SIGN(pu1_src[0] - pu1_src_top_left[0])

    LDR         r14, gi1_table_edge_idx_addr_1 @table pointer
ulbl1:
    add         r14,r14,pc
    SUBS        r11,r9,r4                   @pu1_src[0] - pu1_src[1 + src_strd]

    MVNLT       r11,#0
    MOVGT       r11,#1                      @SIGN(pu1_src[0] - pu1_src[1 + src_strd])
    ADD         r4,r12,r11                  @SIGN(pu1_src[0] - pu1_src_top_left[0]) +  SIGN(pu1_src[0] - pu1_src[1 + src_strd])
    ADD         r4,r4,#2                    @edge_idx

    LDRSB       r12,[r14,r4]                @edge_idx = gi1_table_edge_idx[edge_idx]
    CMP         r12,#0                      @0 != edge_idx
    BEQ         PU1_AVAIL_7_LOOP
    LDRSB       r10,[r6,r12]                @pi1_sao_offset[edge_idx]
    ADD         r9,r9,r10                   @pu1_src[0] + pi1_sao_offset[edge_idx]
    USAT        r9,#8,r9                    @u1_pos_0_0_tmp = CLIP3(pu1_src[0] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1)

PU1_AVAIL_7_LOOP:
    LDRB        r14,[r5,#7]                 @pu1_avail[7]
    CMP         r14,#0
    SUB         r10,r7,#1                   @wd - 1
    SUB         r11,r8,#1                   @ht - 1
    MLA         r12,r11,r1,r10              @wd - 1 + (ht - 1) * src_strd
    ADD         r12,r12,r0                  @pu1_src[wd - 1 + (ht - 1) * src_strd]
    LDRB        r10,[r12]                   @u1_pos_wd_ht_tmp = pu1_src[wd - 1 + (ht - 1) * src_strd]
    BEQ         PU1_AVAIL

    SUB         r4,r12,r1                   @pu1_src[(wd - 1 + (ht - 1) * src_strd) - src_strd]
    LDRB        r11,[r4,#-1]                @Load pu1_src[wd - 1 + (ht - 1) * src_strd - 1 - src_strd]
    ADD         r14,r12,r1                  @pu1_src[(wd - 1 + (ht - 1) * src_strd) + src_strd]

    SUBS        r11,r10,r11                 @pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd- 1 - src_strd]
    LDRB        r4,[r14,#1]                 @Load pu1_src[wd - 1 + (ht - 1) * src_strd + 1 + src_strd]

    MVNLT       r11,#0
    MOVGT       r11,#1                      @SIGN(pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd- 1 - src_strd])

    SUBS        r4,r10,r4                   @pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd + 1 + src_strd]
    MVNLT       r4,#0
    MOVGT       r4,#1                       @SIGN(pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd + 1 + src_strd])

    ADD         r11,r11,r4                  @Add 2 sign value
    ADD         r11,r11,#2                  @edge_idx
    LDR         r14, gi1_table_edge_idx_addr_2 @table pointer
ulbl2:
    add         r14,r14,pc

    LDRSB       r12,[r14,r11]               @edge_idx = gi1_table_edge_idx[edge_idx]
    CMP         r12,#0
    BEQ         PU1_AVAIL
    LDRSB       r11,[r6,r12]                @pi1_sao_offset[edge_idx]
    ADD         r10,r10,r11                 @pu1_src[wd - 1 + (ht - 1) * src_strd] + pi1_sao_offset[edge_idx]
    USAT        r10,#8,r10                  @u1_pos_wd_ht_tmp = CLIP3(pu1_src[wd - 1 + (ht - 1) * src_strd] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1)

PU1_AVAIL:
    MOV         r12,r8                      @Move ht
    VMOV.I8     Q0,#2                       @const_2 = vdupq_n_s8(2)
    LDRB        r11,[r5,#3]                 @pu1_avail[3]

    MOV         r14,r2                      @Move pu1_src_left to pu1_src_left_cpy
    VMOV.I16    Q1,#0                       @const_min_clip = vdupq_n_s16(0)
    CMP         r11,#0

    LDRB        r5,[r5,#2]                  @pu1_avail[2]
    VMOV.I16    Q2,#255                     @const_max_clip = vdupq_n_u16((1 << bit_depth) - 1)
    SUBEQ       r12,r12,#1                  @ht_tmp--

    CMP         r5,#0
    VLD1.8      D7,[r6]                     @offset_tbl = vld1_s8(pi1_sao_offset)
    LDR         r11, gi1_table_edge_idx_addr_3 @table pointer
ulbl3:
    add         r11,r11,pc

    ADDEQ       r0,r0,r1                    @pu1_src += src_strd
    VLD1.8      D6,[r11]                    @edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    SUBEQ       r12,r12,#1                  @ht_tmp--

    MOV         r6,r7                       @move wd to r6 loop_count
    VMOV.S8     Q4,#0xFF                    @au1_mask = vdupq_n_s8(-1)
    ADDEQ       r14,r14,#1                  @pu1_src_left_cpy += 1

    STR         r0,[sp,#144]                @Store pu1_src in sp
    CMP         r7,#16                      @Compare wd with 16

    BLT         WIDTH_RESIDUE               @If not jump to WIDTH_RESIDUE where loop is unrolled for 8 case
    CMP         r8,#4                       @Compare ht with 4
    BLE         WD_16_HT_4_LOOP             @If jump to WD_16_HT_4_LOOP

WIDTH_LOOP_16:
    LDR         r7,[sp,#wd_offset]          @Loads wd

    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    CMP         r6,r7                       @col == wd
    LDRBEQ      r8,[r5]                     @pu1_avail[0]
    MOVNE       r8,#-1                      @au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

    VMOV.8      d8[0],r8                    @au1_mask = vsetq_lane_s8((-1||pu1_avail[0]), au1_mask, 0)
    CMP         r6,#16                      @if(col == 16)
    BNE         SKIP_AU1_MASK_VAL
    LDRB        r8,[r5,#1]                  @pu1_avail[1]
    VMOV.8      d9[7],r8                    @au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)

SKIP_AU1_MASK_VAL:
    LDRB        r11,[r5,#2]                 @pu1_avail[2]
    CMP         r11,#0

    SUBEQ       r8,r0,r1                    @pu1_src - src_strd
    MOVNE       r8,r3                       @pu1_src_top_cpy
    SUB         r8,r8,#1                    @pu1_src_top_cpy - 1 || pu1_src - src_strd - 1

    LDR         r7,[sp,#wd_offset]          @Loads wd
    VLD1.8      D10,[r8]!                   @pu1_top_row = vld1q_u8(pu1_src - src_strd - 1) || vld1q_u8(pu1_src_top_cpy - 1)
    VLD1.8      D11,[r8]                    @pu1_top_row = vld1q_u8(pu1_src - src_strd - 1) || vld1q_u8(pu1_src_top_cpy - 1)
    SUB         r8,#8
    ADD         r3,r3,#16

    ADD         r5,sp,#66                   @*au1_src_left_tmp
    VLD1.8      D12,[r0]!                   @pu1_cur_row = vld1q_u8(pu1_src)
    VLD1.8      D13,[r0]                    @pu1_cur_row = vld1q_u8(pu1_src)
    SUB         r0,#8
    LDR         r4,[sp,#ht_offset]          @Loads ht

    SUB         r7,r7,r6                    @(wd - col)
    VCGT.U8     Q7,Q6,Q5                    @vcgtq_u8(pu1_cur_row, pu1_top_row)
    LDR         r8,[sp,#152]                @Loads *pu1_src

    ADD         r7,r7,#15                   @15 + (wd - col)
    VCLT.U8     Q8,Q6,Q5                    @vcltq_u8(pu1_cur_row, pu1_top_row)
    ADD         r7,r8,r7                    @pu1_src[0 * src_strd + 15 + (wd - col)]

    SUB         r5,r5,#1
    VSUB.U8     Q7,Q8,Q7                    @sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

AU1_SRC_LEFT_LOOP:
    LDRB        r8,[r7],r1                  @load the value and increment by src_strd
    STRB        r8,[r5,#1]!                 @store it in the stack pointer
    SUBS        r4,r4,#1                    @decrement the loop count
    BNE         AU1_SRC_LEFT_LOOP

    ADD         r8,r0,r1                    @I Iteration *pu1_src + src_strd
    VMOV.I8     Q9,#0
    LDR         r4,[sp,#pu1_avail_offset]   @I Loads pu1_avail

    MOV         r7,r12                      @row count, move ht_tmp to r7
    VLD1.8      D16,[r8]!                   @I pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D17,[r8]                    @I pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r8,#8
    LDRB        r4,[r4,#2]                  @I pu1_avail[2]

    LDRB        r5,[r8,#16]                 @I pu1_src_cpy[src_strd + 16]
    VMOV.8      D18[0],r5                   @I pu1_next_row_tmp = vsetq_lane_u8(pu1_src_cpy[src_strd + 16], pu1_next_row_tmp, 0)

    VEXT.8      Q9,Q8,Q9,#1                 @I pu1_next_row_tmp = vextq_u8(pu1_next_row, pu1_next_row_tmp, 1)
    CMP         r4,#0                       @I
    BNE         SIGN_UP_CHANGE_DONE         @I

SIGN_UP_CHANGE:
    SUB         r2,r12,r7                   @I ht_tmp - row
    LDRB        r11,[r0]                    @I pu1_src_cpy[0]
    ADD         r2,r14,r2                   @I pu1_src_left_cpy[ht_tmp - row]

    LDRB        r5,[r2,#-1]                 @I load the value
    SUBS        r4,r11,r5                   @I pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]
    MVNLT       r4,#0                       @I
    MOVGT       r4,#1                       @I SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row])
    VMOV.8      D14[0],r4                   @I sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]), sign_up, 0)

SIGN_UP_CHANGE_DONE:
    VCGT.U8     Q5,Q6,Q9                    @I vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)
    VADD.I8     Q12,Q0,Q7                   @I edge_idx = vaddq_s8(const_2, sign_up)

    VCLT.U8     Q9,Q6,Q9                    @I vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    VSUB.U8     Q5,Q9,Q5                    @I sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    VADD.I8     Q12,Q12,Q5                  @I edge_idx = vaddq_s8(edge_idx, sign_down)
    VTBL.8      D18,{D6},D24                @I vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VTBL.8      D19,{D6},D25                @I vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))

    VAND        Q9,Q9,Q4                    @I edge_idx = vandq_s8(edge_idx, au1_mask)

    VNEG.S8     Q7,Q5                       @I sign_up = vnegq_s8(sign_down)
    VTBL.8      D10,{D7},D18                @I offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VEXT.8      Q7,Q7,Q7,#15                @I sign_up = vextq_s8(sign_up, sign_up, 15)

    VMOVL.U8    Q10,D12                     @I pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VTBL.8      D11,{D7},D19                @I offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    VADDW.S8    Q10,Q10,D10                 @I pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    VMAX.S16    Q10,Q10,Q1                  @I pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VMOVL.U8    Q11,D13                     @I pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))

    VMIN.U16    Q10,Q10,Q2                  @I pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))
    VMOV        Q6,Q8                       @I pu1_cur_row = pu1_next_row

    VADDW.S8    Q11,Q11,D11                 @I pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    VMOVN.I16   D20,Q10                     @I vmovn_s16(pi2_tmp_cur_row.val[0])

    VMAX.S16    Q11,Q11,Q1                  @I pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    SUB         r7,r7,#1                    @I Decrement the ht_tmp loop count by 1

    VMIN.U16    Q11,Q11,Q2                  @I pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    VMOVN.I16   D21,Q11                     @I vmovn_s16(pi2_tmp_cur_row.val[1])

PU1_SRC_LOOP:

    VST1.8      {Q10},[r0],r1               @I vst1q_u8(pu1_src_cpy, pu1_cur_row)
    ADD         r8,r0,r1                    @II iteration *pu1_src + src_strd

    VLD1.8      D16,[r8]!                   @II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D17,[r8]                    @II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r8,#8
    ADD         r11,r8,r1                   @III iteration *pu1_src + src_strd

    LDRB        r5,[r8,#16]                 @II pu1_src_cpy[src_strd + 16]
    VLD1.8      D30,[r11]!                  @III pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D31,[r11]                   @III pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r11,#8
    LDRB        r4,[r0]                     @II pu1_src_cpy[0]

    LDRB        r8,[r11,#16]                @III pu1_src_cpy[src_strd + 16]
    VMOV.8      D28[0],r5                   @II pu1_next_row_tmp = vsetq_lane_u8(pu1_src_cpy[src_strd + 16], pu1_next_row_tmp, 0)

    SUB         r5,r12,r7                   @II ht_tmp - row
    VEXT.8      Q11,Q8,Q14,#1               @II pu1_next_row_tmp = vextq_u8(pu1_next_row, pu1_next_row_tmp, 1)
    ADD         r5,r14,r5                   @II pu1_src_left_cpy[ht_tmp - row]

    LDRB        r5,[r5,#-1]                 @II load the value
    VMOV.8      D18[0],r8                   @III pu1_next_row_tmp = vsetq_lane_u8(pu1_src_cpy[src_strd + 16], pu1_next_row_tmp, 0)
    SUB         r7,r7,#1                    @II Decrement the ht_tmp loop count by 1

    SUBS        r4,r4,r5                    @II pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]
    VEXT.8      Q9,Q15,Q9,#1                @III pu1_next_row_tmp = vextq_u8(pu1_next_row, pu1_next_row_tmp, 1)
    LDRB        r2,[r0,r1]                  @III pu1_src_cpy[0]

    VCGT.U8     Q12,Q6,Q11                  @II vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)
    SUB         r5,r12,r7                   @III ht_tmp - row

    MVNLT       r4,#0                       @II
    VCLT.U8     Q11,Q6,Q11                  @II vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    ADD         r5,r14,r5                   @III pu1_src_left_cpy[ht_tmp - row]

    MOVGT       r4,#1                       @II SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row])
    VSUB.U8     Q12,Q11,Q12                 @II sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    LDRB        r5,[r5,#-1]                 @III load the value

    SUBS        r2,r2,r5                    @III pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]
    VMOV.8      D14[0],r4                   @II sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]), sign_up, 0)

    MVNLT       r2,#0                       @III
    VCGT.U8     Q5,Q8,Q9                    @III vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)
    MOVGT       r2,#1                       @III SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row])

    VADD.I8     Q11,Q0,Q7                   @II edge_idx = vaddq_s8(const_2, sign_up)
    VADD.I8     Q11,Q11,Q12                 @II edge_idx = vaddq_s8(edge_idx, sign_down)

    VCLT.U8     Q9,Q8,Q9                    @III vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    VTBL.8      D22,{D6},D22                @II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VNEG.S8     Q7,Q12                      @II sign_up = vnegq_s8(sign_down)

    VSUB.U8     Q5,Q9,Q5                    @III sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    VTBL.8      D23,{D6},D23                @II vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    VEXT.8      Q7,Q7,Q7,#15                @II sign_up = vextq_s8(sign_up, sign_up, 15)

    VAND        Q11,Q11,Q4                  @II edge_idx = vandq_s8(edge_idx, au1_mask)
    VMOV.8      D14[0],r2                   @III sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]), sign_up, 0)

    VADD.I8     Q9,Q0,Q7                    @III edge_idx = vaddq_s8(const_2, sign_up)
    VTBL.8      D24,{D7},D22                @II offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VADD.I8     Q9,Q9,Q5                    @III edge_idx = vaddq_s8(edge_idx, sign_down)

    VMOVL.U8    Q13,D12                     @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VTBL.8      D18,{D6},D18                @III vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VNEG.S8     Q7,Q5                       @III sign_up = vnegq_s8(sign_down)

    VADDW.S8    Q13,Q13,D24                 @II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    VTBL.8      D19,{D6},D19                @III vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    VEXT.8      Q7,Q7,Q7,#15                @III sign_up = vextq_s8(sign_up, sign_up, 15)

    VAND        Q9,Q9,Q4                    @III edge_idx = vandq_s8(edge_idx, au1_mask)
    VMOVL.U8    Q10,D16                     @III pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    VMAX.S16    Q13,Q13,Q1                  @II pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VTBL.8      D10,{D7},D18                @III offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    VADDW.S8    Q10,Q10,D10                 @III pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    VMIN.U16    Q13,Q13,Q2                  @II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))
    VTBL.8      D25,{D7},D23                @II offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    VMAX.S16    Q10,Q10,Q1                  @III pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    VMOVL.U8    Q14,D13                     @II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    VMIN.U16    Q10,Q10,Q2                  @III pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    VADDW.S8    Q14,Q14,D25                 @II pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    VTBL.8      D11,{D7},D19                @III offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    VMAX.S16    Q14,Q14,Q1                  @II pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)

    VMIN.U16    Q14,Q14,Q2                  @II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))
    VMOVL.U8    Q9,D17                      @III pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))

    VMOV        Q6,Q15                      @III pu1_cur_row = pu1_next_row
    VMOVN.I16   D26,Q13                     @II vmovn_s16(pi2_tmp_cur_row.val[0])

    VMOVN.I16   D27,Q14                     @II vmovn_s16(pi2_tmp_cur_row.val[1])
    VADDW.S8    Q9,Q9,D11                   @III pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)

    VMAX.S16    Q9,Q9,Q1                    @III pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    VMOVN.I16   D20,Q10                     @III vmovn_s16(pi2_tmp_cur_row.val[0])

    SUB         r7,r7,#1                    @III Decrement the ht_tmp loop count by 1
    VMIN.U16    Q9,Q9,Q2                    @III pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))
    CMP         r7,#1                       @III

    VST1.8      {Q13},[r0],r1               @II vst1q_u8(pu1_src_cpy, pu1_cur_row)
    VMOVN.I16   D21,Q9                      @III vmovn_s16(pi2_tmp_cur_row.val[1])

    BGT         PU1_SRC_LOOP                @III If not equal jump to PU1_SRC_LOOP
    BLT         INNER_LOOP_DONE

    VST1.8      {Q10},[r0],r1               @III vst1q_u8(pu1_src_cpy, pu1_cur_row)
    ADD         r8,r0,r1                    @*pu1_src + src_strd

    LDRB        r2,[r0]                     @pu1_src_cpy[0]
    VLD1.8      D16,[r8]!                   @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D17,[r8]                    @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r8,#8
    LDRB        r5,[r8,#16]                 @pu1_src_cpy[src_strd + 16]

    SUB         r11,r12,r7                  @ht_tmp - row
    VMOV.8      D18[0],r5                   @pu1_next_row_tmp = vsetq_lane_u8(pu1_src_cpy[src_strd + 16], pu1_next_row_tmp, 0)
    ADD         r11,r14,r11                 @pu1_src_left_cpy[ht_tmp - row]

    LDRB        r5,[r11,#-1]                @load the value
    VEXT.8      Q9,Q8,Q9,#1                 @pu1_next_row_tmp = vextq_u8(pu1_next_row, pu1_next_row_tmp, 1)
    SUBS        r4,r2,r5                    @pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]

    VCGT.U8     Q5,Q6,Q9                    @vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)
    MVNLT       r4,#0

    MOVGT       r4,#1                       @SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row])
    VCLT.U8     Q9,Q6,Q9                    @vcltq_u8(pu1_cur_row, pu1_next_row_tmp)

    VMOV.8      D14[0],r4                   @sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]), sign_up, 0)
    VSUB.U8     Q5,Q9,Q5                    @sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    VADD.I8     Q9,Q0,Q7                    @edge_idx = vaddq_s8(const_2, sign_up)
    VADD.I8     Q9,Q9,Q5                    @edge_idx = vaddq_s8(edge_idx, sign_down)

    VTBL.8      D18,{D6},D18                @vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    VNEG.S8     Q7,Q5                       @sign_up = vnegq_s8(sign_down)

    VTBL.8      D19,{D6},D19                @vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    VEXT.8      Q7,Q7,Q7,#15                @sign_up = vextq_s8(sign_up, sign_up, 15)

    VAND        Q9,Q9,Q4                    @edge_idx = vandq_s8(edge_idx, au1_mask)

    VTBL.8      D10,{D7},D18                @offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))

    VMOVL.U8    Q10,D12                     @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    VTBL.8      D11,{D7},D19                @offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    VADDW.S8    Q10,Q10,D10                 @pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    VMAX.S16    Q10,Q10,Q1                  @pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    VMOVL.U8    Q6,D13                      @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))

    VMIN.U16    Q10,Q10,Q2                  @pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))
    VADDW.S8    Q6,Q6,D11                   @pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)

    VMAX.S16    Q6,Q6,Q1                    @pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    VMOVN.I16   D20,Q10                     @vmovn_s16(pi2_tmp_cur_row.val[0])

    VMIN.U16    Q6,Q6,Q2                    @pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))
    VMOVN.I16   D21,Q6                      @vmovn_s16(pi2_tmp_cur_row.val[1])


INNER_LOOP_DONE:
    ADD         r5,sp,#66                   @*au1_src_left_tmp
    VST1.8      {Q10},[r0],r1               @vst1q_u8(pu1_src_cpy, pu1_cur_row)
    LDR         r2,[sp,#156]                @Loads *pu1_src_left

    LDR         r8,[sp,#ht_offset]          @Loads ht
    SUB         r5,r5,#1

    SUB         r2,r2,#1
SRC_LEFT_LOOP:
    LDRB        r7,[r5,#1]!                 @au1_src_left_tmp[row]
    SUBS        r8,r8,#1
    STRB        r7,[r2,#1]!                 @pu1_src_left[row] = au1_src_left_tmp[row]
    BNE         SRC_LEFT_LOOP

    SUB         r6,r6,#16                   @Decrement the wd loop count by 16
    CMP         r6,#8                       @Check whether residue remains
    BLT         RE_ASSINING_LOOP            @Jump to re-assigning loop
    LDR         r7,[sp,#wd_offset]          @Loads wd
    LDR         r0,[sp,#144]                @Loads *pu1_src
    SUB         r7,r7,r6
    ADD         r0,r0,r7
    BGT         WIDTH_LOOP_16               @If not equal jump to width_loop
    BEQ         WIDTH_RESIDUE               @If residue remains jump to residue loop


WD_16_HT_4_LOOP:
    LDR         r7,[sp,#wd_offset]          @Loads wd
    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    CMP         r6,r7                       @col == wd
    LDRBEQ      r8,[r5]                     @pu1_avail[0]
    MOVNE       r8,#-1                      @au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

    VMOV.8      d8[0],r8                    @au1_mask = vsetq_lane_s8((-1||pu1_avail[0]), au1_mask, 0)
    CMP         r6,#16                      @if(col == 16)
    BNE         SKIP_AU1_MASK_VAL_WD_16_HT_4
    LDRB        r8,[r5,#1]                  @pu1_avail[1]
    VMOV.8      d9[7],r8                    @au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)

SKIP_AU1_MASK_VAL_WD_16_HT_4:
    LDRB        r8,[r5,#2]                  @pu1_avail[2]
    CMP         r8,#0

    SUBEQ       r8,r0,r1                    @pu1_src - src_strd
    MOVNE       r8,r3
    SUB         r8,r8,#1                    @pu1_src_top_cpy - 1 || pu1_src - src_strd - 1

    LDR         r7,[sp,#wd_offset]          @Loads wd
    VLD1.8      D10,[r8]!                   @pu1_top_row = vld1q_u8(pu1_src - src_strd - 1) || vld1q_u8(pu1_src_top_cpy - 1)
    VLD1.8      D11,[r8]                    @pu1_top_row = vld1q_u8(pu1_src - src_strd - 1) || vld1q_u8(pu1_src_top_cpy - 1)
    SUB         r8,#8
    ADD         r3,r3,#16

    ADD         r5,sp,#66                   @*au1_src_left_tmp
    VLD1.8      D12,[r0]!                   @pu1_cur_row = vld1q_u8(pu1_src)
    VLD1.8      D13,[r0]                    @pu1_cur_row = vld1q_u8(pu1_src)
    SUB         r0,#8
    LDR         r4,[sp,#ht_offset]          @Loads ht

    SUB         r7,r7,r6                    @(wd - col)
    VCGT.U8     Q7,Q6,Q5                    @vcgtq_u8(pu1_cur_row, pu1_top_row)
    LDR         r8,[sp,#152]                @Loads *pu1_src

    ADD         r7,r7,#15                   @15 + (wd - col)
    VCLT.U8     Q8,Q6,Q5                    @vcltq_u8(pu1_cur_row, pu1_top_row)
    ADD         r7,r8,r7                    @pu1_src[0 * src_strd + 15 + (wd - col)]

    SUB         r5,r5,#1
    VSUB.U8     Q7,Q8,Q7                    @sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

AU1_SRC_LEFT_LOOP_WD_16_HT_4:
    LDRB        r8,[r7],r1                  @load the value and increment by src_strd
    SUBS        r4,r4,#1                    @decrement the loop count
    STRB        r8,[r5,#1]!                 @store it in the stack pointer
    BNE         AU1_SRC_LEFT_LOOP_WD_16_HT_4

    VMOV.I8     Q9,#0
    MOV         r7,r12                      @row count, move ht_tmp to r7

PU1_SRC_LOOP_WD_16_HT_4:
    ADD         r8,r0,r1                    @*pu1_src + src_strd
    VLD1.8      D16,[r8]!                   @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D17,[r8]                    @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r8,#8

    LDRB        r5,[r8,#16]                 @pu1_src_cpy[src_strd + 16]
    VMOV.8      D18[0],r5                   @pu1_next_row_tmp = vsetq_lane_u8(pu1_src_cpy[src_strd + 16], pu1_next_row_tmp, 0)
    VEXT.8      Q9,Q8,Q9,#1                 @pu1_next_row_tmp = vextq_u8(pu1_next_row, pu1_next_row_tmp, 1)

    CMP         r7,r12
    BLT         SIGN_UP_CHANGE_WD_16_HT_4
    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    LDRB        r5,[r5,#2]                  @pu1_avail[2]
    CMP         r5,#0
    BNE         SIGN_UP_CHANGE_DONE_WD_16_HT_4

SIGN_UP_CHANGE_WD_16_HT_4:
    LDRB        r8,[r0]                     @pu1_src_cpy[0]
    SUB         r5,r12,r7                   @ht_tmp - row
    ADD         r5,r14,r5                   @pu1_src_left_cpy[ht_tmp - row]
    LDRB        r5,[r5,#-1]                 @load the value
    SUBS        r8,r8,r5                    @pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]
    MVNLT       r8,#0
    MOVGT       r8,#1                       @SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row])
    VMOV.8      d14[0],r8                   @sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]), sign_up, 0)

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
    VEXT.8      Q7,Q7,Q7,#15                @sign_up = vextq_s8(sign_up, sign_up, 15)

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
    LDR         r2,[sp,#156]                @Loads *pu1_src_left
    SUB         r5,r5,#1
    SUB         r2,r2,#1

SRC_LEFT_LOOP_WD_16_HT_4:
    LDRB        r7,[r5,#1]!                 @au1_src_left_tmp[row]
    STRB        r7,[r2,#1]!                 @pu1_src_left[row] = au1_src_left_tmp[row]
    SUBS        r8,r8,#1
    BNE         SRC_LEFT_LOOP_WD_16_HT_4

    SUBS        r6,r6,#16                   @Decrement the wd loop count by 16
    BLE         RE_ASSINING_LOOP            @Jump to re-assigning loop


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
    LDRB        r11,[r5,#2]                 @pu1_avail[2]
    VLD1.8      D12,[r0]!                   @pu1_cur_row = vld1q_u8(pu1_src)
    VLD1.8      D13,[r0]                    @pu1_cur_row = vld1q_u8(pu1_src)
    SUB         r0,#8
    CMP         r11,#0

    SUBEQ       r8,r0,r1                    @pu1_src - src_strd
    MOVNE       r8,r3

    SUB         r8,r8,#1

    ADD         r5,sp,#66                   @*au1_src_left_tmp
    VLD1.8      D10,[r8]!                   @pu1_top_row = vld1q_u8(pu1_src_top_cpy - 1)
    VLD1.8      D11,[r8]!                   @pu1_top_row = vld1q_u8(pu1_src_top_cpy - 1)
    LDR         r7,[sp,#wd_offset]          @Loads wd

    LDR         r4,[sp,#ht_offset]          @Loads ht
    VCGT.U8     Q7,Q6,Q5                    @vcgtq_u8(pu1_cur_row, pu1_top_row)
    SUB         r7,r7,#1                    @(wd - 1)

    LDR         r8,[sp,#152]                @Loads *pu1_src
    VCLT.U8     Q8,Q6,Q5                    @vcltq_u8(pu1_cur_row, pu1_top_row)
    SUB         r5,r5,#1

    ADD         r7,r8,r7                    @pu1_src[0 * src_strd + (wd - 1)]
    VSUB.U8     Q7,Q8,Q7                    @sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))


AU1_SRC_LEFT_LOOP_RESIDUE:
    LDRB        r8,[r7],r1                  @load the value and increment by src_strd
    SUBS        r4,r4,#1                    @decrement the loop count
    STRB        r8,[r5,#1]!                 @store it in the stack pointer
    BNE         AU1_SRC_LEFT_LOOP_RESIDUE


    MOV         r7,r12                      @row count, move ht_tmp to r7

PU1_SRC_LOOP_RESIDUE:
    VMOV.I8     Q9,#0
    ADD         r8,r0,r1                    @*pu1_src + src_strd
    VLD1.8      D16,[r8]!                   @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    VLD1.8      D17,[r8]                    @pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    SUB         r8,#8

    LDRB        r8,[r8,#16]                 @pu1_src_cpy[src_strd + 16]
    VMOV.8      d18[0],r8                   @pu1_next_row_tmp = vsetq_lane_u8(pu1_src_cpy[src_strd + 16], pu1_next_row_tmp, 0)
    VEXT.8      Q9,Q8,Q9,#1                 @pu1_next_row_tmp = vextq_u8(pu1_next_row, pu1_next_row_tmp, 1)

    CMP         r7,r12
    BLT         SIGN_UP_CHANGE_RESIDUE
    LDR         r5,[sp,#pu1_avail_offset]   @Loads pu1_avail
    LDRB        r5,[r5,#2]                  @pu1_avail[2]
    CMP         r5,#0
    BNE         SIGN_UP_CHANGE_DONE_RESIDUE

SIGN_UP_CHANGE_RESIDUE:
    LDRB        r8,[r0]                     @pu1_src_cpy[0]
    SUB         r5,r12,r7                   @ht_tmp - row

    ADD         r5,r14,r5
    LDRB        r5,[r5,#-1]                 @load the value
    SUBS        r8,r8,r5                    @pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]
    MVNLT       r8,#0
    MOVGT       r8,#1                       @SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row])
    VMOV.8      d14[0],r8                   @sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[0] - pu1_src_left_cpy[ht_tmp - 1 - row]), sign_up, 0)

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
    VEXT.8      Q7,Q7,Q7,#15                @sign_up = vextq_s8(sign_up, sign_up, 15)

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
    ADD         r5,sp,#66                   @*au1_src_left_tmp

    LDR         r2,[sp,#156]                @Loads *pu1_src_left
    SUB         r5,r5,#1

    SUB         r2,r2,#1

SRC_LEFT_LOOP_RESIDUE:
    LDRB        r7,[r5,#1]!                 @au1_src_left_tmp[row]
    SUBS        r8,r8,#1
    STRB        r7,[r2,#1]!                 @pu1_src_left[row] = au1_src_left_tmp[row]
    BNE         SRC_LEFT_LOOP_RESIDUE


RE_ASSINING_LOOP:
    LDR         r8,[sp,#ht_offset]          @Loads ht
    LDR         r7,[sp,#wd_offset]          @Loads wd

    LDR         r0,[sp,#152]                @Loads *pu1_src
    SUB         r8,r8,#1                    @ht - 1

    MLA         r6,r8,r1,r7                 @wd - 1 + (ht - 1) * src_strd
    STRB        r9,[r0]                     @pu1_src_org[0] = u1_pos_0_0_tmp

    LDR         r4,[sp,#pu1_src_top_left_offset] @Loads pu1_src_top_left
    ADD         r6,r0,r6                    @pu1_src[wd - 1 + (ht - 1) * src_strd]

    ADD         r12,sp,#2
    STRB        r10,[r6,#-1]                @pu1_src_org[wd - 1 + (ht - 1) * src_strd] = u1_pos_wd_ht_tmp

    LDRB        r11,[sp]                    @load u1_src_top_left_tmp from stack pointer
    LDR         r3,[sp,#148]                @Loads pu1_src_top

    STRB        r11,[r4]                    @*pu1_src_top_left = u1_src_top_left_tmp

SRC_TOP_LOOP:
    VLD1.8      D0,[r12]!                   @pu1_src_top[col] = au1_src_top_tmp[col]
    SUBS        r7,r7,#8                    @Decrement the width
    VST1.8      D0,[r3]!                    @pu1_src_top[col] = au1_src_top_tmp[col]
    BNE         SRC_TOP_LOOP

END_LOOPS:
    ADD         sp,sp,#160
    vpop        {d8  -  d15}
    LDMFD       sp!,{r4-r12,r15}            @Reload the registers from SP



