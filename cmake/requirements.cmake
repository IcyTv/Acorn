CPMAddPackage("gh:skypjack/entt@3.8.1")
CPMAddPackage(
  NAME yaml-cpp
  GITHUB_REPOSITORY jbeder/yaml-cpp
  VERSION 0.70
  GIT_TAG 0579ae3d976091d7d664aa9d2527e0d0cff25763
  OPTIONS
    "YAML_CPP_BUILD_TESTS Off"
    "YAML_CPP_BUILD_CONTRIB Off"
    "YAML_CPP_BUILD_TOOLS Off"
)
CPMAddPackage(
    NAME GLFW
    GITHUB_REPOSITORY glfw/glfw
    GIT_TAG 3.3.4
    OPTIONS
        "GLFW_BUILD_TESTS OFF"
        "GLFW_BUILD_EXAMPLES OFF"
        "GLFW_BULID_DOCS OFF"
)
CPMAddPackage(
    NAME glm
    GITHUB_REPOSITORY g-truc/glm
    VERSION 0.9.9.8
    GIT_TAG 0.9.9.8
)
CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    VERSION 1.9.2
    OPTIONS
        "SPDLOG_ENABLE_PCH ON"
)
CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG docking
    VERSION docking
    OPTIONS
        "INCLUDE_DIR ."
)
CPMAddPackage("gh:nothings/stb#master")
CPMAddPackage("gh:nemequ/portable-snippets#master")
