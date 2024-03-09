#pragma once
#include "Kaidel/Core/Base.h"
namespace Kaidel {

	struct TextureHandle;
	struct TextureArrayHandle;

	class TextureView : public IRCCounter<false> {
	public:
		static Ref<TextureView> Create(const TextureHandle& handle);
		static Ref<TextureView> Create(const TextureArrayHandle& handle);

		virtual uint64_t GetRendererID()const = 0;

		virtual void Bind(uint32_t slot = 0)const =0;
		virtual void Unbind()const = 0;


		virtual ~TextureView() = default;

	};
}
