#pragma once
#include "MonoTypeDefinitions.h"
#include "Kaidel/Scene/Entity.h"
#include "ScriptField.h"
#include <string>
#include <unordered_map>
namespace Kaidel {
	class ScriptClass : public IRCCounter<false> {
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, MonoImage* image = nullptr);
		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, size_t parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
		MonoClass* GetClass()const { return m_MonoClass; }
		void SetClass(MonoClass* klass, const std::string& className, const std::string& nameSpace) {
			m_MonoClass = klass; 
			m_ClassName = className;
			m_Namespace = nameSpace;
		}
		const std::unordered_map<std::string, ScriptField>& GetFields() { return m_Fields; }
	private:
		std::string m_Namespace;
		std::string m_ClassName;
		std::unordered_map<std::string, ScriptField> m_Fields;
		MonoClass* m_MonoClass = nullptr;
		friend class ScriptEngine;
	};
}
