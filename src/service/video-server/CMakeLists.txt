# CMakeLists.txt for video-server

# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(video-server VERSION 1.0.0 LANGUAGES CXX)

# 包含公共CMake配置
include(${CMAKE_CURRENT_SOURCE_DIR}/../../CMakeCommonLib)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LW_INCLUDE_DIR}
)

# 源文件
set(SOURCE_FILES
    src/LogService.cpp
    src/ConfigService.cpp
    src/PluginManager.cpp
    src/StreamManager.cpp
    src/DeviceManager.cpp
    src/ZLMediaKitAdapter.cpp
    src/DatabaseManager.cpp
    src/VideoServer.cpp
    src/main.cpp
)

# 插件源文件
set(PLUGIN_FILES
    plugins/RTSPPlugin.cpp
    plugins/HikvisionPlugin.cpp
)

# 可执行文件
add_executable(video-server ${SOURCE_FILES})

# 链接库
target_link_libraries(video-server
    ${LW_LIB_DIR}/liblwlog.so
    ${LW_LIB_DIR}/liblwcomm.so
    sqlite3
    pthread
    dl
)

# 插件构建
add_subdirectory(plugins)

# 安装配置
install(TARGETS video-server
    DESTINATION ${LW_BIN_DIR}
)

# 配置文件安装
install(FILES config/config.json
    DESTINATION ${LW_CONFIG_DIR}
)

# 创建必要的目录
install(DIRECTORY DESTINATION ${LW_BIN_DIR}/plugins)
install(DIRECTORY DESTINATION ${LW_BIN_DIR}/logs)

# 显示构建信息
message(STATUS "Video server build configuration:")
message(STATUS "  Source files: ${SOURCE_FILES}")
message(STATUS "  Plugin files: ${PLUGIN_FILES}")
message(STATUS "  Include directories: ${CMAKE_INCLUDE_PATH}")
message(STATUS "  Library directories: ${CMAKE_LIBRARY_PATH}")
