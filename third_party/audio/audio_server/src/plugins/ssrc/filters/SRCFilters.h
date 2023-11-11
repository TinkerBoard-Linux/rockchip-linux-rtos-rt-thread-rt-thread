/**
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************
 * @file    SRCFilters.h
 * @author  Jair Wu
 * @version V0.1
 * @date    19-12-2019
 * @brief   SRC filters
 ******************************************************************************/

/****************************************************************************
 *
 * The actual polyphase filters.  These filters can have any number of
 * polyphases but must have NUMTAPS taps.
 *
 ****************************************************************************/

#include "filters/8k_to_16k_16bit_2ch.h" // 8k to 16k, 22k to 44k and  24k to 48k use same filter
#include "filters/11k_to_16k_16bit_2ch.h"
#include "filters/22k_to_16k_16bit_2ch.h"
#include "filters/24k_to_16k_16bit_2ch.h"
#include "filters/32k_to_16k_16bit_2ch.h"
#include "filters/44k_to_16k_16bit_2ch.h"
#include "filters/48k_to_16k_16bit_2ch.h"

#include "filters/48k_to_16k_16bit_4ch.h"

#include "filters/8k_to_44k_16bit_2ch.h"
#include "filters/11k_to_44k_16bit_2ch.h"
#include "filters/16k_to_44k_16bit_2ch.h"
#include "filters/32k_to_44k_16bit_2ch.h"
#include "filters/48k_to_44k_16bit_2ch.h"

#include "filters/8k_to_48k_16bit_2ch.h"
#include "filters/11k_to_48k_16bit_2ch.h"
#include "filters/16k_to_48k_16bit_2ch.h"
#include "filters/22k_to_48k_16bit_2ch.h"
#include "filters/32k_to_48k_16bit_2ch.h"
#include "filters/44k_to_48k_16bit_2ch.h"

#define _8K_TO_16K      0x00020000
#define _11K_TO_16K     0x00017384
#define _22K_TO_16K     0x0000b9c2
#define _24K_TO_16K     0x0000aaaa
#define _32K_TO_16K     0x00008000
#define _44K_TO_16K     0x00005CE1
#define _48K_TO_16K     0x00005555

#define _8K_TO_44K      0x00058333
#define _11K_TO_44K     0x00040000
#define _16K_TO_44K     0x0002c199
#define _32K_TO_44K     0x000160cc
#define _48K_TO_44K     0x0000eb33

#define _8K_TO_48K      0x00060000
#define _11K_TO_48K     0x00045a8e
#define _16K_TO_48K     0x00030000
#define _22K_TO_48K     0x00022d47
#define _32K_TO_48K     0x00018000
#define _44K_TO_48K     0x000116a3

