#include "KDpch.h"
#include "Kaidel/Core/ImageResize.h"
#include "OpenGLTextureArray.h"
#include "OpenGLTextureFormat.h"
#include <stb_image.h>
#include <stb_image_resize.h>

#include <glm/glm.hpp>
namespace Kaidel {


	namespace Utils {
		static GLint CreateTextureArray(uint32_t width,uint32_t height,uint32_t depth,TextureFormat format) {
			uint32_t texture = 0;
			glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
			glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, Utils::KaidelTextureFormatToGLInternalFormat(format), width, height, depth);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			return texture;
		}

		static ArraySlotSpecification MakeSpec(void* data,uint32_t width,uint32_t height,bool shouldFlip) {
			ArraySlotSpecification spec;
			spec.ShouldFlip = shouldFlip;
			spec.LoadedWidth = width;
			spec.LoadedHeight = height;
			spec.Data.resize(width * height + 1);
			spec.Valid = true;

			if(data!=nullptr)
				std::memcpy(spec.Data.data(), data, width * height);
			return spec;
		}
		static ArraySlotSpecification MakeSpec(const std::string& src, uint32_t width, uint32_t height, bool shouldFlip) {
			ArraySlotSpecification spec;

			spec.ShouldFlip = shouldFlip;
			spec.Source = src;
			spec.LoadedWidth = width;
			spec.LoadedHeight = height;
			spec.Valid = true;
			return spec;
		}

		static bool NeedsResize(uint32_t w, uint32_t h, uint32_t expectedWidth, uint32_t expectedHeight) { return w != expectedWidth || h != expectedHeight; }


		

		static uint32_t ResizeTextureArray(uint32_t oldTexture, uint32_t w, uint32_t h, uint32_t d, uint32_t setCount, TextureFormat format, uint32_t newDepth) {

			uint32_t newTexture = CreateTextureArray(w, h, newDepth, format);
			glCopyImageSubData(oldTexture, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, newTexture, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, w, h, setCount);
			glDeleteTextures(1, &oldTexture);

			return newTexture;
		}


		static void PushTextureData(uint32_t array,uint32_t width,uint32_t height,uint32_t setCount,TextureFormat format,void* data) {
			glBindTexture(GL_TEXTURE_2D_ARRAY, array);
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, setCount, width, height, 1, Utils::KaidelTextureFormatToGLFormat(format), GL_UNSIGNED_BYTE, data);
		}
		static void ReplaceTextureData(uint32_t array, uint32_t width, uint32_t height, uint32_t slot, TextureFormat format, void* data) {
			glBindTexture(GL_TEXTURE_2D_ARRAY, array);
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, slot , width, height, 1, Utils::KaidelTextureFormatToGLFormat(format), GL_UNSIGNED_BYTE, data);
		}




		static void InsertTextureData(uint32_t& array, uint32_t width, uint32_t height,uint32_t& depth,uint32_t& setCount, uint32_t slot, TextureFormat format, void* data) {


			//Storage doesn't exist and out of bounds
			if (slot >= depth) {
				array = ResizeTextureArray(array, width, height, depth, setCount, format, (slot + 1) * 2);
				depth = (slot + 1) * 2;
				setCount = slot + 1;
				if (data == nullptr)
					return;
				ReplaceTextureData(array, width, height, slot, format, data);
				return;
			}

			//Storage already exists but out of bounds
			if (slot >= setCount) {
				ReplaceTextureData(array, width, height, slot, format, data);
				setCount = slot + 1;
				if (data == nullptr)
					return;
				return;
			}

			//Storage already exists and inside bounds
			{
				//Outside depth
				if (setCount + 1 > depth) {
					uint32_t newArray = CreateTextureArray(width, height, (setCount + 1) * 2,format);
					//Copy from [0,slot-1]
					glCopyImageSubData(array, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, newArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, slot);
					if (data != nullptr) {
						//Set data
						ReplaceTextureData(newArray, width, height, slot, format, data);
					}
					//Copy from [slot,setCount)
					glCopyImageSubData(array, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, newArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, slot + 1, width, height, setCount-slot);
					array = newArray;
					depth = (setCount + 1) * 2;
					setCount++;
				}
				else {
					//Copy from i:[slot,setCount) to i + 1
					for (uint32_t i = setCount - 1; i >= slot; --i) {
						glCopyImageSubData(array, GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, array, GL_TEXTURE_2D_ARRAY, 0, 0, 0, i + 1, width, height, 1);
					}
					if (data == nullptr)
						return;
					//Set data
					ReplaceTextureData(array, width, height, slot, format, data);
					setCount++;
				}
			}
		}



		

	}



	void OpenGLTexture2DArray::PushWithResize(void* data, uint32_t width, uint32_t height) {

		if (m_SetCount + 1 > m_Depth) {
			ResizeTextureArray((m_SetCount + 1) * 2);
		}

		if (data == nullptr)
			return;

		void* sclImage = Utils::ResizeTexture(data, m_TextureFormat, width, height, m_Width, m_Height);
		Utils::PushTextureData(m_RendererID, m_Width, m_Height,m_SetCount, m_TextureFormat, sclImage);
		delete[] sclImage;
	}

	void OpenGLTexture2DArray::Push(void* data) {
		if (m_SetCount + 1 > m_Depth) {
			ResizeTextureArray((m_SetCount + 1) * 2);
		}
		if (data == nullptr)
			return;
		Utils::PushTextureData(m_RendererID, m_Width, m_Height,m_SetCount, m_TextureFormat, data);
	}

	void OpenGLTexture2DArray::ReplaceWithResize(void* data, uint32_t width, uint32_t height, uint32_t slot) {

		if (data == nullptr)
			return;
		void* sclImage = Utils::ResizeTexture(data, m_TextureFormat, width, height, m_Width, m_Height);
		Utils::ReplaceTextureData(m_RendererID, m_Width, m_Height, slot, m_TextureFormat, sclImage);
		delete[] sclImage;
	}
	void OpenGLTexture2DArray::Replace(void* data,uint32_t slot) {
		if (data == nullptr)
			return;
		Utils::ReplaceTextureData(m_RendererID, m_Width, m_Height, slot, m_TextureFormat, data);
	}


	void OpenGLTexture2DArray::InsertWithResize(void* data, uint32_t width, uint32_t height, uint32_t slot) {
		void* sclImage = Utils::ResizeTexture(data, m_TextureFormat, width, height, m_Width, m_Height);
		uint32_t arrayTmp = m_RendererID;
		Utils::InsertTextureData(arrayTmp, m_Width, m_Height, m_Depth, m_SetCount, slot, m_TextureFormat, sclImage);
		delete[] sclImage;
		if (arrayTmp != m_RendererID) {
			glDeleteTextures(1, &m_RendererID);
			m_RendererID = arrayTmp;
		}
	}
	void OpenGLTexture2DArray::Insert(void* data, uint32_t slot){
		uint32_t arrayTmp = m_RendererID;
		Utils::InsertTextureData(arrayTmp, m_Width,m_Height, m_Depth, m_SetCount, slot, m_TextureFormat, data);
		if (arrayTmp != m_RendererID) {
			glDeleteTextures(1, &m_RendererID);
			m_RendererID = arrayTmp;
		}
	}


	OpenGLTexture2DArray::OpenGLTexture2DArray(uint32_t width, uint32_t height, TextureFormat textureFormat, bool loadsSpecs)
		:m_Width(width), m_Height(height), m_Depth(2),m_TextureFormat(textureFormat),m_LoadsSpecs(loadsSpecs)
	{
		m_RendererID = Utils::CreateTextureArray(m_Width, m_Height, m_Depth, m_TextureFormat);
		

	}
	OpenGLTexture2DArray::~OpenGLTexture2DArray() {
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		glDeleteTextures(1, &m_RendererID);
	}
	
	void OpenGLTexture2DArray::Bind(uint32_t slot)const {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
	}

	uint32_t OpenGLTexture2DArray::PushTexture(void* data, uint32_t width, uint32_t height, bool shouldFlip) {
		Utils::NeedsResize(width, height, m_Width, m_Height) ? PushWithResize(data, width, height) : Push(data);

		ArraySlotSpecification spec = Utils::MakeSpec(data, width, height, shouldFlip);
		m_Specifications.push_back(spec);

		uint32_t ret = m_SetCount++;
		RecreateViews();
		return ret;
	}
	uint32_t OpenGLTexture2DArray::PushTexture(const std::string& src, bool shouldFlip) {

		int w, h, channels;
		uint8_t* data;
		{
			Timer timer("Image Loading");
			stbi_set_flip_vertically_on_load(shouldFlip);
			data = stbi_load(src.c_str(), &w, &h, &channels, 4);
		}
		KD_CORE_ASSERT(data);
		
		Utils::NeedsResize(w, h, m_Width, m_Height) ? PushWithResize(data, w, h) : Push(data);

		ArraySlotSpecification spec = Utils::MakeSpec(src, m_Width, m_Height, shouldFlip);
		m_Specifications.push_back(spec);

		uint32_t ret = m_SetCount++;
		RecreateViews();
		return ret;
	}
	
	void OpenGLTexture2DArray::ResizeTextureArray(uint32_t newLayerCount) {
		m_RendererID = Utils::ResizeTextureArray(m_RendererID, m_Width, m_Height, m_Depth, m_SetCount, m_TextureFormat, newLayerCount);
		m_Depth = newLayerCount;

		RecreateViews();
		m_Specifications.reserve(newLayerCount);
	}
	
	TextureArrayHandle OpenGLTexture2DArray::GetHandle(uint32_t index) const {
		TextureArrayHandle handle{};
		handle.Array = const_cast<OpenGLTexture2DArray*>(this);
		handle.SlotIndex = index;
		return handle;
	}

	Ref<TextureView> OpenGLTexture2DArray::GetView(uint32_t index) {
		auto& view = m_GeneratedViews.at(index);
		if (view)
			return view;
		return view = TextureView::Create(GetHandle(index));
	}

	void OpenGLTexture2DArray::ClearLayer(uint32_t slot, const float* data)const {
		glClearTexSubImage(m_RendererID, 0, 0, 0, slot, m_Width, m_Height, 1, Utils::KaidelTextureFormatToGLFormat(m_TextureFormat), GL_FLOAT, data);
	}
	void OpenGLTexture2DArray::Clear(const float* data) const{
		glClearTexImage(m_RendererID, 0, Utils::KaidelTextureFormatToGLFormat(m_TextureFormat), GL_FLOAT, data);
	}

	#pragma region SpecHandling


	void OpenGLTexture2DArray::InsertSpec(const ArraySlotSpecification& spec,uint32_t slot) {
		//Storage doesn't exist and out of bounds
		if (slot >= m_Specifications.capacity()) {
			m_Specifications.reserve((slot + 1) * 2);
			m_Specifications.resize(slot + 1);
			m_Specifications.at(slot) = spec;
			return;
		}

		//Storage already exists but out of bounds
		if (slot >= m_Specifications.size()) {
			m_Specifications.resize(slot + 1);
			m_Specifications.at(slot) = spec;
			return;
		}

		//Storage already exists and inside bounds
		{
			m_Specifications.insert(m_Specifications.begin() + slot, spec);
		}
	}

	void OpenGLTexture2DArray::InsertView(uint32_t slot) {
		//Storage doesn't exist and out of bounds
		if (slot >= m_GeneratedViews.capacity()) {
			m_GeneratedViews.reserve((slot + 1) * 2);
			m_GeneratedViews.resize(slot + 1);
			m_GeneratedViews.at(slot) = TextureView::Create(GetHandle(slot));
			return;
		}

		//Storage already exists but out of bounds
		if (slot >= m_GeneratedViews.size()) {
			m_GeneratedViews.resize(slot + 1);
			m_GeneratedViews.at(slot) = TextureView::Create(GetHandle(slot));
			return;
		}

		//Storage already exists and inside bounds
		{
			m_GeneratedViews.insert(m_GeneratedViews.begin() + slot, TextureView::Create(GetHandle(slot)));

		}
	}
	 

	#pragma endregion


	void OpenGLTexture2DArray::InsertOrReplaceTexture(uint32_t slot, void* data, uint32_t width, uint32_t height, bool shouldFlip) {
		if (slot > m_SetCount) {
			InsertTexture(slot, data, width, height, shouldFlip);
		}
		else {
			ReplaceTexture(slot, data, width, height,shouldFlip);
		}
	}
	void OpenGLTexture2DArray::InsertOrReplaceTexture(uint32_t slot, const std::string& src, bool shouldFlip){
		if (slot >= m_SetCount) {
			InsertTexture(slot, src,shouldFlip);
		}
		else {
			ReplaceTexture(slot, src,shouldFlip);
		}
	}

	void OpenGLTexture2DArray::RecreateViews() {
		m_GeneratedViews.resize(m_SetCount);
		for (uint32_t i = 0; i < m_GeneratedViews.size(); ++i) {
			m_GeneratedViews.at(i) = TextureView::Create(GetHandle(i));
		}
	}

	void OpenGLTexture2DArray::RecreateViews(uint32_t start,uint32_t end) {
		for (uint32_t i = start; i < m_GeneratedViews.size() && i < end; ++i) {
			m_GeneratedViews.at(i) = TextureView::Create(GetHandle(i));
		}
	}

	void OpenGLTexture2DArray::InsertTexture(uint32_t slot, void* data, uint32_t width, uint32_t height, bool shouldFlip){
		Utils::NeedsResize(width, height, m_Width, m_Height) ? InsertWithResize(data, width, height, slot) : Insert(data, slot);
		
		ArraySlotSpecification spec = Utils::MakeSpec(data, width, height, shouldFlip);
		InsertSpec(spec, slot);
		RecreateViews();
	}
	void OpenGLTexture2DArray::InsertTexture(uint32_t slot, const std::string& src, bool shouldFlip){
		int w, h, channels;
		uint8_t* data;
		{
			Timer timer("Image Loading");
			stbi_set_flip_vertically_on_load(shouldFlip);
			data = stbi_load(src.c_str(), &w, &h, &channels, 4);
		}
		KD_CORE_ASSERT(data);
		Utils::NeedsResize(w, h, m_Width, m_Height) ? InsertWithResize(data, w, h, slot) : Insert(data, slot);

		ArraySlotSpecification spec = Utils::MakeSpec(src, m_Width, m_Height, shouldFlip);
		InsertSpec(spec, slot);
		RecreateViews();
	}
	
	void OpenGLTexture2DArray::ReplaceTexture(uint32_t slot, void* data, uint32_t width, uint32_t height, bool shouldFlip){
		Utils::NeedsResize(width, height, m_Width, m_Height) ? ReplaceWithResize(data,width,height,slot) : Replace(data,slot);

		ArraySlotSpecification spec = Utils::MakeSpec(data, width, height, shouldFlip);
		m_Specifications.at(slot) = spec;
	}
	void OpenGLTexture2DArray::ReplaceTexture(uint32_t slot, const std::string& src, bool shouldFlip) {
		int w, h, channels;
		uint8_t* data;
		{
			Timer timer("Image Loading");
			stbi_set_flip_vertically_on_load(shouldFlip);
			data = stbi_load(src.c_str(), &w, &h, &channels, 4);
		}
		KD_CORE_ASSERT(data);

		Utils::NeedsResize(w, h, m_Width, m_Height) ? ReplaceWithResize(data, w, h, slot) : Replace(data, slot);

		ArraySlotSpecification spec = Utils::MakeSpec(src, m_Width, m_Height, shouldFlip);
		m_Specifications.at(slot) = spec;
	}


}
