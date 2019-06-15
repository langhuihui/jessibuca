/*
 * libde265 example application "sherlock265".
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of sherlock265, an example application using libde265.
 *
 * sherlock265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * sherlock265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sherlock265.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VIDEODECODER_HH
#define VIDEODECODER_HH

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtGui>
#ifdef HAVE_SWSCALE
#ifdef __cplusplus
extern "C" {
#endif
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif
#endif

#include "VideoWidget.hh"
#include "de265.h"


class VideoDecoder : public QThread
{
  Q_OBJECT

public:
  VideoDecoder();
  ~VideoDecoder();

  void init(const char* filename);

protected:
  void run();  // thread entry point

public slots:
  void startDecoder();
  void stopDecoder();
  void singleStepDecoder();

  void showCBPartitioning(bool flag);
  void showTBPartitioning(bool flag);
  void showPBPartitioning(bool flag);
  void showIntraPredMode(bool flag);
  void showPBPredMode(bool flag);
  void showQuantPY(bool flag);
  void showMotionVec(bool flag);
  void showTiles(bool flag);
  void showSlices(bool flag);
  void showDecodedImage(bool flag);

signals:
  void displayImage(QImage*);

private:
  // de265 decoder

  FILE* mFH;
  //input_context_FILE inputctx;
  //rbsp_buffer buf;
  de265_decoder_context* ctx;
  const de265_image* img;

  QMutex mutex;

  QImage mImgBuffers[2];
  int    mNextBuffer;
  int    mFrameCount;

  bool   mPlayingVideo;
  bool   mVideoEnded;
  bool   mSingleStep;


  bool   mShowDecodedImage;
  bool   mShowQuantPY;
  bool   mCBShowPartitioning;
  bool   mTBShowPartitioning;
  bool   mPBShowPartitioning;
  bool   mShowIntraPredMode;
  bool   mShowPBPredMode;
  bool   mShowMotionVec;
  bool   mShowTiles;
  bool   mShowSlices;

  void decoder_loop();

  void init_decoder(const char* filename);
  void free_decoder();

  void show_frame(const de265_image* img);
#ifdef HAVE_VIDEOGFX
  void convert_frame_libvideogfx(const de265_image* img, QImage & qimg);
#endif
#ifdef HAVE_SWSCALE
  SwsContext* sws;
  int width;
  int height;
  void convert_frame_swscale(const de265_image* img, QImage & qimg);
#endif
};

#endif
