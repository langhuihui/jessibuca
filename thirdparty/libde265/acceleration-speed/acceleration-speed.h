/*
 * H.265 video codec.
 * Copyright (c) 2015 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libde265.
 *
 * libde265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libde265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libde265.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACCELERATION_SPEED_H
#define ACCELERATION_SPEED_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <string>
#include <stack>
#include <memory>

#include "libde265/image.h"
#include "libde265/image-io.h"


class DSPFunc
{
public:
  DSPFunc() { next = first; first = this; }
  virtual ~DSPFunc() { }

  virtual const char* name() const = 0;

  virtual int getBlkWidth() const = 0;
  virtual int getBlkHeight() const = 0;

  virtual void runOnBlock(int x,int y) = 0;
  virtual DSPFunc* referenceImplementation() const { return NULL; }

  virtual bool prepareNextImage(std::shared_ptr<const de265_image>) = 0;

  bool runOnImage(std::shared_ptr<const de265_image> img, bool compareToReference);
  virtual bool compareToReferenceImplementation() { return false; }

  static DSPFunc* first;
  DSPFunc* next;
};


#endif
