#pragma once
#include <filesystem>
#include <string>
#include "Kaidel/Core/Base.h"
#include <unordered_map>
extern "C" {
	typedef struct _MonoClass		MonoClass;
	typedef struct _MonoObject		MonoObject;
	typedef struct _MonoMethod		MonoMethod;
	typedef struct _MonoImage		MonoImage;
	typedef struct _MonoAssembly	MonoAssembly;
}
namespace Kaidel {
	class Scene;
	class Entity;
	class ScriptClass {
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className,MonoImage* image = nullptr);
		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, size_t parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
		MonoClass* GetClass()const { return m_MonoClass; }
	private:
		std::string m_Namespace;
		std::string m_ClassName;
		MonoClass* m_MonoClass = nullptr;
	};
	class ScriptInstance {
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass,Entity entity);
		void InvokeOnCreate();
		void InvokeOnUpdate(float ts);
	private:
		Ref<ScriptClass> m_ScriptClass;
		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor= nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
	};
	class ScriptEngine {
	public:
		static void Init();
		static void Shutdown();
		static void LoadAssembly(const std::filesystem::path& path);
		static void LoadAppAssembly(const std::filesystem::path& path);
		static const std::unordered_map<std::string, Ref<ScriptClass>>& GetClasses();
		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();
		static bool ClassExists(const std::string& fullName);
		static void OnCreateEntity(Entity& entity);
		static void OnUpdateEntity(Entity& entity, float ts);
		static Scene* GetSceneContext();
		static MonoImage* GetCoreAssemblyImage();
	private:
		static void LoadAssemblyClasses(MonoAssembly* assembly);
		static void InitMono();
		static void ShutdownMono();
	};
	

}
