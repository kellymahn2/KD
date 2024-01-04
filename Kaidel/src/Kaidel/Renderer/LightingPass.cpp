#include "KDpch.h"
#include "LightingPass.h"
#include "LightingRenderer.h"
#include "Kaidel/Core/JobSystem.h"
#include "Kaidel/Scene/Components.h"
namespace Kaidel {


	static Ref<Framebuffer> s_LightingFrameBuffer;

	uint64_t LightingPass::GetColorAttachment() {
		if (!s_LightingFrameBuffer) {
			FramebufferSpecification fbSpec;
			fbSpec.Attachments = { FramebufferTextureFormat::RGBA8 , FramebufferTextureFormat::DEPTH32 };
			fbSpec.Samples = 1;
			fbSpec.Width = 1280;
			fbSpec.Height = 720;
			s_LightingFrameBuffer = Framebuffer::Create(fbSpec);
		}
		return s_LightingFrameBuffer->GetColorAttachmentRendererID(0);
	}
	void LightingPass::Render() {


		if (!s_LightingFrameBuffer) {
			FramebufferSpecification fbSpec;
			fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH32 };
			fbSpec.Samples = 1;
			fbSpec.Width = Config.Width;
			fbSpec.Height = Config.Height;
			s_LightingFrameBuffer = Framebuffer::Create(fbSpec);
		}

		if (auto spec = s_LightingFrameBuffer->GetSpecification(); (spec.Width != Config.Width || spec.Height != Config.Height)&&spec.Width>0&&spec.Height>0) {
			s_LightingFrameBuffer->Resize(Config.Width, Config.Height);
		}

		float defColors[4] = { 0.1f,.1f,.1,1.0f };
		s_LightingFrameBuffer->Bind();
		s_LightingFrameBuffer->ClearAttachment(0, defColors);
		s_LightingFrameBuffer->ClearDepthAttachment(1.0f);
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
