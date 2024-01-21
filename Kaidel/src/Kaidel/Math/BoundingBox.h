#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
namespace Math{
	// Definition of the AABB struct
	struct AABB {
		glm::vec3 Min;  // Minimum corner of the AABB
		glm::vec3 Max;  // Maximum corner of the AABB
	};
	struct Frustum {
		glm::vec4 Planes[6];  // Array to store the six frustum planes
		glm::mat4 ViewProjectionMatrix;
		// Method to extract frustum planes from Projection * View matrix
		void ExtractPlanesFromProjectionView(const glm::mat4& viewProjectionMatrix) {
			Planes[0] = viewProjectionMatrix[3] + viewProjectionMatrix[0];  // Left
			Planes[1] = viewProjectionMatrix[3] - viewProjectionMatrix[0];  // Right
			Planes[2] = viewProjectionMatrix[3] - viewProjectionMatrix[1];  // Bottom
			Planes[3] = viewProjectionMatrix[3] + viewProjectionMatrix[1];  // Top
			Planes[4] = viewProjectionMatrix[3] - viewProjectionMatrix[2];  // Near
			Planes[5] = viewProjectionMatrix[3] + viewProjectionMatrix[2];  // Far

			// Normalize the plane normals
			for (int i = 0; i < 6; ++i) {
				Planes[i] /= glm::length(glm::vec3(Planes[i]));
			}
			ViewProjectionMatrix = viewProjectionMatrix;
		}

		// Method to check if AABB is in frustum with object transformation
		bool IsAABBInFrustum(const glm::mat4& modelMatix, const AABB& aabb) const {
			// Transform the AABB corners by the object's transformation matrix
			glm::vec4 Corners[8];
			Corners[0] = ViewProjectionMatrix * modelMatix * glm::vec4(aabb.Min.x, aabb.Min.y, aabb.Min.z, 1.0f);
			Corners[1] = ViewProjectionMatrix * modelMatix * glm::vec4(aabb.Max.x, aabb.Min.y, aabb.Min.z, 1.0f);
			Corners[2] = ViewProjectionMatrix * modelMatix * glm::vec4(aabb.Min.x, aabb.Max.y, aabb.Min.z, 1.0f);
			Corners[3] = ViewProjectionMatrix * modelMatix * glm::vec4(aabb.Max.x, aabb.Max.y, aabb.Min.z, 1.0f);
			Corners[4] = ViewProjectionMatrix * modelMatix * glm::vec4(aabb.Min.x, aabb.Min.y, aabb.Max.z, 1.0f);
			Corners[5] = ViewProjectionMatrix * modelMatix * glm::vec4(aabb.Max.x, aabb.Min.y, aabb.Max.z, 1.0f);
			Corners[6] = ViewProjectionMatrix * modelMatix * glm::vec4(aabb.Min.x, aabb.Max.y, aabb.Max.z, 1.0f);
			Corners[7] = ViewProjectionMatrix * modelMatix * glm::vec4(aabb.Max.x, aabb.Max.y, aabb.Max.z, 1.0f);

			for (uint32_t i = 0; i < 8; ++i) {
				Corners[i] /= Corners[i].w;
			}
			// Check each frustum plane against the transformed AABB
			for (int i = 0; i < 6; ++i) {
				int InsideCount = 8;  // Number of AABB corners inside the frustum plane

				for (int j = 0; j < 8; ++j) {
					if (glm::dot(glm::vec3(Planes[i]), glm::vec3(Corners[j])) + Planes[i].w < 0.0f) {
						// AABB corner is outside the frustum plane
						InsideCount--;
					}
				}

				if (InsideCount == 0) {
					// AABB is completely outside the frustum
					return false;
				}
			}

			// AABB is at least partially inside the frustum
			return true;
		}
	};

}
