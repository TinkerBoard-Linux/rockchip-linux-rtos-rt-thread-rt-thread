/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include <stdio.h>
#include "AudioConfig.h"
#include "audio_pcm.h"

static struct pcm *capture_handle = NULL;
static audio_player_mutex_handle capture_lock = NULL;

int capture_device_open_impl(struct capture_device *self, capture_device_cfg_t *cfg)
{
    struct pcm_config *config = NULL;

    RK_AUDIO_LOG_D("cfg->frame_size = %d.\n", cfg->frame_size);
    if (!capture_lock)
    {
        capture_lock = audio_mutex_create();
        if (!capture_lock)
        {
            RK_AUDIO_LOG_E("capture lock create failed!");
            return RK_AUDIO_FAILURE;
        }
    }
    audio_mutex_lock(capture_lock);

    if (capture_handle)
        goto OPEN_SUCCESS;
    config = audio_malloc(sizeof(struct pcm_config));
#ifdef CAPTURE_ALWAYS_4CH
    config->channels = 4;
#else
    config->channels = cfg->channels ? cfg->channels : 2;
#endif
    config->rate = cfg->samplerate ? cfg->samplerate : 16000;
    config->bits = cfg->bits ? cfg->bits : 16;
    config->period_size = cfg->frame_size;
    config->period_count = 4;

#ifdef OS_IS_FREERTOS
    capture_handle = pcm_open(rkos_audio_get_id(AUDIO_FLAG_RDONLY), AUDIO_FLAG_RDONLY);
#else
    capture_handle = pcm_open((uint32_t)cfg->device_name, AUDIO_FLAG_RDONLY);
#endif
    if (!capture_handle)
        return RK_AUDIO_FAILURE;
    if (pcm_set_config(capture_handle, *config))
    {
        pcm_close(capture_handle);
        audio_free(config);
        capture_handle = NULL;
        return RK_AUDIO_FAILURE;
    }
    audio_free(config);
OPEN_SUCCESS:

    RK_AUDIO_LOG_V("Open Capture success.\n");

    return RK_AUDIO_SUCCESS;
}

int capture_device_start_impl(struct capture_device *self)
{
    int cap_ret = 0;
    if (capture_handle == NULL)
    {
        RK_AUDIO_LOG_E("Should call open first\n");
        return RK_AUDIO_FAILURE;
    }

    RK_AUDIO_LOG_D("\n");
    pcm_prepare(capture_handle);
    cap_ret = pcm_start(capture_handle);
    RK_AUDIO_LOG_D("pcm start return %d.\n", cap_ret);
    if (cap_ret != 0)
        return RK_AUDIO_FAILURE;

    return RK_AUDIO_SUCCESS;
}

int capture_set_volume(int vol)
{
    return pcm_set_volume(capture_handle, vol, AUDIO_FLAG_RDONLY);
}

int capture_get_volume(void)
{
    return pcm_get_volume(capture_handle, AUDIO_FLAG_RDONLY);
}

int capture_device_read_impl(struct capture_device *self, const char *data, size_t data_len)
{
    int read_err = 0;
    if (capture_handle == NULL)
    {
        RK_AUDIO_LOG_E("Should call open first\n");
        return RK_AUDIO_FAILURE;
    }

    read_err = pcm_read(capture_handle, (void *)data, data_len);
    if (read_err < 0)
    {
        RK_AUDIO_LOG_E("read_frames: %d\n", read_err);
    }
    if (read_err == -EINVAL)
    {
        pcm_prepare(capture_handle);
    }

    return data_len;

#ifdef USE_ALSA
    int write_frames = snd_pcm_writei(capture_handle, data, period_size);
    if (write_frames < 0)
    {
        RK_AUDIO_LOG_E("write_frames: %d, %s", write_frames, snd_strerror(write_frames));
    }
    if (write_frames == -EPIPE)
    {
        snd_pcm_prepare(capture_handle);
    }
    return data_len;
#endif
}

int capture_device_stop_impl(struct capture_device *self)
{
#ifdef CAPTURE_ALWAYS_4CH
    return RK_AUDIO_SUCCESS;
#endif
    if (capture_handle == NULL)
        return RK_AUDIO_SUCCESS;
    int stop_err = 0;

    RK_AUDIO_LOG_D("\n");

    stop_err = pcm_stop(capture_handle);
    RK_AUDIO_LOG_D("stop_err = %d\n", stop_err);
    if (stop_err != 0)
        return RK_AUDIO_FAILURE;

    return RK_AUDIO_SUCCESS;
}

int capture_device_abort_impl(struct capture_device *self)
{
    RK_AUDIO_LOG_D("\n");
    return RK_AUDIO_SUCCESS;
}

int capture_device_close_impl(struct capture_device *self)
{
    RK_AUDIO_LOG_D("\n");
#ifdef CAPTURE_ALWAYS_4CH
    audio_mutex_unlock(capture_lock);
#else
    if (capture_handle)
        pcm_close(capture_handle);
    capture_handle = NULL;
    audio_mutex_unlock(capture_lock);
#endif

    return RK_AUDIO_SUCCESS;
}
