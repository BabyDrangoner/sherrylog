cmake_minimum_required(VERSION 3.10)
project(sherrylog VERSION 1.0.0)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -O0 -ggdb -Wall -Wno-deprecated -Wno-unused-function -Wno-builtin-macro-redefined")

# 位置无关代码（编译为 .so 必须）
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ===========================
# 选项：是否编译测例
# 优先读取 CMake 选项 BUILD_TESTS，若未传则尝试读取同名环境变量
# ===========================
if(NOT DEFINED BUILD_TESTS)
    if(DEFINED ENV{BUILD_TESTS})
        set(BUILD_TESTS $ENV{BUILD_TESTS} CACHE BOOL "Build test programs" FORCE)
    else()
        set(BUILD_TESTS OFF CACHE BOOL "Build test programs")
    endif()
endif()
message(STATUS "BUILD_TESTS = ${BUILD_TESTS}")

# ===========================
# 头文件路径
# ===========================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ===========================
# 查找依赖
# ===========================
find_package(Boost REQUIRED)
find_library(YAMLCPP_LIB NAMES yaml-cpp REQUIRED)
find_path(YAMLCPP_INCLUDE NAMES yaml-cpp/yaml.h REQUIRED)

if(NOT YAMLCPP_LIB)
    message(FATAL_ERROR "yaml-cpp library not found!")
endif()
if(NOT YAMLCPP_INCLUDE)
    message(FATAL_ERROR "yaml-cpp headers not found!")
endif()

message(STATUS "yaml-cpp lib:     ${YAMLCPP_LIB}")
message(STATUS "yaml-cpp include: ${YAMLCPP_INCLUDE}")
message(STATUS "Boost include:    ${Boost_INCLUDE_DIRS}")

include_directories(
    ${YAMLCPP_INCLUDE}
    ${Boost_INCLUDE_DIRS}
)

# ===========================
# 源文件
# ===========================
set(SHERRYLOG_SRCS
    src/log.cc
    src/config.cc
    src/thread.cc
    src/util.cc
)

# ===========================
# 编译为共享库
# ===========================
add_library(sherrylog SHARED ${SHERRYLOG_SRCS})

target_link_libraries(sherrylog
    ${YAMLCPP_LIB}
    pthread
)

set_target_properties(sherrylog PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "sherrylog"
)

# ===========================
# 安装规则
# ===========================
include(GNUInstallDirs)

install(TARGETS sherrylog
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# 安装公共头文件
install(FILES
    src/log.h
    src/config.h
    src/thread.h
    src/util.h
    src/singleton.h
    src/noncopyable.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sherrylog
)

# ===========================
# 测例
# ===========================
if(BUILD_TESTS)
    message(STATUS "Building tests ...")
    set(TEST_SRCS
        tests/test_log_basic.cc
        tests/test_log_file.cc
        tests/test_log_config.cc
        tests/test_log_thread.cc
    )
    foreach(TEST_SRC ${TEST_SRCS})
        get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
        add_executable(${TEST_NAME} ${TEST_SRC})
        target_include_directories(${TEST_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${YAMLCPP_INCLUDE}
            ${Boost_INCLUDE_DIRS}
        )
        target_link_libraries(${TEST_NAME} sherrylog ${YAMLCPP_LIB} pthread)
        # 注册到 ctest
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()
    enable_testing()
endif()

# ===========================
# 本地安装目标（install到项目内部 deploy/）
# ===========================
set(LOCAL_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deploy)

add_custom_target(local_install
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LOCAL_INSTALL_DIR}/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LOCAL_INSTALL_DIR}/include/sherrylog
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:sherrylog>
        ${LOCAL_INSTALL_DIR}/lib/
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/src/log.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/thread.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/util.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/singleton.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/noncopyable.h
        ${LOCAL_INSTALL_DIR}/include/sherrylog/
    # 创建 soname / link 符号链接
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        libsherrylog.so.1.0.0
        ${LOCAL_INSTALL_DIR}/lib/libsherrylog.so.1
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        libsherrylog.so.1.0.0
        ${LOCAL_INSTALL_DIR}/lib/libsherrylog.so
    DEPENDS sherrylog
    COMMENT "Installing sherrylog to ${LOCAL_INSTALL_DIR}"
)

message(STATUS "")
message(STATUS "=== sherrylog build configuration ===")
message(STATUS "  Build type    : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install dir   : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Deploy dir    : ${LOCAL_INSTALL_DIR}")
message(STATUS "  Build tests   : ${BUILD_TESTS}")
message(STATUS "======================================")
