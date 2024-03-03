#pragma once
#include "AssetManager.h"
#include "Kaidel/Core/Base.h"
namespace Kaidel {

	class AssetSerializer {
	public:

		bool Serialize(const FileSystem::path& path);
		bool Deserialize(const FileSystem::path& path);


	private:
	};
}
