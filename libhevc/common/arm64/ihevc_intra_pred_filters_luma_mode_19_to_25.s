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
//* @file
//*  ihevc_intra_pred_luma_mode_19_to_25.s
//*
//* @brief
//*  contains function definitions for intra prediction dc filtering.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* @author
//*  naveen sr
//*
//* @par list of functions:
//*
//*
//* @remarks
//*  none
//*
//*******************************************************************************
//*/
///**
//*******************************************************************************
//*
//* @brief
//*    luma intraprediction filter for dc input
//*
//* @par description:
//*
//* @param[in] pu1_ref
//*  uword8 pointer to the source
//*
//* @param[out] pu1_dst
//*  uword8 pointer to the destination
//*
//* @param[in] src_strd
//*  integer source stride
//*
//* @param[in] dst_strd
//*  integer destination stride
//*
//* @param[in] nt
//*  size of tranform block
//*
//* @param[in] mode
//*  type of filtering
//*
//* @returns
//*
//* @remarks
//*  none
//*
//*******************************************************************************
//*/

//void ihevc_intra_pred_luma_mode_19_to_25(uword8* pu1_ref,
//                               word32 src_strd,
//                               uword8* pu1_dst,
//                               word32 dst_strd,
//                               word32 nt,
//                               word32 mode)
//
//**************variables vs registers*****************************************
//x0 => *pu1_ref
//x1 => src_strd
//x2 => *pu1_dst
//x3 => dst_strd

//stack contents from #40
//    nt
//    mode

.text
.align 4
.include "ihevc_neon_macros.s"



.globl ihevc_intra_pred_luma_mode_19_to_25_av8
.extern gai4_ihevc_ang_table
.extern gai4_ihevc_inv_ang_table
.extern gau1_ihevc_planar_factor

.type ihevc_intra_pred_luma_mode_19_to_25_av8, %function

ihevc_intra_pred_luma_mode_19_to_25_av8:

    // stmfd sp!, {x4-x12, x14}            //stack stores the values of the arguments

    stp         d9,d10,[sp,#-16]!
    stp         d12,d13,[sp,#-16]!
    stp         d14,d15,[sp,#-16]!
    stp         x19, x20,[sp,#-16]!

    adrp        x7,  :got:gai4_ihevc_ang_table
    ldr         x7,  [x7, #:got_lo12:gai4_ihevc_ang_table]

    adrp        x8,  :got:gai4_ihevc_inv_ang_table
    ldr         x8,  [x8, #:got_lo12:gai4_ihevc_inv_ang_table]

    add         x7, x7, x5, lsl #2          //gai4_ihevc_ang_table[mode]
    add         x8, x8, x5, lsl #2          //gai4_ihevc_inv_ang_table
    sub         x8, x8, #48                 //gai4_ihevc_inv_ang_table[mode - 12]

    ldr         w7,  [x7]                   //intra_pred_ang
    sxtw        x7,w7
    sub         sp, sp, #132                //ref_temp[2 * max_cu_size + 1]

    ldr         w8,  [x8]                   //inv_ang
    sxtw        x8,w8
    add         x6, sp, x4                  //ref_temp + nt

    mul         x9, x4, x7                  //nt*intra_pred_ang

    sub         x6, x6, #1                  //ref_temp + nt - 1

    add         x1, x0, x4, lsl #1          //x1 = &src[2nt]
    dup         v30.8b,w7                   //intra_pred_ang

    mov         x7, x4

    asr         x9, x9, #5

    ld1         {v0.s}[0],[x1],#4           // pu1_ref[two_nt + k]

    st1         {v0.s}[0],[x6],#4           //ref_temp[k + nt - 1] = pu1_ref[two_nt + k]//

    subs        x7, x7, #4
    beq         end_loop_copy
    sub         x1, x1,#4
    sub         x6, x6,#4
    subs        x7,x7,#4
    beq         loop_copy_8
    subs        x7,x7,#8
    beq         loop_copy_16

loop_copy_32:
    ld1         {v0.8b},[x1],#8
    ld1         {v1.8b},[x1],#8
    ld1         {v2.8b},[x1],#8
    ld1         {v3.8b},[x1],#8

    st1         {v0.8b},[x6],#8
    st1         {v1.8b},[x6],#8
    st1         {v2.8b},[x6],#8
    st1         {v3.8b},[x6],#8
    b           end_loop_copy

loop_copy_16:
    ld1         {v0.8b},[x1],#8
    ld1         {v1.8b},[x1],#8

    st1         {v0.8b},[x6],#8
    st1         {v1.8b},[x6],#8
    b           end_loop_copy

loop_copy_8:
    ld1         {v0.8b},[x1],#8
    st1         {v0.8b},[x6],#8

end_loop_copy:

    ldrb        w11, [x1]
    sxtw        x11,w11
    strb        w11, [x6]
    sxtw        x11,w11

    cmn         x9, #1
    bge         linear_filtering

    add         x6, sp, x4                  //ref_temp + nt
    sub         x6, x6, #2                  //ref_temp + nt - 2

    mov         x12, #-1

    sub         x20, x9, x12                //count to take care off ref_idx
    neg         x9, x20

    add         x1, x0, x4, lsl #1          //x1 = &src[2nt]

    mov         x7, #128                    //inv_ang_sum

loop_copy_ref_idx:

    add         x7, x7, x8                  //inv_ang_sum += inv_ang
    lsr         x14, x7, #8
    neg         x20,x14
    ldrb        w11, [x1, x20]
    sxtw        x11,w11
//    ldrb        x11, [x1, -x7, lsr #8]
    strb        w11, [x6], #-1
    sxtw        x11,w11

    subs        x9, x9, #1

    bne         loop_copy_ref_idx


linear_filtering:
//    after copy
//    below code is taken from mode 27 to 33 and modified

    adrp        x6,  :got:gai4_ihevc_ang_table //loads word32 gai4_ihevc_ang_table[35]
    ldr         x6,  [x6, #:got_lo12:gai4_ihevc_ang_table]

    add         x8,x6,x5,lsl #2             //*gai4_ihevc_ang_table[mode]
    ldr         w9, [x8]                    //intra_pred_ang = gai4_ihevc_ang_table[mode]
    sxtw        x9,w9
    adrp        x1, :got:gau1_ihevc_planar_factor //used for ((row + 1) * intra_pred_ang) row values
    ldr         x1, [x1, #:got_lo12:gau1_ihevc_planar_factor]
    add         x6,x1,#1

    add         x8, sp, x4                  //ref_temp + nt
    sub         x8, x8,#1                   //ref_temp + nt -1

    tst         x4,#7
    mov         x14,#0                      //row
    mov         x12,x4
    bne         core_loop_4

core_loop_8:
    add         x8,x8,#1                    //pu1_ref_main_idx += (two_nt + 1)
    dup         v0.8b,w9                    //intra_pred_ang
    lsr         x12, x4, #3                 //divide by 8

    movi        v1.8b, #32
    mul         x7, x4, x12

    movi        v6.8h, #31
    //lsl            x12,x3,#3

    mov         x1,x8
    //sub            x12,x12,x4
    mov         x5,x4
    mov         x11,#1

prologue:
    ld1         {v3.8b},[x6]                //loads the row value
    smull       v2.8h, v3.8b, v0.8b         //pos = ((row + 1) * intra_pred_ang)
    and         v4.16b,  v2.16b ,  v6.16b   //dup_const_fract(fract = pos & (31))
    xtn         v4.8b,  v4.8h
    shrn        v5.8b, v2.8h,#5             //idx = pos >> 5

    dup         v31.8b, v4.b[0]
    add         x0,x2,x3

    umov        w14, v5.s[0]                //(i row)extract idx to the r register
    sxtw        x14,w14

    dup         v29.8b, v4.b[1]             //(ii)
    sbfx        x9,x14,#0,#8

    add         x10,x8,x9                   //(i row)*pu1_ref[ref_main_idx]

    ld1         {v23.8b},[x10],x11          //(i row)ref_main_idx
    sbfx        x9,x14,#8,#8

    ld1         {v9.8b},[x10]               //(i row)ref_main_idx_1
    add         x12,x8,x9                   //(ii)*pu1_ref[ref_main_idx]

    sbfx        x9,x14,#16,#8
    sub         v30.8b,  v1.8b ,  v31.8b    //32-fract(dup_const_32_fract)
    add         x10,x8,x9                   //(iii)*pu1_ref[ref_main_idx]

    ld1         {v12.8b},[x12],x11          //(ii)ref_main_idx
    umull       v10.8h, v23.8b, v30.8b      //(i row)vmull_u8(ref_main_idx, dup_const_32_fract)

    ld1         {v13.8b},[x12]              //(ii)ref_main_idx_1
    umlal       v10.8h, v9.8b, v31.8b       //(i row)vmull_u8(ref_main_idx_1, dup_const_fract)

    dup         v27.8b, v4.b[2]             //(iii)
    sub         v28.8b,  v1.8b ,  v29.8b    //(ii)32-fract(dup_const_32_fract)
    sbfx        x9,x14,#24,#8

    dup         v25.8b, v4.b[3]             //(iv)
    umull       v14.8h, v12.8b, v28.8b      //(ii)vmull_u8(ref_main_idx, dup_const_32_fract)
    add         x12,x8,x9                   //(iv)*pu1_ref[ref_main_idx]

    ld1         {v16.8b},[x10],x11          //(iii)ref_main_idx
    umlal       v14.8h, v13.8b, v29.8b      //(ii)vmull_u8(ref_main_idx_1, dup_const_fract)

    ld1         {v17.8b},[x10]              //(iii)ref_main_idx_1
    rshrn       v10.8b, v10.8h,#5           //(i row)shift_res = vrshrn_n_u16(add_res, 5)

    ld1         {v20.8b},[x12],x11          //(iv)ref_main_idx
    sub         v26.8b,  v1.8b ,  v27.8b    //(iii)32-fract(dup_const_32_fract)

    ld1         {v21.8b},[x12]              //(iv)ref_main_idx_1

    dup         v31.8b, v4.b[4]             //(v)
    umull       v18.8h, v16.8b, v26.8b      //(iii)vmull_u8(ref_main_idx, dup_const_32_fract)

    umov        w14, v5.s[1]                //extract idx to the r register
    sxtw        x14,w14
    umlal       v18.8h, v17.8b, v27.8b      //(iii)vmull_u8(ref_main_idx_1, dup_const_fract)

    st1         {v10.8b},[x2],#8            //(i row)
    rshrn       v14.8b, v14.8h,#5           //(ii)shift_res = vrshrn_n_u16(add_res, 5)

    sbfx        x9,x14,#0,#8
    dup         v29.8b, v4.b[5]             //(vi)
    add         x10,x8,x9                   //(v)*pu1_ref[ref_main_idx]

    ld1         {v23.8b},[x10],x11          //(v)ref_main_idx
    sub         v24.8b,  v1.8b ,  v25.8b    //(iv)32-fract(dup_const_32_fract)

    umull       v22.8h, v20.8b, v24.8b      //(iv)vmull_u8(ref_main_idx, dup_const_32_fract)
    sbfx        x9,x14,#8,#8

    ld1         {v9.8b},[x10]               //(v)ref_main_idx_1
    umlal       v22.8h, v21.8b, v25.8b      //(iv)vmull_u8(ref_main_idx_1, dup_const_fract)

    st1         {v14.8b},[x0],x3            //(ii)
    rshrn       v18.8b, v18.8h,#5           //(iii)shift_res = vrshrn_n_u16(add_res, 5)

    add         x12,x8,x9                   //(vi)*pu1_ref[ref_main_idx]
    dup         v27.8b, v4.b[6]             //(vii)

    sbfx        x9,x14,#16,#8
    sub         v30.8b,  v1.8b ,  v31.8b    //(v)32-fract(dup_const_32_fract)
    add         x10,x8,x9                   //(vii)*pu1_ref[ref_main_idx]

    ld1         {v12.8b},[x12],x11          //(vi)ref_main_idx
    umull       v10.8h, v23.8b, v30.8b      //(v)vmull_u8(ref_main_idx, dup_const_32_fract)

    ld1         {v13.8b},[x12]              //(vi)ref_main_idx_1
    umlal       v10.8h, v9.8b, v31.8b       //(v)vmull_u8(ref_main_idx_1, dup_const_fract)

    st1         {v18.8b},[x0],x3            //(iii)
    rshrn       v22.8b, v22.8h,#5           //(iv)shift_res = vrshrn_n_u16(add_res, 5)

    dup         v25.8b, v4.b[7]             //(viii)
    sbfx        x9,x14,#24,#8

    ld1         {v16.8b},[x10],x11          //(vii)ref_main_idx
    sub         v28.8b,  v1.8b ,  v29.8b    //(vi)32-fract(dup_const_32_fract)

    ld1         {v17.8b},[x10]              //(vii)ref_main_idx_1
    umull       v14.8h, v12.8b, v28.8b      //(vi)vmull_u8(ref_main_idx, dup_const_32_fract)

    add         x12,x8,x9                   //(viii)*pu1_ref[ref_main_idx]
    umlal       v14.8h, v13.8b, v29.8b      //(vi)vmull_u8(ref_main_idx_1, dup_const_fract)
    subs        x4,x4,#8

    st1         {v22.8b},[x0],x3            //(iv)
    rshrn       v10.8b, v10.8h,#5           //(v)shift_res = vrshrn_n_u16(add_res, 5)

    ld1         {v20.8b},[x12],x11          //(viii)ref_main_idx
    sub         v26.8b,  v1.8b ,  v27.8b    //(vii)32-fract(dup_const_32_fract)

    ld1         {v21.8b},[x12]              //(viii)ref_main_idx_1
    umull       v18.8h, v16.8b, v26.8b      //(vii)vmull_u8(ref_main_idx, dup_const_32_fract)

    add         x20,x8,#8
    csel        x8, x20, x8,gt
    umlal       v18.8h, v17.8b, v27.8b      //(vii)vmull_u8(ref_main_idx_1, dup_const_fract)
    sub         x20,x7,#8
    csel        x7, x20, x7,gt

    st1         {v10.8b},[x0],x3            //(v)
    rshrn       v14.8b, v14.8h,#5           //(vi)shift_res = vrshrn_n_u16(add_res, 5)

    beq         epilogue

    ld1         {v5.8b},[x6]                //loads the row value
    smull       v2.8h, v5.8b, v0.8b         //pos = ((row + 1) * intra_pred_ang)
    and         v4.16b,  v2.16b ,  v6.16b   //dup_const_fract(fract = pos & (31))
    xtn         v4.8b,  v4.8h
    shrn        v3.8b, v2.8h,#5             //idx = pos >> 5
    umov        w14, v3.s[0]                //(i)extract idx to the r register
    sxtw        x14,w14
    sbfx        x9,x14,#0,#8
    add         x10,x8,x9                   //(i)*pu1_ref[ref_main_idx]

kernel_8_rows:
    dup         v31.8b, v4.b[0]
    subs        x4,x4,#8
    sbfx        x9,x14,#8,#8

    ld1         {v23.8b},[x10],x11          //(i)ref_main_idx
    sub         v24.8b,  v1.8b ,  v25.8b    //(viii)32-fract(dup_const_32_fract)

    add         x20,x6,#8                   //increment the row value
    csel        x6, x20, x6,le
    add         x12,x8,x9                   //(ii)*pu1_ref[ref_main_idx]

    ld1         {v9.8b},[x10]               //(i)ref_main_idx_1
    umull       v22.8h, v20.8b, v24.8b      //(viii)vmull_u8(ref_main_idx, dup_const_32_fract)

    ld1         {v5.8b},[x6]                //loads the row value
    umlal       v22.8h, v21.8b, v25.8b      //(viii)vmull_u8(ref_main_idx_1, dup_const_fract)

    dup         v29.8b, v4.b[1]             //(ii)
    rshrn       v18.8b, v18.8h,#5           //(vii)shift_res = vrshrn_n_u16(add_res, 5)

    sbfx        x9,x14,#16,#8

    st1         {v14.8b},[x0],x3            //(vi)
    sub         v30.8b,  v1.8b ,  v31.8b    //(i)32-fract(dup_const_32_fract)

    add         x10,x8,x9                   //(iii)*pu1_ref[ref_main_idx]

    ld1         {v12.8b},[x12],x11          //(ii)ref_main_idx
    umull       v10.8h, v23.8b, v30.8b      //(i)vmull_u8(ref_main_idx, dup_const_32_fract)

    ld1         {v13.8b},[x12]              //(ii)ref_main_idx_1
    umlal       v10.8h, v9.8b, v31.8b       //(i)vmull_u8(ref_main_idx_1, dup_const_fract)

    sbfx        x9,x14,#24,#8
    csel        x4, x5, x4,le               //reload nt

    umov        w14, v3.s[1]                //extract idx to the r register
    sxtw        x14,w14
    rshrn       v22.8b, v22.8h,#5           //(viii)shift_res = vrshrn_n_u16(add_res, 5)

    dup         v27.8b, v4.b[2]             //(iii)
    sub         v28.8b,  v1.8b ,  v29.8b    //(ii)32-fract(dup_const_32_fract)
    add         x12,x8,x9                   //(iv)*pu1_ref[ref_main_idx]

    ld1         {v16.8b},[x10],x11          //(iii)ref_main_idx
    umull       v14.8h, v12.8b, v28.8b      //(ii)vmull_u8(ref_main_idx, dup_const_32_fract)

    st1         {v18.8b},[x0],x3            //(vii)
    umlal       v14.8h, v13.8b, v29.8b      //(ii)vmull_u8(ref_main_idx_1, dup_const_fract)

    ld1         {v17.8b},[x10]              //(iii)ref_main_idx_1
    rshrn       v10.8b, v10.8h,#5           //(i)shift_res = vrshrn_n_u16(add_res, 5)

    dup         v25.8b, v4.b[3]             //(iv)
    smull       v2.8h, v5.8b, v0.8b         //pos = ((row + 1) * intra_pred_ang)

    st1         {v22.8b},[x0]               //(viii)
    sub         v26.8b,  v1.8b ,  v27.8b    //(iii)32-fract(dup_const_32_fract)

    ld1         {v20.8b},[x12],x11          //(iv)ref_main_idx
    umull       v18.8h, v16.8b, v26.8b      //(iii)vmull_u8(ref_main_idx, dup_const_32_fract)

    ld1         {v21.8b},[x12]              //(iv)ref_main_idx_1
    umlal       v18.8h, v17.8b, v27.8b      //(iii)vmull_u8(ref_main_idx_1, dup_const_fract)

    sbfx        x9,x14,#0,#8
    add         x0,x2,x3

    dup         v31.8b, v4.b[4]             //(v)
    rshrn       v14.8b, v14.8h,#5           //(ii)shift_res = vrshrn_n_u16(add_res, 5)

    add         x10,x8,x9                   //(v)*pu1_ref[ref_main_idx]
    sbfx        x9,x14,#8,#8

    st1         {v10.8b},[x2],#8            //(i)
    sub         v24.8b,  v1.8b ,  v25.8b    //(iv)32-fract(dup_const_32_fract)

    dup         v29.8b, v4.b[5]             //(vi)
    umull       v22.8h, v20.8b, v24.8b      //(iv)vmull_u8(ref_main_idx, dup_const_32_fract)

    dup         v27.8b, v4.b[6]             //(vii)
    umlal       v22.8h, v21.8b, v25.8b      //(iv)vmull_u8(ref_main_idx_1, dup_const_fract)

    add         x12,x8,x9                   //(vi)*pu1_ref[ref_main_idx]
    sbfx        x9,x14,#16,#8

    dup         v25.8b, v4.b[7]             //(viii)
    rshrn       v18.8b, v18.8h,#5           //(iii)shift_res = vrshrn_n_u16(add_res, 5)

    ld1         {v23.8b},[x10],x11          //(v)ref_main_idx
    and         v4.16b,  v2.16b ,  v6.16b   //dup_const_fract(fract = pos & (31))

    ld1         {v9.8b},[x10]               //(v)ref_main_idx_1
    shrn        v3.8b, v2.8h,#5             //idx = pos >> 5

    st1         {v14.8b},[x0],x3            //(ii)
    rshrn       v22.8b, v22.8h,#5           //(iv)shift_res = vrshrn_n_u16(add_res, 5)

    add         x10,x8,x9                   //(vii)*pu1_ref[ref_main_idx]
    sbfx        x9,x14,#24,#8

    ld1         {v12.8b},[x12],x11          //(vi)ref_main_idx
    sub         v30.8b,  v1.8b ,  v31.8b    //(v)32-fract(dup_const_32_fract)

    ld1         {v13.8b},[x12]              //(vi)ref_main_idx_1
    umull       v10.8h, v23.8b, v30.8b      //(v)vmull_u8(ref_main_idx, dup_const_32_fract)

    umov        w14, v3.s[0]                //(i)extract idx to the r register
    sxtw        x14,w14
    umlal       v10.8h, v9.8b, v31.8b       //(v)vmull_u8(ref_main_idx_1, dup_const_fract)

    add         x12,x8,x9                   //(viii)*pu1_ref[ref_main_idx]
    csel        x8, x1, x8,le               //reload the source to pu1_src+2nt

    ld1         {v16.8b},[x10],x11          //(vii)ref_main_idx
    sub         v28.8b,  v1.8b ,  v29.8b    //(vi)32-fract(dup_const_32_fract)

    st1         {v18.8b},[x0],x3            //(iii)
    umull       v14.8h, v12.8b, v28.8b      //(vi)vmull_u8(ref_main_idx, dup_const_32_fract)

    ld1         {v17.8b},[x10]              //(vii)ref_main_idx_1
    umlal       v14.8h, v13.8b, v29.8b      //(vi)vmull_u8(ref_main_idx_1, dup_const_fract)

    ld1         {v20.8b},[x12],x11          //(viii)ref_main_idx
    rshrn       v10.8b, v10.8h,#5           //(v)shift_res = vrshrn_n_u16(add_res, 5)

    ld1         {v21.8b},[x12]              //(viii)ref_main_idx_1
    sub         v26.8b,  v1.8b ,  v27.8b    //(vii)32-fract(dup_const_32_fract)

    add         x20,x8,#8                   //increment the source next set 8 columns in same row
    csel        x8, x20, x8,gt
    lsl         x20, x3,#3
    csel        x12,x20,x12,le
    sub         x20,x12,x5
    csel        x12, x20, x12,le

    st1         {v22.8b},[x0],x3            //(iv)
    umull       v18.8h, v16.8b, v26.8b      //(vii)vmull_u8(ref_main_idx, dup_const_32_fract)

    st1         {v10.8b},[x0],x3            //(v)
    umlal       v18.8h, v17.8b, v27.8b      //(vii)vmull_u8(ref_main_idx_1, dup_const_fract)

    add         x20,x2,x12                  //increment the dst pointer to 8*dst_strd - nt
    csel        x2, x20, x2,le
    sbfx        x9,x14,#0,#8

    xtn         v4.8b,  v4.8h
    rshrn       v14.8b, v14.8h,#5           //(vi)shift_res = vrshrn_n_u16(add_res, 5)

    subs        x7,x7,#8
    add         x10,x8,x9                   //(i)*pu1_ref[ref_main_idx]

    bne         kernel_8_rows

epilogue:
    st1         {v14.8b},[x0],x3            //(vi)
    rshrn       v18.8b, v18.8h,#5           //(vii)shift_res = vrshrn_n_u16(add_res, 5)

    sub         v24.8b,  v1.8b ,  v25.8b    //(viii)32-fract(dup_const_32_fract)
    umull       v22.8h, v20.8b, v24.8b      //(viii)vmull_u8(ref_main_idx, dup_const_32_fract)
    umlal       v22.8h, v21.8b, v25.8b      //(viii)vmull_u8(ref_main_idx_1, dup_const_fract)

    st1         {v18.8b},[x0],x3            //(vii)
    rshrn       v22.8b, v22.8h,#5           //(viii)shift_res = vrshrn_n_u16(add_res, 5)

    st1         {v22.8b},[x0],x3            //(viii)
    b           end_loops

core_loop_4:
    add         x6,x8,#1                    //pu1_ref_main_idx +=  1
    mov         x8,#0

    add         x5,x8,#1                    //row + 1
    mul         x5, x5, x9                  //pos = ((row + 1) * intra_pred_ang)
    asr         x14, x5, #5                 //if(fract_prev > fract)
    and         x5,x5,#31                   //fract = pos & (31)
    add         x10,x6,x14                  //pu1_ref_main_idx += 1
    add         x11,x10,#1                  //pu1_ref_main_idx_1 += 1
    dup         v0.8b,w5                    //dup_const_fract
    sub         x20,x5,#32
    neg         x4, x20
    dup         v1.8b,w4                    //dup_const_32_fract

//inner_loop_4
    ld1         {v2.s}[0],[x10]             //ref_main_idx
    add         x8,x8,#1
//    mov            x14,x5                            @fract_prev = fract

    ld1         {v3.s}[0],[x11]             //ref_main_idx_1
    add         x5,x8,#1                    //row + 1
    mul         x5, x5, x9                  //pos = ((row + 1) * intra_pred_ang)
    asr         x14, x5, #5                 // pos >> 5
    and         x5,x5,#31                   //fract = pos & (31)
    add         x10,x6,x14                  //pu1_ref_main_idx += 1
    add         x11,x10,#1                  //pu1_ref_main_idx_1 += 1

    dup         v6.8b,w5                    //dup_const_fract
    umull       v4.8h, v2.8b, v1.8b         //vmull_u8(ref_main_idx, dup_const_32_fract)

    sub         x20,x5,#32
    neg         x4, x20
    dup         v7.8b,w4                    //dup_const_32_fract
    umlal       v4.8h, v3.8b, v0.8b         //vmull_u8(ref_main_idx_1, dup_const_fract)

    ld1         {v23.s}[0],[x10]            //ref_main_idx
    add         x8,x8,#1

    ld1         {v9.s}[0],[x11]             //ref_main_idx_1
    rshrn       v4.8b, v4.8h,#5             //shift_res = vrshrn_n_u16(add_res, 5)

//    mov            x14,x5                            @fract_prev = fract
    add         x5,x8,#1                    //row + 1
    mul         x5, x5, x9                  //pos = ((row + 1) * intra_pred_ang)
    asr         x14, x5, #5                 //if(fract_prev > fract)
    and         x5,x5,#31                   //fract = pos & (31)
    add         x10,x6,x14                  //ref_main + idx
    add         x11,x10,#1                  //pu1_ref_main_idx_1 += 1

    dup         v12.8b,w5                   //dup_const_fract
    umull       v10.8h, v23.8b, v7.8b       //vmull_u8(ref_main_idx, dup_const_32_fract)

    sub         x20,x5,#32
    neg         x4, x20
    dup         v13.8b,w4                   //dup_const_32_fract
    umlal       v10.8h, v9.8b, v6.8b        //vmull_u8(ref_main_idx_1, dup_const_fract)

    ld1         {v14.s}[0],[x10]            //ref_main_idx
    add         x8,x8,#1

    st1         {v4.s}[0],[x2],x3
    rshrn       v10.8b, v10.8h,#5           //shift_res = vrshrn_n_u16(add_res, 5)

    ld1         {v15.s}[0],[x11]            //ref_main_idx_1
//    mov            x14,x5                            @fract_prev = fract
    add         x5,x8,#1                    //row + 1
    mul         x5, x5, x9                  //pos = ((row + 1) * intra_pred_ang)
    asr         x14, x5, #5                 //if(fract_prev > fract)
    and         x5,x5,#31                   //fract = pos & (31)
    add         x10,x6,x14                  //pu1_ref_main_idx += 1
    add         x11,x10,#1                  //pu1_ref_main_idx_1 += 1

    dup         v18.8b,w5                   //dup_const_fract
    umull       v16.8h, v14.8b, v13.8b      //vmull_u8(ref_main_idx, dup_const_32_fract)

    sub         x20,x5,#32
    neg         x4, x20
    dup         v19.8b,w4                   //dup_const_32_fract
    umlal       v16.8h, v15.8b, v12.8b      //vmull_u8(ref_main_idx_1, dup_const_fract)

    ld1         {v20.s}[0],[x10]            //ref_main_idx

    st1         {v10.s}[0],[x2],x3
    rshrn       v16.8b, v16.8h,#5           //shift_res = vrshrn_n_u16(add_res, 5)
    ld1         {v21.s}[0],[x11]            //ref_main_idx_1

    umull       v22.8h, v20.8b, v19.8b      //vmull_u8(ref_main_idx, dup_const_32_fract)
    umlal       v22.8h, v21.8b, v18.8b      //vmull_u8(ref_main_idx_1, dup_const_fract)

    st1         {v16.s}[0],[x2],x3
    rshrn       v22.8b, v22.8h,#5           //shift_res = vrshrn_n_u16(add_res, 5)

    st1         {v22.s}[0],[x2],x3

end_loops:
    add         sp, sp, #132
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16
    ldp         d14,d15,[sp],#16
    ldp         d12,d13,[sp],#16
    ldp         d9,d10,[sp],#16
    ret






