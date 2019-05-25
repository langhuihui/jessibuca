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
//* //file
//*  ihevc_inter_pred_chroma_horz_neon.s
//*
//* //brief
//*  contains function definitions for inter prediction  interpolation.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* //author
//*  yogeswaran rs / akshaya mukund
//*
//* //par list of functions:
//*
//*
//* //remarks
//*  none
//*
//*******************************************************************************
//*/
///**
//*******************************************************************************
//*
//* //brief
//*       chroma interprediction filter to store horizontal 16bit ouput
//*
//* //par description:
//*    applies a horizontal filter with coefficients pointed to  by 'pi1_coeff'
//*    to the elements pointed by 'pu1_src' and  writes to the location pointed
//*    by 'pu1_dst'  no downshifting or clipping is done and the output is  used
//*    as an input for vertical filtering or weighted  prediction
//*
//* //param[in] pu1_src
//*  uword8 pointer to the source
//*
//* //param[out] pi2_dst
//*  word16 pointer to the destination
//*
//* //param[in] src_strd
//*  integer source stride
//*
//* //param[in] dst_strd
//*  integer destination stride
//*
//* //param[in] pi1_coeff
//*  word8 pointer to the filter coefficients
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
//void ihevc_inter_pred_chroma_horz_w16out(uword8 *pu1_src,
//                                          word16 *pi2_dst,
//                                          word32 src_strd,
//                                          word32 dst_strd,
//                                          word8 *pi1_coeff,
//                                          word32 ht,
//                                          word32 wd)
//**************variables vs registers*****************************************
//x0 => *pu1_src
//x1 => *pi2_dst
//x2 =>  src_strd
//x3 =>  dst_strd


.text
.align 4

.include "ihevc_neon_macros.s"

.globl ihevc_inter_pred_chroma_horz_w16out_av8


.type ihevc_inter_pred_chroma_horz_w16out_av8, %function

ihevc_inter_pred_chroma_horz_w16out_av8:

    // stmfd sp!, {x4-x12, x14}                    //stack stores the values of the arguments

    stp         d10,d11,[sp,#-16]!
    stp         d12,d13,[sp,#-16]!
    stp         d14,d15,[sp,#-16]!
    stp         x19, x20,[sp,#-16]!

    mov         x15,x4 // pi1_coeff
    mov         x16,x5 // ht
    mov         x17,x6 // wd

    mov         x4,x15                      //loads pi1_coeff
    mov         x6,x16                      //loads ht
    mov         x10,x17                     //loads wd

    ld1         {v0.8b},[x4]                //coeff = vld1_s8(pi1_coeff)
    subs        x14,x6,#0                   //checks for ht == 0
    abs         v2.8b, v0.8b                //vabs_s8(coeff)

//******* added
    mov         x11, #2
//******* added ends

    ble         end_loops

    dup         v24.8b, v2.b[0]             //coeffabs_0 = vdup_lane_u8(coeffabs, 0)
    sub         x12,x0,#2                   //pu1_src - 2
    dup         v25.8b, v2.b[1]             //coeffabs_1 = vdup_lane_u8(coeffabs, 1)
    add         x4,x12,x2                   //pu1_src_tmp2_8 = pu1_src + src_strd
    dup         v26.8b, v2.b[2]             //coeffabs_2 = vdup_lane_u8(coeffabs, 2)

    tst         x10,#3                      //checks wd for multiples of 4
    lsl         x5, x10, #1                 //2wd

    dup         v27.8b, v2.b[3]             //coeffabs_3 = vdup_lane_u8(coeffabs, 3)

    and         x7,x14,#1                   //added                //calculating ht_residue ht_residue = (ht & 1)
    sub         x14,x14,x7                  //added                //decrement height by ht_residue(residue value is calculated outside)

    bne         outer_loop_4                // this branching happens when the width is 2 or 6

    cmp         x10,#12
    beq         skip_16

    cmp         x10,#8
    bge         outer_loop_16

skip_16:
    tst         x6,#3

//******* removal
    //mov        x11,#8
//******* removal ends

    sub         x9,x0,#2
    beq         outer_loop_ht_4             //this branching happens when the height is a a multiple of 4



//     cmp        x10,#12
//     beq     outer_loop_8
//     cmp        x10,#16
//     bge    outer_loop_16
    b           outer_loop_8



outer_loop_16:
    add         x4,x12,x2


    and         x0, x12, #31
    add         x20,x12, x2 , lsl #1
    prfm        PLDL1KEEP,[x20]






    add         x19,x12,#8
    ld1         { v0.2s},[x12],x11          //vector load pu1_src
    ld1         { v1.2s},[x19],x11          //vector load pu1_src
    mov         x10,x5                      //2wd
    mul         x14, x14 , x10
    ld1         { v2.2s},[x12],x11          //vector load pu1_src
    ld1         { v3.2s},[x19],x11          //vector load pu1_src
    add         x20,x4, x2 , lsl #1
    prfm        PLDL1KEEP,[x20]
    mov         x9,#10
    ld1         { v4.2s},[x12],x11          //vector load pu1_src
    ld1         { v5.2s},[x19],x11          //vector load pu1_src
    sub         x20,x3,#8
    neg         x6, x20
    sub         x8,x3,#8
    ld1         { v6.2s},[x12],x9           //vector load pu1_src
    ld1         { v7.2s},[x19],x9           //vector load pu1_src


    add         x19,x4,#8
    umull       v30.8h, v2.8b, v25.8b       //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    ld1         { v29.2s},[x4],x11          //vector load pu1_src
    ld1         { v31.2s},[x19],x11         //vector load pu1_src

    umlsl       v30.8h, v0.8b, v24.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         { v10.2s},[x4],x11          //vector load pu1_src
    ld1         { v11.2s},[x19],x11         //vector load pu1_src

    umlal       v30.8h, v4.8b, v26.8b       //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    ld1         { v12.2s},[x4],x11          //vector load pu1_src
    ld1         { v13.2s},[x19],x11         //vector load pu1_src

    umlsl       v30.8h, v6.8b, v27.8b       //mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    ld1         { v14.4s},[x4],x9           //vector load pu1_src
    ld1         { v15.2s},[x19],x9          //vector load pu1_src

    umull       v28.8h, v3.8b, v25.8b
    lsl         x6,x6,#1
    sub         x20,x5,x3,lsl #1
    neg         x3, x20
    umlsl       v28.8h, v1.8b, v24.8b
    lsl         x8,x8,#1
    sub         x20,x5,x2,lsl #1
    neg         x7, x20
    umlal       v28.8h, v5.8b, v26.8b

    umlsl       v28.8h, v7.8b, v27.8b
    cmp         x14,#32
    beq         epilog_end
    sub         x14, x14,#64

inner_loop_16:

    // and            x7, x12, #31                    //decrement the wd loop
    // cmp            x7, x0
    add         x20,x12, x2 , lsl #2
    prfm        PLDL1KEEP,[x20]
    add         x20,x4, x2 , lsl #2
    prfm        PLDL1KEEP,[x20]


    subs        x10,x10,#16

    umull       v22.8h, v10.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//



//     add x20,x12,x2,lsl #1
    //csel x12, x20, x12,eq
//     sub x20,x12,x5
    //csel x12, x20, x12,eq
    add         x20,x12,x7
    csel        x12, x20, x12,eq
    add         x20,x12,x2
    csel        x4, x20, x4,eq


    st1         { v30.8h}, [x1],#16
    umlsl       v22.8h, v29.8b, v24.8b      //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//




    add         x19,x12,#8
    ld1         { v0.2s},[x12],x11          //vector load pu1_src
    ld1         { v1.2s},[x19],x11          //vector load pu1_src
    umlal       v22.8h, v12.8b, v26.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//




    ld1         { v2.2s},[x12],x11          //vector load pu1_src
    ld1         { v3.2s},[x19],x11          //vector load pu1_src
    umlsl       v22.8h, v14.8b, v27.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//


    ld1         { v4.2s},[x12],x11          //vector load pu1_src
    ld1         { v5.2s},[x19],x11          //vector load pu1_src
    umull       v20.8h, v11.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//

    st1         { v28.8h}, [x1],x8
    umlsl       v20.8h, v31.8b, v24.8b      //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         { v6.2s},[x12],x9           //vector load pu1_src
    ld1         { v7.2s},[x19],x9           //vector load pu1_src
    umlal       v20.8h, v13.8b, v26.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    add         x19,x4,#8
    ld1         { v29.2s},[x4],x11          //vector load pu1_src
    ld1         { v31.2s},[x19],x11         //vector load pu1_src
    umlsl       v20.8h, v15.8b, v27.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//


    ld1         { v10.2s},[x4],x11          //vector load pu1_src
    ld1         { v11.2s},[x19],x11         //vector load pu1_src
    umull       v30.8h, v2.8b, v25.8b       //mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         { v12.2s},[x4],x11          //vector load pu1_src
    ld1         { v13.2s},[x19],x11         //vector load pu1_src
    umlsl       v30.8h, v0.8b, v24.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         { v14.2s},[x4],x9           //vector load pu1_src
    ld1         { v15.2s},[x19],x9          //vector load pu1_src
    umlal       v30.8h, v4.8b, v26.8b       //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    st1         { v22.8h},[x1],#16          //store the result pu1_dst
    umlsl       v30.8h, v6.8b, v27.8b       //mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    csel        x10, x5, x10,eq             //2wd
    umull       v28.8h, v3.8b, v25.8b



    umlsl       v28.8h, v1.8b, v24.8b
    st1         { v20.8h},[x1],x6           //store the result pu1_dst


    add         x20,x1,x3,lsl #1
    csel        x1, x20, x1,eq
    umlal       v28.8h, v5.8b, v26.8b

    subs        x14,x14,#32                 //decrement the ht loop
    umlsl       v28.8h, v7.8b, v27.8b



//     mov            x0, x7
    bgt         inner_loop_16



    add         x14,x14,#64
    cmp         x14,#32
    beq         epilog_end

epilog:

    st1         { v30.8h}, [x1],#16
    umull       v22.8h, v10.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    st1         { v28.8h}, [x1],x8



    umlsl       v22.8h, v29.8b, v24.8b      //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    subs        x10,x10,#16                 //decrement the wd loop
    umlal       v22.8h, v12.8b, v26.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
//     add x20,x12,x2,lsl #1
    //csel x12, x20, x12,eq
    add         x20,x12,x7
    csel        x12, x20, x12,eq
    umlsl       v22.8h, v14.8b, v27.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//
    // sub x20,x12,x5
    //csel x12, x20, x12,eq
    csel        x10, x5, x10,eq             //2wd
    add         x20,x12,x2
    csel        x4, x20, x4,eq
    umull       v20.8h, v11.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//

    add         x19,x12,#8
    ld1         { v0.2s},[x12],x11          //vector load pu1_src
    ld1         { v1.2s},[x19],x11          //vector load pu1_src

    umlsl       v20.8h, v31.8b, v24.8b      //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         { v2.2s},[x12],x11          //vector load pu1_src
    ld1         { v3.2s},[x19],x11          //vector load pu1_src

    umlal       v20.8h, v13.8b, v26.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    ld1         { v4.2s},[x12],x11          //vector load pu1_src
    ld1         { v5.2s},[x19],x11          //vector load pu1_src

    umlsl       v20.8h, v15.8b, v27.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//
    ld1         { v6.2s},[x12],x9           //vector load pu1_src
    ld1         { v7.2s},[x19],x9           //vector load pu1_src
    umull       v30.8h, v2.8b, v25.8b       //mul_res = vmull_u8(src[0_3], coeffabs_3)//

    add         x19,x4,#8
    ld1         { v29.2s},[x4],x11          //vector load pu1_src
    ld1         { v31.2s},[x19],x11         //vector load pu1_src
    umlsl       v30.8h, v0.8b, v24.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         { v10.2s},[x4],x11          //vector load pu1_src
    ld1         { v11.2s},[x19],x11         //vector load pu1_src
    umlal       v30.8h, v4.8b, v26.8b       //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    umlsl       v30.8h, v6.8b, v27.8b       //mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    ld1         { v12.2s},[x4],x11          //vector load pu1_src
    ld1         { v13.2s},[x19],x11         //vector load pu1_src
    umull       v28.8h, v3.8b, v25.8b

    ld1         { v14.2s},[x4],x9           //vector load pu1_src
    ld1         { v15.2s},[x19],x9          //vector load pu1_src

    umlsl       v28.8h, v1.8b, v24.8b
    st1         { v22.8h},[x1],#16          //store the result pu1_dst
    umlal       v28.8h, v5.8b, v26.8b
    st1         { v20.8h},[x1],x6           //store the result pu1_dst
    umlsl       v28.8h, v7.8b, v27.8b
    add         x20,x1,x3,lsl #1
    csel        x1, x20, x1,eq


epilog_end:

    umull       v22.8h, v10.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    umlsl       v22.8h, v29.8b, v24.8b      //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    umlal       v22.8h, v12.8b, v26.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    umlsl       v22.8h, v14.8b, v27.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//


    umull       v20.8h, v11.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    umlsl       v20.8h, v31.8b, v24.8b      //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    umlal       v20.8h, v13.8b, v26.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    umlsl       v20.8h, v15.8b, v27.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//


    st1         { v30.8h}, [x1],#16
    st1         { v28.8h}, [x1],x8
    st1         { v22.8h},[x1],#16          //store the result pu1_dst
    st1         { v20.8h},[x1],x6           //store the result pu1_dst


    mov         x6,x16                      //loads ht

    and         x7,x6,#1

    cmp         x7,#0
    mov         x10,x5
    add         x20,x12,x2,lsl #1
    csel        x12, x20, x12,ne
    sub         x20,x12,x5
    csel        x12, x20, x12,ne
    add         x20,x1,x3,lsl #1
    csel        x1, x20, x1,ne


    bgt         loop_residue_4

    b           end_loops




outer_loop_8:

    add         x6,x1,x3,lsl #1             //pu1_dst + dst_strd
    mov         x10,x5                      //2wd
    add         x4,x12,x2                   //pu1_src + src_strd

inner_loop_8:
    //ld1 {v0.2s, v1.2s},[x12],x11                //vector load pu1_src
    ld1         {v0.2s},[x12],x11           //vector load pu1_src
    ld1         {v1.2s},[x12],x11           //vector load pu1_src
    ld1         {v2.2s},[x12],x11           //vector load pu1_src
    ld1         {v3.2s},[x12],x11           //vector load pu1_src


    //vext.u8    d2,d0,d1,#2                        //vector extract of src[0_2]
    umull       v29.8h, v1.8b, v25.8b       //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    umlsl       v29.8h, v0.8b, v24.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    //vext.u8    d4,d0,d1,#4                        //vector extract of src[0_4]
    //vext.u8    d6,d0,d1,#6                        //vector extract of src[0_6]
    umlal       v29.8h, v2.8b, v26.8b       //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    umlsl       v29.8h, v3.8b, v27.8b       //mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    //ld1 {v12.2s, v13.2s},[x4],x11                //vector load pu1_src + src_strd
    ld1         {v4.2s},[x4],x11            //vector load pu1_src
    ld1         {v5.2s},[x4],x11            //vector load pu1_src
    ld1         {v6.2s},[x4],x11            //vector load pu1_src
    ld1         {v7.2s},[x4],x11            //vector load pu1_src
    //vext.u8    d14,d12,d13,#2                    //vector extract of src[0_2]
    umull       v10.8h, v5.8b, v25.8b       //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    umlsl       v10.8h, v4.8b, v24.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    //vext.u8    d16,d12,d13,#4                    //vector extract of src[0_4]
    //vext.u8    d18,d12,d13,#6                    //vector extract of src[0_6]
    umlal       v10.8h, v6.8b, v26.8b       //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    umlsl       v10.8h, v7.8b, v27.8b       //mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    st1         {v29.8h}, [x1],#16

    subs        x10,x10,#8                  //decrement the wd loop
    st1         {v10.8h},[x6],#16           //store the result pu1_dst
    bgt         inner_loop_8

    sub         x12,x12,x5
    subs        x14,x14,#2                  //decrement the ht loop
    sub         x1,x1,x5,lsl #1
    add         x12,x12,x2,lsl #1
    add         x1,x1,x3,lsl #2
    bgt         outer_loop_8

    cmp         x7,#0
    mov         x10,x5
    bgt         loop_residue_4

    b           end_loops



//height if 4 comes
outer_loop_ht_4:

    mov         x10,x5

prologue_ht_4:
    lsl         x8, x3, #1

inner_loop_ht_4:

    mov         x12,x9
    mov         x4,x1

    sub         x0, x2, #6                  // not sure if x0 needs to be preserved

    ld1         {v0.2s},[x12],x11           //(1)vector load pu1_src
    ld1         {v1.2s},[x12],x11           //(1)vector load pu1_src
    ld1         {v2.2s},[x12],x11           //(1)vector load pu1_src
    ld1         {v3.2s},[x12],x0            //(1)vector load pu1_src

    ld1         {v4.2s},[x12],x11           //(2)vector load pu1_src
    ld1         {v5.2s},[x12],x11           //(2)vector load pu1_src
    ld1         {v6.2s},[x12],x11           //(2)vector load pu1_src
    ld1         {v7.2s},[x12],x0            //(2)vector load pu1_src

    ld1         {v14.2s},[x12],x11          //(3)vector load pu1_src
    umull       v29.8h, v1.8b, v25.8b       //(1)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         {v15.2s},[x12],x11          //(3)vector load pu1_src
    umlsl       v29.8h, v0.8b, v24.8b       //(1)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v16.2s},[x12],x11          //(3)vector load pu1_src
    umlal       v29.8h, v2.8b, v26.8b       //(1)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    ld1         {v17.2s},[x12],x0           //(3)vector load pu1_src
    umlsl       v29.8h, v3.8b, v27.8b       //(1)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    ld1         {v18.2s},[x12],x11          //(4)vector load pu1_src
    umull       v10.8h, v5.8b, v25.8b       //(2)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         {v19.2s},[x12],x11          //(4)vector load pu1_src
    umlsl       v10.8h, v4.8b, v24.8b       //(2)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v20.2s},[x12],x11          //(4)vector load pu1_src
    umlal       v10.8h, v6.8b, v26.8b       //(2)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    ld1         {v21.2s},[x12],x2           //(4)vector load pu1_src
    umlsl       v10.8h, v7.8b, v27.8b       //(2)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    add         x9,x9,#8                    //(core loop)

    subs        x10,x10,#8                  //(prologue)decrement the wd loop
    beq         epilogue

core_loop:
    st1         {v29.8h},[x4],x8            //(1)store the result pu1_dst
    mov         x12,x9

    ld1         {v0.2s},[x12],x11           //(1_1)vector load pu1_src
    umull       v12.8h, v15.8b, v25.8b      //(3)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         {v1.2s},[x12],x11           //(1_1)vector load pu1_src
    umlsl       v12.8h, v14.8b, v24.8b      //(3)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v2.2s},[x12],x11           //(1_1)vector load pu1_src
    umlal       v12.8h, v16.8b, v26.8b      //(3)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    ld1         {v3.2s},[x12],x0            //(1_1)vector load pu1_src
    umlsl       v12.8h, v17.8b, v27.8b      //(3)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    st1         {v10.8h},[x4],x8            //(2)store the result pu1_dst
    add         x9,x9,#8                    //(core loop)

    ld1         {v4.2s},[x12],x11           //(2_1)vector load pu1_src
    umull       v22.8h, v19.8b, v25.8b      //(4)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         {v5.2s},[x12],x11           //(2_1)vector load pu1_src
    umlsl       v22.8h, v18.8b, v24.8b      //(4)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v6.2s},[x12],x11           //(2_1)vector load pu1_src
    umlal       v22.8h, v20.8b, v26.8b      //(4)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    ld1         {v7.2s},[x12],x0            //(2_1)vector load pu1_src
    umlsl       v22.8h, v21.8b, v27.8b      //(4)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    st1         {v12.8h},[x4],x8            //(3)store the result pu1_dst
    add         x1,x1,#16                   //(core loop)

    ld1         {v14.2s},[x12],x11          //(3_1)vector load pu1_src
    umull       v29.8h, v1.8b, v25.8b       //(1_1)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         {v15.2s},[x12],x11          //(3_1)vector load pu1_src
    umlsl       v29.8h, v0.8b, v24.8b       //(1_1)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v16.2s},[x12],x11          //(3_1)vector load pu1_src
    umlal       v29.8h, v2.8b, v26.8b       //(1_1)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    ld1         {v17.2s},[x12],x0           //(3_1)vector load pu1_src
    umlsl       v29.8h, v3.8b, v27.8b       //(1_1)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    st1         {v22.8h}, [x4], x8          //(4)store the result pu1_dst
    subs        x10,x10,#8                  //(core loop)

    umull       v10.8h, v5.8b, v25.8b       //(2_1)mul_res = vmull_u8(src[0_3], coeffabs_3)//
    ld1         {v18.2s},[x12],x11          //(4_1)vector load pu1_src

    ld1         {v19.2s},[x12],x11          //(4_1)vector load pu1_src
    umlsl       v10.8h, v4.8b, v24.8b       //(2_1)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v20.2s},[x12],x11          //(4_1)vector load pu1_src
    umlal       v10.8h, v6.8b, v26.8b       //(2_1)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    mov         x4, x1                      //(core loop)

    ld1         {v21.2s},[x12],x0           //(4_1)vector load pu1_src
    umlsl       v10.8h, v7.8b, v27.8b       //(2_1)mul_res = vmlal_u8(src[0_1], coeffabs_1)//



    bgt         core_loop                   //loopback

epilogue:
    umull       v12.8h, v15.8b, v25.8b      //(3)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    umlsl       v12.8h, v14.8b, v24.8b      //(3)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    umlal       v12.8h, v16.8b, v26.8b      //(3)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    umlsl       v12.8h, v17.8b, v27.8b      //(3)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    st1         {v29.8h},[x4], x8           //(1)store the result pu1_dst

    umull       v22.8h, v19.8b, v25.8b      //(4)mul_res = vmull_u8(src[0_3], coeffabs_3)//
    umlsl       v22.8h, v18.8b, v24.8b      //(4)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    umlal       v22.8h, v20.8b, v26.8b      //(4)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    umlsl       v22.8h, v21.8b, v27.8b      //(4)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    st1         {v10.8h},[x4], x8           //(2)store the result pu1_dst

    st1         {v12.8h},[x4], x8           //(3)store the result pu1_dst

    add         x1,x1,#16                   //(core loop)

    st1         {v22.8h},[x4], x8           //(4)store the result pu1_dst

    sub         x9,x9,x5
    subs        x14,x14,#4                  //decrement the ht loop
    sub         x1,x1,x5,lsl #1
    add         x9,x9,x2,lsl #2
    add         x1,x1,x3,lsl #3
    bgt         outer_loop_ht_4

    cmp         x7,#0
    mov         x10,x5
    csel        x12, x9, x12,gt
    csel        x4, x1, x4,gt
    bgt         loop_residue_4

    b           end_loops

outer_loop_4:
    add         x6,x1,x3,lsl #1             //pu1_dst + dst_strd
    mov         x10,x5
    add         x4,x12,x2                   //pu1_src + src_strd

inner_loop_4:
    //ld1 {v0.2s, v1.2s},[x12]                    //vector load pu1_src
    ld1         {v20.2s},[x12],x11          //vector load pu1_src
    ld1         {v21.2s},[x12],x11          //vector load pu1_src
    ld1         {v22.2s},[x12],x11          //vector load pu1_src
    ld1         {v23.2s},[x12]              //vector load pu1_src

//**** removal
    //add        x12,x12,#4                        //increment the input pointer
//**** removal ends
//**** addn
    sub         x12,x12,#2                  //increment the input pointer
//**** addn ends
    ld1         {v16.2s},[x4],x11           //vector load pu1_src
    ld1         {v17.2s},[x4],x11           //vector load pu1_src
    ld1         {v18.2s},[x4],x11           //vector load pu1_src
    ld1         {v19.2s},[x4]               //vector load pu1_src
    //vext.u8    d2,d0,d1,#2                        //vector extract of src[0_2]
    //vext.u8    d4,d0,d1,#4                        //vector extract of src[0_4]
    //ld1 {v12.2s, v13.2s},[x4]                    //vector load pu1_src + src_strd
    //vext.u8    d6,d0,d1,#6                        //vector extract of src[0_6]

    //add        x4,x4,#4                        //increment the input pointer
    sub         x4,x4,#2
    //vext.u8    d14,d12,d13,#2                    //vector extract of src[0_2]
    //vext.u8    d16,d12,d13,#4                    //vector extract of src[0_4]
    //vext.u8    d18,d12,d13,#6                    //vector extract of src[0_6]

//**** removal
    //zip1 v0.2s, v0.2s, v12.2s
    //zip2  v12.2s, v0.2s, v12.2s                             //vector zip the i iteration and ii interation in single register
    //zip1 v2.2s, v2.2s, v14.2s
    //zip2  v14.2s, v2.2s, v14.2s
    //zip1 v4.2s, v4.2s, v16.2s
    //zip2  v16.2s, v4.2s, v16.2s
    //zip1 v6.2s, v6.2s, v18.2s
    //zip2  v18.2s, v6.2s, v18.2s
//**** removal ends
//**** addn
    zip1        v0.2s, v20.2s, v16.2s
    zip2        v4.2s, v20.2s, v16.2s       //vector zip the i iteration and ii interation in single register
    zip1        v1.2s, v21.2s, v17.2s
    zip2        v5.2s, v21.2s, v17.2s
    zip1        v2.2s, v22.2s, v18.2s
    zip2        v6.2s, v22.2s, v18.2s
    zip1        v3.2s, v23.2s, v19.2s
    zip2        v7.2s, v23.2s, v19.2s
//**** addn ends

    umull       v29.8h, v1.8b, v25.8b       //arithmetic operations for ii iteration in the same time
    umlsl       v29.8h, v0.8b, v24.8b
    umlal       v29.8h, v2.8b, v26.8b
    umlsl       v29.8h, v3.8b, v27.8b

    st1         {v29.d}[0],[x1],#8          //store the i iteration result which is in upper part of the register
    subs        x10,x10,#4                  //decrement the wd by 4

    st1         {v29.d}[1],[x6],#8          //store the ii iteration result which is in lower part of the register

    bgt         inner_loop_4

    sub         x12,x12,x5
    subs        x14,x14,#2                  //decrement the ht by 2
    sub         x1,x1,x5,lsl #1
    add         x12,x12,x2,lsl #1
    add         x1,x1,x3,lsl #2
    bgt         outer_loop_4

    cmp         x7,#0
    mov         x10,x5
    beq         end_loops

loop_residue_4:

    mov         x10,x5                      //2wd

loop_residue:

    //ld1 {v0.2s, v1.2s},[x12]                    //vector load pu1_src
    ld1         {v20.2s},[x12],x11          //vector load pu1_src
    ld1         {v21.2s},[x12],x11          //vector load pu1_src
    ld1         {v22.2s},[x12],x11          //vector load pu1_src
    ld1         {v23.2s},[x12]              //vector load pu1_src
    //vext.u8        d2,d0,d1,#2                //vector extract of src[0_2]
    //umull v8.8h, v2.8b, v25.8b                //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    //umlsl v8.8h, v0.8b, v24.8b                //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    //vext.u8        d4,d0,d1,#4                //vector extract of src[0_4]
    //add            x12,x12,#4                //pu1_src + 4
    sub         x12, x12, #2
    //vext.u8        d6,d0,d1,#6                //vector extract of src[0_6]
    //umlal v8.8h, v4.8b, v26.8b                //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    //umlsl v8.8h, v6.8b, v27.8b                //mul_res = vmlal_u8(src[0_1], coeffabs_1)//
    umull       v29.8h, v21.8b, v25.8b
    umlsl       v29.8h, v20.8b, v24.8b
    umlal       v29.8h, v22.8b, v26.8b
    umlsl       v29.8h, v23.8b, v27.8b

    st1         {v29.1d},[x1]               //store the result pu1_dst
    subs        x10,x10,#4                  //decrement the wd loop
    add         x1,x1,#8                    //pi2_dst + 8

    bgt         loop_residue                //loop again

    //inner loop ends
    //add            x8,x3,lsl #1            //2*dst_strd
    //sub             x8,x8,x5,lsl #1            //2*dst_strd - 2wd
    //sub             x9,x2,x5                //src_strd - 2wd
    //subs             x7,x7,#1                //decrement the ht loop
    //add             x12,x12,x9                //pu1_src + src_strd
    //add            x1,x1,x8                //pu1_dst + 2*dst_strd
    //bgt              outer_loop_residue_4    //loop again
    //b                 end_loops                //jumps to end

end_loops:

    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16
    ldp         d14,d15,[sp],#16
    ldp         d12,d13,[sp],#16
    ldp         d10,d11,[sp],#16
    ret






