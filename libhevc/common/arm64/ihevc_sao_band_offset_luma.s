///*****************************************************************************
//*
//* Copyright (C) 2012 Ittiam Systems Pvt Ltd, Bangalore
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at:
//*
//* http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.
//*
//*****************************************************************************/
///**
//*******************************************************************************
//* ,:file
//*  ihevc_sao_band_offset_luma.s
//*
//* ,:brief
//*  Contains function definitions for inter prediction  interpolation.
//* Functions are coded using NEON  intrinsics and can be compiled using// ARM
//* RVCT
//*
//* ,:author
//*  Parthiban V
//*
//* ,:par List of Functions:
//*
//*
//* ,:remarks
//*  None
//*
//*******************************************************************************
//*/
//void ihevc_sao_band_offset_luma(UWORD8 *pu1_src,
//                           WORD32 src_strd,
//                           UWORD8 *pu1_src_left,
//                           UWORD8 *pu1_src_top,
//                           UWORD8 *pu1_src_top_left,
//                           WORD32 sao_band_pos,
//                           WORD8 *pi1_sao_offset,
//                           WORD32 wd,
//                           WORD32 ht)
//
//**************Variables Vs Registers*****************************************
//x0 =>    *pu1_src
//x1 =>    src_strd
//x2 =>    *pu1_src_left
//x3 =>    *pu1_src_top
//x4    =>    *pu1_src_top_left
//x5    =>    sao_band_pos
//x6    =>    *pi1_sao_offset
//x7    =>    wd
//x8    =>    ht


.set WIDE_REFERENCE, 0
.set ARCHITECTURE, 5
.set DO1STROUNDING, 0

.include "ihevc_neon_macros.s"

.text
.p2align 2

.globl gu1_table_band_idx
.globl ihevc_sao_band_offset_luma_av8

ihevc_sao_band_offset_luma_av8:

    // STMFD sp!, {x4-x12, x14}            //stack stores the values of the arguments

    LDR         w8,[sp]                     //Loads ht


    stp         d13,d14,[sp,#-16]!
    stp         d8,d15,[sp,#-16]!           // Storing d15 using { sub sp,sp,#8; str d15,[sp] } is giving bus error.
                                            // d8 is used as dummy register and stored along with d15 using stp. d8 is not used in the function.
    stp         x19, x20,[sp,#-16]!

    MOV         x9,x8                       //Move the ht to x9 for loop counter
    ADD         x10,x0,x7                   //pu1_src[row * src_strd + (wd)]

    SUB         x10,x10,#1                  //wd-1
    ADRP        x14, :got:gu1_table_band_idx
    LDR         x14, [x14, #:got_lo12:gu1_table_band_idx]

SRC_LEFT_LOOP:
    LDRB        w11,[x10]
    add         x10, x10, x1                //Load the value
    SUBS        x9,x9,#1                    //Decrement the loop counter
    STRB        w11,[x2],#1                 //Store the value in pu1_src_left pointer
    BNE         SRC_LEFT_LOOP

    ADD         x9,x3,x7                    //pu1_src_top[wd]
    LD1         {v1.8b},[x14],#8            //band_table.val[0]

    LSL         x11,x5,#3
    LD1         {v2.8b},[x14],#8            //band_table.val[1]

    LDRB        w10,[x9,#-1]
    dup         v31.8b,w11                  //band_pos
    SUB         x12,x8,#1                   //ht-1

    STRB        w10,[x4]                    //store to pu1_src_top_left[0]
    LD1         {v3.8b},[x14],#8            //band_table.val[2]
    mul         x12, x12, x1                //ht-1 * src_strd

    ADD         x4,x12,x0                   //pu1_src[(ht - 1) * src_strd]
    LD1         {v4.8b},[x14],#8            //band_table.val[3]
    MOV         x9,x7                       //Move the wd to x9 for loop counter

SRC_TOP_LOOP:                               //wd is always multiple of 8
    LD1         {v0.8b},[x4],#8             //Load pu1_src[(ht - 1) * src_strd + col]
    SUBS        x9,x9,#8                    //Decrement the loop counter by 8
    ST1         {v0.8b},[x3],#8             //Store to pu1_src_top[col]
    BNE         SRC_TOP_LOOP

    LD1         {v30.8b},[x6]               //pi1_sao_offset load
    ADD         v5.8b,  v1.8b ,  v31.8b     //band_table.val[0] = vadd_u8(band_table.val[0], band_pos)

    dup         v29.8b, v30.b[1]            //vdup_n_u8(pi1_sao_offset[1])
    ADD         v6.8b,  v2.8b ,  v31.8b     //band_table.val[1] = vadd_u8(band_table.val[1], band_pos)

    dup         v28.8b, v30.b[2]            //vdup_n_u8(pi1_sao_offset[2])
    ADD         v7.8b,  v3.8b ,  v31.8b     //band_table.val[2] = vadd_u8(band_table.val[2], band_pos)

    dup         v27.8b, v30.b[3]            //vdup_n_u8(pi1_sao_offset[3])
    ADD         v21.8b,  v4.8b ,  v31.8b    //band_table.val[3] = vadd_u8(band_table.val[3], band_pos)

    dup         v26.8b, v30.b[4]            //vdup_n_u8(pi1_sao_offset[4])
    ADD         v1.8b,  v5.8b ,  v29.8b     //band_table.val[0] = vadd_u8(band_table.val[0], vdup_n_u8(pi1_sao_offset[1]))

    movi        v29.8b, #16                 //vdup_n_u8(16)
    ADD         v2.8b,  v6.8b ,  v28.8b     //band_table.val[1] = vadd_u8(band_table.val[1], vdup_n_u8(pi1_sao_offset[2]))

    CMP         x5,#28
    ADD         v3.8b,  v7.8b ,  v27.8b     //band_table.val[2] = vadd_u8(band_table.val[2], vdup_n_u8(pi1_sao_offset[3]))

    ADD         v4.8b,  v21.8b ,  v26.8b    //band_table.val[3] = vadd_u8(band_table.val[3], vdup_n_u8(pi1_sao_offset[4]))
    BLT         SAO_BAND_POS_0

SAO_BAND_POS_28:                            //case 28

    cmhs        v25.8b,  v29.8b ,  v4.8b    //vcle_u8(band_table.val[3], vdup_n_u8(16))

    BNE         SAO_BAND_POS_29
    ORR         v4.8b,  v4.8b ,  v25.8b     //band_table.val[3] = vorr_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK

SAO_BAND_POS_29:                            //case 29
    CMP         x5,#29
    cmhs        v24.8b,  v29.8b ,  v3.8b    //vcle_u8(band_table.val[2], vdup_n_u8(16))

    BNE         SAO_BAND_POS_30
    ORR         v3.8b,  v3.8b ,  v24.8b     //band_table.val[2] = vorr_u8(band_table.val[2], au1_cmp)

    AND         v4.8b,  v4.8b ,  v25.8b     //band_table.val[3] = vand_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK

SAO_BAND_POS_30:                            //case 30
    CMP         x5,#30
    cmhs        v23.8b,  v29.8b ,  v2.8b    //vcle_u8(band_table.val[1], vdup_n_u8(16))

    BNE         SAO_BAND_POS_31
    ORR         v2.8b,  v2.8b ,  v23.8b     //band_table.val[1] = vorr_u8(band_table.val[1], au1_cmp)

    AND         v3.8b,  v3.8b ,  v24.8b     //band_table.val[2] = vand_u8(band_table.val[2], au1_cmp)
    B           SWITCH_BREAK

SAO_BAND_POS_31:                            //case 31
    CMP         x5,#31
    BNE         SWITCH_BREAK

    cmhs        v22.8b,  v29.8b ,  v1.8b    //vcle_u8(band_table.val[0], vdup_n_u8(16))
    ORR         v1.8b,  v1.8b ,  v22.8b     //band_table.val[0] = vorr_u8(band_table.val[0], au1_cmp)

    AND         v2.8b,  v2.8b ,  v23.8b     //band_table.val[1] = vand_u8(band_table.val[1], au1_cmp)

SAO_BAND_POS_0:
    CMP         x5,#0                       //case 0
    BNE         SWITCH_BREAK

    cmhs        v22.8b,  v29.8b ,  v1.8b    //vcle_u8(band_table.val[0], vdup_n_u8(16))
    AND         v1.8b,  v1.8b ,  v22.8b     //band_table.val[0] = vand_u8(band_table.val[0], au1_cmp)

SWITCH_BREAK:

    mov         v1.d[1],v2.d[0]
    mov         v2.d[0],v3.d[0]
    mov         v2.d[1],v4.d[0]

SWITCH_BREAK_1:

    MOV         x4,x0                       //pu1_src_cpy
    MOV         x11,x8                      //move ht
    ADD         x5,x4,x1

HEIGHT_LOOP:
    ADD         x6,x5,x1
    LD1         {v13.8b},[x4]               //au1_cur_row = vld1_u8(pu1_src_cpy)

    ADD         x10,x6,x1
    LD1         {v15.8b},[x5]               //au1_cur_row = vld1_u8(pu1_src_cpy)

    LD1         {v17.8b},[x6]               //au1_cur_row = vld1_u8(pu1_src_cpy)

    LD1         {v19.8b},[x10]              //au1_cur_row = vld1_u8(pu1_src_cpy)
    SUB         v14.8b,  v13.8b ,  v31.8b   //vsub_u8(au1_cur_row, band_pos)

    TBX         v13.8b, {v1.16b- v2.16b},v14.8b //vtbx4_u8(au1_cur_row, band_table, vsub_u8(au1_cur_row, band_pos))
    SUB         v16.8b,  v15.8b ,  v31.8b   //vsub_u8(au1_cur_row, band_pos)

    TBX         v15.8b, {v1.16b- v2.16b},v16.8b //vtbx4_u8(au1_cur_row, band_table, vsub_u8(au1_cur_row, band_pos))
    SUB         v18.8b,  v17.8b ,  v31.8b   //vsub_u8(au1_cur_row, band_pos)

    TBX         v17.8b, {v1.16b- v2.16b},v18.8b //vtbx4_u8(au1_cur_row, band_table, vsub_u8(au1_cur_row, band_pos))
    SUB         v20.8b,  v19.8b ,  v31.8b   //vsub_u8(au1_cur_row, band_pos)

    TBX         v19.8b, {v1.16b- v2.16b},v20.8b //vtbx4_u8(au1_cur_row, band_table, vsub_u8(au1_cur_row, band_pos))
    ST1         {v13.8b},[x4],x1            //vst1_u8(pu1_src_cpy, au1_cur_row)

    ST1         {v15.8b},[x5]               //vst1_u8(pu1_src_cpy, au1_cur_row)
    SUBS        x11,x11,#4                  //Decrement the ht loop count by 4

    ST1         {v17.8b},[x6],x1            //vst1_u8(pu1_src_cpy, au1_cur_row)

    ADD         x4,x6,x1
    ST1         {v19.8b},[x10]              //vst1_u8(pu1_src_cpy, au1_cur_row)
    ADD         x5,x4,x1

    BNE         HEIGHT_LOOP

    SUBS        x7,x7,#8                    //Decrement the width loop by 8
    ADD         x0,x0,#8
    BNE         SWITCH_BREAK_1

    // LDMFD sp!,{x4-x12,x15}               //Reload the registers from SP
    ldp         x19, x20,[sp], #16
    ldp         d8,d15,[sp],#16             // Loading d15 using { ldr d15,[sp]; add sp,sp,#8 } is giving bus error.
                                            // d8 is used as dummy register and loaded along with d15 using ldp. d8 is not used in the function.
    ldp         d13,d14,[sp],#16
    ret



