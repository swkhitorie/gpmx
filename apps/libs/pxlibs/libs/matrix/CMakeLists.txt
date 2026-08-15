
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/matrix)

if(BUILD_TEST)
	add_subdirectory(test)
endif()
