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
@*  ihevc_intra_pred_filters_vert.s
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

@void ihevc_intra_pred_luma_ver(uword8* pu1_ref,
@                               word32 src_strd,
@                               uword8* pu1_dst,
@                               word32 dst_strd,
@                               word32 nt,
@                               word32 mode)
@
@**************variables vs registers*****************************************
@r0 => *pu1_ref
@r1 => src_strd
@r2 => *pu1_dst
@r3 => dst_strd

@stack contents from #104
@   nt
@   mode

.equ    nt_offset,      104

.text
.align 4




.globl ihevc_intra_pred_luma_ver_a9q

.type ihevc_intra_pred_luma_ver_a9q, %function

ihevc_intra_pred_luma_ver_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8 - d15}
    ldr         r4,[sp,#nt_offset]          @loads nt

    lsl         r5, r4, #1                  @2nt

    cmp         r4, #16
    beq         blk_16
    blt         blk_4_8

    add         r5, r5, #1                  @2nt+1
    add         r6, r0, r5                  @&src[2nt+1]

copy_32:
    add         r5, r2, r3
    vld1.8      {d20,d21}, [r6]!            @16 loads (col 0:15)
    add         r8, r5, r3

    add         r10, r8, r3
    vld1.8      {d22,d23}, [r6]             @16 loads (col 16:31)
    lsl         r11, r3, #2

    add         r11, r11, #0xfffffff0
    vst1.8      {d20,d21}, [r2]!
    vst1.8      {d20,d21}, [r5]!
    vst1.8      {d20,d21}, [r8]!
    vst1.8      {d20,d21}, [r10]!

    vst1.8      {d22,d23}, [r2], r11
    vst1.8      {d22,d23}, [r5], r11
    vst1.8      {d22,d23}, [r8], r11
    vst1.8      {d22,d23}, [r10], r11

    subs        r4, r4, #8

kernel_copy_32:
    vst1.8      {d20,d21}, [r2]!
    vst1.8      {d20,d21}, [r5]!
    vst1.8      {d20,d21}, [r8]!
    vst1.8      {d20,d21}, [r10]!

    vst1.8      {d22,d23}, [r2], r11
    vst1.8      {d22,d23}, [r5], r11
    vst1.8      {d22,d23}, [r8], r11
    vst1.8      {d22,d23}, [r10], r11

    subs        r4, r4, #8

    vst1.8      {d20,d21}, [r2]!
    vst1.8      {d20,d21}, [r5]!
    vst1.8      {d20,d21}, [r8]!
    vst1.8      {d20,d21}, [r10]!

    vst1.8      {d22,d23}, [r2], r11
    vst1.8      {d22,d23}, [r5], r11
    vst1.8      {d22,d23}, [r8], r11
    vst1.8      {d22,d23}, [r10], r11

    bne         kernel_copy_32

    vst1.8      {d20,d21}, [r2]!
    vst1.8      {d20,d21}, [r5]!
    vst1.8      {d20,d21}, [r8]!
    vst1.8      {d20,d21}, [r10]!

    vst1.8      {d22,d23}, [r2], r11
    vst1.8      {d22,d23}, [r5], r11
    vst1.8      {d22,d23}, [r8], r11
    vst1.8      {d22,d23}, [r10], r11

    b           end_func

blk_16:
    add         r6, r0, r5                  @&src[2nt]

    ldrb        r11, [r6], #1               @src[2nt]

    vdup.8      q11, r11                    @src[2nt]
    ldrb        r12, [r6]                   @src[2nt+1]

    vld1.8      {d16,d17}, [r6]             @ld for repl to cols src[2nt+1+col(0:15)] (0 ignored for stores)
    add         r6, r6, #0xffffffef         @subtract -9 to take it to src[2nt-1-row(15)]

    vdup.8      q12, r12                    @src[2nt+1]
    vdup.16     q15, r12
    lsl         r5, r3, #3                  @8*stride

    vld1.8      {d26,d27}, [r6]!            @load src[2nt-1-row](rows 0:15)
    add         r5, r2, r5                  @r5 ->

    vmov.i64    d18, #0x00000000000000ff
    vhsub.u8    q13, q13, q11               @(src[2nt-1-row] - src[2nt])>>1
    @vsubl.u8   q0, d26, d22
    @vsubl.u8   q14, d27, d22

    @vshr.s16   q0, q0, #1
    @vshr.s16   q14, q14, #1

    vmov.i64    d19, d17
    @vaddl.s8   q0, d24, d26
    vmovl.s8    q0, d26
    vmovl.s8    q14, d27
    vqadd.s16   q0, q0, q15
    vqadd.s16   q14, q14, q15

    vmov.i64    d10, #0x00000000000000ff
    @vaddl.s8   q1, d25, d27

    vqmovun.s16 d25, q0
    vqmovun.s16 d24, q14
    @vmovn.u16  d25, q0
    @vmovn.u16  d24, q1


    vrev64.8    q12, q12

    vmov.i64    d11, d17

    vbsl        d18, d24, d16               @only select row values from q12(predpixel)
    vbsl        d10, d25, d16

    vmov.i64    d8, #0x00000000000000ff
    vmov.i64    d9, d17

    vmov.i64    d6, #0x00000000000000ff
    vmov.i64    d7, d17

    vst1.8      {d18,d19}, [r2], r3
    vshr.s64    d24, d24, #8

    vst1.8      {d10,d11}, [r5], r3
    vshr.s64    d25, d25, #8


    vbsl        d8, d24, d16
    vbsl        d6, d25, d16

    vst1.8      {d8,d9}, [r2], r3
    vshr.s64    d24, d24, #8

    vst1.8      {d6,d7}, [r5], r3
    vshr.s64    d25, d25, #8

    subs        r4, #8

    vmov.i64    d18, #0x00000000000000ff
    @vmov.i64   d19, d17

    vmov.i64    d10, #0x00000000000000ff
    @vmov.i64   d11, d17


loop_16:


    vmov.i64    d8, #0x00000000000000ff

    vmov.i64    d6, #0x00000000000000ff

    vbsl        d18, d24, d16               @only select row values from q12(predpixel)
    vbsl        d10, d25, d16

    vst1.8      {d18,d19}, [r2], r3
    vshr.s64    d24, d24, #8

    vst1.8      {d10,d11}, [r5], r3
    vshr.s64    d25, d25, #8

    vmov.i64    d18, #0x00000000000000ff

    vmov.i64    d10, #0x00000000000000ff

    vbsl        d8, d24, d16
    vbsl        d6, d25, d16

    vst1.8      {d8,d9}, [r2], r3
    vshr.s64    d24, d24, #8

    vst1.8      {d6,d7}, [r5], r3
    vshr.s64    d25, d25, #8

    subs        r4, r4, #4

    bne         loop_16

    vmov.i64    d8, #0x00000000000000ff

    vmov.i64    d6, #0x00000000000000ff

    vbsl        d18, d24, d16               @only select row values from q12(predpixel)
    vbsl        d10, d25, d16

    vst1.8      {d18,d19}, [r2], r3
    vshr.s64    d24, d24, #8

    vst1.8      {d10,d11}, [r5], r3
    vshr.s64    d25, d25, #8

    vbsl        d8, d24, d16
    vbsl        d6, d25, d16

    vst1.8      {d8,d9}, [r2], r3

    vst1.8      {d6,d7}, [r5], r3

    b           end_func


blk_4_8:
    vmov.i64    d11, #0x00000000000000ff
    add         r6, r0, r5                  @&src[2nt]

    vmov.i64    d10, #0x00000000000000ff
    ldrb        r11, [r6], #1               @src[2nt]

    vdup.8      d22, r11                    @src[2nt]
    ldrb        r12, [r6]                   @src[2nt+1]

    vld1.8      d16, [r6]                   @ld for repl to cols src[2nt+1+col(0:3 or 0:7)](0 ignored for st)
    add         r6, r6, #0xfffffff7         @subtract -9 to take it to src[2nt-1-row(15)]

    vdup.8      d24, r12                    @src[2nt+1]
    vdup.16     q15, r12

    vld1.8      d26, [r6]!                  @load src[2nt-1-row](rows 0:15)

    vmov.i64    d18, #0x00000000000000ff
    vhsub.u8    d26, d26, d22               @(src[2nt-1-row] - src[2nt])>>1
    @vsubl.u8   q13, d26, d22

    @vshr.s16   q13, q13, #1

    vmov.i64    d19, #0x00000000000000ff
    vmovl.s8    q13, d26
    @vaddl.s8   q0, d24, d26
    vqadd.s16   q0, q13, q15

    vqmovun.s16 d24, q0
    @vmovn.s16  d24, q0

    vrev64.8    d24, d24

    cmp         r4, #4
    beq         blk_4

    vbsl        d18, d24, d16               @only select row values from q12(predpixel)

    vst1.8      d18, [r2], r3
    vshr.s64    d24, d24, #8

    vmov.i64    d18, #0x00000000000000ff

    vbsl        d19, d24, d16

    vst1.8      d19, [r2], r3
    vshr.s64    d24, d24, #8

    vmov.i64    d19, #0x00000000000000ff

    vbsl        d10, d24, d16

    vst1.8      d10, [r2], r3
    vshr.s64    d24, d24, #8

    vmov.i64    d10, #0x00000000000000ff

    vbsl        d11, d24, d16

    vst1.8      d11, [r2], r3
    vshr.s64    d24, d24, #8

    vmov.i64    d11, #0x00000000000000ff

    vbsl        d18, d24, d16               @only select row values from q12(predpixel)

    vst1.8      d18, [r2], r3
    vshr.s64    d24, d24, #8

    vbsl        d19, d24, d16

    vst1.8      d19, [r2], r3
    vshr.s64    d24, d24, #8

    vbsl        d10, d24, d16

    vst1.8      d10, [r2], r3
    vshr.s64    d24, d24, #8

    vbsl        d11, d24, d16

    vst1.8      d11, [r2], r3
    vshr.s64    d24, d24, #8

    b           end_func


blk_4:
    vbsl        d18, d24, d16               @only select row values from q12(predpixel)

    vst1.32     d18[0], [r2], r3
    vshr.s64    d24, d24, #8

    vbsl        d19, d24, d16

    vst1.32     d19[0], [r2], r3
    vshr.s64    d24, d24, #8

    vbsl        d10, d24, d16

    vst1.32     d10[0], [r2], r3
    vshr.s64    d24, d24, #8

    vbsl        d11, d24, d16
    vst1.32     d11[0], [r2], r3


end_func:
    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp

