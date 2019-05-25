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
@/*******************************************************************************
@* @file
@*  ihevcd_itrans_recon_dc_chroma.s
@*
@* @brief
@*  contains function definitions itrans and recon for dc only case
@*
@* @author
@*  ittiam
@*
@* @par list of functions:
@*
@*
@* @remarks
@*  none
@*
@*******************************************************************************/

.text


.globl ihevcd_itrans_recon_dc_chroma_a9q

.type ihevcd_itrans_recon_dc_chroma_a9q, %function

ihevcd_itrans_recon_dc_chroma_a9q:

@void ihevcd_itrans_recon_dc_chroma(uword8 *pu1_pred,
@                            uword8 *pu1_dst,
@                            word32 pred_strd,
@                            word32 dst_strd,
@                            word32 log2_trans_size,
@                            word16 i2_coeff_value)

@r0:pu1_pred
@r1:pu1_dest
@r2:pred_strd
@r3:dst_strd



    push        {r0-r11,lr}
    ldr         r4,[sp,#0x34]               @loads log2_trans_size
    ldr         r5,[sp,#0x38]               @ loads i2_coeff_value

    mov         r10,#1
    lsl         r4,r10,r4                   @    trans_size = (1 << log2_trans_size)@
    mov         r6,#64 @ 1 << (shift1 - 1)@
    mov         r7,#2048                    @ 1<<(shift2-1)

    add         r8,r6,r5,lsl #6
    ssat        r8,#16,r8,asr #7
    add         r5,r7,r8,lsl #6
    ssat        r6,#16,r5,asr #12
    mov         r9,r4
    mov         r8,r4

    @ r6 has the dc_value
    @ r4 has the trans_size value
    @ r8 has the row value
    @ r9 has the col value
    vdup.s16    q0,r6
    cmp         r4,#4
    beq         row_loop_4chroma


row_loop_chroma:
    mov         r9,r4


col_loop_chroma:

    mov         r7,r0
    vld2.8      {d2,d3},[r7],r2
    vld2.8      {d4,d5},[r7],r2
    vld2.8      {d6,d7},[r7],r2
    vld2.8      {d8,d9},[r7],r2

    vld2.8      {d10,d11},[r7],r2
    vld2.8      {d12,d13},[r7],r2
    vld2.8      {d14,d15},[r7],r2
    vld2.8      {d16,d17},[r7]

    add         r0,r0,#16


    vaddw.u8    q15,q0,d2
    vaddw.u8    q14,q0,d4
    vaddw.u8    q13,q0,d6
    vaddw.u8    q12,q0,d8
    vaddw.u8    q11,q0,d10
    vaddw.u8    q10,q0,d12
    vaddw.u8    q9,q0,d14


    mov         r11,r1
    vqmovun.s16 d2,q15
    vqmovun.s16 d4,q14
    vqmovun.s16 d6,q13
    vqmovun.s16 d8,q12

    vaddw.u8    q15,q0,d16

    vqmovun.s16 d10,q11
    vqmovun.s16 d12,q10
    vqmovun.s16 d14,q9
    vqmovun.s16 d16,q15

    vst2.8      {d2,d3},[r11],r3
    vst2.8      {d4,d5},[r11],r3
    vst2.8      {d6,d7},[r11],r3
    vst2.8      {d8,d9},[r11],r3

    vst2.8      {d10,d11},[r11],r3
    vst2.8      {d12,d13},[r11],r3
    vst2.8      {d14,d15},[r11],r3
    vst2.8      {d16,d17},[r11]

    add         r1,r1,#16

    subs        r9,r9,#8
    bgt         col_loop_chroma

    subs        r8,r8,#8

    add         r0,r0,r2,lsl #3
    add         r1,r1,r3,lsl #3
    sub         r0,r0,r4,lsl #1
    sub         r1,r1,r4,lsl #1
    bgt         row_loop_chroma
    b           end_loops_chroma


row_loop_4chroma:
    mov         r9,r10


col_loop_4chroma:


    vld2.8      {d2,d3},[r0],r2
    vld2.8      {d4,d5},[r0],r2
    vld2.8      {d6,d7},[r0],r2
    vld2.8      {d8,d9},[r0]




    vaddw.u8    q15,q0,d2
    vaddw.u8    q14,q0,d4
    vaddw.u8    q13,q0,d6
    vaddw.u8    q12,q0,d8



    vqmovun.s16 d2,q15
    vqmovun.s16 d4,q14
    vqmovun.s16 d6,q13
    vqmovun.s16 d8,q12


    vzip.8      d2,d3
    vzip.8      d4,d5
    vzip.8      d6,d7
    vzip.8      d8,d9

    vst1.u32    {d2},[r1],r3
    vst1.u32    {d4},[r1],r3
    vst1.u32    {d6},[r1],r3
    vst1.u32    {d8},[r1]

end_loops_chroma:
    pop         {r0-r11,pc}


