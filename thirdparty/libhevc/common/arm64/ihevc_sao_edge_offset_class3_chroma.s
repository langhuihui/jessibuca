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
//*  ihevc_sao_edge_offset_class3_chroma.s
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
//void ihevc_sao_edge_offset_class3_chroma(UWORD8 *pu1_src,
//                              WORD32 src_strd,
//                              UWORD8 *pu1_src_left,
//                              UWORD8 *pu1_src_top,
//                              UWORD8 *pu1_src_top_left,
//                              UWORD8 *pu1_src_top_right,
//                              UWORD8 *pu1_src_bot_left,
//                              UWORD8 *pu1_avail,
//                              WORD8 *pi1_sao_offset_u,
//                              WORD8 *pi1_sao_offset_v,
//                              WORD32 wd,
//                              WORD32 ht)
//**************Variables Vs Registers*****************************************
//x0 =>    *pu1_src
//x1 =>    src_strd
//x2 =>    *pu1_src_left
//x3 =>    *pu1_src_top
//x4    =>    *pu1_src_top_left
//x5    =>    *pu1_avail
//x6    =>    *pi1_sao_offset_u
//x9 =>  *pi1_sao_offset_v
//x7    =>    wd
//x8=>    ht

.text
.p2align 2
.include "ihevc_neon_macros.s"
.globl gi1_table_edge_idx
.globl ihevc_sao_edge_offset_class3_chroma_av8

ihevc_sao_edge_offset_class3_chroma_av8:


    // STMFD sp!,{x4-x12,x14}            //stack stores the values of the arguments


    ldr         x8,[sp,#0]
    ldr         x9,[sp,#8]
    ldr         w10,[sp,#16]
    ldr         w11,[sp,#24]


    // STMFD sp!, {x4-x12, x14}            //stack stores the values of the arguments
    stp         x19, x20,[sp,#-16]!
    stp         x21, x22,[sp,#-16]!
    stp         x23, x24,[sp,#-16]!
    stp         x25, x26,[sp,#-16]!
    stp         x27, x28,[sp,#-16]!

    mov         x15,x4 // *pu1_src_top_left 0x28
    mov         x16,x5 // *pu1_src_top_right 0x2c
    mov         x17,x6 // *pu1_src_bot_left 0x30
    mov         x21,x7 // *pu1_avail 0x34
    mov         x22,x8 // *pi1_sao_offset_u 0x38
    mov         x23,x9 // *pi1_sao_offset_v 0x3c
    mov         x24,x10 // wd 0x40
    mov         x25,x11 // ht 0x44


    mov         w7, w24                     //Loads wd
    mov         w8, w25                     //Loads ht
    SUB         x9,x7,#2                    //wd - 2

    mov         x4, x15                     //Loads pu1_src_top_left
    LDRH        w10,[x3,x9]                 //pu1_src_top[wd - 2]

    MOV         x9,x7                       //Move width to x9 for loop count

    mov         x5, x21                     //Loads pu1_avail
    mov         x6, x22                     //Loads pi1_sao_offset_u

    mov         x22, x3                     //Store pu1_src_top in sp
    SUB         sp,sp,#0xE0                 //Decrement the stack pointer to store some temp arr values

    STRH        w10,[sp]                    //u1_src_top_left_tmp = pu1_src_top[wd - 2]
    SUB         x10,x8,#1                   //ht-1
    madd        x11, x10, x1, x0            //pu1_src[(ht - 1) * src_strd + col]
    ADD         x12,sp,#10                  //temp array

AU1_SRC_TOP_LOOP:
    LD1         {v0.8b},[x11],#8            //pu1_src[(ht - 1) * src_strd + col]
    SUBS        x9,x9,#8                    //Decrement the loop count by 8
    ST1         {v0.8b},[x12],#8            //au1_src_top_tmp[col] = pu1_src[(ht - 1) * src_strd + col]
    BNE         AU1_SRC_TOP_LOOP

PU1_AVAIL_5_LOOP_U:
    LDRB        w9,[x5,#5]                  //pu1_avail[5]
    CMP         x9,#0
    SUB         x14,x7,#2                   //[wd - 2]
    LDRB        w9,[x0,x14]                 //u1_pos_0_0_tmp_u = pu1_src[wd - 2]
    SUB         x11,x7,#1                   //[wd - 1]
    LDRB        w10,[x0,x11]                //u1_pos_0_0_tmp_v = pu1_src[wd - 1]
    BEQ         PU1_AVAIL_6_LOOP_U

    mov         x11, x16                    //Load pu1_src_top_right from sp
    LDRB        w11,[x11]                   //pu1_src_top_right[0]
    SUB         x12,x9,x11                  //pu1_src[wd - 2] - pu1_src_top_right[0]
    CMP         x12,#0
    movn        x20,#0
    csel        x12, x20, x12,LT
    MOV         x20,#1
    csel        x12, x20, x12,GT            //SIGN(pu1_src[wd - 2] - pu1_src_top_right[0])
    ADD         x11,x0,x1                   //pu1_src + src_strd
    SUB         x14,x14,#2                  //[wd - 2 - 2]
    LDRB        w14,[x11,x14]               //pu1_src[wd - 2 - 2 + src_strd]
    SUB         x11,x9,x14                  //pu1_src[wd - 2] - pu1_src[wd - 2 - 2 + src_strd]
    CMP         x11,#0
    movn        x20,#0
    csel        x11, x20, x11,LT
    MOV         x20,#1
    csel        x11, x20, x11,GT            //SIGN(pu1_src[wd - 2] - pu1_src[wd - 2 - 2 + src_strd])
    ADD         x11,x12,x11                 //SIGN(pu1_src[wd - 2] - pu1_src_top_right[0]) +  SIGN(pu1_src[wd - 2] - pu1_src[wd - 2 - 2 + src_strd])
    ADD         x11,x11,#2                  //edge_idx
    ADRP        x14, :got:gi1_table_edge_idx //table pointer
    LDR         x14, [x14, #:got_lo12:gi1_table_edge_idx]

    LDRSB       x12,[x14,x11]               //edge_idx = gi1_table_edge_idx[edge_idx]
    CMP         x12,#0                      //0 != edge_idx
    BEQ         PU1_AVAIL_5_LOOP_V
    LDRSB       x11,[x6,x12]                //pi1_sao_offset_u[edge_idx]
    ADD         x9,x9,x11                   //pu1_src[wd - 2] + pi1_sao_offset_u[edge_idx]
    mov         x20,#255
    cmp         x9,x20
    csel        x9, x20, x9, ge             //u1_pos_0_0_tmp_u = CLIP3(pu1_src[wd - 2] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1)
    mov         x20,#0
    cmp         x9,x20
    csel        x9, x20, x9, LT             //u1_pos_0_0_tmp_u = CLIP3(pu1_src[wd - 2] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1)

PU1_AVAIL_5_LOOP_V:

    mov         x11, x16                    //Load pu1_src_top_right from sp
    LDRB        w11,[x11,#1]                //pu1_src_top_right[1]
    SUB         x12,x10,x11                 //pu1_src[wd - 1] - pu1_src_top_right[1]
    CMP         x12,#0
    movn        x20,#0
    csel        x12, x20, x12,LT
    MOV         x20,#1
    csel        x12, x20, x12,GT            //SIGN(pu1_src[wd - 1] - pu1_src_top_right[1])
    ADD         x11,x0,x1                   //pu1_src + src_strd
    SUB         x14,x7,#3                   //[wd - 1 - 2]
    LDRB        w14,[x11,x14]               //pu1_src[wd - 1 - 2 + src_strd]
    SUB         x11,x10,x14                 //pu1_src[wd - 1] - pu1_src[wd - 1 - 2 + src_strd]
    CMP         x11,#0
    movn        x20,#0
    csel        x11, x20, x11,LT
    MOV         x20,#1
    csel        x11, x20, x11,GT            //SIGN(pu1_src[wd - 1] - pu1_src[wd - 1 - 2 + src_strd])
    ADD         x11,x12,x11                 //SIGN(pu1_src[wd - 1] - pu1_src_top_right[1]) +  SIGN(pu1_src[wd - 1] - pu1_src[wd - 1 - 2 + src_strd])
    ADD         x11,x11,#2                  //edge_idx
    ADRP        x14, :got:gi1_table_edge_idx //table pointer
    LDR         x14, [x14, #:got_lo12:gi1_table_edge_idx]

    LDRSB       x12,[x14,x11]               //edge_idx = gi1_table_edge_idx[edge_idx]
    CMP         x12,#0                      //0 != edge_idx
    BEQ         PU1_AVAIL_6_LOOP_U
    mov         x11, x23                    //Loads pi1_sao_offset_v
    LDRSB       x11,[x11,x12]               //pi1_sao_offset_v[edge_idx]
    ADD         x10,x10,x11                 //pu1_src[wd - 1] + pi1_sao_offset_v[edge_idx]
    mov         x20,#255
    cmp         x10,x20
    csel        x10, x20, x10, ge           //u1_pos_0_0_tmp_v = CLIP3(pu1_src[wd - 1] + pi1_sao_offset_v[edge_idx], 0, (1 << bit_depth) - 1)
    mov         x20,#0
    cmp         x10,x20
    csel        x10, x20, x10, LT           //u1_pos_0_0_tmp_v = CLIP3(pu1_src[wd - 1] + pi1_sao_offset_v[edge_idx], 0, (1 << bit_depth) - 1)

PU1_AVAIL_6_LOOP_U:
    STRB        w9,[sp,#6]
    STRB        w10,[sp,#7]
    mov         x26, x0                     //Store pu1_src in sp

    LDRB        w10,[x5,#6]                 //pu1_avail[6]
    CMP         x10,#0
    SUB         x11,x8,#1                   //ht - 1
    madd        x12, x11, x1, x0            //pu1_src[(ht - 1) * src_strd]
    LDRB        w10,[x12]                   //u1_pos_wd_ht_tmp_u = pu1_src[(ht - 1) * src_strd]
    LDRB        w9,[x12,#1]                 //u1_pos_wd_ht_tmp_v = pu1_src[(ht - 1) * src_strd + 1]
    BEQ         PU1_AVAIL_3_LOOP

    SUB         x11,x12,x1                  //pu1_src[(ht - 1) * src_strd - src_strd]
    ADD         x11,x11,#2                  //pu1_src[(ht - 1) * src_strd +  2 - src_strd]
    LDRB        w11,[x11]                   //Load pu1_src[(ht - 1) * src_strd +  2 - src_strd]
    SUB         x11,x10,x11                 //pu1_src[(ht - 1) * src_strd] - pu1_src[(ht - 1) * src_strd +  2 - src_strd]
    CMP         x11,#0
    movn        x20,#0
    csel        x11, x20, x11,LT
    MOV         x20,#1
    csel        x11, x20, x11,GT            //SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src[(ht - 1) * src_strd +  2 - src_strd])

    mov         x14, x17                    //Load pu1_src_bot_left from sp
    LDRB        w14,[x14]                   //Load pu1_src_bot_left[0]
    SUB         x14,x10,x14                 //pu1_src[(ht - 1) * src_strd] - pu1_src_bot_left[0]
    CMP         x14,#0
    movn        x20,#0
    csel        x14, x20, x14,LT
    MOV         x20,#1
    csel        x14, x20, x14,GT            //SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src_bot_left[0])

    ADD         x11,x11,x14                 //Add 2 sign value
    ADD         x11,x11,#2                  //edge_idx
    ADRP        x14, :got:gi1_table_edge_idx //table pointer
    LDR         x14, [x14, #:got_lo12:gi1_table_edge_idx]

    LDRSB       x14,[x14,x11]               //edge_idx = gi1_table_edge_idx[edge_idx]
    CMP         x14,#0
    BEQ         PU1_AVAIL_6_LOOP_V
    LDRSB       x11,[x6,x14]                //pi1_sao_offset_u[edge_idx]
    ADD         x10,x10,x11                 //pu1_src[(ht - 1) * src_strd] + pi1_sao_offset[edge_idx]
    mov         x20,#255
    cmp         x10,x20
    csel        x10, x20, x10, ge           //u1_pos_wd_ht_tmp = CLIP3(pu1_src[(ht - 1) * src_strd] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1)
    mov         x20,#0
    cmp         x10,x20
    csel        x10, x20, x10, LT           //u1_pos_wd_ht_tmp = CLIP3(pu1_src[(ht - 1) * src_strd] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1)

PU1_AVAIL_6_LOOP_V:
    ADD         x12,x12,#1                  //pu1_src[(ht - 1) * src_strd + 1]
    SUB         x11,x12,x1                  //pu1_src[(ht - 1) * src_strd + 1) - src_strd]
    ADD         x11,x11,#2                  //pu1_src[(ht - 1) * src_strd + 2 - src_strd]
    LDRB        w11,[x11]                   //Load pu1_src[(ht - 1) * src_strd + 2 - src_strd]
    SUB         x11,x9,x11                  //pu1_src[(ht - 1) * src_strd + 1] - pu1_src[(ht - 1) * src_strd + 1 + 2 - src_strd]
    CMP         x11,#0
    movn        x20,#0
    csel        x11, x20, x11,LT
    MOV         x20,#1
    csel        x11, x20, x11,GT            //SIGN(pu1_src[(ht - 1) * src_strd + 1] - pu1_src[(ht - 1) * src_strd + 1 + 2 - src_strd])

    mov         x14, x17                    //Load pu1_src_bot_left from sp
    LDRB        w14,[x14,#1]                //Load pu1_src_bot_left[1]
    SUB         x14,x9,x14                  //pu1_src[(ht - 1) * src_strd + 1] - pu1_src_bot_left[1]
    CMP         x14,#0
    movn        x20,#0
    csel        x14, x20, x14,LT
    MOV         x20,#1
    csel        x14, x20, x14,GT            //SIGN(pu1_src[(ht - 1) * src_strd + 1] - pu1_src_bot_left[1])

    ADD         x11,x11,x14                 //Add 2 sign value
    ADD         x11,x11,#2                  //edge_idx
    ADRP        x14, :got:gi1_table_edge_idx //table pointer
    LDR         x14, [x14, #:got_lo12:gi1_table_edge_idx]

    LDRSB       x12,[x14,x11]               //edge_idx = gi1_table_edge_idx[edge_idx]
    CMP         x12,#0
    BEQ         PU1_AVAIL_3_LOOP
    mov         x14, x23                    //Loads pi1_sao_offset_v
    LDRSB       x11,[x14,x12]               //pi1_sao_offset_v[edge_idx]
    ADD         x9,x9,x11                   //pu1_src[(ht - 1) * src_strd] + pi1_sao_offset[edge_idx]
    mov         x20,#255
    cmp         x9,x20
    csel        x9, x20, x9, ge             //u1_pos_wd_ht_tmp_v = CLIP3(pu1_src[(ht - 1) * src_strd] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1)
    mov         x20,#0
    cmp         x9,x20
    csel        x9, x20, x9, LT             //u1_pos_wd_ht_tmp_v = CLIP3(pu1_src[(ht - 1) * src_strd] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1)

PU1_AVAIL_3_LOOP:
    STRB        w10,[sp,#8]
    STRB        w9,[sp,#9]
    mov         x27, x2                     //Store pu1_src_left in sp

    MOV         x12,x8                      //Move ht
    MOV         x14,x2                      //Move pu1_src_left to pu1_src_left_cpy
    LDRB        w11,[x5,#3]                 //pu1_avail[3]
    CMP         x11,#0
    BNE         PU1_AVAIL_2_LOOP
    SUB         x12,x12,#1                  //ht_tmp--

PU1_AVAIL_2_LOOP:
    LDRB        w5,[x5,#2]                  //pu1_avail[2]
    CMP         x5,#0
    BNE         PU1_AVAIL_2_LOOP_END

    ADD         x0,x0,x1                    //pu1_src += src_strd
    SUB         x12,x12,#1                  //ht_tmp--
    ADD         x14,x14,#2                  //pu1_src_left_cpy += 2

PU1_AVAIL_2_LOOP_END:
    mov         x28, x0                     //Store pu1_src in sp
    movi        v0.16b, #2                  //const_2 = vdupq_n_s8(2)
    movi        v2.8h, #0                   //const_min_clip = vdupq_n_s16(0)
    movi        v4.8h, #255                 //const_max_clip = vdupq_n_u16((1 << bit_depth) - 1)
    LD1         {v6.8b},[x6]                //offset_tbl_u = vld1_s8(pi1_sao_offset_u)
    mov         x6, x23                     //Loads pi1_sao_offset_v
    LD1         {v7.8b},[x6]                //offset_tbl_v = vld1_s8(pi1_sao_offset_v)
    ADRP        x2, :got:gi1_table_edge_idx //table pointer
    LDR         x2, [x2, #:got_lo12:gi1_table_edge_idx]

    //VLD1.8        D6,[x6]                        @edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    movi        v1.16b, #0xFF               //au1_mask = vdupq_n_s8(-1)
    MOV         x6,x7                       //move wd to x6 loop_count

    CMP         x7,#16                      //Compare wd with 16
    BLT         WIDTH_RESIDUE               //If not jump to WIDTH_RESIDUE where loop is unrolled for 8 case
    CMP         x8,#4                       //Compare ht with 4
    BLE         WD_16_HT_4_LOOP             //If jump to WD_16_HT_4_LOOP

WIDTH_LOOP_16:
    mov         w7, w24                     //Loads wd
    CMP         x6,x7                       //col == wd
    mov         x5, x21                     //Loads pu1_avail

    LDRb        w20, [x5]                   //pu1_avail[0]
    csel        w8,w20,w8,EQ
    MOV         x20,#-1
    csel        x8, x20, x8,NE

    mov         v1.b[0], w8                 //au1_mask = vsetq_lane_s8(-1, au1_mask, 0)
    LDRB        w11,[x5,#2]                 //pu1_avail[2]

    CMP         x6,#16                      //if(col == 16)
    mov         v1.b[1], w8                 //au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

    BNE         SKIP_AU1_MASK_VAL
    LDRB        w8,[x5,#1]                  //pu1_avail[1]
    mov         v1.b[14], w8                //au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)
    mov         v1.b[15], w8                //au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)

SKIP_AU1_MASK_VAL:
    CMP         x11,#0
    LD1         {v5.16b},[x0]               //pu1_cur_row = vld1q_u8(pu1_src)
    //LD1 {v13.8b},[x0]                        //pu1_cur_row = vld1q_u8(pu1_src)
    //SUB x0, x0,#8
    ADD         x5,sp,#0x4B                 //*au1_src_left_tmp

    SUB         x20,x0,x1                   //pu1_src - src_strd
    csel        x8, x20, x8,EQ
    movi        v18.16b, #0
    csel        x8, x3, x8,NE

    ADD         x8,x8,#2                    //pu1_src - src_strd + 2
    LD1         {v3.16b},[x8]               //pu1_top_row = vld1q_u8(pu1_src - src_strd + 2)
    //LD1 {v11.8b},[x8]                        //pu1_top_row = vld1q_u8(pu1_src - src_strd + 2)
    //SUB x8, x8,#8
    ADD         x3,x3,#16

    mov         w4, w25                     //Loads ht
    cmhi        v17.16b,  v5.16b ,  v3.16b  //vcgtq_u8(pu1_cur_row, pu1_top_row)
    mov         w7, w24                     //Loads wd

    SUB         x7,x7,x6                    //(wd - col)
    cmhi        v16.16b,  v3.16b ,  v5.16b  //vcltq_u8(pu1_cur_row, pu1_top_row)
    ADD         x7,x7,#14                   //15 + (wd - col)

    mov         x8, x26                     //Loads *pu1_src
    SUB         v17.16b,  v16.16b ,  v17.16b //sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    ADD         x7,x8,x7                    //pu1_src[0 * src_strd + 15 + (wd - col)]

AU1_SRC_LEFT_LOOP:
    LDRH        w8,[x7]                     //load the value and increment by src_strd
    SUBS        x4,x4,#1                    //decrement the loop count

    STRH        w8,[x5],#2                  //store it in the stack pointer
    ADD         x7,x7,x1
    BNE         AU1_SRC_LEFT_LOOP


    MOV         x7,x12                      //row count, move ht_tmp to x7
    movi        v18.16b, #0                 //I
    ADD         x11,x0,x1                   //I *pu1_src + src_strd

    SUB         x5,x12,x7                   //I ht_tmp - row
    LD1         {v16.16b},[x11]             //I pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //LD1 {v17.8b},[x11]                    //I pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x11, x11,#8
    ADD         x8,x14,x5,LSL #1            //I pu1_src_left_cpy[(ht_tmp - row) * 2]

    LDRH        w5,[x8,#2]                  //I
    mov         v18.h[7], w5                //I vsetq_lane_u8
    mov         x11, x21                    //I Loads pu1_avail

    LDRB        w11,[x11,#2]                //I pu1_avail[2]
    EXT         v18.16b,  v18.16b ,  v16.16b,#14 //I pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 14)
    CMP         x11,#0                      //I
    BNE         SIGN_UP_CHANGE_DONE         //I

    LDRB        w8,[x0,#14]                 //I pu1_src_cpy[14]
    SUB         x5,x0,x1                    //I

    LDRB        w11,[x5,#16]                //I load the value pu1_src_cpy[16 - src_strd]

    LDRB        w9,[x0,#15]                 //I pu1_src_cpy[15]
    SUB         x8,x8,x11                   //I pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd]

    LDRB        w10,[x5,#17]                //I load the value pu1_src_cpy[17 - src_strd]
    CMP         x8,#0                       //I

    movn        x20,#0
    csel        x8, x20, x8,LT              //I
    SUB         x9,x9,x10                   //I pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]

    MOV         x20,#1
    csel        x8, x20, x8,GT              //I SIGN(pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd])
    CMP         x9,#0                       //I

    movn        x20,#0
    csel        x9, x20, x9,LT              //I
    mov         v17.b[14], w8               //I sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[14] -pu1_src_cpy[16 - src_strd]), sign_up, 0)
    MOV         x20,#1
    csel        x9, x20, x9,GT              //I SIGN(pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]

    mov         v17.b[15], w9               //I sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] -pu1_src_cpy[17 - src_strd]), sign_up, 1)

SIGN_UP_CHANGE_DONE:
    LD1         {v28.8b},[x2]               //edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    cmhi        v20.16b,  v5.16b ,  v18.16b //I vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)

    cmhi        v22.16b,  v18.16b ,  v5.16b //I vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    SUB         v22.16b,  v22.16b ,  v20.16b //I sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    ADD         v18.16b,  v0.16b ,  v17.16b //I edge_idx = vaddq_s8(const_2, sign_up)
    ADD         v18.16b,  v18.16b ,  v22.16b //I edge_idx = vaddq_s8(edge_idx, sign_down)
    TBL         v18.16b, {v28.16b},v18.16b  //I vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    NEG         v17.16b, v22.16b            //I sign_up = vnegq_s8(sign_down)

    //TBL v19.8b, {v28.16b},v19.8b                //I vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    EXT         v17.16b,  v17.16b ,  v17.16b,#2 //I sign_up = vextq_s8(sign_up, sign_up, 2)

    Uxtl        v20.8h, v5.8b               //I pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    AND         v18.16b,  v18.16b ,  v1.16b //I edge_idx = vandq_s8(edge_idx, au1_mask)
    mov         v19.d[0],v18.d[1]

    UZP1        v31.8b, v18.8b, v19.8b
    UZP2        v19.8b, v18.8b, v19.8b      //I
    mov         v18.8b,v31.8b
    TBL         v22.8b, {v6.16b},v18.8b     //I
    TBL         v23.8b, {v7.16b},v19.8b     //I
    ZIP1        v31.8b, v22.8b, v23.8b
    ZIP2        v23.8b, v22.8b, v23.8b      //I
    mov         v22.8b,v31.8b

    Uxtl2       v18.8h, v5.16b              //I pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    SADDW       v20.8h,  v20.8h ,  v22.8b   //I pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    SMAX        v20.8h,  v20.8h ,  v2.8h    //I pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    UMIN        v20.8h,  v20.8h ,  v4.8h    //I pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    mov         v5.16b, v16.16b             //I pu1_cur_row = pu1_next_row
    SADDW       v18.8h,  v18.8h ,  v23.8b   //I pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)

    SUB         x7,x7,#1                    //I Decrement the ht_tmp loop count by 1
    SMAX        v18.8h,  v18.8h ,  v2.8h    //I pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)

    UMIN        v18.8h,  v18.8h ,  v4.8h    //I pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))


PU1_SRC_LOOP:
    ADD         x11,x0,x1,LSL #1            //II *pu1_src + src_strd
    xtn         v20.8b,  v20.8h             //I vmovn_s16(pi2_tmp_cur_row.val[0])
    SUB         x5,x12,x7                   //II ht_tmp - row

    ADD         x4,x0,x1                    //III *pu1_src + src_strd
    xtn2        v20.16b,  v18.8h            //I vmovn_s16(pi2_tmp_cur_row.val[1])
    ADD         x8,x14,x5,LSL #1            //II pu1_src_left_cpy[(ht_tmp - row) * 2]

    LDRH        w9,[x8,#2]
    LD1         {v16.16b},[x11]             //II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //LD1 {v17.8b},[x11]                    //II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x11, x11,#8
    LDRB        w10,[x4,#14]                //II pu1_src_cpy[14]

    LDRB        w8,[x4,#15]                 //II pu1_src_cpy[15]
    mov         v28.h[7], w9                //II vsetq_lane_u8
    ADD         x4,x11,x1                   //III *pu1_src + src_strd

    LDRB        w5,[x0,#17]                 //II load the value pu1_src_cpy[17 - src_strd]
    LD1         {v30.16b},[x4]              //III pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //LD1 {v31.8b},[x4]                    //III pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x4, x4,#8
    LDRB        w11,[x0,#16]                //II load the value pu1_src_cpy[16 - src_strd]

    SUB         x7,x7,#1                    //II Decrement the ht_tmp loop count by 1
    ST1         { v20.16b},[x0],x1          //I vst1q_u8(pu1_src_cpy, pu1_cur_row)
    SUB         x10,x10,x11                 //II pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd]

    CMP         x10,#0                      //II
    EXT         v28.16b,  v28.16b ,  v16.16b,#14 //II pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 14)
    SUB         x8,x8,x5                    //II pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]

    movn        x20,#0
    csel        x10, x20, x10,LT            //II
    LD1         {v21.8b},[x2]               //edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    MOV         x20,#1
    csel        x10, x20, x10,GT            //II SIGN(pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd])

    CMP         x8,#0                       //II
    mov         v17.b[14], w10              //II sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[14] -pu1_src_cpy[16 - src_strd]), sign_up, 0)
    movn        x20,#0
    csel        x8, x20, x8,LT              //II

    MOV         x20,#1
    csel        x8, x20, x8,GT              //II SIGN(pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]
    SUB         x10,x12,x7                  //III ht_tmp - row
    mov         v17.b[15], w8               //II sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] -pu1_src_cpy[17 - src_strd]), sign_up, 1)
    ADD         x11,x14,x10,LSL #1          //III pu1_src_left_cpy[(ht_tmp - row) * 2]

    CMP         x7,#1                       //III
    cmhi        v22.16b,  v5.16b ,  v28.16b //II vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)
    BNE         NEXT_ROW_POINTER_ASSIGNED_2 //III

    mov         x5, x21                     //III Loads pu1_avail
    LDRB        w5,[x5,#3]                  //III pu1_avail[3]
    CMP         x5,#0                       //III
    SUB         x20,x4,#4                   //III pu1_src[src_strd - 2]
    csel        x11, x20, x11,NE

NEXT_ROW_POINTER_ASSIGNED_2:
    LDRH        w5,[x11,#2]                 //III
    cmhi        v24.16b,  v28.16b ,  v5.16b //II vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    ADD         x11,x0,x1                   //III

    LDRB        w9,[x11,#14]                //III pu1_src_cpy[14]
    mov         v18.h[7], w5                //III vsetq_lane_u8
    LDRB        w8,[x11,#15]                //III pu1_src_cpy[15]

    LDRB        w11,[x0,#16]                //III load the value pu1_src_cpy[16 - src_strd]
    SUB         v24.16b,  v24.16b ,  v22.16b //II sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    LDRB        w10,[x0,#17]                //III load the value pu1_src_cpy[17 - src_strd]

    SUB         x9,x9,x11                   //III pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd]
    EXT         v18.16b,  v18.16b ,  v30.16b,#14 //III pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 14)
    SUB         x10,x8,x10                  //III pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]

    CMP         x9,#0                       //III
    ADD         v26.16b,  v0.16b ,  v17.16b //II edge_idx = vaddq_s8(const_2, sign_up)
    movn        x20,#0
    csel        x9, x20, x9,LT              //III

    MOV         x20,#1
    csel        x9, x20, x9,GT              //III SIGN(pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd])
    ADD         v26.16b,  v26.16b ,  v24.16b //II edge_idx = vaddq_s8(edge_idx, sign_down)
    CMP         x10,#0                      //III

    NEG         v17.16b, v24.16b            //II sign_up = vnegq_s8(sign_down)
    TBL         v26.16b, {v21.16b},v26.16b  //II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    movn        x20,#0
    csel        x10, x20, x10,LT            //III
    MOV         x20,#1
    csel        x10, x20, x10,GT            //III SIGN(pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]

    EXT         v17.16b,  v17.16b ,  v17.16b,#2 //II sign_up = vextq_s8(sign_up, sign_up, 2)
    //TBL v27.8b, {v21.16b},v27.8b                //II vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    cmhi        v22.16b,  v16.16b ,  v18.16b //III vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)

    mov         v17.b[14], w9               //III sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[14] -pu1_src_cpy[16 - src_strd]), sign_up, 0)
    AND         v26.16b,  v26.16b ,  v1.16b //II edge_idx = vandq_s8(edge_idx, au1_mask)
    mov         v27.d[0],v26.d[1]

    mov         v17.b[15], w10              //III sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] -pu1_src_cpy[17 - src_strd]), sign_up, 1)
    UZP1        v31.8b, v26.8b, v27.8b
    UZP2        v27.8b, v26.8b, v27.8b      //II
    mov         v26.8b,v31.8b

    cmhi        v20.16b,  v18.16b ,  v16.16b //III vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    TBL         v24.8b, {v6.16b},v26.8b     //II
    SUB         v22.16b,  v20.16b ,  v22.16b //III sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    ADD         v18.16b,  v0.16b ,  v17.16b //III edge_idx = vaddq_s8(const_2, sign_up)
    TBL         v25.8b, {v7.16b},v27.8b     //II
    ADD         v18.16b,  v18.16b ,  v22.16b //III edge_idx = vaddq_s8(edge_idx, sign_down)

    LD1         {v20.8b},[x2]               //edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    ZIP1        v31.8b, v24.8b, v25.8b
    ZIP2        v25.8b, v24.8b, v25.8b      //II
    mov         v24.8b,v31.8b

    Uxtl        v28.8h, v5.8b               //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    TBL         v18.16b, {v20.16b},v18.16b  //III vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    NEG         v17.16b, v22.16b            //III sign_up = vnegq_s8(sign_down)

    SADDW       v28.8h,  v28.8h ,  v24.8b   //II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    //TBL v19.8b, {v20.16b},v19.8b                //III vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    EXT         v17.16b,  v17.16b ,  v17.16b,#2 //III sign_up = vextq_s8(sign_up, sign_up, 2)

    Uxtl2       v26.8h, v5.16b              //II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    AND         v18.16b,  v18.16b ,  v1.16b //III edge_idx = vandq_s8(edge_idx, au1_mask)
    mov         v19.d[0],v18.d[1]

    Uxtl        v20.8h, v16.8b              //III pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    UZP1        v31.8b, v18.8b, v19.8b
    UZP2        v19.8b, v18.8b, v19.8b      //III
    mov         v18.8b,v31.8b

    SMAX        v28.8h,  v28.8h ,  v2.8h    //II pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    TBL         v22.8b, {v6.16b},v18.8b     //III
    UMIN        v28.8h,  v28.8h ,  v4.8h    //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    SADDW       v26.8h,  v26.8h ,  v25.8b   //II pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    TBL         v23.8b, {v7.16b},v19.8b     //III
    SMAX        v26.8h,  v26.8h ,  v2.8h    //II pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)

    Uxtl2       v18.8h, v16.16b             //III pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    ZIP1        v31.8b, v22.8b, v23.8b
    ZIP2        v23.8b, v22.8b, v23.8b      //III
    mov         v22.8b,v31.8b

    xtn         v28.8b,  v28.8h             //II vmovn_s16(pi2_tmp_cur_row.val[0])
    SADDW       v20.8h,  v20.8h ,  v22.8b   //III pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    mov         v5.16b, v30.16b             //III pu1_cur_row = pu1_next_row
    UMIN        v26.8h,  v26.8h ,  v4.8h    //II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    SUB         x7,x7,#1                    //III Decrement the ht_tmp loop count by 1
    SMAX        v20.8h,  v20.8h ,  v2.8h    //III pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    CMP         x7,#1                       //III

    xtn2        v28.16b,  v26.8h            //II vmovn_s16(pi2_tmp_cur_row.val[1])
    UMIN        v20.8h,  v20.8h ,  v4.8h    //III pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    SADDW       v18.8h,  v18.8h ,  v23.8b   //III pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)

    SMAX        v18.8h,  v18.8h ,  v2.8h    //III pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)

    ST1         { v28.16b},[x0],x1          //II vst1q_u8(pu1_src_cpy, pu1_cur_row)
    UMIN        v18.8h,  v18.8h ,  v4.8h    //III pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    BGT         PU1_SRC_LOOP                //If not equal jump to PU1_SRC_LOOP
    BLT         INNER_LOOP_DONE


    ADD         x11,x0,x1,LSL #1            //*pu1_src + src_strd
    xtn         v20.8b,  v20.8h             //III vmovn_s16(pi2_tmp_cur_row.val[0])
    SUB         x5,x12,x7                   //ht_tmp - row

    ADD         x8,x14,x5,LSL #1            //pu1_src_left_cpy[(ht_tmp - row) * 2]
    xtn2        v20.16b,  v18.8h            //III vmovn_s16(pi2_tmp_cur_row.val[1])
    CMP         x7,#1

    LDRB        w4,[x0,#16]                 //load the value pu1_src_cpy[16 - src_strd]
    LD1         {v16.16b},[x11]             //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //LD1 {v17.8b},[x11]                    //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x11, x11,#8
    LDRB        w9,[x0,#17]                 //load the value pu1_src_cpy[17 - src_strd]

    BNE         NEXT_ROW_POINTER_ASSIGNED_3
    mov         x5, x21                     //Loads pu1_avail
    LDRB        w5,[x5,#3]                  //pu1_avail[3]
    CMP         x5,#0
    SUB         x20,x11,#4                  //pu1_src[src_strd - 2]
    csel        x8, x20, x8,NE

NEXT_ROW_POINTER_ASSIGNED_3:
    LDRH        w5,[x8,#2]
    ST1         { v20.16b},[x0],x1          //III vst1q_u8(pu1_src_cpy, pu1_cur_row)
    LDRB        w8,[x0,#14]                 //pu1_src_cpy[14]

    SUB         x8,x8,x4                    //pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd]
    mov         v18.h[7], w5                //vsetq_lane_u8
    LDRB        w10,[x0,#15]                //pu1_src_cpy[15]

    CMP         x8,#0
    EXT         v18.16b,  v18.16b ,  v16.16b,#14 //pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 14)
    SUB         x10,x10,x9                  //pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]

    movn        x20,#0
    csel        x8, x20, x8,LT
    LD1         {v28.8b},[x2]               //edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    MOV         x20,#1
    csel        x8, x20, x8,GT              //SIGN(pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd])

    CMP         x10,#0
    mov         v17.b[14], w8               //sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[14] -pu1_src_cpy[16 - src_strd]), sign_up, 0)
    movn        x20,#0
    csel        x10, x20, x10,LT

    MOV         x20,#1
    csel        x10, x20, x10,GT            //SIGN(pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]
    mov         v17.b[15], w10              //sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] -pu1_src_cpy[17 - src_strd]), sign_up, 1)
    cmhi        v20.16b,  v5.16b ,  v18.16b //vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)

    cmhi        v22.16b,  v18.16b ,  v5.16b //vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    SUB         v22.16b,  v22.16b ,  v20.16b //sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    ADD         v18.16b,  v0.16b ,  v17.16b //edge_idx = vaddq_s8(const_2, sign_up)
    ADD         v18.16b,  v18.16b ,  v22.16b //edge_idx = vaddq_s8(edge_idx, sign_down)
    TBL         v18.16b, {v28.16b},v18.16b  //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    //TBL v19.8b, {v28.16b},v19.8b                //vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))

    AND         v18.16b,  v18.16b ,  v1.16b //edge_idx = vandq_s8(edge_idx, au1_mask)
    mov         v19.d[0],v18.d[1]

    Uxtl        v20.8h, v5.8b               //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    UZP1        v31.8b, v18.8b, v19.8b
    UZP2        v19.8b, v18.8b, v19.8b
    mov         v18.8b,v31.8b

    TBL         v22.8b, {v6.16b},v18.8b
    TBL         v23.8b, {v7.16b},v19.8b

    Uxtl2       v18.8h, v5.16b              //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    ZIP1        v31.8b, v22.8b, v23.8b
    ZIP2        v23.8b, v22.8b, v23.8b
    mov         v22.8b,v31.8b

    SADDW       v20.8h,  v20.8h ,  v22.8b   //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    SMAX        v20.8h,  v20.8h ,  v2.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    UMIN        v20.8h,  v20.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    SADDW       v18.8h,  v18.8h ,  v23.8b   //pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    SMAX        v18.8h,  v18.8h ,  v2.8h    //pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    UMIN        v18.8h,  v18.8h ,  v4.8h    //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))


INNER_LOOP_DONE:

    mov         w8, w25                     //Loads ht
    xtn         v20.8b,  v20.8h             //III vmovn_s16(pi2_tmp_cur_row.val[0])
    ADD         x5,sp,#0x4B                 //*au1_src_left_tmp

    LSL         x8,x8,#1
    xtn2        v20.16b,  v18.8h            //III vmovn_s16(pi2_tmp_cur_row.val[1])
    mov         x11, x27                    //Loads *pu1_src_left

SRC_LEFT_LOOP:
    LDR         w7, [x5],#4                 //au1_src_left_tmp[row]
    SUBS        x8,x8,#4
    STR         w7, [x11],#4                //pu1_src_left[row] = au1_src_left_tmp[row]
    BNE         SRC_LEFT_LOOP

    SUBS        x6,x6,#16                   //Decrement the wd loop count by 16
    ST1         { v20.16b},[x0],x1          //III vst1q_u8(pu1_src_cpy, pu1_cur_row)
    CMP         x6,#8                       //Check whether residue remains

    BLT         RE_ASSINING_LOOP            //Jump to re-assigning loop
    mov         w7, w24                     //Loads wd
    mov         x0, x28                     //Loads *pu1_src
    SUB         x7,x7,x6
    ADD         x0,x0,x7
    BGT         WIDTH_LOOP_16               //If not equal jump to width_loop
    BEQ         WIDTH_RESIDUE               //If residue remains jump to residue loop

WD_16_HT_4_LOOP:
    mov         w7, w24                     //Loads wd

    mov         x5, x21                     //Loads pu1_avail
    CMP         x6,x7                       //col == wd

    LDRb        w20, [x5]                   //pu1_avail[0]
    csel        w8,w20,w8,EQ
    MOV         x20,#-1
    csel        x8, x20, x8,NE
    mov         v1.b[0], w8                 //au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

    CMP         x6,#16                      //if(col == 16)
    mov         v1.b[1], w8                 //au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

    BNE         SKIP_AU1_MASK_VAL_WD_16_HT_4
    LDRB        w8,[x5,#1]                  //pu1_avail[1]
    mov         v1.b[14], w8                //au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)
    mov         v1.b[15], w8                //au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)

SKIP_AU1_MASK_VAL_WD_16_HT_4:
    LDRB        w11,[x5,#2]                 //pu1_avail[2]
    SUB         x20,x0,x1                   //pu1_src - src_strd
    CMP         x11,#0
    csel        x8, x20, x8,EQ

    csel        x8, x3, x8,NE
    LD1         {v5.16b},[x0]               //pu1_cur_row = vld1q_u8(pu1_src)
    //LD1 {v13.8b},[x0]                        //pu1_cur_row = vld1q_u8(pu1_src)
    //SUB x0, x0,#8
    ADD         x8,x8,#2                    //pu1_src - src_strd + 2

    ADD         x3,x3,#16
    LD1         {v3.16b},[x8]               //pu1_top_row = vld1q_u8(pu1_src - src_strd + 2)
    //LD1 {v11.8b},[x8]                        //pu1_top_row = vld1q_u8(pu1_src - src_strd + 2)
    //SUB x8, x8,#8
    ADD         x5,sp,#0x4B                 //*au1_src_left_tmp

    mov         w4, w25                     //Loads ht
    cmhi        v17.16b,  v5.16b ,  v3.16b  //vcgtq_u8(pu1_cur_row, pu1_top_row)
    mov         w7, w24                     //Loads wd

    SUB         x7,x7,x6                    //(wd - col)
    cmhi        v16.16b,  v3.16b ,  v5.16b  //vcltq_u8(pu1_cur_row, pu1_top_row)
    ADD         x7,x7,#14                   //15 + (wd - col)

    mov         x8, x26                     //Loads *pu1_src
    SUB         v17.16b,  v16.16b ,  v17.16b //sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    ADD         x7,x8,x7                    //pu1_src[0 * src_strd + 15 + (wd - col)]

AU1_SRC_LEFT_LOOP_WD_16_HT_4:
    LDRH        w8,[x7]                     //load the value and increment by src_strd
    SUBS        x4,x4,#1                    //decrement the loop count

    STRH        w8,[x5],#2                  //store it in the stack pointer
    ADD         x7,x7,x1
    BNE         AU1_SRC_LEFT_LOOP_WD_16_HT_4

    movi        v18.16b, #0
    MOV         x7,x12                      //row count, move ht_tmp to x7

PU1_SRC_LOOP_WD_16_HT_4:
    ADD         x9,x0,x1                    //*pu1_src + src_strd

    mov         x5, x21                     //Loads pu1_avail
    LD1         {v16.16b},[x9]              //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //LD1 {v17.8b},[x9]                        //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x9, x9,#8
    LDRB        w5,[x5,#3]                  //pu1_avail[3]

    SUB         x11,x12,x7                  //ht_tmp - row
    ADD         x8,x14,x11,LSL #1           //pu1_src_left_cpy[(ht_tmp - row) * 2]
    ADD         x8,x8,#2                    //pu1_src_left_cpy[(ht_tmp - row + 1) * 2]

    CMP         x5,#0
    BEQ         NEXT_ROW_POINTER_ASSIGNED_WD_16_HT_4
    CMP         x7,#1
    SUB         x20,x9,#2                   //pu1_src[src_strd - 2]
    csel        x8, x20, x8,EQ

NEXT_ROW_POINTER_ASSIGNED_WD_16_HT_4:
    LDRH        w5,[x8]
    mov         v18.h[7], w5                //vsetq_lane_u8
    EXT         v18.16b,  v18.16b ,  v16.16b,#14 //pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 14)

    CMP         x7,x12
    BLT         SIGN_UP_CHANGE_WD_16_HT_4
    mov         x5, x21                     //Loads pu1_avail
    LDRB        w5,[x5,#2]                  //pu1_avail[2]
    CMP         x5,#0
    BNE         SIGN_UP_CHANGE_DONE_WD_16_HT_4

SIGN_UP_CHANGE_WD_16_HT_4:
    LDRB        w8,[x0,#14]                 //pu1_src_cpy[14]
    SUB         x9,x0,x1

    LDRB        w5,[x9,#16]                 //load the value pu1_src_cpy[16 - src_strd]

    LDRB        w10,[x0,#15]                //pu1_src_cpy[15]
    SUB         x8,x8,x5                    //pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd]

    LDRB        w11,[x9,#17]                //load the value pu1_src_cpy[17 - src_strd]
    CMP         x8,#0

    movn        x20,#0
    csel        x8, x20, x8,LT
    SUB         x10,x10,x11                 //pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]

    MOV         x20,#1
    csel        x8, x20, x8,GT              //SIGN(pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd])

    CMP         x10,#0
    mov         v17.b[14], w8               //sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[14] -pu1_src_cpy[16 - src_strd]), sign_up, 0)
    movn        x20,#0
    csel        x10, x20, x10,LT

    MOV         x20,#1
    csel        x10, x20, x10,GT            //SIGN(pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]
    mov         v17.b[15], w10              //sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] -pu1_src_cpy[17 - src_strd]), sign_up, 1)

SIGN_UP_CHANGE_DONE_WD_16_HT_4:
    LD1         {v20.8b},[x2]               //edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    cmhi        v22.16b,  v5.16b ,  v18.16b //vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)

    cmhi        v24.16b,  v18.16b ,  v5.16b //vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    SUB         v24.16b,  v24.16b ,  v22.16b //sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    ADD         v26.16b,  v0.16b ,  v17.16b //edge_idx = vaddq_s8(const_2, sign_up)
    ADD         v26.16b,  v26.16b ,  v24.16b //edge_idx = vaddq_s8(edge_idx, sign_down)

    mov         v20.d[1],v20.d[0]
    NEG         v17.16b, v24.16b            //sign_up = vnegq_s8(sign_down)
    TBL         v26.16b, {v20.16b},v26.16b  //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))

    //TBL v27.8b, {v20.16b},v27.8b                //vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    EXT         v17.16b,  v17.16b ,  v17.16b,#2 //sign_up = vextq_s8(sign_up, sign_up, 2)

    Uxtl        v28.8h, v5.8b               //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    AND         v26.16b,  v26.16b ,  v1.16b //edge_idx = vandq_s8(edge_idx, au1_mask)
    mov         v27.d[0],v26.d[1]

    UZP1        v31.8b, v26.8b, v27.8b
    UZP2        v27.8b, v26.8b, v27.8b
    mov         v26.8b,v31.8b
    TBL         v24.8b, {v6.16b},v26.8b
    TBL         v25.8b, {v7.16b},v27.8b
    ZIP1        v31.8b, v24.8b, v25.8b
    ZIP2        v25.8b, v24.8b, v25.8b
    mov         v24.8b,v31.8b

    Uxtl2       v30.8h, v5.16b              //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    SADDW       v28.8h,  v28.8h ,  v24.8b   //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    SMAX        v28.8h,  v28.8h ,  v2.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    UMIN        v28.8h,  v28.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    mov         v5.16b, v16.16b             //pu1_cur_row = pu1_next_row
    SADDW       v30.8h,  v30.8h ,  v25.8b   //pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)

    SMAX        v30.8h,  v30.8h ,  v2.8h    //pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    UMIN        v30.8h,  v30.8h ,  v4.8h    //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    xtn         v28.8b,  v28.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])
    xtn2        v28.16b,  v30.8h            //vmovn_s16(pi2_tmp_cur_row.val[1])

    SUBS        x7,x7,#1                    //Decrement the ht_tmp loop count by 1
    ST1         { v28.16b},[x0],x1          //vst1q_u8(pu1_src_cpy, pu1_cur_row)
    BNE         PU1_SRC_LOOP_WD_16_HT_4     //If not equal jump to PU1_SRC_LOOP_WD_16_HT_4

    mov         w8, w25                     //Loads ht
    ADD         x5,sp,#0x4B                 //*au1_src_left_tmp
    mov         x11, x27                    //Loads *pu1_src_left

SRC_LEFT_LOOP_WD_16_HT_4:
    LDR         w7, [x5],#4                 //au1_src_left_tmp[row]
    SUBS        x8,x8,#2
    STR         w7, [x11],#4                //pu1_src_left[row] = au1_src_left_tmp[row]
    BNE         SRC_LEFT_LOOP_WD_16_HT_4

    SUBS        x6,x6,#16                   //Decrement the wd loop count by 16
    BLE         RE_ASSINING_LOOP            //Jump to re-assigning loop
    mov         w7, w24                     //Loads wd
    mov         x0, x28                     //Loads *pu1_src
    SUB         x7,x7,x6
    ADD         x0,x0,x7
    BGT         WD_16_HT_4_LOOP             //If not equal jump to width_loop

WIDTH_RESIDUE:
    mov         w7, w24                     //Loads wd

    mov         x5, x21                     //Loads pu1_avail
    CMP         x6,x7                       //wd_residue == wd

    LDRb        w20, [x5]                   //pu1_avail[0]
    csel        w8,w20,w8,EQ

    MOV         x20,#-1
    csel        x8, x20, x8,NE
    LDRB        w11,[x5,#1]                 //pu1_avail[1]

    LDRB        w9,[x5,#2]                  //pu1_avail[2]
    mov         v1.b[0], w8                 //au1_mask = vsetq_lane_s8(-1, au1_mask, 0)
    CMP         x9,#0

    SUB         x20,x0,x1                   //pu1_src - src_strd
    csel        x10, x20, x10,EQ
    mov         v1.b[1], w8                 //au1_mask = vsetq_lane_s8(-1, au1_mask, 0)
    csel        x10, x3, x10,NE

    ADD         x10,x10,#2                  //pu1_src - src_strd + 2
    mov         v1.b[6], w11                //au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)
    ADD         x5,sp,#0x4B                 //*au1_src_left_tmp

    mov         w4, w25                     //Loads ht
    mov         v1.b[7], w11                //au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)
    mov         w7, w24                     //Loads wd

    mov         x8, x26                     //Loads *pu1_src
    LD1         {v3.16b},[x10]              //pu1_top_row = vld1q_u8(pu1_src - src_strd + 2)
    //LD1 {v11.8b},[x10]                    //pu1_top_row = vld1q_u8(pu1_src - src_strd + 2)
    //SUB x10, x10,#8
    SUB         x7,x7,#2                    //(wd - 2)

    ADD         x7,x8,x7                    //pu1_src[0 * src_strd + (wd - 2)]

AU1_SRC_LEFT_LOOP_RESIDUE:
    LDRH        w8,[x7]                     //load the value and increment by src_strd
    ADD         x7,x7,x1
    STRH        w8,[x5],#2                  //store it in the stack pointer
    SUBS        x4,x4,#1                    //decrement the loop count
    BNE         AU1_SRC_LEFT_LOOP_RESIDUE

    LD1         {v5.16b},[x0]               //pu1_cur_row = vld1q_u8(pu1_src)
    //LD1 {v13.8b},[x0]                        //pu1_cur_row = vld1q_u8(pu1_src)
    //SUB x0, x0,#8

    movi        v18.16b, #0
    cmhi        v17.16b,  v5.16b ,  v3.16b  //vcgtq_u8(pu1_cur_row, pu1_top_row)

    cmhi        v16.16b,  v3.16b ,  v5.16b  //vcltq_u8(pu1_cur_row, pu1_top_row)
    SUB         v17.16b,  v16.16b ,  v17.16b //sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    MOV         x7,x12                      //row count, move ht_tmp to x7

PU1_SRC_LOOP_RESIDUE:
    ADD         x9,x0,x1                    //*pu1_src + src_strd

    SUB         x11,x12,x7                  //ht_tmp - row
    LD1         {v16.16b},[x9]              //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //LD1 {v17.8b},[x9]                        //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x9, x9,#8
    mov         x5, x21                     //Loads pu1_avail

    LDRB        w5,[x5,#3]                  //pu1_avail[3]
    ADD         x8,x14,x11,LSL #1           //pu1_src_left_cpy[(ht_tmp - row) * 2]

    CMP         x5,#0
    ADD         x8,x8,#2                    //pu1_src_left_cpy[(ht_tmp - row + 1) * 2]

    BEQ         NEXT_ROW_POINTER_ASSIGNED_RESIDUE
    CMP         x7,#1
    SUB         x20,x9,#2                   //pu1_src[src_strd - 2]
    csel        x8, x20, x8,EQ

NEXT_ROW_POINTER_ASSIGNED_RESIDUE:
    LDRB        w5,[x8]

    LDRB        w8,[x8,#1]
    mov         v18.b[14], w5               //vsetq_lane_u8
    CMP         x7,x12

    mov         v18.b[15], w8               //vsetq_lane_u8
    EXT         v18.16b,  v18.16b ,  v16.16b,#14 //pu1_next_row_tmp = vextq_u8(pu1_next_row_tmp, pu1_next_row, 14)

    BLT         SIGN_UP_CHANGE_RESIDUE
    mov         x5, x21                     //Loads pu1_avail
    LDRB        w5,[x5,#2]                  //pu1_avail[2]
    CMP         x5,#0
    BNE         SIGN_UP_CHANGE_DONE_RESIDUE

SIGN_UP_CHANGE_RESIDUE:
    LDRB        w8,[x0,#14]                 //pu1_src_cpy[14]
    SUB         x9,x0,x1

    LDRB        w5,[x9,#16]                 //load the value pu1_src_cpy[16 - src_strd]

    LDRB        w10,[x0,#15]                //pu1_src_cpy[15]
    SUB         x8,x8,x5                    //pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd]

    LDRB        w11,[x9,#17]                //load the value pu1_src_cpy[17 - src_strd]
    CMP         x8,#0

    movn        x20,#0
    csel        x8, x20, x8,LT
    SUB         x10,x10,x11                 //pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]

    MOV         x20,#1
    csel        x8, x20, x8,GT              //SIGN(pu1_src_cpy[14] - pu1_src_cpy[16 - src_strd])

    CMP         x10,#0
    mov         v17.b[14], w8               //sign_up = sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[14] -pu1_src_cpy[16 - src_strd]), sign_up, 0)
    movn        x20,#0
    csel        x10, x20, x10,LT

    MOV         x20,#1
    csel        x10, x20, x10,GT            //SIGN(pu1_src_cpy[15] - pu1_src_cpy[17 - src_strd]
    mov         v17.b[15], w10              //sign_up = vsetq_lane_s8(SIGN(pu1_src_cpy[15] -pu1_src_cpy[17 - src_strd]), sign_up, 1)

SIGN_UP_CHANGE_DONE_RESIDUE:
    LD1         {v20.8b},[x2]               //edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    cmhi        v22.16b,  v5.16b ,  v18.16b //vcgtq_u8(pu1_cur_row, pu1_next_row_tmp)

    cmhi        v24.16b,  v18.16b ,  v5.16b //vcltq_u8(pu1_cur_row, pu1_next_row_tmp)
    SUB         v24.16b,  v24.16b ,  v22.16b //sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    ADD         v26.16b,  v0.16b ,  v17.16b //edge_idx = vaddq_s8(const_2, sign_up)
    ADD         v26.16b,  v26.16b ,  v24.16b //edge_idx = vaddq_s8(edge_idx, sign_down)

    mov         v20.d[1],v20.d[0]
    NEG         v17.16b, v24.16b            //sign_up = vnegq_s8(sign_down)
    TBL         v26.16b, {v20.16b},v26.16b  //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))

    //TBL v27.8b, {v20.16b},v27.8b                //vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    EXT         v17.16b,  v17.16b ,  v17.16b,#2 //sign_up = vextq_s8(sign_up, sign_up, 14)

    Uxtl        v28.8h, v5.8b               //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    AND         v26.16b,  v26.16b ,  v1.16b //edge_idx = vandq_s8(edge_idx, au1_mask)
    mov         v27.d[0],v26.d[1]

    UZP1        v31.8b, v26.8b, v27.8b
    UZP2        v27.8b, v26.8b, v27.8b
    mov         v26.8b,v31.8b
    TBL         v24.8b, {v6.16b},v26.8b
    TBL         v25.8b, {v7.16b},v27.8b
    ZIP1        v31.8b, v24.8b, v25.8b
    ZIP2        v25.8b, v24.8b, v25.8b
    mov         v24.8b,v31.8b

    mov         v5.16b, v16.16b             //pu1_cur_row = pu1_next_row
    SADDW       v28.8h,  v28.8h ,  v24.8b   //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    SMAX        v28.8h,  v28.8h ,  v2.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    UMIN        v28.8h,  v28.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    SUBS        x7,x7,#1                    //Decrement the ht_tmp loop count by 1
    xtn         v30.8b,  v28.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])

    ST1         {v30.8b},[x0],x1            //vst1q_u8(pu1_src_cpy, pu1_cur_row)

    BNE         PU1_SRC_LOOP_RESIDUE        //If not equal jump to PU1_SRC_LOOP

    mov         w8, w25                     //Loads ht
    ADD         x5,sp,#0x4B                 //*au1_src_left_tmp

    mov         x11, x27                    //Loads *pu1_src_left

SRC_LEFT_LOOP_RESIDUE:
    LDR         w7, [x5],#4                 //au1_src_left_tmp[row]
    SUBS        x8,x8,#2
    STR         w7, [x11],#4                //pu1_src_left[row] = au1_src_left_tmp[row]
    BNE         SRC_LEFT_LOOP_RESIDUE


RE_ASSINING_LOOP:
    mov         w7, w24                     //Loads wd
    mov         w8, w25                     //Loads ht

    mov         x0, x26                     //Loads *pu1_src
    SUB         x10,x7,#2                   //wd - 2

    LDRH        w9,[sp,#6]
    SUB         x8,x8,#1                    //ht - 1

    STRH        w9,[x0,x10]                 //pu1_src_org[0] = u1_pos_0_0_tmp
    madd        x6, x8, x1, x0              //pu1_src[(ht - 1) * src_strd]

    mov         x4, x15                     //Loads pu1_src_top_left

    LDRH        w9,[sp,#8]
    ADD         x12,sp,#10

    STRH        w9,[x6]                     //pu1_src_org[(ht - 1) * src_strd] = u1_pos_wd_ht_tmp_u

    LDRH        w10,[sp]                    //load u1_src_top_left_tmp from stack pointer
    STRH        w10,[x4]                    //*pu1_src_top_left = u1_src_top_left_tmp
    mov         x3, x22                     //Loads pu1_src_top

SRC_TOP_LOOP:
    LD1         {v0.8b},[x12],#8            //pu1_src_top[col] = au1_src_top_tmp[col]
    SUBS        x7,x7,#8                    //Decrement the width
    ST1         {v0.8b},[x3],#8             //pu1_src_top[col] = au1_src_top_tmp[col]
    BNE         SRC_TOP_LOOP

END_LOOPS:
    ADD         sp,sp,#0xE0
    // LDMFD sp!,{x4-x12,x15}             //Reload the registers from SP
    ldp         x27, x28,[sp],#16
    ldp         x25, x26,[sp],#16
    ldp         x23, x24,[sp],#16
    ldp         x21, x22,[sp],#16
    ldp         x19, x20,[sp],#16

    ret



