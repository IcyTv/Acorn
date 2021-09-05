#Install Vulkan (shaderc...) dependencies
project(Acorn)

message(STATUS "Acorn")
include(../cmake/bundle.cmake)

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
find_library(SPIRV_Cross_Core_LIBRARY NAMES spirv-cross-core HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
find_library(SPIRV_Cross_GLSL_LIBRARY NAMES spirv-cross-glsl HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
find_library(SPIRV_Cross_Reflect_LIBRARY NAMES spirv-cross-reflect HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)

set(VULKAN_LIBS "${Shaderc_LIBRARY};${SPIRV_Tools_LIBRARY};${SPIRV_Cross_Core_LIBRARY};${SPIRV_Cross_GLSL_LIBRARY};${SPIRV_Cross_Reflect_LIBRARY}")

message(STATUS "Found Vulkan Libs ${VULKAN_LIBS}")

#TODO -d to debug versions of vulkan libs??
# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     set(VULKAN_LINKS shadercd;SPIRV-Toolsd;spirv-cross-cored;spirv-cross-reflectd;spirv-cross-glsld)
# else()
#     set(VULKAN_LINKS shaderc;SPIRV-Tools;spirv-cross-core;spirv-cross-reflect;spirv-cross-glsl)
# endif()

# find_library(ImGui_Lib imgui REQUIRED)
# find_package(imgui CONFIG REQUIRED)
# set(CMAKE_FIND_DEBUG_MODE ON)
# find_package(ImGui CONFIG REQUIRED)
# find_package(freetype CONFIG REQUIRED)
# find_package(EnTT CONFIG REQUIRED)
# find_package(glfw CONFIG REQUIRED)
# find_package(glm CONFIG REQUIRED)
# find_package(spdlog CONFIG REQUIRED)
# # find_package(stb CONFIG REQUIRED)
# find_package(yaml-cpp CONFIG REQUIRED)

add_subdirectory(vendor/glad)

message(STATUS "Imgui ${imgui_SOURCE_DIR}")

file(GLOB_RECURSE SOURCE_FILES
    CONFIGURE_DEPENDS    
    "src/**/*.cpp"
    # "vendor/stb_image/stb_image.cpp"
    "${imgui_SOURCE_DIR}/**/*.cpp"
)
file(GLOB_RECURSE HEADER_FILES
    CONFIGURE_DEPENDS
    "src/**/*.h"
    "${imgui_SOURCE_DIR}/**/*.h"
)

list(FILTER SOURCE_FILES EXCLUDE REGEX "_deps/")
list(FILTER HEADER_FILES EXCLUDE REGEX "_deps/")

#TODO move to submodule -> Seperate build
# set(IMGUI_ROOT vendor/ImGui)

# set(IMGUI_SOURCES
#     "${IMGUI_ROOT}/imgui.cpp"
#     "${IMGUI_ROOT}/imgui_draw.cpp"
#     "${IMGUI_ROOT}/imgui_demo.cpp"
#     "${IMGUI_ROOT}/imgui_tables.cpp"
#     "${IMGUI_ROOT}/imgui_widgets.cpp"
#     "${IMGUI_ROOT}/backends/imgui_impl_glfw.cpp"
#     "${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp"
# )
# set(IMGUI_HEADERS
#     "${IMGUI_ROOT}/imgui.h"
#     "${IMGUI_ROOT}/imconfig.h"
#     "${IMGUI_ROOT}/imgui_internal.h"
#     "${IMGUI_ROOT}/imgui.h"
#     "${IMGUI_ROOT}/imstb_rectpack.h"
#     "${IMGUI_ROOT}/imstb_textedit.h"
#     "${IMGUI_ROOT}/imstb_truetype.h"
# )
    

include_directories(AFTER ${VULKAN_PATH}/Include)
link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)

set(LIBS "EnTT;glad;glfw;glm;spdlog;yaml-cpp;${VULKAN_LINKS}")

# link_libraries(${LIBS})

add_library(Acorn 
    STATIC
    ${SOURCE_FILES} ${HEADER_FILES}
)

target_precompile_headers(Acorn PRIVATE src/acpch.h)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Acorn PRIVATE -DAC_ENABLE_ASSERTS -DPSNIP_DEBUG -DAC_PROFILE=0)
endif()

target_link_directories(Acorn PUBLIC "${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;")
target_link_libraries(Acorn PUBLIC "${LIBS}")

target_include_directories(Acorn PUBLIC "${VULKAN_PATH}/Include")
target_include_directories(Acorn PUBLIC
    src
    ${imgui_SOURCE_DIR}
    "${portable-snippets_SOURCE_DIR}"
    "${entt_SOURCE_DIR}/src"
    "${glfw_SOURCE_DIR}/include"
    "${glm_SOURCE_DIR}"
    "${spdlog_SOURCE_DIR}/include"
    "${stb_SOURCE_DIR}"
    "${yaml-cpp_SOURCE_DIR}/include"
    vendor/ImGui
    vendor/ImPlot
    vendor/ImGuizmo
    vendor/ImNodes
    ${SHADERC_INCLUDE_DIRS})

set_target_properties(Acorn PROPERTIES
    CXX_STANDARD 20
    PUBLIC_HEADER "${HEADER_FILES}"
)

bundle_static_library(Acorn Acorn_budled)

install(
    TARGETS Acorn
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION includes
)

# include(CMakePackageConfigHelpers)
