#pragma once

#include <Acorn.h>

#include <filesystem>
#include <unordered_map>

namespace Acorn
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void OnImGuiRender();

	private:
		bool IsTexture(std::filesystem::path path);
		uint32_t GetTexturePreview(std::filesystem::path path);

	private:
		std::filesystem::path m_CurrentPath;

		Ref<Texture2d> m_FolderIcon;
		Ref<Texture2d> m_FileIcon;
		Ref<Texture2d> m_AcornFileIcon;

		std::unordered_map<std::string, Ref<Texture2d>> m_TexturePreviews;
	};
}