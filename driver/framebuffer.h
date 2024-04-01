#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <linux/fb.h>

typedef uint16_t framebuffer_color_t;
#define COLOR_SIZE (sizeof(framebuffer_color_t))

#define COLOR_BLACK (0x0000U)
#define COLOR_WHITE (0xffffU)
#define COLOR_GREY (0xe73cU)

typedef struct framebuffer_t {
    size_t screen_size;
    size_t width;
    size_t height;
    int dev_fb;
    struct fb_var_screeninfo vinfo;
    void* screen;
} framebuffer_t;

void framebuffer_exit(framebuffer_t* fb);
framebuffer_t *framebuffer_init();


#ifdef __cplusplus
}
#endif

#endif//__FRAMEBUFFER_H__