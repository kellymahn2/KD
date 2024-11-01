#include "KDpch.h"
#include "Transform.h"

namespace Kaidel {

	void Basis::orthonormalize() {
		// Gram-Schmidt Process

		glm::vec3 x = get_column(0);
		glm::vec3 y = get_column(1);
		glm::vec3 z = get_column(2);

		x = glm::normalize(x);
		y = (y - x * (glm::dot(x,y)));
		y = glm::normalize(y);
		z = (z - x * (glm::dot(x,z)) - y * (glm::dot(y,z)));
		z = glm::normalize(z);

		set_column(0, x);
		set_column(1, y);
		set_column(2, z);
	}

	void Transform3D::orthonormalize() {
		basis.orthonormalize();
	}
}
