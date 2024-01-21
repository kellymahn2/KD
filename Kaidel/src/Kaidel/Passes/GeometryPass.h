#pragma once
#include "Kaidel/Scene/Scene.h"
#include "Kaidel/Renderer/Framebuffer.h"
#include "Kaidel/Renderer/3D/Renderer3D.h"


namespace Kaidel {


	struct GeometryPassInput {
		Ref<Scene> Scene;
		glm::mat4 CameraVP;
		glm::vec3 CameraPosition;
		Ref<Framebuffer> OutputBuffer;
	};

	struct GeometryPassOutput {

	};

	struct GeometryPass {
		GeometryPassInput Input;
		GeometryPassOutput Output;
		
		void Apply() {
			Renderer3DBeginData beginData;
			beginData.CameraPosition = Input.CameraPosition;
			beginData.CameraVP = Input.CameraVP;
			Renderer3D::Begin(beginData);
		}
	};

}
