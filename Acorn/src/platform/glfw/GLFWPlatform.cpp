#include "acpch.h"

#include "platform/glfw/GLFWPlatform.h"

#include <GLFW/glfw3.h>

namespace Acorn
{

	float GLFWPlatform::GetTimeImpl() const
	{
		return (float)glfwGetTime();
	}

	const char* GLFWPlatform::GetNameImpl() const
	{
		return "OpenGl/GLFW";
	}

	void* GLFWPlatform::GetCurrentContextImpl()
	{
		return (void*)glfwGetCurrentContext();
	}

}