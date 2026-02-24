cmake_minimum_required(VERSION 3.12)

project(edge-core LANGUAGES CXX C)

include(../../CMakeCommonLib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    .
    ${LW_SRC_DIR}/comms/
    ${LW_INCLUDE_DIR}
)

add_executable(${PROJECT_NAME}
    main.cpp
    base/version.cpp
    cfgm/cfgm_service.cpp
    cfgm/config_manager.cpp
    cloud/cloud_handler.cpp
    logm/log_manager.cpp
    regm/registry_manager.cpp
    resm/resource_monitor.cpp
    ../../comms/edge-util/edge_util.c
    security/security_context.cpp
)

target_link_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${LW_LIB_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    lwlog
    lwcomm
    mosquitto
    vsoa_dto
    platform_sdk
    PocoFoundation
    PocoJSON
    PocoData
    PocoDataSQLite
    ssl
    crypto
)

install(TARGETS ${PROJECT_NAME} 
    ARCHIVE DESTINATION ${LW_LIB_DIR}
    LIBRARY DESTINATION ${LW_LIB_DIR}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)