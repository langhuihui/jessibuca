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
//*******************************************************************************
//* @file
//*  ihevc_intra_pred_chroma_ver_neon.s
//*
//* @brief
//*  contains function definitions for intra prediction dc filtering.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* @author
//*  yogeswaran rs
//*
//* @par list of functions:
//*
//*
//* @remarks
//*  none
//*
//*******************************************************************************
//*/
///**
//*******************************************************************************
//*
//* @brief
//*    luma intraprediction filter for dc input
//*
//* @par description:
//*
//* @param[in] pu1_ref
//*  uword8 pointer to the source
//*
//* @param[out] pu1_dst
//*  uword8 pointer to the destination
//*
//* @param[in] src_strd
//*  integer source stride
//*
//* @param[in] dst_strd
//*  integer destination stride
//*
//* @param[in] nt
//*  size of tranform block
//*
//* @param[in] mode
//*  type of filtering
//*
//* @returns
//*
//* @remarks
//*  none
//*
//*******************************************************************************
//*/

//void ihevc_intra_pred_chroma_ver(uword8 *pu1_ref,
//        word32 src_strd,
//        uword8 *pu1_dst,
//        word32 dst_strd,
//        word32 nt,
//        word32 mode)
//**************variables vs registers*****************************************
//x0 => *pu1_ref
//x1 => src_strd
//x2 => *pu1_dst
//x3 => dst_strd

//stack contents from #40
//    nt
//    mode

.text
.align 4
.include "ihevc_neon_macros.s"


.globl ihevc_intra_pred_chroma_ver_av8

.type ihevc_intra_pred_chroma_ver_av8, %function

ihevc_intra_pred_chroma_ver_av8:

    // stmfd sp!, {x4-x12, x14}            //stack stores the values of the arguments
    push_v_regs
    stp         x19, x20,[sp,#-16]!

    lsl         x5, x4, #2                  //4nt


    cmp         x4, #8
    beq         blk_8
    blt         blk_4

copy_16:
    add         x5, x5, #2                  //2nt+2
    add         x6, x0, x5                  //&src[2nt+1]

    add         x5, x2, x3                  //pu1_dst + dst_strd
    ld2         {v20.8b, v21.8b}, [x6],#16  //16 loads (col 0:15)
    add         x8, x5, x3

    add         x10, x8, x3
    ld2         {v22.8b, v23.8b}, [x6]      //16 loads (col 16:31)
    lsl         x11, x3, #2

    sub         x11, x11, #16


    st2         {v20.8b, v21.8b}, [x2],#16
    st2         {v20.8b, v21.8b}, [x5],#16
    st2         {v20.8b, v21.8b}, [x8],#16
    st2         {v20.8b, v21.8b}, [x10],#16

    st2         {v22.8b, v23.8b}, [x2], x11
    st2         {v22.8b, v23.8b}, [x5], x11
    st2         {v22.8b, v23.8b}, [x8], x11
    st2         {v22.8b, v23.8b}, [x10], x11

    subs        x4, x4, #4

kernel_copy_16:
    st2         {v20.8b, v21.8b}, [x2],#16
    st2         {v20.8b, v21.8b}, [x5],#16
    st2         {v20.8b, v21.8b}, [x8],#16
    st2         {v20.8b, v21.8b}, [x10],#16

    st2         {v22.8b, v23.8b}, [x2], x11
    st2         {v22.8b, v23.8b}, [x5], x11
    st2         {v22.8b, v23.8b}, [x8], x11
    st2         {v22.8b, v23.8b}, [x10], x11

    subs        x4, x4, #4


    st2         {v20.8b, v21.8b}, [x2],#16
    st2         {v20.8b, v21.8b}, [x5],#16
    st2         {v20.8b, v21.8b}, [x8],#16
    st2         {v20.8b, v21.8b}, [x10],#16

    st2         {v22.8b, v23.8b}, [x2], x11
    st2         {v22.8b, v23.8b}, [x5], x11
    st2         {v22.8b, v23.8b}, [x8], x11
    st2         {v22.8b, v23.8b}, [x10], x11

    subs        x4, x4, #4

    st2         {v20.8b, v21.8b}, [x2],#16
    st2         {v20.8b, v21.8b}, [x5],#16
    st2         {v20.8b, v21.8b}, [x8],#16
    st2         {v20.8b, v21.8b}, [x10],#16

    st2         {v22.8b, v23.8b}, [x2], x11
    st2         {v22.8b, v23.8b}, [x5], x11
    st2         {v22.8b, v23.8b}, [x8], x11
    st2         {v22.8b, v23.8b}, [x10], x11

    subs        x4, x4, #4
    bne         kernel_copy_16

    b           end_func

blk_8:

    add         x5, x5, #2                  //2nt+2
    add         x6, x0, x5                  //&src[2nt+1]

    add         x5, x2, x3                  //pu1_dst + dst_strd
    ld2         {v20.8b, v21.8b}, [x6],#16  //16 loads (col 0:15)
    add         x8, x5, x3

    add         x10, x8, x3
    ld2         {v22.8b, v23.8b}, [x6]      //16 loads (col 16:31)

    lsl         x11,x3,#2

    st2         {v20.8b, v21.8b}, [x2],x11
    st2         {v20.8b, v21.8b}, [x5],x11
    st2         {v20.8b, v21.8b}, [x8],x11
    st2         {v20.8b, v21.8b}, [x10],x11

    st2         {v20.8b, v21.8b}, [x2]
    st2         {v20.8b, v21.8b}, [x5]
    st2         {v20.8b, v21.8b}, [x8]
    st2         {v20.8b, v21.8b}, [x10]

    subs        x4, x4, #8
    beq         end_func

blk_4:

    //lsl        x5, x4, #2            @4nt
    add         x5, x5, #2                  //2nt+2
    add         x6, x0, x5                  //&src[2nt+1]

    ld1         {v0.8b},[x6]
    add         x5, x2, x3                  //pu1_dst + dst_strd

    st1         {v0.8b},[x2]
    add         x8, x5, x3
    st1         {v0.8b},[x5]
    add         x10, x8, x3
    st1         {v0.8b},[x8]
    st1         {v0.8b},[x10]



end_func:
    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16
    pop_v_regs
    ret



