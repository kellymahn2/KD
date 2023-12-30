#pragma once

#include <string>
#include <unordered_map>
#include "Kaidel/Core/Base.h"

namespace Kaidel {

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
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::string& path);
	protected:
		static std::unordered_map < std::string , Ref<Texture2D>> s_Map;
	};

	class Texture2DArray {
	public:
		virtual ~Texture2DArray() = default;

		
		virtual void SetTextureData(void* data, uint32_t width,uint32_t height,uint32_t index) = 0;
		virtual void Bind(uint32_t slot = 0)const = 0;
		virtual uint32_t PushTexture(void* data, uint32_t width,uint32_t height) = 0;
		virtual uint32_t PushTexture(const std::string& src) = 0;
		static Ref<Texture2DArray> Create(uint32_t width, uint32_t height);
	};

	class Depth2DArray {
	public:
		virtual ~Depth2DArray() = default;

		static Ref<Depth2DArray> Create(uint32_t width, uint32_t height);

		virtual void Bind(uint32_t slot = 0)const = 0;
		virtual void ClearLayer(uint32_t index, float value) = 0;
		virtual uint32_t PushDepth(uint32_t width,uint32_t height) = 0;
	};


}
