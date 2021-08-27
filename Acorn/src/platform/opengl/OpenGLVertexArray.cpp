#include "acpch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Acorn
{
	uint32_t ToOpenGLType(ShaderDataType type)
	{
		switch (type)
		{
		case Acorn::ShaderDataType::None:
			return 0;
		case Acorn::ShaderDataType::Float:
		case Acorn::ShaderDataType::Float2:
		case Acorn::ShaderDataType::Float3:
		case Acorn::ShaderDataType::Float4:
		case Acorn::ShaderDataType::Mat3:
		case Acorn::ShaderDataType::Mat4:
			return GL_FLOAT;
		case Acorn::ShaderDataType::Int:
		case Acorn::ShaderDataType::Int2:
		case Acorn::ShaderDataType::Int3:
		case Acorn::ShaderDataType::Int4:
			return GL_INT;
		case Acorn::ShaderDataType::Bool:
			return GL_BOOL;
		default:
			AC_CORE_ASSERT(false, "Invalid ShaderDataType");
			return 0;
		}
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		AC_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererId);
	}

	void OpenGLVertexArray::Bind() const
	{
		AC_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererId);
	}

	void OpenGLVertexArray::Unbind() const
	{
		AC_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		AC_PROFILE_FUNCTION();

		AC_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "No Layout for Vertex Buffer Set");

		glBindVertexArray(m_RendererId);
		vertexBuffer->Bind();

		uint32_t index = 0;
		for (const auto& element : vertexBuffer->GetLayout())
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index++, 
				element.GetComponentCount(), 
				ToOpenGLType(element.Type), 
				element.Normalized ? GL_TRUE : GL_FALSE, 
				vertexBuffer->GetLayout().GetStride(), 
				(const void*)(intptr_t)element.Offset
			);
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		AC_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererId);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		AC_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererId);
	}

}