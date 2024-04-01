#include <assert.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>

#include "font_bitmap.h"

#define WORD_ASCII_MAX_SIZE (5 * 1024LU)
#define WORD_ZH_MAP_MAX_SIZE (257 * 1024LU)

typedef struct font_data_t {
    size_t size;
    uint8_t data[0];
} font_data_t;

#define LOG_DBG(fmt, ...) fprintf(stdout, fmt, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

int is_gb2312_chinese(const uint8_t* gb)
{
    if (gb[0] >= 0xB0 && gb[0] <= 0xF7 && gb[1] >= 0xA1 && gb[1] <= 0xFE) {
        return 1;
    }
    return 0;
}

int is_gb2312_ascii(const uint8_t* gb)
{
    if (gb[0] <= 0xff)
        return 1;
    return 0;
}

void unload_font(font_data_t* map)
{
    if (!map)
        return;
    free(map);
}

font_data_t* load_font(const char* filename, size_t max_data_size)
{
    FILE* fphzk = NULL;
    fphzk = fopen(filename, "rb");
    if (fphzk == NULL) {
        fprintf(stderr, "error hzk16\n");
        return NULL;
    }
    fseek(fphzk, 0, SEEK_END);
    long file_size = ftell(fphzk);
    if (file_size + 1 > max_data_size) {
        LOG_ERR("font file too long! %ld > %ld\n", file_size, max_data_size);
        return NULL;
    }
    fseek(fphzk, 0, SEEK_SET);

    size_t map_size = sizeof(font_data_t) + file_size + 1;
    font_data_t* map = (font_data_t*)malloc(map_size);
    if (!map) {
        LOG_ERR("fail to malloc size(%zu) word map. \n", map_size);
        return NULL;
    }
    memset(map, 0, map_size);
    map->size = map_size;

    int ret = fread((char*)map->data, sizeof(char), file_size, fphzk);
    if (ret <= 0) {
        LOG_ERR("fail to read font: %d\n", ret);
        return NULL;
    }
    fclose(fphzk);

    return map;
}

void font_bitmap_exit(font_bitmap_t *fb)
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

font_bitmap_t *font_bitmap_init()
{
    font_bitmap_t *fb = (font_bitmap_t *)malloc(sizeof(font_bitmap_t));
    if (!fb) {
        LOG_ERR("fail to malloc font bitmap");
        return NULL;
    }
    memset(fb, 0, sizeof(font_bitmap_t));

    fb->ascii = load_font("ASC8x16", WORD_ASCII_MAX_SIZE);
    if (!fb->ascii) {
        LOG_ERR("fail to load ascii\n");
        unload_font(fb->ascii);
        return NULL;
    }
    fb->zh = load_font("hzk16x16h", WORD_ZH_MAP_MAX_SIZE);
    if (!fb->zh) {
        LOG_ERR("fail to load zh\n");
        unload_font(fb->zh);
        return NULL;
    }

    return fb;
}

word_bitmap_t* gb2312_ascii_to_word_bitmap(const font_data_t* wm, const uint8_t* gb)
{
    assert(gb && wm && wm->size && "arg is null");
    int offset = 0;
    word_bitmap_t* p_word = NULL;

    offset = ((unsigned int)(gb[0])) * 16;
    if (offset + 32 > wm->size) {
        LOG_DBG("ascii word(%x) offset overload: offset(%d)+32 > size(%zu)\n", *gb, offset, wm->size);
        return NULL;
    }
    p_word = (word_bitmap_t*)(wm->data + offset);

    return p_word;
}

word_bitmap_t* gb2312_zh_to_word_bitmap(const font_data_t* wm, const uint8_t* gb)
{
    assert(gb && wm && wm->size && "arg is null");
    int offset = 0;
    word_bitmap_t* p_word = NULL;

    offset = (94 * (unsigned int)(gb[0] - 0xa0 - 1) + (gb[1] - 0xa0 - 1)) * 32;
    if (offset + 32 > wm->size) {
        LOG_DBG("zh word(%x, %x) offset overload: offset(%d)+32 > size(%zu)", gb[0], gb[1], offset, wm->size);
        return NULL;
    }
    p_word = (word_bitmap_t*)(wm->data + offset);

    return p_word;
}

word_bitmap_t* gb2312_to_word_bitmap(const font_data_t* wm, const uint8_t* gb)
{
    if (is_gb2312_ascii(gb))
        return gb2312_ascii_to_word_bitmap(wm, gb);
    return gb2312_zh_to_word_bitmap(wm, gb);
}

#define __XTEST__
#ifdef __XTEST__

void display_ascii_word(word_bitmap_t* p_word)
{
    assert(p_word);

    const uint8_t* buffer = (const uint8_t*)p_word;
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

    const uint8_t* buffer = (const uint8_t*)p_word;
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

void display_word(word_bitmap_t* p_word, int is_zh)
{
    if (!is_zh)
        return display_ascii_word(p_word);
    return display_zh_word(p_word);
}

int main(void)
{
    std::string input;
    std::cin >> input;

    std::cout << "input len: " << input.length() << "\n";

    font_bitmap_t* wm = font_bitmap_init();
    if (!wm) {
        LOG_ERR("fail to init font.");
        return -1;
    }

    for (size_t i = 0; i < input.length(); i) {
        // 有个字符是结束标志故判断没关系.
        const uint8_t* gb = (const uint8_t*)(input.c_str() + i);
        int is_zh = is_gb2312_chinese(gb);
        if (!is_zh) {
            LOG_DBG("skip input[%zu] ch: %c\n", i, input[i]);
            word_bitmap_t* wb = gb2312_ascii_to_word_bitmap(wm->ascii, gb);
            if (!wb) {
                LOG_ERR("fail to read gb(%hhu)\n", gb[0]);
                return -1;
            }
                
            display_word(wb, is_zh);
            i += 1;
            continue;
        }

        word_bitmap_t* wb = gb2312_zh_to_word_bitmap(wm->zh, gb);
        if (!wb) {
            LOG_ERR("fail to read gb(%hhu, %hhu)\n", gb[0], gb[1]);
            return -1;
        }
        display_word(wb, is_zh);

        // gb2312 一个汉字2个字节.
        i += 2;
    }

    font_bitmap_exit(wm);

    return 0;
}

#endif //__XTEST__
