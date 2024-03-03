#pragma once
#include "Kaidel/Core/Base.h"
#include <vector>
namespace Kaidel {

	enum class CubeMapSide {
		None = 0,
		Left = 1<<0,
		Right = 1 <<1,
		Top = 1 <<2 ,
		Bottom = 1<<3,
		Front = 1 <<4,
		Back = 1<<5
	};

	struct CubeMapSideSpecification {
		CubeMapSide Side;
		void* Data;
		uint32_t width;
		uint32_t height;
	};

	class CubeMap : public IRCCounter<false> {
	public:
		static Ref<CubeMap> Create(uint32_t width,uint32_t height);
		virtual void Bind(uint32_t slot = 0)const = 0;
		virtual void SetData(const std::array<CubeMapSideSpecification,6>& sides,uint32_t sideCount,uint32_t width,uint32_t height) = 0;
		virtual void SetAll(void* data,uint32_t width,uint32_t height) = 0;
		virtual uint32_t GetWidth()const = 0;
		virtual uint32_t GetHeight()const = 0;
		
	};
}
