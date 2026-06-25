#pragma once
#include "AssetMetadata.h"

namespace Kaidel {

	enum class AssetType {
		None = 0,
		Texture2D,
		Font,
		Model,
		Mesh,
		StandardMaterial,
		VisualMaterial,
		Scene,
		Count
	};

#define DECL_ASSET_TYPE(type) static AssetType GetStaticAssetType() { return AssetType::type; } \
							  virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }

	class AssetBase : public IRCCounter<true> {
	public:
		
		AssetBase() {

		}
		virtual AssetType GetAssetType() const 
		{
			KD_ASSERT(false);
			return AssetType::Texture2D;
		}

		AssetHandle GetHandle() const { return m_AssetHandle; }

		virtual ~AssetBase() = default;

		virtual bool IsLoaded() const 
		{
			return (uint64_t)m_AssetHandle != 0;
		}

	private:
		AssetHandle m_AssetHandle = AssetHandle(0);

		friend class AssetManager;
	};

	struct Asset {
		Asset(Ref<AssetBase> asset = {})
			: AssetPtr(std::move(asset))
		{
		}

		template<typename T> 
		Ref<T> As() const 
		{
			return AssetPtr;
		}

		operator bool() const 
		{
			return Loaded();
		}

		bool Loaded() const 
		{
			return AssetPtr && AssetPtr->IsLoaded();
		}

		AssetBase* operator->() const { return AssetPtr.Get(); }
		
		Ref<AssetBase> AssetPtr;
	};
}
