cmake_minimum_required(VERSION 3.11)

cmake_policy(SET CMP0074 NEW)

set(CMAKE_VS_MSBUILD_COMMAND "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/MSBuild/Current/Bin/MSBuild.exe")

set(CMAKE_CXX_STANDARD 20)
# add_compile_options(
#        -Wall -Werror -Wno-error=maybe-uninitialized
#        $<$<CONFIG:RELEASE>:-Ofast>
#        $<$<CONFIG:DEBUG>-O0>
#        $<$<CONFIG:DEBUG>-ggdb3>
# )
# add_compile_definitions(
#         $<$<CONFIG:RELEASE>:NDEBUG>
#         $<$<CONFIG:RELEASE>:BOOST_DISABLE_ASSERTS>
# )

include("cmake/Hunter/HunterGate.cmake")

option(HUNTER_STATUS_DEBUG "Print a lot of info" ON) # Good
set(HUNTER_JOBS_NUMBER 6 CACHE STRING "Hunter jobs number")

HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.314.tar.gz"
    SHA1 "95c47c92f68edb091b5d6d18924baabe02a6962a"
    LOCAL
)

project(Acorn)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(Acorn)

# add_custom_target(link_commands ALL
#     COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/
# )