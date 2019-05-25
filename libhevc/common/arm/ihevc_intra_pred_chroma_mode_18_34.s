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

@void ihevc_intra_pred_chroma_mode_18_34(uword8 *pu1_ref,
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

@stack contents from #104
@   nt
@   mode
@   pi1_coeff

.equ    nt_offset,          104
.equ    mode_offset,        108

.text
.align 4




.globl ihevc_intra_pred_chroma_mode_18_34_a9q

.type ihevc_intra_pred_chroma_mode_18_34_a9q, %function

ihevc_intra_pred_chroma_mode_18_34_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments
    vpush       {d8 - d15}

    ldr         r4,[sp,#nt_offset]
    ldr         r5,[sp,#mode_offset]

    cmp         r4,#4
    beq         mode2_4

    mov         r12,r4
    mov         r11,r4
    add         r0,r0,r4,lsl #2

    cmp         r5,#0x22
    mov         r10,r2

    add         r0,r0,#4

    subne       r0,r0,#4
    moveq       r6,#2
    movne       r6,#-2
    mov         r8,r0


kernel:


    vld1.8      {d0,d1},[r8],r6
    vst1.8      {d0,d1},[r10],r3
    vld1.8      {d2,d3},[r8],r6
    vst1.8      {d2,d3},[r10],r3
    vld1.8      {d4,d5},[r8],r6
    vst1.8      {d4,d5},[r10],r3
    vld1.8      {d6,d7},[r8],r6
    vst1.8      {d6,d7},[r10],r3
    vld1.8      {d8,d9},[r8],r6
    vst1.8      {d8,d9},[r10],r3
    vld1.8      {d10,d11},[r8],r6
    vst1.8      {d10,d11},[r10],r3
    vld1.8      {d12,d13},[r8],r6
    vst1.8      {d12,d13},[r10],r3
    vld1.8      {d14,d15},[r8],r6
    vst1.8      {d14,d15},[r10],r3

    subs        r12,r12,#8
    bne         kernel

    cmp         r11,#16
    add         r8,r0,#16
    add         r10,r2,#16
    sub         r11,#16
    mov         r12,#16
    beq         kernel
    b           end_func

mode2_4:

    add         r0,r0,#20
    cmp         r5,#0x22
    subne       r0,r0,#4

    moveq       r8,#2
    movne       r8,#-2

    vld1.8      {d0},[r0],r8
    vst1.32     {d0},[r2],r3

    vld1.8      {d0},[r0],r8
    vst1.32     {d0},[r2],r3

    vld1.8      {d0},[r0],r8
    vst1.32     {d0},[r2],r3

    vld1.8      {d0},[r0],r8
    vst1.32     {d0},[r2],r3

end_func:
    vpop        {d8 - d15}
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp






