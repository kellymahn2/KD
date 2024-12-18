#include "KDpch.h"
#include "Projection.h"


namespace Kaidel {
	void Projection::set_perspective(float p_fovy_degrees, float p_aspect, float p_z_near, float p_z_far, bool p_flip_fov) {
		//if (p_flip_fov) {
		//	p_fovy_degrees = get_fovy(p_fovy_degrees, 1.0 / p_aspect);
		//}

		float sine, cotangent, deltaZ;
		float radians = glm::radians(p_fovy_degrees / 2.0);

		deltaZ = p_z_far - p_z_near;
		sine = glm::sin(radians);

		if ((deltaZ == 0) || (sine == 0) || (p_aspect == 0)) {
			return;
		}
		cotangent = glm::cos(radians) / sine;

		set_identity();

		columns[0][0] = cotangent / p_aspect;
		columns[1][1] = cotangent;
		columns[2][2] = -(p_z_far + p_z_near) / deltaZ;
		columns[2][3] = -1;
		columns[3][2] = -2 * p_z_near * p_z_far / deltaZ;
		columns[3][3] = 0;
	}
	std::vector<Plane> Projection::get_projection_planes(const Transform3D& p_transform) const {
		/** Fast Plane Extraction from combined modelview/projection matrices.
		 * References:
		 * https://web.archive.org/web/20011221205252/https://www.markmorley.com/opengl/frustumculling.html
		 * https://web.archive.org/web/20061020020112/https://www2.ravensoft.com/users/ggribb/plane%20extraction.pdf
		 */

		std::vector<Plane> planes;
		planes.resize(6);

		const float* matrix = (const float*)columns;

		Plane new_plane;

		///////--- Near Plane ---///////
		new_plane = Plane(matrix[3] + matrix[2],
			matrix[7] + matrix[6],
			matrix[11] + matrix[10],
			matrix[15] + matrix[14]);

		new_plane.normal = -new_plane.normal;
		new_plane.normalize();

		planes[0] = p_transform.xform(new_plane);

		///////--- Far Plane ---///////
		new_plane = Plane(matrix[3] - matrix[2],
			matrix[7] - matrix[6],
			matrix[11] - matrix[10],
			matrix[15] - matrix[14]);

		new_plane.normal = -new_plane.normal;
		new_plane.normalize();

		planes[1] = p_transform.xform(new_plane);

		///////--- Left Plane ---///////
		new_plane = Plane(matrix[3] + matrix[0],
			matrix[7] + matrix[4],
			matrix[11] + matrix[8],
			matrix[15] + matrix[12]);

		new_plane.normal = -new_plane.normal;
		new_plane.normalize();

		planes[2] = p_transform.xform(new_plane);

		///////--- Top Plane ---///////
		new_plane = Plane(matrix[3] - matrix[1],
			matrix[7] - matrix[5],
			matrix[11] - matrix[9],
			matrix[15] - matrix[13]);

		new_plane.normal = -new_plane.normal;
		new_plane.normalize();

		planes[3] = p_transform.xform(new_plane);

		///////--- Right Plane ---///////
		new_plane = Plane(matrix[3] - matrix[0],
			matrix[7] - matrix[4],
			matrix[11] - matrix[8],
			matrix[15] - matrix[12]);

		new_plane.normal = -new_plane.normal;
		new_plane.normalize();

		planes[4] = p_transform.xform(new_plane);

		///////--- Bottom Plane ---///////
		new_plane = Plane(matrix[3] + matrix[1],
			matrix[7] + matrix[5],
			matrix[11] + matrix[9],
			matrix[15] + matrix[13]);

		new_plane.normal = -new_plane.normal;
		new_plane.normalize();

		planes[5] = p_transform.xform(new_plane);

		return planes;
	}

	bool Projection::get_endpoints(const Transform3D& p_transform, Vector3* p_8points) const {
		std::vector<Plane> planes = get_projection_planes(Transform3D());
		const Planes intersections[8][3] = {
			{ PLANE_FAR, PLANE_LEFT, PLANE_TOP },
			{ PLANE_FAR, PLANE_LEFT, PLANE_BOTTOM },
			{ PLANE_FAR, PLANE_RIGHT, PLANE_TOP },
			{ PLANE_FAR, PLANE_RIGHT, PLANE_BOTTOM },
			{ PLANE_NEAR, PLANE_LEFT, PLANE_TOP },
			{ PLANE_NEAR, PLANE_LEFT, PLANE_BOTTOM },
			{ PLANE_NEAR, PLANE_RIGHT, PLANE_TOP },
			{ PLANE_NEAR, PLANE_RIGHT, PLANE_BOTTOM },
		};

		for (int i = 0; i < 8; i++) {
			glm::vec3 point;
			Plane a = planes[intersections[i][0]];
			Plane b = planes[intersections[i][1]];
			Plane c = planes[intersections[i][2]];
			bool res = a.intersect_3(b, c, &point);
			ERR_FAIL_COND_V(!res, false);
			p_8points[i] = p_transform.xform(point);
		}

		return true;
	}


	void Projection::set_identity() {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				columns[i][j] = (i == j) ? 1 : 0;
			}
		}
	}
}
