#pragma once
#include "Kaidel/Core/Base.h"
#include <vector>
namespace Kaidel {

	enum CubeMapSide_ {
		None = 0,
		CubeMapSide_Left = 1<<0,
		CubeMapSide_Right = 1 <<1,
		CubeMapSide_Top = 1 <<2 ,
		CubeMapSide_Bottom = 1<<3,
		CubeMapSide_Front = 1 <<4,
		CubeMapSide_Back = 1<<5
	};

	struct CubeMapSide {
		CubeMapSide_ Side;
		void* Data;
	};

	class CubeMap {
	public:
		static Ref<CubeMap> Create(uint32_t width,uint32_t height);
		virtual void Bind(uint32_t slot = 0)const = 0;
		virtual void SetData(const std::vector<CubeMapSide>& sides,uint32_t width,uint32_t height) = 0;
		virtual void SetAll(void* data,uint32_t width,uint32_t height) = 0;
		virtual uint32_t GetWidth()const = 0;
		virtual uint32_t GetHeight()const = 0;

	};
}
