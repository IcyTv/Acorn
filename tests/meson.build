assert(not get_option('profiling'), 'Cannot build tests with profiling enabled!')

all_test_deps = []
all_test_dep_libs = []
project_test_sources = []

test_main = files('test_main.cpp')

gtest_dep = gtest.get_variable('gtest_dep')

test_deps = [
	gtest_dep,
]

subdir('unittests')


project_test_sources += test_main
all_test_deps += test_deps
# all_test_dep_libs += test_dep_libs

all_testes = executable('all_tests', 
    project_test_sources, 
    include_directories : inc, 
    dependencies : all_test_deps, 
    link_with : all_test_dep_libs,
)
