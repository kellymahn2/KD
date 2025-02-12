#pragma once

#include "Kaidel/Core/Base.h"

#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Kaidel/Renderer/GraphicsAPI/Sampler.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
namespace Kaidel {

	struct MSDFData;

	class Font : public IRCCounter<false>{
	public:
		Font(const Path& path);
		~Font();
		const MSDFData* GetMSDFData()const { return m_Data; }

		Ref<Texture2D> GetAtlasTexture()const { return m_AtlasTexture; }
		Ref<DescriptorSet> GetSet()const { return m_Set; }
	private:
		void CreateDescriptor();
	private:
		MSDFData* m_Data;
		Ref<Texture2D> m_AtlasTexture;
		Ref<DescriptorSet> m_Set;
	};
}
