#include "acpch.h"

#include "WindowsPlatform.h"

#include <GLFW/glfw3.h>

namespace Acorn
{

	float WindowsPlatform::GetTimeImpl() const
	{
		return (float) glfwGetTime();
	}

	const char* WindowsPlatform::GetNameImpl() const
	{
		return "OpenGl/GLFW";
	}

}