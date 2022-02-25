project('Acorn', 'cpp', 
	version: '0.1.0', 
	license: 'MIT', 
	default_options: [
		'cpp_std=c++20',
		'default_library=static',
		'b_vscrt=static_from_buildtype'
	]
)

add_global_arguments('-DACORN_STATIC_DEFINE', language: 'cpp')

subdir('Acorn')
subdir('OakTree')
subdir('Acorn/tools/IDLParser')

gtest = subproject('gtest')

if get_option('enable-tests')
	subdir('tests')
endif

python = import('python').find_installation('python3')

# This regex excludes any sources from the third_party, tests, benchmarks, subprojects,
# gtest and vendor related files.
regex = '(Acorn.(src|include)|OakTree.src).*'
# This adds clang tidy support
configure_file(input : '.clang-tidy',
               output : '.clang-tidy',
               copy : true)
configure_file(input : 'bin/run-clang-tidy.py',
			   output : 'run-clang-tidy.py',
			   copy : true)

run_target('tidy',
	command: [python, 'run-clang-tidy.py', '-format', '-p=' + meson.build_root(), regex]
)