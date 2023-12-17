#include "KDpch.h"
#include "Kaidel/Renderer/Shader.h"
#include "Light.h"
#include <cmath>

namespace Kaidel {

	

	float GetMaxPointLightCoverage(float constantCoefficient, float linearCoefficient, float quadraticCoefficient)
	{
		float a = quadraticCoefficient;
		float b = linearCoefficient;
		float c = constantCoefficient - (1 / std::numeric_limits<float>::epsilon());

		float discriminant = (b * b) - (4 * a * c);

		if (discriminant < 0.0)
			return INFINITY;

		float root1 = (-b + std::sqrt(discriminant)) / (2 * a);
		float root2 = (-b - std::sqrt(discriminant)) / (2 * a);

		return std::max(std::initializer_list<float>{root1, root2, 0.0f});
	}

}
