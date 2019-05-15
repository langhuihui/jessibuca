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


#include "libde265/x86/sse-dct.h"
#include "dct.h"
#include "dct-scalar.h"


class DSPFunc_IDCT_SSE_4x4 : public DSPFunc_IDCT_Base
{
public:
  DSPFunc_IDCT_SSE_4x4() : DSPFunc_IDCT_Base(4) { }

  virtual const char* name() const { return "IDCT-SSE-4x4"; }

  virtual DSPFunc* referenceImplementation() const { return &idct_scalar_4x4; }

  virtual void runOnBlock(int x,int y) {
    memset(out,0,4*4);
    ff_hevc_transform_4x4_add_8_sse4(out, xy2coeff(x,y), 4);
  }
};

class DSPFunc_IDCT_SSE_8x8 : public DSPFunc_IDCT_Base
{
public:
  DSPFunc_IDCT_SSE_8x8() : DSPFunc_IDCT_Base(8) { }

  virtual const char* name() const { return "IDCT-SSE-8x8"; }

  virtual DSPFunc* referenceImplementation() const { return &idct_scalar_8x8; }

  virtual void runOnBlock(int x,int y) {
    memset(out,0,8*8);
    ff_hevc_transform_8x8_add_8_sse4(out, xy2coeff(x,y), 8);
  }
};

class DSPFunc_IDCT_SSE_16x16 : public DSPFunc_IDCT_Base
{
public:
  DSPFunc_IDCT_SSE_16x16() : DSPFunc_IDCT_Base(16) { }

  virtual const char* name() const { return "IDCT-SSE-16x16"; }

  virtual DSPFunc* referenceImplementation() const { return &idct_scalar_16x16; }

  virtual void runOnBlock(int x,int y) {
    memset(out,0,16*16);
    ff_hevc_transform_16x16_add_8_sse4(out, xy2coeff(x,y), 16);
  }
};

class DSPFunc_IDCT_SSE_32x32 : public DSPFunc_IDCT_Base
{
public:
  DSPFunc_IDCT_SSE_32x32() : DSPFunc_IDCT_Base(32) { }

  virtual const char* name() const { return "IDCT-SSE-32x32"; }

  virtual DSPFunc* referenceImplementation() const { return &idct_scalar_32x32; }

  virtual void runOnBlock(int x,int y) {
    memset(out,0,32*32);
    ff_hevc_transform_32x32_add_8_sse4(out, xy2coeff(x,y), 32);
  }
};

DSPFunc_IDCT_SSE_4x4   idct_sse_4x4;
DSPFunc_IDCT_SSE_8x8   idct_sse_8x8;
DSPFunc_IDCT_SSE_16x16 idct_sse_16x16;
DSPFunc_IDCT_SSE_32x32 idct_sse_32x32;





class DSPFunc_FDCT_SSE_4x4 : public DSPFunc_FDCT_Base
{
public:
  DSPFunc_FDCT_SSE_4x4() : DSPFunc_FDCT_Base(4) { }

  virtual const char* name() const { return "FDCT-SSE-4x4-to-be-implemented"; }

  virtual DSPFunc* referenceImplementation() const { return &fdct_scalar_4x4; }

  virtual void runOnBlock(int x,int y) {
    // <<< function to be implemented >>>

    // fdct_4x4_8_fallback(coeffs, residuals+x+y*stride, stride);

    bool D = false;
    if (D) { dump(x,y); }
  }
};


class DSPFunc_FDCT_SSE_8x8 : public DSPFunc_FDCT_Base
{
public:
  DSPFunc_FDCT_SSE_8x8() : DSPFunc_FDCT_Base(8) { }

  virtual const char* name() const { return "FDCT-SSE-8x8-to-be-implemented"; }

  virtual DSPFunc* referenceImplementation() const { return &fdct_scalar_8x8; }

  virtual void runOnBlock(int x,int y) {
    // <<< function to be implemented >>>

    // fdct_8x8_8_fallback(coeffs, residuals+x+y*stride, stride);

    bool D = false;
    if (D) { dump(x,y); }
  }
};


class DSPFunc_FDCT_SSE_16x16 : public DSPFunc_FDCT_Base
{
public:
  DSPFunc_FDCT_SSE_16x16() : DSPFunc_FDCT_Base(16) { }

  virtual const char* name() const { return "FDCT-SSE-16x16-to-be-implemented"; }

  virtual DSPFunc* referenceImplementation() const { return &fdct_scalar_16x16; }

  virtual void runOnBlock(int x,int y) {
    // <<< function to be implemented >>>

    // fdct_16x16_8_fallback(coeffs, residuals+x+y*stride, stride);

    bool D = false;
    if (D) { dump(x,y); }
  }
};

class DSPFunc_FDCT_SSE_32x32 : public DSPFunc_FDCT_Base
{
public:
  DSPFunc_FDCT_SSE_32x32() : DSPFunc_FDCT_Base(32) { }

  virtual const char* name() const { return "FDCT-SSE-32x32-to-be-implemented"; }

  virtual DSPFunc* referenceImplementation() const { return &fdct_scalar_32x32; }

  virtual void runOnBlock(int x,int y) {
    // <<< function to be implemented >>>

    // fdct_32x32_8_fallback(coeffs, residuals+x+y*stride, stride);

    bool D = false;
    if (D) { dump(x,y); }
  }
};

DSPFunc_FDCT_SSE_4x4   fdct_sse_4x4;
DSPFunc_FDCT_SSE_8x8   fdct_sse_8x8;
DSPFunc_FDCT_SSE_16x16 fdct_sse_16x16;
DSPFunc_FDCT_SSE_32x32 fdct_sse_32x32;
