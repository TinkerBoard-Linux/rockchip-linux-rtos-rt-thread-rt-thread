/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

struct record_pcm
{
    bool has_post;
    char read_buf[1024];
    encode_input_t input;
    encode_output_t output;
    encode_post_t post;
    void *userdata;
};
typedef struct record_pcm *record_pcm_handle_t;

int record_pcm_init(struct record_encoder *self, record_encoder_cfg_t *cfg)
{
    record_pcm_handle_t pcm = (record_pcm_handle_t) audio_calloc(1, sizeof(*pcm));
    if (!pcm)
        return RK_AUDIO_FAILURE;
    pcm->has_post = false;
    pcm->input = cfg->input;
    pcm->output = cfg->output;
    pcm->post = cfg->post;
    pcm->userdata = cfg->userdata;
    self->userdata = (void *) pcm;
    return RK_AUDIO_SUCCESS;
}
record_encoder_error_t record_pcm_process(struct record_encoder *self)
{
    record_pcm_handle_t pcm = (record_pcm_handle_t) self->userdata;
    RK_AUDIO_LOG_D("in\n");
    while (1)
    {
        int read_bytes = pcm->input(pcm->userdata, pcm->read_buf, sizeof(pcm->read_buf));
        //OS_LOG_D(record_pcm,"record_pcm_process:%d\n",read_bytes);
        if (read_bytes == 0)
        {
            RK_AUDIO_LOG_V("pcm->input finish \n");
            return RECORD_ENCODER_SUCCESS;
        }
        else if (read_bytes == -1)
        {
            RK_AUDIO_LOG_E("pcm->input failed \n");
            return RECORD_ENCODER_INPUT_ERROR;
        }
        int write_bytes = pcm->output(pcm->userdata, pcm->read_buf, read_bytes);
        if (write_bytes == -1)
        {
            RK_AUDIO_LOG_E("pcm->output failed \n");
            return RECORD_ENCODER_OUTPUT_ERROR;
        }
    }
    return RECORD_ENCODER_SUCCESS;
}

bool record_pcm_get_post_state(struct record_encoder *self)
{
    record_pcm_handle_t pcm = (record_pcm_handle_t) self->userdata;
    return pcm->has_post;
}
void record_pcm_destroy(struct record_encoder *self)
{
    record_pcm_handle_t pcm = (record_pcm_handle_t) self->userdata;
    if (pcm)
    {
        audio_free(pcm);
    }
}
