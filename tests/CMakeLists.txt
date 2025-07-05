set(TEST_SOURCES
    test_static.cpp
    test_static_bind.cpp
    test_lambda.cpp
    test_lambda_bind.cpp
    test_class.cpp
    test_class_bind.cpp
    test_shared.cpp
    test_shared_bind.cpp
    test_macros.cpp
)

set(tests_executables)

foreach(test_file IN LISTS TEST_SOURCES)
    get_filename_component(file_name ${test_file} NAME_WE)
    add_executable(${file_name} ${test_file})
    target_link_libraries(${file_name} PRIVATE signals)
    target_include_directories(${file_name} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    add_test(NAME ${file_name} COMMAND ${file_name})
    list(APPEND tests_executables ${file_name})
endforeach()

add_custom_target(tests_all DEPENDS ${tests_executables})
