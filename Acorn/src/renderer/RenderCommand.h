#pragma once

#include "RendererApi.h"

#include <glm/vec4.hpp>

namespace Acorn
{
	class RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererApi->Init();
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererApi->SetViewport(x, y, width, height);
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererApi->SetClearColor(color);
		}

		inline static void Clear()
		{
			s_RendererApi->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0)
		{
			s_RendererApi->DrawIndexed(vertexArray, count);
		}

		inline static const char* GetVendor() { return s_RendererApi->GetVendor(); }
		inline static const char* GetRenderer() { return s_RendererApi->GetRenderer(); }
		inline static const char* GetVersion() { return s_RendererApi->GetVersion(); }
	private:
		static Scope<RendererApi> s_RendererApi;
	};
}