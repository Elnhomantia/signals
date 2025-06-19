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
<<<<<<< HEAD

# ============ Tests ============

enable_testing()
add_subdirectory(tests)

add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_dependencies(signals check)
=======
>>>>>>> 743a6bc (minimal release. Rebase for v1.1.0)
