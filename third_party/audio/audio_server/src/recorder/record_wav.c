/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

struct record_wav
{
    bool has_post;
    char read_buf[1024];
    encode_input_t input;
    encode_output_t output;
    encode_post_t post;
    struct wav_header header;
    long total_byte;
    void *userdata;
};

int record_wav_init(struct record_encoder *self, record_encoder_cfg_t *cfg)
{
    struct record_wav *wav = (struct record_wav *)audio_calloc(1, sizeof(struct record_wav));
    struct audio_config wav_cfg = *(struct audio_config *)self->userdata;
    if (!wav)
        return -1;
    wav->has_post = false;
    wav->input = cfg->input;
    wav->output = cfg->output;
    wav->post = cfg->post;
    wav->userdata = cfg->userdata;
    wav->total_byte = 0;
    cfg->header_size = sizeof(struct wav_header);
    wav_header_init(&wav->header, wav_cfg.sample_rate, wav_cfg.bits, wav_cfg.channels);
    RK_AUDIO_LOG_D("cfg:r %d b %d c %d\n", wav_cfg.sample_rate, wav_cfg.bits, wav_cfg.channels);
    self->userdata = (void *)wav;

    return 0;
}

record_encoder_error_t record_wav_process(struct record_encoder *self)
{
    struct record_wav * wav = (struct record_wav *) self->userdata;
    RK_AUDIO_LOG_D("\n");
    wav->output(wav->userdata, (char *)&wav->header, sizeof(struct wav_header));
    while (1)
    {
        int read_bytes = wav->input(wav->userdata, wav->read_buf, sizeof(wav->read_buf));
        //RK_AUDIO_LOG_D("record_wav_process:%d\n",read_bytes);
        if (read_bytes == 0)
        {
            RK_AUDIO_LOG_W("wav->input finish \n");
            wav_header_complete(&wav->header, wav->total_byte);
            wav->output(wav->userdata, (char *)&wav->header, sizeof(struct wav_header));
            return RECORD_ENCODER_SUCCESS;
        }
        else if (read_bytes == -1)
        {
            RK_AUDIO_LOG_E("wav->input failed \n");
            wav_header_complete(&wav->header, wav->total_byte);
            wav->output(wav->userdata, (char *)&wav->header, sizeof(struct wav_header));
            return RECORD_ENCODER_INPUT_ERROR;
        }
        int write_bytes = wav->output(wav->userdata, wav->read_buf, read_bytes);
        wav->total_byte += write_bytes;
        if (write_bytes == -1)
        {
            RK_AUDIO_LOG_E("wav->output failed \n");
            return RECORD_ENCODER_OUTPUT_ERROR;
        }
    }

    return RECORD_ENCODER_SUCCESS;
}

bool record_wav_get_post_state(struct record_encoder *self)
{
    struct record_wav * wav = (struct record_wav *) self->userdata;

    return wav->has_post;
}

void record_wav_destroy(struct record_encoder *self)
{
    struct record_wav * wav = (struct record_wav *) self->userdata;
    if (wav)
    {
        audio_free(wav);
    }
}
