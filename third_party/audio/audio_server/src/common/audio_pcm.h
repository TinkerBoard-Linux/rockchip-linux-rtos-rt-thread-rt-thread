/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef __AUDIO_SERVER_PCM_H__
#define __AUDIO_SERVER_PCM_H__

#include "AudioConfig.h"

enum
{
    /* Start from -100 to avoid the return value of rk_audio */
    PCM_WRONG_TYPE = -100,
    PCM_WRONG_LENGTH = -101,
};

/*
 * PCM API
 */
#define PCM_OUT        0x00000000
#define PCM_IN         0x10000000
#define PCM_MMAP       0x00000001
#define PCM_NOIRQ      0x00000002
#define PCM_NORESTART  0x00000004 /* PCM_NORESTART - when set, calls to
                                   * pcm_write for a playback stream will not
                                   * attempt to restart the stream in the case
                                   * of an underflow, but will return -EPIPE
                                   * instead.  After the first -EPIPE error, the
                                   * stream is considered to be stopped, and a
                                   * second call to pcm_write will attempt to
                                   * restart the stream.
                                   */

/* Configuration for a stream */
struct pcm_config
{
    unsigned int want_ch;
    unsigned int channels;
    unsigned int rate;
    unsigned int bits;
    unsigned int period_size;
    unsigned int period_count;
};

struct pcm *pcm_open(const uint32_t dev_id, int flag);
int pcm_set_config(struct pcm *pcm_dev, struct pcm_config config);
int pcm_set_volume(struct pcm *pcm_dev, int vol, int flag);
int pcm_get_volume(struct pcm *pcm_dev, int flag);
int pcm_start(struct pcm *pcm_dev);
int pcm_prepare(struct pcm *pcm_dev);
int pcm_start(struct pcm *pcm_dev);
int pcm_stop(struct pcm *pcm_dev);
int pcm_close(struct pcm *pcm_dev);
unsigned long pcm_write(struct pcm *pcm_dev, void *data, unsigned long bytes);
unsigned long pcm_read(struct pcm *pcm_dev, void *data, unsigned long bytes);

#endif /* __AUDIO_SERVER_PCM_H__ */
