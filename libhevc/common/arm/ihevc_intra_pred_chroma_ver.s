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
@*******************************************************************************
@* @file
@*  ihevc_intra_pred_chroma_ver_neon.s
@*
@* @brief
@*  contains function definitions for intra prediction dc filtering.
@* functions are coded using neon  intrinsics and can be compiled using

@* rvct
@*
@* @author
@*  yogeswaran rs
@*
@* @par list of functions:
@*
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/
@/**
@*******************************************************************************
@*
@* @brief
@*    luma intraprediction filter for dc input
@*
@* @par description:
@*
@* @param[in] pu1_ref
@*  uword8 pointer to the source
@*
@* @param[out] pu1_dst
@*  uword8 pointer to the destination
@*
@* @param[in] src_strd
@*  integer source stride
@*
@* @param[in] dst_strd
@*  integer destination stride
@*
@* @param[in] nt
@*  size of tranform block
@*
@* @param[in] mode
@*  type of filtering
@*
@* @returns
@*
@* @remarks
@*  none
@*
@*******************************************************************************
@*/

@void ihevc_intra_pred_chroma_ver(uword8 *pu1_ref,
@        word32 src_strd,
@        uword8 *pu1_dst,
@        word32 dst_strd,
@        word32 nt,
@        word32 mode)
@**************variables vs registers*****************************************
@r0 => *pu1_ref
@r1 => src_strd
@r2 => *pu1_dst
@r3 => dst_strd

@stack contents from #40
@   nt
@   mode

.equ    nt_offset,      40

.text
.align 4




.globl ihevc_intra_pred_chroma_ver_a9q

.type ihevc_intra_pred_chroma_ver_a9q, %function

ihevc_intra_pred_chroma_ver_a9q:

    stmfd       sp!, {r4-r12, r14}          @stack stores the values of the arguments

    ldr         r4,[sp,#nt_offset]          @loads nt
    lsl         r5, r4, #2                  @4nt


    cmp         r4, #8
    beq         blk_8
    blt         blk_4

copy_16:
    add         r5, r5, #2                  @2nt+2
    add         r6, r0, r5                  @&src[2nt+1]

    add         r5, r2, r3                  @pu1_dst + dst_strd
    vld2.8      {d20,d21}, [r6]!            @16 loads (col 0:15)
    add         r8, r5, r3

    add         r10, r8, r3
    vld2.8      {d22,d23}, [r6]             @16 loads (col 16:31)
    lsl         r11, r3, #2

    add         r11, r11, #0xfffffff0


    vst2.8      {d20,d21}, [r2]!
    vst2.8      {d20,d21}, [r5]!
    vst2.8      {d20,d21}, [r8]!
    vst2.8      {d20,d21}, [r10]!

    vst2.8      {d22,d23}, [r2], r11
    vst2.8      {d22,d23}, [r5], r11
    vst2.8      {d22,d23}, [r8], r11
    vst2.8      {d22,d23}, [r10], r11

    subs        r4, r4, #4

kernel_copy_16:
    vst2.8      {d20,d21}, [r2]!
    vst2.8      {d20,d21}, [r5]!
    vst2.8      {d20,d21}, [r8]!
    vst2.8      {d20,d21}, [r10]!

    vst2.8      {d22,d23}, [r2], r11
    vst2.8      {d22,d23}, [r5], r11
    vst2.8      {d22,d23}, [r8], r11
    vst2.8      {d22,d23}, [r10], r11

    subs        r4, r4, #4


    vst2.8      {d20,d21}, [r2]!
    vst2.8      {d20,d21}, [r5]!
    vst2.8      {d20,d21}, [r8]!
    vst2.8      {d20,d21}, [r10]!

    vst2.8      {d22,d23}, [r2], r11
    vst2.8      {d22,d23}, [r5], r11
    vst2.8      {d22,d23}, [r8], r11
    vst2.8      {d22,d23}, [r10], r11

    subs        r4, r4, #4

    vst2.8      {d20,d21}, [r2]!
    vst2.8      {d20,d21}, [r5]!
    vst2.8      {d20,d21}, [r8]!
    vst2.8      {d20,d21}, [r10]!

    vst2.8      {d22,d23}, [r2], r11
    vst2.8      {d22,d23}, [r5], r11
    vst2.8      {d22,d23}, [r8], r11
    vst2.8      {d22,d23}, [r10], r11

    subs        r4, r4, #4
    bne         kernel_copy_16

    b           end_func

blk_8:

    add         r5, r5, #2                  @2nt+2
    add         r6, r0, r5                  @&src[2nt+1]

    add         r5, r2, r3                  @pu1_dst + dst_strd
    vld2.8      {d20,d21}, [r6]!            @16 loads (col 0:15)
    add         r8, r5, r3

    add         r10, r8, r3
    vld2.8      {d22,d23}, [r6]             @16 loads (col 16:31)

    lsl         r11,r3,#2

    vst2.8      {d20,d21}, [r2],r11
    vst2.8      {d20,d21}, [r5],r11
    vst2.8      {d20,d21}, [r8],r11
    vst2.8      {d20,d21}, [r10],r11

    vst2.8      {d20,d21}, [r2]
    vst2.8      {d20,d21}, [r5]
    vst2.8      {d20,d21}, [r8]
    vst2.8      {d20,d21}, [r10]

    subs        r4, r4, #8
    beq         end_func

blk_4:

    @lsl        r5, r4, #2          @4nt
    add         r5, r5, #2                  @2nt+2
    add         r6, r0, r5                  @&src[2nt+1]

    vld1.8      {d0},[r6]
    add         r5, r2, r3                  @pu1_dst + dst_strd

    vst1.8      {d0},[r2]
    add         r8, r5, r3
    vst1.8      {d0},[r5]
    add         r10, r8, r3
    vst1.8      {d0},[r8]
    vst1.8      {d0},[r10]



end_func:
    ldmfd       sp!,{r4-r12,r15}            @reload the registers from sp



