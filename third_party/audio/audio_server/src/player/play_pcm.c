/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

struct play_pcm
{
    bool has_post;
    char read_buf[1024];
    decode_input_t input;
    decode_output_t output;
    decode_post_t post;
    void *userdata;
};
typedef struct play_pcm *play_pcm_handle_t;

int play_pcm_init(struct play_decoder *self, play_decoder_cfg_t *cfg)
{
    play_pcm_handle_t pcm = (play_pcm_handle_t) audio_calloc(1, sizeof(*pcm));
    if (!pcm)
        return RK_AUDIO_FAILURE;
    pcm->has_post = false;
    pcm->input = cfg->input;
    pcm->output = cfg->output;
    pcm->post = cfg->post;
    pcm->userdata = cfg->userdata;
    self->userdata = (void *) pcm;
    if (cfg->start_time)
        return player_preprocess_seek(pcm->userdata, cfg->start_time * 48000 * 2 * 2);

    return RK_AUDIO_SUCCESS;
}
play_decoder_error_t play_pcm_process(struct play_decoder *self)
{
    play_pcm_handle_t pcm = (play_pcm_handle_t) self->userdata;
    RK_AUDIO_LOG_D("in\n");
    pcm->post(pcm->userdata, 48000, 16, 2);
    pcm->has_post = true;
    while (1)
    {
        int read_bytes = pcm->input(pcm->userdata, pcm->read_buf, sizeof(pcm->read_buf) / 2);
        if (read_bytes == 0)
        {
            return RK_AUDIO_SUCCESS;
        }
        else if (read_bytes == -1)
        {
            return RK_AUDIO_FAILURE;
        }
        int write_bytes = pcm->output(pcm->userdata, pcm->read_buf, read_bytes);
        if (write_bytes == -1)
        {
            return RK_AUDIO_FAILURE;
        }
    }
    return RK_AUDIO_SUCCESS;
}

bool play_pcm_get_post_state(struct play_decoder *self)
{
    play_pcm_handle_t pcm = (play_pcm_handle_t) self->userdata;
    return pcm->has_post;
}

void play_pcm_destroy(struct play_decoder *self)
{
    play_pcm_handle_t pcm = (play_pcm_handle_t) self->userdata;
    if (pcm)
    {
        audio_free(pcm);
    }
}
