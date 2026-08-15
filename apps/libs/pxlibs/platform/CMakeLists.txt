
add_library(px_platform 
    px_log.cpp
    module.cpp
    px_task.cpp
)

target_link_libraries(px_platform PUBLIC lib_gpm)
target_include_directories(px_platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

