#pragma once
#include "Kaidel/Core/Base.h"

#include <glm/glm.hpp>


namespace Kaidel {
	struct Plane {
		glm::vec3 normal;
		float d = 0;

		Plane() = default;

		void normalize() {
			float l = glm::length(normal);
			if (l == 0) {
				*this = Plane(0, 0, 0, 0);
				return;
			}
			normal /= l;
			d /= l;
		}
		Plane(float p_a, float p_b, float p_c, float p_d) :
			normal(p_a, p_b, p_c),
			d(p_d) {}

	};

	struct Basis {
		glm::vec3 rows[3] = {
			glm::vec3(1, 0, 0),
			glm::vec3(0, 1, 0),
			glm::vec3(0, 0, 1)
		};

		glm::vec3 get_column(int p_index) const {
			// Get actual basis axis column (we store transposed as rows for performance).
			return glm::vec3(rows[0][p_index], rows[1][p_index], rows[2][p_index]);
		}

		void set_column(int p_index, const glm::vec3& p_value) {
			// Set actual basis axis column (we store transposed as rows for performance).
			rows[0][p_index] = p_value.x;
			rows[1][p_index] = p_value.y;
			rows[2][p_index] = p_value.z;
		}

		void orthonormalize();



	};


	struct Transform3D {
		Basis basis;
		glm::vec3 origin;

		void orthonormalize();

		Plane xform_fast(const Plane& p_plane, const Basis& p_basis_inverse_transpose) const {
			// Transform a single point on the plane.
			glm::vec3 point = p_plane.normal * p_plane.d;
			point = xform(point);

			// Use inverse transpose for correct normals with non-uniform scaling.
			glm::vec3 normal = p_basis_inverse_transpose.xform(p_plane.normal);
			normal = glm::normalize(normal);

			glm::vec3 d = glm::dot(normal,point);
			return Plane(normal, d);
		}

		Plane xform(const Plane& p_plane) const {
			Basis b = basis.inverse();
			b.transpose();
			return xform_fast(p_plane, b);
		}

	};
}
