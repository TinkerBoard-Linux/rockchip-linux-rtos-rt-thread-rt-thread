/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef RECORD_WAV_H
#define RECORD_WAV_H
#ifdef __cplusplus
extern "C" {
#endif

int record_wav_init(struct record_encoder *self, record_encoder_cfg_t *cfg);
record_encoder_error_t record_wav_process(struct record_encoder *self);
bool record_wav_get_post_state(struct record_encoder *self);
void record_wav_destroy(struct record_encoder *self);

#define DEFAULT_WAV_ENCODER { \
        .type = "wav", \
        .init = record_wav_init, \
        .process = record_wav_process, \
        .get_post_state = record_wav_get_post_state, \
        .destroy = record_wav_destroy, \
    }

#ifdef __cplusplus
}
#endif
#endif
