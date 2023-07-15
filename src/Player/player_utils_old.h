#ifndef _PLAYER_UTILS_HPP_
#define _PLAYER_UTILS_HPP_

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavutil/avutil.h>
};

#define CODEC_CAP_TRUNCATED 0x0008      // libav source
#define CODEC_FLAG_TRUNCATED 0x00010000 // libav source

void SaveAvFrame(AVFrame *avFrame, const char *filename);
void avcodec_get_frame_defaults(AVFrame *frame);

#endif