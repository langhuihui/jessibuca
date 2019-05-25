@/*****************************************************************************
@*
@* Copyright (C) 2012 Ittiam Systems Pvt Ltd, Bangalore
@*
@* Licensed under the Apache License, Version 2.0 (the "License");
@* you may not use this file except in compliance with the License.
@* You may obtain a copy of the License at:
@*
@* http://www.apache.org/licenses/LICENSE-2.0
@*
@* Unless required by applicable law or agreed to in writing, software
@* distributed under the License is distributed on an "AS IS" BASIS,
@* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@* See the License for the specific language governing permissions and
@* limitations under the License.
@*
@*****************************************************************************/
@/**
@/*******************************************************************************
@* @file
@*  ihevcd_fmt_conv_420sp_to_rgba8888.s
@*
@* @brief
@*  contains function definitions for format conversions
@*
@* @author
@*  ittiam
@*
@* @par list of functions:
@*
@*
@* @remarks
@*  none
@*
@*******************************************************************************/
    .equ DO1STROUNDING, 0

    @ ARM
    @
    @ PRESERVE8

.text
.p2align 2




@/*****************************************************************************
@*                                                                            *
@*  Function Name    : ihevcd_fmt_conv_420sp_to_rgba8888()                    *
@*                                                                            *
@*  Description      : This function conversts the image from YUV422 color    *
@*                     space to RGB888 color space. The function can be       *
@*                     invoked at the MB level.                               *
@*                                                                            *
@*  Arguments        : R0           pubY                                      *
@*                     R1           pubUV                                     *
@*                     R2           pusRGB                                    *
@*                     R3           pusRGB                                    *
@*                     [R13 #40]    usHeight                                  *
@*                     [R13 #44]    usWidth                                   *
@*                     [R13 #48]    usStrideY                                 *
@*                     [R13 #52]    usStrideU                                 *
@*                     [R13 #56]    usStrideV                                 *
@*                     [R13 #60]    usStrideRGB                               *
@*                                                                            *
@*  Values Returned  : None                                                   *
@*                                                                            *
@*  Register Usage   : R0 - R14                                               *
@*                                                                            *
@*  Stack Usage      : 40 Bytes                                               *
@*                                                                            *
@*  Interruptibility : Interruptible                                          *
@*                                                                            *
@*  Known Limitations                                                         *
@*       Assumptions: Image Width:     Assumed to be multiple of 16 and       *
@*                     greater than or equal to 16                *
@*                     Image Height:    Assumed to be even.                   *
@*                                                                            *
@*  Revision History :                                                        *
@*         DD MM YYYY   Author(s)       Changes (Describe the changes made)   *
@*         07 06 2010   Varshita        Draft                                 *
@*         07 06 2010   Naveen Kr T     Completed                             *
@*         05 08 2013   Naveen K P      Modified for HEVC                     *
@*****************************************************************************/
    .global ihevcd_fmt_conv_420sp_to_rgba8888_a9q
.type ihevcd_fmt_conv_420sp_to_rgba8888_a9q, function
ihevcd_fmt_conv_420sp_to_rgba8888_a9q:

    @// push the registers on the stack
    STMFD       SP!,{R4-R12,LR}


    @//R0 - Y PTR
    @//R1 - UV PTR
    @//R2 - RGB PTR
    @//R3 - RGB PTR
    @//R4 - PIC WIDTH
    @//R5 - PIC HT
    @//R6 - STRIDE Y
    @//R7 - STRIDE U
    @//R8 - STRIDE V
    @//R9 - STRIDE RGB

    @//ONE ROW PROCESSING AT A TIME

    @//THE FOUR CONSTANTS ARE:
    @//C1=0x3311,C2=0xF379,C3=0xE5F8,C4=0x4092

    @PLD        [R0]
    @PLD        [R1]
    @PLD        [R2]


    @/* can be loaded from a defined const type */
    MOVW        R10,#0x3311
    VMOV.16     D0[0],R10                   @//C1

    MOVW        R10,#0xF379
    VMOV.16     D0[1],R10                   @//C2

    MOVW        R10,#0xE5F8
    VMOV.16     D0[2],R10                   @//C3

    MOVW        R10,#0x4092
    VMOV.16     D0[3],R10                   @//C4

    @//LOAD CONSTANT 128 INTO A CORTEX REGISTER
    MOV         R10,#128
    VDUP.8      D1,R10

    @//D0 HAS C1-C2-C3-C4
    @// load other parameters from stack
    LDR         R5,[sp,#40]
    @LDR  R4,[sp,#44]
    LDR         R6,[sp,#44]
    LDR         R7,[sp,#48]
    @LDR  R8,[sp,#52]
    LDR         R9,[sp,#52]

    @// calculate offsets, offset = stride - width
    SUB         R10,R6,R3                   @// luma offset
    SUB         R11,R7,R3
    @, LSR #1   @// u offset
    @SUB     R12,R8,R3, LSR #1  @// v offset
    SUB         R14,R9,R3                   @// rgb offset in pixels

    @// calculate height loop count
    MOV         R5,R5, LSR #1               @// height_cnt = height / 16

    @// create next row pointers for rgb and luma data
    ADD         R7,R0,R6                    @// luma_next_row = luma + luma_stride
    ADD         R8,R2,R9,LSL #2             @// rgb_next_row = rgb + rgb_stride

LABEL_YUV420SP_TO_RGB8888_HEIGHT_LOOP:

    @//LOAD VALUES OF U&V AND COMPUTE THE R,G,B WEIGHT VALUES.
    VLD1.8      {D2,D3},[R1]!               @//LOAD 8 VALUES OF UV
    @//VLD1.8 {D3},[R2]!            @//LOAD 8 VALUES OF V

    @// calculate width loop count
    MOV         R6,R3, LSR #4               @// width_cnt = width / 16

    @//COMPUTE THE ACTUAL RGB VALUES,WE CAN DO TWO ROWS AT A TIME
    @//LOAD VALUES OF Y 8-BIT VALUES
    VLD2.8      {D30,D31},[R0]!             @//D0 - Y0,Y2,Y4,Y6,Y8,Y10,Y12,Y14 row 1
                                            @//D1 - Y1,Y3,Y5,Y7,Y9,Y11,Y13,Y15
    VLD2.8      {D28,D29},[R7]!             @//D0 - Y0,Y2,Y4,Y6,Y8,Y10,Y12,Y14 row2
                                            @//D1 - Y1,Y3,Y5,Y7,Y9,Y11,Y13,Y15

    SUBS        R6,R6,#1
    BEQ         LABEL_YUV420SP_TO_RGB8888_WIDTH_LOOP_SKIP

LABEL_YUV420SP_TO_RGB8888_WIDTH_LOOP:
    @VMOV.I8 Q1,#128
    VUZP.8      D2,D3


    @//NEED TO SUBTRACT (U-128) AND (V-128)
    @//(D2-D1),(D3-D1)
    VSUBL.U8    Q2,D2,D1                    @//(U-128)
    VSUBL.U8    Q3,D3,D1                    @//(V-128)

    @//LOAD VALUES OF U&V for next row
    VLD1.8      {D2,D3},[R1]!               @//LOAD 8 VALUES OF U
    @//VLD1.8 {D3},[R2]!            @//LOAD 8 VALUES OF V

    @PLD        [R0]
    PLD         [R1]

    @//NEED TO MULTIPLY WITH Q2,Q3 WITH CO-EEFICIENTS
    VMULL.S16   Q4,D4,D0[3]                 @//(U-128)*C4 FOR B
    VMULL.S16   Q5,D5,D0[3]                 @//(U-128)*C4 FOR B

    VMULL.S16   Q10,D6,D0[0]                @//(V-128)*C1 FOR R
    VMULL.S16   Q11,D7,D0[0]                @//(V-128)*C1 FOR R

    VMULL.S16   Q6,D4,D0[1]                 @//(U-128)*C2 FOR G
    VMLAL.S16   Q6,D6,D0[2]                 @//Q6 = (U-128)*C2 + (V-128)*C3
    VMULL.S16   Q7,D5,D0[1]                 @//(U-128)*C2 FOR G
    VMLAL.S16   Q7,D7,D0[2]                 @//Q7 = (U-128)*C2 + (V-128)*C3

    @//NARROW RIGHT SHIFT BY 13 FOR R&B
    VQSHRN.S32  D8,Q4,#13                   @//D8 = (U-128)*C4>>13 4 16-BIT VALUES
    VQSHRN.S32  D9,Q5,#13                   @//D9 = (U-128)*C4>>13 4 16-BIT VALUES
    @//Q4 - WEIGHT FOR B

    @//NARROW RIGHT SHIFT BY 13 FOR R&B
    VQSHRN.S32  D10,Q10,#13                 @//D10 = (V-128)*C1>>13 4 16-BIT VALUES
    VQSHRN.S32  D11,Q11,#13                 @//D11 = (V-128)*C1>>13 4 16-BIT VALUES
    @//Q5 - WEIGHT FOR R

    @//NARROW RIGHT SHIFT BY 13 FOR G
    VQSHRN.S32  D12,Q6,#13                  @//D12 = [(U-128)*C2 + (V-128)*C3]>>13 4 16-BIT VALUES
    VQSHRN.S32  D13,Q7,#13                  @//D13 = [(U-128)*C2 + (V-128)*C3]>>13 4 16-BIT VALUES
    @//Q6 - WEIGHT FOR G

    VADDW.U8    Q7,Q4,D30                   @//Q7 - HAS Y + B
    VADDW.U8    Q8,Q5,D30                   @//Q8 - HAS Y + R
    VADDW.U8    Q9,Q6,D30                   @//Q9 - HAS Y + G

    VADDW.U8    Q10,Q4,D31                  @//Q10 - HAS Y + B
    VADDW.U8    Q11,Q5,D31                  @//Q11 - HAS Y + R
    VADDW.U8    Q12,Q6,D31                  @//Q12 - HAS Y + G

    VQMOVUN.S16 D14,Q7
    VQMOVUN.S16 D15,Q9
    VQMOVUN.S16 D16,Q8
    VMOV.I8     D17,#0

    VZIP.8      D14,D15
    VZIP.8      D16,D17
    VZIP.16     Q7,Q8


    VQMOVUN.S16 D20,Q10
    VQMOVUN.S16 D21,Q12
    VQMOVUN.S16 D22,Q11
    VMOV.I8     D23,#0

    VZIP.8      D20,D21
    VZIP.8      D22,D23
    VZIP.16     Q10,Q11

    VZIP.32     Q7,Q10
    VZIP.32     Q8,Q11

    VST1.32     D14,[R2]!
    VST1.32     D15,[R2]!
    VST1.32     D20,[R2]!
    VST1.32     D21,[R2]!
    VST1.32     D16,[R2]!
    VST1.32     D17,[R2]!
    VST1.32     D22,[R2]!
    VST1.32     D23,[R2]!

    @//D14-D20 - TOALLY HAVE 16 VALUES
    @//WE NEED TO SHIFT R,G,B VALUES TO GET 5BIT,6BIT AND 5BIT COMBINATIONS
    VADDW.U8    Q7,Q4,D28                   @//Q7 - HAS Y + B
    VADDW.U8    Q8,Q5,D28                   @//Q2 - HAS Y + R
    VADDW.U8    Q9,Q6,D28                   @//Q3 - HAS Y + G

    VADDW.U8    Q10,Q4,D29                  @//Q10 - HAS Y + B
    VADDW.U8    Q11,Q5,D29                  @//Q11 - HAS Y + R
    VADDW.U8    Q12,Q6,D29                  @//Q12 - HAS Y + G

    @//COMPUTE THE ACTUAL RGB VALUES,WE CAN DO TWO ROWS AT A TIME
    @//LOAD VALUES OF Y 8-BIT VALUES
    VLD2.8      {D30,D31},[R0]!             @//D0 - Y0,Y2,Y4,Y6,Y8,Y10,Y12,Y14 row 1
                                            @//D1 - Y1,Y3,Y5,Y7,Y9,Y11,Y13,Y15
    VLD2.8      {D28,D29},[R7]!             @//D0 - Y0,Y2,Y4,Y6,Y8,Y10,Y12,Y14 row2
                                            @//D1 - Y1,Y3,Y5,Y7,Y9,Y11,Y13,Y15

    PLD         [R0]
    PLD         [R7]

    VQMOVUN.S16 D14,Q7
    VQMOVUN.S16 D15,Q9
    VQMOVUN.S16 D16,Q8
    VMOV.I8     D17,#0

    VZIP.8      D14,D15
    VZIP.8      D16,D17
    VZIP.16     Q7,Q8


    VQMOVUN.S16 D20,Q10
    VQMOVUN.S16 D21,Q12
    VQMOVUN.S16 D22,Q11
    VMOV.I8     D23,#0

    VZIP.8      D20,D21
    VZIP.8      D22,D23
    VZIP.16     Q10,Q11

    VZIP.32     Q7,Q10
    VZIP.32     Q8,Q11

    VST1.32     D14,[R8]!
    VST1.32     D15,[R8]!
    VST1.32     D20,[R8]!
    VST1.32     D21,[R8]!
    VST1.32     D16,[R8]!
    VST1.32     D17,[R8]!
    VST1.32     D22,[R8]!
    VST1.32     D23,[R8]!

    SUBS        R6,R6,#1                    @// width_cnt -= 1
    BNE         LABEL_YUV420SP_TO_RGB8888_WIDTH_LOOP

LABEL_YUV420SP_TO_RGB8888_WIDTH_LOOP_SKIP:
    @VMOV.I8 Q1,#128
    VUZP.8      D2,D3


    @//NEED TO SUBTRACT (U-128) AND (V-128)
    @//(D2-D1),(D3-D1)
    VSUBL.U8    Q2,D2,D1                    @//(U-128)
    VSUBL.U8    Q3,D3,D1                    @//(V-128)


    @//NEED TO MULTIPLY WITH Q2,Q3 WITH CO-EEFICIENTS
    VMULL.S16   Q4,D4,D0[3]                 @//(U-128)*C4 FOR B
    VMULL.S16   Q5,D5,D0[3]                 @//(U-128)*C4 FOR B

    VMULL.S16   Q10,D6,D0[0]                @//(V-128)*C1 FOR R
    VMULL.S16   Q11,D7,D0[0]                @//(V-128)*C1 FOR R

    VMULL.S16   Q6,D4,D0[1]                 @//(U-128)*C2 FOR G
    VMLAL.S16   Q6,D6,D0[2]                 @//Q6 = (U-128)*C2 + (V-128)*C3
    VMULL.S16   Q7,D5,D0[1]                 @//(U-128)*C2 FOR G
    VMLAL.S16   Q7,D7,D0[2]                 @//Q7 = (U-128)*C2 + (V-128)*C3

    @//NARROW RIGHT SHIFT BY 13 FOR R&B
    VQSHRN.S32  D8,Q4,#13                   @//D8 = (U-128)*C4>>13 4 16-BIT VALUES
    VQSHRN.S32  D9,Q5,#13                   @//D9 = (U-128)*C4>>13 4 16-BIT VALUES
    @//Q4 - WEIGHT FOR B

    @//NARROW RIGHT SHIFT BY 13 FOR R&B
    VQSHRN.S32  D10,Q10,#13                 @//D10 = (V-128)*C1>>13 4 16-BIT VALUES
    VQSHRN.S32  D11,Q11,#13                 @//D11 = (V-128)*C1>>13 4 16-BIT VALUES
    @//Q5 - WEIGHT FOR R

    @//NARROW RIGHT SHIFT BY 13 FOR G
    VQSHRN.S32  D12,Q6,#13                  @//D12 = [(U-128)*C2 + (V-128)*C3]>>13 4 16-BIT VALUES
    VQSHRN.S32  D13,Q7,#13                  @//D13 = [(U-128)*C2 + (V-128)*C3]>>13 4 16-BIT VALUES
    @//Q6 - WEIGHT FOR G

    VADDW.U8    Q7,Q4,D30                   @//Q7 - HAS Y + B
    VADDW.U8    Q8,Q5,D30                   @//Q8 - HAS Y + R
    VADDW.U8    Q9,Q6,D30                   @//Q9 - HAS Y + G

    VADDW.U8    Q10,Q4,D31                  @//Q10 - HAS Y + B
    VADDW.U8    Q11,Q5,D31                  @//Q11 - HAS Y + R
    VADDW.U8    Q12,Q6,D31                  @//Q12 - HAS Y + G

    VQMOVUN.S16 D14,Q7
    VQMOVUN.S16 D15,Q9
    VQMOVUN.S16 D16,Q8
    VMOV.I8     D17,#0

    VZIP.8      D14,D15
    VZIP.8      D16,D17
    VZIP.16     Q7,Q8


    VQMOVUN.S16 D20,Q10
    VQMOVUN.S16 D21,Q12
    VQMOVUN.S16 D22,Q11
    VMOV.I8     D23,#0

    VZIP.8      D20,D21
    VZIP.8      D22,D23
    VZIP.16     Q10,Q11

    VZIP.32     Q7,Q10
    VZIP.32     Q8,Q11

    VST1.32     D14,[R2]!
    VST1.32     D15,[R2]!
    VST1.32     D20,[R2]!
    VST1.32     D21,[R2]!
    VST1.32     D16,[R2]!
    VST1.32     D17,[R2]!
    VST1.32     D22,[R2]!
    VST1.32     D23,[R2]!

    @//D14-D20 - TOALLY HAVE 16 VALUES
    @//WE NEED TO SHIFT R,G,B VALUES TO GET 5BIT,6BIT AND 5BIT COMBINATIONS
    VADDW.U8    Q7,Q4,D28                   @//Q7 - HAS Y + B
    VADDW.U8    Q8,Q5,D28                   @//Q2 - HAS Y + R
    VADDW.U8    Q9,Q6,D28                   @//Q3 - HAS Y + G

    VADDW.U8    Q10,Q4,D29                  @//Q10 - HAS Y + B
    VADDW.U8    Q11,Q5,D29                  @//Q11 - HAS Y + R
    VADDW.U8    Q12,Q6,D29                  @//Q12 - HAS Y + G


    VQMOVUN.S16 D14,Q7
    VQMOVUN.S16 D15,Q9
    VQMOVUN.S16 D16,Q8
    VMOV.I8     D17,#0

    VZIP.8      D14,D15
    VZIP.8      D16,D17
    VZIP.16     Q7,Q8


    VQMOVUN.S16 D20,Q10
    VQMOVUN.S16 D21,Q12
    VQMOVUN.S16 D22,Q11
    VMOV.I8     D23,#0

    VZIP.8      D20,D21
    VZIP.8      D22,D23
    VZIP.16     Q10,Q11

    VZIP.32     Q7,Q10
    VZIP.32     Q8,Q11

    VST1.32     D14,[R8]!
    VST1.32     D15,[R8]!
    VST1.32     D20,[R8]!
    VST1.32     D21,[R8]!
    VST1.32     D16,[R8]!
    VST1.32     D17,[R8]!
    VST1.32     D22,[R8]!
    VST1.32     D23,[R8]!

    @// Adjust the address pointers
    ADD         R0,R7,R10                   @// luma = luma_next + offset
    ADD         R2,R8,R14,LSL #2            @// rgb = rgb_next + offset

    ADD         R7,R0,R3                    @// luma_next = luma + width
    ADD         R8,R2,R3,LSL #2             @// rgb_next_row = rgb + width

    ADD         R1,R1,R11                   @// adjust u pointer
    @ADD        R2,R2,R12           @// adjust v pointer

    ADD         R7,R7,R10                   @// luma_next = luma + width + offset (because of register crunch)
    ADD         R8,R8,R14,LSL #2            @// rgb_next_row = rgb + width + offset

    SUBS        R5,R5,#1                    @// height_cnt -= 1

    BNE         LABEL_YUV420SP_TO_RGB8888_HEIGHT_LOOP

    @//POP THE REGISTERS
    LDMFD       SP!,{R4-R12,PC}




    .section .note.GNU-stack,"",%progbits

