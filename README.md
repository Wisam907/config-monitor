# config-monitor

一个轻量级的 **配置监控与应用组件**，适合在 **C/C++ 项目** 中使用。  
支持 **异步配置更新**、**重复更新过滤**、**线程安全**，并保证 **apply 执行期间不会被中途的配置更新干扰**。  

---

## ✨ 功能特性

- **异步更新**  
  配置更新不会阻塞调用线程，实际应用在后台 worker 线程执行。

- **过滤策略**  
  允许用户定义 `filter` 回调，自动跳过无效或无意义的配置变化。  
  （例如 WiFi 配置频繁切换时，只应用最终稳定的配置）

- **线程安全**  
  内部使用 `pthread_mutex` 与 `pthread_cond` 保证线程安全。

- **Apply 原子性**  
  Worker 在执行一次 apply 时，**不会被后续 update 打断**。  
  新的配置会在当前 apply 完成后立即应用，确保执行完整性。

- **可移植**  
  只依赖标准 C 和 `pthread`，支持 Linux / RTOS / POSIX 系统。

---

## 📦 项目结构

```

config-monitor/
├── CMakeLists.txt       # 构建脚本
├── include/
│   └── config_monitor.h # 头文件
├── src/
│   └── config_monitor.c # 实现
└── examples/
└── main.c           # 使用示例

```

---

## 🚀 快速开始

### 构建

```bash
git clone https://github.com/yourname/config-monitor.git
cd config-monitor
mkdir build && cd build
cmake ..
make
```

### 安装

```bash
sudo make install
```

### 在其他项目中使用

```cmake
find_package(config-monitor 1.0 REQUIRED)
target_link_libraries(myapp PRIVATE config-monitor::config-monitor)
```

---

## 🛠️ 使用示例

定义一个配置结构体：

```c
typedef struct {
    int mode;
    int param;
} my_config_t;
```

实现应用函数（支持长时间执行）：

```c
void apply_my_config(const void* cfg, size_t size, void* ctx) {
    const my_config_t* c = (const my_config_t*)cfg;
    printf("[apply] start mode=%d, param=%d\n", c->mode, c->param);
    sleep(2); // 模拟耗时操作
    printf("[apply] end mode=%d, param=%d\n", c->mode, c->param);
}
```

实现过滤函数（可选）：

```c
bool my_filter(const void* new_cfg, size_t new_size,
               const void* old_cfg, size_t old_size,
               void* ctx) {
    const my_config_t* n = (const my_config_t*)new_cfg;
    const my_config_t* o = (const my_config_t*)old_cfg;
    if (!old_cfg) return true;
    if (n->mode != o->mode) return true;
    if (abs(n->param - o->param) > 5) return true;
    return false; // 变化太小，跳过
}
```

初始化并运行：

```c
config_monitor_t* monitor = config_monitor_create(apply_my_config, my_filter, NULL);
pthread_t th;
pthread_create(&th, NULL, config_monitor_worker, monitor);

// 投递配置
my_config_t cfg = { .mode = 0, .param = 1 };
config_monitor_update(monitor, &cfg, sizeof(cfg));

cfg = (my_config_t){ .mode = 0, .param = 10 }; // apply 期间更新，会在当前 apply 完成后应用
config_monitor_update(monitor, &cfg, sizeof(cfg));

cfg = (my_config_t){ .mode = 1, .param = 20 };
config_monitor_update(monitor, &cfg, sizeof(cfg));

// 等待一段时间，模拟操作
sleep(5);

// 退出
config_monitor_shutdown(monitor);
pthread_join(th, NULL);
config_monitor_destroy(monitor);
```

---

### 运行示例输出

```
[apply] start mode=1, param=1
[apply] end mode=1, param=1
[apply] start mode=3, param=20
[apply] end mode=3, param=20
```

说明：

1. **apply 执行期间的更新不会中断当前 apply**
2. **中间无效参数被 filter_fn 丢弃**
3. **最新有效配置在 apply 完成后立即执行**

---

## 📜 License

MIT License. 欢迎自由使用、修改和分发。

---

## 💡 应用场景

* **WiFi 参数管理**：忽略中间频繁变化，只应用最终配置
* **IoT 设备配置**：远程下发参数时避免重复执行
* **网络模块切换**：AP / STA 模式切换时只应用最新一次
* **任何需要“配置过滤 + 原子应用”的场景
