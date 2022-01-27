#pragma once

#include <string>

#include "core/Core.h"

namespace Acorn
{

	enum class TextureFiltering
	{
		Nearest,
		Linear,
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void SetSubData(void* data, uint32_t dataSize, uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetRendererId() const = 0;

		// Fixme temporary, should be moved to instantiation parameters
		virtual void SetTextureFiltering(TextureFiltering filtering) = 0;

		virtual std::string GetPath() const = 0;

		virtual void Bind(uint8_t slot = 0) const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2d : public Texture
	{
	public:
		static Ref<Texture2d> Create(uint32_t width, uint32_t height, uint32_t bpp);
		static Ref<Texture2d> Create(uint32_t width, uint32_t height);
		static Ref<Texture2d> Create(const std::string& path, uint32_t width, uint32_t height);
		static Ref<Texture2d> Create(const std::string& path);

		static Ref<Texture2d> FromRenderId(uint32_t id);
	};

	class AsyncTextureLoader
	{
	public:
		virtual void Load() = 0;
		virtual Ref<Texture2d> Upload() = 0;

		virtual ~AsyncTextureLoader(){};

		static Ref<AsyncTextureLoader> Create(const std::string& path, int width, int height);
	};
}