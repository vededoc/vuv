include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# For Windows: prevent overriding the parent project's compiler/linker settings
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(vuv_smoke_test smoke_test.cpp)
target_link_libraries(vuv_smoke_test PRIVATE vuv GTest::gtest_main)
target_include_directories(vuv_smoke_test PRIVATE ${CMAKE_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(vuv_smoke_test)
