/*
 * @Author: Macadaimas--309533080@qq.com
 * @Date: 2025-09-26 22:15:16
 * @LastEditors: Macadaimas--309533080@qq.com
 * @LastEditTime: 2025-09-26 23:07:48
 * @Description: 
 * @FilePath: /config_monitor/examples/main.c
 * 
 */
#include "../include/config_monitor.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct {
    int mode;
    int param;
} my_config_t;

// 模拟长时间 apply
void apply_my_config(const void* cfg, size_t size, void* ctx) {
    const my_config_t* c = (const my_config_t*)cfg;
    printf("[apply] start mode=%d, param=%d\n", c->mode, c->param);
    sleep(2); // 模拟 apply 耗时
    printf("[apply] end mode=%d, param=%d\n", c->mode, c->param);
}

// 过滤函数：只有 mode 不同或 param 差异 > 5 才应用
bool my_filter(const void* new_cfg, size_t new_size,
               const void* old_cfg, size_t old_size,
               void* ctx) {
    const my_config_t* n = (const my_config_t*)new_cfg;
    const my_config_t* o = (const my_config_t*)old_cfg;
    if (!old_cfg) return true;
    if (n->mode != o->mode) return true;
    if (abs(n->param - o->param) > 5) return true;
    return false;
}

int main() {
    pthread_t th;
    config_monitor_t* monitor = config_monitor_create(apply_my_config, my_filter, NULL);
    pthread_create(&th, NULL, config_monitor_worker, monitor);

    my_config_t cfg;

    cfg = (my_config_t){.mode = 1, .param = 1};
    config_monitor_update(monitor, &cfg, sizeof(cfg));

    sleep(1); // apply 还没完成
    cfg = (my_config_t){.mode = 2, .param = 10};
    config_monitor_update(monitor, &cfg, sizeof(cfg));

    sleep(1); // apply 还没完成
    cfg = (my_config_t){.mode = 3, .param = 20};
    config_monitor_update(monitor, &cfg, sizeof(cfg));

    sleep(5);

    config_monitor_shutdown(monitor);
    pthread_join(th, NULL);
    config_monitor_destroy(monitor);
    return 0;
}
