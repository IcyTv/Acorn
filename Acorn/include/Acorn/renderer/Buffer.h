#pragma once

#include "core/Core.h"
#include <string>
#include <vector>

namespace Acorn
{
	enum class ShaderDataType : uint8_t
	{
		None = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4,
		Int,
		Int2,
		Int3,
		Int4,
		Bool,
	};

	static uint32_t operator*(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::None:
				return 0;
			case ShaderDataType::Float:
				return 4;
			case ShaderDataType::Float2:
				return 8;
			case ShaderDataType::Float3:
				return 12;
			case ShaderDataType::Float4:
				return 16;
			case ShaderDataType::Mat3:
				return 4 * 3 * 3;
			case ShaderDataType::Mat4:
				return 4 * 4 * 4;
			case ShaderDataType::Int:
				return 4;
			case ShaderDataType::Int2:
				return 8;
			case ShaderDataType::Int3:
				return 12;
			case ShaderDataType::Int4:
				return 16;
			case ShaderDataType::Bool:
				return 1;
			default:
				AC_CORE_ASSERT(false, "Invalid ShaderDataType!");
				return 0;
		}
	}

	struct BufferElement
	{
	public:
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement()
			: Name(""), Type(ShaderDataType::None), Size(0), Offset(0), Normalized(false)
		{
		}

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(*type), Offset(0), Normalized(normalized)
		{
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::None:
					return 0;
				case ShaderDataType::Int:
				case ShaderDataType::Float:
				case ShaderDataType::Bool:
					return 1;
				case ShaderDataType::Float2:
				case ShaderDataType::Int2:
					return 2;
				case ShaderDataType::Float3:
				case ShaderDataType::Int3:
					return 3;
				case ShaderDataType::Float4:
				case ShaderDataType::Int4:
					return 4;
				case ShaderDataType::Mat3:
					return 3 * 3;
				case ShaderDataType::Mat4:
					return 4 * 4;
				default:
					AC_CORE_ASSERT(false, "Invalid Shader Data Type");
					return 0;
			}
		}
	};

	class BufferLayout
	{
	public:
		// TODO struct reflection
		BufferLayout() {}
		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		inline uint32_t GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetsAndStride()
		{
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = m_Stride;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual const void* GetData() const = 0;
		virtual void* GetDataPtr() const = 0;

		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual const BufferLayout& GetLayout() const = 0;

		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};
}