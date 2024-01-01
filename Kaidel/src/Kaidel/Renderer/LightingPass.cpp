#include "KDpch.h"
#include "LightingPass.h"
#include "LightingRenderer.h"
#include "Kaidel/Core/JobSystem.h"
#include "Kaidel/Scene/Components.h"
namespace Kaidel {


	static Ref<Framebuffer> s_LightingFrameBuffer;

	void LightingPass::Render() {


		if (!s_LightingFrameBuffer) {
			FramebufferSpecification fbSpec;
			fbSpec.Attachments = { FramebufferTextureFormat::RGBA8,FramebufferTextureFormat::RGBA8 ,FramebufferTextureFormat::RGBA8 };
			fbSpec.Samples = 1;
			fbSpec.Width = Config.Width;
			fbSpec.Height = Config.Height;
			s_LightingFrameBuffer = Framebuffer::Create(fbSpec);
		}

		if (auto spec = s_LightingFrameBuffer->GetSpecification(); (spec.Width != Config.Width || spec.Height != Config.Height)&&spec.Width>0&&spec.Height>0) {
			s_LightingFrameBuffer->Resize(Config.Width, Config.Height);
		}

		s_LightingFrameBuffer->Bind();
		float defLights[4] = { 0.0f };
		s_LightingFrameBuffer->ClearAttachment(0, defLights);
		s_LightingFrameBuffer->ClearAttachment(1, defLights);
		s_LightingFrameBuffer->ClearAttachment(2, defLights);
		LightingRenderer lightingRenderer;
		//TODO: We need materials as well.
		lightingRenderer.BeginRendering();
		auto view = Config.Scene->m_Registry.view<TransformComponent,CubeRendererComponent>();
		for (auto e : view) {
			JobSystem::GetMainJobSystem().Execute([&view,e,&lightingRenderer]() {
				auto [tc,crc] = view.get<TransformComponent,CubeRendererComponent>(e);
				lightingRenderer.DrawCube(tc.GetTransform(),crc.Material?crc.Material->GetIndex():0);
				});
		}
		JobSystem::GetMainJobSystem().Wait();
		lightingRenderer.EndRendering();

		s_LightingFrameBuffer->Unbind();
	}

}
