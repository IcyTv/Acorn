
compiler = meson.get_compiler('cpp')

compile_args = ''
compile_args += '-DAcorn_EXPORTS=1'
if get_option('default_library') == 'static'
	compile_args += ',-DACORN_STATIC_DEFINE=1'
endif

message('compile_args: ' + compile_args)

cmake = import('cmake')

# 'This is a system dependecy'
threads = dependency('threads')

# 'Adding v8 first, because it is the most expensive'
v8 = dependency('v8', required: get_option('v8'), fallback: ['v8', 'v8_dep'])

if not v8.found()
	add_project_arguments('-DNO_SCRIPTING', language: 'cpp')
endif

glm = dependency('glm')
opengl = dependency('gl')
glfw = dependency('glfw3', fallback: ['glfw3', 'glfw_dep'])
glad = dependency('glad')
spdlog = dependency('spdlog')
# fmt = dependency('fmt_inc')
fmt_proj = subproject('fmt')
fmt_inc = fmt_proj.get_variable('fmt_inc')
fmt = declare_dependency(include_directories: fmt_inc)

yaml_cpp = dependency('yaml-cpp')
box2d = dependency('box2d', default_options: ['default_library=static'])
boost = dependency('boost')
imgui = dependency('imgui', default_options: ['opengl=enabled', 'glfw=enabled', 'default_library=static'])
magic_enum = dependency('magic_enum')
openssl = dependency('openssl')
json = dependency('nlohmann_json')

spirv_cross = dependency('SPIRV-Cross', fallback: ['SPIRV-Cross', 'spirv_cross_dep'])
spirv_headers_dep = dependency('SPIRV-Headers', fallback: ['SPIRV-Headers', 'spirv_headers_dep'])
spvtools_dep = dependency('SPIRV-Tools', default_options: ['default_library=static'])
spvtools_opt_dep = dependency('SPIRV-Tools-opt', fallback: ['SPIRV-Tools', 'spirv_tools_opt_dep'], default_options: ['default_library=static'])

glsllang_dep = dependency('glsllang', fallback: ['glslang', 'glslang_dep'], default_options: ['enable_hlsl=enabled', 'enable_opt=enabled'])


shaderc = dependency('shaderc', static : false, fallback : ['shaderc', 'shaderc_dep'], default_options: ['enable_nv_exts=disabled'])

stb_proj = subproject('stb_image')
stb_image_lib = stb_proj.get_variable('stb_image_lib')
stb_image_include = stb_proj.get_variable('stb_image_include')
stb_image = declare_dependency(include_directories: stb_image_include, link_with: stb_image_lib)


entt_subproject = cmake.subproject('entt')
entt = entt_subproject.dependency('EnTT')

subdir('vendor')

inc = include_directories('src', 'src/Acorn', 'src/pch')

dependencies = [
	boost,
	box2d,
	entt,
	fmt,
	glad,
	glfw,
	glm,
	imgui,
	imguizmo,
	json,
	magic_enum,
	opengl,
	openssl,
	pfd,
	shaderc,
	spdlog,
	spirv_cross,
	spvtools_opt_dep,
	stb_image,
	threads,
	tracy,
	yaml_cpp,
]

if v8.found()
	dependencies += [
		v8pp,
		v8,
	]
endif

if host_machine.system() == 'windows'
	dependencies += [
		compiler.find_library('winmm'),
		compiler.find_library('dbghelp'),
		compiler.find_library('shlwapi'),
		compiler.find_library('bcrypt'),
	]
endif

subdir('src')

libacorn_dep = declare_dependency(include_directories: inc, link_with: libacorn, dependencies: dependencies)