#include "KDpch.h"
#include "Copier.h"
#include "Kaidel/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLCopier.h"
namespace Kaidel {
	bool TextureCopier::Copy(const TextureHandle& dst, const TextureHandle& src){
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return OpenGLTextureCopier::Copy(dst, src);
		}
		KD_CORE_ASSERT(false, "Unknown Renderer API");
		return false;
	}
	bool TextureCopier::Copy(const TextureHandle& dst, const FramebufferAttachmentHandle& src){
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return OpenGLTextureCopier::Copy(dst, src);
		}
		KD_CORE_ASSERT(false, "Unknown Renderer API");
		return false;
	}
	bool TextureCopier::Copy(const TextureHandle& dst, const TextureArrayHandle& src){
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return OpenGLTextureCopier::Copy(dst, src);
		}
		KD_CORE_ASSERT(false, "Unknown Renderer API");
		return false;
	}
	bool TextureCopier::Copy(const FramebufferAttachmentHandle& dst, const TextureHandle& src){
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return OpenGLTextureCopier::Copy(dst, src);
		}
		KD_CORE_ASSERT(false, "Unknown Renderer API");
		return false;
	}
	bool TextureCopier::Copy(Ref<Framebuffer> dst, Ref<Framebuffer> src){
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return OpenGLTextureCopier::Copy(dst, src);
		}
		KD_CORE_ASSERT(false, "Unknown Renderer API");
		return false;
	}
	bool TextureCopier::Copy(const FramebufferAttachmentHandle& dst, const TextureArrayHandle& src){
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return OpenGLTextureCopier::Copy(dst, src);
		}
		KD_CORE_ASSERT(false, "Unknown Renderer API");
		return false;
	}
	bool TextureCopier::Copy(const TextureArrayHandle& dst, const TextureHandle& src){
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return OpenGLTextureCopier::Copy(dst, src);
		}
		KD_CORE_ASSERT(false, "Unknown Renderer API");
		return false;
	}
	bool TextureCopier::Copy(const TextureArrayHandle& dst, const FramebufferAttachmentHandle& src){
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return OpenGLTextureCopier::Copy(dst, src);
		}
		KD_CORE_ASSERT(false, "Unknown Renderer API");
		return false;
	}
	bool TextureCopier::Copy(const TextureArrayHandle& dst, const TextureArrayHandle& src) {
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return OpenGLTextureCopier::Copy(dst, src);
		}
		KD_CORE_ASSERT(false, "Unknown Renderer API");
		return false;
	}
}
