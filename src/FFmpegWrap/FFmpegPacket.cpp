#include "FFmpegPacket.hpp"

FFmpegPacket::FFmpegPacket(std::shared_ptr<AVPacket> packet)
{
    if (packet)
    {
        _pack = std::move(packet);
    }
    else
    {
        _pack.reset(av_packet_alloc(), [](AVPacket *ptr) {
            av_packet_free(&ptr);
        });
    }
}

AVPacket *FFmpegPacket::get()
{
    return _pack.get();
}