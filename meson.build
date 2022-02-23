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