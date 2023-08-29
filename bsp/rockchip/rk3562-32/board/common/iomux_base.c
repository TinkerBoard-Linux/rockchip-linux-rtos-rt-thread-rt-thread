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

#ifdef RT_USING_UART0
RT_WEAK  void uart0_m0_iomux_config(void)
{
    HAL_PINCTRL_SetIOMUX(GPIO_BANK0,
                         GPIO_PIN_D0 |
                         GPIO_PIN_D1,
                         PIN_CONFIG_MUX_FUNC1);
}
#endif

/**
 * @brief  Config iomux for RK3562
 */
RT_WEAK  void rt_hw_iomux_config(void)
{
#ifdef RT_USING_UART0
    uart0_m0_iomux_config();
#endif
}
