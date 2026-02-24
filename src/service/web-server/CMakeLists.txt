cmake_minimum_required(VERSION 3.16)
project(web-server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include (../../CMakeCommonLib)

# Include directories
include_directories(
    .
    ${LW_INCLUDE_DIR}
)

# Source files
set(SOURCE_FILES
    App.cpp
    ErrorHandler.cpp
    service/DriverService.cpp
    service/DeviceService.cpp
    service/PointService.cpp
    service/AlarmService.cpp
    service/LinkageService.cpp
    service/DictService.cpp
    service/UserService.cpp
    service/VideoService.cpp
)

# Create executable
add_executable(web-server ${SOURCE_FILES})

target_link_directories(
    web-server
    PRIVATE
    ${LW_LIB_DIR}
)
# Link libraries
target_link_libraries(web-server
PRIVATE
    lwcomm
    lwlog
    vsoa_dto
    oatpp
    oatpp-sqlite
    oatpp-swagger
)

# Install target
install(TARGETS web-server
    DESTINATION ${LW_BIN_DIR}
)
