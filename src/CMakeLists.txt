# aux_source_directory(base/*.cc log/*.cc net/*.cc SRC_LIST)

file(GLOB SRC_LIST
    # 列出所有需要编译的源文件
    base/*.cc
    log/*.cc
    net/*.cc
)

add_library(tinymuduo SHARED ${SRC_LIST})