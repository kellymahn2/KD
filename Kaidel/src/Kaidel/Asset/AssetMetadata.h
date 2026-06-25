#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Core/UUID.h"


namespace Kaidel {
	using AssetHandle = UUID;

	struct AssetMetadata {
		//TODO: Remove for doing asset packs.
		FileSystem::path Path;
		std::string AssetName;


		AssetHandle Handle;

		AssetHandle RootHandle = AssetHandle(0);

		bool Dirty = false;

		AssetMetadata() {
			Handle = AssetHandle();
			
			Path = "";
		}

		AssetMetadata(const FileSystem::path& path, AssetHandle handle = AssetHandle())
			: Handle(handle), Path(path)
		{
		}

		AssetMetadata(const AssetMetadata& rhs)
		{
			Path = rhs.Path;
			AssetName = rhs.AssetName;
			Handle = rhs.Handle;
			RootHandle = rhs.RootHandle;
		}

		AssetMetadata& operator=(const AssetMetadata& rhs)
		{
			Path = rhs.Path;
			AssetName = rhs.AssetName;
			Handle = rhs.Handle;
			RootHandle = rhs.RootHandle;

			return *this;
		}
	};
}
