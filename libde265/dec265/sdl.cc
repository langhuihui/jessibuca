/*
 * libde265 example application "dec265".
 * Copyright (c) 2013-2014 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of dec265, an example application using libde265.
 *
 * dec265 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dec265 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dec265.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sdl.hh"
#include <assert.h>


bool SDL_YUV_Display::init(int frame_width, int frame_height, enum SDL_Chroma chroma)
{
  // reduce image size to a multiple of 8 (apparently required by YUV overlay)

  frame_width  &= ~7;
  frame_height &= ~7;

  mChroma = chroma;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0 ) {
    printf("SDL_Init() failed: %s\n", SDL_GetError( ) );
    SDL_Quit();
    return false;
  }

  const SDL_VideoInfo* info = SDL_GetVideoInfo();
  if( !info ) {
    printf("SDL_GetVideoInfo() failed: %s\n", SDL_GetError() );
    SDL_Quit();
    return false;
  }

  Uint8 bpp = info->vfmt->BitsPerPixel;

  Uint32 vflags;
  if (info->hw_available)
    vflags = SDL_HWSURFACE;
  else
    vflags = SDL_SWSURFACE;

  // set window title
  const char *window_title = "SDL YUV display";
  SDL_WM_SetCaption(window_title, NULL);

  mScreen = SDL_SetVideoMode(frame_width, frame_height, bpp, vflags);
  if (mScreen == NULL) {
    printf("SDL: Couldn't set video mode to %dx%d,%d bpp: %s",
           frame_width, frame_height, bpp, SDL_GetError());
    SDL_Quit();
    return false;
  }

  uint32_t pixelFormat;
  switch (mChroma) {
  case SDL_CHROMA_MONO: pixelFormat = SDL_YV12_OVERLAY; break;
  case SDL_CHROMA_420:  pixelFormat = SDL_YV12_OVERLAY; break;
  case SDL_CHROMA_422:  pixelFormat = SDL_YUY2_OVERLAY; break;
    case SDL_CHROMA_444:  pixelFormat = SDL_YV12_OVERLAY; break;
      //case SDL_CHROMA_444:  pixelFormat = SDL_YUY2_OVERLAY; break;
  }

  mYUVOverlay = SDL_CreateYUVOverlay(frame_width, frame_height, pixelFormat, mScreen);
  if (mYUVOverlay == NULL ) {
    printf("SDL: Couldn't create SDL YUV overlay: %s",SDL_GetError());
    SDL_Quit();
    return false;
  }

  rect.x = 0;
  rect.y = 0;
  rect.w = frame_width;
  rect.h = frame_height;

  mWindowOpen=true;

  return true;
}

void SDL_YUV_Display::display(const unsigned char *Y,
                              const unsigned char *U,
                              const unsigned char *V,
                              int stride, int chroma_stride)
{
  if (!mWindowOpen) return;
  if (SDL_LockYUVOverlay(mYUVOverlay) < 0) return;

  if (mChroma == SDL_CHROMA_420) {
    display420(Y,U,V,stride,chroma_stride);
  }
  else if (mChroma == SDL_CHROMA_422) {
    display422(Y,U,V,stride,chroma_stride);
  }
  else if (mChroma == SDL_CHROMA_444) {
    display444as420(Y,U,V,stride,chroma_stride);
    //display444as422(Y,U,V,stride,chroma_stride);
  }
  else if (mChroma == SDL_CHROMA_MONO) {
    display400(Y,stride);
  }

  SDL_UnlockYUVOverlay(mYUVOverlay);

  SDL_DisplayYUVOverlay(mYUVOverlay, &rect);
}


void SDL_YUV_Display::display420(const unsigned char *Y,
                                 const unsigned char *U,
                                 const unsigned char *V,
                                 int stride, int chroma_stride)
{
  if (stride == rect.w && chroma_stride == rect.w/2) {

    // fast copy

    memcpy(mYUVOverlay->pixels[0], Y, rect.w * rect.h);
    memcpy(mYUVOverlay->pixels[1], V, rect.w * rect.h / 4);
    memcpy(mYUVOverlay->pixels[2], U, rect.w * rect.h / 4);
  }
  else {
    // copy line by line, because sizes are different

    for (int y=0;y<rect.h;y++)
      {
        memcpy(mYUVOverlay->pixels[0]+y*rect.w, Y+stride*y, rect.w);
      }

    for (int y=0;y<rect.h/2;y++)
      {
        memcpy(mYUVOverlay->pixels[2]+y*rect.w/2, U+chroma_stride*y, rect.w/2);
        memcpy(mYUVOverlay->pixels[1]+y*rect.w/2, V+chroma_stride*y, rect.w/2);
      }
  }
}


void SDL_YUV_Display::display400(const unsigned char *Y, int stride)
{
  if (stride == rect.w) {

    // fast copy

    memcpy(mYUVOverlay->pixels[0], Y, rect.w * rect.h);
  }
  else {
    // copy line by line, because sizes are different

    for (int y=0;y<rect.h;y++)
      {
        memcpy(mYUVOverlay->pixels[0]+y*rect.w, Y+stride*y, rect.w);
      }
  }

  // clear chroma planes

  memset(mYUVOverlay->pixels[1], 0x80, rect.w * rect.h / 4);
  memset(mYUVOverlay->pixels[2], 0x80, rect.w * rect.h / 4);
}


void SDL_YUV_Display::display422(const unsigned char *Y,
                                 const unsigned char *U,
                                 const unsigned char *V,
                                 int stride, int chroma_stride)
{
  for (int y=0;y<rect.h;y++)
    {
      unsigned char* p = mYUVOverlay->pixels[0] + y*rect.w *2;

      const unsigned char* Yp = Y + y*stride;
      const unsigned char* Up = U + y*chroma_stride;
      const unsigned char* Vp = V + y*chroma_stride;

      for (int x=0;x<rect.w;x+=2) {
        *p++ = Yp[x];
        *p++ = Up[x/2];
        *p++ = Yp[x+1];
        *p++ = Vp[x/2];
      }
    }
}


/* This converts down 4:4:4 input to 4:2:2 for display, as SDL does not support
   any 4:4:4 pixel format.
 */
void SDL_YUV_Display::display444as422(const unsigned char *Y,
                                      const unsigned char *U,
                                      const unsigned char *V,
                                      int stride, int chroma_stride)
{
  for (int y=0;y<rect.h;y++)
    {
      unsigned char* p = mYUVOverlay->pixels[0] + y*rect.w *2;

      const unsigned char* Yp = Y + y*stride;
      const unsigned char* Up = U + y*chroma_stride;
      const unsigned char* Vp = V + y*chroma_stride;

      for (int x=0;x<rect.w;x+=2) {
        *p++ = Yp[x];
        *p++ = Up[x];
        *p++ = Yp[x+1];
        *p++ = Vp[x];
      }
    }
}


void SDL_YUV_Display::display444as420(const unsigned char *Y,
                                      const unsigned char *U,
                                      const unsigned char *V,
                                      int stride, int chroma_stride)
{
  for (int y=0;y<rect.h;y++)
    {
      unsigned char* p = mYUVOverlay->pixels[0] + y*rect.w;
      memcpy(p, Y+y*stride, rect.w);
    }

  for (int y=0;y<rect.h;y+=2)
    {
      unsigned char* u = mYUVOverlay->pixels[2] + y/2*rect.w/2;
      unsigned char* v = mYUVOverlay->pixels[1] + y/2*rect.w/2;

      for (int x=0;x<rect.w;x+=2) {
        u[x/2] = (U[ y   *chroma_stride + x] + U[ y   *chroma_stride + x +1] +
                  U[(y+1)*chroma_stride + x] + U[(y+1)*chroma_stride + x +1])/4;
        v[x/2] = (V[ y   *chroma_stride + x] + V[ y   *chroma_stride + x +1] +
                  V[(y+1)*chroma_stride + x] + V[(y+1)*chroma_stride + x +1])/4;

        //u[x/2] = U[y*chroma_stride + x];
        //v[x/2] = V[y*chroma_stride + x];
      }
    }
}


bool SDL_YUV_Display::doQuit() const
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      return true;
    }
  }

  return false;
}

void SDL_YUV_Display::close()
{
  SDL_FreeYUVOverlay(mYUVOverlay);
  SDL_Quit();

  mWindowOpen=false;
}
