cmake_minimum_required(VERSION 3.17)

project(servermgr VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include CMakeCommonLib
include(${CMAKE_CURRENT_SOURCE_DIR}/../../CMakeCommonLib)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LW_INCLUDE_DIR}
)

# Source files
set(SOURCE_FILES
    src/config_parser.cpp
    src/servermgr.cpp
    src/App.cpp
    src/service/ServiceManagerService.cpp
)

# Find required packages
find_package(Threads REQUIRED)

# Add executable
add_executable(servermgr ${SOURCE_FILES})

target_link_directories(
    servermgr
    PRIVATE
    ${LW_LIB_DIR}
)

# Link libraries
 target_link_libraries(servermgr PRIVATE Threads::Threads)
 target_link_libraries(servermgr
    PRIVATE
    lwcomm
    lwlog
    vsoa_dto
    oatpp
)

# Install target
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LW_LIB_DIR}
    LIBRARY DESTINATION ${LW_LIB_DIR}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)

# Add servermgrctl executable
set(CTL_SOURCE_FILES
    ctl/src/main.cpp
    ctl/src/servermgrctl.cpp
)

add_executable(servermgrctl ${CTL_SOURCE_FILES})

target_include_directories(servermgrctl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/ctl/src
)

target_link_libraries(servermgrctl PRIVATE Threads::Threads)

install(TARGETS servermgrctl
    ARCHIVE DESTINATION ${LW_LIB_DIR}
    LIBRARY DESTINATION ${LW_LIB_DIR}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)
