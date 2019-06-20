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
@ *  ihevc_itrans_recon_8x8_neon.s
@ *
@ * @brief
@ *  contains function definitions for single stage  inverse transform
@ *
@ * @author
@ *  anand s
@ *
@ * @par list of functions:
@ *  - ihevc_itrans_recon_8x8()
@ *
@ * @remarks
@ *  none
@ *
@ *******************************************************************************
@*/

@/**
@ *******************************************************************************
@ *
@ * @brief
@ *  this function performs inverse transform  and reconstruction for 8x8
@ * input block
@ *
@ * @par description:
@ *  performs inverse transform and adds the prediction  data and clips output
@ * to 8 bit
@ *
@ * @param[in] pi2_src
@ *  input 8x8 coefficients
@ *
@ * @param[in] pi2_tmp
@ *  temporary 8x8 buffer for storing inverse
@ *
@ *  transform
@ *  1st stage output
@ *
@ * @param[in] pu1_pred
@ *  prediction 8x8 block
@ *
@ * @param[out] pu1_dst
@ *  output 8x8 block
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
@ * @param[in] shift
@ *  output shift
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

@void ihevc_itrans_recon_8x8(word16 *pi2_src,
@                            word16 *pi2_tmp,
@                            uword8 *pu1_pred,
@                            uword8 *pu1_dst,
@                            word32 src_strd,
@                            word32 pred_strd,
@                            word32 dst_strd,
@                            word32 zero_cols
@                            word32 zero_rows               )

@**************variables vs registers*************************
@   r0 => *pi2_src
@   r1 => *pi2_tmp
@   r2 => *pu1_pred
@   r3 => *pu1_dst
@   src_strd
@   pred_strd
@   dst_strd
@   zero_cols

.equ    src_stride_offset,     104
.equ    pred_stride_offset,    108
.equ    out_stride_offset,     112
.equ    zero_cols_offset,      116
.equ    zero_rows_offset,      120


.text
.align 4




.set width_x_size_x5 ,   40
.set width_x_size_x2 ,   32
.set shift_stage1_idct ,   7
.set shift_stage2_idct ,   12

.globl ihevc_itrans_recon_8x8_a9q

.extern g_ai2_ihevc_trans_8_transpose

g_ai2_ihevc_trans_8_transpose_addr:
.long g_ai2_ihevc_trans_8_transpose - ulbl1 - 8

.type ihevc_itrans_recon_8x8_a9q, %function

ihevc_itrans_recon_8x8_a9q:
@//register usage.extern        - loading and until idct of columns
@// cosine constants    -   d0
@// sine constants      -   d1
@// row 0 first half    -   d2      -   y0
@// row 1 first half    -   d6      -   y1
@// row 2 first half    -   d3      -   y2
@// row 3 first half    -   d7      -   y3
@// row 4 first half    -   d10     -   y4
@// row 5 first half    -   d14     -   y5
@// row 6 first half    -   d11     -   y6
@// row 7 first half    -   d15     -   y7

@// row 0 second half   -   d4      -   y0
@// row 1 second half   -   d8      -   y1
@// row 2 second half   -   d5      -   y2
@// row 3 second half   -   d9      -   y3
@// row 4 second half   -   d12     -   y4
@// row 5 second half   -   d16     -   y5
@// row 6 second half   -   d13     -   y6
@// row 7 second half   -   d17     -   y7

    @// copy the input pointer to another register
    @// step 1 : load all constants
    stmfd       sp!,{r4-r12,lr}
    vpush       {d8  -  d15}

    ldr         r8, [sp, #pred_stride_offset]    @ prediction stride
    ldr         r7, [sp, #out_stride_offset]     @ destination stride
    ldr         r6, [sp, #src_stride_offset]     @ src stride
    ldr         r12, [sp, #zero_cols_offset]
    ldr         r11, [sp, #zero_rows_offset]
    mov         r6,r6,lsl #1                @ x sizeof(word16)
    add         r9,r0,r6, lsl #1            @ 2 rows

    add         r10,r6,r6, lsl #1           @ 3 rows

    sub         r10,r10, #8                 @ - 4 cols * sizeof(word16)
    sub         r5,r6, #8                   @ src_strd - 4 cols * sizeof(word16)

@   ldr         r14,=g_imp4d_cxa8_idct_q15
    ldr         r14,g_ai2_ihevc_trans_8_transpose_addr
ulbl1:
    add         r14,r14,pc
    vld1.16     {d0,d1},[r14]               @//d0,d1 are used for storing the constant data

    @//step 2 load all the input data
    @//step 3 operate first 4 colums at a time

    and         r11,r11,#0xff
    and         r12,r12,#0xff

    cmp         r11,#0xf0
    bge         skip_last4_rows


    vld1.16     d2,[r0]!
    vld1.16     d3,[r9]!
    vld1.16     d4,[r0],r5
    vmull.s16   q10,d2,d0[0]                @// y0 * cos4(part of c0 and c1)
    vld1.16     d5,[r9],r5
    vmull.s16   q9,d3,d1[2]                 @// y2 * sin2 (q3 is freed by this time)(part of d1)
    vld1.16     d6,[r0]!
    vld1.16     d7,[r9]!
    vmull.s16   q12,d6,d0[1]                @// y1 * cos1(part of b0)
    vld1.16     d8,[r0],r10
    vmull.s16   q13,d6,d0[3]                @// y1 * cos3(part of b1)
    vld1.16     d9,[r9],r10
    vmull.s16   q14,d6,d1[1]                @// y1 * sin3(part of b2)
    vld1.16     d10,[r0]!
    vmull.s16   q15,d6,d1[3]                @// y1 * sin1(part of b3)
    vld1.16     d11,[r9]!
    vmlal.s16   q12,d7,d0[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vld1.16     d12,[r0],r5
    vmlsl.s16   q13,d7,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vld1.16     d13,[r9],r5
    vmlsl.s16   q14,d7,d0[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vld1.16     d14,[r0]!
    vmlsl.s16   q15,d7,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)
    vld1.16     d15,[r9]!
    vmull.s16   q11,d10,d0[0]               @// y4 * cos4(part of c0 and c1)
    vld1.16     d16,[r0],r10
    vmull.s16   q3,d3,d0[2]                 @// y2 * cos2(part of d0)
    vld1.16     d17,[r9],r10

    @/* this following was activated when alignment is not there */
@// vld1.16     d2,[r0]!
@// vld1.16     d3,[r2]!
@// vld1.16     d4,[r0]!
@// vld1.16     d5,[r2]!
@// vld1.16     d6,[r0]!
@// vld1.16     d7,[r2]!
@// vld1.16     d8,[r0],r3
@// vld1.16     d9,[r2],r3
@// vld1.16     d10,[r0]!
@// vld1.16     d11,[r2]!
@// vld1.16     d12,[r0]!
@// vld1.16     d13,[r2]!
@// vld1.16     d14,[r0]!
@// vld1.16     d15,[r2]!
@// vld1.16     d16,[r0],r3
@// vld1.16     d17,[r2],r3




    vmlal.s16   q12,d14,d1[1]               @// y1 * cos1 + y3 * cos3 + y5 * sin3(part of b0)
    vmlsl.s16   q13,d14,d0[1]               @// y1 * cos3 - y3 * sin1 - y5 * cos1(part of b1)
    vmlal.s16   q14,d14,d1[3]               @// y1 * sin3 - y3 * cos1 + y5 * sin1(part of b2)
    vmlal.s16   q15,d14,d0[3]               @// y1 * sin1 - y3 * sin3 + y5 * cos3(part of b3)

    vmlsl.s16   q9,d11,d0[2]                @// d1 = y2 * sin2 - y6 * cos2(part of a0 and a1)
    vmlal.s16   q3,d11,d1[2]                @// d0 = y2 * cos2 + y6 * sin2(part of a0 and a1)

    vadd.s32    q5,q10,q11                  @// c0 = y0 * cos4 + y4 * cos4(part of a0 and a1)
    vsub.s32    q10,q10,q11                 @// c1 = y0 * cos4 - y4 * cos4(part of a0 and a1)

    vmlal.s16   q12,d15,d1[3]               @// b0 = y1 * cos1 + y3 * cos3 + y5 * sin3 + y7 * sin1(part of r0,r7)
    vmlsl.s16   q13,d15,d1[1]               @// b1 = y1 * cos3 - y3 * sin1 - y5 * cos1 - y7 * sin3(part of r1,r6)
    vmlal.s16   q14,d15,d0[3]               @// b2 = y1 * sin3 - y3 * cos1 + y5 * sin1 + y7 * cos3(part of r2,r5)
    vmlsl.s16   q15,d15,d0[1]               @// b3 = y1 * sin1 - y3 * sin3 + y5 * cos3 - y7 * cos1(part of r3,r4)

    vadd.s32    q7,q5,q3                    @// a0 = c0 + d0(part of r0,r7)
    vsub.s32    q5,q5,q3                    @// a3 = c0 - d0(part of r3,r4)
    vsub.s32    q11,q10,q9                  @// a2 = c1 - d1(part of r2,r5)
    vadd.s32    q9,q10,q9                   @// a1 = c1 + d1(part of r1,r6)

    vadd.s32    q10,q7,q12                  @// a0 + b0(part of r0)
    vsub.s32    q3,q7,q12                   @// a0 - b0(part of r7)

    vadd.s32    q12,q11,q14                 @// a2 + b2(part of r2)
    vsub.s32    q11,q11,q14                 @// a2 - b2(part of r5)

    vadd.s32    q14,q9,q13                  @// a1 + b1(part of r1)
    vsub.s32    q9,q9,q13                   @// a1 - b1(part of r6)

    vadd.s32    q13,q5,q15                  @// a3 + b3(part of r3)
    vsub.s32    q15,q5,q15                  @// a3 - b3(part of r4)

    vqrshrn.s32 d2,q10,#shift_stage1_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d15,q3,#shift_stage1_idct   @// r7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d3,q12,#shift_stage1_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d14,q11,#shift_stage1_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d6,q14,#shift_stage1_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d11,q9,#shift_stage1_idct   @// r6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d7,q13,#shift_stage1_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d10,q15,#shift_stage1_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)


    b           last4_cols



skip_last4_rows:



    vld1.16     d2,[r0]!
    vld1.16     d3,[r9]!
    vld1.16     d4,[r0],r5
    vld1.16     d5,[r9],r5
    vld1.16     d6,[r0]!
    vld1.16     d7,[r9]!
    vld1.16     d8,[r0],r10
    vld1.16     d9,[r9],r10



    vmov.s16    q6,#0
    vmov.s16    q8,#0




    vmull.s16   q12,d6,d0[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d6,d0[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d6,d1[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d6,d1[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d7,d0[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d7,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d7,d0[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d7,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)

    vmull.s16   q9,d3,d1[2]                 @// y2 * sin2 (q3 is freed by this time)(part of d1)
    vmull.s16   q3,d3,d0[2]                 @// y2 * cos2(part of d0)

    vmull.s16   q10,d2,d0[0]                @// y0 * cos4(part of c0 and c1)


    vadd.s32    q7,q10,q3                   @// a0 = c0 + d0(part of r0,r7)
    vsub.s32    q5,q10,q3                   @// a3 = c0 - d0(part of r3,r4)
    vsub.s32    q11,q10,q9                  @// a2 = c1 - d1(part of r2,r5)
    vadd.s32    q9,q10,q9                   @// a1 = c1 + d1(part of r1,r6)

    vadd.s32    q10,q7,q12                  @// a0 + b0(part of r0)
    vsub.s32    q3,q7,q12                   @// a0 - b0(part of r7)

    vadd.s32    q12,q11,q14                 @// a2 + b2(part of r2)
    vsub.s32    q11,q11,q14                 @// a2 - b2(part of r5)

    vadd.s32    q14,q9,q13                  @// a1 + b1(part of r1)
    vsub.s32    q9,q9,q13                   @// a1 - b1(part of r6)

    vadd.s32    q13,q5,q15                  @// a3 + b3(part of r3)
    vsub.s32    q15,q5,q15                  @// a3 - b3(part of r4)

    vqrshrn.s32 d2,q10,#shift_stage1_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d15,q3,#shift_stage1_idct   @// r7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d3,q12,#shift_stage1_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d14,q11,#shift_stage1_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d6,q14,#shift_stage1_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d11,q9,#shift_stage1_idct   @// r6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d7,q13,#shift_stage1_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d10,q15,#shift_stage1_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)


last4_cols:


    cmp         r12,#0xf0
    bge         skip_last4cols

    vmull.s16   q12,d8,d0[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d8,d0[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d8,d1[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d8,d1[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d0[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d0[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)

    vmull.s16   q9,d5,d1[2]                 @// y2 * sin2 (q4 is freed by this time)(part of d1)
    vmull.s16   q4,d5,d0[2]                 @// y2 * cos2(part of d0)

    vmull.s16   q10,d4,d0[0]                @// y0 * cos4(part of c0 and c1)
    vmull.s16   q11,d12,d0[0]               @// y4 * cos4(part of c0 and c1)

    vmlal.s16   q12,d16,d1[1]               @// y1 * cos1 + y3 * cos3 + y5 * sin3(part of b0)
    vmlsl.s16   q13,d16,d0[1]               @// y1 * cos3 - y3 * sin1 - y5 * cos1(part of b1)
    vmlal.s16   q14,d16,d1[3]               @// y1 * sin3 - y3 * cos1 + y5 * sin1(part of b2)
    vmlal.s16   q15,d16,d0[3]               @// y1 * sin1 - y3 * sin3 + y5 * cos3(part of b3)

    vmlsl.s16   q9,d13,d0[2]                @// d1 = y2 * sin2 - y6 * cos2(part of a0 and a1)
    vmlal.s16   q4,d13,d1[2]                @// d0 = y2 * cos2 + y6 * sin2(part of a0 and a1)

    vadd.s32    q6,q10,q11                  @// c0 = y0 * cos4 + y4 * cos4(part of a0 and a1)
    vsub.s32    q10,q10,q11                 @// c1 = y0 * cos4 - y4 * cos4(part of a0 and a1)

    vmlal.s16   q12,d17,d1[3]               @// b0 = y1 * cos1 + y3 * cos3 + y5 * sin3 + y7 * sin1(part of e0,e7)
    vmlsl.s16   q13,d17,d1[1]               @// b1 = y1 * cos3 - y3 * sin1 - y5 * cos1 - y7 * sin3(part of e1,e6)
    vmlal.s16   q14,d17,d0[3]               @// b2 = y1 * sin3 - y3 * cos1 + y5 * sin1 + y7 * cos3(part of e2,e5)
    vmlsl.s16   q15,d17,d0[1]               @// b3 = y1 * sin1 - y3 * sin3 + y5 * cos3 - y7 * cos1(part of e3,e4)

    vadd.s32    q8,q6,q4                    @// a0 = c0 + d0(part of e0,e7)
    vsub.s32    q6,q6,q4                    @// a3 = c0 - d0(part of e3,e4)
    vsub.s32    q11,q10,q9                  @// a2 = c1 - d1(part of e2,e5)
    vadd.s32    q9,q10,q9                   @// a1 = c1 + d1(part of e1,e6)

    vadd.s32    q10,q8,q12                  @// a0 + b0(part of e0)
    vsub.s32    q4,q8,q12                   @// a0 - b0(part of e7)

    vadd.s32    q12,q11,q14                 @// a2 + b2(part of e2)
    vsub.s32    q11,q11,q14                 @// a2 - b2(part of e5)

    vadd.s32    q14,q9,q13                  @// a1 + b1(part of e1)
    vsub.s32    q9,q9,q13                   @// a1 - b1(part of e6)

    vadd.s32    q13,q6,q15                  @// a3 + b3(part of e3)
    vsub.s32    q15,q6,q15                  @// a3 - b3(part of r4)

    vqrshrn.s32 d4,q10,#shift_stage1_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d17,q4,#shift_stage1_idct   @// r7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d5,q12,#shift_stage1_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d16,q11,#shift_stage1_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d8,q14,#shift_stage1_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d13,q9,#shift_stage1_idct   @// r6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d9,q13,#shift_stage1_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d12,q15,#shift_stage1_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)
    b           end_skip_last4cols



skip_last4cols:






    vtrn.16     q1,q3                       @//[r3,r1],[r2,r0] first qudrant transposing

    vtrn.16     q5,q7                       @//[r7,r5],[r6,r4] third qudrant transposing


    vtrn.32     d6,d7                       @//r0,r1,r2,r3 first qudrant transposing continued.....
    vtrn.32     d2,d3                       @//r0,r1,r2,r3 first qudrant transposing continued.....

    vtrn.32     d10,d11                     @//r4,r5,r6,r7 third qudrant transposing continued.....
    vtrn.32     d14,d15                     @//r4,r5,r6,r7 third qudrant transposing continued.....


    vmull.s16   q12,d6,d0[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d6,d0[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d6,d1[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d6,d1[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d7,d0[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d7,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d7,d0[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d7,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)

    vmull.s16   q10,d2,d0[0]                @// y0 * cos4(part of c0 and c1)
@   vmull.s16   q11,d4,d0[0]                    @// y4 * cos4(part of c0 and c1)

    vmull.s16   q9,d3,d1[2]                 @// y2 * sin2 (q3 is freed by this time)(part of d1)
    vmull.s16   q3,d3,d0[2]                 @// y2 * cos2(part of d0)




    vsub.s32    q11,q10,q3                  @// a3 = c0 - d0(part of r3,r4)
    vadd.s32    q2,q10,q3                   @// a0 = c0 + d0(part of r0,r7)


    vadd.s32    q1,q2,q12

    vsub.s32    q3,q2,q12

    vadd.s32    q4,q11,q15

    vsub.s32    q12,q11,q15

    vqrshrn.s32 d5,q4,#shift_stage2_idct
    vqrshrn.s32 d2,q1,#shift_stage2_idct
    vqrshrn.s32 d9,q3,#shift_stage2_idct
    vqrshrn.s32 d6,q12,#shift_stage2_idct

    vsub.s32    q11,q10,q9                  @// a2 = c1 - d1(part of r2,r5)
    vadd.s32    q9,q10,q9                   @// a1 = c1 + d1(part of r1,r6)


    vadd.s32    q15,q11,q14

    vsub.s32    q12,q11,q14

    vadd.s32    q14,q9,q13

    vsub.s32    q11,q9,q13
    vqrshrn.s32 d4,q15,#shift_stage2_idct
    vqrshrn.s32 d7,q12,#shift_stage2_idct
    vqrshrn.s32 d3,q14,#shift_stage2_idct
    vqrshrn.s32 d8,q11,#shift_stage2_idct










    vmull.s16   q12,d14,d0[1]               @// y1 * cos1(part of b0)

    vmull.s16   q13,d14,d0[3]               @// y1 * cos3(part of b1)
    vmull.s16   q14,d14,d1[1]               @// y1 * sin3(part of b2)
    vmull.s16   q15,d14,d1[3]               @// y1 * sin1(part of b3)

    vmlal.s16   q12,d15,d0[3]               @// y1 * cos1 + y3 * cos3(part of b0)
    vtrn.16     d2,d3
    vmlsl.s16   q13,d15,d1[3]               @// y1 * cos3 - y3 * sin1(part of b1)
    vtrn.16     d4,d5
    vmlsl.s16   q14,d15,d0[1]               @// y1 * sin3 - y3 * cos1(part of b2)
    vtrn.16     d6,d7
    vmlsl.s16   q15,d15,d1[1]               @// y1 * sin1 - y3 * sin3(part of b3)
    vtrn.16     d8,d9
    vmull.s16   q10,d10,d0[0]               @// y0 * cos4(part of c0 and c1)
    vtrn.32     d2,d4

    vtrn.32     d3,d5
    vmull.s16   q9,d11,d1[2]                @// y2 * sin2 (q7 is freed by this time)(part of d1)
    vtrn.32     d6,d8
    vmull.s16   q7,d11,d0[2]                @// y2 * cos2(part of d0)
    vtrn.32     d7,d9


    add         r4,r2,r8, lsl #1            @ r4 = r2 + pred_strd * 2    => r4 points to 3rd row of pred data


    add         r5,r8,r8, lsl #1            @


    add         r0,r3,r7, lsl #1            @ r0 points to 3rd row of dest data


    add         r10,r7,r7, lsl #1           @


    vswp        d3,d6


    vswp        d5,d8


    vsub.s32    q11,q10,q7                  @// a3 = c0 - d0(part of r3,r4)
    vadd.s32    q6,q10,q7                   @// a0 = c0 + d0(part of r0,r7)


    vadd.s32    q0,q6,q12


    vsub.s32    q12,q6,q12


    vadd.s32    q6,q11,q15


    vsub.s32    q7,q11,q15

    vqrshrn.s32 d10,q0,#shift_stage2_idct
    vqrshrn.s32 d17,q12,#shift_stage2_idct
    vqrshrn.s32 d13,q6,#shift_stage2_idct
    vqrshrn.s32 d14,q7,#shift_stage2_idct

    vsub.s32    q11,q10,q9                  @// a2 = c1 - d1(part of r2,r5)
    vadd.s32    q9,q10,q9                   @// a1 = c1 + d1(part of r1,r6)


    vadd.s32    q0,q11,q14


    vsub.s32    q12,q11,q14


    vadd.s32    q14,q9,q13


    vsub.s32    q13,q9,q13
    vld1.8      d18,[r2],r8

    vqrshrn.s32 d12,q0,#shift_stage2_idct
    vld1.8      d20,[r2],r5


    vqrshrn.s32 d15,q12,#shift_stage2_idct
    vld1.8      d19,[r2],r8




    vqrshrn.s32 d11,q14,#shift_stage2_idct
    vld1.8      d22,[r4],r8




    vqrshrn.s32 d16,q13,#shift_stage2_idct
    vld1.8      d21,[r2],r5


    b           pred_buff_addition
end_skip_last4cols:



@/* now the idct of columns is done, transpose so that row idct done efficiently(step5) */
    vtrn.16     q1,q3                       @//[r3,r1],[r2,r0] first qudrant transposing
    vtrn.16     q2,q4                       @//[r3,r1],[r2,r0] second qudrant transposing
    vtrn.16     q5,q7                       @//[r7,r5],[r6,r4] third qudrant transposing
    vtrn.16     q6,q8                       @//[r7,r5],[r6,r4] fourth qudrant transposing

    vtrn.32     d6,d7                       @//r0,r1,r2,r3 first qudrant transposing continued.....
    vtrn.32     d2,d3                       @//r0,r1,r2,r3 first qudrant transposing continued.....
    vtrn.32     d4,d5                       @//r0,r1,r2,r3 second qudrant transposing continued.....
    vtrn.32     d8,d9                       @//r0,r1,r2,r3 second qudrant transposing continued.....
    vtrn.32     d10,d11                     @//r4,r5,r6,r7 third qudrant transposing continued.....
    vtrn.32     d14,d15                     @//r4,r5,r6,r7 third qudrant transposing continued.....
    vtrn.32     d12,d13                     @//r4,r5,r6,r7 fourth qudrant transposing continued.....
    vtrn.32     d16,d17                     @//r4,r5,r6,r7 fourth qudrant transposing continued.....

    @//step6 operate on first four rows and find their idct
    @//register usage.extern        - storing and idct of rows
@// cosine constants    -   d0
@// sine constants      -   d1
@// element 0 first four    -   d2      -   y0
@// element 1 first four    -   d6      -   y1
@// element 2 first four    -   d3      -   y2
@// element 3 first four    -   d7      -   y3
@// element 4 first four    -   d4      -   y4
@// element 5 first four    -   d8      -   y5
@// element 6 first four    -   d5      -   y6
@// element 7 first four    -   d9      -   y7
@// element 0 second four   -   d10     -   y0
@// element 1 second four   -   d14     -   y1
@// element 2 second four   -   d11     -   y2
@// element 3 second four   -   d15     -   y3
@// element 4 second four   -   d12     -   y4
@// element 5 second four   -   d16     -   y5
@// element 6 second four   -   d13     -   y6
@// element 7 second four   -   d17     -   y7

    @// map between first kernel code seq and current
@//     d2  ->  d2
@//     d6  ->  d6
@//     d3  ->  d3
@//     d7  ->  d7
@//     d10 ->  d4
@//     d14 ->  d8
@//     d11 ->  d5
@//     d15 ->  d9
@//     q3  ->  q3
@//     q5  ->  q2
@//     q7  ->  q4

    vmull.s16   q12,d6,d0[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d6,d0[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d6,d1[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d6,d1[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d7,d0[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d7,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d7,d0[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d7,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)

    vmull.s16   q10,d2,d0[0]                @// y0 * cos4(part of c0 and c1)
    vmull.s16   q11,d4,d0[0]                @// y4 * cos4(part of c0 and c1)

    vmull.s16   q9,d3,d1[2]                 @// y2 * sin2 (q3 is freed by this time)(part of d1)
    vmull.s16   q3,d3,d0[2]                 @// y2 * cos2(part of d0)


    vmlal.s16   q12,d8,d1[1]                @// y1 * cos1 + y3 * cos3 + y5 * sin3(part of b0)
    vmlsl.s16   q13,d8,d0[1]                @// y1 * cos3 - y3 * sin1 - y5 * cos1(part of b1)
    vmlal.s16   q14,d8,d1[3]                @// y1 * sin3 - y3 * cos1 + y5 * sin1(part of b2)
    vmlal.s16   q15,d8,d0[3]                @// y1 * sin1 - y3 * sin3 + y5 * cos3(part of b3)

    vmlsl.s16   q9,d5,d0[2]                 @// d1 = y2 * sin2 - y6 * cos2(part of a0 and a1)
    vmlal.s16   q3,d5,d1[2]                 @// d0 = y2 * cos2 + y6 * sin2(part of a0 and a1)

    vadd.s32    q1,q10,q11                  @// c0 = y0 * cos4 + y4 * cos4(part of a0 and a1)
    vsub.s32    q10,q10,q11                 @// c1 = y0 * cos4 - y4 * cos4(part of a0 and a1)

    vmlal.s16   q12,d9,d1[3]                @// b0 = y1 * cos1 + y3 * cos3 + y5 * sin3 + y7 * sin1(part of r0,r7)
    vmlsl.s16   q13,d9,d1[1]                @// b1 = y1 * cos3 - y3 * sin1 - y5 * cos1 - y7 * sin3(part of r1,r6)
    vmlal.s16   q14,d9,d0[3]                @// b2 = y1 * sin3 - y3 * cos1 + y5 * sin1 + y7 * cos3(part of r2,r5)
    vmlsl.s16   q15,d9,d0[1]                @// b3 = y1 * sin1 - y3 * sin3 + y5 * cos3 - y7 * cos1(part of r3,r4)

    vsub.s32    q11,q1,q3                   @// a3 = c0 - d0(part of r3,r4)
    vadd.s32    q2,q1,q3                    @// a0 = c0 + d0(part of r0,r7)


    vadd.s32    q1,q2,q12

    vsub.s32    q3,q2,q12

    vadd.s32    q4,q11,q15

    vsub.s32    q12,q11,q15

    vqrshrn.s32 d5,q4,#shift_stage2_idct
    vqrshrn.s32 d2,q1,#shift_stage2_idct
    vqrshrn.s32 d9,q3,#shift_stage2_idct
    vqrshrn.s32 d6,q12,#shift_stage2_idct

    vsub.s32    q11,q10,q9                  @// a2 = c1 - d1(part of r2,r5)
    vadd.s32    q9,q10,q9                   @// a1 = c1 + d1(part of r1,r6)


    vadd.s32    q15,q11,q14

    vsub.s32    q12,q11,q14

    vadd.s32    q14,q9,q13

    vsub.s32    q11,q9,q13
    vqrshrn.s32 d4,q15,#shift_stage2_idct
    vqrshrn.s32 d7,q12,#shift_stage2_idct
    vqrshrn.s32 d3,q14,#shift_stage2_idct
    vqrshrn.s32 d8,q11,#shift_stage2_idct










    vmull.s16   q12,d14,d0[1]               @// y1 * cos1(part of b0)

    vmull.s16   q13,d14,d0[3]               @// y1 * cos3(part of b1)
    vmull.s16   q14,d14,d1[1]               @// y1 * sin3(part of b2)
    vmull.s16   q15,d14,d1[3]               @// y1 * sin1(part of b3)

    vmlal.s16   q12,d15,d0[3]               @// y1 * cos1 + y3 * cos3(part of b0)
    vtrn.16     d2,d3
    vmlsl.s16   q13,d15,d1[3]               @// y1 * cos3 - y3 * sin1(part of b1)
    vtrn.16     d4,d5
    vmlsl.s16   q14,d15,d0[1]               @// y1 * sin3 - y3 * cos1(part of b2)
    vtrn.16     d6,d7
    vmlsl.s16   q15,d15,d1[1]               @// y1 * sin1 - y3 * sin3(part of b3)
    vtrn.16     d8,d9
    vmull.s16   q10,d10,d0[0]               @// y0 * cos4(part of c0 and c1)
    vtrn.32     d2,d4
    vmull.s16   q11,d12,d0[0]               @// y4 * cos4(part of c0 and c1)
    vtrn.32     d3,d5
    vmull.s16   q9,d11,d1[2]                @// y2 * sin2 (q7 is freed by this time)(part of d1)
    vtrn.32     d6,d8
    vmull.s16   q7,d11,d0[2]                @// y2 * cos2(part of d0)
    vtrn.32     d7,d9
    vmlal.s16   q12,d16,d1[1]               @// y1 * cos1 + y3 * cos3 + y5 * sin3(part of b0)

    add         r4,r2,r8, lsl #1            @ r4 = r2 + pred_strd * 2    => r4 points to 3rd row of pred data
    vmlsl.s16   q13,d16,d0[1]               @// y1 * cos3 - y3 * sin1 - y5 * cos1(part of b1)

    add         r5,r8,r8, lsl #1            @
    vmlal.s16   q14,d16,d1[3]               @// y1 * sin3 - y3 * cos1 + y5 * sin1(part of b2)

    add         r0,r3,r7, lsl #1            @ r0 points to 3rd row of dest data
    vmlal.s16   q15,d16,d0[3]               @// y1 * sin1 - y3 * sin3 + y5 * cos3(part of b3)

    add         r10,r7,r7, lsl #1           @
    vmlsl.s16   q9,d13,d0[2]                @// d1 = y2 * sin2 - y6 * cos2(part of a0 and a1)


    vmlal.s16   q7,d13,d1[2]                @// d0 = y2 * cos2 + y6 * sin2(part of a0 and a1)

    vadd.s32    q6,q10,q11                  @// c0 = y0 * cos4 + y4 * cos4(part of a0 and a1)
    vsub.s32    q10,q10,q11                 @// c1 = y0 * cos4 - y4 * cos4(part of a0 and a1)

    vmlal.s16   q12,d17,d1[3]               @// b0 = y1 * cos1 + y3 * cos3 + y5 * sin3 + y7 * sin1(part of r0,r7)
    vswp        d3,d6
    vmlsl.s16   q13,d17,d1[1]               @// b1 = y1 * cos3 - y3 * sin1 - y5 * cos1 - y7 * sin3(part of r1,r6)

    vswp        d5,d8
    vmlal.s16   q14,d17,d0[3]               @// b2 = y1 * sin3 - y3 * cos1 + y5 * sin1 + y7 * cos3(part of r2,r5)
    vmlsl.s16   q15,d17,d0[1]               @// b3 = y1 * sin1 - y3 * sin3 + y5 * cos3 - y7 * cos1(part of r3,r4)

    vsub.s32    q11,q6,q7                   @// a3 = c0 - d0(part of r3,r4)
    vadd.s32    q6,q6,q7                    @// a0 = c0 + d0(part of r0,r7)


    vadd.s32    q0,q6,q12


    vsub.s32    q12,q6,q12


    vadd.s32    q6,q11,q15


    vsub.s32    q7,q11,q15

    vqrshrn.s32 d10,q0,#shift_stage2_idct
    vqrshrn.s32 d17,q12,#shift_stage2_idct
    vqrshrn.s32 d13,q6,#shift_stage2_idct
    vqrshrn.s32 d14,q7,#shift_stage2_idct

    vsub.s32    q11,q10,q9                  @// a2 = c1 - d1(part of r2,r5)
    vadd.s32    q9,q10,q9                   @// a1 = c1 + d1(part of r1,r6)


    vadd.s32    q0,q11,q14


    vsub.s32    q12,q11,q14


    vadd.s32    q14,q9,q13


    vsub.s32    q13,q9,q13
    vld1.8      d18,[r2],r8

    vqrshrn.s32 d12,q0,#shift_stage2_idct
    vld1.8      d20,[r2],r5


    vqrshrn.s32 d15,q12,#shift_stage2_idct
    vld1.8      d19,[r2],r8




    vqrshrn.s32 d11,q14,#shift_stage2_idct
    vld1.8      d22,[r4],r8




    vqrshrn.s32 d16,q13,#shift_stage2_idct
    vld1.8      d21,[r2],r5




pred_buff_addition:


    vtrn.16     d10,d11
    vld1.8      d24,[r4],r5

    vtrn.16     d12,d13
    vld1.8      d23,[r4],r8

    vaddw.u8    q1,q1,d18
    vld1.8      d25,[r4],r5

    vtrn.16     d14,d15
    vaddw.u8    q2,q2,d22

    vtrn.16     d16,d17
    vaddw.u8    q3,q3,d20

    vtrn.32     d10,d12
    vaddw.u8    q4,q4,d24

    vtrn.32     d11,d13
    vtrn.32     d14,d16
    vtrn.32     d15,d17

    vswp        d11,d14
    vswp        d13,d16

@ row values stored in the q register.

@q1 :r0
@q3: r1
@q2: r2
@q4: r3
@q5: r4
@q7: r5
@q6: r6
@q8: r7



@/// adding the prediction buffer









    @ load prediction data





    @adding recon with prediction





    vaddw.u8    q5,q5,d19
    vqmovun.s16 d2,q1
    vaddw.u8    q7,q7,d21
    vqmovun.s16 d4,q2
    vaddw.u8    q6,q6,d23
    vqmovun.s16 d6,q3
    vaddw.u8    q8,q8,d25
    vqmovun.s16 d8,q4







    vst1.8      {d2},[r3],r7
    vqmovun.s16 d10,q5
    vst1.8      {d6},[r3],r10
    vqmovun.s16 d14,q7
    vst1.8      {d4},[r0],r7
    vqmovun.s16 d12,q6
    vst1.8      {d8},[r0],r10
    vqmovun.s16 d16,q8







    vst1.8      {d10},[r3],r7
    vst1.8      {d14},[r3],r10
    vst1.8      {d12},[r0],r7
    vst1.8      {d16},[r0],r10




    vpop        {d8  -  d15}
    ldmfd       sp!,{r4-r12,pc}





