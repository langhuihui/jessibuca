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
@*  ihevc_intra_pred_luma_mode2_neon.s
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

@void ihevc_intra_pred_luma_mode2(uword8 *pu1_ref,
@                                 word32 src_strd,
@                                 uword8 *pu1_dst,
@                                 word32 dst_strd,
@                                 word32 nt,
@                                 word32 mode)
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




.globl ihevc_intra_pred_chroma_mode2_a9q

.type ihevc_intra_pred_chroma_mode2_a9q, %function

ihevc_intra_pred_chroma_mode2_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8 - d15}

    ldr         r4,[sp,#nt_offset]          @loads nt
    mov         r8,#-4

    cmp         r4,#4
    beq         mode2_4

    add         r0,r0,r4,lsl #2

    sub         r0,r0,#0x12                 @src[1]
    add         r10,r0,#-2

prologue_cpy_32:

    vld2.8      {d0,d1},[r0],r8

    mov         r11,r4
    vrev64.8    d16,d0
    vrev64.8    d17,d1

    vld2.8      {d2,d3},[r10],r8
    mov         r6, r2

    vld2.8      {d4,d5},[r0],r8
    vld2.8      {d6,d7},[r10],r8
    lsr         r1, r4, #3

    vld2.8      {d8,d9},[r0],r8
    vld2.8      {d10,d11},[r10],r8
    vld2.8      {d12,d13},[r0],r8
    mul         r1, r4, r1

    vld2.8      {d14,d15},[r10],r8
    add         r7,r6,r3

    vrev64.8    d18,d2
    vrev64.8    d19,d3
    lsl         r5, r3, #2

    vrev64.8    d20,d4
    vrev64.8    d21,d5
    add         r9,r7,r3

    vrev64.8    d22,d6
    vrev64.8    d23,d7

    vrev64.8    d24,d8
    vrev64.8    d25,d9

    vrev64.8    d26,d10
    subs        r1,r1,#8

    vrev64.8    d27,d11

    vrev64.8    d28,d12
    vrev64.8    d29,d13

    vrev64.8    d30,d14
    add         r14,r9,r3
    vrev64.8    d31,d15

    beq         epilogue_mode2

    sub         r12,r4,#8

kernel_mode2:

    vst2.8      {d16,d17},[r6],r5
    vst2.8      {d18,d19},[r7],r5
    subs        r11,r11,#8
    vst2.8      {d20,d21},[r9],r5
    vst2.8      {d22,d23},[r14],r5
    vst2.8      {d24,d25},[r6],r5
    addgt       r2,r2,#16
    vst2.8      {d26,d27},[r7],r5
    vst2.8      {d28,d29},[r9],r5
    vst2.8      {d30,d31},[r14],r5

    vld2.8      {d0,d1},[r0],r8
    movle       r11,r4

    vld2.8      {d2,d3},[r10],r8
    vld2.8      {d4,d5},[r0],r8
    addle       r2, r2, r3, lsl #2
    vld2.8      {d6,d7},[r10],r8
    vrev64.8    d16,d0

    vld2.8      {d8,d9},[r0],r8
    vld2.8      {d10,d11},[r10],r8
    suble       r2, r6,#16
    vld2.8      {d12,d13},[r0],r8
    vrev64.8    d17,d1
    vld2.8      {d14,d15},[r10],r8

    subs        r12,r12,#8
    mov         r6, r2
    addle       r0, r0, r4,lsl #1
    add         r7, r6, r3

    vrev64.8    d18,d2
    suble       r0, r0, #16
    vrev64.8    d19,d3

    vrev64.8    d20,d4
    movle       r12,r4
    vrev64.8    d21,d5

    vrev64.8    d22,d6
    add         r9, r7, r3
    vrev64.8    d23,d7

    vrev64.8    d24,d8
    add         r10,r0,#-2
    vrev64.8    d25,d9

    vrev64.8    d26,d10
    subs        r1, r1, #8
    vrev64.8    d27,d11

    vrev64.8    d28,d12
    vrev64.8    d29,d13

    vrev64.8    d30,d14
    add         r14, r9, r3
    vrev64.8    d31,d15

    bne         kernel_mode2

epilogue_mode2:

    vst2.8      {d16,d17},[r6],r5
    vst2.8      {d18,d19},[r7],r5
    vst2.8      {d20,d21},[r9],r5
    vst2.8      {d22,d23},[r14],r5
    vst2.8      {d24,d25},[r6],r5
    vst2.8      {d26,d27},[r7],r5
    vst2.8      {d28,d29},[r9],r5
    vst2.8      {d30,d31},[r14],r5

    b           end_func

mode2_4:

    lsl         r12,r4,#1
    add         r0,r0,r12
    sub         r0,r0,#2

    vld2.8      {d12,d13},[r0],r8
    vshl.i64    d0,d12,#32
    add         r10,r0,#2
    vshl.i64    d1,d13,#32

    vrev64.8    d0,d0
    vld2.8      {d14,d15},[r10],r8
    vshl.i64    d2,d14,#32

    vrev64.8    d1,d1
    vshl.i64    d3,d15,#32
    vzip.8      d0,d1
    vst1.8      {d0},[r2],r3

    vrev64.8    d2,d2
    vld2.8      {d16,d17},[r0],r8
    vshl.i64    d4,d16,#32
    vrev64.8    d3,d3
    vshl.i64    d5,d17,#32
    vzip.8      d2,d3
    vrev64.8    d4,d4
    vrev64.8    d5,d5
    vst1.8      {d2},[r2],r3


    vld2.8      {d18,d19},[r10],r8
    vshl.i64    d6,d18,#32

    vzip.8      d4,d5
    vshl.i64    d7,d19,#32
    vrev64.8    d6,d6
    vst1.8      {d4},[r2],r3

    vrev64.8    d7,d7
    vzip.8      d6,d7
    vst1.8      {d6},[r2],r3

end_func:
    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp






