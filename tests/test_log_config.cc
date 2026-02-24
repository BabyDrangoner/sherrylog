/**
 * test_log_config.cc
 * 测试通过 YAML 配置动态初始化 Logger（名称、级别、Appender）
 */
#include "log.h"
#include "config.h"
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstdio>

static const std::string TMP_LOG = "/tmp/sherrylog_config_test.log";

static std::string readFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) return "";
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

void test_yaml_config_logger() {
    std::cout << "=== [test_yaml_config_logger] ===" << std::endl;

    std::remove(TMP_LOG.c_str());

    // 构造 YAML 配置：定义一个名为 "app" 的 logger，写入文件
    std::string yaml_str = R"(
logs:
  - name: app
    level: info
    appenders:
      - type: FileLogAppender
        file: )" + TMP_LOG + R"(
      - type: StdoutLogAppender
        level: warn
)";

    YAML::Node root = YAML::Load(yaml_str);
    sherry::Config::LoadFromYaml(root);

    auto logger = SYLAR_LOG_NAME("app");
    // 级别来自配置 info，DEBUG 不应出现
    SYLAR_LOG_DEBUG(logger)  << "cfg-debug: should NOT appear";
    SYLAR_LOG_INFO(logger)   << "cfg-info:  should appear in file";
    SYLAR_LOG_WARN(logger)   << "cfg-warn:  should appear in file and stdout";
    SYLAR_LOG_ERROR(logger)  << "cfg-error: should appear";

    // 等文件写完（clearAppenders 会触发析构）
    logger->clearAppenders();

    std::string content = readFile(TMP_LOG);
    std::cout << "--- file content ---\n" << content << "--- end ---\n";

    assert(content.find("cfg-debug: should NOT appear") == std::string::npos);
    assert(content.find("cfg-info:  should appear in file") != std::string::npos);
    assert(content.find("cfg-error: should appear") != std::string::npos);

    std::cout << "[PASS] test_yaml_config_logger" << std::endl;
}

void test_config_lookup() {
    std::cout << "\n=== [test_config_lookup] ===" << std::endl;

    // 注册一个整数配置项
    auto cfg_port = sherry::Config::Lookup("server.port", 8080, "server port");
    assert(cfg_port->getValue() == 8080);

    // 通过 YAML 修改
    std::string yaml_str = "server:\n  port: 9090\n";
    YAML::Node root = YAML::Load(yaml_str);
    sherry::Config::LoadFromYaml(root);

    assert(cfg_port->getValue() == 9090);

    // 通过 Lookup 再次获取，应同一实例且值已更新
    auto cfg_port2 = sherry::Config::Lookup<int>("server.port");
    assert(cfg_port2 != nullptr);
    assert(cfg_port2->getValue() == 9090);

    std::cout << "server.port = " << cfg_port->getValue() << std::endl;
    std::cout << "[PASS] test_config_lookup" << std::endl;
}

void test_config_listener() {
    std::cout << "\n=== [test_config_listener] ===" << std::endl;

    int change_count = 0;
    std::string old_val_captured, new_val_captured;

    auto cfg_name = sherry::Config::Lookup("app.name", std::string("default"), "app name");
    cfg_name->addListener([&](const std::string& ov, const std::string& nv){
        ++change_count;
        old_val_captured = ov;
        new_val_captured = nv;
    });

    assert(cfg_name->getValue() == "default");

    std::string yaml1 = "app:\n  name: sherrylog\n";
    sherry::Config::LoadFromYaml(YAML::Load(yaml1));
    assert(change_count == 1);
    assert(old_val_captured == "default");
    assert(new_val_captured == "sherrylog");

    // 相同值不触发回调
    sherry::Config::LoadFromYaml(YAML::Load(yaml1));
    assert(change_count == 1);

    std::string yaml2 = "app:\n  name: v2\n";
    sherry::Config::LoadFromYaml(YAML::Load(yaml2));
    assert(change_count == 2);
    assert(new_val_captured == "v2");

    std::cout << "listener fired " << change_count << " times" << std::endl;
    std::cout << "[PASS] test_config_listener" << std::endl;
}

int main() {
    std::cout << "================================" << std::endl;
    std::cout << "  sherrylog :: test_log_config  " << std::endl;
    std::cout << "================================" << std::endl;

    test_yaml_config_logger();
    test_config_lookup();
    test_config_listener();

    std::cout << "\n[ALL PASS] test_log_config" << std::endl;
    return 0;
}
