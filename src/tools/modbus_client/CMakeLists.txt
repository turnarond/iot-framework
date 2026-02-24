cmake_minimum_required(VERSION 3.10)

project(modbus_client VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project root directory
set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Include directories
include_directories(
    ${PROJECT_ROOT_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Source files
set(SOURCES
    src/main.cpp
)

# Executable
add_executable(modbus_client ${SOURCES})

# Link libraries
target_link_libraries(modbus_client PRIVATE
    pthread
)

# Install target
install(TARGETS modbus_client
    RUNTIME DESTINATION ${PROJECT_ROOT_DIR}/bin
)

message(STATUS "Modbus client build configuration completed")
