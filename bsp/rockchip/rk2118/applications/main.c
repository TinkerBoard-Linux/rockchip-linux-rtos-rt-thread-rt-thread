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
    uint32_t value1, value2;
    uint32_t start, time1, time2;

    for (int i = 0; i < 100; i++)
    {
        start = SysTick->VAL;
        value1 = SAI0->VERSION;
        time1 = start - SysTick->VAL;

        start = SysTick->VAL;
        value2 = TIMER0->CURRENT_VALUE[0];
        time2 = start - SysTick->VAL;

        rt_kprintf("read register latency: %d(value=0x%x), %d(value=0x%x)\n",
                   time1, value1, time2, value2);
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
    //tfm_ns_interface_init();
    //psa_framework_version();
    return 0;
}

extern void coremark_main(void);
extern void linpack_main(void);
void cpu_stress(int argc, char **argv)
{
    uint32_t loop = 1;
    uint32_t i = 0;
    uint32_t *ptr;

    if (argc == 2)
        loop = atoi(argv[1]);

    rt_kprintf("loop=%d\n", loop);
    while (i++ < loop)
    {
        ptr = (uint32_t *)rt_malloc(20 * 1024);
        RT_ASSERT(ptr);
        memset(ptr, 0x55, 20 * 1024);
        rt_free(ptr);
#ifdef RT_USING_COREMARK
        coremark_main();
#endif
#ifdef RT_USING_LINPACK
        linpack_main();
#endif
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(cpu_stress, cpu stress test);
#endif
