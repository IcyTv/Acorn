project(OakTree VERSION 0.0.1 LANGUAGES CXX)

file(GLOB_RECURSE SOURCE_FILES
"src/*.cpp"
)
file(GLOB_RECURSE HEADER_FILES
"src/*.h"
)

message(STATUS "OakTree src files ${SOURCE_FILES}")

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:main")

add_executable(OakTree ${SOURCE_FILES} ${HEADER_FILES})

target_link_directories(OakTree PRIVATE "../vendor") #TODO fix
target_link_libraries(OakTree Acorn)
add_dependencies(OakTree Acorn)

find_package(imguizmo CONFIG REQUIRED)
target_link_libraries(OakTree imguizmo::imguizmo)

target_include_directories(OakTree PUBLIC 
    src
    "${CMAKE_SOURCE_DIR}/vendor/v8-v142-x64.9.3.345.3/include"
)

add_custom_command(TARGET OakTree
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res
)
add_custom_command(TARGET OakTree
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
        add_custom_command(TARGET OakTree
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/../Acorn/${LIB}.dll" "${CMAKE_CURRENT_BINARY_DIR}/${LIB.dll}"
        )
    endforeach()

    foreach(LIB ${DEBUG_SYMBOLS_TO_COPY})
        message(STATUS "Copying ${LIB}")
        add_custom_command(TARGET OakTree
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/../Acorn/${LIB}" "${CMAKE_CURRENT_BINARY_DIR}/${LIB}"
        )
    endforeach()
endif()


target_compile_options(OakTree PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wpedantic -DV8_COMPRESS_POINTERS>)

# target_link_directories(OakTree PUBLIC "${CMAKE_SOURCE_DIR}/../v8/v8/out.gn/x64.debug/obj")
# target_link_libraries(OakTree v8_monolith)

add_custom_command(TARGET OakTree
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/bin/snapshot_blob.bin" "${CMAKE_CURRENT_BINARY_DIR}/snapshot_blob.bin"
)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # set(V8_LIB_PATH "${CMAKE_SOURCE_DIR}/vendor/v8.redist-v142-x64.9.3.345.3/lib/Debug")
    # target_link_directories(Acorn PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/v8-v140-x64.7.4.288.11/lib/Debug")
else()
    # set(V8_LIB_PATH "${CMAKE_SOURCE_DIR}/vendor/v8.redist-v142-x64.9.3.345.3/lib/Release")
    # target_link_directories(Acorn PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/v8-v140-x64.7.4.288.11/lib/Release")
endif()

# set(V8_LIB_PATH "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/bin/")
# set(V8_LIBS "icudt69.dll;icuin69.dll;icuio69.dll;icutu69.dll;icuuc69.dll;v8_libbase.dll;v8_libplatform.dll;v8.dll;zlib1.dll")

# foreach(LIB ${V8_LIBS})
#     message(STATUS "Copying ${LIB}")
#     add_custom_command(TARGET OakTree
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different "${V8_LIB_PATH}/${LIB}" "${CMAKE_CURRENT_BINARY_DIR}/${LIB}"
#     )
# endforeach()
# # TODO other platforms
# target_include_directories(OakTree PUBLIC $<TARGET_PROPERTY:Acorn,INCLUDE_DIRECTORIES>)

# add_custom_command(TARGET OakTree
#     POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Acorn> "${CMAKE_CURRENT_BINARY_DIR}"
#     COMMENT "Copy dll file to ${CMAKE_CURRENT_BINARY_DIR} directory" VERBATIM
# )

# install(
#     TARGETS OakTree
#     EXPORT OakTree
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )
