#include "acpch.h"

#include "platform/windows/window/WindowsWindow.h"

#include "Acorn/events/ApplicationEvent.h"
#include "Acorn/events/KeyEvent.h"
#include "Acorn/events/MouseEvent.h"

#include "platform/opengl/OpenGLContext.h"

namespace Acorn
{
	static uint8_t s_GLFWWindowCount = 0;

	void GLFWErrorCallback(int errorCode, const char* message)
	{
		AC_CORE_ERROR("GLFW: [{0}] {1}", errorCode, message);
	}

	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		AC_PROFILE_FUNCTION();
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		AC_PROFILE_FUNCTION();

		Shutdown();
	}

	void WindowsWindow::OnUpdate()
	{
		AC_PROFILE_FUNCTION();

		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		AC_PROFILE_FUNCTION();

		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

	void WindowsWindow::Maximize()
	{
		AC_PROFILE_FUNCTION();

		glfwMaximizeWindow(m_Window);
		m_Data.Maximized = true;
	}

	void WindowsWindow::UnMaximize()
	{

		AC_PROFILE_FUNCTION();
		glfwRestoreWindow(m_Window);
		m_Data.Maximized = false;
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		AC_PROFILE_FUNCTION();

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.Maximized = props.Maximized;

		AC_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (s_GLFWWindowCount == 0)
		{
			int success = glfwInit();
			AC_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

#ifdef AC_DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

		if (m_Data.Maximized)
			glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		AC_CORE_ASSERT(m_Window, "Could not create window!");
		s_GLFWWindowCount++;

		// FIXME this should be a generic context
		m_Context = CreateScope<OpenGLContext>(m_Window);

		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// Setup GLFW Callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
								  {
									  WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

									  data.Width = width;
									  data.Height = height;

									  WindowResizeEvent event(width, height);
									  data.EventCallback(event); });

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
								   {
									   WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

									   WindowCloseEvent event;
									   data.EventCallback(event); });

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
						   {
							   WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

							   switch (action)
							   {
								   case GLFW_PRESS:
								   {
									   KeyPressedEvent event((KeyCode)(key), mods, 0);
									   data.EventCallback(event);
									   break;
								   }
								   case GLFW_RELEASE:
								   {
									   KeyReleasedEvent event((KeyCode)(key), mods);
									   data.EventCallback(event);
									   break;
								   }
								   case GLFW_REPEAT:
								   {
									   KeyPressedEvent event((KeyCode)(key), mods, 1);
									   data.EventCallback(event);
									   break;
								   }
							   } });

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, uint32_t keycode)
							{
								WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
								KeyTypedEvent event((KeyCode)(keycode), 0);
								data.EventCallback(event); });

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
								   {
									   WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

									   switch (action)
									   {
										   case GLFW_PRESS:
										   {
											   MouseButtonPressedEvent event(button);
											   data.EventCallback(event);
											   break;
										   }
										   case GLFW_RELEASE:
										   {
											   MouseButtonReleasedEvent event(button);
											   data.EventCallback(event);
											   break;
										   }
									   } });

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double offsetX, double offsetY)
							  {
								  WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

								  MouseScrolledEvent event((float)offsetX, (float)offsetY);

								  data.EventCallback(event); });

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double x, double y)
								 {
									 WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

									 MouseMovedEvent event(x, y);
									 data.EventCallback(event); });
	}

	void WindowsWindow::Shutdown()
	{
		AC_PROFILE_FUNCTION();

		// Delete the "context" before destroying the window, becuase we need the OpenGL context to be valid
		// In order to release the resources
		m_Context.reset();

		glfwDestroyWindow(m_Window);
		s_GLFWWindowCount--;

		if (s_GLFWWindowCount == 0)
		{

			AC_CORE_INFO("No windows, Shutting down GLFW!");
			glfwTerminate();
		}
	}
}