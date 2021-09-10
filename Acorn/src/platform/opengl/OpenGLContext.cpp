#include "OpenGLContext.h"
#include "acpch.h"

#include "Acorn/core/Core.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

void gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			AC_CORE_ASSERT(false, message);
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			AC_CORE_WARN("[GL]: {0}", message);
			break;
		case GL_DEBUG_SEVERITY_LOW:
			AC_CORE_INFO("[GL]: {0}", message);
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			//AC_CORE_TRACE("[GL]: {0}", message);
			break;
		default:
			AC_CORE_FATAL("Unknwon message type!");
			AC_CORE_ASSERT(false, "[GL]: {0}", message);
			break;
	}
}

namespace Acorn
{

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		AC_CORE_ASSERT(windowHandle, "Window Handle for OpenGLContext is null!");
	}

	void OpenGLContext::Init()
	{
		AC_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		AC_CORE_ASSERT(status, "Failed to Load Glad");

#ifdef AC_DEBUG
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(gl_error_callback, NULL);
#endif

		AC_CORE_INFO("OpenGL Info");
		AC_CORE_INFO("\tVendor: {0}", glGetString(GL_VENDOR));
		AC_CORE_INFO("\tRenderer: {0}", glGetString(GL_RENDERER));
		AC_CORE_INFO("\tVersion: {0}", glGetString(GL_VERSION));

#ifdef AC_ENABLE_ASSERTS
		int majorVersion, minorVersion;
		glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

		AC_CORE_ASSERT(majorVersion > 4 || (majorVersion == 4 && minorVersion >= 5), "Acorn requires OpenGL 4.5 or above");
#endif
	}

	void OpenGLContext::SwapBuffers()
	{
		AC_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

}