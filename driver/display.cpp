#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "display.h"
#include "font_bitmap.h"

/*
 * @ Display
 * @ 原点点定义: 0点为屏幕正方的左上角
 * @ 方向定义
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

#define LOG_DBG(fmt, ...) fprintf(stdout, "[%s:%s:%u] " fmt, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) fprintf(stderr, "[%s:%s:%u] " fmt, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define DISPLAY_TABS_OF_SPACE (2)
#define DISPLAY_SPACE_WIDTH_BIT (ASCII_WORD_SIZE)
#define DISPLAY_SPACE_HEIGHT_BIT (FONT_HEIGHT_WORD_SIZE)
#define ROLE_ASSISTANT_HEIGHT_START (0)
#define ROLE_USER_HEIGHT_START (11 * FONT_HEIGHT_WORD_SIZE) // 共15行，分配 11 行给 AI

typedef struct display_t {
    size_t cache_size;
    font_bitmap_t* font;
    framebuffer_t* fb_info;
    uint8_t* cache;
} display_t;

static display_t g_display = { 0 };

static inline size_t view_add_x(view_t* v, size_t add)
{
    if (v->now_x + add > v->start_x + v->width)
        return 0;
    return v->now_x + add;
}

static inline size_t view_add_y(view_t* v, size_t add)
{
    if (v->now_y + add > v->start_y + v->height)
        return 0;
    return v->now_y + add;
}

static inline size_t display_cul_cache_offset(display_t* d, size_t x, size_t y)
{
    size_t offset = (y * d->fb_info->width + x) * COLOR_SIZE;
    return offset < (d->cache_size - COLOR_SIZE - 1) ? offset : 0;
}

static inline void display_set_cache_color(display_t* d, size_t x, size_t y, framebuffer_color_t color)
{
    *(framebuffer_color_t*)&d->cache[display_cul_cache_offset(d, x, y)] = color;
}

inline void display_fflush(display_t* d)
{
    memcpy(g_display.fb_info->screen, d->cache, g_display.fb_info->screen_size);
}

static inline int display_cul_next_line(display_t* d, view_t* v, size_t* x, size_t* y, gb2312_word_type_t type)
{
    size_t next_x = *x;
    size_t next_y = *y;
    size_t space = (type == GB2312_CHINESE ? ZH_WORD_SIZE : ASCII_WORD_SIZE);

    size_t real_width = v->start_x + v->width > d->fb_info->width
        ? d->fb_info->width - v->start_x
        : v->width;

    if (next_x >= real_width || next_x + space >= real_width) {
        next_y += FONT_HEIGHT_WORD_SIZE;
        next_x = v->start_x; // 右侧剩余空间不够了
        display_cul_next_line(d, v, &next_x, &next_y, type);
    }

    size_t real_height = v->start_y + v->height > d->fb_info->height
        ? d->fb_info->height - v->start_y
        : v->height;

    if (next_y >= real_height) {
        next_y = v->start_y;
        display_cul_next_line(d, v, &next_x, &next_y, type);
    }

    *x = next_x;
    *y = next_y;
    return 0;
}

static void display_draw_space_word(display_t* d, view_t* v)
{
    size_t next_x = view_add_x(v, ASCII_WORD_SIZE);
    size_t next_y = v->now_y;
    int ret = display_cul_next_line(d, v, &next_x, &next_y, GB2312_ASCII);
    if (ret < 0) {
        LOG_DBG("display over flow! cx(%zu) y(%zu)\n", next_x, next_y);
        return;
    }
    v->now_x = next_x;
    v->now_y = next_y;
}

static inline void display_draw_tabs_word(display_t* d, view_t* v)
{
    for (size_t i = 0; i < DISPLAY_TABS_OF_SPACE; ++i)
        display_draw_space_word(d, v);
}

static inline void display_draw_endl_word(display_t* d, view_t* v)
{
    size_t next_x = 0;
    size_t next_y = view_add_y(v, FONT_HEIGHT_WORD_SIZE);
    int ret = display_cul_next_line(d, v, &next_x, &next_y, GB2312_ASCII);
    if (ret < 0) {
        LOG_ERR("fail to draw endl, x(%zu) y(%zu) to nx(%zu), ny(%zu)",
            v->now_x, v->now_y, next_x, next_y);
        return;
    }
    v->now_x = next_x;
    v->now_y = next_y;
}

static inline void display_draw_ascii_word(display_t* d, view_t* v, word_bitmap_t* wb)
{
    assert(d && v && wb && "arg failed!");

    const uint8_t* buffer = (const uint8_t*)wb->ascii;
    static unsigned char key[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
    size_t next_x = v->now_x;
    size_t next_y = v->now_y;
    int success = 0;

    for (int k = 0; k < FONT_HEIGHT_WORD_SIZE; ++k, next_y += 1) {
        for (int i = 0; i < BIT_SIZE; ++i, next_x += 1) {
            success = 0;
            int ret = display_cul_next_line(d, v, &next_x, &next_y, GB2312_ASCII);
            if (ret < 0) {
                LOG_ERR("fail to draw ascii nx(%zu) ny(%zu) next line.\n", next_x, next_y);
                return;
            }

            int flag = buffer[k * 1] & key[i];
            display_set_cache_color(d, next_x, next_y, flag ? v->font_color : COLOR_BLACK);
            success = 1;
        }
        if (success)
            next_x = v->now_x;
    }
    if (success) {
        v->now_x += ASCII_WORD_SIZE;
        display_cul_next_line(d, v, &v->now_x, &v->now_y, GB2312_ASCII);
    }
}

static inline void display_draw_zh_word(display_t* d, view_t* v, word_bitmap_t* wb)
{
    assert(d && v && wb && "arg failed!");

    const uint8_t* buffer = (const uint8_t*)wb->zh;
    static unsigned char key[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
    size_t next_x = v->now_x;
    size_t next_y = v->now_y;
    int success = 0;

    for (int k = 0; k < FONT_HEIGHT_WORD_SIZE; ++k, next_y += 1) {
        for (int j = 0; j < GB2312_ZH_BIT; ++j) {
            success = 0;
            for (int i = 0; i < BIT_SIZE; ++i, next_x += 1) {
                gb2312_word_type_t break_line = (j == 0 ? GB2312_CHINESE : GB2312_ASCII); // 第二边界有可能又越界
                int ret = display_cul_next_line(d, v, &next_x, &next_y, break_line);
                if (ret < 0) {
                    LOG_ERR("fail to draw zh nx(%zu) ny(%zu) next line.\n", next_x, next_y);
                    return;
                }
                int flag = buffer[k * 2 + j] & key[i];
                display_set_cache_color(d, next_x, next_y, flag ? v->font_color : COLOR_BLACK);
                success = 1;
            }
        }
        if (success) { }
        next_x = v->now_x;
    }
    if (success) {
        v->now_x += ZH_WORD_SIZE;
        display_cul_next_line(d, v, &v->now_x, &v->now_y, GB2312_CHINESE);
    }
}

int display_view_print(display_t* d, view_t* v, const char* str, size_t str_len)
{
    if (!d || !str || !str_len) {
        return 0;
    }

    for (size_t i = 0; i < str_len; i) {
        // 有个字符是结束标志故判断没关系.
        const uint8_t* gb = (const uint8_t*)str + i;
        word_bitmap_t* wb = gb2312_to_word_bitmap(d->font, gb);
        gb2312_word_type_t type = get_gb2312_word_type(gb);
        switch (type) {
        case GB2312_ASCII:
            LOG_DBG("try draw ascii: %c\n", str[i]);

            switch (*gb) {
            case '\n':
                display_draw_tabs_word(d, v);
                break;
            case '\t':
                display_draw_endl_word(d, v);
                break;
            default:
                break;
            }
            if (isgraph(*gb))
                display_draw_ascii_word(d, v, wb);

            i += GB2312_ASCII_BIT;
            continue;
        case GB2312_CHINESE:
            LOG_DBG("try draw zh(0x%x, 0x%x) \n", str[i], str[i + 1]);
            display_draw_zh_word(d, v, wb);
            i += GB2312_ZH_BIT;
            continue;
            ;
        default:
            i += sizeof(char);
            continue;
        }
    }

    return 0;
}

void view_cache_clear(display_t* d, view_t* v)
{
    size_t start_offset = 0;
    for (size_t i = v->start_y; i < v->height; ++i) {
        start_offset = display_cul_cache_offset(d, v->start_x, i);
        memset(d->cache + start_offset, COLOR_BLACK, v->width);
    }
}

/*
 *
 *   +-----+
 *   |clear|  assistant
 *   |-----|
 *   |     |  user
 *   +-----+
 *
 * */
inline void assistant_view_clear(display_t* d, view_t* v)
{
    view_cache_clear(d, v);
}

int assistant_view_print(display_t* d, view_t* v, const char* str, size_t str_len)
{
    assert(d && v && "arg failed.");
    if (!str || !str_len) {
        return 0;
    }
    assistant_view_clear(d, v);
    static const char* ai_prefic = "AI: ";
    v->now_x = 0;
    v->now_y = 0;

    display_view_print(d, v, ai_prefic, strlen(ai_prefic));
    v->now_x = strlen(ai_prefic) * GB2312_ASCII_BIT * BIT_SIZE;
    v->now_y = 0;

    display_view_print(d, v, str, str_len);

    display_fflush(&g_display);
    return 0;
}

/*
 *
 *   +-----+
 *   |     |  assistant
 *   |-----|
 *   |clear|  user
 *   +-----+
 *
 * */
inline void user_view_clear(display_t* d, view_t* v)
{
    view_cache_clear(d, v);
}

int user_view_print(display_t* d, view_t* v, const char* str, size_t str_len)
{
    assert(d && v && "arg failed.");
    if (!str || !str_len) {
        return 0;
    }
    user_view_clear(d, v);
    static const char* user_prefic = "USER: ";
    v->now_x = 0;
    v->now_y = ROLE_USER_HEIGHT_START;

    display_view_print(d, v, user_prefic, strlen(user_prefic));
    v->now_x = strlen(user_prefic) * GB2312_ASCII_BIT * BIT_SIZE;
    v->now_y = ROLE_USER_HEIGHT_START;

    display_view_print(d, v, str, str_len);

    display_fflush(&g_display);
    return 0;
}

void display_exit(display_t* d)
{
    if (d->font) {
        font_bitmap_exit(d->font);
        d->font = NULL;
    }
    if (d->fb_info) {
        framebuffer_exit(d->fb_info);
        d->fb_info = NULL;
    }
    if (d->cache) {
        free(d->cache);
        d->cache = NULL;
        d->cache_size = 0;
    }
}

static inline void display_cache_clear(display_t* d)
{
    assert(d && "arg failed.");
    memset(g_display.cache, COLOR_BLACK, g_display.cache_size);
    display_fflush(&g_display);
}

display_t* display_init()
{
    if (g_display.fb_info) {
        return &g_display;
    }
    g_display.font = font_bitmap_init();
    if (!g_display.font) {
        LOG_ERR("fail to init font.\n");
        return NULL;
    }

    g_display.fb_info = framebuffer_init();
    if (!g_display.fb_info) {
        LOG_ERR("fail to init framebuffer.\n");
        display_exit(&g_display);
        return NULL;
    }

    g_display.cache_size = g_display.fb_info->screen_size;
    g_display.cache = (uint8_t*)malloc(g_display.cache_size);
    if (!g_display.cache) {
        LOG_ERR("fail to malloc display.\n");
        display_exit(&g_display);
        return NULL;
    }
    display_cache_clear(&g_display);

    return &g_display;
}

#define __XTEST__
#ifdef __XTEST__

#include <iostream>
#include <string>

int main(void)
{
    std::string input;
    std::cin >> input;

    std::cout << "input len: " << input.length() << "\n";

    display_t* d = display_init();
    if (!d) {
        LOG_ERR("fail to init display.");
        return -1;
    }
    LOG_DBG("0\n");

    view_t v = {
        .start_x = 0,
        .start_y = 0,
        .width = d->fb_info->width,
        .height = ROLE_USER_HEIGHT_START,
        .now_x = 0,
        .now_y = 0,
        .font_color = COLOR_WHITE
    };
    view_t uv = {
        .start_x = 0,
        .start_y = ROLE_USER_HEIGHT_START,
        .width = d->fb_info->width,
        .height = d->fb_info->height - ROLE_USER_HEIGHT_START,
        .now_x = 0,
        .now_y = ROLE_USER_HEIGHT_START,
        .font_color = COLOR_WHITE,
    };

    display_view_print(d, &v, input.c_str(), input.length());
    //assistant_view_print(d, &v, input.c_str(), input.length());


    display_fflush(d);

    display_exit(d);

    return 0;
}

#endif //__XTEST__