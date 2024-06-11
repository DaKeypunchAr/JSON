cmake_minimum_required(VERSION 3.28...3.29)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project(json VERSION 1.0.2 LANGUAGES CXX)

add_library(json "src/JSON.cpp")
target_include_directories(json PUBLIC "include" "${CMAKE_CURRENT_BINARY_DIR}")

get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
configure_file("src/root.hpp.in" root.hpp @ONLY)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(json PUBLIC _DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(json PUBLIC _RELEASE)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_compile_definitions(json PUBLIC _32BIT)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_compile_definitions(json PUBLIC _64BIT)
endif()