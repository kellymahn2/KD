#pragma once
#include "Kaidel/Core/UUID.h"
#include "Kaidel/Core/Base.h"
#include <string>
namespace Kaidel {
	class _Asset {
	public:
		_Asset();
		//Internal _Asset Name 
		inline virtual std::string GetAssetName() = 0;
		inline UUID GetUniqueID() { return m_AssetID; }
		//Display Name on UI 
		inline virtual std::string GetDisplayName() { return GetAssetName(); }
	protected:
		UUID m_AssetID;
	};

	using UniqueAsset = Scope<_Asset>;
	using Asset = Ref <_Asset>;

}
