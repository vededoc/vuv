cmake_minimum_required(VERSION 3.16)
project(vuv)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MEM_CHECK)
    message(">>> vuv build for memory check <<< ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=address -lasan")
endif()

add_compile_options("-Werror=return-type")

include(GNUInstallDirs)

file(GLOB VUV_SRCS src/*.cpp)
add_library(vuv ${VUV_SRCS})

target_include_directories(vuv PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(vuv PUBLIC uv pthread dl)

option(VUV_BUILD_TESTS "Build vuv tests" OFF)

if(VUV_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---- install rules ----
install(TARGETS vuv
    EXPORT vuvTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vuv
    FILES_MATCHING PATTERN "*.h")

install(EXPORT vuvTargets
    FILE vuvTargets.cmake
    NAMESPACE vuv::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vuv)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vuvConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/vuvConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vuv)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/vuvConfigVersion.cmake
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/vuvConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/vuvConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vuv)
