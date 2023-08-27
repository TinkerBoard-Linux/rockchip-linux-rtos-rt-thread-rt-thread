/**
  * Copyright (c) 2020 Fuzhou Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  ******************************************************************************
  * @file    drv_spinand.c
  * @version V1.0
  * @brief   spi nand interface
  *
  * Change Logs:
  * Date           Author          Notes
  * 2020-06-16     Dingqiang Lin   the first version
  *
  ******************************************************************************
  */

/** @addtogroup RKBSP_Driver_Reference
 *  @{
 */

/** @addtogroup SPINAND
 *  @{
 */

/** @defgroup SPINAND_How_To_Use How To Use
 *  @{

    SPI_NAND is a framework protocol layer based on SPI Nand flash. It needs to be combined
 with the corresponding driver layer to complete the transmission of the protocol.

 @} */

#include <rtthread.h>

#ifdef RT_USING_SPINAND
#include <rthw.h>
#include <rtdevice.h>
#include <drivers/mtd_nand.h>

#include "board.h"
#include "hal_bsp.h"
#include "drv_clock.h"
#include "drv_fspi.h"
#include "hal_base.h"
#include "mini_ftl.h"


/********************* Private MACRO Definition ******************************/
/** @defgroup SPINAND_Private_Macro Private Macro
 *  @{
 */
// #define SPINAND_DEBUG
#ifdef SPINAND_DEBUG
#define spinand_dbg(...)     rt_kprintf(__VA_ARGS__)
#else
#define spinand_dbg(...)
#endif

#define MTD_TO_SPINAND(mtd) ((struct SPI_NAND *)mtd->priv)

/** @} */  // SPINAND_Private_Macro

/********************* Private Structure Definition **************************/
/** @defgroup SPINAND_Private_Structure Private Structure
 *  @{
 */

static struct rt_mutex spinand_lock;

int spinand_read(rt_mtd_t *mtd, loff_t from, struct mtd_io_desc *ops)
{
    int ret;

    spinand_dbg("%s addr= %lx len= %x\n", __func__, (uint32_t)from, (uint32_t)ops->datlen);

    rt_mutex_take(&spinand_lock, RT_WAITING_FOREVER);
    ret = mftl_mtd_read(mtd, from, ops);
    rt_mutex_release(&spinand_lock);

    return ret;
}

int spinand_write(rt_mtd_t *mtd, loff_t to, struct mtd_io_desc *ops)
{
    int ret;

    spinand_dbg("%s addr= %lx len= %x\n", __func__, (uint32_t)to, (uint32_t)ops->datlen);
    rt_mutex_take(&spinand_lock, RT_WAITING_FOREVER);
    ret = mftl_mtd_write(mtd, to, ops);
    rt_mutex_release(&spinand_lock);

    return ret;
}

int spinand_erase(rt_mtd_t *mtd, loff_t addr, size_t len)
{
    int ret;

    spinand_dbg("%s addr= %lx len= %lx\n", __func__, (uint32_t)addr, (uint32_t)len);
    rt_mutex_take(&spinand_lock, RT_WAITING_FOREVER);
    ret = mftl_mtd_erase(mtd, addr, len);
    rt_mutex_release(&spinand_lock);

    return ret;
}

int spinand_isbad(rt_mtd_t *mtd, uint32_t block)
{
    struct SPI_NAND *spinand = MTD_TO_SPINAND(mtd);
    int32_t ret = RT_EOK;

    spinand_dbg("%s blk= %lx\n", __func__, block);
    rt_mutex_take(&spinand_lock, RT_WAITING_FOREVER);
    ret = HAL_SPINAND_IsBad(spinand, block * spinand->pagePerBlk);
    rt_mutex_release(&spinand_lock);

    return ret;
}

int spinand_markbad(rt_mtd_t *mtd, uint32_t block)
{
    struct SPI_NAND *spinand = MTD_TO_SPINAND(mtd);
    int32_t ret = RT_EOK;

    spinand_dbg("%s blk= %lx\n", __func__, block);
    rt_mutex_take(&spinand_lock, RT_WAITING_FOREVER);
    ret = HAL_SPINAND_MarkBad(spinand, block * spinand->pagePerBlk);
    rt_mutex_release(&spinand_lock);

    return ret;
}

static const struct mtd_ops ops =
{
    spinand_erase,
    spinand_read,
    spinand_write,
    spinand_isbad,
    spinand_markbad,
};

#if defined(RT_USING_SPINAND_FSPI_HOST)
static HAL_Status fspi_xfer(struct SPI_NAND_HOST *spi, struct HAL_SPI_MEM_OP *op)
{
    struct rt_fspi_device *fspi_device = (struct rt_fspi_device *)spi->userdata;

    return rt_fspi_xfer(fspi_device, op);
}

static int rockchip_sfc_delay_lines_tuning(struct SPI_NAND *spinand, struct rt_fspi_device *fspi_device)
{
    uint8_t id_temp[SPINAND_MAX_ID_LEN];
    uint16_t cell_max = (uint16_t)rt_fspi_get_max_dll_cells(fspi_device);
    uint16_t right, left = 0;
    uint16_t step = HAL_FSPI_DLL_TRANING_STEP;
    bool dll_valid = false;
    uint32_t final;

    HAL_SPINAND_Init(spinand);
    for (right = 0; right <= cell_max; right += step)
    {
        int ret;

        ret = rt_fspi_set_delay_lines(fspi_device, right);
        if (ret)
        {
            dll_valid = false;
            break;
        }
        ret = HAL_SPINAND_ReadID(spinand, id_temp);
        if (ret)
        {
            dll_valid = false;
            break;
        }

        spinand_dbg("dll read flash id:%x %x %x\n",
                    id_temp[0], id_temp[1], id_temp[2]);

        ret = HAL_SPINAND_IsFlashSupported(id_temp);
        if (dll_valid && !ret)
        {
            right -= step;

            break;
        }
        if (!dll_valid && ret)
            left = right;

        if (ret)
            dll_valid = true;

        /* Add cell_max to loop */
        if (right == cell_max)
            break;
        if (right + step > cell_max)
            right = cell_max - step;
    }

    if (dll_valid && (right - left) >= HAL_FSPI_DLL_TRANING_VALID_WINDOW)
    {
        if (left == 0 && right < cell_max)
            final = left + (right - left) * 2 / 5;
        else
            final = left + (right - left) / 2;
    }
    else
    {
        final = 0;
    }

    if (final)
    {
        spinand_dbg("spinand %d %d %d dll training success in %dMHz max_cells=%u\n",
                    left, right, final, spinand->spi->speed, cell_max);
        return rt_fspi_set_delay_lines(fspi_device, final);
    }
    else
    {
        rt_kprintf("spinor %d %d dll training failed in %dMHz, reduce the frequency\n",
                   left, right, spinand->spi->speed);
        rt_fspi_dll_disable(fspi_device);
        return -1;
    }
}

RT_WEAK struct rt_fspi_device g_fspi_spinand =
{
    .host_id = 0,
    .dev_type = DEV_SPINAND,
    .chip_select = 0,
};

static uint32_t spinand_adapt(struct SPI_NAND *spinand)
{
    struct rt_fspi_device *fspi_device = &g_fspi_spinand;
    uint32_t ret;
    int dll_result = 0;

    spinand_dbg("spinand_adapt in\n");
    ret = rt_hw_fspi_device_register(fspi_device);
    if (ret)
    {
        return ret;
    }

    /* Designated host to SPI_NAND */
    if (RT_SPINAND_SPEED > 0 && RT_SPINAND_SPEED <= SPINAND_SPEED_MAX)
    {
        spinand->spi->speed = RT_SPINAND_SPEED;
    }
    else
    {
        spinand->spi->speed = SPINAND_SPEED_DEFAULT;
    }
    spinand->spi->speed = rt_fspi_set_speed(fspi_device, spinand->spi->speed);

#ifdef RT_USING_SPINAND_FSPI_HOST_CS1_GPIO
    if (!fspi_device->cs_gpio.gpio)
    {
        rt_kprintf("it's needed to redefine g_fspi_spinand with cs_gpio in iomux.c!\n");
        return -RT_ERROR;
    }
#endif

    spinand->spi->userdata = (void *)fspi_device;
    spinand->spi->mode = HAL_SPI_MODE_3 | HAL_SPI_RX_QUAD;
    spinand->spi->xfer = fspi_xfer;
    spinand_dbg("%s fspi initial\n", __func__);
    rt_fspi_controller_init(fspi_device);

    if (spinand->spi->speed > HAL_FSPI_SPEED_THRESHOLD)
    {
        dll_result = rockchip_sfc_delay_lines_tuning(spinand, fspi_device);
    }
    else
    {
        rt_fspi_dll_disable(fspi_device);
    }

    /* Init SPI_NAND abstract */
    spinand_dbg("%s spinand initial\n", __func__);
    ret = HAL_SPINAND_Init(spinand);
    if (ret)
    {
        uint8_t idByte[5];

        HAL_SPINAND_ReadID(spinand, idByte);
        rt_kprintf("SPI Nand ID: %x %x %x\n", idByte[0], idByte[1], idByte[2]);
    }

    if (dll_result)
    {
        rt_fspi_set_speed(fspi_device, HAL_FSPI_SPEED_THRESHOLD);
        rt_kprintf("%s dll turning failed %d\n", __func__, dll_result);
    }

    return ret;
}
#elif defined(RT_USING_SPINAND_SPI_HOST)
static HAL_Status SPI_Xfer(struct SPI_NAND_HOST *spi, struct HAL_SPI_MEM_OP *op)
{
    struct rt_spi_device *spi_device = (struct rt_spi_device *)spi->userdata;
    struct rt_spi_configuration cfg;
    uint32_t pos = 0;
    const uint8_t *tx_buf = NULL;
    uint8_t *rx_buf = NULL;
    uint8_t op_buf[HAL_SPI_OP_LEN_MAX];
    int32_t op_len;
    int32_t i, ret;

    if (op->data.nbytes)
    {
        if (op->data.dir == HAL_SPI_MEM_DATA_IN)
            rx_buf = op->data.buf.in;
        else
            tx_buf = op->data.buf.out;
    }

//    rt_kprintf("%s %x %lx\n", __func__, op->cmd.opcode, op->data.nbytes);
    op_len = sizeof(op->cmd.opcode) + op->addr.nbytes + op->dummy.nbytes;
    op_buf[pos++] = op->cmd.opcode;

    if (op->addr.nbytes)
    {
        for (i = 0; i < op->addr.nbytes; i++)
            op_buf[pos + i] = op->addr.val >> (8 * (op->addr.nbytes - i - 1));
        pos += op->addr.nbytes;
    }

    if (op->dummy.nbytes)
        memset(&op_buf[pos], 0xff, op->dummy.nbytes);

    cfg.data_width = 8;
    cfg.mode = spi->mode | RT_SPI_MSB;
    cfg.max_hz = spi->speed;
    rt_spi_configure(spi_device, &cfg);

    if (tx_buf)
    {
        ret = rt_spi_send_then_send(spi_device, op_buf, op_len, tx_buf, op->data.nbytes);
        if (ret)
            ret = HAL_ERROR;
    }
    else if (rx_buf)
    {
        ret = rt_spi_send_then_recv(spi_device, op_buf, op_len, rx_buf, op->data.nbytes);
        if (ret)
            ret = HAL_ERROR;
    }
    else
    {
        ret = rt_spi_send(spi_device, op_buf, op_len);
        if (ret != op_len)
            ret = HAL_ERROR;
        else
            ret = HAL_OK;
    }

//    rt_kprintf("%s finished %d\n", __func__, ret);

    return ret;
}

static uint32_t spinand_adapt(struct SPI_NAND *spinand)
{
    struct rt_spi_device *spi_device = NULL;

#if defined(RT_SPINAND_SPI_DEVICE_NAME)
    spi_device = (struct rt_spi_device *)rt_device_find(RT_SPINAND_SPI_DEVICE_NAME);
#endif
    if (!spi_device)
    {
        rt_kprintf("%s can not find %s\n", __func__, RT_SPINAND_SPI_DEVICE_NAME);

        return RT_EINVAL;
    }

    /* Designated host to SPI Nand */
    spinand->spi->userdata = (void *)spi_device;
    spinand->spi->mode = HAL_SPI_MODE_3;
    spinand->spi->xfer = SPI_Xfer;
    if (RT_SPINAND_SPEED > 0 && RT_SPINAND_SPEED <= HAL_SPI_MASTER_MAX_SCLK_OUT)
    {
        spinand->spi->speed = RT_SPINAND_SPEED;
    }
    else
    {
        spinand->spi->speed = HAL_SPI_MASTER_MAX_SCLK_OUT;
    }

    /* Init SPI Nand abstract */
    return HAL_SPINAND_Init(spinand);
}
#else
static uint32_t spinand_adapt(struct SPI_NAND *spinandF)
{
    return RT_EINVAL;
}
#endif

/** @} */  // SPINAND_Private_Function

/********************* Public Function Definition ****************************/

/** @defgroup SPINAND_Public_Functions Public Functions
 *  @{
 */

/* define partitions to it, mtd_spinand reserved for spi nand dev */
struct mtd_part spinand_parts[1] =
{
    { "spinand0", 0, 0, },
};

/**
 * @brief  Init SPI_NAND framwork and apply to use.
 * @attention The SPI_NAND will be enabled when board initialization, do not
 *      dynamically switch SPINAND unless specifically required.
 */
int rt_hw_spinand_init(void)
{
    struct mtd_info *mtd_dev;
    struct SPI_NAND *spinand;
    struct SPI_NAND_HOST *spi;
    int32_t ret;

    mtd_dev = (struct mtd_info *)rt_calloc(1, sizeof(*mtd_dev));
    RT_ASSERT(mtd_dev);
    spinand = (struct SPI_NAND *)rt_calloc(1, sizeof(*spinand));
    RT_ASSERT(spinand);
    spi = (struct SPI_NAND_HOST *)rt_calloc(1, sizeof(*spi));
    RT_ASSERT(spi);
    spinand->spi = spi;

    ret = spinand_adapt(spinand);
    if (ret)
    {
        rt_kprintf("SPI Nand init adapt error, ret= %ld\n", ret);
        goto exit;
    }

    /* flash feature setting */
    if (rt_mutex_init(&(spinand_lock), "spinandLock", RT_IPC_FLAG_FIFO) != RT_EOK)
    {
        rt_kprintf("Init mutex error\n");
        RT_ASSERT(0);
    }

    /* register mtd spinand */
    mtd_dev->sector_size     = spinand->secPerPage * SPINAND_SECTOR_SIZE;
    mtd_dev->writesize_shift = __rt_ffs(mtd_dev->sector_size) - 1;
    mtd_dev->writesize_mask  = mtd_dev->sector_size - 1;
    mtd_dev->block_size      = mtd_dev->sector_size  * spinand->pagePerBlk;
    mtd_dev->erasesize_shift = __rt_ffs(mtd_dev->block_size) - 1;
    mtd_dev->erasesize_mask  = mtd_dev->block_size - 1;
    mtd_dev->oob_size        = spinand->secPerPage * 16;
    mtd_dev->oob_avail        = spinand->secPerPage * 2;
    mtd_dev->offset          = 0;
    mtd_dev->size            = spinand->size;
    mtd_dev->priv            = spinand;
    mtd_dev->ops             = &ops;

    spinand_dbg("sector_size %lx\n", mtd_dev->sector_size);
    spinand_dbg("writesize_shift %lx\n", mtd_dev->writesize_shift);
    spinand_dbg("writesize_mask %lx\n", mtd_dev->writesize_mask);
    spinand_dbg("block_size %lx\n", mtd_dev->block_size);
    spinand_dbg("erasesize_shift %lx\n", mtd_dev->erasesize_shift);
    spinand_dbg("erasesize_mask %lx\n", mtd_dev->erasesize_mask);
    spinand_dbg("oob_size %lx\n", mtd_dev->oob_size);
    spinand_dbg("oob_avail %lx\n", mtd_dev->oob_avail);
    spinand_dbg("size %lx\n", mtd_dev->size);

    spinand_parts[0].size   = (uint32_t)mtd_dev->size;

    ret = rt_mtd_register(mtd_dev, (const struct mtd_part *)spinand_parts, HAL_ARRAY_SIZE(spinand_parts));
    ret = mini_ftl_register(mtd_dev);
    if (ret < 0)
    {
        rt_kprintf("mini_ftl_register register fail %d\n", ret);
        goto exit;
    }
exit:
    if (ret < 0)
    {
        rt_free(spinand->spi);
        rt_free(spinand);
        rt_free(mtd_dev);
    }

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_spinand_init);

/** @} */  // SPINAND_Public_Function

#endif

/** @} */  // SPINAND

/** @} */  // RKBSP_Common_Driver
