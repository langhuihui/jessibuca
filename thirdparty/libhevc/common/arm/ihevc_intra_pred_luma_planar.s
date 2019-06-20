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




.globl ihevc_intra_pred_luma_planar_a9q
.extern gau1_ihevc_planar_factor
.extern gau1_ihevc_planar_factor_1

gau1_ihevc_planar_factor_addr:
.long gau1_ihevc_planar_factor - ulbl1 - 8

gau1_ihevc_planar_factor_1_addr:
.long gau1_ihevc_planar_factor_1 - ulbl2 - 8


.type ihevc_intra_pred_luma_planar_a9q, %function

ihevc_intra_pred_luma_planar_a9q:

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
    add         r6, r6, r0
    ldr         r7, [r6]
    vdup.s8     d0, r7                      @src[nt-1]

    add         r6, r4, r4,lsl #1           @3nt
    add         r6, r6, #1                  @3nt + 1
    add         r6, r6, r0
    ldr         r7, [r6]
    vdup.s8     d1, r7                      @src[3nt+1]

    add         r6, r4, r4                  @2nt
    add         r14, r6, #1                 @2nt+1
    sub         r6, r6, #1                  @2nt-1
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

@@ ========== ***************** =====================
prolog:
tf_sz_8_16_32:

    mov         r7, r4                      @column counter (set to no of cols)
    mov         r9, r4, lsr #3              @divide nt by 8
    mul         r7, r7, r9                  @multiply width * height
    ldr         r5, gau1_ihevc_planar_factor_1_addr @loads table of coeffs
ulbl2:
    add         r5,r5,pc
    sub         r6, r6, #7
    mov         r8, r2
    lsl         r9, r3, #3                  @4*stride
    rsb         r9, r9, #8                  @8-4*stride
    mov         r10, r4                     @nt
    sub         r10, r10, #8                @nt - 8

col_loop_8_16_32:

    vld1.s8     d8, [r12]                   @(1-8)load 8 coeffs [col+1]
    vdup.16     q6, r4                      @(1)
    vld1.s8     d4, [r6]                    @(1-8)src[2nt-1-row]
    vsub.s8     d9, d2, d8                  @(1-8)[nt-1-col]


    vmlal.u8    q6, d5, d0                  @(1)(row+1) *   src[nt-1]

    vld1.s8     d3, [r14]                   @(1-8)load 8 src[2nt+1+col]
    vmlal.u8    q6, d8, d1                  @(1)(col+1) *   src[3nt+1]

    vdup.s8     d20, d4[7]                  @(1)
    vmlal.u8    q6, d6, d3                  @(1)(nt-1-row)  *   src[2nt+1+col]

    vdup.s8     d21, d4[6]                  @(2)
    vmlal.u8    q6, d9, d20                 @(1)(nt-1-col)  *   src[2nt-1-row]

    vdup.16     q15, r4                     @(2)
    vadd.s8     d5, d5, d7                  @(1)

    vsub.s8     d6, d6, d7                  @(1)

    vdup.s8     d22, d4[5]                  @(3)
    vmlal.u8    q15, d5, d0                 @(2)

    vdup.16     q14, r4                     @(3)
    vmlal.u8    q15, d8, d1                 @(2)

    vmlal.u8    q15, d6, d3                 @(2)
    vmlal.u8    q15, d9, d21                @(2)

    vshl.s16    q6, q6, q7                  @(1)shr

    vadd.s8     d5, d5, d7                  @(2)
    vsub.s8     d6, d6, d7                  @(2)

    vmovn.i16   d12, q6                     @(1)
    vmlal.u8    q14, d5, d0                 @(3)

    vdup.8      d23, d4[4]                  @(4)
    vmlal.u8    q14, d8, d1                 @(3)

    vdup.16     q5, r4                      @(4)
    vmlal.u8    q14, d6, d3                 @(3)

    vst1.s8     d12, [r2], r3               @(1)str 8 values
    vmlal.u8    q14, d9, d22                @(3)

    vshl.s16    q15, q15, q7                @(2)shr

    vadd.s8     d5, d5, d7                  @(3)
    vsub.s8     d6, d6, d7                  @(3)

    vmovn.i16   d30, q15                    @(2)
    vmlal.u8    q5, d5, d0                  @(4)

    vdup.8      d20, d4[3]                  @(5)
    vmlal.u8    q5, d8, d1                  @(4)

    vdup.16     q8, r4                      @(5)
    vmlal.u8    q5, d6, d3                  @(4)

    vst1.s8     d30, [r2], r3               @(2)str 8 values
    vmlal.u8    q5, d9, d23                 @(4)

    vshl.s16    q14, q14, q7                @(3)shr

    vadd.s8     d5, d5, d7                  @(4)
    vsub.s8     d6, d6, d7                  @(4)

    vmovn.i16   d28, q14                    @(3)
    vmlal.u8    q8, d5, d0                  @(5)

    vdup.8      d21, d4[2]                  @(6)
    vmlal.u8    q8, d8, d1                  @(5)

    vdup.16     q9, r4                      @(6)
    vmlal.u8    q8, d6, d3                  @(5)

    vst1.s8     d28, [r2], r3               @(3)str 8 values
    vmlal.u8    q8, d9, d20                 @(5)

    vshl.s16    q5, q5, q7                  @(4)shr
    vadd.s8     d5, d5, d7                  @(5)
    vsub.s8     d6, d6, d7                  @(5)

    vmovn.i16   d10, q5                     @(4)
    vmlal.u8    q9, d5, d0                  @(6)

    vdup.8      d22, d4[1]                  @(7)
    vmlal.u8    q9, d8, d1                  @(6)

    vdup.16     q13, r4                     @(7)
    vmlal.u8    q9, d6, d3                  @(6)

    vst1.s8     d10, [r2], r3               @(4)str 8 values
    vmlal.u8    q9, d9, d21                 @(6)

    vshl.s16    q8, q8, q7                  @(5)shr

    vadd.s8     d5, d5, d7                  @(6)
    vsub.s8     d6, d6, d7                  @(6)

    vmovn.i16   d16, q8                     @(5)
    vmlal.u8    q13, d5, d0                 @(7)

    vdup.8      d23, d4[0]                  @(8)
    vmlal.u8    q13, d8, d1                 @(7)

    vdup.16     q12, r4                     @(8)
    vmlal.u8    q13, d6, d3                 @(7)

    vst1.s8     d16, [r2], r3               @(5)str 8 values
    vmlal.u8    q13, d9, d22                @(7)

    vshl.s16    q9, q9, q7                  @(6)shr

    vadd.s8     d5, d5, d7                  @(7)
    vsub.s8     d6, d6, d7                  @(7)

    vmovn.i16   d18, q9                     @(6)
    vmlal.u8    q12, d5, d0                 @(8)


    vmlal.u8    q12, d8, d1                 @(8)

    vmlal.u8    q12, d6, d3                 @(8)

    vst1.s8     d18, [r2], r3               @(6)str 8 values
    vmlal.u8    q12, d9, d23                @(8)

    vshl.s16    q13, q13, q7                @(7)shr

    subs        r7, r7, #8

    beq         epilog

    subs        r1, r1, #8                  @row counter
    addgt       r12, r12, #8                @col inc
    addgt       r14, r14, #8                @also for col inc
    movle       r1, r4                      @nt reloaded (refresh the value)
    addle       r12, r11, #1                @r12 reset

    movle       r14, r0                     @r14 reset
    vld1.s8     d8, [r12]                   @(1n)(1-8)load 8 coeffs [col+1]

    suble       r6, r6, #8                  @for next set of rows
    vld1.s8     d3, [r14]                   @(1n)(1-8)load 8 src[2nt+1+col]

    addle       r5, r5, #8
    vdup.16     q6, r4                      @(1n)(1)

    vld1.s8     d5, [r5]

    vld1.s8     d4, [r6]                    @(1n)(1-8)src[2nt-1-row]
    vsub.s8     d9, d2, d8                  @(1n)(1-8)[nt-1-col]

    vdup.s8     d20, d4[7]                  @(1n)(1)
    vsub.s8     d6, d2, d5

    beq         epilog

kernel_plnr:

    cmp         r1, #0                      @ (cond loop)
    vshl.s16    q12, q12, q7                @(8)shr

    vmovn.i16   d26, q13                    @(7)
    vmlal.u8    q6, d5, d0                  @(1)(row+1) *   src[nt-1]

    vmovn.i16   d24, q12                    @(8)
    vmlal.u8    q6, d8, d1                  @(1)(col+1) *   src[3nt+1]

    vdup.s8     d21, d4[6]                  @(2)
    vmlal.u8    q6, d6, d3                  @(1)(nt-1-row)  *   src[2nt+1+col]

    vdup.16     q15, r4                     @(2)
    vmlal.u8    q6, d9, d20                 @(1)(nt-1-col)  *   src[2nt-1-row]

    vst1.s8     d26, [r2], r3               @(7)str 8 values
    vadd.s8     d5, d5, d7                  @(1)

    vst1.s8     d24, [r2], r3               @(8)str 8 values
    vsub.s8     d6, d6, d7                  @(1)

    addgt       r2, r2, r9                  @since more cols to fill, dst + 8 - 6*strd (cond loop)
    vmlal.u8    q15, d5, d0                 @(2)

    suble       r2, r2, r10                 @else go to next set of rows, dst - (nt-8) (cond loop)
    vmlal.u8    q15, d8, d1                 @(2)

    vdup.s8     d22, d4[5]                  @(3)
    vmlal.u8    q15, d6, d3                 @(2)

    vdup.16     q14, r4                     @(3)
    vmlal.u8    q15, d9, d21                @(2)

    vshl.s16    q6, q6, q7                  @(1)shr

    vadd.s8     d5, d5, d7                  @(2)
    movle       r1, r4                      @nt reloaded (refresh the value)    (cond loop)

    vsub.s8     d6, d6, d7                  @(2)
    subs        r1, r1, #8                  @row counter (loop)

    vmovn.i16   d12, q6                     @(1)
    vmlal.u8    q14, d5, d0                 @(3)

    vdup.8      d23, d4[4]                  @(4)
    vmlal.u8    q14, d8, d1                 @(3)

    vdup.16     q5, r4                      @(4)
    vmlal.u8    q14, d6, d3                 @(3)

    vst1.s8     d12, [r2], r3               @(1)str 8 values
    vmlal.u8    q14, d9, d22                @(3)

    vshl.s16    q15, q15, q7                @(2)shr

    vadd.s8     d5, d5, d7                  @(3)

    vsub.s8     d6, d6, d7                  @(3)

    vmovn.i16   d30, q15                    @(2)
    vmlal.u8    q5, d5, d0                  @(4)

    vdup.8      d20, d4[3]                  @(5)
    vmlal.u8    q5, d8, d1                  @(4)

    vdup.16     q8, r4                      @(5)
    vmlal.u8    q5, d6, d3                  @(4)

    vst1.s8     d30, [r2], r3               @(2)str 8 values
    vmlal.u8    q5, d9, d23                 @(4)

    vshl.s16    q14, q14, q7                @(3)shr

    vadd.s8     d5, d5, d7                  @(4)

    vsub.s8     d6, d6, d7                  @(4)

    vmovn.i16   d28, q14                    @(3)
    vmlal.u8    q8, d5, d0                  @(5)

    vdup.8      d21, d4[2]                  @(6)
    vmlal.u8    q8, d8, d1                  @(5)

    vdup.16     q9, r4                      @(6)
    vmlal.u8    q8, d6, d3                  @(5)

    vst1.s8     d28, [r2], r3               @(3)str 8 values
    vmlal.u8    q8, d9, d20                 @(5)

    addle       r12, r11, #1                @r12 reset (cond loop)
    vshl.s16    q5, q5, q7                  @(4)shr

    addgt       r12, r12, #8                @col inc (cond loop)
    vadd.s8     d5, d5, d7                  @(5)

    addgt       r14, r14, #8                @also for col inc (cond loop)
    vsub.s8     d6, d6, d7                  @(5)

    vmovn.i16   d10, q5                     @(4)
    vmlal.u8    q9, d5, d0                  @(6)

    vdup.8      d22, d4[1]                  @(7)
    vmlal.u8    q9, d8, d1                  @(6)

    vdup.16     q13, r4                     @(7)
    vmlal.u8    q9, d6, d3                  @(6)

    vst1.s8     d10, [r2], r3               @(4)str 8 values
    vmlal.u8    q9, d9, d21                 @(6)

    movle       r14, r0                     @r14 reset (cond loop)
    vshl.s16    q8, q8, q7                  @(5)shr

    suble       r6, r6, #8                  @for next set of rows (cond loop)
    vadd.s8     d5, d5, d7                  @(6)

    addle       r5, r5, #8                  @ (cond loop)
    vsub.s8     d6, d6, d7                  @(6)

    vmovn.i16   d16, q8                     @(5)
    vmlal.u8    q13, d5, d0                 @(7)

    vdup.8      d23, d4[0]                  @(8)
    vmlal.u8    q13, d8, d1                 @(7)

    vdup.16     q12, r4                     @(8)
    vmlal.u8    q13, d6, d3                 @(7)

    vst1.s8     d16, [r2], r3               @(5)str 8 values
    vmlal.u8    q13, d9, d22                @(7)

    vld1.s8     d4, [r6]                    @(1n)(1-8)src[2nt-1-row]
    vshl.s16    q9, q9, q7                  @(6)shr

    vadd.s8     d5, d5, d7                  @(7)

    vsub.s8     d6, d6, d7                  @(7)

    vmovn.i16   d18, q9                     @(6)
    vmlal.u8    q12, d5, d0                 @(8)

    vld1.s8     d5, [r5]                    @(row+1 value)
    vmlal.u8    q12, d8, d1                 @(8)

    vdup.s8     d20, d4[7]                  @(1n)(1)
    vmlal.u8    q12, d6, d3                 @(8)

    vst1.s8     d18, [r2], r3               @(6)str 8 values
    vmlal.u8    q12, d9, d23                @(8)

    vld1.s8     d8, [r12]                   @(1n)(1-8)load 8 coeffs [col+1]
    vsub.s8     d6, d2, d5                  @(nt-1-row) value

    subs        r7, r7, #8                  @col counter

    vld1.s8     d3, [r14]                   @(1n)(1-8)load 8 src[2nt+1+col]
    vshl.s16    q13, q13, q7                @(7)shr

    vdup.16     q6, r4                      @(1n)(1)
    vsub.s8     d9, d2, d8                  @(1n)(1-8)[nt-1-col]

    bne         kernel_plnr

epilog:

    vmovn.i16   d26, q13                    @(7)
    vst1.s8     d26, [r2], r3               @(7)str 8 values

    vshl.s16    q12, q12, q7                @(8)shr
    vmovn.i16   d24, q12                    @(8)
    vst1.s8     d24, [r2], r3               @(8)str 8 values

@@ ========== ***************** =====================

    beq         end_loop

tf_sz_4:
    vld1.s8     d10, [r14]                  @load src[2nt+1+col]
    vld1.s8     d8, [r12], r10              @load 8 coeffs [col+1]
loop_sz_4:
    mov         r10, #4                     @reduce inc to #4 for 4x4
    ldr         r7, [r6], #-1               @src[2nt-1-row] (dec to take into account row)
    vdup.s8     d4, r7                      @src[2nt-1-row]

    vsub.s8     d9, d2, d8                  @[nt-1-col]

    vmull.u8    q6, d5, d0                  @(row+1)    *   src[nt-1]
    vmlal.u8    q6, d6, d10                 @(nt-1-row) *   src[2nt+1+col]
    vmlal.u8    q6, d8, d1                  @(col+1)    *   src[3nt+1]
    vmlal.u8    q6, d9, d4                  @(nt-1-col) *   src[2nt-1-row]
@   vadd.i16    q6, q6, q8          @add (nt)
@   vshl.s16    q6, q6, q7          @shr
@   vmovn.i16   d12, q6
    vrshrn.s16  d12,q6,#3
    vst1.s32    {d12[0]}, [r2], r3

    vadd.s8     d5, d5, d7                  @row++ [(row+1)++]
    vsub.s8     d6, d6, d7                  @[nt-1-row]--
    subs        r1, r1, #1

    bne         loop_sz_4

end_loop:
    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp








