#include "acpch.h"

#include "Application.h"
#include "renderer/Renderer.h"
#include "Timestep.h"
#include "core/Platform.h"
#include "input/KeyCodes.h"

#include <chrono>
#include <iomanip>

namespace Acorn
{
	Application::Application(const std::string& name)
	{
		AC_PROFILE_FUNCTION();
		AC_CORE_ASSERT(!s_Instance, "Can only have one Application");
		s_Instance = this;

		WindowProps props(name);
		m_Window = Scope<Window>(Window::Create(props));
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();

		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		AC_PROFILE_FUNCTION();
	}

	void Application::Run()
	{
		AC_PROFILE_FUNCTION();

		while (m_Running)
		{
			AC_PROFILE_SCOPE("Application::Run::Runloop");

			float time = Platform::GetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				{
					AC_PROFILE_SCOPE("Application::Run::OnUpdate")
					for (Layer *layer : m_LayerStack)
						layer->OnUpdate(timestep);
				}
			}

			{

				AC_PROFILE_SCOPE("Application::Run::ImGui")
					m_ImGuiLayer->Begin();

				{
					AC_PROFILE_SCOPE("Application::Run::OnImGuiRender")
						for (Layer* layer : m_LayerStack)
							layer->OnImGuiRender(timestep);
				}

				m_ImGuiLayer->End();
			}

			{
				AC_PROFILE_SCOPE("Application::Run::WindowUpdate")
				m_Window->OnUpdate();
			}
		}
	}

	void Application::OnEvent(Event &e)
	{
		AC_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

		if (e.GetEventType() == EventType::KeyPressed)
		{
			KeyPressedEvent &ke = (KeyPressedEvent &)e;
			if (ke.GetKeyCode() == AC_KEY_F6)
			{
				if (!m_IsProfiling && ke.GetRepeatCount() == 0)
				{
					m_IsProfiling = true;
					auto timestamp = std::chrono::system_clock::now();

					std::time_t now_tt = std::chrono::system_clock::to_time_t(timestamp);
					std::tm tm = *std::localtime(&now_tt);

					std::stringstream nameStream;
					nameStream << "AcornProfiling-Runtime-" << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
					std::string name = nameStream.str();
					nameStream << ".json";
					std::string filename = nameStream.str();

					AC_CORE_TRACE("Starting Profiling {}", name);
					AC_PROFILE_BEGIN_SESSION(name, filename);
				}
				else if (m_IsProfiling && ke.GetRepeatCount() == 0)
				{
					m_IsProfiling = false;
					AC_CORE_TRACE("Ended Profiling");
					AC_PROFILE_END_SESSION();
				}
			}
		}

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent &event)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent &event)
	{
		AC_PROFILE_FUNCTION();

		if (event.GetWidth() == 0 || event.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;

		Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());

		return false;
	}

	void Application::PushLayer(Layer *layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer *overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

}