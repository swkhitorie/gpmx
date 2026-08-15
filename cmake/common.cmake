cmake_minimum_required(VERSION 3.10)

add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${PROJ_ROOT_DIR}/compile_commands.json
    DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
    COMMENT "Copying compile_commands.json to source directory"
)

if(CMAKE_VERSION VERSION_LESS "3.12")
    find_package(PythonInterp 3 REQUIRED)
    set(Python3_EXECUTABLE ${PYTHON_EXECUTABLE})
else()
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
endif()

if(NOT Python3_EXECUTABLE)
    message(FATAL_ERROR "Python 3 not found. Please install Python 3:\n"
        "    Ubuntu: sudo apt install python3 python3-dev python3-pip\n"
        "    macOS: brew install python")
endif()

option(PYTHON_COVERAGE "Python code coverage" OFF)
if(PYTHON_COVERAGE)
    message(STATUS "python coverage enabled")
    set(Python3_EXECUTABLE "coverage run -p")
endif()

if(BUILD_TEST)
    message(STATUS "GoogleTest On")
    enable_testing()
else()
    message(STATUS "GoogleTest Off")
endif()
