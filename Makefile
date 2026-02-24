# =============================================================
#  sherrylog — 顶层 Makefile
#
#  用法示例：
#    make                    # 仅编译 so（不编译测例）
#    make BUILD_TESTS=1      # 编译 so + 测例
#    BUILD_TESTS=1 make      # 同上（环境变量方式）
#    make test               # 运行测例（需先 BUILD_TESTS=1 make）
#    make deploy             # 编译并 deploy 到 ./deploy/
#    make clean              # 清理编译产物（保留 build/）
#    make distclean          # 删除 build/ 和 deploy/
#    make BUILD_TYPE=Release # Release 模式构建
#    make verbose            # 开启 cmake verbose
# =============================================================

# ---------- 可通过命令行 / 环境变量覆盖的参数 ----------
BUILD_TYPE  ?= Debug
BUILD_TESTS ?= OFF
VERBOSE     ?= OFF

# build 目录（固定，CMake 输出放这里；用 _build 避免与 make target 'build' 冲突）
BUILD_DIR   := _build
# deploy 目录
DEPLOY_DIR  := deploy

# CMake 可执行文件
CMAKE       := cmake
MAKE_FLAGS  :=

ifeq ($(VERBOSE),ON)
    MAKE_FLAGS += VERBOSE=1
endif

# ---------- 传给 CMake 的 -D 选项 ----------
CMAKE_ARGS :=
CMAKE_ARGS += -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
CMAKE_ARGS += -DBUILD_TESTS=$(BUILD_TESTS)

# ---------- 颜色输出（可选） ----------
BOLD  := \033[1m
RESET := \033[0m
GREEN := \033[1;32m
CYAN  := \033[1;36m

# ==============================================================

.PHONY: all build configure test deploy clean distclean help verbose

## 默认目标：编译 so
all: build

## 配置（幂等，build 目录已存在时也可重新配置）
configure: | $(BUILD_DIR)
	@printf "$(CYAN)>>> cmake configure  BUILD_TYPE=$(BUILD_TYPE)  BUILD_TESTS=$(BUILD_TESTS)$(RESET)\n"
	cd $(BUILD_DIR) && $(CMAKE) .. $(CMAKE_ARGS)

## 编译共享库（以及测例，若 BUILD_TESTS=1）
build: configure
	@printf "$(CYAN)>>> cmake build$(RESET)\n"
	$(CMAKE) --build $(BUILD_DIR) -- $(MAKE_FLAGS) -j$$(nproc)

## 运行测例（BUILD_TESTS=1 编译后才有可执行文件）
test: build
	@printf "$(CYAN)>>> ctest$(RESET)\n"
	cd $(BUILD_DIR) && ctest --output-on-failure

## 编译并安装到 ./deploy/
deploy: build
	@printf "$(CYAN)>>> local_install -> $(DEPLOY_DIR)/$(RESET)\n"
	$(CMAKE) --build $(BUILD_DIR) --target local_install

## 清理编译结果（保留 CMake 缓存，不删 build/）
clean:
	@printf "$(CYAN)>>> clean$(RESET)\n"
	$(CMAKE) --build $(BUILD_DIR) --target clean 2>/dev/null || true

## 彻底清理 build/ 和 deploy/
distclean:
	@printf "$(CYAN)>>> distclean: removing $(BUILD_DIR)/ $(DEPLOY_DIR)/$(RESET)\n"
	rm -rf $(BUILD_DIR) $(DEPLOY_DIR)

## 开启 verbose 模式编译 so（等同 make VERBOSE=ON）
verbose:
	$(MAKE) VERBOSE=ON build

## 打印帮助
help:
	@printf "$(BOLD)sherrylog Makefile targets$(RESET)\n"
	@printf "  $(GREEN)make$(RESET)                      编译 so (BUILD_TYPE=$(BUILD_TYPE))\n"
	@printf "  $(GREEN)make BUILD_TESTS=ON$(RESET)        编译 so + 测例\n"
	@printf "  $(GREEN)make test$(RESET)                  运行测例 (需先 BUILD_TESTS=ON make)\n"
	@printf "  $(GREEN)make deploy$(RESET)                编译并安装到 ./deploy/\n"
	@printf "  $(GREEN)make clean$(RESET)                 清理编译产物\n"
	@printf "  $(GREEN)make distclean$(RESET)             删除 build/ 和 deploy/\n"
	@printf "  $(GREEN)make BUILD_TYPE=Release$(RESET)    Release 模式构建\n"
	@printf "  $(GREEN)make verbose$(RESET)               开启 cmake verbose 输出\n"
	@printf "\n  可用环境变量:\n"
	@printf "    BUILD_TESTS  = OFF | ON    是否编译测例\n"
	@printf "    BUILD_TYPE   = Debug | Release | RelWithDebInfo\n"
	@printf "    VERBOSE      = OFF | ON    是否开启编译详细输出\n"

# ----------- 保证 build 目录存在 -----------
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
