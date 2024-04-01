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
display_t *display_init();

void display_fflush(display_t *d);

int display_view_print(display_t* d, view_t *v, const char* str, size_t str_len);

int assistant_view_print(display_t* d, view_t *v, const char* str, size_t str_len);
int user_view_print(display_t* d, view_t *v, const char* str, size_t str_len);

#ifdef __cplusplus
}
#endif

#endif//__DISPLAY_H__