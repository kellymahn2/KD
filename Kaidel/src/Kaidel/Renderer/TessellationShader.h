#pragma once
#include "Kaidel/Core/Base.h"
#include "glm/glm.hpp"
namespace Kaidel {

	class TessellationShader {
	public:
		static Ref<TessellationShader> Create(const FileSystem::path& tcsPath, const FileSystem::path& tesPath);

		virtual ~TessellationShader() {}

		virtual void Bind() = 0;
		virtual void Unbind() = 0;


		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

	};


}
