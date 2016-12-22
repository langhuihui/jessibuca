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


#include "libde265/encoder/analyze.h"
#include "libde265/encoder/encoder-context.h"
#include <assert.h>
#include <limits>
#include <math.h>
#include <iostream>


#define ENCODER_DEVELOPMENT 1


void diff_blk(int16_t* out,int out_stride,
              const uint8_t* a_ptr, int a_stride,
              const uint8_t* b_ptr, int b_stride,
              int blkSize)
{
  for (int by=0;by<blkSize;by++)
    for (int bx=0;bx<blkSize;bx++)
      {
        out[by*out_stride+bx] = a_ptr[by*a_stride+bx] - b_ptr[by*b_stride+bx];
      }
}


static bool has_nonzero_value(const int16_t* data, int n)
{
  for (int i=0;i<n;i++)
    if (data[i]) return true;

  return false;
}


void show_debug_image(const de265_image* input, int slot);

/*
  void encode_transform_unit(encoder_context* ectx,
  enc_tb* tb,
  const de265_image* input,
  //int16_t* residual, int stride,
  int x0,int y0, // luma position
  int log2TbSize, // chroma adapted
  const enc_cb* cb,
  int cIdx)
  {
  }
*/

void analyze_transform_unit(encoder_context* ectx,
                            enc_tb* tb,
                            const de265_image* input, // TODO: probably pass pixels/stride directly
                            //int16_t* residual, int stride,
                            int x0,int y0, // luma position
                            int log2TbSize, // chroma adapted
                            const enc_cb* cb,
                            int cIdx)
{
  int xC = x0;
  int yC = y0;
  int tbSize = 1<<log2TbSize;
  if (cIdx>0) { xC>>=1; yC>>=1; }

  enum PredMode predMode = cb->PredMode;

  int16_t blk[32*32]; // residual

  // --- do intra prediction ---

  if (predMode==MODE_INTRA) {
    enum IntraPredMode intraPredMode;

    if (cIdx==0) {
      intraPredMode = ectx->img->get_IntraPredMode(x0,y0);
    }
    else {
      intraPredMode = cb->intra.chroma_mode;
    }

    decode_intra_prediction(ectx->img, xC,  yC,   intraPredMode,  tbSize  , cIdx);


    // --- subtract prediction from prediction ---

    uint8_t* pred = ectx->img->get_image_plane(cIdx);
    int stride = ectx->img->get_image_stride(cIdx);

    diff_blk(blk,tbSize,
             input->get_image_plane_at_pos(cIdx,xC,yC), input->get_image_stride(cIdx),
             &pred[yC*stride+xC],stride, tbSize);
  }
  else {
    // --- subtract prediction from prediction ---

    uint8_t* pred = ectx->prediction->get_image_plane(cIdx);
    int stride = ectx->prediction->get_image_stride(cIdx);

    //printBlk("input",input->get_image_plane_at_pos(cIdx,xC,yC), tbSize, input->get_image_stride(cIdx));
    //printBlk("prediction", pred,tbSize, stride);

    diff_blk(blk,tbSize,
             input->get_image_plane_at_pos(cIdx,xC,yC), input->get_image_stride(cIdx),
             &pred[yC*stride+xC],stride, tbSize);

    //printBlk("residual", blk,tbSize,tbSize);
  }


  //show_debug_image(ectx->img, 0);




  // --- forward transform ---

  tb->alloc_coeff_memory(cIdx, tbSize);

  int trType;
  if (cIdx==0 && log2TbSize==2 && predMode==MODE_INTRA) trType=1; // TODO: inter mode
  else trType=0;

  fwd_transform(&ectx->acceleration, tb->coeff[cIdx], tbSize, log2TbSize, trType,  blk, tbSize);


  // --- quantization ---

  quant_coefficients(tb->coeff[cIdx], tb->coeff[cIdx], log2TbSize,  cb->qp, true);

  tb->cbf[cIdx] = has_nonzero_value(tb->coeff[cIdx], 1<<(log2TbSize<<1));
}


static void recursive_cbfChroma(CABAC_encoder_estim* cabac,
                                enc_tb* tb, int log2TrafoSize, int trafoDepth)
{
  float bits_pre = cabac->getRDBits();

  // --- CBF CB/CR ---

  // For 4x4 luma, there is no signaling of chroma CBF, because only the
  // chroma CBF for 8x8 is relevant.
  if (log2TrafoSize>2) {
    if (trafoDepth==0 || tb->parent->cbf[1]) {
      encode_cbf_chroma(cabac, trafoDepth, tb->cbf[1]);
    }
    if (trafoDepth==0 || tb->parent->cbf[2]) {
      encode_cbf_chroma(cabac, trafoDepth, tb->cbf[2]);
    }
  }

  if (tb->split_transform_flag) {
    for (int i=0;i<4;i++) {
      recursive_cbfChroma(cabac, tb->children[i], log2TrafoSize-1, trafoDepth+1);
    }
  }

  float bits_post = cabac->getRDBits();

  tb->rate = tb->rate_withoutCbfChroma + (bits_post - bits_pre);
}


enc_tb* encode_transform_tree_no_split(encoder_context* ectx,
                                       context_model_table& ctxModel,
                                       const de265_image* input,
                                       const enc_tb* parent,
                                       enc_cb* cb,
                                       int x0,int y0, int xBase,int yBase, int log2TbSize,
                                       int blkIdx,
                                       int trafoDepth, int MaxTrafoDepth, int IntraSplitFlag)
{
  de265_image* img = ectx->img;

  int stride = ectx->img->get_image_stride(0);

  uint8_t* luma_plane = ectx->img->get_image_plane(0);
  uint8_t* cb_plane = ectx->img->get_image_plane(1);
  uint8_t* cr_plane = ectx->img->get_image_plane(2);

  // --- compute transform coefficients ---

  enc_tb* tb = new enc_tb();

  tb->parent = parent;
  tb->split_transform_flag = false;
  tb->log2Size = log2TbSize;
  tb->x = x0;
  tb->y = y0;
  tb->cbf[0] = tb->cbf[1] = tb->cbf[2] = 0;


  // luma block

  analyze_transform_unit(ectx, tb, input, x0,y0, log2TbSize, cb, 0 /* Y */);


  // chroma blocks

  if (log2TbSize > 2) {
    // if TB is > 4x4, do chroma transform of half size
    analyze_transform_unit(ectx, tb, input, x0,y0, log2TbSize-1, cb, 1 /* Cb */);
    analyze_transform_unit(ectx, tb, input, x0,y0, log2TbSize-1, cb, 2 /* Cr */);
  }
  else if (blkIdx==3) {
    // if TB size is 4x4, do chroma transform for last sub-block
    analyze_transform_unit(ectx, tb, input, xBase,yBase, log2TbSize, cb, 1 /* Cb */);
    analyze_transform_unit(ectx, tb, input, xBase,yBase, log2TbSize, cb, 2 /* Cr */);
  }


  // reconstruction

  tb->reconstruct(ectx, ectx->img, cb, blkIdx);



  // measure rate

  CABAC_encoder_estim estim;
  estim.set_context_models(&ctxModel);


  tb->rate_withoutCbfChroma = 0;

  const seq_parameter_set* sps = &ectx->img->sps;


  if (log2TbSize <= sps->Log2MaxTrafoSize &&
      log2TbSize >  sps->Log2MinTrafoSize &&
      trafoDepth < MaxTrafoDepth &&
      !(IntraSplitFlag && trafoDepth==0))
    {
      encode_split_transform_flag(ectx, &estim, log2TbSize, 0);
      tb->rate_withoutCbfChroma += estim.getRDBits();
      estim.reset();
    }

  // --- CBF CB/CR ---

  if (cb->PredMode == MODE_INTRA || trafoDepth != 0 ||
      tb->cbf[1] || tb->cbf[2]) {
    encode_cbf_luma(&estim, trafoDepth==0, tb->cbf[0]);
  }

  encode_transform_unit(ectx,&estim, tb,cb, x0,y0, xBase,yBase, log2TbSize, trafoDepth, blkIdx);

  tb->rate_withoutCbfChroma += estim.getRDBits();

  estim.reset(); // TODO: not needed ?

  recursive_cbfChroma(&estim,tb,log2TbSize,trafoDepth);

  //float rate_cbfChroma = estim.getRDBits();
  //tb->rate = tb->rate_withoutCbfChroma + rate_cbfChroma;


  // measure distortion

  int tbSize = 1<<log2TbSize;
  tb->distortion = SSD(input->get_image_plane_at_pos(0, x0,y0), input->get_image_stride(0),
                       img  ->get_image_plane_at_pos(0, x0,y0), img  ->get_image_stride(0),
                       tbSize, tbSize);

  return tb;
}


enc_tb* Algo_TB_Split::encode_transform_tree_split(encoder_context* ectx,
                                                   context_model_table& ctxModel,
                                                   const de265_image* input,
                                                   const enc_tb* parent,
                                                   enc_cb* cb,
                                                   int x0,int y0, int log2TbSize,
                                                   int TrafoDepth, int MaxTrafoDepth,
                                                   int IntraSplitFlag)
{
  const de265_image* img = ectx->img;

  enc_tb* tb = new enc_tb();

  tb->parent = parent;
  tb->split_transform_flag = true;
  tb->log2Size = log2TbSize;
  tb->x = x0;
  tb->y = y0;

  tb->distortion = 0;
  tb->rate = 0;
  tb->rate_withoutCbfChroma = 0;


  // Since we try to code all sub-blocks, we enable all CBF flags.
  // Should we see later that the child TBs are zero, we clear those flags later.

  tb->cbf[0]=1;
  tb->cbf[1]=1;
  tb->cbf[2]=1;


  context_model ctxModelCbfChroma[4];
  for (int i=0;i<4;i++) {
    ctxModelCbfChroma[i] = ctxModel[CONTEXT_MODEL_CBF_CHROMA+i];
  }


  // --- encode all child nodes ---

  for (int i=0;i<4;i++) {
    int dx = (i&1)  << (log2TbSize-1);
    int dy = (i>>1) << (log2TbSize-1);

    if (cb->PredMode == MODE_INTRA) {
      tb->children[i] = mAlgo_TB_IntraPredMode->analyze(ectx, ctxModel, input, tb, cb,
                                                        x0+dx, y0+dy, x0,y0,
                                                        log2TbSize-1, i,
                                                        TrafoDepth+1, MaxTrafoDepth, IntraSplitFlag);
    }
    else {
      tb->children[i] = this->analyze(ectx, ctxModel, input, tb, cb,
                                      x0+dx, y0+dy, x0,y0,
                                      log2TbSize-1, i,
                                      TrafoDepth+1, MaxTrafoDepth, IntraSplitFlag);
    }

    tb->distortion            += tb->children[i]->distortion;
    tb->rate_withoutCbfChroma += tb->children[i]->rate_withoutCbfChroma;
  }

  tb->set_cbf_flags_from_children();


  // --- add rate for this TB level ---

  CABAC_encoder_estim estim;
  estim.set_context_models(&ctxModel);




  const seq_parameter_set* sps = &ectx->img->sps;

  if (log2TbSize <= sps->Log2MaxTrafoSize &&
      log2TbSize >  sps->Log2MinTrafoSize &&
      TrafoDepth < MaxTrafoDepth &&
      !(IntraSplitFlag && TrafoDepth==0))
    {
      encode_split_transform_flag(ectx, &estim, log2TbSize, 1);
      tb->rate_withoutCbfChroma += estim.getRDBits();
      estim.reset();
    }

  // restore chroma CBF context models

  for (int i=0;i<4;i++) {
    ctxModel[CONTEXT_MODEL_CBF_CHROMA+i] = ctxModelCbfChroma[i];
  }

  recursive_cbfChroma(&estim,tb, log2TbSize, TrafoDepth);
  //tb->rate = tb->rate_withoutCbfChroma + estim.getRDBits();

  return tb;
}



struct Logging_TB_Split : public Logging
{
  int skipTBSplit, noskipTBSplit;
  int zeroBlockCorrelation[6][2][5];

  const char* name() const { return "tb-split"; }

  void print(const encoder_context* ectx, const char* filename)
  {
    printf("%d %d\n\n",skipTBSplit, noskipTBSplit);

    for (int tb=3;tb<=5;tb++) {
      for (int z=0;z<=1;z++) {
        float total = 0;

        for (int c=0;c<5;c++)
          total += zeroBlockCorrelation[tb][z][c];

        for (int c=0;c<5;c++) {
          printf("%d %d %d : %d %5.2f\n", tb,z,c,
                 zeroBlockCorrelation[tb][z][c],
                 total==0 ? 0 : zeroBlockCorrelation[tb][z][c]/total*100);
        }
      }
    }


    for (int z=0;z<2;z++) {
      printf("\n");
      for (int tb=3;tb<=5;tb++) {
        float total = 0;

        for (int c=0;c<5;c++)
          total += zeroBlockCorrelation[tb][z][c];

        printf("%dx%d ",1<<tb,1<<tb);

        for (int c=0;c<5;c++) {
          printf("%5.2f ", total==0 ? 0 : zeroBlockCorrelation[tb][z][c]/total*100);
        }
        printf("\n");
      }
    }
  }
} logging_tb_split;



enc_tb*
Algo_TB_Split_BruteForce::analyze(encoder_context* ectx,
                                  context_model_table& ctxModel,
                                  const de265_image* input,
                                  const enc_tb* parent,
                                  enc_cb* cb,
                                  int x0,int y0, int xBase,int yBase, int log2TbSize,
                                  int blkIdx,
                                  int TrafoDepth, int MaxTrafoDepth,
                                  int IntraSplitFlag)
{
  bool test_split = (log2TbSize > 2 &&
                     TrafoDepth < MaxTrafoDepth &&
                     log2TbSize > ectx->sps.Log2MinTrafoSize);

  bool test_no_split = true;
  if (IntraSplitFlag && TrafoDepth==0) test_no_split=false; // we have to split
  if (log2TbSize > ectx->sps.Log2MaxTrafoSize) test_no_split=false;

  context_model_table ctxSplit;
  if (test_split) {
    ctxSplit = ctxModel.copy();
  }


  enc_tb* tb_no_split = NULL;
  enc_tb* tb_split    = NULL;
  float rd_cost_no_split = std::numeric_limits<float>::max();
  float rd_cost_split    = std::numeric_limits<float>::max();

  if (test_no_split) {
    tb_no_split = encode_transform_tree_no_split(ectx, ctxModel, input, parent,
                                                 cb, x0,y0, xBase,yBase, log2TbSize,
                                                 blkIdx,
                                                 TrafoDepth,MaxTrafoDepth,IntraSplitFlag);

    rd_cost_no_split = tb_no_split->distortion + ectx->lambda * tb_no_split->rate;

    if (log2TbSize <= mParams.zeroBlockPrune()) {
      bool zeroBlock = tb_no_split->isZeroBlock();

      if (zeroBlock) {
        test_split = false;
        logging_tb_split.skipTBSplit++;
      }
      else
        logging_tb_split.noskipTBSplit++;
    }
  }


  if (test_split) {
    tb_split = encode_transform_tree_split(ectx, ctxSplit, input, parent, cb,
                                           x0,y0, log2TbSize,
                                           TrafoDepth, MaxTrafoDepth, IntraSplitFlag);

    rd_cost_split    = tb_split->distortion    + ectx->lambda * tb_split->rate;
  }


  if (test_split && test_no_split) {
    bool zero_block = tb_no_split->isZeroBlock();

    int nChildZero = 0;
    for (int i=0;i<4;i++) {
      if (tb_split->children[i]->isZeroBlock()) nChildZero++;
    }

    logging_tb_split.zeroBlockCorrelation[log2TbSize][zero_block ? 0 : 1][nChildZero]++;
  }


  bool split = (rd_cost_split < rd_cost_no_split);

  if (split) {
    ctxModel = ctxSplit;

    delete tb_no_split;
    assert(tb_split);
    return tb_split;
  }
  else {
    delete tb_split;
    assert(tb_no_split);
    tb_no_split->reconstruct(ectx, ectx->img,
                             cb, blkIdx);

    return tb_no_split;
  }
}
