cmake_minimum_required(VERSION 3.10)
project(config-monitor C)

set(CMAKE_C_STANDARD 99)

# ---------------- 库 ----------------
add_library(config-monitor src/config_monitor.c)
target_include_directories(config-monitor PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(config-monitor PROPERTIES EXPORT_NAME config-monitor)

# ---------------- 示例程序 ----------------
add_executable(config-monitor-demo examples/main.c)
target_link_libraries(config-monitor-demo PRIVATE config-monitor pthread)

# ---------------- 安装 ----------------
include(GNUInstallDirs)

install(TARGETS config-monitor
        EXPORT config-monitorTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT config-monitorTargets
        FILE config-monitorTargets.cmake
        NAMESPACE config-monitor::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/config-monitor)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/config-monitorConfigVersion.cmake
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/config-monitorConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/config-monitor
)
