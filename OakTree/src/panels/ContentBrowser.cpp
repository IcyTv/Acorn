#include "ContentBrowser.h"

#include "OakLayer.h"

#include <future>
#include <imgui.h>
#include <queue>
#include <unordered_set>

namespace Acorn
{
	//It makes no sense to resize this, since we would have to reload the texture etc...
	constexpr int TEXTURE_THUMB_SIZE = 128;

	static std::queue<std::future<std::pair<std::string, Ref<Texture2d>>>> s_TextureLoadQueue;
	static std::unordered_set<std::string> s_TextureLoading;
	//To be changed
	const std::filesystem::path s_AssetsDirectory = "res";

	static std::pair<std::string, Ref<Texture2d>> LoadTexture(std::filesystem::path path)
	{
		std::string filename = path.string();

		Ref<Texture2d> texture = Texture2d::Create(filename, TEXTURE_THUMB_SIZE, TEXTURE_THUMB_SIZE);
		return std::make_pair(filename, texture);
	}

	ContentBrowserPanel::ContentBrowserPanel(OakLayer* layer)
		: m_CurrentPath(s_AssetsDirectory), m_Layer(layer)
	{
		m_FolderIcon = Texture2d::Create("res/textures/icons/appicns/appicns_FolderGeneric.png");
		m_FileIcon = Texture2d::Create("res/textures/icons/appicns/appicns_OtherDocument.png");
		m_AcornFileIcon = Texture2d::Create("res/textures/icons/AcornFile.png");
		m_JSFileIcon = Texture2d::Create("res/textures/icons/JSFile.png");
	}

	//TODO initialize on construction, add directory watch
	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (!std::filesystem::exists(m_CurrentPath))
		{
			AC_CORE_ASSERT(false, "File does not exist, {}", m_CurrentPath.string());
		}

		if (!s_TextureLoadQueue.empty())
		{
			AC_CORE_TRACE("Waiting for thumnail load");
			s_TextureLoadQueue.front().wait();
			std::pair<std::string, Ref<Texture2d>> texturePreview = s_TextureLoadQueue.front().get();
			s_TextureLoading.erase(texturePreview.first);
			s_TextureLoadQueue.pop();
			m_TexturePreviews.emplace(texturePreview);
		}

		if (m_CurrentPath != s_AssetsDirectory)
		{
			if (ImGui::SmallButton(".."))
			{
				m_CurrentPath = m_CurrentPath.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;

		ImGui::Columns(std::max((int)(panelWidth / cellSize), 1), NULL, false);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.9f, 0.0f, 0.0f, 0.0f});

		for (auto& file : std::filesystem::directory_iterator(m_CurrentPath))
		{
			std::string fileName = file.path().filename().string();

			ImGui::PushID(fileName.c_str());

			uint32_t id = 0;
			if (file.is_directory())
			{
				id = m_FolderIcon->GetRendererId();
			}
			else if (IsTexture(file))
			{
				id = GetTexturePreview(file);
			}
			else if (file.path().extension() == ".acorn")
			{
				id = m_AcornFileIcon->GetRendererId();
			}
			else if (file.path().extension() == ".js")
			{
				id = m_JSFileIcon->GetRendererId();
			}
			else
			{
				id = m_FileIcon->GetRendererId();
			}

			ImGui::ImageButton((ImTextureID)(intptr_t)id, {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});

			//TODO move to enum
			const char* itemType = "CONTENT_BROWSER_ASSET";

			if (IsTexture(file))
			{
				itemType = "TEXTURE_ASSET";
			}
			else if (file.path().extension() == ".acorn")
			{
				//TODO
			}
			else if (file.path().extension() == ".ts")
			{
				itemType = "JS_SCRIPT_FILE";
			}

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* wFileName = file.path().c_str();
				ImGui::SetDragDropPayload(itemType, wFileName, wcslen(wFileName) * sizeof(wchar_t) + 2, ImGuiCond_Once);
				ImGui::Image((ImTextureID)(intptr_t)id, {thumbnailSize / 4, thumbnailSize / 4}, {0, 1}, {1, 0});
				ImGui::EndDragDropSource();
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				//TODO store file extension
				if (file.is_directory())
				{
					m_CurrentPath = file.path();
				}
				else if (file.path().extension() == ".ts")
				{
#ifdef AC_PLATFORM_WINDOWS
					std::string command = "explorer";
#else
	#error Unknown platform
#endif
					command.append(" ");
					command.append(file.path().string());
					system(command.c_str());
				}
				else if (file.path().extension() == ".acorn")
				{
					m_Layer->OpenScene(file.path());
				}
			}
			ImGui::TextWrapped("%s", fileName.c_str());
			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::PopStyleColor();

		ImGui::Columns(1);

		// ImGui::SliderFloat("Thumbnail", &thumbnailSize, 16.0f, 512.0f);
		// ImGui::SliderFloat("Padding", &padding, 0.0f, 32.0f);

		ImGui::End();
	}

	bool ContentBrowserPanel::IsTexture(std::filesystem::path path)
	{
		return path.extension() == ".png";
		//|| path.extension() == ".jpg" || path.extension() == ".jpeg";
	}

	uint32_t ContentBrowserPanel::GetTexturePreview(std::filesystem::path path)
	{
		//TODO move to thread
		std::string filename = path.string();
		auto texture = m_TexturePreviews.find(filename);

		if (s_TextureLoading.find(filename) != s_TextureLoading.end())
		{
			return m_FileIcon->GetRendererId();
		}
		else if (texture != m_TexturePreviews.end())
		{
			return texture->second->GetRendererId();
		}
		else
		{
			s_TextureLoading.emplace(filename);

			auto future = std::async(std::launch::deferred, LoadTexture, path);
			s_TextureLoadQueue.push(std::move(future));
			return m_FileIcon->GetRendererId();
		}
	}

}