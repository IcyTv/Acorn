inc = include_directories('src')

sources = files(
	'src/GenericLexer.cpp',
	'src/V8IDLGenerators.cpp',
	'src/IDLParser.cpp',
	'src/Utf16View.cpp',
	'src/main.cpp',
)

args = dependency('args')

idl_parser = executable('IDLParser', sources, include_directories: inc, dependencies: [args, boost])