cmake_minimum_required(VERSION 3.17)

# Variables
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
set(ENABLE_PROFILING ON CACHE BOOL "Enable Profiling")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES CACHE BOOL "Export all symbols")
set(VCPKG_LIBRARY_LINKAGE static CACHE STRING "Vcpkg dynamic linking")
# TODO platform specific
# set(VCPKG_TARGET_TRIPLET x64-windows-static CACHE STRING "Vcpkg target triplet")
set(VCPKG_TARGET_TRIPLET x64-windows-static CACHE STRING "Vcpkg target triplet")

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Setting toolchain file for vpckg")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
elseif(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND NOT OVERRIDE_VCPKG_REQUIREMENT)
    message(FATAL_ERROR "Vcpkg is required to build Acorn. Please install it from https://vcpkg.io")
else()
    message(STATUS "Toolchain file is ${CMAKE_TOOLCHAIN_FILE}")
endif()
    
project(Acorn)

add_library(PkgConfig::GLIB2 STATIC IMPORTED)
set_target_properties(
    PkgConfig::GLIB2 PROPERTIES
        IMPORTED_LOCATION "C:/cygwin64/lib/libglib-2.0.dll.a"
)

cmake_policy(SET CMP0074 NEW)

set(CMAKE_CXX_STANDARD 20)

option(OVERRIDE_VCPKG_REQUIREMENT OFF)

# add_subdirectory(Acorn/vendor/v8pp)
# add_subdirectory(vendor/v8-cmake)
add_subdirectory(Acorn)
add_subdirectory(OakTree)

include(CPack)