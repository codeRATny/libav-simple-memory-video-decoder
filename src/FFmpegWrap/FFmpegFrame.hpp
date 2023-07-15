#ifndef _FFMPEG_FRAME_HPP_
#define _FFMPEG_FRAME_HPP_

#include <memory>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
}

class FFmpegFrame
{
public:
    using Ptr = std::shared_ptr<FFmpegFrame>;

    FFmpegFrame(std::shared_ptr<AVFrame> frame = nullptr);
    ~FFmpegFrame();

    AVFrame *get();
private:
    std::shared_ptr<AVFrame> _frame;
};

#endif