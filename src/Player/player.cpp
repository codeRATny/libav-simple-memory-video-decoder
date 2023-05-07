#include "player.hpp"

#define LOGS(lvl, message) LOG("INPUT SETUP", lvl, message)
#define LOGP(lvl, message) LOG("PARSER", lvl, message)
#define LOGD(lvl, message) LOG("DECODER", lvl, message)
#define LOGR(lvl, message) LOG("DATA READER", lvl, message)
#define LOGM(lvl, message) LOG("PLAYER", lvl, message)

int Player::setup_input(Player *context)
{
    context->_setup_sem.wait();
    int ret;

    // reader buff
    context->_ibuf = (uint8_t *)av_malloc(context->_ibuf_size);

    // create context for reading from memory
    context->_avio_in = avio_alloc_context(context->_ibuf, context->_ibuf_size, 0, context, &read_packet, NULL, NULL);
    context->_format_context = avformat_alloc_context();
    context->_format_context->pb = context->_avio_in;

    // open empty io
    avformat_open_input(&context->_format_context, NULL, NULL, NULL);

    for (int i = 0; i < context->_format_context->nb_streams; i++)
    {
        if (context->_format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            context->_video_stream = i;
            break;
        }
    }

    LOGS(LOG_INFO, "READY TO PLAY");
    LOGS(LOG_LVL, "Stream index" + std::to_string(context->_video_stream));

    context->_start_sem.post();

    return 0;
}

bool Player::grab_frames(Player *context)
{
    // waiting for play
    context->_start_sem.wait();

    // join setup thread
    context->_setup_thread.join();

    if (!context->_format_context->streams[context->_video_stream] || !context->_format_context->streams[context->_video_stream]->codec)
    {
        LOGP(LOG_WARN, "Not valid stream or codec");
        exit(1);
    }

    if (context->_format_context->streams[context->_video_stream]->codec->codec_type != AVMEDIA_TYPE_VIDEO)
    {
        LOGP(LOG_WARN, "Skipping audio");
        context->Stop();
    }

    context->_format_context->streams[context->_video_stream]->codec->codec = avcodec_find_decoder(context->_format_context->streams[context->_video_stream]->codec->codec_id);
    if (context->_format_context->streams[context->_video_stream]->codec->codec == NULL)
    {
        LOGP(LOG_WARN, "Decoder not found, skipping...");
        context->Stop();
    }

    LOGP(LOG_INFO, "Codec " + std::string(context->_format_context->streams[context->_video_stream]->codec->codec->name));

    if (avcodec_open2(context->_format_context->streams[context->_video_stream]->codec, context->_format_context->streams[context->_video_stream]->codec->codec, NULL) < 0)
    {
        LOGP(LOG_ERR, "Codec open err");
        context->Stop();
    }

    // adding opened codec
    context->_codec_context = context->_format_context->streams[context->_video_stream]->codec;

    if (context->_format_context->streams[context->_video_stream]->codec->codec->capabilities & CODEC_CAP_TRUNCATED)
    {
        context->_codec_context->flags |= CODEC_FLAG_TRUNCATED;
    }

    int rt;

    // endless loop for parse packs
    do
    {
        AVPacket pack;

        LOGP(LOG_INFO, "receive new data");

        while ((rt = av_read_frame(context->_format_context, &pack)) >= 0)
        {
            context->_parser_mutex.lock();
            context->_queue_packs.push({pack.dts, pack.pts, std::move(pack)});
            context->_queue_sem.post();
            context->_parser_mutex.unlock();
        }

        LOGP(LOG_INFO, "waiting data");
        context->_append_sem.wait();

        // drop eos
        context->_format_context->pb->eof_reached = 0;
    } while (true);

    return 0;
}

// e.g. for getting filename for saving frames
static std::string get_filename()
{
    static int i;
    i++;
    return "frame_" + std::to_string(i) + ".yuv";
}

void Player::decode_queue(Player *context)
{
    int frameFinished;
    context->_frame = av_frame_alloc();

    // endless loop for decoding queue of packs
    do
    {
        // waiting for data on queue
        context->_queue_sem.wait();

        context->_decoder_mutex.lock();

        // decode
        avcodec_get_frame_defaults(context->_frame);
        avcodec_send_packet(context->_codec_context, &context->_queue_packs.front().avpack);
        avcodec_receive_frame(context->_codec_context, context->_frame);

        // free mem and save
        av_free_packet(&context->_queue_packs.front().avpack);
        if (context->_frame->pkt_size != -1)
            SaveAvFrame(context->_frame, get_filename().c_str());
        av_frame_unref(context->_frame);

        context->_queue_packs.pop();

        context->_decoder_mutex.unlock();
    } while (true);
}

Player::Player()
{
    av_log_set_level(AV_LOG_QUIET);

    _append_buff_size_update_counter = 0;
    _format_context = NULL;
    _codec_context = NULL;
    _video_stream = -1;
    _frame = NULL;
    _avio_in = NULL;

    _state = STATE_NO_DATA | STATE_STOP;
    _append_buff_size = DEFAULT_BUFF_SIZE;
    _ibuf_size = SIZE_TS_PACK;

    _append_buff = (char *)malloc(_append_buff_size);
}

Player::~Player()
{
    if (_setup_thread.joinable())
    {
        pthread_cancel(_setup_thread.native_handle());
        _setup_thread.join();
    }

    if (_parser_thread.joinable())
    {
        pthread_cancel(_parser_thread.native_handle());
        _parser_thread.join();
    }

    if (_decoder_thread.joinable())
    {
        pthread_cancel(_decoder_thread.native_handle());
        _decoder_thread.join();
    }

    drop_queue();

    if (_avio_in)
    {
        av_freep(&_avio_in->buffer);
        avio_context_free(&_avio_in);
    }

    if (_codec_context)
        avcodec_close(_codec_context);

    if (_format_context)
    {
        avformat_close_input(&_format_context);
    }

    if (_append_buff)
        free(_append_buff);
    
    if (_frame)
        av_frame_free(&_frame);
}

void Player::Play()
{
    if (_state & STATE_PAUSE)
    {
        _state ^= STATE_PAUSE;
        _state |= STATE_PLAY;
        _parser_mutex.unlock();
        _decoder_mutex.unlock();

        LOGM(LOG_INFO, "RESUME");
    }
    else if (_state & STATE_STOP)
    {
        _state ^= STATE_STOP;
        _state |= STATE_PLAY;
        _setup_thread = std::thread(setup_input, this);
        _parser_thread = std::thread(grab_frames, this);
        _decoder_thread = std::thread(decode_queue, this);

        LOGM(LOG_INFO, "PLAYING");
    }
}

void Player::drop_queue()
{
    while (_queue_packs.size() != 0)
    {
        av_free_packet(&_queue_packs.front().avpack);
        _queue_packs.pop();
    }
}

void Player::Stop()
{
    if (_setup_thread.joinable())
    {
        pthread_cancel(_setup_thread.native_handle());
        _setup_thread.join();
    }

    if (_parser_thread.joinable())
    {
        pthread_cancel(_parser_thread.native_handle());
        _parser_thread.join();
    }

    if (_decoder_thread.joinable())
    {
        pthread_cancel(_decoder_thread.native_handle());
        _decoder_thread.join();
    }

    drop_queue();

    if (_codec_context)
    {
        avcodec_close(_codec_context);
        _codec_context = NULL;
    }

    if (_avio_in)
    {
        av_freep(&_avio_in->buffer);
        avio_context_free(&_avio_in);
        _avio_in = NULL;
    }

    if (_frame)
    {
        av_frame_free(&_frame);
        _frame = NULL;
    }

    if (_format_context)
    {
        avformat_close_input(&_format_context);
        _format_context = NULL;
    }

    if (_append_buff)
    {
        free(_append_buff);
        _append_buff = NULL;
    }

    LOGM(LOG_INFO, "STOPPED");
}

void Player::Pause()
{
    if (_state & STATE_PLAY)
    {
        _state ^= STATE_PLAY;
        _state |= STATE_PAUSE;
        _parser_mutex.lock();
        _decoder_mutex.lock();

        LOGM(LOG_INFO, "PAUSED");
    }
}

int Player::read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    Player *ctx = (Player *)opaque;
    std::lock_guard<std::mutex> lock(ctx->_data_mutex);

    if (ctx->_data.tellg() >= ctx->_data.tellp())
    {
        if (ctx->_state & STATE_HAVE_DATA)
        {
            ctx->_state ^= STATE_HAVE_DATA;
            ctx->_state |= STATE_NO_DATA;
            LOGR(LOG_WARN, "no data");
        }
        return AVERROR_EOF;
    }

    ctx->_data.read((char *)buf, buf_size);

    return buf_size;
}

void Player::AppendData(const char *new_data, size_t new_data_size)
{
    std::lock_guard<std::mutex> lock(_data_mutex);

    size_t old_data_size = _data.tellp() - _data.tellg();

    if ((_append_buff_size < old_data_size) | (_append_buff_size_update_counter == 10))
    {
        free(_append_buff);
        _append_buff_size = old_data_size;
        _append_buff = (char *)malloc(_append_buff_size);
    }
    else
    {
        _append_buff_size_update_counter++;
    }

    _data.read(_append_buff, old_data_size);
    _data.str("");
    _data.clear();

    _data.write(_append_buff, old_data_size);
    _data.write(new_data, new_data_size);

    _data.seekg(0);

    if ((_state & STATE_STOP) && (_state & STATE_NO_DATA))
    {
        _state ^= STATE_NO_DATA;
        _state |= STATE_HAVE_DATA;
        _setup_sem.post();
    }

    if ((_state & STATE_PLAY) && (_state & STATE_NO_DATA))
    {
        _state ^= STATE_NO_DATA;
        _state |= STATE_HAVE_DATA;
        _append_sem.post();
    }
}