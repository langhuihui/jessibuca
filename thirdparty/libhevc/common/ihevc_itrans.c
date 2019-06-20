/******************************************************************************
*
* Copyright (C) 2012 Ittiam Systems Pvt Ltd, Bangalore
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at:
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************************/
/**
 *******************************************************************************
 * @file
 *  ihevc_itrans.c
 *
 * @brief
 *  Contains function definitions for single stage  inverse transform
 *
 * @author
 *  100470
 *
 * @par List of Functions:
 *  - ihevc_itrans_4x4_ttype1()
 *  - ihevc_itrans_4x4()
 *  - ihevc_itrans_8x8()
 *  - ihevc_itrans_16x16()
 *  - ihevc_itrans_32x32()
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include "ihevc_typedefs.h"
#include "ihevc_macros.h"
#include "ihevc_platform_macros.h"
#include "ihevc_defs.h"
#include "ihevc_trans_tables.h"
#include "ihevc_func_selector.h"
#include "ihevc_trans_macros.h"

#define NON_OPTIMIZED 1

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs Single stage  Inverse transform type 1 (DST) for
 * 4x4 input block
 *
 * @par Description:
 *  Performs single stage 4x4 inverse transform type 1  by utilizing the
 * symmetry of transformation matrix  and reducing number of multiplications
 * wherever  possible but keeping the number of operations
 * (addition,multiplication and shift)same
 *
 * @param[in] pi2_src
 *  Input 4x4 coefficients
 *
 * @param[out] pi2_dst
 *  Output 4x4 block
 *
 * @param[in] src_strd
 *  Input stride
 *
 * @param[in] dst_strd
 *  Output Stride
 *
 * @param[in] i4_shift
 *  Output shift
 *
 * @param[in] zero_cols
 *  Zero columns in pi2_src
 *
 * @returns  Void
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */


void ihevc_itrans_4x4_ttype1(WORD16 *pi2_src,
                             WORD16 *pi2_dst,
                             WORD32 src_strd,
                             WORD32 dst_strd,
                             WORD32 i4_shift,
                             WORD32 zero_cols)
{
    WORD32 i, c[4];
    WORD32 add;

    add = 1 << (i4_shift - 1);

    for(i = 0; i < TRANS_SIZE_4; i++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            memset(pi2_dst, 0, TRANS_SIZE_4 * sizeof(WORD16));
        }
        else
        {
            // Intermediate Variables
            c[0] = pi2_src[0] + pi2_src[2 * src_strd];
            c[1] = pi2_src[2 * src_strd] + pi2_src[3 * src_strd];
            c[2] = pi2_src[0] - pi2_src[3 * src_strd];
            c[3] = 74 * pi2_src[src_strd];

            pi2_dst[0] =
                            CLIP_S16((29 * c[0] + 55 * c[1] + c[3] + add) >> i4_shift);
            pi2_dst[1] =
                            CLIP_S16((55 * c[2] - 29 * c[1] + c[3] + add) >> i4_shift);
            pi2_dst[2] =
                            CLIP_S16((74 * (pi2_src[0] - pi2_src[2 * src_strd] + pi2_src[3 * src_strd]) + add) >> i4_shift);
            pi2_dst[3] =
                            CLIP_S16((55 * c[0] + 29 * c[2] - c[3] + add) >> i4_shift);
        }
        pi2_src++;
        pi2_dst += dst_strd;
        zero_cols = zero_cols >> 1;
    }
}


/**
 *******************************************************************************
 *
 * @brief
 *  This function performs Single stage  Inverse transform for 4x4 input
 * block
 *
 * @par Description:
 *  Performs single stage 4x4 inverse transform by utilizing  the symmetry of
 * transformation matrix and reducing number  of multiplications wherever
 * possible but keeping the  number of operations(addition,multiplication and
 * shift)  same
 *
 * @param[in] pi2_src
 *  Input 4x4 coefficients
 *
 * @param[out] pi2_dst
 *  Output 4x4 block
 *
 * @param[in] src_strd
 *  Input stride
 *
 * @param[in] dst_strd
 *  Output Stride
 *
 * @param[in] i4_shift
 *  Output shift
 *
 * @param[in] zero_cols
 *  Zero columns in pi2_src
 *
 * @returns  Void
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

#if NON_OPTIMIZED
void ihevc_itrans_4x4(WORD16 *pi2_src,
                      WORD16 *pi2_dst,
                      WORD32 src_strd,
                      WORD32 dst_strd,
                      WORD32 i4_shift,
                      WORD32 zero_cols)
{
    WORD32 j;
    WORD32 e[2], o[2];
    WORD32 add;

    add = 1 << (i4_shift - 1);

    for(j = 0; j < TRANS_SIZE_4; j++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            memset(pi2_dst, 0, TRANS_SIZE_4 * sizeof(WORD16));
        }
        else
        {

            /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
            o[0] = g_ai2_ihevc_trans_4[1][0] * pi2_src[src_strd]
                            + g_ai2_ihevc_trans_4[3][0] * pi2_src[3 * src_strd];
            o[1] = g_ai2_ihevc_trans_4[1][1] * pi2_src[src_strd]
                            + g_ai2_ihevc_trans_4[3][1] * pi2_src[3 * src_strd];
            e[0] = g_ai2_ihevc_trans_4[0][0] * pi2_src[0]
                            + g_ai2_ihevc_trans_4[2][0] * pi2_src[2 * src_strd];
            e[1] = g_ai2_ihevc_trans_4[0][1] * pi2_src[0]
                            + g_ai2_ihevc_trans_4[2][1] * pi2_src[2 * src_strd];

            pi2_dst[0] =
                            CLIP_S16(((e[0] + o[0] + add) >> i4_shift));
            pi2_dst[1] =
                            CLIP_S16(((e[1] + o[1] + add) >> i4_shift));
            pi2_dst[2] =
                            CLIP_S16(((e[1] - o[1] + add) >> i4_shift));
            pi2_dst[3] =
                            CLIP_S16(((e[0] - o[0] + add) >> i4_shift));

        }
        pi2_src++;
        pi2_dst += dst_strd;
        zero_cols = zero_cols >> 1;
    }
}
#else
void ihevc_itrans_4x4(WORD16 *pi2_src,
                      WORD16 *pi2_dst,
                      WORD32 src_strd,
                      WORD32 dst_strd,
                      WORD32 i4_shift,
                      WORD32 zero_cols)
{
    WORD32 j;
    WORD32 e[2], o[2];
    WORD32 add;

    add = 1 << (i4_shift - 1);

    /***************************************************************************/
    /* Transform Matrix 4x4                                                    */
    /*      0   1   2   3                                                      */
    /* 0 { 64, 64, 64, 64},                                                    */
    /* 1 { 83, 36,-36,-83},                                                    */
    /* 2 { 64,-64,-64, 64},                                                    */
    /* 3 { 36,-83, 83,-36}                                                     */
    /***************************************************************************/

    for(j = 0; j < TRANS_SIZE_4; j++)
    {
        WORD32 temp;

        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            memset(pi2_dst, 0, TRANS_SIZE_4 * sizeof(WORD16));
        }
        else
        {
            /* Common operation in o[0] and o[1] */
            temp = (pi2_src[src_strd] + pi2_src[3 * src_strd]) * 36;

            o[0] = temp + 47 * pi2_src[src_strd];
            o[1] = temp - 119 * pi2_src[3 * src_strd];
            e[0] = (pi2_src[0] + pi2_src[2 * src_strd]) << 6;
            e[1] = (pi2_src[0] - pi2_src[2 * src_strd]) << 6;

            pi2_dst[0] =
                            CLIP_S16(((e[0] + o[0] + add) >> i4_shift));
            pi2_dst[1] =
                            CLIP_S16(((e[1] + o[1] + add) >> i4_shift));
            pi2_dst[2] =
                            CLIP_S16(((e[1] - o[1] + add) >> i4_shift));
            pi2_dst[3] =
                            CLIP_S16(((e[0] - o[0] + add) >> i4_shift));
        }
        pi2_src++;
        pi2_dst += dst_strd;
        zero_cols = zero_cols >> 1;
    }
}
#endif

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs Single stage  Inverse transform for 8x8 input
 * block
 *
 * @par Description:
 *  Performs single stage 8x8 inverse transform by utilizing  the symmetry of
 * transformation matrix and reducing number  of multiplications wherever
 * possible but keeping the  number of operations(addition,multiplication and
 * shift)  same
 *
 * @param[in] pi2_src
 *  Input 8x8 coefficients
 *
 * @param[out] pi2_dst
 *  Output 8x8 block
 *
 * @param[in] src_strd
 *  Input stride
 *
 * @param[in] dst_strd
 *  Output Stride
 *
 * @param[in] i4_shift
 *  Output shift
 *
 * @param[in] zero_cols
 *  Zero columns in pi2_src
 *
 * @returns  Void
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

#if NON_OPTIMIZED
void ihevc_itrans_8x8(WORD16 *pi2_src,
                      WORD16 *pi2_dst,
                      WORD32 src_strd,
                      WORD32 dst_strd,
                      WORD32 i4_shift,
                      WORD32 zero_cols)
{
    WORD32 j, k;
    WORD32 e[4], o[4];
    WORD32 ee[2], eo[2];
    WORD32 add;

    add = 1 << (i4_shift - 1);

    for(j = 0; j < TRANS_SIZE_8; j++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            memset(pi2_dst, 0, TRANS_SIZE_8 * sizeof(WORD16));
        }
        else
        {
            /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
            for(k = 0; k < 4; k++)
            {
                o[k] = g_ai2_ihevc_trans_8[1][k] * pi2_src[src_strd]
                                + g_ai2_ihevc_trans_8[3][k]
                                                * pi2_src[3 * src_strd]
                                + g_ai2_ihevc_trans_8[5][k]
                                                * pi2_src[5 * src_strd]
                                + g_ai2_ihevc_trans_8[7][k]
                                                * pi2_src[7 * src_strd];
            }

            eo[0] = g_ai2_ihevc_trans_8[2][0] * pi2_src[2 * src_strd]
                            + g_ai2_ihevc_trans_8[6][0] * pi2_src[6 * src_strd];
            eo[1] = g_ai2_ihevc_trans_8[2][1] * pi2_src[2 * src_strd]
                            + g_ai2_ihevc_trans_8[6][1] * pi2_src[6 * src_strd];
            ee[0] = g_ai2_ihevc_trans_8[0][0] * pi2_src[0]
                            + g_ai2_ihevc_trans_8[4][0] * pi2_src[4 * src_strd];
            ee[1] = g_ai2_ihevc_trans_8[0][1] * pi2_src[0]
                            + g_ai2_ihevc_trans_8[4][1] * pi2_src[4 * src_strd];

            /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
            e[0] = ee[0] + eo[0];
            e[3] = ee[0] - eo[0];
            e[1] = ee[1] + eo[1];
            e[2] = ee[1] - eo[1];
            for(k = 0; k < 4; k++)
            {
                pi2_dst[k] =
                                CLIP_S16(((e[k] + o[k] + add) >> i4_shift));
                pi2_dst[k + 4] =
                                CLIP_S16(((e[3 - k] - o[3 - k] + add) >> i4_shift));
            }
        }
        pi2_src++;
        pi2_dst += dst_strd;
        zero_cols = zero_cols >> 1;
    }
}

#else
void ihevc_itrans_8x8(WORD16 *pi2_src,
                      WORD16 *pi2_dst,
                      WORD32 src_strd,
                      WORD32 dst_strd,
                      WORD32 i4_shift,
                      WORD32 zero_cols)
{
    /* Transform Matrix 8x8                          */
    /*              0    1    2   3   4   5   6   7  */
    /*     0 -      64   64   64  64  64  64  64  64 */
    /*     1 -      89   75   50  18 -18 -50 -75 -89 */
    /*     2 -      83   36  -36 -83 -83 -36  36  83 */
    /*     3 -      75  -18  -89 -50  50  89  18 -75 */
    /*     4 -      64  -64  -64  64  64 -64 -64  64 */
    /*     5 -      50  -89   18  75 -75 -18  89 -50 */
    /*     6 -      36  -83   83 -36 -36  83 -83  36 */
    /*     7 -      18  -50   75 -89  89 -75  50 -18 */

    /* 0th and 4th row will have no multiplications */
    /* 2nd and 6th row has only two coefff multiplies */
    /* 1st, 3rd, 5th and 7th rows have o mirror symmetry */
    WORD32 j, k;
    WORD32 temp1, temp2;
    WORD32 e[4], o[4];
    WORD32 ee[2], eo[2];
    WORD32 add;

    add = 1 << (i4_shift - 1);

    for(j = 0; j < TRANS_SIZE_8; j++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            memset(pi2_dst, 0, TRANS_SIZE_8 * sizeof(WORD16));
        }
        else
        {

            /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
            /*
             o[0] = 89 *pi2_src[8] +  75 *pi2_src[3*8] +  50 *pi2_src[5*8] +  18 *pi2_src[7*8];
             o[1] = 75 *pi2_src[8] + -18 *pi2_src[3*8] + -89 *pi2_src[5*8] + -50 *pi2_src[7*8];
             o[2] = 50 *pi2_src[8] + -89 *pi2_src[3*8] +  18 *pi2_src[5*8] +  75 *pi2_src[7*8];
             o[3] = 18 *pi2_src[8] + -50 *pi2_src[3*8] +  75 *pi2_src[5*8] + -89 *pi2_src[7*8];
             */

            /* Optimization: 4 mul + 2 add  ---> 3 mul + 3 add */
            /*
             temp1 = (pi2_src[8  ] + pi2_src[3*8]) * 75;
             temp2 = (pi2_src[5*8] + pi2_src[7*8]) * 50;

             o[0] = temp1 + 14 * pi2_src[8  ] + temp2 - 32 * pi2_src[7*8];
             o[1] = temp1 - 93 * pi2_src[3*8] - temp2 - 39 * pi2_src[5*8];
             */

            temp1 = (pi2_src[src_strd] + pi2_src[3 * src_strd]) * 75;
            temp2 = (pi2_src[5 * src_strd] + pi2_src[7 * src_strd]) * 50;

            o[0] = temp1 + 14 * pi2_src[src_strd] + temp2
                            - (pi2_src[7 * src_strd] << 5);
            o[1] = temp1 - 93 * pi2_src[3 * src_strd] - temp2
                            - 39 * pi2_src[5 * src_strd];

            /* Optimization: 4 mul + 2 add  ---> 3 mul + 3 add */
            /*
             temp1 = (pi2_src[8  ] - pi2_src[3*8]) * 50;
             temp2 = (pi2_src[5*8] + pi2_src[7*8]) * 75;

             o[2] = temp1 - 39 * pi2_src[3*8] + temp2 -  57 * pi2_src[5*8];
             o[3] = temp1 - 32 * pi2_src[8  ] + temp2 - 164 * pi2_src[7*8];
             */

            temp1 = (pi2_src[src_strd] - pi2_src[3 * src_strd]) * 50;
            temp2 = (pi2_src[5 * src_strd] + pi2_src[7 * src_strd]) * 75;

            o[2] = temp1 - 39 * pi2_src[3 * src_strd] + temp2
                            - 57 * pi2_src[5 * src_strd];
            o[3] = temp1 - (pi2_src[src_strd] << 5) + temp2
                            - 164 * pi2_src[7 * src_strd];

            /*
             eo[0] = 83 *pi2_src[ 2*8 ] +  36 *pi2_src[ 6*8 ];
             eo[1] = 36 *pi2_src[ 2*8 ] + -83 *pi2_src[ 6*8 ];
             ee[0] = 64 *pi2_src[ 0   ] +  64 *pi2_src[ 4*8 ];
             ee[1] = 64 *pi2_src[ 0   ] + -64 *pi2_src[ 4*8 ];
             */

            /* Optimization: 4 mul + 2 add  ---> 3 mul + 3 add */
            temp1 = (pi2_src[2 * src_strd] + pi2_src[6 * src_strd]) * 36;
            eo[0] = temp1 + 47 * pi2_src[2 * src_strd];
            eo[1] = temp1 - 119 * pi2_src[6 * src_strd];

            /* Optimization: 4 mul + 2 add  ---> 2 i4_shift + 2 add */
            ee[0] = (pi2_src[0] + pi2_src[4 * src_strd]) << 6;
            ee[1] = (pi2_src[0] - pi2_src[4 * src_strd]) << 6;

            e[0] = ee[0] + eo[0];
            e[3] = ee[0] - eo[0];
            e[1] = ee[1] + eo[1];
            e[2] = ee[1] - eo[1];

            for(k = 0; k < 4; k++)
            {
                pi2_dst[k] =
                                CLIP_S16(((e[k] + o[k] + add) >> i4_shift));
                pi2_dst[k + 4] =
                                CLIP_S16(((e[3 - k] - o[3 - k] + add) >> i4_shift));
            }
        }
        pi2_src++;
        pi2_dst += dst_strd;
        zero_cols = zero_cols >> 1;
    }

}
#endif


/**
 *******************************************************************************
 *
 * @brief
 *  This function performs Single stage  Inverse transform for 16x16 input
 * block
 *
 * @par Description:
 *  Performs single stage 16x16 inverse transform by  utilizing the symmetry
 * of transformation matrix  and reducing number of multiplications wherever
 * possible  but keeping the number of operations  (addition,multiplication
 * and shift) same
 *
 * @param[in] pi2_src
 *  Input 16x16 coefficients
 *
 * @param[out] pi2_dst
 *  Output 16x16 block
 *
 * @param[in] src_strd
 *  Input stride
 *
 * @param[in] dst_strd
 *  Output Stride
 *
 * @param[in] i4_shift
 *  Output shift
 *
 * @param[in] zero_cols
 *  Zero columns in pi2_src
 *
 * @returns  Void
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */

#if NON_OPTIMIZED
void ihevc_itrans_16x16(WORD16 *pi2_src,
                        WORD16 *pi2_dst,
                        WORD32 src_strd,
                        WORD32 dst_strd,
                        WORD32 i4_shift,
                        WORD32 zero_cols)
{
    WORD32 j, k;
    WORD32 e[8], o[8];
    WORD32 ee[4], eo[4];
    WORD32 eee[2], eeo[2];
    WORD32 add;

    add = 1 << (i4_shift - 1);

    for(j = 0; j < TRANS_SIZE_16; j++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            memset(pi2_dst, 0, TRANS_SIZE_16 * sizeof(WORD16));
        }
        else
        {
            /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
            for(k = 0; k < 8; k++)
            {
                o[k] = g_ai2_ihevc_trans_16[1][k] * pi2_src[src_strd]
                                + g_ai2_ihevc_trans_16[3][k]
                                                * pi2_src[3 * src_strd]
                                + g_ai2_ihevc_trans_16[5][k]
                                                * pi2_src[5 * src_strd]
                                + g_ai2_ihevc_trans_16[7][k]
                                                * pi2_src[7 * src_strd]
                                + g_ai2_ihevc_trans_16[9][k]
                                                * pi2_src[9 * src_strd]
                                + g_ai2_ihevc_trans_16[11][k]
                                                * pi2_src[11 * src_strd]
                                + g_ai2_ihevc_trans_16[13][k]
                                                * pi2_src[13 * src_strd]
                                + g_ai2_ihevc_trans_16[15][k]
                                                * pi2_src[15 * src_strd];
            }
            for(k = 0; k < 4; k++)
            {
                eo[k] = g_ai2_ihevc_trans_16[2][k] * pi2_src[2 * src_strd]
                                + g_ai2_ihevc_trans_16[6][k]
                                                * pi2_src[6 * src_strd]
                                + g_ai2_ihevc_trans_16[10][k]
                                                * pi2_src[10 * src_strd]
                                + g_ai2_ihevc_trans_16[14][k]
                                                * pi2_src[14 * src_strd];
            }
            eeo[0] = g_ai2_ihevc_trans_16[4][0] * pi2_src[4 * src_strd]
                            + g_ai2_ihevc_trans_16[12][0]
                                            * pi2_src[12 * src_strd];
            eee[0] =
                            g_ai2_ihevc_trans_16[0][0] * pi2_src[0]
                                            + g_ai2_ihevc_trans_16[8][0]
                                                            * pi2_src[8
                                                                            * src_strd];
            eeo[1] = g_ai2_ihevc_trans_16[4][1] * pi2_src[4 * src_strd]
                            + g_ai2_ihevc_trans_16[12][1]
                                            * pi2_src[12 * src_strd];
            eee[1] =
                            g_ai2_ihevc_trans_16[0][1] * pi2_src[0]
                                            + g_ai2_ihevc_trans_16[8][1]
                                                            * pi2_src[8
                                                                            * src_strd];

            /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
            for(k = 0; k < 2; k++)
            {
                ee[k] = eee[k] + eeo[k];
                ee[k + 2] = eee[1 - k] - eeo[1 - k];
            }
            for(k = 0; k < 4; k++)
            {
                e[k] = ee[k] + eo[k];
                e[k + 4] = ee[3 - k] - eo[3 - k];
            }
            for(k = 0; k < 8; k++)
            {
                pi2_dst[k] =
                                CLIP_S16(((e[k] + o[k] + add) >> i4_shift));
                pi2_dst[k + 8] =
                                CLIP_S16(((e[7 - k] - o[7 - k] + add) >> i4_shift));
            }
        }
        pi2_src++;
        pi2_dst += dst_strd;
        zero_cols = zero_cols >> 1;
    }
}
#else
void ihevc_itrans_16x16(WORD16 *pi2_src,
                        WORD16 *pi2_dst,
                        WORD32 src_strd,
                        WORD32 dst_strd,
                        WORD32 i4_shift,
                        WORD32 zero_cols)
{
    WORD32 j, k;
    WORD32 e[8], o[8];
    WORD32 ee[4], eo[4];
    WORD32 eee[2], eeo[2];
    WORD32 add;
    WORD32 temp1, temp2;

    add = 1 << (i4_shift - 1);
    /***************************************************************************/
    /* Transform Matrix 16x16                                                  */
    /*       0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15     */
    /* 0  { 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},   */
    /* 1  { 90, 87, 80, 70, 57, 43, 25,  9, -9,-25,-43,-57,-70,-80,-87,-90},   */
    /* 2  { 89, 75, 50, 18,-18,-50,-75,-89,-89,-75,-50,-18, 18, 50, 75, 89},   */
    /* 3  { 87, 57,  9,-43,-80,-90,-70,-25, 25, 70, 90, 80, 43, -9,-57,-87},   */
    /* 4  { 83, 36,-36,-83,-83,-36, 36, 83, 83, 36,-36,-83,-83,-36, 36, 83},   */
    /* 5  { 80,  9,-70,-87,-25, 57, 90, 43,-43,-90,-57, 25, 87, 70, -9,-80},   */
    /* 6  { 75,-18,-89,-50, 50, 89, 18,-75,-75, 18, 89, 50,-50,-89,-18, 75},   */
    /* 7  { 70,-43,-87,  9, 90, 25,-80,-57, 57, 80,-25,-90, -9, 87, 43,-70},   */
    /* 8  { 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64, 64,-64,-64, 64},   */
    /* 9  { 57,-80,-25, 90, -9,-87, 43, 70,-70,-43, 87,  9,-90, 25, 80,-57},   */
    /* 10 { 50,-89, 18, 75,-75,-18, 89,-50,-50, 89,-18,-75, 75, 18,-89, 50},   */
    /* 11 { 43,-90, 57, 25,-87, 70,  9,-80, 80, -9,-70, 87,-25,-57, 90,-43},   */
    /* 12 { 36,-83, 83,-36,-36, 83,-83, 36, 36,-83, 83,-36,-36, 83,-83, 36},   */
    /* 13 { 25,-70, 90,-80, 43,  9,-57, 87,-87, 57, -9,-43, 80,-90, 70,-25},   */
    /* 14 { 18,-50, 75,-89, 89,-75, 50,-18,-18, 50,-75, 89,-89, 75,-50, 18},   */
    /* 15 {  9,-25, 43,-57, 70,-80, 87,-90, 90,-87, 80,-70, 57,-43, 25, -9}    */
    /***************************************************************************/

    for(j = 0; j < TRANS_SIZE_16; j++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            memset(pi2_dst, 0, TRANS_SIZE_16 * sizeof(WORD16));
        }
        else
        {
            /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
            {
                /*
                 o[k] = g_ai2_ihevc_trans_16[ 1][k]*pi2_src[ src_strd   ] + g_ai2_ihevc_trans_16[ 3][k]*pi2_src[ 3*src_strd   ] + g_ai2_ihevc_trans_16[ 5][k]*pi2_src[ 5*src_strd   ] + g_ai2_ihevc_trans_16[ 7][k]*pi2_src[ 7*src_strd   ] +
                 g_ai2_ihevc_trans_16[ 9][k]*pi2_src[ 9*src_strd   ] + g_ai2_ihevc_trans_16[11][k]*pi2_src[11*src_strd   ] + g_ai2_ihevc_trans_16[13][k]*pi2_src[13*src_strd   ] + g_ai2_ihevc_trans_16[15][k]*pi2_src[15*src_strd   ];
                 */

                o[0] = 90 * pi2_src[src_strd] + 87 * pi2_src[3 * src_strd]
                                + 80 * pi2_src[5 * src_strd]
                                + 70 * pi2_src[7 * src_strd]
                                + 57 * pi2_src[9 * src_strd]
                                + 43 * pi2_src[11 * src_strd]
                                + 25 * pi2_src[13 * src_strd]
                                + 9 * pi2_src[15 * src_strd];

                o[1] = 87 * pi2_src[src_strd] + 57 * pi2_src[3 * src_strd]
                                + 9 * pi2_src[5 * src_strd]
                                + -43 * pi2_src[7 * src_strd]
                                + -80 * pi2_src[9 * src_strd]
                                + -90 * pi2_src[11 * src_strd]
                                + -70 * pi2_src[13 * src_strd]
                                + -25 * pi2_src[15 * src_strd];

                o[2] = 80 * pi2_src[src_strd] + 9 * pi2_src[3 * src_strd]
                                + -70 * pi2_src[5 * src_strd]
                                + -87 * pi2_src[7 * src_strd]
                                + -25 * pi2_src[9 * src_strd]
                                + 57 * pi2_src[11 * src_strd]
                                + 90 * pi2_src[13 * src_strd]
                                + 43 * pi2_src[15 * src_strd];

                o[3] = 70 * pi2_src[src_strd] + -43 * pi2_src[3 * src_strd]
                                + -87 * pi2_src[5 * src_strd]
                                + 9 * pi2_src[7 * src_strd]
                                + 90 * pi2_src[9 * src_strd]
                                + 25 * pi2_src[11 * src_strd]
                                + -80 * pi2_src[13 * src_strd]
                                + -57 * pi2_src[15 * src_strd];

                o[4] = 57 * pi2_src[src_strd] + -80 * pi2_src[3 * src_strd]
                                + -25 * pi2_src[5 * src_strd]
                                + 90 * pi2_src[7 * src_strd]
                                + -9 * pi2_src[9 * src_strd]
                                + -87 * pi2_src[11 * src_strd]
                                + 43 * pi2_src[13 * src_strd]
                                + 70 * pi2_src[15 * src_strd];

                o[5] = 43 * pi2_src[src_strd] + -90 * pi2_src[3 * src_strd]
                                + 57 * pi2_src[5 * src_strd]
                                + 25 * pi2_src[7 * src_strd]
                                + -87 * pi2_src[9 * src_strd]
                                + 70 * pi2_src[11 * src_strd]
                                + 9 * pi2_src[13 * src_strd]
                                + -80 * pi2_src[15 * src_strd];

                o[6] = 25 * pi2_src[src_strd] + -70 * pi2_src[3 * src_strd]
                                + 90 * pi2_src[5 * src_strd]
                                + -80 * pi2_src[7 * src_strd]
                                + 43 * pi2_src[9 * src_strd]
                                + 9 * pi2_src[11 * src_strd]
                                + -57 * pi2_src[13 * src_strd]
                                + 87 * pi2_src[15 * src_strd];

                o[7] = 9 * pi2_src[src_strd] + -25 * pi2_src[3 * src_strd]
                                + 43 * pi2_src[5 * src_strd]
                                + -57 * pi2_src[7 * src_strd]
                                + 70 * pi2_src[9 * src_strd]
                                + -80 * pi2_src[11 * src_strd]
                                + 87 * pi2_src[13 * src_strd]
                                + -90 * pi2_src[15 * src_strd];
            }
            {
                temp1 = (pi2_src[2 * src_strd] + pi2_src[6 * src_strd]) * 75;
                temp2 = (pi2_src[10 * src_strd] + pi2_src[14 * src_strd]) * 50;
                eo[0] = temp1 + 14 * pi2_src[2 * src_strd] + temp2
                                - (pi2_src[14 * src_strd] << 5);
                eo[1] = temp1 - 93 * pi2_src[6 * src_strd] - temp2
                                - 39 * pi2_src[10 * src_strd];

                temp1 = (pi2_src[2 * src_strd] - pi2_src[6 * src_strd]) * 50;
                temp2 = (pi2_src[10 * src_strd] + pi2_src[14 * src_strd]) * 75;
                eo[2] = temp1 - 39 * pi2_src[6 * src_strd] + temp2
                                - 57 * pi2_src[10 * src_strd];
                eo[3] = temp1 - (pi2_src[2 * src_strd] << 5) + temp2
                                - 164 * pi2_src[14 * src_strd];
            }

            temp1 = (pi2_src[4 * src_strd] + pi2_src[12 * src_strd]) * 36;
            eeo[0] = temp1 + 47 * pi2_src[4 * src_strd];
            eeo[1] = temp1 - 119 * pi2_src[12 * src_strd];

            eee[0] = (pi2_src[0] + pi2_src[8 * src_strd]) << 6;
            eee[1] = (pi2_src[0] - pi2_src[8 * src_strd]) << 6;

            /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
            for(k = 0; k < 2; k++)
            {
                ee[k] = eee[k] + eeo[k];
                ee[k + 2] = eee[1 - k] - eeo[1 - k];
            }
            for(k = 0; k < 4; k++)
            {
                e[k] = ee[k] + eo[k];
                e[k + 4] = ee[3 - k] - eo[3 - k];
            }
            for(k = 0; k < 8; k++)
            {
                pi2_dst[k] =
                                CLIP_S16(((e[k] + o[k] + add) >> i4_shift));
                pi2_dst[k + 8] =
                                CLIP_S16(((e[7 - k] - o[7 - k] + add) >> i4_shift));
            }
        }
        pi2_src++;
        pi2_dst += dst_strd;
        zero_cols = zero_cols >> 1;
    }
}
#endif

/**
 *******************************************************************************
 *
 * @brief
 *  This function performs Single stage  Inverse transform for 32x32 input
 * block
 *
 * @par Description:
 *  Performs single stage 32x32 inverse transform by  utilizing the symmetry
 * of transformation matrix and  reducing number of multiplications wherever
 * possible  but keeping the number of operations  (addition,multiplication
 * and shift) same
 *
 * @param[in] pi2_src
 *  Input 32x32 coefficients
 *
 * @param[out] pi2_dst
 *  Output 32x32 block
 *
 * @param[in] src_strd
 *  Input stride
 *
 * @param[in] dst_strd
 *  Output Stride
 *
 * @param[in] i4_shift
 *  Output shift
 *
 * @param[in] zero_cols
 *  Zero columns in pi2_src
 *
 * @returns  Void
 *
 * @remarks
 *  None
 *
 *******************************************************************************
 */


void ihevc_itrans_32x32(WORD16 *pi2_src,
                        WORD16 *pi2_dst,
                        WORD32 src_strd,
                        WORD32 dst_strd,
                        WORD32 i4_shift,
                        WORD32 zero_cols)
{
    WORD32 j, k;
    WORD32 e[16], o[16];
    WORD32 ee[8], eo[8];
    WORD32 eee[4], eeo[4];
    WORD32 eeee[2], eeeo[2];
    WORD32 add;

    add = 1 << (i4_shift - 1);

    for(j = 0; j < TRANS_SIZE_32; j++)
    {
        /* Checking for Zero Cols */
        if((zero_cols & 1) == 1)
        {
            memset(pi2_dst, 0, TRANS_SIZE_32 * sizeof(WORD16));
        }
        else
        {
            /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
            for(k = 0; k < 16; k++)
            {
                o[k] = g_ai2_ihevc_trans_32[1][k] * pi2_src[src_strd]
                                + g_ai2_ihevc_trans_32[3][k]
                                                * pi2_src[3 * src_strd]
                                + g_ai2_ihevc_trans_32[5][k]
                                                * pi2_src[5 * src_strd]
                                + g_ai2_ihevc_trans_32[7][k]
                                                * pi2_src[7 * src_strd]
                                + g_ai2_ihevc_trans_32[9][k]
                                                * pi2_src[9 * src_strd]
                                + g_ai2_ihevc_trans_32[11][k]
                                                * pi2_src[11 * src_strd]
                                + g_ai2_ihevc_trans_32[13][k]
                                                * pi2_src[13 * src_strd]
                                + g_ai2_ihevc_trans_32[15][k]
                                                * pi2_src[15 * src_strd]
                                + g_ai2_ihevc_trans_32[17][k]
                                                * pi2_src[17 * src_strd]
                                + g_ai2_ihevc_trans_32[19][k]
                                                * pi2_src[19 * src_strd]
                                + g_ai2_ihevc_trans_32[21][k]
                                                * pi2_src[21 * src_strd]
                                + g_ai2_ihevc_trans_32[23][k]
                                                * pi2_src[23 * src_strd]
                                + g_ai2_ihevc_trans_32[25][k]
                                                * pi2_src[25 * src_strd]
                                + g_ai2_ihevc_trans_32[27][k]
                                                * pi2_src[27 * src_strd]
                                + g_ai2_ihevc_trans_32[29][k]
                                                * pi2_src[29 * src_strd]
                                + g_ai2_ihevc_trans_32[31][k]
                                                * pi2_src[31 * src_strd];
            }
            for(k = 0; k < 8; k++)
            {
                eo[k] = g_ai2_ihevc_trans_32[2][k] * pi2_src[2 * src_strd]
                                + g_ai2_ihevc_trans_32[6][k]
                                                * pi2_src[6 * src_strd]
                                + g_ai2_ihevc_trans_32[10][k]
                                                * pi2_src[10 * src_strd]
                                + g_ai2_ihevc_trans_32[14][k]
                                                * pi2_src[14 * src_strd]
                                + g_ai2_ihevc_trans_32[18][k]
                                                * pi2_src[18 * src_strd]
                                + g_ai2_ihevc_trans_32[22][k]
                                                * pi2_src[22 * src_strd]
                                + g_ai2_ihevc_trans_32[26][k]
                                                * pi2_src[26 * src_strd]
                                + g_ai2_ihevc_trans_32[30][k]
                                                * pi2_src[30 * src_strd];
            }
            for(k = 0; k < 4; k++)
            {
                eeo[k] = g_ai2_ihevc_trans_32[4][k] * pi2_src[4 * src_strd]
                                + g_ai2_ihevc_trans_32[12][k]
                                                * pi2_src[12 * src_strd]
                                + g_ai2_ihevc_trans_32[20][k]
                                                * pi2_src[20 * src_strd]
                                + g_ai2_ihevc_trans_32[28][k]
                                                * pi2_src[28 * src_strd];
            }
            eeeo[0] = g_ai2_ihevc_trans_32[8][0] * pi2_src[8 * src_strd]
                            + g_ai2_ihevc_trans_32[24][0]
                                            * pi2_src[24 * src_strd];
            eeeo[1] = g_ai2_ihevc_trans_32[8][1] * pi2_src[8 * src_strd]
                            + g_ai2_ihevc_trans_32[24][1]
                                            * pi2_src[24 * src_strd];
            eeee[0] = g_ai2_ihevc_trans_32[0][0] * pi2_src[0]
                            + g_ai2_ihevc_trans_32[16][0]
                                            * pi2_src[16 * src_strd];
            eeee[1] = g_ai2_ihevc_trans_32[0][1] * pi2_src[0]
                            + g_ai2_ihevc_trans_32[16][1]
                                            * pi2_src[16 * src_strd];

            /* Combining e and o terms at each hierarchy levels to calculate the final spatial domain vector */
            eee[0] = eeee[0] + eeeo[0];
            eee[3] = eeee[0] - eeeo[0];
            eee[1] = eeee[1] + eeeo[1];
            eee[2] = eeee[1] - eeeo[1];
            for(k = 0; k < 4; k++)
            {
                ee[k] = eee[k] + eeo[k];
                ee[k + 4] = eee[3 - k] - eeo[3 - k];
            }
            for(k = 0; k < 8; k++)
            {
                e[k] = ee[k] + eo[k];
                e[k + 8] = ee[7 - k] - eo[7 - k];
            }
            for(k = 0; k < 16; k++)
            {
                pi2_dst[k] =
                                CLIP_S16(((e[k] + o[k] + add) >> i4_shift));
                pi2_dst[k + 16] =
                                CLIP_S16(((e[15 - k] - o[15 - k] + add) >> i4_shift));
            }
        }
        pi2_src++;
        pi2_dst += dst_strd;
        zero_cols = zero_cols >> 1;
    }
}

