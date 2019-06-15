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
#include "libde265/fallback-dct.h"
#include "libde265/image-io.h"

#include "acceleration-speed.h"


/* TODO: for more realistic input to IDCTs, we could save the real coefficients in
   a decoder run and use this data as input.
 */


bool show_help=false;
bool do_check=false;
bool do_time=false;
bool do_eval=false;
int  img_width=352;
int  img_height=288;
int  nframes=1000;
int  repeat=10;
std::string function;
std::string input_file;

static struct option long_options[] = {
  {"help",    no_argument,       0, 'H' },
  {"input",   required_argument, 0, 'i' },
  {"width",   required_argument, 0, 'w' },
  {"height",  required_argument, 0, 'h' },
  {"nframes", required_argument, 0, 'n' },
  {"function",required_argument, 0, 'f' },
  {"check",   no_argument,       0, 'c' },
  {"time",    no_argument,       0, 't' },
  {"eval",    no_argument,       0, 'e' },
  {"repeat",  required_argument, 0, 'r' },
  {0,            0,              0,  0  }
};



DSPFunc* DSPFunc::first = NULL;


bool DSPFunc::runOnImage(std::shared_ptr<const de265_image> img, bool compareToReference)
{
  int w = img->get_width(0);
  int h = img->get_height(0);

  int blkWidth  = getBlkWidth();
  int blkHeight = getBlkHeight();

  bool success = true;

  for (int y=0;y<=h-blkHeight;y+=blkHeight)
    for (int x=0;x<=w-blkWidth;x+=blkWidth) {
      runOnBlock(x,y);

      if (compareToReference) {
        referenceImplementation()->runOnBlock(x,y);
        success &= compareToReferenceImplementation();
      }
    }

  return success;
}



int main(int argc, char** argv)
{
  while (1) {
    int option_index = 0;

    int c = getopt_long(argc, argv, "Hci:w:h:n:f:ter:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'H': show_help=true; break;
    case 'c': do_check=true; break;
    case 'w': img_width =atoi(optarg); break;
    case 'h': img_height=atoi(optarg); break;
    case 'n': nframes=atoi(optarg); break;
    case 'f': function=optarg; break;
    case 'i': input_file=optarg; break;
    case 't': do_time=true; break;
    case 'e': do_eval=true; break;
    case 'r': repeat=atoi(optarg); break;
    }
  }


  if (show_help) {
    fprintf(stderr,
            "acceleration-speed  SIMD DSP function testing tool\n"
            "--------------------------------------------------\n"
            "      --help           show help\n"
            "  -i, --input NAME     input YUV file\n"
            "  -w, --width #        input width (default: 352)\n"
            "  -h, --height #       input height (default: 288)\n"
            "  -n, --nframes #      number of frames to process (default: 1000)\n"
            "  -f, --function NAME  which function to test (see below)\n"
            "  -r, --repeat #       number of repetitions for each image (default: 10)\n"
            "  -c, --check          compare function result against its reference code\n"
            "\n"
            "these functions are known:\n"
            );

    std::stack<const char*> funcnames;

    for (DSPFunc* func = DSPFunc::first;
         func ;
         func=func->next) {
      funcnames.push(func->name());
    }

    while (!funcnames.empty()) {
      fprintf(stderr,
              "  %s\n", funcnames.top());
      funcnames.pop();
    }

    return 0;
  }


  // --- find DSP function with the given name ---

  if (function.empty()) {
    fprintf(stderr,"No function specified. Use option '--function'.\n");
    exit(10);
  }

  DSPFunc* algo = NULL;
  for (DSPFunc* f = DSPFunc::first; f ; f=f->next) {
    if (strcasecmp(f->name(), function.c_str())==0) {
      algo = f;
      break;
    }
  }

  if (algo==NULL) {
    fprintf(stderr,"Argument to '--function' invalid. No function with that name.\n");
    exit(10);
  }

  if (do_check && !algo->referenceImplementation()) {
    fprintf(stderr,"cannot check function result: no reference function defined for the selected function.\n");
    exit(10);
  }


  ImageSource_YUV image_source;
  image_source.set_input_file(input_file.c_str(), img_width, img_height);

  int img_counter=0;

  bool eof = false;
  for (int f=0; f<nframes ; f++)
    {
      std::shared_ptr<de265_image> image(image_source.get_image());
      if (!image) {
        eof=true;
        break;
      }

      img_counter++;

      if (algo->referenceImplementation()) {
        algo->referenceImplementation()->prepareNextImage(image);
      }

      if (algo->prepareNextImage(image)) {
        printf("run %d times on image %d\n",repeat,img_counter);

        for (int r=0;r<repeat;r++) {
          bool success = algo->runOnImage(image, do_check);
          if (!success) {
            fprintf(stderr, "computation mismatch to reference implementation...\n");
            exit(10);
          }
        }
      }
    }

  return 0;
}
