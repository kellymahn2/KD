#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"


#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/2D\Renderer2D.h"
#include "Kaidel/Renderer/3D\Renderer3D.h"
#include "Kaidel/Core/JobSystem.h"
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Renderer/3D/Light.h"
#include "Kaidel/Renderer/3D/Material.h"

#include <glm/gtx/compatibility.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
namespace Kaidel {

	SceneRenderer::SceneRenderer(void* scene)
		:m_Context(scene)
	{
	}

	void SceneRenderer::Reset()
	{
	}
	


	void SceneRenderer::Render()
	{
		
	}

}
