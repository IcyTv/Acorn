#include "acpch.h"
#include "ImGuiLayer.h"
#include "core/Application.h"
#include "input/KeyCodes.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "implot.h"

#include "gui/terminal/Terminal.h"
#include "renderer/RenderCommand.h"

//Temporary
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Acorn
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach()
	{
		AC_PROFILE_FUNCTION();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImPlot::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

	}

	void ImGuiLayer::OnDetach()
	{
		AC_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImPlot::DestroyContext();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::Begin()
	{
		AC_PROFILE_FUNCTION();

		const static std::string imguiGroup = "ImGui Render";

#ifdef AC_DEBUG
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, (int)imguiGroup.size(), imguiGroup.c_str());
#endif

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		AC_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

#ifdef AC_DEBUG
		glPopDebugGroup();
#endif
	}

	void ImGuiLayer::OnImGuiRender(Timestep timestep)
	{
	/*	AC_PROFILE_FUNCTION();

		m_FrameTimes[m_CurrentFrame++] = timestep * 1000.0f;
		m_FrameSum += timestep * 1000.0f;

		if (m_ShowingFrameGraph) {
			ImGui::Begin("Timing", &m_ShowingFrameGraph);
			ImGui::Text("Average Frame Time %.2f ms", m_AverageFrameTime);
			ImGui::Text("Current Frame Time %.2f ms", timestep * 1000.0f);
			ImPlot::FitNextPlotAxes(true, true);
			ImPlot::SetNextPlotLimitsY(0, 200);
			if (ImPlot::BeginPlot("Frame Times (ms)", "Frame No.", "Frame Time (ms)", ImVec2(-1, -1)), ImPlotFlags_NoHighlight | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMousePos | ImPlotFlags_NoLegend)
			{
				ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
				ImPlot::SetNextFillStyle(ImVec4(0.0, 1.0, 0.0, 1.0));
				ImPlot::PlotBars("Frame Times", &m_FrameTimes[0], m_CurrentFrame, 1.0f, 0.0f, 0);
				ImPlot::SetNextFillStyle(ImVec4(1.0, 0.0, 0.0, 1.0));
				ImPlot::PlotBars("Frame Times", &m_FrameTimes[m_CurrentFrame], MAX_FRAME_TIMES - m_CurrentFrame, 1.0f, m_CurrentFrame);
				ImPlot::SetNextFillStyle(ImVec4(0.0, 0.0, 0.0, 0.0));
				float tmp[1] = { m_AverageFrameTime * 3 };
				ImPlot::PlotBars("", tmp, 1);
				ImPlot::EndPlot();
			}
			ImGui::End();
		}
		
		if (m_ShowingTerminal && m_ShowingFromTerminal)
		{
			if (ImGui::Begin("Logging", &m_ShowingTerminal, ImGuiWindowFlags_NoScrollbar))
			{
				m_ShowingFromTerminal = m_Terminal.show();
			}
			ImGui::End();
		}

		if (ImGui::Begin("Renderer"))
		{
			ImGui::Text("Vendor %s", RenderCommand::GetVendor());
			ImGui::Text("Renderer: %s", RenderCommand::GetRenderer());
			ImGui::Text("Version: %s", RenderCommand::GetVersion());
			ImGui::End();
		}

		if (m_CurrentFrame >= MAX_FRAME_TIMES)
		{
			m_CurrentFrame = 0;
			m_AverageFrameTime = m_FrameSum / MAX_FRAME_TIMES;
			m_FrameSum = 0;
		}*/
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

}