cmake_minimum_required(VERSION 3.16)

project(signals VERSION 1.1.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(signals INTERFACE include/signals.h include/macros.h)
target_include_directories(signals INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/Elth/signals>
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Elth/signals)
install(TARGETS signals EXPORT signalsTargets)
install(EXPORT signalsTargets
    FILE signalsTargets.cmake
    NAMESPACE Elth::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/signals
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/signalsConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
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

# ============ Tests ============

include(CTest)
enable_testing()
add_subdirectory(tests)

add_custom_target(check ALL
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS signals tests_all
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
