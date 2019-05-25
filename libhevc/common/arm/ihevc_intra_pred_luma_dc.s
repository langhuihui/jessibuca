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
@*  ihevc_intra_pred_filters_dc.s
@*
@* @brief
@*  contains function definitions for intra prediction dc filtering.
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
@*    luma intraprediction filter for dc input
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

@void ihevc_intra_pred_luma_dc(uword8 *pu1_ref,
@                              word32 src_strd,
@                              uword8 *pu1_dst,
@                              word32 dst_strd,
@                              word32 nt,
@                              word32 mode)
@
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




.globl ihevc_intra_pred_luma_dc_a9q

.type ihevc_intra_pred_luma_dc_a9q, %function

ihevc_intra_pred_luma_dc_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8 - d15}
    ldr         r4,[sp,#nt_offset]          @loads nt

@********** testing
    @mov        r6, #128
    @b      prologue_cpy_32
@********** testing

    mov         r11, #2                     @mov #2 to r11 (to be used to add to 2dc_val & 3dc_val)
    mov         r9, #0
    vmov        d17, r11, r9

    clz         r5, r4

    add         r6, r0, r4                  @&src[nt]
    rsb         r5, r5, #32                 @log2nt
    add         r7, r0, r4, lsl #1          @&src[2nt]

    add         r8, r7, #1                  @&src[2nt+1]
    mvn         r5, r5
    add         r5, r5, #1
    vdup.32     d8, r5

    ldrb        r14, [r8]
    vshl.i64    d8, d8, #32

    sub         r9, r7, #1                  @&src[2nt-1]
    vshr.s64    d8, d8, #32

    mov         r7, r8                      @r7 also stores 2nt+1

    ldrb        r12, [r9]
    add         r14, r14, r12               @src[2nt+1] + src[2nt-1]
    add         r14, r14, r11               @src[2nt+1] + src[2nt-1] + 2

    cmp         r4, #4
    beq         dc_4

    mov         r10, r4                     @nt

add_loop:
    vld1.s8     d0, [r6]!                   @load from src[nt]
    mov         r5, #0                      @
    vld1.s8     d1, [r8]!                   @load from src[2nt+1]

    vpaddl.u8   d2, d0

    vmov        d6, r4, r5                  @store nt to accumulate
    vpaddl.u8   d3, d1

    vld1.s8     d0, [r6]!                   @load from src[nt] (extra load for 8)

    vld1.s8     d1, [r8]!                   @load from src[2nt+1] (extra load for 8)
    vadd.u16    d4, d2, d3


    vpaddl.u16  d5, d4


    vpadal.u32  d6, d5                      @accumulate all inp into d6 (end for nt==8)

    subs        r10, #8
    beq         epil_add_loop

core_loop_add:
    vpaddl.u8   d2, d0
    subs        r10, #8
    vpaddl.u8   d3, d1



    vadd.u16    d4, d2, d3
    vld1.s8     d0, [r6]!                   @load from src[nt] (extra load for 16)

    vpaddl.u16  d5, d4
    vld1.s8     d1, [r8]!                   @load from src[2nt+1] (extra load for 16)

    vpadal.u32  d6, d5                      @accumulate all inp into d6
    bne         core_loop_add

epil_add_loop:

    vshl.s64    d9, d6, d8                  @(dc_val) shr by log2nt+1
    cmp         r4, #32

    vmov        d28, r14, r5                @src[2nt+1]+2+src[2nt-1] moved to d28
    moveq       r6, #128

    vdup.8      d16, d9[0]                  @dc_val
    vshl.s64    d13, d9, #1                 @2*dc

    beq         prologue_cpy_32

    vadd.i64    d14, d13, d28               @src[2nt+1]+2+src[2nt-1]+2dc_val
    movne       r6, #0                      @nt

    vshr.u16    d15, d14, #2                @final dst[0]'s value in d15[0]
    movne       r10, r4

    vadd.i64    d11, d13, d9                @3*dc
    sub         r12, r3, r3, lsl #3         @-7*strd

    vadd.i64    d11, d11, d17               @3*dc + 2
    add         r12, r12, #8                @offset after one 8x8 block (-7*strd + 8)

    vdup.16     q12, d11[0]                 @3*dc + 2 (moved to all lanes)
    sub         r0, r3, r4                  @strd - nt

prologue_col:
    @0th column and 0-7 rows done here
    @r8 and r9 (2nt+1+col 2nt-1-row)

    mov         r8, r7                      @&src[2nt+1]

    add         r0, r0, #8                  @strd - nt + 8
    vld1.s8     d0, [r8]!                   @col 1::7 load (prol)
    sub         r9, r9, #7                  @&src[2nt-1-row]

    vld1.s8     d1, [r9]                    @row 7::1 (0 also) load (prol)
    sub         r9, r9, #8

    vmovl.u8    q10, d0

    vld1.s8     d6, [r8]                    @col 8::15 load (prol extra)
    vadd.i16    q10, q10, q12               @col 1::7 add 3dc+2 (prol)

    vmovl.u8    q11, d1
    vqshrun.s16 d2, q10, #2                 @columns shr2 movn (prol)

    vmovl.u8    q13, d6
    vadd.i16    q11, q11, q12               @row 1::7 add 3dc+2 (prol)

    vmov.i64    d19, #0x00000000000000ff    @
    vqshrun.s16 d3, q11, #2                 @rows shr2 movn (prol)

    vbsl        d19, d15, d2                @first row with dst[0]
    vadd.i16    q13, q13, q12               @col 8::15 add 3dc+2 (prol extra)

    vrev64.8    d3, d3

    vst1.8      d19, [r2], r3               @store row 0 (prol)
    vshr.s64    d3, d3, #8                  @row 0 shift (prol) (first value to be ignored)

    vmov.i64    d20, #0x00000000000000ff    @byte mask row 1 (prol)

loop_again_col_row:

    vbsl        d20, d3, d16                @row 1  (prol)

    vmov.i64    d21, #0x00000000000000ff    @byte mask row 2 (prol)
    vshr.s64    d3, d3, #8                  @row 1 shift (prol)

    vst1.8      d20, [r2], r3               @store row 1 (prol)
    vqshrun.s16 d4, q13, #2                 @columns shr2 movn (prol extra)


    vbsl        d21, d3, d16                @row 2 (prol)

    vmov.i64    d20, #0x00000000000000ff    @byte mask row 3 (prol)
    vshr.s64    d3, d3, #8                  @row 2 shift (prol)

    vst1.8      d21, [r2], r3               @store row 2 (prol)


    vbsl        d20, d3, d16                @row 3  (prol)

    vmov.i64    d21, #0x00000000000000ff    @byte mask row 4 (prol)
    vshr.s64    d3, d3, #8                  @row 3 shift (prol)

    vst1.8      d20, [r2], r3               @store row 3 (prol)


    vbsl        d21, d3, d16                @row 4 (prol)

    vmov.i64    d20, #0x00000000000000ff    @byte mask row 5 (prol)
    vshr.s64    d3, d3, #8                  @row 4 shift (prol)

    vst1.8      d21, [r2], r3               @store row 4 (prol)


    vbsl        d20, d3, d16                @row 5 (prol)

    vmov.i64    d21, #0x00000000000000ff    @byte mask row 6 (prol)
    vshr.s64    d3, d3, #8                  @row 5 shift (prol)

    vst1.8      d20, [r2], r3               @store row 5 (prol)

    vld1.s8     d1, [r9]                    @row 8::15 load (prol extra)

    vbsl        d21, d3, d16                @row 6 (prol)

    vmovl.u8    q11, d1

    vmov.i64    d20, #0x00000000000000ff    @byte mask row 7 (prol)
    vshr.s64    d3, d3, #8                  @row 6 shift (prol)

    vst1.8      d21, [r2], r3               @store row 6 (prol)

    vbsl        d20, d3, d16                @row 7 (prol)
    vadd.i16    q11, q11, q12               @row 8::15 add 3dc+2 (prol extra)

    vshr.s64    d3, d3, #8                  @row 7 shift (prol)
    vst1.8      d20, [r2], r12              @store row 7 (prol)

    subs        r10, r10, #8                @counter for cols

    beq         end_func
    blt         copy_16


    vmov.i64    d20, #0x00000000000000ff    @byte mask row 9 (prol)
    vqshrun.s16 d3, q11, #2                 @rows shr2 movn (prol)

    vrev64.8    d3, d3

    vst1.8      d4, [r2], r3                @store 2nd col (for 16x16)

    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r0               @go to next row for 16


    vbsl        d20, d3, d16                @row 9  (prol)
    subs        r10, r10, #8

    vst1.8      d20, [r2], r3               @store row 9 (prol)
    vshr.s64    d3, d3, #8                  @row 9 shift (prol)

    vmov.i64    d20, #0x00000000000000ff    @byte mask row 9 (prol)

    b           loop_again_col_row


copy_16:
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2], r3
    vst1.8      d16, [r2]

    b           end_func

prologue_cpy_32:
    mov         r9, #128
    @sub        r7, r3, #-24
    add         r5, r2, r3
    add         r8, r5, r3
    add         r10, r8, r3
    vdup.8      q10, d16[0]
    lsl         r6, r3, #2
    add         r6, r6, #0xfffffff0

    vst1.8      {d20,d21}, [r2]!
    vst1.8      {d20,d21}, [r5]!
    vst1.8      {d20,d21}, [r8]!
    vst1.8      {d20,d21}, [r10]!

    vst1.8      {d20,d21}, [r2], r6
    vst1.8      {d20,d21}, [r5], r6
    vst1.8      {d20,d21}, [r8], r6
    vst1.8      {d20,d21}, [r10], r6

    sub         r9, r9, #32                 @32x32 prol/epil counter dec

kernel_copy:
    vst1.8      {d20,d21}, [r2]!
    vst1.8      {d20,d21}, [r5]!
    vst1.8      {d20,d21}, [r8]!
    vst1.8      {d20,d21}, [r10]!

    vst1.8      {d20,d21}, [r2], r6
    vst1.8      {d20,d21}, [r5], r6
    vst1.8      {d20,d21}, [r8], r6
    vst1.8      {d20,d21}, [r10], r6

    subs        r9, r9, #32

    vst1.8      {d20,d21}, [r2]!
    vst1.8      {d20,d21}, [r5]!
    vst1.8      {d20,d21}, [r8]!
    vst1.8      {d20,d21}, [r10]!

    vst1.8      {d20,d21}, [r2], r6
    vst1.8      {d20,d21}, [r5], r6
    vst1.8      {d20,d21}, [r8], r6
    vst1.8      {d20,d21}, [r10], r6

    bne         kernel_copy

epilogue_copy:
    vst1.8      {d20,d21}, [r2]!
    vst1.8      {d20,d21}, [r5]!
    vst1.8      {d20,d21}, [r8]!
    vst1.8      {d20,d21}, [r10]!

    vst1.8      {d20,d21}, [r2]
    vst1.8      {d20,d21}, [r5]
    vst1.8      {d20,d21}, [r8]
    vst1.8      {d20,d21}, [r10]

    b           end_func


dc_4:
    vld1.s8     d0, [r6]!                   @load from src[nt]
    vld1.s8     d1, [r8]!                   @load from src[2nt+1]

    vpaddl.u8   d2, d0
    mov         r5, #0                      @
    vmov        d6, r4, r5                  @store nt to accumulate
    vpaddl.u8   d3, d1

    vadd.u16    d4, d2, d3


    vpaddl.u16  d5, d4
    vmov.i64    d30, #0x00000000ffffffff

    vand        d5, d5, d30

    vmov        d28, r14, r5                @src[2nt+1]+2+src[2nt-1] moved to d28
    vadd.i64    d6, d6, d5                  @accumulate all inp into d6 (end for nt==8)

    vshl.s64    d9, d6, d8                  @(dc_val) shr by log2nt+1
    mov         r8, r7                      @&src[2nt+1]

    vshl.s64    d13, d9, #1                 @2*dc
    sub         r9, r9, #3                  @&src[2nt-1-row]

    vdup.8      d16, d9[0]                  @dc_val
    vadd.i64    d14, d13, d28               @src[2nt+1]+2+src[2nt-1]+2dc_val

    vshr.u16    d15, d14, #2                @final dst[0]'s value in d15[0]
    sub         r12, r3, r3, lsl #2         @-3*strd
    vadd.i64    d11, d13, d9                @3*dc

    vadd.i64    d11, d11, d17               @3*dc + 2
    add         r12, r12, #4                @offset after one 4x4 block (-3*strd + 4)

    vdup.16     q12, d11[0]                 @3*dc + 2 (moved to all lanes)
    sub         r0, r3, r4                  @strd - nt


    vld1.s8     d0, [r8]                    @col 1::3 load (prol)
    vld1.s8     d1, [r9]                    @row 3::1 (0 also) load (prol)

    vmovl.u8    q10, d0

    vmovl.u8    q11, d1
    vadd.i16    q10, q10, q12               @col 1::7 add 3dc+2 (prol)

    vadd.i16    q11, q11, q12               @row 1::7 add 3dc+2 (prol)

    vmov.i64    d19, #0x00000000000000ff    @
    vqshrun.s16 d2, q10, #2                 @columns shr2 movn (prol)

    vmov.i64    d20, #0x00000000000000ff    @byte mask row 1 (prol)
    vqshrun.s16 d3, q11, #2                 @rows shr2 movn (prol)


    vbsl        d19, d15, d2                @first row with dst[0]

    vrev64.8    d3, d3

    vst1.32     d19[0], [r2], r3            @store row 0 (prol)
    vshr.s64    d3, d3, #40                 @row 0 shift (prol) (first value to be ignored)

    vmov.i64    d21, #0x00000000000000ff    @byte mask row 2 (prol)

    vbsl        d20, d3, d16                @row 1  (prol)
    vshr.s64    d3, d3, #8                  @row 1 shift (prol)

    vst1.32     d20[0], [r2], r3            @store row 1 (prol)

    vbsl        d21, d3, d16                @row 2 (prol)

    vmov.i64    d20, #0x00000000000000ff    @byte mask row 3 (prol)

    vshr.s64    d3, d3, #8                  @row 2 shift (prol)
    vst1.32     d21[0], [r2], r3            @store row 2 (prol)

    vbsl        d20, d3, d16                @row 3  (prol)
    vst1.32     d20[0], [r2]                @store row 3 (prol)

epilogue_end:
end_func:
    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp







