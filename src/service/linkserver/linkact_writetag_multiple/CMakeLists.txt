cmake_minimum_required(VERSION 3.15)

project(linkact_writetag_multiple LANGUAGES CXX)

include(../../../CMakeCommonLib)

set (CMAKE_CXX_STANDARD 17)

include_directories(
    ${LW_INCLUDE_DIR}
    ../
)

link_directories(
    ${LW_LIB_DIR}
)

add_library(${PROJECT_NAME} SHARED
    writetag_multiple.cpp
)

target_link_libraries(${PROJECT_NAME}
    lwlog
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LW_LIB_DIR}
    LIBRARY DESTINATION ${LW_LIB_DIR}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)

