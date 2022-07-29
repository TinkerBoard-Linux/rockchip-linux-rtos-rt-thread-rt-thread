/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"
#include <string.h>

int wav_header_init(struct wav_header *head, int samplerate, int bits, int channels)
{
    memcpy(head->chunk_id, "RIFF", 4);
    memcpy(head->format, "WAVE", 4);
    memcpy(head->subchunk1_id, "fmt ", 4);
    head->subchunk1_size = 16;
    head->audio_format = 1;
    head->num_channels = channels;
    head->samplerate = samplerate;
    head->byterate = samplerate * channels * bits / 8;
    head->block_align = channels * bits / 8;
    head->bits_per_sample = bits;
    memcpy(head->subchunk2_id, "data", 4);
    return 0;
}

int wav_header_complete(struct wav_header *head, int samples)
{
    head->subchunk2_size = samples * head->num_channels * head->bits_per_sample / 8;
    head->chunk_size = head->subchunk2_size + 36;
    return 0;
}