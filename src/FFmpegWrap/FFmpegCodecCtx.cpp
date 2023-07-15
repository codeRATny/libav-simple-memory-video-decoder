#include "FFmpegCodecCtx.hpp"

void FFmpegCodecCtx::Open(AVCodecID codec_id)
{
    LOG(LOG_INFO, "Open codec with id = " + std::to_string(codec_id));
    _codec_ctx.reset(avcodec_alloc_context3(avcodec_find_decoder(codec_id)), [](AVCodecContext *ptr){
        avcodec_free_context(&ptr);
    });

    avcodec_open2(_codec_ctx.get(), _codec_ctx->codec, NULL);

    LOG(LOG_INFO, "Was opened codec with id = " + std::to_string(_codec_ctx->codec_id));
}

FFmpegFrame::Ptr FFmpegCodecCtx::Decode(FFmpegPacket::Ptr &packet)
{
    auto ret = avcodec_send_packet(_codec_ctx.get(), packet->get());

    if (ret < 0)
    {
        LOG(LOG_ERR, "Error - " + std::to_string(ret));
        return nullptr;
    }

    auto out_frame = std::make_shared<FFmpegFrame>();
    ret = avcodec_receive_frame(_codec_ctx.get(), out_frame->get());
    LOG(LOG_INFO, "Receive frame from decoder");

    if (ret != 0)
    {
        char buff[512];
        av_make_error_string(buff, 512, ret);
        LOG(LOG_ERR, "Error - " + std::string(buff));
        return nullptr;
    }

    LOG(LOG_INFO, "New frame: width = " + std::to_string(out_frame->get()->width) + " height = " + std::to_string(out_frame->get()->height));

    return out_frame;
}

AVCodecContext *FFmpegCodecCtx::get()
{
    return _codec_ctx.get();
}