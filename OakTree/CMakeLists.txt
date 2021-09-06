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

target_link_libraries(OakTree PRIVATE Acorn)

target_include_directories(OakTree PUBLIC src)
target_include_directories(OakTree PUBLIC $<TARGET_PROPERTY:Acorn,INCLUDE_DIRECTORIES>)