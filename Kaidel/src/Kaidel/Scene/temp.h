#pragma once

#include "Kaidel/Core/Base.h"
#include "SceneRenderer.h"
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <vector>

#undef near
#undef far

namespace Kaidel {

	struct DirectionalLightCascadeShadowData {
		glm::vec4 CascadeOffsets;
		glm::vec4 CascadeScales;
		glm::mat4 ShadowMatrix;
	};
	struct DirectionalLightShadowData {
		glm::mat4 GlobalShadowMatrix;
		std::vector<DirectionalLightCascadeShadowData> Cascades;
	};


	static void GetEndpoints(const glm::mat4& invViewProjMatrix, glm::vec3* endpoints) {

		endpoints[0] = glm::vec3(-1.0f, 1.0f, 0.0f);
		endpoints[1] = glm::vec3(1.0f, 1.0f, 0.0f);
		endpoints[2] = glm::vec3(1.0f, -1.0f, 0.0f);
		endpoints[3] = glm::vec3(-1.0f, -1.0f, 0.0f);
		endpoints[4] = glm::vec3(-1.0f, 1.0f, 1.0f);
		endpoints[5] = glm::vec3(1.0f, 1.0f, 1.0f);
		endpoints[6] = glm::vec3(1.0f, -1.0f, 1.0f);
		endpoints[7] = glm::vec3(-1.0f, -1.0f, 1.0f);

		for (int i = 0; i < 8; ++i) {
			glm::vec4 corner = invViewProjMatrix * glm::vec4(endpoints[i], 1.0f);
			endpoints[i] = corner / corner.w;
		}
	}

	static glm::vec3 GetCenter(const glm::vec3* endpoints) {
		glm::vec3 center = glm::vec3(0, 0, 0);
		for (int i = 0; i < 8; ++i) {
			center += endpoints[i];
		}

		return center / 8.0f;
	}

	static float GetRadius(const glm::vec3* endpoints, const glm::vec3& center) {
		float radius = 0.0f;

		for (int i = 0; i < 8; ++i) {
			float d = glm::length(endpoints[i] - center);
			if (d > radius)
				radius = d;
		}

		return radius;
	}

	double Snapped(double p_value, double p_step) {
		if (p_step != 0) {
			p_value = glm::floor(p_value / p_step + 0.5) * p_step;
		}
		return p_value;
	}

	static glm::mat4 CreateLightViewMatrix(const glm::vec3& origin, const glm::mat4& rotation) {
		glm::mat4 view = rotation;
		view[3] += glm::vec4(origin, 0.0f);

		return glm::inverse(view);
	}

	static glm::vec3 Transform(const glm::vec3& v, const glm::mat4& mat) {
		glm::vec4 vec = mat * glm::vec4(v, 1.0f);
		return vec / vec.w;
	}

	// Makes the "global" shadow matrix used as the reference point for the cascades
	static glm::mat4 MakeGlobalShadowMatrix(const glm::mat4& invViewProj, const glm::vec3& lightDirection)
	{
		// Get the 8 points of the view frustum in world space
		glm::vec3 frustumCorners[8] =
		{
			glm::vec3(-1.0f,  1.0f, 0.0f),
			glm::vec3(1.0f,  1.0f, 0.0f),
			glm::vec3(1.0f, -1.0f, 0.0f),
			glm::vec3(-1.0f, -1.0f, 0.0f),
			glm::vec3(-1.0f,  1.0f, 1.0f),
			glm::vec3(1.0f,  1.0f, 1.0f),
			glm::vec3(1.0f, -1.0f, 1.0f),
			glm::vec3(-1.0f, -1.0f, 1.0f),
		};

		glm::vec3 frustumCenter(0.0f);
		for (uint32_t i = 0; i < 8; ++i)
		{
			frustumCorners[i] = Transform(frustumCorners[i], invViewProj);
			frustumCenter += frustumCorners[i];
		}

		frustumCenter /= 8.0f;

		// Pick the up vector to use for the light camera
		glm::vec3 upDir = glm::vec3(0.0f, 1.0f, 0.0f);

		// Create a temporary view matrix for the light
		glm::vec3 lightCameraPos = frustumCenter;
		glm::vec3 lookAt = frustumCenter + lightDirection;
		glm::mat4 lightView = glm::lookAt(lightCameraPos, lookAt, upDir);

		// Get position of the shadow camera
		glm::vec3 shadowCameraPos = frustumCenter - lightDirection * 0.5f;

		// Come up with a new orthographic camera for the shadow caster
		glm::mat4 shadowCamera = glm::ortho(-.5f, .5f, -.5f,
											.5f, 0.0f, 1.0f);

		glm::mat4 shadowMatrix = shadowCamera * glm::lookAt(shadowCameraPos, frustumCenter, upDir);

		glm::mat4 texScaleBias = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 1.0f));
		texScaleBias[3] = glm::vec4(0.5f, 0.5f, 0.0f,1.0f);
		return texScaleBias * shadowMatrix;
	}

	static DirectionalLightShadowData ComputeLightViewProjections(
		const SceneData& sceneData,
		const std::vector<float> cascadeSplits,
		const glm::mat4& lightRotationMatrix,
		int shadowMapResolution) 
	{
		DirectionalLightShadowData res;
		res.Cascades.resize(cascadeSplits.size());

		glm::vec3 lightDir = -glm::normalize(glm::vec3(lightRotationMatrix[2]));

		glm::mat4 invViewProj = glm::inverse(sceneData.ViewProj);
		
		glm::mat4 globalShadowMatrix = MakeGlobalShadowMatrix(invViewProj, lightDir);

		res.GlobalShadowMatrix = globalShadowMatrix;

		for (uint32_t cascadeIdx = 0; cascadeIdx < cascadeSplits.size(); ++cascadeIdx) {
			glm::vec3 frustumCornersWS[8] =
			{
				glm::vec3(-1.0f,  1.0f, 0.0f),
				glm::vec3(1.0f,  1.0f, 0.0f),
				glm::vec3(1.0f, -1.0f, 0.0f),
				glm::vec3(-1.0f, -1.0f, 0.0f),
				glm::vec3(-1.0f,  1.0f, 1.0f),
				glm::vec3(1.0f,  1.0f, 1.0f),
				glm::vec3(1.0f, -1.0f, 1.0f),
				glm::vec3(-1.0f, -1.0f, 1.0f),
			};

			float prevSplitDist = cascadeIdx == 0 ?  0.0f: cascadeSplits[cascadeIdx - 1];
			float splitDist = cascadeSplits[cascadeIdx];


			for (uint32_t i = 0; i < 8; ++i) {
				glm::vec4 corner = invViewProj * glm::vec4(frustumCornersWS[i], 1.0f);
				frustumCornersWS[i] = corner / corner.w;
			}

			// Get the corners of the current cascade slice of the view frustum
			for (uint32_t i = 0; i < 4; ++i)
			{
				glm::vec3 cornerRay = frustumCornersWS[i + 4] - frustumCornersWS[i];
				glm::vec3 nearCornerRay = cornerRay * prevSplitDist;
				glm::vec3 farCornerRay = cornerRay * splitDist;
				frustumCornersWS[i + 4] = frustumCornersWS[i] + farCornerRay;
				frustumCornersWS[i] = frustumCornersWS[i] + nearCornerRay;
			}

			// Calculate the centroid of the view frustum slice
			glm::vec3 frustumCenter(0.0f);
			for (uint32_t i = 0; i < 8; ++i)
				frustumCenter = frustumCenter + frustumCornersWS[i];
			frustumCenter *= 1.0f / 8.0f;

			glm::vec3 upDir;
			glm::vec3 minExtents;
			glm::vec3 maxExtents;
			{
				// This needs to be constant for it to be stable
				upDir = glm::vec3(0.0f, 1.0f, 0.0f);

				// Calculate the radius of a bounding sphere surrounding the frustum corners
				float sphereRadius = 0.0f;
				for (uint32_t i = 0; i < 8; ++i)
				{
					float dist = glm::length(frustumCornersWS[i] - frustumCenter);
					sphereRadius = std::max(sphereRadius, dist);
				}

				sphereRadius = std::ceil(sphereRadius * 16.0f) / 16.0f;

				maxExtents = glm::vec3(sphereRadius, sphereRadius, sphereRadius);
				minExtents = -maxExtents;
			}

			glm::vec3 cascadeExtents = maxExtents - minExtents;

			// Get position of the shadow camera
			glm::vec3 shadowCameraPos = frustumCenter - lightDir * -minExtents.z;

			glm::mat4 shadowCamera = glm::ortho(minExtents.x, maxExtents.x, minExtents.y,
											maxExtents.y, 0.0f, cascadeExtents.z);
			//KD_CORE_INFO("{}", cascadeExtents.z);

			glm::mat4 shadowLookAt = glm::lookAt(shadowCameraPos, frustumCenter, upDir);

			glm::mat4 shadowMatrix = shadowCamera * shadowLookAt;
			{
				// Create the rounding matrix, by projecting the world-space origin and determining
				// the fractional offset in texel space
				glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				shadowOrigin = shadowMatrix * shadowOrigin;
				shadowOrigin = shadowOrigin * (float)shadowMapResolution / 2.0f;

				glm::vec4 roundedOrigin = glm::round(shadowOrigin);
				glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
				roundOffset = roundOffset * 2.0f / (float)shadowMapResolution;
				roundOffset.z = 0.0f;
				roundOffset.w = 0.0f;

				glm::mat4 shadowProj = shadowCamera;
				shadowProj[3] += roundOffset;
				shadowCamera = shadowProj;
			}

			shadowMatrix = shadowCamera * shadowLookAt;

			glm::mat4 texScaleBias;
			texScaleBias[0] = glm::vec4(0.5f, 0.0f, 0.0f, 0.0f);	
			texScaleBias[1] = glm::vec4(0.0f, 0.5f, 0.0f, 0.0f);
			texScaleBias[2] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
			texScaleBias[3] = glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
			shadowMatrix = texScaleBias * shadowMatrix;

			// Calculate the position of the lower corner of the cascade partition, in the UV space
			// of the first cascade partition
			glm::mat4 invCascadeMat = glm::inverse(shadowMatrix);
			glm::vec3 cascadeCorner = Transform(glm::vec3(0.0f),invCascadeMat);
			
			cascadeCorner = Transform(cascadeCorner, globalShadowMatrix);

			// Do the same for the upper corner
			glm::vec3 otherCorner = Transform(glm::vec3(1.0f, 1.0f, 1.0f), invCascadeMat);
			otherCorner = Transform(otherCorner, globalShadowMatrix);

			// Calculate the scale and offset
			glm::vec3 cascadeScale = glm::vec3(1.0f, 1.0f, 1.0f) / (otherCorner - cascadeCorner);
			
			res.Cascades[cascadeIdx].CascadeOffsets = glm::vec4(-cascadeCorner, 0.0f);
			res.Cascades[cascadeIdx].CascadeScales= glm::vec4(cascadeScale, 1.0f);
			res.Cascades[cascadeIdx].ShadowMatrix = shadowCamera * shadowLookAt;
		}
		
		return res;
	}
}
