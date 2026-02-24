cmake_minimum_required(VERSION 3.15)
project(linkserver LANGUAGES CXX)

include(../../../CMakeCommonLib)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${LW_INCLUDE_DIR}
    ../
)

link_directories(
    ${LW_LIB_DIR}
)

add_executable(${PROJECT_NAME}
    main.cpp
    rule_engine.cpp
    linkevt_manager.cpp
    linkact_manager.cpp
)
if (CMAKE_SYSTEM_NAME STREQUAL "sylixos")
    target_link_libraries(${PROJECT_NAME}
        dsohandle
)
endif()

target_link_libraries(${PROJECT_NAME}
    PocoFoundation
    PocoUtil
    PocoData
    PocoDataSQLite
    PocoJSON
    lwlog
    lwcomm
    platform_sdk
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LW_LIB_DIR}
    LIBRARY DESTINATION ${LW_LIB_DIR}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)
