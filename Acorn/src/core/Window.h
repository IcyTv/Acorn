#pragma once

#include "acpch.h"

#include "core/Core.h"
#include "events/Event.h"

namespace Acorn
{
	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;
		bool Maximized;

		WindowProps(const std::string& title = "Acorn",
					uint32_t width = 1920,
					uint32_t height = 1080,
					bool maximized = false)
			: Title(title), Width(width), Height(height), Maximized(maximized)
		{
		}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void Maximize() = 0;
		virtual void UnMaximize() = 0;

		virtual void* GetNativeWindow() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};
}