#include "player_utils.hpp"

void SaveAvFrame(AVFrame *avFrame, const char *filename)
{
    FILE *fDump = fopen(filename, "ab");

    uint32_t pitchY = avFrame->linesize[0];
    uint32_t pitchU = avFrame->linesize[1];
    uint32_t pitchV = avFrame->linesize[2];

    uint8_t *avY = avFrame->data[0];
    uint8_t *avU = avFrame->data[1];
    uint8_t *avV = avFrame->data[2];

    for (int i = 0; i < avFrame->height; i++)
    {
        fwrite(avY, avFrame->width, 1, fDump);
        avY += pitchY;
    }

    for (int i = 0; i < avFrame->height / 2; i++)
    {
        fwrite(avU, avFrame->width / 2, 1, fDump);
        avU += pitchU;
    }

    for (int i = 0; i < avFrame->height / 2; i++)
    {
        fwrite(avV, avFrame->width / 2, 1, fDump);
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