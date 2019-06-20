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
@*  ihevc_sao_band_offset_luma.s
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
@void ihevc_sao_band_offset_luma(UWORD8 *pu1_src,
@                           WORD32 src_strd,
@                           UWORD8 *pu1_src_left,
@                           UWORD8 *pu1_src_top,
@                           UWORD8 *pu1_src_top_left,
@                           WORD32 sao_band_pos,
@                           WORD8 *pi1_sao_offset,
@                           WORD32 wd,
@                           WORD32 ht)
@
@**************Variables Vs Registers*****************************************
@r0 =>  *pu1_src
@r1 =>  src_strd
@r2 =>  *pu1_src_left
@r3 =>  *pu1_src_top
@r4 =>  *pu1_src_top_left
@r5 =>  sao_band_pos
@r6 =>  *pi1_sao_offset
@r7 =>  wd
@r8 =>  ht

.equ    pu1_src_top_left_offset,    104
.equ    sao_band_pos_offset,        108
.equ    pi1_sao_offset,             112
.equ    wd_offset,                  116
.equ    ht_offset,                  120

.text
.p2align 2

.extern gu1_table_band_idx
.globl ihevc_sao_band_offset_luma_a9q

gu1_table_band_idx_addr:
.long gu1_table_band_idx - ulbl1 - 8

ihevc_sao_band_offset_luma_a9q:

    STMFD       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8  -  d15}

    LDR         r8,[sp,#ht_offset]          @Loads ht
    LDR         r7,[sp,#wd_offset]          @Loads wd

    MOV         r9,r8                       @Move the ht to r9 for loop counter
    LDR         r5,[sp,#sao_band_pos_offset]    @Loads sao_band_pos
    ADD         r10,r0,r7                   @pu1_src[row * src_strd + (wd)]

    LDR         r4,[sp,#pu1_src_top_left_offset]    @Loads pu1_src_top_left
    SUB         r10,r10,#1                  @wd-1
    LDR         r14, gu1_table_band_idx_addr
ulbl1:
    add         r14,r14,pc

SRC_LEFT_LOOP:
    LDRB        r11,[r10],r1                @Load the value
    SUBS        r9,r9,#1                    @Decrement the loop counter
    STRB        r11,[r2],#1                 @Store the value in pu1_src_left pointer
    BNE         SRC_LEFT_LOOP

    ADD         r9,r3,r7                    @pu1_src_top[wd]
    VLD1.8      D1,[r14]!                   @band_table.val[0]
    LDR         r6,[sp,#pi1_sao_offset]     @Loads pi1_sao_offset

    LSL         r11,r5,#3
    VLD1.8      D2,[r14]!                   @band_table.val[1]

    LDRB        r10,[r9,#-1]
    VDUP.8      D31,r11                     @band_pos
    SUB         r12,r8,#1                   @ht-1

    STRB        r10,[r4]                    @store to pu1_src_top_left[0]
    VLD1.8      D3,[r14]!                   @band_table.val[2]
    MUL         r12,r12,r1                  @ht-1 * src_strd

    ADD         r4,r12,r0                   @pu1_src[(ht - 1) * src_strd]
    VLD1.8      D4,[r14]!                   @band_table.val[3]
    MOV         r9,r7                       @Move the wd to r9 for loop counter

SRC_TOP_LOOP:                               @wd is always multiple of 8
    VLD1.8      D0,[r4]!                    @Load pu1_src[(ht - 1) * src_strd + col]
    SUBS        r9,r9,#8                    @Decrement the loop counter by 8
    VST1.8      D0,[r3]!                    @Store to pu1_src_top[col]
    BNE         SRC_TOP_LOOP

    VLD1.8      D30,[r6]                    @pi1_sao_offset load
    VADD.I8     D5,D1,D31                   @band_table.val[0] = vadd_u8(band_table.val[0], band_pos)

    VDUP.8      D29,D30[1]                  @vdup_n_u8(pi1_sao_offset[1])
    VADD.I8     D6,D2,D31                   @band_table.val[1] = vadd_u8(band_table.val[1], band_pos)

    VDUP.8      D28,D30[2]                  @vdup_n_u8(pi1_sao_offset[2])
    VADD.I8     D7,D3,D31                   @band_table.val[2] = vadd_u8(band_table.val[2], band_pos)

    VDUP.8      D27,D30[3]                  @vdup_n_u8(pi1_sao_offset[3])
    VADD.I8     D8,D4,D31                   @band_table.val[3] = vadd_u8(band_table.val[3], band_pos)

    VDUP.8      D26,D30[4]                  @vdup_n_u8(pi1_sao_offset[4])
    VADD.I8     D1,D5,D29                   @band_table.val[0] = vadd_u8(band_table.val[0], vdup_n_u8(pi1_sao_offset[1]))

    VMOV.I8     D29,#16                     @vdup_n_u8(16)
    VADD.I8     D2,D6,D28                   @band_table.val[1] = vadd_u8(band_table.val[1], vdup_n_u8(pi1_sao_offset[2]))

    CMP         r5,#28
    VADD.I8     D3,D7,D27                   @band_table.val[2] = vadd_u8(band_table.val[2], vdup_n_u8(pi1_sao_offset[3]))

    VADD.I8     D4,D8,D26                   @band_table.val[3] = vadd_u8(band_table.val[3], vdup_n_u8(pi1_sao_offset[4]))
    BLT         SAO_BAND_POS_0

SAO_BAND_POS_28:                            @case 28

    VCLE.U8     D12,D4,D29                  @vcle_u8(band_table.val[3], vdup_n_u8(16))

    BNE         SAO_BAND_POS_29
    VORR.U8     D4,D4,D12                   @band_table.val[3] = vorr_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK

SAO_BAND_POS_29:                            @case 29
    CMP         r5,#29
    VCLE.U8     D11,D3,D29                  @vcle_u8(band_table.val[2], vdup_n_u8(16))

    BNE         SAO_BAND_POS_30
    VORR.U8     D3,D3,D11                   @band_table.val[2] = vorr_u8(band_table.val[2], au1_cmp)

    VAND.U8     D4,D4,D12                   @band_table.val[3] = vand_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK

SAO_BAND_POS_30:                            @case 30
    CMP         r5,#30
    VCLE.U8     D10,D2,D29                  @vcle_u8(band_table.val[1], vdup_n_u8(16))

    BNE         SAO_BAND_POS_31
    VORR.U8     D2,D2,D10                   @band_table.val[1] = vorr_u8(band_table.val[1], au1_cmp)

    VAND.U8     D3,D3,D11                   @band_table.val[2] = vand_u8(band_table.val[2], au1_cmp)
    B           SWITCH_BREAK

SAO_BAND_POS_31:                            @case 31
    CMP         r5,#31
    BNE         SWITCH_BREAK

    VCLE.U8     D9,D1,D29                   @vcle_u8(band_table.val[0], vdup_n_u8(16))
    VORR.U8     D1,D1,D9                    @band_table.val[0] = vorr_u8(band_table.val[0], au1_cmp)

    VAND.U8     D2,D2,D10                   @band_table.val[1] = vand_u8(band_table.val[1], au1_cmp)

SAO_BAND_POS_0:
    CMP         r5,#0                       @case 0
    BNE         SWITCH_BREAK

    VCLE.U8     D9,D1,D29                   @vcle_u8(band_table.val[0], vdup_n_u8(16))
    VAND.U8     D1,D1,D9                    @band_table.val[0] = vand_u8(band_table.val[0], au1_cmp)

SWITCH_BREAK:
    MOV         r4,r0                       @pu1_src_cpy
    MOV         r11,r8                      @move ht
    ADD         r5,r4,r1

HEIGHT_LOOP:
    ADD         r6,r5,r1
    VLD1.8      D13,[r4]                    @au1_cur_row = vld1_u8(pu1_src_cpy)

    ADD         r10,r6,r1
    VLD1.8      D15,[r5]                    @au1_cur_row = vld1_u8(pu1_src_cpy)

    VLD1.8      D17,[r6]                    @au1_cur_row = vld1_u8(pu1_src_cpy)

    VLD1.8      D19,[r10]                   @au1_cur_row = vld1_u8(pu1_src_cpy)
    VSUB.I8     D14,D13,D31                 @vsub_u8(au1_cur_row, band_pos)

    VTBX.8      D13,{D1-D4},D14             @vtbx4_u8(au1_cur_row, band_table, vsub_u8(au1_cur_row, band_pos))
    VSUB.I8     D16,D15,D31                 @vsub_u8(au1_cur_row, band_pos)

    VTBX.8      D15,{D1-D4},D16             @vtbx4_u8(au1_cur_row, band_table, vsub_u8(au1_cur_row, band_pos))
    VSUB.I8     D18,D17,D31                 @vsub_u8(au1_cur_row, band_pos)

    VTBX.8      D17,{D1-D4},D18             @vtbx4_u8(au1_cur_row, band_table, vsub_u8(au1_cur_row, band_pos))
    VSUB.I8     D20,D19,D31                 @vsub_u8(au1_cur_row, band_pos)

    VTBX.8      D19,{D1-D4},D20             @vtbx4_u8(au1_cur_row, band_table, vsub_u8(au1_cur_row, band_pos))
    VST1.8      D13,[r4],r1                 @vst1_u8(pu1_src_cpy, au1_cur_row)

    VST1.8      D15,[r5]                    @vst1_u8(pu1_src_cpy, au1_cur_row)
    SUBS        r11,r11,#4                  @Decrement the ht loop count by 4

    VST1.8      D17,[r6],r1                 @vst1_u8(pu1_src_cpy, au1_cur_row)

    ADD         r4,r6,r1
    VST1.8      D19,[r10]                   @vst1_u8(pu1_src_cpy, au1_cur_row)
    ADD         r5,r4,r1

    BNE         HEIGHT_LOOP

    SUBS        r7,r7,#8                    @Decrement the width loop by 8
    ADD         r0,r0,#8
    BNE         SWITCH_BREAK

    vpop        {d8  -  d15}
    LDMFD       sp!,{r4-r12,r15}            @Reload the registers from SP




