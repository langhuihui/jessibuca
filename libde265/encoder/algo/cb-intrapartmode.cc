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


#include "libde265/encoder/algo/cb-intrapartmode.h"
#include "libde265/encoder/algo/coding-options.h"
#include "libde265/encoder/encoder-context.h"
#include <assert.h>
#include <limits>
#include <math.h>
#include <iostream>


#define ENCODER_DEVELOPMENT 1



enc_cb* Algo_CB_IntraPartMode_BruteForce::analyze(encoder_context* ectx,
                                                  context_model_table& ctxModel,
                                                  enc_cb* cb_in)
{
  const int log2CbSize = cb_in->log2Size;
  const int x = cb_in->x;
  const int y = cb_in->y;

  const bool can_use_NxN = ((log2CbSize == ectx->sps.Log2MinCbSizeY) &&
                            (log2CbSize >  ectx->sps.Log2MinTrafoSize));


  // test all modes

  assert(cb_in->pcm_flag==0);


  // 0: 2Nx2N  (always checked)
  // 1:  NxN   (only checked at MinCbSize)

  CodingOptions options(ectx,cb_in,ctxModel);
  CodingOption option[2];
  option[0] = options.new_option(true);
  option[1] = options.new_option(can_use_NxN);

  options.start();

  for (int p=0;p<2;p++)
    if (option[p]) {
      option[p].begin();

      enc_cb* cb = option[p].get_cb();

      // --- set intra prediction mode ---

      cb->PartMode = (p==0 ? PART_2Nx2N : PART_NxN);

      ectx->img->set_pred_mode(x,y, log2CbSize, cb->PredMode);  // TODO: probably unnecessary
      ectx->img->set_PartMode (x,y, cb->PartMode);

      // encode transform tree

      int IntraSplitFlag= (cb->PredMode == MODE_INTRA && cb->PartMode == PART_NxN);
      int MaxTrafoDepth = ectx->sps.max_transform_hierarchy_depth_intra + IntraSplitFlag;

      cb->transform_tree = mTBIntraPredModeAlgo->analyze(ectx, option[p].get_context(),
                                                         ectx->imgdata->input, NULL, cb,
                                                         x,y, x,y, log2CbSize,
                                                         0,
                                                         0, MaxTrafoDepth, IntraSplitFlag);

      cb->distortion = cb->transform_tree->distortion;
      cb->rate       = cb->transform_tree->rate;


      // rate for cu syntax

      logtrace(LogSymbols,"$1 part_mode=%d\n",cb->PartMode);
      if (log2CbSize == ectx->sps.Log2MinCbSizeY) {
        int bin = (cb->PartMode==PART_2Nx2N);
        option[p].get_cabac()->reset();
        option[p].get_cabac()->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+0, bin);
        cb->rate += option[p].get_cabac()->getRDBits();
      }

      option[p].end();
    }

  options.compute_rdo_costs();
  return options.return_best_rdo();
}


enc_cb* Algo_CB_IntraPartMode_Fixed::analyze(encoder_context* ectx,
                                             context_model_table& ctxModel,
                                             enc_cb* cb)
{
  std::cout << "CB-IntraPartMode in size=" << (1<<cb->log2Size)
            << " hash=" << ctxModel.debug_dump() << "\n";

  enum PartMode PartMode = mParams.partMode();


  const int log2CbSize = cb->log2Size;
  const int x = cb->x;
  const int y = cb->y;


  // NxN can only be applied at minimum CB size.
  // If we are not at the minimum size, we have to use 2Nx2N.

  if (PartMode==PART_NxN && log2CbSize != ectx->sps.Log2MinCbSizeY) {
    PartMode = PART_2Nx2N;
  }


  // --- set intra prediction mode ---

  cb->PartMode = PartMode;
  ectx->img->set_PartMode(x,y, PartMode);


  // encode transform tree

  int IntraSplitFlag= (cb->PredMode == MODE_INTRA && cb->PartMode == PART_NxN);
  int MaxTrafoDepth = ectx->sps.max_transform_hierarchy_depth_intra + IntraSplitFlag;

  cb->transform_tree = mTBIntraPredModeAlgo->analyze(ectx, ctxModel,
                                                     ectx->imgdata->input, NULL, cb,
                                                     cb->x,cb->y, cb->x,cb->y, log2CbSize,
                                                     0,
                                                     0, MaxTrafoDepth, IntraSplitFlag);


  // rate and distortion for this CB

  cb->distortion = cb->transform_tree->distortion;
  cb->rate       = cb->transform_tree->rate;

  std::cout << "SUM TB-tree hinter PartMode " << cb->rate << "\n";


  // rate for cu syntax

  CABAC_encoder_estim estim;
  estim.set_context_models(&ctxModel);

  //encode_coding_unit(ectx,&estim,cb,x,y,log2CbSize, false);

  //encode_part_mode(ectx,&estim, MODE_INTRA, PartMode, 0);

  logtrace(LogSymbols,"$1 part_mode=%d\n",PartMode);
  if (log2CbSize == ectx->sps.Log2MinCbSizeY) {
    int bin = (PartMode==PART_2Nx2N);
    estim.write_CABAC_bit(CONTEXT_MODEL_PART_MODE+0, bin);
  }

  cb->rate += estim.getRDBits();

  std::cout << "CB-IntraPartMode out size=" << (1<<cb->log2Size)
            << " hash=" << ctxModel.debug_dump() << "\n";

  return cb;
}
