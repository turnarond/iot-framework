cmake_minimum_required(VERSION 3.20)
project(aco-registry LANGUAGES CXX)

# --- C++ 标准 ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- 输出目录 ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- Conan 集成（Conan 2.x）---
# 自动包含由 `conan install` 生成的 CMake 工具链和依赖
if(EXISTS "${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake")
    include(${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake)
else()
    message(WARNING "Conan toolchain not found. Run: conan install . --output-folder=build --build=missing")
endif()

# 查找由 Conan 提供的包
find_package(oatpp REQUIRED)
find_package(oatpp-sqlite REQUIRED)
find_package(oatpp-swagger REQUIRED)
find_package(SQLite3 REQUIRED)
# find_package(nlohmann_json CONFIG REQUIRED)  # 可选，用于日志等

# --- 源文件 ---
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/service/*.cpp
    src/service/*.hpp
    src/App.cpp
    src/ErrorHandler.cpp
)

# --- 可执行文件 ---
add_executable(aco-registry ${SOURCES})

# --- 包含目录 ---
target_include_directories(aco-registry PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# --- 编译选项（可选：启用 Oat++ 推荐设置）---
target_compile_definitions(aco-registry PRIVATE
    OATPP_DISABLE_ENV_OBJECT_COUNTERS  # 减少内存开销（生产推荐）
    # OATPP_DISABLE_LOGV                # 禁用 verbose 日志
)

# --- 链接依赖 ---
target_link_libraries(aco-registry PRIVATE
    oatpp::oatpp
    oatpp::oatpp-sqlite
    oatpp::oatpp-swagger
    SQLite::SQLite3
    crypto
)

# --- 复制 schema.sql 到构建目录（方便运行）---
# configure_file(schema.sql ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/schema.sql COPYONLY)

# # --- 启用测试（未来扩展）---
# enable_testing()

# --- 可选：安装目标 ---
install(TARGETS aco-registry
    RUNTIME DESTINATION bin
)
install(FILES schema.sql DESTINATION share/aco-registry)