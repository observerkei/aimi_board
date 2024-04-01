#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "framebuffer.h"

struct display_t;

void display_exit(display_t *d);
display_t *display_init();


void display_fflush(display_t *d);

int display_view_print(display_t* d, const char* str, size_t str_len, size_t cur_x, size_t cur_y);

int assistant_view_print(display_t* d, const char* str, size_t str_len);
int user_view_print(display_t* d, char* str, size_t str_len);
void display_set_font_color(display_t* d, framebuffer_color_t color);

#ifdef __cplusplus
}
#endif

#endif//__DISPLAY_H__