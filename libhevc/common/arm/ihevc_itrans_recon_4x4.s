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
@ *  ihevc_itrans_recon_4x4_neon.s
@ *
@ * @brief
@ *  contains function definitions for single stage  inverse transform
@ *
@ * @author
@ *  naveen sr
@ *
@ * @par list of functions:
@ *  - ihevc_itrans_recon_4x4()
@ *
@ * @remarks
@ *  none
@ *
@ *******************************************************************************
@*/
@ /**
@ *******************************************************************************
@ *
@ * @brief
@ *  this function performs inverse transform  and reconstruction for 4x4
@ * input block
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
@void ihevc_itrans_recon_4x4(word16 *pi2_src,
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



.globl ihevc_itrans_recon_4x4_a9q

.extern g_ai2_ihevc_trans_4_transpose

g_ai2_ihevc_trans_4_transpose_addr:
.long g_ai2_ihevc_trans_4_transpose - ulbl1 - 8

.type ihevc_itrans_recon_4x4_a9q, %function

ihevc_itrans_recon_4x4_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8  -  d15}

    ldr         r8,g_ai2_ihevc_trans_4_transpose_addr
ulbl1:
    add         r8,r8,pc

    ldr         r4,[sp,#src_strd_offset]    @loading src_strd
    ldr         r5,[sp,#pred_strd_offset]   @loading pred_strd
    add         r4,r4,r4                    @ src_strd in terms of word16

    ldr         r6,[sp,#dst_strd_offset]    @loading dst_strd
    ldr         r7,[sp,#zero_cols_offset]   @loading zero_cols
    add         r9,r0,r4                    @ pi2_src[0] + src_strd



    vld1.16     d4,[r8]                     @loading first row of g_ai2_ihevc_trans_4_transpose
    @ d4 = {36,64,83,64}
    @index = 3  2  1  0
    add         r10,r9,r4, lsl #1           @ 3*src_strd
    add         r4,r4,r4
    vld1.16     d1,[r9]                     @loading pi2_src 2nd row
    vld1.16     d3,[r10]                    @loading pi2_src 4th row
    vld1.16     d0,[r0],r4                  @loading pi2_src 1st row
    vld1.16     d2,[r0],r4                  @loading pi2_src 3rd row


    @ first stage computation starts
    vmull.s16   q3,d1,d4[1]                 @83 * pi2_src[1]
    vmlal.s16   q3,d3,d4[3]                 @o[0] = 83 * pi2_src[1] + 36 * pi2_src[3]
    vmull.s16   q4,d1,d4[3]                 @36 * pi2_src[1]
    vld1.32     d22[0], [r2],r5
    vmlsl.s16   q4,d3,d4[1]                 @o[1] = 36 * pi2_src[1] - 83 * pi2_src[3]

    vaddl.s16   q5,d0,d2                    @pi2_src[0] + pi2_src[2]
    vsubl.s16   q6,d0,d2                    @pi2_src[0] - pi2_src[2]
    vshl.s32    q5,q5,#6                    @e[0] = 64*(pi2_src[0] + pi2_src[2])
    vshl.s32    q6,q6,#6                    @e[1] = 64*(pi2_src[0] - pi2_src[2])

    vadd.s32    q7,q5,q3                    @((e[0] + o[0] )
    vadd.s32    q8,q6,q4                    @((e[1] + o[1])
    vsub.s32    q9,q6,q4                    @((e[1] - o[1])
    vsub.s32    q10,q5,q3                   @((e[0] - o[0])

    vqrshrn.s32 d0,q7,#shift_stage1_idct    @pi2_out[0] = clip_s16((e[0] + o[0] + add)>>shift) )
    vqrshrn.s32 d1,q8,#shift_stage1_idct    @pi2_out[1] = clip_s16((e[1] + o[1] + add)>>shift) )
    vqrshrn.s32 d2,q9,#shift_stage1_idct    @pi2_out[2] = clip_s16((e[0] - o[0] + add)>>shift) )
    vqrshrn.s32 d3,q10,#shift_stage1_idct   @pi2_out[3] = clip_s16((e[0] - o[0] + add)>>shift) )

    vtrn.16     d0,d1
    vtrn.16     d2,d3
    vtrn.32     d0,d2
    vtrn.32     d1,d3

    @ first stage ends
    @ output in d0,d1,d2,d3
    @ second stage starts
    vmull.s16   q3,d1,d4[1]                 @83 * pi2_src[1]
    vld1.32     d22[1], [r2],r5
    vmlal.s16   q3,d3,d4[3]                 @o[0] = 83 * pi2_src[1] + 36 * pi2_src[3]
    vmull.s16   q4,d1,d4[3]                 @36 * pi2_src[1]
    vmlsl.s16   q4,d3,d4[1]                 @o[1] = 36 * pi2_src[1] - 83 * pi2_src[3]
    vld1.32     d23[0], [r2],r5

    vaddl.s16   q5,d0,d2                    @pi2_src[0] + pi2_src[2]
    vsubl.s16   q6,d0,d2                    @pi2_src[0] - pi2_src[2]
    vshl.s32    q5,q5,#6                    @e[0] = 64*(pi2_src[0] + pi2_src[2])
    vshl.s32    q6,q6,#6                    @e[1] = 64*(pi2_src[0] - pi2_src[2])


    vadd.s32    q7,q5,q3                    @((e[0] + o[0] )
    vadd.s32    q8,q6,q4                    @((e[1] + o[1])
    vsub.s32    q9,q6,q4                    @((e[1] - o[1])
    vsub.s32    q10,q5,q3                   @((e[0] - o[0])

    vqrshrn.s32 d0,q7,#shift_stage2_idct    @pi2_out[0] = clip_s16((e[0] + o[0] + add)>>shift) )
    vqrshrn.s32 d1,q8,#shift_stage2_idct    @pi2_out[1] = clip_s16((e[1] + o[1] + add)>>shift) )
    vqrshrn.s32 d2,q9,#shift_stage2_idct    @pi2_out[2] = clip_s16((e[0] - o[0] + add)>>shift) )
    vqrshrn.s32 d3,q10,#shift_stage2_idct   @pi2_out[3] = clip_s16((e[0] - o[0] + add)>>shift) )
    vld1.32     d23[1], [r2],r5

    vtrn.16     d0,d1
    vtrn.16     d2,d3
    vtrn.32     d0,d2
    vtrn.32     d1,d3
    @ second stage ends
    @ output in d0,d1,d2,d3
    @ second stage computation ends

    @ loading pred

    vaddw.u8    q0,q0,d22                   @ pi2_out(16bit) + pu1_pred(8bit)
    vaddw.u8    q1,q1,d23                   @ pi2_out(16bit) + pu1_pred(8bit)
    vqmovun.s16 d0,q0                       @ clip_u8(pi2_out(16bit) + pu1_pred(8bit))
    vqmovun.s16 d1,q1                       @ clip_u8(pi2_out(16bit) + pu1_pred(8bit))

    @ storing destination
    vst1.32     {d0[0]},[r3],r6
    vst1.32     {d0[1]},[r3],r6
    vst1.32     {d1[0]},[r3],r6
    vst1.32     {d1[1]},[r3],r6

    vpop        {d8  -  d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp





