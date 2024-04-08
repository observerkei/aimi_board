#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "framebuffer.h"


/*
 *    @ Display
 *    @ 原点点定义: 0点为屏幕正方的左上角
 *    @ 方向定义
 * 
 *        |<--width-->
 *    —— 0+----------> x++
 *    ^   |
 *    |   |
 * height |
 *    |   |
 *    v   v
 *        y++
 *
 * */
struct display_t;

/*
 *   @ display是显示器, 显示器里面可包含多个视窗view
 *   display
 *   +----------------------+
 *   |  view_start  +-----+ |  
 *   |  +-------+   |view2| |
 *   |  | view1 |   +-----+ |
 *   |  |       |           |
 *   |  +-------+           |  
 *   +----------------------+
 * 
 * */
typedef struct view_t {
    const size_t start_x;            // 视图开始x位置
    const size_t start_y;            // 视图开始y位置
    const size_t width;              // 视图宽
    const size_t height;             // 视图高
    size_t now_x;                    // 当前绘制x位置
    size_t now_y;                    // 当前绘制y位置
    framebuffer_color_t font_color;  // 绘制颜色
} view_t;

/**
 * 释放显示设备所占用的内存空间。
 *
 * @param d 指向显示设备的指针。
 */
void display_exit(display_t *d);

/**
 * 初始化显示设备。
 *
 * @param fb_dev 帧缓冲设备文件的路径。
 * @param font_path 字体文件的路径。
 * @return 指向初始化后的显示设备的指针。
 */
display_t *display_init(const char *fb_dev, const char *font_path);

/**
 * 设置调试模式。
 *
 * @param enable 是否启用调试模式，1 为启用，0 为禁用。
 */
void display_set_debug(char enable);

/**
 * 获取显示设备的宽度。
 *
 * @param d 指向显示设备的指针。
 * @return 显示设备的宽度。
 */
size_t display_get_width(display_t *d);

/**
 * 获取显示设备的高度。
 *
 * @param d 指向显示设备的指针。
 * @return 显示设备的高度。
 */
size_t display_get_height(display_t *d);

/**
 * 刷新显示设备的内容。
 *
 * @param d 指向显示设备的指针。
 */
void display_fflush(display_t *d);

/**
 * 清空视图的内容。
 *
 * @param d 指向显示设备的指针。
 * @param v 指向视图的指针。
 */
void display_view_clear(display_t* d, view_t* v);

/**
 * 在缓存上设置指定位置的颜色。
 *
 * @param d 指向显示设备的指针。
 * @param x 指定位置的横坐标。
 * @param y 指定位置的纵坐标。
 * @param color 要设置的颜色。
 */
void display_set_cache_color(display_t* d, size_t x, size_t y, framebuffer_color_t color);

/**
 * 在视图上打印字符串。(支持中文)
 *
 * @param d 指向显示设备的指针。
 * @param v 指向视图的指针。
 * @param from_code 字符串的编码格式。
 * @param str 要打印的字符串。
 * @param str_len 要打印的字符串长度。
 * @return 打印成功返回 0，失败返回 -1。
 */
int display_view_print(display_t* d, view_t *v, const char *from_code, const char* str, size_t str_len);


#ifdef __cplusplus
}
#endif

#endif//__DISPLAY_H__