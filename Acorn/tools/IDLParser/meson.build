inc = include_directories('src')

# TODO I don't know if I actually want to handle templates this way...
# Maybe in the future we read tpl files from the disk, or we write them directly in the header file...

tpl_files = files(
	'src/EnumHeader.tpl',
	'src/WrapperHeader.tpl',
	'src/WrapperImplementation.tpl'
)

python = import('python').find_installation('python3')

foreach file : tpl_files
	run_command(python, ['tpl_generator.py', file])
endforeach

sources = files(
	# 'src/InjaRenderer.cpp',
	'src/GenericLexer.cpp',
	'src/V8IDLGenerators.cpp',
	'src/SourceGenerator.cpp',
	'src/IDLParser.cpp',
	'src/Utf16View.cpp',
	'src/main.cpp',
)

args = dependency('args')
inja = dependency('inja')

idl_parser = executable('IDLParser', sources, include_directories: inc, dependencies: [args, boost, inja])