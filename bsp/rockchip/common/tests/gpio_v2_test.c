/**
  * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  ******************************************************************************
  * @file    gpio_test_rk3308.c
  * @author  tony.zheng
  * @version V0.1
  * @date    2022/8/19
  * @brief   gpio test
  *
  ******************************************************************************
  */

#include <rtthread.h>
#include <rtdevice.h>

#if defined(RT_USING_COMMON_TEST_GPIO) && defined(RT_USING_COMMON_TEST_GPIO_V2)

#include <unistd.h>
#include <stdlib.h>
#include "hal_base.h"

#define USE_DEVICE_OPS
#define DEV_NAME "pin"

struct TGPIO_INFO
{
    char *desc;
    struct GPIO_REG *gpio;
    uint32_t bank;
};

struct TGPIO_INFO gpios[] =
{
    { "gpio0", GPIO0, GPIO_BANK0 },
#ifdef GPIO1
    { "gpio1", GPIO1, GPIO_BANK1 },
#endif
#ifdef GPIO2
    { "gpio2", GPIO2, GPIO_BANK2 },
#endif
#ifdef GPIO3
    { "gpio3", GPIO3, GPIO_BANK3 },
#endif
#ifdef GPIO4
    { "gpio4", GPIO4, GPIO_BANK4 },
#endif
};
#define GPIO_BANK_NUM (HAL_ARRAY_SIZE(gpios))

#define PIN_NUM(p) ((p & GPIO_PIN_MASK) >> GPIO_PIN_SHIFT)
#define PIN_BANK(p) ((p & GPIO_BANK_MASK) >> GPIO_BANK_SHIFT)

static uint32_t ouput_level_table[2] =
{
    PIN_LOW,
    PIN_HIGH
};

static uint32_t output_drven_table[4] =
{
    PIN_CONFIG_DRV_LEVEL0,
    PIN_CONFIG_DRV_LEVEL1,
    PIN_CONFIG_DRV_LEVEL2,
    PIN_CONFIG_DRV_LEVEL3
};

static uint32_t input_mode_table[3] =
{
    PIN_MODE_INPUT,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_INPUT_PULLDOWN,
};

static uint32_t irq_mode_table[5] =
{
    PIN_IRQ_MODE_RISING,
    PIN_IRQ_MODE_FALLING,
    PIN_IRQ_MODE_RISING_FALLING,
    PIN_IRQ_MODE_HIGH_LEVEL,
    PIN_IRQ_MODE_LOW_LEVEL,
};

#ifdef USE_DEVICE_OPS
static rt_device_t pin_dev = RT_NULL;
#endif
static struct rt_device_pin_mode   pin_mode;
static struct rt_device_pin_status pin_status;
static uint32_t isr_flag = 0;

static void irq_callback(void *args)
{
    struct rt_device_pin_status *pstatus = (struct rt_device_pin_status *)args;

    isr_flag = 1;
#ifdef USE_DEVICE_OPS
    rt_device_read(pin_dev, 0, pstatus, sizeof(struct rt_device_pin_status));
#else
    pstatus->status = rt_pin_read(pstatus->pin);
#endif
    rt_pin_detach_irq(pstatus->pin);
    rt_pin_irq_enable(pstatus->pin, PIN_IRQ_DISABLE);

    rt_kprintf("isr: gpio%d pin%d input level %d\n", PIN_BANK(pstatus->pin), PIN_NUM(pstatus->pin), pstatus->status);
}

static void gpio_test(uint32_t bank, uint32_t pin, uint32_t dir, uint32_t parm1, uint32_t parm2)
{
    rt_err_t ret;
    uint32_t i;

    rt_kprintf("bank = %d, pin = %d, dir = %d, parm1 = %d, parm2 = %d\n",
               bank, pin, dir, parm1, parm2);

    pin_mode.pin = BANK_PIN(bank, pin);
    pin_status.pin = BANK_PIN(bank, pin);

#ifdef USE_DEVICE_OPS
    pin_dev = rt_device_find(DEV_NAME);
    if (pin_dev == RT_NULL)
    {
        rt_kprintf("find device '%s' error!\n", DEV_NAME);
        return;
    }
    ret = rt_device_open(pin_dev, RT_DEVICE_FLAG_RDWR);
    if (ret != RT_EOK)
    {
        rt_kprintf("open device '%s' error!\n", DEV_NAME);
        return;
    }
#endif
    if (dir)
    {
        HAL_PINCTRL_SetParam(bank, 0x01UL << pin, output_drven_table[parm2]);

        pin_mode.mode = PIN_MODE_OUTPUT;
#ifdef USE_DEVICE_OPS
        rt_device_control(pin_dev, 0, &pin_mode);

        pin_status.status = ouput_level_table[parm1];
        rt_device_write(pin_dev, 0, &pin_status, sizeof(struct rt_device_pin_status));
#else
        rt_pin_mode(pin_mode.pin, pin_mode.mode);
        rt_pin_write(pin_mode.pin, ouput_level_table[parm1]);
#endif
        rt_kprintf("gpio%d pin%d output level %d\n", bank, pin, parm1);
    }
    else
    {
        pin_mode.mode = input_mode_table[parm1];
#ifdef USE_DEVICE_OPS
        rt_device_control(pin_dev, 0, &pin_mode);
#else
        rt_pin_mode(pin_mode.pin, pin_mode.mode);
#endif
        rt_pin_attach_irq(pin_mode.pin, irq_mode_table[parm2], irq_callback, (void *)&pin_status);

        rt_pin_irq_enable(pin_mode.pin, PIN_IRQ_ENABLE);

        rt_kprintf("wait for gpio%d pin%d input...\n", bank, pin);

        //wait isr
        isr_flag = 0;
        for (i = 0; i < 60; i++)
        {
            HAL_DelayMs(1000);
            if (isr_flag)
            {
                break;
            }
        }
        if (i >= 60)
        {
            rt_kprintf("wait for gpio%d pin%d input TIMEOUT!\n", bank, pin);
            rt_pin_detach_irq(pin_mode.pin);
            rt_pin_irq_enable(pin_mode.pin, PIN_IRQ_DISABLE);
        }
    }
}

static void tgpio_main(int argc, char **argv)
{
    char *pstr;
    uint32_t i;
    uint32_t bank  = (uint32_t) -1;
    uint32_t pin   = (uint32_t) -1;
    uint32_t dir   = (uint32_t) -1;
    uint32_t parm1 = (uint32_t) -1;
    uint32_t parm2 = (uint32_t) -1;

    if (argc > 5)
    {
        for (i = 0; i < GPIO_BANK_NUM; i++)
        {
            if (!strcmp(gpios[i].desc, argv[1]))
            {
                bank = gpios[i].bank;
                break;
            }
        }
        if (i < GPIO_BANK_NUM)
        {
            // Get pin num
            pin  = strtol(argv[2], &pstr, 10);
            if ((*pstr != 0) || (pin >= 32))
            {
                goto usage;
            }

            // Get dir
            if (!strcmp("-o", argv[3]))
            {
                dir = 1;

                // Get parm1: output level (0~1)
                parm1  = strtol(argv[4], &pstr, 10);
                if ((*pstr != 0)  || (parm1 > 1))
                {
                    goto usage;
                }

                // Get param2: output driven (0~3)
                parm2  = strtol(argv[5], &pstr, 10);
                if ((*pstr != 0)  || (parm1 > 3))
                {
                    goto usage;
                }
            }
            else if (!strcmp("-i", argv[3]))
            {
                dir = 0;

                // Get parm1: input pull mode (0~2)
                parm1  = strtol(argv[4], &pstr, 10);
                if ((*pstr != 0)  || (parm1 > 2))
                {
                    goto usage;
                }

                // Get param2: input irq mode (0~4)
                parm2  = strtol(argv[5], &pstr, 10);
                if ((*pstr != 0)  || (parm1 > 4))
                {
                    goto usage;
                }
            }
            else
            {
                goto usage;
            }

            gpio_test(bank, pin, dir, parm1, parm2);

            return;
        }
    }

usage:
    rt_kprintf("out:usage: tgpio <group> <pin_num> <dir> <param1> <param2>\n\n");
    rt_kprintf("  group:   group            (gpio0 ~ 4)\n");
    rt_kprintf("  pin_num: pin number       (0 ~ 31)\n");
    rt_kprintf("  dir:     direction        (-o, -i)\n");
    rt_kprintf("  param1:  output level     (0:low 1:high)\n");
    rt_kprintf("           input pull mode  (0:normal 1:up 2:down)\n");
    rt_kprintf("  param2:  output driving level (0 ~ 3)\n");
    rt_kprintf("           input irq mode   (0:rising 1:falling 2:edge 4:low 5:high)\n\n");
    rt_kprintf("example(output): tgpio gpio0 24 -o 1 2\n");
    rt_kprintf("example(input):  tgpio gpio0 24 -i 1 0\n\n");
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(tgpio_main, tgpio, gpio test cmd);
#endif
#endif
