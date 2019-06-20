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
@*  ihevc_intra_pred_luma_mode_18_34_neon.s
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

@void ihevc_intra_pred_luma_mode_18_34(uword8 *pu1_ref,
@                                      word32 src_strd,
@                                      uword8 *pu1_dst,
@                                      word32 dst_strd,
@                                      word32 nt,
@                                      word32 mode)
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
.equ    mode_offset,    44

.text
.align 4




.globl ihevc_intra_pred_luma_mode_18_34_a9q

.type ihevc_intra_pred_luma_mode_18_34_a9q, %function

ihevc_intra_pred_luma_mode_18_34_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments


    ldr         r4,[sp,#nt_offset]
    ldr         r5,[sp,#mode_offset]

    cmp         r4,#4
    beq         mode2_4

    mov         r11,r4
    mov         r12,r4
    sub         r14,r4,#8

    add         r0,r0,r4,lsl #1

    cmp         r5,#0x22
    mov         r10,r2

    add         r0,r0,#2
    subne       r0,r0,#2
    moveq       r6,#1
    movne       r6,#-1
    mov         r8,r0

prologue_cpy_32:

    vld1.8      {d0},[r8],r6
    lsr         r1, r4, #3
    vld1.8      {d1},[r8],r6
    mul         r1, r4, r1
    vld1.8      {d2},[r8],r6
    vld1.8      {d3},[r8],r6
    subs        r1,r1,#8
    vld1.8      {d4},[r8],r6
    vld1.8      {d5},[r8],r6
    vld1.8      {d6},[r8],r6

    vld1.8      {d7},[r8],r6


    beq         epilogue_mode2
    sub         r11,r11,#8

    cmp         r5,#0x22
    addne       r0,r0,#8
    movne       r8,r0
    bne         kernel_mode18
    @add        r8,r0,#8

kernel_mode2:
    vst1.8      {d0},[r10],r3
    vst1.8      {d1},[r10],r3
    subs        r12,r12,#8
    vst1.8      {d2},[r10],r3
    addne       r2,r2,#8
    vst1.8      {d3},[r10],r3

    vld1.8      {d0},[r8],r6
    vst1.8      {d4},[r10],r3

    vst1.8      {d5},[r10],r3
    vld1.8      {d1},[r8],r6
    vst1.8      {d6},[r10],r3
    vld1.8      {d2},[r8],r6
    vst1.8      {d7},[r10],r3

    vld1.8      {d3},[r8],r6
    subeq       r2,r10,r14
    vld1.8      {d4},[r8],r6
    mov         r10,r2
    vld1.8      {d5},[r8],r6
    moveq       r12,r4
    vld1.8      {d6},[r8],r6
    subs        r11,r11,#8

    vld1.8      {d7},[r8],r6

    addeq       r0,r0,#8
    moveq       r11,r4
    moveq       r8,r0

    subs        r1, r1, #8

    bne         kernel_mode2

    b           epilogue_mode2

kernel_mode18:
    vst1.8      {d0},[r10],r3
    vst1.8      {d1},[r10],r3
    subs        r12,r12,#8
    vst1.8      {d2},[r10],r3
    addne       r2,r2,#8
    vst1.8      {d3},[r10],r3

    vld1.8      {d0},[r8],r6
    vst1.8      {d4},[r10],r3

    vst1.8      {d5},[r10],r3
    vld1.8      {d1},[r8],r6

    vst1.8      {d6},[r10],r3
    vld1.8      {d2},[r8],r6
    vst1.8      {d7},[r10],r3

    vld1.8      {d3},[r8],r6
    subeq       r2,r10,r14
    vld1.8      {d4},[r8],r6
    mov         r10,r2
    vld1.8      {d5},[r8],r6
    moveq       r12,r4
    vld1.8      {d6},[r8],r6
    subs        r11,r11,#8
    vld1.8      {d7},[r8],r6

    addne       r0,r0,#8
    moveq       r11,r4
    subeq       r0,r8,r14
    subs        r1, r1, #8
    mov         r8,r0

    bne         kernel_mode18


epilogue_mode2:

    vst1.8      {d0},[r10],r3
    vst1.8      {d1},[r10],r3
    vst1.8      {d2},[r10],r3
    vst1.8      {d3},[r10],r3
    vst1.8      {d4},[r10],r3
    vst1.8      {d5},[r10],r3
    vst1.8      {d6},[r10],r3
    vst1.8      {d7},[r10],r3

    b           end_func

mode2_4:

    add         r0,r0,#10
    cmp         r5,#0x22
    subne       r0,r0,#2

    moveq       r8,#1
    movne       r8,#-1

    vld1.8      {d0},[r0],r8
    vst1.32     {d0[0]},[r2],r3

    vld1.8      {d0},[r0],r8
    vst1.32     {d0[0]},[r2],r3

    vld1.8      {d0},[r0],r8
    vst1.32     {d0[0]},[r2],r3

    vld1.8      {d0},[r0],r8
    vst1.32     {d0[0]},[r2],r3

end_func:
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp







