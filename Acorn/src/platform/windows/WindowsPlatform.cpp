#include "acpch.h"

#include "platform/windows/WindowsPlatform.h"

#include <GLFW/glfw3.h>

namespace Acorn
{

	float WindowsPlatform::GetTimeImpl() const
	{
		return (float)glfwGetTime();
	}

	const char* WindowsPlatform::GetNameImpl() const
	{
		return "OpenGl/GLFW";
	}

	void* WindowsPlatform::GetCurrentContextImpl()
	{
		return (void*)glfwGetCurrentContext();
	}

}