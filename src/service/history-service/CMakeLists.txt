#
# Copyright (c) 2025 ACOAUTO Team.
# All rights reserved.
#
# Detailed license information can be found in the LICENSE file.
#

cmake_minimum_required(VERSION 3.10)

project(history-service CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../comms/lwdistcomm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../platform_sdk/include
)

# Source files
set(SOURCE_FILES
    history_service.cpp
    vsoa_handler.cpp
    dds_handler.cpp
    storage/file_storage.cpp
)

# Header files
set(HEADER_FILES
    history_storage.h
    history_service.h
    vsoa_handler.h
    dds_handler.h
)

# Add library
add_library(history-service SHARED
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

# Link libraries
target_link_libraries(history-service
    lwdistcomm
    platform-sdk
    pthread
    stdc++fs
    nlohmann_json
)

# Set output directory
set_target_properties(history-service PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build
)

# Install rules
install(TARGETS history-service
    LIBRARY DESTINATION lib
)

install(FILES ${HEADER_FILES}
    DESTINATION include/history-service
)
