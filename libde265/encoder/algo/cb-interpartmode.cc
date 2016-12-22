/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * Authors: struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "libde265/encoder/algo/cb-interpartmode.h"
#include "libde265/encoder/algo/coding-options.h"
#include "libde265/encoder/encoder-context.h"
#include <assert.h>
#include <limits>
#include <math.h>



enc_cb* Algo_CB_InterPartMode::codeAllPBs(encoder_context* ectx,
                                          context_model_table& ctxModel,
                                          enc_cb* cb)
{
  int x = cb->x;
  int y = cb->y;
  int log2Size = cb->log2Size;
  int w = 1<<log2Size;
  int s; // splitSize;

  int nPB;
  switch (cb->PartMode) {
  case PART_2Nx2N:
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 0, x,y,1<<log2Size,1<<log2Size);
    break;

  case PART_NxN:
    s = 1<<(log2Size-1);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 0, x  ,y  ,s,s);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 1, x+s,y  ,s,s);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 2, x  ,y+s,s,s);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 3, x+s,y+s,s,s);
    break;

  case PART_2NxN:
    s = 1<<(log2Size-1);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 0, x,y  ,w,s);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 1, x,y+s,w,s);
    break;

  case PART_Nx2N:
    s = 1<<(log2Size-1);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 0, x  ,y,s,w);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 1, x+s,y,s,w);
    break;

  case PART_2NxnU:
    s = 1<<(log2Size-2);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 0, x,y  ,w,s);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 1, x,y+s,w,w-s);
    break;

  case PART_2NxnD:
    s = 1<<(log2Size-2);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 0, x,y    ,w,w-s);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 1, x,y+w-s,w,s);
    break;

  case PART_nLx2N:
    s = 1<<(log2Size-2);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 0, x  ,y,s  ,w);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 1, x+s,y,w-s,w);
    break;

  case PART_nRx2N:
    s = 1<<(log2Size-2);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 0, x    ,y,w-s,w);
    cb = mChildAlgo->analyze(ectx, ctxModel, cb, 1, x+w-s,y,s  ,w);
    break;
  }

  return cb;
}


enc_cb* Algo_CB_InterPartMode_Fixed::analyze(encoder_context* ectx,
                                             context_model_table& ctxModel,
                                             enc_cb* cb)
{
  const int x = cb->x;
  const int y = cb->y;

  enum PartMode partMode = mParams.partMode();

  cb->PartMode = partMode;
  ectx->img->set_PartMode(x,y, partMode);

  cb = codeAllPBs(ectx,ctxModel,cb);

  return cb;
}
