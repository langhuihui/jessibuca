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
//*  ihevc_sao_band_offset_chroma.s
//*
//* ,:brief
//*  Contains function definitions for inter prediction  interpolation.
//* Functions are coded using NEON  intrinsics and can be compiled using@ ARM
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
//void ihevc_sao_band_offset_chroma(UWORD8 *pu1_src,
//                           WORD32 src_strd,
//                           UWORD8 *pu1_src_left,
//                           UWORD8 *pu1_src_top,
//                           UWORD8 *pu1_src_top_left,
//                           WORD32 sao_band_pos_u,
//                           WORD32 sao_band_pos_v,
//                           WORD8 *pi1_sao_offset_u,
//                           WORD8 *pi1_sao_offset_v,
//                           WORD32 wd,
//                           WORD32 ht)
//
//**************Variables Vs Registers*****************************************
//x0 =>    *pu1_src
//x1 =>    src_strd
//x2 =>    *pu1_src_left
//x3 =>    *pu1_src_top
//x4    =>    *pu1_src_top_left 40
//x5    =>    sao_band_pos_u 44
//x6    =>    sao_band_pos_v 48
//x7    =>    *pi1_sao_offset_u 52
//x8    =>    *pi1_sao_offset_v 56
//x9    =>    wd 60
//x10=>    ht 64

.text
.p2align 2
.include "ihevc_neon_macros.s"

.globl gu1_table_band_idx
.globl ihevc_sao_band_offset_chroma_av8

ihevc_sao_band_offset_chroma_av8:
    mov         x8,#0
    mov         x9,#0
    mov         x10,#0

    ldr         x8,[sp,#0]
    ldr         w9,[sp,#8]
    ldr         w10,[sp,#16]

    push_v_regs
    // stmfd sp!, {x4-x12, x14}                //stack stores the values of the arguments
    stp         x19, x20,[sp,#-16]!
    stp         x21, x22,[sp,#-16]!
    stp         x23, x24,[sp,#-16]!

    mov         x15,x4 // pu1_src_top_left 40
    mov         x16,x5 // sao_band_pos_u 44
    mov         x17,x6 // sao_band_pos_v 48
    mov         x19,x7 // pi1_sao_offset_u 52
    mov         x20,x8 // pi1_sao_offset_v 56
    mov         x21,x9 // wd 60
    mov         x22,x10 // ht 64

    MOV         x4, x15                     //Loads pu1_src_top_left
    MOV         x10, x22                    //Loads ht

    MOV         x9, x21                     //Loads wd
    MOV         x11,x10                     //Move the ht to x9 for loop counter

    ADD         x12,x0,x9                   //pu1_src[row * src_strd + (wd)]
    ADRP        x14, :got:gu1_table_band_idx
    LDR         x14, [x14, #:got_lo12:gu1_table_band_idx]

    SUB         x12,x12,#2                  //wd-2

SRC_LEFT_LOOP:
    LDRH        w5,[x12]                    //Load the value
    ADD         x12,x12,x1
    SUBS        x11,x11,#1                  //Decrement the loop counter
    STRH        w5,[x2],#2                  //Store the value in pu1_src_left pointer
    BNE         SRC_LEFT_LOOP

    MOV         x5, x16                     //Loads sao_band_pos_u
    LD1         {v1.8b},[x14],#8            //band_table_u.val[0]
    ADD         x12,x3,x9                   //pu1_src_top[wd]

    sub         x23,x12,#2
    LDRH        w11,[x23]
    LD1         {v2.8b},[x14],#8            //band_table_u.val[1]
    LSL         x6,x5,#3                    //sao_band_pos_u

    STRH        w11,[x4]                    //store to pu1_src_top_left[0]
    LD1         {v3.8b},[x14],#8            //band_table_u.val[2]
    MOV         x7, x19                     //Loads pi1_sao_offset_u

    SUB         x4,x10,#1                   //ht-1
    dup         v31.8b,w6                   //band_pos_u
    mul         x4, x4, x1                  //ht-1 * src_strd

    ADD         x4,x4,x0                    //pu1_src[(ht - 1) * src_strd]
    LD1         {v4.8b},[x14],#8            //band_table_u.val[3]
    MOV         x11,x9                      //Move the wd to x9 for loop counter

SRC_TOP_LOOP:                               //wd is always multiple of 8
    LD1         {v0.8b},[x4],#8             //Load pu1_src[(ht - 1) * src_strd + col]
    SUBS        x11,x11,#8                  //Decrement the loop counter by 8
    ST1         {v0.8b},[x3],#8             //Store to pu1_src_top[col]
    BNE         SRC_TOP_LOOP

    LD1         {v30.8b},[x7]               //pi1_sao_offset_u load
    ADD         v5.8b,  v1.8b ,  v31.8b     //band_table_u.val[0] = vadd_u8(band_table_u.val[0], sao_band_pos_u)

    dup         v29.8b, v30.b[1]            //vdup_n_u8(pi1_sao_offset_u[1])
    ADD         v6.8b,  v2.8b ,  v31.8b     //band_table_u.val[1] = vadd_u8(band_table_u.val[1], sao_band_pos_u)

    dup         v28.8b, v30.b[2]            //vdup_n_u8(pi1_sao_offset_u[2])
    ADD         v7.8b,  v3.8b ,  v31.8b     //band_table_u.val[2] = vadd_u8(band_table_u.val[2], sao_band_pos_u)

    dup         v27.8b, v30.b[3]            //vdup_n_u8(pi1_sao_offset_u[3])
    ADD         v8.8b,  v4.8b ,  v31.8b     //band_table_u.val[3] = vadd_u8(band_table_u.val[3], sao_band_pos_u)

    CMP         x5,#28
    dup         v26.8b, v30.b[4]            //vdup_n_u8(pi1_sao_offset_u[4])
    ADRP        x14, :got:gu1_table_band_idx
    LDR         x14, [x14, #:got_lo12:gu1_table_band_idx]

    movi        v30.8b, #16                 //vdup_n_u8(16)
    ADD         v1.8b,  v5.8b ,  v29.8b     //band_table_u.val[0] = vadd_u8(band_table_u.val[0], vdup_n_u8(pi1_sao_offset_u[1]))

    LD1         {v9.8b},[x14],#8            //band_table_v.val[0]
    ADD         v2.8b,  v6.8b ,  v28.8b     //band_table_u.val[1] = vadd_u8(band_table_u.val[1], vdup_n_u8(pi1_sao_offset_u[2]))

    LD1         {v10.8b},[x14],#8           //band_table_v.val[1]
    ADD         v3.8b,  v7.8b ,  v27.8b     //band_table_u.val[2] = vadd_u8(band_table_u.val[2], vdup_n_u8(pi1_sao_offset_u[3]))

    MOV         x6, x17                     //Loads sao_band_pos_v
    ADD         v4.8b,  v8.8b ,  v26.8b     //band_table_u.val[3] = vadd_u8(band_table_u.val[3], vdup_n_u8(pi1_sao_offset_u[4]))
    LSL         x11,x6,#3                   //sao_band_pos_v

    BLT         SAO_BAND_POS_U_0

SAO_BAND_POS_U_28:                          //case 28
    cmhs        v13.8b,  v30.8b ,  v4.8b    //vcle_u8(band_table.val[3], vdup_n_u8(16))
    BNE         SAO_BAND_POS_U_29

    ORR         v4.8b,  v4.8b ,  v13.8b     //band_table.val[3] = vorr_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK_U

SAO_BAND_POS_U_29:                          //case 29
    CMP         x5,#29

    cmhs        v14.8b,  v30.8b ,  v3.8b    //vcle_u8(band_table.val[2], vdup_n_u8(16))
    BNE         SAO_BAND_POS_U_30
    ORR         v3.8b,  v3.8b ,  v14.8b     //band_table.val[2] = vorr_u8(band_table.val[2], au1_cmp)

    AND         v4.8b,  v4.8b ,  v13.8b     //band_table.val[3] = vand_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK_U

SAO_BAND_POS_U_30:                          //case 30
    CMP         x5,#30

    cmhs        v15.8b,  v30.8b ,  v2.8b    //vcle_u8(band_table.val[1], vdup_n_u8(16))
    BNE         SAO_BAND_POS_U_31
    ORR         v2.8b,  v2.8b ,  v15.8b     //band_table.val[1] = vorr_u8(band_table.val[1], au1_cmp)

    AND         v3.8b,  v3.8b ,  v14.8b     //band_table.val[2] = vand_u8(band_table.val[2], au1_cmp)

SAO_BAND_POS_U_31:                          //case 31
    CMP         x5,#31
    BNE         SWITCH_BREAK_U

    cmhs        v16.8b,  v30.8b ,  v1.8b    //vcle_u8(band_table.val[0], vdup_n_u8(16))
    ORR         v1.8b,  v1.8b ,  v16.8b     //band_table.val[0] = vorr_u8(band_table.val[0], au1_cmp)

    AND         v2.8b,  v2.8b ,  v15.8b     //band_table.val[1] = vand_u8(band_table.val[1], au1_cmp)
    B           SWITCH_BREAK_U

SAO_BAND_POS_U_0:
    CMP         x5,#0                       //case 0
    BNE         SWITCH_BREAK_U

    cmhs        v16.8b,  v30.8b ,  v1.8b    //vcle_u8(band_table.val[0], vdup_n_u8(16))
    AND         v1.8b,  v1.8b ,  v16.8b     //band_table.val[0] = vand_u8(band_table.val[0], au1_cmp)

SWITCH_BREAK_U:
    dup         v30.8b,w11                  //band_pos_v
    MOV         x8, x20                     //Loads pi1_sao_offset_v

    LD1         {v11.8b},[x14],#8           //band_table_v.val[2]
    ADD         v13.8b,  v9.8b ,  v30.8b    //band_table_v.val[0] = vadd_u8(band_table_v.val[0], band_pos_v)

    LD1         {v12.8b},[x14],#8           //band_table_v.val[3]
    ADD         v14.8b,  v10.8b ,  v30.8b   //band_table_v.val[1] = vadd_u8(band_table_v.val[1], band_pos_v)

    LD1         {v25.8b},[x8]               //pi1_sao_offset_v load
    ADD         v15.8b,  v11.8b ,  v30.8b   //band_table_v.val[2] = vadd_u8(band_table_v.val[2], band_pos_v)

    dup         v29.8b, v25.b[1]            //vdup_n_u8(pi1_sao_offset_v[1])
    ADD         v16.8b,  v12.8b ,  v30.8b   //band_table_v.val[3] = vadd_u8(band_table_v.val[3], band_pos_v)

    dup         v28.8b, v25.b[2]            //vdup_n_u8(pi1_sao_offset_v[2])
    ADD         v9.8b,  v13.8b ,  v29.8b    //band_table_v.val[0] = vadd_u8(band_table_v.val[0], vdup_n_u8(pi1_sao_offset_v[1]))

    dup         v27.8b, v25.b[3]            //vdup_n_u8(pi1_sao_offset_v[3])
    ADD         v10.8b,  v14.8b ,  v28.8b   //band_table_v.val[1] = vadd_u8(band_table_v.val[1], vdup_n_u8(pi1_sao_offset_v[2]))

    dup         v26.8b, v25.b[4]            //vdup_n_u8(pi1_sao_offset_v[4])
    ADD         v11.8b,  v15.8b ,  v27.8b   //band_table_v.val[2] = vadd_u8(band_table_v.val[2], vdup_n_u8(pi1_sao_offset_v[3]))

    movi        v29.8b, #16                 //vdup_n_u8(16)
    ADD         v12.8b,  v16.8b ,  v26.8b   //band_table_v.val[3] = vadd_u8(band_table_v.val[3], vdup_n_u8(pi1_sao_offset_v[4]))
    AND         x12,x9,#0xf

    CMP         x6,#28
    BLT         SAO_BAND_POS_V_0

SAO_BAND_POS_V_28:                          //case 28
    cmhs        v17.8b,  v29.8b ,  v12.8b   //vcle_u8(band_table.val[3], vdup_n_u8(16))
    BNE         SAO_BAND_POS_V_29
    ORR         v12.8b,  v12.8b ,  v17.8b   //band_table.val[3] = vorr_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK_V

SAO_BAND_POS_V_29:                          //case 29
    CMP         x6,#29

    cmhs        v18.8b,  v29.8b ,  v11.8b   //vcle_u8(band_table.val[2], vdup_n_u8(16))
    BNE         SAO_BAND_POS_V_30
    ORR         v11.8b,  v11.8b ,  v18.8b   //band_table.val[2] = vorr_u8(band_table.val[2], au1_cmp)

    AND         v12.8b,  v12.8b ,  v17.8b   //band_table.val[3] = vand_u8(band_table.val[3], au1_cmp)
    B           SWITCH_BREAK_V

SAO_BAND_POS_V_30:                          //case 30
    CMP         x6,#30

    cmhs        v19.8b,  v29.8b ,  v10.8b   //vcle_u8(band_table.val[1], vdup_n_u8(16))
    BNE         SAO_BAND_POS_V_31
    ORR         v10.8b,  v10.8b ,  v19.8b   //band_table.val[1] = vorr_u8(band_table.val[1], au1_cmp)

    AND         v11.8b,  v11.8b ,  v18.8b   //band_table.val[2] = vand_u8(band_table.val[2], au1_cmp)
    B           SWITCH_BREAK_V

SAO_BAND_POS_V_31:                          //case 31
    CMP         x6,#31
    BNE         SWITCH_BREAK_V

    cmhs        v20.8b,  v29.8b ,  v9.8b    //vcle_u8(band_table.val[0], vdup_n_u8(16))
    ORR         v9.8b,  v9.8b ,  v20.8b     //band_table.val[0] = vorr_u8(band_table.val[0], au1_cmp)

    AND         v10.8b,  v10.8b ,  v19.8b   //band_table.val[1] = vand_u8(band_table.val[1], au1_cmp)
    B           SWITCH_BREAK_V

SAO_BAND_POS_V_0:
    CMP         x6,#0                       //case 0
    BNE         SWITCH_BREAK_V

    cmhs        v20.8b,  v29.8b ,  v9.8b    //vcle_u8(band_table.val[0], vdup_n_u8(16))
    AND         v9.8b,  v9.8b ,  v20.8b     //band_table.val[0] = vand_u8(band_table.val[0], au1_cmp)

SWITCH_BREAK_V:
    CMP         x9,#16
    MOV         x4,x0                       //pu1_src_cpy
    mov         v1.d[1],v2.d[0]
    mov         v2.d[0],v3.d[0]
    mov         v2.d[1],v4.d[0]
    mov         v9.d[1],v10.d[0]
    mov         v10.d[0],v11.d[0]
    mov         v10.d[1],v12.d[0]
    BLT         WIDTH_RESIDUE

WIDTH_LOOP:                                 //Width is assigned to be multiple of 16
    MOV         x4,x0                       //pu1_src_cpy
    MOV         x11,x10                     //move ht
    ADD         x5,x4,x1

HEIGHT_LOOP:                                //unrolled for 4 rows

    ADD         x6,x5,x1
    LD2         {v5.8b, v6.8b},[x4]         //vld1q_u8(pu1_src_cpy)
    ADD         x7,x6,x1

    LD2         {v13.8b, v14.8b},[x5]       //vld1q_u8(pu1_src_cpy)
    SUB         v7.8b,  v5.8b ,  v31.8b     //vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    LD2         {v17.8b, v18.8b},[x6]       //vld1q_u8(pu1_src_cpy)
    SUB         v8.8b,  v6.8b ,  v30.8b     //vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    LD2         {v21.8b, v22.8b},[x7]       //vld1q_u8(pu1_src_cpy)
    SUB         v15.8b,  v13.8b ,  v31.8b   //vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    TBX         v5.8b, {v1.16b- v2.16b},v7.8b //vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    SUB         v16.8b,  v14.8b ,  v30.8b   //vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    TBX         v6.8b, {v9.16b- v10.16b},v8.8b //vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    SUB         v19.8b,  v17.8b ,  v31.8b   //vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    TBX         v13.8b, {v1.16b- v2.16b},v15.8b //vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    SUB         v20.8b,  v18.8b ,  v30.8b   //vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    TBX         v14.8b, {v9.16b- v10.16b},v16.8b //vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    SUB         v23.8b,  v21.8b ,  v31.8b   //vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    ST2         {v5.8b, v6.8b},[x4]         //vst1q_u8(pu1_src_cpy, au1_cur_row)
    SUB         v24.8b,  v22.8b ,  v30.8b   //vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    SUBS        x11,x11,#4                  //Decrement the ht loop count by 4
    TBX         v17.8b, {v1.16b- v2.16b},v19.8b //vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))

    ST2         {v13.8b, v14.8b},[x5]       //vst1q_u8(pu1_src_cpy, au1_cur_row)

    TBX         v18.8b, {v9.16b- v10.16b},v20.8b //vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    TBX         v21.8b, {v1.16b- v2.16b},v23.8b //vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    TBX         v22.8b, {v9.16b- v10.16b},v24.8b //vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))

    ST2         {v17.8b, v18.8b},[x6],x1    //vst1q_u8(pu1_src_cpy, au1_cur_row)

    ADD         x4,x6,x1
    ST2         {v21.8b, v22.8b},[x7]       //vst1q_u8(pu1_src_cpy, au1_cur_row)
    ADD         x5,x4,x1

    BNE         HEIGHT_LOOP

    SUB         x9,x9,#16                   //Decrement the width loop by 16
    ADD         x0,x0,#16
    CMP         x9,#8
    BGT         WIDTH_LOOP
    BLT         END_LOOP
    MOV         x4,x0                       //pu1_src_cpy

WIDTH_RESIDUE:                              //If width is not multiple of 16

    ADD         x5,x4,x1
    LD2         {v5.8b, v6.8b},[x4]         //vld1q_u8(pu1_src_cpy)
    ADD         x6,x5,x1

    ADD         x7,x6,x1
    LD2         {v13.8b, v14.8b},[x5]       //vld1q_u8(pu1_src_cpy)
    SUB         v7.8b,  v5.8b ,  v31.8b     //vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    LD2         {v17.8b, v18.8b},[x6]       //vld1q_u8(pu1_src_cpy)
    SUB         v8.8b,  v6.8b ,  v30.8b     //vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    TBX         v5.8b, {v1.16b- v2.16b},v7.8b //vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    SUB         v15.8b,  v13.8b ,  v31.8b   //vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    TBX         v6.8b, {v9.16b- v10.16b},v8.8b //vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    SUB         v16.8b,  v14.8b ,  v30.8b   //vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    LD2         {v21.8b, v22.8b},[x7]       //vld1q_u8(pu1_src_cpy)
    SUB         v19.8b,  v17.8b ,  v31.8b   //vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    TBX         v13.8b, {v1.16b- v2.16b},v15.8b //vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    SUB         v20.8b,  v18.8b ,  v30.8b   //vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    TBX         v14.8b, {v9.16b- v10.16b},v16.8b //vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    ZIP1        v28.8b, v5.8b, v6.8b
    ZIP2        v6.8b, v5.8b, v6.8b
    mov         v5.8b, v28.8b

    TBX         v17.8b, {v1.16b- v2.16b},v19.8b //vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    SUB         v23.8b,  v21.8b ,  v31.8b   //vsub_u8(au1_cur_row_deint.val[0], band_pos_u)

    ST1         {v5.8b},[x4]                //vst1q_u8(pu1_src_cpy, au1_cur_row)
    ZIP1        v28.8b, v13.8b, v14.8b
    ZIP2        v14.8b, v13.8b, v14.8b
    mov         v13.8b, v28.8b

    TBX         v18.8b, {v9.16b- v10.16b},v20.8b //vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    SUB         v24.8b,  v22.8b ,  v30.8b   //vsub_u8(au1_cur_row_deint.val[1], band_pos_v)

    ST1         {v13.8b},[x5]               //vst1q_u8(pu1_src_cpy, au1_cur_row)
    SUBS        x10,x10,#4                  //Decrement the ht loop count by 4

    TBX         v21.8b, {v1.16b- v2.16b},v23.8b //vtbx4_u8(au1_cur_row_deint.val[0], band_table_u, vsub_u8(au1_cur_row_deint.val[0], band_pos_u))
    ZIP1        v28.8b, v17.8b, v18.8b
    ZIP2        v18.8b, v17.8b, v18.8b
    mov         v17.8b, v28.8b

    TBX         v22.8b, {v9.16b- v10.16b},v24.8b //vtbx4_u8(au1_cur_row_deint.val[1], band_table_v, vsub_u8(au1_cur_row_deint.val[1], band_pos_v))
    ST1         {v17.8b},[x6],x1            //vst1q_u8(pu1_src_cpy, au1_cur_row)
    ZIP1        v28.8b, v21.8b, v22.8b
    ZIP2        v22.8b, v21.8b, v22.8b
    mov         v21.8b, v28.8b

    ADD         x4,x6,x1
    ST1         {v21.8b},[x7]               //vst1q_u8(pu1_src_cpy, au1_cur_row)
    ADD         x5,x4,x1

    BNE         WIDTH_RESIDUE

END_LOOP:
    // LDMFD sp!,{x4-x12,x15}            //Reload the registers from SP
    ldp         x23, x24,[sp],#16
    ldp         x21, x22,[sp],#16
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret



