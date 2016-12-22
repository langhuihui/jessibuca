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

#ifndef ENCODE_H
#define ENCODE_H

#include "libde265/image.h"
#include "libde265/decctx.h"
#include "libde265/image-io.h"
#include "libde265/alloc_pool.h"

class encoder_context;
class enc_cb;


class enc_node
{
 public:
  enc_node() { mReconstruction=NULL; }
  virtual ~enc_node() { delete[] mReconstruction; }

  uint16_t x,y;
  uint8_t  log2Size : 3;

  virtual void save(const de265_image*);
  virtual void restore(de265_image*);

 private:
  uint8_t* mReconstruction;
};


class enc_tb : public enc_node
{
 public:
  enc_tb();
  ~enc_tb();

  const enc_tb* parent;

  uint8_t split_transform_flag : 1;
  uint8_t TrafoDepth : 2;  // 2 bits enough ? (TODO)

  uint8_t cbf[3];

  union {
    // split
    struct {
      enc_tb* children[4];
    };

    // non-split
    struct {
      int16_t* coeff[3];
    };
  };

  float distortion;  // total distortion for this level of the TB tree (including all children)
  float rate;        // total rate for coding this TB level and all children
  float rate_withoutCbfChroma;

  void set_cbf_flags_from_children();

  void reconstruct(encoder_context* ectx,
                   de265_image* img,
                   const enc_cb* cb, int blkIdx=0) const;

  bool isZeroBlock() const { return cbf[0]==false && cbf[1]==false && cbf[2]==false; }

  void alloc_coeff_memory(int cIdx, int tbSize);

  static void* operator new(const size_t size) { return mMemPool.new_obj(size); }
  static void operator delete(void* obj) { mMemPool.delete_obj(obj); }

private:
  static alloc_pool mMemPool;

  void reconstruct_tb(encoder_context* ectx,
                      de265_image* img, int x0,int y0, int log2TbSize,
                      const enc_cb* cb, int cIdx) const;
};


struct enc_pb_inter
{
  /* absolute motion information (for MV-prediction candidates)
   */
  MotionVectorSpec motion;

  /* specification how to code the motion vector in the bitstream
   */
  motion_spec    spec;


  // NOT TRUE: refIdx in 'spec' is not used. It is taken from 'motion'
  // Currently, information is duplicated. Same as with inter_pred_idc/predFlag[].

  /* SPEC:
  int8_t  refIdx[2]; // not used
  int16_t mvd[2][2];

  uint8_t inter_pred_idc : 2; // enum InterPredIdc
  uint8_t mvp_l0_flag : 1;
  uint8_t mvp_l1_flag : 1;
  uint8_t merge_flag : 1;
  uint8_t merge_idx  : 3;
  */
};


class enc_cb : public enc_node
{
public:
  enc_cb();
  ~enc_cb();

  const enc_cb* parent;

  uint8_t split_cu_flag : 1;
  uint8_t ctDepth : 2;

  union {
    // split
    struct {
      enc_cb* children[4];   // undefined when split_cu_flag==false
    };

    // non-split
    struct {
      uint8_t qp : 6;
      uint8_t cu_transquant_bypass_flag : 1; // currently unused
      uint8_t pcm_flag : 1;

      enum PredMode PredMode; // : 6;
      enum PartMode PartMode; // : 3;

      union {
        struct {
          enum IntraPredMode pred_mode[4];
          enum IntraPredMode chroma_mode;
        } intra;

        struct {
          enc_pb_inter pb[4];

          uint8_t rqt_root_cbf : 1;
        } inter;
      };

      const enc_tb* transform_tree;
    };
  };


  float distortion;
  float rate;


  void set_rqt_root_bf_from_children_cbf();

  /* Save CB reconstruction in the node and restore it again to the image.
     Pixel data and metadata.
   */
  virtual void save(const de265_image*);
  virtual void restore(de265_image*);


  /* Decode this CB: pixel data and write metadata to image.
   */
  void reconstruct(encoder_context* ectx,de265_image* img) const;


  // memory management

  static void* operator new(const size_t size) { return mMemPool.new_obj(size); }
  static void operator delete(void* obj) { mMemPool.delete_obj(obj); }

 private:
  void write_to_image(de265_image*) const;

  static alloc_pool mMemPool;
};



inline int childX(int x0, int idx, int log2CbSize)
{
  return x0 + ((idx&1) << (log2CbSize-1));
}

inline int childY(int y0, int idx, int log2CbSize)
{
  return y0 + ((idx>>1) << (log2CbSize-1));
}



void encode_split_cu_flag(encoder_context* ectx,
                          CABAC_encoder* cabac,
                          int x0, int y0, int ctDepth, int split_flag);

void encode_transform_tree(encoder_context* ectx,
                           CABAC_encoder* cabac,
                           const enc_tb* tb, const enc_cb* cb,
                           int x0,int y0, int xBase,int yBase,
                           int log2TrafoSize, int trafoDepth, int blkIdx,
                           int MaxTrafoDepth, int IntraSplitFlag, bool recurse);

void encode_coding_unit(encoder_context* ectx,
                        CABAC_encoder* cabac,
                        const enc_cb* cb, int x0,int y0, int log2CbSize, bool recurse);

/* returns
   1  - forced split
   0  - forced non-split
   -1 - optional split
*/
enum SplitType {
  ForcedNonSplit = 0,
  ForcedSplit    = 1,
  OptionalSplit  = 2
};

SplitType get_split_type(const seq_parameter_set* sps,
                         int x0,int y0, int log2CbSize);


void encode_split_transform_flag(encoder_context* ectx,
                                 CABAC_encoder* cabac,
                                 int log2TrafoSize, int split_flag);

void encode_merge_idx(encoder_context* ectx,
                      CABAC_encoder* cabac,
                      int mergeIdx);

void encode_cu_skip_flag(encoder_context* ectx,
                         CABAC_encoder* cabac,
                         const enc_cb* cb,
                         bool skip);

void encode_cbf_luma(CABAC_encoder* cabac,
                     bool zeroTrafoDepth, int cbf_luma);

void encode_cbf_chroma(CABAC_encoder* cabac,
                       int trafoDepth, int cbf_chroma);

void encode_transform_unit(encoder_context* ectx,
                           CABAC_encoder* cabac,
                           const enc_tb* tb, const enc_cb* cb,
                           int x0,int y0, int xBase,int yBase,
                           int log2TrafoSize, int trafoDepth, int blkIdx);


void encode_quadtree(encoder_context* ectx,
                     CABAC_encoder* cabac,
                     const enc_cb* cb, int x0,int y0, int log2CbSize, int ctDepth,
                     bool recurse);

void encode_ctb(encoder_context* ectx,
                CABAC_encoder* cabac,
                enc_cb* cb, int ctbX,int ctbY);


class de265_encoder
{
 public:
  virtual ~de265_encoder() { }
};

#endif
