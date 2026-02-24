cmake_minimum_required(VERSION 3.23)

project(dds-publisher)

include(../../CMakeCommonLib)

include_directories(
    ${LW_INCLUDE_DIR}
    ${LW_INCLUDE_DIR}/lwdistcomm
)

link_directories(
    ${LW_LIB_DIR}
)

set(SOURCES
    publisher.cpp
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    lwdistcomm
    pthread
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)
