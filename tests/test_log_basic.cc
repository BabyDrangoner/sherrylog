/**
 * test_log_basic.cc
 * 测试基础日志功能：日志级别、宏输出、格式化输出
 */
#include "log.h"
#include <iostream>
#include <cassert>

// 全局 root logger
static auto g_logger = SYLAR_LOG_ROOT();

void test_log_levels() {
    std::cout << "=== [test_log_levels] ===" << std::endl;

    // root logger 默认级别 DEBUG，所有级别都应输出
    SYLAR_LOG_DEBUG(g_logger)  << "this is a DEBUG message";
    SYLAR_LOG_INFO(g_logger)   << "this is an INFO message";
    SYLAR_LOG_WARN(g_logger)   << "this is a WARN message";
    SYLAR_LOG_ERROR(g_logger)  << "this is an ERROR message";
    SYLAR_LOG_FATAL(g_logger)  << "this is a FATAL message";

    std::cout << "[PASS] test_log_levels" << std::endl;
}

void test_log_level_filter() {
    std::cout << "\n=== [test_log_level_filter] ===" << std::endl;

    auto logger = SYLAR_LOG_NAME("filter_test");
    // 设置为 WARN，DEBUG/INFO 不应输出
    logger->setLevel(sherry::LogLevel::WARN);

    SYLAR_LOG_DEBUG(logger) << "DEBUG: should NOT appear";
    SYLAR_LOG_INFO(logger)  << "INFO:  should NOT appear";
    SYLAR_LOG_WARN(logger)  << "WARN:  should appear";
    SYLAR_LOG_ERROR(logger) << "ERROR: should appear";

    std::cout << "[PASS] test_log_level_filter" << std::endl;
}

void test_log_fmt() {
    std::cout << "\n=== [test_log_fmt] ===" << std::endl;

    SYLAR_LOG_FMT_INFO(g_logger, "formatted: name=%s age=%d pi=%.2f", "sherry", 18, 3.14);
    SYLAR_LOG_FMT_ERROR(g_logger, "error code: %d msg: %s", 404, "not found");

    std::cout << "[PASS] test_log_fmt" << std::endl;
}

void test_named_logger() {
    std::cout << "\n=== [test_named_logger] ===" << std::endl;

    auto logger1 = SYLAR_LOG_NAME("module.A");
    auto logger2 = SYLAR_LOG_NAME("module.B");

    // 同名获取同一实例
    auto logger1_dup = SYLAR_LOG_NAME("module.A");
    assert(logger1.get() == logger1_dup.get());

    SYLAR_LOG_INFO(logger1) << "hello from module.A";
    SYLAR_LOG_INFO(logger2) << "hello from module.B";

    std::cout << "[PASS] test_named_logger" << std::endl;
}

void test_level_tostring() {
    std::cout << "\n=== [test_level_tostring] ===" << std::endl;

    assert(std::string("DEBUG") == sherry::LogLevel::ToString(sherry::LogLevel::DEBUG));
    assert(std::string("INFO")  == sherry::LogLevel::ToString(sherry::LogLevel::INFO));
    assert(std::string("WARN")  == sherry::LogLevel::ToString(sherry::LogLevel::WARN));
    assert(std::string("ERROR") == sherry::LogLevel::ToString(sherry::LogLevel::ERROR));
    assert(std::string("FATAL") == sherry::LogLevel::ToString(sherry::LogLevel::FATAL));

    assert(sherry::LogLevel::FromString("debug") == sherry::LogLevel::DEBUG);
    assert(sherry::LogLevel::FromString("INFO")  == sherry::LogLevel::INFO);
    assert(sherry::LogLevel::FromString("WARN")  == sherry::LogLevel::WARN);

    std::cout << "[PASS] test_level_tostring" << std::endl;
}

int main() {
    std::cout << "==============================" << std::endl;
    std::cout << "  sherrylog :: test_log_basic " << std::endl;
    std::cout << "==============================" << std::endl;

    test_log_levels();
    test_log_level_filter();
    test_log_fmt();
    test_named_logger();
    test_level_tostring();

    std::cout << "\n[ALL PASS] test_log_basic" << std::endl;
    return 0;
}
