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
    size_t screen_size;              // 屏幕占用内存大小
    size_t width;                    // 屏幕宽度
    size_t height;                   // 屏幕高度
    int dev_fb;                      // 屏幕设备描述符
    struct fb_var_screeninfo vinfo;  // 屏幕信息
    void* screen;                    // 屏幕内存
} framebuffer_t;

/**
 * 释放帧缓冲区所占用的内存空间。
 *
 * @param fb 指向帧缓冲区的指针。
 */
void framebuffer_exit(framebuffer_t* fb);

/**
 * 初始化帧缓冲区。
 *
 * @param dev_file 帧缓冲设备文件的路径。
 * @return 指向初始化后的帧缓冲区的指针。
 */
framebuffer_t *framebuffer_init(const char *dev_file);


#ifdef __cplusplus
}
#endif

#endif//__FRAMEBUFFER_H__