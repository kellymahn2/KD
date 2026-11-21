#include "KDpch.h"
#include "AssetManager.h"

#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "Kaidel/Asset/AssetImporters/MaterialSerializer.h"
#include "Kaidel/Asset/AssetImporters/SpriteSheetSerializer.h"
#include "Kaidel/Asset/AssetImporters/AnimationSerializer.h"
#include "Kaidel/Scene/SceneSerializer.h"
#include "Kaidel/Renderer/Text/FontLibrary.h"
#include "Kaidel/Scene/ModelLoader.h"
#include "Kaidel/Project/Project.h"
#include "Kaidel/Renderer/Sprite.h"

#include "AssetImporters/MetadataFile.h"

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

	} static* s_Data;

	namespace Utils {
		static std::string AssetTypeToString(AssetType type)
		{
			switch (type)
			{
			case Kaidel::AssetType::SpriteSheet: return "SpriteSheet";
			case Kaidel::AssetType::Font: return "Font";
			case Kaidel::AssetType::Model: return "Model";
			case Kaidel::AssetType::Mesh: return "Mesh";
			case Kaidel::AssetType::StandardMaterial: return "StandardMaterial";
			case Kaidel::AssetType::VisualShader: return "VisualShader";
			}
		}

		static AssetType StringToAssetType(const std::string& name)
		{
#define DECL_MAP(type) {#type, AssetType::##type}
			static std::unordered_map<std::string, AssetType> s_Map =
			{
				DECL_MAP(SpriteSheet),
				DECL_MAP(Font),
				DECL_MAP(Model),
				DECL_MAP(Mesh),
				DECL_MAP(StandardMaterial),
				DECL_MAP(VisualShader)
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

	Kaidel::AssetHandle AssetManager::GetAssetHandleAtPath(const Path& path)
	{
		std::shared_lock<std::shared_mutex> lock(s_Mutex);

		auto it = s_Data->ReverseRegistry.find(path);

		if (it != s_Data->ReverseRegistry.end())
			return it->second;

		return 0;
	}

	bool AssetManager::IsAssetHandleValid(AssetHandle handle)
	{
		if (handle == 0)
		{
			return false;
		}

		std::shared_lock<std::shared_mutex> lock(s_Mutex);

		auto it = s_Data->TotalMap.find(handle);

		return it != s_Data->TotalMap.end();
	}

	bool AssetManager::IsBuiltIn(AssetHandle handle)
	{
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

		KD_CORE_ASSERT(metadata.Type != AssetType::None);

		s_Data->Registry[metadata.Handle] = metadata;
		s_Data->ReverseRegistry[metadata.Path] = metadata.Handle;
		if (asset)
		{
			asset->m_AssetHandle = metadata.Handle;
		}
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
		Ref<AssetBase> skip;

		if (metadata.Path.empty())
		{
			auto it = s_Data->LoadedMap.find(metadata.Handle);

			if (it != s_Data->LoadedMap.end() && it->second)
			{
				skip = it->second;
			}
		}
		else if (auto it = s_Data->ReverseRegistry.find(metadata.Path); it != s_Data->ReverseRegistry.end())
		{
			skip = s_Data->LoadedMap.at(it->second);
		}

		if (skip)
		{
			Asset asset = Asset(skip);
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

		auto& path = metadata.Path.empty() ? s_Data->Registry.at(metadata.Handle).Path : metadata.Path;

		MetadataFile* file = new MetadataFile(path, true);

		file->Resolve(handle);
		
		Ref<AssetBase>& ptr = s_Data->TotalMap[handle];
		Ref<AssetBase>& ptrLoaded = s_Data->LoadedMap[handle];
		AssetMetadata& mtd = s_Data->Registry[handle];
		mtd.Path = path;
		s_Data->ReverseRegistry[path] = handle;
		mtd.Handle = handle;
		mtd.AssetName = path.filename().string();
		mtd.Type = type;
		
		s_Mutex.unlock();

		AssetManagerLoadSpecification loadSpecs;
		loadSpecs.LoadPolicy = loadPolicy;
		loadSpecs.Metadata = &mtd;
		loadSpecs.OnLoad = std::move(onLoad);
		loadSpecs.Ptr = &ptr;
		loadSpecs.PtrToLoaded = &ptrLoaded;
		loadSpecs.Type = type;
		loadSpecs.MtdFile = file;
		loadSpecs.OnLoad = {};

		LoadAssetLocked(loadSpecs);

		return Asset(ptr);
	}


	Kaidel::Asset AssetManager::ResolveDependency(AssetHandle handle, AssetType type, bool required)
	{
		AssetMetadata mtd;
		mtd.Handle = handle;
		mtd.Type = type;

		Asset asset = AssetManager::LoadAsset(mtd, mtd.Type);

		if (required)
		{
			AssetManager::WaitOnAssets({ asset });
		}

		return asset;
	}

	void AssetManager::RemoveAsset(AssetHandle handle)
	{
		RemoveAsset(handle, GetAssetMetadata(handle).Path);
	}

	void AssetManager::RemoveAsset(AssetHandle handle, const Path& path)
	{
		std::unique_lock<std::shared_mutex> lock(s_Mutex);

		auto& data = s_Data;

		s_Data->ReverseRegistry.erase(path);

		s_Data->Registry.erase(handle);

		s_Data->LoadedMap.erase(handle);

		s_Data->TotalMap.erase(handle);
	}

	void AssetManager::RemoveAssetAtPath(const Path& path)
	{
		AssetHandle handle = GetAssetHandleAtPath(path);
		KD_CORE_ASSERT(handle);

		RemoveAsset(handle, path);
	}

	void AssetManager::ResetAssetPath(AssetHandle handle, const Path& oldPath, const Path& newPath)
	{
		KD_CORE_ASSERT(GetAssetAtPath(oldPath));
		
		std::unique_lock<std::shared_mutex> lock(s_Mutex);

		s_Data->ReverseRegistry.erase(oldPath);
		s_Data->ReverseRegistry[newPath] = handle;

		s_Data->Registry[handle].Path = newPath;

		Project::ConsumePathByFileWatcher(oldPath, 1);
		Project::ConsumePathByFileWatcher(newPath, 1);

		FileSystem::rename(oldPath, newPath);

		//update metadata paths
		Path oldMtdPath = oldPath.string() + ".mtd";
		Path newMtdPath = newPath.string() + ".mtd";

		if (FileSystem::exists(oldMtdPath))
		{
			//remove the old one and log it probably.
			if (FileSystem::exists(newMtdPath))
			{
				FileSystem::remove(newMtdPath);
				KD_INFO("Metadata path {} already existed and is overwritten.", newMtdPath);
			}

			FileSystem::rename(oldMtdPath, newMtdPath);
		}
	}

	void AssetManager::ResetAssetParentPath(AssetHandle handle, const Path& oldPath, const Path& newPath)
	{
		KD_CORE_ASSERT(GetAssetAtPath(oldPath) && oldPath.filename() == newPath.filename());

		std::unique_lock<std::shared_mutex> lock(s_Mutex);

		s_Data->ReverseRegistry.erase(oldPath);
		s_Data->ReverseRegistry[newPath] = handle;

		s_Data->Registry[handle].Path = newPath;
	}

	std::pair<Asset, AssetHandle> AssetManager::RefreshAsset(const FileSystem::path& path, AssetLoadPolicy loadPolicy)
	{
		s_Mutex.lock_shared();

		AssetHandle handle = s_Data->ReverseRegistry[path];

		auto& mtd = s_Data->Registry[handle];
		KD_CORE_ASSERT(mtd.Type != AssetType::None);

		Ref<AssetBase>& ptr = s_Data->TotalMap[handle];
		Ref<AssetBase>& ptrLoaded = s_Data->LoadedMap[handle];

		MetadataFile* file = new MetadataFile(mtd.Path);

		file->Resolve(handle);

		s_Mutex.unlock_shared();

		AssetManagerLoadSpecification loadSpecs;
		loadSpecs.LoadPolicy = loadPolicy;
		loadSpecs.Metadata = &mtd;
		loadSpecs.Ptr = &ptr;
		loadSpecs.PtrToLoaded = &ptrLoaded;
		loadSpecs.Type = mtd.Type;
		loadSpecs.MtdFile = file;
		loadSpecs.OnLoad = [](Asset) {};

		LoadAssetLocked(loadSpecs);

		return std::make_pair(Asset(ptr), mtd.Handle);
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

	void AssetManager::Serialize(const FileSystem::path& path, Asset asset, bool forceSerialize, MetadataFile* mtdFile)
	{
		if (!forceSerialize && !s_Data->Registry.at(asset->GetHandle()).Dirty)
		{
			return;
		}

		bool hadMtd = mtdFile != nullptr;

		if (!hadMtd)
		{
			mtdFile = new MetadataFile(path);
		}

		if (mtdFile->IsCreated())
			mtdFile->Resolve(asset->GetHandle());

		Project::ConsumePathByFileWatcher(path, 1 + (FileSystem::exists(path) ? 1 : 1));
		
		switch (asset->GetAssetType())
		{
		case AssetType::SpriteSheet:
		{
			SpriteSheetSerializer().Serialize(asset.As<SpriteSheet>(), mtdFile);
		} break;
		case AssetType::StandardMaterial:
		{
			MaterialSerializer().Serialize(path, asset.As<StandardMaterial>(), mtdFile);
		} break;
		case AssetType::VisualShader:
		{
			MaterialSerializer().Serialize(path, asset.As<VisualShader>(), mtdFile);
		} break;
		case AssetType::Scene:
		{
			SceneSerializer(asset.As<Scene>()).Serialize(path.string());
		} break;
		case AssetType::Animation:
		{
			AnimationSerailizer().Serialize(path, asset.As<AnimationTree>());
		} break;
		case AssetType::ShaderMaterial:
		{
			MaterialSerializer().Serialize(path, asset.As<ShaderMaterial>(), mtdFile);
		} break;
		}

		if (!hadMtd)
		{
			delete mtdFile;
			SetDirty(asset->GetHandle(), false);
		}
	}

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
			{".png", AssetType::SpriteSheet},
			{".jpg", AssetType::SpriteSheet},
			{".jpeg", AssetType::SpriteSheet},
			
			//Fonts
			{".ttf", AssetType::Font},

			//Models
			{".gltf", AssetType::Model},
			//Materials
			{".mat", AssetType::StandardMaterial},
			{".vshader", AssetType::VisualShader},
			{".smat", AssetType::ShaderMaterial},
			//{".fbx", AssetType::Model} // TODO: Add support

			//Animations
			{".anim", AssetType::Animation },
		};


		return s_Map;
	}


	Kaidel::AssetType AssetManager::GetTypeFromExtension(const std::string& extension)
	{
		if (auto it = GetSupportedAssets().find(extension); it != GetSupportedAssets().end())
		{
			return it->second;
		}

		return AssetType::None;
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
		case Kaidel::AssetType::SpriteSheet:
		{
			return CreateRef<SpriteSheet>();
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
			return CreateRef<StandardMaterial>(StandardMaterial::Initializer{});
		} break;
		case Kaidel::AssetType::VisualShader:
		{
			return CreateRef<VisualShader>();
		} break;
		case Kaidel::AssetType::Scene:
		{
			return CreateRef<Scene>();
		} break;
		case Kaidel::AssetType::Animation:
		{
			return CreateRef<AnimationTree>();
		} break;
		case Kaidel::AssetType::ShaderMaterial:
		{
			return CreateRef<ShaderMaterial>(Ref<VisualShader>(nullptr));
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
		case AssetType::SpriteSheet:
		{
			Ref<Texture2D> texture = TextureLibrary::Load(loadSpecs.Metadata->Path, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			SpriteSheetSerializer().Deserialize(DynamicPointerCast<SpriteSheet>(ptr), texture, loadSpecs.MtdFile);
		} break;
		case AssetType::Font:
		{
			FontLibrary::LoadInto(DynamicPointerCast<Font>(ptr), loadSpecs.Metadata->Path);
		} break;
		case AssetType::Model:
		{
			ModelLoader modelLoader;
			modelLoader.LoadInto(DynamicPointerCast<Model>(ptr), *loadSpecs.Metadata, loadSpecs.MtdFile);
		} break;
		case AssetType::StandardMaterial:
		{
			MaterialSerializer loader;
			loader.Deserialize(DynamicPointerCast<StandardMaterial>(ptr), loadSpecs.Metadata->Path, loadSpecs.MtdFile);
		} break;
		case AssetType::VisualShader:
		{
			MaterialSerializer loader;
			loader.Deserialize(DynamicPointerCast<VisualShader>(ptr), loadSpecs.Metadata->Path, loadSpecs.MtdFile);
		} break;

		case AssetType::Scene:
		{
			SceneSerializer serializer(DynamicPointerCast<Scene>(ptr));
			serializer.Deserialize(loadSpecs.Metadata->Path.string());
		} break;
		case AssetType::Animation:
		{
			AnimationSerailizer().Deserialize(DynamicPointerCast<AnimationTree>(ptr), loadSpecs.Metadata->Path);
		} break;
		case AssetType::ShaderMaterial:
		{
			MaterialSerializer loader;
			loader.Deserialize(DynamicPointerCast<ShaderMaterial>(ptr), loadSpecs.Metadata->Path, loadSpecs.MtdFile);
		} break;
		default:
			KD_CORE_ASSERT(false);
			break;
		}

		loadSpecs.Ptr->Get()->m_AssetHandle = loadSpecs.Metadata->Handle;

		//Serialize it since this might be a fresh asset or something.
		if (loadSpecs.MtdFile->IsCreated())
		{
			Serialize(loadSpecs.Metadata->Path, Asset(ptr), true, loadSpecs.MtdFile);
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
			
			if (loadSpecs.OnLoad)
			{
				Application::Get().SubmitToMainThread([onLoad = loadSpecs.OnLoad, ptr = *loadSpecs.Ptr]() {
						onLoad(Asset(ptr));
				});
			}
			

			timer.Stop();
			//KD_INFO("Finished loading asset: {} in {}ms", loadSpecs.Metadata->Path, timer.GetMS());
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
