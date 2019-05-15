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

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <unistd.h>


const bool D = false;


/* There are numerical stability problems in the matrix inverse.
   Switching to long double seems to provide enough accuracy.
   TODO: in the long term, use a better regression algorithm.
 */
typedef long double FP;


struct datapoint
{
  double rate;
  double distortion;
};

struct BjoentegaardParams
{
  // a*log^3 R + b*log^2 R + c*log R + d
  double a,b,c,d;

  double minRate, maxRate;
};

std::vector<datapoint> curveA,curveB;
BjoentegaardParams paramsA,paramsB;

#define RATE_NORMALIZATION_FACTOR 1 //(1/1000.0)



FP invf(int i,int j,const FP* m)
{
  int o = 2+(j-i);

  i += 4+o;
  j += 4-o;

#define e(a,b) m[ ((j+b)%4)*4 + ((i+a)%4) ]

    FP inv =
      + e(+1,-1)*e(+0,+0)*e(-1,+1)
      + e(+1,+1)*e(+0,-1)*e(-1,+0)
      + e(-1,-1)*e(+1,+0)*e(+0,+1)
      - e(-1,-1)*e(+0,+0)*e(+1,+1)
      - e(-1,+1)*e(+0,-1)*e(+1,+0)
      - e(+1,-1)*e(-1,+0)*e(+0,+1);

    return (o%2)?inv : -inv;

    #undef e

}

bool inverseMatrix4x4(const FP *m, FP *out)
{
  FP inv[16];

  for(int i=0;i<4;i++)
    for(int j=0;j<4;j++)
      inv[j*4+i] = invf(i,j,m);

  FP D = 0;

  for(int k=0;k<4;k++) D += m[k] * inv[k*4];

  if (D == 0) return false;

  D = 1.0 / D;

  for (int i = 0; i < 16; i++)
    out[i] = inv[i] * D;

  return true;

}



BjoentegaardParams fitParams(const std::vector<datapoint>& curve)
{
  // build regression matrix

  int n = curve.size();

  FP X[4*n];  // regression matrix
  FP XT[n*4]; // transpose of X

  for (int i=0;i<n;i++) {
    FP x = log(curve[i].rate) * RATE_NORMALIZATION_FACTOR;

    X[4*i + 0] = 1;
    X[4*i + 1] = x;
    X[4*i + 2] = x*x;
    X[4*i + 3] = x*x*x;

    if (D) printf("%f %f %f %f ;\n",1.0,(double)x,(double)(x*x),(double)(x*x*x));

    XT[i+0*n] = 1;
    XT[i+1*n] = x;
    XT[i+2*n] = x*x;
    XT[i+3*n] = x*x*x;
  }

  if (D) {
    printf("rate: ");
    for (int i=0;i<n;i++) {
      printf("%f ; ",curve[i].rate);
    }
    printf("\n");

    printf("distortion: ");
    for (int i=0;i<n;i++) {
      printf("%f ; ",curve[i].distortion);
    }
    printf("\n");
  }

  // calc X^T * X

  FP XTX[4*4];
  for (int y=0;y<4;y++)
    for (int x=0;x<4;x++) {
      FP sum=0;

      for (int i=0;i<n;i++)
        {
          sum += XT[y*n + i] * X[x + i*4];
        }

      XTX[y*4+x] = sum;
    }

  FP XTXinv[4*4];

  inverseMatrix4x4(XTX, XTXinv);

  if (D) {
    for (int y=0;y<4;y++) {
      for (int x=0;x<4;x++) {
        printf("%f ",(double)XTXinv[y*4+x]);
      }
      printf("\n");
    }
  }

  // calculate pseudo-inverse XP = (X^T * X)^-1 * X^T

  FP XP[n*4];

  for (int y=0;y<4;y++) {
    for (int x=0;x<n;x++) {
      FP sum=0;

      for (int i=0;i<4;i++)
        {
          sum += XTXinv[y*4 + i] * XT[x + i*n];
        }

      XP[y*n+x] = sum;
    }
  }

  // calculate regression parameters

  FP p[4];

  for (int k=0;k<4;k++)
    {
      FP sum=0;

      for (int i=0;i<n;i++) {
        sum += XP[k*n + i] * curve[i].distortion;
      }

      p[k]=sum;
    }


  BjoentegaardParams param;
  param.d = p[0];
  param.c = p[1];
  param.b = p[2];
  param.a = p[3];


  // find min and max rate

  param.minRate = curve[0].rate;
  param.maxRate = curve[0].rate;

  for (int i=1;i<n;i++) {
    param.minRate = std::min(param.minRate, curve[i].rate);
    param.maxRate = std::max(param.maxRate, curve[i].rate);
  }

  return param;
}

FP evalIntegralAt(const BjoentegaardParams& p, double x)
{
  FP sum = 0;

  // integral of: d

  sum += p.d * x;

  // integral of: c*log(x)

  sum += p.c * x* (log(x)-1);

  // integral of: b*log(x)^2

  sum += p.b * x * ((log(x)-2)*log(x)+2);

  // integral of: a*log(x)^3

  sum += p.a * x * (log(x)*((log(x)-3)*log(x)+6)-6);

  return sum;
}


double calcBjoentegaard(const BjoentegaardParams& paramsA,
                        const BjoentegaardParams& paramsB,
                        double min_rate, double max_rate)
{
  double mini = std::max(paramsA.minRate, paramsB.minRate);
  double maxi = std::min(paramsA.maxRate, paramsB.maxRate);

  if (min_rate >= 0) mini = std::max(mini, min_rate);
  if (max_rate >= 0) maxi = std::min(maxi, max_rate);

  if (D) printf("range: %f %f\n",mini,maxi);

  FP intA = evalIntegralAt(paramsA, maxi) - evalIntegralAt(paramsA, mini);
  FP intB = evalIntegralAt(paramsB, maxi) - evalIntegralAt(paramsB, mini);

  if (D) printf("int1:%f int2:%f\n",(double)intA,(double)intB);

  return (intA-intB)/(maxi-mini);
}


std::vector<datapoint> readRDFile(const char* filename, float min_rate, float max_rate)
{
  std::vector<datapoint> curve;
  std::ifstream istr(filename);

  for (;;)
    {
      std::string line;
      getline(istr, line);
      if (istr.eof())
        break;

      if (line[0]=='#') continue;

      std::stringstream sstr(line);
      datapoint p;
      sstr >> p.rate >> p.distortion;

      if (min_rate>=0 && p.rate < min_rate) continue;
      if (max_rate>=0 && p.rate > max_rate) continue;

      curve.push_back(p);
    }

  return curve;
}


int main(int argc, char** argv)
{
  float min_rate = -1;
  float max_rate = -1;

  int c;
  while ((c=getopt(argc,argv, "l:h:")) != -1) {
    switch (c) {
    case 'l': min_rate = atof(optarg); break;
    case 'h': max_rate = atof(optarg); break;
    }
  }

  curveA = readRDFile(argv[optind], min_rate, max_rate);
  paramsA = fitParams(curveA);

  printf("params A: %f %f %f %f\n",paramsA.a,paramsA.b,paramsA.c,paramsA.d);

  printf("gnuplot: %f*log(x)**3+%f*log(x)**2+%f*log(x)+%f\n",paramsA.a,paramsA.b,paramsA.c,paramsA.d);

  if (optind+1<argc) {
    curveB = readRDFile(argv[optind+1], min_rate, max_rate);
    paramsB = fitParams(curveB);

    printf("params B: %f %f %f %f\n",paramsB.a,paramsB.b,paramsB.c,paramsB.d);

    printf("gnuplot: %f*log(x)**3+%f*log(x)**2+%f*log(x)+%f\n",paramsB.a,paramsB.b,paramsB.c,paramsB.d);

    double delta = calcBjoentegaard(paramsA,paramsB, min_rate,max_rate);

    printf("Bjoentegaard delta: %f dB   (A-B -> >0 -> first (A) is better)\n",delta);

    if (delta>=0) {
      printf("-> first is better by %f dB\n",delta);
    }
    else {
      printf("-> second is better by %f dB\n",-delta);
    }
  }

  return 0;
}
