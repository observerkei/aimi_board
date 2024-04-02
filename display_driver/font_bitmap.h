#ifndef __FONT_BITMAP_H__
#define __FONT_BITMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// 功能: 读取GB2312点阵字体资源文件, 转化为点阵字结构
// example: font_bitmap.c::main()

#include "framebuffer.h"

#define GB2312_ZH_BIT (sizeof(uint16_t))
#define GB2312_ASCII_BIT (sizeof(uint8_t))

struct font_data_t;
#define BIT_SIZE (8)
#define ZH_WORD_SIZE (GB2312_ZH_BIT*BIT_SIZE) // 16 * FRAMEBUFFER==2 
#define ASCII_WORD_SIZE (GB2312_ASCII_BIT*BIT_SIZE)
#define FONT_HEIGHT_WORD_SIZE (16)

typedef union {
    uint8_t zh[ZH_WORD_SIZE]; // width-16bit, height-16bit
    uint8_t ascii[ASCII_WORD_SIZE];// width-8bit, height-16bit
} word_bitmap_t;

typedef struct font_bitmap_t {
    font_data_t *ascii;
    font_data_t *zh;
} font_bitmap_t;

typedef enum gb2312_word_type_t {
    GB2312_UNDEFINE = 0,
    GB2312_ASCII,
    GB2312_CHINESE,
} gb2312_word_type_t;

void font_bitmap_exit(font_bitmap_t* map);
font_bitmap_t *font_bitmap_init(const char *font_path);


gb2312_word_type_t get_gb2312_word_type(const uint8_t* gb);
int is_gb2312_chinese(const uint8_t* gb);
int is_gb2312_ascii(const uint8_t* gb);

word_bitmap_t* gb2312_zh_to_word_bitmap(const font_data_t* wm, const uint8_t* gb);
word_bitmap_t* gb2312_ascii_to_word_bitmap(const font_data_t* wm, const uint8_t* gb);
word_bitmap_t* gb2312_to_word_bitmap(const font_bitmap_t* wm, const uint8_t* gb);

/*
 * 任意编码到 GB2312 的转换.
 * @param from_code: 来源编码
 * @param src_size: 来源字符串长度
 * @param str: 来源字符串
 * @param dest_size: 目标缓冲区空间大小
 * @param dest: 转换结果保存缓冲区
 * @return:
 *      失败返回 < 0
 *      成功返回 使用的字符长度.
 *      
 */
int str_to_gb2312(const char* from_code, size_t src_size, const char* src, const size_t dest_size, char* dest);

#ifdef __cplusplus
}
#endif

#endif //__FONT_BITMAP_H__