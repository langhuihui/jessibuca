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
*  ihevc_sao_atom_intr.c
*
* @brief
*  Contains function definitions for Sample adaptive offset(SAO) used in-loop
* filtering
*
* @author
* 100592
*
* @par List of Functions:
*   - ihevc_sao_band_offset_luma_ssse3()
*   - ihevc_sao_band_offset_chroma_ssse3()
*   - ihevc_sao_edge_offset_class0_ssse3()
*   - ihevc_sao_edge_offset_class0_chroma_ssse3()
*   - ihevc_sao_edge_offset_class1_ssse3()
*   - ihevc_sao_edge_offset_class1_chroma_ssse3()
*   - ihevc_sao_edge_offset_class2_ssse3()
*   - ihevc_sao_edge_offset_class2_chroma_ssse3()
*   - ihevc_sao_edge_offset_class3_ssse3()
*   - ihevc_sao_edge_offset_class3_chroma_ssse3()
*
* @remarks
*  None
*
*******************************************************************************
*/
/*****************************************************************************/
/* File Includes                                                             */
/*****************************************************************************/
#include <stdio.h>

#include "ihevc_typedefs.h"
#include "ihevc_platform_macros.h"
#include "ihevc_macros.h"
#include "ihevc_func_selector.h"
#include "ihevc_defs.h"
#include "ihevc_tables_x86_intr.h"
#include "ihevc_common_tables.h"
#include "ihevc_sao.h"

#include <immintrin.h>

#define NUM_BAND_TABLE  32
/**
*******************************************************************************
*
* @brief
* Has two sets of functions : band offset and edge offset both for luma and chroma
* edge offset has horizontal ,vertical, 135 degree and 45 degree
*
* @par Description:
*
*
* @param[in-out] pu1_src
*  Pointer to the source
*
* @param[in] src_strd
*  Source stride
*
* @param[in-out] pu1_src_left
*  source left boundary
*
* @param[in-out] pu1_src_top
* Source top boundary
*
* @param[in-out] pu1_src_top_left
*  Source top left boundary
*
* @param[in] pu1_src_top_right
*  Source top right boundary
*
* @param[in] pu1_src_bot_left
*  Source bottom left boundary
*
* @param[in] pu1_avail
*  boundary availability flags
*
* @param[in] pi1_sao_offset_u
*  Chroma U sao offset values
*
* @param[in] pi1_sao_offset_v
*  Chroma V sao offset values
*
* @param[in] pi1_sao_offset
*  Luma sao offset values
*
* @param[in] wd
*  width of the source

* @param[in] ht
*  height of the source
* @returns
*
* @remarks
*  None
*
*******************************************************************************
*/


void ihevc_sao_band_offset_luma_ssse3(UWORD8 *pu1_src,
                                      WORD32 src_strd,
                                      UWORD8 *pu1_src_left,
                                      UWORD8 *pu1_src_top,
                                      UWORD8 *pu1_src_top_left,
                                      WORD32 sao_band_pos,
                                      WORD8 *pi1_sao_offset,
                                      WORD32 wd,
                                      WORD32 ht)
{
    WORD32 row, col;
    UWORD8 *pu1_src_cpy;
    WORD32 wd_rem;
    WORD8 offset = 0;

    __m128i src_temp0_8x16b, src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b;
    __m128i band_table0_8x16b, band_table1_8x16b, band_table2_8x16b, band_table3_8x16b;
    __m128i tmp_set_128i_1, tmp_set_128i_2, tmp_set_128i_3, tmp_set_128i_4;
    __m128i band_pos_16x8b;
    __m128i sao_offset;
    __m128i cmp_mask, cmp_store;

    /* Updating left and top-left and top */
    for(row = 0; row < ht; row++)
    {
        pu1_src_left[row] = pu1_src[row * src_strd + (wd - 1)];
    }
    pu1_src_top_left[0] = pu1_src_top[wd - 1];
    for(col = 0; col < wd; col += 8)
    {
        tmp_set_128i_1 = _mm_loadl_epi64((__m128i *)(pu1_src + (ht - 1) * src_strd + offset));
        _mm_storel_epi64((__m128i *)(pu1_src_top + offset), tmp_set_128i_1);
        offset += 8;
    }

    //replicating sao_band_pos as 8 bit value 16 times


    band_pos_16x8b = _mm_set1_epi16((WORD16)(sao_band_pos << 3));
    //value set for sao_offset extraction
    tmp_set_128i_1  = _mm_set_epi8(128, 1, 128, 1, 128, 1, 128, 1, 128, 1, 128, 1, 128, 1, 128, 1);
    tmp_set_128i_2  = _mm_set_epi8(128, 2, 128, 2, 128, 2, 128, 2, 128, 2, 128, 2, 128, 2, 128, 2);
    tmp_set_128i_3  = _mm_set_epi8(128, 3, 128, 3, 128, 3, 128, 3, 128, 3, 128, 3, 128, 3, 128, 3);
    tmp_set_128i_4  = _mm_set_epi8(128, 4, 128, 4, 128, 4, 128, 4, 128, 4, 128, 4, 128, 4, 128, 4);

    //loaded sao offset values
    sao_offset      = _mm_loadl_epi64((__m128i *)pi1_sao_offset);

    //loading 16bit 32values of gu2_table_band_idx consecutively in 4 registers
    band_table0_8x16b = _mm_load_si128((__m128i *)(gu2_table_band_idx));
    band_table1_8x16b = _mm_load_si128((__m128i *)(gu2_table_band_idx + 8));
    band_table2_8x16b = _mm_load_si128((__m128i *)(gu2_table_band_idx + 16));
    band_table3_8x16b = _mm_load_si128((__m128i *)(gu2_table_band_idx + 24));

    //band_position addition
    band_table0_8x16b = _mm_add_epi16(band_table0_8x16b, band_pos_16x8b);
    band_table1_8x16b = _mm_add_epi16(band_table1_8x16b, band_pos_16x8b);
    band_table2_8x16b = _mm_add_epi16(band_table2_8x16b, band_pos_16x8b);
    band_table3_8x16b = _mm_add_epi16(band_table3_8x16b, band_pos_16x8b);
    //sao_offset duplication
    tmp_set_128i_1  = _mm_shuffle_epi8(sao_offset, tmp_set_128i_1);
    tmp_set_128i_2  = _mm_shuffle_epi8(sao_offset, tmp_set_128i_2);
    tmp_set_128i_3  = _mm_shuffle_epi8(sao_offset, tmp_set_128i_3);
    tmp_set_128i_4  = _mm_shuffle_epi8(sao_offset, tmp_set_128i_4);
    //settng for comparision
    cmp_mask = _mm_set1_epi16(16);
    cmp_store = _mm_set1_epi16(0x00ff);

    //sao_offset addition
    band_table0_8x16b = _mm_add_epi16(band_table0_8x16b, tmp_set_128i_1);
    band_table1_8x16b = _mm_add_epi16(band_table1_8x16b, tmp_set_128i_2);
    band_table2_8x16b = _mm_add_epi16(band_table2_8x16b, tmp_set_128i_3);
    band_table3_8x16b = _mm_add_epi16(band_table3_8x16b, tmp_set_128i_4);
    //masking upper 8bit values of each  16 bit band table value
    band_table0_8x16b = _mm_and_si128(band_table0_8x16b, cmp_store);
    band_table1_8x16b = _mm_and_si128(band_table1_8x16b, cmp_store);
    band_table2_8x16b = _mm_and_si128(band_table2_8x16b, cmp_store);
    band_table3_8x16b = _mm_and_si128(band_table3_8x16b, cmp_store);

    switch(sao_band_pos)
    {
        case 0:
            tmp_set_128i_2 = _mm_cmpgt_epi16(cmp_mask, band_table0_8x16b);
            band_table0_8x16b = _mm_and_si128(band_table0_8x16b, tmp_set_128i_2);
            break;
        case 28:
            tmp_set_128i_2 = _mm_cmpgt_epi16(cmp_mask, band_table3_8x16b);
            band_table3_8x16b = _mm_or_si128(band_table3_8x16b, tmp_set_128i_2);
            break;
        case 29:
            tmp_set_128i_2 = _mm_cmpgt_epi16(cmp_mask, band_table2_8x16b);
            band_table2_8x16b = _mm_or_si128(band_table2_8x16b, tmp_set_128i_2);
            tmp_set_128i_2 = _mm_cmpgt_epi16(cmp_mask, band_table3_8x16b);
            band_table3_8x16b = _mm_and_si128(band_table3_8x16b, tmp_set_128i_2);
            break;
        case 30:
            tmp_set_128i_2 = _mm_cmpgt_epi16(cmp_mask, band_table1_8x16b);
            band_table1_8x16b = _mm_or_si128(band_table1_8x16b, tmp_set_128i_2);
            tmp_set_128i_2 = _mm_cmpgt_epi16(cmp_mask, band_table2_8x16b);
            band_table2_8x16b = _mm_and_si128(band_table2_8x16b, tmp_set_128i_2);
            break;
        case 31:
            tmp_set_128i_2 = _mm_cmpgt_epi16(cmp_mask, band_table0_8x16b);
            band_table0_8x16b = _mm_or_si128(band_table0_8x16b, tmp_set_128i_2);
            tmp_set_128i_2 = _mm_cmpgt_epi16(cmp_mask, band_table1_8x16b);
            band_table1_8x16b = _mm_and_si128(band_table1_8x16b, tmp_set_128i_2);
            break;
        default:
            break;
    }
    //sao_offset is reused for zero cmp mask.
    sao_offset = _mm_setzero_si128();
    tmp_set_128i_1 = _mm_set1_epi8(1);
    //tmp_set_128i_2 = _mm_set_epi8 (128,7,128,6,128,5,128,4,128,3,128,2,128,1,128,0);
    cmp_mask = _mm_packus_epi16(cmp_mask, cmp_mask); //cmp_msk=dup16(16);

    //masking upper 8bit values of each  16 bit band table value
    band_table0_8x16b = _mm_and_si128(band_table0_8x16b, cmp_store);
    band_table1_8x16b = _mm_and_si128(band_table1_8x16b, cmp_store);
    band_table2_8x16b = _mm_and_si128(band_table2_8x16b, cmp_store);
    band_table3_8x16b = _mm_and_si128(band_table3_8x16b, cmp_store);

    //band table 8x16 four registers are packed into 16x8 two registers:  band_table0_8x16b and band_table2_8x16b
    band_table0_8x16b = _mm_packus_epi16(band_table0_8x16b, band_table1_8x16b);
    band_table2_8x16b = _mm_packus_epi16(band_table2_8x16b, band_table3_8x16b);

    band_table3_8x16b = _mm_slli_epi16(cmp_mask, 1); // to compare if value is greater than 31
    band_pos_16x8b = _mm_packus_epi16(band_pos_16x8b, band_pos_16x8b); //band_pos is now 8 bit aligned
    band_table3_8x16b = _mm_sub_epi8(band_table3_8x16b, tmp_set_128i_1); // to compare if value is greater than 31

    cmp_mask = _mm_sub_epi8(cmp_mask, tmp_set_128i_1);
    //  band_pos_16x8b = _mm_or_si128(band_pos_16x8b,cmp_store);

    for(col = wd; col >= 16; col -= 16)
    {
        pu1_src_cpy = pu1_src;
        for(row = ht; row > 0; row -= 2)
        {


            //row = 0 load 8 pixel values from 7:0 pos. relative to cur. pos.
            src_temp0_8x16b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));
            // row = 1
            src_temp2_8x16b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));



            //saturated substract 8 bit
            tmp_set_128i_1 = _mm_sub_epi8(src_temp0_8x16b, band_pos_16x8b);
            tmp_set_128i_3 = _mm_sub_epi8(src_temp2_8x16b, band_pos_16x8b);
            //if the values less than 0 put ff
            tmp_set_128i_2 = _mm_cmpgt_epi8(sao_offset, tmp_set_128i_1);
            tmp_set_128i_4 = _mm_cmpgt_epi8(sao_offset, tmp_set_128i_3);
            tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2);
            tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4);
            //if the values gret=ater than 31 put ff
            tmp_set_128i_2 = _mm_cmpgt_epi8(tmp_set_128i_1, band_table3_8x16b);
            tmp_set_128i_4 = _mm_cmpgt_epi8(tmp_set_128i_3, band_table3_8x16b);
            tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2);
            tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4);


            //row 0 and row1
            //if the values >16 then put ff ,cmp_mask = dup16(15)
            cmp_store = _mm_cmpgt_epi8(tmp_set_128i_1, cmp_mask);
            //values 16 to 31 for row 0 & 1 but values <16 ==0
            tmp_set_128i_2 = _mm_and_si128(tmp_set_128i_1, cmp_store);
            // values 0 to 15 for row 0 & 1
            tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, cmp_store);
            //values 16 to 31 for row 0 & 1 but values <16 masked to ff
            cmp_store = _mm_cmpeq_epi8(tmp_set_128i_2, sao_offset);
            tmp_set_128i_2 = _mm_or_si128(tmp_set_128i_2, cmp_store);
            //row 2 and  row 3
            //if the values >16 then put ff ,cmp_mask = dup16(15)
            cmp_store = _mm_cmpgt_epi8(tmp_set_128i_3, cmp_mask);
            //values 16 to 31 for row 2 & 3 but values <16 ==0
            tmp_set_128i_4 = _mm_and_si128(tmp_set_128i_3, cmp_store);
            // values 0 to 15 for row 2 & 3
            tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, cmp_store);
            //values 16 to 31 for row 2 & 3 but values <16 masked to ff
            cmp_store = _mm_cmpeq_epi8(tmp_set_128i_4, sao_offset);
            tmp_set_128i_4 = _mm_or_si128(tmp_set_128i_4, cmp_store);

            //row 0 and row 1
            //to preserve pixel values in which no offset needs to be added.
            cmp_store = _mm_cmpeq_epi8(tmp_set_128i_1, tmp_set_128i_2);
            src_temp0_8x16b = _mm_and_si128(src_temp0_8x16b, cmp_store);

            //row 2 and row 3
            //to preserve pixel values in which no offset needs to be added.
            cmp_store = _mm_cmpeq_epi8(tmp_set_128i_3, tmp_set_128i_4);
            src_temp2_8x16b = _mm_and_si128(src_temp2_8x16b, cmp_store);

            //indexing 0 - 15 bandtable indexes
            tmp_set_128i_1 = _mm_shuffle_epi8(band_table0_8x16b, tmp_set_128i_1);
            tmp_set_128i_3 = _mm_shuffle_epi8(band_table0_8x16b, tmp_set_128i_3);
            tmp_set_128i_2 = _mm_shuffle_epi8(band_table2_8x16b, tmp_set_128i_2);
            tmp_set_128i_4 = _mm_shuffle_epi8(band_table2_8x16b, tmp_set_128i_4);
            // combining all offsets results
            tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2);
            tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4);
            // combing results woth the pixel values
            src_temp0_8x16b = _mm_or_si128(src_temp0_8x16b, tmp_set_128i_1);
            src_temp2_8x16b = _mm_or_si128(src_temp2_8x16b, tmp_set_128i_3);


            //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
            _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_8x16b);
            // row = 1
            _mm_storeu_si128((__m128i *)(pu1_src_cpy + src_strd), src_temp2_8x16b);

            pu1_src_cpy += (src_strd << 1);
        }
        pu1_src += 16;
    }
    wd_rem = wd & 0xF;
    if(wd_rem)
    {pu1_src_cpy = pu1_src;
        for(row = ht; row > 0; row -= 4)
        {


            //row = 0 load 8 pixel values from 7:0 pos. relative to cur. pos.
            src_temp0_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy));
            // row = 1
            src_temp1_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd));
            // row = 2
            src_temp2_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd));
            // row = 3
            src_temp3_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd));
            //row0 and row1 packed and row2 and row3 packed

            src_temp0_8x16b = _mm_unpacklo_epi64(src_temp0_8x16b, src_temp1_8x16b);
            src_temp2_8x16b = _mm_unpacklo_epi64(src_temp2_8x16b, src_temp3_8x16b);

            //saturated substract 8 bit
            tmp_set_128i_1 = _mm_sub_epi8(src_temp0_8x16b, band_pos_16x8b);
            tmp_set_128i_3 = _mm_sub_epi8(src_temp2_8x16b, band_pos_16x8b);
            //if the values less than 0 put ff
            tmp_set_128i_2 = _mm_cmpgt_epi8(sao_offset, tmp_set_128i_1);
            tmp_set_128i_4 = _mm_cmpgt_epi8(sao_offset, tmp_set_128i_3);
            tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2);
            tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4);
            //if the values gret=ater than 31 put ff
            tmp_set_128i_2 = _mm_cmpgt_epi8(tmp_set_128i_1, band_table3_8x16b);
            tmp_set_128i_4 = _mm_cmpgt_epi8(tmp_set_128i_3, band_table3_8x16b);
            tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2);
            tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4);



            //row 0 and row1
            //if the values >16 then put ff ,cmp_mask = dup16(15)
            cmp_store = _mm_cmpgt_epi8(tmp_set_128i_1, cmp_mask);
            //values 16 to 31 for row 0 & 1 but values <16 ==0
            tmp_set_128i_2 = _mm_and_si128(tmp_set_128i_1, cmp_store);
            // values 0 to 15 for row 0 & 1
            tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, cmp_store);
            //values 16 to 31 for row 0 & 1 but values <16 masked to ff
            cmp_store = _mm_cmpeq_epi8(tmp_set_128i_2, sao_offset);
            tmp_set_128i_2 = _mm_or_si128(tmp_set_128i_2, cmp_store);
            //row 2 and  row 3
            //if the values >16 then put ff ,cmp_mask = dup16(15)
            cmp_store = _mm_cmpgt_epi8(tmp_set_128i_3, cmp_mask);
            //values 16 to 31 for row 2 & 3 but values <16 ==0
            tmp_set_128i_4 = _mm_and_si128(tmp_set_128i_3, cmp_store);
            // values 0 to 15 for row 2 & 3
            tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, cmp_store);
            //values 16 to 31 for row 2 & 3 but values <16 masked to ff
            cmp_store = _mm_cmpeq_epi8(tmp_set_128i_4, sao_offset);
            tmp_set_128i_4 = _mm_or_si128(tmp_set_128i_4, cmp_store);

            //row 0 and row 1
            //to preserve pixel values in which no offset needs to be added.
            cmp_store = _mm_cmpeq_epi8(tmp_set_128i_1, tmp_set_128i_2);
            src_temp0_8x16b = _mm_and_si128(src_temp0_8x16b, cmp_store);

            //row 2 and row 3
            //to preserve pixel values in which no offset needs to be added.
            cmp_store = _mm_cmpeq_epi8(tmp_set_128i_3, tmp_set_128i_4);
            src_temp2_8x16b = _mm_and_si128(src_temp2_8x16b, cmp_store);

            //indexing 0 - 15 bandtable indexes
            tmp_set_128i_1 = _mm_shuffle_epi8(band_table0_8x16b, tmp_set_128i_1);
            tmp_set_128i_3 = _mm_shuffle_epi8(band_table0_8x16b, tmp_set_128i_3);
            tmp_set_128i_2 = _mm_shuffle_epi8(band_table2_8x16b, tmp_set_128i_2);
            tmp_set_128i_4 = _mm_shuffle_epi8(band_table2_8x16b, tmp_set_128i_4);
            // combining all offsets results
            tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2);
            tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4);
            // combing results woth the pixel values
            src_temp0_8x16b = _mm_or_si128(src_temp0_8x16b, tmp_set_128i_1);
            src_temp2_8x16b = _mm_or_si128(src_temp2_8x16b, tmp_set_128i_3);

            //Getting row1 separately
            src_temp1_8x16b = _mm_srli_si128(src_temp0_8x16b, 8);
            //Getting row3 separately
            src_temp3_8x16b = _mm_srli_si128(src_temp2_8x16b, 8);

            //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
            _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_8x16b);
            // row = 1
            _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), src_temp1_8x16b);
            // row = 2
            _mm_storel_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd), src_temp2_8x16b);
            // row = 3
            _mm_storel_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd), src_temp3_8x16b);

            pu1_src_cpy += (src_strd << 2);

        }
        pu1_src += 8;
    }


}

void ihevc_sao_band_offset_chroma_ssse3(UWORD8 *pu1_src,
                                        WORD32 src_strd,
                                        UWORD8 *pu1_src_left,
                                        UWORD8 *pu1_src_top,
                                        UWORD8 *pu1_src_top_left,
                                        WORD32 sao_band_pos_u,
                                        WORD32 sao_band_pos_v,
                                        WORD8 *pi1_sao_offset_u,
                                        WORD8 *pi1_sao_offset_v,
                                        WORD32 wd,
                                        WORD32 ht)
{
    WORD32 row, col;
    WORD8 offset = 0;


    __m128i src_temp0_8x16b, src_temp1_8x16b, src_temp2_8x16b, src_temp3_8x16b;
    __m128i cmp_msk2;
    __m128i band_table0_16x8b, band_table1_16x8b, band_table2_16x8b, band_table3_16x8b;
    __m128i tmp_set_128i_1, tmp_set_128i_2, tmp_set_128i_3, tmp_set_128i_4;
    __m128i band_pos_u_16x8b, band_pos_v_16x8b;
    __m128i sao_offset;
    __m128i cmp_mask;


    /* Updating left and top and top-left */
    for(row = 0; row < ht; row++)
    {
        pu1_src_left[2 * row] = pu1_src[row * src_strd + (wd - 2)];
        pu1_src_left[2 * row + 1] = pu1_src[row * src_strd + (wd - 1)];
    }
    pu1_src_top_left[0] = pu1_src_top[wd - 2];
    pu1_src_top_left[1] = pu1_src_top[wd - 1];
    for(col = 0; col < wd; col += 8)
    {
        tmp_set_128i_1 = _mm_loadl_epi64((__m128i *)(pu1_src + (ht - 1) * src_strd + offset));
        _mm_storel_epi64((__m128i *)(pu1_src_top + offset), tmp_set_128i_1);
        offset += 8;
    }

    { // band _table creation
        __m128i temp0_8x16b, temp1_8x16b, temp2_8x16b, temp3_8x16b;
        // Band table for U component : band_table0_16x8b and band_table2_16x8b
        //replicating sao_band_pos as 8 bit value 16 times
        band_pos_u_16x8b = _mm_set1_epi16((WORD16)(sao_band_pos_u << 3));
        //value set for sao_offset extraction
        tmp_set_128i_1  = _mm_set_epi8(128, 1, 128, 1, 128, 1, 128, 1, 128, 1, 128, 1, 128, 1, 128, 1);
        tmp_set_128i_2  = _mm_set_epi8(128, 2, 128, 2, 128, 2, 128, 2, 128, 2, 128, 2, 128, 2, 128, 2);
        tmp_set_128i_3  = _mm_set_epi8(128, 3, 128, 3, 128, 3, 128, 3, 128, 3, 128, 3, 128, 3, 128, 3);
        tmp_set_128i_4  = _mm_set_epi8(128, 4, 128, 4, 128, 4, 128, 4, 128, 4, 128, 4, 128, 4, 128, 4);

        //loaded sao offset values
        sao_offset      = _mm_loadl_epi64((__m128i *)pi1_sao_offset_u);

        //loading 16bit 32values of gu2_table_band_idx consecutively in 4 registers
        band_table0_16x8b = _mm_load_si128((__m128i *)(gu2_table_band_idx));
        band_table1_16x8b = _mm_load_si128((__m128i *)(gu2_table_band_idx + 8));
        band_table2_16x8b = _mm_load_si128((__m128i *)(gu2_table_band_idx + 16));
        band_table3_16x8b = _mm_load_si128((__m128i *)(gu2_table_band_idx + 24));

        //band_position addition
        band_table0_16x8b = _mm_add_epi16(band_table0_16x8b, band_pos_u_16x8b);
        band_table1_16x8b = _mm_add_epi16(band_table1_16x8b, band_pos_u_16x8b);
        band_table2_16x8b = _mm_add_epi16(band_table2_16x8b, band_pos_u_16x8b);
        band_table3_16x8b = _mm_add_epi16(band_table3_16x8b, band_pos_u_16x8b);
        //sao_offset duplication
        temp0_8x16b = _mm_shuffle_epi8(sao_offset, tmp_set_128i_1);
        temp1_8x16b = _mm_shuffle_epi8(sao_offset, tmp_set_128i_2);
        temp2_8x16b = _mm_shuffle_epi8(sao_offset, tmp_set_128i_3);
        temp3_8x16b = _mm_shuffle_epi8(sao_offset, tmp_set_128i_4);

        //sao_offset addition
        band_table0_16x8b = _mm_add_epi16(band_table0_16x8b, temp0_8x16b);
        band_table1_16x8b = _mm_add_epi16(band_table1_16x8b, temp1_8x16b);
        band_table2_16x8b = _mm_add_epi16(band_table2_16x8b, temp2_8x16b);
        band_table3_16x8b = _mm_add_epi16(band_table3_16x8b, temp3_8x16b);
        //reuse for clipping
        temp1_8x16b = _mm_set1_epi16(0x00ff);
        //settng for comparision
        cmp_mask = _mm_set1_epi16(16);

        //masking upper 8bit values of each  16 bit band table value
        band_table0_16x8b = _mm_and_si128(band_table0_16x8b, temp1_8x16b);
        band_table1_16x8b = _mm_and_si128(band_table1_16x8b, temp1_8x16b);
        band_table2_16x8b = _mm_and_si128(band_table2_16x8b, temp1_8x16b);
        band_table3_16x8b = _mm_and_si128(band_table3_16x8b, temp1_8x16b);

        //temp1_8x16b reuse for compare storage
        switch(sao_band_pos_u)
        {
            case 0:
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table0_16x8b);
                band_table0_16x8b = _mm_and_si128(band_table0_16x8b, temp3_8x16b);
                break;
            case 28:
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table3_16x8b);
                band_table3_16x8b = _mm_or_si128(band_table3_16x8b, temp3_8x16b);
                break;
            case 29:
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table2_16x8b);
                band_table2_16x8b = _mm_or_si128(band_table2_16x8b, temp3_8x16b);
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table3_16x8b);
                band_table3_16x8b = _mm_and_si128(band_table3_16x8b, temp3_8x16b);
                break;
            case 30:
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table1_16x8b);
                band_table1_16x8b = _mm_or_si128(band_table1_16x8b, temp3_8x16b);
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table2_16x8b);
                band_table2_16x8b = _mm_and_si128(band_table2_16x8b, temp3_8x16b);
                break;
            case 31:
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table0_16x8b);
                band_table0_16x8b = _mm_or_si128(band_table0_16x8b, temp3_8x16b);
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table1_16x8b);
                band_table1_16x8b = _mm_and_si128(band_table1_16x8b, temp3_8x16b);
                break;
            default:
                break;
        }
        //masking upper 8bit values of each  16 bit band table value
        band_table0_16x8b = _mm_and_si128(band_table0_16x8b, temp1_8x16b);
        band_table1_16x8b = _mm_and_si128(band_table1_16x8b, temp1_8x16b);
        band_table2_16x8b = _mm_and_si128(band_table2_16x8b, temp1_8x16b);
        band_table3_16x8b = _mm_and_si128(band_table3_16x8b, temp1_8x16b);
        //band table 8x16 four registers are packed into 16x8 two registers:  band_table0_8x16b and band_table2_8x16b
        band_table0_16x8b = _mm_packus_epi16(band_table0_16x8b, band_table1_16x8b);
        band_table2_16x8b = _mm_packus_epi16(band_table2_16x8b, band_table3_16x8b);
        // Band table for U component over

        // Band table for V component : band_table1_16x8b and band_table3_16x8b
        // replicating sao_band_pos as 8 bit value 16 times
        band_pos_v_16x8b = _mm_set1_epi16((WORD16)(sao_band_pos_v << 3));

        //loaded sao offset values
        sao_offset      = _mm_loadl_epi64((__m128i *)pi1_sao_offset_v);

        //loading 16bit 32values of gu2_table_band_idx consecutively in 4 registers
        temp0_8x16b = _mm_load_si128((__m128i *)(gu2_table_band_idx));
        band_table1_16x8b = _mm_load_si128((__m128i *)(gu2_table_band_idx + 8));
        temp2_8x16b = _mm_load_si128((__m128i *)(gu2_table_band_idx + 16));
        band_table3_16x8b = _mm_load_si128((__m128i *)(gu2_table_band_idx + 24));

        //band_position addition
        temp0_8x16b = _mm_add_epi16(temp0_8x16b, band_pos_v_16x8b);
        band_table1_16x8b = _mm_add_epi16(band_table1_16x8b, band_pos_v_16x8b);
        temp2_8x16b = _mm_add_epi16(temp2_8x16b, band_pos_v_16x8b);
        band_table3_16x8b = _mm_add_epi16(band_table3_16x8b, band_pos_v_16x8b);
        //sao_offset duplication
        tmp_set_128i_1  = _mm_shuffle_epi8(sao_offset, tmp_set_128i_1);
        tmp_set_128i_2  = _mm_shuffle_epi8(sao_offset, tmp_set_128i_2);
        tmp_set_128i_3  = _mm_shuffle_epi8(sao_offset, tmp_set_128i_3);
        tmp_set_128i_4  = _mm_shuffle_epi8(sao_offset, tmp_set_128i_4);

        //sao_offset addition
        temp0_8x16b = _mm_add_epi16(temp0_8x16b, tmp_set_128i_1);
        band_table1_16x8b = _mm_add_epi16(band_table1_16x8b, tmp_set_128i_2);
        temp2_8x16b = _mm_add_epi16(temp2_8x16b, tmp_set_128i_3);
        band_table3_16x8b = _mm_add_epi16(band_table3_16x8b, tmp_set_128i_4);

        //masking upper 8bit values of 16 bit band table value
        temp0_8x16b = _mm_and_si128(temp0_8x16b, temp1_8x16b);
        band_table1_16x8b = _mm_and_si128(band_table1_16x8b, temp1_8x16b);
        temp2_8x16b = _mm_and_si128(temp2_8x16b, temp1_8x16b);
        band_table3_16x8b = _mm_and_si128(band_table3_16x8b, temp1_8x16b);
        //temp1_8x16b reuse for compare storage

        switch(sao_band_pos_v)
        {
            case 0:
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, temp0_8x16b);
                temp0_8x16b = _mm_and_si128(temp0_8x16b, temp3_8x16b);
                break;
            case 28:
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table3_16x8b);
                band_table3_16x8b = _mm_or_si128(band_table3_16x8b, temp3_8x16b);
                break;
            case 29:
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, temp2_8x16b);
                temp2_8x16b = _mm_or_si128(temp2_8x16b, temp3_8x16b);
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table3_16x8b);
                band_table3_16x8b = _mm_and_si128(band_table3_16x8b, temp3_8x16b);
                break;
            case 30:
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table1_16x8b);
                band_table1_16x8b = _mm_or_si128(band_table1_16x8b, temp3_8x16b);
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, temp2_8x16b);
                temp2_8x16b = _mm_and_si128(temp2_8x16b, temp3_8x16b);
                break;
            case 31:
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, temp0_8x16b);
                temp0_8x16b = _mm_or_si128(temp0_8x16b, temp3_8x16b);
                temp3_8x16b = _mm_cmpgt_epi16(cmp_mask, band_table1_16x8b);
                band_table1_16x8b = _mm_and_si128(band_table1_16x8b, temp3_8x16b);
                break;
            default:
                break;
        }
        //masking upper 8bit values of each  16 bit band table value
        temp0_8x16b = _mm_and_si128(temp0_8x16b, temp1_8x16b);
        band_table1_16x8b = _mm_and_si128(band_table1_16x8b, temp1_8x16b);
        temp2_8x16b = _mm_and_si128(temp2_8x16b, temp1_8x16b);
        band_table3_16x8b = _mm_and_si128(band_table3_16x8b, temp1_8x16b);
        //band table 8x16 four registers are packed into 16x8 two registers:  band_table0_8x16b and band_table2_8x16b
        band_table1_16x8b = _mm_packus_epi16(temp0_8x16b, band_table1_16x8b);
        band_table3_16x8b = _mm_packus_epi16(temp2_8x16b, band_table3_16x8b);
        //band table for u and v created
    }
    {
        UWORD8 *pu1_src_cpy;
        WORD32 wd_rem;


        //sao_offset is reused for zero cmp mask.
        sao_offset = _mm_setzero_si128();
        tmp_set_128i_1 = _mm_set1_epi8(1);
        //tmp_set_128i_2 = _mm_set_epi8 (128,7,128,6,128,5,128,4,128,3,128,2,128,1,128,0);
        cmp_mask = _mm_packus_epi16(cmp_mask, cmp_mask); //cmp_msk=dup16(16);
        //to avoid ffff to be saturated to 0 instead it should be to ff

        cmp_msk2 = _mm_slli_epi16(cmp_mask, 1); // to compare if value is greater than 31
        band_pos_u_16x8b = _mm_packus_epi16(band_pos_u_16x8b, band_pos_u_16x8b); //band_pos_u is now 8 bit aligned
        band_pos_v_16x8b = _mm_packus_epi16(band_pos_v_16x8b, band_pos_v_16x8b); //band_pos_v is now 8 bit aligned
        cmp_msk2 = _mm_sub_epi8(cmp_msk2, tmp_set_128i_1); // to compare if value is greater than 31

        cmp_mask = _mm_sub_epi8(cmp_mask, tmp_set_128i_1);

        for(col = wd; col >= 16; col -= 16)
        {
            pu1_src_cpy = pu1_src;
            for(row = ht; row > 0; row -= 2)
            {
                //row = 0 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp0_8x16b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));
                // row = 1
                src_temp3_8x16b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));


                //odd values
                src_temp1_8x16b = _mm_srli_epi16(src_temp0_8x16b, 8);
                src_temp2_8x16b = _mm_srli_epi16(src_temp3_8x16b, 8);
                //even values
                src_temp0_8x16b = _mm_slli_epi16(src_temp0_8x16b, 8);
                src_temp3_8x16b = _mm_slli_epi16(src_temp3_8x16b, 8);
                src_temp0_8x16b = _mm_srli_epi16(src_temp0_8x16b, 8);
                src_temp3_8x16b = _mm_srli_epi16(src_temp3_8x16b, 8);
                //combining odd values
                src_temp2_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp2_8x16b);
                //combining even values
                src_temp0_8x16b = _mm_packus_epi16(src_temp0_8x16b, src_temp3_8x16b);

                //saturated substract 8 bit
                tmp_set_128i_1 = _mm_sub_epi8(src_temp0_8x16b, band_pos_u_16x8b);
                tmp_set_128i_3 = _mm_sub_epi8(src_temp2_8x16b, band_pos_v_16x8b);
                //if the values less than 0 put ff
                tmp_set_128i_2 = _mm_cmpgt_epi8(sao_offset, tmp_set_128i_1);
                tmp_set_128i_4 = _mm_cmpgt_epi8(sao_offset, tmp_set_128i_3);
                tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2);
                tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4);
                //if the values greater than 31 put ff
                tmp_set_128i_2 = _mm_cmpgt_epi8(tmp_set_128i_1, cmp_msk2);
                tmp_set_128i_4 = _mm_cmpgt_epi8(tmp_set_128i_3, cmp_msk2);
                tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2);
                tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4);
                // registers reused to increase performance
                //if the values >16 then put ff ,cmp_mask = dup16(15) row 0 and row1
                src_temp1_8x16b = _mm_cmpgt_epi8(tmp_set_128i_1, cmp_mask);
                //if the values >16 then put ff ,cmp_mask = dup16(15) row 2 and  row 3
                src_temp3_8x16b = _mm_cmpgt_epi8(tmp_set_128i_3, cmp_mask);

                //values 16 to 31 for row 0 & 1 but values <16 ==0
                tmp_set_128i_2 = _mm_and_si128(tmp_set_128i_1, src_temp1_8x16b);
                // values 0 to 15 for row 0 & 1
                tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, src_temp1_8x16b);
                //values 16 to 31 for row 2 & 3 but values <16 ==0
                tmp_set_128i_4 = _mm_and_si128(tmp_set_128i_3, src_temp3_8x16b);
                // values 0 to 15 for row 2 & 3
                tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, src_temp3_8x16b);

                //values 16 to 31 for row 0 & 1 but values <16 masked to ff row 0 and row1
                src_temp1_8x16b = _mm_cmpeq_epi8(tmp_set_128i_2, sao_offset);
                //values 16 to 31 for row 0 & 1 but values <16 masked to ff row 2 and  row 3
                src_temp3_8x16b = _mm_cmpeq_epi8(tmp_set_128i_4, sao_offset);
                tmp_set_128i_2 = _mm_or_si128(tmp_set_128i_2, src_temp1_8x16b);
                tmp_set_128i_4 = _mm_or_si128(tmp_set_128i_4, src_temp3_8x16b);


                //to choose which pixel values to preserve in row 0 and row 1
                src_temp1_8x16b = _mm_cmpeq_epi8(tmp_set_128i_1, tmp_set_128i_2);
                //to choose which pixel values to preserve in row 2 and row 3
                src_temp3_8x16b = _mm_cmpeq_epi8(tmp_set_128i_3, tmp_set_128i_4);
                //values of all rows to which no offset needs to be added preserved.
                src_temp0_8x16b = _mm_and_si128(src_temp0_8x16b, src_temp1_8x16b);
                src_temp2_8x16b = _mm_and_si128(src_temp2_8x16b, src_temp3_8x16b);

                //indexing 0 - 15 bandtable indexes
                tmp_set_128i_1 = _mm_shuffle_epi8(band_table0_16x8b, tmp_set_128i_1); //U low
                tmp_set_128i_3 = _mm_shuffle_epi8(band_table1_16x8b, tmp_set_128i_3); //V low
                //indexing 16 -31 bandtable indexes
                tmp_set_128i_2 = _mm_shuffle_epi8(band_table2_16x8b, tmp_set_128i_2); //U high
                tmp_set_128i_4 = _mm_shuffle_epi8(band_table3_16x8b, tmp_set_128i_4); //V high
                // combining all offsets results
                tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2); //U
                tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4); //V
                // combing results with the pixel values
                src_temp0_8x16b = _mm_or_si128(src_temp0_8x16b, tmp_set_128i_1);
                src_temp2_8x16b = _mm_or_si128(src_temp2_8x16b, tmp_set_128i_3);
                //reorganising even and odd values
                src_temp1_8x16b = _mm_unpacklo_epi8(src_temp0_8x16b, src_temp2_8x16b);
                src_temp3_8x16b = _mm_unpackhi_epi8(src_temp0_8x16b, src_temp2_8x16b);


                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp1_8x16b);
                // row = 1
                _mm_storeu_si128((__m128i *)(pu1_src_cpy + src_strd), src_temp3_8x16b);


                pu1_src_cpy += (src_strd << 1);

            }
            pu1_src += 16;
        }

        wd_rem = wd & 0xF;
        if(wd_rem)
        {
            pu1_src_cpy = pu1_src;
            for(row = ht; row > 0; row -= 4)
            {
                //row = 0 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp0_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy));
                // row = 1
                src_temp1_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_temp2_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd));
                // row = 3
                src_temp3_8x16b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd));
                //row0 and row1 packed and row2 and row3 packed

                src_temp0_8x16b = _mm_unpacklo_epi64(src_temp0_8x16b, src_temp1_8x16b);
                src_temp3_8x16b = _mm_unpacklo_epi64(src_temp2_8x16b, src_temp3_8x16b);
                //odd values
                src_temp1_8x16b = _mm_srli_epi16(src_temp0_8x16b, 8);
                src_temp2_8x16b = _mm_srli_epi16(src_temp3_8x16b, 8);
                //even values
                src_temp0_8x16b = _mm_slli_epi16(src_temp0_8x16b, 8);
                src_temp3_8x16b = _mm_slli_epi16(src_temp3_8x16b, 8);
                src_temp0_8x16b = _mm_srli_epi16(src_temp0_8x16b, 8);
                src_temp3_8x16b = _mm_srli_epi16(src_temp3_8x16b, 8);
                //combining odd values
                src_temp2_8x16b = _mm_packus_epi16(src_temp1_8x16b, src_temp2_8x16b);
                //combining even values
                src_temp0_8x16b = _mm_packus_epi16(src_temp0_8x16b, src_temp3_8x16b);

                //saturated substract 8 bit
                tmp_set_128i_1 = _mm_sub_epi8(src_temp0_8x16b, band_pos_u_16x8b);
                tmp_set_128i_3 = _mm_sub_epi8(src_temp2_8x16b, band_pos_v_16x8b);
                //if the values less than 0 put ff
                tmp_set_128i_2 = _mm_cmpgt_epi8(sao_offset, tmp_set_128i_1);
                tmp_set_128i_4 = _mm_cmpgt_epi8(sao_offset, tmp_set_128i_3);
                tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2);
                tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4);
                //if the values greater than 31 put ff
                tmp_set_128i_2 = _mm_cmpgt_epi8(tmp_set_128i_1, cmp_msk2);
                tmp_set_128i_4 = _mm_cmpgt_epi8(tmp_set_128i_3, cmp_msk2);
                tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2);
                tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4);
                // registers reused to increase performance
                //if the values >16 then put ff ,cmp_mask = dup16(15) row 0 and row1
                src_temp1_8x16b = _mm_cmpgt_epi8(tmp_set_128i_1, cmp_mask);
                //if the values >16 then put ff ,cmp_mask = dup16(15) row 2 and  row 3
                src_temp3_8x16b = _mm_cmpgt_epi8(tmp_set_128i_3, cmp_mask);

                //values 16 to 31 for row 0 & 1 but values <16 ==0
                tmp_set_128i_2 = _mm_and_si128(tmp_set_128i_1, src_temp1_8x16b);
                // values 0 to 15 for row 0 & 1
                tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, src_temp1_8x16b);
                //values 16 to 31 for row 2 & 3 but values <16 ==0
                tmp_set_128i_4 = _mm_and_si128(tmp_set_128i_3, src_temp3_8x16b);
                // values 0 to 15 for row 2 & 3
                tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, src_temp3_8x16b);

                //values 16 to 31 for row 0 & 1 but values <16 masked to ff row 0 and row1
                src_temp1_8x16b = _mm_cmpeq_epi8(tmp_set_128i_2, sao_offset);
                //values 16 to 31 for row 0 & 1 but values <16 masked to ff row 2 and  row 3
                src_temp3_8x16b = _mm_cmpeq_epi8(tmp_set_128i_4, sao_offset);
                tmp_set_128i_2 = _mm_or_si128(tmp_set_128i_2, src_temp1_8x16b);
                tmp_set_128i_4 = _mm_or_si128(tmp_set_128i_4, src_temp3_8x16b);


                //to choose which pixel values to preserve in row 0 and row 1
                src_temp1_8x16b = _mm_cmpeq_epi8(tmp_set_128i_1, tmp_set_128i_2);
                //to choose which pixel values to preserve in row 2 and row 3
                src_temp3_8x16b = _mm_cmpeq_epi8(tmp_set_128i_3, tmp_set_128i_4);
                //values of all rows to which no offset needs to be added preserved.
                src_temp0_8x16b = _mm_and_si128(src_temp0_8x16b, src_temp1_8x16b);
                src_temp2_8x16b = _mm_and_si128(src_temp2_8x16b, src_temp3_8x16b);

                //indexing 0 - 15 bandtable indexes
                tmp_set_128i_1 = _mm_shuffle_epi8(band_table0_16x8b, tmp_set_128i_1); //U low
                tmp_set_128i_3 = _mm_shuffle_epi8(band_table1_16x8b, tmp_set_128i_3); //V low
                //indexing 16 -31 bandtable indexes
                tmp_set_128i_2 = _mm_shuffle_epi8(band_table2_16x8b, tmp_set_128i_2); //U high
                tmp_set_128i_4 = _mm_shuffle_epi8(band_table3_16x8b, tmp_set_128i_4); //V high
                // combining all offsets results
                tmp_set_128i_1 = _mm_or_si128(tmp_set_128i_1, tmp_set_128i_2); //U
                tmp_set_128i_3 = _mm_or_si128(tmp_set_128i_3, tmp_set_128i_4); //V
                // combing results with the pixel values
                src_temp0_8x16b = _mm_or_si128(src_temp0_8x16b, tmp_set_128i_1);
                src_temp2_8x16b = _mm_or_si128(src_temp2_8x16b, tmp_set_128i_3);
                //reorganising even and odd values
                src_temp1_8x16b = _mm_unpacklo_epi8(src_temp0_8x16b, src_temp2_8x16b);
                src_temp3_8x16b = _mm_unpackhi_epi8(src_temp0_8x16b, src_temp2_8x16b);
                //Getting row1 separately
                src_temp0_8x16b = _mm_srli_si128(src_temp1_8x16b, 8);
                //Getting row3 separately
                src_temp2_8x16b = _mm_srli_si128(src_temp3_8x16b, 8);

                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp1_8x16b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), src_temp0_8x16b);
                // row = 2
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd), src_temp3_8x16b);
                // row = 3
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd), src_temp2_8x16b);

                pu1_src_cpy += (src_strd << 2);

            }
            pu1_src += 16;
        }


    }
}



void ihevc_sao_edge_offset_class0_ssse3(UWORD8 *pu1_src,
                                        WORD32 src_strd,
                                        UWORD8 *pu1_src_left,
                                        UWORD8 *pu1_src_top,
                                        UWORD8 *pu1_src_top_left,
                                        UWORD8 *pu1_src_top_right,
                                        UWORD8 *pu1_src_bot_left,
                                        UWORD8 *pu1_avail,
                                        WORD8 *pi1_sao_offset,
                                        WORD32 wd,
                                        WORD32 ht)
{
    WORD32 row, col;
    UWORD8 *pu1_src_cpy, *pu1_src_left_cpy, *pu1_src_left_str, *pu1_left_tmp;
    UWORD8 au1_mask[MAX_CTB_SIZE], *au1_mask_cpy;
    UWORD8 au1_src_left_tmp[MAX_CTB_SIZE + 8];
    UWORD8 au1_src_left_tmp1[MAX_CTB_SIZE + 8];
    UWORD8 u1_avail0, u1_avail1;
    WORD32 wd_rem;
    WORD32 offset = 0;
    __m128i src_temp0_16x8b, src_temp1_16x8b;
    __m128i left0_16x8b, left1_16x8b;
    __m128i cmp_gt0_16x8b, cmp_lt0_16x8b, cmp_gt1_16x8b, cmp_lt1_16x8b;
    __m128i edge0_16x8b, edge1_16x8b;
    __m128i au1_mask8x16b;
    __m128i edge_idx_8x16b, sao_offset_8x16b;
    __m128i const2_16x8b, const0_16x8b;
    __m128i left_store_16x8b;
    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);

    au1_mask8x16b = _mm_set1_epi8(0xff);

    /* Update  top and top-left arrays */

    *pu1_src_top_left = pu1_src_top[wd - 1];

    for(col = wd; col >= 16; col -= 16)
    {
        const0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src + offset + (ht - 1) * src_strd));
        _mm_storeu_si128((__m128i *)(pu1_src_top + offset), const0_16x8b);
        offset += 16;
    }

    //setting availability mask to ff size MAX_CTB_SIZE
    for(col = 0; col < MAX_CTB_SIZE; col += 16)
        _mm_storeu_si128((__m128i *)(au1_mask + col), au1_mask8x16b);
    for(row = 0; row < ht; row++)
    {
        au1_src_left_tmp[row] = pu1_src_left[row];
    }
    edge_idx_8x16b   = _mm_loadl_epi64((__m128i *)gi1_table_edge_idx);
    sao_offset_8x16b = _mm_loadl_epi64((__m128i *)pi1_sao_offset);

    //availability mask creation
    u1_avail0 = pu1_avail[0];
    u1_avail1 = pu1_avail[1];
    au1_mask[0] = u1_avail0;
    au1_mask[wd - 1] = u1_avail1;

    const2_16x8b = _mm_set1_epi8(2);
    const0_16x8b = _mm_setzero_si128();
    pu1_src_left_cpy = au1_src_left_tmp;
    pu1_src_left_str = au1_src_left_tmp1;
    {
        au1_mask_cpy = au1_mask;
        for(col = wd; col >= 16; col -= 16)
        {
            pu1_src_cpy = pu1_src;
            au1_mask8x16b = _mm_loadu_si128((__m128i *)au1_mask_cpy);
            //pu1_src_left_cpy =au1_src_left_tmp;
            for(row = ht; row > 0; row -= 2)
            {

                left_store_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_left_cpy));
                //row = 0 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));
                // row = 1
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));

                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, left_store_16x8b, 2);
                //row 1 left
                left1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, left_store_16x8b, 15);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp1_16x8b, 15);
                //row 0 left
                left0_16x8b = _mm_alignr_epi8(src_temp0_16x8b, left_store_16x8b, 15);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 15);


                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, left0_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(left0_16x8b, src_temp0_16x8b);
                cmp_gt1_16x8b = _mm_subs_epu8(src_temp1_16x8b, left1_16x8b);
                cmp_lt1_16x8b = _mm_subs_epu8(left1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                cmp_gt1_16x8b = _mm_cmpeq_epi8(cmp_gt1_16x8b, const0_16x8b);
                cmp_lt1_16x8b = _mm_cmpeq_epi8(cmp_lt1_16x8b, const0_16x8b);
                //combining the appropriate sign change
                left0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                left1_16x8b = _mm_sub_epi8(cmp_gt1_16x8b, cmp_lt1_16x8b);

                //row = 0 right
                edge0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 1));
                // row = 1 right
                edge1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd + 1));
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, edge0_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(edge0_16x8b, src_temp0_16x8b);
                cmp_gt1_16x8b = _mm_subs_epu8(src_temp1_16x8b, edge1_16x8b);
                cmp_lt1_16x8b = _mm_subs_epu8(edge1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                cmp_gt1_16x8b = _mm_cmpeq_epi8(cmp_gt1_16x8b, const0_16x8b);
                cmp_lt1_16x8b = _mm_cmpeq_epi8(cmp_lt1_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                edge1_16x8b = _mm_sub_epi8(cmp_gt1_16x8b, cmp_lt1_16x8b);

                //combining sign-left and sign_right
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, left0_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, left1_16x8b);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);

                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //cnvert to 16 bit then add and then saturated pack
                left0_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, left0_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, left0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                left0_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, left0_16x8b);
                src_temp1_16x8b = _mm_unpackhi_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge1_16x8b, left0_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp1_16x8b);


                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storeu_si128((__m128i *)(pu1_src_cpy + src_strd), src_temp1_16x8b);

                pu1_src_cpy += (src_strd << 1);
                pu1_src_left_cpy += 2;
                pu1_src_left_str += 2;
            }
            au1_mask_cpy += 16;
            pu1_src += 16;
            pu1_src_left_cpy -= ht;
            pu1_src_left_str -= ht;

            pu1_left_tmp = pu1_src_left_cpy;
            pu1_src_left_cpy = pu1_src_left_str;
            pu1_src_left_str = pu1_left_tmp;
        }

        wd_rem = wd & 0xF;
        if(wd_rem)
        {

            cmp_gt1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src + (ht - 1) * src_strd));
            _mm_storel_epi64((__m128i *)(pu1_src_top + offset), cmp_gt1_16x8b);

            au1_mask8x16b = _mm_loadl_epi64((__m128i *)au1_mask_cpy);
            pu1_src_cpy = pu1_src;
            au1_mask8x16b = _mm_unpacklo_epi64(au1_mask8x16b, au1_mask8x16b);
            //pu1_src_left_cpy =au1_src_left_tmp;
            for(row = ht; row > 0; row -= 4)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_left_cpy));
                //row = 0 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy));
                // row = 1
                cmp_gt0_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd));
                // row  = 2
                src_temp1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd));
                // row = 3
                cmp_gt1_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd));


                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, left_store_16x8b, 4);
                //row 3 left
                edge0_16x8b = _mm_slli_si128(cmp_gt1_16x8b, 8);
                cmp_lt1_16x8b = _mm_alignr_epi8(cmp_gt1_16x8b, left_store_16x8b, 15);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, edge0_16x8b, 15);
                //row 2 left
                edge0_16x8b = _mm_slli_si128(src_temp1_16x8b, 8);
                left1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, left_store_16x8b, 15);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, edge0_16x8b, 15);
                //row 1 left
                edge0_16x8b = _mm_slli_si128(cmp_gt0_16x8b, 8);
                cmp_lt0_16x8b = _mm_alignr_epi8(cmp_gt0_16x8b, left_store_16x8b, 15);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, edge0_16x8b, 15);
                //row 0 left
                edge0_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                left0_16x8b = _mm_alignr_epi8(src_temp0_16x8b, left_store_16x8b, 15);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, edge0_16x8b, 15);

                // packing rows together for 16 SIMD operations
                src_temp0_16x8b = _mm_unpacklo_epi64(src_temp0_16x8b, cmp_gt0_16x8b);
                src_temp1_16x8b = _mm_unpacklo_epi64(src_temp1_16x8b, cmp_gt1_16x8b);
                // packing rows together for 16 SIMD operations
                left0_16x8b = _mm_unpacklo_epi64(left0_16x8b, cmp_lt0_16x8b);
                left1_16x8b = _mm_unpacklo_epi64(left1_16x8b, cmp_lt1_16x8b);

                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, left0_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(left0_16x8b, src_temp0_16x8b);
                cmp_gt1_16x8b = _mm_subs_epu8(src_temp1_16x8b, left1_16x8b);
                cmp_lt1_16x8b = _mm_subs_epu8(left1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                cmp_gt1_16x8b = _mm_cmpeq_epi8(cmp_gt1_16x8b, const0_16x8b);
                cmp_lt1_16x8b = _mm_cmpeq_epi8(cmp_lt1_16x8b, const0_16x8b);
                //combining the appropriate sign change
                left0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                left1_16x8b = _mm_sub_epi8(cmp_gt1_16x8b, cmp_lt1_16x8b);

                //row = 0 right
                edge0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 1));
                // row = 1 right
                cmp_gt0_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd + 1));
                // row = 2 right
                edge1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd + 1));
                // row = 3 right
                cmp_gt1_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd + 1));
                // packing rows together for 16 SIMD operations
                edge0_16x8b = _mm_unpacklo_epi64(edge0_16x8b, cmp_gt0_16x8b);
                edge1_16x8b = _mm_unpacklo_epi64(edge1_16x8b, cmp_gt1_16x8b);

                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, edge0_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(edge0_16x8b, src_temp0_16x8b);
                cmp_gt1_16x8b = _mm_subs_epu8(src_temp1_16x8b, edge1_16x8b);
                cmp_lt1_16x8b = _mm_subs_epu8(edge1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                cmp_gt1_16x8b = _mm_cmpeq_epi8(cmp_gt1_16x8b, const0_16x8b);
                cmp_lt1_16x8b = _mm_cmpeq_epi8(cmp_lt1_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                edge1_16x8b = _mm_sub_epi8(cmp_gt1_16x8b, cmp_lt1_16x8b);

                //combining sign-left and sign_right
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, left0_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, left1_16x8b);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //shuffle to get sao offset
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);

                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //cnvert to 16 bit then add and then saturated pack
                left0_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, left0_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, left0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                left0_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, left0_16x8b);
                src_temp1_16x8b = _mm_unpackhi_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge1_16x8b, left0_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp1_16x8b);
                //separting row 1 and row 3
                cmp_lt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);
                cmp_lt1_16x8b = _mm_srli_si128(src_temp1_16x8b, 8);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_lt0_16x8b);
                // row = 2
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd), src_temp1_16x8b);
                // row = 3
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd), cmp_lt1_16x8b);

                pu1_src_cpy += (src_strd << 2);
                pu1_src_left_cpy += 4;
                pu1_src_left_str += 4;
            }
            pu1_src += wd;
            pu1_src_left_cpy -= ht;
            pu1_src_left_str -= ht;

            pu1_left_tmp = pu1_src_left_cpy;
            pu1_src_left_cpy = pu1_src_left_str;
            pu1_src_left_str = pu1_left_tmp;
        }
        for(row = 0; row < ht; row++)
        {
            pu1_src_left[row] = pu1_src_left_cpy[row];
        }
    }
}


void ihevc_sao_edge_offset_class0_chroma_ssse3(UWORD8 *pu1_src,
                                               WORD32 src_strd,
                                               UWORD8 *pu1_src_left,
                                               UWORD8 *pu1_src_top,
                                               UWORD8 *pu1_src_top_left,
                                               UWORD8 *pu1_src_top_right,
                                               UWORD8 *pu1_src_bot_left,
                                               UWORD8 *pu1_avail,
                                               WORD8 *pi1_sao_offset_u,
                                               WORD8 *pi1_sao_offset_v,
                                               WORD32 wd,
                                               WORD32 ht)
{
    WORD32 row, col;
    UWORD8 *pu1_src_cpy, *pu1_src_left_cpy, *pu1_src_left_str, *pu1_left_tmp;
    UWORD8 au1_mask[MAX_CTB_SIZE], *au1_mask_cpy;
    UWORD8 au1_src_left_tmp[2 * (MAX_CTB_SIZE + 8)];
    UWORD8 au1_src_left_tmp1[2 * (MAX_CTB_SIZE + 8)];
    UWORD8 u1_avail0, u1_avail1;
    WORD32 wd_rem;
    WORD32 offset = 0;

    __m128i src_temp0_16x8b, src_temp1_16x8b;
    __m128i left0_16x8b, left1_16x8b;
    __m128i cmp_gt0_16x8b, cmp_lt0_16x8b;
    __m128i edge0_16x8b, edge1_16x8b;
    __m128i au1_mask8x16b;
    __m128i edge_idx_8x16b, sao_offset_8x16b;
    __m128i const2_16x8b, const0_16x8b;
    __m128i left_store_16x8b;
    __m128i chroma_offset_8x16b;
    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);

    au1_mask8x16b = _mm_set1_epi8(0xff);

    /* Update  top and top-left arrays */
    pu1_src_top_left[0] = pu1_src_top[wd - 2];
    pu1_src_top_left[1] = pu1_src_top[wd - 1];;

    for(col = wd; col >= 16; col -= 16)
    {
        const0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src + offset + (ht - 1) * src_strd));
        _mm_storeu_si128((__m128i *)(pu1_src_top + offset), const0_16x8b);
        offset += 16;
    }
    for(row = 0; row < 2 * ht; row++)
    {
        au1_src_left_tmp[row] = pu1_src_left[row];
    }
    //setting availability mask to ff size MAX_CTB_SIZE
    for(col = 0; col < MAX_CTB_SIZE; col += 16)
        _mm_storeu_si128((__m128i *)(au1_mask + col), au1_mask8x16b);

    edge_idx_8x16b   = _mm_loadl_epi64((__m128i *)gi1_table_edge_idx);
    sao_offset_8x16b = _mm_loadl_epi64((__m128i *)pi1_sao_offset_u);
    const0_16x8b = _mm_loadl_epi64((__m128i *)pi1_sao_offset_v);
    chroma_offset_8x16b = _mm_set1_epi16(0x0800);
    //availability mask creation
    u1_avail0 = pu1_avail[0];
    u1_avail1 = pu1_avail[1];
    au1_mask[0] = u1_avail0;
    au1_mask[1] = u1_avail0;
    au1_mask[wd - 1] = u1_avail1;
    au1_mask[wd - 2] = u1_avail1;
    sao_offset_8x16b = _mm_unpacklo_epi64(sao_offset_8x16b, const0_16x8b);
    const2_16x8b = _mm_set1_epi8(2);
    const0_16x8b = _mm_setzero_si128();

    {
        pu1_src_left_cpy = au1_src_left_tmp;
        pu1_src_left_str = au1_src_left_tmp1;
        au1_mask_cpy = au1_mask;
        for(col = wd; col >= 16; col -= 16)
        {
            pu1_src_cpy = pu1_src;
            au1_mask8x16b = _mm_loadu_si128((__m128i *)au1_mask_cpy);

            for(row = ht; row > 0; row -= 2)
            {

                left_store_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_left_cpy));
                //row = 0 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));
                // row = 1
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));

                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, left_store_16x8b, 4);
                //row 1 left
                left1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, left_store_16x8b, 14);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp1_16x8b, 14);
                //row 0 left
                left0_16x8b = _mm_alignr_epi8(src_temp0_16x8b, left_store_16x8b, 14);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 14);


                //separating +ve and and -ve values.row 0 left
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, left0_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(left0_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                left0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //separating +ve and and -ve values.row 1 left
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, left1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(left1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                left1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);


                //row = 0 right
                edge0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2));
                // row = 1 right
                edge1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd + 2));
                //separating +ve and and -ve values.row 0 right
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, edge0_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(edge0_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //separating +ve and and -ve values.row 1 right
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, edge1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(edge1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //combining sign-left and sign_right
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, left0_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, left1_16x8b);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, chroma_offset_8x16b);

                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //cnvert to 16 bit then add and then saturated pack
                left0_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, left0_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, left0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                left0_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, left0_16x8b);
                src_temp1_16x8b = _mm_unpackhi_epi8(src_temp1_16x8b, const0_16x8b);
                edge1_16x8b = _mm_unpackhi_epi8(edge1_16x8b, left0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, edge1_16x8b);
                src_temp1_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp1_16x8b);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storeu_si128((__m128i *)(pu1_src_cpy + src_strd), src_temp1_16x8b);

                pu1_src_cpy += (src_strd << 1);
                pu1_src_left_cpy += 4;
                pu1_src_left_str += 4;
            }
            au1_mask_cpy += 16;
            pu1_src += 16;
            pu1_src_left_cpy -= 2 * ht;
            pu1_src_left_str -= 2 * ht;

            pu1_left_tmp = pu1_src_left_cpy;
            pu1_src_left_cpy = pu1_src_left_str;
            pu1_src_left_str = pu1_left_tmp;
        }

        wd_rem = wd & 0xF;
        if(wd_rem)
        {

            cmp_gt0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src + (ht - 1) * src_strd));
            _mm_storel_epi64((__m128i *)(pu1_src_top + offset), cmp_gt0_16x8b);

            au1_mask8x16b = _mm_loadl_epi64((__m128i *)au1_mask_cpy);
            pu1_src_cpy = pu1_src;
            au1_mask8x16b = _mm_unpacklo_epi64(au1_mask8x16b, au1_mask8x16b);

            for(row = ht; row > 0; row -= 4)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_left_cpy));
                //row = 0 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy));
                // row = 1
                cmp_gt0_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd));
                // row  = 2
                src_temp1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd));
                // row = 3
                cmp_lt0_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd));


                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, left_store_16x8b, 8);
                //row 3 left
                edge0_16x8b = _mm_slli_si128(cmp_lt0_16x8b, 8);
                left0_16x8b = _mm_alignr_epi8(cmp_lt0_16x8b, left_store_16x8b, 14);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, edge0_16x8b, 14);
                //row 2 left
                edge0_16x8b = _mm_slli_si128(src_temp1_16x8b, 8);
                left1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, left_store_16x8b, 14);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, edge0_16x8b, 14);


                // packing rows together for 16 SIMD operations
                src_temp1_16x8b = _mm_unpacklo_epi64(src_temp1_16x8b, cmp_lt0_16x8b);
                left1_16x8b = _mm_unpacklo_epi64(left1_16x8b, left0_16x8b);

                //row 1 left
                edge0_16x8b = _mm_slli_si128(cmp_gt0_16x8b, 8);
                edge1_16x8b = _mm_alignr_epi8(cmp_gt0_16x8b, left_store_16x8b, 14);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, edge0_16x8b, 14);
                //row 0 left
                edge0_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                left0_16x8b = _mm_alignr_epi8(src_temp0_16x8b, left_store_16x8b, 14);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, edge0_16x8b, 14);
                // packing rows together for 16 SIMD operations
                src_temp0_16x8b = _mm_unpacklo_epi64(src_temp0_16x8b, cmp_gt0_16x8b);
                left0_16x8b = _mm_unpacklo_epi64(left0_16x8b, edge1_16x8b);

                //separating +ve and and -ve values.for row 2 and row 3
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, left1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(left1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                left1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);





                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, left0_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(left0_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                left0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);


                //row = 0 right
                edge0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 2));
                // row = 1 right
                cmp_gt0_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd + 2));
                // row = 2 right
                edge1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd + 2));
                // row = 3 right
                cmp_lt0_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd + 2));
                // packing rows together for 16 SIMD operations
                edge0_16x8b = _mm_unpacklo_epi64(edge0_16x8b, cmp_gt0_16x8b);
                edge1_16x8b = _mm_unpacklo_epi64(edge1_16x8b, cmp_lt0_16x8b);

                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, edge0_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(edge0_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, edge1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(edge1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //combining sign-left and sign_right
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, left0_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, left1_16x8b);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //shuffle to get sao offset
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, chroma_offset_8x16b);

                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //cnvert to 16 bit then add and then saturated pack
                left0_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, left0_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, left0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                left0_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, left0_16x8b);
                src_temp1_16x8b = _mm_unpackhi_epi8(src_temp1_16x8b, const0_16x8b);
                edge1_16x8b = _mm_unpackhi_epi8(edge1_16x8b, left0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, edge1_16x8b);
                src_temp1_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp1_16x8b);

                //seaprting row 1 and row 3
                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);
                cmp_lt0_16x8b = _mm_srli_si128(src_temp1_16x8b, 8);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                // row = 2
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd), src_temp1_16x8b);
                // row = 3
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd), cmp_lt0_16x8b);

                pu1_src_cpy += (src_strd << 2);
                pu1_src_left_cpy += 8;
                pu1_src_left_str += 8;
            }
            pu1_src += wd;
            pu1_src_left_cpy -= 2 * ht;
            pu1_src_left_str -= 2 * ht;

            pu1_left_tmp = pu1_src_left_cpy;
            pu1_src_left_cpy = pu1_src_left_str;
            pu1_src_left_str = pu1_left_tmp;
        }
        for(row = 0; row < 2 * ht; row++)
        {
            pu1_src_left[row] = pu1_src_left_cpy[row];
        }
    }

}


void ihevc_sao_edge_offset_class1_ssse3(UWORD8 *pu1_src,
                                        WORD32 src_strd,
                                        UWORD8 *pu1_src_left,
                                        UWORD8 *pu1_src_top,
                                        UWORD8 *pu1_src_top_left,
                                        UWORD8 *pu1_src_top_right,
                                        UWORD8 *pu1_src_bot_left,
                                        UWORD8 *pu1_avail,
                                        WORD8 *pi1_sao_offset,
                                        WORD32 wd,
                                        WORD32 ht)
{
    WORD32 row, col;
    UWORD8 *pu1_src_top_cpy;
    UWORD8 *pu1_src_cpy;
    WORD32 wd_rem;


    __m128i src_top_16x8b, src_bottom_16x8b;
    __m128i src_temp0_16x8b, src_temp1_16x8b;
    __m128i signup0_16x8b, signdwn1_16x8b;
    __m128i cmp_gt0_16x8b, cmp_lt0_16x8b;
    __m128i edge0_16x8b, edge1_16x8b;
    __m128i edge_idx_8x16b, sao_offset_8x16b;
    __m128i const2_16x8b, const0_16x8b;

    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);


    /* Updating left and top-left  */
    for(row = 0; row < ht; row++)
    {
        pu1_src_left[row] = pu1_src[row * src_strd + (wd - 1)];
    }
    *pu1_src_top_left = pu1_src_top[wd - 1];



    pu1_src_top_cpy = pu1_src_top;
    edge_idx_8x16b   = _mm_loadl_epi64((__m128i *)gi1_table_edge_idx);
    sao_offset_8x16b = _mm_loadl_epi64((__m128i *)pi1_sao_offset);

    /* Update height and source pointers based on the availability flags */
    if(0 == pu1_avail[2])
    {
        pu1_src_top_cpy = pu1_src;
        pu1_src += src_strd;
        ht--;
    }
    if(0 == pu1_avail[3])
    {
        ht--;
    }

    const2_16x8b = _mm_set1_epi8(2);
    const0_16x8b = _mm_setzero_si128();

    {
        WORD32 ht_rem;
        for(col = wd; col >= 16; col -= 16)
        {
            pu1_src_cpy = pu1_src;
            src_top_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_top_cpy + wd - col));
            //row = 0
            src_temp0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

            for(row = ht; row >= 2; row -= 2)
            {

                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));


                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_temp1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row1-row0
                edge1_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b);

                //row1 -bottom
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //combining sign-left and sign_right
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, signdwn1_16x8b);

                //for the next iteration signup0_16x8b = -signdwn1_16x8b
                signup0_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //copying the next top
                src_top_16x8b = src_temp1_16x8b;
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpackhi_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp1_16x8b);

                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storeu_si128((__m128i *)(pu1_src_cpy + src_strd), src_temp1_16x8b);

                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);
            }
            ht_rem = ht & 0x1;

            if(ht_rem)
            {
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                //current row -next row
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and botton and constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);

                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                //copying the next top
                src_top_16x8b = src_temp0_16x8b;
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
            }
            //updating top flag
            _mm_storeu_si128((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 16;
        }

        wd_rem = wd & 0xF;
        if(wd_rem)
        {
            pu1_src_cpy = pu1_src;
            src_top_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_top_cpy + wd - col));
            //row = 0
            src_temp0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy));
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
            signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);
            for(row = ht; row >= 4; row -= 4)
            {
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd));

                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_temp1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //row1-row0
                edge1_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b);
                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)
                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //row1 -row2
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)
                //packing row 0 n row 1
                src_temp0_16x8b = _mm_unpacklo_epi64(src_temp0_16x8b, src_temp1_16x8b);
                //row = 3
                src_top_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd));
                // row = 4
                src_temp1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 4 * src_strd));

                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty
                signdwn1_16x8b = _mm_slli_si128(signdwn1_16x8b, 8); //allign left (1-2)
                //separating +ve and and -ve values.(2,3)
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_top_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_bottom_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-3)

                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signdwn1_16x8b, 8); //(2-3) ,(1-2) (substract with down)
                edge1_16x8b = _mm_slli_si128(edge1_16x8b, 8);
                //separating +ve and and -ve values.(3,4)
                cmp_gt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_top_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(3-4)
                //combining sign-left and sign_right
                edge1_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge1_16x8b, 8); //(3-4),(2-3)

                edge1_16x8b = _mm_sub_epi8(edge1_16x8b, signup0_16x8b); //(3,2)

                //packing row 2 n row 3
                src_bottom_16x8b = _mm_unpacklo_epi64(src_bottom_16x8b, src_top_16x8b);
                //for the next iteration signup0_16x8b = -signdwn1_16x8b
                signup0_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b); //(4-3)

                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //the next top already in  src_top_16x8b
                //src_top_16x8b = src_temp1_16x8b;
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_bottom_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_bottom_16x8b = _mm_unpackhi_epi8(src_bottom_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                src_bottom_16x8b = _mm_add_epi16(src_bottom_16x8b, cmp_lt0_16x8b);
                src_bottom_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_bottom_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);
                cmp_lt0_16x8b = _mm_srli_si128(src_bottom_16x8b, 8);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                //row = 2
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd), src_bottom_16x8b);
                // row = 3
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd), cmp_lt0_16x8b);

                src_temp0_16x8b = src_temp1_16x8b;
                signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);
                pu1_src_cpy += (src_strd << 2);

            }
            ht_rem = ht & 0x2;
            if(ht_rem)
            {

                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd));

                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_temp1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row1-row0
                edge1_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b);
                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)
                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //row1 -row2
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty
                //for the next iteration signup0_16x8b = -signdwn1_16x8b
                signup0_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b); //(2-1) for next
                src_top_16x8b = src_temp1_16x8b;
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);

                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);

                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);

                //the next top already in  src_top_16x8b
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, src_temp1_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);

                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);

            }
            ht_rem = ht & 0x1;
            if(ht_rem)
            {

                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_bottom_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd));

                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8);
                edge0_16x8b = _mm_srli_si128(edge0_16x8b, 8);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                src_top_16x8b = src_temp0_16x8b;
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, const0_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                pu1_src_cpy += (src_strd);

            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
            }
            _mm_storel_epi64((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 8;
        }
    }
}

void ihevc_sao_edge_offset_class1_chroma_ssse3(UWORD8 *pu1_src,
                                               WORD32 src_strd,
                                               UWORD8 *pu1_src_left,
                                               UWORD8 *pu1_src_top,
                                               UWORD8 *pu1_src_top_left,
                                               UWORD8 *pu1_src_top_right,
                                               UWORD8 *pu1_src_bot_left,
                                               UWORD8 *pu1_avail,
                                               WORD8 *pi1_sao_offset_u,
                                               WORD8 *pi1_sao_offset_v,
                                               WORD32 wd,
                                               WORD32 ht)
{
    WORD32 row, col;
    UWORD8 *pu1_src_top_cpy;
    UWORD8 *pu1_src_cpy;
    WORD32 wd_rem;


    __m128i src_top_16x8b, src_bottom_16x8b;
    __m128i src_temp0_16x8b, src_temp1_16x8b;
    __m128i signup0_16x8b, signdwn1_16x8b;
    __m128i cmp_gt0_16x8b, cmp_lt0_16x8b;
    __m128i edge0_16x8b, edge1_16x8b;
    __m128i edge_idx_8x16b, sao_offset_8x16b;
    __m128i const2_16x8b, const0_16x8b;
    __m128i chroma_offset_8x16b;

    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);

    /* Updating left and top and top-left */
    for(row = 0; row < ht; row++)
    {
        pu1_src_left[2 * row] = pu1_src[row * src_strd + (wd - 2)];
        pu1_src_left[2 * row + 1] = pu1_src[row * src_strd + (wd - 1)];
    }
    pu1_src_top_left[0] = pu1_src_top[wd - 2];
    pu1_src_top_left[1] = pu1_src_top[wd - 1];



    pu1_src_top_cpy = pu1_src_top;
    edge_idx_8x16b   = _mm_loadl_epi64((__m128i *)gi1_table_edge_idx);
    sao_offset_8x16b = _mm_loadl_epi64((__m128i *)pi1_sao_offset_u);
    const0_16x8b = _mm_loadl_epi64((__m128i *)pi1_sao_offset_v);
    chroma_offset_8x16b = _mm_set1_epi16(0x0800);
    /* Update height and source pointers based on the availability flags */
    if(0 == pu1_avail[2])
    {
        pu1_src_top_cpy = pu1_src;
        pu1_src += src_strd;
        ht--;
    }
    if(0 == pu1_avail[3])
    {
        ht--;
    }
    sao_offset_8x16b = _mm_unpacklo_epi64(sao_offset_8x16b, const0_16x8b);
    const2_16x8b = _mm_set1_epi8(2);
    const0_16x8b = _mm_setzero_si128();


    {
        WORD32 ht_rem;



        for(col = wd; col >= 16; col -= 16)
        {
            pu1_src_cpy = pu1_src;
            src_top_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_top_cpy + wd - col));
            //row = 0
            src_temp0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

            for(row = ht; row >= 2; row -= 2)
            {

                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));


                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_temp1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row1-row0
                edge1_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b);

                //row1 -bottom
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //combining sign-left and sign_right
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, signdwn1_16x8b);

                //for the next iteration signup0_16x8b = -signdwn1_16x8b
                signup0_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //copying the next top
                src_top_16x8b = src_temp1_16x8b;


                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, chroma_offset_8x16b);

                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpackhi_epi8(src_temp1_16x8b, const0_16x8b);
                edge1_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, edge1_16x8b);
                src_temp1_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp1_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storeu_si128((__m128i *)(pu1_src_cpy + src_strd), src_temp1_16x8b);

                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);
            }
            ht_rem = ht & 0x1;

            if(ht_rem)
            {
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                //current row -next row
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and botton and constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                //copying the next top
                src_top_16x8b = src_temp0_16x8b;

                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);

                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
            }
            //updating top flag
            _mm_storeu_si128((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 16;
        }

        wd_rem = wd & 0xF;
        if(wd_rem)
        {
            pu1_src_cpy = pu1_src;
            src_top_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_top_cpy + wd - col));
            //row = 0
            src_temp0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy));
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
            signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);
            for(row = ht; row >= 4; row -= 4)
            {
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd));

                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_temp1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //row1-row0
                edge1_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b);
                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)
                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //row1 -row2
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)
                //packing row 0 n row 1
                src_temp0_16x8b = _mm_unpacklo_epi64(src_temp0_16x8b, src_temp1_16x8b);
                //row = 3
                src_top_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd));
                // row = 4
                src_temp1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 4 * src_strd));

                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty
                signdwn1_16x8b = _mm_slli_si128(signdwn1_16x8b, 8); //allign left (1-2)
                //separating +ve and and -ve values.(2,3)
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_top_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_bottom_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-3)

                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signdwn1_16x8b, 8); //(2-3) ,(1-2) (substract with down)
                edge1_16x8b = _mm_slli_si128(edge1_16x8b, 8);
                //separating +ve and and -ve values.(3,4)
                cmp_gt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_top_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(3-4)
                //combining sign-left and sign_right
                edge1_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge1_16x8b, 8); //(3-4),(2-3)

                edge1_16x8b = _mm_sub_epi8(edge1_16x8b, signup0_16x8b); //(3,2)

                //packing row 2 n row 3
                src_bottom_16x8b = _mm_unpacklo_epi64(src_bottom_16x8b, src_top_16x8b);
                //for the next iteration signup0_16x8b = -signdwn1_16x8b
                signup0_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b); //(4-3)
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, chroma_offset_8x16b);

                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //the next top already in  src_top_16x8b
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_bottom_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_bottom_16x8b = _mm_unpackhi_epi8(src_bottom_16x8b, const0_16x8b);
                edge1_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_bottom_16x8b = _mm_add_epi16(src_bottom_16x8b, edge1_16x8b);
                src_bottom_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_bottom_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);
                cmp_lt0_16x8b = _mm_srli_si128(src_bottom_16x8b, 8);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                //row = 2
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd), src_bottom_16x8b);
                // row = 3
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd), cmp_lt0_16x8b);

                src_temp0_16x8b = src_temp1_16x8b;
                signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);
                pu1_src_cpy += (src_strd << 2);

            }
            ht_rem = ht & 0x2;
            if(ht_rem)
            {

                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadl_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd));

                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_temp1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row1-row0
                edge1_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b);
                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)
                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //row1 -row2
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty
                //for the next iteration signup0_16x8b = -signdwn1_16x8b
                signup0_16x8b = _mm_sub_epi8(cmp_lt0_16x8b, cmp_gt0_16x8b); //(2-1) for next
                src_top_16x8b = src_temp1_16x8b;

                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);

                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);

                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                //the next top already in  src_top_16x8b
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, src_temp1_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);

                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);

            }
            ht_rem = ht & 0x1;
            if(ht_rem)
            {

                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_bottom_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy + src_strd));

                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                src_top_16x8b = src_temp0_16x8b;

                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8);
                edge0_16x8b = _mm_srli_si128(edge0_16x8b, 8);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);

                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, const0_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                pu1_src_cpy += (src_strd);

            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
            }
            _mm_storel_epi64((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 8;
        }
    }
}

/* 135 degree filtering */
void ihevc_sao_edge_offset_class2_ssse3(UWORD8 *pu1_src,
                                        WORD32 src_strd,
                                        UWORD8 *pu1_src_left,
                                        UWORD8 *pu1_src_top,
                                        UWORD8 *pu1_src_top_left,
                                        UWORD8 *pu1_src_top_right,
                                        UWORD8 *pu1_src_bot_left,
                                        UWORD8 *pu1_avail,
                                        WORD8 *pi1_sao_offset,
                                        WORD32 wd,
                                        WORD32 ht)
{
    WORD32 row, col;
    UWORD8 *pu1_src_top_cpy, *pu1_src_left_cpy, *pu1_src_left_cpy2;
    UWORD8 *pu1_left_tmp, *pu1_src_left_str, *pu1_src_left_str2;
    UWORD8 *pu1_firstleft;
    UWORD8 *pu1_src_cpy, *pu1_src_org;
    UWORD8 au1_mask[MAX_CTB_SIZE], *au1_mask_cpy;
    UWORD8 au1_src_left_tmp[MAX_CTB_SIZE + 8];
    UWORD8 au1_src_left_tmp1[MAX_CTB_SIZE + 8];
    WORD32 wd_rem;
    UWORD8 u1_pos_0_0_tmp, u1_pos_wd_ht_tmp;
    WORD32 ht_tmp, ht_0;

    WORD32 bit_depth;
    UWORD8 u1_avail0, u1_avail1;

    __m128i src_top_16x8b, src_bottom_16x8b;
    __m128i src_temp0_16x8b, src_temp1_16x8b;
    __m128i signup0_16x8b, signdwn1_16x8b;
    __m128i cmp_gt0_16x8b, cmp_lt0_16x8b;
    __m128i edge0_16x8b, edge1_16x8b;
    __m128i au1_mask8x16b;
    __m128i edge_idx_8x16b, sao_offset_8x16b;
    __m128i const2_16x8b, const0_16x8b;
    __m128i left_store_16x8b;
    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);

    ht_0 = ht; ht_tmp = ht;
    au1_mask8x16b = _mm_set1_epi8(0xff);

    //setting availability mask to ff size MAX_CTB_SIZE
    for(col = 0; col < MAX_CTB_SIZE; col += 16)
        _mm_storeu_si128((__m128i *)(au1_mask + col), au1_mask8x16b);
    for(row = 0; row < ht; row++)
    {
        au1_src_left_tmp[row] = pu1_src_left[row];
    }
    bit_depth = BIT_DEPTH_LUMA;
    pu1_src_org = pu1_src;
    pu1_src_top_cpy = pu1_src_top;
    pu1_src_left_cpy2 = au1_src_left_tmp;
    pu1_src_left_cpy = au1_src_left_tmp;
    pu1_src_left_str2 = au1_src_left_tmp1;
    pu1_src_left_str = au1_src_left_tmp1;
    edge_idx_8x16b   = _mm_loadl_epi64((__m128i *)gi1_table_edge_idx);
    sao_offset_8x16b = _mm_loadl_epi64((__m128i *)pi1_sao_offset);


    /* If top-left is available, process separately */
    if(0 != pu1_avail[4])
    {
        WORD8 edge_idx;

        edge_idx = 2 + SIGN(pu1_src[0] - pu1_src_top_left[0]) +
                        SIGN(pu1_src[0] - pu1_src[1 + src_strd]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_0_tmp = CLIP3(pu1_src[0] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_0_tmp = pu1_src[0];
        }
    }
    else
    {
        u1_pos_0_0_tmp = pu1_src[0];
    }

    /* If bottom-right is available, process separately */
    if(0 != pu1_avail[7])
    {
        WORD8 edge_idx;

        edge_idx = 2 + SIGN(pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd - 1 - src_strd]) +
                        SIGN(pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd + 1 + src_strd]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_ht_tmp = CLIP3(pu1_src[wd - 1 + (ht - 1) * src_strd] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_ht_tmp = pu1_src[wd - 1 + (ht - 1) * src_strd];
        }
    }
    else
    {
        u1_pos_wd_ht_tmp = pu1_src[wd - 1 + (ht - 1) * src_strd];
    }
    pu1_firstleft = pu1_src_top_left;

    /* Update height and source pointers based on the availability flags */
    if(0 == pu1_avail[2])
    {
        pu1_firstleft = pu1_src_left_cpy2;
        pu1_src_left_cpy2++;
        pu1_src_left_str2++;
        pu1_src_top_cpy = pu1_src;
        pu1_src += src_strd;
        ht--;
    }
    if(0 == pu1_avail[3])
    {
        ht--;
        ht_0--;
    }
    //storing top left in a mmx register
    left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_firstleft);
    const2_16x8b = _mm_set1_epi8(2);
    const0_16x8b = _mm_setzero_si128();
    left_store_16x8b = _mm_slli_si128(left_store_16x8b, 15);
    //update top -left
    *pu1_src_top_left = pu1_src_top[wd - 1];
    //availability mask creation
    u1_avail0 = pu1_avail[0];
    u1_avail1 = pu1_avail[1];
    au1_mask[0] = u1_avail0;
    au1_mask[wd - 1] = u1_avail1;
    {
        WORD32 ht_rem;


        pu1_src_left_cpy = pu1_src_left_cpy2;
        pu1_src_left_str = pu1_src_left_str2;
        au1_mask_cpy = au1_mask;
        for(col = wd; col >= 16; col -= 16)
        {
            pu1_src_cpy = pu1_src;
            src_top_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_top_cpy + wd - col));
            //row = 0
            src_temp0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));
            src_top_16x8b = _mm_alignr_epi8(src_top_16x8b, left_store_16x8b, 15);
            //loading the mask
            au1_mask8x16b = _mm_loadu_si128((__m128i *)au1_mask_cpy);
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);


            for(row = ht; row >= 2; row -= 2)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 1 right
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd + 1));
                //to insert left in row 0
                signdwn1_16x8b = _mm_slli_si128(left_store_16x8b, 15);
                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp0_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //manipulation for row 1 - row 0
                signdwn1_16x8b = _mm_alignr_epi8(src_temp0_16x8b, signdwn1_16x8b, 15);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(0-1)
                //row1-row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                // row = 2 right
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd + 1));
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-0)


                //row1 -bottom
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                // row = 2
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));

                //combining sign-left and sign_right
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);

                //storing the row 1 left for next row.
                signup0_16x8b = _mm_slli_si128(left_store_16x8b, 14);

                //combining sign-left and sign_right
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, signdwn1_16x8b);
                //manipulation for bottom - row 1
                signup0_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signup0_16x8b, 15);
                //eliminating old left for row 0 and row 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 2);
                //bottom - row1
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signup0_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signup0_16x8b, src_bottom_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //for the next iteration bottom -row1
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row1  getting it right for left of next block
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp1_16x8b, 15);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //row0  getting it right for left of next block
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 15);
                //copying the next top
                src_top_16x8b = src_temp1_16x8b;
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpackhi_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp1_16x8b);

                //store left boundary
                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storeu_si128((__m128i *)(pu1_src_cpy + src_strd), src_temp1_16x8b);

                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);
                pu1_src_left_cpy += 2;
                pu1_src_left_str += 2;
            }
            ht_rem = ht & 0x1;

            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd + 1));
                //current row -next row
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and botton and constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                //eliminating old left for row 0 and row 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 1);

                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);

                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);

                //row0  getting it right for left of next block
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 15);
                //copying the next top
                src_top_16x8b = src_temp0_16x8b;
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);
                //store left boundary
                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);

                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                pu1_src_cpy += (src_strd);
                pu1_src_left_cpy += 1;
                pu1_src_left_str += 1;
            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
                pu1_src_left_str[0] = pu1_src_cpy[15];
            }
            if(0 == pu1_avail[2])
            {
                pu1_src_left_str[-ht_0] = pu1_src[15 - src_strd];
            }

            //for the top left of next part of the block
            left_store_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_top_cpy + wd - col));
            //updating top flag
            _mm_storeu_si128((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 16;
            au1_mask_cpy += 16;


            pu1_left_tmp = pu1_src_left_cpy2;
            pu1_src_left_cpy2 = pu1_src_left_str2;
            pu1_src_left_str2 = pu1_left_tmp;

            pu1_src_left_cpy = pu1_src_left_cpy2;
            pu1_src_left_str = pu1_src_left_str2;
        }

        wd_rem = wd & 0xF;
        if(wd_rem)
        {
            pu1_src_left_cpy = pu1_src_left_cpy2;
            pu1_src_left_str = pu1_src_left_str2;
            pu1_src_cpy = pu1_src;
            src_top_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_top_cpy + wd - col));
            //row = 0
            src_temp0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy));
            src_top_16x8b = _mm_alignr_epi8(src_top_16x8b, left_store_16x8b, 15);
            au1_mask8x16b = _mm_loadl_epi64((__m128i *)au1_mask_cpy); //????
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //preparing au1_mask
            au1_mask8x16b = _mm_unpacklo_epi64(au1_mask8x16b, au1_mask8x16b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
            signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);

            for(row = ht; row >= 4; row -= 4)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));
                //right row1
                signdwn1_16x8b = _mm_srli_si128(src_temp1_16x8b, 1);
                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);
                //manipulation for row 1 -row 0
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 15);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //row 0 left
                signdwn1_16x8b = _mm_alignr_epi8(src_temp0_16x8b, signdwn1_16x8b, 15);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row 1 -row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //row1-row0
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)

                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //right row2
                signdwn1_16x8b = _mm_srli_si128(src_bottom_16x8b, 1);
                //packing row 0 n row 1
                src_temp0_16x8b = _mm_unpacklo_epi64(src_temp0_16x8b, src_temp1_16x8b);
                //row1 -row2
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)
                //manipulation for row 2 -row 1
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 14);
                //row 1 left
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 15);
                //row = 3
                src_top_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 3 * src_strd));

                // row = 4
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 4 * src_strd));

                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty

                //separating +ve and and -ve values.(2,1)
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);
                //manipulation for row 3 -row 2
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 13);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //row 2 left
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 15);
                //combining the appropriate sign change
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-1)

                //separating +ve and and -ve values.(3,2)
                cmp_gt0_16x8b = _mm_subs_epu8(src_top_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_top_16x8b);
                signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8); //aligned left (2-1)
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //right row3
                signdwn1_16x8b = _mm_srli_si128(src_top_16x8b, 1);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(3-2)

                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(3-2) ,(2-1)

                //separating +ve and and -ve values.(2,3)
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);
                //right row 4
                signdwn1_16x8b =  _mm_srli_si128(src_temp1_16x8b, 1);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-3)

                //separating +ve and and -ve values.(3,bottom)
                cmp_gt0_16x8b = _mm_subs_epu8(src_top_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_top_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                edge1_16x8b = _mm_slli_si128(edge1_16x8b, 8); //aligned left (2-3)
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(3-bottom)
                edge1_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge1_16x8b, 8); //(3-bottom),(2-3)

                //manipulation for bottom -row 3
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 12);
                //eliminating old left for row 0,1,2,3
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 4);
                //packing row 2 n row 3
                src_bottom_16x8b = _mm_unpacklo_epi64(src_bottom_16x8b, src_top_16x8b);
                //row 3 left
                signdwn1_16x8b = _mm_alignr_epi8(src_top_16x8b, signdwn1_16x8b, 15);
                //loading row 3 right into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_bottom_16x8b, 15);
                //adding bottom and top values of row 2 and row 3
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, signup0_16x8b); //(3,2)
                //separating +ve and and -ve values.(botttom,3)
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);
                //to store right of row 2
                signdwn1_16x8b = _mm_slli_si128(src_bottom_16x8b, 8);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(bottom -3) for next iteration

                //storing right of row 2into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 15);
                //to store right of row 0
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                //storing right of row 1 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 15);

                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);

                //storing right of row 0 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 15);
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_bottom_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_bottom_16x8b = _mm_unpackhi_epi8(src_bottom_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                src_bottom_16x8b = _mm_add_epi16(src_bottom_16x8b, cmp_lt0_16x8b);
                src_bottom_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_bottom_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);
                cmp_lt0_16x8b = _mm_srli_si128(src_bottom_16x8b, 8);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                //row = 2
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd), src_bottom_16x8b);
                // row = 3
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd), cmp_lt0_16x8b);

                src_temp0_16x8b = src_temp1_16x8b;
                signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);
                pu1_src_cpy += (src_strd << 2);
                pu1_src_left_cpy += 4;
                pu1_src_left_str += 4;
            }
            ht_rem = ht & 0x2;
            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));

                //row 0 -row 1
                signdwn1_16x8b = _mm_srli_si128(src_temp1_16x8b, 1);
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);
                //manipulation for row 1 -row 0
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 15);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //manipulation for row 1 - row 0
                signdwn1_16x8b = _mm_alignr_epi8(src_temp0_16x8b, signdwn1_16x8b, 15);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //row1-row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign chang
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row 1 -bottom
                signdwn1_16x8b = _mm_srli_si128(src_bottom_16x8b, 1);

                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)
                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //row1 -bottom
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)
                //manipulation for bottom -row1
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 14);
                //manipulation for bottom- row 1
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 15);
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty
                //bottom - row 1
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);

                //eliminating old left for row 0,1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 2);
                signdwn1_16x8b = _mm_slli_si128(src_temp1_16x8b, 8);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //for the next iteration signup0_16x8b
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-1) for next

                //storing right of row 1 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 15);
                //for storing right of row 1
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);

                src_top_16x8b = src_temp1_16x8b;
                //storing right of row 0 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 15);

                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);

                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);

                //the next top already in  src_top_16x8b
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, src_temp1_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);
                pu1_src_left_cpy += 2;
                pu1_src_left_str += 2;
            }
            ht_rem = ht & 0x1;
            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                //left store manipulation 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 1);
                //row 0 -row1
                signdwn1_16x8b = _mm_srli_si128(src_bottom_16x8b, 1);
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                //for row 0 right to put into left store
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8);
                edge0_16x8b = _mm_srli_si128(edge0_16x8b, 8);
                //filling the left boundary value
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 15);

                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                src_top_16x8b = src_temp0_16x8b;
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, const0_16x8b);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                pu1_src_cpy += (src_strd);
                pu1_src_left_cpy += 1;
                pu1_src_left_str += 1;
            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
                pu1_src_left_str[0] = pu1_src_cpy[7];
            }

            if(0 == pu1_avail[2])
            {
                pu1_src_left_str[-ht_0] = pu1_src[7 - src_strd];
            }

            _mm_storel_epi64((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 8;
            au1_mask_cpy += 16;

            pu1_left_tmp = pu1_src_left_cpy2;
            pu1_src_left_cpy2 = pu1_src_left_str2;
            pu1_src_left_str2 = pu1_left_tmp;

            pu1_src_left_cpy = pu1_src_left_cpy2;
            pu1_src_left_str = pu1_src_left_str2;
        }
        pu1_src_org[0] = u1_pos_0_0_tmp;
        pu1_src_org[wd - 1 + (ht_tmp - 1) * src_strd] = u1_pos_wd_ht_tmp;
        pu1_src_left_cpy = (0 == pu1_avail[2]) ? (pu1_src_left_cpy - 1) : pu1_src_left_cpy;
        for(row = 0; row < ht_tmp; row++)
        {
            pu1_src_left[row] = pu1_src_left_cpy[row];
        }
    }

}

/* 135 degree filtering */
void ihevc_sao_edge_offset_class2_chroma_ssse3(UWORD8 *pu1_src,
                                               WORD32 src_strd,
                                               UWORD8 *pu1_src_left,
                                               UWORD8 *pu1_src_top,
                                               UWORD8 *pu1_src_top_left,
                                               UWORD8 *pu1_src_top_right,
                                               UWORD8 *pu1_src_bot_left,
                                               UWORD8 *pu1_avail,
                                               WORD8 *pi1_sao_offset_u,
                                               WORD8 *pi1_sao_offset_v,
                                               WORD32 wd,
                                               WORD32 ht)
{
    WORD32 row, col;
    UWORD8 *pu1_src_top_cpy, *pu1_src_left_cpy, *pu1_src_left_cpy2;
    UWORD8 *pu1_left_tmp, *pu1_src_left_str, *pu1_src_left_str2;
    UWORD8 *pu1_firstleft;
    UWORD8 *pu1_src_cpy, *pu1_src_org;
    UWORD8 au1_mask[MAX_CTB_SIZE], *au1_mask_cpy;
    UWORD8 au1_src_left_tmp[2 * (MAX_CTB_SIZE + 8)];
    UWORD8 au1_src_left_tmp1[2 * (MAX_CTB_SIZE + 8)];
    WORD32 wd_rem;
    UWORD8 u1_pos_0_0_tmp_u, u1_pos_0_0_tmp_v, u1_pos_wd_ht_tmp_u, u1_pos_wd_ht_tmp_v;
    WORD32 ht_tmp;
    WORD32 ht_0;

    WORD32 bit_depth;
    UWORD8 u1_avail0, u1_avail1;

    __m128i src_temp0_16x8b, src_temp1_16x8b;
    __m128i signup0_16x8b, signdwn1_16x8b;
    __m128i cmp_gt0_16x8b, cmp_lt0_16x8b;
    __m128i edge0_16x8b, edge1_16x8b;
    __m128i src_top_16x8b, src_bottom_16x8b;
    __m128i au1_mask8x16b;
    __m128i edge_idx_8x16b, sao_offset_8x16b;
    __m128i const2_16x8b, const0_16x8b;
    __m128i left_store_16x8b;
    __m128i chroma_offset_8x16b;

    UNUSED(pu1_src_top_right);
    UNUSED(pu1_src_bot_left);

    ht_0 = ht; ht_tmp = ht;
    au1_mask8x16b = _mm_set1_epi8(0xff);
    /* Updating left and top-left  */
    for(row = 0; row < 2 * ht; row++)
    {
        au1_src_left_tmp[row] = pu1_src_left[row];
    }
    //setting availability mask to ff size MAX_CTB_SIZE
    for(col = 0; col < MAX_CTB_SIZE; col += 16)
        _mm_storeu_si128((__m128i *)(au1_mask + col), au1_mask8x16b);
    bit_depth = BIT_DEPTH_LUMA;
    pu1_src_org = pu1_src;
    pu1_src_top_cpy = pu1_src_top;
    pu1_src_left_cpy2 = au1_src_left_tmp;
    pu1_src_left_cpy = au1_src_left_tmp;
    pu1_src_left_str2 = au1_src_left_tmp1;
    pu1_src_left_str = au1_src_left_tmp1;
    edge_idx_8x16b   = _mm_loadl_epi64((__m128i *)gi1_table_edge_idx);
    sao_offset_8x16b = _mm_loadl_epi64((__m128i *)pi1_sao_offset_u);
    const0_16x8b = _mm_loadl_epi64((__m128i *)pi1_sao_offset_v);
    chroma_offset_8x16b = _mm_set1_epi16(0x0800);

    /* If top-left is available, process separately */
    if(0 != pu1_avail[4])
    {
        WORD32 edge_idx;

        /* U */
        edge_idx = 2 + SIGN(pu1_src[0] - pu1_src_top_left[0]) +
                        SIGN(pu1_src[0] - pu1_src[2 + src_strd]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_0_tmp_u = CLIP3(pu1_src[0] + pi1_sao_offset_u[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_0_tmp_u = pu1_src[0];
        }

        /* V */
        edge_idx = 2 + SIGN(pu1_src[1] - pu1_src_top_left[1]) +
                        SIGN(pu1_src[1] - pu1_src[1 + 2 + src_strd]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_0_tmp_v = CLIP3(pu1_src[1] + pi1_sao_offset_v[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_0_tmp_v = pu1_src[1];
        }
    }
    else
    {
        u1_pos_0_0_tmp_u = pu1_src[0];
        u1_pos_0_0_tmp_v = pu1_src[1];
    }

    /* If bottom-right is available, process separately */
    if(0 != pu1_avail[7])
    {
        WORD32 edge_idx;

        /* U */
        edge_idx = 2 + SIGN(pu1_src[wd - 2 + (ht - 1) * src_strd] - pu1_src[wd - 2 + (ht - 1) * src_strd - 2 - src_strd]) +
                        SIGN(pu1_src[wd - 2 + (ht - 1) * src_strd] - pu1_src[wd - 2 + (ht - 1) * src_strd + 2 + src_strd]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_ht_tmp_u = CLIP3(pu1_src[wd - 2 + (ht - 1) * src_strd] + pi1_sao_offset_u[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_ht_tmp_u = pu1_src[wd - 2 + (ht - 1) * src_strd];
        }

        /* V */
        edge_idx = 2 + SIGN(pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd - 2 - src_strd]) +
                        SIGN(pu1_src[wd - 1 + (ht - 1) * src_strd] - pu1_src[wd - 1 + (ht - 1) * src_strd + 2 + src_strd]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_ht_tmp_v = CLIP3(pu1_src[wd - 1 + (ht - 1) * src_strd] + pi1_sao_offset_v[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_ht_tmp_v = pu1_src[wd - 1 + (ht - 1) * src_strd];
        }
    }
    else
    {
        u1_pos_wd_ht_tmp_u = pu1_src[wd - 2 + (ht - 1) * src_strd];
        u1_pos_wd_ht_tmp_v = pu1_src[wd - 1 + (ht - 1) * src_strd];
    }
    pu1_firstleft = pu1_src_top_left;

    /* Update height and source pointers based on the availability flags */
    if(0 == pu1_avail[2])
    {
        pu1_firstleft = pu1_src_left_cpy2;
        pu1_src_left_cpy2 += 2;
        pu1_src_left_str2 += 2;
        pu1_src_top_cpy = pu1_src;
        pu1_src += src_strd;
        ht--;
    }
    if(0 == pu1_avail[3])
    {
        ht--;
        ht_0--;
    }
    //storing top left in a mmx register
    left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_firstleft);
    sao_offset_8x16b = _mm_unpacklo_epi64(sao_offset_8x16b, const0_16x8b);
    const2_16x8b = _mm_set1_epi8(2);
    const0_16x8b = _mm_setzero_si128();
    left_store_16x8b = _mm_slli_si128(left_store_16x8b, 14);

    //availability mask creation
    u1_avail0 = pu1_avail[0];
    u1_avail1 = pu1_avail[1];
    au1_mask[0] = u1_avail0;
    au1_mask[1] = u1_avail0;
    au1_mask[wd - 1] = u1_avail1;
    au1_mask[wd - 2] = u1_avail1;

    /* top-left arrays */
    pu1_src_top_left[0] = pu1_src_top[wd - 2];
    pu1_src_top_left[1] = pu1_src_top[wd - 1];
    {
        WORD32 ht_rem;
        au1_mask_cpy = au1_mask;

        pu1_src_left_cpy = pu1_src_left_cpy2;
        pu1_src_left_str = pu1_src_left_str2;
        for(col = wd; col >= 16; col -= 16)
        {
            pu1_src_cpy = pu1_src;
            src_top_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_top_cpy + wd - col));
            //row = 0
            src_temp0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));
            src_top_16x8b = _mm_alignr_epi8(src_top_16x8b, left_store_16x8b, 14);
            //loading the mask
            au1_mask8x16b = _mm_loadu_si128((__m128i *)au1_mask_cpy);
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);


            for(row = ht; row >= 2; row -= 2)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 1 right
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd + 2));
                //to insert left in row 0
                signdwn1_16x8b = _mm_slli_si128(left_store_16x8b, 14);
                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp0_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //manipulation for row 1 - row 0
                signdwn1_16x8b = _mm_alignr_epi8(src_temp0_16x8b, signdwn1_16x8b, 14);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(0-1)
                //row1-row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                 // row = 2 right
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd + 2));
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-0)


                //row1 -bottom
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                // row = 2
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));

                //combining sign-left and sign_right
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);

                //storing the row 1 left for next row.
                signup0_16x8b = _mm_slli_si128(left_store_16x8b, 12);

                //combining sign-left and sign_right
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, signdwn1_16x8b);
                //manipulation for bottom - row 1
                signup0_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signup0_16x8b, 14);
                //eliminating old left for row 0 and row 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 4);
                //bottom - row1
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signup0_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signup0_16x8b, src_bottom_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //for the next iteration bottom -row1
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row1  getting it right for left of next iteration
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp1_16x8b, 14);
                //copying the next top
                src_top_16x8b = src_temp1_16x8b;
                //row0  getting its right for left of next iteration.
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 14);


                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, chroma_offset_8x16b);


                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpackhi_epi8(src_temp1_16x8b, const0_16x8b);
                edge1_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, edge1_16x8b);
                src_temp1_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp1_16x8b);

                //store left boundary
                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storeu_si128((__m128i *)(pu1_src_cpy + src_strd), src_temp1_16x8b);

                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);
                pu1_src_left_cpy += 4;
                pu1_src_left_str += 4;
            }
            ht_rem = ht & 0x1;

            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd + 2));
                //current row -next row
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and botton and constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);

                //eliminating old left for row 0 and row 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 2);
                //copying the next top
                src_top_16x8b = src_temp0_16x8b;
                //row0  getting it right for left of next block
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 14);

                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);

                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);

                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);

                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                pu1_src_cpy += (src_strd);
                pu1_src_left_cpy += 2;
                pu1_src_left_str += 2;
            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
                pu1_src_left_str[1] = pu1_src_cpy[15];
                pu1_src_left_str[0] = pu1_src_cpy[14];
            }
            if(0 == pu1_avail[2])
            {
                pu1_src_left_str[-2 * ht_0] = pu1_src[14 - src_strd];
                pu1_src_left_str[-2 * ht_0 + 1] = pu1_src[15 - src_strd];
            }

            //for the top left of next part of the block
            left_store_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_top_cpy + wd - col));
            //updating top flag
            _mm_storeu_si128((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 16;
            au1_mask_cpy += 16;

            pu1_left_tmp = pu1_src_left_cpy2;
            pu1_src_left_cpy2 = pu1_src_left_str2;
            pu1_src_left_str2 = pu1_left_tmp;

            pu1_src_left_cpy = pu1_src_left_cpy2;
            pu1_src_left_str = pu1_src_left_str2;
        }
        wd_rem = wd & 0xF;
        if(wd_rem)
        {
            pu1_src_left_cpy = pu1_src_left_cpy2;
            pu1_src_left_str = pu1_src_left_str2;
            pu1_src_cpy = pu1_src;
            src_top_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_top_cpy + wd - col));
            //row = 0
            src_temp0_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_cpy));
            src_top_16x8b = _mm_alignr_epi8(src_top_16x8b, left_store_16x8b, 14);
            au1_mask8x16b = _mm_loadl_epi64((__m128i *)au1_mask_cpy); //????
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //preparing au1_mask
            au1_mask8x16b = _mm_unpacklo_epi64(au1_mask8x16b, au1_mask8x16b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
            signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);

            for(row = ht; row >= 4; row -= 4)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));
                //right row1
                signdwn1_16x8b = _mm_srli_si128(src_temp1_16x8b, 2);
                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);
                //manipulation for row 1 -row 0
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 14);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //row 0 left
                signdwn1_16x8b = _mm_alignr_epi8(src_temp0_16x8b, signdwn1_16x8b, 14);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row 1 -row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //row1-row0
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)

                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //right row2
                signdwn1_16x8b = _mm_srli_si128(src_bottom_16x8b, 2);
                //packing row 0 n row 1
                src_temp0_16x8b = _mm_unpacklo_epi64(src_temp0_16x8b, src_temp1_16x8b);
                //row1 -row2
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)
                //manipulation for row 2 -row 1
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 12);
                //row 1 left
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 14);
                //row = 3
                src_top_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 3 * src_strd));

                // row = 4
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 4 * src_strd));

                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty

                //separating +ve and and -ve values.(2,1)
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);
                //manipulation for row 3 -row 2
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 10);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //row 2 left
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 14);
                //combining the appropriate sign change
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-1)

                //separating +ve and and -ve values.(3,2)
                cmp_gt0_16x8b = _mm_subs_epu8(src_top_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_top_16x8b);
                signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8); //aligned left (2-1)
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //right row3
                signdwn1_16x8b = _mm_srli_si128(src_top_16x8b, 2);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(3-2)

                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(3-2) ,(2-1)

                //separating +ve and and -ve values.(2,3)
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);
                //right row 4
                signdwn1_16x8b =  _mm_srli_si128(src_temp1_16x8b, 2);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-3)

                //separating +ve and and -ve values.(3,bottom)
                cmp_gt0_16x8b = _mm_subs_epu8(src_top_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_top_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                edge1_16x8b = _mm_slli_si128(edge1_16x8b, 8); //aligned left (2-3)
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(3-bottom)
                edge1_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge1_16x8b, 8); //(3-bottom),(2-3)

                //manipulation for bottom -row 3
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 8);
                //eliminating old left for row 0,1,2,3
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 8);
                //packing row 2 n row 3
                src_bottom_16x8b = _mm_unpacklo_epi64(src_bottom_16x8b, src_top_16x8b);
                //row 3 left
                signdwn1_16x8b = _mm_alignr_epi8(src_top_16x8b, signdwn1_16x8b, 14);

                //adding bottom and top values of row 2 and row 3
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, signup0_16x8b); //(3,2)
                //separating +ve and and -ve values.(botttom,3)
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(bottom -3) for next iteration

                //to store right of row 2
                signdwn1_16x8b = _mm_slli_si128(src_bottom_16x8b, 8);
                //loading row 3 right into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_bottom_16x8b, 14);
                //storing right of row 2into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 14);
                //to store right of row 0
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                //storing right of row 1 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 14);
                //storing right of row 0 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 14);

                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);

                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, chroma_offset_8x16b);

                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_bottom_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_bottom_16x8b = _mm_unpackhi_epi8(src_bottom_16x8b, const0_16x8b);
                edge1_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_bottom_16x8b = _mm_add_epi16(src_bottom_16x8b, edge1_16x8b);
                src_bottom_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_bottom_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);
                cmp_lt0_16x8b = _mm_srli_si128(src_bottom_16x8b, 8);


                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                //row = 2
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd), src_bottom_16x8b);
                // row = 3
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd), cmp_lt0_16x8b);

                src_temp0_16x8b = src_temp1_16x8b;
                signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);
                pu1_src_cpy += (src_strd << 2);
                pu1_src_left_cpy += 8;
                pu1_src_left_str += 8;
            }
            ht_rem = ht & 0x2;
            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));

                //row 0 -row 1
                signdwn1_16x8b = _mm_srli_si128(src_temp1_16x8b, 2);
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);
                //manipulation for row 1 -row 0
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 14);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //manipulation for row 1 - row 0
                signdwn1_16x8b = _mm_alignr_epi8(src_temp0_16x8b, signdwn1_16x8b, 14);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //row1-row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign chang
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row 1 -bottom
                signdwn1_16x8b = _mm_srli_si128(src_bottom_16x8b, 2);

                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)
                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //row1 -bottom
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)
                //manipulation for bottom -row1
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 12);
                //eliminating old left for row 0,1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 4);
                //manipulation for bottom- row 1
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 14);
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty
                //bottom - row 1
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);

                //shifting row 1
                signdwn1_16x8b = _mm_slli_si128(src_temp1_16x8b, 8);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //for the next iteration signup0_16x8b
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-1) for next
                //storing right of row 1 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 14); //for storing right of row 0
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                //the next top  in  src_top_16x8b
                src_top_16x8b = src_temp1_16x8b;
                //storing right of row 0 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 14);


                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);

                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);

                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);

                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                //the next top already in  src_top_16x8b
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, src_temp1_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);
                pu1_src_left_cpy += 4;
                pu1_src_left_str += 4;
            }
            ht_rem = ht & 0x1;
            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));

                //row 0 -row1
                signdwn1_16x8b = _mm_srli_si128(src_bottom_16x8b, 2);
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);

                //for row 0 right to put into left store
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                //left store manipulation 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 2);
                src_top_16x8b = src_temp0_16x8b;
                //filling the left boundary value
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 14);

                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8);
                edge0_16x8b = _mm_srli_si128(edge0_16x8b, 8);


                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);

                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);

                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, const0_16x8b);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                pu1_src_cpy += (src_strd);
                pu1_src_left_cpy += 2;
                pu1_src_left_str += 2;
            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
                pu1_src_left_str[1] = pu1_src_cpy[7];
                pu1_src_left_str[0] = pu1_src_cpy[6];
            }

            if(0 == pu1_avail[2])
            {
                pu1_src_left_str[-2 * ht_0] = pu1_src[6 - src_strd];
                pu1_src_left_str[-2 * ht_0 + 1] = pu1_src[7 - src_strd];
            }

            _mm_storel_epi64((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 8;

            pu1_left_tmp = pu1_src_left_cpy2;
            pu1_src_left_cpy2 = pu1_src_left_str2;
            pu1_src_left_str2 = pu1_left_tmp;

            pu1_src_left_cpy = pu1_src_left_cpy2;
            pu1_src_left_str = pu1_src_left_str2;
        }
        pu1_src_org[0] = u1_pos_0_0_tmp_u;
        pu1_src_org[1] = u1_pos_0_0_tmp_v;
        pu1_src_org[wd - 2 + (ht_tmp - 1) * src_strd] = u1_pos_wd_ht_tmp_u;
        pu1_src_org[wd - 1 + (ht_tmp - 1) * src_strd] = u1_pos_wd_ht_tmp_v;
        pu1_src_left_cpy = (0 == pu1_avail[2]) ? (pu1_src_left_cpy - 2) : pu1_src_left_cpy;
        for(row = 0; row < 2 * ht_tmp; row++)
        {
            pu1_src_left[row] = pu1_src_left_cpy[row];
        }
    }

}

void ihevc_sao_edge_offset_class3_ssse3(UWORD8 *pu1_src,
                                        WORD32 src_strd,
                                        UWORD8 *pu1_src_left,
                                        UWORD8 *pu1_src_top,
                                        UWORD8 *pu1_src_top_left,
                                        UWORD8 *pu1_src_top_right,
                                        UWORD8 *pu1_src_bot_left,
                                        UWORD8 *pu1_avail,
                                        WORD8 *pi1_sao_offset,
                                        WORD32 wd,
                                        WORD32 ht)
{
    WORD32 row, col;
    UWORD8 *pu1_src_top_cpy, *pu1_src_left_cpy, *pu1_src_left_cpy2;
    UWORD8 *pu1_left_tmp, *pu1_src_left_str, *pu1_src_left_str2;
    UWORD8 *pu1_src_cpy, *pu1_src_org;
    UWORD8 au1_src_left_tmp[MAX_CTB_SIZE + 8];
    UWORD8 au1_src_left_tmp1[MAX_CTB_SIZE + 8];
    UWORD8 au1_mask[MAX_CTB_SIZE], *au1_mask_cpy;
    WORD32 wd_rem;
    UWORD8 u1_pos_wd_0_tmp, u1_pos_0_ht_tmp;
    WORD32 ht_tmp;
    WORD32 bit_depth;
    UWORD8 u1_avail0, u1_avail1;

    __m128i src_top_16x8b, src_bottom_16x8b;
    __m128i src_temp0_16x8b, src_temp1_16x8b;
    __m128i signup0_16x8b, signdwn1_16x8b;
    __m128i cmp_gt0_16x8b, cmp_lt0_16x8b;
    __m128i edge0_16x8b, edge1_16x8b;
    __m128i au1_mask8x16b;
    __m128i edge_idx_8x16b, sao_offset_8x16b;
    __m128i const2_16x8b, const0_16x8b;
    __m128i left_store_16x8b;

    ht_tmp = ht;
    au1_mask8x16b = _mm_set1_epi8(0xff);

    au1_src_left_tmp[0] = pu1_src[(wd - 1)];
    //manipulation for bottom left
    for(row = 1; row < ht; row++)
    {
        au1_src_left_tmp[row] = pu1_src_left[row];
    }
    au1_src_left_tmp[ht] = pu1_src_bot_left[0];

    *pu1_src_top_left = pu1_src_top[wd - 1];
    //setting availability mask to ff size MAX_CTB_SIZE
    for(col = 0; col < MAX_CTB_SIZE; col += 16)
        _mm_storeu_si128((__m128i *)(au1_mask + col), au1_mask8x16b);
    bit_depth = BIT_DEPTH_LUMA;
    pu1_src_org = pu1_src;
    pu1_src_top_cpy = pu1_src_top;
    pu1_src_left_cpy2 = au1_src_left_tmp;
    pu1_src_left_cpy = au1_src_left_tmp;
    pu1_src_left_str2 = au1_src_left_tmp1;
    pu1_src_left_str = au1_src_left_tmp1;
    edge_idx_8x16b   = _mm_loadl_epi64((__m128i *)gi1_table_edge_idx);
    sao_offset_8x16b = _mm_loadl_epi64((__m128i *)pi1_sao_offset);

    /* If top-right is available, process separately */
    if(0 != pu1_avail[5])
    {
        WORD32 edge_idx;

        edge_idx = 2 + SIGN(pu1_src[wd - 1] - pu1_src_top_right[0]) +
                        SIGN(pu1_src[wd - 1] - pu1_src[wd - 1 - 1 + src_strd]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_0_tmp = CLIP3(pu1_src[wd - 1] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_0_tmp = pu1_src[wd - 1];
        }
    }
    else
    {
        u1_pos_wd_0_tmp = pu1_src[wd - 1];
    }

    /* If bottom-left is available, process separately */
    if(0 != pu1_avail[6])
    {
        WORD32 edge_idx;

        edge_idx = 2 + SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src[(ht - 1) * src_strd + 1 - src_strd]) +
                        SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src_bot_left[0]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_ht_tmp = CLIP3(pu1_src[(ht - 1) * src_strd] + pi1_sao_offset[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_ht_tmp = pu1_src[(ht - 1) * src_strd];
        }
    }
    else
    {
        u1_pos_0_ht_tmp = pu1_src[(ht - 1) * src_strd];
    }



    /* Update height and source pointers based on the availability flags */
    if(0 == pu1_avail[2])
    {
        pu1_src_left_cpy2++;
        pu1_src_left_str2++;
        pu1_src_top_cpy = pu1_src;
        pu1_src += src_strd;
        ht--;
    }
    if(0 == pu1_avail[3])
    {
        ht--;
    }


    const2_16x8b = _mm_set1_epi8(2);
    const0_16x8b = _mm_setzero_si128();


    //availability mask creation
    u1_avail0 = pu1_avail[0];
    u1_avail1 = pu1_avail[1];
    au1_mask[0] = u1_avail0;
    au1_mask[wd - 1] = u1_avail1;
    {
        WORD32 ht_rem;

        pu1_src_left_cpy = pu1_src_left_cpy2;
        pu1_src_left_str = pu1_src_left_str2;
        au1_mask_cpy = au1_mask;
        for(col = wd; col >= 16; col -= 16)
        {
            pu1_src_cpy = pu1_src;
            src_top_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_top_cpy + wd - col + 1));
            //row = 0
            src_temp0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));

            //loading the mask
            au1_mask8x16b = _mm_loadu_si128((__m128i *)au1_mask_cpy);
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

            for(row = ht; row >= 2; row -= 2)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_left_cpy));
                //row = 1
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                //to insert left in row 1
                signdwn1_16x8b = _mm_slli_si128(left_store_16x8b, 14);
                // row = 0 right
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 1));

                //manipulation for row 1 - row 0
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 15);
                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);

                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(0-1)
                //combining sign-left and sign_right
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);

                //row1-row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);

                // row = 2
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));
                // row = 1 right
                signdwn1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd + 1));
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-0)

                //bottom - row1
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //for the next iteration bottom -row1
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //to insert left in row 1
                signdwn1_16x8b = _mm_slli_si128(left_store_16x8b, 13);
                //manipulation for row 1 - bottom
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 15);

                //row1 -bottom
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //combining sign-left and sign_right
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, signdwn1_16x8b);

                //eliminating old left for row 0 and row 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 2);

                //row1  getting it right for left of next block
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp1_16x8b, 15);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //row0  getting it right for left of next block
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 15);
                //copying the next top
                src_top_16x8b = src_temp1_16x8b;
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpackhi_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp1_16x8b);
                //store left boundary
                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storeu_si128((__m128i *)(pu1_src_cpy + src_strd), src_temp1_16x8b);

                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);
                pu1_src_left_cpy += 2;
                pu1_src_left_str += 2;
            }
            ht_rem = ht & 0x1;

            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                //to insert left in row 1
                signdwn1_16x8b = _mm_slli_si128(left_store_16x8b, 14);
                //manipulation for row 1 - row 0
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 15);

                //current row -next row
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and bottom and constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                //eliminating old left for row 0 and row 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 1);

                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);

                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);

                //row0  getting it right for left of next block
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 15);
                //copying the next top
                src_top_16x8b = src_temp0_16x8b;
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);
                //store left boundary
                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);

                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                pu1_src_cpy += (src_strd);
                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_left_cpy++;
                pu1_src_left_str++;
            }
            {   //for bottom right
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 1);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 15);
                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
            }
            //for the top left of next part of the block
            left_store_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_top_cpy + wd - col));
            //updating top flag
            _mm_storeu_si128((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 16;
            au1_mask_cpy += 16;

            pu1_left_tmp = pu1_src_left_cpy2;
            pu1_src_left_cpy2 = pu1_src_left_str2;
            pu1_src_left_str2 = pu1_left_tmp;

            pu1_src_left_cpy = pu1_src_left_cpy2;
            pu1_src_left_str = pu1_src_left_str2;
        }

        wd_rem = wd & 0xF;
        if(wd_rem)
        {
            pu1_src_cpy = pu1_src;
            pu1_src_left_cpy = pu1_src_left_cpy2;
            pu1_src_left_str = pu1_src_left_str2;
            src_top_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_top_cpy + wd - col + 1));
            //row = 0
            src_temp0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));
            au1_mask8x16b = _mm_loadl_epi64((__m128i *)au1_mask_cpy); //????
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //preparing au1_mask
            au1_mask8x16b = _mm_unpacklo_epi64(au1_mask8x16b, au1_mask8x16b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
            signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);

            for(row = ht; row >= 4; row -= 4)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));
                //manipulation for row 0 -row 1
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 14);
                //row 1 left
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 15);
                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //manipulatiing for row 1 -row 0
                signdwn1_16x8b = _mm_srli_si128(src_temp0_16x8b, 1);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row 1 -row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //row1-row0
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)

                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //manipulation for row 1 -row 2
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 13);
                //row 2 left
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 15);
                //packing row 0 n row 1
                src_temp0_16x8b = _mm_unpacklo_epi64(src_temp0_16x8b, src_temp1_16x8b);
                //row1 -row2
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)

                //row 1 right
                signdwn1_16x8b = _mm_srli_si128(src_temp1_16x8b, 1);
                //row = 3
                src_top_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 3 * src_strd));

                // row = 4
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 4 * src_strd));

                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty

                //separating +ve and and -ve values.(2,1)
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //row 2 right
                signdwn1_16x8b = _mm_srli_si128(src_bottom_16x8b, 1);
                //combining the appropriate sign change
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-1)

                //separating +ve and and -ve values.(3,2)
                cmp_gt0_16x8b = _mm_subs_epu8(src_top_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_top_16x8b);
                signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8); //aligned left (2-1)
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //manipulation for row 2 -row 3
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 12);
                //row 3 left
                signdwn1_16x8b = _mm_alignr_epi8(src_top_16x8b, signdwn1_16x8b, 15);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(3-2)

                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(3-2) ,(2-1)

                //separating +ve and and -ve values.(2,3)
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);

                //manipulation for row 3 -bottom
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 11);
                //bottom left
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 15);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-3)

                //separating +ve and and -ve values.(3,bottom)
                cmp_gt0_16x8b = _mm_subs_epu8(src_top_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_top_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                edge1_16x8b = _mm_slli_si128(edge1_16x8b, 8); //aligned left (2-3)
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(3-bottom)
                edge1_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge1_16x8b, 8); //(3-bottom),(2-3)


                //eliminating old left for row 0,1,2,3
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 4);
                //packing row 2 n row 3
                src_bottom_16x8b = _mm_unpacklo_epi64(src_bottom_16x8b, src_top_16x8b);
                //row 3 right
                signdwn1_16x8b = _mm_srli_si128(src_top_16x8b, 1);
                //loading row 3 right into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_bottom_16x8b, 15);
                //adding bottom and top values of row 2 and row 3
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, signup0_16x8b); //(3,2)
                //separating +ve and and -ve values.(botttom,3)
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);
                //to store right of row 2
                signdwn1_16x8b = _mm_slli_si128(src_bottom_16x8b, 8);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(bottom -3) for next iteration

                //storing right of row 2into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 15);
                //to store right of row 0
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                //storing right of row 1 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 15);

                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);

                //storing right of row 0 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 15);
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_bottom_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_bottom_16x8b = _mm_unpackhi_epi8(src_bottom_16x8b, const0_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                src_bottom_16x8b = _mm_add_epi16(src_bottom_16x8b, cmp_lt0_16x8b);
                src_bottom_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_bottom_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);
                cmp_lt0_16x8b = _mm_srli_si128(src_bottom_16x8b, 8);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                //row = 2
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd), src_bottom_16x8b);
                // row = 3
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd), cmp_lt0_16x8b);

                src_temp0_16x8b = src_temp1_16x8b;
                signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);
                pu1_src_cpy += (src_strd << 2);
                pu1_src_left_cpy += 4;
                pu1_src_left_str += 4;
            }
            ht_rem = ht & 0x2;
            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));

                //manipulation for row 0 -row 1
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 14);
                //bottom left
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 15);
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //manipulation for row 1 - row 0
                signdwn1_16x8b = _mm_srli_si128(src_temp0_16x8b, 1);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //row1-row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign chang
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //manipulation for row 1 -bottom
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 13);
                //bottom left
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 15);

                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)
                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //row1 -bottom
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)
                //manipulation for bottom- row 1 (row 1 right)
                signdwn1_16x8b = _mm_srli_si128(src_temp1_16x8b, 1);
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty
                //bottom - row 1
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);

                //eliminating old left for row 0,1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 2);
                signdwn1_16x8b = _mm_slli_si128(src_temp1_16x8b, 8);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //for the next iteration signup0_16x8b
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-1) for next

                //storing right of row 1 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 15);
                //for storing right of row 1
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);

                src_top_16x8b = src_temp1_16x8b;
                //storing right of row 0 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 15);

                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);

                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);

                //the next top already in  src_top_16x8b
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                cmp_lt0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, src_temp1_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);
                pu1_src_left_cpy += 2;
                pu1_src_left_str += 2;
            }
            ht_rem = ht & 0x1;
            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));


                //manipulation for row 0 -bottom
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 14);
                //bottom left
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 15);
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                //for row 0 right to put into left store
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8);
                edge0_16x8b = _mm_srli_si128(edge0_16x8b, 8);
                //left store manipulation 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 1);
                //filling the left boundary value
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 15);

                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                src_top_16x8b = src_temp0_16x8b;
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, const0_16x8b);

                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                pu1_src_cpy += (src_strd);
                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_left_cpy++;
                pu1_src_left_str++;
            }
            {   //for bottom right
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 1);
                src_temp0_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 15);
                _mm_storel_epi64((__m128i *)(pu1_src_left_str), left_store_16x8b);
            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
            }
            _mm_storel_epi64((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 8;

            pu1_left_tmp = pu1_src_left_cpy2;
            pu1_src_left_cpy2 = pu1_src_left_str2;
            pu1_src_left_str2 = pu1_left_tmp;

            pu1_src_left_cpy = pu1_src_left_cpy2;
            pu1_src_left_str = pu1_src_left_str2;

        }
        pu1_src_org[wd - 1] = u1_pos_wd_0_tmp;
        pu1_src_org[(ht_tmp - 1) * src_strd] = u1_pos_0_ht_tmp;
        pu1_src_left_cpy = (0 == pu1_avail[2]) ? (pu1_src_left_cpy - 1) : pu1_src_left_cpy;
        pu1_src_left[0] = au1_src_left_tmp[0];
        for(row = 1; row < ht_tmp; row++)
        {
            pu1_src_left[row] = pu1_src_left_cpy[row];
        }
    }

}

void ihevc_sao_edge_offset_class3_chroma_ssse3(UWORD8 *pu1_src,
                                               WORD32 src_strd,
                                               UWORD8 *pu1_src_left,
                                               UWORD8 *pu1_src_top,
                                               UWORD8 *pu1_src_top_left,
                                               UWORD8 *pu1_src_top_right,
                                               UWORD8 *pu1_src_bot_left,
                                               UWORD8 *pu1_avail,
                                               WORD8 *pi1_sao_offset_u,
                                               WORD8 *pi1_sao_offset_v,
                                               WORD32 wd,
                                               WORD32 ht)
{
    WORD32 row, col;
    UWORD8 *pu1_src_top_cpy, *pu1_src_left_cpy, *pu1_src_left_cpy2;
    UWORD8 *pu1_src_cpy, *pu1_src_org;
    UWORD8 au1_src_left_tmp[2 * (MAX_CTB_SIZE + 8)];
    UWORD8 au1_mask[MAX_CTB_SIZE], *au1_mask_cpy;
    WORD32 wd_rem;
    UWORD8 u1_pos_wd_0_tmp_u, u1_pos_wd_0_tmp_v, u1_pos_0_ht_tmp_u, u1_pos_0_ht_tmp_v;
    WORD32 ht_tmp;
    WORD32 bit_depth;
    UWORD8 u1_avail0, u1_avail1;

    __m128i src_top_16x8b, src_bottom_16x8b;
    __m128i src_temp0_16x8b, src_temp1_16x8b;
    __m128i signup0_16x8b, signdwn1_16x8b;
    __m128i cmp_gt0_16x8b, cmp_lt0_16x8b;
    __m128i edge0_16x8b, edge1_16x8b;
    __m128i au1_mask8x16b;
    __m128i edge_idx_8x16b, sao_offset_8x16b;
    __m128i left_store_16x8b;
    __m128i const0_16x8b, const2_16x8b;
    __m128i chroma_offset_8x16b;

    ht_tmp = ht;
    au1_mask8x16b = _mm_set1_epi8(0xff);


    au1_src_left_tmp[0] = pu1_src[(wd - 2)];
    au1_src_left_tmp[1] = pu1_src[(wd - 1)];
    //manipulation for bottom left
    for(row = 2; row < 2 * ht; row++)
    {
        au1_src_left_tmp[row] = pu1_src_left[row];
    }
    au1_src_left_tmp[2 * ht] = pu1_src_bot_left[0];
    au1_src_left_tmp[2 * ht + 1] = pu1_src_bot_left[1];

    pu1_src_top_left[0] = pu1_src_top[wd - 2];
    pu1_src_top_left[1] = pu1_src_top[wd - 1];
    //setting availability mask to ff size MAX_CTB_SIZE
    for(col = 0; col < MAX_CTB_SIZE; col += 16)
        _mm_storeu_si128((__m128i *)(au1_mask + col), au1_mask8x16b);
    bit_depth = BIT_DEPTH_LUMA;
    pu1_src_org = pu1_src;
    pu1_src_top_cpy = pu1_src_top;
    pu1_src_left_cpy2 = au1_src_left_tmp;
    pu1_src_left_cpy = au1_src_left_tmp;
    edge_idx_8x16b   = _mm_loadl_epi64((__m128i *)gi1_table_edge_idx);
    sao_offset_8x16b = _mm_loadl_epi64((__m128i *)pi1_sao_offset_u);
    const0_16x8b = _mm_loadl_epi64((__m128i *)pi1_sao_offset_v);
    chroma_offset_8x16b = _mm_set1_epi16(0x0800);
    /* If top-right is available, process separately */
    if(0 != pu1_avail[5])
    {
        WORD32 edge_idx;

        /* U */
        edge_idx = 2 + SIGN(pu1_src[wd - 2] - pu1_src_top_right[0]) +
                        SIGN(pu1_src[wd - 2] - pu1_src[wd - 2 - 2 + src_strd]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_0_tmp_u = CLIP3(pu1_src[wd - 2] + pi1_sao_offset_u[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_0_tmp_u = pu1_src[wd - 2];
        }

        /* V */
        edge_idx = 2 + SIGN(pu1_src[wd - 1] - pu1_src_top_right[1]) +
                        SIGN(pu1_src[wd - 1] - pu1_src[wd - 1 - 2 + src_strd]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_wd_0_tmp_v = CLIP3(pu1_src[wd - 1] + pi1_sao_offset_v[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_wd_0_tmp_v = pu1_src[wd - 1];
        }
    }
    else
    {
        u1_pos_wd_0_tmp_u = pu1_src[wd - 2];
        u1_pos_wd_0_tmp_v = pu1_src[wd - 1];
    }

    /* If bottom-left is available, process separately */
    if(0 != pu1_avail[6])
    {
        WORD32 edge_idx;

        /* U */
        edge_idx = 2 + SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src[(ht - 1) * src_strd + 2 - src_strd]) +
                        SIGN(pu1_src[(ht - 1) * src_strd] - pu1_src_bot_left[0]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_ht_tmp_u = CLIP3(pu1_src[(ht - 1) * src_strd] + pi1_sao_offset_u[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_ht_tmp_u = pu1_src[(ht - 1) * src_strd];
        }

        /* V */
        edge_idx = 2 + SIGN(pu1_src[(ht - 1) * src_strd + 1] - pu1_src[(ht - 1) * src_strd + 1 + 2 - src_strd]) +
                        SIGN(pu1_src[(ht - 1) * src_strd + 1] - pu1_src_bot_left[1]);

        edge_idx = gi1_table_edge_idx[edge_idx];

        if(0 != edge_idx)
        {
            u1_pos_0_ht_tmp_v = CLIP3(pu1_src[(ht - 1) * src_strd + 1] + pi1_sao_offset_v[edge_idx], 0, (1 << bit_depth) - 1);
        }
        else
        {
            u1_pos_0_ht_tmp_v = pu1_src[(ht - 1) * src_strd + 1];
        }
    }
    else
    {
        u1_pos_0_ht_tmp_u = pu1_src[(ht - 1) * src_strd];
        u1_pos_0_ht_tmp_v = pu1_src[(ht - 1) * src_strd + 1];
    }



    /* Update height and source pointers based on the availability flags */
    if(0 == pu1_avail[2])
    {
        pu1_src_left_cpy2 += 2;
        pu1_src_top_cpy = pu1_src;
        pu1_src += src_strd;
        ht--;
    }
    if(0 == pu1_avail[3])
    {
        ht--;
    }

    sao_offset_8x16b = _mm_unpacklo_epi64(sao_offset_8x16b, const0_16x8b);
    const2_16x8b = _mm_set1_epi8(2);
    const0_16x8b = _mm_setzero_si128();


    //availability mask creation
    u1_avail0 = pu1_avail[0];
    u1_avail1 = pu1_avail[1];
    au1_mask[0] = u1_avail0;
    au1_mask[1] = u1_avail0;
    au1_mask[wd - 1] = u1_avail1;
    au1_mask[wd - 2] = u1_avail1;
    {
        WORD32 ht_rem;
        au1_mask_cpy = au1_mask;
        for(col = wd; col >= 16; col -= 16)
        {
            pu1_src_cpy = pu1_src;
            src_top_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_top_cpy + wd - col + 2));
            //row = 0
            src_temp0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));

            //loading the mask
            au1_mask8x16b = _mm_loadu_si128((__m128i *)au1_mask_cpy);
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
            pu1_src_left_cpy = pu1_src_left_cpy2;

            for(row = ht; row >= 2; row -= 2)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_left_cpy));
                //row = 1
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                //to insert left in row 1
                signdwn1_16x8b = _mm_slli_si128(left_store_16x8b, 12);
                // row = 0 right
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2));

                //manipulation for row 1 - row 0
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 14);
                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);

                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(0-1)
                //combining sign-left and sign_right
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);

                //row1-row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, src_bottom_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);

                // row = 2
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));
                // row = 1 right
                signdwn1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd + 2));
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-0)

                //bottom - row1
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //for the next iteration bottom -row1
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //to insert left in row 1
                signdwn1_16x8b = _mm_slli_si128(left_store_16x8b, 10);
                //manipulation for row 1 - bottom
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 14);

                //row1 -bottom
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //combining sign-left and sign_right
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, signdwn1_16x8b);

                //eliminating old left for row 0 and row 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 4);
                //row1  getting it right for left of next block
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp1_16x8b, 14);
                //row0  getting it right for left of next block
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 14);
                //copying the next top
                src_top_16x8b = src_temp1_16x8b;


                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);

                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, chroma_offset_8x16b);

                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpackhi_epi8(src_temp1_16x8b, const0_16x8b);
                edge1_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, edge1_16x8b);
                src_temp1_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp1_16x8b);
                //store left boundary
                _mm_storel_epi64((__m128i *)(pu1_src_left_cpy), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storeu_si128((__m128i *)(pu1_src_cpy + src_strd), src_temp1_16x8b);

                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);
                pu1_src_left_cpy += 4;
            }
            ht_rem = ht & 0x1;

            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                //to insert left in row 1
                signdwn1_16x8b = _mm_slli_si128(left_store_16x8b, 12);
                //manipulation for row 1 - row 0
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 14);

                //current row -next row
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and bottom and constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                //eliminating old left for row 0 and row 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 2);
                //row0  getting it right for left of next block
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 14);
                //copying the next top
                src_top_16x8b = src_temp0_16x8b;

                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);

                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);


                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);

                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                //store left boundary
                _mm_storel_epi64((__m128i *)(pu1_src_left_cpy), left_store_16x8b);

                _mm_storeu_si128((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                pu1_src_cpy += (src_strd);
                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_left_cpy += 2;
            }
            {   //for bottom right
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 2);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 14);
                _mm_storel_epi64((__m128i *)(pu1_src_left_cpy), left_store_16x8b);
            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
            }
            //for the top left of next part of the block
            left_store_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_top_cpy + wd - col));
            //updating top flag
            _mm_storeu_si128((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 16;
            au1_mask_cpy += 16;
        }
        pu1_src_left_cpy = pu1_src_left_cpy2;
        wd_rem = wd & 0xF;
        if(wd_rem)
        {
            pu1_src_cpy = pu1_src;
            src_top_16x8b = _mm_loadl_epi64((__m128i *)(pu1_src_top_cpy + wd - col + 2));
            //row = 0
            src_temp0_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy));
            au1_mask8x16b = _mm_loadl_epi64((__m128i *)au1_mask_cpy); //????
            //separating +ve and and -ve values.
            cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, src_top_16x8b);
            cmp_lt0_16x8b = _mm_subs_epu8(src_top_16x8b, src_temp0_16x8b);
            //creating mask 00 for +ve and -ve values and FF for zero.
            cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
            cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
            //preparing au1_mask
            au1_mask8x16b = _mm_unpacklo_epi64(au1_mask8x16b, au1_mask8x16b);
            //combining the appropriate sign change
            signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
            signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);
            pu1_src_left_cpy = pu1_src_left_cpy2;
            for(row = ht; row >= 4; row -= 4)
            {
                left_store_16x8b = _mm_loadu_si128((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));
                //manipulation for row 0 -row 1
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 12);
                //row 1 left
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 14);
                //row 0 -row1
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //manipulatiing for row 1 -row 0
                signdwn1_16x8b = _mm_srli_si128(src_temp0_16x8b, 2);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //row 1 -row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //row1-row0
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)

                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //manipulation for row 1 -row 2
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 10);
                //row 2 left
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 14);
                //packing row 0 n row 1
                src_temp0_16x8b = _mm_unpacklo_epi64(src_temp0_16x8b, src_temp1_16x8b);
                //row1 -row2
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)

                //row 1 right
                signdwn1_16x8b = _mm_srli_si128(src_temp1_16x8b, 2);
                //row = 3
                src_top_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 3 * src_strd));

                // row = 4
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + 4 * src_strd));

                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty

                //separating +ve and and -ve values.(2,1)
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //row 2 right
                signdwn1_16x8b = _mm_srli_si128(src_bottom_16x8b, 2);
                //combining the appropriate sign change
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-1)

                //separating +ve and and -ve values.(3,2)
                cmp_gt0_16x8b = _mm_subs_epu8(src_top_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_top_16x8b);
                signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8); //aligned left (2-1)
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //manipulation for row 2 -row 3
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 8);
                //row 3 left
                signdwn1_16x8b = _mm_alignr_epi8(src_top_16x8b, signdwn1_16x8b, 14);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(3-2)

                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(3-2) ,(2-1)

                //separating +ve and and -ve values.(2,3)
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);

                //manipulation for row 3 -bottom
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 6);
                //bottom left
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 14);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-3)

                //separating +ve and and -ve values.(3,bottom)
                cmp_gt0_16x8b = _mm_subs_epu8(src_top_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_top_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                edge1_16x8b = _mm_slli_si128(edge1_16x8b, 8); //aligned left (2-3)
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(3-bottom)
                edge1_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge1_16x8b, 8); //(3-bottom),(2-3)


                //eliminating old left for row 0,1,2,3
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 8);
                //packing row 2 n row 3
                src_bottom_16x8b = _mm_unpacklo_epi64(src_bottom_16x8b, src_top_16x8b);
                //row 3 right
                signdwn1_16x8b = _mm_srli_si128(src_top_16x8b, 2);
                //loading row 3 right into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_bottom_16x8b, 14);
                //adding bottom and top values of row 2 and row 3
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, signup0_16x8b); //(3,2)
                //separating +ve and and -ve values.(botttom,3)
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);
                //to store right of row 2
                signdwn1_16x8b = _mm_slli_si128(src_bottom_16x8b, 8);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(bottom -3) for next iteration

                //storing right of row 2into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 14);
                //to store right of row 0
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                //storing right of row 1 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 14);
                //storing right of row 0 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 14);


                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, const2_16x8b);
                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge1_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                edge1_16x8b = _mm_and_si128(edge1_16x8b, au1_mask8x16b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                edge1_16x8b = _mm_add_epi8(edge1_16x8b, chroma_offset_8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                edge1_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge1_16x8b);

                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_unpackhi_epi8(src_temp0_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_temp0_16x8b);

                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge1_16x8b);
                cmp_gt0_16x8b = _mm_unpacklo_epi8(src_bottom_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge1_16x8b, signdwn1_16x8b);
                src_bottom_16x8b = _mm_unpackhi_epi8(src_bottom_16x8b, const0_16x8b);
                edge1_16x8b = _mm_unpackhi_epi8(edge1_16x8b, signdwn1_16x8b);
                cmp_gt0_16x8b = _mm_add_epi16(cmp_gt0_16x8b, cmp_lt0_16x8b);
                src_bottom_16x8b = _mm_add_epi16(src_bottom_16x8b, edge1_16x8b);
                src_bottom_16x8b = _mm_packus_epi16(cmp_gt0_16x8b, src_bottom_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);
                cmp_lt0_16x8b = _mm_srli_si128(src_bottom_16x8b, 8);
                _mm_storel_epi64((__m128i *)(pu1_src_left_cpy), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                //row = 2
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 2 * src_strd), src_bottom_16x8b);
                // row = 3
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + 3 * src_strd), cmp_lt0_16x8b);

                src_temp0_16x8b = src_temp1_16x8b;
                signup0_16x8b = _mm_slli_si128(signup0_16x8b, 8);
                pu1_src_cpy += (src_strd << 2);
                pu1_src_left_cpy += 8;
            }
            ht_rem = ht & 0x2;
            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_temp1_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));
                // row = 2
                src_bottom_16x8b =  _mm_loadu_si128((__m128i *)(pu1_src_cpy + 2 * src_strd));

                //manipulation for row 0 -row 1
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 12);
                //bottom left
                signdwn1_16x8b = _mm_alignr_epi8(src_temp1_16x8b, signdwn1_16x8b, 14);
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //manipulation for row 1 - row 0
                signdwn1_16x8b = _mm_srli_si128(src_temp0_16x8b, 2);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //row1-row0
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign chang
                edge1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);

                //manipulation for row 1 -bottom
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 10);
                //bottom left
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 14);

                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8); //aligned left (0-1)
                signup0_16x8b = _mm_alignr_epi8(edge1_16x8b, signup0_16x8b, 8); //(1-0),(0-top)
                //row1 -bottom
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp1_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp1_16x8b);

                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                signdwn1_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(1-2)
                edge0_16x8b = _mm_alignr_epi8(signdwn1_16x8b, edge0_16x8b, 8); //(1-2),(0-1)

                //manipulation for bottom- row 1 (row 1 right)
                signdwn1_16x8b = _mm_srli_si128(src_temp1_16x8b, 2);
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b); //(1,0) sign_up empty
                //bottom - row 1
                cmp_gt0_16x8b = _mm_subs_epu8(src_bottom_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_bottom_16x8b);

                //eliminating old left for row 0,1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 4);
                signdwn1_16x8b = _mm_slli_si128(src_temp1_16x8b, 8);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //for the next iteration signup0_16x8b
                signup0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b); //(2-1) for next

                //storing right of row 1 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 14);
                //for storing right of row 1
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);

                src_top_16x8b = src_temp1_16x8b;
                //storing right of row 0 into left
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 14);

                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);

                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);
                //the next top already in  src_top_16x8b
                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp1_16x8b = _mm_unpacklo_epi8(src_temp1_16x8b, const0_16x8b);
                edge0_16x8b = _mm_unpackhi_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp1_16x8b = _mm_add_epi16(src_temp1_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, src_temp1_16x8b);

                cmp_gt0_16x8b = _mm_srli_si128(src_temp0_16x8b, 8);

                _mm_storel_epi64((__m128i *)(pu1_src_left_cpy), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                // row = 1
                _mm_storel_epi64((__m128i *)(pu1_src_cpy + src_strd), cmp_gt0_16x8b);
                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_cpy += (src_strd << 1);
                pu1_src_left_cpy += 4;
            }
            ht_rem = ht & 0x1;
            if(ht_rem)
            {
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                //row = 1 load 8 pixel values from 7:0 pos. relative to cur. pos.
                src_bottom_16x8b = _mm_loadu_si128((__m128i *)(pu1_src_cpy + src_strd));


                //manipulation for row 0 -bottom
                signdwn1_16x8b =  _mm_slli_si128(left_store_16x8b, 12);
                //bottom left
                signdwn1_16x8b = _mm_alignr_epi8(src_bottom_16x8b, signdwn1_16x8b, 14);
                //separating +ve and and -ve values.
                cmp_gt0_16x8b = _mm_subs_epu8(src_temp0_16x8b, signdwn1_16x8b);
                cmp_lt0_16x8b = _mm_subs_epu8(signdwn1_16x8b, src_temp0_16x8b);
                //creating mask 00 for +ve and -ve values and FF for zero.
                cmp_gt0_16x8b = _mm_cmpeq_epi8(cmp_gt0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_cmpeq_epi8(cmp_lt0_16x8b, const0_16x8b);
                //combining the appropriate sign change
                edge0_16x8b = _mm_sub_epi8(cmp_gt0_16x8b, cmp_lt0_16x8b);
                //adding top and down substraction
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, signup0_16x8b);
                //for row 0 right to put into left store
                signdwn1_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                //adding constant 2
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, const2_16x8b);
                edge0_16x8b = _mm_slli_si128(edge0_16x8b, 8);
                edge0_16x8b = _mm_srli_si128(edge0_16x8b, 8);
                //left store manipulation 1
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 2);
                //filling the left boundary value
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, signdwn1_16x8b, 14);
                src_top_16x8b = src_temp0_16x8b;

                //shuffle to get sao index
                edge0_16x8b = _mm_shuffle_epi8(edge_idx_8x16b, edge0_16x8b);
                //using availability mask
                edge0_16x8b = _mm_and_si128(edge0_16x8b, au1_mask8x16b);
                //adding chroma offset to access U and V
                edge0_16x8b = _mm_add_epi8(edge0_16x8b, chroma_offset_8x16b);
                //shuffle to get sao offset
                edge0_16x8b = _mm_shuffle_epi8(sao_offset_8x16b, edge0_16x8b);

                //cnvert to 16 bit then add and then saturated pack
                signdwn1_16x8b =  _mm_cmpgt_epi8(const0_16x8b, edge0_16x8b);
                src_temp0_16x8b = _mm_unpacklo_epi8(src_temp0_16x8b, const0_16x8b);
                cmp_lt0_16x8b = _mm_unpacklo_epi8(edge0_16x8b, signdwn1_16x8b);
                src_temp0_16x8b = _mm_add_epi16(src_temp0_16x8b, cmp_lt0_16x8b);
                src_temp0_16x8b = _mm_packus_epi16(src_temp0_16x8b, const0_16x8b);

                _mm_storel_epi64((__m128i *)(pu1_src_left_cpy), left_store_16x8b);
                //row = 0 store 8 pixel values from 7:0 pos. relative to cur. pos.
                _mm_storel_epi64((__m128i *)(pu1_src_cpy), src_temp0_16x8b);
                pu1_src_cpy += (src_strd);
                src_temp0_16x8b = src_bottom_16x8b;
                pu1_src_left_cpy += 2;
            }
            {   //for bottom right
                left_store_16x8b = _mm_loadl_epi64((__m128i *)pu1_src_left_cpy);
                left_store_16x8b = _mm_srli_si128(left_store_16x8b, 2);
                src_temp0_16x8b = _mm_slli_si128(src_temp0_16x8b, 8);
                left_store_16x8b = _mm_alignr_epi8(left_store_16x8b, src_temp0_16x8b, 14);
                _mm_storel_epi64((__m128i *)(pu1_src_left_cpy), left_store_16x8b);
            }
            if(0 == pu1_avail[3])
            {
                src_top_16x8b = src_bottom_16x8b;
            }

            _mm_storel_epi64((__m128i *)(pu1_src_top + wd - col), src_top_16x8b);
            pu1_src += 8;
        }
        pu1_src_org[wd - 2] = u1_pos_wd_0_tmp_u;
        pu1_src_org[wd - 1] = u1_pos_wd_0_tmp_v;
        pu1_src_org[(ht_tmp - 1) * src_strd] = u1_pos_0_ht_tmp_u;
        pu1_src_org[(ht_tmp - 1) * src_strd + 1] = u1_pos_0_ht_tmp_v;
        for(row = 0; row < 2 * ht_tmp; row++)
        {
            pu1_src_left[row] = au1_src_left_tmp[row];
        }
    }

}
