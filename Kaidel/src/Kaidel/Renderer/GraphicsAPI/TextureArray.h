#pragma once
#include "Kaidel/Core/Base.h"
#include "Core.h"
#include <string>
namespace Kaidel {

	
	struct TextureArrayHandle;

	class Texture2DArray : public IRCCounter<false> {
	public:
		virtual ~Texture2DArray() = default;

		virtual void SetTextureData(void* data, uint32_t width, uint32_t height, uint32_t index) = 0;
		virtual void Bind(uint32_t slot = 0)const = 0;

		virtual uint32_t PushTexture(void* data, uint32_t width, uint32_t height, bool shouldFlip = true) = 0;
		virtual uint32_t PushTexture(const std::string& src, bool shouldFlip = true) = 0;

		virtual TextureArrayHandle GetHandle(uint32_t index) const = 0;

		virtual void ClearLayer(uint32_t slot, const float* data)const = 0;
		virtual void Clear(const float* data) const = 0;


		static Ref<Texture2DArray> Create(uint32_t width, uint32_t height, TextureFormat textureFormat);
	};


	struct TextureArrayHandle {
		Ref<Texture2DArray> Array;
		uint64_t SlotIndex;
	};
}
