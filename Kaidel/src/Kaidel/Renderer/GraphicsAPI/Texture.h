#pragma once

#include <string>
#include <unordered_map>
#include "Kaidel/Core/Base.h"

namespace Kaidel {


	enum class TextureFormat {
		None = 0,

		RGBA32F,//4-component 128-bits
		RGBA32UI,//4-component 128-bits
		RGBA32I,//4-component 128-bits

		RGB32F,//3-component 96-bits
		RGB32UI,//3-component 96-bits
		RGB32I,//3-component 96-bits

		RGBA16F,//4-component 64-bits
		RGBA16,//4-component 64-bits
		RGBA16UI,//4-component 64-bits
		RGBA16NORM,//4-component 64-bits
		RGBA16I,//4-component 64-bits

		RG32F,//2-component 64-bits
		RG32UI,//2-component 64-bits
		RG32I,//2-component 64-bits

		RGBA8,//4-component 32-bits
		RGBA8UI,//4-component 32-bits
		RGBA8NORM,//4-component 32-bits
		RGBA8I,//4-component 32-bits

		RG16F,//2-component 32-bits
		RG16,//2-component 32-bits
		RG16UI,//2-component 32-bits
		RG16NORM,//2-component 32-bits
		RG16I,//2-component 32-bits

		R32F,//1-component 32-bits
		R32UI,//1-component 32-bits
		R32I,//1-component 32-bits

		RG8,//2-component 16-bits
		RG8UI,//2-component 16-bits
		RG8NORM,//2-component 16-bits
		RG8I,//2-component 16-bits

		R16F,//1-component 16-bits
		R16,//1-component 16-bits
		R16UI,//1-component 16-bits
		R16NORM,//1-component 16-bits
		R16I,//1-component 16-bits

		R8,//1-component 8-bits
		R8UI,//1-component 8-bits
		R8NORM,//1-component 8-bits
		R8I,//1-component 8-bits

	};


	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint64_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual const std::string& GetPath()const = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height,TextureFormat format);
		static Ref<Texture2D> Create(const std::string& path);
	protected:
		static std::unordered_map < std::string , Ref<Texture2D>> s_Map;
	};

	class Texture2DArray {
	public:
		virtual ~Texture2DArray() = default;

		
		virtual void SetTextureData(void* data, uint32_t width,uint32_t height,uint32_t index) = 0;
		virtual void Bind(uint32_t slot = 0)const = 0;
		virtual uint32_t PushTexture(void* data, uint32_t width,uint32_t height,bool shouldFlip = true) = 0;
		virtual uint32_t PushTexture(const std::string& src,bool shouldFlip = true) = 0;
		static Ref<Texture2DArray> Create(uint32_t width, uint32_t height);
	};

	class Depth2DArray {
	public:
		virtual ~Depth2DArray() = default;

		static Ref<Depth2DArray> Create(uint32_t width, uint32_t height);

		virtual void Bind(uint32_t slot = 0)const = 0;
		virtual void ClearLayer(uint32_t index, float value) = 0;
		virtual uint32_t PushDepth(uint32_t width,uint32_t height) = 0;
		virtual void PopDepth() = 0;
		virtual uint32_t GetSize()const = 0;
		virtual uint32_t GetRendererID() const = 0;
	};


}
