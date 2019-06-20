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
//*  ihevc_intra_pred_filters_planar.s
//*
//* @brief
//*  contains function definitions for inter prediction  interpolation.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* @author
//*  akshaya mukund
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
//*    luma intraprediction filter for planar input
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
//* @param[in] pi1_coeff
//*  word8 pointer to the planar coefficients
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

//void ihevc_intra_pred_luma_planar(uword8* pu1_ref,
//                                  word32 src_strd,
//                                  uword8* pu1_dst,
//                                  word32 dst_strd,
//                                  word32 nt,
//                                  word32 mode,
//                   word32 pi1_coeff)
//**************variables vs registers*****************************************
//x0 => *pu1_ref
//x1 => src_strd
//x2 => *pu1_dst
//x3 => dst_strd

//stack contents from #40
//    nt
//    mode
//    pi1_coeff

.text
.align 4
.include "ihevc_neon_macros.s"


.globl ihevc_intra_pred_chroma_planar_av8
.extern gau1_ihevc_planar_factor


.type ihevc_intra_pred_chroma_planar_av8, %function

ihevc_intra_pred_chroma_planar_av8:

    // stmfd sp!, {x4-x12, x14}            //stack stores the values of the arguments

    stp         d10,d11,[sp,#-16]!
    stp         d12,d13,[sp,#-16]!
    stp         d8,d14,[sp,#-16]!           // Storing d14 using { sub sp,sp,#8; str d14,[sp] } is giving bus error.
                                            // d8 is used as dummy register and stored along with d14 using stp. d8 is not used in the function.
    stp         x19, x20,[sp,#-16]!

    adrp        x11, :got:gau1_ihevc_planar_factor //loads table of coeffs
    ldr         x11, [x11, #:got_lo12:gau1_ihevc_planar_factor]

    clz         w5,w4
    sub         x20, x5, #32
    neg         x5, x20
    dup         v14.8h,w5
    neg         v14.8h, v14.8h              //shr value (so vneg)
    dup         v2.8b,w4                    //nt
    dup         v16.8h,w4                   //nt

    sub         x6, x4, #1                  //nt-1
    add         x6, x0,x6,lsl #1            //2*(nt-1)
    ldr         w7,  [x6]
    sxtw        x7,w7
    dup         v0.4h,w7                    //src[nt-1]

    add         x6, x4, x4,lsl #1           //3nt
    add         x6, x6, #1                  //3nt + 1
    lsl         x6,x6,#1                    //2*(3nt + 1)

    add         x6, x6, x0
    ldr         w7,  [x6]
    sxtw        x7,w7
    dup         v1.4h,w7                    //src[3nt+1]


    add         x6, x4, x4                  //2nt
    add         x14, x6, #1                 //2nt+1
    lsl         x14,x14,#1                  //2*(2nt+1)
    sub         x6, x6, #1                  //2nt-1
    lsl         x6,x6,#1                    //2*(2nt-1)
    add         x6, x6, x0                  //&src[2nt-1]
    add         x14, x14, x0                //&src[2nt+1]

    mov         x8, #1                      //row+1 (row is first 0)
    sub         x9, x4, x8                  //nt-1-row (row is first 0)

    dup         v5.8b,w8                    //row + 1
    dup         v6.8b,w9                    //nt - 1 - row
    mov         v7.8b, v5.8b                //mov #1 to d7 to used for inc for row+1 and dec for nt-1-row

    add         x12, x11, #1                //coeffs (to be reloaded after every row)
    mov         x1, x4                      //nt (row counter) (dec after every row)
    mov         x5, x2                      //dst (to be reloaded after every row and inc by dst_strd)
    mov         x10, #8                     //increment for the coeffs
    mov         x0, x14                     //&src[2nt+1] (to be reloaded after every row)

    cmp         x4, #4
    beq         tf_sz_4



    mov         x10,x6
tf_sz_8_16:
    ld1         {v10.8b, v11.8b}, [x14],#16 //load src[2nt+1+col]
    ld1         {v17.8b},[x12],#8
    mov         v25.8b, v17.8b
    zip1        v29.8b, v17.8b, v25.8b
    zip2        v25.8b, v17.8b, v25.8b
    mov         v17.d[0], v29.d[0]
    sub         v30.8b,  v2.8b ,  v17.8b    //[nt-1-col]
    sub         v31.8b,  v2.8b ,  v25.8b




loop_sz_8_16:

    ldr         w7,  [x6], #-2              //src[2nt-1-row] (dec to take into account row)
    sxtw        x7,w7
    umull       v12.8h, v5.8b, v0.8b        //(row+1)    *    src[nt-1]
    ldr         w11,  [x6], #-2             //src[2nt-1-row] (dec to take into account row)
    sxtw        x11,w11
    umlal       v12.8h, v6.8b, v10.8b       //(nt-1-row)    *    src[2nt+1+col]
    dup         v4.4h,w7                    //src[2nt-1-row]
    umlal       v12.8h, v17.8b, v1.8b       //(col+1)    *    src[3nt+1]
    dup         v3.4h,w11                   //src[2nt-1-row]
    umlal       v12.8h, v30.8b, v4.8b       //(nt-1-col)    *    src[2nt-1-row]



    umull       v28.8h, v5.8b, v0.8b
    ldr         w7,  [x6], #-2              //src[2nt-1-row] (dec to take into account row)
    sxtw        x7,w7
    umlal       v28.8h, v6.8b, v11.8b
    add         v18.8b,  v5.8b ,  v7.8b     //row++ [(row+1)++]c


    umlal       v28.8h, v31.8b, v4.8b
    sub         v19.8b,  v6.8b ,  v7.8b     //[nt-1-row]--
    umlal       v28.8h, v25.8b, v1.8b
    dup         v4.4h,w7                    //src[2nt-1-row]

    umull       v26.8h, v18.8b, v0.8b       //(row+1)    *    src[nt-1]
    add         v12.8h,  v12.8h ,  v16.8h   //add (nt)
    umlal       v26.8h, v19.8b, v10.8b      //(nt-1-row)    *    src[2nt+1+col]
    sshl        v12.8h, v12.8h, v14.8h      //shr
    umlal       v26.8h, v17.8b, v1.8b       //(col+1)    *    src[3nt+1]
    add         v28.8h,  v28.8h ,  v16.8h
    umlal       v26.8h, v30.8b, v3.8b       //(nt-1-col)    *    src[2nt-1-row]
    sshl        v28.8h, v28.8h, v14.8h





    umull       v24.8h, v18.8b, v0.8b
    add         v5.8b,  v18.8b ,  v7.8b     //row++ [(row+1)++]
    umlal       v24.8h, v19.8b, v11.8b
    sub         v6.8b,  v19.8b ,  v7.8b     //[nt-1-row]--
    umlal       v24.8h, v25.8b, v1.8b
    xtn         v12.8b,  v12.8h
    umlal       v24.8h, v31.8b, v3.8b
    xtn         v13.8b,  v28.8h




    add         v26.8h,  v26.8h ,  v16.8h   //add (nt)
    umull       v22.8h, v5.8b, v0.8b        //(row+1)    *    src[nt-1]
    sshl        v26.8h, v26.8h, v14.8h      //shr
    umlal       v22.8h, v6.8b, v10.8b       //(nt-1-row)    *    src[2nt+1+col]
    st1         {v12.2s, v13.2s}, [x2], x3
    umlal       v22.8h, v17.8b, v1.8b       //(col+1)    *    src[3nt+1]
    add         v24.8h,  v24.8h ,  v16.8h
    umlal       v22.8h, v30.8b, v4.8b       //(nt-1-col)    *    src[2nt-1-row]
    sshl        v24.8h, v24.8h, v14.8h

    umull       v20.8h, v5.8b, v0.8b
    add         v18.8b,  v5.8b ,  v7.8b     //row++ [(row+1)++]c
    umlal       v20.8h, v6.8b, v11.8b
    sub         v19.8b,  v6.8b ,  v7.8b     //[nt-1-row]--
    umlal       v20.8h, v31.8b, v4.8b

    ldr         w11,  [x6], #-2             //src[2nt-1-row] (dec to take into account row)
    sxtw        x11,w11
    umlal       v20.8h, v25.8b, v1.8b
    dup         v3.4h,w11                   //src[2nt-1-row]
    add         v22.8h,  v22.8h ,  v16.8h   //add (nt)

    umull       v12.8h, v18.8b, v0.8b       //(row+1)    *    src[nt-1]
    xtn         v26.8b,  v26.8h
    umlal       v12.8h, v19.8b, v10.8b      //(nt-1-row)    *    src[2nt+1+col]
    xtn         v27.8b,  v24.8h

    umlal       v12.8h, v17.8b, v1.8b       //(col+1)    *    src[3nt+1]
    sshl        v22.8h, v22.8h, v14.8h      //shr

    umlal       v12.8h, v30.8b, v3.8b       //(nt-1-col)    *    src[2nt-1-row]
    add         v20.8h,  v20.8h ,  v16.8h

    umull       v28.8h, v18.8b, v0.8b
    st1         {v26.2s, v27.2s}, [x2], x3

    umlal       v28.8h, v19.8b, v11.8b
    add         v5.8b,  v18.8b ,  v7.8b     //row++ [(row+1)++]

    sub         v6.8b,  v19.8b ,  v7.8b     //[nt-1-row]--
    umlal       v28.8h, v25.8b, v1.8b

    umlal       v28.8h, v31.8b, v3.8b
    sshl        v20.8h, v20.8h, v14.8h


    add         v12.8h,  v12.8h ,  v16.8h   //add (nt)
    xtn         v22.8b,  v22.8h


    add         v28.8h,  v28.8h ,  v16.8h
    xtn         v23.8b,  v20.8h


    sshl        v12.8h, v12.8h, v14.8h      //shr
    st1         {v22.2s, v23.2s}, [x2], x3
    sshl        v28.8h, v28.8h, v14.8h





    xtn         v20.8b,  v12.8h
    xtn         v21.8b,  v28.8h

    st1         {v20.2s, v21.2s}, [x2], x3


    subs        x1, x1, #4

    bne         loop_sz_8_16




    cmp         x4,#16

    bne         end_loop


    sub         x4, x4,#16
    dup         v5.8b,w8                    //row + 1
    dup         v6.8b,w9                    //nt - 1 - row
    mov         v7.8b, v5.8b                //mov #1 to d7 to used for inc for row+1 and dec for nt-1-row

    mov         x6,x10
    mov         x1,#16
    sub         x2,x2,x3,lsl #4
    add         x2,x2,#16

    ld1         {v10.8b, v11.8b}, [x14],#16 //load src[2nt+1+col]
    ld1         {v17.8b},[x12],#8
    mov         v25.8b, v17.8b
    zip1        v29.8b, v17.8b, v25.8b
    zip2        v25.8b, v17.8b, v25.8b
    mov         v17.d[0], v29.d[0]
    sub         v30.8b,  v2.8b ,  v17.8b    //[nt-1-col]
    sub         v31.8b,  v2.8b ,  v25.8b

    beq         loop_sz_8_16



tf_sz_4:
    ld1         {v10.8b},[x14]              //load src[2nt+1+col]
    ld1         {v17.8b},[x12], x10         //load 8 coeffs [col+1]
    mov         v25.8b, v17.8b
    zip1        v29.8b, v17.8b, v25.8b
    zip2        v25.8b, v17.8b, v25.8b
    mov         v17.d[0], v29.d[0]
loop_sz_4:
    //mov        x10, #4                @reduce inc to #4 for 4x4
    ldr         w7,  [x6], #-2              //src[2nt-1-row] (dec to take into account row)
    sxtw        x7,w7
    dup         v4.4h,w7                    //src[2nt-1-row]

    sub         v25.8b,  v2.8b ,  v17.8b    //[nt-1-col]

    umull       v12.8h, v5.8b, v0.8b        //(row+1)    *    src[nt-1]
    umlal       v12.8h, v6.8b, v10.8b       //(nt-1-row)    *    src[2nt+1+col]
    umlal       v12.8h, v17.8b, v1.8b       //(col+1)    *    src[3nt+1]
    umlal       v12.8h, v25.8b, v4.8b       //(nt-1-col)    *    src[2nt-1-row]
//    vadd.i16    q6, q6, q8            @add (nt)
//    vshl.s16     q6, q6, q7            @shr
//    vmovn.i16     d12, q6
    rshrn       v12.8b, v12.8h,#3

    st1         {v12.2s},[x2], x3

    add         v5.8b,  v5.8b ,  v7.8b      //row++ [(row+1)++]
    sub         v6.8b,  v6.8b ,  v7.8b      //[nt-1-row]--
    subs        x1, x1, #1

    bne         loop_sz_4

end_loop:
    // ldmfd sp!,{x4-x12,x15}                   //reload the registers from sp
    ldp         x19, x20,[sp],#16
    ldp         d8,d14,[sp],#16             // Loading d14 using { ldr d14,[sp]; add sp,sp,#8 } is giving bus error.
                                            // d8 is used as dummy register and loaded along with d14 using ldp. d8 is not used in the function.
    ldp         d12,d13,[sp],#16
    ldp         d10,d11,[sp],#16
    ret







