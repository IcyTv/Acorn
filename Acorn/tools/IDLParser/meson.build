inc = include_directories('src')

sources = files(
	'src/GenericLexer.cpp',
	'src/V8IDLGenerators.cpp',
	'src/SourceGenerator.cpp',
	'src/IDLParser.cpp',
	'src/Utf16View.cpp',
	'src/main.cpp',
)

tpl_files = files(
    'src/WrapperHeader.tpl',
    'src/WrapperImplementation.tpl',
    'src/Enum.tpl',
)

boost = dependency('boost')
args = dependency('args')
inja = dependency('inja')
fmt = dependency('fmt')
magic_enum = dependency('magic_enum')

fs = import('fs')

configured_templates = []

foreach tpl_file: tpl_files
    configured_templates += configure_file(
        input: tpl_file,
        output: fs.name(tpl_file),
        copy: true
    )
endforeach

idl_parser = executable('IDLParser',
    sources,
    include_directories: inc,
    dependencies: [args, inja, fmt, boost, magic_enum],
    link_depends: configured_templates,
)

idl_generator = generator(idl_parser,
    output: ['@BASENAME@Wrapper.h', '@BASENAME@Wrapper.cpp'],
    arguments: ['-I', '-H', '@INPUT@', '-vvv', '-o', '@OUTPUT0@', '-o', '@OUTPUT1@'],
    depends: idl_parser,
)