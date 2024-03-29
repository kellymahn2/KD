#pragma once
#include "Kaidel/Core/Base.h"

#include "TextureView.h"
#include "Kaidel/Assets/AssetManager.h"
#include "Core.h"
#include <string>
namespace Kaidel {

	
	struct TextureArrayHandle;

	struct ArraySlotSpecification {
		uint32_t LoadedWidth = 0, LoadedHeight = 0;
		bool ShouldFlip = false;

		std::string Source = "";

		std::string Data = "";

		bool Valid = false;
	};


	class Texture2DArray : public _Asset {
	public:
		virtual ~Texture2DArray() = default;

		virtual void SetTextureData(void* data, uint32_t width, uint32_t height, uint32_t index) = 0;
		virtual void Bind(uint32_t slot = 0)const = 0;

		virtual uint32_t PushTexture(void* data, uint32_t width, uint32_t height, bool shouldFlip = true) = 0;
		virtual uint32_t PushTexture(const std::string& src, bool shouldFlip = true) = 0;

		virtual void InsertTexture(uint32_t slot,void* data, uint32_t width, uint32_t height, bool shouldFlip = true) = 0;
		virtual void InsertTexture(uint32_t slot,const std::string& src, bool shouldFlip = true) = 0;

		virtual void InsertOrReplaceTexture(uint32_t slot, void* data, uint32_t width, uint32_t height, bool shouldFlip = true) = 0;
		virtual void InsertOrReplaceTexture(uint32_t slot, const std::string& src, bool shouldFlip = true) = 0;



		virtual void ReplaceTexture(uint32_t slot, void* data, uint32_t width, uint32_t height, bool shouldFlip = true) = 0;
		virtual void ReplaceTexture(uint32_t slot, const std::string& src, bool shouldFlip = true) = 0;


		virtual TextureArrayHandle GetHandle(uint32_t index) const = 0;

		virtual Ref<TextureView> GetView(uint32_t index) = 0;

		virtual void ClearLayer(uint32_t slot, const float* data)const = 0;
		virtual void Clear(const float* data) const = 0;

		virtual TextureFormat GetFormat()const = 0;

		static Ref<Texture2DArray> Create(uint32_t width, uint32_t height, TextureFormat textureFormat, bool loadsSpecs = true);

		virtual const std::unordered_map<std::string, uint32_t>& GetLoadedTextures() = 0;

		virtual const std::vector<ArraySlotSpecification>& GetSpecifications()const = 0;

		virtual uint32_t GetWidth()const = 0;
		virtual uint32_t GetHeight()const = 0;
		virtual uint32_t GetLayerCount()const = 0;
		virtual uint32_t GetCapacity()const = 0;

	};


	struct TextureArrayHandle {
		Ref<Texture2DArray> Array;
		uint64_t SlotIndex;
	};
}
