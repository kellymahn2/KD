#include "KDpch.h"
#include "ScriptEngine.h"
#include "ScriptRegistry.h"
#include "Kaidel/Scene/Scene.h"
#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Core/Application.h"
#include "Kaidel/Core/Buffer.h"


#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/tabledefs.h>
#include <glm/glm.hpp>
#include <FileWatch.h>

namespace Kaidel {
	
#pragma region Utils
	namespace Utils {
#define MapType(X,Y) {#X,ScriptFieldType::##Y},
#define InternalMapType(X) MapType(KaidelCore.##X,X)
#define InBuiltMapType(X,Y) MapType(System.##X,Y)
		static std::unordered_map< std::string, ScriptFieldType> s_ScriptFieldType = {
			InBuiltMapType(Single,Float)
			InBuiltMapType(Double,Double)
			InBuiltMapType(Int16,Short)
			InBuiltMapType(UInt16,UShort)
			InBuiltMapType(Int32,Int)
			InBuiltMapType(UInt32,UInt)
			InBuiltMapType(Int64,Long)
			InBuiltMapType(UInt64,ULong)	
			InBuiltMapType(Byte,Byte)
			InBuiltMapType(SByte,SByte)
			InBuiltMapType(Char,Char)
			InBuiltMapType(String,String)
			InBuiltMapType(Boolean,Bool)

			InternalMapType(Entity,Entity)
			InternalMapType(Vector2,Vector2)
			InternalMapType(Vector3,Vector3)
			InternalMapType(Vector4,Vector4)

		};
		static char* ReadBytes(const std::string& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				// Failed to open the file
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = (uint32_t)(end - stream.tellg());

			if (size == 0)
			{
				// File is empty
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = size;
			return buffer;
		}

		static MonoAssembly* LoadMonoAssembly(const std::string& assemblyPath)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath, &fileSize);

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				// Log some error message using the errorMessage data
				return nullptr;
			}

			MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
			mono_image_close(image);

			// Don't forget to free the file data
			delete[] fileData;

			return assembly;
		}

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* type) {
			const char* typeName = mono_type_get_name(type);
			if (s_ScriptFieldType.find(typeName) == s_ScriptFieldType.end())
				return ScriptFieldType::None;
			return s_ScriptFieldType.at(typeName);
		}
		const char* ScriptFieldTypeToString(ScriptFieldType fieldType) {
#define Switch(Type) case Kaidel::ScriptFieldType::##Type: return #Type
			switch (fieldType)
			{
				case Kaidel::ScriptFieldType::None:return "<Invalid>";
				Switch(Float);
				Switch(Double);
				Switch(Short);
				Switch(UShort);
				Switch(Int);
				Switch(UInt);
				Switch(Long);
				Switch(ULong);
				Switch(Byte);
				Switch(SByte);
				Switch(Char);
				Switch(String);
				Switch(Bool);
				Switch(Entity);
				Switch(Vector2);
				Switch(Vector3);
				Switch(Vector4);
			}
#undef Switch
			return "";
		}
	}
#pragma endregion
	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;
		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;
		
		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;
		ScriptClass EntityClass;
		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, std::unordered_map<std::string,Ref<ScriptInstance>>> EntityInstances;
		std::unordered_map<UUID, std::unordered_map<Ref<ScriptClass>,ScriptFieldMap>>	EntityScriptFields;
		std::filesystem::path CoreAssemblyFilePath;
		std::filesystem::path AppAssemblyFilePath;

		Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
		bool AssemblyReloadPending = false;
		bool ReloadClassFields = false;

		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* s_Data = nullptr;


	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();
		InitMono();
		LoadAssembly("Resources/Scripts/KaidelCore.dll");
		LoadAppAssembly("SandboxProject/assets/scripts/Binaries/Sandbox.dll");
		LoadAssemblyClasses(s_Data->AppAssembly);
		ScriptRegistry::RegisterComponents();
		ScriptRegistry::RegisterFunctions();
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_Data->SceneContext = scene;
		if (s_Data->ReloadClassFields) {

			for (auto& [entityID, allFields] : s_Data->EntityScriptFields) {

				auto cpyFields = allFields;

				for (auto& [klass, fieldMap] : cpyFields) {
					std::string fullName;
					if (!klass->m_Namespace.empty()) {
						fullName = fmt::format("{}.{}", klass->m_Namespace, klass->m_ClassName);
					}
					else {
						fullName = klass->m_ClassName;
					}
					if (s_Data->EntityClasses.find(fullName) == s_Data->EntityClasses.end()) {
						allFields.erase(klass);
						continue;
					}
					auto cpyFieldMap = fieldMap;
					for (auto& [fieldName, field] : cpyFieldMap) {
						if (klass->m_Fields.find(fieldName) == klass->m_Fields.end()) {
							fieldMap.erase(fieldName);
							continue;
						}
						fieldMap.at(fieldName).Field = klass->m_Fields.at(fieldName);
					}
				}
				allFields = std::move(cpyFields);
			}
			s_Data->ReloadClassFields = false;
		}
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->SceneContext = nullptr;
		s_Data->EntityInstances.clear();
		if (s_Data->AssemblyReloadPending) {
			Application::Get().SubmitToMainThread([]() {
				s_Data->AppAssemblyFileWatcher.reset();
				ScriptEngine::ReloadAssembly();
				});
		}
	}

	bool ScriptEngine::ClassExists(const std::string& fullName)
	{
		return s_Data->EntityClasses.find(fullName) != s_Data->EntityClasses.end();
	}

	void ScriptEngine::OnCreateEntity(Entity& entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();

		UUID entityID = entity.GetUUID();
		auto entityIT = s_Data->EntityScriptFields.find(entityID);
		for (auto& name : sc.ScriptNames) {
			if (ScriptEngine::ClassExists(name))
			{

				Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[name], entity);
				s_Data->EntityInstances[entityID][name] = instance;

				// Copy field values
				if (entityIT!= s_Data->EntityScriptFields.end())
				{
					for (auto& [fieldName, fieldInstance] : (*entityIT).second.at(s_Data->EntityClasses[name])) {
						instance->SetFieldValueImpl(fieldInstance.Field, fieldInstance.m_Data);
					}
				}

				instance->InvokeOnCreate();
			}
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity& entity, float ts)
	{
		KD_CORE_ASSERT(s_Data->EntityInstances.find(entity.GetUUID()) != s_Data->EntityInstances.end());
		for (auto& scripts : s_Data->EntityInstances.at(entity.GetUUID())) {
			scripts.second->InvokeOnUpdate(ts);
		}
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& path)
	{
		s_Data->CoreAssemblyFilePath = path;
		s_Data->AppDomain = mono_domain_create_appdomain("KaidelCoreRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);
		// Move this maybe
		s_Data->CoreAssembly = Utils::LoadMonoAssembly(path.string());
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		s_Data->EntityClass = ScriptClass("KaidelCore", "Entity", s_Data->CoreAssemblyImage);
	}
	static void OnAppAssemblyEvent(const std::string& path, const filewatch::Event eventType) {
		if (s_Data->SceneContext) {
			s_Data->AssemblyReloadPending = eventType == filewatch::Event::modified;
			return;
		}
		if (!s_Data->AssemblyReloadPending&&eventType == filewatch::Event::modified) {
			s_Data->AssemblyReloadPending = true;
			using namespace std::chrono_literals;
			Application::Get().SubmitToMainThread([]() {
				s_Data->AppAssemblyFileWatcher.reset();
				ScriptEngine::ReloadAssembly(); 
				});
		}
	}
	void ScriptEngine::LoadAppAssembly(const std::filesystem::path& path)
	{
		s_Data->AppAssemblyFilePath = path;
		s_Data->AppAssembly = Utils::LoadMonoAssembly(path.string());
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
		s_Data->AppAssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>(path.string(),&OnAppAssemblyEvent);
		s_Data->AssemblyReloadPending = false;
	}

	const std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetClasses()
	{
		return s_Data->EntityClasses;
	}





	void ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		std::unordered_map < std::string, Ref<ScriptClass>> classes;



		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			std::string fullName;
			if (strlen(nameSpace) != 0) {
				fullName = fmt::format("{}.{}", nameSpace, name);
			}
			else {
				fullName = name;
			}
			MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);
			if (monoClass == s_Data->EntityClass.GetClass())
				continue;
			bool isEntityDerived = mono_class_is_subclass_of(monoClass, s_Data->EntityClass.GetClass(), false);
			if (!isEntityDerived)
				continue;
			Ref<ScriptClass> scriptClass;
			if (s_Data->EntityClasses.find(fullName) != s_Data->EntityClasses.end())
			{
				scriptClass = classes[fullName] = s_Data->EntityClasses.at(fullName);
				scriptClass->SetClass(monoClass, name, nameSpace);
			}
			else {
				scriptClass = classes[fullName] = CreateRef<ScriptClass>(nameSpace,name,s_Data->AppAssemblyImage);
			}
			void* iterator = nullptr;
			//TODO: implement support classes getting deleted.
			scriptClass->m_Fields.clear();
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator)) {
				std::string name = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				if (flags & FIELD_ATTRIBUTE_PUBLIC) {
					MonoType* fieldType = mono_field_get_type(field);
					const char* typeName = Utils::ScriptFieldTypeToString(Utils::MonoTypeToScriptFieldType(fieldType));
					scriptClass->m_Fields[name] = { name,Utils::MonoTypeToScriptFieldType(fieldType) ,field};
				}
			}
		}
		s_Data->EntityClasses = std::move(classes);
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");
		MonoDomain* rootDomain = mono_jit_init("KaidelRuntime");
		KD_CORE_ASSERT(rootDomain);

		// Store the root domain pointer
		s_Data->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_Data->AppDomain);
		mono_jit_cleanup(s_Data->RootDomain);
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		KD_CORE_ASSERT(s_Data->CoreAssemblyImage, "Core Assembly not Loaded");
		return s_Data->CoreAssemblyImage;
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass) {
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	std::unordered_map< std::string , Ref<ScriptInstance >> ScriptEngine::GetEntityScriptInstances(UUID entityID)
	{
		auto it = s_Data->EntityInstances.find(entityID);
		if (it == s_Data->EntityInstances.end())
			return {};
		return it->second;
	}

	std::unordered_map<Ref<ScriptClass>, ScriptFieldMap>& ScriptEngine::GetScriptFieldMaps(UUID entityID)
	{
		return s_Data->EntityScriptFields[entityID];
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{
		if (s_Data->EntityClasses.find(name) == s_Data->EntityClasses.end())
			return nullptr;
		return s_Data->EntityClasses.at(name);
	}

	ScriptFieldInstance* ScriptEngine::AddScriptFieldInstance(UUID entityID, const std::string& name, ScriptFieldType type,Ref<ScriptClass>scriptClass)
	{
		if (scriptClass->m_Fields.find(name) == scriptClass->m_Fields.end())
			return nullptr;
		s_Data->EntityScriptFields[entityID][scriptClass][name].Field = scriptClass->GetFields().at(name);
		return &s_Data->EntityScriptFields.at(entityID).at(scriptClass).at(name);
	}
	//TODO: Make Reloading Work.
	void ScriptEngine::ReloadAssembly()
	{

		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_Data->AppDomain);
		//mono_domain_free(s_Data->AppDomain, true);
		{
			{
				LoadAssembly(s_Data->CoreAssemblyFilePath);
			}
			{
				LoadAppAssembly(s_Data->AppAssemblyFilePath);
			}
			{
				LoadAssemblyClasses(s_Data->AppAssembly);
			}
		}
		s_Data->EntityClass = ScriptClass("KaidelCore", "Entity", s_Data->CoreAssemblyImage);
		s_Data->ReloadClassFields = true;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className,MonoImage* image)
		:m_Namespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(image?image:s_Data->AppAssemblyImage, classNamespace.c_str(), className.c_str());
	}
MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, size_t parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), (int)parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance,MonoMethod* method, void** params)
	{
		return mono_runtime_invoke(method, instance, params,nullptr);
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		:m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();
		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = m_ScriptClass->GetMethod("OnUpdate", 1);
		UUID id = entity.GetUUID();
		void* params = &id;
		m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &params);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		void* params = &ts;
		m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &params);
	}

	void ScriptInstance::GetFieldValueImpl(const ScriptField& field, void* block)
	{
		mono_field_get_value(m_Instance, field.Field, block);
	}

	void ScriptInstance::SetFieldValueImpl(const ScriptField& field, const void* value)
	{
		mono_field_set_value(m_Instance, field.Field, (void*)value);
	}

	const char* ScriptInstance::GetStringFieldValueImpl(const ScriptField& field)
	{
		MonoString* b = nullptr;
		//MonoString* b = nullptr;
		mono_field_get_value(m_Instance, field.Field, &b);
		const char* s = mono_string_to_utf8(b);
		return s;
	}

	void ScriptInstance::SetStringFieldValueImpl(const ScriptField& field, const char* value)
	{
		MonoString* str = mono_string_new_len(s_Data->AppDomain, value, (uint32_t)strlen(value));
		mono_field_set_value(m_Instance, field.Field, (void*)str);
	}


}
