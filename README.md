# sherrylog

基于 [sylar](https://github.com/sylar-yin/sylar) 框架 log 模块剥离的独立日志库，可单独编译为共享库（`.so`）供其他项目引用。

## 特性

- 多级别日志：`DEBUG / INFO / WARN / ERROR / FATAL`
- 多 Appender：控制台输出（`StdoutLogAppender`）、文件输出（`FileLogAppender`）
- 可自定义日志格式（`LogFormatter` + pattern）
- 支持多 Logger 实例（按名称管理）
- 基于 YAML 配置文件动态初始化 Logger / Appender / 级别
- 配置项变更监听（`ConfigVar::addListener`）
- 线程安全（Spinlock / RWMutex 保护）

## 目录结构

```
sherrylog/
├── CMakeLists.txt          # 构建配置（支持 BUILD_TESTS 开关）
├── Makefile                # 顶层 Makefile（透传环境变量给 CMake）
├── src/                    # 库源码与头文件
│   ├── log.h / log.cc      # 核心日志模块
│   ├── config.h / config.cc# YAML 配置系统
│   ├── thread.h / thread.cc# Mutex / RWMutex / Spinlock / Thread
│   ├── util.h / util.cc    # 工具函数（时间、backtrace 等）
│   ├── singleton.h         # 单例模板
│   └── noncopyable.h       # 不可拷贝基类
├── tests/                  # 测例（BUILD_TESTS=ON 时编译）
│   ├── test_log_basic.cc   # 基础：级别、宏、fmt、named logger
│   ├── test_log_file.cc    # 文件 Appender、多 Appender、级别过滤
│   ├── test_log_config.cc  # YAML 配置加载、ConfigVar、Listener
│   └── test_log_thread.cc  # 多线程并发写日志
└── deploy/                 # make deploy 产物（不纳入版本管理）
    ├── include/sherrylog/
    └── lib/
```

## 依赖

| 依赖 | 版本 | 说明 |
|------|------|------|
| GCC / Clang | 支持 C++17 | |
| CMake | ≥ 3.10 | |
| yaml-cpp | ≥ 0.5 | YAML 配置解析 |
| Boost | ≥ 1.65 | `lexical_cast` 等 |
| pthread | — | 系统标准线程库 |

Ubuntu / Debian 一键安装：

```bash
apt install -y build-essential cmake libyaml-cpp-dev libboost-all-dev
```

## 构建

### 快速开始

```bash
# 仅编译共享库
make

# 编译共享库 + 测例
make BUILD_TESTS=ON

# 运行测例
make BUILD_TESTS=ON test

# 编译并安装到 ./deploy/
make deploy

# Release 模式
make BUILD_TYPE=Release

# 开启 cmake verbose 输出
make verbose
```

### 环境变量

| 变量 | 默认值 | 说明 |
|------|--------|------|
| `BUILD_TESTS` | `OFF` | 是否编译测例，`ON` 开启 |
| `BUILD_TYPE` | `Debug` | CMake 构建类型：`Debug` / `Release` / `RelWithDebInfo` |
| `VERBOSE` | `OFF` | 是否输出完整编译命令 |

环境变量与命令行参数等价：

```bash
BUILD_TESTS=ON BUILD_TYPE=Release make deploy
# 等同于
make BUILD_TESTS=ON BUILD_TYPE=Release deploy
```

### 清理

```bash
make clean       # 清理编译产物（保留 CMake 缓存）
make distclean   # 删除 _build/ 和 deploy/
```

### deploy 产物

`make deploy` 后，`./deploy/` 目录结构如下：

```
deploy/
├── include/sherrylog/
│   ├── log.h
│   ├── config.h
│   ├── thread.h
│   ├── util.h
│   ├── singleton.h
│   └── noncopyable.h
└── lib/
    ├── libsherrylog.so       -> libsherrylog.so.1.0.0
    ├── libsherrylog.so.1     -> libsherrylog.so.1.0.0
    └── libsherrylog.so.1.0.0
```

## 接入其他项目

将 `deploy/` 复制到目标项目，或在 CMakeLists.txt 中指向 sherrylog 的 deploy 路径：

```cmake
set(SHERRYLOG_DIR /path/to/sherrylog/deploy)

include_directories(${SHERRYLOG_DIR}/include)
link_directories(${SHERRYLOG_DIR}/lib)

target_link_libraries(your_target sherrylog pthread yaml-cpp)
```

运行时需确保动态库可被找到：

```bash
export LD_LIBRARY_PATH=/path/to/sherrylog/deploy/lib:$LD_LIBRARY_PATH
# 或系统级
ldconfig /path/to/sherrylog/deploy/lib
```

## API 使用示例

### 基础用法

```cpp
#include <sherrylog/log.h>

// 使用全局 root logger
auto logger = SYLAR_LOG_ROOT();

SYLAR_LOG_DEBUG(logger) << "debug message";
SYLAR_LOG_INFO(logger)  << "info  message";
SYLAR_LOG_WARN(logger)  << "warn  message";
SYLAR_LOG_ERROR(logger) << "error: code=" << 404;

// 格式化输出
SYLAR_LOG_FMT_INFO(logger, "name=%s age=%d", "sherry", 18);
```

### 具名 Logger

```cpp
// 按模块分 logger，互相独立
auto net_log = SYLAR_LOG_NAME("network");
auto db_log  = SYLAR_LOG_NAME("database");

net_log->setLevel(sherry::LogLevel::WARN);  // 只输出 WARN 及以上

SYLAR_LOG_INFO(net_log)  << "连接建立";   // 被过滤
SYLAR_LOG_ERROR(net_log) << "连接断开";   // 输出
```

### 文件 Appender

```cpp
auto logger  = SYLAR_LOG_NAME("app");
auto fileApp = std::make_shared<sherry::FileLogAppender>("/var/log/app.log");
fileApp->setLevel(sherry::LogLevel::INFO);
logger->addAppender(fileApp);

// 同时保留控制台输出
auto stdApp = std::make_shared<sherry::StdoutLogAppender>();
stdApp->setLevel(sherry::LogLevel::ERROR);
logger->addAppender(stdApp);
```

### YAML 配置

`config.yaml`：

```yaml
logs:
  - name: app
    level: info
    appenders:
      - type: FileLogAppender
        file: /var/log/app.log
      - type: StdoutLogAppender
        level: warn
```

加载配置：

```cpp
#include <sherrylog/log.h>
#include <sherrylog/config.h>
#include <yaml-cpp/yaml.h>

YAML::Node root = YAML::LoadFile("config.yaml");
sherry::Config::LoadFromYaml(root);

// 配置生效后，"app" logger 已按 yaml 初始化
auto logger = SYLAR_LOG_NAME("app");
SYLAR_LOG_INFO(logger) << "loaded from yaml config";
```

### ConfigVar 监听器

```cpp
auto cfg = sherry::Config::Lookup("server.port", 8080, "server port");

cfg->addListener([](const int& old_val, const int& new_val) {
    std::cout << "port changed: " << old_val << " -> " << new_val << std::endl;
});

// 加载新配置后自动触发回调
sherry::Config::LoadFromYaml(YAML::Load("server:\n  port: 9090\n"));
```

## 日志格式

默认 pattern：`%d%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n`

| 占位符 | 含义 |
|--------|------|
| `%d` | 时间（默认 `%Y-%m-%d %H:%M:%S`，可 `%d{%H:%M:%S}` 自定义） |
| `%t` | 线程 ID |
| `%N` | 线程名 |
| `%F` | 协程 ID |
| `%p` | 日志级别 |
| `%c` | Logger 名称 |
| `%f` | 源文件名 |
| `%l` | 行号 |
| `%m` | 日志内容 |
| `%n` | 换行 |
| `%T` | Tab |
| `%r` | 程序启动经过毫秒数 |

自定义格式示例：

```cpp
logger->setFormatter("[%d{%H:%M:%S}][%p] %m%n");
```

## 测例说明

| 测例 | 覆盖点 |
|------|--------|
| `test_log_basic` | 级别宏输出、级别过滤、`fmt` 格式化、named logger、`ToString/FromString` |
| `test_log_file` | 文件写入验证、Appender 级别过滤、多 Appender 并存 |
| `test_log_config` | YAML 加载 Logger、`ConfigVar` 读写、Listener 回调触发次数 |
| `test_log_thread` | 8 线程 × 50 条共 400 行并发写入无丢失、线程名写入日志 |
