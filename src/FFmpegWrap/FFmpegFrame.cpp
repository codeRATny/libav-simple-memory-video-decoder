#include "FFmpegFrame.hpp"

FFmpegFrame::FFmpegFrame(std::shared_ptr<AVFrame> frame)
{
    if (frame)
    {
        _frame = std::move(frame);
    }
    else
    {
        _frame.reset(av_frame_alloc(), [](AVFrame *ptr){
            av_frame_free(&ptr);
        });
    }
}

FFmpegFrame::~FFmpegFrame()
{
}

AVFrame *FFmpegFrame::get()
{
    return _frame.get();
}