#pragma once
#include "Texture.h"
namespace Kaidel {


	class TextureAtlas2D: public Texture2D{
	public:
		//static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<TextureAtlas2D> Create(const std::string& path);
		virtual ~TextureAtlas2D() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual const std::string& GetPath()const = 0;
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual bool operator==(const TextureAtlas2D& other) const = 0;

		virtual Ref<SubTexture2D> GetSubTexture(uint32_t x, uint32_t y)=0;
		
	protected:
		static std::unordered_map < std::string, Ref<TextureAtlas2D>> s_Map;

	};
}
