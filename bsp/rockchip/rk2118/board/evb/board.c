/*
 * Copyright (c) 2024 Rockchip Electronics Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-02-07     Cliff Chen   first implementation
 */

#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include "drv_clock.h"
#include "drv_uart.h"
#include "drv_cache.h"
#include "drv_heap.h"
#include "hal_base.h"
#include "hal_bsp.h"
#include "iomux.h"
#ifdef RT_USING_I2C
#include "drv_i2c.h"
#endif

#ifdef RT_USING_PWM_REGULATOR
#include "drv_pwm_regulator.h"
#endif
#ifdef HAL_PWR_MODULE_ENABLED
#include "drv_regulator.h"
#endif

const struct clk_init clk_inits[] =
{
    INIT_CLK("PLL_GPLL", PLL_GPLL, 800000000),
    INIT_CLK("PLL_VPLL0", PLL_VPLL0, 1179648000),
    INIT_CLK("PLL_VPLL1", PLL_VPLL1, 903168000),
    INIT_CLK("PLL_GPLL_DIV", PLL_GPLL_DIV, 800000000),
    INIT_CLK("PLL_VPLL0_DIV", PLL_VPLL0_DIV, 294912000),
    INIT_CLK("PLL_VPLL1_DIV", PLL_VPLL1_DIV, 150528000),
    INIT_CLK("CLK_DSP0_SRC", CLK_DSP0_SRC, 400000000),
    INIT_CLK("CLK_DSP1", CLK_DSP1, 200000000),
    INIT_CLK("CLK_DSP2", CLK_DSP2, 200000000),
    INIT_CLK("ACLK_NPU", ACLK_NPU, 400000000),
    INIT_CLK("HCLK_NPU", HCLK_NPU, 150000000),
    INIT_CLK("CLK_STARSE0", CLK_STARSE0, 400000000),
    INIT_CLK("CLK_STARSE1", CLK_STARSE1, 400000000),
    INIT_CLK("ACLK_BUS", ACLK_BUS, 300000000),
    INIT_CLK("HCLK_BUS", HCLK_BUS, 150000000),
    INIT_CLK("PCLK_BUS", PCLK_BUS, 150000000),
    INIT_CLK("ACLK_HSPERI", ACLK_HSPERI, 150000000),
    INIT_CLK("ACLK_PERIB", ACLK_PERIB, 150000000),
    INIT_CLK("HCLK_PERIB", HCLK_PERIB, 150000000),
    INIT_CLK("CLK_INT_VOICE0", CLK_INT_VOICE0, 49152000),
    INIT_CLK("CLK_INT_VOICE1", CLK_INT_VOICE1, 45158400),
    INIT_CLK("CLK_INT_VOICE2", CLK_INT_VOICE2, 98304000),
    INIT_CLK("CLK_FRAC_UART0", CLK_FRAC_UART0, 64000000),
    INIT_CLK("CLK_FRAC_UART1", CLK_FRAC_UART1, 11289600),
    INIT_CLK("CLK_FRAC_VOICE0", CLK_FRAC_VOICE0, 24576000),
    INIT_CLK("CLK_FRAC_VOICE1", CLK_FRAC_VOICE1, 22579200),
    INIT_CLK("CLK_FRAC_COMMON0", CLK_FRAC_COMMON0, 12288000),
    INIT_CLK("CLK_FRAC_COMMON1", CLK_FRAC_COMMON1, 11289600),
    INIT_CLK("CLK_FRAC_COMMON2", CLK_FRAC_COMMON2, 8192000),
    INIT_CLK("PCLK_PMU", PCLK_PMU, 100000000),
    INIT_CLK("CLK_32K_FRAC", CLK_32K_FRAC, 32768),
    INIT_CLK("CLK_MAC_OUT", CLK_MAC_OUT, 50000000),
    /* Audio */
    INIT_CLK("MCLK_PDM", MCLK_PDM, 100000000),
    INIT_CLK("CLKOUT_PDM", CLKOUT_PDM, 3072000),
    INIT_CLK("MCLK_SPDIFTX", MCLK_SPDIFTX, 6144000),
    INIT_CLK("MCLK_OUT_SAI0", MCLK_OUT_SAI0, 12288000),
    INIT_CLK("MCLK_OUT_SAI1", MCLK_OUT_SAI1, 12288000),
    INIT_CLK("MCLK_OUT_SAI2", MCLK_OUT_SAI2, 12288000),
    INIT_CLK("MCLK_OUT_SAI3", MCLK_OUT_SAI3, 12288000),
    INIT_CLK("MCLK_OUT_SAI4", MCLK_OUT_SAI4, 12288000),
    INIT_CLK("MCLK_OUT_SAI5", MCLK_OUT_SAI5, 12288000),
    INIT_CLK("MCLK_OUT_SAI6", MCLK_OUT_SAI6, 12288000),
    INIT_CLK("MCLK_OUT_SAI7", MCLK_OUT_SAI7, 12288000),
    INIT_CLK("SCLK_SAI0", SCLK_SAI0, 12288000),
    INIT_CLK("SCLK_SAI1", SCLK_SAI1, 12288000),
    INIT_CLK("SCLK_SAI2", SCLK_SAI2, 12288000),
    INIT_CLK("SCLK_SAI3", SCLK_SAI3, 12288000),
    INIT_CLK("SCLK_SAI4", SCLK_SAI4, 12288000),
    INIT_CLK("SCLK_SAI5", SCLK_SAI5, 12288000),
    INIT_CLK("SCLK_SAI6", SCLK_SAI6, 12288000),
    INIT_CLK("SCLK_SAI7", SCLK_SAI7, 12288000),
    { /* sentinel */ },
};

#ifdef RT_USING_I2C
const struct rockchip_i2c_config rockchip_i2c_config_table[] =
{
    {
        .id = I2C0,
        .speed = I2C_100K,
    },
    { /* sentinel */ }
};
#endif

#if defined(RT_USING_UART0)
const struct uart_board g_uart0_board =
{
    .baud_rate = UART_BR_1500000,
    .dev_flag = ROCKCHIP_UART_SUPPORT_FLAG_DEFAULT,
    .bufer_size = RT_SERIAL_RB_BUFSZ,
    .name = "uart0",
};
#endif /* RT_USING_UART0 */

#if defined(RT_USING_UART2)
const struct uart_board g_uart2_board =
{
    .baud_rate = UART_BR_1500000,
    .dev_flag = ROCKCHIP_UART_SUPPORT_FLAG_DEFAULT,
    .bufer_size = RT_SERIAL_RB_BUFSZ,
    .name = "uart2",
};
#endif /* RT_USING_UART2 */

#ifdef RT_USING_UNCACHE_HEAP
extern const rt_uint32_t __uncache_heap_start__[];
extern const rt_uint32_t __uncache_heap_end__[];
#endif
#ifdef RT_USING_LARGE_HEAP
extern const rt_uint32_t __large_heap_start__[];
extern const rt_uint32_t __large_heap_end__[];
#endif
#ifdef __ARMCC_VERSION
extern const rt_uint32_t Image$$ARM_LIB_HEAP$$Limit[];
extern const rt_uint32_t Image$$ARM_LIB_STACK$$Base[];
#define HEAP_START       Image$$ARM_LIB_HEAP$$Limit
#define HEAP_END         Image$$ARM_LIB_STACK$$Base
#else
extern const rt_uint32_t __ddr_heap_begin__[];
extern const rt_uint32_t __ddr_heap_end__[];
extern const rt_uint32_t __heap_begin__[];
extern const rt_uint32_t __heap_end__[];
#ifdef RK2118_CPU_CORE0
#define HEAP_START       (__ddr_heap_begin__)
#define HEAP_END         (__ddr_heap_end__)
#else
#define HEAP_START       (__heap_begin__)
#define HEAP_END         (__heap_end__)
#endif
#endif

#ifdef HAL_PWR_MODULE_ENABLED
#ifdef RT_USING_PWM_REGULATOR
RT_WEAK struct pwr_pwm_info_desc pwm_pwr_desc[] =
{
    {
        .name = "pwm0",
        .chanel = 3,
        .invert = true,
    },
    {
        .name = "pwm0",
        .chanel = 2,
        .invert = true,
    },
    { /* sentinel */ },
};

RT_WEAK struct regulator_desc regulators[] =
{
    {
        .flag = REGULATOR_FLG_PWM | REGULATOR_FLG_LOCK,
        .desc.pwm_desc = {
            .flag = DESC_FLAG_LINEAR(PWR_CTRL_VOLT_RUN | PWR_CTRL_PWR_EN),
            .info = {
                .pwrId = PWR_ID_CORE,
            },
            .pwrId = PWR_ID_CORE,
            .period = 25000,
            .minVolt = 810000,
            .maxVlot = 1000000,
            .voltage = 900000,
            .pwm = &pwm_pwr_desc[0],
        },
    },
    {
        .flag = REGULATOR_FLG_PWM | REGULATOR_FLG_LOCK,
        .desc.pwm_desc = {
            .flag = DESC_FLAG_LINEAR(PWR_CTRL_VOLT_RUN | PWR_CTRL_PWR_EN),
            .info = {
                .pwrId = PWR_ID_DSP_CORE,
            },
            .pwrId = PWR_ID_DSP_CORE,
            .period = 25000,
            .minVolt = 800000,
            .maxVlot = 1100000,
            .voltage = 900000,
            .pwm = &pwm_pwr_desc[1],
        },
    },
    { /* sentinel */ },
};

RT_WEAK const struct regulator_init regulator_inits[] =
{
    REGULATOR_INIT("vdd_core",  PWR_ID_CORE,       900000, 1, 900000, 1),
    REGULATOR_INIT("vdd_dsp",   PWR_ID_DSP_CORE,   900000, 1, 900000, 1),
    { /* sentinel */ },
};
#endif
#endif

/**
 * This function will initial Pisces board.
 */
void rt_hw_board_init()
{
    /* mpu init */
    mpu_init();

    /* HAL_Init */
    HAL_Init();

    /* hal bsp init */
    BSP_Init();

    /* System tick init */
    rt_hw_interrupt_install(SysTick_IRQn, systick_isr, RT_NULL, "tick");
    HAL_SetTickFreq(1000 / RT_TICK_PER_SECOND);
    HAL_SYSTICK_Init();

#ifdef RT_USING_PIN
    rt_hw_iomux_config();
#endif

    rt_system_heap_init((void *)HEAP_START, (void *)HEAP_END);
#ifdef RT_USING_UNCACHE_HEAP
    rt_uncache_heap_init((void *)__uncache_heap_start__, (void *)__uncache_heap_end__);
#endif
#ifdef RT_USING_LARGE_HEAP
    rt_large_heap_init((void *)__large_heap_start__, (void *)__large_heap_end__);
#endif

    /* Initial usart deriver, and set console device */
    rt_hw_usart_init();

#ifdef RT_USING_PWM_REGULATOR
#ifdef HAL_PWR_MODULE_ENABLED
    regulator_desc_init(regulators);
#endif
#endif

    clk_init(clk_inits, true);

    /* Update system core clock after clk_init */
    SystemCoreClockUpdate();

#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

#ifdef RT_USING_SWO_PRINTF
    rt_console_set_output_hook(swo_console_hook);
#endif

    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}
