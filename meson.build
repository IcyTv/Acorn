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

if get_option('enable-tests')
	add_global_arguments('-DAC_TEST=1', language: 'cpp')
endif

if get_option('profiling')
	# compile_args += ',-DAC_PROFILE=1'
	add_project_arguments('-DAC_PROFILE=1', language: 'cpp')
	# compile_args += ',-DENABLE_TRACY'
	add_project_arguments('-DTRACY_ENABLE', language: 'cpp')
endif
if get_option('debug')
	# compile_args += ',-DAC_DEBUG=1'
	add_project_arguments('-DAC_DEBUG=1', language: 'cpp')
	# compile_args += ',-DAC_ENABLE_ASSERTS'
	add_project_arguments('-DAC_ENABLE_ASSERTS', language: 'cpp')
endif

subdir('Acorn/tools/IDLParser')
subdir('Acorn')
subdir('OakTree')

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