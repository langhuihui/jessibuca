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
@*  ihevc_sao_band_offset_chroma.s
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
@void ihevc_sao_band_offset_chroma(UWORD8 *pu1_src,
@                           WORD32 src_strd,
@                           UWORD8 *pu1_src_left,
@                           UWORD8 *pu1_src_top,
@                           UWORD8 *pu1_src_top_left,
@                           WORD32 sao_band_pos_u,
@                           WORD32 sao_band_pos_v,
@                           WORD8 *pi1_sao_offset_u,
@                           WORD8 *pi1_sao_offset_v,
@                           WORD32 wd,
@                           WORD32 ht)
@
@**************Variables Vs Registers*****************************************
@r0 =>  *pu1_src
@r1 =>  src_strd
@r2 =>  *pu1_src_left
@r3 =>  *pu1_src_top
@r4 =>  *pu1_src_top_left
@r5 =>  sao_band_pos_u
@r6 =>  sao_band_pos_v
@r7 =>  *pi1_sao_offset_u
@r8 =>  *pi1_sao_offset_v
@r9 =>  wd
@r10=>  ht

.equ    pu1_src_top_left_offset,    104
.equ    sao_band_pos_u_offset,      108
.equ    sao_band_pos_v_offset,      112
.equ    pi1_sao_u_offset,           116
.equ    pi1_sao_v_offset,           120
.equ    wd_offset,                  124
.equ    ht_offset,                  128

.text
.p2align 2

.extern gu1_table_band_idx
.globl ihevc_sao_band_offset_chroma_a9q

gu1_table_band_idx_addr_1:
.long gu1_table_band_idx - ulbl1 - 8

gu1_table_band_idx_addr_2:
.long gu1_table_band_idx - ulbl2 - 8

ihevc_sao_band_offset_chroma_a9q:

    STMFD       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8  -  d15}
    LDR         r4,[sp,#pu1_src_top_left_offset]    @Loads pu1_src_top_left
    LDR         r10,[sp,#ht_offset]         @Loads ht

    LDR         r9,[sp,#wd_offset]          @Loads wd
    MOV         r11,r10                     @Move the ht to r9 for loop counter

    ADD         r12,r0,r9                   @pu1_src[row * src_strd + (wd)]
    LDR         r14, gu1_table_band_idx_addr_1
ulbl1:
    add         r14,r14,pc
    SUB         r12,r12,#2                  @wd-2

SRC_LEFT_LOOP:
    LDRH        r5,[r12],r1                 @Load the value
    SUBS        r11,r11,#1                  @Decrement the loop counter
    STRH        r5,[r2],#2                  @Store the value in pu1_src_left pointer
    BNE         SRC_LEFT_LOOP

    LDR         r5,[sp,#sao_band_pos_u_offset]  @Loads sao_band_pos_u
    VLD1.8      D1,[r14]!                   @band_table_u.val[0]
    ADD         r12,r3,r9                   @pu1_src_top[wd]

    LDRH        r11,[r12,#-2]
    VLD1.8      D2,[r14]!                   @band_table_u.val[1]
    LSL         r6,r5,#3                    @sao_band_pos_u

    STRH        r11,[r4]                    @store to pu1_src_top_left[0]
    VLD1.8      D3,[r14]!                   @band_table_u.val[2]
    LDR         r7,[sp,#pi1_sao_u_offset]   @Loads pi1_sao_offset_u

    SUB         r4,r10,#1                   @ht-1
    VDUP.8      D31,r6                      @band_pos_u
    MUL         r4,r4,r1                    @ht-1 * src_strd

    ADD         r4,r4,r0                    @pu1_src[(ht - 1) * src_strd]
    VLD1.8      D4,[r14]!                   @band_table_u.val[3]
    MOV         r11,r9                      @Move the wd to r9 for loop counter

SRC_TOP_LOOP:                               @wd is always multiple of 8
    VLD1.8      D0,[r4]!                    @Load pu1_src[(ht - 1) * src_strd + col]
    SUBS        r11,r11,#8                  @Decrement the loop counter by 8
    VST1.8      D0,[r3]!                    @Store to pu1_src_top[col]
    BNE         SRC_TOP_LOOP

    VLD1.8      D30,[r7]                    @pi1_sao_offset_u load
    VADD.I8     D5,D1,D31                   @band_table_u.val[0] = vadd_u8(band_table_u.val[0], sao_band_pos_u)

    VDUP.8      D29,D30[1]                  @vdup_n_u8(pi1_sao_offset_u[1])
    VADD.I8     D6,D2,D31                   @band_table_u.val[1] = vadd_u8(band_table_u.val[1], sao_band_pos_u)

    VDUP.8      D28,D30[2]                  @vdup_n_u8(pi1_sao_offset_u[2])
    VADD.I8     D7,D3,D31                   @band_table_u.val[2] = vadd_u8(band_table_u.val[2], sao_band_pos_u)

    VDUP.8      D27,D30[3]                  @vdup_n_u8(pi1_sao_offset_u[3])
    VADD.I8     D8,D4,D31                   @band_table_u.val[3] = vadd_u8(band_table_u.val[3], sao_band_pos_u)

    CMP         r5,#28
    VDUP.8      D26,D30[4]                  @vdup_n_u8(pi1_sao_offset_u[4])
    LDR         r14, gu1_table_band_idx_addr_2
ulbl2:
    add         r14,r14,pc

    VMOV.I8     D30,#16                     @vdup_n_u8(16)
    VADD.I8     D1,D5,D29                   @band_table_u.val[0] = vadd_u8(band_table_u.val[0], vdup_n_u8(pi1_sao_offset_u[1]))

    VLD1.8      D9,[r14]!                   @band_table_v.val[0]
    VADD.I8     D2,D6,D28                   @band_table_u.val[1] = vadd_u8(band_table_u.val[1], vdup_n_u8(pi1_sao_offset_u[2]))

    VLD1.8      D10,[r14]!                  @band_table_v.val[1]
    VADD.I8     D3,D7,D27                   @band_table_u.val[2] = vadd_u8(band_table_u.val[2], vdup_n_u8(pi1_sao_offset_u[3]))

    LDR         r6,[sp,#sao_band_pos_v_offset]  @Loads sao_band_pos_v
    VADD.I8     D4,D8,D26                   @band_table_u.val[3] = vadd_u8(band_table_u.val[3], vdup_n_u8(pi1_sao_offset_u[4]))
    LSL         r11,r6,#3                   @sao_band_pos_v

    BLT         SAO_BAND_POS_U_0

SAO_BAND_POS_U_28:                          @case 28
    VCLE.U8     D13,D4,D30                  @vcle_u8(band_table.val[3], vdup_n_u8(16))
    BNE         SAO_BAND_POS_U_29

    VORR.U8     D4,D4,D13                   @band_table.val[3] = vorr_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK_U

SAO_BAND_POS_U_29:                          @case 29
    CMP         r5,#29

    VCLE.U8     D14,D3,D30                  @vcle_u8(band_table.val[2], vdup_n_u8(16))
    BNE         SAO_BAND_POS_U_30
    VORR.U8     D3,D3,D14                   @band_table.val[2] = vorr_u8(band_table.val[2], au1_cmp)

    VAND.U8     D4,D4,D13                   @band_table.val[3] = vand_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK_U

SAO_BAND_POS_U_30:                          @case 30
    CMP         r5,#30

    VCLE.U8     D15,D2,D30                  @vcle_u8(band_table.val[1], vdup_n_u8(16))
    BNE         SAO_BAND_POS_U_31
    VORR.U8     D2,D2,D15                   @band_table.val[1] = vorr_u8(band_table.val[1], au1_cmp)

    VAND.U8     D3,D3,D14                   @band_table.val[2] = vand_u8(band_table.val[2], au1_cmp)

SAO_BAND_POS_U_31:                          @case 31
    CMP         r5,#31
    BNE         SWITCH_BREAK_U

    VCLE.U8     D16,D1,D30                  @vcle_u8(band_table.val[0], vdup_n_u8(16))
    VORR.U8     D1,D1,D16                   @band_table.val[0] = vorr_u8(band_table.val[0], au1_cmp)

    VAND.U8     D2,D2,D15                   @band_table.val[1] = vand_u8(band_table.val[1], au1_cmp)
    B           SWITCH_BREAK_U

SAO_BAND_POS_U_0:
    CMP         r5,#0                       @case 0
    BNE         SWITCH_BREAK_U

    VCLE.U8     D16,D1,D30                  @vcle_u8(band_table.val[0], vdup_n_u8(16))
    VAND.U8     D1,D1,D16                   @band_table.val[0] = vand_u8(band_table.val[0], au1_cmp)

SWITCH_BREAK_U:
    VDUP.8      D30,r11                     @band_pos_v
    LDR         r8,[sp,#pi1_sao_v_offset]   @Loads pi1_sao_offset_v

    VLD1.8      D11,[r14]!                  @band_table_v.val[2]
    VADD.I8     D13,D9,D30                  @band_table_v.val[0] = vadd_u8(band_table_v.val[0], band_pos_v)

    VLD1.8      D12,[r14]!                  @band_table_v.val[3]
    VADD.I8     D14,D10,D30                 @band_table_v.val[1] = vadd_u8(band_table_v.val[1], band_pos_v)

    VLD1.8      D25,[r8]                    @pi1_sao_offset_v load
    VADD.I8     D15,D11,D30                 @band_table_v.val[2] = vadd_u8(band_table_v.val[2], band_pos_v)

    VDUP.8      D29,D25[1]                  @vdup_n_u8(pi1_sao_offset_v[1])
    VADD.I8     D16,D12,D30                 @band_table_v.val[3] = vadd_u8(band_table_v.val[3], band_pos_v)

    VDUP.8      D28,D25[2]                  @vdup_n_u8(pi1_sao_offset_v[2])
    VADD.I8     D9,D13,D29                  @band_table_v.val[0] = vadd_u8(band_table_v.val[0], vdup_n_u8(pi1_sao_offset_v[1]))

    VDUP.8      D27,D25[3]                  @vdup_n_u8(pi1_sao_offset_v[3])
    VADD.I8     D10,D14,D28                 @band_table_v.val[1] = vadd_u8(band_table_v.val[1], vdup_n_u8(pi1_sao_offset_v[2]))

    VDUP.8      D26,D25[4]                  @vdup_n_u8(pi1_sao_offset_v[4])
    VADD.I8     D11,D15,D27                 @band_table_v.val[2] = vadd_u8(band_table_v.val[2], vdup_n_u8(pi1_sao_offset_v[3]))

    VMOV.I8     D29,#16                     @vdup_n_u8(16)
    VADD.I8     D12,D16,D26                 @band_table_v.val[3] = vadd_u8(band_table_v.val[3], vdup_n_u8(pi1_sao_offset_v[4]))
    AND         r12,r9,#0xf

    CMP         r6,#28
    BLT         SAO_BAND_POS_V_0

SAO_BAND_POS_V_28:                          @case 28
    VCLE.U8     D17,D12,D29                 @vcle_u8(band_table.val[3], vdup_n_u8(16))
    BNE         SAO_BAND_POS_V_29
    VORR.U8     D12,D12,D17                 @band_table.val[3] = vorr_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK_V

SAO_BAND_POS_V_29:                          @case 29
    CMP         r6,#29

    VCLE.U8     D18,D11,D29                 @vcle_u8(band_table.val[2], vdup_n_u8(16))
    BNE         SAO_BAND_POS_V_30
    VORR.U8     D11,D11,D18                 @band_table.val[2] = vorr_u8(band_table.val[2], au1_cmp)

    VAND.U8     D12,D12,D17                 @band_table.val[3] = vand_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK_V

SAO_BAND_POS_V_30:                          @case 30
    CMP         r6,#30

    VCLE.U8     D19,D10,D29                 @vcle_u8(band_table.val[1], vdup_n_u8(16))
    BNE         SAO_BAND_POS_V_31
    VORR.U8     D10,D10,D19                 @band_table.val[1] = vorr_u8(band_table.val[1], au1_cmp)

    VAND.U8     D11,D11,D18                 @band_table.val[2] = vand_u8(band_table.val[2], au1_cmp)
    B           SWITCH_BREAK_V

SAO_BAND_POS_V_31:                          @case 31
    CMP         r6,#31
    BNE         SWITCH_BREAK_V

    VCLE.U8     D20,D9,D29                  @vcle_u8(band_table.val[0], vdup_n_u8(16))
    VORR.U8     D9,D9,D20                   @band_table.val[0] = vorr_u8(band_table.val[0], au1_cmp)

    VAND.U8     D10,D10,D19                 @band_table.val[1] = vand_u8(band_table.val[1], au1_cmp)
    B           SWITCH_BREAK_V

SAO_BAND_POS_V_0:
    CMP         r6,#0                       @case 0
    BNE         SWITCH_BREAK_V

    VCLE.U8     D20,D9,D29                  @vcle_u8(band_table.val[0], vdup_n_u8(16))
    VAND.U8     D9,D9,D20                   @band_table.val[0] = vand_u8(band_table.val[0], au1_cmp)

SWITCH_BREAK_V:
    CMP         r9,#16
    MOV         r4,r0                       @pu1_src_cpy
    BLT         WIDTH_RESIDUE

WIDTH_LOOP:                                 @Width is assigned to be multiple of 16
    MOV         r4,r0                       @pu1_src_cpy
    MOV         r11,r10                     @move ht
    ADD         r5,r4,r1

HEIGHT_LOOP:                                @unrolled for 4 rows
    ADD         r6,r5,r1
    VLD2.8      {D5,D6},[r4]                @vld1q_u8(pu1_src_cpy)
    ADD         r7,r6,r1

    VLD2.8      {D13,D14},[r5]              @vld1q_u8(pu1_src_cpy)
    VSUB.I8     D7,D5,D31                   @vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    VLD2.8      {D17,D18},[r6]              @vld1q_u8(pu1_src_cpy)
    VSUB.I8     D8,D6,D30                   @vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    VLD2.8      {D21,D22},[r7]              @vld1q_u8(pu1_src_cpy)
    VSUB.I8     D15,D13,D31                 @vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    VTBX.8      D5,{D1-D4},D7               @vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    VSUB.I8     D16,D14,D30                 @vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    VTBX.8      D6,{D9-D12},D8              @vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    VSUB.I8     D19,D17,D31                 @vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    VTBX.8      D13,{D1-D4},D15             @vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    VSUB.I8     D20,D18,D30                 @vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    VTBX.8      D14,{D9-D12},D16            @vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    VSUB.I8     D23,D21,D31                 @vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    VST2.8      {D5,D6},[r4]                @vst1q_u8(pu1_src_cpy, au1_cur_row)
    VSUB.I8     D24,D22,D30                 @vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    SUBS        r11,r11,#4                  @Decrement the ht loop count by 4
    VTBX.8      D17,{D1-D4},D19             @vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))

    VST2.8      {D13,D14},[r5]              @vst1q_u8(pu1_src_cpy, au1_cur_row)

    VTBX.8      D18,{D9-D12},D20            @vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    VTBX.8      D21,{D1-D4},D23             @vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    VTBX.8      D22,{D9-D12},D24            @vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))

    VST2.8      {D17,D18},[r6],r1           @vst1q_u8(pu1_src_cpy, au1_cur_row)

    ADD         r4,r6,r1
    VST2.8      {D21,D22},[r7]              @vst1q_u8(pu1_src_cpy, au1_cur_row)
    ADD         r5,r4,r1

    BNE         HEIGHT_LOOP

    SUB         r9,r9,#16                   @Decrement the width loop by 16
    ADD         r0,r0,#16
    CMP         r9,#8
    BGT         WIDTH_LOOP
    BLT         END_LOOP
    MOV         r4,r0                       @pu1_src_cpy

WIDTH_RESIDUE:                              @If width is not multiple of 16
    ADD         r5,r4,r1
    VLD2.8      {D5,D6},[r4]                @vld1q_u8(pu1_src_cpy)
    ADD         r6,r5,r1

    ADD         r7,r6,r1
    VLD2.8      {D13,D14},[r5]              @vld1q_u8(pu1_src_cpy)
    VSUB.I8     D7,D5,D31                   @vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    VLD2.8      {D17,D18},[r6]              @vld1q_u8(pu1_src_cpy)
    VSUB.I8     D8,D6,D30                   @vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    VTBX.8      D5,{D1-D4},D7               @vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    VSUB.I8     D15,D13,D31                 @vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    VTBX.8      D6,{D9-D12},D8              @vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    VSUB.I8     D16,D14,D30                 @vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    VLD2.8      {D21,D22},[r7]              @vld1q_u8(pu1_src_cpy)
    VSUB.I8     D19,D17,D31                 @vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    VTBX.8      D13,{D1-D4},D15             @vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    VSUB.I8     D20,D18,D30                 @vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    VTBX.8      D14,{D9-D12},D16            @vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    VZIP.8      D5,D6

    VTBX.8      D17,{D1-D4},D19             @vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    VSUB.I8     D23,D21,D31                 @vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    VST1.8      {D5},[r4]                   @vst1q_u8(pu1_src_cpy, au1_cur_row)
    VZIP.8      D13,D14

    VTBX.8      D18,{D9-D12},D20            @vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    VSUB.I8     D24,D22,D30                 @vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    VST1.8      {D13},[r5]                  @vst1q_u8(pu1_src_cpy, au1_cur_row)
    SUBS        r10,r10,#4                  @Decrement the ht loop count by 4

    VTBX.8      D21,{D1-D4},D23             @vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    VZIP.8      D17,D18

    VTBX.8      D22,{D9-D12},D24            @vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    VST1.8      {D17},[r6],r1               @vst1q_u8(pu1_src_cpy, au1_cur_row)
    VZIP.8      D21,D22

    ADD         r4,r6,r1
    VST1.8      {D21},[r7]                  @vst1q_u8(pu1_src_cpy, au1_cur_row)
    ADD         r5,r4,r1

    BNE         WIDTH_RESIDUE

END_LOOP:
    vpop        {d8  -  d15}
    LDMFD       sp!,{r4-r12,r15}            @Reload the registers from SP



