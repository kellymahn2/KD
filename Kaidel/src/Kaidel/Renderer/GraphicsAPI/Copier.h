#pragma once
#include "Core.h"
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Kaidel/Renderer/GraphicsAPI/TextureArray.h"

namespace Kaidel {
	class TextureCopier{
	public:
		
		static bool Copy(const TextureHandle& dst,const TextureHandle& src);
		static bool Copy(const TextureHandle& dst, const FramebufferAttachmentHandle& src);
		static bool Copy(const TextureHandle& dst, const TextureArrayHandle& src);

		static bool Copy(const FramebufferAttachmentHandle& dst, const TextureHandle& src);
		static bool Copy(Ref<Framebuffer> dst, Ref<Framebuffer> src);
		static bool Copy(const FramebufferAttachmentHandle& dst, const TextureArrayHandle& src);

		static bool Copy(const TextureArrayHandle& dst, const TextureHandle& src);
		static bool Copy(const TextureArrayHandle& dst, const FramebufferAttachmentHandle& src);
		static bool Copy(const TextureArrayHandle& dst,const TextureArrayHandle& src);

	};
}
