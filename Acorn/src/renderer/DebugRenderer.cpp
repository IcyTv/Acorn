#include "acpch.h"

#include "2d/SubTexture2d.h"
#include "DebugRenderer.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "VertexArray.h"

#include "BatchRenderer.h"

#include "utils/fonts/IconsFontAwesome4.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// #include <ft2build.h>
// #include FT_FREETYPE_H

#define length(array) ((sizeof(array)) / (sizeof(array[0])))

constexpr int FONT_SIZE = 48;
constexpr int FONT_TEXTURE_SIZE = 1024;

namespace Acorn
{
	namespace debug
	{
		struct DebugVertex
		{
			glm::vec3 SquareVertices;
			glm::vec3 BillboardPosition;
			glm::vec2 BillboardScale;
			glm::vec4 Color;
			glm::vec2 TexCoord;
			float TexIndex;

			int EntityId = -1;
		};

		struct DebugRendererStorage
		{
			const float SizeScalar = 0.1f;

			uint32_t CurrentWidth = 0;
			uint32_t CurrentHeight = 0;

			Ref<Shader> BillboardShader;
			std::unordered_map<unsigned long, Ref<ext2d::SubTexture>> TextureMap;

			Ref<Texture2d> IconTexture;

			glm::vec4 QuadVertexPositions[4];

			Scope<BatchRenderer<DebugVertex, 6, 4>> QuadRenderer;

			// FT_Library FreeTypeLibrary;
			// FT_Face IconFont;

			struct CameraData
			{
				glm::mat4 ViewProjection;
				glm::vec4 CameraRight;
				glm::vec4 CameraUp;
			};

			CameraData CameraBuffer;
			Ref<UniformBuffer> CameraUniform;
		};

		static DebugRendererStorage s_Data;

		static void LoadChar(unsigned long charCode)
		{
			if (s_Data.TextureMap.find(charCode) != s_Data.TextureMap.end())
				return;

			AC_CORE_TRACE("Char Code Test {0:x}", charCode);
			// FT_ULong charIndex = FT_Get_Char_Index(s_Data.IconFont, charCode);
			// if (FT_Load_Glyph(s_Data.IconFont, charIndex, FT_LOAD_RENDER))
			// 	AC_CORE_ASSERT(false, "Failed to load glyph!");

			// FT_GlyphSlot glyph = s_Data.IconFont->glyph;
			// int width = glyph->bitmap.width;
			// int height = glyph->bitmap.rows;

			// s_Data.IconTexture->SetSubData(glyph->bitmap.buffer, width * height, s_Data.CurrentWidth, s_Data.CurrentHeight, width, height);

			// Ref<ext2d::SubTexture> subTexture = ext2d::SubTexture::CreateFromCoords(s_Data.IconTexture, {s_Data.CurrentWidth, s_Data.CurrentHeight}, {width, height});
			// s_Data.TextureMap.insert(std::make_pair(charCode, subTexture));

			// s_Data.CurrentWidth += width;
			// if (s_Data.CurrentWidth > FONT_TEXTURE_SIZE)
			// {
			// 	s_Data.CurrentWidth = 0;
			// 	s_Data.CurrentHeight += height;
			// }
		}

		void Renderer::Init()
		{
			AC_PROFILE_FUNCTION();

			s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
			s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
			s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
			s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

			s_Data.IconTexture = Texture2d::Create(FONT_TEXTURE_SIZE, FONT_TEXTURE_SIZE, 1);
			char* data = new char[1024 * 1024]{0};
			s_Data.IconTexture->SetData(data, FONT_TEXTURE_SIZE * FONT_TEXTURE_SIZE);
			delete[] data;

			s_Data.IconTexture->SetTextureFiltering(TextureFiltering::Linear);

			// if (FT_Init_FreeType(&s_Data.FreeTypeLibrary))
			// {
			// 	AC_CORE_ASSERT(false, "Failed to initialize FreeType Library");
			// }

			// if (FT_New_Face(s_Data.FreeTypeLibrary, "res/fonts/fontawesome-webfont.ttf", 0, &s_Data.IconFont))
			// {
			// 	AC_CORE_ASSERT(false, "Failed to load font");
			// }

			// if (FT_Set_Pixel_Sizes(s_Data.IconFont, 0, FONT_SIZE))
			// {
			// 	AC_CORE_ASSERT(false, "Failed to set font size");
			// }

			// FT_Matrix matrix;
			// float angle = glm::radians(180.0f);
			// matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
			// matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
			// matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
			// matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);
			// FT_Set_Transform(s_Data.IconFont, &matrix, 0);

			// LoadChar(ICON_FA_CAMERA_HEX);
			// LoadChar(ICON_FA_ANGLE_RIGHT_HEX);

			// FT_Done_Face(s_Data.IconFont);
			// FT_Done_FreeType(s_Data.FreeTypeLibrary);

			AC_CORE_TRACE("Loaded fonts");

			std::array<uint32_t, 6> indices = {0, 1, 2, 2, 3, 0};

			BufferLayout layout =
				{{ShaderDataType::Float3, "a_SquareVertices"},
				 {ShaderDataType::Float3, "a_BillboardPosition"},
				 {ShaderDataType::Float2, "a_BillboardScale"},
				 {ShaderDataType::Float4, "a_Color"},
				 {ShaderDataType::Float2, "a_TexCoord"},
				 {ShaderDataType::Float, "a_TexIndex"},
				 {ShaderDataType::Int, "a_EntityId"}};
			s_Data.BillboardShader = Shader::Create("res/shaders/Billboard.shader");

			s_Data.QuadRenderer = CreateScope<BatchRenderer<DebugVertex, 6, 4>>(s_Data.BillboardShader, indices, layout);
			s_Data.QuadRenderer->AddDefaultTexture(s_Data.IconTexture);

			s_Data.BillboardShader->Bind();

			s_Data.CameraUniform = UniformBuffer::Create(sizeof(DebugRendererStorage::CameraData), 0);
		}

		void Renderer::ShutDown()
		{
		}

		void Renderer::Begin(const EditorCamera& camera)
		{
			AC_PROFILE_FUNCTION();

			s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
			s_Data.CameraBuffer.CameraRight = glm::vec4(camera.GetRightDirection(), 0.0f);
			s_Data.CameraBuffer.CameraUp = glm::vec4(camera.GetUpDirection(), 0.0f);
			s_Data.CameraUniform->SetData(&s_Data.CameraBuffer, sizeof(DebugRendererStorage::CameraData));

			s_Data.QuadRenderer->Begin();

			// s_Data.QuadRenderer->GetShader()->SetMat4("u_ViewProjection", camera.GetViewProjection());
			// s_Data.QuadRenderer->GetShader()->SetFloat3("u_CameraRight", camera.GetRightDirection());
			// s_Data.QuadRenderer->GetShader()->SetFloat3("u_CameraUp", camera.GetUpDirection());
		}

		void Renderer::End()
		{
			s_Data.QuadRenderer->End();
		}

		void Renderer::DrawGizmo(GizmoType type, const glm::vec3& position, int entityId, const glm::vec4& color, const glm::vec2& scale)
		{

			unsigned long name;
			switch (type)
			{
				case GizmoType::Camera:
					name = ICON_FA_CAMERA_HEX;
					break;
				default:
					AC_CORE_ASSERT(false, "Unknown GizmoType");
					return;
			}
			Ref<ext2d::SubTexture> subTexture = s_Data.TextureMap[name];
			const glm::vec2* texCoords = subTexture->GetTexCoords();

			std::array<DebugVertex, 4> vertices;

			constexpr size_t quadVertexCount = 4;
			constexpr size_t textureIndex = 0;

			for (size_t i = 0; i < quadVertexCount; i++)
			{
				vertices[i].SquareVertices = s_Data.QuadVertexPositions[i];
				vertices[i].BillboardPosition = position;
				vertices[i].BillboardScale = scale * s_Data.SizeScalar;
				vertices[i].Color = color;
				vertices[i].TexCoord = texCoords[i];
				vertices[i].TexIndex = textureIndex;
				vertices[i].EntityId = entityId;
			}

			s_Data.QuadRenderer->Draw(0, vertices);
		}

		uint32_t Renderer::GetDrawCalls()
		{
			return s_Data.QuadRenderer->GetStats().DrawCalls;
		}

		uint32_t Renderer::GetQuadCount()
		{
			return s_Data.QuadRenderer->GetStats().QuadCount;
		}

		uint32_t Renderer::GetIndexCount()
		{
			return s_Data.QuadRenderer->GetStats().GetTotalIndexCount();
		}

		uint32_t Renderer::GetVertexCount()
		{
			return s_Data.QuadRenderer->GetStats().GetTotalVertexCount();
		}
	}

}