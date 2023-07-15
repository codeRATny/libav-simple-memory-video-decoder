#ifndef _FFMPEG_PACKET_HPP_
#define _FFMPEG_PACKET_HPP_

#include <memory>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
}

class FFmpegPacket
{
public:
    using Ptr = std::shared_ptr<FFmpegPacket>;

    FFmpegPacket(std::shared_ptr<AVPacket> packet = nullptr);
    ~FFmpegPacket() = default;
    AVPacket *get();

private:
    std::shared_ptr<AVPacket> _pack;
};

#endif