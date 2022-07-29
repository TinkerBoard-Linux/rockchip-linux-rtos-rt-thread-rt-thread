/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef _AUDIO_CONFIG_H
#define _AUDIO_CONFIG_H

#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <errno.h>

#if defined(__RK_OS__)
#define OS_IS_FREERTOS
#define RK_AUDIO_USED   __USED
#define RK_AUDIO_UNUSED __UNUSED

#include "BspConfig.h"
#include "BSP.h"
#include "driver/dma.h"
#include "driver/interrupt.h"
#include "driver/rk_audio.h"
#include "driver/rk_cache.h"
#include "hal_base.h"
#include "kernel/RKOS.h"
#include "kernel/TaskPlugin.h"
#include "kernel/device.h"
#include "rkos_typedef.h"
#include "subsys/global.h"
#include "subsys/SystemApi.h"
#include "subsys/SysInfoSave.h"

#ifdef CONFIG_DRIVER_DSP
#include "driver/DspDevice.h"
#define AUDIO_USING_DSP
#endif
#ifndef CONFIG_AUDIO_CODEC_RUN_ON_DSP
#define RK_AUDIO_CODEC_RUN_ON_M4
#endif

#ifdef CONFIG_AUDIO_DECODER_MP3
#define AUDIO_DECODER_MP3
#include "mp3_hal.h"
#endif
#ifdef CONFIG_AUDIO_DECODER_AMR
#define AUDIO_DECODER_AMR
#include "codec/decode/amr/amr_dec_hal.h"
#endif
#ifdef CONFIG_AUDIO_ENCODER_AMR
#define AUDIO_ENCODER_AMR
#include "codec/encode/amr/amr_enc_hal.h"
#endif
#ifdef CONFIG_AUDIO_ENCODER_SPEEX
#define AUDIO_ENCODER_SPEEX
#endif

#define AUDIO_USING_HTTP
#define CAPTURE_ALWAYS_4CH
#define PLAYER_TASK_PRIORITY        28
#define RECORDER_TASK_PRIORITY      28

#define RK_AUDIO_SUCCESS            RK_SUCCESS
#define RK_AUDIO_FAILURE            RK_ERROR
#define RK_AUDIO_CACHE_INVALIDATE   RK_HW_CACHE_INVALIDATE
#define RK_AUDIO_CACHE_FLUSH        RK_HW_CACHE_CLEAN
#define AUDIO_FLAG_RDONLY           READ_ONLY
#define AUDIO_FLAG_WRONLY           WRITE_ONLY
#define AUDIO_FLAG_RDWR             READ_WRITE
#define AUDIO_TICK_PER_SEC          CONFIG_FREERTOS_TICK_HZ
#define AUDIO_TIMER_PERIOD          1
#define AUDIO_TIMER_ONCE            0

#elif defined(__RT_THREAD__)
#define OS_IS_RTTHREAD
#define RK_AUDIO_USED RT_USED
#define RK_AUDIO_UNUSED RT_UNUSED
#include "rtthread.h"
#include "rtdef.h"
#include "rtdevice.h"
#include "rthw.h"
#include "rk_audio.h"
#include "drv_heap.h"
#include "hal_audio.h"
#include "hal_base.h"
#ifdef AUDIO_ENCODER_AMR
#include "codec/encode/amr/amr_enc_hal.h"
#endif
#ifdef AUDIO_DECODER_AMR
#include "codec/decode/amr/amr_dec_hal.h"
#endif
#ifdef AUDIO_DECODER_MP3
#include "mp3_hal.h"
#endif

#ifdef RT_USING_DSP
#define AUDIO_USING_DSP
#include "drv_dsp.h"
#endif
#ifndef AUDIO_CODEC_RUN_ON_DSP
#define RK_AUDIO_CODEC_RUN_ON_M4
#endif
#ifdef RT_USING_NET_HTTP
#define AUDIO_USING_HTTP
#endif
#define PLAYER_TASK_PRIORITY        14
#define RECORDER_TASK_PRIORITY      14

#define RK_AUDIO_SUCCESS            0
#define RK_AUDIO_FAILURE            -1
#define RK_AUDIO_CACHE_INVALIDATE   RT_HW_CACHE_INVALIDATE
#define RK_AUDIO_CACHE_FLUSH        RT_HW_CACHE_FLUSH
#define AUDIO_FLAG_RDONLY           RT_DEVICE_FLAG_RDONLY
#define AUDIO_FLAG_WRONLY           RT_DEVICE_FLAG_WRONLY
#define AUDIO_FLAG_RDWR             RT_DEVICE_FLAG_RDWR
#define AUDIO_TICK_PER_SEC          RT_TICK_PER_SECOND
#define AUDIO_TIMER_PERIOD          RT_TIMER_FLAG_PERIODIC
#define AUDIO_TIMER_ONCE            RT_TIMER_FLAG_ONE_SHOT

typedef void *HDC;
#else
#error  "Not Support System OS"
#endif

#include "audio_ctrl.h"
#include "audio_server.h"
#ifdef AUDIO_ENABLE_PLAYER
#include "play_mp3.h"
#include "play_pcm.h"
#endif
#ifdef AUDIO_ENABLE_RECORDER
#include "record_amr.h"
#include "record_pcm.h"
#include "record_wav.h"
#endif

#define RK_AUDIO_CASE_CHECK(x,y)    ((x) | ((y) << 16))

#define RK_AUDIO_TRUE   (1)
#define RK_AUDIO_FALSE  (0)
#define AMR_AB_CORE_SHARE_ADDR_INVALID (-1)
#ifndef MAX_VOLUME
#define MAX_VOLUME 26
#endif
#ifndef MAX_VOLUME_TABLE
#define MAX_VOLUME_TABLE 32
#endif

#ifdef CONFIG_COMPONENTS_PLAYER_FADE_IN_FADE_OUT
#define PLAYER_FADE_IN      1
#define PLAYER_FADE_OUT     1
#else
#define PLAYER_FADE_IN      0
#define PLAYER_FADE_OUT     0
#endif
#define FADE_OUT_DELAY_MS   1

#define RECORD_CARD_CHANNEL_0   0
#define RECORD_CARD_CHANNEL_1   1
#define RECORD_CARD_CHANNEL_2   2
#define RECORD_CARD_CHANNEL_ALL 3

#define RK_AUDIO_LOG_LEVEL_D    4
#define RK_AUDIO_LOG_LEVEL_V    3
#define RK_AUDIO_LOG_LEVEL_W    2
#define RK_AUDIO_LOG_LEVEL_E    1
#define RK_AUDIO_LOG_LEVEL_N    0   // hide all log
#define RK_AUDIO_LOG_LEVEL  RK_AUDIO_LOG_LEVEL_V

#if (RK_AUDIO_LOG_LEVEL >= RK_AUDIO_LOG_LEVEL_D)
#define RK_AUDIO_LOG_D(fmt, ...)    printf( "[D]%s-%d: " fmt"\n", __func__, __LINE__, ##__VA_ARGS__ )
#else
#define RK_AUDIO_LOG_D(fmt, ...)
#endif

#if (RK_AUDIO_LOG_LEVEL >= RK_AUDIO_LOG_LEVEL_V)
#define RK_AUDIO_LOG_V(fmt, ...)    printf( "[V]%s-%d: " fmt"\n", __func__, __LINE__, ##__VA_ARGS__ )
#else
#define RK_AUDIO_LOG_V(fmt, ...)
#endif

#if (RK_AUDIO_LOG_LEVEL >= RK_AUDIO_LOG_LEVEL_W)
#define RK_AUDIO_LOG_W(fmt, ...)    printf( "[W]%s-%d: " fmt"\n", __func__, __LINE__, ##__VA_ARGS__ )
#else
#define RK_AUDIO_LOG_W(fmt, ...)
#endif

#if (RK_AUDIO_LOG_LEVEL >= RK_AUDIO_LOG_LEVEL_E)
#define RK_AUDIO_LOG_E(fmt, ...)    printf( "[E]%s-%d: " fmt"\n", __func__, __LINE__, ##__VA_ARGS__ )
#else
#define RK_AUDIO_LOG_E(fmt, ...)
#endif

extern uint32_t g_BcoreRetval;
typedef struct
{
    void (*run)(void *args);
    void *args;
} audio_thread_cfg_t;

enum audio_task_state
{
    AUDIO_TASK_IDLE,
    AUDIO_TASK_BUSY,
    AUDIO_TASK_ERROR
};

typedef enum
{
    AUDIO_QUEUE_STATE_WAIT_READABLE = 0x1,
    AUDIO_QUEUE_STATE_WAIT_WRITABLE = 0x2,
    AUDIO_QUEUE_STATE_STOPPED = 0x4,
    AUDIO_QUEUE_STATE_FINISHED = 0x8
} audio_player_queue_state_t;

struct audio_player_queue
{
    char **item;
    char *item_temp;
    size_t item_size;
    size_t item_count;
    size_t fill;
    size_t read_pos;
    size_t write_pos;
    audio_player_mutex_handle lock;
    audio_player_semaphore_handle read_sem;
    audio_player_semaphore_handle write_sem;
    audio_player_queue_state_t state;
};

typedef enum
{
    AUDIO_STREAM_STATE_IDLE = 0x0,
    AUDIO_STREAM_STATE_RUN = 0x1,
    AUDIO_STREAM_STATE_WAIT_READABLE = 0x2,
    AUDIO_STREAM_STATE_WAIT_WRITABLE = 0x4,
    AUDIO_STREAM_STATE_STOPPED = 0x8,
    AUDIO_STREAM_STATE_FINISHED = 0x10,
    AUDIO_STREAM_STATE_RESET = 0x20,
} audio_player_stream_state_t;

struct audio_player_stream
{
    char *buf;
    size_t buf_size;
    size_t fill;
    size_t read_pos;
    size_t write_pos;
    audio_player_mutex_handle lock;
    audio_player_semaphore_handle read_sem;
    audio_player_semaphore_handle write_sem;
    audio_player_stream_state_t state;
};

audio_player_thread_handle audio_thread_create(char *name, uint32_t StackDeep, uint32_t Priority, audio_thread_cfg_t *cfg);
void audio_thread_exit(audio_player_thread_handle self);
audio_player_semaphore_handle audio_semaphore_create(void);
int audio_semaphore_take(audio_player_semaphore_handle self);
int audio_semaphore_try_take(audio_player_semaphore_handle self);
int audio_semaphore_give(audio_player_semaphore_handle self);
void audio_semaphore_destroy(audio_player_semaphore_handle self);
audio_player_mutex_handle audio_mutex_create(void);
int audio_mutex_lock(audio_player_mutex_handle self);
int audio_mutex_unlock(audio_player_mutex_handle self);
void audio_mutex_destroy(audio_player_mutex_handle self);
void *audio_malloc(size_t size);
void *audio_timer_create(char *name, uint32_t period, uint32_t reload, void *param, void (*timer_callback)(void *));
int audio_timer_control(void *timer, uint32_t new_period, uint32_t over_time);
int audio_timer_start(void *timer);
int audio_timer_stop(void *timer);
int audio_timer_delete(void *timer);
void audio_free(void *ptr);
void audio_free_uncache(void *ptr);
void *audio_calloc(size_t nmemb, size_t size);
void *audio_realloc(void *ptr, size_t size);
void *audio_malloc_uncache(size_t size);
void *audio_device_open(const uint32_t dev_id, int flag);
int audio_device_control(void *dev, uint32_t cmd, void *arg);
int audio_device_close(void *dev);
unsigned long audio_device_write(void *dev, char *data, unsigned long frames);
unsigned long audio_device_read(void *dev, char *data, unsigned long frames);
void audio_device_set_vol(void *dev, uint32_t vol);
int audio_device_get_vol(void *dev);
void audio_device_set_gain(void *dev, uint32_t dB);
int audio_device_get_gain(void *dev);
void *audio_open_dsp(uint32_t freq);
int32_t audio_ctrl_dsp(void *dsp, int cmd, void *arg);
void audio_close_dsp(void *dsp);
void audio_sleep(uint32_t ms);
void audio_cache_ops(int ops, void *addr, int size);
int audio_fopen(char *path, char *mode);
int audio_fread(void *buffer, size_t size, size_t count, int stream);
int audio_fwrite(const void *buffer, size_t size, size_t count, int stream);
int audio_fclose(int fd);
int audio_fsync(int fd);
uint32_t audio_fstat(char *file_path, int stream);
uint32_t audio_ftell(int stream);
int audio_fseek(int stream, int32_t offset, uint32_t pos);

int check_native_audio_type(char *target, char *type);

#endif  /* _AUDIO_CONFIG_H */
