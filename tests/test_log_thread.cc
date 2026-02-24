/**
 * test_log_thread.cc
 * 测试多线程并发写日志的线程安全性
 */
#include "log.h"
#include "thread.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <atomic>
#include <cassert>
#include <cstdio>

static const std::string TMP_LOG = "/tmp/sherrylog_thread_test.log";

static std::string readFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs) return "";
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

static int countLines(const std::string& s) {
    int cnt = 0;
    for (char c : s) if (c == '\n') ++cnt;
    return cnt;
}

void test_concurrent_logging() {
    std::cout << "=== [test_concurrent_logging] ===" << std::endl;

    std::remove(TMP_LOG.c_str());

    auto logger = SYLAR_LOG_NAME("thread_test");
    logger->clearAppenders();
    logger->setLevel(sherry::LogLevel::DEBUG);

    auto fileApp = std::make_shared<sherry::FileLogAppender>(TMP_LOG);
    fileApp->setLevel(sherry::LogLevel::DEBUG);
    logger->addAppender(fileApp);

    const int THREAD_COUNT = 8;
    const int LOG_PER_THREAD = 50;

    std::atomic<int> started{0};
    std::vector<sherry::Thread::ptr> threads;

    for (int i = 0; i < THREAD_COUNT; ++i) {
        std::string tname = "worker-" + std::to_string(i);
        threads.push_back(std::make_shared<sherry::Thread>([&, i](){
            ++started;
            // 等所有线程就绪后同时开始
            while (started < THREAD_COUNT) {
                sched_yield();
            }
            for (int j = 0; j < LOG_PER_THREAD; ++j) {
                SYLAR_LOG_INFO(logger)
                    << "thread=" << i << " seq=" << j;
            }
        }, tname));
    }

    for (auto& t : threads) {
        t->join();
    }

    logger->clearAppenders();

    std::string content = readFile(TMP_LOG);
    int lines = countLines(content);
    int expected = THREAD_COUNT * LOG_PER_THREAD;

    std::cout << "expected lines: " << expected
              << "  actual lines: " << lines << std::endl;

    assert(lines == expected
        && "log line count mismatch, possible race condition!");

    std::cout << "[PASS] test_concurrent_logging" << std::endl;
}

void test_thread_name_in_log() {
    std::cout << "\n=== [test_thread_name_in_log] ===" << std::endl;

    std::remove(TMP_LOG.c_str());

    auto logger = SYLAR_LOG_NAME("tname_test");
    logger->clearAppenders();
    logger->setLevel(sherry::LogLevel::DEBUG);

    auto fileApp = std::make_shared<sherry::FileLogAppender>(TMP_LOG);
    logger->addAppender(fileApp);

    bool done = false;
    sherry::Thread t([&](){
        SYLAR_LOG_INFO(logger) << "from named thread";
        done = true;
    }, "my-worker");
    t.join();

    logger->clearAppenders();

    std::string content = readFile(TMP_LOG);
    assert(content.find("my-worker") != std::string::npos
        && "thread name should appear in log");
    assert(content.find("from named thread") != std::string::npos);

    std::cout << "log line: " << content;
    std::cout << "[PASS] test_thread_name_in_log" << std::endl;
}

int main() {
    std::cout << "=================================" << std::endl;
    std::cout << "  sherrylog :: test_log_thread   " << std::endl;
    std::cout << "=================================" << std::endl;

    test_concurrent_logging();
    test_thread_name_in_log();

    std::cout << "\n[ALL PASS] test_log_thread" << std::endl;
    return 0;
}
