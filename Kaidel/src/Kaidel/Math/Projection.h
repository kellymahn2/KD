#pragma once
#include "Kaidel/Core/Base.h"
#include "Transform.h"
#include <glm/glm.hpp>

namespace Kaidel {
	template <typename T>
	class Vector;

	struct Transform3D;

	

	struct Projection {
		enum Planes {
			PLANE_NEAR,
			PLANE_FAR,
			PLANE_LEFT,
			PLANE_TOP,
			PLANE_RIGHT,
			PLANE_BOTTOM
		};

		glm::vec4 columns[4];

		void set_perspective(float p_fovy_degrees, float p_aspect, float p_z_near, float p_z_far, bool p_flip_fov = false);

		void set_identity();
		bool get_endpoints(const Transform3D& p_transform, glm::vec3* p_8points) const;

		std::vector<Plane> get_projection_planes(const Transform3D& p_transform) const;


		Projection();
		Projection(const glm::vec4& p_x, const glm::vec4& p_y, const glm::vec4& p_z, const glm::vec4& p_w);
		Projection(const Transform3D& p_transform);
		~Projection();
	};
}
