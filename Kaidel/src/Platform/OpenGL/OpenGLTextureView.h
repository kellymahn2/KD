#pragma once

#include "Kaidel/Renderer/GraphicsAPI/TextureView.h"

namespace Kaidel {

	class OpenGLTextureView : public TextureView{
	public:
		OpenGLTextureView(const TextureHandle& handle);
		OpenGLTextureView(const TextureArrayHandle& handle);

		~OpenGLTextureView();

		virtual uint64_t GetRendererID()const override{ return m_RendererID; }

		virtual void Bind(uint32_t slot = 0)const override;
		virtual void Unbind()const override;

	private:
		uint32_t m_RendererID;
		mutable uint32_t m_LastBoundSlot;
	};
}
