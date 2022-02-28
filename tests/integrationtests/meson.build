integration_tests_sources = files(
	'src/BasicLayer.cpp',
	'src/BasicApplication.cpp'
)

integration_tests = executable('integration_tests',
	[integration_tests_sources, test_main],
	include_directories: include_directories('src'),
	dependencies: test_deps + [libacorn_dep]
)

test('Acorn Tests', integration_tests, args: ['--gtest_color=yes']) #TODO base on bColor(?) builtin

project_test_sources += integration_tests_sources