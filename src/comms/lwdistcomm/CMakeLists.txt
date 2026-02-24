cmake_minimum_required(VERSION 3.23)

project(lwdistcomm)

set(CMAKE_C_STANDARD 11)

include(../../CMakeCommonLib)

include_directories(
    ${LW_INCLUDE_DIR}
    ${LW_INCLUDE_DIR}/libevent
    ./include
)

link_directories(
    ${LW_LIB_DIR}
)

# Source files
set(SOURCES
    src/address/address.c
    src/message/message.c
    src/security/security.c
    src/transport/transport.c
    src/client/client.c
    src/server/server.c
    # DDS related files
    src/dds/domain_participant.c
    src/dds/topic.c
    src/dds/publisher.c
    src/dds/subscriber.c
    src/dds/data_writer.c
    src/dds/data_reader.c
    src/dds/qos.c
    src/dds/discovery.c
    src/dds/dds_transport.c
    src/dds/dds_simple.c
    src/dds/spdp.c
)

# Create shared library
add_library(${PROJECT_NAME} SHARED
    ${SOURCES}
)

# Link libraries
# TODO: Add OpenSSL or other crypto libraries if needed
# target_link_libraries(${PROJECT_NAME} ssl crypto)

# Install library
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${LW_LIB_DIR}
    LIBRARY DESTINATION ${LW_LIB_DIR}
    RUNTIME DESTINATION ${LW_BIN_DIR}
)

# Install header files
install(FILES  "${CMAKE_CURRENT_LIST_DIR}/include/lwdistcomm.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}
)
install(FILES  "${CMAKE_CURRENT_LIST_DIR}/include/types.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}
)
install(FILES  "${CMAKE_CURRENT_LIST_DIR}/include/address.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}
)
install(FILES  "${CMAKE_CURRENT_LIST_DIR}/include/message.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}
)
install(FILES  "${CMAKE_CURRENT_LIST_DIR}/include/security.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}
)
install(FILES  "${CMAKE_CURRENT_LIST_DIR}/include/client.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}
)
install(FILES  "${CMAKE_CURRENT_LIST_DIR}/include/server.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}
)

# Install DDS header files
install(FILES  "${CMAKE_CURRENT_LIST_DIR}/include/dds/dds.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}/dds
)
install(FILES  "${CMAKE_CURRENT_LIST_DIR}/include/dds/dds_types.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}/dds
)
install(FILES  "${CMAKE_CURRENT_LIST_DIR}/include/dds/dds_simple.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}/dds
)

install(FILES  "${CMAKE_CURRENT_LIST_DIR}/include/dds/spdp.h"
    DESTINATION ${LW_INCLUDE_DIR}/${PROJECT_NAME}/dds
)

# Build test executable
add_executable(test_dds test/test_dds.c)
target_link_libraries(test_dds lwdistcomm pthread)
target_include_directories(test_dds PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Build SIGPIPE test executable
add_executable(test_sigpipe test/test_sigpipe.c)
target_link_libraries(test_sigpipe lwdistcomm pthread)
target_include_directories(test_sigpipe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Build simple DDS test executable
add_executable(test_dds_simple test/test_dds_simple.c)
target_link_libraries(test_dds_simple lwdistcomm pthread)
target_include_directories(test_dds_simple PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
