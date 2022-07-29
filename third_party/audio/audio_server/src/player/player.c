/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

#define PLAYBACK_FRAMESIZE      (4096)  /* bytes */
#define DEVICE_IDLE_TIMEOUT     (2)     /* seconds */
#define SCALE_TABLE_SIZE        (4096)
#define MAX_HEADER_LEN          (1024)

play_decoder_t *g_default_decoder = NULL;
play_decoder_cfg_t *decoder_cfg = NULL;
play_preprocessor_cfg_t *processor_cfg = NULL;

struct player
{
    struct audio_player_queue *preprocess_queue;
    struct audio_player_queue *decode_queue;
    struct audio_player_queue *play_queue;

    struct audio_player_stream *preprocess_stream;
    struct audio_player_stream *decode_stream;

    audio_player_mutex_handle play_lock;
    audio_player_mutex_handle state_lock;
    audio_player_semaphore_handle pause_sem;
    audio_player_semaphore_handle stop_sem;

    audio_player_thread_handle preprocess_task;
    audio_player_thread_handle decode_task;
    audio_player_thread_handle play_task;

    int preprocess_task_state;
    int decode_task_state;
    int play_task_state;

    player_state_t state;

    const char *tag;

    player_listen_cb listen;
    void *userdata;

    const char *name;
    playback_device_t device;
    play_preprocessor_t preprocessor;

    char *target;
    int samplerate;
    int bits;
    int channels;
    uint32_t start_time;
    uint32_t cur_time;
    uint32_t total_time;
    uint32_t total_length;

    int resample_rate;
    int playback;
    int playback_start;
    int need_free;
};

int player_init(void)
{
#ifdef AUDIO_DECODER_MP3
    play_decoder_t mp3_decoder = DEFAULT_MP3_DECODER;
    player_register_decoder("mp3", &mp3_decoder);
#endif

#ifdef AUDIO_DECODER_AMR
    play_decoder_t amr_decoder = DEFAULT_AMR_DECODER;
    player_register_decoder("amr", &amr_decoder);
#endif

    play_decoder_t wav_decoder = DEFAULT_WAV_DECODER;
    player_register_decoder("wav", &wav_decoder);
    play_decoder_t pcm_decoder = DEFAULT_PCM_DECODER;
    player_register_decoder("pcm", &pcm_decoder);

    return 0;
}

int player_list_decoder(void)
{
    if (g_default_decoder == NULL)
    {
        RK_AUDIO_LOG_W("No decoder");
        return RK_AUDIO_FAILURE;
    }

    play_decoder_t *p = g_default_decoder;
    int i = 0;
    while (p)
    {
        RK_AUDIO_LOG_V("[Decoder %d] type:[%s]", i, p->type);
        p = p->next;
        i++;
    }

    return RK_AUDIO_SUCCESS;
}

int player_register_decoder(const char *type, play_decoder_t *decoder)
{
    play_decoder_t *dec;
    play_decoder_t *p = g_default_decoder;

    while (p)
    {
        if (!strcmp(type, p->type))
        {
            RK_AUDIO_LOG_V("Decoder [%s] already exist", type);
            return RK_AUDIO_SUCCESS;
        }
        p = p->next;
    }

    dec = audio_malloc(sizeof(play_decoder_t));
    if (!dec)
    {
        RK_AUDIO_LOG_E("malloc %d failed", sizeof(play_decoder_t));
        return RK_AUDIO_FAILURE;
    }
    dec->type = audio_malloc(strlen(type) + 1);
    if (!dec->type)
    {
        RK_AUDIO_LOG_E("malloc %d failed", strlen(type));
        audio_free(dec);
        return RK_AUDIO_FAILURE;
    }
    memset(dec->type, 0x0, strlen(type) + 1);
    memcpy(dec->type, type, strlen(type));
    dec->check = decoder->check;
    dec->init = decoder->init;
    dec->process = decoder->process;
    dec->get_post_state = decoder->get_post_state;
    dec->destroy = decoder->destroy;
    dec->support_seek = decoder->support_seek;
    dec->next = NULL;
    dec->userdata = NULL;
    if (g_default_decoder == NULL)
    {
        g_default_decoder = dec;
    }
    else
    {
        p = g_default_decoder;
        while (p->next)
            p = p->next;
        p->next = dec;
    }

    return RK_AUDIO_SUCCESS;
}

int player_unregister_decoder(const char *type)
{
    play_decoder_t *target = NULL;
    play_decoder_t *p = g_default_decoder;

    if (!strcmp(type, p->type))
    {
        g_default_decoder = p->next;
        audio_free(p->type);
        audio_free(p);
        return RK_AUDIO_SUCCESS;
    }
    while (p->next)
    {
        if (!strcmp(type, p->next->type))
        {
            target = p->next;
            break;
        }
        p = p->next;
    }
    if (target)
    {
        p->next = target->next;
        audio_free(target->type);
        audio_free(target);
        return RK_AUDIO_SUCCESS;
    }
    else
    {
        RK_AUDIO_LOG_V("No this decoder [%s]", type);
        return RK_AUDIO_FAILURE;
    }
}

void *preprocess_run(void *data)
{
    player_handle_t player = (player_handle_t) data;
    media_sdk_msg_t msg;
    media_sdk_msg_t decode_msg;
    play_preprocessor_t preprocessor;
    int res;
    char *read_buf;
    char *header_buf;
    size_t read_size = 0;
    size_t frame_size = 0;
    if (!processor_cfg)
    {
        processor_cfg = (play_preprocessor_cfg_t *)audio_malloc(sizeof(*processor_cfg));
        RK_AUDIO_LOG_D("malloc processor_cfg...");
    }

__PREPROCESS_ERROR:
    while (1)
    {
        player->preprocess_task_state = AUDIO_TASK_IDLE;
        if (audio_queue_receive(player->preprocess_queue, &msg) == -1)
        {
            RK_AUDIO_LOG_E("can't get preprocess msg");
            goto  __PREPROCESS_ERROR;
        }
        player->preprocess_task_state = AUDIO_TASK_BUSY;

        preprocessor = player->preprocessor;
        RK_AUDIO_LOG_D("preprocessor.type:%s", preprocessor.type);
        if (!preprocessor.type)
        {
            preprocessor.type = "unknown";
            RK_AUDIO_LOG_E("can't get preprocessor.type");
            goto  __PREPROCESS_ERROR;
        }

        processor_cfg->target = player->target;
        processor_cfg->tag = player->tag;
        processor_cfg->isOta = RK_AUDIO_FALSE;
        processor_cfg->seek_pos = 0;
        if (player->start_time && preprocessor.seek == NULL)
            player->start_time = 0;

        res = preprocessor.init(&preprocessor, processor_cfg);
        if (res)
        {
            RK_AUDIO_LOG_E("init fail res:%d", res);
            player->state = PLAYER_STATE_ERROR;
            if (player->listen)
            {
                player->listen(player, PLAY_INFO_PREPROCESS, player->userdata);
            }
            goto  __PREPROCESS_ERROR;
        }

        player->total_length = processor_cfg->file_size;
        frame_size = processor_cfg->frame_size;
        read_buf = audio_malloc(frame_size);
        if (!read_buf)
        {
            RK_AUDIO_LOG_E("can't audio_malloc buffer %d", frame_size);
            player->state = PLAYER_STATE_ERROR;
            if (player->listen)
            {
                player->listen(player, PLAY_INFO_PREPROCESS, player->userdata);
            }
            goto PREPROCESS_EXIT_2;
        }
        header_buf = audio_malloc(MAX_HEADER_LEN);
        if (!header_buf)
        {
            RK_AUDIO_LOG_E("can't audio_malloc buffer %d", MAX_HEADER_LEN);
            player->state = PLAYER_STATE_ERROR;
            if (player->listen)
            {
                player->listen(player, PLAY_INFO_PREPROCESS, player->userdata);
            }
            goto PREPROCESS_EXIT_1;
        }

        decode_msg.type = msg.type;
        decode_msg.player.mode = msg.player.mode;
        decode_msg.player.target = msg.player.target;
        decode_msg.player.need_free = msg.player.need_free;
        decode_msg.player.end_session = msg.player.end_session;
        decode_msg.player.type = processor_cfg->type;
        decode_msg.player.priv_data = header_buf;

        RK_AUDIO_LOG_D("send msg to decode");
        if (msg.type == CMD_PLAYER_PLAY)
        {
            read_size = preprocessor.read(&preprocessor, header_buf, MAX_HEADER_LEN);
            audio_stream_start(player->preprocess_stream);
            audio_queue_send(player->decode_queue, &decode_msg);
            if (read_size > 0)
                audio_stream_write(player->preprocess_stream, header_buf, read_size);
        }
        else if (msg.type == CMD_PLAYER_SEEK)
        {
            audio_stream_resume(player->preprocess_stream);
        }
        int timeout = 0;
        do
        {
retry:
            if (!audio_queue_is_empty(player->preprocess_queue))
            {
                if (audio_queue_receive(player->preprocess_queue, &msg) == -1)
                {
                    RK_AUDIO_LOG_E("can't get preprocess msg");
                    audio_stream_stop(player->preprocess_stream);
                    audio_free(header_buf);
                    audio_free(read_buf);
                    preprocessor.destroy(&preprocessor);
                    goto  __PREPROCESS_ERROR;
                }
                if (msg.type == CMD_PLAYER_SEEK)
                {
                    processor_cfg->seek_pos = msg.player.seek_pos;
                    preprocessor.seek(&preprocessor, processor_cfg);
                    audio_stream_resume(player->preprocess_stream);
                }
            }
            read_size = preprocessor.read(&preprocessor, read_buf, frame_size);
            if (read_size == -101)
            {
                if ((player->state == PLAYER_STATE_IDLE) || (player->state == PLAYER_STATE_ERROR))
                {
                    RK_AUDIO_LOG_V("Player finish");
                    audio_stream_finish(player->preprocess_stream);
                    break;
                }
                timeout++;
                if (timeout > 20)
                {
                    //more than 10s exit
                    RK_AUDIO_LOG_E("Read timeout");
                    audio_stream_stop(player->preprocess_stream);
                    break;
                }
                RK_AUDIO_LOG_D("player state = %d", player->state);
                goto retry;
            }
            else if (read_size <= 0)
            {
                RK_AUDIO_LOG_V("read_size = %d, break", read_size);
                audio_stream_finish(player->preprocess_stream);
                break;
            }
            timeout = 0;
        }
        while (audio_stream_write(player->preprocess_stream, read_buf, read_size) != -1);
        audio_free(header_buf);
PREPROCESS_EXIT_1:
        audio_free(read_buf);
PREPROCESS_EXIT_2:
        preprocessor.destroy(&preprocessor);
        RK_AUDIO_LOG_V("out");
    }
}

static SRCState *g_pSRC = NULL;
static int g_is_need_resample = 0;
static char *g_resample_buffer;
#define MAX_RESAMPLE_BUFFER_SIZE (7168)

int decoder_input(void *userdata, char *data, size_t data_len)
{
    player_handle_t player = (player_handle_t) userdata;
    return audio_stream_read(player->preprocess_stream, data, data_len);
}

void player_audio_resample_init(int samplerate, int resample_rate)
{
    g_resample_buffer = audio_malloc(2 * MAX_RESAMPLE_BUFFER_SIZE * sizeof(char));
    g_pSRC = (SRCState *)audio_malloc(sizeof(SRCState));
    if (g_pSRC != NULL)
    {
        SRCInit(g_pSRC, samplerate, resample_rate);
    }
}

static short rs_input_buffer[51 * 4 + 128];
int player_audio_resample(player_handle_t player, char *data, size_t data_len)
{
    int resample_out_len = 0;
    if ((g_resample_buffer == NULL) || (g_pSRC == NULL))
    {
        return 0;
    }
    for (int i = 0; i < data_len / 128 / 2; i++)
    {
        memset((char *)rs_input_buffer, 0, 2 * 128);
        memcpy((char *)&rs_input_buffer[51 * 2], &data[i * 128 * 2], 128 * 2);
        resample_out_len += SRCFilter(g_pSRC, (short *)&rs_input_buffer[51 * 2], (short *)&g_resample_buffer[resample_out_len * 2], 128);
    }
    resample_out_len *= 2;
    int ret = audio_stream_write(player->decode_stream, g_resample_buffer, resample_out_len);
    return ret;
}

int decoder_output(void *userdata, char *data, size_t data_len)
{
    player_handle_t player = (player_handle_t) userdata;
    media_sdk_msg_t msg;
    int i = 0;
    int j = 0;
    int frame_bytes = player->bits / 8;
    int isMono = (player->channels == 1) ? 1 : 0;
    char *data_stero;
    int ret;

    if (player->playback_start == 0)
    {
        msg.type = CMD_PLAYER_PLAY;
        msg.player.mode = PLAY_MODE_PROMPT;
        msg.player.target = NULL;
        msg.player.need_free = false;
        msg.player.end_session = false;
        audio_queue_send(player->play_queue, &msg);
        player->playback_start = 1;
        RK_AUDIO_LOG_V("Playback run");
    }
    /* if MONO, copy left channel data stream to right channels. */
    if (isMono)
    {
        j = data_len - frame_bytes;
        data_len *= 2;
        data_stero = audio_malloc(data_len);
        for (i = data_len - 2 * frame_bytes; i >= 0; i = i - frame_bytes * 2)
        {
            memcpy(data_stero + i, data + j, frame_bytes);
            memcpy(data_stero + i + frame_bytes, data + j, frame_bytes);
            j -= frame_bytes;
        }
    }
    else
    {
        data_stero = data;
    }

    if (g_is_need_resample)
        ret = player_audio_resample(player, data_stero, data_len);
    else
        ret = audio_stream_write(player->decode_stream, data_stero, data_len);

    if (isMono)
        audio_free(data_stero);

    return ret;
}

void player_audio_resample_deinit(void)
{
    RK_AUDIO_LOG_D("player_audio_resample_deinit\n");
    if (g_resample_buffer)
        audio_free(g_resample_buffer);
    if (g_pSRC)
    {
        audio_free(g_pSRC);
    }
}

uint32_t player_get_target_and_seek(player_handle_t self, char *file_name)
{
    if (self->target && file_name)
    {
        memcpy(file_name, self->target, strlen(self->target) + 1);
    }

    return player_get_cur_time(self);
}

void player_set_seek(long offset)
{
    /* Set cfg->start_time when call player_play */
}

int decoder_post(void *userdata, int samplerate, int bits, int channels)
{
    player_handle_t player = (player_handle_t)userdata;
    player->samplerate = samplerate;
    player->bits = bits;
    player->channels = channels;

    if (player->resample_rate && samplerate != player->resample_rate)
    {
        RK_AUDIO_LOG_V("%d != %d, need resample", samplerate, player->resample_rate);
        player_audio_resample_init(samplerate, player->resample_rate);
        memset((char *)rs_input_buffer, 0, 2 * (51 * 4 + 128));
        player->samplerate = player->resample_rate;
        g_is_need_resample = 1;
    }
    else
    {
        g_is_need_resample = 0;
    }

    return RK_AUDIO_SUCCESS;
}

void *decoder_run(void *data)
{
    player_handle_t player = (player_handle_t) data;
    //play_decoder_cfg_t* decoder_cfg = (play_decoder_cfg_t*)audio_calloc(1,sizeof(*decoder_cfg));
    play_decoder_t decoder;
    bool is_found_decoder = false;
    play_decoder_error_t decode_res;
    media_sdk_msg_t decode_msg;
    if (!decoder_cfg)
    {
        decoder_cfg = (play_decoder_cfg_t *)audio_calloc(1, sizeof(*decoder_cfg));
        decoder_cfg->input = decoder_input;
        decoder_cfg->output = decoder_output;
        decoder_cfg->post = decoder_post;
        decoder_cfg->userdata = player;
    }

    while (1)
    {
        player->decode_task_state = AUDIO_TASK_IDLE;
        if (audio_queue_receive(player->decode_queue, &decode_msg) == -1)
        {
            RK_AUDIO_LOG_E("can't get msg");
            return NULL;
        }
        player->decode_task_state = AUDIO_TASK_BUSY;
        play_decoder_t *p = g_default_decoder;
        is_found_decoder = false;
        /* Find decoder by header info */
        while (p)
        {
            if (p->check && p->check(decode_msg.player.priv_data, MAX_HEADER_LEN) == 0)
            {
                decoder = *p;
                is_found_decoder = true;
                RK_AUDIO_LOG_V("Found decoder %s.", p->type);
                break;
            }
            p = p->next;
        }

        /* Find decoder by file extension */
        if (is_found_decoder == false)
        {
            play_decoder_t *p = g_default_decoder;
            while (is_found_decoder == false && p)
            {
                if (!strcasecmp(decode_msg.player.type, p->type))
                {
                    decoder = *p;
                    is_found_decoder = true;
                    break;
                }
                p = p->next;
            }
        }

        if (!is_found_decoder)
        {
            RK_AUDIO_LOG_E("can't found decoder");
            audio_stream_stop(player->preprocess_stream);
            audio_mutex_lock(player->state_lock);
            player->state = PLAYER_STATE_ERROR;
            if (player->listen)
                player->listen(player, PLAY_INFO_DECODE, player->userdata);
            audio_mutex_unlock(player->state_lock);
        }
        else
        {
            if (player->start_time && !decoder.support_seek)
                player->start_time = 0;
            RK_AUDIO_LOG_D("decode init.");
#ifdef CONFIG_FWANALYSIS_SEGMENT
            FW_LoadSegment(decoder.segment, SEGMENT_OVERLAY_ALL);
#endif
            decoder_cfg->start_time = player->start_time;
            if (decoder.init(&decoder, decoder_cfg))
            {
                RK_AUDIO_LOG_E("decoder init fail");
                audio_stream_stop(player->preprocess_stream);
                audio_mutex_lock(player->state_lock);
                player->state = PLAYER_STATE_ERROR;
                if (player->listen)
                    player->listen(player, PLAY_INFO_DECODE, player->userdata);
                audio_mutex_unlock(player->state_lock);
                continue;
            }
            audio_stream_start(player->decode_stream);
            decode_res = decoder.process(&decoder);
            switch (decode_res)
            {

            case PLAY_DECODER_INPUT_ERROR:
            case PLAY_DECODER_OUTPUT_ERROR:
            case PLAY_DECODER_DECODE_ERROR:
                RK_AUDIO_LOG_E("decode res %d", decode_res);
#ifdef CONFIG_FWANALYSIS_SEGMENT
                FW_RemoveSegment(decoder.segment);
#endif
                audio_stream_stop(player->decode_stream);
                audio_stream_stop(player->preprocess_stream);
                if (!decoder.get_post_state(&decoder) || player->playback_start == 0)
                {
                    audio_mutex_lock(player->state_lock);
                    player->state = PLAYER_STATE_ERROR;
                    if (player->listen)
                    {
                        player->listen(player, PLAY_INFO_DECODE, player->userdata);
                    }
                    audio_mutex_unlock(player->state_lock);
                }
                break;

            default:
                RK_AUDIO_LOG_D("audio_stream_finish");
#ifdef CONFIG_FWANALYSIS_SEGMENT
                FW_RemoveSegment(decoder.segment);
#endif
                audio_stream_stop(player->preprocess_stream);
                audio_stream_finish(player->decode_stream);
                if (!decoder.get_post_state(&decoder) || player->playback_start == 0)
                {
                    audio_mutex_lock(player->state_lock);
                    player->state = PLAYER_STATE_ERROR;
                    if (player->listen)
                    {
                        player->listen(player, PLAY_INFO_IDLE, player->userdata);
                    }
                    audio_mutex_unlock(player->state_lock);
                }
                break;
            }
#if 0
            if (!decoder.get_post_state(&decoder))
            {
                audio_mutex_lock(player->state_lock);
                player->state = PLAYER_STATE_IDLE;
                if (player->listen)
                {
                    player->listen(player, PLAY_INFO_IDLE, player->userdata);
                }
                audio_mutex_unlock(player->state_lock);
            }
#endif
            RK_AUDIO_LOG_D("decoder process return value:%d\n", decode_res);
            decoder.destroy(&decoder);
            if (g_is_need_resample)
            {
                g_is_need_resample = 0;
                player_audio_resample_deinit();
            }
        }
    }
}

void *playback_run(void *data)
{
    player_handle_t player = (player_handle_t) data;
    playback_device_t device = player->device;
    playback_device_cfg_t device_cfg;
    media_sdk_msg_t msg;
    play_info_t info;
    char *read_buf = NULL;
    char *mute_buf = NULL;
    int read_size;
    size_t frame_size;
    static size_t oddframe = 0;
    int pcm_start_ret = 0;
    int byte2ms = 0;
    uint64_t total_byte;
    uint32_t idle_time_out;
    int32_t fade_in_samples = 0;
    int first_frame;
    while (1)
    {
        player->play_task_state = AUDIO_TASK_IDLE;
        if (audio_queue_receive(player->play_queue, &msg) == -1)
        {
            RK_AUDIO_LOG_E("receive data failed");
            return NULL;
        }
        player->play_task_state = AUDIO_TASK_BUSY;
        if (msg.player.end_session == true)
        {
            if (msg.player.need_free == true)
                audio_semaphore_give(player->stop_sem);
            continue;
        }
        device_cfg.samplerate = player->samplerate;
        device_cfg.bits = player->bits;
        device_cfg.channels = player->channels;
        device_cfg.card_name = player->name;
        device_cfg.frame_size = PLAYBACK_FRAMESIZE;
        if (!player->playback)
        {
            RK_AUDIO_LOG_D("No playback, break");
            player->state = PLAYER_STATE_RUNNING;
            continue;
        }
        RK_AUDIO_LOG_V("start");
        if (device.open(&device, &device_cfg))
        {
            audio_stream_stop(player->preprocess_stream);
            audio_stream_stop(player->decode_stream);
            audio_mutex_lock(player->state_lock);
            player->state = PLAYER_STATE_ERROR;
            if (player->listen)
            {
                player->listen(player, PLAY_INFO_STOP, player->userdata);
            }
            audio_mutex_unlock(player->state_lock);
            RK_AUDIO_LOG_E("device open fail");
            continue;
        }
        else
        {
            frame_size = device_cfg.frame_size;
            pcm_start_ret = device.start(&device);
            if (pcm_start_ret != 0)
            {
                RK_AUDIO_LOG_E("pcm device start fail.");
                audio_stream_stop(player->preprocess_stream);
                audio_stream_stop(player->decode_stream);
                audio_mutex_lock(player->state_lock);
                player->state = PLAYER_STATE_ERROR;
                if (player->listen)
                {
                    player->listen(player, PLAY_INFO_STOP, player->userdata);
                }
                audio_mutex_unlock(player->state_lock);
                continue;
            }
            read_buf = audio_malloc(frame_size * 2);
            if (!read_buf)
            {
                RK_AUDIO_LOG_E("create read buf fail");
                audio_stream_stop(player->preprocess_stream);
                audio_stream_stop(player->decode_stream);
                audio_mutex_lock(player->state_lock);
                player->state = PLAYER_STATE_ERROR;
                if (player->listen)
                {
                    player->listen(player, PLAY_INFO_STOP, player->userdata);
                }
                audio_mutex_unlock(player->state_lock);
                device.stop(&device);
                device.close(&device);
                continue;
            }
PLAYBACK_START:
            memset(read_buf, 0, frame_size * 2);
            /* byte2ms = channels * (bits >> 3) * (rate / 1000), channels will always be 2 */
            byte2ms = 2 * (player->bits >> 3) * player->samplerate / 1000;
            /* If decoder seek not accurate, this may not accurate */
            if (player->start_time)
            {
                total_byte = player->start_time * 1000 * byte2ms;
                player->cur_time = player->start_time * 1000;
            }
            else
            {
                total_byte = 0;
                player->cur_time = 0;
            }
            idle_time_out =
                device_cfg.samplerate * DEVICE_IDLE_TIMEOUT *
                (device_cfg.bits >> 3) * device_cfg.channels / device_cfg.frame_size;
            player->state = PLAYER_STATE_RUNNING;
            first_frame = 1;
            info = PLAY_INFO_IDLE;
            while (1)
            {
                //OS_LOG_D(player,"playback_run:read frame_size:%d",frame_size);
                memset(read_buf + oddframe, 0x0, frame_size);
                read_size = audio_stream_read(player->decode_stream, read_buf + oddframe, frame_size);
                if (read_size == -1)
                {
                    RK_AUDIO_LOG_E("Force stop");
                    info = PLAY_INFO_STOP;
                    /* Fade out */
                    short *data = (short *)(read_buf + oddframe);
                    short *last = (short *)((oddframe == 0) ? (read_buf + frame_size) : read_buf);
                    short last_data_l = *(last + frame_size / 2 - 2);
                    short last_data_r = *(last + frame_size / 2 - 1);
                    fade_in_samples = SCALE_TABLE_SIZE;
                    while (fade_in_samples > 0)
                    {
                        uint32_t len =
                            (frame_size / 2 / sizeof(short)) < SCALE_TABLE_SIZE ?
                            (frame_size / 2 / sizeof(short)) : SCALE_TABLE_SIZE;
                        for (int i = 0; i < len; i++)
                        {
                            float s = (float)(fade_in_samples - i) / SCALE_TABLE_SIZE;
                            s *= s;
                            data[i * 2] = (short)((float)last_data_l * s);
                            data[i * 2 + 1] = (short)((float)last_data_r * s);
                        }
                        device.write(&device, (char *)data, frame_size);
                        memset((char *)data, 0, frame_size);
                        fade_in_samples -= len;
                    }
                    break;
                }
                if (read_size == 0)
                {
                    RK_AUDIO_LOG_V("Finish");
                    audio_stream_stop(player->decode_stream);
                    break;
                }
                if (first_frame == 1)
                {
                    RK_AUDIO_LOG_V("Fade in");
                    first_frame = 0;
                    fade_in_samples = SCALE_TABLE_SIZE;
                }
                if (fade_in_samples > 0)
                {
                    /* Fade in */
                    short *data = (short *)(read_buf + oddframe);
                    uint32_t len =
                        (frame_size / 2 / sizeof(short)) < fade_in_samples ?
                        (frame_size / 2 / sizeof(short)) : fade_in_samples;
                    for (int i = 0; i < len; i++)
                    {
                        float s = (float)(i + SCALE_TABLE_SIZE - fade_in_samples + 1) / SCALE_TABLE_SIZE;
                        s *= s;
                        data[i * 2] *= s;
                        data[i * 2 + 1] *= s;
                    }
                    fade_in_samples -= len;
                }
                switch (player->state)
                {
                case PLAYER_STATE_PAUSED:
                {
                    RK_AUDIO_LOG_D("PLAYER_STATE_PAUSED.");
                    RK_AUDIO_LOG_D("play pause");
                    mute_buf = (oddframe == 0) ? (read_buf + frame_size) : read_buf;
                    memset(mute_buf, 0x0, frame_size);
                    while (audio_semaphore_try_take(player->pause_sem))
                        device.write(&device, mute_buf, frame_size);
                    if (player->state == PLAYER_STATE_STOP)
                    {
                        RK_AUDIO_LOG_D("play resume->stop");
                        info = PLAY_INFO_STOP;
                        goto PLAYBACK_STOP;
                    }
                    RK_AUDIO_LOG_D("play resume");
                    /* fall through */
                }
                case PLAYER_STATE_RUNNING:
                {
                    total_byte += read_size;
                    player->cur_time = total_byte / byte2ms;
                    device.write(&device, read_buf + oddframe, read_size);
                    break;
                }
                default:
                {
                    break;
                }
                }
                if (read_size < frame_size)
                {
                    RK_AUDIO_LOG_D("underrun.");
                    break;
                }
                oddframe = (oddframe == 0) ? frame_size : 0;
            }
PLAYBACK_STOP:
            audio_mutex_lock(player->state_lock);
            player->state = PLAYER_STATE_IDLE;
            if (player->listen)
            {
                player->listen(player, info, player->userdata);
            }
            audio_mutex_unlock(player->state_lock);
            mute_buf = (oddframe == 0) ? (read_buf + frame_size) : read_buf;
            memset(mute_buf, 0x0, frame_size);
            while (idle_time_out--)
            {
                device.write(&device, mute_buf, frame_size);
                if (audio_queue_is_empty(player->play_queue) != true)
                {
                    audio_queue_receive(player->play_queue, &msg);
                    if (msg.player.end_session == true)
                    {
                        if (msg.player.need_free == true)
                            audio_semaphore_give(player->stop_sem);
                        idle_time_out = 0;
                        break;
                    }
                    else
                    {
                        goto PLAYBACK_START;
                    }
                }
            }
            device.stop(&device);
            device.close(&device);
            if (read_buf)
            {
                RK_AUDIO_LOG_D("free read_buf");
                audio_free(read_buf);
                read_buf = NULL;
            }
            RK_AUDIO_LOG_V("stop");
        }
    }
}

player_handle_t player_create(player_cfg_t *cfg)
{
    player_handle_t player = (player_handle_t) audio_calloc(1, sizeof(*player));
    uint32_t preprocess_stack_size;
    uint32_t decoder_stack_size;
    uint32_t playback_stack_size;

    RK_AUDIO_LOG_D("in");
    if (player)
    {
        memset((void *)player, 0x0, sizeof(player_handle_t));
        player->preprocess_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        player->decode_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        player->play_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        player->preprocess_stream = audio_stream_create(cfg->preprocess_buf_size);
        player->decode_stream = audio_stream_create(cfg->decode_buf_size);
        player->state_lock = audio_mutex_create();
        player->play_lock = audio_mutex_create();
        player->pause_sem = audio_semaphore_create();
        player->stop_sem = audio_semaphore_create();
        player->tag = cfg->tag;
        player->listen = cfg->listen;
        player->userdata = cfg->userdata;
        player->name = cfg->name;
        player->device = cfg->device;
        player->resample_rate = cfg->resample_rate ? cfg->resample_rate : 48000;
        player->state = PLAYER_STATE_IDLE;

        preprocess_stack_size = cfg->preprocess_stack_size ? cfg->preprocess_stack_size : 4096;
        decoder_stack_size = cfg->decoder_stack_size ? cfg->decoder_stack_size : 1024 * 12;
        playback_stack_size = cfg->playback_stack_size ? cfg->playback_stack_size : 2048;
        audio_thread_cfg_t c =
        {
            .run = (void *)preprocess_run,
            .args = player
        };
        player->preprocess_task = audio_thread_create("preprocess_task",
                                                      preprocess_stack_size,
                                                      PLAYER_TASK_PRIORITY, &c);
        c.run = (void *)decoder_run;
        c.args = player;
        player->decode_task = audio_thread_create("decode_task",
                                                  decoder_stack_size,
                                                  PLAYER_TASK_PRIORITY, &c);
        c.run = (void *)playback_run;
        c.args = player;
        player->play_task = audio_thread_create("play_task",
                                                playback_stack_size,
                                                PLAYER_TASK_PRIORITY, &c);
        RK_AUDIO_LOG_V("Success %s 0x%lx 0x%lx 0x%lx",
                       player->name,
                       preprocess_stack_size,
                       decoder_stack_size,
                       playback_stack_size);
    }
    else
    {
        RK_AUDIO_LOG_E("Failure");
    }

    return player;
}

int player_play(player_handle_t self, play_cfg_t *cfg)
{
    player_handle_t player = self;
    media_sdk_msg_t msg ;
    int time_out = 1000;

    audio_mutex_lock(player->play_lock);
    if (player->state == PLAYER_STATE_RUNNING)
        player_stop(player);

    player->preprocessor = cfg->preprocessor;
    player->samplerate = cfg->samplerate;
    player->bits = cfg->bits;
    player->channels = cfg->channels;
    player->start_time = cfg->start_time;
    player->cur_time = 0;
    player->total_time = 0;
    player->playback = cfg->info_only ? 0 : 1;
    player->playback_start = 0;
    msg.type = CMD_PLAYER_PLAY;
    msg.player.mode = PLAY_MODE_PROMPT;
    msg.player.need_free = cfg->need_free;
    msg.player.end_session = false;

    if (player->need_free && player->target != NULL)
    {
        audio_free(player->target);
        player->target = NULL;
    }
    player->need_free = cfg->need_free;
    if (cfg->need_free)
    {
        player->target = audio_malloc(strlen(cfg->target) + 1);
        if (player->target == NULL)
        {
            RK_AUDIO_LOG_E("no mem!");
            return RK_AUDIO_FAILURE;
        }
        memset(player->target, 0x0, strlen(cfg->target) + 1);
        memcpy(player->target, cfg->target, strlen(cfg->target));
    }
    else
    {
        player->target = cfg->target;
    }
    RK_AUDIO_LOG_V("Target [%s]", player->target);

    audio_queue_send(player->preprocess_queue, &msg);
    while ((self->state != PLAYER_STATE_RUNNING) && (self->state != PLAYER_STATE_ERROR))
    {
        audio_sleep(10);
        time_out--;
        if (!time_out)
            break;
    }
    if (self->state == PLAYER_STATE_RUNNING && self->playback == 0)
    {
        audio_stream_resume(player->preprocess_stream);
        player_stop(player);
        goto PLAYER_PLAY_OUT;
    }
PLAYER_PLAY_OUT:
    audio_mutex_unlock(player->play_lock);

    return RK_AUDIO_SUCCESS;
}

int player_preprocess_seek(player_handle_t self, uint32_t pos)
{
    media_sdk_msg_t msg;

    if (!self || !self->preprocess_queue)
        return RK_AUDIO_FAILURE;

    msg.type = CMD_PLAYER_SEEK;
    msg.player.seek_pos = pos;
    if (audio_queue_send(self->preprocess_queue, &msg) != RK_AUDIO_SUCCESS)
        return RK_AUDIO_FAILURE;
    audio_stream_reset(self->preprocess_stream);

    return RK_AUDIO_SUCCESS;
}

player_state_t player_get_state(player_handle_t self)
{
    return self->state;
}

int player_get_cur_time(player_handle_t self)
{
    uint32_t time = self->cur_time;
    /* After seek, cur time may be not accurate */
    if (self->total_time && self->total_time < time)
        return self->total_time;
    else
        return time;
}

uint32_t player_get_file_length(player_handle_t self)
{
    return self->total_length;
}

void player_set_total_time(player_handle_t self, uint32_t time)
{
    self->total_time = time;
}

uint32_t player_get_total_time(player_handle_t self)
{
    return self->total_time;
}

int player_stop(player_handle_t self)
{
    int result;
    audio_mutex_lock(self->state_lock);
    self->cur_time = 0;
    if (self->state)
    {
        audio_stream_stop(self->preprocess_stream);
        RK_AUDIO_LOG_D("audio_stream_stop preprocess_stream");
        audio_stream_stop(self->decode_stream);
        if (self->state == PLAYER_STATE_PAUSED)
        {
            self->state = PLAYER_STATE_STOP;
            audio_semaphore_give(self->pause_sem);
        }
        audio_mutex_unlock(self->state_lock);
        while (self->playback && (self->state != PLAYER_STATE_IDLE) && (self->state != PLAYER_STATE_ERROR))
        {
            audio_sleep(10);
        }
        self->state = PLAYER_STATE_IDLE;
        result = 0;
        RK_AUDIO_LOG_V("stop player,pause/running state\n");
    }
    else
    {
        self->state = PLAYER_STATE_IDLE;
        audio_mutex_unlock(self->state_lock);
        RK_AUDIO_LOG_V("stop player,idle state\n");
        result = 0;
    }

    return result;
}

int player_device_stop(player_handle_t self, int wait)
{
    audio_mutex_lock(self->play_lock);
    if (self->state == PLAYER_STATE_RUNNING)
        player_stop(self);
    media_sdk_msg_t msg;
    msg.player.end_session = true;
    if (wait)
        msg.player.need_free = true;
    else
        msg.player.need_free = false;
    audio_queue_send(self->play_queue, &msg);
    if (wait)
        audio_semaphore_take(self->stop_sem);
    audio_mutex_unlock(self->play_lock);

    return RK_AUDIO_SUCCESS;
}

int player_pause(player_handle_t self)
{
    audio_mutex_lock(self->state_lock);
    if (self->state == PLAYER_STATE_RUNNING)
    {
        self->state = PLAYER_STATE_PAUSED;
    }
    audio_mutex_unlock(self->state_lock);

    return RK_AUDIO_SUCCESS;
}

int player_resume(player_handle_t self)
{
    audio_mutex_lock(self->state_lock);
    if (self->state == PLAYER_STATE_PAUSED)
    {
        self->state = PLAYER_STATE_RUNNING;
        audio_semaphore_give(self->pause_sem);
    }
    audio_mutex_unlock(self->state_lock);

    return RK_AUDIO_SUCCESS;
}

int player_wait_idle(player_handle_t self)
{
    if (self->listen)
    {
        audio_mutex_lock(self->state_lock);
        self->listen(self, self->state, self->userdata);
        if (self->state == PLAYER_STATE_IDLE)
        {
            RK_AUDIO_LOG_D("idle.....");
            audio_mutex_unlock(self->state_lock);
            return PLAYER_STATE_IDLE;
        }
        audio_mutex_unlock(self->state_lock);
    }
    return RK_AUDIO_SUCCESS;
}

int player_close(player_handle_t self)
{
    return player_device_stop(self, 1);
}

void player_destroy(player_handle_t self)
{
    player_handle_t player = self;
    int time_out = 300;
    RK_AUDIO_LOG_D("player_destory in");
    if (player)
    {
        player_device_stop(self, 1);
        while (player->preprocess_task_state == AUDIO_TASK_BUSY ||
               player->decode_task_state == AUDIO_TASK_BUSY ||
               player->play_task_state == AUDIO_TASK_BUSY)
        {
            audio_sleep(10);
            time_out--;
            if (time_out <= 0)
            {
                RK_AUDIO_LOG_E("Wait player idle timeout");
                return;
            }
        }
        audio_thread_exit(player->preprocess_task);
        audio_thread_exit(player->decode_task);
        audio_thread_exit(player->play_task);
        audio_queue_destroy(player->preprocess_queue);
        audio_queue_destroy(player->decode_queue);
        audio_queue_destroy(player->play_queue);
        audio_stream_destroy(player->preprocess_stream);
        audio_stream_destroy(player->decode_stream);
        audio_mutex_destroy(player->state_lock);
        audio_mutex_destroy(player->play_lock);
        audio_semaphore_destroy(player->pause_sem);
        audio_semaphore_destroy(player->stop_sem);
        if (player->need_free && player->target != NULL)
        {
            audio_free(player->target);
            player->target = NULL;
        }
    }
    if (decoder_cfg)
    {
        audio_free(decoder_cfg);
        decoder_cfg = NULL;
    }
    if (processor_cfg)
    {
        audio_free(processor_cfg);
        processor_cfg = NULL;
    }
    RK_AUDIO_LOG_D("player_destory player free.");
    audio_free(player);
    player = NULL;
}

void player_deinit()
{
    if (g_default_decoder)
    {
        RK_AUDIO_LOG_D("player deinit.");
        play_decoder_t *p = g_default_decoder;
        int i = 0;
        while (p)
        {
            g_default_decoder = p->next;
            RK_AUDIO_LOG_D("free [Decoder %d] type:[%s]", i, p->type);
            audio_free(p->type);
            audio_free(p);
            p = g_default_decoder;
            i++;
        }
        g_default_decoder = NULL;
    }
}
