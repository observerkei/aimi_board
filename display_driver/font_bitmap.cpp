#include <assert.h>
#include <cstdint>
#include <cstring>
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <string>

#include "debug.h"
#include "font_bitmap.h"

#define WORD_ASCII_MAX_SIZE (5 * 1024LU)     // ASCII 字体最大大小
#define WORD_ZH_MAP_MAX_SIZE (257 * 1024LU)  // 中文字体 最大大小

typedef struct font_data_t {
    size_t size;     // 字体数据大小
    uint8_t data[0]; // 字体数据
} font_data_t;

/**
 * 判断是否为 GB2312 编码的中文字符。
 *
 * @param gb 指向 GB2312 编码的字符的指针。
 * @return 如果是中文字符则返回 1，否则返回 0。
 */
int is_gb2312_chinese(const uint8_t* gb)
{
    if (gb[0] >= 0xA1 && gb[0] <= 0xFE
        && gb[1] >= 0xA1 && gb[1] <= 0xFE) {
        return 1;
    }
    return 0;
}

/**
 * 判断是否为 GB2312 编码的 ASCII 字符。
 *
 * @param gb 指向 GB2312 编码的字符的指针。
 * @return 如果是 ASCII 字符则返回 1，否则返回 0。
 */
int is_gb2312_ascii(const uint8_t* gb)
{
    if (gb[0] < 0x80)
        return 1;
    return 0;
}

/**
 * 获取 GB2312 编码字符的类型。
 *
 * @param gb 指向 GB2312 编码的字符的指针。
 * @return GB2312 编码字符的类型。
 */
gb2312_word_type_t get_gb2312_word_type(const uint8_t* gb)
{
    if (is_gb2312_ascii(gb))
        return GB2312_ASCII;
    if (is_gb2312_chinese(gb))
        return GB2312_CHINESE;
    return GB2312_UNDEFINE;
}

void unload_font(font_data_t* map)
{
    if (!map)
        return;
    free(map);
}

/**
 * 加载字体文件
 *
 * @param filename 字体文件路径
 * @param max_data_size 字体文件的最大大小
 * 
 * @return 成功返回 非NULL 失败返回 NULL
 */
font_data_t* load_font(const char* filename, size_t max_data_size)
{
    FILE* fphzk = NULL;
    fphzk = fopen(filename, "rb");
    if (fphzk == NULL) {
        fprintf(stderr, "error hzk16");
        return NULL;
    }
    fseek(fphzk, 0, SEEK_END);
    long file_size = ftell(fphzk);
    if (file_size + 1 > max_data_size) {
        LOG_ERR("font file too long! %ld > %ld", file_size, max_data_size);
        return NULL;
    }
    fseek(fphzk, 0, SEEK_SET);

    size_t map_size = sizeof(font_data_t) + file_size + 1;
    font_data_t* map = (font_data_t*)malloc(map_size);
    if (!map) {
        LOG_ERR("fail to malloc size(%zu) word map. ", map_size);
        return NULL;
    }
    memset(map, 0, map_size);
    map->size = map_size;

    int ret = fread((char*)map->data, sizeof(char), file_size, fphzk);
    if (ret <= 0) {
        LOG_ERR("fail to read font: %d", ret);
        return NULL;
    }
    fclose(fphzk);

    return map;
}

/**
 * 释放字体位图所占用的内存空间。
 *
 * @param map 指向字体位图的指针。
 */
void font_bitmap_exit(font_bitmap_t* fb)
{
    if (!fb)
        return;
    if (fb->ascii) {
        unload_font(fb->ascii);
        fb->ascii = NULL;
    }
    if (fb->zh) {
        unload_font(fb->zh);
        fb->zh = NULL;
    }
}

/**
 * 初始化字体位图。
 *
 * @param font_path 字体文件的路径。
 * @return 指向初始化后的字体位图的指针。
 */
font_bitmap_t* font_bitmap_init(const char *font_path)
{
    assert(font_path && "font path fail!");
    font_bitmap_t* fb = (font_bitmap_t*)malloc(sizeof(font_bitmap_t));
    if (!fb) {
        LOG_ERR("fail to malloc font bitmap");
        return NULL;
    }
    memset(fb, 0, sizeof(font_bitmap_t));

    std::string font_name = font_path;
    font_name += "/ascii_8x16";

    fb->ascii = load_font(font_name.c_str(), WORD_ASCII_MAX_SIZE);
    if (!fb->ascii) {
        LOG_ERR("fail to load ascii");
        unload_font(fb->ascii);
        return NULL;
    }
    LOG_DBG("load font %s", font_name.c_str());

    font_name = font_path;
    font_name += "/gb2312_16x16";
    fb->zh = load_font(font_name.c_str(), WORD_ZH_MAP_MAX_SIZE);
    if (!fb->zh) {
        LOG_ERR("fail to load zh");
        unload_font(fb->zh);
        return NULL;
    }
    LOG_DBG("load font %s", font_name.c_str());

    return fb;
err:
    return NULL;
}

/**
 * 使用特定字体将 GB2312 编码的 ASCII 字符转换为字位图。
 *
 * @param wm 指向字体数据的指针。
 * @param gb 指向 GB2312 编码的 ASCII 字符的指针。
 * @return 指向生成的字位图的指针。
 */
word_bitmap_t* gb2312_ascii_to_word_bitmap(const font_data_t* wm, const uint8_t* gb)
{
    assert(gb && wm && wm->size && "arg is null");
    int offset = 0;
    word_bitmap_t* p_word = NULL;

    offset = (uint32_t)gb[0] * FONT_HEIGHT_WORD_SIZE;
    if (offset + FONT_HEIGHT_WORD_SIZE > wm->size) {
        LOG_DBG("ascii word(%x) offset overload: offset(%d)+32 > size(%zu)", 
            *gb, offset, wm->size);
        return NULL;
    }
    p_word = (word_bitmap_t*)(wm->data + offset);

    return p_word;
}

/**
 * 使用特定字体将 GB2312 编码的中文字符转换为字位图。
 *
 * @param wm 指向字体数据的指针。
 * @param gb 指向 GB2312 编码的字符的指针。
 * @return 指向生成的字位图的指针。
 */
word_bitmap_t* gb2312_zh_to_word_bitmap(const font_data_t* wm, const uint8_t* gb)
{
    assert(gb && wm && wm->size && "arg is null");
    int offset = 0;
    word_bitmap_t* p_word = NULL;

#define GB2312_ZH_START (0xa0)
#define FONT_ZH_BITMAP_SIZE (32)
#define GB2312_ZONE_CODE_ZH_SIZE (94)

    offset = (GB2312_ZONE_CODE_ZH_SIZE * (uint32_t)(gb[0] - GB2312_ZH_START - 1) 
        + (gb[1] - GB2312_ZH_START - 1)) * FONT_ZH_BITMAP_SIZE;
    if (offset + FONT_ZH_BITMAP_SIZE > wm->size) {
        LOG_DBG("zh word(%x, %x) offset overload: offset(%d)+32 > size(%zu)", 
            gb[0], gb[1], offset, wm->size);
        return NULL;
    }
    p_word = (word_bitmap_t*)(wm->data + offset);

#undef GB2312_ZONE_CODE_ZH_SIZE
#undef FONT_ZH_BITMAP_SIZE
#undef GB2312_ZH_START

    return p_word;
}

/**
 * 使用特定字体将 GB2312 编码的字符转换为字位图。
 *
 * @param wm 指向字体位图数据的指针。
 * @param gb 指向 GB2312 编码的字符的指针。
 * @return 指向生成的字位图的指针。
 */
word_bitmap_t* gb2312_to_word_bitmap(const font_bitmap_t* wm, const uint8_t* gb)
{
    if (is_gb2312_ascii(gb))
        return gb2312_ascii_to_word_bitmap(wm->ascii, gb);
    if (is_gb2312_chinese(gb))
        return gb2312_zh_to_word_bitmap(wm->zh, gb);
    return NULL;
}

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
int str_to_gb2312(const char* from_code, size_t src_size, const char* src, const size_t dest_size, char* dest)
{
    // 创建 iconv 转换句柄 GB2312 <- from_code
    iconv_t cd = iconv_open("GB2312", from_code);
    if (cd == (iconv_t)-1) {
        LOG_ERR("fail to iconv open gb2312");
        return -1;
    }

    char* p_src = (char*)src;
    char* p_dest = dest;
    size_t leat_size = dest_size;
    if (iconv(cd, &p_src, &src_size, &p_dest, &leat_size) == (size_t)-1) {
        LOG_ERR("fail to iconv gb2312");
        iconv_close(cd);
        return -1;
    }

    // 关闭 iconv 转换句柄
    iconv_close(cd);

#if DETAIL_LOG_ENABLE
    LOG_DBG("conv %s: %s(%02x, %02x, %02x, %02x) to GB2312: %s(%02x, %02x)",
        from_code, src, src[0], src[1], src[2], src[4], dest, dest[0], dest[1]);
#endif // DETAIL_LOG_ENABLE

    return dest_size - leat_size;
}

#ifdef __XTEST__

#include <iostream>

char g_dbg_enable = 1;

void display_ascii_word(word_bitmap_t* p_word)
{
    assert(p_word);

    const uint8_t* buffer = (const uint8_t*)p_word->ascii;
    static unsigned char key[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

    for (int k = 0; k < 16; k++) {
        for (int j = 0; j < 1; j++) {
            for (int i = 0; i < 8; i++) {
                int flag = buffer[k * 1 + j] & key[i];
                printf("%s", flag ? "●" : "○");
            }
        }
        printf("\n");
    }

    for (int k = 0; k < 31; k++) {
        printf("0x%02X,", buffer[k]);
    }

    printf("\n");
}

void display_zh_word(word_bitmap_t* p_word)
{
    assert(p_word);

    const uint8_t* buffer = (const uint8_t*)p_word->zh;
    static unsigned char key[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

    for (int k = 0; k < 16; k++) {
        for (int j = 0; j < 2; j++) {
            for (int i = 0; i < 8; i++) {
                int flag = buffer[k * 2 + j] & key[i];
                printf("%s", flag ? "●" : "○");
            }
        }
        printf("\n");
    }

    for (int k = 0; k < 31; k++) {
        printf("0x%02X,", buffer[k]);
    }

    printf("\n");
}

void display_word(word_bitmap_t* p_word, gb2312_word_type_t type)
{
    if (type == GB2312_ASCII)
        return display_ascii_word(p_word);
    if (type == GB2312_CHINESE)
        return display_zh_word(p_word);
}

int main(void)
{
    std::string input;
    std::cin >> input;

    std::cout << "input len: " << input.length() << "\n";

    font_bitmap_t* wm = font_bitmap_init("display_driver/font");
    if (!wm) {
        LOG_ERR("fail to init font.");
        return -1;
    }

    for (size_t i = 0; i < input.length(); i) {
        // 有个字符是结束标志故判断没关系.
        const uint8_t* gb = (const uint8_t*)(input.c_str() + i);
        gb2312_word_type_t type = get_gb2312_word_type(gb);
        word_bitmap_t* wb = gb2312_to_word_bitmap(wm, gb);
        if (!wb) {
            LOG_ERR("fail to read gb(%hhu, %hhu)", gb[0], gb[1]);
            return -1;
        }
        display_word(wb, type);
        if (type == GB2312_CHINESE)
            i += 2;
        else
            i += 1;
        // gb2312 一个汉字2个字节.
    }

    font_bitmap_exit(wm);

    return 0;
}

#endif //__XTEST__
