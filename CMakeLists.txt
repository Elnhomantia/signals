cmake_minimum_required(VERSION 3.16)

project(signals LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(signals INTERFACE include/signal.h include/macros.h)
target_include_directories(signals INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS signals EXPORT signalsTargets)
install(EXPORT signalsTargets
    FILE signalsTargets.cmake
    NAMESPACE signals::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/signals
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/signalsConfigVersion.cmake
    VERSION 1.1.2
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/signalsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/signals
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/signalsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/signalsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/signals
)
