include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

# For Windows: prevent overriding the parent project's compiler/linker settings
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(VUV_TEST_SOURCES
    smoke_test.cpp
    test_loop.cpp
    test_bufstore.cpp
    test_uv_idle_immediate.cpp
    test_async.cpp
    test_timer.cpp
    test_check_prepare.cpp
    test_poll.cpp
    test_signal.cpp
    test_dns.cpp
    test_udp.cpp
    test_tcp.cpp
    test_spawn.cpp
)

add_executable(vuv_smoke_test ${VUV_TEST_SOURCES})
target_link_libraries(vuv_smoke_test PRIVATE vuv GTest::gtest_main)
target_include_directories(vuv_smoke_test PRIVATE ${CMAKE_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(vuv_smoke_test)
