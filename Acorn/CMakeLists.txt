project(Acorn VERSION 0.0.1 LANGUAGES CXX)


include("../cmake/bundle.cmake")

find_package(Vulkan REQUIRED)

if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()

if(NOT Vulkan_FOUND)
    # CMake may fail to locate the libraries but could be able to 
    # provide some path in Vulkan SDK include directory variable
    # 'Vulkan_INCLUDE_DIRS', try to extract path from this.
    message(STATUS "Failed to locate Vulkan SDK, retrying again...")
    if(EXISTS "${VULKAN_PATH}")
        message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
    else()
        message(FATAL_ERROR "Failed to locate Vulkan SDK, aborting...")
        return()
    endif()
endif()

if(WIN32)
    set(VULKAN_DEBUG_LIBS_NAMES "shaderc_sharedd;spirv-cross-cored;spirv-cross-glsld")
else()
    set(VULKAN_DEBUG_LIBS_NAMES "shaderc_shared;spirv-cross-core;spirv-cross-glsl")
endif()

foreach(LIB_NAME ${VULKAN_DEBUG_LIBS_NAMES})
    find_library(FOUND_LIB_${LIB_NAME} NAMES "${LIB_NAME}" HINTS "${VULKAN_PATH}/Lib" "${VULKAN_PATH}/Bin" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
    if(NOT FOUND_LIB_${LIB_NAME})
        message(FATAL_ERROR "Failed to locate ${LIB_NAME}")
    else()
        list(APPEND VULKAN_DEBUG_LIBS ${FOUND_LIB_${LIB_NAME}})
    endif()
endforeach()

set(VULKAN_RELEASE_LIBS "shaderc_shared;spirv-cross-core;spirv-cross-glsl")

find_package(ImGui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(V8 CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(box2d REQUIRED)
find_package(cereal REQUIRED)
find_package(OpenSSL REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
find_path(MAGIC_ENUM_INCLUDE_DIRS "magic_enum.hpp")

file(GLOB_RECURSE SOURCE_FILES
    CONFIGURE_DEPENDS    
    "src/*.cpp"
    "vendor/stb_image/stb_image.cpp"
    "vendor/v8pp/v8pp/*.cpp"
)
file(GLOB_RECURSE HEADER_FILES
    CONFIGURE_DEPENDS
    "src/*.h"
)

#Add Library
# add_library(Acorn SHARED
#     ${SOURCE_FILES} ${HEADER_FILES}
# )
add_library(Acorn STATIC
    ${SOURCE_FILES} ${HEADER_FILES}
)
add_library(Acorn::Acorn ALIAS Acorn)

# Build Settings
target_precompile_headers(Acorn PRIVATE src/acpch.h)
# set_target_properties(Acorn PROPERTIES UNITY_BUILD ON)

# Include Directories
target_include_directories(Acorn
    PRIVATE 
        "${VULKAN_PATH}/Include"
        "${STB_INCLUDE_DIRS}"
        "${MAGIC_ENUM_INCLUDE_DIRS}"
        "${CMAKE_CURRENT_BINARY_DIR}"
        # "${CMAKE_SOURCE_DIR}/../v8/v8/include"
    PUBLIC
        $<INSTALL_INTERFACE:src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:src/Acorn>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Acorn>
        $<INSTALL_INTERFACE:vendor/v8pp>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/v8pp>
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Acorn PRIVATE -DAC_ENABLE_ASSERTS -DPSNIP_DEBUG -DAC_PROFILE=1 -DAC_DEBUG)
endif()

if(WIN32)
    target_compile_definitions(Acorn PRIVATE -DAC_PLATFORM_WINDOWS)
endif()

#Export header ()
# include(GenerateExportHeader)
# generate_export_header(Acorn)

target_compile_definitions(Acorn PRIVATE -DGLFW_INCLUDE_NONE -DAcorn_EXPORTS)

set_target_properties(Acorn PROPERTIES
    CXX_STANDARD 20
    PUBLIC_HEADER "${HEADER_FILES}"
    ENABLE_EXPORTS ON
)

target_compile_options(Acorn PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wpedantic -DV8_COMPRESS_POINTERS>)
    # -Werror>)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(Acorn PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -O0 -ggdb -fno-omit-frame-pointer -g -Wno-unused-parameter>
        )
endif()

#Linking

target_link_directories(Acorn PRIVATE "${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(V8_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/v8-v142-x64.9.3.345.3/lib/Debug")
    # target_link_directories(Acorn PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/v8-v140-x64.7.4.288.11/lib/Debug")
else()
    set(V8_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/v8-v142-x64.9.3.345.3/lib/Release")
    # target_link_directories(Acorn PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/v8-v140-x64.7.4.288.11/lib/Release")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
# target_link_libraries(Acorn PRIVATE V8)
target_link_libraries(Acorn PUBLIC EnTT::EnTT)
target_link_libraries(Acorn PRIVATE glad::glad)
target_link_libraries(Acorn PUBLIC glfw ${OPENGL_gl_LIBRARY})
target_link_libraries(Acorn PUBLIC glm::glm)
target_link_libraries(Acorn PUBLIC spdlog::spdlog)
target_link_libraries(Acorn PUBLIC freetype)
target_link_libraries(Acorn PUBLIC imgui::imgui)
target_link_libraries(Acorn PRIVATE implot::implot)
target_link_libraries(Acorn PRIVATE imguizmo::imguizmo)
target_link_libraries(Acorn PRIVATE yaml-cpp)
target_link_libraries(Acorn PRIVATE Threads::Threads)
target_link_libraries(Acorn PUBLIC V8::V8)
target_link_libraries(Acorn PRIVATE Boost::boost bcrypt)
target_link_libraries(Acorn PRIVATE box2d::box2d)
target_link_libraries(Acorn PRIVATE cereal)
target_link_libraries(Acorn PRIVATE OpenSSL::Crypto)

# set(V8_LIB_PATH "${CMAKE_SOURCE_DIR}/../v8/v8/out.gn/x64.debug/obj")

# get_filename_component(V8_LIB_NAME ${V8_LIB_PATH} ABSOLUTE)

# target_link_libraries(Acorn PRIVATE
#     "${V8_LIB_PATH}/v8_libbase.dll.lib"
#     "${V8_LIB_PATH}/v8_libplatform.dll.lib"
#     "${V8_LIB_PATH}/v8.dll.lib"
#     "${V8_LIB_PATH}/zlib.dll.lib"
# )
# target_link_libraries(Acorn PRIVATE v8_snapshot v8_libbase v8_libplatform)

# target_link_directories(Acorn PUBLIC "${V8_LIB_PATH};${V8_LIB_PATH}/third_party")
# target_link_libraries(Acorn PUBLIC v8_monolith)
# target_link_libraries(Acorn PRIVATE v8pp) # TODO add support for Release

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(Acorn PRIVATE ${VULKAN_DEBUG_LIBS})
else()
    target_link_libraries(Acorn PRIVATE ${VULKAN_RELEASE_LIBS})
endif()

get_target_property(Includes Acorn INCLUDE_DIRECTORIES)
message(STATUS "Include dirs: ${Includes}")

###############################################
# Installation
###############################################

# install(CODE [[
#     message(STATUS "PATH env $ENV{PATH}")
# ]])

# include(GNUInstallDirs)
# set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Acorn)

# message(STATUS "install inc dir ${CMAKE_INSTALL_INCLUDEDIR}")

# install(TARGETS Acorn
#     EXPORT AcornTargets
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )
# install(FILES
#     ${CMAKE_CURRENT_BINARY_DIR}/acorn_export.h
#     DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )

# set_target_properties(Acorn PROPERTIES EXPORT_NAME Acorn)

# # Export the targets to a script
# install(EXPORT AcornTargets
#     FILE
#         AcornTargets.cmake
#     NAMESPACE
#         Acorn::
#     DESTINATION
#         ${INSTALL_CONFIGDIR}
# )

# #Create a ConfigVersion.cmake file
# include(CMakePackageConfigHelpers)
# write_basic_package_version_file(
#     ${CMAKE_CURRENT_BINARY_DIR}/AcornConfigVersion.cmake
#     VERSION ${PROJECT_VERSION}
#     COMPATIBILITY AnyNewerVersion
# )

# configure_package_config_file(
#     ${CMAKE_CURRENT_LIST_DIR}/cmake/AcornConfig.cmake.in
#     ${CMAKE_CURRENT_BINARY_DIR}/AcornConfig.cmake
#     INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
# )

# #Install the config, configversion and custom find modules
# install(FILES
#     ${CMAKE_CURRENT_BINARY_DIR}/AcornConfigVersion.cmake
#     ${CMAKE_CURRENT_BINARY_DIR}/AcornConfig.cmake
#     DESTINATION ${INSTALL_CONFIGDIR}
# )

# ###############################################
# # Exporting from the build tree
# ###############################################

# export(
#     EXPORT AcornTargets
#     FILE ${CMAKE_CURRENT_BINARY_DIR}/AcornTargets.cmake
#     NAMESPACE Acorn::
# )

# # Register package in the user package Registry
# export(PACKAGE Acorn)