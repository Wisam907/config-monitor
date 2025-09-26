/*
 * @Author: Macadaimas--309533080@qq.com
 * @Date: 2025-09-26 22:14:52
 * @LastEditors: Macadaimas--309533080@qq.com
 * @LastEditTime: 2025-09-26 23:01:46
 * @Description: 
 * @FilePath: /config_monitor/include/config_monitor.h
 * 
 */
#ifndef CONFIG_MONITOR_H
#define CONFIG_MONITOR_H

#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VERSION_SAFE_RESET (UINT_MAX - 1000)

typedef void (*config_apply_fn)(const void* cfg, size_t size, void* ctx);

typedef bool (*config_filter_fn)(const void* new_cfg, size_t new_size,
                                 const void* old_cfg, size_t old_size,
                                 void* ctx);

typedef struct config_monitor config_monitor_t;

config_monitor_t* config_monitor_create(config_apply_fn apply_fn,
                                        config_filter_fn filter_fn,
                                        void* ctx);

void config_monitor_update(config_monitor_t* m, const void* cfg, size_t size);

void* config_monitor_worker(void* arg);

void config_monitor_shutdown(config_monitor_t* m);

void config_monitor_destroy(config_monitor_t* m);

#ifdef __cplusplus
}
#endif // CONFIG_MONITOR_H

#endif