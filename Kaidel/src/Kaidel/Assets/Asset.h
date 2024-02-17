#pragma once
#include "Kaidel/Core/UUID.h"
#include "Kaidel/Core/Base.h"
#include <string>
#include <unordered_map>
namespace Kaidel {


	template<typename _AssetType,typename _ContainerType = Ref<_AssetType>>
	class Asset {
	public:

		Asset(const _ContainerType& asset)
			:m_AssetData(asset)
		{
			m_UsageCount = new uint64_t;
			*m_UsageCount = 1;
			PushToMaps();
		}

		Asset()
			:m_AssetData({}),m_IsValidAsset(false)
		{}


		Asset(const Asset& rhs) {
			m_IsValidAsset = rhs.m_IsValidAsset;
			m_AssetID = rhs.m_AssetID;
			m_AssetData = rhs.m_AssetData;
			m_UsageCount = rhs.m_UsageCount;
			if(m_IsValidAsset)
				++(*m_UsageCount);
		}

		Asset(Asset&& rhs) {
			m_IsValidAsset = rhs.m_IsValidAsset;
			rhs.m_IsValidAsset = false;
			m_AssetID = rhs.m_AssetID;
			m_AssetData = std::move(rhs.m_AssetData);
			m_UsageCount = rhs.m_UsageCount;
			rhs.m_UsageCount = nullptr;
		}

		Asset& operator=(const Asset& rhs) {
			if (m_IsValidAsset) {
				(*m_UsageCount)--;
			}
			m_IsValidAsset = rhs.m_IsValidAsset;
			m_AssetID = rhs.m_AssetID;
			m_AssetData = rhs.m_AssetData;
			m_UsageCount = rhs.m_UsageCount;
			if(m_IsValidAsset)
				++(*m_UsageCount);
			return *this;
		}
		Asset& operator=(Asset&& rhs) {
			if (m_IsValidAsset) {
				(*m_UsageCount)--;
			}
			m_IsValidAsset = rhs.m_IsValidAsset;
			rhs.m_IsValidAsset = false;
			m_AssetID = rhs.m_AssetID;
			m_AssetData = std::move(rhs.m_AssetData);
			m_UsageCount = rhs.m_UsageCount;
			rhs.m_UsageCount = nullptr;
			return *this;
		}
		~Asset() {
			if (m_IsValidAsset) {
				(*m_UsageCount)--;
			}
		}



		_AssetType* operator ->() { return m_AssetData.operator->(); }
		const _AssetType* operator ->() const { return m_AssetData.operator->(); }

		_AssetType& operator*() { return *m_AssetData; }
		const _AssetType& operator*() const { return *m_AssetData; }

		_ContainerType& GetContainer() { return m_AssetData; }

		UUID GetAssetID() const { return m_AssetID; }

		bool IsValid() const { return m_IsValidAsset == true; }

		static Asset& GetAsset(UUID uuid) {
			KD_CORE_ASSERT(AssetExists(uuid), "Asset doesn't exist");
			return s_AssetMap.at(uuid);
		}
		static bool AssetExists(UUID uuid) {
			return s_AssetMap.find(uuid) != s_AssetMap.end();
		}

		static const auto& GetAssetMap() { return s_AssetMap; }

	private:


		UUID m_AssetID;
		_ContainerType m_AssetData;
		uint64_t* m_UsageCount = nullptr;
		
		
		
		bool m_IsValidAsset = true;
		void PushToMaps() {
			while (s_AssetMap.find(m_AssetID) != s_AssetMap.end()) {
				m_AssetID = UUID();
			}
			s_AssetMap[m_AssetID] = *this;
		}
		static inline std::unordered_map<UUID, Asset> s_AssetMap;


		template<typename U>
		friend struct AssetHandle;
	};

	template<typename T>
	struct AssetHandle {
		Asset<T> Handle;
		AssetHandle() 
			:Handle()
		{}

		T* operator ->() { return Handle.operator->(); }
		const T* operator ->() const { return Handle.operator->(); }

		T& operator*() { return Handle.operator*(); }
		const T& operator*() const { return Handle.operator*(); }


		AssetHandle(const Asset<T>& asset)
			:Handle(asset)
		{}

		operator bool()const { return Handle.IsValid(); }
		bool operator==(const AssetHandle& rhs)const {
			return Handle.GetAssetID() == rhs.Handle.GetAssetID();
		}

	};

}

namespace std {
	template<typename T>
	struct hash<Kaidel::AssetHandle<T>> {

		std::size_t operator()(const Kaidel::AssetHandle<T>& handle) const {
			return handle.Handle.GetAssetID().operator size_t();
		}


	};


}
