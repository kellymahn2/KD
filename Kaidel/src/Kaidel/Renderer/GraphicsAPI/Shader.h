#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include "Buffer.h"
#include <glm/glm.hpp>
namespace Kaidel {


	enum class ShaderType {
		None,
		VertexShader,
		FragmentShader,
		GeometryShader,
		TessellationControlShader,
		TessellationEvaluationShader
	};

	struct ShaderSpecification {
		std::string EntryPoint;
		std::vector<uint32_t> SPIRV;
		ShaderType Type;
	};

	class Shader : public IRCCounter<false>
	{
	public:

		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		//virtual void SetInt(const std::string& name, int value) = 0;
		//virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		//virtual void SetFloat(const std::string& name, float value) = 0;
		//virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		//virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		//virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		//virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual RendererID GetRendererID()const = 0;

		virtual const ShaderSpecification& GetSpecification()const = 0;
		
		static Ref<Shader> Create(const ShaderSpecification& specification);
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
	class TypedBufferInput : public IRCCounter<false> {
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


	class UAVInput : public IRCCounter<false> {
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

	class ComputeShader : public IRCCounter<false> {
	public:
		static Ref<ComputeShader> Create(const std::string& filepath);
		virtual ~ComputeShader() {}
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void SetUAVInput(Ref<UAVInput> uav,uint32_t slot = 0) = 0;
		virtual void SetTypedBufferInput(Ref<TypedBufferInput> tbi, uint32_t slot) = 0;
		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual void Execute(uint64_t x, uint64_t y, uint64_t z) const = 0;
		virtual void Wait() const = 0;
	private:

	};

	struct ShaderSource {

		ShaderSource(const Path& path);

		std::string FinalSource;

		std::unordered_map<Path, ShaderSource> IncludedFiles;


	};


}
