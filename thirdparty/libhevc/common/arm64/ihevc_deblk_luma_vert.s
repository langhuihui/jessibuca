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
///*******************************************************************************
//* //file
//*  ihevc_deblk_luma_vert.s
//*
//* //brief
//*  contains function definitions for inter prediction  interpolation.
//* functions are coded using neon  intrinsics and can be compiled using

//* rvct
//*
//* //author
//*  anand s
//*
//* //par list of functions:
//*
//*
//* //remarks
//*  none
//*
//*******************************************************************************/

.text
.align 4



.extern gai4_ihevc_tc_table
.extern gai4_ihevc_beta_table

.globl ihevc_deblk_luma_vert_av8

.type ihevc_deblk_luma_vert_av8, %function

ihevc_deblk_luma_vert_av8:

    sxtw        x5,w5
    sxtw        x6,w6
    stp         d8,d9,[sp,#-16]!
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
    bgt         l1.56
    cmp         x7,#0x0
    mov         x20,#0x0
    csel        x7, x20, x7,lt              // x7 has the beta_index value
l1.56:

//     bic      x2,x2,#1
    asr         x2,x2,#1

    add         x3,x3,x2,lsl #1
    cmp         x3,#0x35
    mov         x20,#0x35
    csel        x3, x20, x3,gt
    bgt         l1.88
    cmp         x3,#0x0
    mov         x20,#0x0
    csel        x3, x20, x3,lt              // x3 has the tc_index value

//    qp_luma = (quant_param_p + quant_param_q + 1) >> 1//
//    beta_indx = clip3(qp_luma + (beta_offset_div2 << 1), 0, 51)//
//    tc_indx = clip3(qp_luma + (2 * (bs >> 1)) + (tc_offset_div2 << 1), 0, 53)//

l1.88:
    adrp        x2, :got:gai4_ihevc_beta_table
    ldr         x2, [x2, #:got_lo12:gai4_ihevc_beta_table]

    movi        v18.8b, #0x2
    adrp        x4, :got:gai4_ihevc_tc_table
    ldr         x4, [x4, #:got_lo12:gai4_ihevc_tc_table]

    ldr         w5,[x2,x7,lsl #2]           // beta
    movi        v16.8h, #0x2
    ldr         w6,[x4,x3,lsl #2]           // tc
    lsl         x8,x6,#1
    cmp         x6,#0
    dup         v19.8b,w8
    sub         x7,x0,#4
    movi        v23.8b, #0x3
    beq         l1.964


    sub         x19,x0,#3
    ld1         {v15.8b},[x7],x1
    ldrb        w8,[x19]                    // -3 value
    ld1         {v1.8b},[x7],x1
    ldrb        w10,[x19,#1]                //-2 value
    ld1         {v29.8b},[x7],x1
    ldrb        w11,[x19,#2]                //-1 value
    ld1         {v0.8b},[x7]
    ldrb        w12,[x0,#0]                 // 0 value
    ldrb        w9,[x0,#1]                  // 1 value
    trn1        v24.8b,v15.8b,v1.8b
    trn2        v1.8b,v15.8b,v1.8b
    ldrb        w2,[x0,#2]                  // 2 value
    trn1        v2.8b,v29.8b,v0.8b
    trn2        v0.8b,v29.8b,v0.8b
    add         x12,x12,x2
    subs        x9,x12,x9,lsl #1            // dq0 value is stored in x9
    csneg       x9,x9,x9,pl
//dq0 = abs( pu1_src[2] - 2 * pu1_src[1] + pu1_src[0] )//
    mov         v29.8b,v24.8b
    trn1        v24.4h,v29.4h,v2.4h
    trn2        v2.4h,v29.4h,v2.4h
    add         x8,x8,x11
    mov         v15.8b,v1.8b
    trn1        v1.4h,v15.4h,v0.4h
    trn2        v0.4h,v15.4h,v0.4h
    subs        x8,x8,x10,lsl #1
    csneg       x8,x8,x8,pl
//  dp0 = abs( pu1_src[-3] - 2 * pu1_src[-2] + pu1_src[-1] )//



    add         x14,x1,x1,lsl #1
    add         x14,x0,x14

    sub         x19,x14,#3
    dup         v4.2s, v24.s[1]
    ldrb        w2,[x19]                    // -2 value
    dup         v7.2s, v2.s[1]
    ldrb        w10,[x19,#1]                // -2 value
    dup         v3.2s, v2.s[0]
    ldrb        w11,[x19,#2]                // -1 value
    dup         v5.2s, v1.s[1]
    ldrb        w12,[x14,#0]                // 0 value
    dup         v6.2s, v1.s[0]
    ldrb        w3,[x14,#1]                 // 1 value
    dup         v2.2s, v0.s[0]
    ldrb        w4,[x14,#2]                 // 2 value


    add         x12,x12,x4
    subs        x12,x12,x3,lsl #1           // dq3value is stored in x12
    csneg       x12,x12,x12,pl
//    dq3 = abs( pu1_src[3 * src_strd + 2] - 2 * pu1_src[3 * src_strd + 1]+ pu1_src[3 * src_strd + 0] )//


    add         x2,x2,x11
    subs        x11,x2,x10,lsl #1
    csneg       x11,x11,x11,pl              // dp3 value is stored in x8
//    dp3 = abs( pu1_src[3 * src_strd - 3] - 2 * pu1_src[3 * src_strd - 2]   + pu1_src[3 * src_strd - 1] )//



    add         x3,x8,x9                    // x3 has the d0 value
    add         x4,x11,x12                  // x4 has the d3 value


//    d0 = dp0 + dq0//
//    d3 = dp3 + dq3//

    add         x14,x8,x11                  // x13 has the value dp
    add         x12,x12,x9                  // x12 has the value  dq
//    dp = dp0 + dp3//
//   dq = dq0 + dq3//

    add         x11, x3, x4                 // x3 has the value d

//   d = d0 + d3//


    cmp         x11,x5
    dup         v22.2s, v0.s[1]
    bge         l1.964

//    if(d < beta)


    // registers which cannont be altered : x3,x4 x5,x6,x12,x13,x0,x1,x11

    // registers for use: x2,x7,x8,x9,x10,
    uqsub       v30.8b,v7.8b,v19.8b
    asr         x10,x5,#2
    uqadd       v31.8b,v7.8b,v19.8b
    cmp         x10,x3,lsl #1
    uaddl       v0.8h,v5.8b,v4.8b
    ble         l1.336

    sub         x19,x0,4
    ldrb        w2,[x19]
    uaddw       v0.8h,  v0.8h ,  v2.8b
    ldrb        w7,[x19,#3]
    umull       v20.8h, v7.8b, v23.8b
    ldrb        w3,[x0,#0]
    umlal       v20.8h, v22.8b, v18.8b
    ldrb        w8,[x0,#3]
//   ubfx   x7,x2,#24,#8           // has the -1 value
//  and    x2,#0xff               // has the -4 value
//  ubfx   x8,x3,#24,#8           // has the 3 value
//  and    x3,#0xff               // x4 has the 0 value

    add         v20.8h,  v20.8h ,  v0.8h
    subs        x8,x8,x3
    rshrn       v22.8b,v20.8h,#3
    csneg       x8,x8,x8,pl
    subs        x2,x2,x7
    umin        v21.8b,  v22.8b ,  v31.8b
    csneg       x2,x2,x2,pl
    umax        v22.8b,  v21.8b ,  v30.8b
    add         x8,x8,x2
    uaddl       v20.8h,v7.8b,v3.8b
    cmp         x8,x5,asr #3
    mla         v20.8h, v0.8h, v16.8h
    bge         l1.336
    uaddw       v0.8h,  v0.8h ,  v7.8b
    subs        x7,x3,x7
    rshrn       v20.8b,v20.8h,#3
    csneg       x7,x7,x7,pl
    rshrn       v0.8b,v0.8h,#2
    mov         x10,#5
    uqadd       v30.8b,v5.8b,v19.8b
    mul         x10, x10, x6
    uqsub       v31.8b,v5.8b,v19.8b
    add         x10, x10,#1
    cmp         x7,x10,asr #1
    bge         l1.336


//        if( (2 * d3 < (beta >> 2)&& ( abs(pu1_src[3] - pu1_src[0]) + abs(pu1_src[-1] - pu1_src[-4])  < (beta >> 3) )
//            && abs(pu1_src[0] - pu1_src[-1]) < ( (5 * tc + 1) >> 1 ) )


    asr         x10,x5,#2
    uqsub       v25.8b,v4.8b,v19.8b
    cmp         x10,x4,lsl #1
    uqadd       v21.8b,v4.8b,v19.8b
    ble         l1.336
    umin        v26.8b,  v20.8b ,  v21.8b
    add         x4,x1,x1,lsl #1
    add         x4,x4,x0
    umax        v20.8b,  v26.8b ,  v25.8b
    sub         x19,x4,#4
    ldrb        w2,[x19]
    umin        v19.8b,  v0.8b ,  v30.8b
    ldrb        w7,[x19,#3]
    umax        v21.8b,  v19.8b ,  v31.8b
    ldrb        w3,[x4,#0]
    lsl         x10,x6,#1
    ldrb        w8,[x4,#3]
//   ubfx   x7,x2,#24,#8           // has the -1 value
//  and    x2,#0xff               // has the -4 value
//  ubfx   x8,x3,#24,#8           // has the 3 value
//  and    x3,#0xff               // x4 has the 0 value
    uaddl       v0.8h,v2.8b,v3.8b
    dup         v19.8b,w10
    subs        x8,x8,x3
    uaddw       v0.8h,  v0.8h ,  v4.8b
    csneg       x8,x8,x8,pl
    uqadd       v30.8b,v2.8b,v19.8b
    subs        x2,x2,x7
    uqsub       v31.8b,v2.8b,v19.8b
    csneg       x2,x2,x2,pl
    uaddl       v26.8h,v5.8b,v6.8b
    add         x8,x8,x2
    mla         v26.8h, v0.8h, v16.8h
    cmp         x8,x5,asr #3
    bge         l1.336
    rshrn       v26.8b,v26.8h,#3
    subs        x7,x3,x7
    uqadd       v27.8b,v3.8b,v19.8b
    csneg       x7,x7,x7,pl
    uqsub       v28.8b,v3.8b,v19.8b
    mov         x10,#5
    umin        v16.8b,  v26.8b ,  v30.8b
    mul         x10, x10, x6
    add         x10, x10,#1
    cmp         x7,x10,asr #1
    umax        v26.8b,  v16.8b ,  v31.8b
    bge         l1.336
    uqadd       v30.8b,v6.8b,v19.8b

    mov         x2,#2
    mov         x4,x21
    uqsub       v31.8b,v6.8b,v19.8b
    mov         x5,x22
    b           end_dep_deq_decision
// x2 has the value of de
// x6 has teh value of tc
// x5 has the value of beta
// x14 has the value of dp
// x12 has the value of dq
// x0 has the value of source address
// x1 has the src stride

l1.336:
    mov         x2,#1
l1.424:
    mov         x11,x5
    mov         x4,x21
    mov         x5,x22

    cmp         x6,#1
    mov         x20,#0
    csel        x9, x20, x9,eq
    mov         x20,#0
    csel        x10, x20, x10,eq
    beq         end_dep_deq_decision

    and         x7,x4,x5

    cmp         x7,#1
    beq         both_flags_set
    cmp         x4,#0
    beq         set_flag_dep_zero


    add         x8,x11,x11,asr #1
    mov         x10,#0
    asr         x8,x8,#3
    cmp         x8,x14
    mov         x20,#1
    csel        x9, x20, x9,gt
    mov         x20,#0
    csel        x9, x20, x9,le
    b           end_dep_deq_decision
set_flag_dep_zero:

    add         x8,x11,x11,asr #1
    mov         x9,#0
    asr         x8,x8,#3
    cmp         x8,x12
    mov         x20,#1
    csel        x10, x20, x10,gt
    mov         x20,#0
    csel        x10, x20, x10,le
    b           end_dep_deq_decision

both_flags_set:
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
end_dep_deq_decision:

//x0=source address
//x1=stride
// x2 =de
// x4=flag p
//x5= flag q
//x6 =tc
// x9 =dep
// x10=deq
//    b    l1.964


    cmp         x2,#2
// x4 has the value of de
    bne         l1.968

    cmp         x5,#0
    beq         l1.780
// x5 has the flag of q

    add         x3,x0,#2
    st1         {v22.b}[0],[x3],x1

    st1         {v22.b}[1],[x3],x1

    st1         {v22.b}[2],[x3],x1

    st1         {v22.b}[3],[x3]
    add         x3,x0,x1
    mov         v29.8b,v20.8b
    trn1        v20.8b,v29.8b,v21.8b
    trn2        v21.8b,v29.8b,v21.8b

    st1         {v20.h}[0],[x0]
    st1         {v21.h}[0],[x3],x1
    st1         {v20.h}[1],[x3],x1
    st1         {v21.h}[1],[x3]


l1.780:
    cmp         x4,#0
    beq         l1.964
    // x4 has the flag p


    dup         v7.2s, v24.s[0]
    sub         x3,x0,#1
    uaddw       v16.8h,  v0.8h ,  v6.8b
    add         x7,x3,x1
    rshrn       v2.8b,v16.8h,#2
    st1         {v26.b}[0],[x3]
    sub         x0,x0,#3
    umin        v16.8b,  v2.8b ,  v27.8b
    st1         {v26.b}[1],[x7],x1
    umull       v2.8h, v6.8b, v23.8b
    umlal       v2.8h, v7.8b, v18.8b
    st1         {v26.b}[2],[x7],x1
    umax        v5.8b,  v16.8b ,  v28.8b
    st1         {v26.b}[3],[x7]
    add         v0.8h,  v2.8h ,  v0.8h
    rshrn       v0.8b,v0.8h,#3


    umin        v1.8b,  v0.8b ,  v30.8b
    umax        v0.8b,  v1.8b ,  v31.8b

    mov         v29.8b,v0.8b
    trn1        v0.8b,v29.8b,v5.8b
    trn2        v5.8b,v29.8b,v5.8b
    st1         {v0.h}[0],[x0],x1
    st1         {v5.h}[0],[x0],x1
    st1         {v0.h}[1],[x0],x1
    st1         {v5.h}[1],[x0]
l1.964:
    ldp         x21, x22,[sp],#16
    ldp         x19, x20,[sp],#16
    ldp         d14,d15,[sp],#16
    ldp         d12,d13,[sp],#16
    ldp         d10,d11,[sp],#16
    ldp         d8,d9,[sp],#16
    ret

l1.968:


    movi        v0.8h, #0x9
    neg         x11, x6
    cmp         x4,#0
    // checks for the flag p
    movi        v16.8h, #0x3
    movi        v24.8b, #0x1


    dup         v30.8b,w11
    and         x11,x6,#0xff
    dup         v31.8b,w11

    usubl       v18.8h,v4.8b,v2.8b
    mul         v18.8h, v18.8h, v0.8h
    usubl       v0.8h,v5.8b,v3.8b



    mul         v16.8h, v0.8h, v16.8h
    sub         v16.8h,  v18.8h ,  v16.8h
    srshr       v16.8h,v16.8h,#4
//   delta = ( 9 * (pu1_src[0] - pu1_src[-1]) - 3 * (pu1_src[1] - pu1_src[-2]) + 8 ) >> 4//

    abs         v0.8h, v16.8h
    xtn         v0.8b,  v0.8h
    // storing the absolute values of delta in d0

    sqxtn       v16.8b,v16.8h
    // storing the clipped values of delta in d16

    movi        v1.8b, #0xa
    dup         v21.8b,w11
    mul         v1.8b, v1.8b, v21.8b
    // d1 stores the value (10 * tc)

//if(abs(delta) < 10 * tc)

    smin        v18.8b,  v16.8b ,  v31.8b
    smax        v20.8b,  v18.8b ,  v30.8b

// delta = clip3(delta, -tc, tc)//
    sxtl        v16.8h, v20.8b
    uxtl        v18.8h, v2.8b
    add         v18.8h,  v18.8h ,  v16.8h

    sqxtun      v22.8b, v18.8h
    uxtl        v18.8h, v4.8b
    sub         v16.8h,  v18.8h ,  v16.8h
    sqxtun      v23.8b, v16.8h
// tmp_p0 = clip_u8(pu1_src[-1] + delta)//
//  tmp_q0 = clip_u8(pu1_src[0] - delta)//
    beq         l1.1272



    cmp         x9,#1
    bne         l1.1212
// checks for the flag dep

    asr         x3,x6,#1


    uaddl       v16.8h,v6.8b,v2.8b
    uaddw       v16.8h,  v16.8h ,  v24.8b
    dup         v18.8b,w3
    sub         x20,x3,#0
    neg         x3, x20
    dup         v19.8b,w3
    ushr        v16.8h,v16.8h,#1
    xtn         v16.8b,  v16.8h

    usubl       v16.8h,v16.8b,v3.8b
    saddw       v16.8h,  v16.8h ,  v20.8b
    sshr        v16.8h,v16.8h,#1
    sqxtn       v16.8b,v16.8h

    smin        v17.8b,  v16.8b ,  v18.8b
    smax        v16.8b,  v19.8b ,  v17.8b




    uxtl        v18.8h, v3.8b
    sxtl        v16.8h, v16.8b
    add         v16.8h,  v18.8h ,  v16.8h

    sqxtun      v16.8b, v16.8h
    mov         v30.8b,v3.8b
    cmhs        v3.8b,v0.8b,v1.8b


    bsl         v3.8b,v30.8b,v16.8b
l1.1212:
    dup         v16.8b,w11
    sub         x12,x0,#3
    sub         x3,x0,#1
//     smul v16.8b, v16.8b, v1.8b
    mov         v29.8b,v6.8b
    trn1        v6.8b,v29.8b,v3.8b
    trn2        v3.8b,v29.8b,v3.8b
    st1         {v6.h}[0],[x12],x1
    cmhs        v16.8b,v0.8b,v1.8b
    st1         {v3.h}[0],[x12],x1
    bsl         v16.8b,v2.8b,v22.8b
    st1         {v16.b}[0],[x3],x1
    st1         {v16.b}[1],[x3],x1
    st1         {v6.h}[1],[x12],x1
    st1         {v16.b}[2],[x3],x1
    st1         {v3.h}[1],[x12]
    st1         {v16.b}[3],[x3]
l1.1272:
    cmp         x5,#0
    beq         l1.964
    // checks for the flag q
    cmp         x10,#1
    bne         l1.1412
    // checks for the flag deq
    mov         v2.8b,v7.8b
    asr         x3,x6,#1

    dup         v6.8b,w3
    sub         x20,x3,#0
    neg         x3, x20
    dup         v16.8b,w3
    uaddl       v2.8h,v2.8b,v4.8b
    uaddw       v2.8h,  v2.8h ,  v24.8b
    ushr        v2.8h,v2.8h,#1
    xtn         v2.8b,  v2.8h

    usubl       v2.8h,v2.8b,v5.8b
    ssubw       v2.8h,  v2.8h ,  v20.8b
    sshr        v2.8h,v2.8h,#1
    sqxtn       v3.8b,v2.8h

    smin        v2.8b,  v3.8b ,  v6.8b
    smax        v3.8b,  v16.8b ,  v2.8b
    //  dup  v6.8b,w2
    //   smul v6.8b, v6.8b, v1.8b



    uxtl        v16.8h, v5.8b
    sxtl        v2.8h, v3.8b
    add         v2.8h,  v16.8h ,  v2.8h
    sqxtun      v3.8b, v2.8h
    mov         v30.8b,v5.8b
    cmhs        v5.8b,v0.8b,v1.8b


    bsl         v5.8b,v30.8b,v3.8b
l1.1412:
    //  dup  v2.8b,w2
    add         x3,x0,#2
    add         x11,x3,x1
    //   smul v1.8b, v2.8b, v1.8b
    st1         {v7.b}[0],[x3]
    st1         {v7.b}[1],[x11],x1
    st1         {v7.b}[2],[x11],x1
    cmhs        v0.8b,v0.8b,v1.8b
    st1         {v7.b}[3],[x11]
    bsl         v0.8b,v4.8b,v23.8b
    mov         v29.8b,v0.8b
    trn1        v0.8b,v29.8b,v5.8b
    trn2        v5.8b,v29.8b,v5.8b
    st1         {v0.h}[0],[x0],x1
    st1         {v5.h}[0],[x0],x1
    st1         {v0.h}[1],[x0],x1
    st1         {v5.h}[1],[x0]

    ldp         x21, x22,[sp],#16
    ldp         x19, x20,[sp],#16
    ldp         d14,d15,[sp],#16
    ldp         d12,d13,[sp],#16
    ldp         d10,d11,[sp],#16
    ldp         d8,d9,[sp],#16
    ret


