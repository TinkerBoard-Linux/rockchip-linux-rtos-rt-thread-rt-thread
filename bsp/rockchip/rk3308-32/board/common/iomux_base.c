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

#ifdef RT_USING_UART2
RT_WEAK  void uart2_m1_iomux_config(void)
{
    /* UART2 M1 RX-4D2 TX-4D3 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK4,
                         GPIO_PIN_D2 |
                         GPIO_PIN_D3,
                         PIN_CONFIG_MUX_FUNC2);
}
#endif

#ifdef RT_USING_UART4
RT_WEAK  void uart4_m0_iomux_config(void)
{
    /* UART4 M0 RX-4B0 TX-4B1 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK4,
                         GPIO_PIN_B0 |
                         GPIO_PIN_B1,
                         PIN_CONFIG_MUX_FUNC1);
}
#endif

#ifdef RT_USING_I2C0
RT_WEAK  void i2c0_m0_iomux_config(void)
{
    /* I2C0 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK1,
                         GPIO_PIN_D0 |
                         GPIO_PIN_D1,
                         PIN_CONFIG_MUX_FUNC2);
}
#endif

#ifdef RT_USING_I2C1
RT_WEAK  void i2c1_m0_iomux_config(void)
{
    /* I2C1 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK0,
                         GPIO_PIN_B3 |
                         GPIO_PIN_B4,
                         PIN_CONFIG_MUX_FUNC1);
}
#endif

#ifdef RT_USING_I2C2
RT_WEAK  void i2c2_m0_iomux_config(void)
{
    /* I2C2 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_A2 |
                         GPIO_PIN_A3,
                         PIN_CONFIG_MUX_FUNC3);

    /* set SOC_CON13 sel plus: GPIO2_A2 IOMUX FUN3 (I2C2_SDA) */
    WRITE_REG_MASK_WE(GRF->SOC_CON13,
                     GRF_SOC_CON13_GPIO2A2_SEL_SRC_CTRL_MASK | GRF_SOC_CON13_GPIO2A2_SEL_PLUS_MASK,
                     GRF_SOC_CON13_GPIO2A2_SEL_SRC_CTRL_MASK | (3 << GRF_SOC_CON13_GPIO2A2_SEL_PLUS_SHIFT));

    /* set SOC_CON13 sel plus: GPIO2_A3 IOMUX FUN3 (I2C2_SCL) */
    WRITE_REG_MASK_WE(GRF->SOC_CON13,
                     GRF_SOC_CON13_GPIO2A3_SEL_SRC_CTRL_MASK | GRF_SOC_CON13_GPIO2A3_SEL_PLUS_MASK,
                     GRF_SOC_CON13_GPIO2A3_SEL_SRC_CTRL_MASK | (3 << GRF_SOC_CON13_GPIO2A3_SEL_PLUS_SHIFT));
}
#endif

#ifdef RT_USING_SPI0
RT_WEAK  void spi0_m0_iomux_config(void)
{
    /* I2C2 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_A0 |
                         GPIO_PIN_A1 |
                         GPIO_PIN_A2 |
                         GPIO_PIN_A3,
                         PIN_CONFIG_MUX_FUNC2);
}
#endif

#ifdef RT_USING_AUDIO_CARD_I2S0
RT_WEAK void i2s0_8ch_m0_iomux_config(void)
{
    /* I2S0 8CH */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK2,
                         GPIO_PIN_A4 |  // I2S0_MCLK(8CH)
                         GPIO_PIN_A5 |  // I2S0_SCLK_TX(8CH)
                         GPIO_PIN_A6 |  // I2S0_SCLK_RX(8CH)
                         GPIO_PIN_A7 |  // I2S0_LRCK_TX(8CH)
                         GPIO_PIN_B0 |  // I2S0_LRCK_RX(8CH)
                         GPIO_PIN_B1 |  // I2S0_SDO0(8CH)
                         GPIO_PIN_B5,   // I2S0_SDI0(8CH)
                         PIN_CONFIG_MUX_FUNC1);
}
#endif

#ifdef RT_USING_AUDIO_CARD_I2S1
RT_WEAK void i2s1_8ch_m0_iomux_config(void)
{
    /* I2S1 8CH M0 */
    HAL_PINCTRL_SetIOMUX(GPIO_BANK1,
                         GPIO_PIN_A2 |  // I2S1_MCLK_M0(8CH)
                         GPIO_PIN_A3 |  // I2S1_SCLK_TX_M0(8CH)
                         GPIO_PIN_A4 |  // I2S1_SCLK_RX_M0(8CH)
                         GPIO_PIN_A5 |  // I2S1_LRCK_TX_M0(8CH)
                         GPIO_PIN_A6 |  // I2S1_LRCK_RX_M0(8CH)
                         GPIO_PIN_A7 |  // I2S1_SDO0_M0(8CH)
                         GPIO_PIN_B3,   // I2S1_SDI0_M0(8CH)
                         PIN_CONFIG_MUX_FUNC2);
}
#endif

/**
 * @brief  Config iomux for RK3308
 */
RT_WEAK  void rt_hw_iomux_config(void)
{
#ifdef RT_USING_UART2
    uart2_m1_iomux_config();
#endif
}
