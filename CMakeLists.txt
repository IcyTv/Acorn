cmake_minimum_required(VERSION 3.17)

# Variables
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_INSPECTOR "Enable ultralight inspector" ON)

set(VCPKG_PREFER_SYSTEM_LIBS ON CACHE BOOL "Prefer System Libraries")
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

cmake_policy(SET CMP0074 NEW)

set(CMAKE_CXX_STANDARD 20)

option(OVERRIDE_VCPKG_REQUIREMENT OFF)

add_subdirectory(../swc-cpp swc-cpp)
# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

add_subdirectory(Acorn)
add_subdirectory(OakTree)
add_subdirectory(Ultralight)

include(CPack)