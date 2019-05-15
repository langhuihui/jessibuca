/*
 * H.265 video codec.
 * Copyright (c) 2015 struktur AG, Dirk Farin <farin@struktur.de>
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

#ifndef ACCELERATION_SPEED_DCT_SCALAR_H
#define ACCELERATION_SPEED_DCT_SCALAR_H

#include "dct.h"

class DSPFunc_FDCT_Scalar_4x4 : public DSPFunc_FDCT_Base
{
public:
  DSPFunc_FDCT_Scalar_4x4() : DSPFunc_FDCT_Base(4) { }

  virtual const char* name() const { return "FDCT-Scalar-4x4"; }

  virtual void runOnBlock(int x,int y) {
    bool D = false;

    fdct_4x4_8_fallback(coeffs, residuals+x+y*stride, stride);

    if (D) { dump(x,y); }
  }
};


class DSPFunc_FDCT_Scalar_8x8 : public DSPFunc_FDCT_Base
{
public:
  DSPFunc_FDCT_Scalar_8x8() : DSPFunc_FDCT_Base(8) { }

  virtual const char* name() const { return "FDCT-Scalar-8x8"; }

  virtual void runOnBlock(int x,int y) {
    bool D = false;

    fdct_8x8_8_fallback(coeffs, residuals+x+y*stride, stride);

    if (D) { dump(x,y); }
  }
};


class DSPFunc_FDCT_Scalar_16x16 : public DSPFunc_FDCT_Base
{
public:
  DSPFunc_FDCT_Scalar_16x16() : DSPFunc_FDCT_Base(16) { }

  virtual const char* name() const { return "FDCT-Scalar-16x16"; }

  virtual void runOnBlock(int x,int y) {
    bool D = false;

    fdct_16x16_8_fallback(coeffs, residuals+x+y*stride, stride);

    if (D) { dump(x,y); }
  }
};

class DSPFunc_FDCT_Scalar_32x32 : public DSPFunc_FDCT_Base
{
public:
  DSPFunc_FDCT_Scalar_32x32() : DSPFunc_FDCT_Base(32) { }

  virtual const char* name() const { return "FDCT-Scalar-32x32"; }

  virtual void runOnBlock(int x,int y) {
    bool D = false;

    fdct_32x32_8_fallback(coeffs, residuals+x+y*stride, stride);

    if (D) { dump(x,y); }
  }
};



class DSPFunc_IDCT_Scalar_4x4 : public DSPFunc_IDCT_Base
{
public:
  DSPFunc_IDCT_Scalar_4x4() : DSPFunc_IDCT_Base(4) { }

  virtual const char* name() const { return "IDCT-Scalar-4x4"; }

  virtual void runOnBlock(int x,int y) {
    memset(out,0,4*4);
    transform_4x4_add_8_fallback(out, xy2coeff(x,y), 4);
  }
};

class DSPFunc_IDCT_Scalar_8x8 : public DSPFunc_IDCT_Base
{
public:
  DSPFunc_IDCT_Scalar_8x8() : DSPFunc_IDCT_Base(8) { }

  virtual const char* name() const { return "IDCT-Scalar-8x8"; }

  virtual void runOnBlock(int x,int y) {
    memset(out,0,8*8);
    transform_8x8_add_8_fallback(out, xy2coeff(x,y), 8);
  }
};

class DSPFunc_IDCT_Scalar_16x16 : public DSPFunc_IDCT_Base
{
public:
  DSPFunc_IDCT_Scalar_16x16() : DSPFunc_IDCT_Base(16) { }

  virtual const char* name() const { return "IDCT-Scalar-16x16"; }

  virtual void runOnBlock(int x,int y) {
    memset(out,0,16*16);
    transform_16x16_add_8_fallback(out, xy2coeff(x,y), 16);
  }
};

class DSPFunc_IDCT_Scalar_32x32 : public DSPFunc_IDCT_Base
{
public:
  DSPFunc_IDCT_Scalar_32x32() : DSPFunc_IDCT_Base(32) { }

  virtual const char* name() const { return "IDCT-Scalar-32x32"; }

  virtual void runOnBlock(int x,int y) {
    memset(out,0,32*32);
    transform_32x32_add_8_fallback(out, xy2coeff(x,y), 32);
  }
};


extern DSPFunc_FDCT_Scalar_4x4   fdct_scalar_4x4;
extern DSPFunc_FDCT_Scalar_8x8   fdct_scalar_8x8;
extern DSPFunc_FDCT_Scalar_16x16 fdct_scalar_16x16;
extern DSPFunc_FDCT_Scalar_32x32 fdct_scalar_32x32;


extern DSPFunc_IDCT_Scalar_4x4   idct_scalar_4x4;
extern DSPFunc_IDCT_Scalar_8x8   idct_scalar_8x8;
extern DSPFunc_IDCT_Scalar_16x16 idct_scalar_16x16;
extern DSPFunc_IDCT_Scalar_32x32 idct_scalar_32x32;

#endif
