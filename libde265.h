#pragma once
#include "libde265/de265.h"
#include "libde265/image.h"
class Libde265 : public VideoDecoder
{
public:
	de265_PTS pts;
	de265_decoder_context *h265DecContext;
	Libde265()
	{
		h265DecContext = de265_new_decoder();
		emscripten_log(0, "H265 init");
	}
	~Libde265()
	{
		de265_free_decoder(h265DecContext);
	}
	void _decode(const char *data, int len) override
	{
		de265_push_NAL(h265DecContext, data, len, 0, nullptr);
		int more = 1;
		while (more)
		{
			more = 0;
			auto err = de265_decode(h265DecContext, &more);
			if (err != DE265_OK)
			{
				emscripten_log(0, "de265_decode：%d", err);
				break;
			}
			const de265_image *img = de265_get_next_picture(h265DecContext);
			if (img)
			{
				//emscripten_log(0, "%d", img);
				int out_stride;
				for (int i = 0; i < 3; i++)
					p_yuv[i] = (u32)de265_get_image_plane(img, i, &out_stride);
				if (videoWidth == 0)
					decodeVideoSize(de265_get_image_width(img, 0), de265_get_image_height(img, 0));
				decodeYUV420();
			}
		}
	}
};