#ifndef _FFMPEG_FORMAT_CTX_HPP_
#define _FFMPEG_FORMAT_CTX_HPP_

#include <memory>
#include <string>
#include <vector>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
}

#include "FFmpegIOCtx.hpp"
#include "FFmpegPacket.hpp"

class FFmpegFormatCtx
{
public:
    using Ptr = std::shared_ptr<FFmpegFormatCtx>;

    FFmpegFormatCtx(std::string url);
    FFmpegFormatCtx(FFmpegIOCtx::Ptr avio, int buff_size);
    std::vector<unsigned int> FindVideoStreamIDX();
    AVCodecID GetCodecID(unsigned int stream_idx);
    void TurnOnRealTimeMode();
    void TurnOffRealTimeMode();
    FFmpegPacket::Ptr ReadPacket();
    ~FFmpegFormatCtx();

private:
    int _buff_size;
    FFmpegIOCtx::Weak _avio_weak;
    FFmpegIOCtx::Ptr _avio;
    std::shared_ptr<AVFormatContext> _format_context;
    AVFormatContext* _format_ptr;
};

#endif