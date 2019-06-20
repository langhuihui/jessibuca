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
//*  ihevcd_fmt_conv_420sp_to_rgba8888.s
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

    .equ DO1STROUNDING, 0

    // ARM
    //
    // PRESERVE8

.text
.p2align 2

.include "ihevc_neon_macros.s"



///*****************************************************************************
//*                                                                            *
//*  Function Name    : ihevcd_fmt_conv_420sp_to_rgba8888()                    *
//*                                                                            *
//*  Description      : This function conversts the image from YUV422 color    *
//*                     space to RGB888 color space. The function can be       *
//*                     invoked at the MB level.                               *
//*                                                                            *
//*  Arguments        : x0           pubY                                      *
//*                     x1           pubUV                                     *
//*                     x2           pusRGB                                    *
//*                     x3           pusRGB                                    *
//*                     [x13 #40]    usHeight                                  *
//*                     [x13 #44]    usWidth                                   *
//*                     [x13 #48]    usStrideY                                 *
//*                     [x13 #52]    usStrideU                                 *
//*                     [x13 #56]    usStrideV                                 *
//*                     [x13 #60]    usStrideRGB                               *
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
//*       Assumptions: Image Width:     Assumed to be multiple of 16 and       *
//*                     greater than or equal to 16                  *
//*                     Image Height:    Assumed to be even.                   *
//*                                                                            *
//*  Revision History :                                                        *
//*         DD MM YYYY   Author(s)       Changes (Describe the changes made)   *
//*         07 06 2010   Varshita        Draft                                 *
//*         07 06 2010   Naveen Kr T     Completed                             *
//*         05 08 2013   Naveen K P      Modified for HEVC                     *
//*****************************************************************************/
    .global ihevcd_fmt_conv_420sp_to_rgba8888_av8
.type ihevcd_fmt_conv_420sp_to_rgba8888_av8, function
ihevcd_fmt_conv_420sp_to_rgba8888_av8:

    //// push the registers on the stack
    // STMFD sp!,{x4-x12,x14}

    stp         d12,d14,[sp,#-16]!
    stp         d8,d15,[sp,#-16]!           // Storing d15 using { sub sp,sp,#8; str d15,[sp] } is giving bus error.
                                            // d8 is used as dummy register and stored along with d15 using stp. d8 is not used in the function.
    stp         x19, x20,[sp,#-16]!


    ////x0 - Y PTR
    ////x1 - UV PTR
    ////x2 - RGB PTR
    ////x3 - RGB PTR
    ////x4 - PIC WIDTH
    ////x5 - PIC HT
    ////x6 - STRIDE Y
    ////x7 - STRIDE U
    ////x8 - STRIDE V
    ////x9 - STRIDE RGB

    ////ONE ROW PROCESSING AT A TIME

    ////THE FOUR CONSTANTS ARE:
    ////C1=0x3311,C2=0xF379,C3=0xE5F8,C4=0x4092

    //PLD        [x0]
    //PLD        [x1]
    //PLD        [x2]


    ///* can be loaded from a defined const type */
    mov         x10,#0x3311
    mov         v0.h[0], w10               ////C1

    mov         x10,#0xF379
    mov         v0.h[1], w10               ////C2

    mov         x10,#0xE5F8
    mov         v0.h[2], w10               ////C3

    mov         x10,#0x4092
    mov         v0.h[3], w10               ////C4

    ////LOAD CONSTANT 128 INTO A CORTEX REGISTER
    MOV         x10,#128
    dup         v1.8b,w10

    ////D0 HAS C1-C2-C3-C4
    //// load other parameters from stack
    mov         x9, x7
    mov         x7, x6
    mov         x6, x5
    mov         x5, x4
    //LDR  x4,[sp,#44]
    //LDR  x8,[sp,#52]

    //// calculate offsets, offset = stride - width
    SUB         x10,x6,x3                   //// luma offset
    SUB         x11,x7,x3
    //, LSR #1    @// u offset
    //SUB     x12,x8,x3, LSR #1    @// v offset
    SUB         x14,x9,x3                   //// rgb offset in pixels

    //// calculate height loop count
    LSR         x5, x5, #1                  //// height_cnt = height / 16

    //// create next row pointers for rgb and luma data
    ADD         x7,x0,x6                    //// luma_next_row = luma + luma_stride
    ADD         x8,x2,x9,LSL #2             //// rgb_next_row = rgb + rgb_stride

LABEL_YUV420SP_TO_RGB8888_HEIGHT_LOOP:

    ////LOAD VALUES OF U&V AND COMPUTE THE R,G,B WEIGHT VALUES.
    LD1         {v2.8b, v3.8b},[x1],#16     ////LOAD 8 VALUES OF UV
    ////VLD1.8 {D3},[x2]!             @//LOAD 8 VALUES OF V

    //// calculate width loop count
    LSR         x6, x3, #4                  //// width_cnt = width / 16

    ////COMPUTE THE ACTUAL RGB VALUES,WE CAN DO TWO ROWS AT A TIME
    ////LOAD VALUES OF Y 8-BIT VALUES
    LD2         {v30.8b, v31.8b},[x0],#16   ////D0 - Y0,Y2,Y4,Y6,Y8,Y10,Y12,Y14 row 1
                                            ////D1 - Y1,Y3,Y5,Y7,Y9,Y11,Y13,Y15
    LD2         {v28.8b, v29.8b},[x7],#16   ////D0 - Y0,Y2,Y4,Y6,Y8,Y10,Y12,Y14 row2
                                            ////D1 - Y1,Y3,Y5,Y7,Y9,Y11,Y13,Y15

    SUBS        x6,x6,#1
    BEQ         LABEL_YUV420SP_TO_RGB8888_WIDTH_LOOP_SKIP

LABEL_YUV420SP_TO_RGB8888_WIDTH_LOOP:
    //VMOV.I8 Q1,#128
    UZP1        v27.8b, v2.8b, v3.8b
    UZP2        v3.8b, v2.8b, v3.8b
    mov         v2.d[0], v27.d[0]

    ////NEED TO SUBTRACT (U-128) AND (V-128)
    ////(D2-D1),(D3-D1)
    uSUBL       v4.8h, v2.8b, v1.8b         ////(U-128)
    uSUBL       v6.8h, v3.8b, v1.8b         ////(V-128)

    ////LOAD VALUES OF U&V for next row
    LD1         {v2.8b, v3.8b},[x1],#16     ////LOAD 8 VALUES OF U
    ////VLD1.8 {D3},[x2]!             @//LOAD 8 VALUES OF V

    //PLD        [x0]
    prfm        PLDL1KEEP,[x1]

    ////NEED TO MULTIPLY WITH Q2,Q3 WITH CO-EEFICIENTS
    sMULL       v5.4s, v4.4h, v0.h[3]      ////(U-128)*C4 FOR B
    sMULL2      v7.4s, v4.8h, v0.h[3]      ////(U-128)*C4 FOR B

    sMULL       v20.4s, v6.4h, v0.h[0]     ////(V-128)*C1 FOR R
    sMULL2      v22.4s, v6.8h, v0.h[0]     ////(V-128)*C1 FOR R

    sMULL       v12.4s, v4.4h, v0.h[1]     ////(U-128)*C2 FOR G
    sMLAL       v12.4s, v6.4h, v0.h[2]     ////Q6 = (U-128)*C2 + (V-128)*C3
    sMULL2      v14.4s, v4.8h, v0.h[1]     ////(U-128)*C2 FOR G
    sMLAL2      v14.4s, v6.8h, v0.h[2]     ////Q7 = (U-128)*C2 + (V-128)*C3

    ////NARROW RIGHT SHIFT BY 13 FOR R&B
    sqshrn      v5.4h, v5.4s,#13            ////D8 = (U-128)*C4>>13 4 16-BIT VALUES
    sqshrn2     v5.8h, v7.4s,#13            ////D9 = (U-128)*C4>>13 4 16-BIT VALUES
    ////Q4 - WEIGHT FOR B

    ////NARROW RIGHT SHIFT BY 13 FOR R&B
    sqshrn      v7.4h, v20.4s,#13           ////D10 = (V-128)*C1>>13 4 16-BIT VALUES
    sqshrn2     v7.8h, v22.4s,#13           ////D11 = (V-128)*C1>>13 4 16-BIT VALUES
    ////Q5 - WEIGHT FOR R

    ////NARROW RIGHT SHIFT BY 13 FOR G
    sqshrn      v12.4h, v12.4s,#13          ////D12 = [(U-128)*C2 + (V-128)*C3]>>13 4 16-BIT VALUES
    sqshrn2     v12.8h, v14.4s,#13          ////D13 = [(U-128)*C2 + (V-128)*C3]>>13 4 16-BIT VALUES
    ////Q6 - WEIGHT FOR G

    UADDW       v14.8h,  v5.8h ,  v30.8b    ////Q7 - HAS Y + B
    UADDW       v16.8h,  v7.8h ,  v30.8b    ////Q8 - HAS Y + R
    UADDW       v18.8h,  v12.8h ,  v30.8b   ////Q9 - HAS Y + G

    UADDW       v20.8h,  v5.8h ,  v31.8b    ////Q10 - HAS Y + B
    UADDW       v22.8h,  v7.8h ,  v31.8b    ////Q11 - HAS Y + R
    UADDW       v24.8h,  v12.8h ,  v31.8b   ////Q12 - HAS Y + G

    sqxtun      v14.8b, v14.8h
    sqxtun      v15.8b, v18.8h
    sqxtun      v16.8b, v16.8h
    movi        v17.8b, #0

    sqxtun      v20.8b, v20.8h
    sqxtun      v21.8b, v24.8h
    sqxtun      v22.8b, v22.8h
    movi        v23.8b, #0

    ZIP1        v27.8b, v14.8b, v15.8b
    ZIP2        v15.8b, v14.8b, v15.8b
    mov         v14.d[0], v27.d[0]
    ZIP1        v27.8b, v16.8b, v17.8b
    ZIP2        v17.8b, v16.8b, v17.8b
    mov         v16.d[0], v27.d[0]

    ZIP1        v27.8b, v20.8b, v21.8b
    ZIP2        v21.8b, v20.8b, v21.8b
    mov         v20.d[0], v27.d[0]
    ZIP1        v27.8b, v22.8b, v23.8b
    ZIP2        v23.8b, v22.8b, v23.8b
    mov         v22.d[0], v27.d[0]

    mov         v14.d[1], v15.d[0]
    mov         v20.d[1], v21.d[0]
    mov         v16.d[1], v17.d[0]
    mov         v22.d[1], v23.d[0]

    ZIP1        v27.8h, v14.8h, v16.8h
    ZIP2        v26.8h, v14.8h, v16.8h

    ZIP1        v25.8h, v20.8h, v22.8h
    ZIP2        v19.8h, v20.8h, v22.8h

    ZIP1        v14.4s, v27.4s, v25.4s
    ZIP2        v20.4s, v27.4s, v25.4s

    ZIP1        v16.4s, v26.4s, v19.4s
    ZIP2        v22.4s, v26.4s, v19.4s

    ST1         {v14.4s},[x2],#16
    ST1         {v20.4s},[x2],#16
    ST1         {v16.4s},[x2],#16
    ST1         {v22.4s},[x2],#16

    ////D14-D20 - TOALLY HAVE 16 VALUES
    ////WE NEED TO SHIFT R,G,B VALUES TO GET 5BIT,6BIT AND 5BIT COMBINATIONS
    UADDW       v14.8h,  v5.8h ,  v28.8b    ////Q7 - HAS Y + B
    UADDW       v16.8h,  v7.8h ,  v28.8b    ////Q2 - HAS Y + R
    UADDW       v18.8h,  v12.8h ,  v28.8b   ////Q3 - HAS Y + G

    UADDW       v20.8h,  v5.8h ,  v29.8b    ////Q10 - HAS Y + B
    UADDW       v22.8h,  v7.8h ,  v29.8b    ////Q11 - HAS Y + R
    UADDW       v24.8h,  v12.8h ,  v29.8b   ////Q12 - HAS Y + G

    ////COMPUTE THE ACTUAL RGB VALUES,WE CAN DO TWO ROWS AT A TIME
    ////LOAD VALUES OF Y 8-BIT VALUES
    LD2         {v30.8b, v31.8b},[x0],#16   ////D0 - Y0,Y2,Y4,Y6,Y8,Y10,Y12,Y14 row 1
                                            ////D1 - Y1,Y3,Y5,Y7,Y9,Y11,Y13,Y15
    LD2         {v28.8b, v29.8b},[x7],#16   ////D0 - Y0,Y2,Y4,Y6,Y8,Y10,Y12,Y14 row2
                                            ////D1 - Y1,Y3,Y5,Y7,Y9,Y11,Y13,Y15

    prfm        PLDL1KEEP,[x0]
    prfm        PLDL1KEEP,[x7]

    sqxtun      v14.8b, v14.8h
    sqxtun      v15.8b, v18.8h
    sqxtun      v16.8b, v16.8h
    movi        v17.8b, #0

    sqxtun      v20.8b, v20.8h
    sqxtun      v21.8b, v24.8h
    sqxtun      v22.8b, v22.8h
    movi        v23.8b, #0

    ZIP1        v27.8b, v14.8b, v15.8b
    ZIP2        v15.8b, v14.8b, v15.8b
    mov         v14.d[0], v27.d[0]
    ZIP1        v27.8b, v16.8b, v17.8b
    ZIP2        v17.8b, v16.8b, v17.8b
    mov         v16.d[0], v27.d[0]

    ZIP1        v27.8b, v20.8b, v21.8b
    ZIP2        v21.8b, v20.8b, v21.8b
    mov         v20.d[0], v27.d[0]
    ZIP1        v27.8b, v22.8b, v23.8b
    ZIP2        v23.8b, v22.8b, v23.8b
    mov         v22.d[0], v27.d[0]

    mov         v14.d[1], v15.d[0]
    mov         v20.d[1], v21.d[0]
    mov         v16.d[1], v17.d[0]
    mov         v22.d[1], v23.d[0]

    ZIP1        v27.8h, v14.8h, v16.8h
    ZIP2        v26.8h, v14.8h, v16.8h

    ZIP1        v25.8h, v20.8h, v22.8h
    ZIP2        v19.8h, v20.8h, v22.8h

    ZIP1        v14.4s, v27.4s, v25.4s
    ZIP2        v20.4s, v27.4s, v25.4s

    ZIP1        v16.4s, v26.4s, v19.4s
    ZIP2        v22.4s, v26.4s, v19.4s

    ST1         {v14.4s},[x8],#16
    ST1         {v20.4s},[x8],#16
    ST1         {v16.4s},[x8],#16
    ST1         {v22.4s},[x8],#16

    SUBS        x6,x6,#1                    //// width_cnt -= 1
    BNE         LABEL_YUV420SP_TO_RGB8888_WIDTH_LOOP

LABEL_YUV420SP_TO_RGB8888_WIDTH_LOOP_SKIP:
    //VMOV.I8 Q1,#128
    UZP1        v27.8b, v2.8b, v3.8b
    UZP2        v3.8b, v2.8b, v3.8b
    mov         v2.d[0], v27.d[0]


    ////NEED TO SUBTRACT (U-128) AND (V-128)
    ////(D2-D1),(D3-D1)
    uSUBL       v4.8h, v2.8b, v1.8b         ////(U-128)
    uSUBL       v6.8h, v3.8b, v1.8b         ////(V-128)


    ////NEED TO MULTIPLY WITH Q2,Q3 WITH CO-EEFICIENTS
    sMULL       v5.4s, v4.4h, v0.h[3]      ////(U-128)*C4 FOR B
    sMULL2      v7.4s, v4.8h, v0.h[3]      ////(U-128)*C4 FOR B

    sMULL       v20.4s, v6.4h, v0.h[0]     ////(V-128)*C1 FOR R
    sMULL2      v22.4s, v6.8h, v0.h[0]     ////(V-128)*C1 FOR R

    sMULL       v12.4s, v4.4h, v0.h[1]     ////(U-128)*C2 FOR G
    sMLAL       v12.4s, v6.4h, v0.h[2]     ////Q6 = (U-128)*C2 + (V-128)*C3
    sMULL2      v14.4s, v4.8h, v0.h[1]     ////(U-128)*C2 FOR G
    sMLAL2      v14.4s, v6.8h, v0.h[2]     ////Q7 = (U-128)*C2 + (V-128)*C3

    ////NARROW RIGHT SHIFT BY 13 FOR R&B
    sqshrn      v5.4h, v5.4s,#13            ////D8 = (U-128)*C4>>13 4 16-BIT VALUES
    sqshrn2     v5.8h, v7.4s,#13            ////D9 = (U-128)*C4>>13 4 16-BIT VALUES
    ////Q4 - WEIGHT FOR B

    ////NARROW RIGHT SHIFT BY 13 FOR R&B
    sqshrn      v7.4h, v20.4s,#13           ////D10 = (V-128)*C1>>13 4 16-BIT VALUES
    sqshrn2     v7.8h, v22.4s,#13           ////D11 = (V-128)*C1>>13 4 16-BIT VALUES
    ////Q5 - WEIGHT FOR R

    ////NARROW RIGHT SHIFT BY 13 FOR G
    sqshrn      v12.4h, v12.4s,#13          ////D12 = [(U-128)*C2 + (V-128)*C3]>>13 4 16-BIT VALUES
    sqshrn2     v12.8h, v14.4s,#13          ////D13 = [(U-128)*C2 + (V-128)*C3]>>13 4 16-BIT VALUES
    ////Q6 - WEIGHT FOR G

    UADDW       v14.8h,  v5.8h ,  v30.8b    ////Q7 - HAS Y + B
    UADDW       v16.8h,  v7.8h ,  v30.8b    ////Q8 - HAS Y + R
    UADDW       v18.8h,  v12.8h ,  v30.8b   ////Q9 - HAS Y + G

    UADDW       v20.8h,  v5.8h ,  v31.8b    ////Q10 - HAS Y + B
    UADDW       v22.8h,  v7.8h ,  v31.8b    ////Q11 - HAS Y + R
    UADDW       v24.8h,  v12.8h ,  v31.8b   ////Q12 - HAS Y + G

    sqxtun      v14.8b, v14.8h
    sqxtun      v15.8b, v18.8h
    sqxtun      v16.8b, v16.8h
    movi        v17.8b, #0

    sqxtun      v20.8b, v20.8h
    sqxtun      v21.8b, v24.8h
    sqxtun      v22.8b, v22.8h
    movi        v23.8b, #0

    ZIP1        v27.8b, v14.8b, v15.8b
    ZIP2        v15.8b, v14.8b, v15.8b
    mov         v14.d[0], v27.d[0]
    ZIP1        v27.8b, v16.8b, v17.8b
    ZIP2        v17.8b, v16.8b, v17.8b
    mov         v16.d[0], v27.d[0]

    ZIP1        v27.8b, v20.8b, v21.8b
    ZIP2        v21.8b, v20.8b, v21.8b
    mov         v20.d[0], v27.d[0]
    ZIP1        v27.8b, v22.8b, v23.8b
    ZIP2        v23.8b, v22.8b, v23.8b
    mov         v22.d[0], v27.d[0]

    mov         v14.d[1], v15.d[0]
    mov         v20.d[1], v21.d[0]
    mov         v16.d[1], v17.d[0]
    mov         v22.d[1], v23.d[0]

    ZIP1        v27.8h, v14.8h, v16.8h
    ZIP2        v26.8h, v14.8h, v16.8h

    ZIP1        v25.8h, v20.8h, v22.8h
    ZIP2        v19.8h, v20.8h, v22.8h

    ZIP1        v14.4s, v27.4s, v25.4s
    ZIP2        v20.4s, v27.4s, v25.4s

    ZIP1        v16.4s, v26.4s, v19.4s
    ZIP2        v22.4s, v26.4s, v19.4s

    ST1         {v14.4s},[x2],#16
    ST1         {v20.4s},[x2],#16
    ST1         {v16.4s},[x2],#16
    ST1         {v22.4s},[x2],#16

    ////D14-D20 - TOALLY HAVE 16 VALUES
    ////WE NEED TO SHIFT R,G,B VALUES TO GET 5BIT,6BIT AND 5BIT COMBINATIONS
    UADDW       v14.8h,  v5.8h ,  v28.8b    ////Q7 - HAS Y + B
    UADDW       v16.8h,  v7.8h ,  v28.8b    ////Q2 - HAS Y + R
    UADDW       v18.8h,  v12.8h ,  v28.8b   ////Q3 - HAS Y + G

    UADDW       v20.8h,  v5.8h ,  v29.8b    ////Q10 - HAS Y + B
    UADDW       v22.8h,  v7.8h ,  v29.8b    ////Q11 - HAS Y + R
    UADDW       v24.8h,  v12.8h ,  v29.8b   ////Q12 - HAS Y + G

    sqxtun      v14.8b, v14.8h
    sqxtun      v15.8b, v18.8h
    sqxtun      v16.8b, v16.8h
    movi        v17.8b, #0

    sqxtun      v20.8b, v20.8h
    sqxtun      v21.8b, v24.8h
    sqxtun      v22.8b, v22.8h
    movi        v23.8b, #0

    ZIP1        v27.8b, v14.8b, v15.8b
    ZIP2        v15.8b, v14.8b, v15.8b
    mov         v14.d[0], v27.d[0]
    ZIP1        v27.8b, v16.8b, v17.8b
    ZIP2        v17.8b, v16.8b, v17.8b
    mov         v16.d[0], v27.d[0]

    ZIP1        v27.8b, v20.8b, v21.8b
    ZIP2        v21.8b, v20.8b, v21.8b
    mov         v20.d[0], v27.d[0]
    ZIP1        v27.8b, v22.8b, v23.8b
    ZIP2        v23.8b, v22.8b, v23.8b
    mov         v22.d[0], v27.d[0]

    mov         v14.d[1], v15.d[0]
    mov         v20.d[1], v21.d[0]
    mov         v16.d[1], v17.d[0]
    mov         v22.d[1], v23.d[0]

    ZIP1        v27.8h, v14.8h, v16.8h
    ZIP2        v26.8h, v14.8h, v16.8h

    ZIP1        v25.8h, v20.8h, v22.8h
    ZIP2        v19.8h, v20.8h, v22.8h

    ZIP1        v14.4s, v27.4s, v25.4s
    ZIP2        v20.4s, v27.4s, v25.4s

    ZIP1        v16.4s, v26.4s, v19.4s
    ZIP2        v22.4s, v26.4s, v19.4s

    ST1         {v14.4s},[x8],#16
    ST1         {v20.4s},[x8],#16
    ST1         {v16.4s},[x8],#16
    ST1         {v22.4s},[x8],#16

    //// Adjust the address pointers
    ADD         x0,x7,x10                   //// luma = luma_next + offset
    ADD         x2,x8,x14,LSL #2            //// rgb = rgb_next + offset

    ADD         x7,x0,x3                    //// luma_next = luma + width
    ADD         x8,x2,x3,LSL #2             //// rgb_next_row = rgb + width

    ADD         x1,x1,x11                   //// adjust u pointer
    //ADD        x2,x2,x12            @// adjust v pointer

    ADD         x7,x7,x10                   //// luma_next = luma + width + offset (because of register crunch)
    ADD         x8,x8,x14,LSL #2            //// rgb_next_row = rgb + width + offset

    SUBS        x5,x5,#1                    //// height_cnt -= 1

    BNE         LABEL_YUV420SP_TO_RGB8888_HEIGHT_LOOP

    ////POP THE REGISTERS
    // LDMFD sp!,{x4-x12,PC}
    ldp         x19, x20,[sp],#16
    ldp         d8,d15,[sp],#16             // Loading d15 using { ldr d15,[sp]; add sp,sp,#8 } is giving bus error.
                                            // d8 is used as dummy register and loaded along with d15 using ldp. d8 is not used in the function.
    ldp         d12,d14,[sp],#16
    ret




    .section .note.GNU-stack,"",%progbits

