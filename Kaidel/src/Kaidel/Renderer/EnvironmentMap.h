#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
#include "Kaidel/Renderer/RendererGlobals.h"
#include "RenderCommand.h"

namespace Kaidel {

	struct EnvironmentMapSpecification {
		Ref<TextureCube> Environment;
		Ref<TextureCube> Irradiance;
		Ref<Texture2D> SpecularLUT;
		Ref<TextureCube> Specular;
	};
	//index 0 is environment map
	//index 1 is irradiance map
	class EnvironmentMap : public IRCCounter<false> {
	public:
		EnvironmentMap(const EnvironmentMapSpecification& specs);

		const EnvironmentMapSpecification& GetSpecification() const { return m_Specification; }

		Ref<DescriptorSet> GetSet() const { return m_Set; }

	private:
		EnvironmentMapSpecification m_Specification;
		Ref<DescriptorSet> m_Set;
	};
}
