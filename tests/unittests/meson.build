unittests_sources = files(
	'layer/LayerStack.cpp'
)

unittests = executable('unittests',
	[unittests_sources, test_main],
	include_directories: inc,
	dependencies: test_deps + [libacorn_dep]
)

test('Acorn Tests', unittests, args: ['--gtest_color=yes']) #TODO base on bColor(?) builtin

project_test_sources += unittests_sources