#include "KDpch.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/D3D/D3DShader.h"
#include "Platform/OpenGL/OpenGLComputeShader.h"
#include "Platform/D3D/D3DComputeShader.h"

namespace Kaidel {

	uint64_t UAVInput::s_UAVCount = 0;


	Ref<Shader> Shader::Create(const ShaderSpecification& specification) {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLShader>(specification);
		//case RendererAPI::API::DirectX: return CreateRef<D3DShader>(specification);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}





	Ref<UAVInput> UAVInput::Create(uint32_t count,uint32_t sizeofElement, void* data) {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLComputeShaderUAVInput>(count,sizeofElement,data);
		case RendererAPI::API::DirectX: return CreateRef<D3DUAVInput>(count,sizeofElement,data);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<TypedBufferInput> TypedBufferInput::Create(TypedBufferInputDataType type, TypedBufferAccessMode accessMode, uint32_t width, uint32_t height, void* data) {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLTypedBufferInput>(type,accessMode,width,height,data);
		case RendererAPI::API::DirectX: return CreateRef<D3DTypedBufferInput>(type,accessMode,width,height,data);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}




	Ref<ComputeShader> ComputeShader::Create(const std::string& filepath) {
		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:return CreateRef<OpenGLComputeShader>(filepath);
		//case RendererAPI::API::DirectX:return CreateRef<D3DComputeShader>(filepath);

		}
		KD_CORE_ASSERT(false, "Unkown RendererAPI!");

		return nullptr;
	}

}
