
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h> 
#include <assert.h>

#include "framebuffer.h"

#define LOG_DBG(fmt, ...) fprintf(stdout, fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

void framebuffer_exit(framebuffer_t* fb)
{
    if (!fb)
        return;

    if (fb->screen) {
        munmap(fb->screen, fb->screen_size);
        fb->screen = NULL;
        fb->screen_size = 0;
    }
    if (fb->dev_fb > 0) {
        close(fb->dev_fb);
        fb->dev_fb = 0;
    }
    free(fb);
}

framebuffer_t* framebuffer_init()
{
    framebuffer_t* fb_info = (framebuffer_t*)malloc(sizeof(framebuffer_t));
    if (!fb_info) {
        LOG_ERR("fail to malloc fb info\n");
        return NULL;
    }
    memset(fb_info, 0, sizeof(framebuffer_t));

    fb_info->dev_fb = open("/dev/fb0", O_RDWR);
    if (-1 == fb_info->dev_fb) {
        LOG_ERR("fail to open: %s", strerror(fb_info->dev_fb));
        framebuffer_exit(fb_info);
        return NULL;
    }

    int ret = ioctl(fb_info->dev_fb, FBIOGET_VSCREENINFO, &fb_info->vinfo);
    if (-1 == ret) {
        LOG_ERR("fail to ioctl: %s", strerror(ret));
        framebuffer_exit(fb_info);
        return NULL;
    }
    fb_info->width = fb_info->vinfo.xres_virtual;
    fb_info->height = fb_info->vinfo.yres_virtual;
    //VIEW_RGB565_SIZE == COLOR_SIZE
    fb_info->screen_size = fb_info->width * fb_info->height * COLOR_SIZE;
    LOG_DBG("Width: %ld, Heigh: %ld\n", fb_info->width, fb_info->height);

    fb_info->screen = mmap(NULL, fb_info->screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_info->dev_fb, 0);
    if (MAP_FAILED == fb_info->screen) {
        perror("fail to get mmap");
        framebuffer_exit(fb_info);
        return NULL;
    }

    return fb_info;
}


#ifdef __XTEST__

int main()
{
    framebuffer_t *fb = framebuffer_init();
    assert(fb);
    framebuffer_exit(fb);
}

#endif//__XTEST__