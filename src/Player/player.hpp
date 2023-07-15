#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_

#include <thread>
#include <string>
#include <queue>
#include <utility>
#include <fstream>
#include <sstream>
#include <mutex>

#include "utils.hpp"
#include "lazy_logs.hpp"
#include "player_utils.hpp"
#include "FFmpegIOCtx.hpp"
#include "FFmpegFormatCtx.hpp"
#include "FFmpegCodecCtx.hpp"

#define DEFAULT_BUFF_SIZE 1024*1024
#define SIZE_TS_PACK 188

enum PlayerStates
{
    STATE_PLAY = 1,
    STATE_STOP = 2,
    STATE_PAUSE = 4,
    STATE_HAVE_DATA = 8,
    STATE_NO_DATA = 16,
};

class Player
{
private:
    std::thread _parser_thread;
    std::thread _decoder_thread;
    std::thread _setup_thread;

    std::mutex _parser_mutex;
    std::mutex _decoder_mutex;
    std::mutex _data_mutex;

    semaphore _setup_sem;
    semaphore _start_sem;
    semaphore _queue_sem;
    semaphore _append_sem;

    int _state;
    unsigned int _video_stream_idx;

    FFmpegFormatCtx::Ptr _format_ctx;
    FFmpegIOCtx::Ptr _avio_ctx;
    FFmpegCodecCtx::Ptr _codec_ctx;

    std::stringstream _data;

    std::queue<FFmpegPacket::Ptr> _queue_packs;

    uint16_t _append_buff_size_update_counter;
    size_t _append_buff_size;
    char *_append_buff;

    void decode_queue();
    void drop_queue();
    bool grab_frames();
    int setup_input();

public:
    void AppendData(const char *new_data, size_t new_data_size);
    Player();
    ~Player();
    void Play();
    void Pause();
    void Stop();
};

#endif