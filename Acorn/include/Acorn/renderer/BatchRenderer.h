#pragma once

#include "RenderCommand.h"
#include "core/Core.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "renderer/VertexArray.h"
#include "utils/PlatformCapabilities.h"

#include <vector>

namespace Acorn
{
	template <typename Vertex, size_t IndicesPerObject, size_t VerticesPerObject, bool DrawLines = false>
	class BatchRenderer
	{
	public:
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t ObjectCount = 0;

			uint32_t GetTotalVertexCount() const { return ObjectCount * VerticesPerObject; }
			uint32_t GetTotalIndexCount() const { return ObjectCount * IndicesPerObject; }
		};

	public:
		BatchRenderer(const Ref<Shader>& shader, const std::array<uint32_t, IndicesPerObject>& indices, const BufferLayout& layout)
			: m_Shader(shader)
		{
			m_TextureSlots.resize(PlatformCapabilities::GetMaxTextureUnits());

			//Setup vertex array
			m_VertexArray = VertexArray::Create();

			m_VertexBuffer = VertexBuffer::Create(MAX_BATCH_SIZE * VerticesPerObject * sizeof(Vertex));
			m_VertexBuffer->SetLayout(layout);

			m_VertexArray->AddVertexBuffer(m_VertexBuffer);

			m_VertexBufferBase = new Vertex[MAX_BATCH_SIZE * IndicesPerObject];

			//Setup index buffer
			uint32_t* bufferIndices = new uint32_t[MAX_BATCH_SIZE * IndicesPerObject];
			uint32_t offset = 0;
			for (size_t i = 0; i < MAX_BATCH_SIZE; i += indices.size())
			{
				for (size_t j = 0; j < indices.size(); j++)
				{
					bufferIndices[i + j] = indices[j] + offset;
				}
				offset += VerticesPerObject;
			}

			Ref<IndexBuffer> indexBuffer;
			indexBuffer = IndexBuffer::Create(bufferIndices, MAX_BATCH_SIZE * IndicesPerObject);
			delete[] bufferIndices;

			m_VertexArray->SetIndexBuffer(indexBuffer);

			uint32_t maxTextureSlots = PlatformCapabilities::GetMaxTextureUnits();
			int32_t* samplers = new int32_t[maxTextureSlots];
			for (uint32_t i = 0; i < maxTextureSlots; i++)
			{
				samplers[i] = i;
			}

			m_Shader->Bind();
			m_Shader->SetIntArray("u_Textures[0]", samplers, maxTextureSlots);
			m_Shader->Unbind();
		}

		~BatchRenderer()
		{
			delete[] m_VertexBufferBase;
		}

		void Begin()
		{
			//TODO do we need this? Since it does not get accessed during BatchRenderer::Draw calls... -> Bind at End() instead and allow multiple BatchRenderers
			// m_Shader->Bind();
			// m_VertexArray->Bind();

			m_TextureSlotIndex = m_MinTextureSlotIndex;
			m_IndexCount = 0;
			m_VertexBufferPtr = m_VertexBufferBase;
		}

		void End()
		{
			m_Shader->Bind();
			m_VertexArray->Bind();
			// uint32_t size = (uint32_t)((uint8_t*)m_VertexBufferPtr - (uint8_t*)m_VertexBufferBase);
			uint32_t size = (m_IndexCount / IndicesPerObject) * VerticesPerObject * sizeof(Vertex);
			m_VertexBuffer->SetData(m_VertexBufferBase, size);

			Flush();
			m_VertexArray->Unbind();
		}

		void Flush()
		{
			for (uint32_t i = 0; i < m_TextureSlotIndex; i++)
			{
				m_TextureSlots[i]->Bind(i);
			}

			m_Shader->Bind();
			m_VertexArray->Bind();
			if constexpr (DrawLines)
				RenderCommand::DrawLines(m_VertexArray, m_IndexCount);
			else
				RenderCommand::DrawIndexed(m_VertexArray, m_IndexCount);

			m_Statistics.DrawCalls++;
		}

		void FlushAndReset()
		{
			End();
			m_IndexCount = 0;
			m_VertexBufferPtr = m_VertexBufferBase;

			m_TextureSlotIndex = m_MinTextureSlotIndex;
		}

		void AddDefaultTexture(const Ref<Texture2d>& texture)
		{
			m_TextureSlots[m_TextureSlotIndex] = texture;
			m_TextureSlotIndex++;
			m_MinTextureSlotIndex++;
		}

		void Draw(const std::array<Vertex, VerticesPerObject>& vertices)
		{
			if (m_IndexCount > MAX_BATCH_SIZE * IndicesPerObject)
			{
				FlushAndReset();
			}

			for (size_t i = 0; i < vertices.size(); i++)
			{
				*m_VertexBufferPtr = vertices[i];
				m_VertexBufferPtr++;
			}

			m_IndexCount += IndicesPerObject;
			m_Statistics.ObjectCount++;
		}

		void Draw(uint32_t textureIndex, const std::array<Vertex, VerticesPerObject>& vertices)
		{
			if (m_IndexCount > MAX_BATCH_SIZE * IndicesPerObject)
			{
				FlushAndReset();
			}

			AC_CORE_ASSERT(textureIndex < m_MinTextureSlotIndex, "Texture index is out of range!");

			for (uint32_t i = 0; i < VerticesPerObject; i++)
			{
				*m_VertexBufferPtr = vertices[i];
				m_VertexBufferPtr++;
			}

			m_IndexCount += IndicesPerObject;
			m_Statistics.ObjectCount++;
		}

		//TODO template check for Vertex.TexIndex
		void Draw(const Ref<Texture2d>& texture, const std::array<Vertex, VerticesPerObject>& vertices)
		{
			if (m_IndexCount > MAX_BATCH_SIZE * IndicesPerObject)
			{
				FlushAndReset();
			}

			float textureIndex = -1.0f;
			for (uint32_t i = 0; i < m_TextureSlotIndex; i++)
			{
				if (m_TextureSlots[i] == texture)
				{
					textureIndex = (float)i;
					break;
				}
			}

			if (textureIndex == -1.0f)
			{
				if (m_TextureSlotIndex >= m_TextureSlots.size())
				{
					FlushAndReset();
				}
				m_TextureSlots[m_TextureSlotIndex] = texture;
				textureIndex = (float)m_TextureSlotIndex;
				m_TextureSlotIndex++;
			}

			for (uint32_t i = 0; i < VerticesPerObject; i++)
			{
				//FIXME: Either enforce TexId field with template or add function to get texture index
				*m_VertexBufferPtr = vertices[i];
				m_VertexBufferPtr->TexIndex = textureIndex;
				m_VertexBufferPtr++;
			}

			m_IndexCount += IndicesPerObject;
			m_Statistics.ObjectCount++;
		}

		Ref<Shader> GetShader()
		{
			return m_Shader;
		}

		Statistics GetStats()
		{
			return m_Statistics;
		}
		void ResetStats()
		{
			memset(&m_Statistics, 0, sizeof(Statistics));
		}

	private:
		static constexpr uint32_t MAX_BATCH_SIZE = 10000;

		uint32_t m_IndexCount = 0;
		uint32_t m_TextureSlotIndex = 0;

		uint32_t m_MinTextureSlotIndex = 0;

		Vertex* m_VertexBufferBase = nullptr;
		Vertex* m_VertexBufferPtr = nullptr;
		std::vector<Ref<Texture2d>> m_TextureSlots;

		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<Shader> m_Shader;

		Statistics m_Statistics;
	};
}