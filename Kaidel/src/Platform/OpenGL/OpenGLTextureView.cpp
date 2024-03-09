#include "KDpch.h"
#include "OpenGLTextureView.h"
#include "OpenGLTexture.h"
#include "OpenGLTextureArray.h"
#include "OpenGLTextureFormat.h"
#include <glad/glad.h>

namespace Kaidel {



	OpenGLTextureView::OpenGLTextureView(const TextureHandle& handle){

		Ref<OpenGLTexture2D> texture = handle.Texture;

		glGenTextures(1, &m_RendererID);

		glTextureView(m_RendererID, GL_TEXTURE_2D, texture->m_RendererID, Utils::KaidelTextureFormatToGLInternalFormat(texture->m_TextureFormat), 0, 1, 0, 1);
	}
	OpenGLTextureView::OpenGLTextureView(const TextureArrayHandle& handle){
		Ref<OpenGLTexture2DArray> textureArray = handle.Array;

		glGenTextures(1, &m_RendererID);
		
		glTextureView(m_RendererID, GL_TEXTURE_2D, textureArray->m_RendererID, Utils::KaidelTextureFormatToGLInternalFormat(textureArray->m_TextureFormat), 0, 1, handle.SlotIndex, 1);
	}
	OpenGLTextureView::~OpenGLTextureView() {
		glDeleteTextures(1, &m_RendererID);
	}


	void OpenGLTextureView::Bind(uint32_t slot)const {
		glBindTextureUnit(slot, m_RendererID);
		m_LastBoundSlot = slot;
	}
	void OpenGLTextureView::Unbind()const{
		if (m_LastBoundSlot == -1)
			return;
		glBindTextureUnit(m_LastBoundSlot, 0);
		m_LastBoundSlot = -1;
	}

}
