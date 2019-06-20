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
//*  ihevc_sao_edge_offset_class0_chroma.s
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
//void ihevc_sao_edge_offset_class0_chroma(UWORD8 *pu1_src,
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
//
//**************Variables Vs Registers*****************************************
//x0 =>    *pu1_src
//x1 =>    src_strd
//x2 =>    *pu1_src_left
//x3 =>    *pu1_src_top
//x4    =>    *pu1_src_top_left
//x7    =>    *pu1_avail
//x8    =>    *pi1_sao_offset_u
//x5    =>    *pi1_sao_offset_v
//x9    =>    wd
//x10=>    ht

.text
.p2align 2
.include "ihevc_neon_macros.s"

.globl gi1_table_edge_idx
.globl ihevc_sao_edge_offset_class0_chroma_av8

ihevc_sao_edge_offset_class0_chroma_av8:

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

    MOV         x9, x24                     //Loads wd

    MOV         x4, x15                     //Loads pu1_src_top_left
    ADD         x11,x3,x9                   //pu1_src_top[wd]

    MOV         x10, x25                    //Loads ht
    movi        v2.16b, #2                  //const_2 = vdupq_n_s8(2)
    SUB         x20,x11,#2
    LDRH        w12,[x20]                   //pu1_src_top[wd - 1]

    MOV         x7, x21                     //Loads pu1_avail
    movi        v4.8h, #0                   //const_min_clip = vdupq_n_s16(0)
    STRH        w12,[x4]                    //*pu1_src_top_left = pu1_src_top[wd - 1]

    MOV         x8, x22                     //Loads pi1_sao_offset_u
    movi        v6.8h, #255                 //const_max_clip = vdupq_n_u16((1 << bit_depth) - 1)
    SUB         x4,x10,#1                   //(ht - 1)

    ADRP        x14, :got:gi1_table_edge_idx //table pointer
    LDR         x14, [x14, #:got_lo12:gi1_table_edge_idx]
    movi        v3.16b, #0xFF               //au1_mask = vdupq_n_s8(-1)
    mul         x4, x4, x1                  //(ht - 1) * src_strd

    MOV         x5, x23                     //Loads pi1_sao_offset_v
    LD1         {v7.8b},[x8]                //offset_tbl = vld1_s8(pi1_sao_offset_u)
    ADD         x4,x4,x0                    //pu1_src[(ht - 1) * src_strd]

    MOV         x6,x0                       //pu1_src_org
    LD1         {v5.8b},[x14]               //edge_idx_tbl = vld1_s8(gi1_table_edge_idx)
    MOV         x12,x9                      //Move wd to x12 for loop count

SRC_TOP_LOOP:                               //wd is always multiple of 8
    LD1         {v0.8b},[x4],#8             //Load pu1_src[(ht - 1) * src_strd + col]
    SUBS        x12,x12,#8                  //Decrement the loop counter by 8
    ST1         {v0.8b},[x3],#8             //Store to pu1_src_top[col]
    BNE         SRC_TOP_LOOP
    ADD         x6,x6,#14                   //pu1_src_org[14]

    MOV         x3,x2                       //pu1_src_left backup to reload later
    LD1         {v0.8b},[x5]                //offset_tbl = vld1_s8(pi1_sao_offset_v)
    CMP         x9,#16                      //Compare wd with 16

    BLT         WIDTH_RESIDUE               //If not jump to WIDTH_RESIDUE where loop is unrolled for 8 case

    MOV         x8,x9                       //move wd to x8 for loop count

WIDTH_LOOP_16:
    CMP         x8,x9                       //if(col == wd)
    BNE         AU1_MASK_FF                 //jump to else part
    LDRB        w12,[x7]                    //pu1_avail[0]
    mov         v3.b[0], w12                //vsetq_lane_s8(pu1_avail[0], au1_mask, 0)
    mov         v3.b[1], w12                //vsetq_lane_s8(pu1_avail[0], au1_mask, 1)
    B           SKIP_AU1_MASK_FF            //Skip the else part

AU1_MASK_FF:
    MOV         x12,#-1                     //move -1 to x12
    mov         v3.h[0], w12                //au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

SKIP_AU1_MASK_FF:
    CMP         x8,#16                      //If col == 16
    BNE         SKIP_MASKING_IF_NOT16       //If not skip masking
    LDRB        w12,[x7,#1]                 //pu1_avail[1]
    mov         v3.b[14], w12               //au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 14)
    mov         v3.b[15], w12               //au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)

SKIP_MASKING_IF_NOT16:
    MOV         x12,x0                      //pu1_src_cpy = pu1_src
    MOV         x4,x10                      //move ht to x4 for loop count

PU1_SRC_LOOP:
    LDRH        w11,[x2]                    //load pu1_src_left since ht - row =0 when it comes first pu1_src_left is incremented later
    LD1         {v19.16b},[x12],x1          //pu1_cur_row = vld1q_u8(pu1_src_cpy)
    //LD1 {v13.8b},[x12],x1                    //pu1_cur_row = vld1q_u8(pu1_src_cpy)
    //SUB x12, x12,#8
    SUB         x5,x9,x8                    //wd - col

    SUB         x14,x10,x4                  //ht - row
    mov         v21.h[7], w11               //vsetq_lane_u16(pu1_src_left[ht - row], pu1_cur_row_tmp, 14,15)
    mul         x14, x14, x1                //(ht - row) * src_strd

    LD1         {v30.16b},[x12]             //II Iteration pu1_cur_row = vld1q_u8(pu1_src_cpy)
    //LD1 {v31.8b},[x12]                    //II Iteration pu1_cur_row = vld1q_u8(pu1_src_cpy)
    //SUB x12, x12,#8
    EXT         v21.16b,  v21.16b ,  v19.16b,#14 //pu1_cur_row_tmp = vextq_u8(pu1_cur_row_tmp, pu1_cur_row, 14)
    SUB         x12,x12,x1

    LDRH        w11,[x2,#2]                 //II load pu1_src_left since ht - row =0
    cmhi        v16.16b,  v19.16b ,  v21.16b //vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
    ADD         x5,x14,x5                   //(ht - row) * src_strd + (wd - col)

    mov         v28.h[7], w11               //II vsetq_lane_u16(pu1_src_left[ht - row], pu1_cur_row_tmp, 14,15)
    cmhi        v18.16b,  v21.16b ,  v19.16b //vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)

    LDRH        w14,[x6,x5]                 //pu1_src_org[(ht - row) * src_strd + 14 + (wd - col)]
    SUB         v20.16b,  v18.16b ,  v16.16b //sign_left = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    SUB         x4,x4,#1

    LDRB        w11,[x12,#16]               //pu1_src_cpy[16]
    EXT         v28.16b,  v28.16b ,  v30.16b,#14 //II pu1_cur_row_tmp = vextq_u8(pu1_cur_row_tmp, pu1_cur_row, 14)

    mov         v21.b[0], w11               //pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[16], pu1_cur_row_tmp, 0)
    cmhi        v26.16b,  v30.16b ,  v28.16b //II vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)

    LDRB        w11,[x12,#17]               //pu1_src_cpy[17]
    cmhi        v24.16b,  v28.16b ,  v30.16b //II vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    STRH        w14,[x2],#2                 //pu1_src_left[(ht - row)] = au1_src_left_tmp[(ht - row)]

    ADD         x12,x12,x1
    mov         v21.b[1], w11               //pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[17], pu1_cur_row_tmp, 1)
    LDRB        w11,[x12,#16]               //II pu1_src_cpy[16]

    EXT         v21.16b,  v19.16b ,  v21.16b,#2 //pu1_cur_row_tmp = vextq_u8(pu1_cur_row, pu1_cur_row_tmp, 2)
    mov         v28.b[0], w11               //II pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[16], pu1_cur_row_tmp, 0)

    LDRB        w11,[x12,#17]               //II pu1_src_cpy[17]
    cmhi        v16.16b,  v19.16b ,  v21.16b //vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
    SUB         x12,x12,x1

    cmhi        v18.16b,  v21.16b ,  v19.16b //vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    mov         v28.b[1], w11               //II pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[17], pu1_cur_row_tmp, 1)

    SUB         v22.16b,  v18.16b ,  v16.16b //sign_right = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    EXT         v28.16b,  v30.16b ,  v28.16b,#2 //II pu1_cur_row_tmp = vextq_u8(pu1_cur_row, pu1_cur_row_tmp, 2)

    ADD         v21.16b,  v2.16b ,  v20.16b //edge_idx = vaddq_s8(const_2, sign_left)

    mov         v5.d[1],v5.d[0]
    ADD         v21.16b,  v21.16b ,  v22.16b //edge_idx = vaddq_s8(edge_idx, sign_right)
    TBL         v21.16b, {v5.16b},v21.16b   //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    SUB         v20.16b,  v24.16b ,  v26.16b //II sign_left = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    cmhi        v26.16b,  v30.16b ,  v28.16b //II vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
//    TBL v15.8b, {v10.16b},v15.8b                //vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    cmhi        v24.16b,  v28.16b ,  v30.16b //II vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)

    AND         v21.16b,  v21.16b ,  v3.16b //edge_idx = vandq_s8(edge_idx, au1_mask)
    mov         v23.d[0],v21.d[1]
    UZP1        v1.8b, v21.8b, v23.8b
    UZP2        v23.8b, v21.8b, v23.8b
    mov         v21.8b, v1.8b

    //mov v11.d[1],v0.d[0]
    //mov v14.d[1],v15.d[0]
    SUB         v22.16b,  v24.16b ,  v26.16b //II sign_right = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    TBL         v16.8b, {v7.16b},v21.8b     //offset = vtbl1_s8(offset_tbl_u, vget_low_s8(edge_idx))
    ADD         v24.16b,  v2.16b ,  v20.16b //II edge_idx = vaddq_s8(const_2, sign_left)

    Uxtl        v18.8h, v19.8b              //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    TBL         v17.8b, {v0.16b},v23.8b
    ADD         v24.16b,  v24.16b ,  v22.16b //II edge_idx = vaddq_s8(edge_idx, sign_right)

    //mov v17.d[0],v16.d[1]
    ZIP1        v1.8b, v16.8b, v17.8b
    ZIP2        v17.8b, v16.8b, v17.8b
    mov         v16.8b, v1.8b
    TBL         v24.16b, {v5.16b},v24.16b   //II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    Uxtl2       v19.8h, v19.16b             //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))

    //mov v16.d[1],v17.d[0]
    SADDW       v18.8h,  v18.8h ,  v16.8b   //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    //TBL v25.8b, {v10.16b},v25.8b                //II vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    SMAX        v18.8h,  v18.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    AND         v24.16b,  v24.16b ,  v3.16b //II edge_idx = vandq_s8(edge_idx, au1_mask)
    mov         v25.d[0],v24.d[1]
    UMIN        v18.8h,  v18.8h ,  v6.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))
    UZP1        v1.8b, v24.8b, v25.8b
    UZP2        v25.8b, v24.8b, v25.8b      //II
    mov         v24.8b, v1.8b

    //mov v24.d[1],v25.d[0]
    SADDW       v19.8h,  v19.8h ,  v17.8b   //pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    TBL         v26.8b, {v7.16b},v24.8b     //II offset = vtbl1_s8(offset_tbl_u, vget_low_s8(edge_idx))
    SMAX        v19.8h,  v19.8h ,  v4.8h    //pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)

    UMIN        v19.8h,  v19.8h ,  v6.8h    //pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))
    TBL         v27.8b, {v0.16b},v25.8b     //II
    xtn         v21.8b,  v18.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])

    //mov v27.d[0],v26.d[1]
    xtn         v23.8b,  v19.8h             //vmovn_s16(pi2_tmp_cur_row.val[1])
    ZIP1        v1.8b, v26.8b, v27.8b
    ZIP2        v27.8b, v26.8b, v27.8b      //II
    mov         v26.8b, v1.8b

    //mov v26.d[1],v27.d[0]
    SUB         x5,x9,x8                    //II wd - col
    Uxtl        v28.8h, v30.8b              //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    SUB         x14,x10,x4                  //II ht - row

    mul         x14, x14, x1                //II (ht - row) * src_strd
    SADDW       v28.8h,  v28.8h ,  v26.8b   //II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    ADD         x5,x14,x5                   //II (ht - row) * src_strd + (wd - col)

    LDRH        w14,[x6,x5]                 //II pu1_src_org[(ht - row) * src_strd + 14 + (wd - col)]
    SMAX        v28.8h,  v28.8h ,  v4.8h    //II pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)

    STRH        w14,[x2],#2                 //II pu1_src_left[(ht - row)] = au1_src_left_tmp[(ht - row)]
    UMIN        v28.8h,  v28.8h ,  v6.8h    //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    //mov       v31.2d[0],v30.2d[1]
    Uxtl2       v30.8h, v30.16b             //II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vmovl_u8(vget_high_u8(pu1_cur_row)))

    SADDW       v30.8h,  v30.8h ,  v27.8b   //II pi2_tmp_cur_row.val[1] = vaddw_s8(pi2_tmp_cur_row.val[1], offset)
    ST1         {v21.8b},[x12],#8           //vst1q_u8(pu1_src_cpy, pu1_cur_row)
    ST1         {v23.8b},[x12],x1
    SUB         x12,x12,#8

    SMAX        v30.8h,  v30.8h ,  v4.8h    //II pi2_tmp_cur_row.val[1] = vmaxq_s16(pi2_tmp_cur_row.val[1], const_min_clip)
    SUBS        x4,x4,#1                    //Decrement row by 1
    UMIN        v30.8h,  v30.8h ,  v6.8h    //II pi2_tmp_cur_row.val[1] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[1]), const_max_clip))

    xtn         v28.8b,  v28.8h             //II vmovn_s16(pi2_tmp_cur_row.val[0])
    xtn         v29.8b,  v30.8h             //II vmovn_s16(pi2_tmp_cur_row.val[1])

    ST1         {v28.8b, v29.8b},[x12],x1   //II vst1q_u8(pu1_src_cpy, pu1_cur_row)

    BNE         PU1_SRC_LOOP                //If not equal jump to the inner loop

    ADD         x0,x0,#16                   //pu1_src += 16

    SUBS        x8,x8,#16                   //Decrement column by 16
    CMP         x8,#8                       //Check whether residue remains
    MOV         x2,x3                       //Reload pu1_src_left
    BEQ         WIDTH_RESIDUE               //If residue remains jump to residue loop
    BGT         WIDTH_LOOP_16               //If not equal jump to width_loop
    BLT         END_LOOPS                   //Jump to end function

WIDTH_RESIDUE:
    SUB         x6,x6,#14
    AND         x8,x9,#0xF                  //wd_rem = wd & 0xF
    CMP         x8,#0                       //Residue check
    BEQ         END_LOOPS                   //No Residue jump to end function

    CMP         x8,x9                       //if(wd_rem == wd)
    BNE         AU1_MASK_FF_RESIDUE         //jump to else part
    LDRB        w12,[x7]                    //pu1_avail[0]
    mov         v3.b[0], w12                //vsetq_lane_s8(pu1_avail[0], au1_mask, 0)
    mov         v3.b[1], w12                //vsetq_lane_s8(pu1_avail[0], au1_mask, 0)
    B           SKIP_AU1_MASK_FF_RESIDUE    //Skip the else part

AU1_MASK_FF_RESIDUE:
    MOV         x12,#-1                     //move -1 to x12
    mov         v3.h[0], w12                //au1_mask = vsetq_lane_s8(-1, au1_mask, 0)

SKIP_AU1_MASK_FF_RESIDUE:
    LDRB        w12,[x7,#1]                 //pu1_avail[1]
    mov         v3.b[6], w12                //au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)
    mov         v3.b[7], w12                //au1_mask = vsetq_lane_s8(pu1_avail[1], au1_mask, 15)

    MOV         x12,x0                      //pu1_src_cpy = pu1_src
    MOV         x4,x10                      //move ht to x4 for loop count

PU1_SRC_LOOP_RESIDUE:
    LDRH        w11,[x2]                    //load pu1_src_left
    LD1         {v19.16b},[x12],x1          //pu1_cur_row = vld1q_u8(pu1_src_cpy)
    //LD1 {v13.8b},[x12],x1                    //pu1_cur_row = vld1q_u8(pu1_src_cpy)
    //SUB x12, x12,#8
    SUB         x5,x9,#2                    //wd - 2

    SUB         x14,x10,x4                  //(ht - row)
    mov         v21.h[7], w11               //vsetq_lane_u8(pu1_src_left[ht - row], pu1_cur_row_tmp, 15)
    LSL         x14,x14,#1                  //(ht - row) * 2

    LD1         {v30.16b},[x12]             //II pu1_cur_row = vld1q_u8(pu1_src_cpy)
    //LD1 {v31.8b},[x12]                    //II pu1_cur_row = vld1q_u8(pu1_src_cpy)
    //SUB x12, x12,#8
    EXT         v21.16b,  v21.16b ,  v19.16b,#14 //pu1_cur_row_tmp = vextq_u8(pu1_cur_row_tmp, pu1_cur_row, 15)
    SUB         x12,x12,x1

    LDRH        w11,[x2,#2]                 //II load pu1_src_left
    cmhi        v16.16b,  v19.16b ,  v21.16b //vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
    mul         x14, x14, x1                //(ht - row) * 2 * src_strd

    cmhi        v18.16b,  v21.16b ,  v19.16b //vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    mov         v28.h[7], w11               //II vsetq_lane_u8(pu1_src_left[ht - row], pu1_cur_row_tmp, 15)

    LDRB        w11,[x12,#16]               //pu1_src_cpy[16]
    SUB         v20.16b,  v18.16b ,  v16.16b //sign_left = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    ADD         x5,x14,x5                   //(ht - row) * 2 * src_strd + (wd - 2)

    mov         v21.b[0], w11               //pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[16], pu1_cur_row_tmp, 0)
    EXT         v28.16b,  v28.16b ,  v30.16b,#14 //II pu1_cur_row_tmp = vextq_u8(pu1_cur_row_tmp, pu1_cur_row, 15)

    LDRB        w11,[x12,#17]               //pu1_src_cpy[17]
    cmhi        v26.16b,  v30.16b ,  v28.16b //II vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
    LDRH        w14,[x6, x5]                //pu1_src_org[(ht - row)  * 2* src_strd + (wd - 2)]

    mov         v21.b[1], w11               //pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[17], pu1_cur_row_tmp, 1)
    cmhi        v24.16b,  v28.16b ,  v30.16b //II vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    ADD         x12,x12,x1

    STRH        w14,[x2],#2                 //pu1_src_left[(ht - row) * 2] = au1_src_left_tmp[(ht - row) * 2]
    EXT         v21.16b,  v19.16b ,  v21.16b,#2 //pu1_cur_row_tmp = vextq_u8(pu1_cur_row, pu1_cur_row_tmp, 1)
    LDRB        w11,[x12,#16]               //II pu1_src_cpy[16]

    cmhi        v16.16b,  v19.16b ,  v21.16b //vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)
    mov         v28.b[0], w11               //II pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[16], pu1_cur_row_tmp, 0)

    LDRB        w11,[x12,#17]               //II pu1_src_cpy[17]
    cmhi        v18.16b,  v21.16b ,  v19.16b //vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    SUB         x4,x4,#1                    //II Decrement row by 1

    SUB         v22.16b,  v18.16b ,  v16.16b //sign_right = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    mov         v28.b[1], w11               //II pu1_cur_row_tmp = vsetq_lane_u8(pu1_src_cpy[17], pu1_cur_row_tmp, 1)
    SUB         x12,x12,x1

    ADD         v21.16b,  v2.16b ,  v20.16b //edge_idx = vaddq_s8(const_2, sign_left)
    EXT         v28.16b,  v30.16b ,  v28.16b,#2 //II pu1_cur_row_tmp = vextq_u8(pu1_cur_row, pu1_cur_row_tmp, 1)

    ADD         v21.16b,  v21.16b ,  v22.16b //edge_idx = vaddq_s8(edge_idx, sign_right)

    SUB         v20.16b,  v24.16b ,  v26.16b //II sign_left = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))
    TBL         v21.16b, {v5.16b},v21.16b   //vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    cmhi        v26.16b,  v30.16b ,  v28.16b //II vcgtq_u8(pu1_cur_row, pu1_cur_row_tmp)

    cmhi        v24.16b,  v28.16b ,  v30.16b //II vcltq_u8(pu1_cur_row, pu1_cur_row_tmp)
    //TBL v15.8b, {v10.16b},v15.8b                //vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    SUB         v22.16b,  v24.16b ,  v26.16b //II sign_right = vreinterpretq_s8_u8(vsubq_u8(cmp_lt, cmp_gt))

    AND         v21.16b,  v21.16b ,  v3.16b //edge_idx = vandq_s8(edge_idx, au1_mask)
    mov         v23.d[0],v21.d[1]
    UZP1        v1.8b, v21.8b, v23.8b
    UZP2        v23.8b, v21.8b, v23.8b
    mov         v21.8b, v1.8b

    ADD         v28.16b,  v2.16b ,  v20.16b //II edge_idx = vaddq_s8(const_2, sign_left)
    TBL         v16.8b, {v7.16b},v21.8b     //offset = vtbl1_s8(offset_tbl_u, vget_low_s8(edge_idx))
    ADD         v28.16b,  v28.16b ,  v22.16b //II edge_idx = vaddq_s8(edge_idx, sign_right)

    Uxtl        v18.8h, v19.8b              //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))
    TBL         v17.8b, {v0.16b},v23.8b
    Uxtl        v24.8h, v30.8b              //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vmovl_u8(vget_low_u8(pu1_cur_row)))

    ZIP1        v1.8b, v16.8b, v17.8b
    ZIP2        v17.8b, v16.8b, v17.8b
    mov         v16.8b, v1.8b
    TBL         v28.16b, {v5.16b},v28.16b   //II vtbl1_s8(edge_idx_tbl, vget_low_s8(edge_idx))
    SADDW       v18.8h,  v18.8h ,  v16.8b   //pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)

    SMAX        v18.8h,  v18.8h ,  v4.8h    //pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    //TBL v29.8b, {v10.16b},v29.8b                //II vtbl1_s8(edge_idx_tbl, vget_high_s8(edge_idx))
    UMIN        v18.8h,  v18.8h ,  v6.8h    //pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    xtn         v18.8b,  v18.8h             //vmovn_s16(pi2_tmp_cur_row.val[0])
    AND         v28.16b,  v28.16b ,  v3.16b //II edge_idx = vandq_s8(edge_idx, au1_mask)
    mov         v29.d[0],v28.d[1]
    SUB         x5,x9,#2                    //II wd - 2
    UZP1        v1.8b, v28.8b, v29.8b
    UZP2        v29.8b, v28.8b, v29.8b      //II
    mov         v28.8b, v1.8b
    SUB         x14,x10,x4                  //II (ht - row)

    LSL         x14,x14,#1                  //II (ht - row) * 2
    TBL         v26.8b, {v7.16b},v28.8b     //II offset = vtbl1_s8(offset_tbl_u, vget_low_s8(edge_idx))
    mul         x14, x14, x1                //II (ht - row) * 2 * src_strd

    ADD         x5,x14,x5                   //II (ht - row) * 2 * src_strd + (wd - 2)
    TBL         v27.8b, {v0.16b},v29.8b     //II
    LDRH        w14,[x6, x5]                //II pu1_src_org[(ht - row)  * 2* src_strd + (wd - 2)]

    ZIP1        v1.8b, v26.8b, v27.8b
    ZIP2        v27.8b, v26.8b, v27.8b      //II
    mov         v26.8b, v1.8b
    ST1         {v18.8b},[x12],x1           //vst1q_u8(pu1_src_cpy, pu1_cur_row)

    STRH        w14,[x2],#2                 //II pu1_src_left[(ht - row) * 2] = au1_src_left_tmp[(ht - row) * 2]
    SADDW       v24.8h,  v24.8h ,  v26.8b   //II pi2_tmp_cur_row.val[0] = vaddw_s8(pi2_tmp_cur_row.val[0], offset)
    SUBS        x4,x4,#1                    //Decrement row by 1

    SMAX        v24.8h,  v24.8h ,  v4.8h    //II pi2_tmp_cur_row.val[0] = vmaxq_s16(pi2_tmp_cur_row.val[0], const_min_clip)
    UMIN        v24.8h,  v24.8h ,  v6.8h    //II pi2_tmp_cur_row.val[0] = vreinterpretq_s16_u16(vminq_u16(vreinterpretq_u16_s16(pi2_tmp_cur_row.val[0]), const_max_clip))

    xtn         v28.8b,  v24.8h             //II vmovn_s16(pi2_tmp_cur_row.val[0])

    ST1         {v28.8b},[x12],x1           //II vst1q_u8(pu1_src_cpy, pu1_cur_row)

    BNE         PU1_SRC_LOOP_RESIDUE        //If not equal jump to the pu1_src loop

END_LOOPS:
    // LDMFD sp!,{x4-x12,x15}             //Reload the registers from SP
    ldp         x25, x26,[sp],#16
    ldp         x23, x24,[sp],#16
    ldp         x21, x22,[sp],#16
    ldp         x19, x20,[sp],#16

    ret





