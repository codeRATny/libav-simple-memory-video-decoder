#include "player.hpp"

int Player::setup_input()
{
    _setup_sem.wait();

    LOG(LOG_INFO, "Creating avio ctx");
    _avio_ctx = std::make_shared<FFmpegIOCtx>();

    LOG(LOG_INFO, "Setuping read cb");
    _avio_ctx->SetOnRead([&](uint8_t *buf, int buf_size){
        std::lock_guard<std::mutex> lock(_data_mutex);

        if (_data.tellg() >= _data.tellp())
        {
            if (_state & STATE_HAVE_DATA)
            {
                _state ^= STATE_HAVE_DATA;
                _state |= STATE_NO_DATA;
                LOG(LOG_WARN, "no data");
            }
            return AVERROR_EOF;
        }

        _data.read((char *)buf, buf_size);

        return buf_size;
    });

    LOG(LOG_INFO, "Creating format ctx");
    _format_ctx = std::make_shared<FFmpegFormatCtx>(_avio_ctx, SIZE_TS_PACK);

    LOG(LOG_INFO, "Call FindVideoStreamIDX");
    auto ids = _format_ctx->FindVideoStreamIDX();

    for (auto& i : ids)
    {
        LOG(LOG_INFO, "Found stream id = " + std::to_string(i));
        _video_stream_idx = ids[i];
    }

    LOG(LOG_INFO, "READY TO PLAY");
    LOG(LOG_LVL, "Stream index" + std::to_string(_video_stream_idx));

    _start_sem.post();

    return 0;
}

bool Player::grab_frames()
{
    // waiting for play
    _start_sem.wait();
    _setup_thread.join();

    auto codec_id = _format_ctx->GetCodecID(_video_stream_idx);

    _codec_ctx = std::make_shared<FFmpegCodecCtx>();
    _codec_ctx->Open(codec_id);

    // endless loop for parse packs
    do
    {
        FFmpegPacket::Ptr packet = std::make_shared<FFmpegPacket>();

        LOG(LOG_INFO, "receive new data");


        while (_format_ctx->ReadPacket(packet) >= 0)
        {
            _parser_mutex.lock();
            _queue_packs.push(std::move(packet));
            _queue_sem.post();
            _parser_mutex.unlock();
        }

        LOG(LOG_INFO, "waiting data");
        _append_sem.wait();

        // drop eos
        _format_ctx->DropEOS();
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

void Player::decode_queue()
{
    // endless loop for decoding queue of packs
    do
    {
        // waiting for data on queue
        _queue_sem.wait();

        _decoder_mutex.lock();

        // decode
        auto frame = _codec_ctx->Decode(_queue_packs.front());

        // save
        if (frame)
            SaveAvFrame(frame, get_filename().c_str());

        _queue_packs.pop();

        _decoder_mutex.unlock();
    } while (true);
}

Player::Player()
{
    // av_log_set_level(AV_LOG_QUIET);
    _format_ctx = nullptr;
    LOG(LOG_INFO, "Init Player");

    _append_buff_size_update_counter = 0;
    _video_stream_idx = -1;

    _state = STATE_NO_DATA | STATE_STOP;

    _append_buff_size = DEFAULT_BUFF_SIZE;
    _append_buff = new char[_append_buff_size];
}

Player::~Player()
{
    Stop();
    _format_ctx = nullptr;
    LOG(LOG_INFO, "Deinit Player");
}

void Player::Play()
{
    if (_state & STATE_PAUSE)
    {
        _state ^= STATE_PAUSE;
        _state |= STATE_PLAY;
        _parser_mutex.unlock();
        _decoder_mutex.unlock();

        LOG(LOG_INFO, "RESUME");
    }
    else if (_state & STATE_STOP)
    {
        _state ^= STATE_STOP;
        _state |= STATE_PLAY;
        _setup_thread = std::thread([&](){
            setup_input();
        });

        _parser_thread = std::thread([&](){
            grab_frames();
        });

        _decoder_thread = std::thread([&](){
            decode_queue();
        });

        LOG(LOG_INFO, "PLAYING");
    }
}

void Player::drop_queue()
{
    while (_queue_packs.size() != 0)
    {
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

    if (_append_buff)
    {
        delete[] _append_buff;
        _append_buff = nullptr;
    }

    LOG(LOG_INFO, "STOPPED");
}

void Player::Pause()
{
    if (_state & STATE_PLAY)
    {
        _state ^= STATE_PLAY;
        _state |= STATE_PAUSE;
        _parser_mutex.lock();
        _decoder_mutex.lock();

        LOG(LOG_INFO, "PAUSED");
    }
}

void Player::AppendData(const char *new_data, size_t new_data_size)
{
    std::lock_guard<std::mutex> lock(_data_mutex);

    size_t old_data_size = _data.tellp() - _data.tellg();

    if ((_append_buff_size < old_data_size) | (_append_buff_size_update_counter == 10))
    {
        delete[] _append_buff;
        _append_buff_size = old_data_size;
        _append_buff = new char[_append_buff_size];
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

    LOG(LOG_INFO, "Append data: size = " + std::to_string(new_data_size));
}