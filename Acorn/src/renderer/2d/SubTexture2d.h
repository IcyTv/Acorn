#pragma once

#include "renderer/Texture.h"

#include <glm/vec2.hpp>

namespace Acorn::ext2d
{
	class SubTexture
	{
	public:
		SubTexture(const Ref<Texture2d>& texture, const glm::vec2& min, const glm::vec2& max);

		const Ref<Texture2d> GetTexture() const { return m_Texture; }
		inline const glm::vec2* GetTexCoords() const { return m_TexCoords; }

		static Ref<SubTexture> CreateFromCoords(const Ref<Texture2d>& texture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize = { 1, 1 });
	
	private:
		Ref<Texture2d> m_Texture;

		glm::vec2 m_TexCoords[4];
	};
}