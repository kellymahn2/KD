#pragma once
#include <filesystem>
#include <string>
#include "Kaidel/Core/Base.h"
#include "Kaidel/Core/UUID.h"

#include "ScriptInstance.h"
#include "ScriptClass.h"
#include <unordered_map>
#include "MonoTypeDefinitions.h"
namespace Kaidel {

	class Scene;
	class Entity;
	class ScriptEngine;
	
	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;
	
	template<typename T>
	class MemoryBuffer {
	public:
		MemoryBuffer() {
			m_Block = (T*)malloc(sizeof(T));

		}
		~MemoryBuffer() {
			free(m_Block);
		}
		T* GetBlock() { return m_Block; }
		T* operator->() { return m_Block; }
		const T* operator->()const { return m_Block; }

		T& operator*() {
			return *m_Block;
		}
		const T& operator*()const { return *m_Block; }
	private:
		T* m_Block;
	};
	
	class ScriptEngine {
	public:
		static void Init();
		static void Shutdown();
		static void LoadAssembly(const std::filesystem::path& path);
		static void ReloadAssembly();
		static void LoadAppAssembly(const std::filesystem::path& path);
		static const std::unordered_map<std::string, Ref<ScriptClass>>& GetClasses();
		static Ref<ScriptClass> GetEntityClass(const std::string& name);
		static  std::unordered_map<Ref<ScriptClass>,ScriptFieldMap>& GetScriptFieldMaps(UUID entityID);
		static std::unordered_map<std::string,Ref<ScriptInstance>> GetEntityScriptInstances(UUID entityID);
		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();
		static bool ClassExists(const std::string& fullName);
		static void OnCreateEntity(Entity& entity);
		static void OnUpdateEntity(Entity& entity, float ts);
		static Scene* GetSceneContext();
		static MonoImage* GetCoreAssemblyImage();
		template<typename T>
		static void AddSerializedField(UUID entityID, const std::string& name, ScriptFieldType type, Ref<ScriptClass> scriptClass,const T& data) {
			ScriptFieldInstance* sfi = AddScriptFieldInstance(entityID, name, type, scriptClass);
			if(sfi != nullptr)
			sfi->SetValue<T>(data);
		}
		static void LoadAssemblyClasses(MonoAssembly* assembly);
	private:
		static ScriptFieldInstance* AddScriptFieldInstance(UUID entityID, const std::string& name, ScriptFieldType type,Ref<ScriptClass> scriptClass);
		static void InitMono();
		static void ShutdownMono();
	};



}
