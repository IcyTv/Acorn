#pragma once

#include <glm/glm.hpp>

#include "VertexArray.h"

namespace Acorn
{
	class RendererApi
	{
	public:
		enum class Api
		{
			None = 0, OpenGL = 1,
		};

	public:
		virtual ~RendererApi() = default;

		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4 color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) = 0;

		inline static Api GetAPI() { return s_API; }

		virtual const char* GetRenderer() const = 0;
		virtual const char* GetVersion() const = 0;
		virtual const char* GetVendor() const = 0;
	private:
		static Api s_API;
	};
}