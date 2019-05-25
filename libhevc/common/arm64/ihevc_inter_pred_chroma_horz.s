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
//*    chroma interprediction filter for horizontal input
//*
//* //par description:
//*    applies a horizontal filter with coefficients pointed to  by 'pi1_coeff'
//*    to the elements pointed by 'pu1_src' and  writes to the location pointed
//*    by 'pu1_dst'  the output is downshifted by 6 and clipped to 8 bits
//*    assumptions : the function is optimized considering the fact width is
//*    multiple of 2,4 or 8. if width is 2, then height  should be multiple of 2.
//*    width 4,8 is optimized further
//*
//* //param[in] pu1_src
//*  uword8 pointer to the source
//*
//* //param[out] pu1_dst
//*  uword8 pointer to the destination
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

//void ihevc_inter_pred_chroma_horz(uword8 *pu1_src,
//                                   uword8 *pu1_dst,
//                                   word32 src_strd,
//                                   word32 dst_strd,
//                                   word8 *pi1_coeff,
//                                   word32 ht,
//                                   word32 wd)
//**************variables vs registers*****************************************
//x0 => *pu1_src
//x1 => *pi2_dst
//x2 =>  src_strd
//x3 =>  dst_strd

.text
.align 4

.include "ihevc_neon_macros.s"

.globl ihevc_inter_pred_chroma_horz_av8

.type ihevc_inter_pred_chroma_horz_av8, %function

ihevc_inter_pred_chroma_horz_av8:

    // stmfd sp!, {x4-x12, x14}                    //stack stores the values of the arguments

    stp         d9,d10,[sp,#-16]!
    stp         d11,d12,[sp,#-16]!
    stp         d13,d14,[sp,#-16]!
    stp         d8,d15,[sp,#-16]!           // Storing d15 using { sub sp,sp,#8; str d15,[sp] } is giving bus error.
                                            // d8 is used as dummy register and stored along with d15 using stp. d8 is not used in the function.
    stp         x19, x20,[sp,#-16]!

    mov         x15,x4 // pi1_coeff
    mov         x16,x5 // ht
    mov         x17,x6 // wd


    mov         x4,x15                      //loads pi1_coeff
    mov         x7,x16                      //loads ht
    mov         x10,x17                     //loads wd

    ld1         {v0.8b},[x4]                //coeff = vld1_s8(pi1_coeff)
    subs        x14,x7,#0                   //checks for ht == 0
    abs         v2.8b, v0.8b                //vabs_s8(coeff)
    mov         x11,#2
    ble         end_loops

    dup         v24.8b, v2.b[0]             //coeffabs_0 = vdup_lane_u8(coeffabs, 0)
    sub         x12,x0,#2                   //pu1_src - 2
    dup         v25.8b, v2.b[1]             //coeffabs_1 = vdup_lane_u8(coeffabs, 1)
    add         x4,x12,x2                   //pu1_src_tmp2_8 = pu1_src + src_strd
    dup         v26.8b, v2.b[2]             //coeffabs_2 = vdup_lane_u8(coeffabs, 2)

    tst         x10,#3                      //checks wd for multiples
    lsl         x5, x10, #1

    dup         v27.8b, v2.b[3]             //coeffabs_3 = vdup_lane_u8(coeffabs, 3)

    bne         outer_loop_4
    cmp         x10,#12
    beq         skip_16

    cmp         x10,#8
    bge         outer_loop_16
skip_16:
    tst         x7,#3

    sub         x9,x0,#2
    beq         outer_loop_ht_4             //jumps to else condition

    b           outer_loop_8


outer_loop_16:
    mov         x10,x5                      //2wd
    mul         x14, x14 , x10

    sub         x20,x3,#16
    neg         x6, x20

    add         x4,x12,x2
    mov         x9,#10
    and         x0, x12, #31
    sub         x20,x5,x3,lsl #1
    neg         x8, x20
    add         x20,x12, x2 , lsl #1
    prfm        PLDL1KEEP,[x20]



    add         x19,x12,#8
    ld1         { v0.2s},[x12],x11          //vector load pu1_src
    ld1         { v1.2s},[x19],x11          //vector load pu1_src
    add         x20,x4, x2 , lsl #1
    prfm        PLDL1KEEP,[x20]

    ld1         { v2.2s},[x12],x11          //vector load pu1_src
    ld1         { v3.2s},[x19],x11          //vector load pu1_src

    ld1         { v4.2s},[x12],x11          //vector load pu1_src
    ld1         { v5.2s},[x19],x11          //vector load pu1_src

    ld1         { v6.2s},[x12],x9           //vector load pu1_src
    ld1         { v7.2s},[x19],x9           //vector load pu1_src


    add         x19,x4,#8
    umull       v30.8h, v2.8b, v25.8b       //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    ld1         { v29.2s},[x4],x11          //vector load pu1_src
    ld1         { v9.2s},[x19],x11          //vector load pu1_src

    umlsl       v30.8h, v0.8b, v24.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         { v10.2s},[x4],x11          //vector load pu1_src
    ld1         { v11.2s},[x19],x11         //vector load pu1_src

    umlal       v30.8h, v4.8b, v26.8b       //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    ld1         { v12.2s},[x4],x11          //vector load pu1_src
    ld1         { v13.2s},[x19],x11         //vector load pu1_src

    umlsl       v30.8h, v6.8b, v27.8b       //mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    ld1         { v14.2s},[x4],x9           //vector load pu1_src
    ld1         { v15.2s},[x19],x9          //vector load pu1_src

    umull       v28.8h, v3.8b, v25.8b

    umlsl       v28.8h, v1.8b, v24.8b


    umlal       v28.8h, v5.8b, v26.8b

    umlsl       v28.8h, v7.8b, v27.8b


    cmp         x14,#32
    beq         epilog_end
    sub         x14, x14,#64

inner_loop_16:




//     bgt            l_2

//    add x20,x12, x2 , lsl #1
    prfm        PLDL1KEEP,[x20]
//    add x20,x4, x2 , lsl #1
    prfm        PLDL1KEEP,[x20]



    subs        x10,x10,#16

    umull       v22.8h, v10.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//


    add         x20,x12,x8
    csel        x12, x20, x12,eq
    add         x20,x12,x2
    csel        x4, x20, x4,eq
    umlsl       v22.8h, v29.8b, v24.8b      //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//



    add         x20,x12, x2 , lsl #2
    prfm        PLDL1KEEP,[x20]
    sqrshrun    v30.8b, v30.8h,#6

    add         x19,x12,#8
    ld1         { v0.2s},[x12],x11          //vector load pu1_src
    ld1         { v1.2s},[x19],x11          //vector load pu1_src

    sqrshrun    v31.8b, v28.8h,#6



    ld1         { v2.2s},[x12],x11          //vector load pu1_src
    ld1         { v3.2s},[x19],x11          //vector load pu1_src
    umlal       v22.8h, v12.8b, v26.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//




    ld1         { v4.2s},[x12],x11          //vector load pu1_src
    ld1         { v5.2s},[x19],x11          //vector load pu1_src
    umlsl       v22.8h, v14.8b, v27.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//


    ld1         { v6.2s},[x12],x9           //vector load pu1_src
    ld1         { v7.2s},[x19],x9           //vector load pu1_src
    umull       v20.8h, v11.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//

    add         x20,x4, x2 , lsl #2
    prfm        PLDL1KEEP,[x20]
    umlsl       v20.8h, v9.8b, v24.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    //mov       v30.s[1],v31.s[0]
    add         x13,x1,#8
    st1         { v30.4h}, [x1],x3
    st1         { v31.4h}, [x13],x3
    umlal       v20.8h, v13.8b, v26.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    add         x19,x4,#8
    ld1         { v29.2s},[x4],x11          //vector load pu1_src
    ld1         { v9.2s},[x19],x11          //vector load pu1_src
    umlsl       v20.8h, v15.8b, v27.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//


    ld1         { v10.2s},[x4],x11          //vector load pu1_src
    ld1         { v11.2s},[x19],x11         //vector load pu1_src
    umull       v30.8h, v2.8b, v25.8b       //mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         { v12.2s},[x4],x11          //vector load pu1_src
    ld1         { v13.2s},[x19],x11         //vector load pu1_src
    umlsl       v30.8h, v0.8b, v24.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         { v14.2s},[x4],x9           //vector load pu1_src
    ld1         { v15.2s},[x19],x11         //vector load pu1_src
    umlal       v30.8h, v4.8b, v26.8b       //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    cmp         x10,#0
    sqrshrun    v22.8b, v22.8h,#6
    sqrshrun    v23.8b, v20.8h,#6



    umlsl       v30.8h, v6.8b, v27.8b       //mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    csel        x10, x5, x10,eq             //2wd
    umull       v28.8h, v3.8b, v25.8b


    //add       x13,x1,#8
    //mov       v22.s[1],v23.s[0]
    st1         { v22.4h},[x1],x6           //store the result pu1_dst
    st1         { v23.4h},[x13],x6          //store the result pu1_dst
    umlsl       v28.8h, v1.8b, v24.8b


    add         x20,x1,x8
    csel        x1, x20, x1,eq
    umlal       v28.8h, v5.8b, v26.8b

    subs        x14,x14,#32                 //decrement the ht loop
    umlsl       v28.8h, v7.8b, v27.8b

//      mov            x0, x7

    bgt         inner_loop_16



    add         x14,x14,#64
    cmp         x14,#32
    beq         epilog_end

epilog:
    sqrshrun    v30.8b, v30.8h,#6
    sqrshrun    v31.8b, v28.8h,#6



    add         x13,x1,#8
    //mov       v30.s[1],v31.s[0]
    st1         { v30.4h}, [x1],x3
    st1         { v31.4h}, [x13],x3

    umull       v22.8h, v10.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//




    umlsl       v22.8h, v29.8b, v24.8b      //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    subs        x10,x10,#16                 //decrement the wd loop
    umlal       v22.8h, v12.8b, v26.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    add         x20,x12,x8
    csel        x12, x20, x12,eq
    umlsl       v22.8h, v14.8b, v27.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//
    csel        x10, x5, x10,eq             //2wd


    add         x20,x12,x2
    csel        x4, x20, x4,eq
    umull       v20.8h, v11.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//

    add         x19,x12,#8
    ld1         { v0.2s},[x12],x11          //vector load pu1_src
    ld1         { v1.2s},[x19],x11          //vector load pu1_src

    umlsl       v20.8h, v9.8b, v24.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
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
    ld1         { v9.2s},[x19],x11          //vector load pu1_src
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
    sqrshrun    v22.8b, v22.8h,#6
    sqrshrun    v23.8b, v20.8h,#6

    //mov       v22.s[1],v23.s[0]
    st1         { v22.4h},[x1],x6           //store the result pu1_dst
    st1         { v23.4h},[x13],x6          //store the result pu1_dst
    umlal       v28.8h, v5.8b, v26.8b

    umlsl       v28.8h, v7.8b, v27.8b
    add         x20,x1,x8
    csel        x1, x20, x1,eq



epilog_end:
    sqrshrun    v30.8b, v30.8h,#6
    sqrshrun    v31.8b, v28.8h,#6


    umull       v22.8h, v10.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    umlsl       v22.8h, v29.8b, v24.8b      //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    umlal       v22.8h, v12.8b, v26.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    umlsl       v22.8h, v14.8b, v27.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//


    umull       v20.8h, v11.8b, v25.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    umlsl       v20.8h, v9.8b, v24.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    umlal       v20.8h, v13.8b, v26.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    umlsl       v20.8h, v15.8b, v27.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//
    sqrshrun    v22.8b, v22.8h,#6
    sqrshrun    v23.8b, v20.8h,#6

    add         x13,x1,#8

    //mov       v30.s[1],v31.s[0]
    st1         { v30.4h}, [x1],x3
    st1         { v31.4h}, [x13],x3

    //mov       v22.s[1],v23.s[0]
    st1         { v22.4h},[x1]              //store the result pu1_dst
    st1         { v23.4h},[x13]             //store the result pu1_dst



    b           end_loops



















outer_loop_8:


    add         x6,x1,x3                    //pu1_dst + dst_strd
    mov         x7,x5
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

    ld1         {v4.2s},[x4],x11            //vector load pu1_src
    ld1         {v5.2s},[x4],x11            //vector load pu1_src
    ld1         {v6.2s},[x4],x11            //vector load pu1_src
    ld1         {v7.2s},[x4],x11            //vector load pu1_src
    //ld1 {v12.2s, v13.2s},[x4],x11                //vector load pu1_src + src_strd
    //vext.u8    d14,d12,d13,#2                    //vector extract of src[0_2]
    umull       v10.8h, v5.8b, v25.8b       //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    umlsl       v10.8h, v4.8b, v24.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    //vext.u8    d16,d12,d13,#4                    //vector extract of src[0_4]
    //vext.u8    d18,d12,d13,#6                    //vector extract of src[0_6]
    sqrshrun    v29.8b, v29.8h,#6           //right shift and saturating narrow result 1
    umlal       v10.8h, v6.8b, v26.8b       //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    umlsl       v10.8h, v7.8b, v27.8b       //mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    st1         {v29.8b},[x1],#8            //store the result pu1_dst

    sqrshrun    v10.8b, v10.8h,#6           //right shift and saturating narrow result 2
    subs        x7,x7,#8                    //decrement the wd loop
    st1         {v10.8b},[x6],#8            //store the result pu1_dst
    bgt         inner_loop_8

    sub         x12,x12,x5
    subs        x14,x14,#2                  //decrement the ht loop
    sub         x1,x1,x5
    add         x12,x12,x2,lsl #1
    add         x1,x1,x3,lsl #1
    bgt         outer_loop_8
    b           end_loops

//height if 4 comes
outer_loop_ht_4:

    mov         x7,x5

prologue_ht_4:

inner_loop_ht_4:

    mov         x12,x9
    mov         x4,x1

    sub         x8, x2, #6

    ld1         {v0.2s},[x12],x11           //(1)vector load pu1_src
    ld1         {v1.2s},[x12],x11           //(1)vector load pu1_src
    ld1         {v2.2s},[x12],x11           //(1)vector load pu1_src
    //ld1 {v3.2s},[x12],x2                //(1)vector load pu1_src
    ld1         {v3.2s},[x12],x8            //(1)vector load pu1_src

    //sub        x12, x12, #6                //(1)

    ld1         {v4.2s},[x12],x11           //(2)vector load pu1_src
    ld1         {v5.2s},[x12],x11           //(2)vector load pu1_src
    ld1         {v6.2s},[x12],x11           //(2)vector load pu1_src
    //ld1 {v7.2s},[x12],x2                //(2)vector load pu1_src
    ld1         {v7.2s},[x12],x8            //(2)vector load pu1_src

    //sub        x12, x12, #6                //(2)

    ld1         {v14.2s},[x12],x11          //(3)vector load pu1_src
    umull       v29.8h, v1.8b, v25.8b       //(1)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         {v15.2s},[x12],x11          //(3)vector load pu1_src
    umlsl       v29.8h, v0.8b, v24.8b       //(1)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v16.2s},[x12],x11          //(3)vector load pu1_src
    umlal       v29.8h, v2.8b, v26.8b       //(1)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    //ld1 {v17.2s},[x12],x2                //(3)vector load pu1_src
    ld1         {v17.2s},[x12],x8           //(3)vector load pu1_src
    umlsl       v29.8h, v3.8b, v27.8b       //(1)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    //sub        x12, x12, #6                //(3)
    umull       v10.8h, v5.8b, v25.8b       //(2)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         {v18.2s},[x12],x11          //(4)vector load pu1_src
    umlsl       v10.8h, v4.8b, v24.8b       //(2)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v19.2s},[x12],x11          //(4)vector load pu1_src
    umlal       v10.8h, v6.8b, v26.8b       //(2)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    ld1         {v20.2s},[x12],x11          //(4)vector load pu1_src
    umlsl       v10.8h, v7.8b, v27.8b       //(2)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    ld1         {v21.2s},[x12],x2           //(4)vector load pu1_src
    sqrshrun    v29.8b, v29.8h,#6           //(1)right shift and saturating narrow result 1

    add         x9,x9,#8                    //(core loop)

    subs        x7,x7,#8                    //(prologue)decrement the wd loop
    beq         epilogue

core_loop:
    mov         x12,x9

    ld1         {v0.2s},[x12],x11           //(1_1)vector load pu1_src
    umull       v12.8h, v15.8b, v25.8b      //(3)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         {v1.2s},[x12],x11           //(1_1)vector load pu1_src
    umlsl       v12.8h, v14.8b, v24.8b      //(3)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v2.2s},[x12],x11           //(1_1)vector load pu1_src
    umlal       v12.8h, v16.8b, v26.8b      //(3)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    //ld1 {v3.2s},[x12],x2                //(1_1)vector load pu1_src
    ld1         {v3.2s},[x12],x8            //(1_1)vector load pu1_src
    umlsl       v12.8h, v17.8b, v27.8b      //(3)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    //sub        x12, x12, #6                //(1_1)

    st1         {v29.8b},[x4],x3            //(1)store the result pu1_dst
    sqrshrun    v10.8b, v10.8h,#6           //(2)right shift and saturating narrow result 2

    ld1         {v4.2s},[x12],x11           //(2_1)vector load pu1_src
    umull       v22.8h, v19.8b, v25.8b      //(4)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         {v5.2s},[x12],x11           //(2_1)vector load pu1_src
    umlsl       v22.8h, v18.8b, v24.8b      //(4)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v6.2s},[x12],x11           //(2_1)vector load pu1_src
    umlal       v22.8h, v20.8b, v26.8b      //(4)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    //ld1 {v7.2s},[x12],x2                //(2_1)vector load pu1_src
    ld1         {v7.2s},[x12],x8            //(2_1)vector load pu1_src
    umlsl       v22.8h, v21.8b, v27.8b      //(4)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    //sub        x12, x12, #6                //(2_1)

    st1         {v10.8b},[x4],x3            //(2)store the result pu1_dst
    sqrshrun    v12.8b, v12.8h,#6           //(3)right shift and saturating narrow result 1

    ld1         {v14.2s},[x12],x11          //(3_1)vector load pu1_src
    umull       v29.8h, v1.8b, v25.8b       //(1_1)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    ld1         {v15.2s},[x12],x11          //(3_1)vector load pu1_src
    umlsl       v29.8h, v0.8b, v24.8b       //(1_1)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v16.2s},[x12],x11          //(3_1)vector load pu1_src
    umlal       v29.8h, v2.8b, v26.8b       //(1_1)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    //ld1 {v17.2s},[x12],x2                //(3_1)vector load pu1_src
    ld1         {v17.2s},[x12],x8           //(3_1)vector load pu1_src
    umlsl       v29.8h, v3.8b, v27.8b       //(1_1)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    //sub        x12, x12, #6                //(3_1)

    st1         {v12.8b},[x4],x3            //(3)store the result pu1_dst
    sqrshrun    v22.8b, v22.8h,#6           //(4)right shift and saturating narrow result 2

    add         x9,x9,#8                    //(core loop)

    umull       v10.8h, v5.8b, v25.8b       //(2_1)mul_res = vmull_u8(src[0_3], coeffabs_3)//
    ld1         {v18.2s},[x12],x11          //(4_1)vector load pu1_src

    ld1         {v19.2s},[x12],x11          //(4_1)vector load pu1_src
    umlsl       v10.8h, v4.8b, v24.8b       //(2_1)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    ld1         {v20.2s},[x12],x11          //(4_1)vector load pu1_src
    umlal       v10.8h, v6.8b, v26.8b       //(2_1)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    ld1         {v21.2s},[x12],x2           //(4_1)vector load pu1_src
    umlsl       v10.8h, v7.8b, v27.8b       //(2_1)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    add         x1,x1,#8                    //(core loop)

    subs        x7,x7,#8                    //(core loop)

    st1         {v22.8b},[x4], x3           //(4)store the result pu1_dst
    sqrshrun    v29.8b, v29.8h,#6           //(1_1)right shift and saturating narrow result 1

    mov         x4, x1                      //(core loop)

    bgt         core_loop                   //loopback

epilogue:
    umull       v12.8h, v15.8b, v25.8b      //(3)mul_res = vmull_u8(src[0_3], coeffabs_3)//

    umlsl       v12.8h, v14.8b, v24.8b      //(3)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    umlal       v12.8h, v16.8b, v26.8b      //(3)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    umlsl       v12.8h, v17.8b, v27.8b      //(3)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    st1         {v29.8b},[x4],x3            //(1)store the result pu1_dst
    sqrshrun    v10.8b, v10.8h,#6           //(2)right shift and saturating narrow result 2

    umull       v22.8h, v19.8b, v25.8b      //(4)mul_res = vmull_u8(src[0_3], coeffabs_3)//
    umlsl       v22.8h, v18.8b, v24.8b      //(4)mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    umlal       v22.8h, v20.8b, v26.8b      //(4)mul_res = vmlsl_u8(src[0_0], coeffabs_0)//

    umlsl       v22.8h, v21.8b, v27.8b      //(4)mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    st1         {v10.8b},[x4],x3            //(2)store the result pu1_dst
    sqrshrun    v12.8b, v12.8h,#6           //(3)right shift and saturating narrow result 1

    st1         {v12.8b},[x4],x3            //(3)store the result pu1_dst

    add         x1,x1,#8                    //(core loop)

    sqrshrun    v22.8b, v22.8h,#6           //(4)right shift and saturating narrow result 2


    st1         {v22.8b},[x4], x3           //(4)store the result pu1_dst

    sub         x9,x9,x5
    subs        x14,x14,#4                  //decrement the ht loop
    sub         x1,x1,x5
    add         x9,x9,x2,lsl #2
    add         x1,x1,x3,lsl #2
    bgt         outer_loop_ht_4
    b           end_loops

outer_loop_4:
    add         x6,x1,x3                    //pu1_dst + dst_strd
    mov         x7,x5
    add         x4,x12,x2                   //pu1_src + src_strd

inner_loop_4:
    //ld1 {v0.2s, v1.2s},[x12]                    //vector load pu1_src

    ld1         {v20.2s},[x12],x11          //vector load pu1_src
    ld1         {v21.2s},[x12],x11          //vector load pu1_src
    ld1         {v22.2s},[x12],x11          //vector load pu1_src
    ld1         {v23.2s},[x12]              //vector load pu1_src

    sub         x12,x12,#2                  //increment the input pointer
    ld1         {v16.2s},[x4],x11           //vector load pu1_src
    ld1         {v17.2s},[x4],x11           //vector load pu1_src
    ld1         {v18.2s},[x4],x11           //vector load pu1_src
    ld1         {v19.2s},[x4]               //vector load pu1_src
    //vext.u8    d2,d0,d1,#2                        //vector extract of src[0_2]
    //vext.u8    d4,d0,d1,#4                        //vector extract of src[0_4]
    //ld1 {v12.2s, v13.2s},[x4]                    //vector load pu1_src + src_strd
    //vext.u8    d6,d0,d1,#6                        //vector extract of src[0_6]

    sub         x4,x4,#2                    //increment the input pointer
    //vext.u8    d14,d12,d13,#2                    //vector extract of src[0_2]
    //vext.u8    d16,d12,d13,#4                    //vector extract of src[0_4]
    //vext.u8    d18,d12,d13,#6                    //vector extract of src[0_6]

    zip1        v0.2s, v20.2s, v16.2s
    zip2        v4.2s, v20.2s, v16.2s       //vector zip the i iteration and ii interation in single register
    zip1        v1.2s, v21.2s, v17.2s
    zip2        v5.2s, v21.2s, v17.2s
    zip1        v2.2s, v22.2s, v18.2s
    zip2        v6.2s, v22.2s, v18.2s
    zip1        v3.2s, v23.2s, v19.2s
    zip2        v7.2s, v23.2s, v19.2s

    umull       v29.8h, v1.8b, v25.8b       //arithmetic operations for ii iteration in the same time
    umlsl       v29.8h, v0.8b, v24.8b
    umlal       v29.8h, v2.8b, v26.8b
    umlsl       v29.8h, v3.8b, v27.8b

    sqrshrun    v29.8b, v29.8h,#6           //narrow right shift and saturating the result
    st1         {v29.s}[0],[x1],#4          //store the i iteration result which is in upper part of the register
    subs        x7,x7,#4                    //decrement the wd by 4

    st1         {v29.s}[1],[x6],#4          //store the ii iteration result which is in lower part of the register

    bgt         inner_loop_4

    sub         x12,x12,x5
    subs        x14,x14,#2                  //decrement the ht by 2
    sub         x1,x1,x5
    add         x12,x12,x2,lsl #1
    add         x1,x1,x3,lsl #1
    bgt         outer_loop_4

end_loops:

    // ldmfd sp!,{x4-x12,x15}                  //reload the registers from sp
    ldp         x19, x20,[sp],#16
    ldp         d8,d15,[sp],#16             // Loading d15 using { ldr d15,[sp]; add sp,sp,#8 } is giving bus error.
                                            // d8 is used as dummy register and loaded along with d15 using ldp. d8 is not used in the function.
    ldp         d13,d14,[sp],#16
    ldp         d11,d12,[sp],#16
    ldp         d9,d10,[sp],#16
    ret








