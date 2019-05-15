/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "libde265/cabac.h"
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>


void simple_getline(char** lineptr,size_t* linelen,FILE* fh)
{
  const int LINESIZE=1000;

  if (*lineptr==NULL) {
    *linelen = LINESIZE;
    *lineptr = (char*)malloc(LINESIZE);
  }

  char* p = *lineptr;

  for (;;) {
    assert(p - *lineptr < LINESIZE);

    int c = fgetc(fh);
    if (c == EOF || c == '\n') {
      *p = 0;
      break;
    }

    *p++ = c;
  }
}


void generate_entropy_table()
{
#if 000
  const int nSymbols=1000*1000*10;
  const int oversample = 10;

  double tab[64][2];

  for (int i=0;i<64;i++)
    for (int k=0;k<2;k++)
      tab[i][k]=0;

  srand(time(0));
  //srand(123);

  int cnt=1;
  for (;;cnt++) {
    printf("-------------------- %d --------------------\n",cnt);

    for (int s=0;s<63;s++) {
      CABAC_encoder_bitstream cabac_mix0;
      CABAC_encoder_bitstream cabac_mix1;
      CABAC_encoder_bitstream cabac_ref;

      for (int i=0;i<nSymbols*oversample;i++) {
        int r = rand();
        int n = (r>>2) % 63;
        int m = (r>>1) & 1;
        int b = r & 1;

        context_model model;
        model.MPSbit = 1;
        model.state  = n;
        cabac_ref.write_CABAC_bit(&model, b);

        model.MPSbit = 1;
        model.state  = n;
        cabac_mix0.write_CABAC_bit(&model, b);

        model.MPSbit = 1;
        model.state  = n;
        cabac_mix1.write_CABAC_bit(&model, b);

        if (i%oversample == oversample/2) {
          model.MPSbit = 1;
          model.state  = s;
          cabac_mix0.write_CABAC_bit(&model, 0);

          model.MPSbit = 1;
          model.state  = s;
          cabac_mix1.write_CABAC_bit(&model, 1);

          //b = rand() & 1;
          //cabac_mix.write_CABAC_bypass(1);
        }

      }

      cabac_ref.flush_CABAC();
      cabac_mix0.flush_CABAC();
      cabac_mix1.flush_CABAC();

      int bits_ref  = cabac_ref.size()*8;
      int bits_mix0 = cabac_mix0.size()*8;
      int bits_mix1 = cabac_mix1.size()*8;

      //printf("bits: %d %d\n",bits_ref,bits_mix);
      int bits_diff0 = bits_mix0-bits_ref;
      int bits_diff1 = bits_mix1-bits_ref;
      //printf("bits diff: %d\n",bits_diff);

      double bits_per_symbol0 = bits_diff0 / double(nSymbols);
      double bits_per_symbol1 = bits_diff1 / double(nSymbols);

      tab[s][0] += bits_per_symbol0;
      tab[s][1] += bits_per_symbol1;

      double bps0 = tab[s][0]/cnt;
      double bps1 = tab[s][1]/cnt;

      printf("/* state=%2d */  0x%05x /* %f */,  0x%05x /* %f */,\n", s,
             (int)(bps1*0x8000), bps1,
             (int)(bps0*0x8000), bps0);
    }

    printf("                0x0010c, 0x3bfbb /* dummy, should never be used */\n");
  }
#endif
}

int probTab[128+2] = {
  1537234,1602970,
  1608644,1815493,
  1822246,2245961,
  916773,1329391,
  1337504,1930659,
  1063692,1707588,
  868294,1532108,
  842934,1555538,
  689043,1396941,
  860184,1789964,
  534165,1258482,
  672508,1598821,
  578782,1476240,
  602247,1613140,
  409393,1206638,
  459294,1356779,
  430124,1359893,
  308326,1050647,
  313100,1099956,
  293887,1088978,
  220901,869582,
  214967,881695,
  197226,856990,
  166131,767761,
  152514,737406,
  128332,663998,
  117638,632653,
  106178,595539,
  90898,539506,
  83437,509231,
  76511,492801,
  64915,443096,
  57847,409809,
  52730,385395,
  45707,354059,
  42018,333028,
  37086,308073,
  33256,284497,
  36130,299172,
  28831,270716,
  25365,244840,
  22850,221896,
  19732,201462,
  17268,183729,
  15252,168106,
  13787,153979,
  12187,141455,
  10821,130337,
  9896,120165,
  8626,112273,
  8162,103886,
  7201,96441,
  6413,89805,
  5886,83733,
  5447,78084,
  4568,73356,
  4388,68831,
  3959,64688,
  3750,60804,
  3407,57271,
  3109,54024,
  2820,51099,
  48569,1451987,
  0,    0,
  0*22686225,    0
};


void generate_entropy_table_probTableWeighted()
{
#if 000
  int64_t probTabSum=0;
  for (int i=0;i<130;i++)
    probTabSum += probTab[i];


  const int nSymbols=1000*1000*10;
  const int oversample = 10;

  double tab[64][2];

  for (int i=0;i<64;i++)
    for (int k=0;k<2;k++)
      tab[i][k]=0;

  srand(time(0));
  //srand(123);

  int cnt=1;
  for (;;cnt++) {
    printf("-------------------- %d --------------------\n",cnt);

    for (int s=0;s<63;s++) {
      CABAC_encoder_bitstream cabac_mix0;
      CABAC_encoder_bitstream cabac_mix1;
      CABAC_encoder_bitstream cabac_ref;

      for (int i=0;i<nSymbols*oversample;i++) {
        int r = rand();

        r %= probTabSum;
        int idx=0;
        while (r>probTab[idx]) {
          r-=probTab[idx];
          idx++;
        }

        assert(idx<=128);

        int n = idx/2;
        int b = idx&1;
        bool bypass = (idx==128);

        printf("%d %d %d\n",n,b,bypass);

        context_model model;
        model.MPSbit = 1;
        model.state  = n;
        if (bypass) cabac_ref.write_CABAC_bypass(1);
        else        cabac_ref.write_CABAC_bit(&model, b);

        model.MPSbit = 1;
        model.state  = n;
        if (bypass) cabac_mix0.write_CABAC_bypass(1);
        else        cabac_mix0.write_CABAC_bit(&model, b);

        model.MPSbit = 1;
        model.state  = n;
        if (bypass) cabac_mix1.write_CABAC_bypass(1);
        else        cabac_mix1.write_CABAC_bit(&model, b);

        if (i%oversample == oversample/2) {
          model.MPSbit = 1;
          model.state  = s;
          cabac_mix0.write_CABAC_bit(&model, 0);

          model.MPSbit = 1;
          model.state  = s;
          cabac_mix1.write_CABAC_bit(&model, 1);

          //b = rand() & 1;
          //cabac_mix.write_CABAC_bypass(1);
        }

      }

      cabac_ref.flush_CABAC();
      cabac_mix0.flush_CABAC();
      cabac_mix1.flush_CABAC();

      int bits_ref  = cabac_ref.size()*8;
      int bits_mix0 = cabac_mix0.size()*8;
      int bits_mix1 = cabac_mix1.size()*8;

      //printf("bits: %d %d\n",bits_ref,bits_mix);
      int bits_diff0 = bits_mix0-bits_ref;
      int bits_diff1 = bits_mix1-bits_ref;
      //printf("bits diff: %d\n",bits_diff);

      double bits_per_symbol0 = bits_diff0 / double(nSymbols);
      double bits_per_symbol1 = bits_diff1 / double(nSymbols);

      tab[s][0] += bits_per_symbol0;
      tab[s][1] += bits_per_symbol1;

      double bps0 = tab[s][0]/cnt;
      double bps1 = tab[s][1]/cnt;

      printf("/* state=%2d */  0x%05x /* %f */,  0x%05x /* %f */,\n", s,
             (int)(bps1*0x8000), bps1,
             (int)(bps0*0x8000), bps0);
    }

    printf("                0x0010c, 0x3bfbb /* dummy, should never be used */\n");
  }
#endif
}


void generate_entropy_table_replay()
{
#if 000
  const int oversample = 10;

  char* lineptr = NULL;
  size_t linelen = 0;

  for (int s=0;s<63;s++) {
    CABAC_encoder_bitstream cabac_mix0;
    CABAC_encoder_bitstream cabac_mix1;
    CABAC_encoder_bitstream cabac_ref;

    int nSymbols = 0;

    FILE* fh = fopen("streamdump-paris-intra","r");

    for (int i=0;i<80000000;i++) {
      simple_getline(&lineptr,&linelen,fh);
      if (feof(fh))
        break;

      int n,b;
      sscanf(lineptr,"%d %d",&n,&b);

      bool bypass = (n==64);

      if ((i%10000)==0)
        { printf("%d  %d %d    \r",i,n,b);
        }

      //printf("%d %d %d\n",n,b,bypass);

      context_model model;
      model.MPSbit = 1;
      model.state  = n;
      if (bypass) cabac_ref.write_CABAC_bypass(1);
      else        cabac_ref.write_CABAC_bit(&model, b);

      model.MPSbit = 1;
      model.state  = n;
      if (bypass) cabac_mix0.write_CABAC_bypass(1);
      else        cabac_mix0.write_CABAC_bit(&model, b);

      model.MPSbit = 1;
      model.state  = n;
      if (bypass) cabac_mix1.write_CABAC_bypass(1);
      else        cabac_mix1.write_CABAC_bit(&model, b);

      if (i%oversample == oversample/2) {
        model.MPSbit = 1;
        model.state  = s;
        cabac_mix0.write_CABAC_bit(&model, 0);

        model.MPSbit = 1;
        model.state  = s;
        cabac_mix1.write_CABAC_bit(&model, 1);

        nSymbols++;

        //b = rand() & 1;
        //cabac_mix.write_CABAC_bypass(1);
      }
    }

    fclose(fh);

    cabac_ref.flush_CABAC();
    cabac_mix0.flush_CABAC();
    cabac_mix1.flush_CABAC();

    int bits_ref  = cabac_ref.size()*8;
    int bits_mix0 = cabac_mix0.size()*8;
    int bits_mix1 = cabac_mix1.size()*8;

    //printf("bits: %d %d\n",bits_ref,bits_mix);
    int bits_diff0 = bits_mix0-bits_ref;
    int bits_diff1 = bits_mix1-bits_ref;
    //printf("bits diff: %d\n",bits_diff);

    double bits_per_symbol0 = bits_diff0 / double(nSymbols);
    double bits_per_symbol1 = bits_diff1 / double(nSymbols);

    double bps0 = bits_per_symbol0;
    double bps1 = bits_per_symbol1;

    printf("/* state=%2d */  0x%05x /* %f */,  0x%05x /* %f */,\n", s,
           (int)(bps1*0x8000), bps1,
           (int)(bps0*0x8000), bps0);
  }

  printf("                0x0010c, 0x3bfbb /* dummy, should never be used */\n");
#endif
}


void test_entropy_table_replay()
{
#if 000
  char* lineptr = NULL;
  size_t linelen = 0;


  CABAC_encoder_bitstream cabac_bs;
  CABAC_encoder_estim     cabac_estim;

  //FILE* fh = fopen("y","r");
  //FILE* fh = fopen("own-dump","r");
  //FILE* fh = fopen("rawstream-dump","r");
  //FILE* fh = fopen("johnny-stream-dump","r");
  FILE* fh = fopen("streamdump-paris-intra","r");

  for (int i=0;i<80000000;i++) {
    simple_getline(&lineptr,&linelen,fh);
    if (feof(fh))
      break;

    int n,b;
    sscanf(lineptr,"%d %d",&n,&b);
    b=!b;
    bool bypass = (n==64);

    if ((i%10000)==0)
      { printf("%d  %d %d    \n",i,n,b);
      }

    context_model model;
    model.MPSbit = 1;
    model.state  = n;
    if (bypass) cabac_bs.write_CABAC_bypass(1);
    else        cabac_bs.write_CABAC_bit(&model, b);

    model.MPSbit = 1;
    model.state  = n;
    if (bypass) cabac_estim.write_CABAC_bypass(1);
    else        cabac_estim.write_CABAC_bit(&model, b);
  }

  fclose(fh);

  printf("bs:%d estim:%d\n",cabac_bs.size(),cabac_estim.size());
#endif
}


int main(int argc, char** argv)
{
  //generate_entropy_table();
  //generate_entropy_table_replay();

  test_entropy_table_replay();

  return 0;
}
