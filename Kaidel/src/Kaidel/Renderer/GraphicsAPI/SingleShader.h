#pragma once


#include "Kaidel/Core/Base.h"


namespace Kaidel {

	enum class ShaderType;

	struct SingleShaderSpecification {
		std::string ControlString;
		bool IsPath = true;
		ShaderType Type;
	};

	class SingleShader : public IRCCounter<false>{
	public:
		virtual ~SingleShader() = default;

		static Ref<SingleShader> CreateShader(const SingleShaderSpecification& specification);

		virtual const SingleShaderSpecification& GetSpecification() const = 0;

	private:

	};
}
