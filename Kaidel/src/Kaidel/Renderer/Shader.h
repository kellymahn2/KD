#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include "Buffer.h"
#include <glm/glm.hpp>
namespace Kaidel {

	typedef int ShaderType;
	class Shader
	{
	public:

		enum ShaderType_ {
			ShaderType_Vertex = 1<<0,
			ShaderType_Pixel = 1<<1,
			ShaderType_Fragment = ShaderType_Pixel,
			ShaderType_Geometry = 1<<2,
			ShaderType_Compute = 1<<3
		};

		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual const std::string& GetName() const = 0;

		static Ref<Shader> Create(const std::filesystem::path &vertexPath, const std::filesystem::path& fragmentPath, const std::string& name = "");
		static Ref<Shader> Create(const std::string& name,const std::string& vertexSrc, const std::string& fragmentSrc);

	protected:
		ShaderType m_ShaderType;
	};

	enum class ComputeShaderInputType {
		None =0,
		TypedBuffer = 1,
		UAV = 2
	};
	
	enum class TypedBufferInputDataType {
		None = 0,
		RGBA8 = 1,
	};
	enum TypedBufferAccessMode {
		Read = 1,
		Write = 2,
		ReadWrite = 3,
	};
	class TypedBufferInput{
	public:
		static Ref<TypedBufferInput> Create(TypedBufferInputDataType type, TypedBufferAccessMode accessMode,uint32_t width, uint32_t height, void* data = nullptr);
		virtual void SetBufferData(void* data, uint32_t width,uint32_t height) = 0;
		virtual ~TypedBufferInput() = default;
		virtual uint64_t GetTextureID()const=0;
	protected:
		virtual void Bind(uint32_t slot = 0)const = 0;
		virtual void Unbind() const = 0;
		friend class OpenGLComputeShader;
		friend class D3DComputeShader;
	};
	class UAVInput{
	public:
		static Ref<UAVInput> Create(uint32_t count,uint32_t sizeofElement,void* data = nullptr);
		virtual void SetBufferData(void* data, uint32_t count) = 0;
		virtual ~UAVInput() = default;
		virtual void Bind(uint32_t slot = 0)const = 0;
		virtual void Unbind() const = 0;

	protected:
		friend class OpenGLComputeShader;
		friend class D3DComputeShader;
		static uint64_t s_UAVCount;
	};
	



	class ComputeShader {
	public:
		static Ref<ComputeShader> Create(const std::string& filepath);
		virtual ~ComputeShader() {}
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void SetUAVInput(Ref<UAVInput> uav,uint32_t slot = 0) = 0;
		virtual void SetTypedBufferInput(Ref<TypedBufferInput> tbi, uint32_t slot) = 0;
		virtual void Execute(uint64_t x, uint64_t y, uint64_t z) const = 0;
		virtual void Wait() const = 0;
	private:

	};


	class ShaderLibrary
	{
	public:
		void Add(const std::string& name, const Ref<Shader>& shader);
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& name, const std::string& filepath);

		Ref<Shader> Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};

}
