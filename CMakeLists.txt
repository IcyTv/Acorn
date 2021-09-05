cmake_minimum_required(VERSION 3.14)

include(cmake/CPM.cmake)
include(cmake/requirements.cmake)

cmake_policy(SET CMP0074 NEW)

set(CMAKE_CXX_STANDARD 20)
project(Acorn)

add_subdirectory(Acorn)
add_subdirectory(OakTree)

# add_custom_target(link_commands ALL
#     COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/
# )