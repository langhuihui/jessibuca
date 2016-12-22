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


#include "libde265/encoder/algo/ctb-qscale.h"
#include "libde265/encoder/encoder-context.h"
#include <assert.h>
#include <limits>
#include <math.h>


#define ENCODER_DEVELOPMENT 1


enc_cb* Algo_CTB_QScale_Constant::analyze(encoder_context* ectx,
                                          context_model_table& ctxModel,
                                          int ctb_x,int ctb_y)
{
  enc_cb* cb = new enc_cb();

  cb->log2Size = ectx->sps.Log2CtbSizeY;
  cb->ctDepth = 0;
  cb->x = ctb_x;
  cb->y = ctb_y;

  ectx->img->set_QPY(ctb_x,ctb_y, cb->log2Size, ectx->active_qp);

  // write currently unused coding options to image
  ectx->img->set_cu_transquant_bypass(ctb_x,ctb_y,cb->log2Size, cb->cu_transquant_bypass_flag);
  ectx->img->set_pcm_flag(ctb_x,ctb_y,cb->log2Size, cb->pcm_flag);

  assert(mChildAlgo);
  return mChildAlgo->analyze(ectx,ctxModel,cb);
}
