#pragma once

#include "Kaidel/Core/Base.h"
#include "AnimationProperty.h"

namespace Kaidel {

	namespace AnimationPropertyNames {
		static const char* PositionX = "Position.X";
		static const char* PositionY = "Position.Y";
		static const char* PositionZ = "Position.Z";

		static const char* RotationX = "Rotation.X";
		static const char* RotationY = "Rotation.Y";
		static const char* RotationZ = "Rotation.Z";

		static const char* ScaleX = "Scale.X";
		static const char* ScaleY = "Scale.Y";
		static const char* ScaleZ = "Scale.Z";
	}

	class AnimationData {
	public:
		AnimationData() = default;

		bool HasProperty(const char* name) { return m_Properties.find(name) != m_Properties.end(); }

		AnimationProperty& AddProperty(const char* name) {
			return m_Properties[name] = {};
		}

		AnimationProperty& GetProperty(const char* name) {
			KD_CORE_ASSERT(HasProperty(name));
			return m_Properties.at(name);
		}

		AnimationProperty* TryGetProperty(const char* name) {
			auto it = m_Properties.find(name);
			if (it != m_Properties.end())
				return &it->second;
			return nullptr;
		}
	private:
		std::unordered_map<const char*, AnimationProperty> m_Properties;
	};
}
