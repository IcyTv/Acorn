project(UltralightApp VERSION 0.0.1 LANGUAGES CXX)

include("cmake/Ultralight.cmake")

file(GLOB_RECURSE SOURCE_FILES
"src/*.cpp"
)
file(GLOB_RECURSE HEADER_FILES
"src/*.h"
)

message(STATUS "UltralightApp src files ${SOURCE_FILES}")

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:main")

add_executable(UltralightApp ${SOURCE_FILES} ${HEADER_FILES})

add_ultralight(UltralightApp)

# target_link_directories(UltralightApp PRIVATE "../vendor") #TODO fix
target_link_libraries(UltralightApp Acorn)
add_dependencies(UltralightApp Acorn)

find_package(imguizmo CONFIG REQUIRED)
target_link_libraries(UltralightApp imguizmo::imguizmo)

target_include_directories(UltralightApp PUBLIC 
    src
    "${CMAKE_SOURCE_DIR}/vendor/v8-v142-x64.9.3.345.3/include"
)

add_custom_command(TARGET UltralightApp
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
)
add_custom_command(TARGET UltralightApp
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/imgui.ini ${CMAKE_CURRENT_BINARY_DIR}/imgui.ini    
)

# set(LIBRARIES_TO_COPY "Acorn;brotlicommon;brotlidec;bz2d;fmtd;freetyped;glfw3;libpng16d;spdlogd;yaml-cppd;zlibd1")
set(LIBRARIES_TO_COPY "")
# set(DEBUG_SYMBOLS_TO_COPY "Acorn.lib;Acorn.pdb")
set(DEBUG_SYMBOLS_TO_COPY "")

if(WIN32)
    foreach(LIB ${LIBRARIES_TO_COPY})
        message(STATUS "Copying ${LIB}")
        add_custom_command(TARGET Ultralight
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/../Acorn/${LIB}.dll" "${CMAKE_CURRENT_BINARY_DIR}/${LIB.dll}"
        )
    endforeach()

    foreach(LIB ${DEBUG_SYMBOLS_TO_COPY})
        message(STATUS "Copying ${LIB}")
        add_custom_command(TARGET Ultralight
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/../Acorn/${LIB}" "${CMAKE_CURRENT_BINARY_DIR}/${LIB}"
        )
    endforeach()
endif()


target_compile_options(UltralightApp PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wpedantic -DV8_COMPRESS_POINTERS>)


set_property(TARGET UltralightApp PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

get_target_property(OUT UltralightApp LINK_LIBRARIES)
message(STATUS "UltralightApp Linked Libs: ${OUT}")

