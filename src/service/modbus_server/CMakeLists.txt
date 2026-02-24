cmake_minimum_required(VERSION 3.10)

project(modbus_server VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include CMakeCommonLib
include(${CMAKE_CURRENT_SOURCE_DIR}/../../CMakeCommonLib)

# Include directories
include_directories(src)
include_directories(${LW_INCLUDE_DIR})

# Source files
set(SOURCE_FILES
    src/modbus_protocol.cpp
    src/modbus_slave.cpp
    src/modbus_server.cpp
    src/main.cpp
)

# Find required packages
find_package(Threads REQUIRED)

# Add executable
add_executable(modbus_server ${SOURCE_FILES})

# Link libraries
target_link_libraries(modbus_server PRIVATE Threads::Threads)
target_link_libraries(modbus_server 
    PRIVATE 
    ${LW_LIB_DIR}/liblwlog.so
    ${LW_LIB_DIR}/liblwcomm.so
)

# Copy config file to build directory
configure_file(config/modbus_server.json modbus_server.json COPYONLY)

# Install target
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LW_LIB_DIR}
    LIBRARY DESTINATION ${LW_LIB_DIR}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)

install (FILES  "${CMAKE_CURRENT_LIST_DIR}/config/modbus_server.json"
    DESTINATION ${LW_CONFIG_DIR}
)
