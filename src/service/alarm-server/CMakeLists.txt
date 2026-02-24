cmake_minimum_required(VERSION 3.15)
project(alarm_server LANGUAGES CXX)

include(../../CMakeCommonLib)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${LW_INCLUDE_DIR}
    .
)

link_directories(
    ${LW_LIB_DIR}
)

add_executable(
    ${PROJECT_NAME}
    main.cpp
    api/alarm_node_subscriber.cpp
    api/alarm_server_controller.cpp
    service/alarm_server.cpp
    service/alarm_rule_processor.cpp
    service/alarm_state_manager.cpp
    service/alarm_log.cpp
)

target_link_libraries(${PROJECT_NAME}
    platform_sdk
    vsoa_dto
    lwlog
    lwcomm
    lwdistcomm
    PocoFoundationd
    PocoDatad
    PocoDataSQLited
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LW_LIB_DIR}
    LIBRARY DESTINATION ${LW_LIB_DIR}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)
