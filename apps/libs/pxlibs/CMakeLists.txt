include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platform)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/matrix)

add_subdirectory(libs)
add_subdirectory(platform)

add_library(pxlibs INTERFACE)

target_include_directories(pxlibs INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/matrix)

target_link_libraries(pxlibs INTERFACE 
    px_platform
    crc
    crc32
    parameters
    airspeed
    controllib
    conversion
    geo
    terrain_estimation
    timesync
    tinybson
    world_magnetic_model
)

if(BUILD_TEST)
target_link_libraries(pxlibs INTERFACE 
    controllib_test
)
endif()
