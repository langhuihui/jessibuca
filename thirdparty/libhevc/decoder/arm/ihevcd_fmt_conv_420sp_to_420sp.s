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
@*  ihevcd_fmt_conv_420sp_to_420sp.s
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
@*  Function Name    : ihevcd_fmt_conv_420sp_to_420sp()                       *
@*                                                                            *
@*  Description      : This function conversts the image from YUV420SP color  *
@*                     space to 420SP color space(UV interleaved).            *
@*                                                                            *
@*  Arguments        : R0           pu1_y                                     *
@*                     R1           pu1_uv                                    *
@*                     R2           pu1_dest_y                                *
@*                     R3           pu1_dest_uv                               *
@*                     [R13 #40]    u2_width                                  *
@*                     [R13 #44]    u2_height                                 *
@*                     [R13 #48]    u2_stridey                                *
@*                     [R13 #52]    u2_stridechroma                           *
@*                     [R13 #56]    u2_dest_stridey                           *
@*                     [R13 #60]    u2_dest_stridechroma                      *
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
@*       Assumptions: Image Width:     Assumed to be multiple of 2 and       *
@*                     Image Height:    Assumed to be even.                   *
@*                                                                            *
@*  Revision History :                                                        *
@*         DD MM YYYY   Author(s)       Changes (Describe the changes made)   *
@*         16 05 2012   Naveen SR     draft                                   *
@*                                                                            *
@*****************************************************************************/

    .global ihevcd_fmt_conv_420sp_to_420sp_a9q
.type ihevcd_fmt_conv_420sp_to_420sp_a9q, %function
ihevcd_fmt_conv_420sp_to_420sp_a9q:

    STMFD       sp!,{r4-r12, lr}


    LDR         r5,[sp,#56]                 @//Load u2_dest_stridey

    LDR         r7,[sp,#48]                 @//Load u2_stridey
    LDR         r8,[sp,#40]                 @//Load u2_width
    LDR         r9,[sp,#44]                 @//Load u2_height

    SUB         r10,r7,r8                   @// Src Y increment
    SUB         r11,r5,r8                   @// Dst Y increment

    @/* Copy Y */

    MOV         r4,r9                       @// Copying height
y_row_loop:
    MOV         r6,r8                       @// Copying width

y_col_loop:
    PLD         [r0, #128]
    SUB         r6,r6,#32
    VLD1.8      D0,[r0]!
    VLD1.8      D1,[r0]!
    VLD1.8      D2,[r0]!
    VLD1.8      D3,[r0]!
    VST1.8      D0,[R2]!
    VST1.8      D1,[R2]!
    VST1.8      D2,[R2]!
    VST1.8      D3,[R2]!
    CMP         r6,#32
    BGE         y_col_loop
    CMP         r6,#0
    BEQ         y_col_loop_end
    @//If non-multiple of 16, then go back by few bytes to ensure 16 bytes can be read
    @//Ex if width is 162, above loop will process 160 pixels. And
    @//Both source and destination will point to 146th pixel and then 16 bytes will be read
    @// and written using VLD1 and VST1
    RSB         r6,r6,#32
    SUB         r0,r0,r6
    SUB         R2,R2,r6
    VLD1.8      D0,[r0]!
    VLD1.8      D1,[r0]!
    VLD1.8      D2,[r0]!
    VLD1.8      D3,[r0]!
    VST1.8      D0,[R2]!
    VST1.8      D1,[R2]!
    VST1.8      D2,[R2]!
    VST1.8      D3,[R2]!

y_col_loop_end:
    ADD         r0, r0, r10
    ADD         R2, R2, r11
    SUBS        r4, r4, #1
    BGT         y_row_loop



    @/* Copy UV */

    LDR         r5,[sp,#60]                 @//Load u2_dest_stridechroma
    LDR         r7,[sp,#52]                 @//Load u2_stridechroma

    MOV         r9,r9,LSR #1                @// height/2
@   MOV     r8,r8,LSR #1            @// Width/2

    MOV         R2,R3                       @pu1_dest_uv

    SUB         r10,r7,r8                   @// Src UV increment
    SUB         r11,r5,r8                   @// Dst UV increment

    MOV         r4,r9                       @// Copying height
uv_row_loop:
    MOV         r6,r8                       @// Copying width

uv_col_loop:

    PLD         [r1, #128]
    SUB         r6,r6,#16
    VLD1.8      D0,[r1]!
    VLD1.8      D1,[r1]!
    VST1.8      D0,[R2]!
    VST1.8      D1,[R2]!
    CMP         r6,#16
    BGE         uv_col_loop
    CMP         r6,#0
    BEQ         u_col_loop_end
    @//If non-multiple of 16, then go back by few bytes to ensure 16 bytes can be read
    @//Ex if width is 162, above loop will process 160 pixels. And
    @//Both source and destination will point to 146th pixel and then 16 bytes will be read
    @// and written using VLD1 and VST1
    RSB         r6,r6,#16
    SUB         r1,r1,r6
    SUB         R2,R2,r6
    VLD1.8      D0, [r1]!
    VLD1.8      D1, [r1]!
    VST1.8      D0, [R2]!
    VST1.8      D1, [R2]!

u_col_loop_end:
    ADD         r1, r1, r10
    ADD         R2, R2, r11
    SUBS        r4, r4, #1
    BGT         uv_row_loop

exit:
    LDMFD       sp!,{r4-r12, pc}


    .section .note.GNU-stack,"",%progbits

