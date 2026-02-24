/**
 * test_log_file.cc
 * 测试文件 Appender：写日志到文件，验证文件内容
 */
#include "log.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <iostream>
#include <cstdio>

static const std::string TEST_LOG_FILE = "/tmp/sherrylog_test.log";

// 读取文件全部内容
static std::string readFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) return "";
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

void test_file_appender() {
    std::cout << "=== [test_file_appender] ===" << std::endl;

    // 清理旧日志
    std::remove(TEST_LOG_FILE.c_str());

    auto logger = SYLAR_LOG_NAME("file_test");
    logger->clearAppenders();
    logger->setLevel(sherry::LogLevel::DEBUG);

    // 添加文件 Appender
    auto fileApp = std::make_shared<sherry::FileLogAppender>(TEST_LOG_FILE);
    fileApp->setLevel(sherry::LogLevel::DEBUG);
    logger->addAppender(fileApp);

    SYLAR_LOG_INFO(logger)  << "line1: info message";
    SYLAR_LOG_WARN(logger)  << "line2: warn message";
    SYLAR_LOG_ERROR(logger) << "line3: error message";

    // 让 appender flush（filestream 析构时关闭）
    logger->clearAppenders();

    std::string content = readFile(TEST_LOG_FILE);
    assert(!content.empty() && "log file should not be empty");
    assert(content.find("line1: info message")  != std::string::npos);
    assert(content.find("line2: warn message")  != std::string::npos);
    assert(content.find("line3: error message") != std::string::npos);

    std::cout << "--- log file content ---" << std::endl;
    std::cout << content;
    std::cout << "--- end ---" << std::endl;
    std::cout << "[PASS] test_file_appender" << std::endl;
}

void test_file_level_filter() {
    std::cout << "\n=== [test_file_level_filter] ===" << std::endl;

    std::remove(TEST_LOG_FILE.c_str());

    auto logger = SYLAR_LOG_NAME("file_filter");
    logger->clearAppenders();
    logger->setLevel(sherry::LogLevel::DEBUG);

    auto fileApp = std::make_shared<sherry::FileLogAppender>(TEST_LOG_FILE);
    // appender 只写 ERROR 及以上
    fileApp->setLevel(sherry::LogLevel::ERROR);
    logger->addAppender(fileApp);

    SYLAR_LOG_DEBUG(logger) << "debug: should NOT in file";
    SYLAR_LOG_INFO(logger)  << "info:  should NOT in file";
    SYLAR_LOG_ERROR(logger) << "error: should be in file";
    SYLAR_LOG_FATAL(logger) << "fatal: should be in file";

    logger->clearAppenders();

    std::string content = readFile(TEST_LOG_FILE);
    assert(content.find("should NOT in file") == std::string::npos);
    assert(content.find("error: should be in file") != std::string::npos);
    assert(content.find("fatal: should be in file") != std::string::npos);

    std::cout << "[PASS] test_file_level_filter" << std::endl;
}

void test_multi_appender() {
    std::cout << "\n=== [test_multi_appender] ===" << std::endl;

    std::remove(TEST_LOG_FILE.c_str());

    auto logger = SYLAR_LOG_NAME("multi_app");
    logger->clearAppenders();
    logger->setLevel(sherry::LogLevel::DEBUG);

    // 同时绑定 stdout + file
    auto stdoutApp = std::make_shared<sherry::StdoutLogAppender>();
    stdoutApp->setLevel(sherry::LogLevel::WARN);

    auto fileApp = std::make_shared<sherry::FileLogAppender>(TEST_LOG_FILE);
    fileApp->setLevel(sherry::LogLevel::DEBUG);

    logger->addAppender(stdoutApp);
    logger->addAppender(fileApp);

    SYLAR_LOG_INFO(logger)  << "multi: info  (file only)";
    SYLAR_LOG_ERROR(logger) << "multi: error (file + stdout)";

    logger->clearAppenders();

    std::string content = readFile(TEST_LOG_FILE);
    assert(content.find("multi: info  (file only)")   != std::string::npos);
    assert(content.find("multi: error (file + stdout)") != std::string::npos);

    std::cout << "[PASS] test_multi_appender" << std::endl;
}

int main() {
    std::cout << "==============================" << std::endl;
    std::cout << "  sherrylog :: test_log_file  " << std::endl;
    std::cout << "==============================" << std::endl;

    test_file_appender();
    test_file_level_filter();
    test_multi_appender();

    std::cout << "\n[ALL PASS] test_log_file" << std::endl;
    return 0;
}
