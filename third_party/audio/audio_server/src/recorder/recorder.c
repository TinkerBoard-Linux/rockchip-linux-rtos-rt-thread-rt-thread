/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

int g_encoder_num;
int g_recordr_freq = 0;
static record_encoder_t *g_default_encoder = NULL;

struct recorder
{
    struct audio_player_queue *write_queue;
    struct audio_player_queue *encode_queue;
    struct audio_player_queue *record_queue;

    struct audio_player_stream *encode_stream;
    struct audio_player_stream *record_stream;

    audio_player_mutex_handle state_lock;
    audio_player_semaphore_handle pause_sem;

    audio_player_thread_handle write_task;
    audio_player_thread_handle encode_task;
    audio_player_thread_handle record_task;

    record_writer_t writer;

    const char *target;
    //os_event_group_handle_t event;

    recorder_state_t state;

    const char *tag;

    recorder_listen_cb listen;
    void *userdata;

    const char *device_name;
    capture_device_t device;

    uint32_t samplerate;
    int bits;
    int channels;
    uint32_t duration;
    uint32_t record_time;

    int header_size;
};

void recoder_freq_init(void)
{
    switch (g_recordr_freq)
    {
    case RECORD_FREQ_LOCAL:
        // rkpm_add_freq_scene(FREQ_LOCALRECODER);
        break;
    default:
        RK_AUDIO_LOG_E("\n==recoder_freq_init: error!!!==\n");
        while (1);
        break;
    }
}

void recoder_freq_deinit(void)
{
    switch (g_recordr_freq)
    {
    case RECORD_FREQ_LOCAL:
        // rkpm_remove_freq_scene(FREQ_LOCALRECODER);
        break;
    default:
        break;
    }
}

int recorder_init(void)
{
#ifdef AUDIO_ENCODER_AMR
    record_encoder_t amr_encoder = DEFAULT_AMR_ENCODER;
    recorder_register_encoder("amr", &amr_encoder);
#endif

    record_encoder_t wav_encoder = DEFAULT_WAV_ENCODER;
    recorder_register_encoder("wav", &wav_encoder);
    record_encoder_t pcm_encoder = DEFAULT_PCM_ENCODER;
    recorder_register_encoder("pcm", &pcm_encoder);

    return RK_AUDIO_SUCCESS;
}

int recorder_list_encoder(void)
{
    if (g_default_encoder == NULL)
    {
        RK_AUDIO_LOG_W("No decoder");
        return RK_AUDIO_FAILURE;
    }

    record_encoder_t *p = g_default_encoder;
    int i = 0;
    while (p)
    {
        RK_AUDIO_LOG_V("[Encoder %d] type:[%s]", i, p->type);
        p = p->next;
        i++;
    }

    return RK_AUDIO_SUCCESS;
}

int recorder_register_encoder(const char *type, record_encoder_t *encoder)
{
    record_encoder_t *enc;
    record_encoder_t *p = g_default_encoder;

    while (p)
    {
        if (!strcmp(type, p->type))
        {
            RK_AUDIO_LOG_V("Encoder [%s] already exist", type);
            return RK_AUDIO_SUCCESS;
        }
        p = p->next;
    }

    enc = audio_malloc(sizeof(record_encoder_t));
    if (!enc)
    {
        RK_AUDIO_LOG_E("malloc failed");
        return RK_AUDIO_FAILURE;
    }
    enc->type = audio_malloc(strlen(type) + 1);
    if (!enc->type)
    {
        RK_AUDIO_LOG_E("malloc %d failed", strlen(type));
        audio_free(enc);
        return RK_AUDIO_FAILURE;
    }
    memset(enc->type, 0x0, strlen(type) + 1);
    memcpy(enc->type, type, strlen(type));
    enc->init = encoder->init;
    enc->process = encoder->process;
    enc->get_post_state = encoder->get_post_state;
    enc->destroy = encoder->destroy;
    enc->next = NULL;
    enc->userdata = NULL;
    if (g_default_encoder == NULL)
    {
        g_default_encoder = enc;
    }
    else
    {
        p = g_default_encoder;
        while (p->next)
            p = p->next;
        p->next = enc;
    }

    return RK_AUDIO_SUCCESS;
}

int recorder_unregister_encoder(const char *type)
{
    record_encoder_t *target = NULL;
    record_encoder_t *p = g_default_encoder;

    if (!strcmp(type, p->type))
    {
        g_default_encoder = p->next;
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
        RK_AUDIO_LOG_V("No this encoder [%s]", type);
        return RK_AUDIO_FAILURE;
    }
}

void write_run(void *data)
{
    recorder_handle_t recorder = (recorder_handle_t) data;
    media_sdk_msg_t msg;
    record_writer_t writer;
    record_writer_cfg_t processor_cfg;
    char *buffer;
    char *header_buf = NULL;
    char *last_buf;
    char *read_buf;
    char *temp_buf = NULL;
    long temp_count = 0;
    long temp_size;
    size_t read_size = 0;
    size_t write_size = 0;
    size_t frame_size = 0;
    int save_to_flash = 0;
    record_info_t info;
    int res;

    RK_AUDIO_LOG_D("in");
    while (1)
    {
        if (audio_queue_receive(recorder->write_queue, &msg) == -1)
        {
            RK_AUDIO_LOG_E("receive error, exit");
            return;
        }
        writer = msg.recorder.writer;
        if (!writer.type)
        {
            writer.type = "unknown";
        }
        if (recorder->header_size)
        {
            header_buf = audio_malloc(recorder->header_size);
            memset(header_buf, 0x0, recorder->header_size);
        }

        processor_cfg.target = msg.recorder.target;
        processor_cfg.tag = recorder->tag;
        /* This is to handle with the case of slow flash writing */
        if ((strncmp(processor_cfg.target, "A:", 2) == 0)
            || (strncmp(processor_cfg.target, "a:", 2) == 0))
        {
            RK_AUDIO_LOG_V("record save to flash");
            save_to_flash = 1;
        }
        else
        {
            save_to_flash = 0;
        }
        RK_AUDIO_LOG_D("writer init begin writer.type:%s", writer.type);
        res = writer.init(&writer, &processor_cfg);
        if (msg.recorder.need_free)
            audio_free(msg.recorder.target);
        if (res)
        {
            RK_AUDIO_LOG_E("writer init err");
            audio_stream_stop(recorder->record_stream);
            audio_stream_stop(recorder->encode_stream);
            audio_mutex_lock(recorder->state_lock);
            recorder->state = RECORDER_STATE_IDLE;
            if (recorder->listen)
            {
                recorder->listen(recorder, RECORD_INFO_WRITER, recorder->userdata);
            }
            audio_mutex_unlock(recorder->state_lock);
            continue;
        }
        frame_size = processor_cfg.frame_size;
        write_size = frame_size;
        buffer = (char *)audio_malloc(frame_size * 2);
        last_buf = NULL;
        read_buf = buffer;
        if (!read_buf)
        {
            RK_AUDIO_LOG_E("can't audio_malloc buffer");
            audio_stream_stop(recorder->record_stream);
            audio_stream_stop(recorder->encode_stream);
            audio_mutex_lock(recorder->state_lock);
            recorder->state = RECORDER_STATE_IDLE;
            if (recorder->listen)
            {
                recorder->listen(recorder, RECORD_INFO_WRITER, recorder->userdata);
            }
            audio_mutex_unlock(recorder->state_lock);
            continue;
        }
        if (save_to_flash)
        {
            temp_size = recorder->samplerate * (recorder->bits >> 3) * recorder->channels * recorder->duration;
            RK_AUDIO_LOG_V("malloc temp_buf %ld.", temp_size);
            temp_count = 0;
            temp_buf = audio_malloc(temp_size);
            if (!temp_buf)
            {
                RK_AUDIO_LOG_E("malloc temp_buf failed %ld.", temp_size);
                audio_free(read_buf);
                audio_stream_stop(recorder->record_stream);
                audio_stream_stop(recorder->encode_stream);
                audio_mutex_lock(recorder->state_lock);
                recorder->state = RECORDER_STATE_IDLE;
                if (recorder->listen)
                {
                    recorder->listen(recorder, RECORD_INFO_WRITER, recorder->userdata);
                }
                audio_mutex_unlock(recorder->state_lock);
                continue;
            }
        }
        info = RECORD_INFO_SUCCESS;
        do
        {
            read_size = audio_stream_read(recorder->encode_stream, read_buf, frame_size);
            // RK_AUDIO_LOG_V("writer read data read_size:%d",read_size);
            if (read_size == 0 || read_size == -1)
            {
                if (recorder->header_size)
                {
                    RK_AUDIO_LOG_D("Stream finish, write head %d", read_size);
                    write_size = frame_size - recorder->header_size;
                    memcpy(header_buf, last_buf + write_size, recorder->header_size);
                }
                writer.write(&writer, last_buf, write_size);
                break;
            }
            if (read_size != frame_size)
            {
                if (recorder->header_size)
                {
                    if (read_size < recorder->header_size)
                    {
                        RK_AUDIO_LOG_D("Stream finish, write head %d", read_size);
                        write_size = frame_size - (recorder->header_size - read_size);
                        memcpy(header_buf, last_buf + write_size, recorder->header_size - read_size);
                        memcpy(header_buf + recorder->header_size - read_size, read_buf, read_size);
                    }
                    else
                    {
                        RK_AUDIO_LOG_D("Stream finish, write head %d", read_size);
                        memcpy(header_buf, read_buf + read_size - recorder->header_size, recorder->header_size);
                    }
                }
                writer.write(&writer, last_buf, write_size);
                if (read_size > recorder->header_size)
                    writer.write(&writer, read_buf, read_size - recorder->header_size);
                break;
            }
            if (last_buf)
            {
                if (save_to_flash)
                {
                    if (temp_count < (temp_size - frame_size))
                    {
                        memcpy(temp_buf + temp_count, last_buf, frame_size);
                        temp_count += read_size;
                    }
                }
                else
                {
                    if (writer.write(&writer, last_buf, frame_size) == -1)
                    {
                        audio_stream_stop(recorder->record_stream);
                        audio_stream_stop(recorder->encode_stream);
                        info = RECORD_INFO_WRITER;
                        break;
                    }
                }
            }
            last_buf = read_buf;
            read_buf = read_buf == buffer ? (buffer + frame_size) : buffer;
        }
        while (1);
        if (save_to_flash)
        {
            writer.write(&writer, temp_buf, temp_count);
            audio_free(temp_buf);
        }
        if (recorder->header_size)
        {
            writer.userdata = (void *)header_buf;
            writer.write(&writer, header_buf, recorder->header_size);
            audio_free(header_buf);
            header_buf = NULL;
            recorder->header_size = 0;
        }

        audio_mutex_lock(recorder->state_lock);
        recorder->state = RECORDER_STATE_IDLE;
        if (recorder->listen)
        {
            recorder->listen(recorder, info, recorder->userdata);
        }
        audio_mutex_unlock(recorder->state_lock);
        RK_AUDIO_LOG_D("write_stream was stopped");
        audio_free(buffer);
        writer.destroy(&writer);
    }
}

int encoder_input(void *userdata, char *data, size_t data_len)
{
    recorder_handle_t recorder = (recorder_handle_t) userdata;
    return audio_stream_read(recorder->record_stream, data, data_len);
}

int encoder_output(void *userdata, const char *data, size_t data_len)
{
    recorder_handle_t recorder = (recorder_handle_t) userdata;
    return audio_stream_write(recorder->encode_stream, data, data_len);
}

int encoder_post(void *userdata, int samplerate, int bits, int channels)
{
    // only decoder need.
    return RK_AUDIO_SUCCESS;
}

void encoder_run(void *data)
{
    recorder_handle_t recorder = (recorder_handle_t) data;
    record_encoder_cfg_t encoder_cfg;
    char *audio_type;
    record_encoder_t encoder;
    bool is_found_encoder = false;
    media_sdk_msg_t encode_msg;
    media_sdk_msg_t msg_to_writer;
    struct audio_config audio_cfg;
    while (1)
    {
        if (audio_queue_receive(recorder->encode_queue, &encode_msg) == -1)
        {
            RK_AUDIO_LOG_E("encode_run can't get msg");
            return;
        }
        audio_type = encode_msg.recorder.type;
        RK_AUDIO_LOG_V("encode_run,get audio_type:%s", audio_type);
        record_encoder_t *p = g_default_encoder;
        is_found_encoder = false;
        while (p)
        {
            if (!strcasecmp(audio_type, p->type))
            {
                encoder = *p;
                is_found_encoder = true;
                break;
            }
            p = p->next;
        }
        if (!is_found_encoder)
        {
            RK_AUDIO_LOG_E("encode_run, cant found encoder");
            audio_stream_stop(recorder->record_stream);
            audio_mutex_lock(recorder->state_lock);
            if (recorder->listen)
                recorder->listen(recorder, RECORD_INFO_ENCODE, recorder->userdata);
            recorder->state = RECORDER_STATE_IDLE;
            audio_mutex_unlock(recorder->state_lock);
        }
        else
        {
            RK_AUDIO_LOG_D("encode_run init begin");
            audio_cfg.sample_rate = recorder->samplerate;
            audio_cfg.bits = recorder->bits;
            audio_cfg.channels = recorder->channels;
#ifdef OS_IS_FREERTOS
            rk_dcache_ops(RK_HW_CACHE_CLEAN, recorder, sizeof(struct recorder));
#else
            rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, recorder, sizeof(struct recorder));
#endif
            encoder.userdata = &audio_cfg;
            encoder_cfg.input = encoder_input;
            encoder_cfg.output = encoder_output;
            encoder_cfg.post = encoder_post;
            encoder_cfg.userdata = recorder;
            encoder_cfg.header_size = 0;
            if (encoder.init(&encoder, &encoder_cfg))
            {
                RK_AUDIO_LOG_E("encode_run, encoder init fail");
                audio_stream_stop(recorder->record_stream);
                audio_mutex_lock(recorder->state_lock);
                recorder->state = RECORDER_STATE_IDLE;
                if (recorder->listen)
                    recorder->listen(recorder, RECORD_INFO_ENCODE, recorder->userdata);
                audio_mutex_unlock(recorder->state_lock);
                continue;
            }
            RK_AUDIO_LOG_D("encode_run init success");
            audio_stream_start(recorder->encode_stream);
            recorder->header_size = encoder_cfg.header_size;
            msg_to_writer.type = CMD_RECORDER_START;
            msg_to_writer.recorder.mode = RECORD_MODE_PROMPT;
            msg_to_writer.recorder.target = encode_msg.recorder.target;
            msg_to_writer.recorder.type = encode_msg.recorder.type;
            msg_to_writer.recorder.writer = encode_msg.recorder.writer;
            msg_to_writer.recorder.need_free = false;
            msg_to_writer.recorder.end_session = false;
            audio_queue_send(recorder->write_queue, &msg_to_writer);

            record_encoder_error_t encode_res = encoder.process(&encoder);
            RK_AUDIO_LOG_D("encode_run process success");
            RK_AUDIO_LOG_D("encode_res:%d", encode_res);

            switch (encode_res)
            {
            case RECORD_ENCODER_INPUT_ERROR:
                RK_AUDIO_LOG_W("record_encoder_INPUT_ERROR");
                audio_stream_finish(recorder->encode_stream);
                break;
            case RECORD_ENCODER_OUTPUT_ERROR:
                RK_AUDIO_LOG_W("record_encoder_OUTPUT_ERROR");
                audio_stream_stop(recorder->record_stream);
                goto _DESTROY_encoder;
            case RECORD_ENCODER_ENCODE_ERROR:
                RK_AUDIO_LOG_W("record_encoder_encode_ERROR");
                audio_stream_stop(recorder->record_stream);
                audio_stream_stop(recorder->encode_stream);
                break;
            default:
                RK_AUDIO_LOG_W("audio_stream_finish");
                audio_stream_stop(recorder->record_stream);
                audio_stream_finish(recorder->encode_stream);
                goto _DESTROY_encoder;
                break;
            }
_DESTROY_encoder:
            RK_AUDIO_LOG_D("encoder process return value:%d", encode_res);
            encoder.destroy(&encoder);
        }
    }
}

void capture_run(void *data)
{
    recorder_handle_t recorder = (recorder_handle_t) data;
    capture_device_t device = recorder->device;
    capture_device_cfg_t device_cfg;
    media_sdk_msg_t msg, encode_msg;
    char *read_buf;
    int read_size, write_size;
    size_t frame_size;
    uint64_t total_byte;
    int byte2ms = 0;
    RK_AUDIO_LOG_D("capture_run start");
    while (1)
    {
        if (audio_queue_receive(recorder->record_queue, &msg) == -1)
        {
            RK_AUDIO_LOG_E("capture_run receive data failed");
            return;
        }
        RK_AUDIO_LOG_D("capture_run:msg.type = %x,%d", &msg.type, msg.type);

        if (msg.type == CMD_RECORDER_START)
        {
            RK_AUDIO_LOG_D("device open start.");
            device_cfg.samplerate = recorder->samplerate;
            device_cfg.bits = recorder->bits;
            device_cfg.channels = recorder->channels;
            device_cfg.device_name = recorder->device_name;
            device_cfg.frame_size = 4096;
            if (device.open(&device, &device_cfg))
            {
                RK_AUDIO_LOG_E("device open failed");
                audio_mutex_lock(recorder->state_lock);
                recorder->state = RECORDER_STATE_IDLE;
                if (recorder->listen)
                {
                    recorder->listen(recorder, RECORD_INFO_STOP, recorder->userdata);
                }
                audio_mutex_unlock(recorder->state_lock);
                continue;
            }

            audio_stream_start(recorder->record_stream);
            encode_msg.type = msg.type;
            encode_msg.recorder.mode = msg.recorder.mode;
            encode_msg.recorder.target = msg.recorder.target;
            encode_msg.recorder.type = msg.recorder.type;
            encode_msg.recorder.writer = msg.recorder.writer;
            encode_msg.recorder.need_free = msg.recorder.need_free;
            encode_msg.recorder.end_session = msg.recorder.end_session;
            encode_msg.recorder.priv_data = "pcm";
            audio_queue_send(recorder->encode_queue, &encode_msg);

            frame_size = device_cfg.frame_size;
            device.start(&device);
            read_buf = (char *)audio_malloc(frame_size);
            if (!read_buf)
            {
                RK_AUDIO_LOG_E("create read buf failed");
                break;
            }
            memset(read_buf, 0, frame_size);
            byte2ms = recorder->channels * (recorder->bits >> 3) * recorder->samplerate / 1000;
            total_byte = 0;
            while (1)
            {
                // RK_AUDIO_LOG_D("audio_stream_read frame_size:%d",frame_size);
                read_size = device.read(&device, read_buf, frame_size);
                if (read_size == -1)
                {
                    audio_stream_stop(recorder->record_stream);
                    break;
                }
                if (read_size == 0)
                {
                    RK_AUDIO_LOG_W("do not read data");
                    audio_stream_finish(recorder->record_stream);
                    break;
                }
                write_size = audio_stream_write(recorder->record_stream, read_buf, read_size);
                if (write_size < read_size)
                {
                    RK_AUDIO_LOG_W("Stream stop/finish, go to stop");
                    break;
                }
                total_byte += write_size;
                recorder->record_time = total_byte / byte2ms;
                if (recorder->duration != 0 && recorder->record_time >= (recorder->duration * 1000))
                {
                    RK_AUDIO_LOG_W("Record finish, %ld ms", recorder->record_time);
                    audio_stream_finish(recorder->record_stream);
                    break;
                }
            }
            audio_free(read_buf);
            device.stop(&device);
            device.close(&device);
            RK_AUDIO_LOG_D("record_run close");
        }
        else if (encode_msg.type == CMD_RECORDER_STOP)
        {
            RK_AUDIO_LOG_D("record_run stop");
            break;
        }
    }
}

recorder_handle_t recorder_create(recorder_cfg_t *cfg)
{
    recorder_handle_t recorder = (recorder_handle_t) audio_calloc(1, sizeof(*recorder));
    uint32_t recordr_stack_size;
    uint32_t encoder_stack_size;
    uint32_t writer_stack_size;

    RK_AUDIO_LOG_D("recorder_create in");
    if (recorder)
    {
        recorder->write_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        recorder->encode_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        recorder->record_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        recorder->record_stream = audio_stream_create(cfg->record_buf_size);
        recorder->encode_stream = audio_stream_create(cfg->encode_buf_size);
        recorder->state_lock = audio_mutex_create();
        recorder->pause_sem = audio_semaphore_create();
        recorder->tag = cfg->tag;
        recorder->listen = cfg->listen;
        recorder->userdata = cfg->userdata;
        recorder->device_name = cfg->device_name;
        recorder->device = cfg->device;
        recorder->state = RECORDER_STATE_IDLE;

        recordr_stack_size = cfg->recordr_stack_size ? cfg->recordr_stack_size : 2048;
        encoder_stack_size = cfg->encoder_stack_size ? cfg->encoder_stack_size : 1024 * 80;
        writer_stack_size = cfg->writer_stack_size ? cfg->writer_stack_size : 2048;
        audio_thread_cfg_t c =
        {
            .run = write_run,
            .args = recorder
        };
        recorder->write_task = audio_thread_create("write_task",
                                                   recordr_stack_size,
                                                   RECORDER_TASK_PRIORITY, &c);
        c.run = encoder_run;
        c.args = recorder;
        recorder->encode_task = audio_thread_create("encode_task",
                                                    encoder_stack_size,
                                                    RECORDER_TASK_PRIORITY, &c);
        c.run = capture_run;
        c.args = recorder;
        recorder->record_task = audio_thread_create("record_task",
                                                    writer_stack_size,
                                                    RECORDER_TASK_PRIORITY, &c);
        RK_AUDIO_LOG_V("Success %s 0x%lx 0x%lx 0x%lx",
                       recorder->device_name,
                       recordr_stack_size,
                       encoder_stack_size,
                       writer_stack_size);
    }
    else
    {
        RK_AUDIO_LOG_E("Failure");
    }

    return recorder;
}

int recorder_record(recorder_handle_t self, record_cfg_t *cfg)
{
    RK_AUDIO_LOG_D("start type %s", cfg->type);

    g_recordr_freq = cfg->freq_t;
    recoder_freq_init();

    recorder_handle_t recorder = self;
    media_sdk_msg_t msg;

    audio_mutex_lock(recorder->state_lock);
    recorder->state = RECORDER_STATE_RUNNING;
    audio_mutex_unlock(recorder->state_lock);
    recorder->samplerate = cfg->samplerate;
    recorder->bits = cfg->bits;
    recorder->channels = cfg->channels;
    recorder->duration = cfg->duration;
    recorder->record_time = 0;
    msg.type = CMD_RECORDER_START;
    msg.recorder.mode = RECORD_MODE_PROMPT;
    msg.recorder.type = cfg->type;
    msg.recorder.writer = cfg->writer;
    msg.recorder.need_free = cfg->need_free;
    if (cfg->need_free)
    {
        msg.recorder.target = audio_malloc(strlen(cfg->target) + 1);
        if (msg.recorder.target == NULL)
        {
            RK_AUDIO_LOG_E("no mem!");
            return RK_AUDIO_FAILURE;
        }
        memset(msg.recorder.target, 0x0, strlen(cfg->target) + 1);
        memcpy(msg.recorder.target, cfg->target, strlen(cfg->target));
    }
    else
    {
        msg.recorder.target = cfg->target;
    }

    msg.recorder.end_session = false;
    audio_queue_send(recorder->record_queue, &msg);
    return RK_AUDIO_SUCCESS;
}

recorder_state_t recorder_get_state(recorder_handle_t self)
{
    recorder_state_t state;
    audio_mutex_lock(self->state_lock);
    state = self->state;
    audio_mutex_unlock(self->state_lock);
    return state;
}

uint32_t recorder_get_cur_time(recorder_handle_t self)
{
    return self->record_time;
}

int recorder_stop(recorder_handle_t self)
{
    media_sdk_msg_t msg;
    int result;
    int time = 0;

    audio_mutex_lock(self->state_lock);
    if (self->state)
    {
        audio_stream_finish(self->record_stream);
        if (self->state == RECORDER_STATE_PAUSED)
        {
            audio_semaphore_give(self->pause_sem);
        }
        audio_mutex_unlock(self->state_lock);
        result = 0;
        msg.type = CMD_RECORDER_STOP;
        audio_queue_send(self->record_queue, &msg);
        while (self->state != RECORDER_STATE_IDLE)
        {
            audio_sleep(100);
            /* Wait 3s */
            time++;
            if (time > 30)
                break;
        }
        RK_AUDIO_LOG_V("recorder_stop stop recorder,pause/running state");
    }
    else
    {
        audio_mutex_unlock(self->state_lock);
        RK_AUDIO_LOG_V("recorder_stop stop recorder,idle state");
        result = 0;
    }

    recoder_freq_deinit();

    return result;
}

int recorder_pause(recorder_handle_t self)
{
    recoder_freq_deinit();
    return RK_AUDIO_SUCCESS;
}

int recorder_resume(recorder_handle_t self)
{
    recoder_freq_init();
    return RK_AUDIO_SUCCESS;
}

int recorder_wait_idle(recorder_handle_t self)
{
    while (self->state != RECORDER_STATE_IDLE)
        audio_sleep(10);

    return RK_AUDIO_SUCCESS;
}

int recorder_close(recorder_handle_t self)
{
    capture_device_t device = self->device;
    device.stop(&device);
    device.close(&device);

    return RK_AUDIO_SUCCESS;
}

void recorder_destroy(recorder_handle_t self)
{
    recorder_handle_t recorder = self;
    RK_AUDIO_LOG_D("in.");
    if (recorder)
    {
        recorder_stop(recorder);
        audio_queue_destroy(recorder->write_queue);
        audio_queue_destroy(recorder->encode_queue);
        audio_queue_destroy(recorder->record_queue);
        audio_stream_destroy(recorder->record_stream);
        audio_stream_destroy(recorder->encode_stream);
        audio_mutex_destroy(recorder->state_lock);
        audio_semaphore_destroy(recorder->pause_sem);
        audio_thread_exit(recorder->write_task);
        audio_thread_exit(recorder->encode_task);
        audio_thread_exit(recorder->record_task);
        audio_free(recorder);
        recorder = NULL;
    }
}

void recorder_deinit(void)
{
    if (g_default_encoder)
    {
        RK_AUDIO_LOG_D("recorder deinit.");
        record_encoder_t *p = g_default_encoder;
        int i = 0;
        while (p)
        {
            g_default_encoder = p->next;
            RK_AUDIO_LOG_V("free [Encoder %d] type:[%s]", i, p->type);
            audio_free(p->type);
            audio_free(p);
            p = g_default_encoder;
            i++;
        }
        g_default_encoder = NULL;
    }
}
