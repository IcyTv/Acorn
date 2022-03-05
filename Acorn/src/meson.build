
sources = files(
	'Acorn/core/Application.cpp',
	'Acorn/core/Log.cpp',
	'Acorn/core/Platform.cpp',
	'Acorn/core/Timestep.cpp',
	'Acorn/core/UUID.cpp',
	'Acorn/debug/FrameProfiler.cpp',
	'Acorn/ecs/components/Components.cpp',
	'Acorn/ecs/components/Rigidbody.cpp',
	'Acorn/ecs/components/SceneCamera.cpp',
	'Acorn/ecs/Entity.cpp',
	'Acorn/ecs/Scene.cpp',
	'Acorn/gui/GUITools.cpp',
	'Acorn/gui/ImGuiBuild.cpp',
	'Acorn/gui/ImGuiLayer.cpp',
	'Acorn/layer/Layer.cpp',
	'Acorn/layer/LayerStack.cpp',
	'Acorn/math/Math.cpp',
	'Acorn/physics/Collider.cpp',
	'Acorn/renderer/2d/Renderer2D.cpp',
	'Acorn/renderer/2d/SubTexture2d.cpp',
	'Acorn/renderer/Buffer.cpp',
	'Acorn/renderer/Camera.cpp',
	'Acorn/renderer/DebugRenderer.cpp',
	'Acorn/renderer/EditorCamera.cpp',
	'Acorn/renderer/Framebuffer.cpp',
	'Acorn/renderer/RenderCommand.cpp',
	'Acorn/renderer/Renderer.cpp',
	'Acorn/renderer/RendererApi.cpp',
	'Acorn/renderer/Shader.cpp',
	'Acorn/renderer/Texture.cpp',
	'Acorn/renderer/UniformBuffer.cpp',
	'Acorn/renderer/VertexArray.cpp',
	'Acorn/serialize/Serializer.cpp',
	'Acorn/templates/OrthographicCameraController.cpp',
	'Acorn/utils/FileUtils.cpp',
	'Acorn/utils/MathUtils.cpp',
	'Acorn/utils/md5.cpp',
	'Acorn/utils/PlatformCapabilities.cpp',
	'platform/opengl/OpenGLBuffer.cpp',
	'platform/opengl/OpenGLContext.cpp',
	'platform/opengl/OpenGLFrameBuffer.cpp',
	'platform/opengl/OpenGLFrameProfiler.cpp',
	'platform/opengl/OpenGLPlatformCapabilities.cpp',
	'platform/opengl/OpenGLRendererApi.cpp',
	'platform/opengl/OpenGLShader.cpp',
	'platform/opengl/OpenGLTexture.cpp',
	'platform/opengl/OpenGLUniformBuffer.cpp',
	'platform/opengl/OpenGLVertexArray.cpp',
	'platform/glfw/input/GLFWInput.cpp',
	'platform/glfw/window/GLFWWindow.cpp',
	'platform/glfw/GLFWPlatform.cpp',
	'platform/glfw/GLFWPlatformUtils.cpp'
)

if v8.found()
	sources += [
		'Acorn/ecs/components/V8Script_internals.cpp',
		'Acorn/ecs/components/V8Script.cpp',
		'Acorn/utils/v8/V8Import.cpp',
		'Acorn/ecs/components/TSCompiler.cpp',
	]
endif

idl_files = files(
    'Acorn/scripting/v8/Vec2.idl',
    'Acorn/scripting/v8/Vec3.idl',
)

fs = import('fs')

foreach idl_file: idl_files
    # run_command(idl_parser, tpl_file, '-I', '-H')
    message('Generating bindings for @0@'.format(idl_file))
    new_file = configure_file(input: idl_file, output: fs.name(idl_file), copy: true)
    message('Generated bindings for @0@'.format(new_file))
    generated = idl_generator.process(new_file)
    sources += generated
endforeach

libacorn = library('acorn', 
	sources, 
	include_directories: inc,
	cpp_pch: 'pch/acpch.h',
	dependencies: dependencies,
	install: true,
	cpp_args: compile_args,
	implicit_include_directories: true
)