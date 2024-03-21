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
#include "drv_thermal.h"
#include "hal_base.h"
#include "hal_bsp.h"
#include "iomux.h"

#ifdef RT_USING_USB_DEVICE
#include "drv_usbd.h"
#endif

RT_WEAK const struct clk_init clk_inits[] =
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
    INIT_CLK("CLK_FRAC_UART1", CLK_FRAC_UART1, 48000000),
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
    INIT_CLK("CLK_TSADC", CLK_TSADC, 1200000),
    INIT_CLK("CLK_TSADC_TSEN", CLK_TSADC_TSEN, 12000000),
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

#if defined(RT_USING_TSADC)
RT_WEAK const struct tsadc_init g_tsadc_init =
{
    .chn_id = {0},
    .chn_num = 1,
    .polarity = TSHUT_LOW_ACTIVE,
    .mode = TSHUT_MODE_CRU,
};
#endif /* RT_USING_TSADC */

#if defined(RT_USING_UART0)
RT_WEAK const struct uart_board g_uart0_board =
{
    .baud_rate = UART_BR_1500000,
    .dev_flag = ROCKCHIP_UART_SUPPORT_FLAG_DEFAULT,
    .bufer_size = RT_SERIAL_RB_BUFSZ,
    .name = "uart0",
};
#endif /* RT_USING_UART0 */

#if defined(RT_USING_UART1)
RT_WEAK const struct uart_board g_uart1_board =
{
    .baud_rate = UART_BR_1500000,
    .dev_flag = ROCKCHIP_UART_SUPPORT_FLAG_DEFAULT,
    .bufer_size = RT_SERIAL_RB_BUFSZ,
    .name = "uart1",
};
#endif /* RT_USING_UART1 */

RT_WEAK void systick_isr(int vector, void *param)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_SYSTICK_IRQHandler();
    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}

extern const rt_uint32_t __code_start__[];
extern const rt_uint32_t __code_end__[];
extern const rt_uint32_t __data_start__[];
extern const rt_uint32_t __data_end__[];
extern const rt_uint32_t __device_start__[];
extern const rt_uint32_t __device_end__[];
extern const rt_uint32_t __ddr_start__[];
extern const rt_uint32_t __ddr_end__[];
#ifdef RT_USING_UNCACHE_HEAP
extern const rt_uint32_t __uncache_heap_start__[];
extern const rt_uint32_t __uncache_heap_end__[];
#endif
extern const rt_uint32_t __spi2apb_buffer_start__[];
extern const rt_uint32_t __spi2apb_buffer_end__[];
RT_WEAK void mpu_init(void)
{
    /* text section: non shared, ro, np, exec, cachable */
    ARM_MPU_SetRegion(0, ARM_MPU_RBAR((rt_uint32_t)__code_start__, 0U, 1U, 1U, 0U), ARM_MPU_RLAR((rt_uint32_t)__code_end__, 0U));
    /* data section: non shared, rw, np, exec, cachable */
    ARM_MPU_SetRegion(1, ARM_MPU_RBAR((rt_uint32_t)__data_start__, 0U, 0U, 1U, 0U), ARM_MPU_RLAR((rt_uint32_t)__data_end__, 1U));
    /* device section: shared, rw, np, xn */
    ARM_MPU_SetRegion(2, ARM_MPU_RBAR((rt_uint32_t)__device_start__, 1U, 0U, 1U, 1U), ARM_MPU_RLAR((rt_uint32_t)__device_end__, 2U));

    /* cachable normal memory, ARM_MPU_ATTR_MEMORY_(NT, WB, RA, WA) */
    ARM_MPU_SetMemAttr(0, ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(0, 0, 1, 0), ARM_MPU_ATTR_MEMORY_(0, 0, 1, 0)));
    ARM_MPU_SetMemAttr(1, ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(1, 1, 1, 0), ARM_MPU_ATTR_MEMORY_(1, 1, 1, 0)));
    /* device memory */
    ARM_MPU_SetMemAttr(2, ARM_MPU_ATTR(ARM_MPU_ATTR_DEVICE, ARM_MPU_ATTR_DEVICE_nGnRnE));

#ifdef RT_USING_UNCACHE_HEAP
    /* uncache heap: non shared, rw, np, exec, uncachable */
    ARM_MPU_SetRegion(3, ARM_MPU_RBAR((rt_uint32_t)__uncache_heap_start__, 0U, 0U, 1U, 1U), ARM_MPU_RLAR((rt_uint32_t)__uncache_heap_end__, 3U));
    /* uncachable normal memory */
    ARM_MPU_SetMemAttr(3, ARM_MPU_ATTR(ARM_MPU_ATTR_NON_CACHEABLE, ARM_MPU_ATTR_NON_CACHEABLE));
#endif
    /* ddr section: non shared, rw, np, exec, cachable */
    ARM_MPU_SetRegion(4, ARM_MPU_RBAR((rt_uint32_t)__ddr_start__, 0U, 0U, 1U, 0U), ARM_MPU_RLAR((rt_uint32_t)__ddr_end__, 4U));
    ARM_MPU_SetMemAttr(4, ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(1, 1, 1, 0), ARM_MPU_ATTR_MEMORY_(1, 1, 1, 0)));

    /* spi2adb buffer: non shared, rw, np, exec, cachable */
    ARM_MPU_SetRegion(5, ARM_MPU_RBAR((rt_uint32_t)__spi2apb_buffer_start__, 0U, 0U, 1U, 0U), ARM_MPU_RLAR((rt_uint32_t)__spi2apb_buffer_end__, 5U));
    ARM_MPU_SetMemAttr(5, ARM_MPU_ATTR(ARM_MPU_ATTR_MEMORY_(1, 1, 1, 0), ARM_MPU_ATTR_MEMORY_(1, 1, 1, 0)));

    ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);
}

#ifdef RT_USING_USB_DEVICE
RT_WEAK struct ep_id g_usb_ep_pool[] =
{
    { 0x0,  USB_EP_ATTR_CONTROL,    USB_DIR_INOUT,  64,   ID_ASSIGNED   },
    { 0x1,  USB_EP_ATTR_BULK,       USB_DIR_IN,     1024, ID_UNASSIGNED },
    { 0x2,  USB_EP_ATTR_BULK,       USB_DIR_OUT,    512,  ID_UNASSIGNED },
    { 0x3,  USB_EP_ATTR_ISOC,       USB_DIR_IN,     1024, ID_UNASSIGNED },
    { 0x4,  USB_EP_ATTR_ISOC,       USB_DIR_OUT,    512,  ID_UNASSIGNED },
    { 0x5,  USB_EP_ATTR_INT,        USB_DIR_IN,     64,   ID_UNASSIGNED },
    { 0x6,  USB_EP_ATTR_INT,        USB_DIR_OUT,    64,   ID_UNASSIGNED },
    { 0xFF, USB_EP_ATTR_TYPE_MASK,  USB_DIR_MASK,   0,    ID_ASSIGNED   },
};
#endif

#ifdef __ARMCC_VERSION
extern const rt_uint32_t Image$$ARM_LIB_HEAP$$Limit[];
extern const rt_uint32_t Image$$ARM_LIB_STACK$$Base[];
#define HEAP_START       Image$$ARM_LIB_HEAP$$Limit
#define HEAP_END         Image$$ARM_LIB_STACK$$Base
#else
extern const rt_uint32_t __heap_begin__[];
extern const rt_uint32_t __heap_end__[];
#define HEAP_START       (__heap_begin__)
#define HEAP_END         (__heap_end__)
#endif

#ifdef RT_USING_SWO_PRINTF
void swo_console_hook(const char *str, int flush)
{
    char ch;

    while (str && *str != '\0')
    {
        ch = *str;
        ITM_SendChar(ch);
        str++;
    }
}
#endif

/**
 * This function will initial Pisces board.
 */
RT_WEAK void rt_hw_board_init()
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

    /* Initial usart deriver, and set console device */
    rt_hw_usart_init();

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

#ifdef __ARMCC_VERSION
extern int $Super$$main(void);
void _start(void)
{
    $Super$$main();
}
#else
extern int entry(void);
void _start(void)
{
    entry();
}
#endif
