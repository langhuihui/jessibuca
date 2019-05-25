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
@ *******************************************************************************
@ * @file
@ *  ihevc_itrans_recon_4x4_ttype1.s
@ *
@ * @brief
@ *  contains function definitions for inverse transform  and reconstruction
@ *
@ *
@ * @author
@ *  naveen sr
@ *
@ * @par list of functions:
@ *  - ihevc_itrans_recon_4x4_ttype1()
@ *
@ * @remarks
@ *  none
@ *
@ *******************************************************************************
@ */

@/* all the functions here are replicated from ihevc_itrans.c and modified to */
@/* include reconstruction */
@
@/**
@ *******************************************************************************
@ *
@ * @brief
@ *  this function performs inverse transform type 1 (dst)  and reconstruction
@ * for 4x4 input block
@ *
@ * @par description:
@ *  performs inverse transform and adds the prediction  data and clips output
@ * to 8 bit
@ *
@ * @param[in] pi2_src
@ *  input 4x4 coefficients
@ *
@ * @param[in] pi2_tmp
@ *  temporary 4x4 buffer for storing inverse
@ *
@ *  transform
@ *  1st stage output
@ *
@ * @param[in] pu1_pred
@ *  prediction 4x4 block
@ *
@ * @param[out] pu1_dst
@ *  output 4x4 block
@ *
@ * @param[in] src_strd
@ *  input stride
@ *
@ * @param[in] pred_strd
@ *  prediction stride
@ *
@ * @param[in] dst_strd
@ *  output stride
@ *
@ * @param[in] zero_cols
@ *  zero columns in pi2_src
@ *
@ * @returns  void
@ *
@ * @remarks
@ *  none
@ *
@ *******************************************************************************
@ */
@void ihevc_itrans_recon_4x4_ttype1(word16 *pi2_src,
@       word16 *pi2_tmp,
@       uword8 *pu1_pred,
@       uword8 *pu1_dst,
@       word32 src_strd,
@       word32 pred_strd,
@       word32 dst_strd,
@       word32 zero_cols)

@**************variables vs registers*************************
@   r0 => *pi2_src
@   r1 => *pi2_tmp
@   r2 => *pu1_pred
@   r3 => *pu1_dst
@   r4 => src_strd
@   r5 => pred_strd
@   r6 => dst_strd
@   r7 => zero_cols

.equ    src_strd_offset,    104
.equ    pred_strd_offset,   108
.equ    dst_strd_offset,    112
.equ    zero_cols_offset,   116

.text
.align 4




.set shift_stage1_idct ,   7
.set shift_stage2_idct ,   12

.globl ihevc_itrans_recon_4x4_ttype1_a9q

.type ihevc_itrans_recon_4x4_ttype1_a9q, %function

ihevc_itrans_recon_4x4_ttype1_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8  -  d15}

    ldr         r4,[sp,#src_strd_offset]    @loading src_strd
    ldr         r5,[sp,#pred_strd_offset]   @loading pred_strd
    ldr         r6,[sp,#dst_strd_offset]    @loading dst_strd
    ldr         r7,[sp,#zero_cols_offset]   @loading zero_cols

    add         r4,r4,r4                    @ src_strd in terms of word16

    mov         r8,#29
    mov         r9,#55
    mov         r10,#74
    mov         r11,#84
    vmov.i16    d4[0],r8
    vld1.16     d0,[r0],r4                  @loading pi2_src 1st row
    vmov.i16    d4[1],r9
    vld1.16     d1,[r0],r4                  @loading pi2_src 2nd row
    vmov.i16    d4[2],r10
    vld1.16     d2,[r0],r4                  @loading pi2_src 3rd row
    vmov.i16    d4[3],r11
    vld1.16     d3,[r0],r4                  @loading pi2_src 4th row

    @ first stage computation starts
    vmull.s16   q3,d1,d4[2]                 @74 * pi2_src[1]
    vmlal.s16   q3,d0,d4[0]                 @74 * pi2_src[1] + 29 * pi2_src[0]
    vmlal.s16   q3,d3,d4[1]                 @74 * pi2_src[1] + 29 * pi2_src[0] + 55 * pi2_src[3]
    vmlal.s16   q3,d2,d4[3]                 @pi2_out[0] = 29* pi2_src[0] + 74 * pi2_src[1] + 84* pi2_src[2] + 55 * pi2_src[3]

    vmull.s16   q4,d1,d4[2]                 @74 * pi2_src[1]
    vmlal.s16   q4,d0,d4[1]                 @74 * pi2_src[1] + 55 * pi2_src[0]
    vmlsl.s16   q4,d2,d4[0]                 @74 * pi2_src[1] + 55 * pi2_src[0] -  29 * pi2_src[2]
    vmlsl.s16   q4,d3,d4[3]                 @pi2_out[1] = 74 * pi2_src[1] + 55 * pi2_src[0] -  29 * pi2_src[2] - 84 * pi2_src[3])

    vmull.s16   q5,d0,d4[2]                 @ 74 * pi2_src[0]
    vmlsl.s16   q5,d2,d4[2]                 @ 74 * pi2_src[0] - 74 * pi2_src[2]
    vmlal.s16   q5,d3,d4[2]                 @pi2_out[2] = 74 * pi2_src[0] - 74 * pi2_src[2] + 74 * pi2_src[3]

    vmull.s16   q6,d2,d4[1]                 @ 55 * pi2_src[2]
    vmlsl.s16   q6,d1,d4[2]                 @ 55 * pi2_src[2] - 74 * pi2_src[1]
    vmlsl.s16   q6,d3,d4[0]                 @ - 74 * pi2_src[1] +   55 * pi2_src[2] - 29 * pi2_src[3]
    vmlal.s16   q6,d0,d4[3]                 @pi2_out[3] = 84 * pi2_src[0] - 74 * pi2_src[1] + 55 * pi2_src[2] - 29 * pi2_src[3]

    vqrshrn.s32 d14,q3,#shift_stage1_idct   @ (pi2_out[0] + rounding ) >> shift_stage1_idct
    vqrshrn.s32 d15,q4,#shift_stage1_idct   @ (pi2_out[1] + rounding ) >> shift_stage1_idct
    vqrshrn.s32 d16,q5,#shift_stage1_idct   @ (pi2_out[2] + rounding ) >> shift_stage1_idct
    vqrshrn.s32 d17,q6,#shift_stage1_idct   @ (pi2_out[3] + rounding ) >> shift_stage1_idct
    vld1.32     d18[0], [r2],r5

    vtrn.16     d14,d15
    vtrn.16     d16,d17
    vtrn.32     d14,d16
    vtrn.32     d15,d17
    @ output in d14,d15,d16,d17
    @ first stage computation ends

    @ second stage computation starts  :  copy pasting 1st stage
    @ register changes
    @ d14 - d0
    @ d15 - d1
    @ d16 - d2
    @ d17 - d3
    vld1.32     d18[1], [r2],r5
    vmull.s16   q3,d15,d4[2]                @74 * pi2_src[1]
    vmlal.s16   q3,d14,d4[0]                @74 * pi2_src[1] + 29 * pi2_src[0]
    vmlal.s16   q3,d17,d4[1]                @74 * pi2_src[1] + 29 * pi2_src[0] + 55 * pi2_src[3]
    vmlal.s16   q3,d16,d4[3]                @pi2_out[0] = 29* pi2_src[0] + 74 * pi2_src[1] + 84* pi2_src[2] + 55 * pi2_src[3]

    vmull.s16   q4,d15,d4[2]                @74 * pi2_src[1]
    vmlal.s16   q4,d14,d4[1]                @74 * pi2_src[1] + 55 * pi2_src[0]
    vmlsl.s16   q4,d16,d4[0]                @74 * pi2_src[1] + 55 * pi2_src[0] -  29 * pi2_src[2]
    vmlsl.s16   q4,d17,d4[3]                @pi2_out[1] = 74 * pi2_src[1] + 55 * pi2_src[0] -  29 * pi2_src[2] - 84 * pi2_src[3])

    vmull.s16   q5,d14,d4[2]                @ 74 * pi2_src[0]
    vmlsl.s16   q5,d16,d4[2]                @ 74 * pi2_src[0] - 74 * pi2_src[2]
    vmlal.s16   q5,d17,d4[2]                @pi2_out[2] = 74 * pi2_src[0] - 74 * pi2_src[2] + 74 * pi2_src[3]
    vld1.32     d19[0], [r2],r5

    vmull.s16   q6,d16,d4[1]                @ 55 * pi2_src[2]
    vmlsl.s16   q6,d15,d4[2]                @  - 74 * pi2_src[1] +   55 * pi2_src[2]
    vmlsl.s16   q6,d17,d4[0]                @ - 74 * pi2_src[1] +   55 * pi2_src[2] - 29 * pi2_src[3]
    vmlal.s16   q6,d14,d4[3]                @pi2_out[3] = 84 * pi2_src[0] - 74 * pi2_src[1] + 55 * pi2_src[2] - 29 * pi2_src[3]

    vqrshrn.s32 d0,q3,#shift_stage2_idct    @ (pi2_out[0] + rounding ) >> shift_stage1_idct
    vqrshrn.s32 d1,q4,#shift_stage2_idct    @ (pi2_out[1] + rounding ) >> shift_stage1_idct
    vqrshrn.s32 d2,q5,#shift_stage2_idct    @ (pi2_out[2] + rounding ) >> shift_stage1_idct
    vqrshrn.s32 d3,q6,#shift_stage2_idct    @ (pi2_out[3] + rounding ) >> shift_stage1_idct
    vld1.32     d19[1], [r2],r5
    vtrn.16     d0,d1
    vtrn.16     d2,d3
    vtrn.32     d0,d2
    vtrn.32     d1,d3
    @ output in d0,d1,d2,d3
    @ second stage computation ends

    @ loading pred

    vaddw.u8    q0,q0,d18                   @ pi2_out(16bit) + pu1_pred(8bit)
    vqmovun.s16 d0,q0                       @ clip_u8(pi2_out(16bit) + pu1_pred(8bit))
    vaddw.u8    q1,q1,d19                   @ pi2_out(16bit) + pu1_pred(8bit)
    vqmovun.s16 d1,q1                       @ clip_u8(pi2_out(16bit) + pu1_pred(8bit))

    @ storing destination
    vst1.32     {d0[0]},[r3],r6
    vst1.32     {d0[1]},[r3],r6
    vst1.32     {d1[0]},[r3],r6
    vst1.32     {d1[1]},[r3],r6

    vpop        {d8  -  d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp









