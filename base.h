#pragma once
//#define USE_H265
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <functional>
#include "basetype.h"
// void* H264SwDecMalloc(u32 size)
// {
// 	return malloc(size);
// }
// void H264SwDecFree(void *ptr)
// {
// 	free(ptr);
// }
// void H264SwDecMemcpy(void *dest, void *src, u32 count)
// {
// 	memcpy(dest, src, count);
// }
// void H264SwDecMemset(void *ptr, i32 value, u32 count)
// {
// 	memset(ptr, value, count);
// }
#include <time.h>
#include <map>
#include <queue>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <time.h>
using namespace std;
using namespace emscripten;
#include "MemoryStream.h"
//#define USEBUFFERARRAY
#include "AudioDecoder.h"

inline int yuv2rgbcalc(int y, int u, int v)
{

	int a0 = 1192 * (y - 16);
	int a1 = 1634 * (v - 128);
	int a2 = 832 * (v - 128);
	int a3 = 400 * (u - 128);
	int a4 = 2066 * (u - 128);

	int r = (a0 + a1) >> 10;
	int g = (a0 - a2 - a3) >> 10;
	int b = (a0 + a4) >> 10;

	if ((r & 255) != r)
	{
		r = min(255, max(0, r));
	}
	if ((g & 255) != g)
	{
		g = min(255, max(0, g));
	}
	if ((b & 255) != b)
	{
		b = min(255, max(0, b));
	}
	return (((((255 << 8) + b) << 8) + g) << 8) + r;
}
inline void yuv420toRGB(u8 *Y, u8 *U, u8 *V, u8 *heap, u32 width, u32 height)
{
	int ystart = 0;
	int ustart = 0;
	int vstart = 0;

	int y = 0;
	int yn = 0;
	int u = 0;
	int v = 0;

	u32 *o = 0;

	int line = 0;
	int col = 0;

	int usave = 0;
	int vsave = 0;

	int ostart = 0;
	int cacheAdr = 0;
	ustart = 0;
	vstart = 0;
	u8 *cacheStart = heap + (width * height << 2);
	for (line = 0; line < height; line += 2)
	{
		usave = ustart;
		vsave = vstart;
		for (col = 0; col < width; col += 2)
		{
			y = Y[ystart];
			yn = Y[ystart + width];

			u = U[ustart];
			v = V[vstart];

			cacheAdr = (y << 16) + (u << 8) + v;
			o = (u32 *)(cacheStart + cacheAdr);
			if (!*o)
				*o = yuv2rgbcalc(y, u, v);
			*(u32 *)(heap + ostart) = *o;

			cacheAdr = ((yn << 16) + (u << 8)) + v;
			o = (u32 *)(cacheStart + cacheAdr);
			if (!*o)
				*o = yuv2rgbcalc(yn, u, v);
			*(u32 *)(heap + (ostart + (width << 2))) = *o;

			//yuv2rgb5(y, u, v, ostart);
			//yuv2rgb5(yn, u, v, (ostart + widthFour)|0);
			ostart += 4;

			// next step only for y. u and v stay the same
			ystart++;
			y = Y[ystart];
			yn = Y[(ystart + width)];

			//yuv2rgb5(y, u, v, ostart);
			cacheAdr = (y << 16) + (u << 8) + v;
			o = (u32 *)(cacheStart + cacheAdr);
			if (!*o)
				*o = yuv2rgbcalc(y, u, v);
			*(u32 *)(heap + ostart) = *o;

			//yuv2rgb5(yn, u, v, (ostart + widthFour)|0);
			cacheAdr = ((yn << 16) + (u << 8)) + v;
			o = (u32 *)(cacheStart + cacheAdr);
			if (!*o)
				*o = yuv2rgbcalc(yn, u, v);
			*(u32 *)(heap + (ostart + (width << 2))) = *o;
			ostart += 4;

			//all positions inc 1
			ystart++;
			ustart++;
			vstart++;
		}
		ostart += (width << 2);
		ystart += width;
	}
}
#include "VideoDecoder.h"
#ifdef USE_LIBDE265
#include "libde265.h"
#define VIDEO_DECODER Libde265
#elif USE_FFMPEG
#include "ffmpeg.h"
#define VIDEO_DECODER FFmpeg
#elif USE_LIBHEVC
#include "libhevc.h"
#define VIDEO_DECODER Libhevc
#else
#include "broadway.h"
#define VIDEO_DECODER Broadway
#endif