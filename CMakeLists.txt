cmake_minimum_required(VERSION 3.17)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Setting toolchain file for vpckg")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
elseif(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND NOT OVERRIDE_VCPKG_REQUIREMENT)
    message(FATAL_ERROR "Vcpkg is required to build Acorn. Please install it from https://vcpkg.io")
else()
    message(STATUS "Toolchain file is ${CMAKE_TOOLCHAIN_FILE}")
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES CACHE BOOL "Export all symbols")

project(Acorn)

cmake_policy(SET CMP0074 NEW)

set(CMAKE_CXX_STANDARD 20)

option(OVERRIDE_VCPKG_REQUIREMENT OFF)

add_subdirectory(Acorn)
add_subdirectory(OakTree)