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

///**
//******************************************************************************
//* //file
//*  ihevc_inter_pred_luma_horz_w16out.s
//*
//* //brief
//*  contains function definitions for inter prediction  interpolation.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* //author
//*  parthiban v
//*
//* //par list of functions:
//*
//*  - ihevc_inter_pred_luma_horz_w16out()
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
//*   interprediction luma filter for horizontal 16bit output
//*
//* //par description:
//*     applies a horizontal filter with coefficients pointed to  by 'pi1_coeff'
//*     to the elements pointed by 'pu1_src' and  writes to the location pointed
//*     by 'pu1_dst'  no downshifting or clipping is done and the output is  used
//*     as an input for vertical filtering or weighted  prediction   assumptions :
//*     the function is optimized considering the fact width is  multiple of 4 or
//*     8. if width is multiple of 4 then height  should be multiple of 2, width 8
//*     is optimized further.
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

//void ihevc_inter_pred_luma_horz_w16out(uword8 *pu1_src,
//                                word16 *pi2_dst,
//                                word32 src_strd,
//                                word32 dst_strd,
//                                word8 *pi1_coeff,
//                                word32 ht,
//                                word32 wd


//x0 - free
//x1 - dst_ptr
//x2 - src_strd
//x3 - dst_strd
//x8 - src_ptx2
//x9 - inner loop counter
//x10 - dst_ptx2
//x11 - free
//x12 - dst_strd2
//x13 - src_strd1
//x14 - wd
//x15 - #1
//x16 - src_ptx1
//x19 - loop_counter
.text
.align 4

.include "ihevc_neon_macros.s"

.globl ihevc_inter_pred_luma_horz_w16out_av8

.type ihevc_inter_pred_luma_horz_w16out_av8, %function

ihevc_inter_pred_luma_horz_w16out_av8:

    // stmfd sp!, {x8-x16, x19}                //stack stores the values of the arguments
    push_v_regs
    stp         x19, x20,[sp,#-16]!
    mov         x20,#1
    bic         x19, x19, x20               // clearing bit[0], so that it goes back to mode
    mov         x8,x4                       //loads pi1_coeff
    mov         x11,x5                      //loads ht


    ld1         {v0.8b},[x8]                //coeff = vld1_s8(pi1_coeff)
    sub         x19,x11,#0                  //checks for ht == 0
    abs         v2.8b, v0.8b                //vabs_s8(coeff)
    mov         x15,#1
    //ble          end_loops
    mov         x14,x6                      //loads wd
    dup         v24.8b, v2.b[0]             //coeffabs_0 = vdup_lane_u8(coeffabs, 0)
    sub         x16,x0,#3                   //pu1_src - 3
    dup         v25.8b, v2.b[1]             //coeffabs_1 = vdup_lane_u8(coeffabs, 1)
    add         x8,x16,x2                   //pu1_src_tmp2_8 = pu1_src + src_strd
    dup         v26.8b, v2.b[2]             //coeffabs_2 = vdup_lane_u8(coeffabs, 2)
    sub         x20,x14,x2,lsl #1           //2*src_strd - wd
    neg         x13, x20
    dup         v27.8b, v2.b[3]             //coeffabs_3 = vdup_lane_u8(coeffabs, 3)
    sub         x20,x14,x3                  //dst_strd - wd
    neg         x12, x20
    dup         v28.8b, v2.b[4]             //coeffabs_4 = vdup_lane_u8(coeffabs, 4)

    dup         v29.8b, v2.b[5]             //coeffabs_5 = vdup_lane_u8(coeffabs, 5)
    and         x11,x19,#1                  //calculating ht_residue ht_residue = (ht & 1)
    dup         v30.8b, v2.b[6]             //coeffabs_6 = vdup_lane_u8(coeffabs, 6)
    sub         x19,x19,x11                 //decrement height by ht_residue(residue value is calculated outside)
    dup         v31.8b, v2.b[7]             //coeffabs_7 = vdup_lane_u8(coeffabs, 7)

    cmp         x11,#1
    beq         odd_height_decision

even_height_decision:
    mov         x11,x1
    cmp         x14,#4
    ble         outer_loop_4

    cmp         x14,#24
    mov         x20,#16
    csel        x14, x20, x14,eq
    add         x20, x12,#8
    csel        x12, x20, x12,eq
    add         x20, x13,#8
    csel        x13, x20, x13,eq

    cmp         x14,#16
    bge         outer_loop_16_branch

    cmp         x14,#12
    add         x20, x12,#4
    csel        x12, x20, x12,eq
    add         x20, x13,#4
    csel        x13, x20, x13,eq
outer_loop_8_branch:
    b           outer_loop_8

outer_loop_16_branch:
    b           outer_loop_16


odd_height_decision:
    cmp         x14,#24
    beq         outer_loop_8_branch
    cmp         x14,#12
    beq         outer_loop_4
    b           even_height_decision

outer_loop4_residual:
    sub         x16,x0,#3                   //pu1_src - 3
    mov         x1,x11
    add         x1, x1,#16
    mov         x14,#4
    add         x16, x16,#8
    mov         x19,#16
    add         x12, x12,#4
    add         x13, x13,#4

outer_loop_4:
    add         x10,x1,x3,lsl #1            //pu1_dst + dst_strd
    add         x8,x16,x2                   //pu1_src + src_strd

    subs        x9,x14,#0                   //checks wd
    ble         end_inner_loop_4

inner_loop_4:
    mov         x15,#1
    ld1         {v20.2s},[x16],x15          //vector load pu1_src
    ld1         {v21.2s},[x16],x15
    ld1         {v22.2s},[x8],x15           //vector load pu1_src + src_strd
    ld1         {v23.2s},[x8],x15

    zip1        v0.2s, v20.2s, v22.2s
    zip2        v12.2s, v20.2s, v22.2s      //vector zip the i iteration and ii interation in single register
    zip1        v1.2s, v21.2s, v23.2s
    zip2        v13.2s, v21.2s, v23.2s

    ld1         {v20.2s},[x16],x15
    ld1         {v21.2s},[x16],x15
    ld1         {v22.2s},[x8],x15
    ld1         {v23.2s},[x8],x15

    zip1        v2.2s, v20.2s, v22.2s
    zip2        v14.2s, v20.2s, v22.2s
    zip1        v3.2s, v21.2s, v23.2s
    zip2        v15.2s, v21.2s, v23.2s

    ld1         {v20.2s},[x16],x15
    ld1         {v21.2s},[x16],x15
    ld1         {v22.2s},[x8],x15
    ld1         {v23.2s},[x8],x15

    zip1        v4.2s, v20.2s, v22.2s
    zip2        v16.2s, v20.2s, v22.2s
    zip1        v5.2s, v21.2s, v23.2s
    zip2        v17.2s, v21.2s, v23.2s

    ld1         {v20.2s},[x16],x15
    ld1         {v21.2s},[x16],x15
    ld1         {v22.2s},[x8],x15
    ld1         {v23.2s},[x8],x15

    //add        x16,x16,#4                        //increment the input pointer
    sub         x16,x16,#4
    //vext.u8    d2,d0,d1,#2                        //vector extract of src[0_2]
    //vext.u8    d3,d0,d1,#3                        //vector extract of src[0_3]
    //vext.u8    d4,d0,d1,#4                        //vector extract of src[0_4]

    //vext.u8    d5,d0,d1,#5                        //vector extract of src[0_5]
    //vext.u8    d6,d0,d1,#6                        //vector extract of src[0_6]
    //vext.u8    d7,d0,d1,#7                        //vector extract of src[0_7]
    //vext.u8    d1,d0,d1,#1                        //vector extract of src[0_1]
    sub         x8,x8,#4
    // add        x8,x8,#4                        //increment the input pointer
    // vext.u8    d14,d12,d13,#2                    //vector extract of src[0_2]
    // vext.u8    d15,d12,d13,#3                    //vector extract of src[0_3]
    // vext.u8    d16,d12,d13,#4                    //vector extract of src[0_4]
    // vext.u8    d17,d12,d13,#5                    //vector extract of src[0_5]
    // vext.u8    d18,d12,d13,#6                    //vector extract of src[0_6]
    // vext.u8    d19,d12,d13,#7                    //vector extract of src[0_7]
    //vext.u8    d13,d12,d13,#1                    //vector extract of src[0_1]






    zip1        v6.2s, v20.2s, v22.2s
    zip2        v18.2s, v20.2s, v22.2s
    zip1        v7.2s, v21.2s, v23.2s
    zip2        v19.2s, v21.2s, v23.2s

    umull       v8.8h, v1.8b, v25.8b        //arithmetic operations for ii iteration in the same time
    umlsl       v8.8h, v0.8b, v24.8b
    umlsl       v8.8h, v2.8b, v26.8b
    umlal       v8.8h, v3.8b, v27.8b
    umlal       v8.8h, v4.8b, v28.8b
    umlsl       v8.8h, v5.8b, v29.8b
    umlal       v8.8h, v6.8b, v30.8b
    umlsl       v8.8h, v7.8b, v31.8b

    // vqrshrun.s16 d8,q4,#6                        //narrow right shift and saturating the result
    st1         {v8.d}[0],[x1],#8           //store the i iteration result which is in upper part of the register
    st1         {v8.d}[1],[x10],#8          //store the ii iteration result which is in lower part of the register
    subs        x9,x9,#4                    //decrement the wd by 4
    bgt         inner_loop_4

end_inner_loop_4:
    subs        x19,x19,#2                  //decrement the ht by 4
    add         x16,x16,x13                 //increment the input pointer 2*src_strd-wd
    add         x1,x10,x12,lsl #1           //increment the output pointer 2*dst_strd-wd
    bgt         outer_loop_4


height_residue_4:

    mov         x11,x5                      //loads ht
    and         x11,x11,#1                  //calculating ht_residue ht_residue = (ht & 1)
    cmp         x11,#0
    //beq        end_loops
    // ldmeqfd sp!,{x8-x16,pc}                  //reload the registers from sp
    bne         lbl280
    ldp         x19, x20,[sp], #16
    pop_v_regs
    ret
lbl280:

outer_loop_height_residue_4:


    subs        x9,x14,#0                   //checks wd
    ble         end_inner_loop_height_residue_4

inner_loop_height_residue_4:
    mov         x15, #1
    ld1         {v0.2s},[x16],x15           //vector load pu1_src
    ld1         {v1.2s},[x16],x15






    // vext.u8    d2,d0,d1,#2                        //vector extract of src[0_2]
    // vext.u8    d3,d0,d1,#3                        //vector extract of src[0_3]
    // vext.u8    d4,d0,d1,#4                        //vector extract of src[0_4]



    //add        x16,x16,#4                        //increment the input pointer
    // vext.u8    d5,d0,d1,#5                        //vector extract of src[0_5]
    // vext.u8    d6,d0,d1,#6                        //vector extract of src[0_6]
    // vext.u8    d7,d0,d1,#7                        //vector extract of src[0_7]
    // vext.u8    d1,d0,d1,#1                        //vector extract of src[0_1]
    ld1         {v2.2s},[x16],x15
    umull       v8.8h, v1.8b, v25.8b        //arithmetic operations for ii iteration in the same time
    ld1         {v3.2s},[x16],x15
    umlsl       v8.8h, v0.8b, v24.8b
    ld1         {v4.2s},[x16],x15
    umlsl       v8.8h, v2.8b, v26.8b
    ld1         {v5.2s},[x16],x15
    umlal       v8.8h, v3.8b, v27.8b
    ld1         {v6.2s},[x16],x15
    umlal       v8.8h, v4.8b, v28.8b
    ld1         {v7.2s},[x16],x15
    umlsl       v8.8h, v5.8b, v29.8b
    sub         x16,x16,#4
    umlal       v8.8h, v6.8b, v30.8b
    umlsl       v8.8h, v7.8b, v31.8b        //store the i iteration result which is in upper part of the register
    subs        x9,x9,#4                    //decrement the wd by 4
    st1         {v8.d}[0],[x1],#8
    bgt         inner_loop_height_residue_4

end_inner_loop_height_residue_4:
    subs        x11,x11,#1                  //decrement the ht by 4
    sub         x20,x14,x2
    neg         x13, x20
    add         x16,x16,x13                 //increment the input pointer src_strd-wd
    add         x1,x1,x12                   //increment the output pointer dst_strd-wd
    bgt         outer_loop_height_residue_4

    // ldmfd sp!,{x8-x16,pc}                  //reload the registers from sp
    ldp         x19, x20,[sp], #16
    pop_v_regs
    ret

outer_loop8_residual:
    sub         x16,x0,#3                   //pu1_src - 3
    mov         x1,x11
    mov         x19,#32
    add         x1, x1,#32
    add         x16, x16,#16
    mov         x14,#8
    add         x12, x12,#8
    add         x13, x13,#8

outer_loop_8:

    add         x10,x1,x3,lsl #1            //pu1_dst + dst_strd
    add         x8,x16,x2                   //pu1_src + src_strd
    subs        x9,x14,#0                   //checks wd

    ble         end_inner_loop_8

inner_loop_8:
    mov         x15, #1
    ld1         {v0.2s},[x16],x15           //vector load pu1_src
    ld1         {v1.2s},[x16],x15
    ld1         {v2.2s},[x16],x15
    ld1         {v3.2s},[x16],x15





    // vext.u8    d2,d0,d1,#2                        //vector extract of src[0_2]
    // vext.u8    d3,d0,d1,#3                        //vector extract of src[0_3]
    // vext.u8    d4,d0,d1,#4                        //vector extract of src[0_4]
    // vext.u8    d5,d0,d1,#5                        //vector extract of src[0_5]
    // vext.u8    d6,d0,d1,#6                        //vector extract of src [0_6]
    // vext.u8    d7,d0,d1,#7                        //vector extract of src[0_7]
    // vext.u8    d1,d0,d1,#1                        //vector extract of src[0_1]
    // vext.u8    d14,d12,d13,#2

    //vext.u8    d15,d12,d13,#3                    //vector extract of src[0_3]
    // vext.u8    d16,d12,d13,#4                    //vector extract of src[0_4]
    // vext.u8    d17,d12,d13,#5                    //vector extract of src[0_5]
    //vext.u8    d18,d12,d13,#6                    //vector extract of src[0_6]
    //vext.u8    d19,d12,d13,#7                    //vector extract of src[0_7]
    //vext.u8    d13,d12,d13,#1                    //vector extract of src[0_1]
    ld1         {v4.2s},[x16],x15
    umull       v8.8h, v1.8b, v25.8b        //mul_res = vmlal_u8(src[0_1], coeffabs_1)//
    ld1         {v5.2s},[x16],x15
    umlal       v8.8h, v3.8b, v27.8b        //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    ld1         {v6.2s},[x16],x15
    umlsl       v8.8h, v0.8b, v24.8b        //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    ld1         {v7.2s},[x16],x15
    umlsl       v8.8h, v2.8b, v26.8b        //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    ld1         {v12.2s},[x8],x15           //vector load pu1_src + src_strd
    umlal       v8.8h, v4.8b, v28.8b        //mul_res = vmlal_u8(src[0_4], coeffabs_4)//
    ld1         {v13.2s},[x8],x15
    umlsl       v8.8h, v5.8b, v29.8b        //mul_res = vmlsl_u8(src[0_5], coeffabs_5)//
    ld1         {v14.2s},[x8],x15
    umlal       v8.8h, v6.8b, v30.8b        //mul_res = vmlal_u8(src[0_6], coeffabs_6)//
    ld1         {v15.2s},[x8],x15
    umlsl       v8.8h, v7.8b, v31.8b        //mul_res = vmlsl_u8(src[0_7], coeffabs_7)//
    ld1         {v16.2s},[x8],x15           //vector load pu1_src + src_strd

    umull       v10.8h, v15.8b, v27.8b      //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    ld1         {v17.2s},[x8],x15
    umlsl       v10.8h, v14.8b, v26.8b      //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    ld1         {v18.2s},[x8],x15
    umlal       v10.8h, v16.8b, v28.8b      //mul_res = vmlal_u8(src[0_4], coeffabs_4)//
    ld1         {v19.2s},[x8],x15           //vector load pu1_src + src_strd
    umlsl       v10.8h, v17.8b, v29.8b      //mul_res = vmlsl_u8(src[0_5], coeffabs_5)//
    // vqrshrun.s16     d20,q4,#6                        //right shift and saturating narrow result 1
    umlal       v10.8h, v18.8b, v30.8b      //mul_res = vmlal_u8(src[0_6], coeffabs_6)//
    umlsl       v10.8h, v19.8b, v31.8b      //mul_res = vmlsl_u8(src[0_7], coeffabs_7)//
    st1         {v8.8h},[x1],#16            //store the result pu1_dst
    umlsl       v10.8h, v12.8b, v24.8b      //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    umlal       v10.8h, v13.8b, v25.8b      //mul_res = vmlal_u8(src[0_1], coeffabs_1)//



    // vqrshrun.s16 d8,q5,#6                        //right shift and saturating narrow result 2
    subs        x9,x9,#8                    //decrement the wd loop
    st1         {v10.8h},[x10],#16          //store the result pu1_dst
    cmp         x9,#4
    bgt         inner_loop_8

end_inner_loop_8:
    subs        x19,x19,#2                  //decrement the ht loop
    add         x16,x16,x13                 //increment the src pointer by 2*src_strd-wd
    add         x1,x10,x12,lsl #1           //increment the dst pointer by 2*dst_strd-wd
    bgt         outer_loop_8





    mov         x14,x6                      //loads wd
    cmp         x14,#12

    beq         outer_loop4_residual

    mov         x11,x5                      //loads ht
    and         x11,x11,#1
    cmp         x11,#1
    beq         height_residue_4

//end_loops

    // ldmfd sp!,{x8-x16,pc}                  //reload the registers from sp
    ldp         x19, x20,[sp], #16
    pop_v_regs
    ret





outer_loop_16:
    mov         x15, #-7
    stp         x0,x11,[sp,#-16]!
    add         x10,x1,x3,lsl #1            //pu1_dst + dst_strd
    add         x8,x16,x2                   //pu1_src + src_strd
    and         x0, x16, #31
    sub         x9,x14,#0                   //checks wd
    //ble          end_loops1
    add         x20,x16, x2, lsl #1
    prfm        PLDL1KEEP,[x20]
    ld1         {v0.2s},[x16],#8            //vector load pu1_src
    ld1         {v1.2s},[x16],x15           //vector load pu1_src
    add         x20,x8, x2, lsl #1
    prfm        PLDL1KEEP,[x20]
    ld1         {v2.2s},[x16],#8
    ld1         {v3.2s},[x16],x15
    ld1         {v4.2s},[x16],#8
    ld1         {v5.2s},[x16],x15
    ld1         {v6.2s},[x16],#8
    ld1         {v7.2s},[x16],x15
    ld1         {v12.2s},[x16],#8
    ld1         {v13.2s},[x16],x15
    umull       v8.8h, v2.8b, v25.8b        //mul_res = vmlal_u8(src[0_1], coeffabs_1)//
    ld1         {v14.2s},[x16],#8
    ld1         {v15.2s},[x16],x15
    umlal       v8.8h, v6.8b, v27.8b        //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    ld1         {v16.2s},[x16],#8
    ld1         {v17.2s},[x16],x15
    umlsl       v8.8h, v0.8b, v24.8b        //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    ld1         {v18.2s},[x16],#8
    ld1         {v19.2s},[x16],x15
    umlsl       v8.8h, v4.8b, v26.8b        //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    umlal       v8.8h, v12.8b, v28.8b       //mul_res = vmlal_u8(src[0_4], coeffabs_4)//
    umlsl       v8.8h, v14.8b, v29.8b       //mul_res = vmlsl_u8(src[0_5], coeffabs_5)//
    umlal       v8.8h, v16.8b, v30.8b       //mul_res = vmlal_u8(src[0_6], coeffabs_6)//
    umlsl       v8.8h, v18.8b, v31.8b       //mul_res = vmlsl_u8(src[0_7], coeffabs_7)//


inner_loop_16:


    subs        x9,x9,#16
    umull       v20.8h, v3.8b, v25.8b

    add         x16, x16,#8
    umlsl       v20.8h, v1.8b, v24.8b

    ld1         {v0.2s},[x8],#8             //vector load pu1_src
    ld1         {v1.2s},[x8],x15            //vector load pu1_src
    umlal       v20.8h, v7.8b, v27.8b

    ld1         {v2.2s},[x8],#8
    ld1         {v3.2s},[x8],x15
    umlsl       v20.8h, v5.8b, v26.8b

    ld1         {v4.2s},[x8],#8
    ld1         {v5.2s},[x8],x15
    umlal       v20.8h, v13.8b, v28.8b

    ld1         {v6.2s},[x8],#8
    ld1         {v7.2s},[x8],x15
    umlal       v20.8h, v17.8b, v30.8b

    ld1         {v12.2s},[x8],#8
    ld1         {v13.2s},[x8],x15
    umlsl       v20.8h, v15.8b, v29.8b

    ld1         {v14.2s},[x8],#8
    ld1         {v15.2s},[x8],x15
    umlsl       v20.8h, v19.8b, v31.8b

    ld1         {v16.2s},[x8],#8
    ld1         {v17.2s},[x8],x15
    umull       v10.8h, v2.8b, v25.8b       //mul_res = vmlal_u8(src[0_1], coeffabs_1)//

    ld1         {v18.2s},[x8],#8
    ld1         {v19.2s},[x8],x15
    umlal       v10.8h, v6.8b, v27.8b       //mul_res = vmull_u8(src[0_3], coeffabs_3)//

    add         x8, x8,#8
    umlsl       v10.8h, v0.8b, v24.8b       //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    add         x20,x16, x2, lsl #2
    prfm        PLDL1KEEP,[x20]
    add         x20,x8, x2, lsl #2
    prfm        PLDL1KEEP,[x20]
    st1         {v8.16b},[x1],#16           //store the result pu1_dst
    umlsl       v10.8h, v4.8b, v26.8b       //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//

    add         x20,x16,x13                 //increment the src pointer by 2*src_strd-wd
    csel        x16, x20, x16,eq
    umlal       v10.8h, v12.8b, v28.8b      //mul_res = vmlal_u8(src[0_4], coeffabs_4)//

    add         x20,x16,x2                  //pu1_src + src_strd
    csel        x8, x20, x8,eq
    umlsl       v10.8h, v14.8b, v29.8b      //mul_res = vmlsl_u8(src[0_5], coeffabs_5)//

//    and            x11, x16, #31
    umlal       v10.8h, v16.8b, v30.8b      //mul_res = vmlal_u8(src[0_6], coeffabs_6)//

    sub         x20,x19,#2
    csel        x19, x20, x19,eq
    umlsl       v10.8h, v18.8b, v31.8b      //mul_res = vmlsl_u8(src[0_7], coeffabs_7)//

    //cmp            x11, x0
    umull       v22.8h, v3.8b, v25.8b

//    add x20,x16, x2, lsl #2
    prfm        PLDL1KEEP,[x20]
    umlsl       v22.8h, v1.8b, v24.8b

    st1         {v20.8h},[x1],#16
    umlal       v22.8h, v7.8b, v27.8b

//    add x20,x8, x2, lsl #2
    prfm        PLDL1KEEP,[x20]
    umlsl       v22.8h, v5.8b, v26.8b

//    mov            x0, x11
    umlal       v22.8h, v13.8b, v28.8b

    cmp         x19,#0
    umlal       v22.8h, v17.8b, v30.8b

    st1         {v10.8h},[x10],#16
    umlsl       v22.8h, v15.8b, v29.8b

    umlsl       v22.8h, v19.8b, v31.8b

    beq         epilog_16

    ld1         {v0.2s},[x16],#8            //vector load pu1_src
    ld1         {v1.2s},[x16],x15           //vector load pu1_src
    ld1         {v2.2s},[x16],#8
    ld1         {v3.2s},[x16],x15
    ld1         {v4.2s},[x16],#8
    ld1         {v5.2s},[x16],x15
    ld1         {v6.2s},[x16],#8
    ld1         {v7.2s},[x16],x15
    ld1         {v12.2s},[x16],#8
    ld1         {v13.2s},[x16],x15
    umull       v8.8h, v2.8b, v25.8b        //mul_res = vmlal_u8(src[0_1], coeffabs_1)//
    ld1         {v14.2s},[x16],#8
    ld1         {v15.2s},[x16],x15
    umlal       v8.8h, v6.8b, v27.8b        //mul_res = vmull_u8(src[0_3], coeffabs_3)//
    ld1         {v16.2s},[x16],#8
    ld1         {v17.2s},[x16],x15
    umlsl       v8.8h, v0.8b, v24.8b        //mul_res = vmlsl_u8(src[0_0], coeffabs_0)//
    ld1         {v18.2s},[x16],#8
    ld1         {v19.2s},[x16],x15
    umlsl       v8.8h, v4.8b, v26.8b        //mul_res = vmlsl_u8(src[0_2], coeffabs_2)//
    umlal       v8.8h, v12.8b, v28.8b       //mul_res = vmlal_u8(src[0_4], coeffabs_4)//
    cmp         x9,#0
    umlsl       v8.8h, v14.8b, v29.8b       //mul_res = vmlsl_u8(src[0_5], coeffabs_5)//
    mov         x20,x14
    csel        x9, x20, x9,eq
    umlal       v8.8h, v16.8b, v30.8b       //mul_res = vmlal_u8(src[0_6], coeffabs_6)//
    st1         {v22.16b},[x10],#16         //store the result pu1_dst
    umlsl       v8.8h, v18.8b, v31.8b       //mul_res = vmlsl_u8(src[0_7], coeffabs_7)//
    add         x20,x10,x12,lsl #1
    csel        x1, x20, x1,eq
    add         x20,x1,x3,lsl #1            //pu1_dst + dst_strd
    csel        x10, x20, x10,eq
    b           inner_loop_16


epilog_16:
//    vqrshrun.s16 d11,q11,#6
    st1         {v22.16b},[x10],#16         //store the result pu1_dst

    ldp         x0,x11,[sp],#16
    mov         x14,x6
    cmp         x14,#24
    beq         outer_loop8_residual
    add         x1,x10,x12,lsl #1
    mov         x11,x5                      //loads ht
    and         x11,x11,#1
    cmp         x11,#1
    beq         height_residue_4

end_loops1:

    // ldmfd sp!,{x8-x16,pc}                  //reload the registers from sp
    ldp         x19, x20,[sp], #16
    pop_v_regs
    ret









