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
//*  ihevc_sao_edge_offset_class1_chroma.s
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
//void ihevc_sao_edge_offset_class1_chroma(UWORD8 *pu1_src,
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
//x7    =>    *pi1_sao_offset_v
//x8    =>    wd
//x9 =>    ht

.text
.p2align 2
.include "ihevc_neon_macros.s"

.globl gi1_table_edge_idx
.globl ihevc_sao_edge_offset_class1_chroma_av8

ihevc_sao_edge_offset_class1_chroma_av8:


    ldr         x8,[sp,#0]
    ldr         x9,[sp,#8]
    ldr         w10,[sp,#16]
    ldr         w11,[sp,#24]



    // STMFD sp!, {x4-x12, x14}            //stack stores the values of the arguments
    stp         x19, x20,[sp,#-16]!
    stp         x21, x22,[sp,#-16]!
    stp         x23, x24,[sp,#-16]!
    stp         x25, x26,[sp,#-16]!

    mov         x15,x4 // *pu1_src_top_left 40
    mov         x16,x5 // *pu1_src_top_right 44
    mov         x17,x6 // *pu1_src_bot_left 48
    mov         x21,x7 // *pu1_avail 52
    mov         x22,x8 // *pi1_sao_offset_u 56
    mov         x23,x9 // *pi1_sao_offset_v 60
    mov         x24,x10 // wd 64
    mov         x25,x11 // ht 68

    mov         x4,x15
    mov         x5,x21
    mov         x6,x22
    mov         x7,x23
    mov         x8,x24
    mov         x9,x25

    SUB         x10,x8,#2                   //wd - 2
    LDRH        w11,[x3,x10]                //pu1_src_top[wd - 2]
    STRH        w11,[x4]                    //*pu1_src_top_left = pu1_src_top[wd - 2]
    ADD         x11,x0,x10                  //pu1_src[row * src_strd + wd - 2]
    MOV         x12,x2                      //Move pu1_src_left pointer to x11
    MOV         x14,x9                      //Move ht to x14 for loop count
SRC_LEFT_LOOP:
    LDRH        w10,[x11]                   //Load pu1_src[row * src_strd + wd - 2]
    ADD         x11,x11,x1
    STRH        w10,[x12],#2                //pu1_src_left[row]
    SUBS        x14, x14,#1                 //Decrement the loop count
    BNE         SRC_LEFT_LOOP               //If not equal to 0 jump to the src_left_loop

    SUB         x12,x9,#1                   //ht - 1
    mul         x12, x12, x1                //(ht - 1) * src_strd
    ADD         x12,x12,x0                  //pu1_src[(ht - 1) * src_strd]

    LDRB        w4,[x5,#2]                  //pu1_avail[2]
    CMP         x4,#0                       //0 == pu1_avail[2]
    ADD         x20,x0,x1                   //pu1_src += src_strd
    csel        x0, x20, x0,EQ
    SUB         x20,x9,#1                   //ht--
    csel        x9, x20, x9,EQ

    LDRB        w4,[x5,#3]                  //pu1_avail[3]
    CMP         x4,#0                       //0 == pu1_avail[3]
    SUB         x20,x9,#1                   //ht--
    csel        x9, x20, x9,EQ

    movi        v0.16b, #2                  //const_2 = vdupq_n_s8(2)
    movi        v2.8h, #0                   //const_min_clip = vdupq_n_s16(0)
    movi        v4.8h, #255                 //const_max_clip = vdupq_n_u16((1 << bit_depth) - 1)
    ADRP        x14, :got:gi1_table_edge_idx //table pointer
    LDR         x14, [x14, #:got_lo12:gi1_table_edge_idx]
    LD1         {v6.8b},[x14]               //edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    LD1         {v7.8b},[x6]                //offset_tbl_u = vld1_s8(pi1_sao_offset_u)
    LD1         {v1.8b},[x7]                //offset_tbl_v = vld1_s8(pi1_sao_offset_v)

    CMP         x8,#16                      //Compare wd with 16
    BLT         WIDTH_RESIDUE               //If not jump to WIDTH_RESIDUE where loop is unrolled for 8 case

WIDTH_LOOP_16:
    LDRB        w4,[x5,#2]                  //pu1_avail[2]
    CMP         x4,#0                       //0 == pu1_avail[2]
    SUB         x20,x0,x1                   //pu1_src -= src_strd
    csel        x11, x20, x11,EQ
    csel        x11, x3, x11,NE             //*pu1_src_top

    MOV         x10,x0                      //*pu1_src

    LD1         {v28.16b},[x11],#16         //pu1_top_row = vld1q_u8(pu1_src_top_cpy || pu1_src - src_strd)
    //LD1 {v29.8b},[x11],#8                    //pu1_top_row = vld1q_u8(pu1_src_top_cpy || pu1_src - src_strd)
    LD1         {v3.16b},[x0],#16           //pu1_cur_row = vld1q_u8(pu1_src)
    //LD1 {v11.8b},[x0],#8                    //pu1_cur_row = vld1q_u8(pu1_src)

    LD1         {v30.16b},[x12],#16         //vld1q_u8(pu1_src[(ht - 1) * src_strd])
    //LD1 {v31.8b},[x12],#8                    //vld1q_u8(pu1_src[(ht - 1) * src_strd])
    cmhi        v5.16b,  v3.16b ,  v28.16b  //vcgtq_u8(pu1_cur_row, pu1_top_row)

    ST1         { v30.16b},[x3],#16         //vst1q_u8(pu1_src_top[col])
    cmhi        v19.16b,  v28.16b ,  v3.16b //vcltq_u8(pu1_cur_row, pu1_top_row)

    SUB         v16.16b,  v19.16b ,  v5.16b //sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    MOV         x11,x9                      //move ht to x11 for loop count

PU1_SRC_LOOP:
    ADD         x10,x10,x1                  //*pu1_src + src_strd
    LD1         {v18.16b},[x10]             //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //LD1 {v19.8b},[x10]                    //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x10, x10,#8
    ADD         x6,x10,x1                   //II Iteration *pu1_src + src_strd

    //mov   v19.d[0],v18.d[1]
    cmhi        v5.16b,  v3.16b ,  v18.16b  //vcgtq_u8(pu1_cur_row, pu1_top_row)
    LD1         {v30.16b},[x6]              //II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //LD1 {v31.8b},[x6]                    //II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x6, x6,#8

    cmhi        v19.16b,  v18.16b ,  v3.16b //vcltq_u8(pu1_cur_row, pu1_top_row)
    SUB         x10,x10,x1

    SUB         v20.16b,  v19.16b ,  v5.16b //sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    Uxtl        v26.8h, v18.8b              //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    ADD         v5.16b,  v0.16b ,  v16.16b  //edge_idx = vaddq_s8(const_2, sign_up)
    Uxtl2       v28.8h, v18.16b             //II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))

    ADD         v5.16b,  v5.16b ,  v20.16b  //edge_idx = vaddq_s8(edge_idx, sign_down)
    cmhi        v22.16b,  v18.16b ,  v30.16b //II vcgtq_u8(pu1_cur_row, pu1_top_row)

    mov         v16.d[1],v16.d[0]
    NEG         v16.16b, v20.16b            //sign_up = vnegq_s8(sign_down)
    TBL         v5.16b, {v6.16b},v5.16b     //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    cmhi        v24.16b,  v30.16b ,  v18.16b //II vcltq_u8(pu1_cur_row, pu1_top_row)

    SUB         v28.16b,  v24.16b ,  v22.16b //II sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    //TBL v13.8b, {v6.16b},v13.8b                //vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    ADD         v22.16b,  v0.16b ,  v16.16b //II edge_idx = vaddq_s8(const_2, sign_up)

    mov         v17.d[0], v5.d[1]
    UZP1        v27.8b, v5.8b, v17.8b
    UZP2        v17.8b, v5.8b, v17.8b
    mov         v5.8b,v27.8b
    NEG         v16.16b, v28.16b            //II sign_up = vnegq_s8(sign_down)
    TBL         v5.8b, {v7.16b},v5.8b       //offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    ADD         v22.16b,  v22.16b ,  v28.16b //II edge_idx = vaddq_s8(edge_idx, sign_down)

    Uxtl        v20.8h, v3.8b               //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    TBL         v17.8b, {v1.16b},v17.8b
    ZIP1        v27.8b, v5.8b, v17.8b
    ZIP2        v17.8b, v5.8b, v17.8b
    mov         v5.8b,v27.8b

    SADDW       v20.8h,  v20.8h ,  v5.8b    //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    TBL         v22.16b, {v6.16b},v22.16b   //II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    SMAX        v20.8h,  v20.8h ,  v2.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    UMIN        v20.8h,  v20.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))
    //TBL v23.8b, {v6.16b},v23.8b                //II vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))

    mov         v23.d[0], v22.d[1]
    UZP1        v27.8b, v22.8b, v23.8b
    UZP2        v23.8b, v22.8b, v23.8b
    mov         v22.8b,v27.8b

    Uxtl2       v28.8h, v3.16b              //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    //VTBL.8        D13,D7,D13                    @offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    mov         v3.16b, v30.16b             //II pu1_cur_row = pu1_next_row

    SADDW       v28.8h,  v28.8h ,  v17.8b   //pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    TBL         v24.8b, {v7.16b},v22.8b     //offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    SMAX        v28.8h,  v28.8h ,  v2.8h    //pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)

    TBL         v25.8b, {v1.16b},v23.8b
    ZIP1        v27.8b, v24.8b, v25.8b
    ZIP2        v25.8b, v24.8b, v25.8b
    mov         v24.8b,v27.8b
    //VTBL.8        D24,D7,D22                    @II offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    UMIN        v28.8h,  v28.8h ,  v4.8h    //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))
    //VTBL.8        D25,D7,D23                    @II offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))

    xtn         v20.8b,  v20.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])
    SADDW       v26.8h,  v26.8h ,  v24.8b   //II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    xtn2        v20.16b,  v28.8h            //vmovn_s16(pi2_tmp_cur_row.val[1])

    Uxtl2       v28.8h, v18.16b             //II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    SADDW       v28.8h,  v28.8h ,  v25.8b   //II pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)


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
    //LD1 {v19.8b},[x10]                    //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x10, x10,#8
    cmhi        v5.16b,  v3.16b ,  v18.16b  //vcgtq_u8(pu1_cur_row, pu1_top_row)
    cmhi        v19.16b,  v18.16b ,  v3.16b //vcltq_u8(pu1_cur_row, pu1_top_row)
    SUB         v20.16b,  v19.16b ,  v5.16b //sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    SUB         x10,x10,x1

    ADD         v22.16b,  v0.16b ,  v16.16b //edge_idx = vaddq_s8(const_2, sign_up)
    ADD         v22.16b,  v22.16b ,  v20.16b //edge_idx = vaddq_s8(edge_idx, sign_down)
    TBL         v22.16b, {v6.16b},v22.16b   //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    //TBL v23.8b, {v6.16b},v23.8b                //vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))

    mov         v23.d[0],v22.d[1]
    UZP1        v27.8b, v22.8b, v23.8b
    UZP2        v23.8b, v22.8b, v23.8b
    mov         v22.8b,v27.8b
    TBL         v24.8b, {v7.16b},v22.8b
    TBL         v25.8b, {v1.16b},v23.8b
    ZIP1        v27.8b, v24.8b, v25.8b
    ZIP2        v25.8b, v24.8b, v25.8b
    mov         v24.8b,v27.8b

    //VTBL.8        D24,D7,D22                    @offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    Uxtl        v26.8h, v3.8b               //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    SADDW       v26.8h,  v26.8h ,  v24.8b   //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    SMAX        v26.8h,  v26.8h ,  v2.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    UMIN        v26.8h,  v26.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    //VTBL.8        D25,D7,D23                    @offset = vtbl1_s8(offset_tbl, vget_high_s8(edge_idx))
    Uxtl2       v28.8h, v3.16b              //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))
    SADDW       v28.8h,  v28.8h ,  v25.8b   //pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    SMAX        v28.8h,  v28.8h ,  v2.8h    //pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    UMIN        v28.8h,  v28.8h ,  v4.8h    //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    xtn         v30.8b,  v26.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])
    xtn2        v30.16b,  v28.8h            //vmovn_s16(pi2_tmp_cur_row.val[1])

    ST1         { v30.16b},[x10],x1         //vst1q_u8(pu1_src_cpy, pu1_cur_row)

PU1_SRC_LOOP_END:
    mov         v3.16b, v18.16b             //pu1_cur_row = pu1_next_row
    SUBS        x8,x8,#16                   //Decrement the wd loop count by 16
    CMP         x8,#8                       //Check whether residue remains
    BEQ         WIDTH_RESIDUE               //If residue remains jump to residue loop
    BGT         WIDTH_LOOP_16               //If not equal jump to width_loop
    BLT         END_LOOPS                   //Jump to end function


WIDTH_RESIDUE:
    LDRB        w4,[x5,#2]                  //pu1_avail[2]
    CMP         x4,#0                       //0 == pu1_avail[2]
    SUB         x20,x0,x1                   //pu1_src -= src_strd
    csel        x11, x20, x11,EQ
    csel        x11, x3, x11,NE             //*pu1_src_top
    MOV         x10,x0

    LD1         {v28.16b},[x11]             //pu1_top_row = vld1q_u8(pu1_src_top_cpy || pu1_src - src_strd)
    //LD1 {v29.8b},[x11],#8                    //pu1_top_row = vld1q_u8(pu1_src_top_cpy || pu1_src - src_strd)
    LD1         {v3.16b},[x0]               //pu1_cur_row = vld1q_u8(pu1_src)
    //LD1 {v11.8b},[x0],#8                    //pu1_cur_row = vld1q_u8(pu1_src)

    LD1         {v30.8b},[x12]              //vld1_u8(pu1_src[(ht - 1) * src_strd])
    ST1         {v30.8b},[x3]               //vst1_u8(pu1_src_top[col])

    cmhi        v5.16b,  v3.16b ,  v28.16b  //vcgtq_u8(pu1_cur_row, pu1_top_row)
    cmhi        v19.16b,  v28.16b ,  v3.16b //vcltq_u8(pu1_cur_row, pu1_top_row)
    SUB         v16.16b,  v19.16b ,  v5.16b //sign_up = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    MOV         x11,x9                      //move ht to x11 for loop count

PU1_SRC_LOOP_RESIDUE:
    ADD         x10,x10,x1                  //*pu1_src + src_strd
    LD1         {v18.16b},[x10]             //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //LD1 {v19.8b},[x10]                    //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x10, x10,#8
    ADD         x6,x10,x1                   //II Iteration *pu1_src + src_strd

    cmhi        v5.16b,  v3.16b ,  v18.16b  //vcgtq_u8(pu1_cur_row, pu1_next_row)
    LD1         {v30.16b},[x6]              //II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //LD1 {v31.8b},[x6]                    //II pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x6, x6,#8

    cmhi        v19.16b,  v18.16b ,  v3.16b //vcltq_u8(pu1_cur_row, pu1_next_row)
    SUB         x10,x10,x1

    SUB         v20.16b,  v19.16b ,  v5.16b //sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    Uxtl        v26.8h, v18.8b              //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    ADD         v5.16b,  v0.16b ,  v16.16b  //edge_idx = vaddq_s8(const_2, sign_up)
    cmhi        v22.16b,  v18.16b ,  v30.16b //II vcgtq_u8(pu1_cur_row, pu1_next_row)

    ADD         v5.16b,  v5.16b ,  v20.16b  //edge_idx = vaddq_s8(edge_idx, sign_down)
    cmhi        v24.16b,  v30.16b ,  v18.16b //II vcltq_u8(pu1_cur_row, pu1_next_row)

    NEG         v16.16b, v20.16b            //sign_up = vnegq_s8(sign_down)
    TBL         v5.8b, {v6.16b},v5.8b       //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    SUB         v20.16b,  v24.16b ,  v22.16b //II sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    UZP1        v27.8b, v5.8b, v17.8b
    UZP2        v17.8b, v5.8b, v17.8b
    mov         v5.8b,v27.8b

    ADD         v22.16b,  v0.16b ,  v16.16b //II edge_idx = vaddq_s8(const_2, sign_up)
    TBL         v5.8b, {v7.16b},v5.8b
    NEG         v16.16b, v20.16b            //II sign_up = vnegq_s8(sign_down)

    TBL         v17.8b, {v1.16b},v17.8b
    ZIP1        v27.8b, v5.8b, v17.8b
    ZIP2        v17.8b, v5.8b, v17.8b
    mov         v5.8b,v27.8b

    //VTBL.8        D12,D7,D12                    @offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))

    ADD         v22.16b,  v22.16b ,  v20.16b //II edge_idx = vaddq_s8(edge_idx, sign_down)
    Uxtl        v20.8h, v3.8b               //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    SADDW       v20.8h,  v20.8h ,  v5.8b    //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    TBL         v22.8b, {v6.16b},v22.8b     //II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    SMAX        v20.8h,  v20.8h ,  v2.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    UZP1        v27.8b, v22.8b, v23.8b
    UZP2        v23.8b, v22.8b, v23.8b
    mov         v22.8b,v27.8b

    UMIN        v20.8h,  v20.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))
    TBL         v24.8b, {v7.16b},v22.8b
    xtn         v20.8b,  v20.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])

    TBL         v25.8b, {v1.16b},v23.8b
    ZIP1        v27.8b, v24.8b, v25.8b
    ZIP2        v25.8b, v24.8b, v25.8b
    mov         v24.8b,v27.8b
    //VTBL.8        D24,D7,D22                    @II offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))

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
    //LD1 {v19.8b},[x10]                    //pu1_next_row = vld1q_u8(pu1_src_cpy + src_strd)
    //SUB x10, x10,#8
    cmhi        v5.16b,  v3.16b ,  v18.16b  //vcgtq_u8(pu1_cur_row, pu1_next_row)
    cmhi        v19.16b,  v18.16b ,  v3.16b //vcltq_u8(pu1_cur_row, pu1_next_row)
    SUB         v20.16b,  v19.16b ,  v5.16b //sign_down = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    SUB         x10,x10,x1

    ADD         v22.16b,  v0.16b ,  v16.16b //edge_idx = vaddq_s8(const_2, sign_up)
    ADD         v22.16b,  v22.16b ,  v20.16b //edge_idx = vaddq_s8(edge_idx, sign_down)
    TBL         v22.8b, {v6.16b},v22.8b     //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))

    UZP1        v27.8b, v22.8b, v23.8b
    UZP2        v23.8b, v22.8b, v23.8b
    mov         v22.8b,v27.8b

    TBL         v24.8b, {v7.16b},v22.8b
    TBL         v25.8b, {v1.16b},v23.8b
    ZIP1        v27.8b, v24.8b, v25.8b
    ZIP2        v25.8b, v24.8b, v25.8b
    mov         v24.8b,v27.8b

    //VTBL.8        D24,D7,D22                    @offset = vtbl1_s8(offset_tbl, vget_low_s8(edge_idx))
    Uxtl        v26.8h, v3.8b               //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    SADDW       v26.8h,  v26.8h ,  v24.8b   //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    SMAX        v26.8h,  v26.8h ,  v2.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    UMIN        v26.8h,  v26.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    xtn         v30.8b,  v26.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])

    ST1         {v30.8b},[x10],x1           //vst1q_u8(pu1_src_cpy, pu1_cur_row)

END_LOOPS:
    // LDMFD sp!,{x4-x12,x15}             //Reload the registers from SP
    ldp         x25, x26,[sp],#16
    ldp         x23, x24,[sp],#16
    ldp         x21, x22,[sp],#16
    ldp         x19, x20,[sp],#16


    ret






