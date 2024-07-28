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
}
