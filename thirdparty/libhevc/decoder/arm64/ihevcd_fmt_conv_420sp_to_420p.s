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
///*******************************************************************************
//* //file
//*  ihevcd_fmt_conv_420sp_to_420p.s
//*
//* //brief
//*  contains function definitions for format conversions
//*
//* //author
//*  ittiam
//*
//* //par list of functions:
//*
//*
//* //remarks
//*  none
//*
//*******************************************************************************/

.text

.include "ihevc_neon_macros.s"




///*****************************************************************************
//*                                                                            *
//*  Function Name    : neon_copy_yuv420sp_to_yuv420p()                       *
//*                                                                            *
//*  Description      : This function conversts the image from YUV420sP color  *
//*                     space to 420SP color space(UV interleaved).                 *
//*                                                                            *
//*  Arguments        : x0           pu1_src_y                                 *
//*                     x1           pu1_src_uv                                *
//*                     x2           pu1_dest_y                                *
//*                     x3           pu1_dest_u                               *
//*                     [x13 #40]    pu1_dest_v                               *
//*                     [x13 #44]    u2_width                                 *
//*                     [x13 #48]    u2_height                                   *
//*                     [x13 #52]    u2_stridey                                *
//*                     [x13 #56]    u2_strideuv                               *
//*                     [x13 #60]    u2_dest_stridey                           *
//*                     [x13 #64]    u2_dest_strideuv                          *
//*                     [x13 #68]    is_u_first                                *
//*                     [x13 #72]    disable_luma_copy                         *
//*                                                                            *
//*  Values Returned  : None                                                   *
//*                                                                            *
//*  Register Usage   : x0 - x14                                               *
//*                                                                            *
//*  Stack Usage      : 40 Bytes                                               *
//*                                                                            *
//*  Interruptibility : Interruptible                                          *
//*                                                                            *
//*  Known Limitations                                                         *
//*       Assumptions: Image Width:     Assumed to be multiple of 2 and       *
//*                     Image Height:    Assumed to be even.                   *
//*                                                                            *
//*  Revision History :                                                        *
//*         DD MM YYYY   Author(s)       Changes (Describe the changes made)   *
//*         16 05 2012   Naveen SR     draft                                     *
//*                                                                            *
//*****************************************************************************/

.globl ihevcd_fmt_conv_420sp_to_420p_av8

.type ihevcd_fmt_conv_420sp_to_420p_av8, %function

ihevcd_fmt_conv_420sp_to_420p_av8:
    // STMFD sp!,{x4-x12, x14}
    push_v_regs
    stp         x19, x20,[sp,#-16]!
    mov         x15, x4
    mov         x8, x5                      ////Load u2_width
    mov         x9, x6                      ////Load u2_height

    LDR         w5, [sp,#88]                ////Load u2_dest_stridey
    sxtw        x5,w5
//    LDR        x6,[sp,#80]                @//Load u2_strideuv

    SUB         x10,x7,x8                   //// Src Y increment
    SUB         x11,x5,x8                   //// Dst Y increment

    LDR         w5, [sp,#112]               ////Load disable_luma_copy flag
    sxtw        x5,w5
    CMP         x5,#0                       ////skip luma if disable_luma_copy is non-zero
    BNE         uv_copy_start

    ///* Copy Y */

    MOV         x4,x9                       //// Copying height
y_row_loop:
    MOV         x6,x8                       //// Copying width

y_col_loop:

    SUB         x6,x6,#16
    ld1         {v0.8b, v1.8b},[x0],#16
    st1         {v0.8b, v1.8b},[x2],#16
    CMP         x6,#16
    BGE         y_col_loop
    CMP         x6,#0
    BEQ         y_col_loop_end
    ////If non-multiple of 16, then go back by few bytes to ensure 16 bytes can be read
    ////Ex if width is 162, above loop will process 160 pixels. And
    ////Both source and destination will point to 146th pixel and then 16 bytes will be read
    //// and written using VLD1 and VST1
    sub         x20,x6,#16
    neg         x6, x20
    SUB         x0,x0,x6
    SUB         x2,x2,x6
    ld1         {v0.8b, v1.8b}, [x0],#16
    st1         {v0.8b, v1.8b}, [x2],#16

y_col_loop_end:
    ADD         x0, x0, x10
    ADD         x2, x2, x11
    SUBS        x4, x4, #1
    BGT         y_row_loop


    ///* Copy UV */
uv_copy_start:

    LDR         w5, [sp,#96]                ////Load u2_dest_strideuv
    sxtw        x5,w5
    LDR         w7, [sp,#80]                ////Load u2_strideuv
    sxtw        x7,w7

    LSR         x9, x9, #1                  //// height/2
//    MOV     x8,x8,LSR #1            @// Width/2

    SUB         x10,x7,x8                   //// Src UV increment
    LSR         x11, x8, #1
    SUB         x11,x5,x11                  //// Dst U and V increment

    mov         x5, x15                     ////Load pu1_dest_v

    LDR         w4, [sp,#104]               ////Load is_u_first_flag
    sxtw        x4,w4
    CMP         x4,#0                       ////Swap U and V dest if is_u_first_flag is zero
    csel        x4, x5, x4,EQ
    csel        x5, x3, x5,EQ
    csel        x3, x4, x3,EQ

    MOV         x4,x9                       //// Copying height
uv_row_loop:
    MOV         x6,x8                       //// Copying width

uv_col_loop:

    SUB         x6,x6,#16

    prfm        PLDL1KEEP,[x1,#128]
    ld2         {v0.8b, v1.8b},[x1],#16
    ST1         {v0.8b},[x3],#8
    ST1         {v1.8b},[x5],#8
    CMP         x6,#16
    BGE         uv_col_loop
    CMP         x6,#0
    BEQ         uv_col_loop_end
    ////If non-multiple of 16, then go back by few bytes to ensure 16 bytes can be read
    ////Ex if width is 162, above loop will process 160 pixels. And
    ////Both source and destination will point to 146th pixel and then 16 bytes will be read
    //// and written using VLD1 and VST1
    sub         x20,x6,#16
    neg         x6, x20
    SUB         x1,x1,x6
    SUB         x3,x3,x6,LSR #1
    SUB         x5,x5,x6,LSR #1
    ld2         {v0.8b, v1.8b}, [x1],#16
    ST1         {v0.8b},[x3],#8
    ST1         {v1.8b},[x5],#8
uv_col_loop_end:
    ADD         x1, x1, x10
    ADD         x3, x3, x11
    ADD         x5, x5, x11
    SUBS        x4, x4, #1
    BGT         uv_row_loop

exit:
    // LDMFD sp!,{x4-x12, pc}
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret






