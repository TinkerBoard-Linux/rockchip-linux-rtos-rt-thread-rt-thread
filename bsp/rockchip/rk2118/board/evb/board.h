/*
 * Copyright (c) 2024 Rockchip Electronics Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-07     Cliff Chen   first implementation
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include "board_base.h"

#define RT_USING_CODEC_TAS5805M

#ifdef RT_USING_CODEC_TAS5805M
#define TAS5805M_I2C_BUS "i2c0"
#define TAS5805M_SDB_GPIO_BANK GPIO_BANK3
#define TAS5805M_SDB_GPIO      GPIO_PIN_A4
#define TAS5805M_SDB_GPIO_GRP  GPIO3
#define TAS5805M_ADR_GPIO_BANK GPIO_BANK3
#define TAS5805M_ADR_GPIO      GPIO_PIN_A5
#define TAS5805M_ADR_GPIO_GRP  GPIO3
#endif

#endif
