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
@*  ihevc_intra_pred_chroma_dc_neon.s
@*
@* @brief
@*  contains function definitions for intra prediction dc filtering.
@* functions are coded using neon  intrinsics and can be compiled using

@* rvct
@*
@* @author
@*  yogeswaran rs
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

@void ihevc_intra_pred_chroma_dc(uword8 *pu1_ref,
@                                word32 src_strd,
@                                uword8 *pu1_dst,
@                                word32 dst_strd,
@                                word32 nt,
@                                word32 mode)
@
@**************variables vs registers*****************************************
@r0 => *pu1_ref
@r1 => src_strd
@r2 => *pu1_dst
@r3 => dst_strd

@stack contents from #40
@   nt
@   mode
@   pi1_coeff

.equ    nt_offset,      40

.text
.align 4




.globl ihevc_intra_pred_chroma_dc_a9q

.type ihevc_intra_pred_chroma_dc_a9q, %function

ihevc_intra_pred_chroma_dc_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments

    ldr         r4,[sp,#nt_offset]          @loads nt
    mov         r9, #0
    vmov        d17, r9, r9

    clz         r5, r4                      @counts leading zeros

    add         r6, r0, r4,lsl #1           @&src[2nt]
    vmov        d18, r9, r9
    rsb         r5, r5, #32                 @log2nt
    add         r7, r0, r4, lsl #2          @&src[4nt]
    mov         r12,r5
    add         r8, r7, #2                  @&src[4nt+2]

    cmp         r4, #4
    beq         dc_4                        @nt=4 loop


add_loop:
    vld2.s8     {d30,d31}, [r6]!            @load from src[nt]
    lsl         r10,r4,#1                   @2nt

    vpaddl.u8   d2, d30
    subs        r10, #0x10

    vld2.s8     {d26,d27}, [r8]!            @load from src[2nt+1]

    vpaddl.u8   d3, d31
    vpaddl.u16  d2, d2
    vpaddl.u16  d3, d3

    vpadal.u32  d17, d2

    vpadal.u32  d18, d3

    vpaddl.u8   d2, d26
    vpaddl.u8   d3, d27

    vpaddl.u16  d2, d2
    vpaddl.u16  d3, d3

    vpadal.u32  d17, d2
    vpadal.u32  d18, d3

    beq         epil_add_loop

core_loop_add:
    vld2.s8     {d30,d31}, [r6]!            @load from src[nt]
    vpaddl.u8   d28, d30
    vpaddl.u8   d3, d31

    vld2.s8     {d26,d27}, [r8]!            @load from src[2nt+1]

    vpaddl.u16  d3, d3
    vpaddl.u16  d29, d28

    vpadal.u32  d18, d3
    vpadal.u32  d17, d29

    vpaddl.u8   d3, d27
    vpaddl.u8   d28, d26

    vpaddl.u16  d3, d3
    vpaddl.u16  d29, d28

    vpadal.u32  d18, d3
    vpadal.u32  d17, d29


epil_add_loop:

    vmov.32     r1,d18[0]
    vmov.32     r11,d17[0]

    add         r1,r1,r4
    add         r11,r11,r4

    lsr         r1,r1,r12
    lsr         r11,r11,r12

    vdup.8      d17,r1
    vdup.8      d16,r11

prologue_cpy_32:

    add         r5, r2, r3
    subs        r9, r4, #8
    lsl         r6, r3, #2
    moveq       r11,r6
    add         r8, r5, r3
    add         r10, r8, r3

    beq         epilogue_copy

    vst2.8      {d16,d17}, [r2]!
    add         r6, r6, #0xfffffff0

    vst2.8      {d16,d17}, [r5]!
    vst2.8      {d16,d17}, [r8]!
    movne       r11,#16
    vst2.8      {d16,d17}, [r10]!


    vst2.8      {d16,d17}, [r2], r6
    vst2.8      {d16,d17}, [r5], r6
    vst2.8      {d16,d17}, [r8], r6
    vst2.8      {d16,d17}, [r10], r6

kernel_copy:
    vst2.8      {d16,d17}, [r2]!
    vst2.8      {d16,d17}, [r5]!
    vst2.8      {d16,d17}, [r8]!
    vst2.8      {d16,d17}, [r10]!

    vst2.8      {d16,d17}, [r2], r6
    vst2.8      {d16,d17}, [r5], r6
    vst2.8      {d16,d17}, [r8], r6
    vst2.8      {d16,d17}, [r10], r6

    vst2.8      {d16,d17}, [r2]!
    vst2.8      {d16,d17}, [r5]!
    vst2.8      {d16,d17}, [r8]!
    vst2.8      {d16,d17}, [r10]!

    vst2.8      {d16,d17}, [r2], r6
    vst2.8      {d16,d17}, [r5], r6
    vst2.8      {d16,d17}, [r8], r6
    vst2.8      {d16,d17}, [r10], r6

epilogue_copy:
    vst2.8      {d16,d17}, [r2],r11
    vst2.8      {d16,d17}, [r5],r11
    vst2.8      {d16,d17}, [r8],r11
    vst2.8      {d16,d17}, [r10],r11

    vst2.8      {d16,d17}, [r2]
    vst2.8      {d16,d17}, [r5]
    vst2.8      {d16,d17}, [r8]
    vst2.8      {d16,d17}, [r10]
    b           end_func

dc_4:
    vld2.s8     {d30,d31},[r6]              @load from src[nt]
    vshl.i64    d3,d30,#32

    vld2.s8     {d26,d27},[r8]              @load from src[2nt+1]
    vshl.i64    d2,d31,#32

    vpaddl.u8   d3,d3
    vpaddl.u8   d2,d2
    vpaddl.u16  d3,d3
    vpaddl.u16  d2,d2
    vpadal.u32  d17,d3
    vpadal.u32  d18,d2

    vshl.i64    d3,d26,#32
    vshl.i64    d2,d27,#32
    vpaddl.u8   d3,d3
    vpaddl.u8   d2,d2
    vpaddl.u16  d3,d3
    vpaddl.u16  d2,d2
    vpadal.u32  d17,d3
    vpadal.u32  d18,d2

    vmov.32     r10,d17[0]
    vmov.32     r11,d18[0]

    add         r10,r10,r4
    add         r11,r11,r4
    lsr         r10,r10,r12
    lsr         r11,r11,r12
    orr         r10,r10,r11,lsl #8
    vdup.16     d0,r10

    vst1.8      {d0},[r2],r3
    vst1.8      {d0},[r2],r3
    vst1.8      {d0},[r2],r3
    vst1.8      {d0},[r2]

end_func:
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp




