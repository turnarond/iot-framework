cmake_minimum_required(VERSION 3.12)

project(lwdrvcmn LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

include(../../CMakeCommonLib)

include_directories(
    ${LW_INCLUDE_DIR}
    ${LW_INCLUDE_DIR}/libevent
    ../../../comms/lwcommunicate
)

link_directories(
    ${LW_LIB_DIR}
    ../../../comms/lwcommunicate/build
)

add_library(${PROJECT_NAME} SHARED
    device.cpp
    dll_main.cpp
    driver.cpp
    maintask.cpp
    drvframework.cpp
    lwdrivercommon.cpp
    user_timer.cpp
    resource_manager.cpp
    config_loader.cpp
)

target_link_libraries(${PROJECT_NAME}
    lwlog
    lwcomm
    lwcommunicate
    tinyxml2
    vsoa_dto
    platform_sdk
    lwipcssn
    sqlite3
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LW_LIB_DIR}
    LIBRARY DESTINATION ${LW_LIB_DIR}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)

install (FILES  "${CMAKE_CURRENT_LIST_DIR}/lwdrvcmn.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}
)

## Mark cpp header files for installation
install(FILES  "${CMAKE_CURRENT_LIST_DIR}/comm_helper.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}
)
