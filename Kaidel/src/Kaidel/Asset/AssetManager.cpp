#include "KDpch.h"
#include "AssetManager.h"

#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "Kaidel/Asset/AssetImporters/MaterialSerializer.h"
#include "Kaidel/Renderer/Text/FontLibrary.h"
#include "Kaidel/Scene/ModelLoader.h"
#include "Kaidel/Project/Project.h"

#include "AssetImporters/MetadataFile.h"

#include "Kaidel/Renderer/Renderer3D.h"

#include <yaml-cpp/yaml.h>

namespace Kaidel {

	static std::shared_mutex s_Mutex;

	struct AssetManagerData {
		JobSystemContext JobContext;
		
		//TotalMap = BuiltInMap + LoadedMap
		AssetMap TotalMap;
		
		AssetMap BuiltInMap;
		AssetMap LoadedMap;

		std::unordered_map<AssetHandle, AssetMetadata> BuiltInRegistry;

		std::unordered_map<AssetHandle, AssetMetadata> Registry;
		std::unordered_map<Path, AssetHandle> ReverseRegistry;

		std::unordered_map<AssetHandle, Ref<Texture2D>> Previews;

	} static* s_Data;


	namespace Utils {
		static std::string AssetTypeToString(AssetType type)
		{
			switch (type)
			{
			case Kaidel::AssetType::Texture2D: return "Texture2D";
			case Kaidel::AssetType::Font: return "Font";
			case Kaidel::AssetType::Model: return "Model";
			case Kaidel::AssetType::Mesh: return "Mesh";
			case Kaidel::AssetType::StandardMaterial: return "StandardMaterial";
			case Kaidel::AssetType::VisualMaterial: return "VisualMaterial";
			}
		}

		static AssetType StringToAssetType(const std::string& name)
		{
#define DECL_MAP(type) {#type, AssetType::##type}
			static std::unordered_map<std::string, AssetType> s_Map =
			{
				DECL_MAP(Texture2D),
				DECL_MAP(Font),
				DECL_MAP(Model),
				DECL_MAP(Mesh),
				DECL_MAP(StandardMaterial),
				DECL_MAP(VisualMaterial)
			};
#undef DECL_MAP

			return s_Map.at(name);
		}

		static bool IsAncestorOf(const FileSystem::path& ancestor, const FileSystem::path& child)
		{
			FileSystem::path a = FileSystem::canonical(ancestor);
			FileSystem::path c = FileSystem::canonical(child);

			if (a.is_relative())
				a = FileSystem::absolute(a);

			if (c.is_relative())
				c = FileSystem::absolute(c);

			if (a == c)
			{
				return true;
			}

			auto aIt = a.begin();
			auto cIt = c.begin();

			while (aIt != a.end() && cIt != c.end())
			{
				if (*aIt != *cIt)
					return false;
				++aIt;
				++cIt;
			}


			return aIt == a.end();
		}

	}

	void AssetManager::Init()
	{
		s_Data = new AssetManagerData;
	}

	void AssetManager::Shutdown()
	{
		delete s_Data;
	}


	const Kaidel::AssetMetadata& AssetManager::GetAssetMetadata(AssetHandle handle)
	{
		std::shared_lock<std::shared_mutex> lock(s_Mutex);

		KD_ASSERT(IsAssetHandleValid(handle));

		if (auto it = s_Data->BuiltInRegistry.find(handle); it != s_Data->BuiltInRegistry.end())
		{
			return it->second;
		}

		return s_Data->Registry.at(handle);
	}

	Asset AssetManager::GetAsset(AssetHandle handle)
	{
		std::shared_lock<std::shared_mutex> lock(s_Mutex);

		KD_ASSERT(IsAssetHandleValid(handle));

		Ref<AssetBase> ptr = s_Data->TotalMap.at(handle);

		return Asset(ptr);
	}


	Kaidel::Asset AssetManager::GetAssetAtPath(const Path& path)
	{
		std::shared_lock<std::shared_mutex> lock(s_Mutex);

		auto it = s_Data->ReverseRegistry.find(path);

		if (it != s_Data->ReverseRegistry.end())
			return s_Data->TotalMap.at(it->second);

		return {};
	}

	bool AssetManager::IsAssetHandleValid(AssetHandle handle)
	{
		std::shared_lock<std::shared_mutex> lock(s_Mutex);

		auto it = s_Data->TotalMap.find(handle);

		return it != s_Data->TotalMap.end();
	}


	bool AssetManager::IsBuiltIn(AssetHandle handle)
	{
		//Hasn't been loaded yet, clearly not a builtin
		if (handle == UUID(0))
			return false;

		return s_Data->BuiltInMap.find(handle) != s_Data->BuiltInMap.end();
	}

	void AssetManager::RegisterBuiltIn(Ref<AssetBase> asset, const std::string& name)
	{
		AssetMetadata mtd;
		mtd.AssetName = name;
		mtd.Handle = s_Data->BuiltInMap.size() + 1;

		std::unique_lock<std::shared_mutex> lock(s_Mutex);

		asset->m_AssetHandle = mtd.Handle;
		s_Data->BuiltInRegistry[mtd.Handle] = mtd;
		s_Data->TotalMap[mtd.Handle] = asset;
		s_Data->BuiltInMap[mtd.Handle] = std::move(asset);
	}

	void AssetManager::RegisterAsset(Ref<AssetBase> asset, const AssetMetadata& metadata)
	{
		std::unique_lock<std::shared_mutex> lock(s_Mutex);

		auto& data = s_Data;

		s_Data->Registry[metadata.Handle] = metadata;
		s_Data->ReverseRegistry[metadata.Path] = metadata.Handle;
		asset->m_AssetHandle = metadata.Handle;
		s_Data->TotalMap[metadata.Handle] = asset;
		s_Data->LoadedMap[metadata.Handle] = std::move(asset);
	}

	Asset AssetManager::LoadAsset(const AssetMetadata& metadata, AssetType type, OnLoadFunction onLoad, AssetLoadPolicy loadPolicy)
	{
		if (auto asset = GetAssetAtPath(metadata.Path); asset.AssetPtr)
		{
			return asset;
		}

		s_Mutex.lock();
		//Recheck to make sure no one has loaded it before this thread
		if (auto it = s_Data->ReverseRegistry.find(metadata.Path); it != s_Data->ReverseRegistry.end())
		{
			Asset asset = s_Data->LoadedMap.at(it->second);
			if (onLoad)
			{
				if (asset.AssetPtr->m_AssetHandle == UUID(0))
				{
					//TODO: figure out a way to add this to the current onLoad list.
				}
				else
				{
					onLoad(asset);
				}
			}
			s_Mutex.unlock();

			return asset;
		}

		AssetHandle handle = metadata.Handle;

		MetadataFile* file = new MetadataFile(metadata.Path);

		file->Resolve(handle);
		
		Ref<AssetBase>& ptr = s_Data->TotalMap[handle];
		Ref<AssetBase>& ptrLoaded = s_Data->LoadedMap[handle];
		AssetMetadata& mtd = s_Data->Registry[handle] = metadata;
		s_Data->ReverseRegistry[metadata.Path] = handle;
		mtd.Handle = handle;
		mtd.AssetName = metadata.Path.filename().string();
		
		s_Mutex.unlock();

		AssetManagerLoadSpecification loadSpecs;
		loadSpecs.LoadPolicy = loadPolicy;
		loadSpecs.Metadata = &mtd;
		loadSpecs.OnLoad = std::move(onLoad);
		loadSpecs.Ptr = &ptr;
		loadSpecs.PtrToLoaded = &ptrLoaded;
		loadSpecs.Type = type;
		loadSpecs.MtdFile = file;
		loadSpecs.OnLoad = [](Asset) {};

		LoadAssetLocked(loadSpecs);

		return Asset(ptr);
	}

	AssetMapIterator AssetManager::BeginTotal()
	{
		s_Mutex.lock_shared();

		return AssetMapIterator{ s_Data->TotalMap };
	}

	Kaidel::AssetMapIterator AssetManager::BeginBuiltIn()
	{
		s_Mutex.lock_shared();

		return AssetMapIterator{ s_Data->BuiltInMap };
	}

	Kaidel::AssetMapIterator AssetManager::BeginLoaded()
	{
		s_Mutex.lock_shared();

		return AssetMapIterator{ s_Data->LoadedMap };
	}

	void AssetManager::Serialize(const FileSystem::path& path, Asset asset)
	{
		MetadataFile mtdFile(path);

		if (mtdFile.IsCreated())
			mtdFile.SerializeHandle(asset->GetHandle());

		const AssetMetadata& metadata = GetAssetMetadata(asset->GetHandle());

		if (metadata.Dirty)
			return;
		
		switch (asset->GetAssetType())
		{
		case AssetType::StandardMaterial:
		{
			MaterialSerializer().Serialize(path, asset.As<StandardMaterialInstance>());
		} break;
		}

		SetDirty(asset->GetHandle(), false);
	}

	/*
	void AssetManager::Serialize(const FileSystem::path& path)
	{
		//Wait for everything to be loaded. might be better to have a canceling system.
		s_Data->JobContext.Wait();
		s_Mutex.lock_shared();

		YAML::Emitter out;

		out << YAML::BeginSeq;

		for (auto& [id, asset] : s_Data->LoadedMap)
		{
			const AssetMetadata& metadata = s_Data->Registry[id];

			AssetType type = asset->GetAssetType();
			
			out << YAML::BeginMap;
		
			out << YAML::Key << "Asset" << YAML::Value << (uint64_t)metadata.Handle;

			if ((uint64_t)metadata.RootHandle != 0)
			{
				out << YAML::Key << "Root" << YAML::Value << (uint64_t)metadata.RootHandle;
			}

			out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(type);

			out << YAML::Key << "URL" << YAML::Value << metadata.Path.string();

			out << YAML::EndMap;

		}

		out << YAML::EndSeq;

		s_Mutex.unlock_shared();

		std::ofstream file(path);
		file << out.c_str();
	}


	void AssetManager::Deserialize(const FileSystem::path& path, bool clear)
	{
		if (clear)
		{
			Clear();
		}

		s_Mutex.lock();


		YAML::Node data = YAML::Load(std::ifstream(path));

		for (auto assetNode : data)
		{
			UUID id = assetNode["Asset"].as<uint64_t>();

			//Parent is responsible for loading children.
			if (assetNode["Root"])
			{
				AssetMetadata metadata("", id);
				metadata.RootHandle = assetNode["Root"].as<uint64_t>();

				//RegisterAsset({}, metadata);
				continue;
			}

			std::string typeString = assetNode["Type"].as<std::string>();

			std::string path = assetNode["URL"].as<std::string>();

			AssetMetadata metadata = AssetMetadata(path, id);

			AssetType type = Utils::StringToAssetType(typeString);

			s_Data->Registry[metadata.Handle] = metadata;
			s_Data->ReverseRegistry[metadata.Path] = metadata.Handle;
			LoadAssetLocked(s_Data->TotalMap[id], s_Data->LoadedMap[id], metadata, type, [](Asset) {}, AssetLoadPolicy::Default);
		}

		s_Mutex.unlock();
	}

	*/

	void AssetManager::Wait()
	{
		s_Data->JobContext.Wait();
	}


	void AssetManager::WaitOnAssets(const std::vector<Asset>& assets, const std::chrono::milliseconds& timeout)
	{
		bool allTrue = false;
		uint32_t restartIndex = -1;

		for (auto& asset : assets)
		{
			if(!asset.AssetPtr)
				continue;

			//Not finished
			while (asset.AssetPtr->m_AssetHandle == UUID(0))
			{
				std::this_thread::sleep_for(timeout);
			}
		}
	}

	void AssetManager::Clear()
	{
		s_Mutex.lock();

		//Wait for everything to be loaded. would be better to have a canceling system.
		s_Data->JobContext.Wait();
		s_Data->LoadedMap.clear();
		s_Data->Registry.clear();
		s_Data->ReverseRegistry.clear();

		s_Data->TotalMap = s_Data->BuiltInMap;

		s_Mutex.unlock();
	}

	const std::unordered_map<std::string, Kaidel::AssetType>& AssetManager::GetSupportedAssets()
	{
		static const std::unordered_map<std::string, Kaidel::AssetType> s_Map = 
		{
			//Texture2D
			{".png", AssetType::Texture2D},
			{".jpg", AssetType::Texture2D},
			{".jpeg", AssetType::Texture2D},
			
			//Fonts
			{".ttf", AssetType::Font},

			//Models
			{".gltf", AssetType::Model},
			{".mat", AssetType::StandardMaterial}
			//{".fbx", AssetType::Model} // TODO: Add support
		};


		return s_Map;
	}


	uint64_t AssetManager::GetAssetCount()
	{
		return s_Data->TotalMap.size();
	}

	void AssetManager::SetAssetName(AssetHandle handle, const std::string& name)
	{
		s_Data->Registry[handle].AssetName = name;
	}

	void AssetManager::SetDirty(AssetHandle handle, bool dirty)
	{
		std::shared_lock<std::shared_mutex> lock(s_Mutex);

		KD_ASSERT(IsAssetHandleValid(handle));

		s_Data->Registry.at(handle).Dirty = dirty;
	}

	Ref<AssetBase> AssetManager::CreateUninitialized(AssetType type)
	{
		switch (type)
		{
		case Kaidel::AssetType::Texture2D:
		{
			return Texture2D::CreateUninitialized();
		} break;
		case Kaidel::AssetType::Font:
		{
			return CreateRef<Font>();
		} break;
		case Kaidel::AssetType::Model:
		{
			return CreateRef<Model>();
		} break;
		case Kaidel::AssetType::StandardMaterial:
		{
			return CreateRef<StandardMaterialInstance>(StandardMaterialInstance::Initializer{});
		} break;
		}

		KD_CORE_ASSERT(false);
		return {};
	}

	void AssetManager::Load(const AssetManagerLoadSpecification& loadSpecs)
	{
		{
			std::ifstream f;
			if (!GainAccessToFile(nullptr, loadSpecs.Metadata->Path))
			{
				return;
			}
		}

		Ref<AssetBase> ptr = *loadSpecs.Ptr;

		switch (loadSpecs.Type)
		{
		case AssetType::Texture2D:
		{
			TextureLibrary::LoadInto(ptr, loadSpecs.Metadata->Path, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);
		} break;
		case AssetType::Font:
		{
			FontLibrary::LoadInto(ptr, loadSpecs.Metadata->Path);
		} break;
		case AssetType::Model:
		{
			ModelLoader modelLoader;
			modelLoader.LoadInto(ptr, loadSpecs.Metadata->Path, loadSpecs.MtdFile);
		} break;
		case AssetType::StandardMaterial:
		{
			MaterialSerializer loader;
			loader.Deserialize(ptr, loadSpecs.Metadata->Path);
		} break;
		}

	}

	void AssetManager::LoadAssetLocked(const AssetManagerLoadSpecification& loadSpecs)
	{
		Task task;

		Ref<AssetBase>& ptr = *loadSpecs.Ptr = CreateUninitialized(loadSpecs.Type);
		*loadSpecs.PtrToLoaded = ptr;
		
		task = [loadSpecs](const JobArgs&) mutable
		{
			Timer timer("");

			timer.Start();

			Load(loadSpecs);

			delete loadSpecs.MtdFile;
			
			loadSpecs.Ptr->Get()->m_AssetHandle = loadSpecs.Metadata->Handle;

			Application::Get().SubmitToMainThread([onLoad = loadSpecs.OnLoad, ptr = *loadSpecs.Ptr]() {
				if(onLoad)
					onLoad(Asset(ptr));
			});

			timer.Stop();
			KD_INFO("Finished loading asset: {} in {}ms", loadSpecs.Metadata->Path, timer.GetMS());
		};

		//Make sure its not set.
		ptr->m_AssetHandle = UUID(0);

		Run(task, loadSpecs.LoadPolicy);
	}

	void AssetManager::Run(Task task, AssetLoadPolicy loadPolicy)
	{
		switch (loadPolicy)
		{
		case Kaidel::AssetLoadPolicy::Synchronous: task(JobArgs()); break;
		case Kaidel::AssetLoadPolicy::Async: JobSystem::Execute(s_Data->JobContext, task); break;
		}
	}

	AssetIteratorType AssetMapIterator::begin()
	{
		return Map.begin();
	}

	AssetIteratorType AssetMapIterator::end()
	{
		return Map.end();
	}

	AssetMapIterator::~AssetMapIterator()
	{
		s_Mutex.unlock_shared();
	}

}
