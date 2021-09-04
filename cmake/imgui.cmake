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
    imgui_internal.h
    imgui.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h

    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
)

#TODO add flags for backend sel

add_library(ImGui INTERFACE ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(ImGui INTERFACE 
    $<BUILD_INTERFACE:${IMGUI_SOURCE_DIR}>)

install(TARGETS ImGui
    CONFIGURATIONS Debug
    RUNTIME DESTINATION Debug/bin)
install(TARGETS ImGui
        CONFIGURATIONS Release
        RUNTIME DESTINATION Release/bin)