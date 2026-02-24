#include "util.h"
#include <execinfo.h>
#include <sys/time.h>
#include <iostream>

namespace sherry{

pid_t GetThreadId(){
    return syscall(SYS_gettid);
}

// 独立 log 模块不依赖 fiber，始终返回 0
uint32_t GetFiberId(){
    return 0;
}

void Backtrace(std::vector<std::string> & bt, int size, int skip){
    void ** array = (void **)malloc((sizeof(void *) * size));
    size_t s = ::backtrace(array, size);

    char ** strings = backtrace_symbols(array, s);
    if(strings == NULL){
        std::cerr << "backtrace_symbols error" << std::endl;
        free(array);
        return;
    }

    for(size_t i = skip;i < s;++i){
        bt.push_back(strings[i]);
    }
    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string & prefix){
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for(size_t i = 0;i < bt.size();++i){
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}

uint64_t GetCurrentMS(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

uint64_t GetCurrentUS(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}

int8_t TypeUtil::ToChar(const std::string& str) {
    if(str.empty()) {
        return 0;
    }
    return *str.begin();
}

int64_t TypeUtil::Atoi(const std::string& str) {
    if(str.empty()) {
        return 0;
    }
    return strtoull(str.c_str(), nullptr, 10);
}

double TypeUtil::Atof(const std::string& str) {
    if(str.empty()) {
        return 0;
    }
    return atof(str.c_str());
}

int8_t TypeUtil::ToChar(const char* str) {
    if(str == nullptr) {
        return 0;
    }
    return str[0];
}

int64_t TypeUtil::Atoi(const char* str) {
    if(str == nullptr) {
        return 0;
    }
    return strtoull(str, nullptr, 10);
}

double TypeUtil::Atof(const char* str) {
    if(str == nullptr) {
        return 0;
    }
    return atof(str);
}

}
