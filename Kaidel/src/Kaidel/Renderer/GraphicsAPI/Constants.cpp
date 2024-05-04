#include "KDpch.h"
#include "Constants.h"



namespace Kaidel {
	namespace Utils {
		bool IsDepthFormat(TextureFormat format) {
			switch (format)
			{
			case Kaidel::TextureFormat::Depth16:
			case Kaidel::TextureFormat::Depth32F:
			case Kaidel::TextureFormat::Depth32:
			case Kaidel::TextureFormat::Depth24Stencil8:
				return true;
			}
			return false;
		}
	}
}
