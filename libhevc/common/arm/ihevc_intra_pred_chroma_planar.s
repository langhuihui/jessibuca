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
@*******************************************************************************
@* @file
@*  ihevc_intra_pred_filters_planar.s
@*
@* @brief
@*  contains function definitions for inter prediction  interpolation.
@* functions are coded using neon  intrinsics and can be compiled using

@* rvct
@*
@* @author
@*  akshaya mukund
@*
@* @par list of functions:
@*
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/
@/**
@*******************************************************************************
@*
@* @brief
@*    luma intraprediction filter for planar input
@*
@* @par description:
@*
@* @param[in] pu1_ref
@*  uword8 pointer to the source
@*
@* @param[out] pu1_dst
@*  uword8 pointer to the destination
@*
@* @param[in] src_strd
@*  integer source stride
@*
@* @param[in] dst_strd
@*  integer destination stride
@*
@* @param[in] pi1_coeff
@*  word8 pointer to the planar coefficients
@*
@* @param[in] nt
@*  size of tranform block
@*
@* @param[in] mode
@*  type of filtering
@*
@* @returns
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/

@void ihevc_intra_pred_luma_planar(uword8* pu1_ref,
@                                  word32 src_strd,
@                                  uword8* pu1_dst,
@                                  word32 dst_strd,
@                                  word32 nt,
@                                  word32 mode,
@                  word32 pi1_coeff)
@**************variables vs registers*****************************************
@r0 => *pu1_ref
@r1 => src_strd
@r2 => *pu1_dst
@r3 => dst_strd

@stack contents from #104
@   nt
@   mode
@   pi1_coeff

.equ    nt_offset,      104

.text
.align 4




.globl ihevc_intra_pred_chroma_planar_a9q
.extern gau1_ihevc_planar_factor

gau1_ihevc_planar_factor_addr:
.long gau1_ihevc_planar_factor - ulbl1 - 8

.type ihevc_intra_pred_chroma_planar_a9q, %function

ihevc_intra_pred_chroma_planar_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8 - d15}

    ldr         r4,[sp,#nt_offset]          @loads nt
    ldr         r11, gau1_ihevc_planar_factor_addr @loads table of coeffs
ulbl1:
    add         r11,r11,pc

    clz         r5, r4
    rsb         r5, r5, #32
    vdup.16     q7, r5
    vneg.s16    q7, q7                      @shr value (so vneg)
    vdup.8      d2, r4                      @nt
    vdup.s16    q8, r4                      @nt

    sub         r6, r4, #1                  @nt-1
    add         r6, r0,r6,lsl #1            @2*(nt-1)
    ldr         r7, [r6]
    vdup.s16    d0, r7                      @src[nt-1]

    add         r6, r4, r4,lsl #1           @3nt
    add         r6, r6, #1                  @3nt + 1
    lsl         r6,r6,#1                    @2*(3nt + 1)

    add         r6, r6, r0
    ldr         r7, [r6]
    vdup.s16    d1, r7                      @src[3nt+1]


    add         r6, r4, r4                  @2nt
    add         r14, r6, #1                 @2nt+1
    lsl         r14,#1                      @2*(2nt+1)
    sub         r6, r6, #1                  @2nt-1
    lsl         r6,#1                       @2*(2nt-1)
    add         r6, r6, r0                  @&src[2nt-1]
    add         r14, r14, r0                @&src[2nt+1]

    mov         r8, #1                      @row+1 (row is first 0)
    sub         r9, r4, r8                  @nt-1-row (row is first 0)

    vdup.s8     d5, r8                      @row + 1
    vdup.s8     d6, r9                      @nt - 1 - row
    vmov        d7, d5                      @mov #1 to d7 to used for inc for row+1 and dec for nt-1-row

    add         r12, r11, #1                @coeffs (to be reloaded after every row)
    mov         r1, r4                      @nt (row counter) (dec after every row)
    mov         r5, r2                      @dst (to be reloaded after every row and inc by dst_strd)
    mov         r10, #8                     @increment for the coeffs
    mov         r0, r14                     @&src[2nt+1] (to be reloaded after every row)

    cmp         r4, #4
    beq         tf_sz_4



    mov         r10,r6
tf_sz_8_16:
    vld1.s8     {d10,d11}, [r14]!           @load src[2nt+1+col]
    vld1.s8     d8, [r12]!
    vmov        d9,d8
    vzip.8      d8,d9
    vsub.s8     d30, d2, d8                 @[nt-1-col]
    vsub.s8     d31, d2, d9




loop_sz_8_16:

    ldr         r7, [r6], #-2               @src[2nt-1-row] (dec to take into account row)
    vmull.u8    q6, d5, d0                  @(row+1)    *   src[nt-1]
    ldr         r11, [r6], #-2              @src[2nt-1-row] (dec to take into account row)
    vmlal.u8    q6, d6, d10                 @(nt-1-row) *   src[2nt+1+col]
    vdup.s16    d4, r7                      @src[2nt-1-row]
    vmlal.u8    q6, d8, d1                  @(col+1)    *   src[3nt+1]
    vdup.s16    d3, r11                     @src[2nt-1-row]
    vmlal.u8    q6, d30, d4                 @(nt-1-col) *   src[2nt-1-row]



    vmull.u8    q14,d5,d0
    ldr         r7, [r6], #-2               @src[2nt-1-row] (dec to take into account row)
    vmlal.u8    q14,d6,d11
    vadd.s8     d18, d5, d7                 @row++ [(row+1)++]c


    vmlal.u8    q14,d31,d4
    vsub.s8     d19, d6, d7                 @[nt-1-row]--
    vmlal.u8    q14,d9,d1
    vdup.s16    d4, r7                      @src[2nt-1-row]

    vmull.u8    q13, d18, d0                @(row+1)    *   src[nt-1]
    vadd.i16    q6, q6, q8                  @add (nt)
    vmlal.u8    q13, d19, d10               @(nt-1-row) *   src[2nt+1+col]
    vshl.s16    q6, q6, q7                  @shr
    vmlal.u8    q13, d8, d1                 @(col+1)    *   src[3nt+1]
    vadd.i16    q14,q14,q8
    vmlal.u8    q13, d30, d3                @(nt-1-col) *   src[2nt-1-row]
    vshl.s16    q14,q14,q7





    vmull.u8    q12,d18,d0
    vadd.s8     d5, d18, d7                 @row++ [(row+1)++]
    vmlal.u8    q12,d19,d11
    vsub.s8     d6, d19, d7                 @[nt-1-row]--
    vmlal.u8    q12,d9,d1
    vmovn.i16   d12, q6
    vmlal.u8    q12,d31,d3
    vmovn.i16   d13,q14




    vadd.i16    q13, q13, q8                @add (nt)
    vmull.u8    q11, d5, d0                 @(row+1)    *   src[nt-1]
    vshl.s16    q13, q13, q7                @shr
    vmlal.u8    q11, d6, d10                @(nt-1-row) *   src[2nt+1+col]
    vst1.s32    {d12,d13}, [r2], r3
    vmlal.u8    q11, d8, d1                 @(col+1)    *   src[3nt+1]
    vadd.i16    q12,q12,q8
    vmlal.u8    q11, d30, d4                @(nt-1-col) *   src[2nt-1-row]
    vshl.s16    q12,q12,q7

    vmull.u8    q10,d5,d0
    vadd.s8     d18, d5, d7                 @row++ [(row+1)++]c
    vmlal.u8    q10,d6,d11
    vsub.s8     d19, d6, d7                 @[nt-1-row]--
    vmlal.u8    q10,d31,d4

    ldr         r11, [r6], #-2              @src[2nt-1-row] (dec to take into account row)
    vmlal.u8    q10,d9,d1
    vdup.s16    d3, r11                     @src[2nt-1-row]
    vadd.i16    q11, q11, q8                @add (nt)

    vmull.u8    q6, d18, d0                 @(row+1)    *   src[nt-1]
    vmovn.i16   d26, q13
    vmlal.u8    q6, d19, d10                @(nt-1-row) *   src[2nt+1+col]
    vmovn.i16   d27,q12

    vmlal.u8    q6, d8, d1                  @(col+1)    *   src[3nt+1]
    vshl.s16    q11, q11, q7                @shr

    vmlal.u8    q6, d30, d3                 @(nt-1-col) *   src[2nt-1-row]
    vadd.i16    q10,q10,q8

    vmull.u8    q14,d18,d0
    vst1.s32    {d26,d27}, [r2], r3

    vmlal.u8    q14,d19,d11
    vadd.s8     d5, d18, d7                 @row++ [(row+1)++]

    vsub.s8     d6, d19, d7                 @[nt-1-row]--
    vmlal.u8    q14,d9,d1

    vmlal.u8    q14,d31,d3
    vshl.s16    q10,q10,q7


    vadd.i16    q6, q6 ,q8                  @add (nt)
    vmovn.i16   d22, q11


    vadd.i16    q14,q14,q8
    vmovn.i16   d23,q10


    vshl.s16    q6, q6, q7                  @shr
    vst1.s32    {d22,d23}, [r2], r3
    vshl.s16    q14,q14,q7





    vmovn.i16   d20, q6
    vmovn.i16   d21,q14

    vst1.s32    {d20,d21}, [r2], r3


    subs        r1, r1, #4

    bne         loop_sz_8_16




    cmp         r4,#16

    bne         end_loop


    sub         r4,#16
    vdup.s8     d5, r8                      @row + 1
    vdup.s8     d6, r9                      @nt - 1 - row
    vmov        d7, d5                      @mov #1 to d7 to used for inc for row+1 and dec for nt-1-row

    mov         r6,r10
    mov         r1,#16
    sub         r2,r2,r3,lsl #4
    add         r2,r2,#16

    vld1.s8     {d10,d11}, [r14]!           @load src[2nt+1+col]
    vld1.s8     d8, [r12]!
    vmov        d9,d8
    vzip.8      d8,d9
    vsub.s8     d30, d2, d8                 @[nt-1-col]
    vsub.s8     d31, d2, d9

    beq         loop_sz_8_16



tf_sz_4:
    vld1.s8     d10, [r14]                  @load src[2nt+1+col]
    vld1.s8     d8, [r12], r10              @load 8 coeffs [col+1]
    vmov        d9,d8
    vzip.8      d8,d9
loop_sz_4:
    @mov        r10, #4             @reduce inc to #4 for 4x4
    ldr         r7, [r6], #-2               @src[2nt-1-row] (dec to take into account row)
    vdup.s16    d4, r7                      @src[2nt-1-row]

    vsub.s8     d9, d2, d8                  @[nt-1-col]

    vmull.u8    q6, d5, d0                  @(row+1)    *   src[nt-1]
    vmlal.u8    q6, d6, d10                 @(nt-1-row) *   src[2nt+1+col]
    vmlal.u8    q6, d8, d1                  @(col+1)    *   src[3nt+1]
    vmlal.u8    q6, d9, d4                  @(nt-1-col) *   src[2nt-1-row]
@   vadd.i16    q6, q6, q8          @add (nt)
@   vshl.s16    q6, q6, q7          @shr
@   vmovn.i16   d12, q6
    vrshrn.s16  d12,q6,#3

    vst1.s32    {d12}, [r2], r3

    vadd.s8     d5, d5, d7                  @row++ [(row+1)++]
    vsub.s8     d6, d6, d7                  @[nt-1-row]--
    subs        r1, r1, #1

    bne         loop_sz_4

end_loop:
    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp







