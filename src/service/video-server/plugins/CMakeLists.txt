# CMakeLists.txt for plugins

# 插件源文件
set(RTSP_PLUGIN_SOURCES
    RTSPPlugin.cpp
)

set(HIKVISION_PLUGIN_SOURCES
    HikvisionPlugin.cpp
)

# 构建RTSP插件
add_library(rtsp_plugin SHARED ${RTSP_PLUGIN_SOURCES})
set_target_properties(rtsp_plugin PROPERTIES
    OUTPUT_NAME "rtsp"
    PREFIX ""
    SUFFIX ".so"
)

# 构建海康插件
add_library(hikvision_plugin SHARED ${HIKVISION_PLUGIN_SOURCES})
set_target_properties(hikvision_plugin PROPERTIES
    OUTPUT_NAME "hikvision"
    PREFIX ""
    SUFFIX ".so"
)

# 链接库
target_link_libraries(rtsp_plugin
    ${LW_LIB_DIR}/liblwlog.so
    pthread
)

target_link_libraries(hikvision_plugin
    ${LW_LIB_DIR}/liblwlog.so
    pthread
)

# 安装插件
install(TARGETS rtsp_plugin hikvision_plugin
    DESTINATION ${LW_BIN_DIR}/plugins
)

# 显示构建信息
message(STATUS "Plugin build configuration:")
message(STATUS "  RTSP plugin sources: ${RTSP_PLUGIN_SOURCES}")
message(STATUS "  Hikvision plugin sources: ${HIKVISION_PLUGIN_SOURCES}")
