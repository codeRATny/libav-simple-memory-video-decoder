#include "FFmpegPacket.hpp"

FFmpegPacket::FFmpegPacket(std::shared_ptr<AVPacket> packet)
{
    if (packet)
    {
        LOG(LOG_INFO, "Init with packet");
        _pack = std::move(packet);
    }
    else
    {
        LOG(LOG_INFO, "Init new packet");
        _pack.reset(av_packet_alloc(), [](AVPacket *ptr) {
            av_packet_free(&ptr);
        });
    }
}

AVPacket *FFmpegPacket::get()
{
    LOG(LOG_INFO, "get()");
    return _pack.get();
}

int FFmpegPacket::GetStreamIdx()
{
    return _pack.get()->stream_index;
}