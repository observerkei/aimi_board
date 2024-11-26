#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
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

#define DISPLAY_TABS_OF_SPACE (2)
#define DISPLAY_SPACE_WIDTH_BIT (ASCII_WORD_SIZE)
#define DISPLAY_SPACE_HEIGHT_BIT (FONT_HEIGHT_WORD_SIZE)

char g_dbg_enable = 1;

typedef struct display_t {
    size_t cache_size;       // 显示缓存大小
    size_t conv_gb2312_size; // 字体转码缓存大小
    font_bitmap_t* font;     // 字体内存
    framebuffer_t* fb_info;  // fb内存
    uint8_t* cache;          // 显示缓存地址
    char* conv_gb2312_cache; // 字体转码缓存地址
} display_t;

/**
 * @brief 增加视图的 x 坐标值。
 *
 * 这个函数通过给定的增量值 `add` 来调整视图 `v` 的 x 坐标。
 *
 * @param v 指向要修改的视图的指针。
 * @param add 要增加的 x 坐标值。
 * @return 返回修改后的 x 坐标值。
 */
static inline size_t view_add_x(view_t* v, size_t add)
{
    return (v->now_x + add > v->start_x + v->width) ? 0 : v->now_x + add;
}

/**
 * @brief 增加视图的 Y 坐标值
 *
 * 此函数根据给定的增量值 `add` 修改视图 `v` 的 Y 坐标。
 * 
 * @param v 指向视图的指针，表示要修改的视图。
 * @param add 要增加的 Y 坐标值。
 * @return 修改后的 Y 坐标值。
 */
static inline size_t view_add_y(view_t* v, size_t add)
{
    return (v->now_y + add > v->start_y + v->height) ? 0 : v->now_y + add;
}

/**
 * @brief 设置调试模式
 *
 * 该函数用于启用或禁用显示模块的调试模式。
 *
 * @param enable 如果为非零值，则启用调试模式；如果为零，则禁用调试模式。
 */
void display_set_debug(char enable)
{
    g_dbg_enable = enable;
}

/**
 * @brief 计算并返回在显示缓存中给定坐标 (x, y) 的偏移量。
 *
 * @param d 指向 display_t 结构体的指针，该结构体包含显示缓存的信息。
 * @param x 指定的 x 坐标，用于计算偏移量。
 * @param y 指定的 y 坐标，用于计算偏移量。
 * @return 返回指定坐标 (x, y) 在显示缓存中的偏移量，单位为字节。
 */
static inline size_t display_cul_cache_offset(display_t* d, size_t x, size_t y)
{
    size_t offset = (y * d->fb_info->width + x) * COLOR_SIZE;
    return offset < (d->cache_size - COLOR_SIZE - 1) ? offset : 0;
}

/**
 * @brief 设置显示缓存中指定位置的颜色
 *
 * 此函数用于在显示缓存中设置指定坐标 (x, y) 的颜色。
 * 
 * @param d 指向显示结构体的指针，包含显示设备的相关信息。
 * @param x 指定的 x 坐标，表示需要设置颜色的位置。
 * @param y 指定的 y 坐标，表示需要设置颜色的位置。
 * @param color 要设置的颜色
 */
inline void display_set_cache_color(display_t* d, size_t x, size_t y, framebuffer_color_t color)
{
    if (!d)
        return;
    *(framebuffer_color_t*)&d->cache[display_cul_cache_offset(d, x, y)] = color;
}

/**
 * @brief 刷新显示缓冲区
 *
 * 此函数用于刷新指定显示设备的缓冲区，确保所有待显示的内容立即输出到显示屏。
 *
 * @param d 指向 display_t 结构的指针，表示要刷新的显示设备。
 */
inline void display_fflush(display_t* d)
{
    if (!d)
        return;
    memcpy(d->fb_info->screen, d->cache, d->fb_info->screen_size);
}

/**
 * @brief 根据输入的字符类型，计算下一个绘制开始位置
 *
 * @param d 指向 display_t 类型的指针，表示显示设备的相关信息。
 * @param v 指向 view_t 类型的指针，表示当前视图的信息。
 * @param x 指向 size_t 类型的指针，表示当前行的起始 x 坐标。
 * @param y 指向 size_t 类型的指针，表示当前行的起始 y 坐标。
 * @param type gb2312_word_type_t 类型的枚举值，表示要显示的字符类型（例如，中文字符、英文字符等）。
 *
 * @return 返回值为 int 类型，通常用于指示函数执行的状态。 0 正常， 非0 异常
 */
static inline int display_cul_next_line(display_t* d, view_t* v, size_t* x, size_t* y, gb2312_word_type_t type)
{
    size_t next_x = *x;
    size_t next_y = *y;
    size_t space = (type == GB2312_CHINESE ? ZH_WORD_SIZE : ASCII_WORD_SIZE);

    size_t real_width = v->start_x + v->width >= d->fb_info->width
        ? d->fb_info->width
        : v->start_x + v->width;

    assert(real_width && "width fail!");

#if DETAIL_LOG_ENABLE
    LOG_DBG("nx(%zu), ny(%zu), real_width(%zu), v->start_x: %zu, "
        "v->width: %zu, d->fb_info->width: %zu",
        next_x, next_y, real_width, v->start_x, v->width, d->fb_info->width);
#endif // DETAIL_LOG_ENABLE

    if (next_x >= real_width || next_x + space >= real_width) {
        next_y += FONT_HEIGHT_WORD_SIZE;
        next_x = v->start_x; // 右侧剩余空间不够了
        display_cul_next_line(d, v, &next_x, &next_y, type);
    }

    size_t real_height = v->start_y + v->height >= d->fb_info->height
        ? d->fb_info->height
        : v->start_y + v->height;

    assert(real_height && "height fail!");

#if DETAIL_LOG_ENABLE
    LOG_DBG("nx(%zu), ny(%zu), real_height(%zu), v->start_y: %zu, "
        "v->height: %zu, d->fb_info->height: %zu",
        next_x, next_y, real_height, v->start_y, v->height, d->fb_info->height);
#endif // DETAIL_LOG_ENABLE

    if (next_y >= real_height) {
        next_y = v->start_y;
        display_cul_next_line(d, v, &next_x, &next_y, type);
    }

    *x = next_x;
    *y = next_y;
    return 0;
}

/**
 * @brief 绘制空格
 *
 * @param d 指向 display_t 结构的指针，表示当前显示的状态和属性。
 * @param v 指向 view_t 结构的指针，表示当前视图的设置和参数。
 */
static void display_draw_space_word(display_t* d, view_t* v)
{
    size_t next_x = view_add_x(v, ASCII_WORD_SIZE);
    size_t next_y = v->now_y;
    int ret = display_cul_next_line(d, v, &next_x, &next_y, GB2312_ASCII);
    if (ret < 0) {
        LOG_DBG("display over flow! cx(%zu) y(%zu)", next_x, next_y);
        return;
    }
    v->now_x = next_x;
    v->now_y = next_y;
}

/**
 * @brief 绘制TAB制表符
 *
 * @param d 指向 display_t 结构的指针，表示当前显示的状态和属性。
 * @param v 指向 view_t 结构的指针，表示当前视图的设置和参数。
 */
static inline void display_draw_tabs_word(display_t* d, view_t* v)
{
    for (size_t i = 0; i < DISPLAY_TABS_OF_SPACE; ++i)
        display_draw_space_word(d, v);
}

/**
 * @brief 绘制换行
 *
 * @param d 指向 display_t 结构的指针，表示当前显示的状态和属性。
 * @param v 指向 view_t 结构的指针，表示当前视图的设置和参数。
 */
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

/**
 * @brief 绘制ASCII字符
 *
 * @param d 指向 display_t 结构的指针，表示当前显示的状态和属性。
 * @param v 指向 view_t 结构的指针，表示当前视图的设置和参数。
 * @param wb ASCII字体位图
 */
static inline void display_draw_ascii_word(display_t* d, view_t* v, word_bitmap_t* wb)
{
    assert(d && v && wb && "arg failed!");

    const uint8_t* buffer = (const uint8_t*)wb->ascii;
    static unsigned char key[BIT_SIZE] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
    size_t next_x = v->now_x;
    size_t next_y = v->now_y;
    int success = 0;

    for (int k = 0; k < FONT_HEIGHT_WORD_SIZE; ++k, next_y += 1) {
        for (int i = 0; i < BIT_SIZE; ++i, next_x += 1) {
            success = 0;
            int ret = display_cul_next_line(d, v, &next_x, &next_y, GB2312_ASCII);
            if (ret < 0) {
                LOG_ERR("fail to draw ascii nx(%zu) ny(%zu) next line.", next_x, next_y);
                return;
            }

            int flag = buffer[k * 1] & key[i];
            display_set_cache_color(d, next_x, next_y, flag ? v->font_color : COLOR_BLACK);

#if DETAIL_LOG_ENABLE
            if (flag)
                LOG_DBG("set %04x in (%zu, %zu) of display(%p) done",
                    v->font_color, next_x, next_y, d);
#endif // DETAIL_LOG_ENABLE

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

/**
 * @brief 绘制GB2312中文字符
 *
 * @param d 指向 display_t 结构的指针，表示当前显示的状态和属性。
 * @param v 指向 view_t 结构的指针，表示当前视图的设置和参数。
 * @param wb GB2312中文字体位图
 */
static inline void display_draw_zh_word(display_t* d, view_t* v, word_bitmap_t* wb)
{
    assert(d && v && wb && "arg failed!");

    const uint8_t* buffer = (const uint8_t*)wb->zh;
    static unsigned char key[BIT_SIZE] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
    size_t next_x = v->now_x;
    size_t next_y = v->now_y;
    int success = 0;

    for (int k = 0; k < FONT_HEIGHT_WORD_SIZE; ++k, next_y += 1) {
        for (int j = 0; j < GB2312_ZH_BIT; ++j) {
            success = 0;
            for (int i = 0; i < BIT_SIZE; ++i, next_x += 1) {
                // 第二边界有可能又越界
                gb2312_word_type_t break_line = (j == 0 ? GB2312_CHINESE : GB2312_ASCII);
                int ret = display_cul_next_line(d, v, &next_x, &next_y, break_line);
                if (ret < 0) {
                    LOG_ERR("fail to draw zh nx(%zu) ny(%zu) next line.", next_x, next_y);
                    return;
                }
                int flag = buffer[k * 2 + j] & key[i];
                display_set_cache_color(d, next_x, next_y, flag ? v->font_color : COLOR_BLACK);

#if DETAIL_LOG_ENABLE
                if (flag)
                    LOG_DBG("set %04x in (%zu, %zu) of display(%p) done",
                        v->font_color, next_x, next_y, d);
#endif // DETAIL_LOG_ENABLE

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


/**
 * @brief 往显示上打印 GB2312中文 + ASCII字符串
 *
 * @param d 指向 display_t 结构的指针，表示当前显示的状态和属性。
 * @param v 指向 view_t 结构的指针，表示当前视图的设置和参数。
 * @param str 被打印的GB2312中文字符串
 * @param str_len 被打印的GB2312中文字符串长度
 * 
 * @return 成功返回 0 失败返回 非0
 */
static int display_view_print_gb2312(display_t* d, view_t* v, const char* str, size_t str_len)
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
            switch (*gb) {
            case '\n':
                LOG_DBG("try draw: `\\n`");
                display_draw_endl_word(d, v);
                break;
            case '\t':
                LOG_DBG("try draw: `\\t`");
                display_draw_tabs_word(d, v);
                break;
            case ' ':
                LOG_DBG("try draw: ` `");
                display_draw_space_word(d, v);
                break;
            default:
                if (isgraph(*gb)) {
                    LOG_DBG("try draw ascii: %c", str[i]);
                    display_draw_ascii_word(d, v, wb);
                } else {
                    LOG_DBG("unknow ch: 0x%02x", str[i]);
                }
                break;
            }

            i += GB2312_ASCII_BIT;
            continue;
        case GB2312_CHINESE:
            LOG_DBG("try draw zh(0x%x, 0x%x) ", str[i], str[i + 1]);
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

/**
 * @brief 拓展字体缓存
 *
 * @param d 指向 display_t 结构的指针，表示当前显示的状态和属性。
 * @param new_size 新拓展内存大小
 * 
 * @return 成功返回 0 失败返回 非0
 */
static int display_extern_conv_cache(display_t* d, size_t new_size)
{
    assert(new_size && "new_size fail!");
    char* new_buffer = (char*)malloc(new_size);
    if (!new_buffer) {
        LOG_ERR("fail to extern conv cache");
        return -1;
    }
    char* old_cache = d->conv_gb2312_cache;
    d->conv_gb2312_cache = new_buffer;
    free(old_cache);

    return 0;
}

/**
 * @brief 显示视图信息
 *
 * @param v 指向 view_t 结构的指针，表示当前视图的设置和参数。
 */
static inline void display_show_view_info(view_t* v)
{
    LOG_DBG("  start_x: %zu, start_y: %zu, width: %zu, height: %zu, "
            "now_x: %zu, now_y: %zu, start_x: %zu, start_y: %zu, font_color: %hu",
        v->start_x, v->start_y, v->width, v->height, v->now_x,
        v->now_y, v->start_x, v->start_y, v->font_color);
}

/**
 * @brief 往显示上打印文字(支持中文)
 *
 * @param d 指向 display_t 结构的指针，表示当前显示的状态和属性。
 * @param v 指向 view_t 结构的指针，表示当前视图的设置和参数。
 * @param from_code 字符编码
 * @param str 被打印的字符
 * @param str_len 被打印的字符长度
 * 
 * @return 成功返回 0 失败返回 非0
 */
int display_view_print(display_t* d, view_t* v, const char* from_code, const char* str, size_t str_len)
{
    if (!d || !v || !from_code || !str || !str_len) {
        LOG_DBG("arg failed: d(%p) v(%p) from_code(%p) str(%p) str_len(%zu) ",
            d, v, from_code, str, str_len);
        return -1;
    }
    LOG_DBG("display(%p) try print.", d);
    display_show_view_info(v);
    // 如果不是GB2312编码，则进行编码转换
    if (0 != strcasecmp("GB2312", from_code)) {
        // 如果转换编码的缓存不够，则拓展缓存
        if (str_len > d->conv_gb2312_size) {
            int ret = display_extern_conv_cache(d, str_len);
            if (ret < 0) {
                LOG_DBG("STR TOO LONG! ");
                return -1;
            }
        }
        // 将编码转换为gb2312
        int len = str_to_gb2312(from_code, str_len, str, 
            d->conv_gb2312_size, d->conv_gb2312_cache);
        if (len < 0) {
            LOG_DBG("fail to conv %s to GB2312", from_code);
            return -1;
        }
        return display_view_print_gb2312(d, v, d->conv_gb2312_cache, len);
    }
    return display_view_print_gb2312(d, v, str, str_len);
}

/**
 * @brief 清空视图
 *
 * @param d 指向 display_t 结构的指针，表示当前显示的状态和属性。
 * @param v 指向 view_t 结构的指针，表示当前视图的设置和参数。
 */
void display_view_clear(display_t* d, view_t* v)
{
    if (!d || !v)
        return;

    size_t start_offset = 0;
    size_t real_width = v->start_x + v->width >= d->fb_info->width
        ? d->fb_info->width
        : v->start_x + v->width;
    size_t real_height = v->start_y + v->height >= d->fb_info->height
        ? d->fb_info->height
        : v->start_y + v->height;

    for (size_t y = v->start_y; y < real_height; ++y) {
        start_offset = display_cul_cache_offset(d, v->start_x, y);
        memset(d->cache + start_offset, COLOR_BLACK, real_width * COLOR_SIZE);
    }
    v->now_x = v->start_x;
    v->now_y = v->start_y;
}

/**
 * @brief 清理显示资源
 *
 * @param d 指向 display_t 结构的指针，表示当前显示的状态和属性。
 */
void display_exit(display_t* d)
{
    if (!d)
        return;

    if (d->font) {
        font_bitmap_exit(d->font);
        d->font = NULL;
    }
    if (d->conv_gb2312_cache) {
        free(d->conv_gb2312_cache);
        d->conv_gb2312_cache = NULL;
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
    LOG_DBG("display(%p) clear success.", d);

    free(d);
}

/**
 * @brief 清理显示缓存
 *
 * @param d 指向 display_t 结构的指针，表示当前显示的状态和属性。
 */
static inline void display_cache_clear(display_t* d)
{
    assert(d && "arg failed.");
    memset(d->cache, COLOR_BLACK, d->cache_size);
    display_fflush(d);
}

/**
 * @brief 获取显示宽度
 *
 * @return 显示宽度
 */
size_t display_get_width(display_t* d)
{
    if (!d)
        return 0;
    return d->fb_info->width;
}

/**
 * @brief 获取显示高度
 *
 * @return 显示高度
 */
size_t display_get_height(display_t* d)
{
    if (!d)
        return 0;
    return d->fb_info->height;
}

#define DEFUALT_SIZE (1024)  // 默认字体转码缓存大小

/**
 * @brief 初始化显示
 * 
 * @param fb_dev fb设备
 * @param font_path 字体路径
 *
 * @return 成功返回 非NULL 失败返回 NULL
 */
display_t* display_init(const char* fb_dev, const char* font_path)
{
    display_t* d = (display_t*)malloc(sizeof(display_t));
    if (!d) {
        LOG_ERR("fail to malloc display");
        return NULL;
    }
    memset(d, 0, sizeof(display_t));

    d->font = font_bitmap_init(font_path);
    if (!d->font) {
        LOG_ERR("fail to init font.");
        goto err;
    }

    d->conv_gb2312_size = DEFUALT_SIZE;
    d->conv_gb2312_cache = (char*)malloc(d->conv_gb2312_size);
    if (!d->conv_gb2312_cache) {
        goto err;
    }
    memset(d->conv_gb2312_cache, 0, d->conv_gb2312_size);

    d->fb_info = framebuffer_init(fb_dev);
    if (!d->fb_info) {
        LOG_ERR("fail to init framebuffer.");
        goto err;
    }

    d->cache_size = d->fb_info->screen_size;
    d->cache = (uint8_t*)malloc(d->cache_size);
    if (!d->cache) {
        LOG_ERR("fail to malloc display.");
        goto err;
    }
    display_cache_clear(d);

    LOG_DBG("display(%p:%zu) create success.", d, d->cache_size);

    return d;
err:
    display_exit(d);
    return NULL;
}

#ifdef __DISPLAY_XTEST__

#include <iostream>
#include <string>
#include <unistd.h>

#define ROLE_ASSISTANT_HEIGHT_START (0)
#define ROLE_USER_HEIGHT_START (11 * FONT_HEIGHT_WORD_SIZE) // 共15行，分配 11 行给 AI

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
    display_view_clear(d, v);
}

int assistant_view_print(display_t* d, view_t* v, const char* from_code, const char* str, size_t str_len)
{
    assert(d && v && "arg failed.");
    if (!str || !str_len) {
        return 0;
    }
    assistant_view_clear(d, v);

    size_t len = strlen("AI: ");
    display_view_print(d, v, "UTF-8", "AI: ", len);
    display_view_print(d, v, from_code, str, str_len);

    display_fflush(d);
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
    display_view_clear(d, v);
}

int user_view_print(display_t* d, view_t* v, const char* from_code, const char* str, size_t str_len)
{
    assert(d && v && "arg failed.");
    if (!str || !str_len) {
        return 0;
    }
    user_view_clear(d, v);

    display_view_print(d, v, "UTF-8", "USER: ", strlen("USER: "));
    display_view_print(d, v, from_code, str, str_len);

    display_fflush(d);
    return 0;
}

int main(void)
{
    display_t* d = display_init("/dev/fb0", "display_driver/font");
    if (!d) {
        LOG_ERR("fail to init display.");
        return -1;
    }

    view_t av = {
        .start_x = 0,
        .start_y = 0,
        .width = display_get_width(d),
        .height = ROLE_USER_HEIGHT_START,
        .now_x = 0,
        .now_y = 0,
        .font_color = COLOR_WHITE
    };
    view_t uv = {
        .start_x = 0,
        .start_y = ROLE_USER_HEIGHT_START,
        .width = display_get_width(d),
        .height = display_get_height(d) - ROLE_USER_HEIGHT_START,
        .now_x = 0,
        .now_y = ROLE_USER_HEIGHT_START,
        .font_color = COLOR_WHITE,
    };

    std::string input;

    while (true) {
        display_view_clear(d, &uv);
        std::cout << "input user msg: \n";
        std::cin >> input;
        user_view_print(d, &uv, "UTF-8", input.c_str(), input.length());

        display_view_clear(d, &av);
        std::cout << "input assistant msg: \n";
        std::cin >> input;
        assistant_view_print(d, &av, "UTF-8", input.c_str(), input.length());

        display_fflush(d);
    }

    display_exit(d);

    return 0;
}

#endif //__DISPLAY_XTEST__