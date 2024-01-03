#include "KDpch.h"
#include "ShadowPass.h"
#include "ShadowRenderer.h"
#include "Kaidel/Core/JobSystem.h"
#include "Kaidel/Scene/Components.h"
namespace Kaidel {
	static Ref<Framebuffer> s_ShadowFrameBuffer;
	void ShadowPass::Render() {
		if (!s_ShadowFrameBuffer) {
			FramebufferSpecification fbSpec{};
			fbSpec.Samples = 1;
			fbSpec.Width = _ShadowMapWidth;
			fbSpec.Height = _ShadowMapHeight;
			s_ShadowFrameBuffer = Framebuffer::Create(fbSpec);
		}
		//SpotLights
		{
			ShadowRenderer shadowRenderer;
			auto depthMaps = SpotLight::GetDepthMaps();
			for (uint32_t lightIndex = 0; lightIndex < SpotLight::GetLightCount(); ++lightIndex) {
				depthMaps->ClearLayer(lightIndex, 1.0f);
				s_ShadowFrameBuffer->SetDepthAttachmentFromArray(depthMaps->GetRendererID(), lightIndex);
				s_ShadowFrameBuffer->Bind();
				shadowRenderer.BeginRendering(lightIndex);
				auto cubeView = Config.Scene->m_Registry.view<TransformComponent,CubeRendererComponent>();
				for (auto e : cubeView) {
					JobSystem::GetMainJobSystem().Execute([&cubeView, e, &shadowRenderer]() {
						auto& tc = cubeView.get<TransformComponent>(e);
						shadowRenderer.DrawCube(tc.GetTransform());
					});
				}
				JobSystem::GetMainJobSystem().Wait();
				shadowRenderer.EndRendering();
				s_ShadowFrameBuffer->Unbind();
			}

		}
	}
}
