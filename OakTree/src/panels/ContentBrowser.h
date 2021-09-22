#pragma once

#include <Acorn.h>

#include <filesystem>
#include <unordered_map>

namespace Acorn
{
	class OakLayer;

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel(OakLayer* layer);

		void OnImGuiRender();

	private:
		bool IsTexture(std::filesystem::path path);
		uint32_t GetTexturePreview(std::filesystem::path path);

	private:
		std::filesystem::path m_CurrentPath;

		OakLayer* m_Layer;

		Ref<Texture2d> m_FolderIcon;
		Ref<Texture2d> m_FileIcon;
		Ref<Texture2d> m_AcornFileIcon;
		Ref<Texture2d> m_JSFileIcon;

		std::unordered_map<std::string, Ref<Texture2d>> m_TexturePreviews;
	};
}