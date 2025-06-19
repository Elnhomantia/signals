cmake_minimum_required(VERSION 3.16)

project(signals LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(signals INTERFACE signal.h macros.h)

include(GNUInstallDirs)
install(TARGETS signals
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
