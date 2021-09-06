#include "ImGuiLayer.h"
#include "acpch.h"
#include "core/Application.h"
#include "input/KeyCodes.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui.h"

#include "renderer/RenderCommand.h"

#include "utils/fonts/IconsFontAwesome4.h"

#include <ImGuizmo.h>
#include <implot.h>

//Temporary
#include <GLFW/glfw3.h>
#include <glad/glad.h>

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

		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImFontConfig font_config;
		font_config.MergeMode = false;
		io.Fonts->AddFontFromFileTTF("res/fonts/Inconsolata-Bold.ttf", 16.0f, &font_config);

		ImFontConfig config;
		config.MergeMode = true;
		config.GlyphMinAdvanceX = 16.0f; // Use if you want to make the icon monospaced
		static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
		io.Fonts->AddFontFromFileTTF("res/fonts/Inconsolata-Regular.ttf", 16.0f, &font_config);
		io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FA, 16.0f, &config, icon_ranges);
		io.Fonts->Build();

		io.FontDefault = io.Fonts->Fonts[1];

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColors();

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
		ImGuizmo::BeginFrame();
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

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);

		//Headers
		colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.1f, 0.1505f, 0.151f, 1.0f);

		//Buttons
		colors[ImGuiCol_Button] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

		//Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.1f, 0.1505f, 0.151f, 1.0f);

		//Tabs
		colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.105f, 0.151f, 1.0f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.38f, 0.3805f, 0.381f, 1.0f);
		colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.2805f, 0.281f, 1.0f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);

		//Title
		colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
	}

	void ImGuiLayer::OnImGuiRender(Timestep timestep)
	{
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