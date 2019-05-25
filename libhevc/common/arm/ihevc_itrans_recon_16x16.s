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
@ *  - ihevc_itrans_recon_16x16()
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

@void ihevc_itrans_recon_16x16(word16 *pi2_src,
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

.equ    src_stride_offset,     104
.equ    pred_stride_offset,    108
.equ    out_stride_offset,     112
.equ    zero_cols_offset,      116
.equ    zero_rows_offset,      120

.text
.align 4






.set shift_stage1_idct ,   7
.set shift_stage2_idct ,   12
@#define zero_cols       r12
@#define zero_rows       r11
.globl ihevc_itrans_recon_16x16_a9q

.extern g_ai2_ihevc_trans_16_transpose

g_ai2_ihevc_trans_16_transpose_addr:
.long g_ai2_ihevc_trans_16_transpose - ulbl1 - 8

.type ihevc_itrans_recon_16x16_a9q, %function

ihevc_itrans_recon_16x16_a9q:

    stmfd       sp!,{r4-r12,lr}
    vpush       {d8  -  d15}
    ldr         r6,[sp,#src_stride_offset]  @ src stride
    ldr         r12,[sp,#zero_cols_offset]
    ldr         r11,[sp,#zero_rows_offset]



    ldr         r14,g_ai2_ihevc_trans_16_transpose_addr
ulbl1:
    add         r14,r14,pc
    vld1.16     {d0,d1,d2,d3},[r14]         @//d0,d1 are used for storing the constant data
    movw        r7,#0xffff
    and         r12,r12,r7
    and         r11,r11,r7
    mov         r6,r6,lsl #1                @ x sizeof(word16)
    add         r9,r0,r6, lsl #1            @ 2 rows

    add         r10,r6,r6, lsl #1           @ 3 rows
    add         r5,r6,r6,lsl #2
    movw        r7,#0xfff0

    cmp         r12,r7
    bge         zero_12cols_decision

    cmp         r12,#0xff00
    bge         zero_8cols_decision




    mov         r14,#4
    cmp         r11,r7
    rsbge       r10,r6,#0

    cmp         r11,#0xff00
    movge       r8,r5
    rsbge       r8,r8,#0
    movlt       r8,r10
    add         r5,r5,r6,lsl #3
    rsb         r5,r5,#0

    b           first_stage_top_four_bottom_four

zero_12cols_decision:
    mov         r14,#1
    cmp         r11,#0xff00
    movge       r8,r5
    movlt       r8,r10
    add         r5,r5,r6,lsl #3
    rsb         r5,r5,#0

    b           first_stage_top_four_bottom_four

zero_8cols_decision:
    mov         r14,#2
    mov         r8,r5
    rsb         r8,r8,#0
    cmp         r11,#0xff00
    movlt       r8,r10
    add         r5,r5,r6,lsl #3
    rsb         r5,r5,#0
    cmp         r11,r7
    rsbge       r10,r6,#0


    b           first_stage_top_four_bottom_four


@d0[0]= 64      d2[0]=64
@d0[1]= 90      d2[1]=57
@d0[2]= 89      d2[2]=50
@d0[3]= 87      d2[3]=43
@d1[0]= 83      d3[0]=36
@d1[1]= 80      d3[1]=25
@d1[2]= 75      d3[2]=18
@d1[3]= 70      d3[3]=9



first_stage:
    add         r0,r0,#8
    add         r9,r9,#8

first_stage_top_four_bottom_four:

    vld1.16     d10,[r0],r6
    vld1.16     d11,[r9],r6
    vld1.16     d6,[r0],r10
    vld1.16     d7,[r9],r10
    cmp         r11,r7
    bge         skip_load4rows

    vld1.16     d4,[r0],r6
    vld1.16     d5,[r9],r6
    vld1.16     d8,[r0],r8
    vld1.16     d9,[r9],r8

@ registers used: q0,q1,q3,q5,q2,q4

@ d10 =r0
@d6= r1
@d11=r2
@d7=r3

skip_load4rows:
    vmull.s16   q12,d6,d0[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d6,d0[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d6,d1[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d6,d1[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d7,d0[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d7,d2[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d7,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d7,d2[3]                @// y1 * sin1 - y3 * sin3(part of b3)






    vmull.s16   q6,d10,d0[0]
    vmlal.s16   q6,d11,d0[2]
    vmull.s16   q7,d10,d0[0]
    vmlal.s16   q7,d11,d1[2]
    vmull.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d2[2]
    vmull.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d3[2]

    bge         skip_last12rows_kernel1


    vmlal.s16   q12,d8,d1[1]
    vmlal.s16   q13,d8,d3[3]
    vmlsl.s16   q14,d8,d1[3]
    vmlsl.s16   q15,d8,d0[3]


    vmlal.s16   q12,d9,d1[3]
    vmlsl.s16   q13,d9,d2[3]
    vmlsl.s16   q14,d9,d0[3]
    vmlal.s16   q15,d9,d3[3]





    vmlal.s16   q6,d4,d1[0]
    vmlal.s16   q6,d5,d1[2]
    vmlal.s16   q7,d4,d3[0]
    vmlsl.s16   q7,d5,d3[2]
    vmlsl.s16   q8,d4,d3[0]
    vmlsl.s16   q8,d5,d0[2]
    vmlsl.s16   q9,d4,d1[0]
    vmlsl.s16   q9,d5,d2[2]

@d0[0]= 64      d2[0]=64
@d0[1]= 90      d2[1]=57
@d0[2]= 89      d2[2]=50
@d0[3]= 87      d2[3]=43
@d1[0]= 83      d3[0]=36
@d1[1]= 80      d3[1]=25
@d1[2]= 75      d3[2]=18
@d1[3]= 70      d3[3]=9
    cmp         r11,#0xff00
    bge         skip_last12rows_kernel1


    vld1.16     d10,[r0],r6
    vld1.16     d11,[r9],r6
    vld1.16     d6,[r0],r10
    vld1.16     d7,[r9],r10
    vld1.16     d4,[r0],r6
    vld1.16     d5,[r9],r6
    vld1.16     d8,[r0],r5
    vld1.16     d9,[r9],r5




    vmlal.s16   q12,d6,d2[1]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d6,d1[1]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d6,d3[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d6,d0[1]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d7,d2[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d7,d0[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d7,d2[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d7,d3[1]                @// y1 * sin1 - y3 * sin3(part of b3)



    vmlal.s16   q12,d8,d3[1]
    vmlsl.s16   q13,d8,d1[3]
    vmlal.s16   q14,d8,d0[1]
    vmlsl.s16   q15,d8,d1[1]


    vmlal.s16   q12,d9,d3[3]
    vmlsl.s16   q13,d9,d3[1]
    vmlal.s16   q14,d9,d2[3]
    vmlsl.s16   q15,d9,d2[1]





    vmlal.s16   q6,d10,d0[0]
    vmlal.s16   q6,d11,d2[2]
    vmlal.s16   q6,d4,d3[0]
    vmlal.s16   q6,d5,d3[2]




    vmlsl.s16   q7,d10,d0[0]
    vmlsl.s16   q7,d11,d0[2]
    vmlsl.s16   q7,d4,d1[0]
    vmlsl.s16   q7,d5,d2[2]


    vmlsl.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d3[2]
    vmlal.s16   q8,d4,d1[0]
    vmlal.s16   q8,d5,d1[2]


    vmlal.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d1[2]
    vmlsl.s16   q9,d4,d3[0]
    vmlsl.s16   q9,d5,d0[2]

skip_last12rows_kernel1:
    vadd.s32    q10,q6,q12
    vsub.s32    q11,q6,q12

    vadd.s32    q6,q7,q13
    vsub.s32    q12,q7,q13

    vadd.s32    q7,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15







    vqrshrn.s32 d30,q10,#shift_stage1_idct  @// r0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d19,q11,#shift_stage1_idct  @// r7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d31,q7,#shift_stage1_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d18,q13,#shift_stage1_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d12,q6,#shift_stage1_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d15,q12,#shift_stage1_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d13,q8,#shift_stage1_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d14,q14,#shift_stage1_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    vst1.16     {d30,d31},[r1]!
    vst1.16     {d18,d19},[r1]!
    sub         r1,r1,#32

    bge         skip_stage1_kernel_load

first_stage_middle_eight:



    vld1.16     d10,[r0],r6
    vld1.16     d11,[r9],r6
    vld1.16     d6,[r0],r10
    vld1.16     d7,[r9],r10
    vld1.16     d4,[r0],r6
    vld1.16     d5,[r9],r6
    vld1.16     d8,[r0],r8
    vld1.16     d9,[r9],r8


skip_stage1_kernel_load:
    vmull.s16   q12,d6,d2[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d6,d2[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d6,d3[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d6,d3[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d7,d1[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d7,d0[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d7,d1[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d7,d3[1]                @// y1 * sin1 - y3 * sin3(part of b3)






    vmull.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d3[2]
    vmull.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d2[2]
    vmull.s16   q8,d10,d0[0]
    vmlsl.s16   q8,d11,d1[2]
    vmull.s16   q9,d10,d0[0]
    vmlsl.s16   q9,d11,d0[2]


    cmp         r11,r7
    bge         skip_last12rows_kernel2

    vmlsl.s16   q12,d8,d3[1]
    vmlal.s16   q13,d8,d2[1]
    vmlal.s16   q14,d8,d0[1]
    vmlal.s16   q15,d8,d2[3]


    vmlal.s16   q12,d9,d0[1]
    vmlal.s16   q13,d9,d3[1]
    vmlsl.s16   q14,d9,d1[1]
    vmlsl.s16   q15,d9,d2[1]



    vmlsl.s16   q11,d4,d1[0]
    vmlal.s16   q11,d5,d2[2]
    vmlsl.s16   q10,d4,d3[0]
    vmlal.s16   q10,d5,d0[2]
    vmlal.s16   q8,d4,d3[0]
    vmlal.s16   q8,d5,d3[2]
    vmlal.s16   q9,d4,d1[0]
    vmlsl.s16   q9,d5,d1[2]

@d0[0]= 64      d2[0]=64
@d0[1]= 90      d2[1]=57
@d0[2]= 89      d2[2]=50
@d0[3]= 87      d2[3]=43
@d1[0]= 83      d3[0]=36
@d1[1]= 80      d3[1]=25
@d1[2]= 75      d3[2]=18
@d1[3]= 70      d3[3]=9
    cmp         r11,#0xff00
    bge         skip_last12rows_kernel2

    vld1.16     d10,[r0],r6
    vld1.16     d11,[r9],r6
    vld1.16     d6,[r0],r10
    vld1.16     d7,[r9],r10
    vld1.16     d4,[r0],r6
    vld1.16     d5,[r9],r6
    vld1.16     d8,[r0],r5
    vld1.16     d9,[r9],r5


    vmlsl.s16   q12,d6,d3[3]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d6,d0[3]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d6,d2[3]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d6,d1[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d7,d0[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d7,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d7,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d7,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)


    vmlal.s16   q12,d8,d2[3]
    vmlal.s16   q13,d8,d3[3]
    vmlsl.s16   q14,d8,d2[1]
    vmlal.s16   q15,d8,d0[3]


    vmlal.s16   q12,d9,d1[3]
    vmlsl.s16   q13,d9,d1[1]
    vmlal.s16   q14,d9,d0[3]
    vmlsl.s16   q15,d9,d0[1]




    vmlal.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d1[2]
    vmlsl.s16   q11,d4,d3[0]
    vmlal.s16   q11,d5,d0[2]



    vmlsl.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d3[2]
    vmlal.s16   q10,d4,d1[0]
    vmlsl.s16   q10,d5,d1[2]


    vmlsl.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d0[2]
    vmlsl.s16   q8,d4,d1[0]
    vmlal.s16   q8,d5,d2[2]



    vmlal.s16   q9,d10,d0[0]
    vmlsl.s16   q9,d11,d2[2]
    vmlal.s16   q9,d4,d3[0]
    vmlsl.s16   q9,d5,d3[2]

skip_last12rows_kernel2:

    vadd.s32    q2,q11,q12
    vsub.s32    q11,q11,q12

    vadd.s32    q3,q10,q13
    vsub.s32    q12,q10,q13

    vadd.s32    q5,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15


    vqrshrn.s32 d18,q2,#shift_stage1_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d31,q11,#shift_stage1_idct  @// r7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d19,q5,#shift_stage1_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d30,q13,#shift_stage1_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d20,q3,#shift_stage1_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d23,q12,#shift_stage1_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d21,q8,#shift_stage1_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d22,q14,#shift_stage1_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)


    @ registers used:   {q2,q4,q6,q7}, {q9,q15,q10,q11}






    vld1.16     {d4,d5},[r1]!
    vld1.16     {d8,d9},[r1]!
    sub         r1,r1,#32

@d4=r0
@d12=r1
@d5=r2
@d13=r3

@d18=r4
@d20=r5
@d19=r6
@d21=r7

@d22=r8
@d30=r9
@d23=r10
@d31=r11

@d14=r12
@d8=r13
@d15=r14
@d9=r15


    vtrn.16     q2,q6
    vtrn.16     q9,q10
    vtrn.16     q11,q15
    vtrn.16     q7,q4



    vtrn.32     d4,d5
    vtrn.32     d12,d13

    vtrn.32     d18,d19
    vtrn.32     d20,d21

    vtrn.32     d22,d23
    vtrn.32     d30,d31

    vtrn.32     d14,d15
    vtrn.32     d8,d9


@ d4 =r0 1- 4 values
@ d5 =r2 1- 4 values
@ d12=r1 1- 4 values
@ d13=r3 1- 4 values

@ d18 =r0 5- 8 values
@ d19 =r2 5- 8 values
@ d20=r1 5- 8 values
@ d21=r3 5- 8 values

@ d22 =r0 9- 12 values
@ d23 =r2 9- 12 values
@ d30=r1 9- 12 values
@ d31=r3 9- 12 values

@ d14 =r0 13-16 values
@ d15 =r2 13- 16 values
@ d8=r1 13- 16 values
@ d9=r3 13- 16 values


    vst1.16     {q2},[r1]!
    vst1.16     {q6},[r1]!

    vst1.16     {q9},[r1]!
    vst1.16     {q10},[r1]!
    vst1.16     {q11},[r1]!
    vst1.16     {q15},[r1]!
    vst1.16     {q7},[r1]!
    vst1.16     {q4},[r1]!


    subs        r14,r14,#1
    bne         first_stage










    mov         r6,r7

    ldr         r8,[sp,#pred_stride_offset] @ prediction stride
    ldr         r7,[sp,#out_stride_offset]  @ destination stride

    mov         r10,#16

    cmp         r12,r6
    subge       r1,r1,#128
    bge         label1

    cmp         r12,#0xff00
    subge       r1,r1,#256
    bge         label_2

    sub         r1,r1,#512
    rsb         r10,r10,#0

label_2:
    add         r9,r1,#128
    add         r11,r9,#128
    add         r0,r11,#128



label1:
@   mov   r6,r1


    mov         r14,#4
    add         r4,r2,r8, lsl #1            @ r4 = r2 + pred_strd * 2    => r4 points to 3rd row of pred data
    add         r5,r8,r8, lsl #1            @
@   add r0,r3,r7, lsl #1    @ r0 points to 3rd row of dest data
@   add r10,r7,r7, lsl #1   @




second_stage:
    vld1.16     {d10,d11},[r1]!
    vld1.16     {d6,d7},[r1],r10
    cmp         r12,r6
    bge         second_stage_process
    vld1.16     {d4,d5},[r9]!
    vld1.16     {d8,d9},[r9],r10

second_stage_process:


    vmull.s16   q12,d6,d0[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d6,d0[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d6,d1[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d6,d1[3]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d7,d0[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d7,d2[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d7,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d7,d2[3]                @// y1 * sin1 - y3 * sin3(part of b3)


    vmull.s16   q6,d10,d0[0]
    vmlal.s16   q6,d11,d0[2]
    vmull.s16   q7,d10,d0[0]
    vmlal.s16   q7,d11,d1[2]
    vmull.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d2[2]
    vmull.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d3[2]

    bge         skip_last8rows_stage2_kernel1

    vmlal.s16   q12,d8,d1[1]
    vmlal.s16   q13,d8,d3[3]
    vmlsl.s16   q14,d8,d1[3]
    vmlsl.s16   q15,d8,d0[3]


    vmlal.s16   q12,d9,d1[3]
    vmlsl.s16   q13,d9,d2[3]
    vmlsl.s16   q14,d9,d0[3]
    vmlal.s16   q15,d9,d3[3]


    vmlal.s16   q6,d4,d1[0]
    vmlal.s16   q6,d5,d1[2]
    vmlal.s16   q7,d4,d3[0]
    vmlsl.s16   q7,d5,d3[2]
    vmlsl.s16   q8,d4,d3[0]
    vmlsl.s16   q8,d5,d0[2]
    vmlsl.s16   q9,d4,d1[0]
    vmlsl.s16   q9,d5,d2[2]

    cmp         r12,#0xff00
    bge         skip_last8rows_stage2_kernel1


    vld1.16     {d10,d11},[r11]!
    vld1.16     {d6,d7},[r11],r10
    vld1.16     {d4,d5},[r0]!
    vld1.16     {d8,d9},[r0],r10





    vmlal.s16   q12,d6,d2[1]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d6,d1[1]                @// y1 * cos3(part of b1)
    vmlsl.s16   q14,d6,d3[1]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d6,d0[1]                @// y1 * sin1(part of b3)

    vmlal.s16   q12,d7,d2[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d7,d0[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d7,d2[1]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlal.s16   q15,d7,d3[1]                @// y1 * sin1 - y3 * sin3(part of b3)



    vmlal.s16   q12,d8,d3[1]
    vmlsl.s16   q13,d8,d1[3]
    vmlal.s16   q14,d8,d0[1]
    vmlsl.s16   q15,d8,d1[1]


    vmlal.s16   q12,d9,d3[3]
    vmlsl.s16   q13,d9,d3[1]
    vmlal.s16   q14,d9,d2[3]
    vmlsl.s16   q15,d9,d2[1]





    vmlal.s16   q6,d10,d0[0]
    vmlal.s16   q6,d11,d2[2]
    vmlal.s16   q6,d4,d3[0]
    vmlal.s16   q6,d5,d3[2]




    vmlsl.s16   q7,d10,d0[0]
    vmlsl.s16   q7,d11,d0[2]
    vmlsl.s16   q7,d4,d1[0]
    vmlsl.s16   q7,d5,d2[2]


    vmlsl.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d3[2]
    vmlal.s16   q8,d4,d1[0]
    vmlal.s16   q8,d5,d1[2]


    vmlal.s16   q9,d10,d0[0]
    vmlal.s16   q9,d11,d1[2]
    vmlsl.s16   q9,d4,d3[0]
    vmlsl.s16   q9,d5,d0[2]






skip_last8rows_stage2_kernel1:



    vadd.s32    q10,q6,q12
    vsub.s32    q11,q6,q12

    vadd.s32    q6,q7,q13
    vsub.s32    q12,q7,q13

    vadd.s32    q7,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15







    vqrshrn.s32 d30,q10,#shift_stage2_idct  @// r0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d19,q11,#shift_stage2_idct  @// r7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d31,q7,#shift_stage2_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d18,q13,#shift_stage2_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d12,q6,#shift_stage2_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d15,q12,#shift_stage2_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d13,q8,#shift_stage2_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d14,q14,#shift_stage2_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    bge         skip_stage2_kernel_load

    @q2,q4,q6,q7 is used
    vld1.16     {d10,d11},[r1]!
    vld1.16     {d6,d7},[r1]!
    vld1.16     {d4,d5},[r9]!
    vld1.16     {d8,d9},[r9]!
skip_stage2_kernel_load:
    sub         r1,r1,#32
    vst1.16     {d30,d31},[r1]!
    vst1.16     {d18,d19},[r1]!
    sub         r1,r1,#32

    vmull.s16   q12,d6,d2[1]                @// y1 * cos1(part of b0)
    vmull.s16   q13,d6,d2[3]                @// y1 * cos3(part of b1)
    vmull.s16   q14,d6,d3[1]                @// y1 * sin3(part of b2)
    vmull.s16   q15,d6,d3[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d7,d1[1]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlsl.s16   q13,d7,d0[1]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlsl.s16   q14,d7,d1[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d7,d3[1]                @// y1 * sin1 - y3 * sin3(part of b3)


    vmull.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d3[2]
    vmull.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d2[2]
    vmull.s16   q8,d10,d0[0]
    vmlsl.s16   q8,d11,d1[2]
    vmull.s16   q9,d10,d0[0]
    vmlsl.s16   q9,d11,d0[2]



    cmp         r12,r6
    bge         skip_last8rows_stage2_kernel2


    vmlsl.s16   q12,d8,d3[1]
    vmlal.s16   q13,d8,d2[1]
    vmlal.s16   q14,d8,d0[1]
    vmlal.s16   q15,d8,d2[3]


    vmlal.s16   q12,d9,d0[1]
    vmlal.s16   q13,d9,d3[1]
    vmlsl.s16   q14,d9,d1[1]
    vmlsl.s16   q15,d9,d2[1]



    vmlsl.s16   q11,d4,d1[0]
    vmlal.s16   q11,d5,d2[2]
    vmlsl.s16   q10,d4,d3[0]
    vmlal.s16   q10,d5,d0[2]
    vmlal.s16   q8,d4,d3[0]
    vmlal.s16   q8,d5,d3[2]
    vmlal.s16   q9,d4,d1[0]
    vmlsl.s16   q9,d5,d1[2]
    cmp         r12,#0xff00
    bge         skip_last8rows_stage2_kernel2

    vld1.16     {d10,d11},[r11]!
    vld1.16     {d6,d7},[r11]!
    vld1.16     {d4,d5},[r0]!
    vld1.16     {d8,d9},[r0]!

    vmlsl.s16   q12,d6,d3[3]                @// y1 * cos1(part of b0)
    vmlsl.s16   q13,d6,d0[3]                @// y1 * cos3(part of b1)
    vmlal.s16   q14,d6,d2[3]                @// y1 * sin3(part of b2)
    vmlal.s16   q15,d6,d1[3]                @// y1 * sin1(part of b3)

    vmlsl.s16   q12,d7,d0[3]                @// y1 * cos1 + y3 * cos3(part of b0)
    vmlal.s16   q13,d7,d1[3]                @// y1 * cos3 - y3 * sin1(part of b1)
    vmlal.s16   q14,d7,d3[3]                @// y1 * sin3 - y3 * cos1(part of b2)
    vmlsl.s16   q15,d7,d1[1]                @// y1 * sin1 - y3 * sin3(part of b3)


    vmlal.s16   q12,d8,d2[3]
    vmlal.s16   q13,d8,d3[3]
    vmlsl.s16   q14,d8,d2[1]
    vmlal.s16   q15,d8,d0[3]


    vmlal.s16   q12,d9,d1[3]
    vmlsl.s16   q13,d9,d1[1]
    vmlal.s16   q14,d9,d0[3]
    vmlsl.s16   q15,d9,d0[1]




    vmlal.s16   q11,d10,d0[0]
    vmlsl.s16   q11,d11,d1[2]
    vmlsl.s16   q11,d4,d3[0]
    vmlal.s16   q11,d5,d0[2]



    vmlsl.s16   q10,d10,d0[0]
    vmlsl.s16   q10,d11,d3[2]
    vmlal.s16   q10,d4,d1[0]
    vmlsl.s16   q10,d5,d1[2]


    vmlsl.s16   q8,d10,d0[0]
    vmlal.s16   q8,d11,d0[2]
    vmlsl.s16   q8,d4,d1[0]
    vmlal.s16   q8,d5,d2[2]



    vmlal.s16   q9,d10,d0[0]
    vmlsl.s16   q9,d11,d2[2]
    vmlal.s16   q9,d4,d3[0]
    vmlsl.s16   q9,d5,d3[2]


skip_last8rows_stage2_kernel2:



    vadd.s32    q2,q11,q12
    vsub.s32    q11,q11,q12

    vadd.s32    q3,q10,q13
    vsub.s32    q12,q10,q13

    vadd.s32    q5,q8,q14
    vsub.s32    q13,q8,q14


    vadd.s32    q8,q9,q15
    vsub.s32    q14,q9,q15


    vqrshrn.s32 d18,q2,#shift_stage2_idct   @// r0 = (a0 + b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d31,q11,#shift_stage2_idct  @// r7 = (a0 - b0 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d19,q5,#shift_stage2_idct   @// r2 = (a2 + b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d30,q13,#shift_stage2_idct  @// r5 = (a2 - b2 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d20,q3,#shift_stage2_idct   @// r1 = (a1 + b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d23,q12,#shift_stage2_idct  @// r6 = (a1 - b1 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d21,q8,#shift_stage2_idct   @// r3 = (a3 + b3 + rnd) >> 7(shift_stage1_idct)
    vqrshrn.s32 d22,q14,#shift_stage2_idct  @// r4 = (a3 - b3 + rnd) >> 7(shift_stage1_idct)

    vld1.16     {d4,d5},[r1]!
    vld1.16     {d8,d9},[r1]!



    @ registers used:   {q2,q4,q6,q7}, {q9,q15,q10,q11}

@d4=r0
@d12=r1
@d5=r2
@d13=r3

@d18=r4
@d20=r5
@d19=r6
@d21=r7

@d22=r8
@d30=r9
@d23=r10
@d31=r11

@d14=r12
@d8=r13
@d15=r14
@d9=r15


    vtrn.16     q2,q6
    vtrn.16     q9,q10
    vtrn.16     q11,q15
    vtrn.16     q7,q4



    vtrn.32     d4,d5
    vtrn.32     d12,d13

    vtrn.32     d18,d19
    vtrn.32     d20,d21

    vtrn.32     d22,d23
    vtrn.32     d30,d31

    vtrn.32     d14,d15
    vtrn.32     d8,d9

@ d4 =r0 1- 4 values
@ d5 =r2 1- 4 values
@ d12=r1 1- 4 values
@ d13=r3 1- 4 values

@ d18 =r0 5- 8 values
@ d19 =r2 5- 8 values
@ d20=r1 5- 8 values
@ d21=r3 5- 8 values

@ d22 =r0 9- 12 values
@ d23 =r2 9- 12 values
@ d30=r1 9- 12 values
@ d31=r3 9- 12 values

@ d14 =r0 13-16 values
@ d15 =r2 13- 16 values
@ d8=r1 13- 16 values
@ d9=r3 13- 16 values


    vswp        d5,d18
    vswp        d23,d14
    vswp        d13,d20
    vswp        d31,d8

@ q2: r0 1-8 values
@ q11: r0 9-16 values
@ q9 : r2 1-8 values
@ q7 : r2 9-16 values
@ q6 : r1 1- 8 values
@ q10: r3 1-8 values
@ q15: r1 9-16 values
@ q4:  r3 9-16 values


@   registers free: q8,q14,q12,q13


    vld1.8      {d16,d17},[r2],r8
    vld1.8      {d28,d29},[r2],r5
    vld1.8      {d24,d25},[r4],r8
    vld1.8      {d26,d27},[r4],r5




    vaddw.u8    q2,q2,d16
    vaddw.u8    q11,q11,d17
    vaddw.u8    q6,q6,d28
    vaddw.u8    q15,q15,d29
    vaddw.u8    q9,q9,d24
    vaddw.u8    q7,q7,d25
    vaddw.u8    q10,q10,d26
    vaddw.u8    q4,q4,d27


    vqmovun.s16 d16,q2
    vqmovun.s16 d17,q11
    vqmovun.s16 d28,q6
    vqmovun.s16 d29,q15
    vqmovun.s16 d24,q9
    vqmovun.s16 d25,q7
    vqmovun.s16 d26,q10
    vqmovun.s16 d27,q4



    vst1.8      {d16,d17},[r3],r7
    vst1.8      {d28,d29},[r3],r7
    vst1.8      {d24,d25},[r3],r7
    vst1.8      {d26,d27},[r3],r7

    subs        r14,r14,#1



    bne         second_stage


    vpop        {d8  -  d15}
    ldmfd       sp!,{r4-r12,pc}











