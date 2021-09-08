project(OakTree)

file(GLOB_RECURSE SOURCE_FILES
"src/*.cpp"
)
file(GLOB_RECURSE HEADER_FILES
"src/*.h"
)

message(STATUS "OakTree src files ${SOURCE_FILES}")

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:main")

add_executable(OakTree ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(OakTree PRIVATE Acorn)

find_package(imguizmo CONFIG REQUIRED)
target_link_libraries(OakTree PRIVATE imguizmo::imguizmo)

target_include_directories(OakTree PUBLIC src)
target_include_directories(OakTree PUBLIC $<TARGET_PROPERTY:Acorn,INCLUDE_DIRECTORIES>)

add_custom_command(TARGET OakTree
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Acorn> "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Copy dll file to ${CMAKE_CURRENT_BINARY_DIR} directory" VERBATIM
)

install(
    TARGETS OakTree
    EXPORT OakTree
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

#FIXME temporary hack
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET OakTree POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin/yaml-cppd.dll
            ${PROJECT_BINARY_DIR}/yaml-cpp.dll
    )
else()
    add_custom_command(TARGET OakTree POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin/yaml-cpp.dll
            ${PROJECT_BINARY_DIR}/yaml-cpp.dll
    )
endif()