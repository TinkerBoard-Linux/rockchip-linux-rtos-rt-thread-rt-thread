/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef _AUDIO_AMR_ENC_HAL_H_
#define _AUDIO_AMR_ENC_HAL_H_

#include "AudioConfig.h"
#include <stddef.h>
//#include "typedef.h"
// #include "ABCoreShare.h"

#define    AMR_AB_CORE_SHARE_ADDR_INVALID (-1)
#define WMF_MR122_FRAME_SIZE 31
#define SAMPLE_PER_FRAME 160

int32_t AudioAmrEncodeOpen(uint32_t A2B_EncAmrAddr);
int32_t AudioAmrEncodeClose(void);
int32_t AudioAmrEncode(void);

#endif        // _AUDIO_AMR_ENC_HAL_H_
