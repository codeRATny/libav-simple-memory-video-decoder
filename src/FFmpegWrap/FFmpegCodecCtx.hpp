#ifndef _FFMPEG_CODEC_CTX_HPP_
#define _FFMPEG_CODEC_CTX_HPP_

#include <memory>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
}

#include "lazy_logs.hpp"
#include "FFmpegFrame.hpp"
#include "FFmpegPacket.hpp"

class FFmpegCodecCtx
{
public:
    using Ptr = std::shared_ptr<FFmpegCodecCtx>;

    FFmpegCodecCtx() = default;
    void Open(AVCodecID codec_id);
    FFmpegFrame::Ptr Decode(FFmpegPacket::Ptr &packet);
    AVCodecContext *get();
    ~FFmpegCodecCtx() = default;

private:
    std::shared_ptr<AVCodecContext> _codec_ctx;
};

#endif