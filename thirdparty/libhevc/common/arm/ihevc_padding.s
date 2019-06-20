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
@ *******************************************************************************
@ * @file
@ *  ihevc_padding_neon.s
@ *
@ * @brief
@ *  contains function definitions padding
@ *
@ * @author
@ *  naveen sr
@ *
@ * @par list of functions:
@ *  - ihevc_pad_left_luma()
@ *  - ihevc_pad_left_chroma()
@ *
@ * @remarks
@ *  none
@ *
@ *******************************************************************************
@*/

@/**
@*******************************************************************************
@*
@* @brief
@*   padding (luma block) at the left of a 2d array
@*
@* @par description:
@*   the left column of a 2d array is replicated for pad_size times at the left
@*
@*
@* @param[in] pu1_src
@*  uword8 pointer to the source
@*
@* @param[in] src_strd
@*  integer source stride
@*
@* @param[in] ht
@*  integer height of the array
@*
@* @param[in] wd
@*  integer width of the array
@*
@* @param[in] pad_size
@*  integer -padding size of the array
@*
@* @param[in] ht
@*  integer height of the array
@*
@* @param[in] wd
@*  integer width of the array
@*
@* @returns
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/
@.if pad_left_luma == c
@void ihevc_pad_left_luma(uword8 *pu1_src,
@                        word32 src_strd,
@                        word32 ht,
@                        word32 pad_size)
@**************variables vs registers*************************
@   r0 => *pu1_src
@   r1 => src_strd
@   r2 => ht
@   r3 => pad_size

.text
.align 4




.globl ihevc_pad_left_luma_a9q

.type ihevc_pad_left_luma_a9q, %function

ihevc_pad_left_luma_a9q:

    stmfd       sp!, {r4-r11,lr}            @stack stores the values of the arguments

loop_start_luma_left:
    @ pad size is assumed to be pad_left = 80
    sub         r4,r0,r3

    ldrb        r8,[r0]
    add         r0,r1
    ldrb        r9,[r0]
    add         r0,r1
    ldrb        r10,[r0]
    add         r0,r1
    ldrb        r11,[r0]
    add         r0,r1

    vdup.u8     q0,r8
    vdup.u8     q1,r9
    vdup.u8     q2,r10
    vdup.u8     q3,r11

    add         r5,r4,r1

    vst1.8      {d0,d1},[r4]!               @128/8 = 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]                @ 16 bytes store

    add         r6,r5,r1

    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]                @128/8 = 16 bytes store

    add         r7,r6,r1

    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]                @128/8 = 16 bytes store

    subs        r2,#4

    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store

    @ total of 4rows*(16*5) = 4 * 80 = 4 * pad_left store

    bne         loop_start_luma_left

    ldmfd       sp!,{r4-r11,pc}             @reload the registers from sp





@/**
@*******************************************************************************
@*
@* @brief
@*   padding (chroma block) at the left of a 2d array
@*
@* @par description:
@*   the left column of a 2d array is replicated for pad_size times at the left
@*
@*
@* @param[in] pu1_src
@*  uword8 pointer to the source
@*
@* @param[in] src_strd
@*  integer source stride
@*
@* @param[in] ht
@*  integer height of the array
@*
@* @param[in] wd
@*  integer width of the array (each colour component)
@*
@* @param[in] pad_size
@*  integer -padding size of the array
@*
@* @param[in] ht
@*  integer height of the array
@*
@* @param[in] wd
@*  integer width of the array
@*
@* @returns
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/
@.if pad_left_chroma == c
@void ihevc_pad_left_chroma(uword8 *pu1_src,
@                            word32 src_strd,
@                            word32 ht,
@                            word32 pad_size)
@{
@   r0 => *pu1_src
@   r1 => src_strd
@   r2 => ht
@   r3 => pad_size



.globl ihevc_pad_left_chroma_a9q

.type ihevc_pad_left_chroma_a9q, %function

ihevc_pad_left_chroma_a9q:

    stmfd       sp!, {r4-r11, lr}           @stack stores the values of the arguments

loop_start_chroma_left:
    @ pad size is assumed to be pad_left = 80
    sub         r4,r0,r3

    ldrh        r8,[r0]
    add         r0,r1
    ldrh        r9,[r0]
    add         r0,r1
    ldrh        r10,[r0]
    add         r0,r1
    ldrh        r11,[r0]
    add         r0,r1

    vdup.u16    q0,r8
    vdup.u16    q1,r9
    vdup.u16    q2,r10
    vdup.u16    q3,r11

    add         r5,r4,r1

    vst1.8      {d0,d1},[r4]!               @128/8 = 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]                @ 16 bytes store

    add         r6,r5,r1

    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]                @128/8 = 16 bytes store

    add         r7,r6,r1

    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]                @128/8 = 16 bytes store

    subs        r2,#4

    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store

    @ total of 4rows*(16*5) = 4 * 80 = 4 * pad_left store

    bne         loop_start_chroma_left

    ldmfd       sp!,{r4-r11,pc}             @reload the registers from sp





@/**
@*******************************************************************************
@*
@* @brief
@* padding (luma block) at the right of a 2d array
@*
@* @par description:
@* the right column of a 2d array is replicated for pad_size times at the right
@*
@*
@* @param[in] pu1_src
@*  uword8 pointer to the source
@*
@* @param[in] src_strd
@*  integer source stride
@*
@* @param[in] ht
@*  integer height of the array
@*
@* @param[in] wd
@*  integer width of the array
@*
@* @param[in] pad_size
@*  integer -padding size of the array
@*
@* @param[in] ht
@*  integer height of the array
@*
@* @param[in] wd
@*  integer width of the array
@*
@* @returns
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/
@.if pad_right_luma == c
@void ihevc_pad_right_luma(uword8 *pu1_src,
@                        word32 src_strd,
@                        word32 ht,
@                        word32 pad_size)
@{
@    word32 row@
@
@    for(row = 0@ row < ht@ row++)
@    {
@        memset(pu1_src, *(pu1_src -1), pad_size)@
@
@        pu1_src += src_strd@
@    }
@}
@
@   r0 => *pu1_src
@   r1 => src_strd
@   r2 => ht
@   r3 => pad_size



.globl ihevc_pad_right_luma_a9q

.type ihevc_pad_right_luma_a9q, %function

ihevc_pad_right_luma_a9q:

    stmfd       sp!, {r4-r11, lr}           @stack stores the values of the arguments

loop_start_luma_right:
    @ pad size is assumed to be pad_left = 80
    mov         r4,r0

    ldrb        r8,[r0, #-1]
    add         r0,r1
    ldrb        r9,[r0, #-1]
    add         r0,r1
    ldrb        r10,[r0, #-1]
    add         r0,r1
    ldrb        r11,[r0, #-1]
    add         r0,r1

    add         r5,r4,r1
    add         r6,r5,r1
    add         r7,r6,r1

    vdup.u8     q0,r8
    vdup.u8     q1,r9
    vdup.u8     q2,r10
    vdup.u8     q3,r11

    vst1.8      {d0,d1},[r4]!               @128/8 = 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]                @ 16 bytes store


    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]                @128/8 = 16 bytes store

    subs        r2,#4

    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]                @128/8 = 16 bytes store

    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]                @128/8 = 16 bytes store


    @ total of 4rows*(16*5) = 4 * 80 = 4 * pad_left store


    bne         loop_start_luma_right

    ldmfd       sp!,{r4-r11,pc}             @reload the registers from sp





@/**
@*******************************************************************************
@*
@* @brief
@@* padding (chroma block) at the right of a 2d array
@*
@* @par description:
@* the right column of a 2d array is replicated for pad_size times at the right
@*
@*
@* @param[in] pu1_src
@@*  uword8 pointer to the source
@*
@* @param[in] src_strd
@*  integer source stride
@*
@* @param[in] ht
@@*  integer height of the array
@*
@* @param[in] wd
@*  integer width of the array (each colour component)
@*
@* @param[in] pad_size
@*  integer -padding size of the array
@*
@* @param[in] ht
@@*  integer height of the array
@*
@* @param[in] wd
@*  integer width of the array
@*
@* @returns
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/
@.if pad_right_chroma == c
@void ihevc_pad_right_chroma(uword8 *pu1_src,
@                        word32 src_strd,
@                        word32 ht,
@                        word32 pad_size)
@   r0 => *pu1_src
@   r1 => src_strd
@   r2 => ht
@   r3 => pad_size



.globl ihevc_pad_right_chroma_a9q

.type ihevc_pad_right_chroma_a9q, %function

ihevc_pad_right_chroma_a9q:

    stmfd       sp!, {r4-r11, lr}           @stack stores the values of the arguments

loop_start_chroma_right:
    @ pad size is assumed to be pad_left = 80
    mov         r4,r0

    ldrh        r8,[r0, #-2]
    add         r0,r1
    ldrh        r9,[r0, #-2]
    add         r0,r1
    ldrh        r10,[r0, #-2]
    add         r0,r1
    ldrh        r11,[r0, #-2]
    add         r0,r1

    vdup.u16    q0,r8
    vdup.u16    q1,r9
    vdup.u16    q2,r10
    vdup.u16    q3,r11

    add         r5,r4,r1

    vst1.8      {d0,d1},[r4]!               @128/8 = 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]!               @ 16 bytes store
    vst1.8      {d0,d1},[r4]                @ 16 bytes store

    add         r6,r5,r1

    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]!               @128/8 = 16 bytes store
    vst1.8      {d2,d3},[r5]                @128/8 = 16 bytes store

    add         r7,r6,r1

    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]!               @128/8 = 16 bytes store
    vst1.8      {d4,d5},[r6]                @128/8 = 16 bytes store

    subs        r2,#4

    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]!               @128/8 = 16 bytes store
    vst1.8      {d6,d7},[r7]                @128/8 = 16 bytes store

    @ total of 4rows*(16*5) = 4 * 80 = 4 * pad_left store

    bne         loop_start_chroma_right

    ldmfd       sp!,{r4-r11,pc}             @reload the registers from sp








