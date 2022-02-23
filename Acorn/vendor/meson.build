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

v8pp_files = files(
	# 'v8pp/v8pp/class.cpp',
	# 'v8pp/v8pp/context.cpp',
	# 'v8pp/v8pp/version.cpp',
)
v8pp_inc = include_directories('v8pp')

# v8pp_lib = static_library('v8pp', include_directories: v8pp_inc, cpp_args: '-w', dependencies: [v8])
v8pp = declare_dependency(include_directories: v8pp_inc)

# Portable File Dialogs (Header Only)
pfd = declare_dependency(include_directories: include_directories('pfd'))
