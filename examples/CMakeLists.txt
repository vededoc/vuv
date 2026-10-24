add_executable(vuv_tcp_echo_timer tcp_echo_timer.cpp)
target_link_libraries(vuv_tcp_echo_timer PRIVATE vuv)
target_include_directories(vuv_tcp_echo_timer PRIVATE ${CMAKE_SOURCE_DIR}/src)
