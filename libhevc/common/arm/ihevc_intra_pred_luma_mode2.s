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




.globl ihevc_intra_pred_luma_mode2_a9q

.type ihevc_intra_pred_luma_mode2_a9q, %function

ihevc_intra_pred_luma_mode2_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8 - d15}
    ldr         r4,[sp,#nt_offset]          @loads nt
    mov         r8,#-2

    cmp         r4,#4
    beq         mode2_4

    add         r0,r0,r4,lsl #1

    sub         r0,r0,#9                    @src[1]
    add         r10,r0,#-1

prologue_cpy_32:

    vld1.8      {d0},[r0],r8
    mov         r11,r4

    vld1.8      {d1},[r10],r8
    mov         r6, r2

    vld1.8      {d2},[r0],r8
    vld1.8      {d3},[r10],r8
    lsr         r1, r4, #3

    vld1.8      {d4},[r0],r8
    vld1.8      {d5},[r10],r8
    vld1.8      {d6},[r0],r8
    mul         r1, r4, r1

    vld1.8      {d7},[r10],r8
    add         r7,r6,r3

    vrev64.8    d8,d0
    vrev64.8    d9,d1
    lsl         r5, r3, #2

    vrev64.8    d10,d2
    vrev64.8    d11,d3
    add         r9,r7,r3

    vrev64.8    d12,d4
    subs        r1,r1,#8

    vrev64.8    d13,d5
    vrev64.8    d14,d6
    vrev64.8    d15,d7
    add         r14,r9,r3

    beq         epilogue_mode2

    sub         r12,r4,#8

kernel_mode2:

    vst1.8      {d8},[r6],r5
    vst1.8      {d9},[r7],r5
    subs        r11,r11,#8

    vst1.8      {d10},[r9],r5
    addgt       r2,r2,#8

    vst1.8      {d11},[r14],r5
    vst1.8      {d12},[r6],r5
    movle       r11,r4

    vst1.8      {d13},[r7],r5
    vst1.8      {d14},[r9],r5
    addle       r2, r2, r3, lsl #2

    vst1.8      {d15},[r14],r5
    vld1.8      {d0},[r0],r8
    sub         r14,r4,#8

    vld1.8      {d1},[r10],r8
    vld1.8      {d2},[r0],r8
    addle       r2, r2, #8

    vld1.8      {d3},[r10],r8
    vld1.8      {d4},[r0],r8
    suble       r2, r6, r14

    vld1.8      {d5},[r10],r8
    subs        r12,r12,#8

    vld1.8      {d6},[r0],r8
    mov         r6, r2

    vld1.8      {d7},[r10],r8
    addle       r0, r0, r4

    vrev64.8    d8,d0
    add         r7, r6, r3

    vrev64.8    d9,d1
    suble       r0, r0, #8

    vrev64.8    d10,d2
    movle       r12,r4

    vrev64.8    d11,d3
    add         r9, r7, r3

    vrev64.8    d12,d4
    add         r10,r0,#-1

    vrev64.8    d13,d5
    subs        r1, r1, #8

    vrev64.8    d14,d6
    add         r14, r9, r3

    vrev64.8    d15,d7

    bne         kernel_mode2

epilogue_mode2:

    vst1.8      {d8},[r6],r5
    vst1.8      {d9},[r7],r5
    vst1.8      {d10},[r9],r5
    vst1.8      {d11},[r14],r5
    vst1.8      {d12},[r6],r5
    vst1.8      {d13},[r7],r5
    vst1.8      {d14},[r9],r5
    vst1.8      {d15},[r14],r5

    b           end_func

mode2_4:

    mov         r8,#-2
    sub         r0,r0,#1
    add         r10,r0,#-1

    vld1.8      {d0},[r0],r8
    add         r5,r2,r3
    vld1.8      {d2},[r10],r8
    add         r6,r5,r3
    vld1.8      {d4},[r0]
    add         r7,r6,r3
    vld1.8      {d6},[r10]

    vrev64.8    d1,d0
    vrev64.8    d3,d2



    vst1.32     {d1[0]},[r2]
    vrev64.8    d5,d4
    vst1.32     {d3[0]},[r5]
    vrev64.8    d7,d6
    vst1.32     {d5[0]},[r6]
    vst1.32     {d7[0]},[r7]

end_func:
    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp







