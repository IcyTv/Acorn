#pragma once

#include "Window.h"
#include "events/ApplicationEvent.h"
#include "events/Event.h"
#include "gui/ImGuiLayer.h"
#include "layer/LayerStack.h"

namespace Acorn
{
	class Application
	{
	public:
		Application(const std::string& name = "Acorn App", bool maximized = false);
		virtual ~Application();

		void Run();
		void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline Window& GetWindow() { return *m_Window; }

		void Close() { m_Running = false; }

		inline static Application& Get() { return *s_Instance; }

		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

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

		bool m_IsProfiling = false;

	private:
		inline static Application* s_Instance = nullptr;
	};

	// To be defined in Client
	Application* CreateApplication();

}
