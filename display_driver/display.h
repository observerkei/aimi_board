#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "framebuffer.h"

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
    const size_t start_x;
    const size_t start_y;
    const size_t width;
    const size_t height;
    size_t now_x;
    size_t now_y;
    framebuffer_color_t font_color;
} view_t;

void display_exit(display_t *d);
display_t *display_init(const char *fb_dev, const char *font_path);

void display_set_debug(char enable);

size_t display_get_width(display_t *d);
size_t display_get_height(display_t *d);

void display_fflush(display_t *d);
void display_view_clear(display_t* d, view_t* v);

// 往缓存上画点的函数
void display_set_cache_color(display_t* d, size_t x, size_t y, framebuffer_color_t color);

// 往缓存上打印的函数(支持中文)
int display_view_print(display_t* d, view_t *v, const char *from_code, const char* str, size_t str_len);

#ifdef __cplusplus
}
#endif

#endif//__DISPLAY_H__