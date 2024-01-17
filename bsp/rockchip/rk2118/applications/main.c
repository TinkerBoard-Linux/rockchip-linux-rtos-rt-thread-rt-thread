/**
  * Copyright (c) 2022 Fuzhou Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  ******************************************************************************
  * @file    main.c
  * @version V0.1
  * @brief   application entry
  *
  * Change Logs:
  * Date           Author          Notes
  * 2022-08-04     Cliff.Chen      first implementation
  *
  ******************************************************************************
  */
#include <rtthread.h>
#include "hal_base.h"
#include "tfm_ns_interface.h"

extern uint32_t psa_framework_version(void);

void test_reg_read(void)
{
    uint32_t value0, value1, value2;
    uint32_t start, time0, time1, time2;

    for (int i = 0; i < 100; i++)
    {
        start = SysTick->VAL;
#ifdef INTMUX0
        value0 = INTMUX0->IRQ_FINALSTATUS_L;
#endif
        time0 = start - SysTick->VAL;

        start = SysTick->VAL;
        value1 = SAI0->VERSION;
        time1 = start - SysTick->VAL;

        start = SysTick->VAL;
        value2 = TIMER0->CURRENT_VALUE[0];
        time2 = start - SysTick->VAL;

        rt_kprintf("read register latency: %d(value=0x%x), %d(value=0x%x), %d(value=0x%x)\n",
                   time0, value0, time1, value1, time2, value2);
    }
}

static double g_double = 3.14;
int main(void)
{
    rt_kprintf("delay 1s test start\n");
    rt_thread_delay(1 * RT_TICK_PER_SECOND);
    rt_kprintf("delay 1s test end\n");

    //test_reg_read();

    __asm volatile("vldr d7, %0" : : "m"(g_double) :);
    tfm_ns_interface_init();
    psa_framework_version();
    return 0;
}
