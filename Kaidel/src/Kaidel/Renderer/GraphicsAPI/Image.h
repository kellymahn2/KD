#pragma once
#include "Kaidel/Core/Base.h"

using ImageShaderID = uint64_t;


namespace Kaidel {

	enum class ImageLayout {
		Undefined = 0,
		General,
		ColorOptimal,
		DepthStencilOptimal,
		DepthStencilReadOptimal,
		ShaderReadOptimal,
		TransferSrc,
		TransferDst,
		DepthOptimal ,
		DepthReadOptimal,
		StencilOptimal,
		StencilReadOptimal,
		ReadOptimal,
		AttachmentOptimal,
		PresentSrc,
	};



	class Image {
	public:
		Image() = default;
		Image(ImageLayout currentLayout)
			:m_CurrentLayout(currentLayout)
		{}
		virtual ~Image() = default;
		virtual void TransitionLayout(ImageLayout finalLayout) = 0;
		ImageLayout GetCurrentLayout()const { return m_CurrentLayout; }

		void AssureLayout(ImageLayout finalLayout){
			if (m_CurrentLayout != finalLayout)
				TransitionLayout(finalLayout);
		}

		virtual ImageShaderID GetShaderID()const = 0;

	protected:
		ImageLayout m_CurrentLayout = ImageLayout::Undefined;
	};


	class FramebufferImage : public Image {
	public:
		FramebufferImage() = default;
		FramebufferImage(ImageShaderID shaderID, ImageLayout currentLayout) 
			:Image(currentLayout), m_ShaderID(shaderID)
		{}
		virtual ~FramebufferImage() = default;
		virtual ImageShaderID GetShaderID()const override { return m_ShaderID; }
	protected:
		ImageShaderID m_ShaderID;
	};

}
