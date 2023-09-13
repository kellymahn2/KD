#pragma once
#include <filesystem>
#include <string>
#include "Kaidel/Core/Base.h"
#include "Kaidel/Core/UUID.h"
#include <unordered_map>
extern "C" {
	typedef struct _MonoClass		MonoClass;
	typedef struct _MonoObject		MonoObject;
	typedef struct _MonoMethod		MonoMethod;
	typedef struct _MonoImage		MonoImage;
	typedef struct _MonoAssembly	MonoAssembly;
	typedef struct _MonoClassField MonoClassField;
}
namespace Kaidel {
	enum class ScriptFieldType :uint64_t{
		None = 0,
		Float, Double,
		Short, UShort, Int, UInt, Long, ULong,
		Byte, SByte, Char,String,
		Bool,


		Entity,
		Vector2,Vector3,Vector4,
	
	};
	class Scene;
	class Entity;
	class ScriptEngine;
	struct ScriptField {
		std::string Name;
		ScriptFieldType Type;
		MonoClassField* Field;
	};
	class ScriptClass {
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className,MonoImage* image = nullptr);
		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, size_t parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
		MonoClass* GetClass()const { return m_MonoClass; }

		const std::unordered_map<std::string, ScriptField>& GetFields() { return m_Fields; }
	private:
		std::string m_Namespace;
		std::string m_ClassName;
		std::unordered_map<std::string, ScriptField> m_Fields;
		MonoClass* m_MonoClass = nullptr;
		friend class ScriptEngine;
	};
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
	class ScriptInstance {
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass,Entity entity);
		MonoObject* GetInstance() { return m_Instance; }
		void InvokeOnCreate();
		void InvokeOnUpdate(float ts);

		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }
		template<typename T>
		T GetFieldValue(const ScriptField& field) {
			MemoryBuffer<T> buffer;
			GetFieldValueImpl(field, buffer.GetBlock());
			return *buffer;
		}
		template<typename T>
		void SetFieldValue(const ScriptField& field, const T& value) {
			SetFieldValueImpl(field, &value);
		}
	private:
		void GetFieldValueImpl(const ScriptField& field,void* block);
		void SetFieldValueImpl(const ScriptField& field, const void* value);
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
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);
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
