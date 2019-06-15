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

#include "dct-scalar.h"


DSPFunc_FDCT_Scalar_4x4   fdct_scalar_4x4;
DSPFunc_FDCT_Scalar_8x8   fdct_scalar_8x8;
DSPFunc_FDCT_Scalar_16x16 fdct_scalar_16x16;
DSPFunc_FDCT_Scalar_32x32 fdct_scalar_32x32;


DSPFunc_IDCT_Scalar_4x4   idct_scalar_4x4;
DSPFunc_IDCT_Scalar_8x8   idct_scalar_8x8;
DSPFunc_IDCT_Scalar_16x16 idct_scalar_16x16;
DSPFunc_IDCT_Scalar_32x32 idct_scalar_32x32;
