#include "KDpch.h"
#include "OpenGLCopier.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/OpenGL/OpenGLTextureArray.h"
namespace Kaidel {

	bool OpenGLTextureCopier::Copy(const TextureHandle& dst, const TextureHandle& src){
		OpenGLTexture2D* dstTexture = (OpenGLTexture2D*)dst.Texture.Get();
		OpenGLTexture2D* srcTexture = (OpenGLTexture2D*)src.Texture.Get();

		glCopyImageSubData(srcTexture->m_RendererID, GL_TEXTURE_2D, 0, 0, 0, 0, dstTexture->m_RendererID, GL_TEXTURE_2D, 0, 0, 0, 0, srcTexture->m_Width, srcTexture->m_Height, 1);
		return true;
	}

	bool OpenGLTextureCopier::Copy(const TextureHandle& dst, const FramebufferAttachmentHandle& src){
		OpenGLTexture2D* dstTexture = (OpenGLTexture2D*)dst.Texture.Get();
		OpenGLFramebuffer* srcFramebuffer = (OpenGLFramebuffer*)src.Framebuffer.Get();
		if (src.AttachmentIndex == -1) {
			glCopyImageSubData(srcFramebuffer->m_DepthAttachment, GL_TEXTURE_2D, 0, 0, 0, 0, dstTexture->m_RendererID, GL_TEXTURE_2D, 0, 0, 0, 0, srcFramebuffer->m_Specification.Width, srcFramebuffer->m_Specification.Height, 1);
		}
		else {
			glCopyImageSubData(srcFramebuffer->m_ColorAttachments.at(src.AttachmentIndex), GL_TEXTURE_2D, 0, 0, 0, 0, dstTexture->m_RendererID, GL_TEXTURE_2D, 0, 0, 0, 0, srcFramebuffer->m_Specification.Width, srcFramebuffer->m_Specification.Height, 1);
		}
		return true;
	}
	bool OpenGLTextureCopier::Copy(const TextureHandle& dst, const TextureArrayHandle& src){
		OpenGLTexture2D* dstTexture = (OpenGLTexture2D*)dst.Texture.Get();
		OpenGLTexture2DArray* srcArray = (OpenGLTexture2DArray*)src.Array.Get();

		glCopyImageSubData(srcArray->m_RendererID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, (GLint)src.SlotIndex, dstTexture->m_RendererID, GL_TEXTURE_2D, 0, 0, 0, 0, srcArray->m_Width, srcArray->m_Height, 1);
		return true;
	}
	bool OpenGLTextureCopier::Copy(const FramebufferAttachmentHandle& dst, const TextureHandle& src){
		OpenGLFramebuffer* dstFramebuffer = (OpenGLFramebuffer*)dst.Framebuffer.Get();
		OpenGLTexture2D* srcTexture = (OpenGLTexture2D*)src.Texture.Get();
		
		if (dst.AttachmentIndex == -1) {
			glCopyImageSubData(srcTexture->m_RendererID, GL_TEXTURE_2D, 0, 0, 0, 0, dstFramebuffer->m_DepthAttachment, GL_TEXTURE_2D, 0, 0, 0, 0, dstFramebuffer->m_Specification.Width, dstFramebuffer->m_Specification.Height, 1);
		}
		else {
			glCopyImageSubData(srcTexture->m_RendererID, GL_TEXTURE_2D, 0, 0, 0, 0, dstFramebuffer->m_ColorAttachments.at(dst.AttachmentIndex), GL_TEXTURE_2D, 0, 0, 0, 0, srcTexture->m_Width, srcTexture->m_Height, 1);
		}
		return true;
	}
	bool OpenGLTextureCopier::Copy(Ref<Framebuffer> dst, Ref<Framebuffer> src){
		OpenGLFramebuffer* dstFramebuffer = (OpenGLFramebuffer*)dst.Get();
		OpenGLFramebuffer* srcFramebuffer = (OpenGLFramebuffer*)src.Get();

		glBlitNamedFramebuffer(srcFramebuffer->m_RendererID, dstFramebuffer->m_RendererID,
			0, 0, srcFramebuffer->m_Specification.Width, srcFramebuffer->m_Specification.Height, 0, 0, dstFramebuffer->m_Specification.Width, dstFramebuffer->m_Specification.Height,
			GL_COLOR_BUFFER_BIT,
			GL_LINEAR);
		return true;
	}
	bool OpenGLTextureCopier::Copy(const FramebufferAttachmentHandle& dst, const TextureArrayHandle& src){
		OpenGLFramebuffer* dstFramebuffer = (OpenGLFramebuffer*)dst.Framebuffer.Get();
		OpenGLTexture2DArray* srcArray = (OpenGLTexture2DArray*)src.Array.Get();

		if (dst.AttachmentIndex == -1) {
			glCopyImageSubData(srcArray->m_RendererID, GL_TEXTURE_2D_ARRAY,0,0,0,(GLint)src.SlotIndex, dstFramebuffer->m_DepthAttachment, GL_TEXTURE_2D, 0, 0, 0, 0, srcArray->m_Width, srcArray->m_Height, 1);
		}
		else {
			glCopyImageSubData(srcArray->m_RendererID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, (GLint)src.SlotIndex, dstFramebuffer->m_ColorAttachments.at(dst.AttachmentIndex), GL_TEXTURE_2D, 0, 0, 0, 0, srcArray->m_Width, srcArray->m_Height, 1);
		}
		return true;
	}
	bool OpenGLTextureCopier::Copy(const TextureArrayHandle& dst, const TextureHandle& src){
		OpenGLTexture2DArray* dstArray= (OpenGLTexture2DArray*)dst.Array.Get();
		OpenGLTexture2D* srcTexture = (OpenGLTexture2D*)src.Texture.Get();

		glCopyImageSubData(srcTexture->m_RendererID, GL_TEXTURE_2D, 0, 0, 0, 0, dstArray->m_RendererID, GL_TEXTURE_2D, 0, 0, 0, (GLint)dst.SlotIndex, srcTexture->m_Width, srcTexture->m_Height, 1);
		return true;
	}
	bool OpenGLTextureCopier::Copy(const TextureArrayHandle& dst, const FramebufferAttachmentHandle& src){
		OpenGLTexture2DArray* dstArray = (OpenGLTexture2DArray*)dst.Array.Get();
		OpenGLFramebuffer* srcFramebuffer = (OpenGLFramebuffer*)src.Framebuffer.Get();

		if (src.AttachmentIndex == -1) {
			glCopyImageSubData(srcFramebuffer->m_DepthAttachment, GL_TEXTURE_2D, 0, 0, 0, 0, dstArray->m_RendererID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, (GLint)dst.SlotIndex, srcFramebuffer->m_Specification.Width, srcFramebuffer->m_Specification.Width, 1);
		}
		else {
			glCopyImageSubData(srcFramebuffer->m_ColorAttachments.at(src.AttachmentIndex), GL_TEXTURE_2D, 0, 0, 0, 0, dstArray->m_RendererID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, (GLint)dst.SlotIndex, srcFramebuffer->m_Specification.Width, srcFramebuffer->m_Specification.Width, 1);
		}
		return true;

	}
	bool OpenGLTextureCopier::Copy(const TextureArrayHandle& dst, const TextureArrayHandle& src) {
		OpenGLTexture2DArray* dstArray = (OpenGLTexture2DArray*)dst.Array.Get();
		OpenGLTexture2DArray* srcArray = (OpenGLTexture2DArray*)src.Array.Get();

		glCopyImageSubData(srcArray->m_RendererID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, (GLint)src.SlotIndex, dstArray->m_RendererID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, (GLint)dst.SlotIndex, srcArray->m_Width, srcArray->m_Height, 1);
		return true;
	}


}
