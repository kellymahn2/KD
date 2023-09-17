#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace Math {

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
	
	template<typename T, glm::qualifier Q>
	glm::mat<4, 4, T, Q> Translate(glm::mat<4, 4, T, Q> const& m, glm::vec<3, T, Q> const& v) {
		return glm::translate(m, v);
	}
	template<typename T, glm::qualifier Q>
		glm::mat<4, 4, T, Q> Translate(glm::vec<3, T, Q> const& v) {
		return glm::translate(glm::mat4(1.0f), v);
	}


}
