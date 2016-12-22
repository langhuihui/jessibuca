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

#include "libde265/encoder/algo/coding-options.h"
#include "libde265/encoder/encoder-context.h"


CodingOptions::CodingOptions(encoder_context* ectx, enc_cb* cb, context_model_table& tab)
{
  mCBInput = cb;
  mContextModelInput = &tab;

  mCurrentlyReconstructedOption=-1;
  mBestRDO=-1;

  mECtx = ectx;
}

CodingOptions::~CodingOptions()
{
}

CodingOption CodingOptions::new_option(bool active)
{
  if (!active) {
    return CodingOption();
  }


  CodingOptionData opt;

  bool firstOption = mOptions.empty();
  if (firstOption) {
    opt.cb = mCBInput;
  }
  else {
    opt.cb = new enc_cb(*mCBInput);
  }

  opt.context = *mContextModelInput;

  CodingOption option(this, mOptions.size());

  mOptions.push_back( std::move(opt) );

  return option;
}


void CodingOptions::start(enum RateEstimationMethod rateMethod)
{
  /* We don't need the input context model anymore.
     Releasing it now may save a copy during a later decouple().
  */
  mContextModelInput->release();

  bool adaptiveContext;
  switch (rateMethod) {
  case Rate_Default:
    adaptiveContext = mECtx->use_adaptive_context;
    break;
  case Rate_FixedContext:
    adaptiveContext = false;
    break;
  case Rate_AdaptiveContext:
    adaptiveContext = true;
    break;
  }

  if (adaptiveContext) {
    /* If we modify the context models in this algorithm,
       we need separate models for each option.
    */
    for (auto& option : mOptions) {
      option.context.decouple();
    }

    cabac = &cabac_adaptive;
  }
  else {
    cabac = &cabac_constant;
  }
}


void CodingOption::begin()
{
  mParent->cabac->reset();
  mParent->cabac->set_context_models( &get_context() );

  if (mParent->mCurrentlyReconstructedOption >= 0) {
    mParent->mOptions[mParent->mCurrentlyReconstructedOption].cb->save(mParent->mECtx->img);
  }

  mParent->mCurrentlyReconstructedOption = mOptionIdx;
}

void CodingOption::end()
{
  assert(mParent->mCurrentlyReconstructedOption == mOptionIdx);
}


void CodingOptions::compute_rdo_costs()
{
  for (int i=0;i<mOptions.size();i++) {
    mOptions[i].rdoCost = mOptions[i].cb->distortion + mECtx->lambda * mOptions[i].cb->rate;
  }
}


enc_cb* CodingOptions::return_best_rdo()
{
  assert(mOptions.size()>0);


  float bestRDOCost = 0;
  bool  first=true;
  int   bestRDO=-1;

  for (int i=0;i<mOptions.size();i++) {
    float cost = mOptions[i].rdoCost;
    if (first || cost < bestRDOCost) {
      bestRDOCost = cost;
      first = false;
      bestRDO = i;
    }
  }


  assert(bestRDO>=0);

  if (bestRDO != mCurrentlyReconstructedOption) {
    mOptions[bestRDO].cb->restore(mECtx->img);
  }

  *mContextModelInput = mOptions[bestRDO].context;


  // delete all CBs except the best one

  for (int i=0;i<mOptions.size();i++) {
    if (i != bestRDO)
      {
        delete mOptions[i].cb;
        mOptions[i].cb = NULL;
      }
  }

  return mOptions[bestRDO].cb;
}
