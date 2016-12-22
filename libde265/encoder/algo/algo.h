/*
 * H.265 video codec.
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * Authors: Dirk Farin <farin@struktur.de>
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

#ifndef ALGO_H
#define ALGO_H

#include "libde265/encoder/encode.h"


/* When entering the next recursion level, it is assumed that
   a valid CB structure is passed down. Ownership is transferred to
   the new algorithm. That algorithm passes back a (possibly different)
   CB structure that the first algorithm should use. The original CB
   structure might have been deleted in the called algorithm.

   The context_model_table passed down is at the current state.
   When the algorithm returns, the state should represent the state
   after running this algorithm.

   When returning from the algorithm, it is also assumed that the
   ectx->img content (reconstruction and metadata) represent the
   current state. When the algorithm tries several variants, it
   has to restore the state to the selected variant.
 */

class Algo_CB
{
 public:
  virtual ~Algo_CB() { }

  /* The context_model_table that is provided can be modified and
     even released in the function. On exit, it should be filled with
     a (optionally new) context_model_table that represents the state
     after encoding the syntax element. However, to speed up computation,
     it is also allowed to not modify the context_model_table at all.
   */
  virtual enc_cb* analyze(encoder_context*,
                          context_model_table&,
                          enc_cb* cb) = 0;
};


class Algo_PB
{
 public:
  virtual ~Algo_PB() { }

  virtual enc_cb* analyze(encoder_context*,
                          context_model_table&,
                          enc_cb* cb,
                          int PBidx, int x,int y,int w,int h) = 0;
};


#endif
