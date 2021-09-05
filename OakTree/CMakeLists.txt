project(OakTree)
file(GLOB_RECURSE SOURCE_FILES
"src/**/*.cpp"
# "vendor/stb_image/stb_image.cpp"
)
file(GLOB_RECURSE HEADER_FILES
"src/**/*.h"
)

message(STATUS "OakTree")

add_executable(OakTree ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(OakTree PUBLIC Acorn)

target_include_directories(OakTree PUBLIC 
    src
    $<TARGET_PROPERTY:Acorn,INCLUDE_DIRECTORIES>
)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

get_target_property(OUT Acorn LINK_LIBRARIES)
message(STATUS "Links ${OUT}")