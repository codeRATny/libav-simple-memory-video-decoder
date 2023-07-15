#include "FFmpegIOCtx.hpp"

FFmpegIOCtx::FFmpegIOCtx()
{
    LOG(LOG_INFO, "Construct FFmpegIOCtx");
    _read_fun_ptr = nullptr;
    _write_fun_ptr = nullptr;
    _seek_fun_ptr = nullptr;
    _buffer = nullptr;
}

FFmpegIOCtx::~FFmpegIOCtx()
{
    LOG(LOG_INFO, "Destruct FFmpegIOCtx");
}

void FFmpegIOCtx::SetOnRead(std::function<int(uint8_t *buf, int buf_size)> cb)
{
    LOG(LOG_INFO, "SetOnRead");
    if (cb)
    {
        _read_cb = std::move(cb);
        _read_fun_ptr = &read_packet;
    }
}

void FFmpegIOCtx::SetOnWrite(std::function<int(uint8_t *buf, int buf_size)> cb)
{
    LOG(LOG_INFO, "SetOnWrite");
    if (cb)
    {
        _write_cb = std::move(cb);
        _write_fun_ptr = &read_packet;
    }
}

void FFmpegIOCtx::SetOnSeek(std::function<int64_t(int64_t offset, int whence)> cb)
{
    LOG(LOG_INFO, "SetOnSeek");
    if (cb)
    {
        _seek_cb = std::move(cb);
        _seek_fun_ptr = &seek_packet;
    }
}

void FFmpegIOCtx::Init(int buffer_size, std::weak_ptr<FFmpegIOCtx> &weak_on_this_obj)
{
    LOG(LOG_INFO, "Init with buff = " + std::to_string(buffer_size));
    _buffer_size = buffer_size;
    _buffer = new unsigned char[buffer_size];

    int write_flag = _write_fun_ptr ? 1 : 0;

    _avio.reset(avio_alloc_context(_buffer, _buffer_size, write_flag, &weak_on_this_obj, _read_fun_ptr, _write_fun_ptr, _seek_fun_ptr),
                [](AVIOContext *ptr)
                {
                    av_freep(&ptr->buffer);
                    avio_context_free(&ptr);
                });
}

int FFmpegIOCtx::read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    auto strong_self = (*(std::weak_ptr<FFmpegIOCtx> *)opaque).lock();

    if (strong_self.get())
        return strong_self->_read_cb(buf, buf_size);

    return AVERROR_EOF;
}

int FFmpegIOCtx::write_packet(void *opaque, uint8_t *buf, int buf_size)
{
    auto strong_self = (*(std::weak_ptr<FFmpegIOCtx> *)opaque).lock();

    if (strong_self.get())
        return strong_self->_write_cb(buf, buf_size);

    return AVERROR_EOF;
}

int64_t FFmpegIOCtx::seek_packet(void *opaque, int64_t offset, int whence)
{
    auto strong_self = (*(std::weak_ptr<FFmpegIOCtx> *)opaque).lock();

    if (strong_self.get())
        return strong_self->_seek_cb(offset, whence);

    return AVERROR_EOF;
}

AVIOContext *FFmpegIOCtx::get()
{
    return _avio.get();
}