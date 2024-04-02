#ifndef __DRIVER_DEBUG_H__
#define __DRIVER_DEBUG_H__

#define LOG_DBG(fmt, ...) fprintf(stdout, "[%s:%s:%u] " fmt "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) fprintf(stderr, "[%s:%s:%u] " fmt "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#endif//__DRIVER_DEBUG_H__