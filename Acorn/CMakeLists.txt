#Install Vulkan (shaderc...) dependencies
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

find_library(Shaderc_LIBRARY NAMES shaderc HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
find_library(SPIRV_Tools_LIBRARY NAMES SPIRV-Tools HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
find_library(SPIRV_Cross_Core_LIBRARY NAMES spirv_cross_core HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
message(STATUS "Shaderc ${Shaderc_LIBRARY} ${SPIRV_Tools_LIBRARY} ${SPIRV_Cross_Core_LIBRARY}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(VULKAN_LINKS shadercd;SPIRV-Toolsd;spirv-cross-cored;spirv-cross-reflectd;spirv-cross-glsld)
else()
    set(VULKAN_LINKS shaderc;SPIRV-Tools;spirv-cross-core;spirv-cross-reflect;spirv-cross-glsl)
endif()

find_package(ImGui CONFIG REQUIRED)
# find_package(freetype CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
# find_package(stb CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

add_subdirectory(vendor/glad)

file(GLOB_RECURSE SOURCE_FILES
    "src/**/*.cpp"
    # "vendor/stb_image/stb_image.cpp"
)
file(GLOB_RECURSE HEADER_FILES
    "src/**/*.h"
)

#TODO move to submodule -> Seperate build
set(IMGUI_ROOT vendor/ImGui)

set(IMGUI_SOURCES
    "${IMGUI_ROOT}/imgui.cpp"
    "${IMGUI_ROOT}/imgui_draw.cpp"
    "${IMGUI_ROOT}/imgui_demo.cpp"
    "${IMGUI_ROOT}/imgui_tables.cpp"
    "${IMGUI_ROOT}/imgui_widgets.cpp"
    "${IMGUI_ROOT}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp"
)
set(IMGUI_HEADERS
    "${IMGUI_ROOT}/imgui.h"
    "${IMGUI_ROOT}/imconfig.h"
    "${IMGUI_ROOT}/imgui_internal.h"
    "${IMGUI_ROOT}/imgui.h"
    "${IMGUI_ROOT}/imstb_rectpack.h"
    "${IMGUI_ROOT}/imstb_textedit.h"
    "${IMGUI_ROOT}/imstb_truetype.h"
)
    

include_directories(AFTER ${VULKAN_PATH}/Include)
link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)

add_library(Acorn 
    ${SOURCE_FILES} ${HEADER_FILES}
    ${IMGUI_SOURCES} ${IMGUI_HEADERS}
)

target_precompile_headers(Acorn PRIVATE src/acpch.h)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Acorn PRIVATE -DAC_ENABLE_ASSERTS -DPSNIP_DEBUG -DAC_PROFILE=0)
endif()


set(LIBS "ImGui;EnTT;glad;glfw;glm;spdlog;yaml-cpp;${VULKAN_LINKS}")

target_link_libraries(Acorn PUBLIC ${LIBS})

target_include_directories(Acorn PUBLIC
    src
    vendor/ImGui
    vendor/ImPlot
    vendor/ImGuizmo
    vendor/ImNodes
    ${IMGUI_ROOT}
    ${SHADERC_INCLUDE_DIRS})
    # ${spdlog_INCLUDE_DIRS})
# target_include_directories(Acorn PRIVATE ${SHADERC_INCLUDE_DIRS})
