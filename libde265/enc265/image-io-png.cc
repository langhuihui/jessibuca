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

#include "image-io-png.h"
#include <assert.h>

#if HAVE_VIDEOGFX
#include <libvideogfx.hh>
using namespace videogfx;


ImageSource_PNG::ImageSource_PNG()
{
  mFilenameTemplate = NULL;
  mNextImageNumber = 1;

  mReachedEndOfStream = false;

  mWidth=mHeight=0;
}

ImageSource_PNG::~ImageSource_PNG()
{
}

bool ImageSource_PNG::set_input_file(const char* filename)
{
  mFilenameTemplate = filename;
  return true;
}

de265_image* ImageSource_PNG::get_image(bool block)
{
  if (mReachedEndOfStream) return NULL;


  // --- construct image filename ---

  char filename[1000];
  sprintf(filename,mFilenameTemplate,mNextImageNumber);
  mNextImageNumber++;


  // --- load image ---

  Image<Pixel> input;
  bool success = videogfx::ReadImage_PNG(input, filename);
  if (!success) {
    mReachedEndOfStream = true;
    return NULL;
  }


  mWidth = input.AskWidth();
  mHeight= input.AskHeight();

  de265_image* img = new de265_image;
  img->alloc_image(mWidth,mHeight,de265_chroma_444, NULL, false,
                   NULL, NULL, 0, NULL, false);
  assert(img); // TODO: error handling


  uint8_t* p;
  int stride;

  for (int c=0;c<3;c++) {
    int h265channel;
    switch (c) {
    case 0: h265channel=2; break; // R
    case 1: h265channel=0; break; // G
    case 2: h265channel=1; break; // B
    }

    p = img->get_image_plane(h265channel);
    stride = img->get_image_stride(h265channel);

    for (int y=0;y<mHeight;y++) {
      memcpy(p, input.AskFrame((BitmapChannel(c)))[y], mWidth);
      p += stride;
    }
  }

  return img;
}

void ImageSource_PNG::skip_frames(int n)
{
  mNextImageNumber += n;
}

#endif
