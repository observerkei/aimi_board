#ifndef __FONT_BITMAP_H__
#define __FONT_BITMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// 功能: 读取GB2312点阵字体资源文件, 转化为点阵字结构
// example: font_bitmap.c::main()

struct font_data_t;
#define ZH_WORD_SIZE (32)
#define ASCII_WORD_SIZE (16)
typedef union {
    uint8_t zh[ZH_WORD_SIZE];
    uint8_t ascii[ASCII_WORD_SIZE];
} word_bitmap_t;

typedef struct font_bitmap_t {
    font_data_t *ascii;
    font_data_t *zh;
} font_bitmap_t;

void font_bitmap_exit(font_bitmap_t* map);
font_bitmap_t *font_bitmap_init();

int is_gb2312_chinese(const uint8_t* gb);
int is_gb2312_ascii(const uint8_t* gb);
word_bitmap_t* gb2312_zh_to_word_bitmap(const font_data_t* wm, const uint8_t* gb);
word_bitmap_t* gb2312_ascii_to_word_bitmap(const font_data_t* wm, const uint8_t* gb);
word_bitmap_t* gb2312_to_word_bitmap(const font_bitmap_t* wm, const uint8_t* gb);

#ifdef __cplusplus
}
#endif

#endif //__FONT_BITMAP_H__