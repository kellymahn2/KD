#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <array>

namespace Kaidel {
	class FrustumCuller {
	public:

		FrustumCuller() = default;
		// Construct with view-projection matrix (projection * view)
		explicit FrustumCuller(const glm::mat4& viewProj)
			: m_viewProj(viewProj) {

			m_Planes[LEFT].x = viewProj[0].w + viewProj[0].x;
			m_Planes[LEFT].y = viewProj[1].w + viewProj[1].x;
			m_Planes[LEFT].z = viewProj[2].w + viewProj[2].x;
			m_Planes[LEFT].w = viewProj[3].w + viewProj[3].x;

			m_Planes[RIGHT].x = viewProj[0].w - viewProj[0].x;
			m_Planes[RIGHT].y = viewProj[1].w - viewProj[1].x;
			m_Planes[RIGHT].z = viewProj[2].w - viewProj[2].x;
			m_Planes[RIGHT].w = viewProj[3].w - viewProj[3].x;

			m_Planes[TOP].x = viewProj[0].w - viewProj[0].y;
			m_Planes[TOP].y = viewProj[1].w - viewProj[1].y;
			m_Planes[TOP].z = viewProj[2].w - viewProj[2].y;
			m_Planes[TOP].w = viewProj[3].w - viewProj[3].y;

			m_Planes[BOTTOM].x = viewProj[0].w + viewProj[0].y;
			m_Planes[BOTTOM].y = viewProj[1].w + viewProj[1].y;
			m_Planes[BOTTOM].z = viewProj[2].w + viewProj[2].y;
			m_Planes[BOTTOM].w = viewProj[3].w + viewProj[3].y;

			m_Planes[BACK].x = viewProj[0].w + viewProj[0].z;
			m_Planes[BACK].y = viewProj[1].w + viewProj[1].z;
			m_Planes[BACK].z = viewProj[2].w + viewProj[2].z;
			m_Planes[BACK].w = viewProj[3].w + viewProj[3].z;

			m_Planes[FRONT].x = viewProj[0].w - viewProj[0].z;
			m_Planes[FRONT].y = viewProj[1].w - viewProj[1].z;
			m_Planes[FRONT].z = viewProj[2].w - viewProj[2].z;
			m_Planes[FRONT].w = viewProj[3].w - viewProj[3].z;

			for (auto i = 0; i < m_Planes.size(); i++)
			{
				float length = sqrtf(m_Planes[i].x * m_Planes[i].x + m_Planes[i].y * m_Planes[i].y + m_Planes[i].z * m_Planes[i].z);
				m_Planes[i] /= length;
			}
		}

		bool IsVisiblePlanes(const glm::mat4& model, const glm::vec3& aabbMin, const glm::vec3& aabbMax)
		{
			glm::vec3 corners[8] = {
				glm::vec3(aabbMin.x, aabbMin.y, aabbMin.z),
				glm::vec3(aabbMax.x, aabbMin.y, aabbMin.z),
				glm::vec3(aabbMin.x, aabbMax.y, aabbMin.z),
				glm::vec3(aabbMax.x, aabbMax.y, aabbMin.z),
				glm::vec3(aabbMin.x, aabbMin.y, aabbMax.z),
				glm::vec3(aabbMax.x, aabbMin.y, aabbMax.z),
				glm::vec3(aabbMin.x, aabbMax.y, aabbMax.z),
				glm::vec3(aabbMax.x, aabbMax.y, aabbMax.z)
			};

			glm::vec3 worldCorners[8];
			for (int i = 0; i < 8; ++i) {
				glm::vec4 c = model * glm::vec4(corners[i], 1.0f);
				worldCorners[i] = glm::vec3(c) / c.w; // in case of perspective, but for affine transforms w should be 1
			}

			for (int i = 0; i < 6; i++)
			{
				int out = 0;
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[0].x, worldCorners[0].y, worldCorners[0].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[1].x, worldCorners[1].y, worldCorners[1].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[2].x, worldCorners[2].y, worldCorners[2].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[3].x, worldCorners[3].y, worldCorners[3].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[4].x, worldCorners[4].y, worldCorners[4].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[5].x, worldCorners[5].y, worldCorners[5].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[6].x, worldCorners[6].y, worldCorners[6].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[7].x, worldCorners[7].y, worldCorners[7].z, 1.0f)) < 0.0) ? 1 : 0);
				if (out == 8) return false;
			}

			return true;
		}

		bool IsVisibleNoZPlane(const glm::mat4& model, const glm::vec3& aabbMin, const glm::vec3& aabbMax) const
		{
			// Local-space corners
			glm::vec3 localCorners[8] = {
				{aabbMin.x, aabbMin.y, aabbMin.z},
				{aabbMax.x, aabbMin.y, aabbMin.z},
				{aabbMin.x, aabbMax.y, aabbMin.z},
				{aabbMax.x, aabbMax.y, aabbMin.z},
				{aabbMin.x, aabbMin.y, aabbMax.z},
				{aabbMax.x, aabbMin.y, aabbMax.z},
				{aabbMin.x, aabbMax.y, aabbMax.z},
				{aabbMax.x, aabbMax.y, aabbMax.z}
			};

			// Transform corners to world space (model only)
			glm::vec4 worldCorners[8];
			for (int i = 0; i < 8; ++i) {
				worldCorners[i] = model * glm::vec4(localCorners[i], 1.0f);
			}

			// Test only the four side planes (ignore FRONT/ BACK)
			for (int i = 0; i < 4; i++)
			{
				int out = 0;
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[0].x, worldCorners[0].y, worldCorners[0].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[1].x, worldCorners[1].y, worldCorners[1].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[2].x, worldCorners[2].y, worldCorners[2].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[3].x, worldCorners[3].y, worldCorners[3].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[4].x, worldCorners[4].y, worldCorners[4].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[5].x, worldCorners[5].y, worldCorners[5].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[6].x, worldCorners[6].y, worldCorners[6].z, 1.0f)) < 0.0) ? 1 : 0);
				out += ((glm::dot(m_Planes[i], glm::vec4(worldCorners[7].x, worldCorners[7].y, worldCorners[7].z, 1.0f)) < 0.0) ? 1 : 0);
				if (out == 8) return false;
			}

			// Not culled by any of the four side planes -> considered visible in XY (NoZ)
			return true;
		}

		bool IsVisibleNoZ(const glm::mat4& model, const glm::vec3& aabbMin, const glm::vec3& aabbMax) const
		{
			glm::vec3 corners[8] = {
				glm::vec3(aabbMin.x, aabbMin.y, aabbMin.z),
				glm::vec3(aabbMax.x, aabbMin.y, aabbMin.z),
				glm::vec3(aabbMin.x, aabbMax.y, aabbMin.z),
				glm::vec3(aabbMax.x, aabbMax.y, aabbMin.z),
				glm::vec3(aabbMin.x, aabbMin.y, aabbMax.z),
				glm::vec3(aabbMax.x, aabbMin.y, aabbMax.z),
				glm::vec3(aabbMin.x, aabbMax.y, aabbMax.z),
				glm::vec3(aabbMax.x, aabbMax.y, aabbMax.z)
			};

			glm::vec3 worldCorners[8];
			for (int i = 0; i < 8; ++i) {
				glm::vec4 c = m_viewProj * model * glm::vec4(corners[i], 1.0f);
				worldCorners[i] = glm::vec3(c) / c.w; // in case of perspective, but for affine transforms w should be 1
			}

			for (uint32_t i = 0; i < 8; ++i)
			{
				if (worldCorners[i].x >= -1.0f && worldCorners[i].x <= 1.0f &&
					worldCorners[i].y >= -1.0f && worldCorners[i].y <= 1.0f)
					return true;
			}

			return false;
		}
		// Test if an AABB (defined in world space by transformed model) is visible.
		// model: world transform for the object
		// aabbMin, aabbMax: in model's local space; they will be transformed by model to world space
		bool IsVisible(const glm::mat4& model, const glm::vec3& aabbMin, const glm::vec3& aabbMax) const {
			// Transform the AABB from local to world space by the model matrix.
			// To avoid transforming all 8 corners every time (which is somewhat expensive),
			// we compute the transformed AABB by transforming the center and extents.
			// However, for simplicity and correctness, here we transform all 8 corners and
			// compute the world-space AABB.

			glm::vec3 corners[8] = {
				glm::vec3(aabbMin.x, aabbMin.y, aabbMin.z),
				glm::vec3(aabbMax.x, aabbMin.y, aabbMin.z),
				glm::vec3(aabbMin.x, aabbMax.y, aabbMin.z),
				glm::vec3(aabbMax.x, aabbMax.y, aabbMin.z),
				glm::vec3(aabbMin.x, aabbMin.y, aabbMax.z),
				glm::vec3(aabbMax.x, aabbMin.y, aabbMax.z),
				glm::vec3(aabbMin.x, aabbMax.y, aabbMax.z),
				glm::vec3(aabbMax.x, aabbMax.y, aabbMax.z)
			};

			glm::vec3 worldCorners[8];
			for (int i = 0; i < 8; ++i) {
				glm::vec4 c = m_viewProj * model * glm::vec4(corners[i], 1.0f);
				worldCorners[i] = glm::vec3(c) / c.w; // in case of perspective, but for affine transforms w should be 1
			}

			for (uint32_t i = 0; i < 8; ++i)
			{
				if (worldCorners[i].x >= -1.0f && worldCorners[i].x <= 1.0f &&
					worldCorners[i].y >= -1.0f && worldCorners[i].y <= 1.0f &&
					worldCorners[i].z >= 0.0f && worldCorners[i].z <= 1.0f)
					return true;
			}

			return false;
		}

	private:
		glm::mat4 m_viewProj;
		enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, BACK = 4, FRONT = 5 };
		std::array<glm::vec4, 6> m_Planes;
	};

}
