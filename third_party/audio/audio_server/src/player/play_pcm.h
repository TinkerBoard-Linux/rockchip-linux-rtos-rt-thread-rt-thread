/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef PLAY_PCM_H
#define PLAY_PCM_H
#ifdef __cplusplus
extern "C" {
#endif

int play_pcm_init(struct play_decoder *self, play_decoder_cfg_t *cfg);
play_decoder_error_t play_pcm_process(struct play_decoder *self);
bool play_pcm_get_post_state(struct play_decoder *self);
void play_pcm_destroy(struct play_decoder *self);

#define DEFAULT_PCM_DECODER { \
        .type = "pcm", \
        .support_seek = 1, \
        .check = NULL, \
        .init = play_pcm_init, \
        .process = play_pcm_process, \
        .get_post_state = play_pcm_get_post_state, \
        .destroy = play_pcm_destroy, \
    }

#ifdef __cplusplus
}
#endif
#endif
