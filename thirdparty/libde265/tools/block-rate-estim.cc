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

#include <vector>
#include <string>
#include <fstream>
#include <math.h>


struct datapoint {
  int   log2blksize;
  float rate;
  float estim;
};

std::vector<datapoint> pts;

#define NBINS 100

/*
 #define ESTIMDIV 100
 #define MAXESTIM 80000
 std::vector<float> bitestim2[MAXESTIM/ESTIMDIV];
*/



void print_bitestim_results(int log2blksize)
{
  float max_estim=0;

  for (int i=0;i<pts.size();i++) {
    if (log2blksize==0 || pts[i].log2blksize==log2blksize) {
      max_estim = std::max(max_estim, pts[i].estim);
    }
  }



  float epsilon = 0.0001;
  float interval = (max_estim+epsilon) / NBINS;

  for (int b=0;b<NBINS;b++) {

    int cnt=0;
    double sum=0;
    float mini=999999;
    float maxi=0;

    for (int i=0;i<pts.size();i++)
      if (log2blksize==0 || pts[i].log2blksize==log2blksize) {
        int bin = pts[i].estim/interval;
        if (bin==b) {
          sum += pts[i].rate;

          mini = std::min(mini,pts[i].rate);
          maxi = std::max(maxi,pts[i].rate);
          cnt++;
        }
      }

    if (cnt>0) {
      double mean = sum/cnt;

      double var = 0;

      for (int i=0;i<pts.size();i++)
        if (log2blksize==0 || pts[i].log2blksize==log2blksize) {
          int bin = pts[i].estim/interval;
          if (bin==b) {
            var += (pts[i].rate-mean)*(pts[i].rate-mean);
          }
        }

      var /= cnt;
      double stddev = sqrt(var);

      printf("%f  %f %f  %f %f  %f %f %d\n",
             (b+0.5)*interval,mean,var,
             mean-stddev,mean+stddev, mini,maxi,
             cnt);
    }
  }
}


int main(int argc,char** argv)
{
  std::string tag = argv[1];

  std::ifstream istr(argv[2]);
  for (;;)
    {
      std::string t;
      int log2blksize;
      datapoint pt;

      istr >> t >> pt.log2blksize >> pt.rate >> pt.estim;

      if (istr.eof()) break;

      if (t == tag) {
        pts.push_back(pt);
      }
    }

  print_bitestim_results(0);

  return 0;
}
