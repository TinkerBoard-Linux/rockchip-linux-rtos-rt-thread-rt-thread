/**
  * Copyright (c) 2023 Fuzhou Rockchip Electronics Co., Ltd
  *
  * SPDX-License-Identifier: Apache-2.0
  ******************************************************************************
  * @file    modetest.c
  * @version V0.1
  * @brief   modetest for rockchip
  *
  * Change Logs:
  * Date           Author          Notes
  * 2023-11-11     Damon Ding      first implementation
  *
  ******************************************************************************
  */

#include <rtdevice.h>
#include <rthw.h>
#include <rtthread.h>

#ifdef RT_USING_COMMON_TEST_DISPLAY
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "display_pattern.h"
#include "dma.h"
#include "drv_display.h"
#include "drv_heap.h"
#include "hal_base.h"

#define MAX_PLANE_CNT   4

struct plane_arg
{
    uint32_t plane_id; /* the id of plane to use */
    uint32_t crtc_id; /* the id of CRTC to bind to */
    uint32_t w, h;
    uint32_t crtc_w, crtc_h;
    uint32_t stride;
    uint32_t zpos;
    uint32_t format;
    int32_t rotation;
    int32_t x, y;
    bool has_position;
    bool test_alpha;
    double scale;
    char format_str[5]; /* need to leave room for terminating \0 */
};

static struct rt_device *g_display_dev;
static struct rt_device *g_backlight_dev;

uint32_t rtt_framebuffer_yrgb[MAX_PLANE_CNT] = {0};
uint32_t rtt_framebuffer_uv[MAX_PLANE_CNT] = {0};

static uint32_t framebuffer_alloc(rt_size_t size)
{
#if defined(RT_USING_LARGE_HEAP)
    return (uint32_t)rt_dma_malloc_large(size);
#else
    return (uint32_t)rt_dma_malloc(size);;
#endif
}

static void framebuffer_free(void *base)
{
#if defined(RT_USING_LARGE_HEAP)
    rt_dma_free_large(base);
#else
    rt_dma_free(base);
#endif
}

static uint32_t get_stride(uint32_t format, uint32_t width)
{
    switch (format)
    {
    case RTGRAPHIC_PIXEL_FORMAT_ARGB888:
    case RTGRAPHIC_PIXEL_FORMAT_ABGR888:
        return width * 4;
    case RTGRAPHIC_PIXEL_FORMAT_RGB888:
        return width * 3;
    case RTGRAPHIC_PIXEL_FORMAT_BGR565:
    case RTGRAPHIC_PIXEL_FORMAT_RGB565:
        return width * 2;
    case RTGRAPHIC_PIXEL_FORMAT_YUV420:
    case RTGRAPHIC_PIXEL_FORMAT_YUV422:
    case RTGRAPHIC_PIXEL_FORMAT_YUV444:
        return width;
    default:
        rt_kprintf("Unsupported format: %d\n", format);
        return width * 4;
    }
}

static void usage(void)
{
    rt_kprintf("modetest [-p]\n\n");
    rt_kprintf("\t-p <plane_id>:<w>x<h>:<crtc_w>x<crtc_h>[+<x>+<y>][#<zpos>][@<format>] \tset plane\n");
    rt_kprintf("\t-a \ttest alpha\n");
}

static uint32_t parse_plane(struct plane_arg *plane, char *p)
{
    char *end;

    plane->plane_id = strtoul(p, &end, 10);
    if (*end != ':')
        return -RT_EINVAL;

    p = end + 1;
    plane->w = strtoul(p, &end, 10);
    if (*end != 'x')
        return -RT_EINVAL;

    p = end + 1;
    plane->h = strtoul(p, &end, 10);
    if (*end == ':')
    {
        p = end + 1;
        plane->crtc_w = strtoul(p, &end, 10);
        if (*end != 'x')
        {
            fprintf(stderr, "invalid crtc_w/h argument\n");
            return -RT_EINVAL;
        }

        p = end + 1;
        plane->crtc_h = strtoul(p, &end, 10);
    }
    else
    {
        plane->crtc_w = plane->w;
        plane->crtc_h = plane->h;
    }

    if (*end == '+' || *end == '-')
    {
        plane->x = strtol(end, &end, 10);
        if (*end != '+' && *end != '-')
            return -RT_EINVAL;
        plane->y = strtol(end, &end, 10);

        plane->has_position = true;
    }

    if (*end == '#')
    {
        plane->zpos = strtol(end + 1, &end, 10);
    }

    if (*end == '@')
    {
        strncpy(plane->format_str, end + 1, 4);
        plane->format_str[4] = '\0';
    }
    else
    {
        strcpy(plane->format_str, "AR24");
        rt_kprintf("failed to find fromat_str, use AR24 as default\n");
    }
    plane->format = util_format_fourcc(plane->format_str);
    if (plane->format == 0)
    {
        rt_kprintf("unknown format %s, use AR24 as default\n", plane->format_str);
        plane->format = RTGRAPHIC_PIXEL_FORMAT_ARGB888;
    }

    return 0;
}

static void set_planes(struct plane_arg *plane,
                       struct CRTC_WIN_STATE *win_config,
                       struct rt_device_graphic_info *graphic_info,
                       uint32_t plane_count)
{
    int i;

    for (i = 0; i < plane_count; i++)
    {
        win_config[i].winEn = true;
        win_config[i].winUpdate = true;
        win_config[i].winId = plane[i].plane_id;
        win_config[i].zpos = plane[i].zpos;
        win_config[i].format = plane[i].format;
        win_config[i].yrgbLength = 0;
        win_config[i].cbcrLength = 0;
        win_config[i].xVir = plane[i].w;

        win_config[i].srcX = 0;
        win_config[i].srcY = 0;
        win_config[i].srcW = plane[i].w;
        win_config[i].srcH = plane[i].h;

        win_config[i].crtcW = plane[i].crtc_w;
        win_config[i].crtcH = plane[i].crtc_h;

        if (!plane[i].has_position)
        {
            win_config[i].crtcX = (graphic_info->width - win_config[i].crtcW) >> 1;
            win_config[i].crtcY = (graphic_info->height - win_config[i].crtcH) >> 1;
        }
        else
        {
            win_config[i].crtcX = plane[i].x;
            win_config[i].crtcY = plane[i].y;
        }

        win_config[i].alphaEn = 0;
    }
}

static void clear_planes(struct rt_device *dev,
                         struct CRTC_WIN_STATE *win_config,
                         struct rt_device_graphic_info *graphic_info,
                         uint32_t plane_count)
{
    rt_err_t ret;
    int i;

    for (i = 0; i < plane_count; i++)
    {
        win_config[i].winEn = false;
        win_config[i].winUpdate = true;
        ret = rt_device_control(g_display_dev,
                                RK_DISPLAY_CTRL_SET_PLANE, &win_config[i]);
        RT_ASSERT(ret == RT_EOK);
    }

    ret = rt_device_control(g_display_dev, RK_DISPLAY_CTRL_COMMIT, NULL);
    RT_ASSERT(ret == RT_EOK);
    rt_thread_mdelay(500);
}

static bool is_yuv_format(uint32_t format)
{
    switch (format)
    {
    case RTGRAPHIC_PIXEL_FORMAT_YUV420:
    case RTGRAPHIC_PIXEL_FORMAT_YUV422:
    case RTGRAPHIC_PIXEL_FORMAT_YUV444:
        return true;
    default:
        return false;
    }
}

static int set_buffers(struct plane_arg *plane,
                       struct CRTC_WIN_STATE *win_config,
                       struct rt_device_graphic_info *graphic_info,
                       uint32_t plane_count)
{
    uint32_t fb_length;
    bool is_yuv;
    int i;

    for (i = 0; i < plane_count; i++)
    {
        uint32_t pitches[4] = {0};
        void *planes[3] = {0};

        is_yuv = is_yuv_format(win_config[i].format);
        pitches[0] = get_stride(win_config[i].format, win_config[i].srcW);
        fb_length = pitches[0] * win_config[i].srcH;
        rtt_framebuffer_yrgb[i] = framebuffer_alloc(fb_length);
        if (rtt_framebuffer_yrgb[i] == RT_NULL)
        {
            rt_kprintf("Failed to malloc memory: 0x%x\n", fb_length);
            return -RT_ENOMEM;
        }
        rt_memset((void *)rtt_framebuffer_yrgb[i], 0, fb_length);
        planes[0] = (void *)rtt_framebuffer_yrgb[i];

        if (is_yuv)
        {
            rtt_framebuffer_uv[i] = framebuffer_alloc(fb_length);
            if (rtt_framebuffer_uv[i] == RT_NULL)
            {
                rt_kprintf("Failed to malloc memory: 0x%x\n", fb_length);
                return -RT_ENOMEM;
            }
            rt_memset((void *)rtt_framebuffer_yrgb[i], 0, fb_length);
            planes[1] = (void *)rtt_framebuffer_uv[i];
        }

        util_fill_pattern(win_config[i].format, planes, win_config[i].srcW, win_config[i].srcH, pitches[0]);
        HAL_DCACHE_CleanByRange((uint32_t)rtt_framebuffer_yrgb[i], fb_length);
        if (is_yuv)
            HAL_DCACHE_CleanByRange((uint32_t)rtt_framebuffer_uv[i], fb_length);

        win_config[i].yrgbAddr = (uint32_t)rtt_framebuffer_yrgb[i];
        win_config[i].cbcrAddr = (uint32_t)rtt_framebuffer_uv[i];
    }

    return 0;
}

static void clear_buffers(struct plane_arg *plane,
                          struct CRTC_WIN_STATE *win_config,
                          struct rt_device_graphic_info *graphic_info,
                          uint32_t plane_count)
{
    bool is_yuv;
    int i;

    for (i = 0; i < plane_count; i++)
    {
        is_yuv = is_yuv_format(win_config[i].format);
        framebuffer_free((void *)rtt_framebuffer_yrgb[i]);
        rtt_framebuffer_yrgb[i] = 0;
        if (is_yuv)
        {
            framebuffer_free((void *)rtt_framebuffer_uv[i]);
            rtt_framebuffer_uv[i] = 0;
        }
    }
}

static int modetest(int argc, char **argv)
{
    rt_err_t ret = RT_EOK;
    struct rt_device_graphic_info *graphic_info;
    struct CRTC_WIN_STATE *win_config = NULL;
    struct plane_arg *plane_args = NULL;
    uint32_t plane_count = 0;
    int opt, i, j;

    if (argc == 1)
    {
        usage();
        return 0;
    }

    optind = 0;
    while ((opt = getopt(argc, argv, "p:a")) != -1)
    {
        switch (opt)
        {
        case 'p':
            plane_args = rt_realloc(plane_args, (plane_count + 1) * sizeof(struct plane_arg));
            if (!plane_args)
            {
                rt_kprintf("memory allocation failed\n");
                return -RT_ENOMEM;
            }
            rt_memset(&plane_args[plane_count], 0, sizeof(*plane_args));

            if (parse_plane(&plane_args[plane_count], optarg) < 0)
                usage();

            plane_count++;
            break;
        case 'a':
            if (plane_count)
                plane_args[plane_count - 1].test_alpha = true;
            break;
        default:
            rt_kprintf("Unknown option: %c\n", opt);
            usage();
            return 0;
        }
    }

    rt_kprintf("Enter modetest! \n");

    g_display_dev = rt_device_find("lcd");
    RT_ASSERT(g_display_dev != RT_NULL);

    ret = rt_device_open(g_display_dev, RT_DEVICE_FLAG_RDWR);
    RT_ASSERT(ret == RT_EOK);

    g_backlight_dev = rt_device_find("backlight");
    if (g_backlight_dev)
    {
        int brightness = 100;
        rt_kprintf("Backlight power on\n");
        rt_device_control(g_backlight_dev, RTGRAPHIC_CTRL_POWERON, NULL);
        rt_device_control(g_backlight_dev, RTGRAPHIC_CTRL_RECT_UPDATE,
                          &brightness);
    }
    else
    {
        rt_kprintf("Failed to find backlight dev\n");
    }

    ret = rt_device_control(g_display_dev, RK_DISPLAY_CTRL_AP_COP_MODE, (uint8_t *)0);
    ret = rt_device_control(g_display_dev, RTGRAPHIC_CTRL_POWERON, NULL);
    RT_ASSERT(ret == RT_EOK);

    graphic_info = (struct rt_device_graphic_info *)rt_calloc(1, sizeof(struct rt_device_graphic_info));
    RT_ASSERT(graphic_info != RT_NULL);

    ret = rt_device_control(g_display_dev,
                            RTGRAPHIC_CTRL_GET_INFO, (void *)graphic_info);
    RT_ASSERT(ret == RT_EOK);

    win_config = (struct CRTC_WIN_STATE *)rt_calloc(plane_count, sizeof(struct CRTC_WIN_STATE));
    RT_ASSERT(win_config != RT_NULL);

    set_planes(plane_args, win_config, graphic_info, plane_count);

    clear_planes(g_display_dev, win_config, graphic_info, plane_count);

    ret = set_buffers(plane_args, win_config, graphic_info, plane_count);
    if (ret)
        return ret;

    for (i = 0; i < plane_count; i++)
    {
        win_config[i].winEn = true;

        ret = rt_device_control(g_display_dev,
                                RK_DISPLAY_CTRL_SET_PLANE, &win_config[i]);
        RT_ASSERT(ret == RT_EOK);

        rt_kprintf("Colorbar test [%dx%d->%dx%d@%dx%d]@%s in win%d\n",
                   win_config[i].srcW, win_config[i].srcH, win_config[i].crtcW, win_config[i].crtcH,
                   win_config[i].crtcX, win_config[i].crtcY, plane_args[i].format_str, win_config[i].winId);
    }

    ret = rt_device_control(g_display_dev, RK_DISPLAY_CTRL_COMMIT, NULL);
    RT_ASSERT(ret == RT_EOK);

    rt_thread_mdelay(200);

    for (i = 0; i < plane_count; i++)
    {
        if (plane_args[i].test_alpha)
        {
            rt_kprintf("Alpha test in win%d\n", win_config[i].winId);
            win_config[i].alphaEn = true;
            for (j = 0; j < 0xff; j += 5)
            {
                win_config[i].globalAlphaValue = j;
                win_config[i].alphaMode = VOP_ALPHA_MODE_USER_DEFINED;
                win_config[i].alphaPreMul = VOP_NON_PREMULT_ALPHA;
                ret = rt_device_control(g_display_dev,
                                        RK_DISPLAY_CTRL_SET_PLANE, &win_config[i]);
                RT_ASSERT(ret == RT_EOK);

                ret = rt_device_control(g_display_dev, RK_DISPLAY_CTRL_COMMIT, NULL);
                RT_ASSERT(ret == RT_EOK);

                rt_thread_mdelay(50);
            }
            win_config[i].globalAlphaValue = j;
            win_config[i].alphaEn = false;
        }
    }

    rt_thread_mdelay(1000);

    for (i = 0; i < plane_count; i++)
    {
        win_config[i].winEn = false;

        ret = rt_device_control(g_display_dev,
                                RK_DISPLAY_CTRL_SET_PLANE, &win_config[i]);
        RT_ASSERT(ret == RT_EOK);
    }

    ret = rt_device_control(g_display_dev, RK_DISPLAY_CTRL_COMMIT, NULL);
    RT_ASSERT(ret == RT_EOK);

    clear_buffers(plane_args, win_config, graphic_info, plane_count);

    if (g_backlight_dev)
    {
        int brightness = 0;
        rt_kprintf("Backlight power off\n");
        ret = rt_device_control(g_backlight_dev, RTGRAPHIC_CTRL_POWEROFF, NULL);
        RT_ASSERT(ret == RT_EOK);
        rt_device_control(g_backlight_dev, RTGRAPHIC_CTRL_RECT_UPDATE,
                          &brightness);
    }
    else
    {
        ret = rt_device_control(g_display_dev, RTGRAPHIC_CTRL_POWEROFF, NULL);
        RT_ASSERT(ret == RT_EOK);
    }

    rt_free(graphic_info);
    rt_free(win_config);
    rt_free(plane_args);

    rt_kprintf("Exit display test!\n");

    return ret;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(modetest, display drive mode test. e.g: modetest);
#endif
#endif
