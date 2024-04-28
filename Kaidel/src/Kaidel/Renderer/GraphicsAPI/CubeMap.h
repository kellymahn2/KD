#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/GraphicsAPI/Constants.h"
#include <vector>
namespace Kaidel {

	enum class CubeMapSide {
		Right,
		Left,
		Top,
		Bottom,
		Back,
		Front,
	};


	class CubeMap : public IRCCounter<false> {
	public:

		virtual ~CubeMap() = default;

		static Ref<CubeMap> Create(uint32_t width,uint32_t height,TextureFormat format);
		virtual void Bind(uint32_t slot = 0)const = 0;
		virtual void SetData(CubeMapSide side, void* data, uint32_t width, uint32_t height) = 0;
		virtual void SetData(CubeMapSide side,const std::string& src, bool shouldFlip) = 0;
		virtual void SetAll(const std::string& src, bool shouldFlip) = 0;
		virtual void SetAll(void* data,uint32_t width,uint32_t height) = 0;
		virtual uint32_t GetWidth()const = 0;
		virtual uint32_t GetHeight()const = 0;
	};
}
