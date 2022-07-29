/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef RECORD_AMR_H
#define RECORD_AMR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "AudioConfig.h"
#ifdef AUDIO_ENCODER_AMR

#define WMF_MR122_FRAME_SIZE 31
#define SAMPLE_PER_FRAME 160

typedef struct _amr_enc_s
{
    void               *_encoder;
    short               _buffer_in[SAMPLE_PER_FRAME];
    unsigned char       _buffer_out[WMF_MR122_FRAME_SIZE + 1];
    struct audio_server_data_share *ab_share_dat;
} amr_enc_t;

int record_amr_init(struct record_encoder *self, record_encoder_cfg_t *cfg);
record_encoder_error_t record_amr_process(struct record_encoder *self);
bool record_amr_get_post_state(struct record_encoder *self);
void record_amr_destroy(struct record_encoder *self);

#define DEFAULT_AMR_ENCODER { \
        .type = "amr", \
        .init = record_amr_init, \
        .process = record_amr_process, \
        .get_post_state = record_amr_get_post_state, \
        .destroy = record_amr_destroy, \
    }
#else
#define DEFAULT_AMR_ENCODER { \
        .type = "null", \
        .init = NULL, \
        .process = NULL, \
        .get_post_state = NULL, \
        .destroy = NULL, \
    }
#endif

#ifdef __cplusplus
}
#endif
#endif
