#pragma once
#include "Asset.h"
#include "AssetMetadata.h"
#include "Kaidel/Core/JobSystem.h"

namespace Kaidel {

	class MetadataFile;

	using AssetMap = std::unordered_map<AssetHandle, Ref<AssetBase>>;

	using AssetIteratorType = typename AssetMap::const_iterator;
	using RegistryIteratorType = typename std::unordered_map<AssetHandle, AssetMetadata>::const_iterator;

	using OnLoadFunction = std::function<void(Asset)>;

	enum class AssetLoadPolicy {
		Synchronous,
		Async,
		Default = Async,
	};

	struct AssetMapIterator {

		AssetIteratorType begin();

		AssetIteratorType end();

		~AssetMapIterator();

		const AssetMap& Map;
	};


	/*
	this class manages all assets from built in to loaded at runtime in the editor or the game itself.
	it uses three maps to distinguish between the two types.
	1) BuiltInMap: this map is used to store the built in assets loaded by the engine itself at initialization.
	2) LoadedMap: this map is used to store the assets loaded at runtime. this is the only map that will be serialized to or deserialized from.
	3) TotalMap: this map is used to store the sum of the BuiltInMap and LoadedMap. the following equation must always hold TotalMap = BuiltInMap + LoadedMap.
	
	all requests of loading to the asset manager can be done both async or on the calling thread but all dependencies of the assets(e.g. textures for models and materials)
	will be loaded on the same thread the root asset is being loaded on(async loading of asset dependencies is NOT currently supported).

	calling any of the functions from multiple threads is valid and supported(e.g. IsAssetHandleValid or LoadAsset).

	the loading of assets by the asset manager works by returning to an uninitialized but allocated asset.

	the process of checking if the asset is fully loaded can be done by asset->GetHandle() != 0.

	the entire process of scheduling the workload of loading multiple assets at once is done by the job system.


	*/
	class AssetManager {
		struct AssetManagerLoadSpecification {
			Ref<AssetBase>* Ptr;
			Ref<AssetBase>* PtrToLoaded;
			AssetMetadata* Metadata;
			AssetType Type;
			OnLoadFunction OnLoad;
			AssetLoadPolicy LoadPolicy;
			MetadataFile* MtdFile;
		};
	public:
		static void Init();
		static void Shutdown();

		static const AssetMetadata& GetAssetMetadata(AssetHandle handle);

		static Asset GetAsset(AssetHandle handle);
		static Asset GetAssetAtPath(const Path& path);
		static AssetHandle GetAssetHandleAtPath(const Path& path);
		static bool IsAssetHandleValid(AssetHandle handle);
		static bool IsBuiltIn(AssetHandle handle);

		static void RegisterBuiltIn(Ref<AssetBase> asset, const std::string& name);

		static void RegisterAsset(Ref<AssetBase> asset, const AssetMetadata& metadata);

		//Loads the asset based on the loading policy
		//any dependencies of the asset (e.g. materials, texture) are loaded synchronously
		static Asset LoadAsset(
			const AssetMetadata& metadata, AssetType type, 
			OnLoadFunction onLoad = [](Asset) {}, AssetLoadPolicy loadPolicy = AssetLoadPolicy::Default);

		//Loads the asset with AssetHande = handle, optionally waiting on it to finish loading(by required flag).
		static Asset ResolveDependency(AssetHandle handle, AssetType type, bool required = true);

		static void RemoveAsset(AssetHandle handle);

		static void RemoveAssetAtPath(const Path& path);

		static void ResetAssetPath(AssetHandle handle, const Path& oldPath, const Path& newPath);
		static void ResetAssetParentPath(AssetHandle handle, const Path& oldPath, const Path& newPath);

		static std::pair<Asset, AssetHandle> RefreshAsset(const FileSystem::path& path, AssetLoadPolicy loadPolicy = AssetLoadPolicy::Default);

		static AssetMapIterator BeginTotal();

		static AssetMapIterator BeginBuiltIn();

		static AssetMapIterator BeginLoaded();

		static void Serialize(const FileSystem::path& path, Asset asset, bool forceSerialize = false, MetadataFile* mtdFile = nullptr);

		static void Deserialize(const FileSystem::path& path, bool clear = false);

		static void Wait();

		static void WaitOnAssets(const std::vector<Asset>& assets, const std::chrono::milliseconds& timeout = std::chrono::milliseconds(200));

		static void Clear();

		static const std::unordered_map<std::string, AssetType>& GetSupportedAssets();

		static AssetType GetTypeFromExtension(const std::string& extension);

		static uint64_t GetAssetCount();

		static void SetAssetName(AssetHandle handle, const std::string& name);

		static void SetDirty(AssetHandle handle, bool dirty = true);

	private:

		static Ref<AssetBase> CreateUninitialized(AssetType type);

		static void Load(const AssetManagerLoadSpecification& loadSpecs);

		static void LoadAssetLocked(const AssetManagerLoadSpecification& loadSpecs);

		static void Run(Task task, AssetLoadPolicy loadPolicy);

		static void RemoveAsset(AssetHandle handle, const Path& path);

		//static AssetHandle (const FileSystem::path& path);
	};
}
