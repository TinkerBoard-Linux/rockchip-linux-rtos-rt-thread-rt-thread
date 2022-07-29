/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef _AUDIO_SPEEX_ENCODE_HAL_H_
#define _AUDIO_SPEEX_ENCODE_HAL_H_

#include "rkos_typedef.h"

#define    SPEEX_AB_CORE_SHARE_ADDR_INVALID (-1)
typedef void *speex_handler;
typedef void (*speex_encoded_func)(const void *, size_t);

typedef enum _duer_sample_rate_e
{
    DUER_SR_8000,
    DUER_SR_16000,
    DUER_SR_TOTAL,
} duer_sample_rate_t;

typedef struct _duer_speex_s
{
    duer_sample_rate_t  _rate;
    void               *_encoder;
    size_t              _frame_size;
    size_t              _frame_cur;
    short              *_buffer_in;
    char               *_buffer_out;
} duer_speex_t;

int speex_encode_init(int rate);
void speex_encode_destroy(void);
int speex_encode_process(const char *data, size_t size, speex_encoded_func func);
#endif        // _AUDIO_SPEEX_ENCODE_HAL_H_
