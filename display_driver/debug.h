#ifndef __DRIVER_DEBUG_H__
#define __DRIVER_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DETAIL_LOG_ENABLE (0)

#define LOG_DBG(fmt, ...) if (g_dbg_enable) fprintf(stdout, "[%s:%s:%u] " fmt "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) if (g_dbg_enable) fprintf(stderr, "[%s:%s:%u] " fmt "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

extern char g_dbg_enable;

#ifdef __cplusplus
}
#endif

#endif//__DRIVER_DEBUG_H__