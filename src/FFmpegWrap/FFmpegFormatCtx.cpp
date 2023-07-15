#include "FFmpegFormatCtx.hpp"

FFmpegFormatCtx::FFmpegFormatCtx(std::string url)
{
    LOG(LOG_INFO, "Construct");
    _format_ctx.reset(avformat_alloc_context(), [](AVFormatContext *ptr){
        avformat_close_input(&ptr);
    });

    _format_ptr = _format_ctx.get();
    avformat_open_input(&_format_ptr, url.c_str(), NULL, NULL);
}

FFmpegFormatCtx::FFmpegFormatCtx(FFmpegIOCtx::Ptr avio, int buff_size)
{
    LOG(LOG_INFO, "Construct with buffer = " + std::to_string(buff_size));
    _format_ctx.reset(avformat_alloc_context(), [](AVFormatContext *ptr){
        avformat_close_input(&ptr);
    });

    _buff_size = buff_size;
    _avio = std::move(avio);
    _avio_weak = _avio;
    _avio->Init(_buff_size, _avio_weak);

    _format_ctx->pb = _avio->get();

    _format_ptr = _format_ctx.get();
    avformat_open_input(&_format_ptr, NULL, NULL, NULL);
}

std::vector<unsigned int> FFmpegFormatCtx::FindVideoStreamIDX()
{
    std::vector<unsigned int> result;

    for (unsigned int i = 0; i < _format_ctx->nb_streams; i++)
    {
        if (_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if (_format_ctx->streams[i])
                if (_format_ctx->streams[i]->codecpar)
                    result.push_back(i);
        }
    }

    return result;
}

AVCodecID FFmpegFormatCtx::GetCodecID(unsigned int stream_idx)
{
    if (stream_idx < _format_ctx->nb_streams)
    {
        if (_format_ctx->streams[stream_idx])
            if (_format_ctx->streams[stream_idx]->codecpar)
                return _format_ctx->streams[stream_idx]->codecpar->codec_id;
    }

    return AV_CODEC_ID_NONE;
}

void FFmpegFormatCtx::TurnOnRealTimeMode()
{

}

void FFmpegFormatCtx::TurnOffRealTimeMode()
{
    
}

int FFmpegFormatCtx::ReadPacket(FFmpegPacket::Ptr &packet)
{
    packet = std::make_shared<FFmpegPacket>();
    return av_read_frame(_format_ctx.get(), packet->get());
}

void FFmpegFormatCtx::DropEOS()
{
    _format_ctx->pb->eof_reached = 0;
}

FFmpegFormatCtx::~FFmpegFormatCtx()
{
    // _avio = nullptr;
    LOG(LOG_INFO, "Destruct");
}