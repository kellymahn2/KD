#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"


#include "Kaidel/Renderer/UniformBuffer.h"
#include "Kaidel\Renderer\Renderer2D.h"
#include "Kaidel\Renderer\Renderer3D.h"
#include "Kaidel\Core\JobSystem.h"
#include "Kaidel/Core/Timer.h"

#include "Kaidel/Renderer/Material.h"
#include "Kaidel/Renderer/Light.h"
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
		m_QuadInsertIndex = 0;
		m_CircleInsertIndex = 0;
		m_LineInsertIndex = 0;
	}

	void Tesselate() {


	}
	
	void SceneRenderer::Render()
	{
		
	}

}
