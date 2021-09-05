cmake_minimum_required(VERSION 3.0)

project(ImGui)
set(IMGUI_VERSION "github-docking")


include(GNUInstallDirs)
set(IMGUI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCE_FILES
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui.cpp

    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
)

set(HEADER_FILES
    imconfig.h
    imgui_internal.h
    imgui.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    backends/imgui_impl_glfw.h
    backends/imgui_impl_opengl3.h
)

message(STATUS "IM source dir ${IMGUI_SOURCE_DIR}")

add_library(ImGui ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(ImGui PUBLIC ${IMGUI_SOURCE_DIR})
set_target_properties(ImGui PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")

install(TARGETS ImGui LIBRARY DESTINATION lib)
foreach(file ${HEADER_FILES})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION include/${dir})
endforeach()

# install(FILES ${HEADER_FILES} DESTINATION include COMPONENT header)

# install(TARGETS ImGui
#         CONFIGURATIONS Release
#         RUNTIME DESTINATION bin)