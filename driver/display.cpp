#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

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
 *    ——  v
 *       y++
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
    framebuffer_color_t font_color;
    font_bitmap_t* font;
    framebuffer_t* fb_info;
    uint8_t* cache;
} display_t;

static display_t g_display = { 0 };

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

static inline int display_cul_next_line(display_t* d, size_t* x, size_t* y, gb2312_word_type_t type)
{
    size_t next_x = *x;
    size_t next_y = *y;
    size_t space = (type == GB2312_CHINESE ? ZH_WORD_SIZE : ASCII_WORD_SIZE);

    if (next_x >= d->fb_info->width || next_x + space >= d->fb_info->width) {
        next_y += FONT_HEIGHT_WORD_SIZE;
        next_x = 0;// 右侧剩余空间不够了
        display_cul_next_line(d, &next_x, &next_y, type);
    }

    if (next_y >= d->fb_info->height) {
        next_y = 0;
        display_cul_next_line(d, &next_x, &next_y, type);
    }

    if (next_x > d->fb_info->width || next_y > d->fb_info->height) {
        LOG_ERR("cul next x(%zu) or y(%zu) to nx(%zu) ny(%zu) fail. ",
            *x, *y, next_x, next_y);
        return -1;
    }
    *x = next_x;
    *y = next_y;
    return 0;
}

static void display_draw_space_word(display_t* d, size_t* x, size_t* y)
{
    size_t next_x = *x + ASCII_WORD_SIZE;
    size_t next_y = *y;
    int ret = display_cul_next_line(d, &next_x, &next_y, GB2312_ASCII);
    if (ret < 0) {
        LOG_DBG("display over flow! cx(%zu) y(%zu)\n", next_x, next_y);
        return;
    }
    *x = next_x;
    *y = next_y;
}

static inline void display_draw_tabs_word(display_t* d, size_t* x, size_t* y)
{
    for (size_t i = 0; i < DISPLAY_TABS_OF_SPACE; ++i)
        display_draw_space_word(d, x, y);
}

static inline void display_draw_endl_word(display_t* d, size_t* x, size_t* y)
{
    size_t next_x = 0;
    size_t next_y = *y + FONT_HEIGHT_WORD_SIZE;
    int ret = display_cul_next_line(d, &next_x, &next_y, GB2312_ASCII);
    if (ret < 0) {
        LOG_ERR("fail to draw endl, x(%zu) y(%zu) to nx(%zu), ny(%zu)",
            *x, *y, next_x, next_y);
        return;
    }
    *x = next_x;
    *y = next_y;
}

static inline void display_draw_ascii_word(display_t* d, size_t* x, size_t* y, word_bitmap_t* wb)
{
    assert(d && x && y && wb && "arg failed!");

    const uint8_t* buffer = (const uint8_t*)wb->ascii;
    static unsigned char key[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
    size_t next_x = *x;
    size_t next_y = *y;
    int success = 0;

    for (int k = 0; k < FONT_HEIGHT_WORD_SIZE; ++k, next_y += 1) {
        for (int i = 0; i < BIT_SIZE; ++i, next_x += 1) {
            success = 0;
            int ret = display_cul_next_line(d, &next_x, &next_y, GB2312_ASCII);
            if (ret < 0) {
                LOG_ERR("fail to draw ascii nx(%zu) ny(%zu) next line.\n", next_x, next_y);
                return;
            }

            int flag = buffer[k * 1] & key[i];
            display_set_cache_color(d, next_x, next_y, flag ? d->font_color : COLOR_BLACK);
            success = 1;
        }
        if (success)
            next_x = *x;
    }
    if (success) {
        *x += ASCII_WORD_SIZE;
        display_cul_next_line(d, x, y, GB2312_ASCII);
    }
}

static inline void display_draw_zh_word(display_t* d, size_t* x, size_t* y, word_bitmap_t* wb)
{
    assert(d && x && y && wb && "arg failed!");

    const uint8_t* buffer = (const uint8_t*)wb->zh;
    static unsigned char key[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
    size_t next_x = *x;
    size_t next_y = *y;
    int success = 0;

    for (int k = 0; k < FONT_HEIGHT_WORD_SIZE; ++k, next_y += 1) {
        for (int j = 0; j < GB2312_ZH_BIT; ++j) {
            success = 0;
            for (int i = 0; i < BIT_SIZE; ++i, next_x += 1) {
                gb2312_word_type_t break_line = (j == 0 ? GB2312_CHINESE : GB2312_ASCII); // 第二边界有可能又越界
                int ret = display_cul_next_line(d, &next_x, &next_y, break_line);
                if (ret < 0) {
                    LOG_ERR("fail to draw zh nx(%zu) ny(%zu) next line.\n", next_x, next_y);
                    return;
                }
                int flag = buffer[k * 2 + j] & key[i];
                display_set_cache_color(d, next_x, next_y, flag ? d->font_color : COLOR_BLACK);
                success = 1;
            }
        }
        if (success) {}
            next_x = *x;
    }
    if (success) {
        *x += ZH_WORD_SIZE;
        display_cul_next_line(d, x, y, GB2312_CHINESE);
    }
}

int display_view_print(display_t* d, const char* str, size_t str_len, size_t cur_x, size_t cur_y)
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
                display_draw_tabs_word(d, &cur_x, &cur_y);
                break;
            case '\t':
                display_draw_endl_word(d, &cur_x, &cur_y);
                break;
            default:
                break;
            }
            if (isgraph(*gb))
                display_draw_ascii_word(d, &cur_x, &cur_y, wb);

            i += GB2312_ASCII_BIT;
            continue;
        case GB2312_CHINESE:
            LOG_DBG("try draw zh(0x%x, 0x%x) \n", str[i], str[i+1]);
            display_draw_zh_word(d, &cur_x, &cur_y, wb);
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

/*
 *
 *   +-----+
 *   |clear|  assistant
 *   |-----|
 *   |     |  user
 *   +-----+
 * 
 * */
void assistant_view_clear(display_t *d)
{
    size_t offset = display_cul_cache_offset(d, d->fb_info->width-1, ROLE_USER_HEIGHT_START);
    memset(d->cache, COLOR_BLACK, offset);
}

int assistant_view_print(display_t* d, const char* str, size_t str_len)
{
    assert(d && "arg failed.");
    if (!str || !str_len) {
        return 0;
    }
    assistant_view_clear(d);
    static const char *ai_prefic = "AI: ";
    display_view_print(d, ai_prefic, strlen(ai_prefic), 0, 0);
    const size_t now_x = strlen(ai_prefic)*GB2312_ASCII_BIT*BIT_SIZE;

    display_view_print(d, str, str_len, now_x, 0);
    
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
void user_view_clear(display_t *d)
{
    size_t assistant_offset = display_cul_cache_offset(d, d->fb_info->width-1, ROLE_USER_HEIGHT_START);
    memset(d->cache + assistant_offset, COLOR_BLACK, d->cache_size - assistant_offset - 1);
}

int user_view_print(display_t* d, char* str, size_t str_len)
{
    assert(d && "arg failed.");
    if (!str || !str_len) {
        return 0;
    }
    user_view_clear(d);
    static const char *user_prefic = "USER: ";

    display_view_print(d, user_prefic, strlen(user_prefic), 0, ROLE_USER_HEIGHT_START);
    const size_t now_x = strlen(user_prefic)*GB2312_ASCII_BIT*BIT_SIZE;

    display_view_print(d, str, str_len, now_x, ROLE_USER_HEIGHT_START);
    
    display_fflush(&g_display);
    return 0;
}

inline void display_set_font_color(display_t* d, framebuffer_color_t color)
{
    d->font_color = color;
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

static inline void display_view_clear(display_t *d)
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
    display_view_clear(&g_display);

    display_set_font_color(&g_display, COLOR_WHITE);

    return &g_display;
}

#define __XTEST__
#ifdef __XTEST__

#include <string>
#include <iostream>

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

    display_view_print(d, input.c_str(), input.length(), 0, 0);
    display_fflush(d);


    display_exit(d);

    return 0;
}

#endif//__XTEST__