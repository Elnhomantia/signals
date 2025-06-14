cmake_minimum_required(VERSION 3.16)

project(signal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(Elth_signal INTERFACE)
add_library(Elth::signal ALIAS Elth_signal)
target_include_directories(Elth_signal INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)


# ============ Install ============

include(GNUInstallDirs)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/signal.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Elth
    RENAME signal
)
install(
    TARGETS Elth_signal
    EXPORT ElthTargets
)
install(
    EXPORT ElthTargets
    NAMESPACE Elth::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Elth
)


# ============ Tests ============

enable_testing()
add_subdirectory(tests)
