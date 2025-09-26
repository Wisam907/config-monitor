/*
 * @Author: Macadaimas--309533080@qq.com
 * @Date: 2025-09-26 22:15:06
 * @LastEditors: Macadaimas--309533080@qq.com
 * @LastEditTime: 2025-09-26 23:02:53
 * @Description: 
 * @FilePath: /config_monitor/src/config_monitor.c
 * 
 */
#include "../include/config_monitor.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct config_monitor {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    bool shutdown;
    unsigned version;

    void* current_cfg;
    size_t current_size;

    void* target_cfg;
    size_t target_size;

    config_apply_fn apply_fn;
    config_filter_fn filter_fn;
    void* ctx;
};

config_monitor_t* config_monitor_create(config_apply_fn apply_fn,
                                        config_filter_fn filter_fn,
                                        void* ctx) {
    config_monitor_t* m = calloc(1, sizeof(config_monitor_t));
    if (!m) return NULL;
    pthread_mutex_init(&m->lock, NULL);
    pthread_cond_init(&m->cond, NULL);
    m->apply_fn = apply_fn;
    m->filter_fn = filter_fn;
    m->ctx = ctx;
    return m;
}

void config_monitor_update(config_monitor_t* m, const void* cfg, size_t size) {
    pthread_mutex_lock(&m->lock);

    // 释放旧的 target_cfg（不影响正在 apply 的拷贝）
    if (m->target_cfg) free(m->target_cfg);

    m->target_cfg = malloc(size);
    memcpy(m->target_cfg, cfg, size);
    m->target_size = size;
    
    // 安全增加 version
    if (m->version >= VERSION_SAFE_RESET) {
        m->version = 0;
        printf("[monitor] reset version to 0 to prevent overflow\n");
    } else {
        m->version++;
    }
    pthread_cond_signal(&m->cond);
    pthread_mutex_unlock(&m->lock);
}

void* config_monitor_worker(void* arg) {
    config_monitor_t* m = (config_monitor_t*)arg;

    while (1) {
        pthread_mutex_lock(&m->lock);
        while (!m->shutdown && !m->target_cfg) {
            pthread_cond_wait(&m->cond, &m->lock);
        }

        if (m->shutdown) {
            pthread_mutex_unlock(&m->lock);
            break;
        }

        // 拷贝 target_cfg，独立应用
        void* cfg_to_apply = malloc(m->target_size);
        memcpy(cfg_to_apply, m->target_cfg, m->target_size);
        size_t size = m->target_size;
        unsigned ver = m->version;

        // 清空 target_cfg（新 update 可以覆盖这里）
        free(m->target_cfg);
        m->target_cfg = NULL;
        m->target_size = 0;

        pthread_mutex_unlock(&m->lock);

        // 执行过滤与 apply
        bool apply = true;
        if (m->filter_fn) {
            apply = m->filter_fn(cfg_to_apply, size,
                                 m->current_cfg, m->current_size,
                                 m->ctx);
        }

        if (apply) {
            if (m->apply_fn) m->apply_fn(cfg_to_apply, size, m->ctx);

            pthread_mutex_lock(&m->lock);
            if (m->current_cfg) free(m->current_cfg);
            m->current_cfg = malloc(size);
            memcpy(m->current_cfg, cfg_to_apply, size);
            m->current_size = size;
            pthread_mutex_unlock(&m->lock);
        } else {
            printf("[monitor] skipped version %u\n", ver);
        }

        free(cfg_to_apply);
        // loop 会继续检查 target_cfg，如果 update 来了，会立即应用最新配置
    }

    return NULL;
}

void config_monitor_shutdown(config_monitor_t* m) {
    pthread_mutex_lock(&m->lock);
    m->shutdown = true;
    pthread_cond_broadcast(&m->cond);
    pthread_mutex_unlock(&m->lock);
}

void config_monitor_destroy(config_monitor_t* m) {
    if (!m) return;
    if (m->current_cfg) free(m->current_cfg);
    if (m->target_cfg) free(m->target_cfg);
    pthread_mutex_destroy(&m->lock);
    pthread_cond_destroy(&m->cond);
    free(m);
}
