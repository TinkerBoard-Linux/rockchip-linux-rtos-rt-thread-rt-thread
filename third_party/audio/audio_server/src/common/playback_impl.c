/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */
#include <stdio.h>
#include "AudioConfig.h"
#include "audio_pcm.h"

static struct pcm *playback_handle = NULL;
static int opened = 0;
#define NO_BUFFER_MODE 1
int playback_device_open_impl(struct playback_device *self, playback_device_cfg_t *cfg)
{
    struct pcm_config *config;
#ifdef OS_IS_FREERTOS
    if (!playback_handle)
        playback_handle = pcm_open(rkos_audio_get_id(AUDIO_FLAG_WRONLY), AUDIO_FLAG_WRONLY);
#else
    if (!playback_handle)
        playback_handle = pcm_open((uint32_t)cfg->card_name, AUDIO_FLAG_WRONLY);
#endif
    if (!playback_handle)
        return RK_AUDIO_FAILURE;
    if (self->userdata)
        config = self->userdata;
    else
        config = audio_malloc(sizeof(struct pcm_config));
    if (!config)
    {
        RK_AUDIO_LOG_E("audio malloc config failed %d Byte\n", sizeof(struct pcm_config));
        pcm_close(playback_handle);
        playback_handle = NULL;
        return RK_AUDIO_FAILURE;
    }
    if (NO_BUFFER_MODE)
        config->channels = 2;
    else
        config->channels = cfg->channels ? cfg->channels : 2;

    config->rate = cfg->samplerate ? cfg->samplerate : 16000;
    config->bits = cfg->bits ? cfg->bits : 16;
    config->period_size = cfg->frame_size;
    config->period_count = 3;

    RK_AUDIO_LOG_D("cfg->frame_size = %d.", cfg->frame_size);
    RK_AUDIO_LOG_V("rate:%d bits:%d ch:%d", config->rate, config->bits, config->channels);
    if (!opened && pcm_set_config(playback_handle, *config))
    {
        pcm_close(playback_handle);
        playback_handle = NULL;
        return RK_AUDIO_FAILURE;
    }
    self->userdata = config;
    RK_AUDIO_LOG_V("Open Playback success.");
    opened = 1;

    return RK_AUDIO_SUCCESS;
}

int playback_device_start_impl(struct playback_device *self)
{
    if (playback_handle == NULL)
    {
        RK_AUDIO_LOG_E("Should call open first\n");
        return RK_AUDIO_FAILURE;
    }
    int err = pcm_prepare(playback_handle);
    err = pcm_start(playback_handle);
    RK_AUDIO_LOG_D("playback_device_start_impl err= %d", err);

    return err;
}

int playback_set_volume(int vol)
{
    return pcm_set_volume(playback_handle, vol, AUDIO_FLAG_WRONLY);
}

int playback_get_volume(void)
{
    return pcm_get_volume(playback_handle, AUDIO_FLAG_WRONLY);
}

int playback_device_write_impl(struct playback_device *self, const char *data, size_t data_len)
{
    int write_err;
    int periodsize;

    if (playback_handle == NULL)
    {
        RK_AUDIO_LOG_E("Should call open first\n");
        return RK_AUDIO_FAILURE;
    }

    if (self->userdata)
    {
        periodsize = ((struct pcm_config *)self->userdata)->period_size;
        if (data_len < periodsize)
        {
            data_len = periodsize;
        }
    }
    write_err = pcm_write(playback_handle, (void *)data, data_len);
    if (write_err < 0)
    {
        RK_AUDIO_LOG_W("write_frames: %d", write_err);
    }
    if (write_err == -EINVAL)
    {
        pcm_prepare(playback_handle);
    }

    return data_len;
}

int playback_device_stop_impl(struct playback_device *self)
{
    int stop_err = 0;

    if (playback_handle == NULL)
        return RK_AUDIO_SUCCESS;

    RK_AUDIO_LOG_V("\n");

    stop_err = pcm_stop(playback_handle);
    RK_AUDIO_LOG_D("stop_err = %d", stop_err);
    if (self->userdata)
    {
        audio_free(self->userdata);
        self->userdata = NULL;
    }
    if (stop_err != 0)
        return RK_AUDIO_FAILURE;

    return RK_AUDIO_SUCCESS;
}

int playback_device_abort_impl(struct playback_device *self)
{
    RK_AUDIO_LOG_D("\n");

    return RK_AUDIO_SUCCESS;
}

void playback_device_close_impl(struct playback_device *self)
{
    RK_AUDIO_LOG_D("\n");

    if (playback_handle)
    {
        pcm_close(playback_handle);
    }
    playback_handle = NULL;
    opened = 0;
}
