/**
  * Copyright (c) 2020 Fuzhou Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  ******************************************************************************
  * @file    spinand_test.c
  * @version V1.0
  * @brief   spi nand test
  *
  * Change Logs:
  * Date           Author          Notes
  * 2020-06-17     Dingqiang Lin   the first version
  *
  ******************************************************************************
  */

#include <rtdevice.h>
#include <rtthread.h>

#ifdef RT_USING_COMMON_TEST_SPINAND

#include <drivers/mtd_nand.h>
#include "hal_base.h"

#define NAND_PAGE_SIZE  4096
#define NAND_SPARE_SIZE 64

#ifndef EUCLEAN
#define EUCLEAN                         117
#endif

struct nand_ops_t
{
    uint32_t (*erase_blk)(uint8_t cs, uint32_t page_addr);
    uint32_t (*prog_page)(uint8_t cs, uint32_t page_addr, uint32_t *data, uint32_t *spare);
    uint32_t (*read_page)(uint8_t cs, uint32_t page_addr, uint32_t *data, uint32_t *spare);
};

static uint32_t *pwrite;
static uint32_t *pread;

static uint32_t *pspare_write;
static uint32_t *pspare_read;

static uint32_t bad_blk_num;
static uint32_t bad_page_num;
static struct nand_ops_t g_nand_ops;
struct rt_mtd_nand_device *mtd_dev = RT_NULL;

static void spinand_test_show_usage()
{
    rt_kprintf("1. spinand_test write page_addr page_num\n");
    rt_kprintf("2. spinand_test read page_addr page_nun\n");
    rt_kprintf("3. spinand_test erase blk_addr\n");
    rt_kprintf("3. spinand_test erase_all\n");
    rt_kprintf("4. spinand_test stress loop\n");
    rt_kprintf("like:\n");
    rt_kprintf("\tspinand_test write 1024 1\n");
    rt_kprintf("\tspinand_test read 1024 1\n");
    rt_kprintf("\tspinand_test erase 16\n");
    rt_kprintf("\tspinand_test erase_all\n");
    rt_kprintf("\tspinand_test stress 5000\n");
}

HAL_Status spinand_dbg_hex(char *s, void *buf, uint32_t width, uint32_t len)
{
    uint32_t i, j;
    unsigned char *p8 = (unsigned char *)buf;
    unsigned short *p16 = (unsigned short *)buf;
    uint32_t *p32 = (uint32_t *)buf;

    j = 0;
    for (i = 0; i < len; i++)
    {
        if (j == 0)
        {
            rt_kprintf("[SPI Nand TEST] %s %p + 0x%lx:", s, buf, i * width);
        }

        if (width == 4)
            rt_kprintf("0x%08lx,", p32[i]);
        else if (width == 2)
            rt_kprintf("0x%04x,", p16[i]);
        else
            rt_kprintf("0x%02x,", p8[i]);

        if (++j >= 16)
        {
            j = 0;
            rt_kprintf("\n");
        }
    }
    rt_kprintf("\n");

    return HAL_OK;
}

static uint32_t nand_flash_test(uint32_t blk_begin, uint32_t blk_end, uint32_t is_recheck)
{
    uint32_t i, block, page;
    int ret;
    uint32_t pages_num = 64;
    uint32_t block_addr = 64;
    uint32_t is_bad_blk = 0;
    char *print_tag = is_recheck ? "read" : "prog";
    uint32_t sec_per_page = 4; /* 4 - 2KB; 8 - 4KB */

    bad_blk_num = 0;
    bad_page_num = 0;
    for (block = blk_begin; block < blk_end; block++)
    {
        is_bad_blk = 0;
        rt_kprintf("Flash %s block: %x\n", print_tag, block);
        if (!is_recheck)
            g_nand_ops.erase_blk(0, block * block_addr);
        for (page = 0; page < pages_num; page++)
        {
            for (i = 0; i < sec_per_page * 128; i++)
                pwrite[i] = ((block * block_addr + page) << 16) + i;
            for (i = 0; i < sec_per_page / 2; i++)
                pspare_write[i] = ((block * block_addr + page) << 16) + 0x5AF0 + i;
            pwrite[0] |= 0xff; /* except bad block marker in data area */
            if (!is_recheck)
                g_nand_ops.prog_page(0, block * block_addr + page, pwrite, pspare_write);
            memset(pread, 0, sec_per_page * 512);
            memset(pspare_read, 0, sec_per_page * 2);
            ret = g_nand_ops.read_page(0, block * block_addr + page, pread, pspare_read);
            if (ret)
                is_bad_blk = 1;
            for (i = 0; i < sec_per_page * 128; i++)
            {
                if (pwrite[i] != pread[i])
                {
                    rt_kprintf("Data not macth,W:%x,R:%x,@%x,pageAddr=%x\n",
                               pwrite[i], pread[i], i, block * block_addr + page);
                    is_bad_blk = 1;
                    break;
                }
            }
            if (is_bad_blk)
            {
                bad_page_num++;
                spinand_dbg_hex("data w:", pwrite, 4, sec_per_page * 128);
                spinand_dbg_hex("data r:", pread, 4, sec_per_page * 128);
                while (1)
                    ;
            }
        }
        if (is_bad_blk)
            bad_blk_num++;
    }
    return 0;
}

static void flash_erase_all_block(uint32_t firstblk)
{
    uint32_t blk;
    uint32_t end_blk = mtd_dev->block_end;

    for (blk = firstblk; blk < end_blk; blk++)
    {
        rt_kprintf("erase blk %d\n", blk);
        g_nand_ops.erase_blk(0, blk * mtd_dev->pages_per_block);
    }

}

static uint32_t nand_flash_stress_test(void)
{
    uint32_t ret = 0;

    rt_kprintf("%s\n", __func__);
    flash_erase_all_block(0);
    rt_kprintf("Flash prog/read test\n");
    ret = nand_flash_test(0, mtd_dev->block_end, 0);
    if (ret)
        return -1;
    rt_kprintf("Flash read recheck test\n");
    ret = nand_flash_test(0, mtd_dev->block_end, 1);
    if (ret)
        return -1;
    rt_kprintf("%s success\n", __func__);
    return ret;
}

static uint32_t erase_blk(uint8_t cs, uint32_t page_addr)
{
    if (rt_mtd_nand_erase_block(mtd_dev, page_addr / mtd_dev->pages_per_block))
        return -1;
    else
        return 0;
}

static uint32_t prog_page(uint8_t cs, uint32_t page_addr, uint32_t *data, uint32_t *spare)
{
    int ret;

    ret = rt_mtd_nand_write(mtd_dev, page_addr, (const rt_uint8_t *)data, mtd_dev->page_size, RT_NULL, 0);
    if (ret)
    {
        rt_kprintf("%s 0x=%x %d\n", __func__, page_addr, ret);
        return -RT_EIO;
    }
    else
    {
        return 0;
    }
}

static uint32_t read_page(uint8_t cs, uint32_t page_addr, uint32_t *data, uint32_t *spare)
{
    int ret;

    ret = rt_mtd_nand_read(mtd_dev, page_addr, (rt_uint8_t *)data, mtd_dev->page_size, RT_NULL, 0);
    if (ret)
    {
        rt_kprintf("%s 0x=%x %d\n", __func__, page_addr, ret);
        return -RT_EIO;
    }
    else
    {
        return 0;
    }
}

static __attribute__((__unused__)) int nand_flash_misc_test(void)
{
    int ret = 0;
    uint32_t blk;

    rt_kprintf("%s\n", __func__);
    rt_kprintf("Bad block operation test\n");

    for (blk = 10; blk < 60; blk++)
    {
        rt_kprintf("%s mark bad block %d\n", __func__, blk);
        ret = rt_mtd_nand_mark_badblock(mtd_dev, blk);
        if (ret)
        {
            rt_kprintf("%s mark bad fail %d\n", __func__, ret);
            while (1)
                ;
        }
        ret = rt_mtd_nand_check_block(mtd_dev, blk);
        if (!ret)
        {
            rt_kprintf("%s mark bad check fail %d\n", __func__, ret);
            while (1)
                ;
        }

        g_nand_ops.erase_blk(0, blk);
    }
    rt_kprintf("%s success\n", __func__);

    return ret;
}

void nand_utils_test(uint32_t loop)
{
    rt_uint32_t size;

    size = mtd_dev->block_total * mtd_dev->pages_per_block * mtd_dev->page_size;
    rt_kprintf("size %d MB\n", (uint32_t)(size / 1024 / 1024));

    pwrite = malloc(NAND_PAGE_SIZE);
    pspare_write = malloc(NAND_SPARE_SIZE);
    pread = malloc(NAND_PAGE_SIZE);
    pspare_read = malloc(NAND_SPARE_SIZE);

    g_nand_ops.erase_blk = erase_blk;
    g_nand_ops.read_page = read_page;
    g_nand_ops.prog_page = prog_page;

    while (loop--)
    {
        nand_flash_stress_test();
    }

    free(pwrite);
    free(pspare_write);
    free(pread);
    free(pspare_read);
}

void spinand_test(int argc, char **argv)
{
    char *cmd;
    rt_uint8_t *buffer = NULL;
    uint32_t page_addr, page_num, block_addr, loop, start_time, end_time, cost_time;
    int i;
    rt_err_t ret;

    if (argc < 2)
        goto out;

    mtd_dev = (struct rt_mtd_nand_device *)rt_device_find("spinand0");
    if (mtd_dev == RT_NULL)
    {
        rt_kprintf("Did not find device: spinand0....\n");
        return;
    }

    cmd = argv[1];
    if (!rt_strcmp(cmd, "write"))
    {
        if (argc != 4)
            goto out;
        page_addr = atoi(argv[2]);
        page_num = atoi(argv[3]);

        if ((page_addr + page_num) / mtd_dev->pages_per_block > mtd_dev->block_end)
        {
            rt_kprintf("write over block end=%d\n", mtd_dev->block_end);
            return;
        }

        buffer = (rt_uint8_t *)rt_malloc_align(mtd_dev->page_size, 64);
        if (!buffer)
        {
            rt_kprintf("spi write alloc buf size %d fail\n", mtd_dev->page_size);
            return;
        }

        for (i = 0; i < mtd_dev->page_size; i++)
            buffer[i] = i % 256;

        start_time = HAL_GetTick();
        for (i = 0; i < page_num; i++)
        {
            ret = rt_mtd_nand_write(mtd_dev, page_addr + i,
                                    (const rt_uint8_t *)buffer, mtd_dev->page_size,
                                    RT_NULL, 0);
            if (ret)
            {
                rt_kprintf("%s write failed, ret=%d\n", __func__, ret);
                break;
            }
        }
        end_time = HAL_GetTick();
        cost_time = (end_time - start_time); /* ms */

        spinand_dbg_hex("write: ", buffer, 4, 16);
        rt_kprintf("spinand write speed: %ld KB/s\n", mtd_dev->page_size / cost_time);

        rt_free_align(buffer);
    }
    else if (!rt_strcmp(cmd, "read"))
    {
        if (argc != 4)
            goto out;
        page_addr = atoi(argv[2]);
        page_num = atoi(argv[3]);

        if ((page_addr + page_num) / mtd_dev->pages_per_block > mtd_dev->block_end)
        {
            rt_kprintf("write over block end=%d\n", mtd_dev->block_end);
            return;
        }

        buffer = (rt_uint8_t *)rt_malloc_align(mtd_dev->page_size, 64);
        if (!buffer)
        {
            rt_kprintf("spi write alloc buf size %d fail\n", mtd_dev->page_size);
            return;
        }

        for (i = 0; i < mtd_dev->page_size; i++)
            buffer[i] = i % 256;

        start_time = HAL_GetTick();
        for (i = 0; i < page_num; i++)
        {
            ret = rt_mtd_nand_read(mtd_dev, page_addr + i,
                                   (rt_uint8_t *)buffer, mtd_dev->page_size,
                                   RT_NULL, 0);
            if (ret)
            {
                rt_kprintf("%s write failed, ret=%d\n", __func__, ret);
                break;
            }
        }
        end_time = HAL_GetTick();
        cost_time = (end_time - start_time); /* ms */

        spinand_dbg_hex("read: ", buffer, 4, 16);
        rt_kprintf("spinand read speed: %ld KB/s\n", mtd_dev->page_size / cost_time);

        rt_free_align(buffer);
    }
    else if (!rt_strcmp(cmd, "erase"))
    {
        if (argc != 3)
            goto out;
        block_addr = atoi(argv[2]);

        if (block_addr > mtd_dev->block_end)
        {
            rt_kprintf("write over block end=%d\n", mtd_dev->block_end);
            return;
        }

        start_time = HAL_GetTick();
        rt_mtd_nand_erase_block(mtd_dev, block_addr);
        end_time = HAL_GetTick();
        cost_time = (end_time - start_time); /* ms */

        rt_kprintf("spinand erase speed: %ld KB/s\n", mtd_dev->page_size * mtd_dev->pages_per_block / cost_time);
    }
    else if (!rt_strcmp(cmd, "erase_all"))
    {
        if (argc != 2)
            goto out;

        g_nand_ops.erase_blk = erase_blk;
        g_nand_ops.read_page = read_page;
        g_nand_ops.prog_page = prog_page;

        flash_erase_all_block(0);
        rt_kprintf("erase all finished\n");
    }
    else if (!rt_strcmp(cmd, "stress"))
    {
        if (argc != 3)
            goto out;
        loop = atoi(argv[2]);

        nand_utils_test(loop);
    }
    else
    {

        goto out;
    }

    return;
out:
    spinand_test_show_usage();
    return;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(spinand_test, spt test cmd);
#endif

#endif
