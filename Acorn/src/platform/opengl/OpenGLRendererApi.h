#pragma once

#include "renderer/RendererApi.h"

namespace Acorn
{
	class OpenGLRendererApi : public RendererApi
	{
	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const glm::vec4 color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count) override;

		virtual const char* GetRenderer() const override;
		virtual const char* GetVersion() const override;
		virtual const char* GetVendor() const override;
	};
}