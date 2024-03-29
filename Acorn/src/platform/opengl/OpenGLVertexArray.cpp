#include "acpch.h"

#include "platform/opengl/OpenGLVertexArray.h"

#include "Acorn/renderer/Buffer.h"

#include <glad/glad.h>

#include <TracyOpenGL.hpp>

namespace Acorn
{
	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:
				return GL_FLOAT;
			case ShaderDataType::Float2:
				return GL_FLOAT;
			case ShaderDataType::Float3:
				return GL_FLOAT;
			case ShaderDataType::Float4:
				return GL_FLOAT;
			case ShaderDataType::Mat3:
				return GL_FLOAT;
			case ShaderDataType::Mat4:
				return GL_FLOAT;
			case ShaderDataType::Int:
				return GL_INT;
			case ShaderDataType::Int2:
				return GL_INT;
			case ShaderDataType::Int3:
				return GL_INT;
			case ShaderDataType::Int4:
				return GL_INT;
			case ShaderDataType::Bool:
				return GL_BOOL;
		}

		AC_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLVertexArray::OpenGLVertexArray");

		glCreateVertexArrays(1, &m_RendererId);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLVertexArray::~OpenGLVertexArray");

		glDeleteVertexArrays(1, &m_RendererId);
	}

	void OpenGLVertexArray::Bind() const
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLVertexArray::Bind");

		glBindVertexArray(m_RendererId);
	}

	void OpenGLVertexArray::Unbind() const
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLVertexArray::Unbind");

		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLVertexArray::AddVertexBuffer");

		AC_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_RendererId);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			switch (element.Type)
			{
				case ShaderDataType::Float:
				case ShaderDataType::Float2:
				case ShaderDataType::Float3:
				case ShaderDataType::Float4:
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex,
										  element.GetComponentCount(),
										  ShaderDataTypeToOpenGLBaseType(element.Type),
										  element.Normalized ? GL_TRUE : GL_FALSE,
										  layout.GetStride(),
										  (const void*)(intptr_t)element.Offset);
					m_VertexBufferIndex++;
					break;
				}
				case ShaderDataType::Int:
				case ShaderDataType::Int2:
				case ShaderDataType::Int3:
				case ShaderDataType::Int4:
				case ShaderDataType::Bool:
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribIPointer(m_VertexBufferIndex,
										   element.GetComponentCount(),
										   ShaderDataTypeToOpenGLBaseType(element.Type),
										   layout.GetStride(),
										   (const void*)(intptr_t)element.Offset);
					m_VertexBufferIndex++;
					break;
				}
				case ShaderDataType::Mat3:
				case ShaderDataType::Mat4:
				{
					uint8_t count = element.GetComponentCount();
					for (uint8_t i = 0; i < count; i++)
					{
						glEnableVertexAttribArray(m_VertexBufferIndex);
						glVertexAttribPointer(m_VertexBufferIndex,
											  count,
											  ShaderDataTypeToOpenGLBaseType(element.Type),
											  element.Normalized ? GL_TRUE : GL_FALSE,
											  layout.GetStride(),
											  (const void*)(intptr_t)(element.Offset + sizeof(float) * count * i));
						glVertexAttribDivisor(m_VertexBufferIndex, 1);
						m_VertexBufferIndex++;
					}
					break;
				}
				default:
					AC_CORE_ASSERT(false, "Unknown ShaderDataType!");
			}
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		AC_PROFILE_FUNCTION();
		TracyGpuZone("OpenGLVertexArray::SetIndexBuffer");

		glBindVertexArray(m_RendererId);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}
}