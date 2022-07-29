/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef PLAY_MP3_H
#define PLAY_MP3_H
#ifdef __cplusplus
extern "C" {
#endif

#include "AudioConfig.h"

#ifdef AUDIO_DECODER_MP3

#define MP3_DECODE_FRAME_COUNT 2
#define    MP3_ID3V2_HEADER_LENGHT (10)
#define    MP3_AB_CORE_SHARE_ADDR_INVALID (0xdeadfcfc)

#ifdef  CONFIG_FWANALYSIS_SEGMENT
#define DEFAULT_MP3_DECODER { \
        .type = "mp3", \
        .support_seek = 1, \
        .segment = SEGMENT_ID_PLAY_MP3, \
        .check = play_mp3_check_impl, \
        .init = play_mp3_init_impl, \
        .process = play_mp3_process_impl, \
        .get_post_state = play_mp3_get_post_state_impl, \
        .destroy = play_mp3_destroy_impl, \
    }
#else
#define DEFAULT_MP3_DECODER { \
        .type = "mp3", \
        .support_seek = 1, \
        .check = play_mp3_check_impl, \
        .init = play_mp3_init_impl, \
        .process = play_mp3_process_impl, \
        .get_post_state = play_mp3_get_post_state_impl, \
        .destroy = play_mp3_destroy_impl, \
    }
#endif

/*
 * ABcoreMp3.h
 * {{{
 */
#include "mp3dec.h"
#include "mp3common.h"

typedef struct _MP3PLAYERINFO
{
    HMP3Decoder mpi_mp3dec;
    MP3FrameInfo mpi_frameinfo;
} MP3PLAYERINFO;

struct play_mp3
{
    bool has_post;
    char read_buf[MAINBUF_SIZE * MP3_DECODE_FRAME_COUNT];
    uint32_t start_time;
    decode_input_t input;
    decode_output_t output;
    decode_post_t post;
    void *userdata;
};
/*
 * ABcoreMp3.h
 * }}}
 */

int play_mp3_check_impl(char *buf, int len);
int play_mp3_init_impl(struct play_decoder *self, play_decoder_cfg_t *cfg);
play_decoder_error_t play_mp3_process_impl(struct play_decoder *self);
bool play_mp3_get_post_state_impl(struct play_decoder *self);
void play_mp3_destroy_impl(struct play_decoder *self);
#endif

#ifdef __cplusplus
}
#endif
#endif
