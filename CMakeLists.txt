cmake_minimum_required(VERSION 3.12)
project(uao)

set(CMAKE_CXX_STANDARD 11)

#版本号
set(NODE_OPENCV_VERSION 1)

#设置输出目录
set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/out)

#库文件
file(GLOB_RECURSE OPENCV3.4.1_LIB "${PROJECT_SOURCE_DIR}/deps/opencv3.4.1/lib/*.a")

#配置包含路劲
include_directories(./deps ./deps/opencv3.4.1/include)

#设置配置文件
configure_file(
        "${PROJECT_SOURCE_DIR}/config.h.in"
        "${PROJECT_SOURCE_DIR}/out/config.h"
)

#配置静态库目录
link_directories(./deps/opencv3.4.1/lib)

add_executable(uao src/main.cpp src/logger.h)
#添加静态库
target_link_libraries(uao ${OPENCV3.4.1_LIB})
