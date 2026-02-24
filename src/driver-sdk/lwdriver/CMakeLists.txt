cmake_minimum_required(VERSION 3.12)
project(lwdriver LANGUAGES CXX)

include(../../CMakeCommonLib)

set(CMAKE_CXX_STANDARD 11)
add_executable(${PROJECT_NAME}
    lwdriver.cpp
)
if (CMAKE_SYSTEM_NAME STREQUAL "sylixos")
target_link_libraries(${PROJECT_NAME}
    dsohandle
)
endif()

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LW_LIB_DIR}
    LIBRARY DESTINATION ${LW_LIB_DIR}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)
