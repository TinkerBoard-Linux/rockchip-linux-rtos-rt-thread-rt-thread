/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef RECORD_PCM_H
#define RECORD_PCM_H
#ifdef __cplusplus
extern "C" {
#endif

int record_pcm_init(struct record_encoder *self, record_encoder_cfg_t *cfg);
record_encoder_error_t record_pcm_process(struct record_encoder *self);
bool record_pcm_get_post_state(struct record_encoder *self);
void record_pcm_destroy(struct record_encoder *self);

#define DEFAULT_PCM_ENCODER { \
        .type = "pcm", \
        .init = record_pcm_init, \
        .process = record_pcm_process, \
        .get_post_state = record_pcm_get_post_state, \
        .destroy = record_pcm_destroy, \
    }

#ifdef __cplusplus
}
#endif
#endif
