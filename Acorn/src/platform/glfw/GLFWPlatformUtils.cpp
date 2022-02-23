#include "Acorn/core/Application.h"
#include "Acorn/utils/PlatformUtils.h"
#include "acpch.h"

#include <filesystem>

#include <GLFW/glfw3.h>
#include <portable-file-dialogs.h>

namespace Acorn
{
	std::string PlatformUtils::OpenFile(const std::vector<std::string>& filters, const std::string& title, const std::string& initialPath, bool multiselect)
	{
		auto selection = pfd::open_file(title, initialPath, filters, multiselect ? pfd::opt::force_path : pfd::opt::none).result();

		if (selection.empty())
		{
			return "";
		}

		if (multiselect)
		{
			AC_CORE_ERROR("Multiselect not supported by the api yet!");
			AC_ASSERT_NOT_REACHED();
			return "";
		}

		AC_CORE_ASSERT(selection.size() == 1, "Selection size is not 1!");
		return selection[0];
	}

	std::string PlatformUtils::SaveFile(const std::vector<std::string>& filters, const std::string& title, const std::string& initialPath)
	{
		auto selection = pfd::save_file(title, initialPath, filters, pfd::opt::none).result();

		if (selection.empty())
		{
			AC_CORE_WARN("File not saved, because the user canceled!");
			return "";
		}

		return selection;
	}
}

#if 0
	#include <commdlg.h>
	#define GLFW_EXPOSE_NATIVE_WIN32
	#include <GLFW/glfw3native.h>

namespace Acorn
{

	std::string PlatformUtils::OpenFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		char szFile[260] = {0};
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}

	std::string PlatformUtils::SaveFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		char szFile[260] = {0};
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.lpstrDefExt = filter;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;

		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}
}

#endif