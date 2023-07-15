#include "player_utils.hpp"

void SaveAvFrame(FFmpegFrame::Ptr frame, const char *filename)
{
    FILE *fDump = fopen(filename, "ab");

    auto frame_ptr = frame->get();

    uint32_t pitchY = frame_ptr->linesize[0];
    uint32_t pitchU = frame_ptr->linesize[1];
    uint32_t pitchV = frame_ptr->linesize[2];

    uint8_t *avY = frame_ptr->data[0];
    uint8_t *avU = frame_ptr->data[1];
    uint8_t *avV = frame_ptr->data[2];

    for (int i = 0; i < frame_ptr->height; i++)
    {
        fwrite(avY, frame_ptr->width, 1, fDump);
        avY += pitchY;
    }

    for (int i = 0; i < frame_ptr->height / 2; i++)
    {
        fwrite(avU, frame_ptr->width / 2, 1, fDump);
        avU += pitchU;
    }

    for (int i = 0; i < frame_ptr->height / 2; i++)
    {
        fwrite(avV, frame_ptr->width / 2, 1, fDump);
        avV += pitchV;
    }

    fclose(fDump);
}

// libav source
void avcodec_get_frame_defaults(AVFrame *frame)
{
#if LIBAVCODEC_VERSION_MAJOR >= 55
    // extended_data should explicitly be freed when needed, this code is unsafe currently
    // also this is not compatible to the <55 ABI/API
    if (frame->extended_data != frame->data && 0)
        av_freep(&frame->extended_data);
#endif

    memset(frame, 0, sizeof(AVFrame));
    av_frame_unref(frame);
}