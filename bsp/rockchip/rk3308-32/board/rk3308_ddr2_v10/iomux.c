/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-09     Cliff chen   first implementation
 */

#include "rtdef.h"
#include "iomux.h"
#include "hal_base.h"

/**
 * @brief  Config io domian for board of rk3308_ddr2p116sd4_v10
 */

void rt_hw_iodomain_config(void)
{
    /* VCC IO 2 voltage select 1v8 */
    GRF->SOC_CON0 = (1 << GRF_SOC_CON0_IO_VSEL2_SHIFT) |
                    (GRF_SOC_CON0_IO_VSEL2_MASK << 16);
}

void spkmute_iomux_config(void)
{
    /* SPK_MUTE is GPIO0_A5 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK0,
                         GPIO_PIN_A5,
                         PIN_CONFIG_MUX_FUNC0);

    HAL_GPIO_SetPinDirection(GPIO0, GPIO_PIN_A5, GPIO_OUT);
    HAL_GPIO_SetPinLevel(GPIO0, GPIO_PIN_A5, GPIO_HIGH);
}

/**
 * @brief  Config iomux for RK3308
 */
void rt_hw_iomux_config(void)
{
    rt_hw_iodomain_config();
    uart2_m1_iomux_config();
    uart4_m0_iomux_config();
    i2c1_m0_iomux_config();
    i2s0_8ch_m0_iomux_config();
    spkmute_iomux_config();
}
