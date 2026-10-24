cmake_minimum_required(VERSION 3.16)
project(vuv VERSION 0.9.0 LANGUAGES CXX)
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
option(VUV_BUILD_EXAMPLES "Build vuv examples" OFF)

if(VUV_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(VUV_BUILD_EXAMPLES)
    add_subdirectory(examples)
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
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/vuvConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/vuvConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vuv)

# ---- CPack packaging ----
set(CPACK_PACKAGE_NAME "vuv")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A modern C++20 wrapper around libuv")
set(CPACK_PACKAGE_VENDOR "vededoc")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR "TGZ;ZIP")
include(CPack)
