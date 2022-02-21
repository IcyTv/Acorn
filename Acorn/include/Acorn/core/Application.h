#pragma once

#include "core/Core.h"

#include "Window.h"
#include "events/ApplicationEvent.h"
#include "events/Event.h"
#include "gui/ImGuiLayer.h"
#include "layer/LayerStack.h"

#include "acorn_export.h"

namespace Acorn
{
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			AC_CORE_ASSERT(index < Count, "Commandline arguments index out of range");
			return Args[index];
		}
	};

	class Application
	{
	public:
		Application(const std::string& name = "Acorn App", ApplicationCommandLineArgs args = ApplicationCommandLineArgs(), bool maximized = false);
		virtual ~Application();

		void Run();
		void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; }

		void Close() { m_Running = false; }

		inline static Application& Get() { return *s_Instance; }

		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		inline const ApplicationCommandLineArgs& GetCommandLineArgs() const { return m_CommandLineArgs; }

	private:
		bool OnWindowClose(WindowCloseEvent& event);
		bool OnWindowResize(WindowResizeEvent& event);

	private:
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;

		float m_LastFrameTime = 0.0f;

		ApplicationCommandLineArgs m_CommandLineArgs;

	private:
		static Application* s_Instance;
	};

	// To be defined in Client
	Application* CreateApplication(ApplicationCommandLineArgs args);

}
