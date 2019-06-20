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
//*  ihevc_intra_pred_filters_dc.s
//*
//* @brief
//*  contains function definitions for intra prediction dc filtering.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* @author
//*  akshaya mukund
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
//* @param[in] pi1_coeff
//*  word8 pointer to the planar coefficients
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

//void ihevc_intra_pred_luma_dc(uword8 *pu1_ref,
//                              word32 src_strd,
//                              uword8 *pu1_dst,
//                              word32 dst_strd,
//                              word32 nt,
//                              word32 mode)
//
//**************variables vs registers*****************************************
//x0 => *pu1_ref
//x1 => src_strd
//x2 => *pu1_dst
//x3 => dst_strd

//stack contents from #40
//    nt
//    mode
//    pi1_coeff

.text
.align 4
.include "ihevc_neon_macros.s"


.globl ihevc_intra_pred_luma_dc_av8

.type ihevc_intra_pred_luma_dc_av8, %function

ihevc_intra_pred_luma_dc_av8:

    // stmfd sp!, {x4-x12, x14}            //stack stores the values of the arguments

    stp         x19, x20,[sp,#-16]!


//********** testing
    //mov        x6, #128
    //b        prologue_cpy_32
//********** testing

    mov         x11, #2                     //mov #2 to x11 (to be used to add to 2dc_val & 3dc_val)
    mov         x9, #0
    mov         v17.s[0], w11
    mov         v17.s[1], w9

    clz         w5,w4

    add         x6, x0, x4                  //&src[nt]
    sub         x20, x5, #32                //log2nt
    neg         x5, x20
    add         x7, x0, x4, lsl #1          //&src[2nt]

    add         x8, x7, #1                  //&src[2nt+1]
    mvn         x5, x5
    add         x5, x5, #1
    dup         v7.2s,w5

    ldrb        w14, [x8]
    sxtw        x14,w14
    shl         d7, d7,#32

    sub         x9, x7, #1                  //&src[2nt-1]
    sshr        d7, d7,#32

    mov         x7, x8                      //x7 also stores 2nt+1

    ldrb        w12, [x9]
    sxtw        x12,w12
    add         x14, x14, x12               //src[2nt+1] + src[2nt-1]
    add         x14, x14, x11               //src[2nt+1] + src[2nt-1] + 2

    cmp         x4, #4
    beq         dc_4

    mov         x10, x4                     //nt

add_loop:
    ld1         {v0.8b},[x6],#8             //load from src[nt]
    mov         x5, #0                      //
    ld1         {v1.8b},[x8],#8             //load from src[2nt+1]

    uaddlp      v2.4h,  v0.8b

    mov         v6.s[0], w4
    mov         v6.s[1], w5                 //store nt to accumulate
    uaddlp      v3.4h,  v1.8b

    ld1         {v0.8b},[x6],#8             //load from src[nt] (extra load for 8)

    ld1         {v1.8b},[x8],#8             //load from src[2nt+1] (extra load for 8)
    add         v4.4h,  v2.4h ,  v3.4h


    uaddlp      v5.2s,  v4.4h


    uadalp      v6.1d,  v5.2s               //accumulate all inp into d6 (end for nt==8)

    subs        x10, x10,#8
    beq         epil_add_loop

core_loop_add:
    uaddlp      v2.4h,  v0.8b
    subs        x10, x10,#8
    uaddlp      v3.4h,  v1.8b



    add         v4.4h,  v2.4h ,  v3.4h
    ld1         {v0.8b},[x6],#8             //load from src[nt] (extra load for 16)

    uaddlp      v5.2s,  v4.4h
    ld1         {v1.8b},[x8],#8             //load from src[2nt+1] (extra load for 16)

    uadalp      v6.1d,  v5.2s               //accumulate all inp into d6
    bne         core_loop_add

epil_add_loop:

    sshl        d18, d6, d7                 //(dc_val) shr by log2nt+1
    cmp         x4, #32

    mov         v28.s[0], w14
    mov         v28.s[1], w5                //src[2nt+1]+2+src[2nt-1] moved to d28
    mov         x20,#128
    csel        x6, x20, x6,eq

    dup         v16.8b, v18.b[0]            //dc_val
    shl         d25, d18,#1                 //2*dc

    beq         prologue_cpy_32

    add         d27,  d25 ,  d28            //src[2nt+1]+2+src[2nt-1]+2dc_val
    mov         x20,#0
    csel        x6, x20, x6,ne              //nt

    ushr        v29.4h, v27.4h,#2           //final dst[0]'s value in d15[0]
    csel        x10, x4, x10,ne

    add         d23,  d25 ,  d18            //3*dc
    sub         x12, x3, x3, lsl #3         //-7*strd

    add         d23,  d23 ,  d17            //3*dc + 2
    add         x12, x12, #8                //offset after one 8x8 block (-7*strd + 8)

    dup         v24.8h, v23.h[0]            //3*dc + 2 (moved to all lanes)
    sub         x0, x3, x4                  //strd - nt

prologue_col:
    //0th column and 0-7 rows done here
    //x8 and x9 (2nt+1+col 2nt-1-row)

    mov         x8, x7                      //&src[2nt+1]

    add         x0, x0, #8                  //strd - nt + 8
    ld1         {v0.8b},[x8],#8             //col 1::7 load (prol)
    sub         x9, x9, #7                  //&src[2nt-1-row]

    ld1         {v1.8b},[x9]                //row 7::1 (0 also) load (prol)
    sub         x9, x9, #8

    uxtl        v20.8h, v0.8b

    ld1         {v6.8b},[x8]                //col 8::15 load (prol extra)
    add         v20.8h,  v20.8h ,  v24.8h   //col 1::7 add 3dc+2 (prol)

    uxtl        v22.8h, v1.8b
    sqshrun     v2.8b, v20.8h,#2            //columns shx2 movn (prol)

    uxtl        v26.8h, v6.8b
    add         v22.8h,  v22.8h ,  v24.8h   //row 1::7 add 3dc+2 (prol)

    movi        d19, #0x00000000000000ff    //
    sqshrun     v3.8b, v22.8h,#2            //rows shx2 movn (prol)

    bsl         v19.8b,  v29.8b ,  v2.8b    //first row with dst[0]
    add         v26.8h,  v26.8h ,  v24.8h   //col 8::15 add 3dc+2 (prol extra)

    rev64       v3.8b,  v3.8b

    st1         {v19.8b},[x2], x3           //store row 0 (prol)
    sshr        d3, d3,#8                   //row 0 shift (prol) (first value to be ignored)

    movi        d20, #0x00000000000000ff    //byte mask row 1 (prol)

loop_again_col_row:

    bsl         v20.8b,  v3.8b ,  v16.8b    //row 1    (prol)

    movi        d21, #0x00000000000000ff    //byte mask row 2 (prol)
    sshr        d3, d3,#8                   //row 1 shift (prol)

    st1         {v20.8b},[x2], x3           //store row 1 (prol)
    sqshrun     v4.8b, v26.8h,#2            //columns shx2 movn (prol extra)


    bsl         v21.8b,  v3.8b ,  v16.8b    //row 2 (prol)

    movi        d20, #0x00000000000000ff    //byte mask row 3 (prol)
    sshr        d3, d3,#8                   //row 2 shift (prol)

    st1         {v21.8b},[x2], x3           //store row 2 (prol)


    bsl         v20.8b,  v3.8b ,  v16.8b    //row 3    (prol)

    movi        d21, #0x00000000000000ff    //byte mask row 4 (prol)
    sshr        d3, d3,#8                   //row 3 shift (prol)

    st1         {v20.8b},[x2], x3           //store row 3 (prol)


    bsl         v21.8b,  v3.8b ,  v16.8b    //row 4 (prol)

    movi        d20, #0x00000000000000ff    //byte mask row 5 (prol)
    sshr        d3, d3,#8                   //row 4 shift (prol)

    st1         {v21.8b},[x2], x3           //store row 4 (prol)


    bsl         v20.8b,  v3.8b ,  v16.8b    //row 5 (prol)

    movi        d21, #0x00000000000000ff    //byte mask row 6 (prol)
    sshr        d3, d3,#8                   //row 5 shift (prol)

    st1         {v20.8b},[x2], x3           //store row 5 (prol)

    ld1         {v1.8b},[x9]                //row 8::15 load (prol extra)

    bsl         v21.8b,  v3.8b ,  v16.8b    //row 6 (prol)

    uxtl        v22.8h, v1.8b

    movi        d20, #0x00000000000000ff    //byte mask row 7 (prol)
    sshr        d3, d3,#8                   //row 6 shift (prol)

    st1         {v21.8b},[x2], x3           //store row 6 (prol)

    bsl         v20.8b,  v3.8b ,  v16.8b    //row 7 (prol)
    add         v22.8h,  v22.8h ,  v24.8h   //row 8::15 add 3dc+2 (prol extra)

    sshr        d3, d3,#8                   //row 7 shift (prol)
    st1         {v20.8b},[x2], x12          //store row 7 (prol)

    subs        x10, x10, #8                //counter for cols

    beq         end_func
    blt         copy_16


    movi        d20, #0x00000000000000ff    //byte mask row 9 (prol)
    sqshrun     v3.8b, v22.8h,#2            //rows shx2 movn (prol)

    rev64       v3.8b,  v3.8b

    st1         {v4.8b},[x2], x3            //store 2nd col (for 16x16)

    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x0           //go to next row for 16


    bsl         v20.8b,  v3.8b ,  v16.8b    //row 9    (prol)
    subs        x10, x10, #8

    st1         {v20.8b},[x2], x3           //store row 9 (prol)
    sshr        d3, d3,#8                   //row 9 shift (prol)

    movi        d20, #0x00000000000000ff    //byte mask row 9 (prol)

    b           loop_again_col_row


copy_16:
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2], x3
    st1         {v16.8b},[x2]

    b           end_func

prologue_cpy_32:
    mov         x9, #128
    //sub        x7, x3, #-24
    add         x5, x2, x3
    add         x8, x5, x3
    add         x10, x8, x3
    dup         v20.16b, v16.b[0]
    lsl         x6, x3, #2
    sub         x6, x6, #16

    st1         {v20.16b}, [x2],#16
    st1         {v20.16b}, [x5],#16
    st1         {v20.16b}, [x8],#16
    st1         {v20.16b}, [x10],#16

    st1         {v20.16b}, [x2], x6
    st1         {v20.16b}, [x5], x6
    st1         {v20.16b}, [x8], x6
    st1         {v20.16b}, [x10], x6

    sub         x9, x9, #32                 //32x32 prol/epil counter dec

kernel_copy:
    st1         {v20.16b}, [x2],#16
    st1         {v20.16b}, [x5],#16
    st1         {v20.16b}, [x8],#16
    st1         {v20.16b}, [x10],#16

    st1         {v20.16b}, [x2], x6
    st1         {v20.16b}, [x5], x6
    st1         {v20.16b}, [x8], x6
    st1         {v20.16b}, [x10], x6

    subs        x9, x9, #32

    st1         {v20.16b}, [x2],#16
    st1         {v20.16b}, [x5],#16
    st1         {v20.16b}, [x8],#16
    st1         {v20.16b}, [x10],#16

    st1         {v20.16b}, [x2], x6
    st1         {v20.16b}, [x5], x6
    st1         {v20.16b}, [x8], x6
    st1         {v20.16b}, [x10], x6

    bne         kernel_copy

epilogue_copy:
    st1         {v20.16b}, [x2],#16
    st1         {v20.16b}, [x5],#16
    st1         {v20.16b}, [x8],#16
    st1         {v20.16b}, [x10],#16

    st1         {v20.16b}, [x2]
    st1         {v20.16b}, [x5]
    st1         {v20.16b}, [x8]
    st1         {v20.16b}, [x10]

    b           end_func


dc_4:
    ld1         {v0.8b},[x6],#8             //load from src[nt]
    ld1         {v1.8b},[x8],#8             //load from src[2nt+1]

    uaddlp      v2.4h,  v0.8b
    mov         x5, #0                      //
    mov         v6.s[0], w4
    mov         v6.s[1], w5                 //store nt to accumulate
    uaddlp      v3.4h,  v1.8b

    add         v4.4h,  v2.4h ,  v3.4h


    uaddlp      v5.2s,  v4.4h
    movi        d30, #0x00000000ffffffff

    and         v5.8b,  v5.8b ,  v30.8b

    mov         v28.s[0], w14
    mov         v28.s[1], w5                //src[2nt+1]+2+src[2nt-1] moved to d28
    add         d6,  d6 ,  d5               //accumulate all inp into d6 (end for nt==8)

    sshl        d18, d6, d7                 //(dc_val) shr by log2nt+1
    mov         x8, x7                      //&src[2nt+1]

    shl         d25, d18,#1                 //2*dc
    sub         x9, x9, #3                  //&src[2nt-1-row]

    dup         v16.8b, v18.b[0]            //dc_val
    add         d27,  d25 ,  d28            //src[2nt+1]+2+src[2nt-1]+2dc_val

    ushr        v29.4h, v27.4h,#2           //final dst[0]'s value in d15[0]
    sub         x12, x3, x3, lsl #2         //-3*strd
    add         d23,  d25 ,  d18            //3*dc

    add         d23,  d23 ,  d17            //3*dc + 2
    add         x12, x12, #4                //offset after one 4x4 block (-3*strd + 4)

    dup         v24.8h, v23.h[0]            //3*dc + 2 (moved to all lanes)
    sub         x0, x3, x4                  //strd - nt


    ld1         {v0.8b},[x8]                //col 1::3 load (prol)
    ld1         {v1.8b},[x9]                //row 3::1 (0 also) load (prol)

    uxtl        v20.8h, v0.8b

    uxtl        v22.8h, v1.8b
    add         v20.8h,  v20.8h ,  v24.8h   //col 1::7 add 3dc+2 (prol)

    add         v22.8h,  v22.8h ,  v24.8h   //row 1::7 add 3dc+2 (prol)

    movi        d19, #0x00000000000000ff    //
    sqshrun     v2.8b, v20.8h,#2            //columns shx2 movn (prol)

    movi        d20, #0x00000000000000ff    //byte mask row 1 (prol)
    sqshrun     v3.8b, v22.8h,#2            //rows shx2 movn (prol)


    bsl         v19.8b,  v29.8b ,  v2.8b    //first row with dst[0]

    rev64       v3.8b,  v3.8b

    st1         {v19.s}[0],[x2], x3         //store row 0 (prol)
    sshr        d3, d3,#40                  //row 0 shift (prol) (first value to be ignored)

    movi        d21, #0x00000000000000ff    //byte mask row 2 (prol)

    bsl         v20.8b,  v3.8b ,  v16.8b    //row 1    (prol)
    sshr        d3, d3,#8                   //row 1 shift (prol)

    st1         {v20.s}[0],[x2], x3         //store row 1 (prol)

    bsl         v21.8b,  v3.8b ,  v16.8b    //row 2 (prol)

    movi        d20, #0x00000000000000ff    //byte mask row 3 (prol)

    sshr        d3, d3,#8                   //row 2 shift (prol)
    st1         {v21.s}[0],[x2], x3         //store row 2 (prol)

    bsl         v20.8b,  v3.8b ,  v16.8b    //row 3    (prol)
    st1         {v20.s}[0],[x2]             //store row 3 (prol)

epilogue_end:
end_func:
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16

    ret





