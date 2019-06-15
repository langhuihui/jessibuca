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

#ifndef IMAGE_IO_PNG_H
#define IMAGE_IO_PNG_H

#include "libde265/image-io.h"
#include <deque>


#if HAVE_VIDEOGFX
class ImageSource_PNG : public ImageSource
{
 public:
  LIBDE265_API ImageSource_PNG();
  virtual LIBDE265_API ~ImageSource_PNG();

  bool LIBDE265_API set_input_file(const char* filename);

  //virtual ImageStatus  get_status();
  virtual LIBDE265_API de265_image* get_image(bool block=true);
  virtual LIBDE265_API void skip_frames(int n);

  virtual LIBDE265_API int get_width() const { return mWidth; }
  virtual LIBDE265_API int get_height() const { return mHeight; }

 private:
  const char* mFilenameTemplate;
  int mNextImageNumber;

  bool mReachedEndOfStream;

  int mWidth,mHeight;
};
#endif

#endif
