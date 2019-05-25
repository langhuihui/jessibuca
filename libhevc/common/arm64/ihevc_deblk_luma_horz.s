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
///*******************************************************************************
//* @file
//*  ihevc_deblk_luma_vert.s
//*
//* @brief
//*  contains function definitions for inter prediction  interpolation.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* @author
//*  anand s
//*
//* @par list of functions:
//*
//*
//* @remarks
//*  none
//*
//*******************************************************************************/

.text
.align 4


.extern gai4_ihevc_tc_table
.extern gai4_ihevc_beta_table
.globl ihevc_deblk_luma_horz_av8

.type ihevc_deblk_luma_horz_av8, %function

ihevc_deblk_luma_horz_av8:
    // stmfd sp!, {x3-x12,x14}
    sxtw        x5,w5
    sxtw        x6,w6
    stp         d8,d9,[sp,#-16]!            // Storing d9 using { sub sp,sp,#8; str d9,[sp] } is giving bus error.
                                            // d8 is used as dummy register and stored along with d9 using stp. d8 is not used in the function.
    stp         d10,d11,[sp,#-16]!
    stp         d12,d13,[sp,#-16]!
    stp         d14,d15,[sp,#-16]!
    stp         x19, x20,[sp,#-16]!
    stp         x21, x22,[sp,#-16]!

    mov         x21,x7
    ldr         w22,[sp,#96]

    add         x3,x3,x4
    add         x3,x3,#1
    asr         x3,x3,#1
    add         x7,x3,x5,lsl #1
    add         x3,x3,x6,lsl #1
    cmp         x7,#0x33
    mov         x20,#0x33
    csel        x7, x20, x7,gt
    bgt         l1.1532
    cmp         x7,#0x0
    mov         x20,#0x0
    csel        x7, x20, x7,lt              // x7 has the beta_index value
l1.1532:
    //     bic      x2,x2,#1
    asr         x2,x2,#1

    add         x3,x3,x2,lsl #1
    cmp         x3,#0x35
    mov         x20,#0x35
    csel        x3, x20, x3,gt
    bgt         l1.1564
    cmp         x3,#0x0
    mov         x20,#0x0
    csel        x3, x20, x3,lt              // x3 has the tc_index value

    //    qp_luma = (quant_param_p + quant_param_q + 1) >> 1@
    //    beta_indx = clip3(qp_luma + (beta_offset_div2 << 1), 0, 51)@
    //    tc_indx = clip3(qp_luma + (2 * (bs >> 1)) + (tc_offset_div2 << 1), 0, 53)@

l1.1564:
    adrp        x2, :got:gai4_ihevc_beta_table
    ldr         x2, [x2, #:got_lo12:gai4_ihevc_beta_table]

    adrp        x4, :got:gai4_ihevc_tc_table
    ldr         x4, [x4, #:got_lo12:gai4_ihevc_tc_table]

    ldr         w5, [x2,x7,lsl #2]          // beta
    ldr         w6, [x4,x3,lsl #2]          // tc



    cmp         x6,#0
    beq         l1.2404
    movi        v0.4h, #0x2
    lsl         x7,x6,#1
    add         x14,x1,x1,lsl #1
    neg         x19,x14
    ldr         w8, [x0,x19]                // -3 value
    dup         v1.8b,w7
    lsl         x19,x1,#1
    neg         x19,x19
    ldr         w10, [x0,x19]               //-2 value
    dup         v23.2s,w8                   // -3 value
    neg         x19,x1
    ldr         w11, [x0,x19]               //-1 value
    dup         v24.2s,w10                  // -2 value
    and         x8,x8,#0xff
    ldr         w12, [x0,#0]                // 0 value
    dup         v25.2s,w11                  // -1 value
    and         x10,x10,#0xff
    ldr         w9, [x0,x1]                 // 1 value
    dup         v26.2s,w12                  // 0 value
    and         x11,x11,#0xff
    lsl         x19,x1,#1
    ldr         w2, [x0,x19]                // 2 value
    dup         v27.2s,w9                   // 1value
    and         x12,x12,#0xff
    dup         v28.2s,w2                   // 2 value
    and         x9,x9,#0xff
    and         x2,x2,#0xff

    add         x12,x12,x2
    subs        x9,x12,x9,lsl #1            // dq0 value is stored in x9
    csneg       x9,x9,x9,pl
    //dq0 = abs( pu1_src[2] - 2 * pu1_src[1] + pu1_src[0] )@

    add         x8,x8,x11
    subs        x8,x8,x10,lsl #1
    csneg       x8,x8,x8,pl                 // dp0 value is stored in x8
    //  dp0 = abs( pu1_src[-3] - 2 * pu1_src[-2] + pu1_src[-1] )@



    add         x3,x1,x1,lsl #1
    add         x14,x0,#3


    neg         x19,x3
    ldrb        w2,[x14,x19]                // -2 value
    lsl         x19,x1,#1
    neg         x19,x19
    ldrb        w10,[x14,x19]               // -2 value
    neg         x19,x1
    ldrb        w11,[x14,x19]               // -1 value
    ldrb        w12,[x14,#0]                // 0 value
    ldrb        w3,[x14,x1]                 // 1 value
    lsl         x19,x1,#1
    ldrb        w4,[x14,x19]                // 2 value


    add         x12,x12,x4
    subs        x12,x12,x3,lsl #1           // dq3value is stored in x12
    csneg       x12,x12,x12,pl
    //    dq3 = abs( pu1_src[3 * src_strd + 2] - 2 * pu1_src[3 * src_strd + 1]+ pu1_src[3 * src_strd + 0] )@


    add         x2,x2,x11
    subs        x11,x2,x10,lsl #1
    csneg       x11,x11,x11,pl              // dp3 value is stored in x8
    //    dp3 = abs( pu1_src[3 * src_strd - 3] - 2 * pu1_src[3 * src_strd - 2]   + pu1_src[3 * src_strd - 1] )@



    add         x3,x8,x9                    // x3 has the d0 value
    add         x4,x11,x12                  // x4 has the d3 value


    //    d0 = dp0 + dq0@
    //    d3 = dp3 + dq3@

    add         x14,x8,x11                  // x13 has the value dp
    add         x12,x12,x9                  // x12 has the value  dq
    //    dp = dp0 + dp3@
    //   dq = dq0 + dq3@

    add         x11, x3, x4                 // x3 has the value d

    //   d = d0 + d3@


    cmp         x11,x5
    bge         l1.2404

    //    if(d < beta)


    // registers which cannont be altered : x3,x4 x5,x6,x12,x13,x0,x1,x11

    // registers for use: x2,x7,x8,x9,x10,

    asr         x10,x5,#2
    uqadd       v30.8b,  v26.8b ,  v1.8b
    cmp         x10,x3,lsl #1
    uqsub       v31.8b,  v26.8b ,  v1.8b
    ble         l1.1840
    add         x10,x1,x1,lsl #1
    uaddl       v6.8h,  v25.8b ,  v26.8b
    neg         x19,x1
    ldr         w2, [x0,x19,lsl #2]         // has the -4 value
    neg         x19, x1
    ldrb        w7,[x0,x19]                 // has the -1 value
    dup         v22.2s,w2                   // -4 value
    uaddw       v7.8h,  v6.8h ,  v27.8b
    ldrb        w3,[x0,#0]                  // x4 has the 0 value
    uqadd       v16.8b,  v27.8b ,  v1.8b
    and         x2,x2,#0xff
    mul         v12.8h, v7.8h, v0.h[0]
    ldr         w8, [x0,x10]                // has the 3 value
    uaddl       v10.8h,  v24.8b ,  v28.8b
    subs        x2,x2,x7
    uqsub       v17.8b,  v27.8b ,  v1.8b
    dup         v29.2s,w8                   // 3 value
    and         x8,x8,#0xff
    add         v12.8h,  v12.8h ,  v10.8h
    csneg       x2,x2,x2,pl
    rshrn       v20.8b, v12.8h,#3
    subs        x8,x8,x3
    csneg       x8,x8,x8,pl
    umin        v18.8b,  v20.8b ,  v30.8b
    add         x8,x8,x2

    cmp         x8,x5,asr #3
    bge         l1.1840
    uaddw       v14.8h,  v7.8h ,  v28.8b
    subs        x7,x3,x7
    umax        v4.8b,  v18.8b ,  v31.8b
    csneg       x7,x7,x7,pl
    uqadd       v30.8b,  v28.8b ,  v1.8b
    mov         x10,#5
    rshrn       v21.8b, v14.8h,#2
    mul         x10, x10, x6
    uqsub       v31.8b,  v28.8b ,  v1.8b
    add         x10, x10,#1
    cmp         x7,x10,asr #1
    umin        v18.8b,  v21.8b ,  v16.8b
    bge         l1.1840


    //        if( (2 * d3 < (beta >> 2)&& ( abs(pu1_src[3] - pu1_src[0]) + abs(pu1_src[-1] - pu1_src[-4])  < (beta >> 3) )
    //            && abs(pu1_src[0] - pu1_src[-1]) < ( (5 * tc + 1) >> 1 ) )

    umax        v5.8b,  v18.8b ,  v17.8b
    asr         x10,x5,#2
    uaddl       v16.8h,  v29.8b ,  v28.8b
    cmp         x10,x4,lsl #1
    ble         l1.1840

    add         x10,x1,x1,lsl #1
    mul         v16.8h, v16.8h, v0.h[0]
    add         x4,x0,#3


    lsl         x19,x1,#2
    neg         x19,x19
    ldrb        w2,[x4,x19]
    add         v16.8h,  v16.8h ,  v14.8h
    neg         x19,x1
    ldrb        w7,[x4,x19]
    rshrn       v19.8b, v16.8h,#3
    ldrb        w3,[x4,#0]
    ldrb        w8,[x4,x10]
    //   ubfx   x7,x2,#24,#8           @ has the -1 value
    //  and    x2,#0xff               @ has the -4 value
    //  ubfx   x8,x3,#24,#8           @ has the 3 value
    //  and    x3,#0xff               @ x4 has the 0 value



    subs        x8,x8,x3
    umin        v18.8b,  v19.8b ,  v30.8b
    csneg       x8,x8,x8,pl
    uaddl       v6.8h,  v25.8b ,  v24.8b
    subs        x2,x2,x7
    umax        v3.8b,  v18.8b ,  v31.8b
    csneg       x2,x2,x2,pl
    uaddw       v7.8h,  v6.8h ,  v26.8b
    add         x8,x8,x2
    uqadd       v30.8b,  v25.8b ,  v1.8b
    cmp         x8,x5,asr #3
    uqsub       v31.8b,  v25.8b ,  v1.8b
    bge         l1.1840
    mul         v12.8h, v7.8h, v0.h[0]
    subs        x7,x3,x7
    uqadd       v16.8b,  v24.8b ,  v1.8b
    csneg       x7,x7,x7,pl
    uaddl       v10.8h,  v23.8b ,  v27.8b
    mov         x10,#5
    uqsub       v17.8b,  v24.8b ,  v1.8b
    mul         x10, x10, x6
    add         v12.8h,  v12.8h ,  v10.8h
    add         x10, x10,#1
    rshrn       v20.8b, v12.8h,#3
    cmp         x7,x10,asr #1
    uaddw       v14.8h,  v7.8h ,  v23.8b
    bge         l1.1840
    umin        v18.8b,  v20.8b ,  v30.8b
    mov         x2,#2
    uqadd       v30.8b,  v23.8b ,  v1.8b
    mov         w4,w21
    umax        v2.8b,  v18.8b ,  v31.8b
    mov         w5,w22
    rshrn       v21.8b, v14.8h,#2
    b           end_dep_deq_decision_horz
    // x2 has the value of de
    // x6 has teh value of tc
    // x5 has the value of beta
    // x14 has the value of dp
    // x12 has the value of dq
    // x0 has the value of source address
    // x1 has the src stride

l1.1840:
    mov         x2,#1

    mov         x11,x5
    mov         w4,w21
    mov         w5,w22

    cmp         x6,#1
    mov         x20,#0
    csel        x9, x20, x9,eq
    mov         x20,#0
    csel        x10, x20, x10,eq
    beq         end_dep_deq_decision_horz

    and         x7,x4,x5
    cmp         x7,#1
    beq         both_flags_set_horz
    cmp         x4,#0
    beq         set_flag_dep_zero_horz


    add         x8,x11,x11,asr #1
    mov         x10,#0
    asr         x8,x8,#3
    cmp         x8,x14
    mov         x20,#1
    csel        x9, x20, x9,gt
    mov         x20,#0
    csel        x9, x20, x9,le
    b           end_dep_deq_decision_horz
set_flag_dep_zero_horz:

    add         x8,x11,x11,asr #1
    mov         x9,#0
    asr         x8,x8,#3
    cmp         x8,x12
    mov         x20,#1
    csel        x10, x20, x10,gt
    mov         x20,#0
    csel        x10, x20, x10,le
    b           end_dep_deq_decision_horz

both_flags_set_horz:
    add         x8,x11,x11,asr #1
    asr         x8,x8,#3
    cmp         x8,x14
    mov         x20,#1
    csel        x9, x20, x9,gt
    mov         x20,#0
    csel        x9, x20, x9,le
    cmp         x8,x12
    mov         x20,#1
    csel        x10, x20, x10,gt
    mov         x20,#0
    csel        x10, x20, x10,le
end_dep_deq_decision_horz:

    //x0=source address
    //x1=stride
    // x2 =de
    // x4=flag p
    //x5= flag q
    //x6 =tc
    // x9 =dep
    // x10=deq



    //    add        x14,x1,x1,lsl #1
    //    lsl        x7,x6,#1
    //    vdup.8    d1,x7
    //    vmov.i16  d0,#0x2
    umin        v18.8b,  v21.8b ,  v16.8b
    cmp         x2,#1
    uqsub       v31.8b,  v23.8b ,  v1.8b
    beq         l1.2408
    uaddl       v7.8h,  v23.8b ,  v22.8b
    cmp         x5,#1

    bne         strong_filtering_p

strong_filtering_q:
    mov         x12,x0
    st1         {v4.s}[0],[x12],x1
    st1         {v5.s}[0],[x12],x1
    st1         {v3.s}[0],[x12]
    cmp         x4,#1
    bne         l1.2404
strong_filtering_p:
    umax        v5.8b,  v18.8b ,  v17.8b
    mov         x12,x0
    mul         v7.8h, v7.8h, v0.h[0]
    sub         x20,x1,#0
    neg         x11, x20
    add         v16.8h,  v7.8h ,  v14.8h
    add         x12,x12,x11
    rshrn       v19.8b, v16.8h,#3
    st1         {v2.s}[0],[x12],x11
    umin        v18.8b,  v19.8b ,  v30.8b
    st1         {v5.s}[0],[x12],x11
    umax        v3.8b,  v18.8b ,  v31.8b
    st1         {v3.s}[0],[x12]

l1.2404:
    // ldmfd sp!, {x3-x12,pc}
    ldp         x21, x22,[sp],#16
    ldp         x19, x20,[sp],#16
    ldp         d14,d15,[sp],#16
    ldp         d12,d13,[sp],#16
    ldp         d10,d11,[sp],#16
    ldp         d8,d9,[sp],#16              // Loading d9 using { ldr d9,[sp]; add sp,sp,#8 } is giving bus error.
                                            // d8 is used as dummy register and loaded along with d9 using ldp. d8 is not used in the function.
    ret

    // x4=flag p
    //x5= flag q
    //x6 =tc
    // x9 =dep
    // x10=deq


    //        d22             -4 value

    //d23        @ -3 value

    //    vdup.32    d24,x11            @ -2 value

    //    vdup.32    d25, x11        @-1 value

    //    vdup.32    d26,x11            @ 0 value

    //    vdup.32    d27,x11            @ 1value

    //    vdup.32    d28,x11            @ 2 value

    //    vdup.32    d29,x11            @ 3 value

l1.2408:

    movi        v0.4h, #0x9

    usubl       v10.8h,  v26.8b ,  v25.8b

    mul         v10.8h, v10.8h, v0.h[0]

    movi        v0.4h, #0x3

    usubl       v12.8h,  v27.8b ,  v24.8b
    mul         v12.8h, v12.8h, v0.h[0]


    dup         v30.8b,w6                   // duplicating the +tc value

    sub         x20,x6,#0
    neg         x12, x20
    dup         v31.8b,w12                  // duplicating the -tc value



    sub         v10.8h,  v10.8h ,  v12.8h



    srshr       v10.8h, v10.8h,#4
    //   delta = ( 9 * (pu1_src[0] - pu1_src[-1]) - 3 * (pu1_src[1] - pu1_src[-2]) + 8 ) >> 4@

    abs         v7.8h, v10.8h
    xtn         v9.8b,  v7.8h
    // storing the absolute values of delta in d9

    sqxtn       v10.8b,  v10.8h
    // storing the clipped values of delta in d16


    smin        v11.8b,  v10.8b ,  v30.8b
    smax        v7.8b,  v31.8b ,  v11.8b    // d8 has the value  delta = clip3(delta, -tc, tc)//


    uxtl        v6.8h, v25.8b

    saddw       v4.8h,  v6.8h ,  v7.8b

    sqxtun      v12.8b, v4.8h
    uxtl        v6.8h, v26.8b
    ssubw       v4.8h,  v6.8h ,  v7.8b
    sqxtun      v13.8b, v4.8h


    mov         x11,#0xa
    mul         x12, x11, x6
    dup         v2.8b,w12                   // d2 has the 10*tc value
    mov         v18.8b, v24.8b
    dup         v0.8b,w6
    sshr        v0.8b,v0.8b,#1
    neg         v1.8b, v0.8b

    cmp         x4,#1
    bne         l1.2724
    cmp         x9,#1
    bne         l1.2700

    // d12 and d13 have the value temp_p0 and temp_q0
    uaddl       v14.8h,  v23.8b ,  v25.8b
    rshrn       v14.8b, v14.8h,#1
    usubl       v14.8h,  v14.8b ,  v24.8b
    saddw       v14.8h,  v14.8h ,  v7.8b
    sqshrn      v14.8b, v14.8h,#1
    smin        v15.8b,  v14.8b ,  v0.8b
    smax        v14.8b,  v1.8b ,  v15.8b

    // d14 has the delta p value
    uxtl        v16.8h, v24.8b
    saddw       v16.8h,  v16.8h ,  v14.8b
    sqxtun      v14.8b, v16.8h

    //  d14 =tmp_p1 = clip_u8(pu1_src[-2 * src_strd] + delta_p)@
    cmhs        v18.8b,v9.8b,v2.8b
    bsl         v18.8b,v24.8b,v14.8b

l1.2700:
    mov         x12,x0
    sub         x20,x1,#0
    neg         x11, x20
    add         x12,x12,x11
    cmhs        v19.8b,v9.8b,v2.8b
    bsl         v19.8b,v25.8b,v12.8b
    st1         {v19.s}[0],[x12],x11
    st1         {v18.s}[0],[x12]
l1.2724:
    cmp         x5,#1
    bne         l1.2404
    cmp         x10,#1
    mov         v18.8b, v27.8b
    bne         l1.2852

    uaddl       v14.8h,  v26.8b ,  v28.8b
    rshrn       v14.8b, v14.8h,#1
    usubl       v14.8h,  v14.8b ,  v27.8b
    ssubw       v14.8h,  v14.8h ,  v7.8b
    sqshrn      v14.8b, v14.8h,#1
    smin        v15.8b,  v14.8b ,  v0.8b
    smax        v14.8b,  v1.8b ,  v15.8b
// d14 has the delta p value
    uxtl        v16.8h, v27.8b
    saddw       v16.8h,  v16.8h ,  v14.8b
    sqxtun      v14.8b, v16.8h
    cmhs        v18.8b,v9.8b,v2.8b
    bsl         v18.8b,v27.8b,v14.8b
l1.2852:
    mov         x12,x0
    cmhs        v19.8b,v9.8b,v2.8b
    bsl         v19.8b,v26.8b,v13.8b
    st1         {v19.s}[0],[x12],x1
    st1         {v18.s}[0],[x12]
    // ldmfd sp!, {x3-x12,x15}
    ldp         x21, x22,[sp],#16
    ldp         x19, x20,[sp],#16
    ldp         d14,d15,[sp],#16
    ldp         d12,d13,[sp],#16
    ldp         d10,d11,[sp],#16
    ldp         d8,d9,[sp],#16              // Loading d9 using { ldr d9,[sp]; add sp,sp,#8 } is giving bus error.
                                            // d8 is used as dummy register and loaded along with d9 using ldp. d8 is not used in the function.
    ret


