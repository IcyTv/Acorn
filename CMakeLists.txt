cmake_minimum_required(VERSION 3.11)

cmake_policy(SET CMP0074 NEW)

set(CMAKE_VS_MSBUILD_COMMAND "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/MSBuild/Current/Bin/MSBuild.exe")

set(CMAKE_CXX_STANDARD 20)
project(Acorn)

add_subdirectory(Acorn)

# add_custom_target(link_commands ALL
#     COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/
# )