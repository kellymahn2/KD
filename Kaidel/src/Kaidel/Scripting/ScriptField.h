#pragma once
#include "MonoTypeDefinitions.h"
#include <string>
#include <memory>
namespace Kaidel {


	enum class ScriptFieldType :uint64_t {
		None = 0,
		Float, Double,
		Short, UShort, Int, UInt, Long, ULong,
		Byte, SByte, Char, String,
		Bool,


		Entity,
		Vector2, Vector3, Vector4,

	};
	struct ScriptField {
		std::string Name;
		ScriptFieldType Type;
		MonoClassField* Field;
	};
	struct ScriptFieldInstance {
		ScriptField Field;
		template<typename T>
		T GetValue() const {
			KD_CORE_ASSERT(sizeof(T) <= sizeof(m_Data));
			return *(T*)m_Data;
		}
		template<typename T>
		void SetValue(const T& value) const {
			KD_CORE_ASSERT(sizeof(T) <= sizeof(m_Data));
			memcpy(m_Data, &value, sizeof(T));
		}
	private:
		mutable char m_Data[32];
		friend class ScriptEngine;
	};
}
