#pragma once
#include "Kaidel/Core/Application.h"
#include "Kaidel/Core/Base.h"
#include "Kaidel/Events/SettingsEvent.h"
#include <stack>
namespace Kaidel {

	enum class AntiAiliasingType {
		None,
		FXAA,
		MSAA
	};
	enum class CullMode {
		None, Front, Back, FrontAndBack
	};

	template<typename T>
	struct Setting {
		std::deque<T> Values;
		Setting(const T& val)
		{
			Set(val);
		}
		void Set(const T& val) {
			if (Values.size() >= 2) {
				Values.pop_front();
				Values.push_back(val);
			}
			else {
				Values.push_back(val);
			}
		}

		bool Changed() {
			return Values.size() >= 2;
		}

		const T& Get() {
			return Values.back();
		}
		const T& GetLast() {
			return Values.front();
		}
	};

	struct RendererSettings {
		AntiAiliasingType AntiAiliasing = AntiAiliasingType::MSAA;
		uint32_t MSAASampleCount = 4;
		CullMode Culling = CullMode::None;

		void SetInShaders(uint32_t slot)const;

	};

	struct HardwareConstants {
		uint32_t MaxMSAASampleCount;
	};

}
