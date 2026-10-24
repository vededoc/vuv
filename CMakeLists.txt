cmake_minimum_required(VERSION 3.16)
project(vuv)
set(CMAKE_CXX_STANDARD 20)

if(MEM_CHECK)
    message(">>> vuv build for memory check <<< ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=address -lasan")
endif()

add_compile_options("-Werror=return-type")

file(GLOB VUV_SRCS src/*.cpp)
add_library(vuv ${VUV_SRCS})
target_include_directories(vuv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vuv PUBLIC uv pthread dl)