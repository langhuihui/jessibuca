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
//*  ihevc_sao_edge_offset_class1.s
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
//void ihevc_sao_edge_offset_class1(UWORD8 *pu1_src,
//                              WORD32 src_strd,
//                              UWORD8 *pu1_src_left,
//                              UWORD8 *pu1_src_top,
//                              UWORD8 *pu1_src_top_left,
//                              UWORD8 *pu1_src_top_right,
//                              UWORD8 *pu1_src_bot_left,
//                              UWORD8 *pu1_avail,
//                              WORD8 *pi1_sao_offset,
//                              WORD32 wd,
//                              WORD32 ht)
//**************Variables Vs Registers*****************************************
//x0 =>    *pu1_src
//x1 =>    src_strd
//x2 =>    *pu1_src_left
//x3 =>    *pu1_src_top
//x4    =>    *pu1_src_top_left
//x5    =>    *pu1_avail
//x6    =>    *pi1_sao_offset
//x7    =>    wd
//x8 =>    ht

.text
.p2align 2

.include "ihevc_neon_macros.s"

.globl gi1_table_edge_idx
.globl ihevc_sao_edge_offset_class1_av8

ihevc_sao_edge_offset_class1_av8:


    // STMFD sp!, {x4-x12, x14}            //stack stores the values of the arguments
    MOV         x5,x7                       //Loads pu1_avail

    LDR         x6,[sp]                     //Loads pi1_sao_offset
    LDR         w7,[sp,#8]                  //Loads wd
    LDR         w8,[sp,#16]                 //Loads ht


    stp         x19, x20,[sp,#-16]!

    SUB         x9,x7,#1                    //wd - 1
    LDRB        w10,[x3,x9]                 //pu1_src_top[wd - 1]
    STRB        w10,[x4]                    //*pu1_src_top_left = pu1_src_top[wd - 1]
    ADD         x10,x0,x9                   //pu1_src[row * src_strd + wd - 1]
    MOV         x11,x2                      //Move pu1_src_left pointer to x11
    MOV         x12,x8                      //Move ht to x12 for loop count
SRC_LEFT_LOOP:
    LDRB        w14,[x10]                   //Load pu1_src[row * src_strd + wd - 1]
    ADD         x10,x10,x1
    STRB        w14,[x11],#1                //pu1_src_left[row]
    SUBS        x12, x12,#1                 //Decrement the loop count
    BNE         SRC_LEFT_LOOP               //If not equal to 0 jump to the src_left_loop

    SUB         x12,x8,#1                   //ht - 1
    mul         x12, x12, x1                //(ht - 1) * src_strd
    ADD         x12,x12,x0                  //pu1_src[(ht - 1) * src_strd]

    LDRB        w4,[x5,#2]                  //pu1_avail[2]
    CMP         x4,#0                       //0 == pu1_avail[2]
    ADD         x20,x0,x1                   //pu1_src += src_strd
    csel        x0, x20, x0,EQ
    SUB         x20,x8,#1                   //ht--
    csel        x8, x20, x8,EQ

    LDRB        w4,[x5,#3]                  //pu1_avail[3]
    CMP         x4,#0                       //0 == pu1_avail[3]
    SUB         x20,x8,#1                   //ht--
    csel        x8, x20, x8,EQ

    movi        v0.16b, #2                  //const_2 = vdupq_n_s8(2)
    movi        v2.8h, #0                   //const_min_clip = vdupq_n_s16(0)
    movi        v4.8h, #255                 //const_max_clip = vdupq_n_u16((1 << bit_depth) - 1)
    ADRP        x14, :got:gi1_table_edge_idx //table pointer
    LDR         x14, [x14, #:got_lo12:gi1_table_edge_idx]
    LD1         {v6.8b},[x14]               //edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    LD1         {v7.8b},[x6]                //offset_tbl = vld1_s8(pi1_sao_offset)

    CMP         x7,#16                      //Compare wd with 16
    BLT         WIDTH_RESIDUE               //If not jump to WIDTH_RESIDUE where loop is unrolled for 8 case

WIDTH_LOOP_16:
    LDRB        w4,[x5,#2]                  //pu1_avail[2]
    CMP         x4,#0                       //0 == pu1_avail[2]
    SUB         x20,x0,x1                   //pu1_src -= src_strd
    csel        x9, x20, x9,EQ
    csel        x9, x3, x9,NE               //*pu1_src_top

    MOV         x10,x0                      //*pu1_src

    LD1         {v1.16b},[x9],#16           //pu1_top_row = vld1q_u8(pu1_src_top_cpy || pu1_src - src_strd)
    LD1         {v3.16b},[x0],#16           //pu1_cur_row = vld1q_u8(pu1_src)

    LD1         {v30.16b},[x12],#16         //vld1q_u8(pu1_src[(ht - 1) * src_strd])
    cmhi        v5.16b,  v3.16b ,  v1.16b   //vcgtq_u8(pu1_cur_row, pu1_top_row)

    ST1         { v30.16b},[x3],#16         //vst1q_u8(pu1_src_top[col])
    cmhi        v17.16b,  v1.16b ,  v3.16b  //vcltq_u8(pu1_cur_row, pu1_top_row)

    SUB         v16.16b,  v17.16b ,  v5.16b //sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    MOV         x11,x8                      //move ht to x11 for loop count

PU1_SRC_LOOP:
    ADD         x10,x10,x1                  //*pu1_src + src_strd
    LD1         {v18.16b},[x10]             //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    ADD         x6,x10,x1                   //II Iteration *pu1_src + src_strd

    cmhi        v5.16b,  v3.16b ,  v18.16b  //vcgtq_u8(pu1_cur_row, pu1_top_row)
    LD1         {v30.16b},[x6]              //II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)

    cmhi        v17.16b,  v18.16b ,  v3.16b //vcltq_u8(pu1_cur_row, pu1_top_row)
    SUB         x10,x10,x1

    SUB         v20.16b,  v17.16b ,  v5.16b //sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    Uxtl        v26.8h, v18.8b              //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    ADD         v5.16b,  v0.16b ,  v16.16b  //edge_idx = vaddq_s8(const_2, sign_up)
    Uxtl2       v28.8h, v18.16b             //II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))

    ADD         v5.16b,  v5.16b ,  v20.16b  //edge_idx = vaddq_s8(edge_idx, sign_down)
    cmhi        v22.16b,  v18.16b ,  v30.16b //II vcgtq_u8(pu1_cur_row, pu1_top_row)

    NEG         v16.16b, v20.16b            //sign_up = vnegq_s8(sign_down)
    TBL         v5.16b, {v6.16b},v5.16b     //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    cmhi        v24.16b,  v30.16b ,  v18.16b //II vcltq_u8(pu1_cur_row, pu1_top_row)

    SUB         v1.16b,  v24.16b ,  v22.16b //II sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
//  TBL v13.8b, {v6.16b},v13.8b                //vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    ADD         v22.16b,  v0.16b ,  v16.16b //II edge_idx = vaddq_s8(const_2, sign_up)


    NEG         v16.16b, v1.16b             //II sign_up = vnegq_s8(sign_down)
    TBL         v5.16b, {v7.16b},v5.16b     //offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    ADD         v22.16b,  v22.16b ,  v1.16b //II edge_idx = vaddq_s8(edge_idx, sign_down)


    Uxtl        v20.8h, v3.8b               //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    TBL         v22.16b, {v6.16b},v22.16b   //II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    SADDW       v20.8h,  v20.8h ,  v5.8b    //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    SMAX        v20.8h,  v20.8h ,  v2.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
//  TBL v23.8b, {v6.16b},v23.8b                //II vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    UMIN        v20.8h,  v20.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))


    Uxtl2       v1.8h, v3.16b               //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
//  TBL v13.8b, {v7.16b},v13.8b                    //offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    mov         v3.16b, v30.16b             //II pu1_cur_row = pu1_next_row

    SADDW2      v1.8h,  v1.8h ,  v5.16b     //pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    TBL         v24.16b, {v7.16b},v22.16b   //II offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    SMAX        v1.8h,  v1.8h ,  v2.8h      //pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)

    UMIN        v1.8h,  v1.8h ,  v4.8h      //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))
//  TBL v25.8b, {v7.16b},v23.8b                    //II offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))

    xtn         v20.8b,  v20.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])
    SADDW       v26.8h,  v26.8h ,  v24.8b   //II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    xtn2        v20.16b,  v1.8h             //vmovn_s16(pi2_tmp_cur_row.val[1])
    SADDW2      v28.8h,  v28.8h ,  v24.16b  //II pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)


    SMAX        v26.8h,  v26.8h ,  v2.8h    //II pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    UMIN        v26.8h,  v26.8h ,  v4.8h    //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    SMAX        v28.8h,  v28.8h ,  v2.8h    //II pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    UMIN        v28.8h,  v28.8h ,  v4.8h    //II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))
    ST1         { v20.16b},[x10],x1         //vst1q_u8(pu1_src_cpy, pu1_cur_row)

    xtn         v30.8b,  v26.8h             //II vmovn_s16(pi2_tmp_cur_row.val[0])
    SUBS        x11,x11,#2                  //II Decrement the ht loop count by 1
    xtn2        v30.16b,  v28.8h            //II vmovn_s16(pi2_tmp_cur_row.val[1])

    ST1         { v30.16b},[x10],x1         //II vst1q_u8(pu1_src_cpy, pu1_cur_row)

    BEQ         PU1_SRC_LOOP_END            //if 0 == pu1_avail[3] || 0 == pu1_avail[2] ht = ht--
    CMP         x11,#1                      //checking any residue remains
    BGT         PU1_SRC_LOOP                //If not equal jump to PU1_SRC_LOOP

    ADD         x10,x10,x1                  //*pu1_src + src_strd
    LD1         {v18.16b},[x10]             //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    cmhi        v5.16b,  v3.16b ,  v18.16b  //vcgtq_u8(pu1_cur_row, pu1_top_row)
    cmhi        v17.16b,  v18.16b ,  v3.16b //vcltq_u8(pu1_cur_row, pu1_top_row)
    SUB         v20.16b,  v17.16b ,  v5.16b //sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    SUB         x10,x10,x1

    ADD         v22.16b,  v0.16b ,  v16.16b //edge_idx = vaddq_s8(const_2, sign_up)
    ADD         v22.16b,  v22.16b ,  v20.16b //edge_idx = vaddq_s8(edge_idx, sign_down)
    TBL         v22.16b, {v6.16b},v22.16b   //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
//  TBL v23.8b, {v6.16b},v23.8b                //vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))

    TBL         v24.16b, {v7.16b},v22.16b   //offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    Uxtl        v26.8h, v3.8b               //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    SADDW       v26.8h,  v26.8h ,  v24.8b   //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    SMAX        v26.8h,  v26.8h ,  v2.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    UMIN        v26.8h,  v26.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

//  TBL v25.8b, {v7.16b},v23.8b                    //offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    Uxtl2       v28.8h, v3.16b              //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    SADDW2      v28.8h,  v28.8h ,  v24.16b  //pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    SMAX        v28.8h,  v28.8h ,  v2.8h    //pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    UMIN        v28.8h,  v28.8h ,  v4.8h    //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    xtn         v30.8b,  v26.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])
    xtn2        v30.16b,  v28.8h            //vmovn_s16(pi2_tmp_cur_row.val[1])

    ST1         { v30.16b},[x10],x1         //vst1q_u8(pu1_src_cpy, pu1_cur_row)

PU1_SRC_LOOP_END:
    mov         v3.16b, v18.16b             //pu1_cur_row = pu1_next_row
    SUBS        x7,x7,#16                   //Decrement the wd loop count by 16
    CMP         x7,#8                       //Check whether residue remains
    BEQ         WIDTH_RESIDUE               //If residue remains jump to residue loop
    BGT         WIDTH_LOOP_16               //If not equal jump to width_loop
    BLT         END_LOOPS                   //Jump to end function


WIDTH_RESIDUE:
    LDRB        w4,[x5,#2]                  //pu1_avail[2]
    CMP         x4,#0                       //0 == pu1_avail[2]
    SUB         x20,x0,x1                   //pu1_src -= src_strd
    csel        x9, x20, x9,EQ
    csel        x9, x3, x9,NE               //*pu1_src_top
    MOV         x10,x0

    LD1         {v1.16b},[x9],#16           //pu1_top_row = vld1q_u8(pu1_src_top_cpy || pu1_src - src_strd)
    LD1         {v3.16b},[x0],#16           //pu1_cur_row = vld1q_u8(pu1_src)

    LD1         {v30.8b},[x12]              //vld1_u8(pu1_src[(ht - 1) * src_strd])
    ST1         {v30.8b},[x3]               //vst1_u8(pu1_src_top[col])

    cmhi        v5.16b,  v3.16b ,  v1.16b   //vcgtq_u8(pu1_cur_row, pu1_top_row)
    cmhi        v17.16b,  v1.16b ,  v3.16b  //vcltq_u8(pu1_cur_row, pu1_top_row)
    SUB         v16.16b,  v17.16b ,  v5.16b //sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    MOV         x11,x8                      //move ht to x11 for loop count

PU1_SRC_LOOP_RESIDUE:
    ADD         x10,x10,x1                  //*pu1_src + src_strd
    LD1         {v18.16b},[x10]             //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    ADD         x6,x10,x1                   //II Iteration *pu1_src + src_strd

    cmhi        v5.16b,  v3.16b ,  v18.16b  //vcgtq_u8(pu1_cur_row, pu1_next_row)
    LD1         {v30.16b},[x6]              //II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)

    cmhi        v17.16b,  v18.16b ,  v3.16b //vcltq_u8(pu1_cur_row, pu1_next_row)
    SUB         x10,x10,x1

    SUB         v20.16b,  v17.16b ,  v5.16b //sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    Uxtl        v26.8h, v18.8b              //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    ADD         v5.16b,  v0.16b ,  v16.16b  //edge_idx = vaddq_s8(const_2, sign_up)
    cmhi        v22.16b,  v18.16b ,  v30.16b //II vcgtq_u8(pu1_cur_row, pu1_next_row)

    ADD         v5.16b,  v5.16b ,  v20.16b  //edge_idx = vaddq_s8(edge_idx, sign_down)
    cmhi        v24.16b,  v30.16b ,  v18.16b //II vcltq_u8(pu1_cur_row, pu1_next_row)

    NEG         v16.16b, v20.16b            //sign_up = vnegq_s8(sign_down)
    TBL         v5.8b, {v6.16b},v5.8b       //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    SUB         v20.16b,  v24.16b ,  v22.16b //II sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    ADD         v22.16b,  v0.16b ,  v16.16b //II edge_idx = vaddq_s8(const_2, sign_up)
    TBL         v5.8b, {v7.16b},v5.8b       //offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    NEG         v16.16b, v20.16b            //II sign_up = vnegq_s8(sign_down)

    ADD         v22.16b,  v22.16b ,  v20.16b //II edge_idx = vaddq_s8(edge_idx, sign_down)
    Uxtl        v20.8h, v3.8b               //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    SADDW       v20.8h,  v20.8h ,  v5.8b    //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    TBL         v22.8b, {v6.16b},v22.8b     //II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    SMAX        v20.8h,  v20.8h ,  v2.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    UMIN        v20.8h,  v20.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))
    TBL         v24.8b, {v7.16b},v22.8b     //II offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    xtn         v20.8b,  v20.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])

    SADDW       v26.8h,  v26.8h ,  v24.8b   //II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    SMAX        v26.8h,  v26.8h ,  v2.8h    //II pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    UMIN        v26.8h,  v26.8h ,  v4.8h    //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    mov         v3.16b, v30.16b             //II pu1_cur_row = pu1_next_row
    ST1         {v20.8b},[x10],x1           //vst1q_u8(pu1_src_cpy, pu1_cur_row)
    xtn         v30.8b,  v26.8h             //II vmovn_s16(pi2_tmp_cur_row.val[0])

    SUBS        x11,x11,#2                  //Decrement the ht loop count by 1
    ST1         {v30.8b},[x10],x1           //II vst1q_u8(pu1_src_cpy, pu1_cur_row)

    BEQ         END_LOOPS
    CMP         x11,#1
    BGT         PU1_SRC_LOOP_RESIDUE        //If not equal jump to PU1_SRC_LOOP


    ADD         x10,x10,x1                  //*pu1_src + src_strd
    LD1         {v18.16b},[x10]             //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    cmhi        v5.16b,  v3.16b ,  v18.16b  //vcgtq_u8(pu1_cur_row, pu1_next_row)
    cmhi        v17.16b,  v18.16b ,  v3.16b //vcltq_u8(pu1_cur_row, pu1_next_row)
    SUB         v20.16b,  v17.16b ,  v5.16b //sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    SUB         x10,x10,x1

    ADD         v22.16b,  v0.16b ,  v16.16b //edge_idx = vaddq_s8(const_2, sign_up)
    ADD         v22.16b,  v22.16b ,  v20.16b //edge_idx = vaddq_s8(edge_idx, sign_down)
    TBL         v22.8b, {v6.16b},v22.8b     //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))

    TBL         v24.8b, {v7.16b},v22.8b     //offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    Uxtl        v26.8h, v3.8b               //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    SADDW       v26.8h,  v26.8h ,  v24.8b   //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    SMAX        v26.8h,  v26.8h ,  v2.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    UMIN        v26.8h,  v26.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    xtn         v30.8b,  v26.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])

    ST1         {v30.8b},[x10],x1           //vst1q_u8(pu1_src_cpy, pu1_cur_row)

END_LOOPS:
    // LDMFD sp!,{x4-x12,x15}             //Reload the registers from SP
    ldp         x19, x20,[sp], #16

    ret






