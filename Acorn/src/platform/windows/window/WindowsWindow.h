#pragma once

#include "core/Window.h"

#include "renderer/GraphicsContext.h"
#include <GLFW/glfw3.h>

namespace Acorn
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline uint32_t GetWidth() const override { return m_Data.Width; }
		inline uint32_t GetHeight() const override { return m_Data.Height; }

		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		virtual void Maximize() override;
		virtual void UnMaximize() override;

		inline virtual void* GetNativeWindow() const override { return (void*)m_Window; }

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		GLFWwindow* m_Window;
		Scope<GraphicsContext> m_Context;

		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool VSync;
			bool Maximized;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};
}
