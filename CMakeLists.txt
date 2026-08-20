option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

cmake_minimum_required(VERSION 3.20)

if(NOT VCPKG_ROOT)
    set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    if(NOT VCPKG_ROOT)
        message(FATAL_ERROR "VCPKG_ROOT environment variable is not set. Please set it to the root of your vcpkg installation.")
    endif()
endif()

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(evhttp-router)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Libevent CONFIG REQUIRED)

add_library(evhttp_router_common INTERFACE)
target_include_directories(evhttp_router_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(evhttp_router_common INTERFACE libevent::core libevent::extra)

add_library(evhttp_router STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/evhttp_router.cpp)
target_link_libraries(evhttp_router PUBLIC evhttp_router_common)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTS)
    find_package(maxtest CONFIG REQUIRED)
    enable_testing()
    add_subdirectory(tests)
endif()