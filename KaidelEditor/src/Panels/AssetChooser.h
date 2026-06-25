#pragma once
#include "Kaidel/Asset/Asset.h"

#include <array>

namespace Kaidel {
	using OnAssetChosen = std::function<void(Asset)>;

	struct AssetFilter {
		std::array<bool, (uint64_t)AssetType::Count> Filters;
		uint64_t ActiveCount = 0;

		AssetFilter() = default;

		AssetFilter(const std::initializer_list<AssetType>& list)
		{
			std::memset(Filters.data(), 0, sizeof(Filters));

			for (auto& type : list)
			{
				Filters[(uint64_t)type] = true;
			}

			ActiveCount = list.size();
		}

		bool Passes(AssetType type) const
		{
			return Filters[(uint64_t)type];
		}
	};


	class AssetChooser {
	public:
		
		static void Render(const AssetFilter& filter, const OnAssetChosen& onChosen);

		static void OnImGuiRender();

	private:
		static bool RenderAssetChooserElement(Ref<AssetBase> asset, const AssetMetadata& metadata, const OnAssetChosen& onChosen);
	};

}

