#TODO move to wrap
tracy_files = files(
	'tracy/TracyClient.cpp',
)
tracy_inc = include_directories('tracy')

tracy_lib = static_library('tracy', tracy_files, include_directories: tracy_inc, cpp_args: '-w')
tracy = declare_dependency(include_directories: tracy_inc, link_with: tracy_lib)

imguizmo_files = files(
	'ImGuizmo/ImGuizmo.cpp',
)
imguizmo_inc = include_directories('ImGuizmo')

imguizmo_lib = static_library('imguizmo', imguizmo_files, include_directories: imguizmo_inc, cpp_args: '-w', dependencies: [imgui])
imguizmo = declare_dependency(include_directories: imguizmo_inc, link_with: imguizmo_lib)