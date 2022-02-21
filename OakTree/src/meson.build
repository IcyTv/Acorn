oak_sources = files(
	'panels/ContentBrowser.cpp',
	'panels/LogPanel.cpp',
	'panels/SceneHierarchy.cpp',
	# 'panels/ShapeEditor.cpp',
	'OakLayer.cpp',
	'OakTree.cpp'
)

oaktree = executable(
	'OakTree',
	oak_sources,
	include_directories: oak_inc,
	dependencies: oak_dependencies,
	install: true,
	install_mode: 'rwx--x--x',
)

# install_symlink('imgui.ini')

# TODO symlink imgui.ini and res
