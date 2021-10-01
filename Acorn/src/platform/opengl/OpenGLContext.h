#pragma once

#include "Acorn/renderer/GraphicsContext.h"

#if AC_PROFILE
	#include "debug/FrameProfiler.h"
#endif

struct GLFWwindow;

namespace Acorn
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);
		~OpenGLContext() {}

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;

#if AC_PROFILE
		Ref<FrameProfiler> m_FrameProfiler;
#endif
	};
}