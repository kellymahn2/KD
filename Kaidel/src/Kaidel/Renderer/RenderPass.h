#pragma once
#include <vector>
#include "Kaidel/Scene/Scene.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "UniformBuffer.h"
#include "Shader.h"
namespace Kaidel {
	typedef int GeometryType;

	struct ShadowPassConfig{
		Ref<Scene> Scene;
	};

	struct ShadowPass {
		ShadowPassConfig PassConfig;
		void Render();
		void DrawCubes(Ref<Scene> scene);
		void RenderSpotLights();
	};

	struct SceneCompositePass {

	};



	struct GeometryPassConfig {
		Ref<Scene> Scene;
		const glm::mat4& ViewProjectionMatrix;
		const glm::vec3& CameraPosition;
	};

	struct GeometryPass {
		enum GeometryType_ {
			GeometryType_None = 0,
			GeometryType_2D = 1<<0,
			GeometryType_3D = 1<<1
		};
		GeometryType GeometryType;
		GeometryPassConfig PassConfig;

		void DrawQuads(Ref<Scene> scene);
		void DrawCircles(Ref<Scene> scene);
		void DrawLines(Ref<Scene> scene);
		void DrawCubes(Ref<Scene> scene);

		void Render();
	};
}
