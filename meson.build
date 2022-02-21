project('Acorn', 'cpp', 
	version: '0.1.0', 
	license: 'MIT', 
	default_options: [
		'cpp_std=c++20',
		'default_library=static'
	])


compiler = meson.get_compiler('cpp')
if compiler.get_id() == 'clang'
	add_global_arguments('-fcolor-diagnostics', language: 'cpp')
elif compiler.get_id() == 'gcc'
	add_global_arguments('-fdiagnostics-color=always', language: 'cpp')
endif

subdir('Acorn')
subdir('OakTree')