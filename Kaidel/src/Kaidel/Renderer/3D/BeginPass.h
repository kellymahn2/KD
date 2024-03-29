#pragma once
#include "Kaidel/Renderer/3D/Material.h"
#include "Kaidel/Renderer/GraphicsAPI/FullAPI.h"
#include "Light.h"
namespace Kaidel {

	struct StartData {
		glm::mat4 CameraVP;
		glm::vec3 CameraPosition;
		Ref<Framebuffer> Outputbuffer;
	};

	

	struct GlobalRenderer3DData {
		struct LightCount {
			int PointLightCount;
			int SpotLightCount;
		};
		struct CameraBufferData {
			glm::mat4 ViewProj;
			glm::vec3 Position;
		};
		CameraBufferData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
		Ref<UniformBuffer> LightCountUniformBuffer;




		Ref<Framebuffer> ShadowDepthBuffer;
		Ref<Shader> ShadowPassShader;


		Ref<Framebuffer> GBuffers;
		Ref<Shader> GeometryPassShader;

		Ref<Shader> LightingPassShader;

		GlobalRenderer3DData();
	};
	namespace RendererBindings{
		static inline constexpr uint32_t CameraBinding = 0;
		static inline constexpr uint32_t LightCountBinding = 1;
		static inline constexpr uint32_t MaterialBinding = _MaterialInternalBindingSlot;
		static inline constexpr uint32_t TextureBinding = _MaterialTexturesBindingSlot;
		static inline constexpr uint32_t SpotLightBinding = _SpotLightBindingSlot;
	}


	extern GlobalRenderer3DData* GlobalRendererData;

	class BeginPass {
	public:
		void Render(const StartData& startData);
	private:
	};
}
