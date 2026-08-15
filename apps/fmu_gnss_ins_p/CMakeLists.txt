cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0148 NEW)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "Proj build: fmu_gnss_ins_p")

project(fmu_gnss_ins_p)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJ_ROOT_DIR ${PROJECT_BINARY_DIR}/../../)

include(${PROJ_ROOT_DIR}/cmake/common.cmake)
if(BUILD_TEST)
include(${PROJ_ROOT_DIR}/cmake/gtest/google_test_check.cmake)
include(${PROJ_ROOT_DIR}/cmake/gtest/add_unit_test.cmake)
endif()

################################
#  gpm library add
################################
set(GPMX_LIB "${PROJ_ROOT_DIR}/build/fmu_gnss_ins_p/fmu_gnss_ins_p.a")
set(MAKE_OS "Linux")

add_custom_command(
    OUTPUT  ${GPMX_LIB}
    COMMAND make all
            APP_SUBPATH=fmu_gnss_ins_p
            OS=${MAKE_OS}
            BUILD_USR_BOARD=unix
            CONFIG_MODULE_UORB=y
            UORB_INCLUDING=${UORB_INCLUDING}
    WORKING_DIRECTORY ${PROJ_ROOT_DIR}
    COMMENT "Building fmu_gnss_ins_p.a using legacy Make"
    JOB_SERVER_AWARE TRUE
)

add_library(lib_gpm STATIC IMPORTED GLOBAL)
set_target_properties(lib_gpm PROPERTIES
    IMPORTED_LOCATION ${GPMX_LIB}
)

add_custom_target(generate_fmu_lib DEPENDS ${GPMX_LIB})
add_dependencies(lib_gpm generate_fmu_lib)

target_link_libraries(lib_gpm INTERFACE pthread m)
target_include_directories(lib_gpm INTERFACE 
    ${PROJ_ROOT_DIR}/gpm/sched/platform
    ${PROJ_ROOT_DIR}/gpm/sched/platform/hrt/queue
    ${PROJ_ROOT_DIR}/gpm/mm
    ${PROJ_ROOT_DIR}/gpm/modules
    ${PROJECT_BINARY_DIR}
)

set(main_src
    ./cjson/cJSON.c
    ./cjson/cJSON_Utils.c
    ./serial/pserial.c
    px_platform.cpp
    main.cpp
)
set(main_inc
    ./cjson/
    ./serial/
)
include_directories(${main_inc})
add_executable(fmu_gnss_ins_p ${main_src})

target_link_libraries(fmu_gnss_ins_p PRIVATE lib_gpm)

add_subdirectory(${PROJ_ROOT_DIR}/apps/libs/ulog_cpp ulog_cpp)
target_link_libraries(fmu_gnss_ins_p PRIVATE ulog_cpp::ulog_cpp)

add_subdirectory(${PROJ_ROOT_DIR}/apps/libs/pxlibs pxlibs)
target_link_libraries(fmu_gnss_ins_p PRIVATE pxlibs pthread m)

add_subdirectory(${PROJ_ROOT_DIR}/apps/libs/pxlibs/modules/replay modules_replay)
target_link_libraries(fmu_gnss_ins_p PRIVATE modules_replay)


