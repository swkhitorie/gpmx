include_guard(GLOBAL)

if(NOT DEFINED _GTEST_HELPER_INITIALIZED)
    set(_GTEST_HELPER_INITIALIZED ON)

    find_package(GTest QUIET)

    if(GTest_FOUND)
        message(STATUS "Using system-installed Google Test")
        set(GTEST_MAIN_LIB GTest::gtest_main)
        set(GTEST_LIB GTest::gtest)
    else()
        message(STATUS "System GTest not found, will fetch from GitHub")
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        release-1.12.1
        )
        FetchContent_MakeAvailable(googletest)
        set(GTEST_MAIN_LIB gtest_main)
        set(GTEST_LIB gtest)
    endif()
endif()
