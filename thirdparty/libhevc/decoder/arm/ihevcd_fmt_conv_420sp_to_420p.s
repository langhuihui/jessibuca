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
@*  ihevcd_fmt_conv_420sp_to_420p.s
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








.text





@/*****************************************************************************
@*                                                                            *
@*  Function Name    : neon_copy_yuv420sp_to_yuv420p()                       *
@*                                                                            *
@*  Description      : This function conversts the image from YUV420sP color  *
@*                     space to 420SP color space(UV interleaved).            *
@*                                                                            *
@*  Arguments        : R0           pu1_src_y                                 *
@*                     R1           pu1_src_uv                                *
@*                     R2           pu1_dest_y                                *
@*                     R3           pu1_dest_u                               *
@*                     [R13 #40]    pu1_dest_v                               *
@*                     [R13 #44]    u2_width                                 *
@*                     [R13 #48]    u2_height                                   *
@*                     [R13 #52]    u2_stridey                                *
@*                     [R13 #56]    u2_strideuv                               *
@*                     [R13 #60]    u2_dest_stridey                           *
@*                     [R13 #64]    u2_dest_strideuv                          *
@*                     [R13 #68]    is_u_first                                *
@*                     [R13 #72]    disable_luma_copy                         *
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

.globl ihevcd_fmt_conv_420sp_to_420p_a9q

.type ihevcd_fmt_conv_420sp_to_420p_a9q, %function

ihevcd_fmt_conv_420sp_to_420p_a9q:
    STMFD       sp!,{r4-r12, lr}

    LDR         r5,[sp,#60]                 @//Load u2_dest_stridey
@   LDR     r6,[sp,#56]             @//Load u2_strideuv
    LDR         r7,[sp,#52]                 @//Load u2_stridey
    LDR         r8,[sp,#44]                 @//Load u2_width
    LDR         r9,[sp,#48]                 @//Load u2_height

    SUB         r10,r7,r8                   @// Src Y increment
    SUB         r11,r5,r8                   @// Dst Y increment

    LDR         r5,[sp,#72]                 @//Load disable_luma_copy flag
    CMP         r5,#0                       @//skip luma if disable_luma_copy is non-zero
    BNE         uv_copy_start

    @/* Copy Y */

    MOV         r4,r9                       @// Copying height
y_row_loop:
    MOV         r6,r8                       @// Copying width

y_col_loop:

    SUB         r6,r6,#16
    vld1.8      {d0,d1},[r0]!
    vst1.8      {d0,d1},[r2]!
    CMP         r6,#16
    BGE         y_col_loop
    CMP         r6,#0
    BEQ         y_col_loop_end
    @//If non-multiple of 16, then go back by few bytes to ensure 16 bytes can be read
    @//Ex if width is 162, above loop will process 160 pixels. And
    @//Both source and destination will point to 146th pixel and then 16 bytes will be read
    @// and written using VLD1 and VST1
    RSB         r6,r6,#16
    SUB         r0,r0,r6
    SUB         r2,r2,r6
    vld1.8      {d0,d1}, [r0]!
    vst1.8      {d0,d1}, [r2]!

y_col_loop_end:
    ADD         r0, r0, r10
    ADD         r2, r2, r11
    SUBS        r4, r4, #1
    BGT         y_row_loop


    @/* Copy UV */
uv_copy_start:

    LDR         r5,[sp,#64]                 @//Load u2_dest_strideuv
    LDR         r7,[sp,#56]                 @//Load u2_strideuv

    MOV         r9,r9,LSR #1                @// height/2
@   MOV     r8,r8,LSR #1            @// Width/2

    SUB         r10,r7,r8                   @// Src UV increment
    MOV         r11,r8,LSR #1
    SUB         r11,r5,r11                  @// Dst U and V increment

    LDR         r5,[sp,#40]                 @//Load pu1_dest_v

    LDR         r4,[sp,#68]                 @//Load is_u_first_flag
    CMP         r4,#0                       @//Swap U and V dest if is_u_first_flag is zero
    MOVEQ       r4,r5
    MOVEQ       r5,r3
    MOVEQ       r3,r4

    MOV         r4,r9                       @// Copying height
uv_row_loop:
    MOV         r6,r8                       @// Copying width

uv_col_loop:

    SUB         r6,r6,#16

    PLD         [r1,#128]
    vld2.8      {d0,d1},[r1]!
    VST1.8      D0,[r3]!
    VST1.8      D1,[r5]!
    CMP         r6,#16
    BGE         uv_col_loop
    CMP         r6,#0
    BEQ         uv_col_loop_end
    @//If non-multiple of 16, then go back by few bytes to ensure 16 bytes can be read
    @//Ex if width is 162, above loop will process 160 pixels. And
    @//Both source and destination will point to 146th pixel and then 16 bytes will be read
    @// and written using VLD1 and VST1
    RSB         r6,r6,#16
    SUB         r1,r1,r6
    SUB         r3,r3,r6,LSR #1
    SUB         r5,r5,r6,LSR #1
    vld2.8      {d0,d1}, [r1]!
    VST1.8      D0, [r3]!
    VST1.8      D1, [r5]!
uv_col_loop_end:
    ADD         r1, r1, r10
    ADD         r3, r3, r11
    ADD         r5, r5, r11
    SUBS        r4, r4, #1
    BGT         uv_row_loop

exit:
    LDMFD       sp!,{r4-r12, pc}






