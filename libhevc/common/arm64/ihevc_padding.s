///*****************************************************************************
//*
//* Copyright (C) 2012 Ittiam Systems Pvt Ltd, Bangalore
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at:
//*
//* http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.
//*
//*****************************************************************************/
///**
// *******************************************************************************
// * //file
// *  ihevc_padding_neon.s
// *
// * //brief
// *  contains function definitions padding
// *
// * //author
// *     naveen sr
// *
// * //par list of functions:
// *  - ihevc_pad_left_luma()
// *  - ihevc_pad_left_chroma()
// *
// * //remarks
// *  none
// *
// *******************************************************************************
//*/

///**
//*******************************************************************************
//*
//* //brief
//*   padding (luma block) at the left of a 2d array
//*
//* //par description:
//*   the left column of a 2d array is replicated for pad_size times at the left
//*
//*
//* //param[in] pu1_src
//*  uword8 pointer to the source
//*
//* //param[in] src_strd
//*  integer source stride
//*
//* //param[in] ht
//*  integer height of the array
//*
//* //param[in] wd
//*  integer width of the array
//*
//* //param[in] pad_size
//*  integer -padding size of the array
//*
//* //param[in] ht
//*  integer height of the array
//*
//* //param[in] wd
//*  integer width of the array
//*
//* //returns
//*
//* //remarks
//*  none
//*
//*******************************************************************************
//*/
//.if pad_left_luma == c
//void ihevc_pad_left_luma(uword8 *pu1_src,
//                        word32 src_strd,
//                        word32 ht,
//                        word32 pad_size)
//**************variables vs registers*************************
//    x0 => *pu1_src
//    x1 => src_strd
//    x2 => ht
//    x3 => pad_size

.text
.align 4

.globl ihevc_pad_left_luma_av8

.type ihevc_pad_left_luma_av8, %function

ihevc_pad_left_luma_av8:

loop_start_luma_left:
    // pad size is assumed to be pad_left = 80
    sub         x4,x0,x3

    ldrb        w8,[x0]
    add         x0,x0,x1
    ldrb        w9,[x0]
    add         x0,x0,x1
    ldrb        w10,[x0]
    add         x0,x0,x1
    ldrb        w11,[x0]
    add         x0,x0,x1

    dup         v0.16b,w8
    dup         v2.16b,w9
    dup         v4.16b,w10
    dup         v6.16b,w11

    add         x5,x4,x1

    st1         {v0.16b},[x4],#16           //128/8 = 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4]               // 16 bytes store

    add         x6,x5,x1

    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5]               //128/8 = 16 bytes store

    add         x7,x6,x1

    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6]               //128/8 = 16 bytes store

    subs        x2, x2,#4

    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7]               //128/8 = 16 bytes store

    // total of 4rows*(16*5) = 4 * 80 = 4 * pad_left store

    bne         loop_start_luma_left

    ret





///**
//*******************************************************************************
//*
//* //brief
//*   padding (chroma block) at the left of a 2d array
//*
//* //par description:
//*   the left column of a 2d array is replicated for pad_size times at the left
//*
//*
//* //param[in] pu1_src
//*  uword8 pointer to the source
//*
//* //param[in] src_strd
//*  integer source stride
//*
//* //param[in] ht
//*  integer height of the array
//*
//* //param[in] wd
//*  integer width of the array (each colour component)
//*
//* //param[in] pad_size
//*  integer -padding size of the array
//*
//* //param[in] ht
//*  integer height of the array
//*
//* //param[in] wd
//*  integer width of the array
//*
//* //returns
//*
//* //remarks
//*  none
//*
//*******************************************************************************
//*/
//.if pad_left_chroma == c
//void ihevc_pad_left_chroma(uword8 *pu1_src,
//                            word32 src_strd,
//                            word32 ht,
//                            word32 pad_size)
//{
//    x0 => *pu1_src
//    x1 => src_strd
//    x2 => ht
//    x3 => pad_size



.globl ihevc_pad_left_chroma_av8

.type ihevc_pad_left_chroma_av8, %function

ihevc_pad_left_chroma_av8:


loop_start_chroma_left:
    // pad size is assumed to be pad_left = 80
    sub         x4,x0,x3

    ldrh        w8,[x0]
    add         x0,x0,x1
    ldrh        w9,[x0]
    add         x0,x0,x1
    ldrh        w10,[x0]
    add         x0,x0,x1
    ldrh        w11,[x0]
    add         x0,x0,x1

    dup         v0.8h,w8
    dup         v2.8h,w9
    dup         v4.8h,w10
    dup         v6.8h,w11

    add         x5,x4,x1

    st1         {v0.16b},[x4],#16           //128/8 = 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4]               // 16 bytes store

    add         x6,x5,x1

    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5]               //128/8 = 16 bytes store

    add         x7,x6,x1

    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6]               //128/8 = 16 bytes store

    subs        x2, x2,#4

    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7]               //128/8 = 16 bytes store

    // total of 4rows*(16*5) = 4 * 80 = 4 * pad_left store

    bne         loop_start_chroma_left

    ret





///**
//*******************************************************************************
//*
//* //brief
//* padding (luma block) at the right of a 2d array
//*
//* //par description:
//* the right column of a 2d array is replicated for pad_size times at the right
//*
//*
//* //param[in] pu1_src
//*  uword8 pointer to the source
//*
//* //param[in] src_strd
//*  integer source stride
//*
//* //param[in] ht
//*  integer height of the array
//*
//* //param[in] wd
//*  integer width of the array
//*
//* //param[in] pad_size
//*  integer -padding size of the array
//*
//* //param[in] ht
//*  integer height of the array
//*
//* //param[in] wd
//*  integer width of the array
//*
//* //returns
//*
//* //remarks
//*  none
//*
//*******************************************************************************
//*/
//.if pad_right_luma == c
//void ihevc_pad_right_luma(uword8 *pu1_src,
//                        word32 src_strd,
//                        word32 ht,
//                        word32 pad_size)
//{
//    word32 row//
//
//    for(row = 0// row < ht// row++)
//    {
//        memset(pu1_src, *(pu1_src -1), pad_size)//
//
//        pu1_src += src_strd//
//    }
//}
//
//    x0 => *pu1_src
//    x1 => src_strd
//    x2 => ht
//    x3 => pad_size



.globl ihevc_pad_right_luma_av8

.type ihevc_pad_right_luma_av8, %function

ihevc_pad_right_luma_av8:


loop_start_luma_right:
    // pad size is assumed to be pad_left = 80
    mov         x4,x0

    ldrb        w8,[x0, #-1]
    add         x0,x0,x1
    ldrb        w9,[x0, #-1]
    add         x0,x0,x1
    ldrb        w10,[x0, #-1]
    add         x0,x0,x1
    ldrb        w11,[x0, #-1]
    add         x0,x0,x1

    add         x5,x4,x1
    add         x6,x5,x1
    add         x7,x6,x1

    dup         v0.16b,w8
    dup         v2.16b,w9
    dup         v4.16b,w10
    dup         v6.16b,w11

    st1         {v0.16b},[x4],#16           //128/8 = 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4]               // 16 bytes store


    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5]               //128/8 = 16 bytes store

    subs        x2, x2,#4

    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6]               //128/8 = 16 bytes store

    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7]               //128/8 = 16 bytes store


    // total of 4rows*(16*5) = 4 * 80 = 4 * pad_left store


    bne         loop_start_luma_right

    ret





///**
//*******************************************************************************
//*
//* //brief
////* padding (chroma block) at the right of a 2d array
//*
//* //par description:
//* the right column of a 2d array is replicated for pad_size times at the right
//*
//*
//* //param[in] pu1_src
////*  uword8 pointer to the source
//*
//* //param[in] src_strd
//*  integer source stride
//*
//* //param[in] ht
////*  integer height of the array
//*
//* //param[in] wd
//*  integer width of the array (each colour component)
//*
//* //param[in] pad_size
//*  integer -padding size of the array
//*
//* //param[in] ht
////*  integer height of the array
//*
//* //param[in] wd
//*  integer width of the array
//*
//* //returns
//*
//* //remarks
//*  none
//*
//*******************************************************************************
//*/
//.if pad_right_chroma == c
//void ihevc_pad_right_chroma(uword8 *pu1_src,
//                        word32 src_strd,
//                        word32 ht,
//                        word32 pad_size)
//    x0 => *pu1_src
//    x1 => src_strd
//    x2 => ht
//    x3 => pad_size



.globl ihevc_pad_right_chroma_av8

.type ihevc_pad_right_chroma_av8, %function

ihevc_pad_right_chroma_av8:


loop_start_chroma_right:
    // pad size is assumed to be pad_left = 80
    mov         x4,x0

    ldrh        w8,[x0, #-2]
    add         x0,x0,x1
    ldrh        w9,[x0, #-2]
    add         x0,x0,x1
    ldrh        w10,[x0, #-2]
    add         x0,x0,x1
    ldrh        w11,[x0, #-2]
    add         x0,x0,x1

    dup         v0.8h,w8
    dup         v2.8h,w9
    dup         v4.8h,w10
    dup         v6.8h,w11

    add         x5,x4,x1

    st1         {v0.16b},[x4],#16           //128/8 = 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4],#16           // 16 bytes store
    st1         {v0.16b},[x4]               // 16 bytes store

    add         x6,x5,x1

    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5],#16           //128/8 = 16 bytes store
    st1         {v2.16b},[x5]               //128/8 = 16 bytes store

    add         x7,x6,x1

    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6],#16           //128/8 = 16 bytes store
    st1         {v4.16b},[x6]               //128/8 = 16 bytes store

    subs        x2, x2,#4

    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7],#16           //128/8 = 16 bytes store
    st1         {v6.16b},[x7]               //128/8 = 16 bytes store

    // total of 4rows*(16*5) = 4 * 80 = 4 * pad_left store

    bne         loop_start_chroma_right

    ret








