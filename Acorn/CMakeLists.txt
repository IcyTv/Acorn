project(Acorn)

message(STATUS "Acorn")

include("../cmake/bundle.cmake")

# find_package(Vulkan REQUIRED)

# if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
#     set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
#     STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
# endif()

# if(NOT Vulkan_FOUND)
#     # CMake may fail to locate the libraries but could be able to 
#     # provide some path in Vulkan SDK include directory variable
#     # 'Vulkan_INCLUDE_DIRS', try to extract path from this.
#     message(STATUS "Failed to locate Vulkan SDK, retrying again...")
#     if(EXISTS "${VULKAN_PATH}")
#         message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
#     else()
#         message(FATAL_ERROR "Failed to locate Vulkan SDK, aborting...")
#         return()
#     endif()
# endif()

# find_library(Shaderc_LIBRARY NAMES shaderc HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Tools_LIBRARY NAMES SPIRV-Tools HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Cross_Core_LIBRARY NAMES spirv-cross-core HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Cross_GLSL_LIBRARY NAMES spirv-cross-glsl HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Cross_Reflect_LIBRARY NAMES spirv-cross-reflect HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)

# set(VULKAN_LIBS "${Shaderc_LIBRARY};${SPIRV_Tools_LIBRARY};${SPIRV_Cross_Core_LIBRARY};${SPIRV_Cross_GLSL_LIBRARY};${SPIRV_Cross_Reflect_LIBRARY}")

# message(STATUS "Found Vulkan Libs ${VULKAN_LIBS}")

#TODO -d to debug versions of vulkan libs??
find_package(ImGui CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_library(shaderc_LIB NAMES shaderc STATIC)
find_library(shaderc_util_LIB NAMES shaderc_util STATIC)
find_path(shaderc_INCLUDE "shaderc/shaderc.hpp")
find_path(STB_INCLUDE_DIRS "stb.h")

message(STATUS "Imgui ${imgui_SOURCE_DIR}")

file(GLOB_RECURSE SOURCE_FILES
    CONFIGURE_DEPENDS    
    "src/**/*.cpp"
    "vendor/stb_image/stb_image.cpp"
)
file(GLOB_RECURSE HEADER_FILES
    CONFIGURE_DEPENDS
    "src/**/*.h"
)
    

include_directories(AFTER ${VULKAN_PATH}/Include)
link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)

add_library(Acorn 
# STATIC
${SOURCE_FILES} ${HEADER_FILES}
)

target_precompile_headers(Acorn PRIVATE src/acpch.h)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
target_compile_definitions(Acorn PRIVATE -DAC_ENABLE_ASSERTS -DPSNIP_DEBUG -DAC_PROFILE=0)
endif()

target_compile_definitions(Acorn PRIVATE -DGLFW_INCLUDE_NONE)

target_link_libraries(Acorn EnTT::EnTT)
target_link_libraries(Acorn glad::glad)
target_link_libraries(Acorn glfw)
target_link_libraries(Acorn glm::glm)
target_link_libraries(Acorn spdlog::spdlog)
target_link_libraries(Acorn imgui::imgui)
target_link_libraries(Acorn yaml-cpp)
target_link_libraries(Acorn spirv-cross-core)
target_link_libraries(Acorn spirv-cross-glsl)
target_link_libraries(Acorn spirv-cross-reflect)
target_link_libraries(Acorn ${shaderc_LIB})
target_link_libraries(Acorn ${shaderc_util_LIB})

message(STATUS "shaderc ${shaderc_LIB}")
# target_link_libraries(Acorn PRIVATE unofficial::portable-snippets::cpu unofficial::portable-snippets::once unofficial::portable-snippets::check unofficial::portable-snippets::clock)

target_include_directories(Acorn PUBLIC "${VULKAN_PATH}/Include")
target_include_directories(Acorn PUBLIC ${STB_INCLUDE_DIRS})
target_include_directories(Acorn PUBLIC ${shaderc_INCLUDE})
target_include_directories(Acorn PUBLIC
    src
    vendor/ImNodes
)

set_target_properties(Acorn PROPERTIES
    CXX_STANDARD 20
    PUBLIC_HEADER "${HEADER_FILES}"
)

install(
    TARGETS Acorn
    EXPORT Acorn
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# bundle_static_library(Acorn libAcorn)

