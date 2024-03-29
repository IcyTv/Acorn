
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

headers = files(
	'Acorn/core/Application.h',
	'Acorn/core/Core.h',
	'Acorn/core/CoreConfig.h',
	'Acorn/core/EntryPoint.h',
	'Acorn/core/Log.h',
	'Acorn/core/Platform.h',
	'Acorn/core/Timestep.h',
	'Acorn/core/UUID.h',
	'Acorn/core/Window.h',
	'Acorn/debug/FrameProfiler.h',
	'Acorn/debug/Instrumentor.h',
	'Acorn/debug/Timer.h',
	'Acorn/ecs/components/Components.h',
	'Acorn/ecs/components/SceneCamera.h',
	'Acorn/ecs/components/ScriptableEntity.h',
	'Acorn/ecs/components/TSCompiler.h',
	'Acorn/ecs/components/V8Script_internals.h',
	'Acorn/ecs/components/V8Script.h',
	'Acorn/ecs/Entity.h',
	'Acorn/ecs/Scene.h',
	'Acorn/events/ApplicationEvent.h',
	'Acorn/events/Event.h',
	'Acorn/events/KeyEvent.h',
	'Acorn/events/MouseEvent.h',
	'Acorn/gui/ImGuiLayer.h',
	'Acorn/gui/GUITools.h',
	'Acorn/input/Input.h',
	'Acorn/input/KeyCodes.h',
	'Acorn/input/MouseButtonCodes.h',
	'Acorn/layer/Layer.h',
	'Acorn/layer/LayerStack.h',
	'Acorn/math/Math.h',
	'Acorn/physics/Collider.h',
	'Acorn/renderer/2d/Renderer2D.h',
	'Acorn/renderer/2d/SubTexture2d.h',
	'Acorn/renderer/BatchRenderer.h',
	'Acorn/renderer/Buffer.h',
	'Acorn/renderer/Camera.h',
	'Acorn/renderer/DebugRenderer.h',
	'Acorn/renderer/EditorCamera.h',
	'Acorn/renderer/Framebuffer.h',
	'Acorn/renderer/GraphicsContext.h',
	'Acorn/renderer/RenderCommand.h',
	'Acorn/renderer/Renderer.h',
	'Acorn/renderer/RendererApi.h',
	'Acorn/renderer/Shader.h',
	'Acorn/renderer/Texture.h',
	'Acorn/renderer/UniformBuffer.h',
	'Acorn/renderer/VertexArray.h',
	'Acorn/serialize/Serializer.h',
	'Acorn/templates/OrthographicCameraController.h',
	'Acorn/utils/fonts/IconsFontAwesome4.h',
	'Acorn/utils/v8/V8Import.h',
	'Acorn/utils/FileUtils.h',
	'Acorn/utils/FixedQueue.h',
	'Acorn/utils/MathUtils.h',
	'Acorn/utils/PlatformCapabilities.h',
	'Acorn/utils/PlatformUtils.h',
	'Acorn/utils/ThreadSafeQueue.h',
	'platform/opengl/OpenGLBuffer.h',
	'platform/opengl/OpenGLContext.h',
	'platform/opengl/OpenGLFrameBuffer.h',
	'platform/opengl/OpenGLFrameProfiler.h',
	'platform/opengl/OpenGLPlatformCapabilities.h',
	'platform/opengl/OpenGLRendererApi.h',
	'platform/opengl/OpenGLShader.h',
	'platform/opengl/OpenGLTexture.h',
	'platform/opengl/OpenGLUniformBuffer.h',
	'platform/opengl/OpenGLVertexArray.h',
	'platform/glfw/window/GLFWWindow.h',
	'platform/glfw/GLFWPlatform.h',
	'acorn_export.h',
	'Acorn.h'
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
    'Acorn/scripting/v8/Transform.idl',
    'Acorn/scripting/v8/Global.idl',
)

fs = import('fs')

configure_file(
	input: './mappingData.json',
	output: 'mappingData.json',
	copy: true
)

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

# TODO filter out headers!
install_subdir('.', install_dir: 'include', strip_directory: true)