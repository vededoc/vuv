cmake_minimum_required(VERSION 3.16)
project(vuv)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MEM_CHECK)
    message(">>> vuv build for memory check <<< ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=address -lasan")
endif()

add_compile_options("-Werror=return-type")

file(GLOB VUV_SRCS src/*.cpp)
add_library(vuv ${VUV_SRCS})
target_include_directories(vuv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(vuv PUBLIC uv pthread dl)

option(VUV_BUILD_TESTS "Build vuv tests" OFF)

if(VUV_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()