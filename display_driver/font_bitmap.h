#ifndef __FONT_BITMAP_H__
#define __FONT_BITMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// 功能: 读取GB2312点阵字体资源文件, 转化为点阵字结构
// example: font_bitmap.c::main()

#include "framebuffer.h"

#define GB2312_ZH_BIT (sizeof(uint16_t))   // 中文 单字占用字节数
#define GB2312_ASCII_BIT (sizeof(uint8_t)) // ASCII 单字占用字节数

struct font_data_t;
#define BIT_SIZE (8) // 位大小
#define ZH_WORD_SIZE (GB2312_ZH_BIT * BIT_SIZE)       // 16 * FRAMEBUFFER==2 中文字体 BIT 大小
#define ASCII_WORD_SIZE (GB2312_ASCII_BIT * BIT_SIZE) // ASCII 字体 BIT 大小
#define FONT_HEIGHT_WORD_SIZE (16) // 字体高度

typedef union {
    uint8_t zh[ZH_WORD_SIZE];        // width-16bit, height-16bit
    uint8_t ascii[ASCII_WORD_SIZE];  // width-8bit, height-16bit
} word_bitmap_t;

typedef struct font_bitmap_t {
    font_data_t* ascii;  // ascii 的字体
    font_data_t* zh;     // gb2312 的字体
} font_bitmap_t;

typedef enum gb2312_word_type_t {
    GB2312_UNDEFINE = 0, // 未定义
    GB2312_ASCII,        // ASCII 类型字符
    GB2312_CHINESE,      // GB2312 中文字符类型
} gb2312_word_type_t;

/**
 * 释放字体位图所占用的内存空间。
 *
 * @param map 指向字体位图的指针。
 */
void font_bitmap_exit(font_bitmap_t* map);

/**
 * 初始化字体位图。
 *
 * @param font_path 字体文件的路径。
 * @return 指向初始化后的字体位图的指针。
 */
font_bitmap_t* font_bitmap_init(const char* font_path);

/**
 * 获取 GB2312 编码字符的类型。
 *
 * @param gb 指向 GB2312 编码的字符的指针。
 * @return GB2312 编码字符的类型。
 */
gb2312_word_type_t get_gb2312_word_type(const uint8_t* gb);

/**
 * 判断是否为 GB2312 编码的中文字符。
 *
 * @param gb 指向 GB2312 编码的字符的指针。
 * @return 如果是中文字符则返回 1，否则返回 0。
 */
int is_gb2312_chinese(const uint8_t* gb);

/**
 * 判断是否为 GB2312 编码的 ASCII 字符。
 *
 * @param gb 指向 GB2312 编码的字符的指针。
 * @return 如果是 ASCII 字符则返回 1，否则返回 0。
 */
int is_gb2312_ascii(const uint8_t* gb);

/**
 * 使用特定字体将 GB2312 编码的中文字符转换为字位图。
 *
 * @param wm 指向字体数据的指针。
 * @param gb 指向 GB2312 编码的字符的指针。
 * @return 指向生成的字位图的指针。
 */
word_bitmap_t* gb2312_zh_to_word_bitmap(const font_data_t* wm, const uint8_t* gb);

/**
 * 使用特定字体将 GB2312 编码的 ASCII 字符转换为字位图。
 *
 * @param wm 指向字体数据的指针。
 * @param gb 指向 GB2312 编码的 ASCII 字符的指针。
 * @return 指向生成的字位图的指针。
 */
word_bitmap_t* gb2312_ascii_to_word_bitmap(const font_data_t* wm, const uint8_t* gb);

/**
 * 使用特定字体将 GB2312 编码的字符转换为字位图。
 *
 * @param wm 指向字体位图数据的指针。
 * @param gb 指向 GB2312 编码的字符的指针。
 * @return 指向生成的字位图的指针。
 */
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