#pragma once
#include "Kaidel/Core/UUID.h"
#include "Kaidel/Core/IRCP.h"
#include "Kaidel/Core/Base.h"
#include"AssetTypeIDs.h"
#include <unordered_map>

namespace Kaidel {
	template<typename T>
	struct Asset {
		UUID UniqueAssetID = { 0 };
		IRCPointer<T, false> Data;
	
		operator bool()const {
			return UniqueAssetID.operator size_t() != 0 && Data.Get() != nullptr;
		}

		T* operator->()const { return Data.operator->(); }

	
	};
	
	template<typename T>
	class SingleAssetManager {
	public:
	
		static bool AssetExists(UUID id) {
			return m_ManagedAssetsByUUID.find(id) != m_ManagedAssetsByUUID.end();
		}
	
		static bool AssetExists(IRCPointer<T, false> asset) {
			return m_ManagedAssetsByAsset.find(asset) != m_ManagedAssetsByAsset.end();
		}
	
	
		static Asset<T> Get(UUID id) {
			if (m_ManagedAssetsByUUID.find(id) != m_ManagedAssetsByUUID.end())
				return Asset<T>{ id, m_ManagedAssetsByUUID.at(id) };
			return {};
		}
	
		static Asset<T> Get(IRCPointer<T, false> asset) {
			if (m_ManagedAssetsByAsset.find(asset) != m_ManagedAssetsByAsset.end())
				return Asset<T>{ m_ManagedAssetsByAsset.at(asset), asset };
			return {};
		}
	
		static Asset<T> Manage(IRCPointer<T, false> asset, UUID id = {}) {
			if (AssetExists(asset))
				return Asset<T>{ m_ManagedAssetsByAsset.at(asset), asset };
	
			while (AssetExists(id))
				id = UUID();
			m_ManagedAssetsByAsset[asset] = id;
			m_ManagedAssetsByUUID[id] = asset;
			return { id,asset };
		}
	
		static Asset<T> UnManage(UUID id) {
			if (!AssetExists(id))
				return {};
			IRCPointer<T, false> ptr = m_ManagedAssetsByUUID.at(id);
			m_ManagedAssetsByUUID.erase(id);
			m_ManagedAssetsByAsset.erase(ptr);
			return { id,ptr };
		}
		static Asset<T> UnManage(IRCPointer<T, false> asset) {
			if (!AssetExists(asset))
				return {};
			UUID id = m_ManagedAssetsByAsset.at(asset);
			m_ManagedAssetsByAsset.erase(asset);
			m_ManagedAssetsByUUID.erase(id);
			return { id,asset };
		}

		static auto& GetManagedAssetsByUUID() { return m_ManagedAssetsByUUID; }
		static auto& GetManagedAssetsByAsset() { return m_ManagedAssetsByAsset; }


	private:
		static inline std::unordered_map<UUID, IRCPointer<T, false>> m_ManagedAssetsByUUID;
		static inline std::unordered_map<IRCPointer<T, false>, UUID> m_ManagedAssetsByAsset;
	};

#define ASSET_EXTENSION_TYPE(val) static AssetType StaticAssetTypeID(){return AssetType::##val;}\
virtual AssetType AssetTypeID()const override{return StaticAssetTypeID();}

	class _Asset : public IRCCounter<false> {
	public:

		_Asset()
		{
		}

		virtual AssetType AssetTypeID()const { return AssetType::None; }

		virtual ~_Asset() = default;

		const FileSystem::path& Path() const { return m_Path; }
		const std::string& Name()const { return m_Name; }
		bool Physical()const { return m_Physical; }
		UUID AssetID()const { return m_AssetID; }

		void Path(const FileSystem::path& path) {
			m_Path = path;
			if (FileSystem::directory_entry(path).exists())
				m_Physical = true;
		}

		void Name(const std::string& name) { m_Name = name; }
		void AssetID(UUID id) { m_AssetID = id; }

	protected:
		bool m_Physical = false;
		FileSystem::path m_Path = "";
		std::string m_Name;
		UUID m_AssetID;
	};


	class AssetManager {
	public:
		static void Manage(Ref<_Asset> asset) {
			if (asset) {
				m_Assets[asset->AssetID()] = asset;
				if (asset->Physical()) {
					m_PhysicalAssets[asset->Path()] = asset;
				}
				else {
					m_NonPhysicalAssets[asset->AssetID()] = asset;
				}
			}
		}


		static Ref<_Asset> AssetByID(UUID id) {
			auto it = m_Assets.find(id);
			if (it != m_Assets.end())
				return it->second;
			return {};
		}


		static Ref<_Asset> AssetsByPath(const FileSystem::path& path) {
			auto it = m_PhysicalAssets.find(path);
			if (it != m_PhysicalAssets.end()) {
				return it->second;
			}
			return {};
		}

	private:
		static inline std::unordered_map<UUID, Ref<_Asset>> m_Assets;

		static inline std::unordered_map<UUID, Ref<_Asset>> m_NonPhysicalAssets;

		static inline std::unordered_map<FileSystem::path, Ref<_Asset>> m_PhysicalAssets;
		friend class AssetSerializer;
		friend class ProjectSerializer;
	};


	class AssetFunctionApplier
	{
	public:
		AssetFunctionApplier(IRCPointer<_Asset> asset)
			: m_Asset(asset)
		{
		}

		template<typename T, typename F, typename... Args>
		bool Apply(const F& func, Args&&... args) {
			if (m_Asset->AssetTypeID() == T::StaticAssetTypeID())
			{
				func(m_Asset, std::forward<Args>(args)...);
				return true;
			}
			return false;
		}
	private:
		IRCPointer<_Asset>  m_Asset;
	};


}
