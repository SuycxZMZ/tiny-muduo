# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -std=c++17 -g -I/usr/local/include

# 链接选项
LDFLAGS = -L/usr/local/lib -ltinymuduo -lpthread

# 源文件
SRCS = async-echoserver.cc async-httpserver.cc echoserver.cc test_asynclog.cc

# 目标文件夹
BIN_DIR = bin

# 目标文件
TARGETS = $(SRCS:.cc=)
TARGETS_BIN = $(addprefix $(BIN_DIR)/, $(TARGETS))

# 默认目标
all: $(TARGETS_BIN)

# 规则
$(BIN_DIR)/%: %.cc
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# 清理规则
clean:
	rm -f $(TARGETS_BIN)