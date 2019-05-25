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
@ * anand s
@ *
@ * @par list of functions:
@ *  - ihevc_itrans_recon_32x32()
@ *
@ * @remarks
@ *  the input buffer is being corrupted
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
@ *  input 16x16 coefficients
@ *
@ * @param[in] pi2_tmp
@ *  temporary 16x16 buffer for storing inverse
@ *
@ *  transform
@ *  1st stage output
@ *
@ * @param[in] pu1_pred
@ *  prediction 16x16 block
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
@ * @param[in] r12
@ *  zero columns in pi2_src
@ *
@ * @returns  void
@ *
@ * @remarks
@ *  none
@ *
@ *******************************************************************************
@ */

@void ihevc_itrans_recon_32x32(word16 *pi2_src,
@                            word16 *pi2_tmp,
@                            uword8 *pu1_pred,
@                            uword8 *pu1_dst,
@                            word32 src_strd,
@                            word32 pred_strd,
@                            word32 dst_strd,
@                            word32 r12
@                            word32 r11             )

@**************variables vs registers*************************
@   r0 => *pi2_src
@   r1 => *pi2_tmp
@   r2 => *pu1_pred
@   r3 => *pu1_dst
@   src_strd
@   pred_strd
@   dst_strd
@   r12
@   r11


@d0[0]= 64      d2[0]=83
@d0[1]= 90      d2[1]=82
@d0[2]= 90      d2[2]=80
@d0[3]= 90      d2[3]=78
@d1[0]= 89      d3[0]=75
@d1[1]= 88      d3[1]=73
@d1[2]= 87      d3[2]=70
@d1[3]= 85      d3[3]=67

@d4[0]= 64      d6[0]=36
@d4[1]= 61      d6[1]=31
@d4[2]= 57      d6[2]=25
@d4[3]= 54      d6[3]=22
@d5[0]= 50      d7[0]=18
@d5[1]= 46      d7[1]=13
@d5[2]= 43      d7[2]=9
@d5[3]= 38      d7[3]=4

.equ    pi2_src_offset,     64
.equ    pi2_tmp_offset,     68
.equ    src_strd_offset,    120
.equ    pred_strd_offset,   124
.equ    dst_strd_offset,    128
.equ    zero_cols_offset,   132
.equ    zero_rows_offset,   136

.text
.align 4





.set shift_stage1_idct ,   7
.set shift_stage2_idct ,   12

@#define zero_cols   r12
@#define zero_rows   r11

.globl ihevc_itrans_recon_32x32_a9q

.extern g_ai2_ihevc_trans_32_transpose

g_ai2_ihevc_trans_32_transpose_addr:
.long g_ai2_ihevc_trans_32_transpose - ulbl1 - 8

r5_addr: .word 0xfffff000
r9_addr: .word 0xffff0000

.type ihevc_itrans_recon_32x32_a9q, %function

ihevc_itrans_recon_32x32_a9q:

    stmfd       sp!,{r0-r12,lr}
    vpush       {d8  -  d15}

    ldr         r6,[sp,#src_strd_offset]    @ src stride
    ldr         r12,[sp,#zero_cols_offset]
    ldr         r11,[sp,#zero_rows_offset]
    mov         r6,r6,lsl #1                @ x sizeof(word16)
    add         r10,r6,r6, lsl #1           @ 3 rows


    mov         r8,r0

    ldr         r14,g_ai2_ihevc_trans_32_transpose_addr
ulbl1:
    add         r14,r14,pc
    vld1.16     {d0,d1,d2,d3},[r14]!
    vld1.16     {d4,d5,d6,d7},[r14]!

@registers which are free
@  r10,r9,r11,r12
    mov         r9,#0xffffff00
    mov         r10,#0xfffffff0
    ldr         r5,r5_addr
    ldr         r7,r9_addr
    cmp         r12,r10
    movhs       r14,#1
    bhs         stage1


    cmp         r12,r9
    movhs       r14,#2
    bhs         stage1

    cmp         r12,r5
    movhs       r14,#3
    bhs         stage1

    cmp         r12,r7
    movhs       r14,#4

    mov         r14,#8
    b           stage1
@.ltorg


dct_stage1:
    add         r8,r8,#8
    mov         r0,r8

stage1:
    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6

    vmull.s16   q12,d8,d0[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d8,d0[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d8,d1[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d8,d1[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d0[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d9,d2[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d5[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmull.s16   q10,d10,d0[0]
    vmlal.s16   q10,d11,d0[2]


    vmull.s16   q11,d10,d0[0]
    vmlal.s16   q11,d11,d1[2]

    vmull.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d2[2]

    vmull.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d3[2]
    cmp         r11,r10
    bhs         shift1

    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6







    vmlal.s16   q12,d14,d1[1]
    vmlal.s16   q13,d14,d3[3]
    vmlal.s16   q14,d14,d6[1]
    vmlsl.s16   q15,d14,d7[1]


    vmlal.s16   q12,d15,d1[3]
    vmlal.s16   q13,d15,d5[1]
    vmlsl.s16   q14,d15,d7[1]
    vmlsl.s16   q15,d15,d3[3]


    vmlal.s16   q10,d12,d1[0]
    vmlal.s16   q10,d13,d1[2]
    vmlal.s16   q11,d12,d3[0]
    vmlal.s16   q11,d13,d4[2]
    vmlal.s16   q8,d12,d5[0]
    vmlal.s16   q8,d13,d7[2]
    vmlal.s16   q9,d12,d7[0]
    vmlsl.s16   q9,d13,d5[2]

    cmp         r11,r9
    bhs         shift1

    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6


    vmlal.s16   q12,d8,d2[1]                @// y1 * cos1(part of b0)
    vmlal.s16   q13,d8,d6[3]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d4[3]                @// y1 * sin3(part of b2)
    vmlsl.s16   q15,d8,d0[1]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d2[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d7[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d2[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d3[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d2[0]
    vmlal.s16   q10,d11,d2[2]


    vmlal.s16   q11,d10,d6[0]
    vmlal.s16   q11,d11,d7[2]

    vmlsl.s16   q8,d10,d6[0]
    vmlsl.s16   q8,d11,d3[2]

    vmlsl.s16   q9,d10,d2[0]
    vmlsl.s16   q9,d11,d1[2]

    cmp         r11,r5
    bhs         shift1


    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6









    vmlal.s16   q12,d14,d3[1]
    vmlsl.s16   q13,d14,d6[1]
    vmlsl.s16   q14,d14,d0[1]
    vmlsl.s16   q15,d14,d6[3]


    vmlal.s16   q12,d15,d3[3]
    vmlsl.s16   q13,d15,d4[3]
    vmlsl.s16   q14,d15,d2[3]
    vmlal.s16   q15,d15,d5[3]


    vmlal.s16   q10,d12,d3[0]
    vmlal.s16   q10,d13,d3[2]
    vmlsl.s16   q11,d12,d7[0]
    vmlsl.s16   q11,d13,d5[2]
    vmlsl.s16   q8,d12,d1[0]
    vmlsl.s16   q8,d13,d1[2]
    vmlsl.s16   q9,d12,d5[0]
    vmlal.s16   q9,d13,d7[2]

    cmp         r11,r7
    bhs         shift1


    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6



    vmlal.s16   q12,d8,d4[1]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d3[1]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d5[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d2[1]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d4[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d7[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d0[0]
    vmlal.s16   q10,d11,d4[2]


    vmlsl.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d2[2]

    vmlsl.s16   q8,d10,d0[0]
    vmlsl.s16   q8,d11,d6[2]

    vmlal.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d0[2]



    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6




    vmlal.s16   q12,d14,d5[1]
    vmlsl.s16   q13,d14,d0[2]
    vmlal.s16   q14,d14,d5[3]
    vmlal.s16   q15,d14,d4[3]


    vmlal.s16   q12,d15,d5[3]
    vmlsl.s16   q13,d15,d1[1]
    vmlal.s16   q14,d15,d3[1]
    vmlsl.s16   q15,d15,d7[3]


    vmlal.s16   q10,d12,d5[0]
    vmlal.s16   q10,d13,d5[2]
    vmlsl.s16   q11,d12,d1[0]
    vmlsl.s16   q11,d13,d0[2]
    vmlal.s16   q8,d12,d7[0]
    vmlal.s16   q8,d13,d4[2]
    vmlal.s16   q9,d12,d3[0]
    vmlal.s16   q9,d13,d6[2]


    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6







    vmlal.s16   q12,d8,d6[1]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d2[3]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d0[1]                @// y1 * sin3(part of b2)
    vmlsl.s16   q15,d8,d4[1]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d6[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d4[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d1[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d0[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d6[0]
    vmlal.s16   q10,d11,d6[2]


    vmlsl.s16   q11,d10,d2[0]
    vmlsl.s16   q11,d11,d3[2]

    vmlal.s16   q8,d10,d2[0]
    vmlal.s16   q8,d11,d0[2]

    vmlsl.s16   q9,d10,d6[0]
    vmlsl.s16   q9,d11,d2[2]

    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6


    vmlal.s16   q12,d14,d7[1]
    vmlsl.s16   q13,d14,d5[3]
    vmlal.s16   q14,d14,d4[1]
    vmlsl.s16   q15,d14,d2[3]


    vmlal.s16   q12,d15,d7[3]
    vmlsl.s16   q13,d15,d7[1]
    vmlal.s16   q14,d15,d6[3]
    vmlsl.s16   q15,d15,d6[1]


    vmlal.s16   q10,d12,d7[0]
    vmlal.s16   q10,d13,d7[2]
    vmlsl.s16   q11,d12,d5[0]
    vmlsl.s16   q11,d13,d6[2]
    vmlal.s16   q8,d12,d3[0]
    vmlal.s16   q8,d13,d5[2]
    vmlsl.s16   q9,d12,d1[0]
    vmlsl.s16   q9,d13,d4[2]



shift1:
    vadd.s32    q4,q10,q12
    vsub.s32    q5,q10,q12

    vadd.s32    q6,q11,q13
    vsub.s32    q12,q11,q13

    vadd.s32    q7,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15


    vqrshrn.s32 d30,q4,#shift_stage1_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d19,q5,#shift_stage1_idct   @// r7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d31,q7,#shift_stage1_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d18,q13,#shift_stage1_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d12,q6,#shift_stage1_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d15,q12,#shift_stage1_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d13,q8,#shift_stage1_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d14,q14,#shift_stage1_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)


    @ registers used q15,q14,q6,q7


    vtrn.16     q15,q6
    vtrn.16     q7,q9

    vtrn.32     d30,d31
    vtrn.32     d12,d13
    vtrn.32     d14,d15
    vtrn.32     d18,d19


@ d30 =r0 1- 4 values
@ d31 =r2 1- 4 values
@ d12=r1 1- 4 values
@ d13=r3 1- 4 values
@ d14 =r0 28-31 values
@ d15 =r2 28- 31 values
@ d18=r1 28- 31 values
@ d19=r3 28- 31 values



    vst1.16     {q15},[r1]!
    vst1.16     {q6},[r1]!
    add         r1,r1,#192
    vst1.16     {q7},[r1]!
    vst1.16     {q9},[r1]!
    sub         r1,r1,#224

    mov         r0,r8





    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6




    vmull.s16   q12,d8,d2[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d8,d2[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d8,d3[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d8,d3[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d6[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d7[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d6[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d4[3]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmull.s16   q10,d10,d0[0]
    vmlal.s16   q10,d11,d4[2]


    vmull.s16   q11,d10,d0[0]
    vmlal.s16   q11,d11,d5[2]

    vmull.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d6[2]

    vmull.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d7[2]
    cmp         r11,r10
    bhs         shift2

    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6


    vmlsl.s16   q12,d14,d4[3]
    vmlsl.s16   q13,d14,d2[1]
    vmlsl.s16   q14,d14,d0[1]
    vmlsl.s16   q15,d14,d2[3]


    vmlsl.s16   q12,d15,d0[3]
    vmlsl.s16   q13,d15,d3[1]
    vmlsl.s16   q14,d15,d6[3]
    vmlal.s16   q15,d15,d5[3]


    vmlsl.s16   q10,d12,d7[0]
    vmlsl.s16   q10,d13,d2[2]
    vmlsl.s16   q11,d12,d5[0]
    vmlsl.s16   q11,d13,d0[2]
    vmlsl.s16   q8,d12,d3[0]
    vmlsl.s16   q8,d13,d3[2]
    vmlsl.s16   q9,d12,d1[0]
    vmlsl.s16   q9,d13,d6[2]

    cmp         r11,r9
    bhs         shift2


    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6







    vmlsl.s16   q12,d8,d4[1]                @// y1 * cos1(part of b0)
    vmlal.s16   q13,d8,d7[1]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d2[3]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d1[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d7[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d9,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d6[3]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlsl.s16   q10,d10,d2[0]
    vmlsl.s16   q10,d11,d6[2]


    vmlsl.s16   q11,d10,d6[0]
    vmlal.s16   q11,d11,d4[2]

    vmlal.s16   q8,d10,d6[0]
    vmlal.s16   q8,d11,d0[2]

    vmlal.s16   q9,d10,d2[0]
    vmlal.s16   q9,d11,d5[2]

    cmp         r11,r5
    bhs         shift2


    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6





    vmlal.s16   q12,d14,d2[3]
    vmlal.s16   q13,d14,d3[3]
    vmlsl.s16   q14,d14,d5[3]
    vmlsl.s16   q15,d14,d0[3]


    vmlal.s16   q12,d15,d1[3]
    vmlsl.s16   q13,d15,d6[3]
    vmlsl.s16   q14,d15,d0[3]
    vmlal.s16   q15,d15,d7[3]


    vmlal.s16   q10,d12,d5[0]
    vmlal.s16   q10,d13,d0[2]
    vmlal.s16   q11,d12,d1[0]
    vmlal.s16   q11,d13,d6[2]
    vmlal.s16   q8,d12,d7[0]
    vmlsl.s16   q8,d13,d2[2]
    vmlsl.s16   q9,d12,d3[0]
    vmlsl.s16   q9,d13,d4[2]


    cmp         r11,r7
    bhs         shift2


    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6







    vmlal.s16   q12,d8,d6[1]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d1[1]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d7[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d0[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d5[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d4[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d2[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d7[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d7[2]


    vmlsl.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d1[2]

    vmlsl.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d5[2]

    vmlal.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d3[2]



    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6


    vmlsl.s16   q12,d14,d0[1]
    vmlal.s16   q13,d14,d6[1]
    vmlal.s16   q14,d14,d4[1]
    vmlsl.s16   q15,d14,d1[1]


    vmlsl.s16   q12,d15,d3[3]
    vmlal.s16   q13,d15,d0[1]
    vmlsl.s16   q14,d15,d5[1]
    vmlsl.s16   q15,d15,d6[1]


    vmlsl.s16   q10,d12,d3[0]
    vmlsl.s16   q10,d13,d1[2]
    vmlsl.s16   q11,d12,d7[0]
    vmlal.s16   q11,d13,d3[2]
    vmlal.s16   q8,d12,d1[0]
    vmlal.s16   q8,d13,d7[2]
    vmlsl.s16   q9,d12,d5[0]
    vmlsl.s16   q9,d13,d2[2]

    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6




    vmlal.s16   q12,d8,d7[3]                @// y1 * cos1(part of b0)
    vmlal.s16   q13,d8,d4[3]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d1[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d2[1]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d3[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d5[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d7[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d5[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlsl.s16   q10,d10,d6[0]
    vmlal.s16   q10,d11,d5[2]


    vmlal.s16   q11,d10,d2[0]
    vmlal.s16   q11,d11,d7[2]

    vmlsl.s16   q8,d10,d2[0]
    vmlsl.s16   q8,d11,d4[2]

    vmlal.s16   q9,d10,d6[0]
    vmlal.s16   q9,d11,d1[2]


    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6





    vmlal.s16   q12,d14,d1[1]
    vmlsl.s16   q13,d14,d0[3]
    vmlal.s16   q14,d14,d1[3]
    vmlsl.s16   q15,d14,d3[1]


    vmlal.s16   q12,d15,d5[3]
    vmlsl.s16   q13,d15,d5[1]
    vmlal.s16   q14,d15,d4[3]
    vmlsl.s16   q15,d15,d4[1]


    vmlal.s16   q10,d12,d1[0]
    vmlal.s16   q10,d13,d3[2]
    vmlsl.s16   q11,d12,d3[0]
    vmlsl.s16   q11,d13,d2[2]
    vmlal.s16   q8,d12,d5[0]
    vmlal.s16   q8,d13,d1[2]
    vmlsl.s16   q9,d12,d7[0]
    vmlsl.s16   q9,d13,d0[2]

shift2:
    vadd.s32    q4,q10,q12
    vsub.s32    q5,q10,q12

    vadd.s32    q6,q11,q13
    vsub.s32    q12,q11,q13

    vadd.s32    q7,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15


    vqrshrn.s32 d30,q4,#shift_stage1_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d19,q5,#shift_stage1_idct   @// r7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d31,q7,#shift_stage1_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d18,q13,#shift_stage1_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d12,q6,#shift_stage1_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d15,q12,#shift_stage1_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d13,q8,#shift_stage1_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d14,q14,#shift_stage1_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    vtrn.16     q15,q6
    vtrn.16     q7,q9

    vtrn.32     d30,d31
    vtrn.32     d12,d13
    vtrn.32     d14,d15
    vtrn.32     d18,d19


    vst1.16     {q15},[r1]!
    vst1.16     {q6},[r1]!
    add         r1,r1,#128
    vst1.16     {q7},[r1]!
    vst1.16     {q9},[r1]!
    sub         r1,r1,#160
    mov         r0,r8



    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6


    vmull.s16   q12,d8,d4[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d8,d4[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d8,d5[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d8,d5[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d3[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d0[2]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmull.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d7[2]


    vmull.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d6[2]

    vmull.s16   q8,d10,d0[0]
    vmlsl.s16   q8,d11,d5[2]

    vmull.s16   q9,d10,d0[0]
    vmlsl.s16   q9,d11,d4[2]

    cmp         r11,r10
    bhs         shift3

    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6




    vmlsl.s16   q12,d14,d5[1]
    vmlsl.s16   q13,d14,d7[3]
    vmlal.s16   q14,d14,d5[3]
    vmlal.s16   q15,d14,d3[1]


    vmlal.s16   q12,d15,d2[1]
    vmlal.s16   q13,d15,d1[1]
    vmlal.s16   q14,d15,d4[3]
    vmlsl.s16   q15,d15,d7[3]


    vmlsl.s16   q10,d12,d1[0]
    vmlal.s16   q10,d13,d6[2]
    vmlsl.s16   q11,d12,d3[0]
    vmlal.s16   q11,d13,d3[2]
    vmlsl.s16   q8,d12,d5[0]
    vmlal.s16   q8,d13,d0[2]
    vmlsl.s16   q9,d12,d7[0]
    vmlal.s16   q9,d13,d2[2]

    cmp         r11,r9
    bhs         shift3

    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6

    vmlal.s16   q12,d8,d6[1]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d5[1]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d0[3]                @// y1 * sin3(part of b2)
    vmlsl.s16   q15,d8,d3[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d1[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d4[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d6[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d0[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d2[0]
    vmlsl.s16   q10,d11,d5[2]


    vmlal.s16   q11,d10,d6[0]
    vmlsl.s16   q11,d11,d0[2]

    vmlsl.s16   q8,d10,d6[0]
    vmlsl.s16   q8,d11,d4[2]

    vmlsl.s16   q9,d10,d2[0]
    vmlal.s16   q9,d11,d6[2]

    cmp         r11,r5
    bhs         shift3


    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6






    vmlsl.s16   q12,d14,d7[1]
    vmlal.s16   q13,d14,d2[1]
    vmlal.s16   q14,d14,d4[1]
    vmlsl.s16   q15,d14,d5[1]


    vmlal.s16   q12,d15,d0[3]
    vmlal.s16   q13,d15,d7[1]
    vmlsl.s16   q14,d15,d1[1]
    vmlsl.s16   q15,d15,d6[1]


    vmlsl.s16   q10,d12,d3[0]
    vmlal.s16   q10,d13,d4[2]
    vmlal.s16   q11,d12,d7[0]
    vmlal.s16   q11,d13,d2[2]
    vmlal.s16   q8,d12,d1[0]
    vmlsl.s16   q8,d13,d6[2]
    vmlal.s16   q9,d12,d5[0]
    vmlsl.s16   q9,d13,d0[2]


    cmp         r11,r7
    bhs         shift3


    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6


    vmlsl.s16   q12,d8,d7[3]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d0[1]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d6[3]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d1[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d0[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d9,d5[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d2[3]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d3[2]


    vmlsl.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d5[2]

    vmlsl.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d1[2]

    vmlal.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d7[2]


    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6



    vmlal.s16   q12,d14,d6[3]
    vmlal.s16   q13,d14,d3[3]
    vmlsl.s16   q14,d14,d1[3]
    vmlal.s16   q15,d14,d7[1]


    vmlal.s16   q12,d15,d1[3]
    vmlsl.s16   q13,d15,d2[3]
    vmlal.s16   q14,d15,d7[1]
    vmlal.s16   q15,d15,d4[1]


    vmlsl.s16   q10,d12,d5[0]
    vmlal.s16   q10,d13,d2[2]
    vmlal.s16   q11,d12,d1[0]
    vmlsl.s16   q11,d13,d7[2]
    vmlsl.s16   q8,d12,d7[0]
    vmlsl.s16   q8,d13,d3[2]
    vmlsl.s16   q9,d12,d3[0]
    vmlal.s16   q9,d13,d1[2]



    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6




    vmlsl.s16   q12,d8,d5[3]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d6[3]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d3[1]                @// y1 * sin3(part of b2)
    vmlsl.s16   q15,d8,d0[1]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d2[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d9,d0[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d2[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d4[3]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d6[0]
    vmlsl.s16   q10,d11,d1[2]


    vmlsl.s16   q11,d10,d2[0]
    vmlal.s16   q11,d11,d4[2]

    vmlal.s16   q8,d10,d2[0]
    vmlsl.s16   q8,d11,d7[2]

    vmlsl.s16   q9,d10,d6[0]
    vmlsl.s16   q9,d11,d5[2]


    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6

    vmlal.s16   q12,d14,d4[3]
    vmlsl.s16   q13,d14,d6[1]
    vmlal.s16   q14,d14,d7[3]
    vmlal.s16   q15,d14,d6[3]


    vmlal.s16   q12,d15,d3[3]
    vmlsl.s16   q13,d15,d3[1]
    vmlal.s16   q14,d15,d2[3]
    vmlsl.s16   q15,d15,d2[1]


    vmlsl.s16   q10,d12,d7[0]
    vmlal.s16   q10,d13,d0[2]
    vmlal.s16   q11,d12,d5[0]
    vmlsl.s16   q11,d13,d1[2]
    vmlsl.s16   q8,d12,d3[0]
    vmlal.s16   q8,d13,d2[2]
    vmlal.s16   q9,d12,d1[0]
    vmlsl.s16   q9,d13,d3[2]

shift3:
    vadd.s32    q4,q10,q12
    vsub.s32    q5,q10,q12

    vadd.s32    q6,q11,q13
    vsub.s32    q12,q11,q13

    vadd.s32    q7,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15


    vqrshrn.s32 d30,q4,#shift_stage1_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d19,q5,#shift_stage1_idct   @// r7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d31,q7,#shift_stage1_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d18,q13,#shift_stage1_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d12,q6,#shift_stage1_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d15,q12,#shift_stage1_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d13,q8,#shift_stage1_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d14,q14,#shift_stage1_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    vtrn.16     q15,q6
    vtrn.16     q7,q9

    vtrn.32     d30,d31
    vtrn.32     d12,d13
    vtrn.32     d14,d15
    vtrn.32     d18,d19


    vst1.16     {q15},[r1]!
    vst1.16     {q6},[r1]!
    add         r1,r1,#64
    vst1.16     {q7},[r1]!
    vst1.16     {q9},[r1]!
    sub         r1,r1,#96

    mov         r0,r8



    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6


    vmull.s16   q12,d8,d6[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d8,d6[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d8,d7[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d8,d7[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d2[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d4[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d5[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d7[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmull.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d3[2]


    vmull.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d2[2]

    vmull.s16   q8,d10,d0[0]
    vmlsl.s16   q8,d11,d1[2]

    vmull.s16   q9,d10,d0[0]
    vmlsl.s16   q9,d11,d0[2]

    cmp         r11,r10
    bhs         shift4

    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6






    vmlal.s16   q12,d14,d0[1]
    vmlal.s16   q13,d14,d1[3]
    vmlal.s16   q14,d14,d4[1]
    vmlal.s16   q15,d14,d6[3]


    vmlsl.s16   q12,d15,d4[1]
    vmlsl.s16   q13,d15,d0[3]
    vmlsl.s16   q14,d15,d2[3]
    vmlsl.s16   q15,d15,d6[1]


    vmlal.s16   q10,d12,d7[0]
    vmlal.s16   q10,d13,d5[2]
    vmlal.s16   q11,d12,d5[0]
    vmlsl.s16   q11,d13,d7[2]
    vmlal.s16   q8,d12,d3[0]
    vmlsl.s16   q8,d13,d4[2]
    vmlal.s16   q9,d12,d1[0]
    vmlsl.s16   q9,d13,d1[2]

    cmp         r11,r9
    bhs         shift4

    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6



    vmlal.s16   q12,d8,d7[3]                @// y1 * cos1(part of b0)
    vmlal.s16   q13,d8,d3[1]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d1[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d5[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d4[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d5[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d0[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d5[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlsl.s16   q10,d10,d2[0]
    vmlal.s16   q10,d11,d1[2]


    vmlsl.s16   q11,d10,d6[0]
    vmlal.s16   q11,d11,d3[2]

    vmlal.s16   q8,d10,d6[0]
    vmlsl.s16   q8,d11,d7[2]

    vmlal.s16   q9,d10,d2[0]
    vmlsl.s16   q9,d11,d2[2]

    cmp         r11,r5
    bhs         shift4


    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6






    vmlsl.s16   q12,d14,d1[1]
    vmlsl.s16   q13,d14,d7[3]
    vmlal.s16   q14,d14,d1[3]
    vmlal.s16   q15,d14,d4[3]


    vmlal.s16   q12,d15,d2[1]
    vmlal.s16   q13,d15,d5[1]
    vmlsl.s16   q14,d15,d3[1]
    vmlsl.s16   q15,d15,d4[1]


    vmlsl.s16   q10,d12,d5[0]
    vmlsl.s16   q10,d13,d7[2]
    vmlsl.s16   q11,d12,d1[0]
    vmlal.s16   q11,d13,d1[2]
    vmlsl.s16   q8,d12,d7[0]
    vmlal.s16   q8,d13,d5[2]
    vmlal.s16   q9,d12,d3[0]
    vmlsl.s16   q9,d13,d3[2]

    cmp         r11,r7
    bhs         shift4


    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6


    vmlsl.s16   q12,d8,d5[3]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d2[3]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d4[3]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d3[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d6[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d9,d0[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d6[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d3[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d0[2]


    vmlsl.s16   q11,d10,d0[0]
    vmlal.s16   q11,d11,d6[2]

    vmlsl.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d2[2]

    vmlal.s16   q9,d10,d0[0]
    vmlsl.s16   q9,d11,d4[2]




    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6






    vmlal.s16   q12,d14,d3[1]
    vmlsl.s16   q13,d14,d2[1]
    vmlal.s16   q14,d14,d7[3]
    vmlal.s16   q15,d14,d2[3]


    vmlsl.s16   q12,d15,d0[3]
    vmlal.s16   q13,d15,d4[3]
    vmlal.s16   q14,d15,d6[3]
    vmlsl.s16   q15,d15,d2[1]


    vmlal.s16   q10,d12,d3[0]
    vmlsl.s16   q10,d13,d6[2]
    vmlal.s16   q11,d12,d7[0]
    vmlsl.s16   q11,d13,d4[2]
    vmlsl.s16   q8,d12,d1[0]
    vmlal.s16   q8,d13,d0[2]
    vmlal.s16   q9,d12,d5[0]
    vmlsl.s16   q9,d13,d5[2]


    vld1.16     d10,[r0],r6
    vld1.16     d8,[r0],r6
    vld1.16     d11,[r0],r6
    vld1.16     d9,[r0],r6





    vmlal.s16   q12,d8,d3[3]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d7[1]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d5[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d1[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d7[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d6[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlsl.s16   q10,d10,d6[0]
    vmlal.s16   q10,d11,d2[2]


    vmlal.s16   q11,d10,d2[0]
    vmlsl.s16   q11,d11,d0[2]

    vmlsl.s16   q8,d10,d2[0]
    vmlal.s16   q8,d11,d3[2]

    vmlal.s16   q9,d10,d6[0]
    vmlsl.s16   q9,d11,d6[2]


    vld1.16     d12,[r0],r6
    vld1.16     d14,[r0],r6
    vld1.16     d13,[r0],r6
    vld1.16     d15,[r0],r6




    vmlsl.s16   q12,d14,d5[1]
    vmlal.s16   q13,d14,d3[3]
    vmlsl.s16   q14,d14,d2[1]
    vmlal.s16   q15,d14,d0[3]


    vmlal.s16   q12,d15,d1[3]
    vmlsl.s16   q13,d15,d1[1]
    vmlal.s16   q14,d15,d0[3]
    vmlsl.s16   q15,d15,d0[1]


    vmlsl.s16   q10,d12,d1[0]
    vmlal.s16   q10,d13,d4[2]
    vmlal.s16   q11,d12,d3[0]
    vmlsl.s16   q11,d13,d5[2]
    vmlsl.s16   q8,d12,d5[0]
    vmlal.s16   q8,d13,d6[2]
    vmlal.s16   q9,d12,d7[0]
    vmlsl.s16   q9,d13,d7[2]

shift4:
    vadd.s32    q4,q10,q12
    vsub.s32    q5,q10,q12

    vadd.s32    q6,q11,q13
    vsub.s32    q12,q11,q13

    vadd.s32    q7,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15


    vqrshrn.s32 d30,q4,#shift_stage1_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d19,q5,#shift_stage1_idct   @// r7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d31,q7,#shift_stage1_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d18,q13,#shift_stage1_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d12,q6,#shift_stage1_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d15,q12,#shift_stage1_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d13,q8,#shift_stage1_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d14,q14,#shift_stage1_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    vtrn.16     q15,q6
    vtrn.16     q7,q9

    vtrn.32     d30,d31
    vtrn.32     d12,d13
    vtrn.32     d14,d15
    vtrn.32     d18,d19


    vst1.16     {q15},[r1]!
    vst1.16     {q6},[r1]!
    vst1.16     {q7},[r1]!
    vst1.16     {q9},[r1]!

    add         r1,r1,#96

    subs        r14,r14,#1
    bne         dct_stage1
second_stage_dct:
@   mov     r0,r1
    ldr         r0,[sp,#pi2_src_offset]
    ldr         r1,[sp,#pi2_tmp_offset]
    ldr         r8,[sp,#pred_strd_offset]   @ prediction stride
    ldr         r7,[sp,#dst_strd_offset]    @ destination stride

@   add r4,r2,r8, lsl #1    @ r4 = r2 + pred_strd * 2    => r4 points to 3rd row of pred data
@   add r5,r8,r8, lsl #1    @
@   sub   r0,r0,#512
    mov         r11,#0xfffffff0
    mov         r5, #0xffffff00
    ldr         r6,r5_addr
    ldr         r9,r9_addr
@   sub     r1,r1,#2048
    mov         r4,r1
    mov         r10,#240
    mov         r14,#8
    b           stage2

@ registers free :

@ arm registers used
@ r8 : predicition stride
@ r7 : destination stride
@ r1: temp buffer
@ r2 : pred buffer
@ r3 : destination buffer
@ r14 : loop counter
@r0 : scratch buffer
@r10 : used as stride
@ r4 : used to store the initial address
@r12 : zero cols
@ r11 : 0xfffffff0
@ r5 : 0xffffff00
dct_stage2:
    add         r4,r4,#32
    mov         r1,r4
stage2:
    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10

    vmull.s16   q12,d8,d0[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d8,d0[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d8,d1[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d8,d1[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d0[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d9,d2[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d5[1]                @// y1 * sin1 - y3 * sin3(part of b3)



    vmull.s16   q10,d10,d0[0]
    vmlal.s16   q10,d11,d0[2]


    vmull.s16   q11,d10,d0[0]
    vmlal.s16   q11,d11,d1[2]

    vmull.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d2[2]

    vmull.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d3[2]
    cmp         r12,r11
    bhs         stage2_shift1

    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10






    vmlal.s16   q12,d14,d1[1]
    vmlal.s16   q13,d14,d3[3]
    vmlal.s16   q14,d14,d6[1]
    vmlsl.s16   q15,d14,d7[1]


    vmlal.s16   q12,d15,d1[3]
    vmlal.s16   q13,d15,d5[1]
    vmlsl.s16   q14,d15,d7[1]
    vmlsl.s16   q15,d15,d3[3]


    vmlal.s16   q10,d12,d1[0]
    vmlal.s16   q10,d13,d1[2]
    vmlal.s16   q11,d12,d3[0]
    vmlal.s16   q11,d13,d4[2]
    vmlal.s16   q8,d12,d5[0]
    vmlal.s16   q8,d13,d7[2]
    vmlal.s16   q9,d12,d7[0]
    vmlsl.s16   q9,d13,d5[2]
    cmp         r12,r5
    bhs         stage2_shift1

    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10

    vmlal.s16   q12,d8,d2[1]                @// y1 * cos1(part of b0)
    vmlal.s16   q13,d8,d6[3]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d4[3]                @// y1 * sin3(part of b2)
    vmlsl.s16   q15,d8,d0[1]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d2[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d7[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d2[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d3[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d2[0]
    vmlal.s16   q10,d11,d2[2]


    vmlal.s16   q11,d10,d6[0]
    vmlal.s16   q11,d11,d7[2]

    vmlsl.s16   q8,d10,d6[0]
    vmlsl.s16   q8,d11,d3[2]

    vmlsl.s16   q9,d10,d2[0]
    vmlsl.s16   q9,d11,d1[2]

    cmp         r12,r6
    bhs         stage2_shift1


    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10





    vmlal.s16   q12,d14,d3[1]
    vmlsl.s16   q13,d14,d6[1]
    vmlsl.s16   q14,d14,d0[1]
    vmlsl.s16   q15,d14,d6[3]


    vmlal.s16   q12,d15,d3[3]
    vmlsl.s16   q13,d15,d4[3]
    vmlsl.s16   q14,d15,d2[3]
    vmlal.s16   q15,d15,d5[3]


    vmlal.s16   q10,d12,d3[0]
    vmlal.s16   q10,d13,d3[2]
    vmlsl.s16   q11,d12,d7[0]
    vmlsl.s16   q11,d13,d5[2]
    vmlsl.s16   q8,d12,d1[0]
    vmlsl.s16   q8,d13,d1[2]
    vmlsl.s16   q9,d12,d5[0]
    vmlal.s16   q9,d13,d7[2]

    cmp         r12,r9
    bhs         stage2_shift1


    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10


    vmlal.s16   q12,d8,d4[1]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d3[1]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d5[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d2[1]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d4[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d7[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d0[0]
    vmlal.s16   q10,d11,d4[2]


    vmlsl.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d2[2]

    vmlsl.s16   q8,d10,d0[0]
    vmlsl.s16   q8,d11,d6[2]

    vmlal.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d0[2]

    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10





    vmlal.s16   q12,d14,d5[1]
    vmlsl.s16   q13,d14,d0[2]
    vmlal.s16   q14,d14,d5[3]
    vmlal.s16   q15,d14,d4[3]


    vmlal.s16   q12,d15,d5[3]
    vmlsl.s16   q13,d15,d1[1]
    vmlal.s16   q14,d15,d3[1]
    vmlsl.s16   q15,d15,d7[3]


    vmlal.s16   q10,d12,d5[0]
    vmlal.s16   q10,d13,d5[2]
    vmlsl.s16   q11,d12,d1[0]
    vmlsl.s16   q11,d13,d0[2]
    vmlal.s16   q8,d12,d7[0]
    vmlal.s16   q8,d13,d4[2]
    vmlal.s16   q9,d12,d3[0]
    vmlal.s16   q9,d13,d6[2]


    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10




    vmlal.s16   q12,d8,d6[1]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d2[3]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d0[1]                @// y1 * sin3(part of b2)
    vmlsl.s16   q15,d8,d4[1]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d6[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d4[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d1[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d0[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d6[0]
    vmlal.s16   q10,d11,d6[2]


    vmlsl.s16   q11,d10,d2[0]
    vmlsl.s16   q11,d11,d3[2]

    vmlal.s16   q8,d10,d2[0]
    vmlal.s16   q8,d11,d0[2]

    vmlsl.s16   q9,d10,d6[0]
    vmlsl.s16   q9,d11,d2[2]

    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10

    vmlal.s16   q12,d14,d7[1]
    vmlsl.s16   q13,d14,d5[3]
    vmlal.s16   q14,d14,d4[1]
    vmlsl.s16   q15,d14,d2[3]


    vmlal.s16   q12,d15,d7[3]
    vmlsl.s16   q13,d15,d7[1]
    vmlal.s16   q14,d15,d6[3]
    vmlsl.s16   q15,d15,d6[1]


    vmlal.s16   q10,d12,d7[0]
    vmlal.s16   q10,d13,d7[2]
    vmlsl.s16   q11,d12,d5[0]
    vmlsl.s16   q11,d13,d6[2]
    vmlal.s16   q8,d12,d3[0]
    vmlal.s16   q8,d13,d5[2]
    vmlsl.s16   q9,d12,d1[0]
    vmlsl.s16   q9,d13,d4[2]

stage2_shift1:
    vadd.s32    q4,q10,q12
    vsub.s32    q5,q10,q12

    vadd.s32    q6,q11,q13
    vsub.s32    q12,q11,q13

    vadd.s32    q7,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15


    vqrshrn.s32 d30,q4,#shift_stage2_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d19,q5,#shift_stage2_idct   @// r7 = (a0 - b0 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d31,q7,#shift_stage2_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d18,q13,#shift_stage2_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d12,q6,#shift_stage2_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d15,q12,#shift_stage2_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d13,q8,#shift_stage2_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d14,q14,#shift_stage2_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage2_idct)


    vtrn.16     q15,q6
    vtrn.16     q7,q9

    vtrn.32     d30,d31
    vtrn.32     d12,d13
    vtrn.32     d14,d15
    vtrn.32     d18,d19


    vst1.16     {q15},[r0]!
    vst1.16     {q6},[r0]!
    vst1.16     {q7},[r0]!
    vst1.16     {q9},[r0]!


    mov         r1,r4






    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10


    vmull.s16   q12,d8,d2[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d8,d2[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d8,d3[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d8,d3[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d6[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d7[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d6[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d4[3]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmull.s16   q10,d10,d0[0]
    vmlal.s16   q10,d11,d4[2]


    vmull.s16   q11,d10,d0[0]
    vmlal.s16   q11,d11,d5[2]

    vmull.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d6[2]

    vmull.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d7[2]

    cmp         r12,r11
    bhs         stage2_shift2

    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10


    vmlsl.s16   q12,d14,d4[3]
    vmlsl.s16   q13,d14,d2[1]
    vmlsl.s16   q14,d14,d0[1]
    vmlsl.s16   q15,d14,d2[3]


    vmlsl.s16   q12,d15,d0[3]
    vmlsl.s16   q13,d15,d3[1]
    vmlsl.s16   q14,d15,d6[3]
    vmlal.s16   q15,d15,d5[3]


    vmlsl.s16   q10,d12,d7[0]
    vmlsl.s16   q10,d13,d2[2]
    vmlsl.s16   q11,d12,d5[0]
    vmlsl.s16   q11,d13,d0[2]
    vmlsl.s16   q8,d12,d3[0]
    vmlsl.s16   q8,d13,d3[2]
    vmlsl.s16   q9,d12,d1[0]
    vmlsl.s16   q9,d13,d6[2]

    cmp         r12,r5
    bhs         stage2_shift2

    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10





    vmlsl.s16   q12,d8,d4[1]                @// y1 * cos1(part of b0)
    vmlal.s16   q13,d8,d7[1]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d2[3]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d1[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d7[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d9,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d6[3]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlsl.s16   q10,d10,d2[0]
    vmlsl.s16   q10,d11,d6[2]


    vmlsl.s16   q11,d10,d6[0]
    vmlal.s16   q11,d11,d4[2]

    vmlal.s16   q8,d10,d6[0]
    vmlal.s16   q8,d11,d0[2]

    vmlal.s16   q9,d10,d2[0]
    vmlal.s16   q9,d11,d5[2]

    cmp         r12,r6
    bhs         stage2_shift2


    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10






    vmlal.s16   q12,d14,d2[3]
    vmlal.s16   q13,d14,d3[3]
    vmlsl.s16   q14,d14,d5[3]
    vmlsl.s16   q15,d14,d0[3]


    vmlal.s16   q12,d15,d1[3]
    vmlsl.s16   q13,d15,d6[3]
    vmlsl.s16   q14,d15,d0[3]
    vmlal.s16   q15,d15,d7[3]


    vmlal.s16   q10,d12,d5[0]
    vmlal.s16   q10,d13,d0[2]
    vmlal.s16   q11,d12,d1[0]
    vmlal.s16   q11,d13,d6[2]
    vmlal.s16   q8,d12,d7[0]
    vmlsl.s16   q8,d13,d2[2]
    vmlsl.s16   q9,d12,d3[0]
    vmlsl.s16   q9,d13,d4[2]

    cmp         r12,r9
    bhs         stage2_shift2


    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10



    vmlal.s16   q12,d8,d6[1]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d1[1]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d7[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d0[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d5[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d4[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d2[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d7[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d7[2]


    vmlsl.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d1[2]

    vmlsl.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d5[2]

    vmlal.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d3[2]

    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10




    vmlsl.s16   q12,d14,d0[1]
    vmlal.s16   q13,d14,d6[1]
    vmlal.s16   q14,d14,d4[1]
    vmlsl.s16   q15,d14,d1[1]


    vmlsl.s16   q12,d15,d3[3]
    vmlal.s16   q13,d15,d0[1]
    vmlsl.s16   q14,d15,d5[1]
    vmlsl.s16   q15,d15,d6[1]


    vmlsl.s16   q10,d12,d3[0]
    vmlsl.s16   q10,d13,d1[2]
    vmlsl.s16   q11,d12,d7[0]
    vmlal.s16   q11,d13,d3[2]
    vmlal.s16   q8,d12,d1[0]
    vmlal.s16   q8,d13,d7[2]
    vmlsl.s16   q9,d12,d5[0]
    vmlsl.s16   q9,d13,d2[2]


    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10


    vmlal.s16   q12,d8,d7[3]                @// y1 * cos1(part of b0)
    vmlal.s16   q13,d8,d4[3]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d1[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d2[1]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d3[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d5[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d7[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d5[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlsl.s16   q10,d10,d6[0]
    vmlal.s16   q10,d11,d5[2]


    vmlal.s16   q11,d10,d2[0]
    vmlal.s16   q11,d11,d7[2]

    vmlsl.s16   q8,d10,d2[0]
    vmlsl.s16   q8,d11,d4[2]

    vmlal.s16   q9,d10,d6[0]
    vmlal.s16   q9,d11,d1[2]


    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10



    vmlal.s16   q12,d14,d1[1]
    vmlsl.s16   q13,d14,d0[3]
    vmlal.s16   q14,d14,d1[3]
    vmlsl.s16   q15,d14,d3[1]


    vmlal.s16   q12,d15,d5[3]
    vmlsl.s16   q13,d15,d5[1]
    vmlal.s16   q14,d15,d4[3]
    vmlsl.s16   q15,d15,d4[1]


    vmlal.s16   q10,d12,d1[0]
    vmlal.s16   q10,d13,d3[2]
    vmlsl.s16   q11,d12,d3[0]
    vmlsl.s16   q11,d13,d2[2]
    vmlal.s16   q8,d12,d5[0]
    vmlal.s16   q8,d13,d1[2]
    vmlsl.s16   q9,d12,d7[0]
    vmlsl.s16   q9,d13,d0[2]

stage2_shift2:
    vadd.s32    q4,q10,q12
    vsub.s32    q5,q10,q12

    vadd.s32    q6,q11,q13
    vsub.s32    q12,q11,q13

    vadd.s32    q7,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15


    vqrshrn.s32 d30,q4,#shift_stage2_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d19,q5,#shift_stage2_idct   @// r7 = (a0 - b0 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d31,q7,#shift_stage2_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d18,q13,#shift_stage2_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d12,q6,#shift_stage2_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d15,q12,#shift_stage2_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d13,q8,#shift_stage2_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d14,q14,#shift_stage2_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage2_idct)

    vtrn.16     q15,q6
    vtrn.16     q7,q9

    vtrn.32     d30,d31
    vtrn.32     d12,d13
    vtrn.32     d14,d15
    vtrn.32     d18,d19


    vst1.16     {q15},[r0]!
    vst1.16     {q6},[r0]!
    vst1.16     {q7},[r0]!
    vst1.16     {q9},[r0]!



    mov         r1,r4




    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10

    vmull.s16   q12,d8,d4[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d8,d4[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d8,d5[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d8,d5[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d3[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d0[2]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmull.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d7[2]


    vmull.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d6[2]

    vmull.s16   q8,d10,d0[0]
    vmlsl.s16   q8,d11,d5[2]

    vmull.s16   q9,d10,d0[0]
    vmlsl.s16   q9,d11,d4[2]

    cmp         r12,r11
    bhs         stage2_shift3

    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10

    vmlsl.s16   q12,d14,d5[1]
    vmlsl.s16   q13,d14,d7[3]
    vmlal.s16   q14,d14,d5[3]
    vmlal.s16   q15,d14,d3[1]


    vmlal.s16   q12,d15,d2[1]
    vmlal.s16   q13,d15,d1[1]
    vmlal.s16   q14,d15,d4[3]
    vmlsl.s16   q15,d15,d7[3]


    vmlsl.s16   q10,d12,d1[0]
    vmlal.s16   q10,d13,d6[2]
    vmlsl.s16   q11,d12,d3[0]
    vmlal.s16   q11,d13,d3[2]
    vmlsl.s16   q8,d12,d5[0]
    vmlal.s16   q8,d13,d0[2]
    vmlsl.s16   q9,d12,d7[0]
    vmlal.s16   q9,d13,d2[2]

    cmp         r12,r5
    bhs         stage2_shift3

    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10



    vmlal.s16   q12,d8,d6[1]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d5[1]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d0[3]                @// y1 * sin3(part of b2)
    vmlsl.s16   q15,d8,d3[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d1[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d4[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d6[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d0[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d2[0]
    vmlsl.s16   q10,d11,d5[2]


    vmlal.s16   q11,d10,d6[0]
    vmlsl.s16   q11,d11,d0[2]

    vmlsl.s16   q8,d10,d6[0]
    vmlsl.s16   q8,d11,d4[2]

    vmlsl.s16   q9,d10,d2[0]
    vmlal.s16   q9,d11,d6[2]

    cmp         r12,r6
    bhs         stage2_shift3

    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10





    vmlsl.s16   q12,d14,d7[1]
    vmlal.s16   q13,d14,d2[1]
    vmlal.s16   q14,d14,d4[1]
    vmlsl.s16   q15,d14,d5[1]


    vmlal.s16   q12,d15,d0[3]
    vmlal.s16   q13,d15,d7[1]
    vmlsl.s16   q14,d15,d1[1]
    vmlsl.s16   q15,d15,d6[1]


    vmlsl.s16   q10,d12,d3[0]
    vmlal.s16   q10,d13,d4[2]
    vmlal.s16   q11,d12,d7[0]
    vmlal.s16   q11,d13,d2[2]
    vmlal.s16   q8,d12,d1[0]
    vmlsl.s16   q8,d13,d6[2]
    vmlal.s16   q9,d12,d5[0]
    vmlsl.s16   q9,d13,d0[2]

    cmp         r12,r9
    bhs         stage2_shift3


    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10


    vmlsl.s16   q12,d8,d7[3]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d0[1]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d6[3]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d1[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d0[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d9,d5[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d2[3]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d3[2]


    vmlsl.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d5[2]

    vmlsl.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d1[2]

    vmlal.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d7[2]

    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10




    vmlal.s16   q12,d14,d6[3]
    vmlal.s16   q13,d14,d3[3]
    vmlsl.s16   q14,d14,d1[3]
    vmlal.s16   q15,d14,d7[1]


    vmlal.s16   q12,d15,d1[3]
    vmlsl.s16   q13,d15,d2[3]
    vmlal.s16   q14,d15,d7[1]
    vmlal.s16   q15,d15,d4[1]


    vmlsl.s16   q10,d12,d5[0]
    vmlal.s16   q10,d13,d2[2]
    vmlal.s16   q11,d12,d1[0]
    vmlsl.s16   q11,d13,d7[2]
    vmlsl.s16   q8,d12,d7[0]
    vmlsl.s16   q8,d13,d3[2]
    vmlsl.s16   q9,d12,d3[0]
    vmlal.s16   q9,d13,d1[2]


    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10


    vmlsl.s16   q12,d8,d5[3]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d6[3]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d3[1]                @// y1 * sin3(part of b2)
    vmlsl.s16   q15,d8,d0[1]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d2[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d9,d0[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d2[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d9,d4[3]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d6[0]
    vmlsl.s16   q10,d11,d1[2]


    vmlsl.s16   q11,d10,d2[0]
    vmlal.s16   q11,d11,d4[2]

    vmlal.s16   q8,d10,d2[0]
    vmlsl.s16   q8,d11,d7[2]

    vmlsl.s16   q9,d10,d6[0]
    vmlsl.s16   q9,d11,d5[2]

    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10



    vmlal.s16   q12,d14,d4[3]
    vmlsl.s16   q13,d14,d6[1]
    vmlal.s16   q14,d14,d7[3]
    vmlal.s16   q15,d14,d6[3]


    vmlal.s16   q12,d15,d3[3]
    vmlsl.s16   q13,d15,d3[1]
    vmlal.s16   q14,d15,d2[3]
    vmlsl.s16   q15,d15,d2[1]


    vmlsl.s16   q10,d12,d7[0]
    vmlal.s16   q10,d13,d0[2]
    vmlal.s16   q11,d12,d5[0]
    vmlsl.s16   q11,d13,d1[2]
    vmlsl.s16   q8,d12,d3[0]
    vmlal.s16   q8,d13,d2[2]
    vmlal.s16   q9,d12,d1[0]
    vmlsl.s16   q9,d13,d3[2]

stage2_shift3:
    vadd.s32    q4,q10,q12
    vsub.s32    q5,q10,q12

    vadd.s32    q6,q11,q13
    vsub.s32    q12,q11,q13

    vadd.s32    q7,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15


    vqrshrn.s32 d30,q4,#shift_stage2_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d19,q5,#shift_stage2_idct   @// r11 = (a0 - b0 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d31,q7,#shift_stage2_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d18,q13,#shift_stage2_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d12,q6,#shift_stage2_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d15,q12,#shift_stage2_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d13,q8,#shift_stage2_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d14,q14,#shift_stage2_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage2_idct)

    vtrn.16     q15,q6
    vtrn.16     q7,q9

    vtrn.32     d30,d31
    vtrn.32     d12,d13
    vtrn.32     d14,d15
    vtrn.32     d18,d19


    vst1.16     {q15},[r0]!
    vst1.16     {q6},[r0]!
    vst1.16     {q7},[r0]!
    vst1.16     {q9},[r0]!




    mov         r1,r4




    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10


    vmull.s16   q12,d8,d6[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d8,d6[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d8,d7[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d8,d7[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d2[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d4[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d5[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d7[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmull.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d3[2]


    vmull.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d2[2]

    vmull.s16   q8,d10,d0[0]
    vmlsl.s16   q8,d11,d1[2]

    vmull.s16   q9,d10,d0[0]
    vmlsl.s16   q9,d11,d0[2]

    cmp         r12,r11
    bhs         stage2_shift4
    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10






    vmlal.s16   q12,d14,d0[1]
    vmlal.s16   q13,d14,d1[3]
    vmlal.s16   q14,d14,d4[1]
    vmlal.s16   q15,d14,d6[3]


    vmlsl.s16   q12,d15,d4[1]
    vmlsl.s16   q13,d15,d0[3]
    vmlsl.s16   q14,d15,d2[3]
    vmlsl.s16   q15,d15,d6[1]


    vmlal.s16   q10,d12,d7[0]
    vmlal.s16   q10,d13,d5[2]
    vmlal.s16   q11,d12,d5[0]
    vmlsl.s16   q11,d13,d7[2]
    vmlal.s16   q8,d12,d3[0]
    vmlsl.s16   q8,d13,d4[2]
    vmlal.s16   q9,d12,d1[0]
    vmlsl.s16   q9,d13,d1[2]

    cmp         r12,r5
    bhs         stage2_shift4

    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10



    vmlal.s16   q12,d8,d7[3]                @// y1 * cos1(part of b0)
    vmlal.s16   q13,d8,d3[1]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d1[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d5[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d9,d4[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d5[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d0[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d5[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlsl.s16   q10,d10,d2[0]
    vmlal.s16   q10,d11,d1[2]


    vmlsl.s16   q11,d10,d6[0]
    vmlal.s16   q11,d11,d3[2]

    vmlal.s16   q8,d10,d6[0]
    vmlsl.s16   q8,d11,d7[2]

    vmlal.s16   q9,d10,d2[0]
    vmlsl.s16   q9,d11,d2[2]

    cmp         r12,r6
    bhs         stage2_shift4


    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10






    vmlsl.s16   q12,d14,d1[1]
    vmlsl.s16   q13,d14,d7[3]
    vmlal.s16   q14,d14,d1[3]
    vmlal.s16   q15,d14,d4[3]


    vmlal.s16   q12,d15,d2[1]
    vmlal.s16   q13,d15,d5[1]
    vmlsl.s16   q14,d15,d3[1]
    vmlsl.s16   q15,d15,d4[1]


    vmlsl.s16   q10,d12,d5[0]
    vmlsl.s16   q10,d13,d7[2]
    vmlsl.s16   q11,d12,d1[0]
    vmlal.s16   q11,d13,d1[2]
    vmlsl.s16   q8,d12,d7[0]
    vmlal.s16   q8,d13,d5[2]
    vmlal.s16   q9,d12,d3[0]
    vmlsl.s16   q9,d13,d3[2]

    cmp         r12,r9
    bhs         stage2_shift4


    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10


    vmlsl.s16   q12,d8,d5[3]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d2[3]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d8,d4[3]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d3[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d6[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d9,d0[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d9,d6[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d3[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlal.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d0[2]


    vmlsl.s16   q11,d10,d0[0]
    vmlal.s16   q11,d11,d6[2]

    vmlsl.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d2[2]

    vmlal.s16   q9,d10,d0[0]
    vmlsl.s16   q9,d11,d4[2]

    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10




    vmlal.s16   q12,d14,d3[1]
    vmlsl.s16   q13,d14,d2[1]
    vmlal.s16   q14,d14,d7[3]
    vmlal.s16   q15,d14,d2[3]


    vmlsl.s16   q12,d15,d0[3]
    vmlal.s16   q13,d15,d4[3]
    vmlal.s16   q14,d15,d6[3]
    vmlsl.s16   q15,d15,d2[1]


    vmlal.s16   q10,d12,d3[0]
    vmlsl.s16   q10,d13,d6[2]
    vmlal.s16   q11,d12,d7[0]
    vmlsl.s16   q11,d13,d4[2]
    vmlsl.s16   q8,d12,d1[0]
    vmlal.s16   q8,d13,d0[2]
    vmlal.s16   q9,d12,d5[0]
    vmlsl.s16   q9,d13,d5[2]


    vld1.16     {d10,d11},[r1]!
    vld1.16     {d8,d9},[r1],r10




    vmlal.s16   q12,d8,d3[3]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d8,d7[1]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d8,d5[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d8,d1[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d9,d7[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d9,d6[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d9,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d9,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)





    vmlsl.s16   q10,d10,d6[0]
    vmlal.s16   q10,d11,d2[2]


    vmlal.s16   q11,d10,d2[0]
    vmlsl.s16   q11,d11,d0[2]

    vmlsl.s16   q8,d10,d2[0]
    vmlal.s16   q8,d11,d3[2]

    vmlal.s16   q9,d10,d6[0]
    vmlsl.s16   q9,d11,d6[2]


    vld1.16     {d12,d13},[r1]!
    vld1.16     {d14,d15},[r1],r10



    vmlsl.s16   q12,d14,d5[1]
    vmlal.s16   q13,d14,d3[3]
    vmlsl.s16   q14,d14,d2[1]
    vmlal.s16   q15,d14,d0[3]


    vmlal.s16   q12,d15,d1[3]
    vmlsl.s16   q13,d15,d1[1]
    vmlal.s16   q14,d15,d0[3]
    vmlsl.s16   q15,d15,d0[1]


    vmlsl.s16   q10,d12,d1[0]
    vmlal.s16   q10,d13,d4[2]
    vmlal.s16   q11,d12,d3[0]
    vmlsl.s16   q11,d13,d5[2]
    vmlsl.s16   q8,d12,d5[0]
    vmlal.s16   q8,d13,d6[2]
    vmlal.s16   q9,d12,d7[0]
    vmlsl.s16   q9,d13,d7[2]

stage2_shift4:
    vadd.s32    q4,q10,q12
    vsub.s32    q5,q10,q12

    vadd.s32    q6,q11,q13
    vsub.s32    q12,q11,q13

    vadd.s32    q7,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15


    vqrshrn.s32 d30,q4,#shift_stage2_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d19,q5,#shift_stage2_idct   @// r11 = (a0 - b0 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d31,q7,#shift_stage2_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d18,q13,#shift_stage2_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d12,q6,#shift_stage2_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d15,q12,#shift_stage2_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d13,q8,#shift_stage2_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage2_idct)
    vqrshrn.s32 d14,q14,#shift_stage2_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage2_idct)



    vtrn.16     q15,q6
    vtrn.16     q7,q9

    vtrn.32     d30,d31
    vtrn.32     d12,d13
    vtrn.32     d14,d15
    vtrn.32     d18,d19



    vst1.16     {q15},[r0]!
    vst1.16     {q6},[r0]!
    vst1.16     {q7},[r0]!
    vst1.16     {q9},[r0]!




    sub         r0,r0,#256
prediction_buffer:


    vld1.16     {d12,d13},[r0]!
    vld1.16     {d14,d15},[r0]!

    add         r0,r0,#32

    vld1.16     {d16,d17},[r0]!
    vld1.16     {d18,d19},[r0]!
    add         r0,r0,#32

    vld1.16     {d20,d21},[r0]!
    vld1.16     {d22,d23},[r0]!


    add         r0,r0,#32

    vld1.16     {d24,d25},[r0]!
    vld1.16     {d26,d27},[r0]!





@ d12 =r0 1- 4 values
@ d13 =r2 1- 4 values
@ d14=r1 1- 4 values
@ d15=r3 1- 4 values

@ d16 =r0 5- 8 values
@ d17 =r2 5- 8 values
@ d18=r1 5- 8 values
@ d19=r3 5- 8 values

@ d20 =r0 9- 12 values
@ d21 =r2 9- 12 values
@ d22=r1 9- 12 values
@ d23=r3 9- 12 values

@ d24 =r0 13-16 values
@ d25 =r2 13- 16 values
@ d26=r1 13- 16 values
@ d27=r3 13- 16 values

    vswp        d13,d16
    vswp        d21,d24
    vswp        d15,d18
    vswp        d23,d26


    vld1.8      {d8,d9},[r2],r8
    vld1.8      {d10,d11},[r2],r8
    vld1.8      {d28,d29},[r2],r8
    vld1.8      {d30,d31},[r2],r8


    vaddw.u8    q6,q6,d8
    vaddw.u8    q10,q10,d9
    vaddw.u8    q7,q7,d10
    vaddw.u8    q11,q11,d11
    vaddw.u8    q8,q8,d28
    vaddw.u8    q12,q12,d29
    vaddw.u8    q9,q9,d30
    vaddw.u8    q13,q13,d31
    sub         r2,r2,r8,lsl #2
    add         r2,r2,#16
    vqmovun.s16 d12,q6
    vqmovun.s16 d13,q10
    vqmovun.s16 d20,q7
    vqmovun.s16 d21,q11
    vqmovun.s16 d14,q8
    vqmovun.s16 d15,q12
    vqmovun.s16 d22,q9
    vqmovun.s16 d23,q13


    vst1.8      {d12,d13},[r3],r7
    vst1.8      {d20,d21},[r3],r7
    vst1.8      {d14,d15},[r3],r7
    vst1.8      {d22,d23},[r3],r7


    sub         r3,r3,r7,lsl #2
    add         r3,r3,#16

    vld1.16     {d12,d13},[r0]!
    vld1.16     {d14,d15},[r0]!

    sub         r0,r0,#96

    vld1.16     {d16,d17},[r0]!
    vld1.16     {d18,d19},[r0]!
    sub         r0,r0,#96

    vld1.16     {d20,d21},[r0]!
    vld1.16     {d22,d23},[r0]!


    sub         r0,r0,#96

    vld1.16     {d24,d25},[r0]!
    vld1.16     {d26,d27},[r0]!


    sub         r0,r0,#64




    vswp        d13,d16
    vswp        d21,d24
    vswp        d15,d18
    vswp        d23,d26


    vld1.8      {d8,d9},[r2],r8
    vld1.8      {d10,d11},[r2],r8
    vld1.8      {d28,d29},[r2],r8
    vld1.8      {d30,d31},[r2],r8


    vaddw.u8    q6,q6,d8
    vaddw.u8    q10,q10,d9
    vaddw.u8    q7,q7,d10
    vaddw.u8    q11,q11,d11
    vaddw.u8    q8,q8,d28
    vaddw.u8    q12,q12,d29
    vaddw.u8    q9,q9,d30
    vaddw.u8    q13,q13,d31
    sub         r2,r2,#16

    vqmovun.s16 d12,q6
    vqmovun.s16 d13,q10
    vqmovun.s16 d20,q7
    vqmovun.s16 d21,q11
    vqmovun.s16 d14,q8
    vqmovun.s16 d15,q12
    vqmovun.s16 d22,q9
    vqmovun.s16 d23,q13


    vst1.8      {d12,d13},[r3],r7
    vst1.8      {d20,d21},[r3],r7
    vst1.8      {d14,d15},[r3],r7
    vst1.8      {d22,d23},[r3],r7

    sub         r3,r3,#16

    subs        r14,r14,#1
    bne         dct_stage2
    vpop        {d8  -  d15}
    ldmfd       sp!,{r0-r12,pc}





