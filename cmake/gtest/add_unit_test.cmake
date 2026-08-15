
#   add_unit_test(TARGET_NAME <name>
#                 SOURCES  <src1> [<src2> ...]
#                 [LIBRARIES <lib1> [<lib2> ...]]
#                )
function(add_unit_test)
    set(options "")
    set(oneValueArgs TARGET_NAME)
    set(multiValueArgs SOURCES LIBRARIES)
    cmake_parse_arguments(UTEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT UTEST_TARGET_NAME)
        message(FATAL_ERROR "add_unit_test: TARGET_NAME is required")
    endif()

    if(NOT UTEST_SOURCES)
        message(FATAL_ERROR "add_unit_test: SOURCES list is required")
    endif()

    add_executable(${UTEST_TARGET_NAME} ${UTEST_SOURCES})

    target_link_libraries(${UTEST_TARGET_NAME}
        PRIVATE
            ${GTEST_MAIN_LIB}
            ${UTEST_LIBRARIES}
    )

    include(GoogleTest)
    gtest_discover_tests(${UTEST_TARGET_NAME})
endfunction()

