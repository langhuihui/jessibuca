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

#include "encode.h"
#include "encoder-context.h"
#include "slice.h"
#include "scan.h"
#include "intrapred.h"
#include "libde265/transform.h"
#include "libde265/fallback-dct.h"
#include <iostream>


int allocTB = 0;
int allocCB = 0;

#define DEBUG_ALLOCS 0



void enc_node::save(const de265_image* img)
{
  delete[] mReconstruction;

  int blkSize = Log2SizeToArea(log2Size);
  mReconstruction = new uint8_t[blkSize * 3/2];

  int w = 1<<log2Size;

  copy_subimage(mReconstruction, w,
                img->get_image_plane_at_pos(0, x,y),
                img->get_image_stride(0),
                w,w);

  copy_subimage(mReconstruction + blkSize, w>>1,
                img->get_image_plane_at_pos(1, x>>1,y>>1),
                img->get_image_stride(1),
                w>>1,w>>1);

  copy_subimage(mReconstruction + blkSize*5/4, w>>1,
                img->get_image_plane_at_pos(2, x>>1,y>>1),
                img->get_image_stride(2),
                w>>1,w>>1);
}


void enc_node::restore(de265_image* img)
{
  assert(mReconstruction);

  int blkSize = Log2SizeToArea(log2Size);
  int w = 1<<log2Size;

  copy_subimage(img->get_image_plane_at_pos(0, x,y),
                img->get_image_stride(0),
                mReconstruction, w,
                w,w);

  copy_subimage(img->get_image_plane_at_pos(1, x>>1,y>>1),
                img->get_image_stride(1),
                mReconstruction + blkSize, w>>1,
                w>>1,w>>1);

  copy_subimage(img->get_image_plane_at_pos(2, x>>1,y>>1),
                img->get_image_stride(2),
                mReconstruction + blkSize*5/4, w>>1,
                w>>1,w>>1);
}


void enc_cb::save(const de265_image* img)
{
  enc_node::save(img);

  // TODO: save metadata in node buffer memory
}


void enc_cb::restore(de265_image* img)
{
  enc_node::restore(img);

  // write back all the metadata

  write_to_image(img);
}


void enc_cb::set_rqt_root_bf_from_children_cbf()
{
  assert(transform_tree);
  inter.rqt_root_cbf = (transform_tree->cbf[0] |
                        transform_tree->cbf[1] |
                        transform_tree->cbf[2]);
}




alloc_pool enc_tb::mMemPool(sizeof(enc_tb));

enc_tb::enc_tb()
  : split_transform_flag(false)
{
  coeff[0]=coeff[1]=coeff[2]=NULL;

  if (DEBUG_ALLOCS) { allocTB++; printf("TB  : %d\n",allocTB); }
}


enc_tb::~enc_tb()
{
  if (split_transform_flag) {
    for (int i=0;i<4;i++) {
      delete children[i];
    }
  }
  else {
    for (int i=0;i<3;i++) {
      delete[] coeff[i];
    }
  }

  if (DEBUG_ALLOCS) { allocTB--; printf("TB ~: %d\n",allocTB); }
}


void enc_tb::alloc_coeff_memory(int cIdx, int tbSize)
{
  assert(coeff[cIdx]==NULL);
  coeff[cIdx] = new int16_t[tbSize*tbSize];
}


void enc_tb::reconstruct_tb(encoder_context* ectx,
                            de265_image* img,
                            int x0,int y0,  // luma
                            int log2TbSize, // chroma adapted
                            const enc_cb* cb, int cIdx) const
{
  // chroma adapted position
  int xC=x0;
  int yC=y0;

  if (cIdx>0) {
    xC>>=1;
    yC>>=1;
  }

  if (cb->PredMode == MODE_INTRA) {

    enum IntraPredMode intraPredMode  = img->get_IntraPredMode(x0,y0);

    if (cIdx>0) {
      intraPredMode = cb->intra.chroma_mode;
      //intraPredMode = lumaPredMode_to_chromaPredMode(intraPredMode, cb->intra.chroma_mode);
    }

    decode_intra_prediction(img, xC,yC,  intraPredMode, 1<< log2TbSize   , cIdx);
  }
  else {
    int size = 1<<log2TbSize;

    uint8_t* dst_ptr  = img->get_image_plane_at_pos(cIdx, xC,  yC  );
    int dst_stride  = img->get_image_stride(cIdx);

    uint8_t* src_ptr  = ectx->prediction->get_image_plane_at_pos(cIdx, xC,  yC  );
    int src_stride  = ectx->prediction->get_image_stride(cIdx);

    for (int y=0;y<size;y++) {
      for (int x=0;x<size;x++) {
        dst_ptr[y*dst_stride+x] = src_ptr[y*src_stride+x];
      }
    }
  }

  ALIGNED_16(int16_t) dequant_coeff[32*32];

  if (cbf[cIdx]) dequant_coefficients(dequant_coeff, coeff[cIdx], log2TbSize, cb->qp);

  //printf("--- quantized coeffs ---\n");
  //printBlk(coeff[0],1<<log2BlkSize,1<<log2BlkSize);

  //printf("--- dequantized coeffs ---\n");
  //printBlk(dequant_coeff[0],1<<log2BlkSize,1<<log2BlkSize);

  //printf("--- plane at %d %d / %d ---\n",x0,y0,cIdx);

  uint8_t* ptr  = img->get_image_plane_at_pos(cIdx, xC,  yC  );
  int stride  = img->get_image_stride(cIdx);

  int trType = (cIdx==0 && log2TbSize==2); // TODO: inter

  //printf("--- prediction %d %d / %d ---\n",x0,y0,cIdx);
  //printBlk("prediction",ptr,1<<log2TbSize,stride);

  if (cbf[cIdx]) inv_transform(&ectx->acceleration,
                               ptr,stride,   dequant_coeff, log2TbSize,   trType);


  //printf("--- RECO intra prediction %d %d ---\n",x0,y0);
  //printBlk("prediction",ptr,1<<log2TbSize,stride);

  //dequant_and_add_transform(accel, img, x0,y0, qp);

  //printf("--- RECO add residual %d %d ---\n",x0,y0);
  //img->printBlk(x0,y0,0,log2CbSize);
}


void enc_tb::reconstruct(encoder_context* ectx,
                         de265_image* img,
                         const enc_cb* cb,
                         int blkIdx) const
{
  if (split_transform_flag) {
    for (int i=0;i<4;i++) {
      children[i]->reconstruct(ectx,img,
                               cb, i);
    }
  }
  else {
    reconstruct_tb(ectx, img, x,y, log2Size, cb, 0);

    if (log2Size>2) {
      reconstruct_tb(ectx, img, x,y, log2Size-1, cb, 1);
      reconstruct_tb(ectx, img, x,y, log2Size-1, cb, 2);
    }
    else if (blkIdx==3) {
      int xBase = x - (1<<log2Size);
      int yBase = y - (1<<log2Size);

      reconstruct_tb(ectx, img, xBase,yBase, log2Size, cb, 1);
      reconstruct_tb(ectx, img, xBase,yBase, log2Size, cb, 2);
    }
  }
}


void enc_tb::set_cbf_flags_from_children()
{
  assert(split_transform_flag);

  cbf[0] = 0;
  cbf[1] = 0;
  cbf[2] = 0;

  for (int i=0;i<4;i++) {
    cbf[0] |= children[i]->cbf[0];
    cbf[1] |= children[i]->cbf[1];
    cbf[2] |= children[i]->cbf[2];
  }
}




alloc_pool enc_cb::mMemPool(sizeof(enc_cb), 200);


enc_cb::enc_cb()
  : split_cu_flag(false),
    cu_transquant_bypass_flag(false),
    pcm_flag(false),
    transform_tree(NULL),
    distortion(0),
    rate(0)
{
  if (DEBUG_ALLOCS) { allocCB++; printf("CB  : %d\n",allocCB); }
}

enc_cb::~enc_cb()
{
  if (split_cu_flag) {
    for (int i=0;i<4;i++) {
      delete children[i];
    }
  }
  else {
    delete transform_tree;
  }

  if (DEBUG_ALLOCS) { allocCB--; printf("CB ~: %d\n",allocCB); }
}


void enc_cb::write_to_image(de265_image* img) const
{
  //printf("write_to_image %d %d size:%d\n",x,y,1<<log2Size);


  if (!split_cu_flag) {
    img->set_log2CbSize(x,y,log2Size, true);
    img->set_ctDepth(x,y,log2Size, ctDepth);
    assert(pcm_flag==0);
    img->set_pcm_flag(x,y,log2Size, pcm_flag);
    img->set_cu_transquant_bypass(x,y,log2Size, cu_transquant_bypass_flag);
    img->set_QPY(x,y,log2Size, qp);
    img->set_pred_mode(x,y, log2Size, PredMode);
    img->set_PartMode(x,y, PartMode);

    if (PredMode == MODE_INTRA) {
      //img->set_ChromaIntraPredMode(x,y,log2Size, intra.chroma_mode);

      if (PartMode == PART_NxN) {
        int h = 1<<(log2Size-1);
        img->set_IntraPredMode(x  ,y  ,log2Size-1, intra.pred_mode[0]);
        img->set_IntraPredMode(x+h,y  ,log2Size-1, intra.pred_mode[1]);
        img->set_IntraPredMode(x  ,y+h,log2Size-1, intra.pred_mode[2]);
        img->set_IntraPredMode(x+h,y+h,log2Size-1, intra.pred_mode[3]);
      }
      else {
        img->set_IntraPredMode(x,y,log2Size, intra.pred_mode[0]);
      }
    }
    else {
      int nC = 1<<log2Size;
      int nC2 = nC>>1;
      int nC4 = nC>>2;
      int nC3 = nC-nC4;
      switch (PartMode) {
      case PART_2Nx2N:
        img->set_mv_info(x,y,nC,nC, inter.pb[0].motion);
        break;
      case PART_NxN:
        img->set_mv_info(x    ,y    ,nC2,nC2, inter.pb[0].motion);
        img->set_mv_info(x+nC2,y    ,nC2,nC2, inter.pb[1].motion);
        img->set_mv_info(x    ,y+nC2,nC2,nC2, inter.pb[2].motion);
        img->set_mv_info(x+nC2,y+nC2,nC2,nC2, inter.pb[3].motion);
        break;
      case PART_2NxN:
        img->set_mv_info(x,y    ,nC,nC2, inter.pb[0].motion);
        img->set_mv_info(x,y+nC2,nC,nC2, inter.pb[1].motion);
        break;
      case PART_Nx2N:
        img->set_mv_info(x    ,y,nC2,nC, inter.pb[0].motion);
        img->set_mv_info(x+nC2,y,nC2,nC, inter.pb[1].motion);
        break;
      case PART_2NxnU:
        img->set_mv_info(x,y    ,nC,nC4, inter.pb[0].motion);
        img->set_mv_info(x,y+nC4,nC,nC3, inter.pb[1].motion);
        break;
      case PART_2NxnD:
        img->set_mv_info(x,y    ,nC,nC3, inter.pb[0].motion);
        img->set_mv_info(x,y+nC3,nC,nC4, inter.pb[1].motion);
        break;
      case PART_nLx2N:
        img->set_mv_info(x    ,y,nC4,nC, inter.pb[0].motion);
        img->set_mv_info(x+nC4,y,nC3,nC, inter.pb[1].motion);
        break;
      case PART_nRx2N:
        img->set_mv_info(x    ,y,nC3,nC, inter.pb[0].motion);
        img->set_mv_info(x+nC3,y,nC4,nC, inter.pb[1].motion);
        break;
      }
    }
  }
  else {
    for (int i=0;i<4;i++) {
      if (children[i]) {
        children[i]->write_to_image(img);
      }
    }
  }
}


void enc_cb::reconstruct(encoder_context* ectx, de265_image* img) const
{
  if (split_cu_flag) {
    for (int i=0;i<4;i++) {
      children[i]->reconstruct(ectx, img);
    }
  }
  else {
    write_to_image(img);
    transform_tree->reconstruct(ectx,img,this,0);
  }
}



void encode_split_cu_flag(encoder_context* ectx,
                          CABAC_encoder* cabac,
                          int x0, int y0, int ctDepth, int split_flag)
{
  logtrace(LogSymbols,"$1 split_cu_flag=%d\n",split_flag);

  // check if neighbors are available

  int availableL = check_CTB_available(ectx->img, x0,y0, x0-1,y0);
  int availableA = check_CTB_available(ectx->img, x0,y0, x0,y0-1);

  int condL = 0;
  int condA = 0;

  if (availableL && ectx->img->get_ctDepth(x0-1,y0) > ctDepth) condL=1;
  if (availableA && ectx->img->get_ctDepth(x0,y0-1) > ctDepth) condA=1;

  int contextOffset = condL + condA;
  int context = contextOffset;

  // decode bit

  logtrace(LogSlice,"> split_cu_flag = %d (context=%d)\n",split_flag,context);

  cabac->write_CABAC_bit(CONTEXT_MODEL_SPLIT_CU_FLAG + context, split_flag);
}


void encode_part_mode(encoder_context* ectx,
                      CABAC_encoder* cabac,
                      enum PredMode PredMode, enum PartMode PartMode, int cLog2CbSize)
{
  logtrace(LogSymbols,"$1 part_mode=%d\n",PartMode);
  logtrace(LogSlice,"> part_mode = %d\n",PartMode);

  if (PredMode == MODE_INTRA) {
    int bin = (PartMode==PART_2Nx2N);
    cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+0, bin);
  }
  else {
    if (PartMode==PART_2Nx2N) {
      cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+0, 1);
      return;
    }
    else {
      cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+0, 0);
    }

    if (cLog2CbSize > ectx->sps.Log2MinCbSizeY) {
      if (ectx->sps.amp_enabled_flag) {
        switch (PartMode) {
        case PART_2NxN:
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+1, 1);
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+3, 1);
          break;
        case PART_Nx2N:
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+1, 0);
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+3, 1);
          break;
        case PART_2NxnU:
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+1, 1);
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+3, 0);
          cabac->write_CABAC_bypass(0);
          break;
        case PART_2NxnD:
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+1, 1);
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+3, 0);
          cabac->write_CABAC_bypass(1);
          break;
        case PART_nLx2N:
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+1, 0);
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+3, 0);
          cabac->write_CABAC_bypass(0);
          break;
        case PART_nRx2N:
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+1, 0);
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+3, 0);
          cabac->write_CABAC_bypass(1);
          break;
        case PART_NxN:
        case PART_2Nx2N:
          assert(false);
          break;
        }
      }
      else {
        if (PartMode==PART_2NxN) {
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+1, 1);
        }
        else {
          assert(PartMode==PART_Nx2N);
          cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+1, 0);
        }
      }
    }
    else {
      if (PartMode==PART_2NxN) {
        cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+1, 1);
      }
      else {
        cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+1, 0);

        if (cLog2CbSize==3) {
          assert(PartMode==PART_Nx2N);
        }
        else {
          if (PartMode==PART_Nx2N) {
            cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+3, 1);
          }
          else {
            assert(PartMode==PART_NxN);
            cabac->write_CABAC_bit(CONTEXT_MODEL_PART_MODE+3, 0);
          }
        }
      }
    }
  }
}


static void encode_pred_mode_flag(encoder_context* ectx,
                                  CABAC_encoder* cabac,
                                  enum PredMode PredMode)
{
  logtrace(LogSlice,"> pred_mode = %d\n",PredMode);

  int flag = (PredMode == MODE_INTRA) ? 1 : 0;

  logtrace(LogSymbols,"$1 pred_mode=%d\n",flag);

  cabac->write_CABAC_bit(CONTEXT_MODEL_PRED_MODE_FLAG, flag);
}


static void encode_prev_intra_luma_pred_flag(encoder_context* ectx,
                                             CABAC_encoder* cabac,
                                             int intraPred)
{
  logtrace(LogSymbols,"$1 prev_intra_luma_pred_flag=%d\n",intraPred>=0);
  int bin = (intraPred>=0);

  logtrace(LogSlice,"> prev_intra_luma_pred_flag = %d\n",bin);

  cabac->write_CABAC_bit(CONTEXT_MODEL_PREV_INTRA_LUMA_PRED_FLAG, bin);
}

static void encode_intra_mpm_or_rem(encoder_context* ectx,
                                    CABAC_encoder* cabac,
                                    int intraPred)
{
  if (intraPred>=0) {
    logtrace(LogSymbols,"$1 mpm_idx=%d\n",intraPred);
    logtrace(LogSlice,"> mpm_idx = %d\n",intraPred);
    assert(intraPred<=2);
    cabac->write_CABAC_TU_bypass(intraPred, 2);
  }
  else {
    logtrace(LogSymbols,"$1 rem_intra_luma_pred_mode=%d\n",-intraPred-1);
    logtrace(LogSlice,"> rem_intra_luma_pred_mode = %d\n",-intraPred-1);
    cabac->write_CABAC_FL_bypass(-intraPred-1, 5);
  }
}


static void encode_intra_chroma_pred_mode(encoder_context* ectx,
                                          CABAC_encoder* cabac,
                                          int mode)
{
  logtrace(LogSymbols,"$1 intra_chroma_pred_mode=%d\n",mode);
  logtrace(LogSlice,"> intra_chroma_pred_mode = %d\n",mode);

  if (mode==4) {
    cabac->write_CABAC_bit(CONTEXT_MODEL_INTRA_CHROMA_PRED_MODE,0);
  }
  else {
    assert(mode<4);

    cabac->write_CABAC_bit(CONTEXT_MODEL_INTRA_CHROMA_PRED_MODE,1);
    cabac->write_CABAC_FL_bypass(mode, 2);
  }
}


/* Optimized variant that tests most likely branch first.
 */
enum IntraChromaPredMode find_chroma_pred_mode(enum IntraPredMode chroma_mode,
                                               enum IntraPredMode luma_mode)
{
  // most likely mode: chroma mode = luma mode

  if (luma_mode==chroma_mode) {
    return INTRA_CHROMA_LIKE_LUMA;
  }


  // check remaining candidates

  IntraPredMode mode = chroma_mode;

  // angular-34 is coded by setting the coded mode equal to the luma_mode
  if (chroma_mode == INTRA_ANGULAR_34) {
    mode = luma_mode;
  }

  switch (mode) {
  case INTRA_PLANAR:     return INTRA_CHROMA_PLANAR_OR_34;
  case INTRA_ANGULAR_26: return INTRA_CHROMA_ANGULAR_26_OR_34;
  case INTRA_ANGULAR_10: return INTRA_CHROMA_ANGULAR_10_OR_34;
  case INTRA_DC:         return INTRA_CHROMA_DC_OR_34;
  default:
    assert(false);
    return INTRA_CHROMA_DC_OR_34;
  }
}



void encode_split_transform_flag(encoder_context* ectx,
                                 CABAC_encoder* cabac,
                                 int log2TrafoSize, int split_flag)
{
  logtrace(LogSymbols,"$1 split_transform_flag=%d\n",split_flag);
  logtrace(LogSlice,"> split_transform_flag = %d\n",split_flag);

  int context = 5-log2TrafoSize;
  assert(context >= 0 && context <= 2);

  cabac->write_CABAC_bit(CONTEXT_MODEL_SPLIT_TRANSFORM_FLAG + context, split_flag);
}


void encode_cbf_luma(CABAC_encoder* cabac,
                     bool zeroTrafoDepth, int cbf_luma)
{
  logtrace(LogSymbols,"$1 cbf_luma=%d\n",cbf_luma);
  logtrace(LogSlice,"> cbf_luma = %d\n",cbf_luma);

  int context = (zeroTrafoDepth ? 1 : 0);

  cabac->write_CABAC_bit(CONTEXT_MODEL_CBF_LUMA + context, cbf_luma);
}


void encode_cbf_chroma(CABAC_encoder* cabac,
                       int trafoDepth, int cbf_chroma)
{
  logtrace(LogSymbols,"$1 cbf_chroma=%d\n",cbf_chroma);
  logtrace(LogSlice,"> cbf_chroma = %d\n",cbf_chroma);

  int context = trafoDepth;
  assert(context >= 0 && context <= 3);

  cabac->write_CABAC_bit(CONTEXT_MODEL_CBF_CHROMA + context, cbf_chroma);
}

static inline void encode_coded_sub_block_flag(encoder_context* ectx,
                                               CABAC_encoder* cabac,
                                               int cIdx,
                                               uint8_t coded_sub_block_neighbors,
                                               int flag)
{
  logtrace(LogSymbols,"$1 coded_sub_block_flag=%d\n",flag);
  logtrace(LogSlice,"# coded_sub_block_flag = %d\n",flag);

  // tricky computation of csbfCtx
  int csbfCtx = ((coded_sub_block_neighbors &  1) |  // right neighbor set  or
                 (coded_sub_block_neighbors >> 1));  // bottom neighbor set   -> csbfCtx=1

  int ctxIdxInc = csbfCtx;
  if (cIdx!=0) {
    ctxIdxInc += 2;
  }

  cabac->write_CABAC_bit(CONTEXT_MODEL_CODED_SUB_BLOCK_FLAG + ctxIdxInc, flag);
}

static inline void encode_significant_coeff_flag_lookup(encoder_context* ectx,
                                                        CABAC_encoder* cabac,
                                                        uint8_t ctxIdxInc,
                                                        int significantFlag)
{
  logtrace(LogSymbols,"$1 significant_coeff_flag=%d\n",significantFlag);
  logtrace(LogSlice,"# significant_coeff_flag = significantFlag\n");
  logtrace(LogSlice,"context: %d\n",ctxIdxInc);

  cabac->write_CABAC_bit(CONTEXT_MODEL_SIGNIFICANT_COEFF_FLAG + ctxIdxInc, significantFlag);
}

static inline void encode_coeff_abs_level_greater1(encoder_context* ectx,
                                                   CABAC_encoder* cabac,
                                                   int cIdx, int i,
                                                   bool firstCoeffInSubblock,
                                                   bool firstSubblock,
                                                   int  lastSubblock_greater1Ctx,
                                                   int* lastInvocation_greater1Ctx,
                                                   int* lastInvocation_coeff_abs_level_greater1_flag,
                                                   int* lastInvocation_ctxSet, int c1,
                                                   int value)
{
  logtrace(LogSymbols,"$1 coeff_abs_level_greater1=%d\n",value);
  logtrace(LogSlice,"# coeff_abs_level_greater1 = %d\n",value);

  logtrace(LogSlice,"  cIdx:%d i:%d firstCoeffInSB:%d firstSB:%d lastSB>1:%d last>1Ctx:%d lastLev>1:%d lastCtxSet:%d\n", cIdx,i,firstCoeffInSubblock,firstSubblock,lastSubblock_greater1Ctx,
	   *lastInvocation_greater1Ctx,
	   *lastInvocation_coeff_abs_level_greater1_flag,
	   *lastInvocation_ctxSet);

  int lastGreater1Ctx;
  int greater1Ctx;
  int ctxSet;

  logtrace(LogSlice,"c1: %d\n",c1);

  if (firstCoeffInSubblock) {
    // block with real DC -> ctx 0
    if (i==0 || cIdx>0) { ctxSet=0; }
    else { ctxSet=2; }

    if (firstSubblock) { lastGreater1Ctx=1; }
    else { lastGreater1Ctx = lastSubblock_greater1Ctx; }

    if (lastGreater1Ctx==0) { ctxSet++; }

    logtrace(LogSlice,"ctxSet: %d\n",ctxSet);

    greater1Ctx=1;
  }
  else { // !firstCoeffInSubblock
    ctxSet = *lastInvocation_ctxSet;
    logtrace(LogSlice,"ctxSet (old): %d\n",ctxSet);

    greater1Ctx = *lastInvocation_greater1Ctx;
    if (greater1Ctx>0) {
      int lastGreater1Flag=*lastInvocation_coeff_abs_level_greater1_flag;
      if (lastGreater1Flag==1) greater1Ctx=0;
      else { /*if (greater1Ctx>0)*/ greater1Ctx++; }
    }
  }

  ctxSet = c1; // use HM algo

  int ctxIdxInc = (ctxSet*4) + (greater1Ctx>=3 ? 3 : greater1Ctx);

  if (cIdx>0) { ctxIdxInc+=16; }

  cabac->write_CABAC_bit(CONTEXT_MODEL_COEFF_ABS_LEVEL_GREATER1_FLAG + ctxIdxInc,  value);

  *lastInvocation_greater1Ctx = greater1Ctx;
  *lastInvocation_coeff_abs_level_greater1_flag = value;
  *lastInvocation_ctxSet = ctxSet;
}

static void encode_coeff_abs_level_greater2(encoder_context* ectx,
                                            CABAC_encoder* cabac,
                                            int cIdx, // int i,int n,
                                            int ctxSet,
                                            int value)
{
  logtrace(LogSymbols,"$1 coeff_abs_level_greater2=%d\n",value);
  logtrace(LogSlice,"# coeff_abs_level_greater2 = %d\n",value);

  int ctxIdxInc = ctxSet;

  if (cIdx>0) ctxIdxInc+=4;

  cabac->write_CABAC_bit(CONTEXT_MODEL_COEFF_ABS_LEVEL_GREATER2_FLAG + ctxIdxInc,  value);
}


bool TU(int val, int maxi)
{
  for (int i=0;i<val;i++) {
    printf("1");
  }
  if (val<maxi) { printf("0"); return false; }
  else return true;
}

void bin(int val, int bits)
{
  for (int i=0;i<bits;i++) {
    int bit = (1<<(bits-1-i));
    if (val&bit) printf("1"); else printf("0");
  }
}

void ExpG(int level, int riceParam)
{
  int prefix = level >> riceParam;
  int suffix = level - (prefix<<riceParam);

  //printf("%d %d ",prefix,suffix);

  int base=0;
  int range=1;
  int nBits=0;
  while (prefix >= base+range) {
    printf("1");
    base+=range;
    range*=2;
    nBits++;
  }

  printf("0.");
  bin(prefix-base, nBits);
  printf(":");
  bin(suffix,riceParam);
}

int blamain()
{
  int riceParam=2;
  int TRMax = 4<<riceParam;

  for (int level=0;level<128;level++)
    {
      printf("%d: ",level);

      int prefixPart = std::min(TRMax, level);

      // code TR prefix

      bool isMaxi = TU(prefixPart>>riceParam, TRMax>>riceParam);
      printf(":");
      if (TRMax>prefixPart) {
        int remain = prefixPart & ((1<<riceParam)-1);
        bin(remain, riceParam);
      }
      printf("|");

      if (isMaxi) {
        ExpG(level-TRMax, riceParam+1);
      }

      printf("\n");
    }

  return 0;
}


static void encode_coeff_abs_level_remaining(encoder_context* ectx,
                                             CABAC_encoder* cabac,
                                             int cRiceParam,
                                             int level)
{
  logtrace(LogSymbols,"$1 coeff_abs_level_remaining=%d\n",level);
  logtrace(LogSlice,"# encode_coeff_abs_level_remaining = %d\n",level);

  int cTRMax = 4<<cRiceParam;
  int prefixPart = std::min(level, cTRMax);

  // --- code prefix with TR ---

  // TU part, length 4 (cTRMax>>riceParam)

  int nOnes = (prefixPart>>cRiceParam);
  cabac->write_CABAC_TU_bypass(nOnes, 4);

  // TR suffix

  if (cTRMax > prefixPart) {
    int remain = prefixPart & ((1<<cRiceParam)-1);
    cabac->write_CABAC_FL_bypass(remain, cRiceParam);
  }


  // --- remainder suffix ---

  if (nOnes==4) {
    int remain = level-cTRMax;
    int ExpGRiceParam = cRiceParam+1;

    int prefix = remain >> ExpGRiceParam;
    int suffix = remain - (prefix<<ExpGRiceParam);

    int base=0;
    int range=1;
    int nBits=0;
    while (prefix >= base+range) {
      cabac->write_CABAC_bypass(1);
      base+=range;
      range*=2;
      nBits++;
    }

    cabac->write_CABAC_bypass(0);
    cabac->write_CABAC_FL_bypass(prefix-base, nBits);
    cabac->write_CABAC_FL_bypass(suffix, ExpGRiceParam);
  }
}

// ---------------------------------------------------------------------------

void findLastSignificantCoeff(const position* sbScan, const position* cScan,
                              const int16_t* coeff, int log2TrafoSize,
                              int* lastSignificantX, int* lastSignificantY,
                              int* lastSb, int* lastPos)
{
  int nSb = 1<<((log2TrafoSize-2)<<1); // number of sub-blocks

  // find last significant coefficient

  for (int i=nSb ; i-->0 ;) {
    int x0 = sbScan[i].x << 2;
    int y0 = sbScan[i].y << 2;
    for (int c=16 ; c-->0 ;) {
      int x = x0 + cScan[c].x;
      int y = y0 + cScan[c].y;

      if (coeff[x+(y<<log2TrafoSize)]) {
        *lastSignificantX = x;
        *lastSignificantY = y;
        *lastSb = i;
        *lastPos= c;

        logtrace(LogSlice,"last significant coeff at: %d;%d, Sb:%d Pos:%d\n", x,y,i,c);

        return;
      }
    }
  }

  // all coefficients == 0 ? cannot be since cbf should be false in this case
  assert(false);
}


bool subblock_has_nonzero_coefficient(const int16_t* coeff, int coeffStride,
                                      const position& sbPos)
{
  int x0 = sbPos.x << 2;
  int y0 = sbPos.y << 2;

  coeff += x0 + y0*coeffStride;

  for (int y=0;y<4;y++) {
    if (coeff[0] || coeff[1] || coeff[2] || coeff[3]) { return true; }
    coeff += coeffStride;
  }

  return false;
}

/*
  Example 16x16:  prefix in [0;7]

  prefix       | last pos
  =============|=============
  0            |   0
  1            |   1
  2            |   2
  3            |   3
  -------------+-------------
     lsb nBits |
  4   0    1   |   4, 5
  5   1    1   |   6, 7
  6   0    2   |   8, 9,10,11
  7   1    2   |  12,13,14,15
*/
void encode_last_signficiant_coeff_prefix(encoder_context* ectx,
                                          CABAC_encoder* cabac,
                                          int log2TrafoSize,
                                          int cIdx, int lastSignificant,
                                          int context_model_index)
{
  logtrace(LogSlice,"> last_significant_coeff_prefix=%d log2TrafoSize:%d cIdx:%d\n",
           lastSignificant,log2TrafoSize,cIdx);

  int cMax = (log2TrafoSize<<1)-1;

  int ctxOffset, ctxShift;
  if (cIdx==0) {
    ctxOffset = 3*(log2TrafoSize-2) + ((log2TrafoSize-1)>>2);
    ctxShift  = (log2TrafoSize+1)>>2;
  }
  else {
    ctxOffset = 15;
    ctxShift  = log2TrafoSize-2;
  }

  for (int binIdx=0;binIdx<lastSignificant;binIdx++)
    {
      int ctxIdxInc = (binIdx >> ctxShift);
      cabac->write_CABAC_bit(context_model_index + ctxOffset + ctxIdxInc, 1);
    }

  if (lastSignificant != cMax) {
    int binIdx = lastSignificant;
    int ctxIdxInc = (binIdx >> ctxShift);
    cabac->write_CABAC_bit(context_model_index + ctxOffset + ctxIdxInc, 0);
  }
}


void split_last_significant_position(int pos, int* prefix, int* suffix, int* nSuffixBits)
{
  logtrace(LogSlice,"split position %d : ",pos);

  // most frequent case

  if (pos<=3) {
    *prefix=pos;
    *suffix=-1; // just to have some defined value
    *nSuffixBits=0;
    logtrace(LogSlice,"prefix=%d suffix=%d (%d bits)\n",*prefix,*suffix,*nSuffixBits);
    return;
  }

  pos -= 4;
  int nBits=1;
  int range=4;
  while (pos>=range) {
    nBits++;
    pos-=range;
    range<<=1;
  }

  *prefix = (1+nBits)<<1;
  if (pos >= (range>>1)) {
    *prefix |= 1;
    pos -= (range>>1);
  }
  *suffix = pos;
  *nSuffixBits = nBits;

  logtrace(LogSlice,"prefix=%d suffix=%d (%d bits)\n",*prefix,*suffix,*nSuffixBits);
}


extern uint8_t* ctxIdxLookup[4 /* 4-log2-32 */][2 /* !!cIdx */][2 /* !!scanIdx */][4 /* prevCsbf */];

/* These values are read from the image metadata:
   - intra prediction mode (x0;y0)
 */
void encode_residual(encoder_context* ectx,
                     CABAC_encoder* cabac,
                     const enc_tb* tb, const enc_cb* cb,
                     int x0,int y0,int log2TrafoSize,int cIdx)
{
  const de265_image* img = ectx->img;
  const seq_parameter_set& sps = img->sps;
  const pic_parameter_set& pps = img->pps;

  int16_t* coeff = tb->coeff[cIdx];

  if (pps.transform_skip_enabled_flag && true /* TODO */) {
  }


#if 0
  printf("write coefficients\n");
  for (int y=0;y<(1<<log2TrafoSize);y++)
    {
      for (int x=0;x<(1<<log2TrafoSize);x++)
        {
          printf("%4d ",coeff[x+y*(1<<log2TrafoSize)]);
        }
      printf("\n");
    }
#endif


  // --- get scan orders ---

  enum PredMode PredMode = cb->PredMode;
  int scanIdx;

  if (PredMode == MODE_INTRA) {
    if (cIdx==0) {
      scanIdx = get_intra_scan_idx_luma(log2TrafoSize, img->get_IntraPredMode(x0,y0));
    }
    else {
      enum IntraPredMode chromaMode = cb->intra.chroma_mode;
      /*
      enum IntraPredMode chromaMode = lumaPredMode_to_chromaPredMode(img->get_IntraPredMode(x0,y0),
                                                                     cb->intra.chroma_mode);
      */
      scanIdx = get_intra_scan_idx_chroma(log2TrafoSize, chromaMode);
    }
  }
  else {
    scanIdx=0;
  }


  const position* ScanOrderSub = get_scan_order(log2TrafoSize-2, scanIdx);
  const position* ScanOrderPos = get_scan_order(2, scanIdx);

  int lastSignificantX, lastSignificantY;
  int lastScanPos;
  int lastSubBlock;
  findLastSignificantCoeff(ScanOrderSub, ScanOrderPos,
                           coeff, log2TrafoSize,
                           &lastSignificantX, &lastSignificantY,
                           &lastSubBlock, &lastScanPos);

  int codedSignificantX = lastSignificantX;
  int codedSignificantY = lastSignificantY;

  if (scanIdx==2) {
    std::swap(codedSignificantX, codedSignificantY);
  }



  int prefixX, suffixX, suffixBitsX;
  int prefixY, suffixY, suffixBitsY;

  split_last_significant_position(codedSignificantX, &prefixX,&suffixX,&suffixBitsX);
  split_last_significant_position(codedSignificantY, &prefixY,&suffixY,&suffixBitsY);

  encode_last_signficiant_coeff_prefix(ectx, cabac, log2TrafoSize, cIdx, prefixX,
                                       CONTEXT_MODEL_LAST_SIGNIFICANT_COEFFICIENT_X_PREFIX);

  encode_last_signficiant_coeff_prefix(ectx, cabac, log2TrafoSize, cIdx, prefixY,
                                       CONTEXT_MODEL_LAST_SIGNIFICANT_COEFFICIENT_Y_PREFIX);


  if (codedSignificantX > 3) {
    cabac->write_CABAC_FL_bypass(suffixX, suffixBitsX);
  }
  if (codedSignificantY > 3) {
    cabac->write_CABAC_FL_bypass(suffixY, suffixBitsY);
  }



  int sbWidth = 1<<(log2TrafoSize-2);
  int CoeffStride = 1<<log2TrafoSize;

  uint8_t coded_sub_block_neighbors[32/4*32/4];  // 64*2 flags
  memset(coded_sub_block_neighbors,0,sbWidth*sbWidth);

  int  c1 = 1;
  bool firstSubblock = true;           // for coeff_abs_level_greater1_flag context model
  int  lastSubblock_greater1Ctx=false; /* for coeff_abs_level_greater1_flag context model
                                          (initialization not strictly needed)
                                       */

  int  lastInvocation_greater1Ctx=0;
  int  lastInvocation_coeff_abs_level_greater1_flag=0;
  int  lastInvocation_ctxSet=0;



  // ----- encode coefficients -----

  //tctx->nCoeff[cIdx] = 0;


  // i - subblock index
  // n - coefficient index in subblock

  for (int i=lastSubBlock;i>=0;i--) {
    position S = ScanOrderSub[i];
    int inferSbDcSigCoeffFlag=0;

    logtrace(LogSlice,"sub block scan idx: %d\n",i);


    // --- check whether this sub-block has to be coded ---

    int sub_block_is_coded = 0;

    if ((i<lastSubBlock) && (i>0)) {
      sub_block_is_coded = subblock_has_nonzero_coefficient(coeff, CoeffStride, S);
      encode_coded_sub_block_flag(ectx, cabac, cIdx,
                                  coded_sub_block_neighbors[S.x+S.y*sbWidth],
                                  sub_block_is_coded);
      inferSbDcSigCoeffFlag=1;
    }
    else if (i==0 || i==lastSubBlock) {
      // first (DC) and last sub-block are always coded
      // - the first will most probably contain coefficients
      // - the last obviously contains the last coded coefficient

      sub_block_is_coded = 1;
    }

    if (sub_block_is_coded) {
      if (S.x > 0) coded_sub_block_neighbors[S.x-1 + S.y  *sbWidth] |= 1;
      if (S.y > 0) coded_sub_block_neighbors[S.x + (S.y-1)*sbWidth] |= 2;
    }

    logtrace(LogSlice,"subblock is coded: %s\n", sub_block_is_coded ? "yes":"no");


    // --- write significant coefficient flags ---

    int16_t  coeff_value[16];
    int16_t  coeff_baseLevel[16];
    int8_t   coeff_scan_pos[16];
    int8_t   coeff_sign[16];
    int8_t   coeff_has_max_base_level[16];
    int nCoefficients=0;


    if (sub_block_is_coded) {
      int x0 = S.x<<2;
      int y0 = S.y<<2;

      int log2w = log2TrafoSize-2;
      int prevCsbf = coded_sub_block_neighbors[S.x+S.y*sbWidth];
      uint8_t* ctxIdxMap = ctxIdxLookup[log2w][!!cIdx][!!scanIdx][prevCsbf];




      // set the last coded coefficient in the last subblock

      if (i==lastSubBlock) {
        coeff_value[nCoefficients] = coeff[lastSignificantX+(lastSignificantY<<log2TrafoSize)];
        coeff_has_max_base_level[nCoefficients] = 1;  // TODO
        coeff_scan_pos[nCoefficients] = lastScanPos;
        nCoefficients++;
      }


      // --- encode all coefficients' significant_coeff flags except for the DC coefficient ---

      int last_coeff =  (i==lastSubBlock) ? lastScanPos-1 : 15;

      for (int n= last_coeff ; n>0 ; n--) {
        int subX = ScanOrderPos[n].x;
        int subY = ScanOrderPos[n].y;
        int xC = x0 + subX;
        int yC = y0 + subY;


        // for all AC coefficients in sub-block, a significant_coeff flag is coded

        int isSignificant = !!tb->coeff[cIdx][xC + (yC<<log2TrafoSize)];

        logtrace(LogSlice,"coeff %d is significant: %d\n", n, isSignificant);

        logtrace(LogSlice,"context idx: %d;%d\n",xC,yC);

        encode_significant_coeff_flag_lookup(ectx, cabac,
                                             ctxIdxMap[xC+(yC<<log2TrafoSize)],
                                             isSignificant);
        //ctxIdxMap[(i<<4)+n]);

        if (isSignificant) {
          coeff_value[nCoefficients] = coeff[xC+(yC<<log2TrafoSize)];
          coeff_has_max_base_level[nCoefficients] = 1;
          coeff_scan_pos[nCoefficients] = n;
          nCoefficients++;

          // since we have a coefficient in the sub-block,
          // we cannot infer the DC coefficient anymore
          inferSbDcSigCoeffFlag = 0;
        }
      }


      // --- decode DC coefficient significance ---

      if (last_coeff>=0) // last coded coefficient (always set to 1) is not the DC coefficient
        {
          if (inferSbDcSigCoeffFlag==0) {
            // if we cannot infert the DC coefficient, it is coded
            int isSignificant = !!tb->coeff[cIdx][x0 + (y0<<log2TrafoSize)];

            logtrace(LogSlice,"DC coeff is significant: %d\n", isSignificant);

            encode_significant_coeff_flag_lookup(ectx, cabac,
                                                 ctxIdxMap[x0+(y0<<log2TrafoSize)],
                                                 isSignificant);

            if (isSignificant) {
              coeff_value[nCoefficients] = coeff[x0+(y0<<log2TrafoSize)];
              coeff_has_max_base_level[nCoefficients] = 1;
              coeff_scan_pos[nCoefficients] = 0;
              nCoefficients++;
            }
          }
          else {
            // we can infer that the DC coefficient must be present
            coeff_value[nCoefficients] = coeff[x0+(y0<<log2TrafoSize)];
            coeff_has_max_base_level[nCoefficients] = 1;
            coeff_scan_pos[nCoefficients] = 0;
            nCoefficients++;
          }
        }
    }



    // --- encode coefficient values ---

    if (nCoefficients) {

      // separate absolute coefficient value and sign

      logtrace(LogSlice,"coefficients to code: ");

      for (int l=0;l<nCoefficients;l++) {
        logtrace(LogSlice,"%d ",coeff_value[l]);

        if (coeff_value[l]<0) {
          coeff_value[l] = -coeff_value[l];
          coeff_sign[l] = 1;
        }
        else {
          coeff_sign[l] = 0;
        }

        coeff_baseLevel[l] = 1;

        logtrace(LogSlice,"(%d) ",coeff_scan_pos[l]);
      }

      logtrace(LogSlice,"\n");


      int ctxSet;
      if (i==0 || cIdx>0) { ctxSet=0; }
      else { ctxSet=2; }

      if (c1==0) { ctxSet++; }
      c1=1;


      // --- encode greater-1 flags ---

      int newLastGreater1ScanPos=-1;

      int lastGreater1Coefficient = libde265_min(8,nCoefficients);
      for (int c=0;c<lastGreater1Coefficient;c++) {
        int greater1_flag = (coeff_value[c]>1);

        encode_coeff_abs_level_greater1(ectx, cabac, cIdx,i,
                                        c==0,
                                        firstSubblock,
                                        lastSubblock_greater1Ctx,
                                        &lastInvocation_greater1Ctx,
                                        &lastInvocation_coeff_abs_level_greater1_flag,
                                        &lastInvocation_ctxSet, ctxSet,
                                        greater1_flag);

        if (greater1_flag) {
          coeff_baseLevel[c]++;

          c1=0;

          if (newLastGreater1ScanPos == -1) {
            newLastGreater1ScanPos=c;
          }
        }
        else {
          coeff_has_max_base_level[c] = 0;

          if (c1<3 && c1>0) {
            c1++;
          }
        }
      }

      firstSubblock = false;
      lastSubblock_greater1Ctx = lastInvocation_greater1Ctx;


      // --- decode greater-2 flag ---

      if (newLastGreater1ScanPos != -1) {
        int greater2_flag = (coeff_value[newLastGreater1ScanPos]>2);
        encode_coeff_abs_level_greater2(ectx,cabac, cIdx, lastInvocation_ctxSet, greater2_flag);
        coeff_baseLevel[newLastGreater1ScanPos] += greater2_flag;
        coeff_has_max_base_level[newLastGreater1ScanPos] = greater2_flag;
      }


      // --- encode coefficient signs ---

      int signHidden = (coeff_scan_pos[0]-coeff_scan_pos[nCoefficients-1] > 3 &&
                        !cb->cu_transquant_bypass_flag);

      for (int n=0;n<nCoefficients-1;n++) {
        cabac->write_CABAC_bypass(coeff_sign[n]);
        //logtrace(LogSlice,"a) sign[%d] = %d\n", n, coeff_sign[n]);
      }

      // n==nCoefficients-1
      if (!pps.sign_data_hiding_flag || !signHidden) {
        cabac->write_CABAC_bypass(coeff_sign[nCoefficients-1]);
        //logtrace(LogSlice,"b) sign[%d] = %d\n", nCoefficients-1, coeff_sign[nCoefficients-1]);
      }
      else {
        assert(coeff_sign[nCoefficients-1] == 0);
      }

      // --- decode coefficient value ---

      int sumAbsLevel=0;
      int uiGoRiceParam=0;

      for (int n=0;n<nCoefficients;n++) {
        int baseLevel = coeff_baseLevel[n];

        int coeff_abs_level_remaining;

        if (coeff_has_max_base_level[n]) {
          logtrace(LogSlice,"value[%d]=%d, base level: %d\n",n,coeff_value[n],coeff_baseLevel[n]);

          coeff_abs_level_remaining = coeff_value[n] - coeff_baseLevel[n];

          encode_coeff_abs_level_remaining(ectx, cabac, uiGoRiceParam,
                                           coeff_abs_level_remaining);

          // (9-462)
          if (baseLevel + coeff_abs_level_remaining > 3*(1<<uiGoRiceParam)) {
            uiGoRiceParam++;
            if (uiGoRiceParam>4) uiGoRiceParam=4;
          }
        }
        else {
          coeff_abs_level_remaining = 0;
        }


        // --- DEBUG: check coefficient ---

#if 0
        int16_t currCoeff = baseLevel + coeff_abs_level_remaining;
        if (coeff_sign[n]) {
          currCoeff = -currCoeff;
        }

        if (pps.sign_data_hiding_flag && signHidden) {
          sumAbsLevel += baseLevel + coeff_abs_level_remaining;

          if (n==nCoefficients-1 && (sumAbsLevel & 1)) {
            currCoeff = -currCoeff;
          }
        }

        assert(currCoeff == coeff_value[n]);
#endif
      }  // iterate through coefficients in sub-block
    }  // if nonZero

  }
}


void encode_transform_unit(encoder_context* ectx,
                           CABAC_encoder* cabac,
                           const enc_tb* tb, const enc_cb* cb,
                           int x0,int y0, int xBase,int yBase,
                           int log2TrafoSize, int trafoDepth, int blkIdx)
{
  if (tb->cbf[0] || tb->cbf[1] || tb->cbf[2]) {
    if (ectx->img->pps.cu_qp_delta_enabled_flag &&
        true /*!ectx->IsCuQpDeltaCoded*/) {
      assert(0);
    }

    if (tb->cbf[0]) {
      encode_residual(ectx,cabac, tb,cb,x0,y0,log2TrafoSize,0);
    }

    // larger than 4x4
    if (log2TrafoSize>2) {
      if (tb->cbf[1]) {
        encode_residual(ectx,cabac,tb,cb,x0,y0,log2TrafoSize-1,1);
      }
      if (tb->cbf[2]) {
        encode_residual(ectx,cabac,tb,cb,x0,y0,log2TrafoSize-1,2);
      }
    }
    else if (blkIdx==3) {
      // cannot check for tb->parent->cbf[], because this may not yet be set
      if (tb->cbf[1]) {
        encode_residual(ectx,cabac,tb,cb,xBase,yBase,log2TrafoSize,1);
      }
      if (tb->cbf[2]) {
        encode_residual(ectx,cabac,tb,cb,xBase,yBase,log2TrafoSize,2);
      }
    }
  }
}


void encode_transform_tree(encoder_context* ectx,
                           CABAC_encoder* cabac,
                           const enc_tb* tb, const enc_cb* cb,
                           int x0,int y0, int xBase,int yBase,
                           int log2TrafoSize, int trafoDepth, int blkIdx,
                           int MaxTrafoDepth, int IntraSplitFlag, bool recurse)
{
  //de265_image* img = ectx->img;
  const seq_parameter_set* sps = &ectx->img->sps;

  if (log2TrafoSize <= sps->Log2MaxTrafoSize &&
      log2TrafoSize >  sps->Log2MinTrafoSize &&
      trafoDepth < MaxTrafoDepth &&
      !(IntraSplitFlag && trafoDepth==0))
    {
      int split_transform_flag = tb->split_transform_flag;
      encode_split_transform_flag(ectx, cabac, log2TrafoSize, split_transform_flag);
    }
  else
    {
      int interSplitFlag=0; // TODO

      bool split_transform_flag = (log2TrafoSize > sps->Log2MaxTrafoSize ||
                                   (IntraSplitFlag==1 && trafoDepth==0) ||
                                   interSplitFlag==1) ? 1:0;

      /*
      printf("split_transform_flag log2TrafoSize:%d Log2MaxTrafoSize:%d "
             "IntraSplitFlag:%d trafoDepth:%d -> %d\n",
             log2TrafoSize,sps->Log2MaxTrafoSize,
             IntraSplitFlag, trafoDepth,
             split_transform_flag);
      */

      assert(tb->split_transform_flag == split_transform_flag);
    }

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
    if (recurse) {
      int x1 = x0 + (1<<(log2TrafoSize-1));
      int y1 = y0 + (1<<(log2TrafoSize-1));

      encode_transform_tree(ectx, cabac, tb->children[0], cb, x0,y0,x0,y0,log2TrafoSize-1,
                            trafoDepth+1, 0, MaxTrafoDepth, IntraSplitFlag, true);
      encode_transform_tree(ectx, cabac, tb->children[1], cb, x1,y0,x0,y0,log2TrafoSize-1,
                            trafoDepth+1, 1, MaxTrafoDepth, IntraSplitFlag, true);
      encode_transform_tree(ectx, cabac, tb->children[2], cb, x0,y1,x0,y0,log2TrafoSize-1,
                            trafoDepth+1, 2, MaxTrafoDepth, IntraSplitFlag, true);
      encode_transform_tree(ectx, cabac, tb->children[3], cb, x1,y1,x0,y0,log2TrafoSize-1,
                            trafoDepth+1, 3, MaxTrafoDepth, IntraSplitFlag, true);
    }
  }
  else {
    if (cb->PredMode == MODE_INTRA || trafoDepth != 0 ||
        tb->cbf[1] || tb->cbf[2]) {
      encode_cbf_luma(cabac, trafoDepth==0, tb->cbf[0]);
    }
    else {
    /* Note: usually, cbf[0] should be TRUE, but while estimating the bitrate, this
       function can also be called with all CBFs FALSE. Usually, this is handled by
       the rqt_root_cbf flag, but during analysis, this is set after the bitrate is estimated.
     */
      // assert(tb->cbf[0]==true);
    }

    encode_transform_unit(ectx,cabac, tb,cb, x0,y0, xBase,yBase, log2TrafoSize, trafoDepth, blkIdx);
  }
}


void encode_cu_skip_flag(encoder_context* ectx,
                         CABAC_encoder* cabac,
                         const enc_cb* cb,
                         bool skip)
{
  logtrace(LogSymbols,"$1 cu_skip_flag=%d\n",skip);

  const de265_image* img = ectx->img;

  int x0 = cb->x;
  int y0 = cb->y;

  // check if neighbors are available

  int availableL = check_CTB_available(img, x0,y0, x0-1,y0);
  int availableA = check_CTB_available(img, x0,y0, x0,y0-1);

  int condL = 0;
  int condA = 0;

  if (availableL && img->get_cu_skip_flag(x0-1,y0)) condL=1;
  if (availableA && img->get_cu_skip_flag(x0,y0-1)) condA=1;

  int contextOffset = condL + condA;
  int context = contextOffset;

  // decode bit

  int bit = skip;

  logtrace(LogSlice,"> cu_skip_flag ctx=%d, bit=%d\n", context,bit);

  cabac->write_CABAC_bit(CONTEXT_MODEL_CU_SKIP_FLAG + context, bit);
}


void encode_merge_idx(encoder_context* ectx,
                      CABAC_encoder* cabac,
                      int mergeIdx)
{
  logtrace(LogSymbols,"$1 merge_idx=%d\n",mergeIdx);
  logtrace(LogSlice,"# merge_idx %d\n", mergeIdx);

  if (ectx->shdr->MaxNumMergeCand <= 1) {
    return; // code nothing, we use only a single merge candidate
  }

  // TU coding, first bin is CABAC, remaining are bypass.
  // cMax = MaxNumMergeCand-1

  cabac->write_CABAC_bit(CONTEXT_MODEL_MERGE_IDX, mergeIdx ? 1 : 0);

  if (mergeIdx>0) {
    int idx=1;

    while (idx<ectx->shdr->MaxNumMergeCand-1) {
      int increase = (idx < mergeIdx);

      cabac->write_CABAC_bypass(increase);
      if (increase) {
        idx++;
      }
      else {
        break;
      }
    }
  }
}


static inline void encode_rqt_root_cbf(encoder_context* ectx,
                                       CABAC_encoder* cabac,
                                       int rqt_root_cbf)
{
  logtrace(LogSymbols,"$1 rqt_root_cbf=%d\n",rqt_root_cbf);
  cabac->write_CABAC_bit(CONTEXT_MODEL_RQT_ROOT_CBF, rqt_root_cbf);
}


void encode_mvd(encoder_context* ectx,
                CABAC_encoder* cabac,
                const int16_t mvd[2])
{
  int mvd0abs = abs_value(mvd[0]);
  int mvd1abs = abs_value(mvd[1]);

  int mvd0_greater_0 = !!(mvd0abs);
  int mvd1_greater_0 = !!(mvd1abs);

  cabac->write_CABAC_bit(CONTEXT_MODEL_ABS_MVD_GREATER01_FLAG+0, mvd0_greater_0);
  cabac->write_CABAC_bit(CONTEXT_MODEL_ABS_MVD_GREATER01_FLAG+0, mvd1_greater_0);

  if (mvd0_greater_0) {
    cabac->write_CABAC_bit(CONTEXT_MODEL_ABS_MVD_GREATER01_FLAG+1, mvd0abs>1);
  }
  if (mvd1_greater_0) {
    cabac->write_CABAC_bit(CONTEXT_MODEL_ABS_MVD_GREATER01_FLAG+1, mvd1abs>1);
  }

  if (mvd0abs) {
    if (mvd0abs>1) {
      cabac->write_CABAC_EGk(mvd0abs-2,1);
    }
    cabac->write_CABAC_bypass(mvd[0]<0);
  }

  if (mvd1abs) {
    if (mvd1abs>1) {
      cabac->write_CABAC_EGk(mvd1abs-2,1);
    }
    cabac->write_CABAC_bypass(mvd[1]<0);
  }
}


void encode_prediction_unit(encoder_context* ectx,
                            CABAC_encoder* cabac,
                            const enc_cb* cb, int pbIdx,
                            int x0,int y0, int w, int h)
{
  const enc_pb_inter& pb = cb->inter.pb[pbIdx];

  logtrace(LogSymbols,"$1 merge_flag=%d\n",pb.spec.merge_flag);
  cabac->write_CABAC_bit(CONTEXT_MODEL_MERGE_FLAG, pb.spec.merge_flag);

  if (pb.spec.merge_flag) {
    assert(false); // TODO
  }
  else {
    if (ectx->shdr->slice_type == SLICE_TYPE_B) {
      assert(false); // TODO
    }

    if (pb.spec.inter_pred_idc != PRED_L1) {
      if (ectx->shdr->num_ref_idx_l0_active > 1) {
        assert(false); // TODO
      //cabac->write_CABAC_bit(CONTEXT_MODEL_REF_IDX_LX, pb.spec.mvp_l0_flag);
      }

      encode_mvd(ectx,cabac, pb.spec.mvd[0]);

      logtrace(LogSymbols,"$1 mvp_lx_flag=%d\n",pb.spec.mvp_l0_flag);
      cabac->write_CABAC_bit(CONTEXT_MODEL_MVP_LX_FLAG, pb.spec.mvp_l0_flag);
    }

    if (pb.spec.inter_pred_idc != PRED_L0) {
      assert(false); // TODO
    }

    /*
enum InterPredIdc
    PRED_L0=0,
    PRED_L1=1,
    PRED_BI=2
    */
  }
}


void encode_coding_unit(encoder_context* ectx,
                        CABAC_encoder* cabac,
                        const enc_cb* cb, int x0,int y0, int log2CbSize, bool recurse)
{
  logtrace(LogSlice,"--- encode CU (%d;%d) ---\n",x0,y0);

  de265_image* img = ectx->img;
  const slice_segment_header* shdr = &ectx->imgdata->shdr;
  const seq_parameter_set* sps = &ectx->img->sps;


  int nCbS = 1<<log2CbSize;


  // write skip_flag

  if (shdr->slice_type != SLICE_TYPE_I) {
    encode_cu_skip_flag(ectx,cabac, cb, cb->PredMode==MODE_SKIP);
  }

  if (cb->PredMode==MODE_SKIP) {
    assert(cb->inter.pb[0].spec.merge_flag);
    encode_merge_idx(ectx,cabac, cb->inter.pb[0].spec.merge_idx);
  }
  else {

    enum PredMode PredMode = cb->PredMode;
    enum PartMode PartMode = PART_2Nx2N;
    int IntraSplitFlag=0;

    if (shdr->slice_type != SLICE_TYPE_I) {
      encode_pred_mode_flag(ectx,cabac, PredMode);
    }

    if (PredMode != MODE_INTRA ||
        log2CbSize == sps->Log2MinCbSizeY) {
      PartMode = cb->PartMode;
      encode_part_mode(ectx,cabac, PredMode, PartMode, log2CbSize);
    }

    if (PredMode == MODE_INTRA) {

      int availableA0 = check_CTB_available(img, x0,y0, x0-1,y0);
      int availableB0 = check_CTB_available(img, x0,y0, x0,y0-1);

      if (PartMode==PART_2Nx2N) {
        logtrace(LogSlice,"x0,y0: %d,%d\n",x0,y0);
        int PUidx = (x0>>sps->Log2MinPUSize) + (y0>>sps->Log2MinPUSize)*sps->PicWidthInMinPUs;

        int candModeList[3];
        fillIntraPredModeCandidates(candModeList,x0,y0,PUidx,
                                    availableA0,availableB0, img);

        for (int i=0;i<3;i++)
          logtrace(LogSlice,"candModeList[%d] = %d\n", i, candModeList[i]);

        enum IntraPredMode mode = cb->intra.pred_mode[0];
        int intraPred = find_intra_pred_mode(mode, candModeList);
        encode_prev_intra_luma_pred_flag(ectx,cabac, intraPred);
        encode_intra_mpm_or_rem(ectx,cabac, intraPred);

        logtrace(LogSlice,"IntraPredMode: %d (candidates: %d %d %d)\n", mode,
                 candModeList[0], candModeList[1], candModeList[2]);
        logtrace(LogSlice,"  MPM/REM = %d\n",intraPred);
      }
      else {
        IntraSplitFlag=1;

        int pbOffset = nCbS/2;
        int PUidx;

        int intraPred[4];
        int childIdx=0;

        for (int j=0;j<nCbS;j+=pbOffset)
          for (int i=0;i<nCbS;i+=pbOffset, childIdx++)
            {
              int x=x0+i, y=y0+j;

              int availableA = availableA0 || (i>0); // left candidate always available for right blk
              int availableB = availableB0 || (j>0); // top candidate always available for bottom blk

              PUidx = (x>>sps->Log2MinPUSize) + (y>>sps->Log2MinPUSize)*sps->PicWidthInMinPUs;

              int candModeList[3];
              fillIntraPredModeCandidates(candModeList,x,y,PUidx,
                                          availableA,availableB, img);

              enum IntraPredMode mode = cb->intra.pred_mode[childIdx];

              assert(ectx->img->get_IntraPredMode(x,y) == mode);

              intraPred[childIdx] = find_intra_pred_mode(mode, candModeList);
            }

        for (int i=0;i<4;i++)
          encode_prev_intra_luma_pred_flag(ectx,cabac, intraPred[i]);

        for (int i=0;i<4;i++)
          encode_intra_mpm_or_rem(ectx,cabac, intraPred[i]);
      }

      IntraChromaPredMode chromaPredMode = find_chroma_pred_mode(cb->intra.chroma_mode,
                                                                 cb->intra.pred_mode[0]);
      encode_intra_chroma_pred_mode(ectx,cabac, chromaPredMode);
    }
    else {
      switch (cb->PartMode) {
      case PART_2Nx2N:
        encode_prediction_unit(ectx,cabac,cb, 0, cb->x,cb->y,1<<cb->log2Size,1<<cb->log2Size);
        break;
      case PART_2NxN:
      case PART_Nx2N:
      case PART_NxN:
      case PART_2NxnU:
      case PART_2NxnD:
      case PART_nLx2N:
      case PART_nRx2N:
        assert(false); // TODO
      }
    }


    if (true) { // !pcm

      if (cb->PredMode != MODE_INTRA &&
          !(cb->PartMode == PART_2Nx2N && cb->inter.pb[0].spec.merge_flag)) {

        //printf("%d %d %d\n",cb->PredMode,cb->PartMode,cb->inter.pb[0].merge_flag);

        encode_rqt_root_cbf(ectx,cabac, cb->inter.rqt_root_cbf);
      }

      //printf("%d;%d encode rqt_root_cbf=%d\n",x0,y0,cb->inter.rqt_root_cbf);

      if (cb->PredMode == MODE_INTRA || cb->inter.rqt_root_cbf) {
        int MaxTrafoDepth;
        if (PredMode == MODE_INTRA)
          { MaxTrafoDepth = sps->max_transform_hierarchy_depth_intra + IntraSplitFlag; }
        else
          { MaxTrafoDepth = sps->max_transform_hierarchy_depth_inter; }


        if (recurse) {
          //printf("%d;%d store transform tree\n",x0,y0);

          encode_transform_tree(ectx,cabac, cb->transform_tree, cb,
                                x0,y0, x0,y0, log2CbSize, 0, 0, MaxTrafoDepth, IntraSplitFlag, true);
        }
      }
    }
  }
}


SplitType get_split_type(const seq_parameter_set* sps,
                         int x0,int y0, int log2CbSize)
{
  /*
    CU split flag:

         | overlaps | minimum ||
    case | border   | size    ||  split
    -----+----------+---------++----------
      A  |    0     |     0   || optional
      B  |    0     |     1   ||    0
      C  |    1     |     0   ||    1
      D  |    1     |     1   ||    0
  */
  if (x0+(1<<log2CbSize) <= sps->pic_width_in_luma_samples &&
      y0+(1<<log2CbSize) <= sps->pic_height_in_luma_samples &&
      log2CbSize > sps->Log2MinCbSizeY) {

    // case A

    return OptionalSplit;
  } else {
    // case B/C/D

    if (log2CbSize > sps->Log2MinCbSizeY) { return ForcedSplit;    }
    else                                  { return ForcedNonSplit; }
  }
}


void encode_quadtree(encoder_context* ectx,
                     CABAC_encoder* cabac,
                     const enc_cb* cb, int x0,int y0, int log2CbSize, int ctDepth,
                     bool recurse)
{
  //de265_image* img = ectx->img;
  const seq_parameter_set* sps = &ectx->img->sps;

  int split_flag = get_split_type(sps,x0,y0,log2CbSize);

  // if it is an optional split, take the decision from the CU flag
  if (split_flag == OptionalSplit) {
    split_flag = cb->split_cu_flag;

    encode_split_cu_flag(ectx,cabac, x0,y0, ctDepth, split_flag);
  }


  if (split_flag) {
    if (recurse) {
      int x1 = x0 + (1<<(log2CbSize-1));
      int y1 = y0 + (1<<(log2CbSize-1));

      encode_quadtree(ectx,cabac, cb->children[0], x0,y0, log2CbSize-1, ctDepth+1, true);

      if (x1<sps->pic_width_in_luma_samples)
        encode_quadtree(ectx,cabac, cb->children[1], x1,y0, log2CbSize-1, ctDepth+1, true);

      if (y1<sps->pic_height_in_luma_samples)
        encode_quadtree(ectx,cabac, cb->children[2], x0,y1, log2CbSize-1, ctDepth+1, true);

      if (x1<sps->pic_width_in_luma_samples &&
          y1<sps->pic_height_in_luma_samples)
        encode_quadtree(ectx,cabac, cb->children[3], x1,y1, log2CbSize-1, ctDepth+1, true);
    }
  }
  else {
    encode_coding_unit(ectx,cabac, cb,x0,y0, log2CbSize, true);
  }
}


void encode_ctb(encoder_context* ectx,
                CABAC_encoder* cabac,
                enc_cb* cb, int ctbX,int ctbY)
{
  logtrace(LogSlice,"----- encode CTB (%d;%d) -----\n",ctbX,ctbY);

#if 0
  printf("MODEL:\n");
  for (int i=0;i<CONTEXT_MODEL_TABLE_LENGTH;i++)
    {
      printf("%d;%d ",
             ectx->ctx_model[i].state,
             ectx->ctx_model[i].MPSbit);

      if ((i%16)==15) printf("\n");
    }
  printf("\n");
#endif

  de265_image* img = ectx->img;
  int log2ctbSize = img->sps.Log2CtbSizeY;

  encode_quadtree(ectx,cabac, cb, ctbX<<log2ctbSize, ctbY<<log2ctbSize, log2ctbSize, 0, true);
}


// ---------------------------------------------------------------------------
