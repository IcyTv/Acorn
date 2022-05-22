#pragma once

#include "Acorn/core/Window.h"

#include "Acorn/renderer/GraphicsContext.h"
#include <GLFW/glfw3.h>

namespace Acorn
{
	class GLFWWindow : public Window
	{
	public:
		GLFWWindow(const WindowProps& props);
		virtual ~GLFWWindow();

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
