#pragma once

#include "RenderCommand.h"
#include "RendererApi.h"

#include "Camera.h"
#include "Shader.h"

namespace Acorn
{
	class Renderer
	{
		friend class Application;

	public:
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void Submit(
			const Ref<Shader>& shader,
			const Ref<VertexArray>& vertexArray,
			const glm::mat4& transform = glm::mat4(1.0f));

		inline static RendererApi::Api GetApi() { return RendererApi::GetAPI(); }

		static void ShutDown();

	private:
		static void Init();
		static void OnWindowResize(uint32_t width, uint32_t height);

	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static Scope<SceneData> m_SceneData;
	};
}