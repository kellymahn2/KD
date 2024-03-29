#pragma once

#include "Core.h"
#include "Kaidel/Core/Base.h"
#include "Kaidel/Assets/AssetManager.h"
#include <string>
#include <unordered_map>
namespace Kaidel {

	//TODO : Store Handles
	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint64_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void Reset(void* data, uint32_t width, uint32_t height) = 0;
		virtual const std::string& GetPath()const = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	struct TextureHandle;

	class Texture2D : public Texture , public _Asset
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height,TextureFormat format);
		static Ref<Texture2D> Create(const std::string& path);

		virtual TextureHandle GetHandle()const = 0;

		virtual TextureFormat GetFormat()const = 0;

		ASSET_EXTENSION_TYPE(Texture2D)

	protected:
		static std::unordered_map < std::string , Ref<Texture2D>> s_Map;
	};

	struct TextureHandle {
		Ref<Texture2D> Texture;
	};
}
