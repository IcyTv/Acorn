#include "acpch.h"

#include "renderer/2d/SubTexture2d.h"
#include "renderer/DebugRenderer.h"
#include "renderer/RenderCommand.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "renderer/UniformBuffer.h"
#include "renderer/VertexArray.h"

#include "renderer/BatchRenderer.h"

#include "core/Core.h"
#include "renderer/2d/Renderer2D.h"
#include "utils/fonts/IconsFontAwesome4.h"

#include <glm/exponential.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/matrix.hpp>

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

		struct LineVertex
		{
			glm::vec3 Position;
			glm::vec4 Color;
		};

		struct CircleVertex
		{
			glm::vec3 WorldPosition;
			glm::vec3 LocalPosition;
			glm::vec4 Color;
			float Thickness;
			float Fade;

			int EntityId;
		};

		struct DebugRendererStorage
		{
			const float SizeScalar = 0.3f;

			uint32_t CurrentWidth = 0;
			uint32_t CurrentHeight = 0;

			Ref<Shader> BillboardShader;
			Ref<Shader> BasicShader;
			Ref<Shader> CircleShader;
			std::unordered_map<unsigned long, Ref<ext2d::SubTexture>> TextureMap;

			Ref<Texture2d> IconTexture;

			glm::vec4 QuadVertexPositions[4];

			Scope<BatchRenderer<DebugVertex, 6, 4>> QuadRenderer;
			Scope<BatchRenderer<CircleVertex, 6, 4>> CircleRenderer;
			Scope<BatchRenderer<LineVertex, 2, 2, true>> LineRenderer;

			struct BillboardData
			{
				glm::mat4 ViewProjection;
				glm::vec4 CameraRight;
				glm::vec4 CameraUp;
			};

			BillboardData BillboardData;
			Ref<UniformBuffer> BillboardUniform;

			struct CameraData
			{
				glm::mat4 ViewProjection;
			};

			CameraData CameraBuffer;
			Ref<UniformBuffer> CameraUniform;
		};

		static DebugRendererStorage s_Data;

		void Renderer::Init()
		{
			AC_PROFILE_FUNCTION();

			s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
			s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
			s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
			s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

			s_Data.IconTexture = Texture2d::Create("res/textures/icon-texture.png");
			s_Data.IconTexture->SetTextureFiltering(TextureFiltering::Linear);

			s_Data.TextureMap[ICON_FA_CAMERA_HEX] = ext2d::SubTexture::CreateFromCoords(s_Data.IconTexture, {0, 0}, {48, 48});

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
			s_Data.BasicShader = Shader::Create("res/shaders/Basic.shader");

			s_Data.QuadRenderer = CreateScope<BatchRenderer<DebugVertex, 6, 4>>(s_Data.BillboardShader, indices, layout);
			s_Data.QuadRenderer->AddDefaultTexture(s_Data.IconTexture);

			std::array<uint32_t, 2> lineIndices = {0, 1};

			BufferLayout lineLayout =
				{{ShaderDataType::Float3, "a_Position"},
				 {ShaderDataType::Float4, "a_Color"}};
			s_Data.LineRenderer = CreateScope<BatchRenderer<LineVertex, 2, 2, true>>(s_Data.BasicShader, lineIndices, lineLayout);

			s_Data.BillboardShader->Bind();

			s_Data.CameraUniform = UniformBuffer::Create(sizeof(DebugRendererStorage::CameraData), 0);
			s_Data.BillboardUniform = UniformBuffer::Create(sizeof(DebugRendererStorage::BillboardData), 1);

			BufferLayout circleLayout = {
				{ShaderDataType::Float3, "a_WorldPosition"},
				{ShaderDataType::Float3, "a_LocalPosition"},
				{ShaderDataType::Float4, "a_Color"},
				{ShaderDataType::Float, "a_Thickness"},
				{ShaderDataType::Float, "a_Fade"},
				{ShaderDataType::Int, "a_EntityId"},
			};

			s_Data.CircleShader = Shader::Create("res/shaders/Circle.shader");
			s_Data.CircleRenderer = CreateScope<BatchRenderer<CircleVertex, 6, 4>>(s_Data.CircleShader, indices, circleLayout);
		}

		void Renderer::ShutDown()
		{
			AC_PROFILE_FUNCTION();

			s_Data.BasicShader.reset();
			s_Data.BillboardShader.reset();
			s_Data.CircleShader.reset();

			s_Data.CircleRenderer.reset();
			s_Data.LineRenderer.reset();
			s_Data.QuadRenderer.reset();

			s_Data.TextureMap.clear();
			s_Data.IconTexture.reset();

			s_Data.CameraUniform.reset();
			s_Data.BillboardUniform.reset();
		}

		void Renderer::Begin(const EditorCamera& camera)
		{
			AC_PROFILE_FUNCTION();

			s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
			s_Data.CameraUniform->SetData(&s_Data.CameraBuffer, sizeof(DebugRendererStorage::CameraData));

			s_Data.BillboardData.ViewProjection = camera.GetViewProjection();
			s_Data.BillboardData.CameraRight = glm::vec4{camera.GetRightDirection(), 1.0f};
			s_Data.BillboardData.CameraUp = glm::vec4{camera.GetUpDirection(), 1.0f};
			s_Data.BillboardUniform->SetData(&s_Data.BillboardData, sizeof(DebugRendererStorage::BillboardData));

			s_Data.QuadRenderer->Begin();
			s_Data.LineRenderer->Begin();
			s_Data.CircleRenderer->Begin();
		}

		void Renderer::End()
		{
			AC_PROFILE_FUNCTION();
			s_Data.BillboardUniform->Bind();
			s_Data.QuadRenderer->End();
			s_Data.CameraUniform->Bind();
			// Clear Depth buffer to render Colliders on top (and apperantly view frustums...)
			RenderCommand::ClearDepth();
			s_Data.LineRenderer->End();
			s_Data.CircleRenderer->End();
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
			AC_CORE_ASSERT(s_Data.TextureMap.find(name) != s_Data.TextureMap.end(), "Texture not found");

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

		void Renderer::DrawCameraFrustum(const Components::CameraComponent& camera, const Components::Transform& transform)
		{
			if (camera.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				glm::mat4 cameraToWorld = camera.Camera.GetProjection() * glm::inverse(transform.GetTransform());
				glm::vec3 axisX = cameraToWorld[0];
				glm::vec3 axisY = cameraToWorld[1];
				glm::vec3 axisZ = cameraToWorld[2];

				float zn = camera.Camera.GetPerspectiveNearClip();
				float zf = camera.Camera.GetPerspectiveFarClip();

				glm::vec3 nearCenter = axisZ * zn;
				glm::vec3 farCenter = axisZ * zf;

				float e = tan(camera.Camera.GetPerspectiveFov() * 0.5f);
				float nearExtY = e * zn;
				float nearExtX = nearExtY * camera.Camera.GetAspectRatio();
				float farExtY = e * zf;
				float farExtX = farExtY * camera.Camera.GetAspectRatio();

				glm::vec3 v[8];

				v[0] = nearCenter - axisX * nearExtX - axisY * nearExtY;
				v[1] = nearCenter - axisX * nearExtX + axisY * nearExtY;
				v[2] = nearCenter + axisX * nearExtX + axisY * nearExtY;
				v[3] = nearCenter + axisX * nearExtX - axisY * nearExtY;
				v[4] = farCenter - axisX * farExtX - axisY * farExtY;
				v[5] = farCenter - axisX * farExtX + axisY * farExtY;
				v[6] = farCenter + axisX * farExtX + axisY * farExtY;
				v[7] = farCenter + axisX * farExtX - axisY * farExtY;

				for (size_t i = 0; i < 8; i++)
				{
					v[i] = v[i] + transform.Translation;
				}

				DrawLine(v[0], v[1]);
				DrawLine(v[1], v[2]);
				DrawLine(v[2], v[3]);
				DrawLine(v[3], v[0]);
				DrawLine(v[4], v[5]);
				DrawLine(v[5], v[6]);
				DrawLine(v[6], v[7]);
				DrawLine(v[7], v[4]);
				DrawLine(v[0], v[4]);
				DrawLine(v[1], v[5]);
				DrawLine(v[2], v[6]);
				DrawLine(v[3], v[7]);
			}
		}

		void Renderer::DrawB2dCollider(const Components::BoxCollider2d& collider, const Components::Transform& transform)
		{
			// This makes sure the collider is offset AND at z=0
			glm::vec4 offset = glm::vec4(collider.GetOffset(), -transform.Translation.z, 0.0f);

			glm::vec3 tl = ((glm::mat4)transform) * (glm::vec4{-collider.GetSize(), 0.0f, 1.0f} + offset);
			glm::vec3 tr = ((glm::mat4)transform) * (glm::vec4{collider.GetSize().x, -collider.GetSize().y, 0.0f, 1.0f} + offset);
			glm::vec3 bl = ((glm::mat4)transform) * (glm::vec4{-collider.GetSize().x, collider.GetSize().y, 0.0f, 1.0f} + offset);
			glm::vec3 br = ((glm::mat4)transform) * (glm::vec4{collider.GetSize(), 0.0f, 1.0f} + offset);

			constexpr glm::vec4 color{0.0f, 0.0f, 1.0f, 1.0f};

			DrawLine(tl, tr, color);
			DrawLine(tr, br, color);
			DrawLine(br, bl, color);
			DrawLine(bl, tl, color);
			DrawLine(tl, br, color);
		}

		void Renderer::DrawB2dCollider(const Components::CircleCollider2d& collider, const Components::Transform& transform)
		{
			AC_PROFILE_FUNCTION();

			constexpr size_t quadVertexCount = 4;
			constexpr glm::vec4 color{0.0f, 0.0f, 1.0f, 1.0f};

			std::array<CircleVertex, quadVertexCount> vertices;

			// TODO think about scaling axis...
			glm::mat4 trans = transform.GetTransform() * glm::scale(glm::mat4{1.0f}, (1.0f / transform.Scale)) * glm::scale(glm::mat4{1.0f}, glm::vec3(transform.Scale.z * collider.GetRadius() * 2)) * glm::translate(glm::mat4{1.0f}, glm::vec3(collider.GetOffset(), 0.0f));

			for (size_t i = 0; i < quadVertexCount; i++)
			{
				vertices[i].WorldPosition = trans * s_Data.QuadVertexPositions[i];
				vertices[i].LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
				vertices[i].Color = color;
				// TODO determine consistent thickness
				vertices[i].Thickness = 0.01f;
				vertices[i].Fade = 0.0005f;
				vertices[i].EntityId = -1;
			}

			s_Data.CircleRenderer->Draw(vertices);

			constexpr float unit = 0.70710678118; // == sqrt(2) / 2;

			float r = collider.GetRadius() * transform.Scale.z;
			glm::vec3 tl = transform.Translation + glm::vec3{-unit * r, unit * r, 0.0f} + glm::vec3(collider.GetOffset(), 0.0f);
			glm::vec3 tr = transform.Translation + glm::vec3{unit * r, -unit * r, 0.0f} + glm::vec3(collider.GetOffset(), 0.0f);

			DrawLine(tl, tr, color);
		}

		void Renderer::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color)
		{
			std::array<LineVertex, 2> vertices;

			vertices[0].Position = start;
			vertices[0].Color = color;

			vertices[1].Position = end;
			vertices[1].Color = color;

			s_Data.LineRenderer->Draw(vertices);
		}

		uint32_t Renderer::GetDrawCalls()
		{
			return s_Data.QuadRenderer->GetStats().DrawCalls;
		}

		uint32_t Renderer::GetQuadCount()
		{
			return s_Data.QuadRenderer->GetStats().ObjectCount;
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