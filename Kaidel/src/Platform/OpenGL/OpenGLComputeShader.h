#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
namespace Kaidel {

	class OpenGLComputeShaderUAVInput : public UAVInput {
	public:
		OpenGLComputeShaderUAVInput(uint32_t count,uint32_t sizeofElement, void* data = nullptr);
		void SetBufferData(void* data, uint32_t count);
		~OpenGLComputeShaderUAVInput() ;
		void Bind(uint32_t slot)const;
		void Unbind()const;
	private:
		mutable uint32_t m_LastBoundSlot = 0;
		uint32_t m_RendererID = 0;
		uint32_t m_SizeofElement;
	};

	class OpenGLTypedBufferInput : public TypedBufferInput {
	public:
		OpenGLTypedBufferInput(TypedBufferInputDataType type, TypedBufferAccessMode accessMode, uint32_t width, uint32_t height, void* data = nullptr);
		~OpenGLTypedBufferInput();
		void SetBufferData(void* data, uint32_t width, uint32_t height);
		virtual uint64_t GetTextureID()const;
	private:
		void Bind(uint32_t slot)const override;
		void Unbind() const;
		uint32_t m_RendererID = 0;
		mutable uint32_t m_LastBoundSlot = 0;
		TypedBufferInputDataType m_InputType;
		TypedBufferAccessMode m_AccessMode;
		int32_t m_Format = 0;
		int32_t m_InternalFormat = 0;
	};




	class OpenGLComputeShader : public ComputeShader {
	public:
		OpenGLComputeShader(const std::string& filepath);
		~OpenGLComputeShader();
		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void SetUAVInput(Ref<UAVInput> uav, uint32_t slot = 0);
		virtual void SetTypedBufferInput(Ref<TypedBufferInput> tbi, uint32_t slot);
		virtual void SetInt(const std::string& name, int value);
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count);
		virtual void SetFloat(const std::string& name, float value);
		virtual void SetFloat2(const std::string& name, const glm::vec2& value);
		virtual void SetFloat3(const std::string& name, const glm::vec3& value);
		virtual void SetFloat4(const std::string& name, const glm::vec4& value);
		virtual void SetMat4(const std::string& name, const glm::mat4& value);


		virtual void Execute(uint64_t x, uint64_t y, uint64_t z) const override;
		virtual void Wait()const;

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_LocalX, m_LocalY, m_LocalZ;
	};
}
