project(Acorn)

message(STATUS "Acorn")

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

# find_library(Shadercd_LIBRARY NAMES shadercd HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Toolsd_LIBRARY NAMES SPIRV-Toolsd HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Cross_Cored_LIBRARY NAMES spirv-cross-cored HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Cross_GLSLd_LIBRARY NAMES spirv-cross-glsld HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Cross_Reflectd_LIBRARY NAMES spirv-cross-reflectd HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)

# find_library(Shaderc_LIBRARY NAMES shaderc HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Tools_LIBRARY NAMES SPIRV-Tools HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Cross_Core_LIBRARY NAMES spirv-cross-core HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Cross_GLSL_LIBRARY NAMES spirv-cross-glsl HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
# find_library(SPIRV_Cross_Reflect_LIBRARY NAMES spirv-cross-reflect HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)

# set(VULKAN_DEBUG_LIBS "${Shadercd_LIBRARY};${SPIRV_Toolsd_LIBRARY};${SPIRV_Cross_Cored_LIBRARY};${SPIRV_Cross_GLSLd_LIBRARY};${SPIRV_Cross_Reflectd_LIBRARY}")
# set(VULKAN_RELEASE_LIBS "${Shaderc_LIBRARY};${SPIRV_Tools_LIBRARY};${SPIRV_Cross_Core_LIBRARY};${SPIRV_Cross_GLSL_LIBRARY};${SPIRV_Cross_Reflect_LIBRARY}")

# if(WIN32)
#     set(VULKAN_DEBUG_LIBS "SPIRV-toolsd;SPIRV-tools-sharedd")
# else()
#     set(VULKAN_DEBUG_LIBS "SPIRV-tools;SPIRV-tools-shared")
# endif()

# set(VULKAN_RELEASE_LIBS "SPIRV-tools;SPIRV-tools-shared")

#TODO -d to debug versions of vulkan libs??
find_package(ImGui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(SPIRV-Tools CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")

message(STATUS "Imgui ${imgui_SOURCE_DIR}")

find_library(shaderc_LIB NAMES "shaderc" REQUIRED)
find_library(shaderc_util_LIB NAMES "shaderc_util" REQUIRED)
# find_library(glslang_LIB NAMES "glslang" REQUIRED)

message(STATUS "glslang ${glslang_LIB}")

file(GLOB_RECURSE SOURCE_FILES
    CONFIGURE_DEPENDS    
    "src/**/*.cpp"
    "vendor/stb_image/stb_image.cpp"
)
file(GLOB_RECURSE HEADER_FILES
    CONFIGURE_DEPENDS
    "src/**/*.h"
)

add_library(Acorn SHARED
${SOURCE_FILES} ${HEADER_FILES}
)

# target_precompile_headers(Acorn PRIVATE src/acpch.h)
# set_target_properties(Acorn PROPERTIES UNITY_BUILD ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
target_compile_definitions(Acorn PRIVATE -DAC_ENABLE_ASSERTS -DPSNIP_DEBUG -DAC_PROFILE=0)
endif()

target_compile_definitions(Acorn PRIVATE -DGLFW_INCLUDE_NONE)

#${VULKAN_PATH}/Bin;
target_link_directories(Acorn PRIVATE "${VULKAN_PATH}/Lib;")

target_link_libraries(Acorn PUBLIC EnTT::EnTT)
target_link_libraries(Acorn PRIVATE glad::glad)
target_link_libraries(Acorn PUBLIC glfw)
target_link_libraries(Acorn PUBLIC glm::glm)
target_link_libraries(Acorn PUBLIC spdlog::spdlog)
target_link_libraries(Acorn PUBLIC imgui::imgui)
target_link_libraries(Acorn PRIVATE implot::implot)
target_link_libraries(Acorn PRIVATE imguizmo::imguizmo)
target_link_libraries(Acorn PRIVATE spirv-cross-core)
target_link_libraries(Acorn PRIVATE spirv-cross-glsl)
target_link_libraries(Acorn PRIVATE yaml-cpp)
target_link_libraries(Acorn PRIVATE glslang SPIRV OGLCompiler)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

target_link_libraries(Acorn PRIVATE ${SPIRV-Tools_LIBRARIES})
target_link_libraries(Acorn PRIVATE ${shaderc_LIB})
target_link_libraries(Acorn PRIVATE ${shaderc_util_LIB})

target_link_libraries(Acorn PRIVATE $<$<CONFIG:Debug>:$VULKAN_DEBUG_LIBS> $<$<CONFIG:Release>:$VULKAN_RELEASE_LIBS>)

# target_include_directories(Acorn PRIVATE "${VULKAN_PATH}/Include")
target_include_directories(Acorn PRIVATE ${STB_INCLUDE_DIRS})
target_include_directories(Acorn PUBLIC src)

set_target_properties(Acorn PROPERTIES
    CXX_STANDARD 20
    PUBLIC_HEADER "${HEADER_FILES}"
    ENABLE_EXPORTS ON
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

# foreach(VULKAN_LIB ${VULKAN_DEBUG_LIBS})
#     message(STATUS "Copying Vulkan ${VULKAN_LIB}")

#     if(EXISTS "${VULKAN_PATH}/Bin/${VULKAN_LIB}.dll")
#         add_custom_command(TARGET Acorn PRE_LINK
#             COMMAND ${CMAKE_COMMAND} -E copy "${VULKAN_PATH}/Bin/${VULKAN_LIB}.dll" "${CMAKE_CURRENT_BINARY_DIR}/${VULKAN_LIB}.dll"
#         )
#     else()
#         message(STATUS "    Vulkan ${VULKAN_LIB} not found")
#     endif()

# endforeach()
