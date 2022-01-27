#pragma once

#include <Acorn.h>

#include "GPUDriverImpl.h"
#include "Ultralight/Bitmap.h"
#include "Ultralight/RefPtr.h"
#include "ecs/components/SceneCamera.h"
#include "renderer/2d/Renderer2D.h"

#include <Ultralight/platform/GPUDriver.h>

#include <glm/fwd.hpp>
#include <map>
#include <vector>

namespace Acorn
{
	class OpenGlGPUDriver : public GPUDriverImpl
	{
	public:
		OpenGlGPUDriver();
		virtual ~OpenGlGPUDriver() = default;

		virtual const char* name() override { return "Acorn"; }

		virtual void BeginDrawing() override;
		virtual void EndDrawing() override;

		virtual void CreateTexture(uint32_t texture_id, ultralight::Ref<ultralight::Bitmap> bitmap) override;
		virtual void UpdateTexture(uint32_t texture_id, ultralight::Ref<ultralight::Bitmap> bitmap) override;
		virtual void BindTexture(uint8_t textureUnit, uint32_t textureId) override;
		virtual void DestroyTexture(uint32_t textureId) override;

		virtual void CreateRenderBuffer(uint32_t renderBufferId, const ultralight::RenderBuffer& renderBuffer) override;
		virtual void BindRenderBuffer(uint32_t renderBufferId) override;
		virtual void ClearRenderBuffer(uint32_t render_buffer_id) override;
		virtual void DestroyRenderBuffer(uint32_t renderBufferId) override;

		virtual void CreateGeometry(uint32_t geometryId, const ultralight::VertexBuffer& vertices, const ultralight::IndexBuffer& indices) override;
		virtual void UpdateGeometry(uint32_t geometry_id,
									const ultralight::VertexBuffer& vertices,
									const ultralight::IndexBuffer& indices) override;

		virtual void DrawGeometry(uint32_t geometry_id,
								  uint32_t indices_count,
								  uint32_t indices_offset,
								  const ultralight::GPUState& state) override;

		virtual void DestroyGeometry(uint32_t geometry_id) override;

		virtual void DrawCommandList() override;

		void BindUltralightTexture(uint32_t ultralight_texture_id);
	};

}