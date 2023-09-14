#pragma once
#include "ScriptClass.h"
#include "MonoTypeDefinitions.h"
#include "Kaidel/Scene/Entity.h"
#include "ScriptField.h"
#include <string>
#include <unordered_map>
namespace Kaidel {
	
	class ScriptInstance {
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);
		MonoObject* GetInstance() { return m_Instance; }
		void InvokeOnCreate();
		void InvokeOnUpdate(float ts);

		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }
		template<typename T>
		T GetFieldValue(const ScriptField& field) {
			GetFieldValueImpl(field, s_MemoryBuffer);
			return *((T*)(char*)s_MemoryBuffer);
		}
		template<>
		char* GetFieldValue<char*>(const ScriptField& field) {
			const char* s = GetStringFieldValueImpl(field);
			strcpy(s_MemoryBuffer, s);
			return s_MemoryBuffer;
		}
		template<typename T>
		void SetFieldValue(const ScriptField& field, const T& value) {
			SetFieldValueImpl(field, &value);
		}
		template<>
		void SetFieldValue<std::string>(const ScriptField& field, const std::string& value) {
			SetStringFieldValueImpl(field, value.c_str());
		}
	private:
		void GetFieldValueImpl(const ScriptField& field, void* block);
		const char* GetStringFieldValueImpl(const ScriptField& field);
		void SetStringFieldValueImpl(const ScriptField& field, const char* value);
		void SetFieldValueImpl(const ScriptField& field, const void* value);
	private:
		Ref<ScriptClass> m_ScriptClass;
		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
		static inline char s_MemoryBuffer[512] = { 0 };
		friend class ScriptEngine;
	};
}
