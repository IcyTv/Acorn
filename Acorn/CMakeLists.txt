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

if(WIN32)
    set(VULKAN_DEBUG_LIBS_NAMES "shaderc_sharedd;spirv-cross-cored;spirv-cross-glsld")
else()
    set(VULKAN_DEBUG_LIBS_NAMES "shaderc_shared;spirv-cross-core;spirv-cross-glsl")
endif()

foreach(LIB_NAME ${VULKAN_DEBUG_LIBS_NAMES})
    message(STATUS "Looking for ${LIB_NAME}")
    find_library(FOUND_LIB_${LIB_NAME} NAMES "${LIB_NAME}" HINTS "${VULKAN_PATH}/Lib" "${VULKAN_PATH}/Bin" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
    message(STATUS "Found ${FOUND_LIB_${LIB_NAME}}")
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
find_path(STB_INCLUDE_DIRS "stb.h")

message(STATUS "Imgui ${imgui_SOURCE_DIR}")

message(STATUS "glslang ${glslang_LIB}")

file(GLOB_RECURSE SOURCE_FILES
    CONFIGURE_DEPENDS    
    "src/*.cpp"
    "vendor/stb_image/stb_image.cpp"
)
file(GLOB_RECURSE HEADER_FILES
    CONFIGURE_DEPENDS
    "src/*.h"
)

message(STATUS "Acorn src files ${SOURCE_FILES}")


add_library(Acorn SHARED
${SOURCE_FILES} ${HEADER_FILES}
)

target_precompile_headers(Acorn PRIVATE src/acpch.h)
set_target_properties(Acorn PROPERTIES UNITY_BUILD ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
target_compile_definitions(Acorn PRIVATE -DAC_ENABLE_ASSERTS -DPSNIP_DEBUG -DAC_PROFILE=0)
endif()

target_compile_definitions(Acorn PRIVATE -DGLFW_INCLUDE_NONE)

#${VULKAN_PATH}/Bin;
target_link_directories(Acorn PRIVATE "${VULKAN_PATH}/Lib;")

target_link_libraries(Acorn PUBLIC EnTT::EnTT)
target_link_libraries(Acorn PRIVATE glad::glad)
target_link_libraries(Acorn PUBLIC glfw ${OPENGL_gl_LIBRARY})
target_link_libraries(Acorn PUBLIC glm::glm)
target_link_libraries(Acorn PUBLIC spdlog::spdlog)
target_link_libraries(Acorn PUBLIC imgui::imgui)
target_link_libraries(Acorn PRIVATE implot::implot)
target_link_libraries(Acorn PRIVATE imguizmo::imguizmo)
target_link_libraries(Acorn PRIVATE yaml-cpp)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Linking vulkan debug libs ${VULKAN_DEBUG_LIBS}")
    target_link_libraries(Acorn PRIVATE ${VULKAN_DEBUG_LIBS})
else()
    target_link_libraries(Acorn PRIVATE ${VULKAN_RELEASE_LIBS})
endif()

target_include_directories(Acorn PRIVATE "${VULKAN_PATH}/Include")
target_include_directories(Acorn PRIVATE ${STB_INCLUDE_DIRS})
target_include_directories(Acorn PUBLIC src)
target_include_directories(Acorn PUBLIC ${PROJECT_BINARY_DIR})

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
export(TARGETS Acorn FILE Acorn.cmake)

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

include(GenerateExportHeader)
generate_export_header(Acorn)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET Acorn
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin/yaml-cppd.dll
            ${PROJECT_BINARY_DIR}/yaml-cpp.dll
    )
else()
    add_custom_command(TARGET Acorn
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/yaml-cpp.dll
            ${PROJECT_BINARY_DIR}/yaml-cpp.dll
    )
endif()