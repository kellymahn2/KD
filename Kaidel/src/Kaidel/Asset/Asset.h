#pragma once
#include "AssetMetadata.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"

namespace Kaidel {

#define DECL_ASSET_TYPE(type) static AssetType GetStaticAssetType() { return AssetType::type; } \
							  virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }\
							  static const char* GetStaticAssetTypeString() { return #type; }\
							  virtual const char* GetAssetTypeString() const override { return GetStaticAssetTypeString(); }

	struct SubAsset;

	class AssetBase : public IRCCounter<true> {
	public:
		
		AssetBase() {

		}
		virtual AssetType GetAssetType() const 
		{
			KD_ASSERT(false);
			return AssetType::None;
		}

		AssetHandle GetHandle() const { return m_AssetHandle; }

		virtual ~AssetBase() = default;

		virtual bool IsLoaded() const 
		{
			return (uint64_t)m_AssetHandle != 0;
		}

		virtual uint64_t GetSubAssetCount() const { return 0; }
		virtual void GetSubAssetAt(uint64_t i, SubAsset& subAsset) { }

		virtual const char* GetAssetTypeString() const = 0;

	protected:
		AssetHandle m_AssetHandle = AssetHandle(0);

		friend class AssetManager;
	};

	//Sub assets aren't necessarily a known asset by the asset manager.
	struct SubAsset
	{
		Ref<AssetBase> Asset;
		uint32_t UserIndex = -1;
	};

	struct Asset {
		Ref<AssetBase> AssetPtr;

		Asset(Ref<AssetBase> asset = {})
			: AssetPtr(std::move(asset))
		{
		}

		template<typename T> 
		Ref<T> As() const 
		{
			return DynamicPointerCast<T>(AssetPtr);
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
		

		template<typename T, typename... Others>
		bool IsOfType() const
		{
			KD_CORE_ASSERT(AssetPtr);

			return _IsOfType<T, Others...>(AssetPtr->GetAssetType());
		}

	private:
		template<typename T, typename... Others>
		static bool _IsOfType(AssetType type)
		{
			if constexpr (sizeof...(Others) == 0)
			{
				return T::GetStaticAssetType() == type;
			}
			else
			{
				return T::GetStaticAssetType() == type || _IsOfType<Others...>(type);
			}
		}
	};
}
