cmake_minimum_required(VERSION 3.17)
project(node_server LANGUAGES CXX)

include(../../CMakeCommonLib)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${LW_INCLUDE_DIR}
    ${LW_INCLUDE_DIR}/libevent
    ${PROJECT_ROOT}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/dto
    ${CMAKE_CURRENT_SOURCE_DIR}/dds
    ${PROJECT_ROOT}/src/comms/lwdistcomm/include
    ${PROJECT_ROOT}/src/common/dto
)

link_directories(
    ${LW_LIB_DIR}
)

add_executable(${PROJECT_NAME}
    main.cpp
    data_center.cpp
    rtdb.cpp
    driver_collector.cpp
    node_server.cpp
    service/HmiPointService.cpp
    hmi_server.cpp
    websocket_server.cpp
    ErrorHandler.cpp
    dds/dds_manager.cpp
)

target_link_options(${PROJECT_NAME} PRIVATE "LINKER:-rpath-link,${LW_LIB_DIR}")

if (CMAKE_SYSTEM_NAME STREQUAL "sylixos")
    target_link_libraries(${PROJECT_NAME}
        dsohandle
    )
endif()
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        platform_sdk
        vsoa_dto
        oatpp
        oatpp-swagger
        lwipcssn
        lwcomm
        lwlog
        lwmsgq
        lwdistcomm
)

# boost system for beast/asio
find_package(Boost COMPONENTS system REQUIRED)
if(Boost_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
endif()

# Try to link jemalloc if available to improve allocator performance in high-concurrency workloads
find_library(JEMALLOC_LIB jemalloc)
if (JEMALLOC_LIB)
    message(STATUS "Found jemalloc: ${JEMALLOC_LIB}, linking against it for better allocation performance")
    target_link_libraries(${PROJECT_NAME} PRIVATE ${JEMALLOC_LIB})
endif()

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LW_LIB_DIR}
    LIBRARY DESTINATION ${LW_LIB_DIR}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)

# Add DDS test client
add_executable(test_dds_client
    test_dds_client.cpp
)

target_link_options(test_dds_client PRIVATE "LINKER:-rpath-link,${LW_LIB_DIR}")

target_link_libraries(test_dds_client
    PRIVATE
        lwlog
        lwdistcomm
)

install(TARGETS test_dds_client
    RUNTIME DESTINATION ${LW_BIN_DIR}
)
