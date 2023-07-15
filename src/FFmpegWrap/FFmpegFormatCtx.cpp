#include "FFmpegFormatCtx.hpp"

FFmpegFormatCtx::FFmpegFormatCtx(std::string url)
{
    _format_context.reset(avformat_alloc_context(), [](AVFormatContext *ptr){
        avformat_close_input(&ptr);
    });

    _format_ptr = _format_context.get();
    avformat_open_input(&_format_ptr, url.c_str(), NULL, NULL);
}

FFmpegFormatCtx::FFmpegFormatCtx(FFmpegIOCtx::Ptr avio, int buff_size)
{
    _format_context.reset(avformat_alloc_context(), [](AVFormatContext *ptr){
        avformat_close_input(&ptr);
    });

    _buff_size = buff_size;
    _avio = std::move(avio);
    _avio_weak = _avio;
    _avio->Init(_buff_size, _avio_weak);

    _format_context->pb = _avio->get();

    _format_ptr = _format_context.get();
    avformat_open_input(&_format_ptr, NULL, NULL, NULL);
}

std::vector<unsigned int> FFmpegFormatCtx::FindVideoStreamIDX()
{
    std::vector<unsigned int> result;

    for (unsigned int i = 0; i < _format_context->nb_streams; i++)
    {
        if (_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if (_format_context->streams[i])
                if (_format_context->streams[i]->codecpar)
                    result.push_back(i);
        }
    }

    return result;
}

AVCodecID FFmpegFormatCtx::GetCodecID(unsigned int stream_idx)
{
    if (stream_idx < _format_context->nb_streams)
    {
        if (_format_context->streams[stream_idx])
            if (_format_context->streams[stream_idx]->codecpar)
                return _format_context->streams[stream_idx]->codecpar->codec_id;
    }

    return AV_CODEC_ID_NONE;
}

void FFmpegFormatCtx::TurnOnRealTimeMode()
{

}

void FFmpegFormatCtx::TurnOffRealTimeMode()
{
    
}

FFmpegPacket::Ptr FFmpegFormatCtx::ReadPacket()
{
    FFmpegPacket::Ptr out_packet = std::make_shared<FFmpegPacket>();
    auto ret = av_read_frame(_format_context.get(), out_packet->get());

    if (ret != 0)
        return nullptr;

    return out_packet;
}

FFmpegFormatCtx::~FFmpegFormatCtx()
{
    _avio = nullptr;
}