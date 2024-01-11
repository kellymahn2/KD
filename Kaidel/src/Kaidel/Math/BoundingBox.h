#pragma once
#include <glm/glm.hpp>
#include <array>
namespace Math {

	struct BoundingBox {
		glm::vec3 MinPoint;
		glm::vec3 MaxPoint;

		inline bool IsInside(const glm::vec3& point)const {
			return point.x >= MinPoint.x && point.x <= MaxPoint.x &&
				point.y >= MinPoint.y && point.y <= MaxPoint.y &&
				point.z >= MinPoint.z && point.z <= MaxPoint.z;
		}
		inline bool Intersects(const BoundingBox& other) const {
			return (MinPoint.x <= other.MaxPoint.x && MaxPoint.x >= other.MinPoint.x) &&
				(MinPoint.y <= other.MaxPoint.y && MaxPoint.y >= other.MinPoint.y) &&
				(MinPoint.z <= other.MaxPoint.z && MaxPoint.z >= other.MinPoint.z);
		}
		BoundingBox Transform(const glm::mat4& transformMatrix) const {
			glm::vec3 transformedMin = glm::vec3(transformMatrix * glm::vec4(MinPoint, 1.0f));
			glm::vec3 transformedMax = glm::vec3(transformMatrix * glm::vec4(MaxPoint, 1.0f));
			glm::vec3 newMin = glm::min(transformedMin, transformedMax);
			glm::vec3 newMax = glm::max(transformedMin, transformedMax);
			return BoundingBox{ newMin, newMax };
		}
	};


	struct Frustum {
		glm::vec4 Left, Right, Top, Bottom, Near, Far;
		Frustum() = default;
		Frustum(const glm::mat4& viewProjMatrix) {
			// Left plane
			Left = viewProjMatrix[3] + viewProjMatrix[0];
			// Right plane
			Right = viewProjMatrix[3] - viewProjMatrix[0];
			// Top plane
			Top = viewProjMatrix[3] - viewProjMatrix[1];
			// Bottom plane
			Bottom = viewProjMatrix[3] + viewProjMatrix[1];
			// Near plane
			Near = viewProjMatrix[3] + viewProjMatrix[2];
			// Far plane
			Far = viewProjMatrix[3] - viewProjMatrix[2];


			//Normalize the planes
			normalizePlane(Left);
			normalizePlane(Right);
			normalizePlane(Top);
			normalizePlane(Bottom);
			normalizePlane(Near);
			normalizePlane(Far);

		}

		inline bool IsCulled(const BoundingBox& boundingBox)const {
			glm::vec3 corners[8] = {
			{boundingBox.MinPoint.x, boundingBox.MinPoint.y, boundingBox.MinPoint.z},
			{boundingBox.MinPoint.x, boundingBox.MinPoint.y, boundingBox.MaxPoint.z},
			{boundingBox.MinPoint.x, boundingBox.MaxPoint.y, boundingBox.MinPoint.z},
			{boundingBox.MinPoint.x, boundingBox.MaxPoint.y, boundingBox.MaxPoint.z},
			{boundingBox.MaxPoint.x, boundingBox.MinPoint.y, boundingBox.MinPoint.z},
			{boundingBox.MaxPoint.x, boundingBox.MinPoint.y, boundingBox.MaxPoint.z},
			{boundingBox.MaxPoint.x, boundingBox.MaxPoint.y, boundingBox.MinPoint.z},
			{boundingBox.MaxPoint.x, boundingBox.MaxPoint.y, boundingBox.MaxPoint.z},
			};

			for (const auto& corner : corners) {
				bool insideFrustum = true;
				for (const auto& plane : { Left, Right, Top, Bottom, Near, Far }) {
					if (glm::dot(plane, glm::vec4(corner, 1.0f)) < 0) {
						insideFrustum = false;
						break;
					}
				}

				if (insideFrustum) {
					return false;
				}
			}

			return true;
		}

	private:
		void  normalizePlane(glm::vec4& plane) {
			float length = glm::length(glm::vec3(plane));
			plane /= length;
		}
	
	};



}
