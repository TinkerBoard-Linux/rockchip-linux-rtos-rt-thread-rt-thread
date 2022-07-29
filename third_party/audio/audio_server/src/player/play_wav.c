/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

struct play_wav
{
    bool has_post;
    char read_buf[1024];
    decode_input_t input;
    decode_output_t output;
    decode_post_t post;
    void *userdata;
};
typedef struct play_wav *play_wav_handle_t;

int play_wav_check_impl(char *buf, int len)
{
    struct wav_header *header = (struct wav_header *)buf;
    return (memcmp(header->chunk_id, "RIFF", 4) || memcmp(header->format, "WAVE", 4));
}

static int play_wav_preprocess(struct play_decoder *self, int seek)
{
    play_wav_handle_t wav = (play_wav_handle_t)self->userdata;
    struct wav_header header;
    int read_bytes;
    uint32_t bitrate;
    uint32_t file_length;
    uint32_t time;

    read_bytes = wav->input(wav->userdata, (char *)&header,
                            sizeof(header));
    if (read_bytes != sizeof(header))
        return PLAY_DECODER_INPUT_ERROR;

    if (memcmp(header.chunk_id, "RIFF", 4) != 0 ||
        memcmp(header.format, "WAVE", 4) != 0)
    {
        return PLAY_DECODER_DECODE_ERROR;
    }

    bitrate = header.samplerate * header.num_channels * (header.bits_per_sample >> 3);
    file_length = player_get_file_length(wav->userdata);
    time = (file_length - sizeof(struct wav_header)) / (bitrate / 1000);
    player_set_total_time(wav->userdata, time);
    RK_AUDIO_LOG_V("wav decoder sample rate: %ld, channels: %d, bits: %d",
                   header.samplerate, header.num_channels, header.bits_per_sample);
    wav->post(wav->userdata, header.samplerate, header.bits_per_sample,
              header.num_channels);
    wav->has_post = true;
    if (seek)
        return player_preprocess_seek(wav->userdata, seek * bitrate  + sizeof(struct wav_header));

    return RK_AUDIO_SUCCESS;
}

int play_wav_init_impl(struct play_decoder *self, play_decoder_cfg_t *cfg)
{
    play_wav_handle_t wav = (play_wav_handle_t)audio_calloc(1, sizeof(*wav));
    if (!wav)
        return RK_AUDIO_FAILURE;
    wav->input = cfg->input;
    wav->output = cfg->output;
    wav->post = cfg->post;
    wav->userdata = cfg->userdata;
    self->userdata = (void *)wav;

    return play_wav_preprocess(self, cfg->start_time);
}

play_decoder_error_t play_wav_process_impl(struct play_decoder *self)
{
    play_wav_handle_t wav = (play_wav_handle_t)self->userdata;
    int read_bytes;
    int write_bytes;

    while (1)
    {
        read_bytes = wav->input(wav->userdata, wav->read_buf,
                                sizeof(wav->read_buf) / 2);
        if (read_bytes == 0)
        {
            return PLAY_DECODER_SUCCESS;
        }
        else if (read_bytes == -1)
        {
            return PLAY_DECODER_SUCCESS;
        }
        write_bytes = wav->output(wav->userdata, wav->read_buf,
                                  read_bytes);
        if (write_bytes == -1)
            return PLAY_DECODER_OUTPUT_ERROR;
    }
    return PLAY_DECODER_SUCCESS;
}
bool play_wav_get_post_state_impl(struct play_decoder *self)
{
    play_wav_handle_t wav = (play_wav_handle_t)self->userdata;
    return wav->has_post;
}
void play_wav_destroy_impl(struct play_decoder *self)
{
    if (!self)
        return;
    play_wav_handle_t wav = (play_wav_handle_t)self->userdata;
    audio_free(wav);
}
