/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef _AUDIO_MP3_HAL_H_
#define _AUDIO_MP3_HAL_H_

#include "AudioConfig.h"

int32_t AudioDecMP3Close(void);
int32_t AudioDecMP3Open(uint32_t A2B_ShareDatAddr);
int32_t AudioDecMP3Process(uint32_t decode_dat);

#endif        // _AUDIO_MP3_HAL_H_
