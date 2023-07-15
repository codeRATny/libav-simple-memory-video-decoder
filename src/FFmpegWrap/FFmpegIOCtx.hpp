#ifndef _FFMPEG_IO_CTX_HPP_
#define _FFMPEG_IO_CTX_HPP_

#include <memory>
#include <functional>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
}

#include "lazy_logs.hpp"

class FFmpegIOCtx : public std::enable_shared_from_this<FFmpegIOCtx>
{
public:
    using Ptr = std::shared_ptr<FFmpegIOCtx>;
    using Weak = std::weak_ptr<FFmpegIOCtx>;

    FFmpegIOCtx();
    ~FFmpegIOCtx();
    void Init(int buffer_size, std::weak_ptr<FFmpegIOCtx> &weak_on_this_obj);
    void SetOnRead(std::function<int(uint8_t *buf, int buf_size)> cb);
    void SetOnWrite(std::function<int(uint8_t *buf, int buf_size)> cb);
    void SetOnSeek(std::function<int64_t(int64_t offset, int whence)> cb);
    AVIOContext *get();

private:
    std::shared_ptr<AVIOContext> _avio;

    std::function<int(uint8_t *buf, int buf_size)> _read_cb;
    std::function<int(uint8_t *buf, int buf_size)> _write_cb;
    std::function<int64_t(int64_t offset, int whence)> _seek_cb;

    int (*_read_fun_ptr)(void *opaque, uint8_t *buf, int buf_size);
    int (*_write_fun_ptr)(void *opaque, uint8_t *buf, int buf_size);
    int64_t (*_seek_fun_ptr)(void *opaque, int64_t offset, int whence);

    int _buffer_size;
    unsigned char *_buffer;

    static int read_packet(void *opaque, uint8_t *buf, int buf_size);
    static int write_packet(void *opaque, uint8_t *buf, int buf_size);
    static int64_t seek_packet(void *opaque, int64_t offset, int whence);
};

#endif